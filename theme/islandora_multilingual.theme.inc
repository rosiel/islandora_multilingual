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
  global $language;
  $lang = strtoupper($language->language); // get the suffix, e.g. 'EN'


  $variables['islandora_object_label'] = $islandora_object->label;
  
  #$title_string = theme('display_title', array('title' => $islandora_object->label));
  #drupal_set_title($title_string);


  $variables['theme_hook_suggestions'][] = 'islandora_pdf__' . str_replace(':', '_', $islandora_object->id); //maybe keep this not fixme

  $variables['parent_collections'] = islandora_get_parents_from_rels_ext($islandora_object); 
  // Remove reference to being in "pubs:collections"
  foreach ($variables['parent_collections'] as $key => $collection) {
      if ($collection->id == 'pubs:collection') {
          unset($variables['parent_collections'][$key]);
      }
  }




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

	// end link url's section
	//
	//
    $variables['islandora_content'] = l($variables['islandora_thumbnail_img'], $variables['islandora_full_url'], array('html' => TRUE)); // FIXME note: i changed preview img to thumbnail img.
	
	// Don't use this anymore.
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
  // define result field names
  $resultFieldArray = array(
    'mods_title_ms_' => array('label' => t('Title'), 'weight' => 0.0),
	'mods_subTitle_ms_' => array('label' => t('Subtitle'), 'weight' => 0.1),
	'mods_partName_ms_' => array('label' => t('Part name'), 'weight' => 0.11),
    'mods_name_aut_ms_' => array('label' => t('Authors'), 'weight' => 0.2),
    'mods_name_ctb_ms_' => array('label' => t('Contributors'), 'weight' => 0.2),
    'mods_publisher_ms_' => array('label' => t('Publisher'), 'weight' => 0.3),
    'mods_dateIssued_ms_' => array('label' => t('Date of Publication'), 'weight' => 0.4),
    'mods_abstract_ms_' => array('label' => t('Description'), 'weight' => 0.7),
    'mods_subject_topic_Topic_ms_' => array('label' => t('Topics'), 'weight' => 0.6),
    'mods_subject_topic_Keyword_ms_' => array('label' => t('Keywords'), 'weight' => 0.65),
    'mods_subject_meeting_ms_' => array('label' => t('Conference'), 'weight' => 0.67),
    'mods_extent_ms_' => array('label' => t('Number of pages'), 'weight' => 0.7),
    'mods_file_size_ms_' => array('label' => t('File Size'), 'weight' => 0.9),
    'mods_internetMediaType_ms_' => array('label' => t('File Type'), 'weight' => 1.1),
    'mods_identifier_isbn_mtt_' => array('label' => t('ISBN'), 'weight' => 1.2),
    'mods_identifier_QA_mtt_' => array('label' => t('QA number'), 'weight' => 1.3),
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
    drupal_set_message(check_plain(t('Object not in Solr index.')) . ' ' . $e->getMessage(), 'error');
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
	  $OBJ_langs[$lang_name] = array(
		  'title' => $lang_name,
		  'href' => $url,
	  );
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
		// Specific field-based customizations
		if ($generic_field == 'mods_extent_ms_') {
			$value = format_plural(implode(' ', $value), '1 page', '@count pages');
		}
		if ($generic_field == 'mods_file_size_ms_') {
			$value = format_size(implode(' ', $value));
		}
        if ($generic_field == 'mods_subject_topic_Keyword_ms_') {
            $links = array();
            foreach ($value as $keyword) {
                $links[] = array(
                    'title' => $keyword,
                    'href' => url("islandora/search/mods_subject_topic_Keyword_ms_EN%3A\"" . $keyword . '"', array('absolute' => TRUE)),
                    'html' => FALSE,
                    'attributes' => array('class' => array('keywords')),
                );
            }
			$value = theme('links', array('links' => $links));
		}
        // split value if the result value is an array (almost always applies)
        if (is_array($value)) {
          $value = implode("<br /> ", $value);
        }
		// assign the value
		$metadata[$generic_field]['value'] = $value;
	  }
    }
  }

  //drupal_set_title(theme('display_title', array('title' => $metadata['mods_title_ms_']['value'], 'subTitle' => isset ($metadata['mods_subTitle_ms']) ? $metadata['mods_subTitle_ms']['value'] : '')));
  drupal_set_title('');
  drupal_set_breadcrumb(array());

  // Get summary or full document
  if (isset($doc['rels.hasSummary'])) {
	  if (strlen($doc['rels.hasSummary'][0]) > 14) {
		  $summary_pid = substr($doc['rels.hasSummary'][0], 12);
	  }
  }
  if (isset($doc['rels.isSummaryOf'])) {
	  if (strlen($doc['rels.isSummaryOf'][0]) > 14) {
		  $fulldoc_pid = substr($doc['rels.isSummaryOf'][0], 12);
	  }
  }

  // Get related docs
  if (isset($doc['rels.isBasedOn'])) {
      $basis_pids = array();
      foreach ($doc['rels.isBasedOn'] as $ref) {
          if (strlen($ref) > 14) {
              $basis_pids[] = substr($ref, 12);
          }
      }
  }
  if (isset($doc['rels.isBasisOf'])) {
      $based_pids = array();
      foreach ($doc['rels.isBasisOf'] as $ref) {
          if (strlen($ref) > 14) {
              $based_pids[] = substr($ref, 12);
          }
      }
  }
  
  // create list of available objects for extended metadata
  $availability_list = theme('links', array('links' => $OBJ_langs, 'attributes' => array('class' =>'languages'),));
  $metadata['language']['label'] = t('Available in:');
  $metadata['language']['class'] = 'language-list';
  $metadata['language']['weight'] = 1.4;
  $metadata['language']['value'] = $availability_list;

  uasort($metadata, 'drupal_sort_weight');
  $variables['mods_array'] = $metadata;

  // Create short list of metadata for the sidebar.
  //
  //
  $title = '';
  $brief_metadata = array();
  $field = 'mods_title_ms_';
  $field = chooseLanguage($doc, $field);
  if ($field) {
	  $brief_metadata[] = array(
		  'label' => '',
		  'class' => 'mods-title',
		  'value' => $doc[$field][0],
	  );
      //set title variable
      $title .= $doc[$field][0];
  }
  $field = 'mods_subTitle_ms_';
  $field = chooseLanguage($doc, $field);
  if ($field) {
	  $brief_metadata[] = array(
		  'label' => '',
		  'class' => 'mods-subtitle',
		  'value' => $doc[$field][0],
	  );
      //set subtitle into title variable
      $title .= ': ' . $doc[$field][0];
  }
  $field = 'mods_partName_ms_';
  $field = chooseLanguage($doc, $field);
  if ($field) {
	  $brief_metadata[] = array(
		  'label' => '',
		  'class' => 'mods-partname',
		  'value' => '(' . $doc[$field][0] . ')',
	  );
  }

  $field = 'mods_genre_local_ms_';
  $field = chooseLanguage($doc, $field);
  if ($field) {
	  $brief_metadata[] = array(
		  'label' => '',
		  'class' => 'mods-doctype',
		  'value' => $doc[$field][0],
	  );
  }
  $field = 'mods_name_aut_ms_';
  $field = chooseLanguage($doc, $field);
  if ($field) {
	  $brief_metadata[] = array(
		  'label' => t('Authors:'),
		  'class' => 'mods-authors',
		  'value' => theme('item_list', array('items' => $doc[$field], 'type' => 'ul', 'title' => '', 'attributes' => array('class' => 'authors'))),
	  );
  }
  $field = 'mods_name_ctb_ms_';
  $field = chooseLanguage($doc, $field);
  if ($field) {
	  $brief_metadata[] = array(
		  'label' => t('Contributors:'),
		  'class' => 'mods-authors',
		  'value' => theme('item_list', array('items' => $doc[$field], 'type' => 'ul', 'title' => '', 'attributes' => array('class' => 'authors'))),
	  );
  }
  $field = 'mods_abstract_ms_';
  $field = chooseLanguage($doc, $field);
  if ($field) {
	  $brief_metadata[] = array(
		  'label' => t('Description:'),
		  'class' => 'mods-abstract',
		  'value' => $doc[$field][0],
	  );
  }
  $brief_metadata[] = array(
	  'label' => t('Available in:'),
	  'class' => 'availability',
	  'value' => $availability_list,
  );

  if (isset($summary_pid)) {
	  $brief_metadata[] = array(
		  'label' => t('Abstract available in:'),
		  'class' => 'availability-abstract',
		  'value' => getAvailabilityByPid($summary_pid),
	  );
  }
  if (isset($fulldoc_pid)) {
	  $brief_metadata[] = array(
		  'label' => t('Full document available in:'),
		  'class' => 'availability-full',
		  'value' => getAvailabilityByPid($fulldoc_pid),
	  );
  }

  $variables['brief_metadata'] = $brief_metadata;
  $title = filter_xss($title, array('i', 'em'));
  drupal_set_title($title, PASS_THROUGH);

	  

}

function getAvailabilityByPid($pid) {
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
    watchdog('rosie', check_plain(t('Object not in Solr index.')) . ' ' . $e->getMessage());
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
  
  include_once DRUPAL_ROOT . '/includes/iso.inc';
  $predefined = _locale_get_predefined_list();
  $OBJ_langs = array();
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
		  $url = url("islandora/object/{$pid}/datastream/OBJ-{$suffix}/view", array('absolute' => TRUE));
		  $OBJ_langs[$lang_name] = array(
			  'title' => $lang_name,
			  'href' => $url,
		  );
	  }
  }
  return theme('links', array('links' => $OBJ_langs, 'attributes' => array('class' =>'languages')));

}
