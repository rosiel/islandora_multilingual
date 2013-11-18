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
        if ($value == '0') {
            $object->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOfCollection',  $pid);
        }
        else {
            $already_set = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $pid);
            if ( empty($already_set)) {
                $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $pid);
            }
        }
            
    }
    $form_state['redirect'] = "islandora/object/{$object->id}/manage";
    return;
}


