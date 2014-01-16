<?php

/**
 * Build the admin interface
 */


function islandora_multilingual_admin_form($form, &$form_state) {
    $form = array();
  $form['islandora_multilingual_languages_setup'] = array(
    '#type' => 'tabfield',
    '#title' => t('Enabled Languages'),
  );
  $form['islandora_multilingual_languages_setup']['islandora_languages'] = array(
    '#type' => 'textfield',
    '#title' => t('Languages'),
    '#description' => t('Select languages in which documents may appear'),
    '#size' => 40,
    '#default_value' => variable_get('islandora_languages', 'EN,ES,FR'),
  );
  return system_settings_form($form);
}

