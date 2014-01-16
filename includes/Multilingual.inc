<?php

/**
* Returns the best available datastream suffix
* @param FedoraObject $object OR a "document" subsection of a returned Solr array.
* @param string $datastream
* returns datastream name string, including suffix, or Null if no datastream was found.
*/

function chooseLanguage(&$object, $datastream = 'ds.OBJ-') {
  global $language;
  $drupal_lang_suffix = strtoupper($language->language);
  $suffix_backup = explode(',', variable_get('islandora_languages', 'EN, ES, FR'));

  if (isset($object['fedora_datastreams_ms'])) {
      // TODO: deprecate the use of ds.XX as a query here. Then remove this if-else and replace dsname with datastream.
      if (in_array($datastream, array('ds.OBJ-', 'ds.TN-', ))) {
          $dsname = substr($datastream, 3);
      }
      else {
          $dsname = $datastream;
      }
      // Check for the datastream value in either the array ['fedora_datastreams_ms'] or ['fedora_datastreams_ms']['value'],
      // depending whether solr was called directly or through islandora.
      if (in_array($dsname . $drupal_lang_suffix, $object['fedora_datastreams_ms'])){
          return $datastream . $drupal_lang_suffix;
      }
      elseif (isset($object['fedora_datastreams_ms']['value'])) {
          if (in_array($dsname . $drupal_lang_suffix, $object['fedora_datastreams_ms']['value'])){
              return $datastream . $drupal_lang_suffix;
          }
      }

      // Cycle through the non-active languages in ['fedora_datastreams_ms'] or ['fedora_datastreams_ms']['value'],
 
      foreach ($suffix_backup as $test_suffix) {
          if (in_array($dsname . $test_suffix, $object['fedora_datastreams_ms'])) {
              return $datastream . $test_suffix;
          }
          else {
              if (isset($object['fedora_datastreams_ms']['value'])) {
                  if (in_array($dsname . $test_suffix, $object['fedora_datastreams_ms']['value'])) {
                      return $datastream . $test_suffix;
                  }
              }
          }
      }
  }
  if (isset($object[$datastream . $drupal_lang_suffix])) {
    $suffix = $drupal_lang_suffix;
  }
  else {
    foreach ($suffix_backup as $test_suffix) {
      if (isset($object[$datastream . $test_suffix])) {
        $suffix = $test_suffix;
        break;
      }
    }
  }
  if (!isset($suffix)) {
    return Null;
  }
  else{
    return $datastream . $suffix;
  }
}

/* 
 * This function, called on post-ingest for objects with CM CEC_multilingual, 
 * assumes that the FedoraObject $object has only one
 * MODS datastream. Any other MODS datastreams will be cleared.
 * 2012-12-31 now writes M instead of X datastreams.
 * 2013-02-11 now takes two arguments, the second being the datastream, and 
 */

function multilingual_create_multiple_mods_datastreams($object, $datastream) {
    if(DEBUG) {
        watchdog('sync', 'create multiple from '.$datastream->id);
    }
 $languages = explode(',', variable_get('islandora_languages', 'EN,'));
 $dsid_orig = NULL;
 $module_path = drupal_get_path('module', 'islandora_multilingual');
 $dsid_orig = $datastream->id;
 if (!isset($dsid_orig)) {
   drupal_set_message(t('No MODS datastream was set.'));
   return "";
 }

 $languages_to_set = array_diff($languages, array(substr($dsid_orig, -2)));

 foreach ($languages_to_set as $language) {
   try {
     $dsid = 'MODS-' . $language;
     if (!isset($object[$dsid])) {
       $ds_content_file = "$module_path/xml/template_MODS_$language.xml";
       $ds = $object->constructDatastream($dsid, 'M');
       $ds->label = $dsid;
       $ds->mimeType = 'text/xml';
       $ds->setContentFromString(file_get_contents($ds_content_file));
       $object->ingestDatastream($ds);
     }

   } catch (exception $e) {
     drupal_set_message(t('@message', array('@message' => check_plain($e->getMessage()))), 'error', FALSE);
   }
 }
 
}

/*
 * Synchronize data between MODS datastreams
 *
 * @param FedoraObject $object
 *   The object to synchronize
 *
 * @param Fedora[Datastream?] $master
 *   The "master" datastream, containing the information to push to the others.
 *   Datastream is marked as "master" by islandora_multilingual_datastream_form_validate.
 */
function multilingual_synchronize_mods_datastreams(FedoraObject $object, $master) {
    $dsid_orig = $master->id;
    $xml_master = new SimpleXMLElement($master->content);
    $xml_master->registerXPathNamespace("mods", "http://www.loc.gov/mods/v3");

    # Check for "master" tag to avoid infinite loops
    $is_master = $xml_master->xpath("/mods:mods/mods:note[@type='master']");
    if (empty($is_master) or (string) $is_master[0] == 'no') {
        return;
    }

    $languages = explode(',', variable_get('islandora_languages', 'EN,'));
    $master_lang = substr($dsid_orig, -2);
    $languages_to_set = array_diff($languages, array($master_lang));
    foreach ($languages_to_set as $language) {
        $sheep = $object['MODS-' . $language];
        if (abs(strtotime($master->createdDate) - strtotime($sheep->createdDate)) > 1) {
            multilingual_sync_xml($master, $sheep); # eventually make this a hook? Or break it passing the xml?
            watchdog('multilingual', "synchronized $language from " . $master->id);
        }
    }
}

/* 
 * Synchronize two MODS streams 
 *
 * FIXME: replace this with an XSLT that merges the two documents. 
 */
function multilingual_sync_xml($ds_master, $ds_sheep) {
    if (!defined('MODS')) define('MODS', "http://www.loc.gov/mods/v3");

    $xml_master = new DOMDocument();
    $xml_master->loadXML($ds_master->content);
    $xml_sheep= new DOMDocument();
    $xml_sheep->loadXML($ds_sheep->content);

    $mods_m = $xml_master->documentElement;
    $mods_s = $xml_sheep->documentElement;

    $xpath_m = new DOMXpath($xml_master);
    $xpath_m->registerNamespace("mods", "http://www.loc.gov/mods/v3");
    $xpath_s = new DOMXpath($xml_sheep);
    $xpath_s->registerNamespace("mods", "http://www.loc.gov/mods/v3");

    # Mark the synchronized (sheep) datastream as "not master".
    $synced = $xpath_s->query('/mods:mods/mods:note[@type="master"]');
    if ($synced->length == 0) {
        $synced = $xml_sheep->createElementNS(MODS, 'note');
        $synced->setAttribute('type', 'master');
        $synced->nodeValue = 'no';
        $mods_s->appendChild($synced);
    }
    else {
        $synced->item(0)->nodeValue = 'no';
    }

    # Sync label field. 
    $label_list_m = $xpath_m->query('/mods:mods/mods:note[@type="label"]');
    if ($label_list_m->length > 0) {
        $label_list_s = $xpath_s->query('/mods:mods/mods:note[@type="label"]');
        if ($label_list_s->length == 0) {
            $label_s = $xml_sheep->createElementNS(MODS, 'note');
            $label_s->setAttribute('type', 'label');
            $label_s->nodeValue = $label_list_m->item(0)->nodeValue;
            $mods_s->appendChild($label_s);
        }
        else {
            $label_list_s->item(0)->nodeValue = $label_list_m->item(0)->nodeValue;
        }
    }
    //------------- THE FOLLOWING SHOULD BE IN A HOOK -----------------

    # Remove old personal names and add new personal names.
    $names_list = $xpath_s->query('/mods:mods/mods:name[@type="personal"]');
    foreach ($names_list as $name) $mods_s->removeChild($name);

    $names_list_m = $xpath_m->query('/mods:mods/mods:name[@type="personal"]');
    foreach ($names_list_m as $name) {
        $name_s = $xml_sheep->importNode($name, TRUE); // MUST do a deep-copy.
        $mods_s->appendChild($name_s);
    }

    // Sync date field.
    $date_list_m = $xpath_m->query('/mods:mods/mods:originInfo/mods:dateIssued[@keyDate="yes"]');
    if ($date_list_m->length > 0) {
        $date_list_s = $xpath_s->query('/mods:mods/mods:originInfo/mods:dateIssued[@keyDate="yes"]');
        if ($date_list_s->length == 0) {
            $originInfo_list_s = $xpath_s->query('/mods:mods/mods:originInfo');
            if ($originInfo_list_s->length == 0) {
                $originInfo_s = $xml_sheep->createElementNS(MODS, 'originInfo');
                $mods_s->appendChild($originInfo_s);
            }
            else {
                $originInfo_s = $originInfo_list_s->item(0);
            }
            $date_s = $xml_sheep->createElementNS(MODS, 'dateIssued');
            $date_s->setAttribute('keyDate', 'yes');
            $originInfo_s->appendChild($date_s);
        }
        elseif ($date_list_s->length > 1) {
            $date_s = $date_list_s->item(0);
            # Add some interesting code to remove excess dates here fixme
        }
        else {
            $date_s = $date_list_s->item(0);
        }
        $date_s->nodeValue = $date_list_m->item(0)->nodeValue;
    }

    //--------- END OF WHAT SHOULD BE IN A HOOK -------------

    $ds_sheep->setContentFromString($xml_sheep->saveXML());
}
    
/**
 * Creates all the derivatives for this content model type
 * (copied from islandora_pdf solution pack)
 *
 * @param FedoraObject $object
 *   The object to generate/add the derivative datastreams to.
 *
 * @return boolean
 *   TRUE if all derivatives were created successfully, FALSE otherwise.
 */
function islandora_multilingual_create_all_derivatives(FedoraObject $object, $dsid = 'OBJ') {
  if (!isset($object[$dsid])) {
    drupal_set_message(t('Could not create image derivatives for %s.  No image file was uploaded as @dsid.', array('%s' => $object->id, '@dsid' => $dsid)), 'error');
    return FALSE;
  }
  $success = TRUE;
  $lang = substr($dsid, 4);
  $file_uri = islandora_multilingual_get_derivative_source_as_unmanaged_file($object, $dsid);
  $success &= islandora_multilingual_add_tn_derivative($object, $file_uri, $lang);
  //$success &= islandora_pdf_add_preview_derivative($object, $file_uri);
  file_unmanaged_delete($file_uri);
  return $success;
}

/**
 * Gets the source of all derivatives as an unmanaged file, deletion of the file is left to the caller.
 * (copied from islandora_pdf solution pack)
 *
 * @param FedoraObject $object
 *   The object to generate/add the derivative datastreams to.
 * @param string $dsid
 *   The datastream to use as the source of the derivatives
 *
 * @return string
 *   The file path to the unmanaged derivative source file.
 */
function islandora_multilingual_get_derivative_source_as_unmanaged_file(FedoraObject $object, $dsid = 'OBJ') {
  $mime_detect = new MimeDetect();
  $ext = $mime_detect->getExtension($object[$dsid]->mimeType);
  $file_name = str_replace(':', '-', $object->id);
  $file_uri = file_create_filename("{$file_name}.{$dsid}.{$ext}", 'temporary://');
  $object[$dsid]->getContent($file_uri);
  return $file_uri;
}

/**
 * Creates a JPG derivative for the TN datastream.
 * (copied from islandora_pdf solution pack)
 *
 * @param FedoraObject $object
 *   The object to add the derivative datastreams to.
 * @param string $file_uri
 *   The URI to the pdf file the derivative will be generated from.
 *
 * @return boolean
 *   TRUE if successful, FALSE otherwise.
 */
function islandora_multilingual_add_tn_derivative(FedoraObject $object, $file_uri, $lang) {
  $width = variable_get('islandora_pdf_thumbnail_width', 300);
  $height = variable_get('islandora_pdf_thumbnail_height', 300);
  return islandora_multilingual_add_jpg_derivative($object, $file_uri, 'TN-' . $lang, $width, $height);
}

/**
 * Ingests a JPG derivative datastream of the given file.
 * (copied from islandora_pdf solution pack)
 *
 * @param FedoraObject $object
 *   The object to add the derivative datastreams to.
 * @param string $file_uri
 *   The URI to the pdf file the derivative will be generated from.
 * @param string $dsid
 *   The datastream ID to use for the derived datastream.
 * @param integer $width
 *   The width to make the derived datastream.
 * @param integer $height
 *   The height to make the derived datastream.
 *
 * @return boolean
 *   TRUE if successful, FALSE otherwise.
 */
function islandora_multilingual_add_jpg_derivative(FedoraObject $object, $file_uri, $dsid, $width, $height) {
  $success = FALSE;
  $derivative_file_uri = islandora_multilingual_create_jpg_derivative($file_uri, $dsid, $width, $height);
  if ($derivative_file_uri !== FALSE) {
    $success = islandora_multilingual_add_datastream($object, $dsid, $derivative_file_uri);
    file_unmanaged_delete($derivative_file_uri);
  }
  if ($success == FALSE) {
    drupal_set_message(t('Failed to create @dsid derivative.', array('@dsid' => $dsid)), 'error');
  }
  return $success;
}

/**
 * Creates a jpg derivative from the given pdf file using imagemagick.
 * (copied from islandora_pdf solution pack)
 *
 * @param string $file_uri
 *   The URI to the pdf file the derivative will be generated from.
 * @param string $dsid
 *   Used to identify the generated file.
 * @param integer $width
 *   The width to make the derived datastream.
 * @param integer $height
 *   The height to make the derived datastream.
 *
 * @return string
 *   A URI to the generated derivative if successful, FALSE otherwise.
 */
function islandora_multilingual_create_jpg_derivative($file_uri, $dsid,  $width, $height) {
  $source = drupal_realpath($file_uri) . '[0]';
  $matches = array();
  preg_match("/\/([^.]*).*$/", $source, $matches); // Get the base name of the source file.
  $temp = drupal_tempnam("temporary://", "{$matches[1]}.$dsid.jpg");
  $dest = drupal_realpath($temp);
  $args['quality'] = '-quality ' . escapeshellarg(variable_get('imagemagick_quality', 75));
  $args['previewsize'] = '-resize ' . escapeshellarg("{$width}x{$height}");
  $args['colors'] = '-colorspace RGB'; // added by Rosie based on Serhiy
  $context = array(
    'source' => $source,
    'destination' => $dest,
  );
  drupal_alter('imagemagick_arguments', $args, $context);
  // To make use of ImageMagick 6's parenthetical command grouping we need to make
  // the $source image the first parameter and $dest the last.
  // See http://www.imagemagick.org/Usage/basics/#cmdline
  $command = escapeshellarg($source) . ' ' . implode(' ', $args) . ' ' . escapeshellarg("jpg:$dest");
  $output = '';
  $ret = -1;
  if (_imagemagick_convert_exec($command, $output, $ret) !== TRUE) {
    $message = 'imagemagick convert failed to create derivative<br/>Error: @ret<br/>Command: @command<br/>Output: @output';
    $variables = array('@ret' => $ret, '@command' => $command, '@output' => $output);
    watchdog('islandora_multilingual', $message, $variables, WATCHDOG_ERROR);
    return FALSE;
  }
  return $dest;
}

/**
 * Adds the given file as a datastream to the given object using the given datastream id to identify it.
 * Copied from islandora_pdf_add_datastream in islandora_solution_pack_pdf/includes/pdf.process.inc.
 *
 * @param FedoraObject $object
 *   The object to add the datasteam to.
 * @param string $datastream_id
 *   The datastream id of the added datastream.
 * @param string $file_uri
 *   A URI to the file containing the content for the datastream.
 */
function islandora_multilingual_add_datastream($object, $datastream_id, $file_uri) {
  try {
    $mime_detector = new MimeDetect();
    $ds = $object->constructDatastream($datastream_id, "M");
    $ds->label = $datastream_id;
    $ds->mimetype = $mime_detector->getMimetype($file_uri);
    $ds->setContentFromFile(drupal_realpath($file_uri), FALSE);
    $object->ingestDatastream($ds);
    return TRUE;
  }
  catch (exception $e) {
    drupal_set_message(t("@message", array("@message" => $e->getMessage())));
    return FALSE;
  }
}

/**
 * Gets the solr results doc for a particular pid.
 * This uses Solr directly, therefore the resulting doc
 * is an array of fieldnames => values.
 * 
 * This is different from when the doc is found through islandora,
 * where field labels etc are applied, and the resulting doc is
 * an array of fieldnames => array('title' => '', 'class' => '', 'value' => values). 
 *
 * @param string $pid
 *   The PID of the object to find. This must be unique, and in the solr field PID.
 */

function getSolrDocByPid($pid) {
      //copied from islandoraSolrQueryProcessor()
  $url = variable_get('islandora_solr_url', 'localhost:8080/solr');
  $pathParts = parse_url($url);


  // call Solr
  $solr = new Apache_Solr_Service($pathParts['host'], $pathParts['port'], $pathParts['path'] . '/');
  $solr->setCreateDocuments(0);

  try {
    $solr_query = 'PID:' . $pid;
    $results = $solr->search($solr_query, 0, 1);
  } catch (Exception $e) {
    watchdog('multilingual', check_plain(t('Object not in Solr index.')) . ' ' . $e->getMessage());
  }

  unset($solr);
  
  if (isset($results)) {
    $rawResponse = $results->getRawResponse();
    $responseArray = json_decode($rawResponse, TRUE);
    $doc = $responseArray['response']['docs'][0];
    unset($results);
  } else {
    $doc = array();
  }
  return $doc;
}

/**
 * Gets a themed list of links to the document in its available languages.
 * Uses the solr doc's "Datastream field" (i.e. fedora_datastreams_ms) where applicable.
 *
 * @param string $pid
 *   The id of the fedora object desired. 
 */
function getAvailabilityByPid($pid) {
  $doc = getSolrDocByPid($pid);
  include_once DRUPAL_ROOT . '/includes/iso.inc';
  $predefined = _locale_get_predefined_list();
  $OBJ_langs = array();

  $languages = array();
  // The new way, with fedora_datastreams_ms
  $datastreams = isset($doc['fedora_datastreams_ms']) ? $doc['fedora_datastreams_ms'] : array();
  $datastreams = isset($datastreams['value']) ? $datastreams['value'] : $datastreams;
  foreach ($datastreams as $key => $datastream) {
      if (preg_match('/OBJ/', $datastream) == 1) {
          $suffix = (strlen($datastream) > 5) ? substr($datastream, 4) : 'EN';
          $languages[] = strtolower($suffix);
      }
  }
  // The old way, using ds.OBJ-XX fields.
  if (empty($languages)) {
      foreach ($doc as $field => $value) {
          // if an OBJ field, record the language and the link to the datastream.
          if (preg_match('/ds.OBJ-/', $field) === 1) {
              $suffix = substr($field, 7);
              $languages[] = strtolower($suffix);
          }
      }
  }

  // Test for permissions. 
  foreach ($languages as $lang_code) {
      $a = 2;      
  }
  
  foreach ($languages as $lang_code) {
      if (!isset($predefined[$lang_code])) {
          continue;
      }
      // get name of language from predefined list; use native language if possible.
      $lang_name = isset($predefined[$lang_code][1]) ? $predefined[$lang_code][1] : $predefined[$lang_code][0];
      $suffix = strtoupper($lang_code);
      $url = url("islandora/object/{$pid}/datastream/OBJ-{$suffix}/view", array('absolute' => TRUE));
      $OBJ_langs[$lang_name] = array(
          'title' => $lang_name,
          'href' => $url,
          //'attributes' => array('onClick' => "ga('send', 'event', 'PDFs', 'Download ' . $lang_code, '$pid');"),
          'attributes' => array('class' => array('pdfdownload')),
      );
	  
  }
  return theme('links', array('links' => $OBJ_langs, 'attributes' => array('class' =>'languages')));

}

/**
 * Gets a formatted title by $pid, by joining title with subtitle using ": " or " : " depending on language.
 *
 * @param string $pid
 *   The id of the fedora object desired.
 *
 * @param string $lang
 *   The lower-case code of the desired language. e.g. 'en'. 
 */
function getTitleByPid($pid, $lang) {
    $doc = getSolrDocByPid($pid);
    $title_template = 'mods_title_ms_';
    $subtitle_template = 'mods_subTitle_ms_';

    $title_field = chooseLanguage($doc, $title_template);
    $title = $doc[$title_field][0];
    $title_lang = substr($title_field, -2);
    if ( isset($doc[$subtitle_template . $title_lang])) {
        $title = ($title_lang == 'fr') ? $title . ' : ' : $title . ': ';
        $title = $title . $doc[$subtitle_template . $title_lang][0];
    }
    return $title;

}

function multilingual_get_vocabulary_values($type = '', $reset = FALSE) {
    // Use a global variable ISLANDORA_MULTILINGUAL_TEXTGROUP plus the type.
    // Replace the cache_set parameter to equal this textgroup too. 

    $data = &drupal_static('islandora_multilingual::' . $type, NULL, FALSE);

    if (!isset($data)) {
        if (!$reset && ($cache = cache_get('islandora_multilingual::' . $type)) && !empty($cache->data)) {
            $data = $cache->data;
        }
        else {
            if (function_exists('i18n_string')) {
                $tg_object = i18n_string_textgroup('islandora_multilingual');
                $results = $tg_object->load_strings();
                foreach ($results as $result) {
                    $result->get_translation('es');
                    $result->get_translation('fr');
                }
            }
            
            $data = array();
            foreach($results as $result) {
                if ($result->type == $type) { 
                    $data[] = array(
                        'location' => $result->location,
                        'en' => $result->source, 
                        'es' => $result->translations['es'] ? $result->translations['es'] : $result->source, 
                        'fr' => $result->translations['fr'] ? $result->translations['fr'] : $result->source,
                        'lid' => $result->lid,
                    );
                }
            }

            cache_set('islandora_multilingual::' . $type, $data);
        }
    }
    return $data;
}


