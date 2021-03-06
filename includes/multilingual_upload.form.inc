<?php

/**
 * @file
 *
 * Handles the uploading of the PDF file as part of the ingest process.
 */

/**
 * Defines a file upload form for uploading the pdf file.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 * @return
 *   The drupal form definition.
 */
function islandora_multilingual_pdf_upload_form(array $form, array &$form_state) {
  $upload_size = min((int)ini_get('post_max_size'), (int)ini_get('upload_max_filesize'));
  $extensions = array('pdf');
  $languages = explode(variable_get('islandora_languages', 'EN,'));
  return array(
    'file' => array(
      '#title' => t('PDF File'),
      '#type' => 'managed_file',
      '#required' => TRUE,
      '#description' => t('Select file to upload.<br/>Files must be less than <b>!size MB.</b><br/>Allowed file types: <b>!ext.</b>', array('!size' => $upload_size, '!ext' => $extensions[0])),
      '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
      '#upload_location' => 'temporary://',
      '#upload_validators' => array(
        'file_validate_extensions' => $extensions,
        'file_validate_size' => array($upload_size * 1024 * 1024), // Assume its specified in MB
      ),
    ),
    'language' => array(
      '#title' => t('Language of file'),
      '#type' => 'select',
      '#options' => array(
        'EN' => t('English'),
	    'ES' => t('Spanish'),
	    'FR' => t('French')
      ),
      '#default' => 'EN', 
    ),
  );
}

/**
 * Addes the uploaded file into the ingestable objects 'OBJ' datastream.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_multilingual_pdf_upload_form_submit(array $form, array &$form_state) {
  $object = $form_state['islandora']['objects'][0];
  $suffix = isset($form_state['values']['language']) ? '-' . $form_state['values']['language'] : '';
  $dsid = 'OBJ' . $suffix;
  if (empty($object[$dsid])) {
    $ds = $object->constructDatastream($dsid, 'M');
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object[$dsid];
  }
  $file = file_load($form_state['values']['file']);
  $path = drupal_realpath($file->uri);
  $ds->setContentFromFile($path, FALSE);
  $ds->label = $file->filename;
  $ds->mimetype = $file->filemime;
}
