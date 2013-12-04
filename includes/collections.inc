<?php

function islandora_multilingual_collection_editor(array $form, array &$form_state, $object) {

    $form_state['islandora_multilingual']['object'] = $object;

    $relationships = ($object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection'));
    $in_collections = array();
    foreach ($relationships as $relationship) {
        $in_collections[] = $relationship['object']['value'];
    }
    $query = 'SELECT $object $title
        FROM <#ri>
        WHERE {
            $object <fedora-model:label> $title ;
            <fedora-model:hasModel> <info:fedora/islandora:collectionCModel> ;
            <fedora-model:state> <fedora-model:Active>
        } ORDER BY $object';


    $tuque = islandora_get_tuque_connection();
    $options_keywords = array();
    $options_series= array();
    if ($tuque) {
        try {
            $results = $tuque->repository->ri->query($query, 'sparql');
            foreach ($results as $result) {
                $pid = $result['object']['value'];
                $label = $result['title']['value'];
                $namespace = explode(':', $pid); 
                if ($namespace[0] == 'pubs' and $pid != 'pubs:collection') {
                    $options_keywords[$pid] = $label . ' (' . $pid . ')';
                }
                else if ($namespace[0] == 'series') {
                    $options_series[$pid] = $label . ' (' . $pid . ')';
                }
            }
        }
        catch (Exception $e) {
            if ($e->getCode() == '404') {
                return FALSE;
            }
            else {
                return NULL;
            }
        }
    }
    else {
        IslandoraTuque::getError();
    }

    $form['wrapper'] = array(
        '#title' => 'Collections',
        '#type' => 'fieldset',
    );
    $form['wrapper']['keywords'] = array(
        '#title' => 'Select collections - keywords',
        '#type' => 'checkboxes',
        '#options' => $options_keywords,
        '#default_value' => $in_collections,
    );
    $form['wrapper']['series'] = array(
        '#title' => 'Select collections - series',
        '#type' => 'checkboxes',
        '#options' => $options_series,
        '#default_value' => $in_collections,
    );

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Save'),
    );

    return $form;
}

function islandora_multilingual_collection_editor_submit($form, &$form_state) {
    $object = $form_state['islandora_multilingual']['object'];
    foreach(array_merge($form_state['values']['keywords'], $form_state['values']['series']) as $pid => $value) {
        if (in_array($pid, array('pubs:ecosystems', 'pubs:greeneconomy', 'pubs:enforcement', 'pubs:climatechange', 'pubs:pollutants'))) {
            $is_theme = True;
        }
        else {
            $is_theme = False;
        }
        if ($value == '0') {
            $object->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOfCollection',  $pid);
            if ($is_theme) {
                islandora_multilingual_collection_set_mods($object, $pid, 'theme', 'delete');
            } 
            else {
                islandora_multilingual_collection_set_mods($object, $pid, 'series', 'delete');
            }
            // Remove keyword from all three languages' mods streams. Leave master as no.
        }
        else {
            $already_set = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $pid);
            if ( empty($already_set) ) {
                $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $pid);
            }
            
            if ($is_theme) {
                islandora_multilingual_collection_set_mods($object, $pid, 'theme', 'add');
            } 
            else {
                islandora_multilingual_collection_set_mods($object, $pid, 'series', 'add');
            }
            
        }
            
    }
    $form_state['redirect'] = "islandora/object/{$object->id}/manage";
    return;
}

function islandora_multilingual_collection_set_mods(FedoraObject $object, $collectionpid, $type, $action) {
    if (!defined('MODS')) define('MODS', "http://www.loc.gov/mods/v3");
    $collection = islandora_object_load($collectionpid);
    if (!$collection) {
        watchdog('islandora_multilingual', 'Collection object ' . $collectionpid . ' not set.');
        return False;
    }
    $languages = explode(',', variable_get('islandora_languages', 'EN'));
    if ($type == 'theme') {
        foreach ($languages as $lang) {
            if (!isset($object['MODS-' . $lang])) {
                continue;
            }
            if (!isset($collection['MODS-' . $lang])) {
                continue;
            }

            # Extract the "title" of the collection in this language.
            $xml_collection = new DOMDocument();
            $xml_collection->loadXML($collection['MODS-'.$lang]->content);
            $mods_collection = $xml_collection->documentElement;
            $xpath_collection = new DOMXpath($xml_collection);
            $xpath_collection->registerNamespace("mods", "http://www.loc.gov/mods/v3");
            $title = $xpath_collection->query('/mods:mods/mods:titleInfo/mods:title');
            if ($title->length == 0){
                watchdog('islandora_multilingual', "No title found for $lang in $collectionpid");
                continue;
            }
            $title = $title->item(0)->nodeValue;

            # Find all subject/topic tags with this value.
            $xml_object = new DOMDocument();
            $xml_object->loadXML($object['MODS-'.$lang]->content);
            $mods_object = $xml_object->documentElement;
            $xpath_object = new DOMXpath($xml_object);
            $xpath_object->registerNamespace("mods", "http://www.loc.gov/mods/v3");
            $already_set = $xpath_object->query('/mods:mods/mods:subject[@displayLabel="Keyword"][@authority="local"]/mods:topic[text()="' . $title . '"]');
            if ($action == 'delete') {
                foreach($already_set as $deleteme) {
                    $mods_object->removeChild($deleteme->parentNode);
                    watchdog('debug', 'deleted ' . $title);
                }
            }
            elseif ($action == 'add') {
                if ($already_set->length > 0) {
                    continue;
                }
                $subject = $xml_object->createElementNS(MODS, 'subject');
                $subject->setAttribute('displayLabel', 'Keyword');
                $subject->setAttribute('authority', 'local');
                $mods_object->appendChild($subject);
                $topic = $xml_object->createElementNS(MODS, 'topic');
                $topic->nodeValue = $title;
                $subject->appendChild($topic);
            }

            $object['MODS-' . $lang]->setContentFromString($xml_object->saveXML());
        }
    }
    elseif ($type == 'series') {
        # Not yet implemented.
        return;
    }
}
