<?php
/** 
 * Requires $variables to be of the form:
 * [name
 */
function custom_islandora_views_preprocess_language_links(&$variables) {
  $available_langs = $variables['lang_array'];
  foreach ($available_langs as $lang => $url) {
    $variables['content'][$lang]['label'] = $lang;
    $variables['content'][$lang]['class'] = 'language';
    if (strpos($url, '?') === FALSE) {
      $base_url = $url;
      $vars = '';
    }
    else {
      $url_parts = explode('?', $url, 2);
      $base_url = $url_parts[0];
      parse_str($url_parts[1], $vars); // parse query part of url into $vars
    }
    $variables['content'][$lang]['value'] = l($lang, $base_url, array("rel"=>"nofollow", "target"=>"_Blank", "query"=>$vars));
  }
}

function islandora_multilingual_preprocess_language_links(&$variables) {
  $available_langs = $variables['lang_array'];
  foreach ($available_langs as $lang => $url) {
    $variables['content'][$lang]['label'] = $lang;
    $variables['content'][$lang]['class'] = 'language';
    if (strpos($url, '?') === FALSE) {
      $base_url = $url;
      $vars = '';
    }
    else {
      $url_parts = explode('?', $url, 2);
      $base_url = $url_parts[0];
      parse_str($url_parts[1], $vars); // parse query part of url into $vars
    }
    $variables['content'][$lang]['value'] = l($lang, $base_url, array("rel"=>"nofollow", "target"=>"_Blank", "query"=>$vars));
  }
}
