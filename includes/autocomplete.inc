<?php

function islandora_multilingual_solr_autocomplete($solr_field, $string) {
#    $solr_field = 'mods_publisher_ms_EN';
    $values = islandora_multilingual_get_solr_values($solr_field);
    $autocomplete_values = array();
    foreach ($values as $value) {
        if (preg_match("/{$string}/i", $value) !== 0) {
          $autocomplete_values[$value] = $value;
        }
    }
    return drupal_json_output($autocomplete_values);
}

function islandora_multilingual_get_solr_values($solr_field) {
    $results = islandora_multilingual_query_solr($solr_field);
    if (isset($results)) {
        $rawResponse = $results->getRawResponse();
        $responseArray = json_decode($rawResponse, TRUE);
        $rawvalues = $responseArray['facet_counts']['facet_fields'][$solr_field];
        $values = array_keys($rawvalues);
    }
    if (!isset($values)) {
        $values = array();
    }
    return $values;
    
}

function islandora_multilingual_query_solr($solr_field) {
    global $base_url;
    $url = variable_get('islandora_solr_url', 'localhost:8080/solr');
    $pathParts = parse_url($url);

    $solr = new Apache_Solr_Service($pathParts['host'], $pathParts['port'], $pathParts['path'] . '/');
    $solr->setCreateDocuments(0);

    try {
        $solr_query = '*:*';
        $params = array('facet' => 'on', 'facet.field' => $solr_field);
        $results = $solr->search($solr_query, 0, 1, $params);
    } catch (Exception $e) {
        drupal_set_message('autocomplete failed to query solr' . ' ' . $e->getMessage());
    }
    unset($solr);

    return $results;
} 

function islandora_multilingual_file_autocomplete($token, $string) {
    switch ($token) {
    case 'projects':
        $file = '/includes/projects.txt';
        $p = drupal_get_path('module', 'islandora_multilingual');
        $file = $p . $file;
        break;
    default: 
        $file = NULL;
    }
    if ($file) {
        $json = json_decode(file_get_contents($file));
        return drupal_json_output($json);
    }else {
        return "{}";
    }
    return "{'value 1' : 'value 1', 'second value' : 'value 2' , 'Third value' : 'value 3'}";
}
