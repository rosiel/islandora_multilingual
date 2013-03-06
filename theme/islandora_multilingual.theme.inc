<?php

/**
 * @file
 * This file contains all theme and preprocess functions
 */

/**
 *
 * @global type $base_url
 * @param array $variables
 *   an array of variables that will be passed to the theme function
 */
function islandora_multilingual_preprocess_islandora_multilingual(&$variables) {
  drupal_add_js('misc/form.js');
  drupal_add_js('misc/collapse.js');
  $islandora_object = $variables['islandora_object'];
  $repository = $islandora_object->repository;
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');

  // Customization for MULTILINGUAL
  module_load_include('inc', 'islandora_multilingual', 'includes/Multilingual');

#  try { // WHY YOU WANT THE DC?? FIXME
#    $dc = $islandora_object['DC']->content;
#    $dc_object = DublinCore::importFromXMLString($dc);
#  } catch (Exception $e) {
#    drupal_set_message(t('Error retrieving object %s %t', array('%s' => $islandora_object->id, '%t' => $e->getMessage())), 'error', FALSE);
#  }
#  $variables['islandora_dublin_core'] = isset($dc_object) ? $dc_object : NULL;
#  $variables['dc_array'] = isset($dc_object) ? $dc_object->asArray() : array(); 
#  // end dublin core

  $variables['islandora_object_label'] = $islandora_object->label;
  
  #$title_string = theme('display_title', array('title' => $islandora_object->label));
  #drupal_set_title($title_string);


  $variables['theme_hook_suggestions'][] = 'islandora_pdf__' . str_replace(':', '_', $islandora_object->id); //maybe keep this not fixme

  $variables['parent_collections'] = islandora_get_parents_from_rels_ext($islandora_object); 


  // Get link url's
  global $base_url;

  // full size url
  $obj_ds = chooseLanguage($islandora_object, 'OBJ-');
  if ($obj_ds) {
    $full_size_url = url("islandora/object/{$islandora_object->id}/datastream/{$obj_ds}/view", array('absolute' => TRUE));
    $variables['islandora_full_url'] = $full_size_url;
  }
  // thumbnail
  $tn_ds = chooseLanguage($islandora_object, 'TN-');
  if ($tn_ds) {
    $thumbnail_size_url = url("islandora/object/{$islandora_object->id}/datastream/{$tn_ds}/view");
    $params = array(
      'title' => $islandora_object->label,
      'path' => $thumbnail_size_url
    );
    $variables['islandora_thumbnail_img'] = theme('image', $params);
/* use thumb instead of preview
  }
  // preview image + link
  if (TRUE || isset($islandora_object['PREVIEW'])) {
    $preview_url = url("islandora/object/{$islandora_object->id}/datastream/PREVIEW/view");
    $params = array(
      'title' => $islandora_object->label,
      'path' => $preview_url
    );
    $variables['islandora_preview_img'] = theme('image', $params);
*/
    $variables['islandora_content'] = l($variables['islandora_thumbnail_img'], $variables['islandora_full_url'], array('html' => TRUE)); // FIXME note: i changed preview img to thumbnail img.
    $download_url = 'islandora/object/' . $islandora_object->id . '/datastream/' . $obj_ds . '/download';
    $download_text = t("Download");
    $variables['islandora_download_link'] = l($download_text, $download_url, array('attributes' => array('class' => array('islandora-pdf-link'))));
    $view_text = t("View");
    $variables['islandora_view_link'] = l($view_text, $full_size_url, array('attributes' => array('class' => array('islandora-pdf-link'))));
  }

  // ---- MODS data from Solr ------------
  // FIXME This should 
  //    (a) be flexible, overridden with path if provide,
  //   and b) be checked against the available languages in the islandora_languages variable.

  global $language;
  $lang = strtoupper($language->language); // get the suffix, e.g. 'EN'

  //copied from islandoraSolrQueryProcessor()
  $url = variable_get('islandora_solr_url', 'localhost:8080/solr');
  $pathParts = parse_url($url);

  // define result field names
  $resultFieldArray = array(
    'mods_title_ms_' => array('label' => t('Title'), 'weight' => 0.0),
    'mods_subTitle_ms_' => array('label' => t('Subtitle'), 'weight' => 0.1),
    'mods_name_aut_ms_' => array('label' => t('Author'), 'weight' => 0.2),
    'mods_name_ctb_ms_' => array('label' => t('Contributor'), 'weight' => 0.2),
    'mods_publisher_ms_' => array('label' => t('Publisher'), 'weight' => 0.3),
    'mods_dateIssued_ms_' => array('label' => t('Date of Publication'), 'weight' => 0.4),
    'mods_abstract_ms_' => array('label' => t('Description'), 'weight' => 0.7),
    'mods_identifier_isbn_mtt_' => array('label' => t('ISBN'), 'weight' => 0.5),
    'mods_subject_topic_Topic_ms' => array('label' => t('Topics'), 'weight' => 0.6),
    'mods_subject_topic_Keyword_ms_' => array('label' => t('Keywords'), 'weight' => 0.65),
    'mods_extent_ms_' => array('label' => t('Number of pages'), 'weight' => 0.7),
    'mods_file_size_ms_' => array('label' => t('File Size'), 'weight' => 0.9),
    'mods_internetMediaType_ms_' => array('label' => t('File Type'), 'weight' => 1.1),
  );


  // call Solr
  $solr = new Apache_Solr_Service($pathParts['host'], $pathParts['port'], $pathParts['path'] . '/');
  $solr->setCreateDocuments(0);

  try {
    $solr_query = 'PID:' . $islandora_object->id;
    $results = $solr->search($solr_query, 0, 1);
  } catch (Exception $e) {
    drupal_set_message(check_plain(t('Error searching solr index')) . ' ' . $e->getMessage(), 'error');
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

  if (!is_array($doc)) {
    watchdog('multilingual', 'Solr results not returned; please wait a few moments or re-index this document.');
  }
  $metadata = array();
  $OBJ_langs = array();
  include_once DRUPAL_ROOT . '/includes/iso.inc';
  $predefined = _locale_get_predefined_list();

  foreach ($doc as $field => $value) {
    // if an OBJ field, record the language and the link to the datastream.
    if (preg_match('/ds.OBJ-/', $field) === 1) {
      $suffix = substr($field, 7);
      $lang_code = strtolower($suffix);
      if (!isset($predefined[$lang_code])) {
        continue;
      }
      // get name of language from predefined list; use native language if possible.
      $lang_name = isset($predefined[$lang_code][1]) ? $predefined[$lang_code][1] : $predefined[$lang_code][0];
      $url = url("islandora/object/{$islandora_object->id}/datastream/OBJ-{$suffix}/view", array('absolute' => TRUE));
      $OBJ_langs[$lang_name] = $url;
    }
    // Otherwise seek only fields suffixed by the chosen suffix $lang.
    if (preg_match('/^.*_' . $lang . '$/', $field) === 1) {
      // strip language code from solr field name
      $generic_field = substr($field, 0, strlen($field)-2);
      // check against the array of result fields to keep (see above)
      if (array_key_exists($generic_field, $resultFieldArray)) {
        // assign 'label', 'class', 'weight'
        $metadata[$generic_field]['label'] = $resultFieldArray[$generic_field]['label'];
        $metadata[$generic_field]['class'] = strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $field));
        $metadata[$generic_field]['weight'] = $resultFieldArray[$generic_field]['weight'];
        // split value if the result value is an array (almost always applies)
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
        $metadata[$generic_field]['value'] = $value;
      }

    }
  }

  drupal_set_title(theme('display_title', array('title' => $metadata['mods_title_ms_']['value'], 'subTitle' => isset ($metadata['mods_subTitle_ms']) ? $metadata['mods_subTitle_ms']['value'] : '')));
  
  // create list of available objects
  $availability_list = theme('language_links', array('lang_array' => $OBJ_langs));
  $metadata['language']['label'] = t('Available in');
  $metadata['language']['class'] = 'language-list';
  $metadata['language']['weight'] = 1.4;
  $metadata['language']['value'] = $availability_list;

  uasort($metadata, 'drupal_sort_weight');
  $variables['mods_array'] = $metadata;

  dpm($variables);
  


}

