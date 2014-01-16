<?php

/**
 * @file
 * This file contains all theme and preprocess functions
 */

/**
 * preprocess for islandora_multilingual theme
 * used for displaying object landing pages
 *
 * @global type $base_url
 * @param array $variables
 *   an array of variables that will be passed to the theme function
 */
function islandora_multilingual_preprocess_islandora_multilingual(&$variables) {
  drupal_add_js('misc/form.js');
  drupal_add_js('misc/collapse.js');
  drupal_add_css(drupal_get_path('module', 'islandora_multilingual') . '/css/islandora_multilingual.theme.css');
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
  // Remove reference to being in "pubs:collections" and the theme collections
  $exclude = array('pubs:collection', 'pubs:pollutants', 'pubs:greeneconomy', 'pubs:ecosystems', 'pubs:enforcement', 'pubs:climatechange');
  foreach ($variables['parent_collections'] as $key => $collection) {
      if (in_array($collection->id, $exclude)) {
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
    if (isset($variables['islandora_full_url'])) {
        $variables['islandora_content'] = l($variables['islandora_thumbnail_img'], $variables['islandora_full_url'], array('html' => TRUE)); // FIXME note: i changed preview img to thumbnail img.
    }
    else {
        $variables['islandora_content'] = $variables['islandora_thumbnail_img'];
    }
	 
	// Don't use this anymore. - replaced by getAvailabilityByPid.
	$download_url = 'islandora/object/' . $islandora_object->id . '/datastream/' . $obj_ds . '/download';
    $download_text = t("Download");
    $variables['islandora_download_link'] = l($download_text, $download_url, array('attributes' => array('class' => array('islandora-pdf-link'))));
    $view_text = t("View");
    //$variables['islandora_view_link'] = l($view_text, $full_size_url, array('attributes' => array('class' => array('islandora-pdf-link'))));
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
    'mods_abstract_ms_' => array('label' => t('Abstract'), 'weight' => 0.7),
    'mods_subject_topic_Keyword_ms_' => array('label' => t('Themes'), 'weight' => 0.6),
    'mods_subject_topic_Topic_ms_' => array('label' => t('Keywords'), 'weight' => 0.65),
    'mods_subject_meeting_ms_' => array('label' => t('Conference'), 'weight' => 0.67),
    'mods_extent_ms_' => array('label' => t('Number of pages'), 'weight' => 0.7),
    'mods_file_size_ms_' => array('label' => t('File Size'), 'weight' => 0.9),
    'mods_internetMediaType_ms_' => array('label' => t('File Type'), 'weight' => 1.1),
    'mods_identifier_isbn_mtt_' => array('label' => t('ISBN'), 'weight' => 1.2),
    'mods_identifier_QA_mtt_' => array('label' => t('QA number'), 'weight' => 1.3),
    'rels.isDerivationOf' => array('label' => t('This document is based on'), 'weight' => 1.4),
    'rels.hasDerivation' => array('label' => t('Based on this document'), 'weight' => 1.5),
  );


  $doc = getSolrDocByPid($islandora_object->id);

  if (!is_array($doc)) {
    watchdog('multilingual', 'Solr results not returned; please wait a few moments or re-index this document.');
  }
  $metadata = array();
  $related = array();
  // Get available languages from $doc.

  foreach ($doc as $field => $value) {
    // Seek only fields suffixed by the chosen suffix $lang.
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
                $query = array('f[0]' => "mods_subject_topic_Keyword_ms_" . $lang . ':"' . $keyword . '"', 'sort' => 'date_facet desc',);
                $links[] = array(
                    'title' => $keyword,
                    'href' => url("islandora/search/ ", array('query' => $query, 'absolute' => TRUE)),
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

    // If it's a rels-ext field, store it for related documents.
    if (preg_match('/^rels.*/', $field) === 1) {
        if (array_key_exists($field, $resultFieldArray)) {
            $metadata[$field]['label'] = $resultFieldArray[$field]['label'];
            $metadata[$field]['class'] = strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $field));
            $metadata[$field]['weight'] = $resultFieldArray[$field]['weight'];
            if (is_array($value)) {
                $links = array();
                foreach ($value as $longpid) {
                    $links[] = array(
                        'title' => getTitleByPid(substr($longpid,12), $lang),
                        'href' => url('islandora/object/' . substr($longpid,12), array('absolute' => TRUE)),
                        'html' => TRUE,
                        'attributes' => array('class' => array('related')),
                    );
                }
                $value = theme('links', array('links' => $links));
            }
            $metadata[$field]['value'] = $value;
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
  if (isset($doc['rels.isDerivationOf'])) {
      $based_on_pids = array();
      foreach ($doc['rels.isDerivationOf'] as $ref) {
          if (strlen($ref) > 14) {
              $based_on_pids[] = substr($ref, 12);
          }
      }
  }
  if (isset($doc['rels.hasDerivation'])) {
      $basis_of_pids = array();
      foreach ($doc['rels.hasDerivation'] as $ref) {
          if (strlen($ref) > 14) {
              $basis_of_pids[] = substr($ref, 12);
          }
      }
  }
  
  // create list of available objects for extended metadata
  $availability_list = getAvailabilityByPid($islandora_object->id);
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
      $title = $language->language == 'fr' ? $title . ' ' : $title;
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
		  'label' => t('Abstract:'),
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



