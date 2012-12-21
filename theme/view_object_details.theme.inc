<?php

/**
 * preprocess the edit template
 * @global string $base_url
 * @param array $variables
 *   theme variables for the edit template
 */
function custom_islandora_views_preprocess_islandora_default_edit(&$variables) {
  $islandora_object = $variables['islandora_object'];
  global $base_url;
  $datastreams = array();
  $variables['islandora_editmetadata_url'] = $base_url . '/islandora/edit_form/' . $islandora_object->id;
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');
  // $variables['add_datastream_form'] = drupal_get_form('islandora_add_datastream_form', $islandora_object->id);
  $header = array(
    array('data' => t('ID')),
    array('data' => t('Label')),
    array('data' => t('Type')),
    array('data' => t('Mime type')),
    array('data' => t('Size')),
    array('data' => t('Operations'), 'colspan' => '3'),
      //array('data' => t('Delete')),
  );
  $table_attributes = array('class' => array('manage-datastreams'));
  $rows = array();
  foreach ($islandora_object as $ds) {
    $rows[] = array(
      array('class' => 'datastream-id', 'data' => l($ds->id, islandora_datastream_get_url($ds, 'view'))),
      array('class' => 'datastream-label', 'data' => $ds->label),
      array('class' => 'datastream-control', 'data' => islandora_control_group_to_human_readable($ds->controlGroup)),
      array('class' => 'datastream-mime', 'data' => $ds->mimeType),
      array('class' => 'datastream-size', 'data' => islandora_datastream_get_human_readable_size($ds)),
      array('class' => 'datastream-download', 'data' => l(t('download'), islandora_datastream_get_url($ds, 'download'))),
      array('class' => 'datastream-edit', 'data' => islandora_datastream_edit_get_link($islandora_object, $ds->id)),
      array('class' => 'datastream-delete', 'data' => islandora_datastream_get_delete_link($ds)),
    );
  }
  $caption = $islandora_object->label . ' - ' . $islandora_object->id;
  $table = array('colgroups' => NULL, 'sticky' => TRUE, 'empty' => 'Error loading datastreams', 'caption' => $caption, 'header' => $header, 'rows' => $rows, 'attributes' => $table_attributes);
  $variables['datastream_table'] = theme_table($table);
  }


/**
 * implements hook_preprocess_THEME().
 * 
 * preprocess for the object view
 * @global string $base_url
 * @param array $variables
 * 
 * @global $language
 * @global $user
 */
function custom_islandora_views_preprocess_view_object_details(&$variables) {
  drupal_add_js('misc/form.js');
  drupal_add_js('misc/collapse.js');
  $islandora_object = $variables['islandora_object'];
  $repository = $islandora_object->repository;
  module_load_include('inc', 'islandora', 'includes/islandora_dublin_core');
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/datastream');

  $variables['parent_collections'] = islandora_get_parents_from_rels_ext($islandora_object);

  // for MULTILINGUAL 
  module_load_include('inc', 'custom_islandora_views', 'includes/Multilingual');

  $datastreams = array();
  // get the info of the datastreams for the "object details" tab - with download links. 
  foreach ($islandora_object as $ds) {
    try {
      $pid = $islandora_object->id;
      $id = $ds->id;
      $label = $ds->label;
      $download_path = islandora_datastream_get_url($ds, 'download');
      $datastreams[$id]['id'] = $id;
      $datastreams[$id]['label'] = $label;
      $datastreams[$id]['label_link'] = l($label, $download_path);
      $datastreams[$id]['download_url'] = $download_path;
      $datastreams[$id]['mimetype'] = $ds->mimetype;
      $datastreams[$id]['size'] = islandora_datastream_get_human_readable_size($ds);
      $datastreams[$id]['created_date'] = $ds->createdDate->format("Y-m-d");
      $datastreams[$id]['class'] = strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $id));	
    }
    catch (RepositoryException $e) {}
  }
  $variables['datastreams'] = $datastreams;

  // get DC datstream content and parse as DC xml.
  try {
    $dc = $islandora_object['DC']->content;
    //$dc_xml = simplexml_load_string($dc);
    $dc_object = DublinCore::import_from_xml_string($dc);
    $dc_array = $dc_object->as_formatted_array();
  } catch (Exception $e) {
    drupal_set_message(t('Error retrieving object %s %t', array('%s' => $islandora_object->id, '%t' => $e->getMessage())), 'error', FALSE);
  }
  $variables['dc_array'] = isset($dc_array) ? $dc_array : array();
  $variables['islandora_dublin_core'] = isset($dc_object) ? $dc_object : NULL;
  $variables['islandora_object_label'] = $islandora_object->label;
  global $base_url;

  if (isset($islandora_object['TN-EN'])) {
    $variables['islandora_thumbnail_url'] = $base_url . '/islandora/object/' . $islandora_object->id . '/datastream/TN-EN/view';
  }


  // ======================= Customizations by Rosie ========================
  // language
  global $language;
  $drupal_lang_suffix = strtoupper($language->language);
  $suffix_backup = array('EN', 'ES', 'FR');
  $language_names = array('EN' => 'English', 'ES' => 'Español', 'FR' => 'Français');

  // Look for languages - and list them with the drupal language first.
  $languages = array_diff($suffix_backup, array($drupal_lang_suffix));
  array_unshift($languages, $drupal_lang_suffix); // e.g. ['EN', 'FR', 'ES']
  
  // find which languages have OBJ- datastreams, i.e. have expressions. -- this uses the Fedora-extracted datastreams... but isn't working...
  $langs_in_object = array();
  foreach ($languages as $lang) {
    if (isset($variables['datastreams']['OBJ-' . $lang])) {
      $langs_in_object[] = $lang;
    }
  }
 
//FIXME cleanup
//   // Choose which language's MODS datastream to display
//   if (in_array($drupal_lang_suffix, $langs_in_object)) {
//     $mods_lang = $drupal_lang_suffix;
//   }
//   else {
//     $mods_lang = array_pop($langs_in_object);
//     if ($mods_lang == NULL) {
//       $mods_lang = '';
//     }
//   }
// 
//   $mods = $islandora_object['MODS-' . $mods_lang]->content;
// Couldn't do that yet - wait for solr results.



  $query = 'PID:' . $islandora_object->id;
  $params = array('wt' => 'phps');

  // Define which solr fields to display (later, make customizable)
  $resultFieldArray = array(
    'mods_title_ms_' => array('label' => t('Title'), 'weight' => 0.0),
    'mods_subTitle_ms_' => array('label' => t('Subtitle'), 'weight' => 0.1),
    'mods_name_aut_ms_' => array('label' => t('Author'), 'weight' => 0.2),
    'mods_name_ctb_ms_' => array('label' => t('Contributor'), 'weight' => 0.2),
    'mods_publisher_ms_' => array('label' => t('Publisher'), 'weight' => 0.3),
    'mods_dateIssued_ms_' => array('label' => t('Date of Publication'), 'weight' => 0.4),
    'mods_abstract_ms_' => array('label' => t('Description'), 'weight' => 0.7),
    'mods_identifier_isbn_mtt_' => array('label' => t('ISBN'), 'weight' => 0.5),
    'mods_subject_topic_Subjects_ms_' => array('label' => t('Topics'), 'weight' => 0.6),
    'mods_subject_topic_Keywords_ms_' => array('label' => t('Keywords'), 'weight' => 0.65),
    'mods_extent_ms_' => array('label' => t('Number of pages'), 'weight' => 0.7),
    'mods_file_size_ms_' => array('label' => t('File Size'), 'weight' => 0.9),
  );

  //copied from islandoraSolrQueryProcessor()
  $url = variable_get('islandora_solr_url', 'localhost:8080/solr');
  $pathParts = parse_url($url);

  // call Solr
  $solr = new Apache_Solr_Service($pathParts['host'], $pathParts['port'], $pathParts['path'] . '/');
  $solr->setCreateDocuments(0);

  try {
    $solr_query = 'PID:' . $islandora_object->id;
    $results = $solr->search($solr_query, 0, 1);
  } catch (Exception $e) {
    drupal_set_message(check_plain(t('Error searching solr index')) . ' ' . $e->getMessage(), 'error');
  }
  if (!isset($results)) {
    watchdog('multilingual', 'No solr results found for search query: ' . $solr_query);
  } else {
    $resultsArray = array();
    $rawResponse = $results->getRawResponse();
    $responseArray = json_decode($rawResponse, TRUE);
    $docs = $responseArray['response']['docs'];
    foreach ($docs as $doc) {
      $rows = array();
      $available_langs = array();
      $mods_lang = chooseLanguage($doc, 'mods_title_ms_');
      foreach ($doc as $field => $value) {
        // if it's an OBJ_XX datastream, keep the language and URL
	if (preg_match('/ds.OBJ-/', $field) === 1) {
	  $language_code = substr($field, 7);
	  $lang = isset($language_names[$language_code]) ? $language_names[$language_code] : $language_code;
	  $available_langs[$lang] = trim($value[0]);
	}
	// otherwise seek fields suffixed by the chosen language
        if (preg_match('/^.*_' . $mods_lang . '$/', $field) === 1) {
	  // strip language code from solr field name
	  $generic_field = substr($field, 0, strlen($field)-2);
	  if (array_key_exists($generic_field, $resultFieldArray)) {
	    // assign 'label', 'class', 'weight' - SEE BELOW for value
            $rows[$generic_field]['label'] = $resultFieldArray[$generic_field]['label'];
            $rows[$generic_field]['class'] = strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $field));
            $rows[$generic_field]['weight'] = $resultFieldArray[$generic_field]['weight'];
            // split value if the result value is an array
            if (is_array($value)) {
              $value = implode("<br /> ", $value);
            }

	    // Specific customizations
	    if ($generic_field == 'mods_extent_ms_') {
	      $value = format_plural($value, '1 page', '@count pages');
	    }
	    if ($generic_field == 'mods_file_size_ms_') {
	      $value = format_size($value);
	    }

	    // assign the value
            $rows[$generic_field]['value'] = $value;
	  }
	}
	// find the thumbnail
	$suffix = chooseLanguage($doc, 'ds.TN-');
        $variables['islandora_thumbnail_url'] = $base_url . '/islandora/object/' . $islandora_object->id . '/datastream/TN-' . $suffix . '/view';


      }
      // format the available languages

      $languages_content = 'test';
      $languages_content = theme('language_links', array('lang_array' => $available_langs));
      $rows['language']['label'] = t('Available Languages');
      $rows['language']['class'] = 'language-list';
      $rows['language']['weight'] = 1.4;
      $rows['language']['value'] = $languages_content;

      uasort($rows, 'drupal_sort_weight');
      
    }

    $variables['test_mods_array'] = $rows;
    if (isset($variables['test_mods_array']['mods_title_ms_'])) {
      $variables['islandora_object_label'] = $variables['test_mods_array']['mods_title_ms_']['value'];
    }
  }
}

