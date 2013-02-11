<?php

function islandora_multilingual_preprocess_display_title(&$variables) {

  $max_len = 100;
  $full_title = $variables['title'];
  if ($variables['subTitle'] != '') {
    $full_title = $full_title . ': ' . $variables['subTitle'];
  }
  if (strlen($full_title) > $max_len) {
    $full_title = $variables['title'];
  }
  if (strlen($full_title) > $max_len) {
    $full_title = substr($full_title,0, $max_len);
    $pos = strrpos($full_title, ' ');
    $full_title = substr($full_title, 0, $pos) . ' (...)';
  }

  $variables['display_title'] = $full_title;

}

