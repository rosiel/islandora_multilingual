<?php

/**
 * Build the admin interface
 */

function multilingual_main() {
    $output = '';

    $types = array(
        'themes' => t('Themes'),
        'keywords' => t('Keywords'),
        'doctypes' => t('Document types'),
        'organizations' => t('Organizations'),
    );

    foreach ($types as $type => $label) {
        $variables = array(
            'title' => $label,
            'show' => TRUE,
            'content' => multilingual_admin_vocab_table($type),
            'description' => 'No values set',
        );
        $output .= theme('admin_block', array('block' => $variables));
    }

   //i18n_string_translation_update(array('islandora_multilingual', 'doctypes', '1', 'name'), 'Informe independiente del Secretariado', 'es', 'Article 13 Secretariat Report');
    $strings['islandora_multilingual']['doctypes']['2']['name'] = 'Background paper'; 

    return $output;
}

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


function multilingual_admin_vocab_table($type) {
    module_load_include('inc', 'islandora_multilingual', 'includes/Multilingual');
    $table = array(
        'header' => array(
            array( 'data' => t('English'),),
            array( 'data' => t('Spanish'),),
            array( 'data' => t('French'),),
            array( 'data' => t('Operations'), 'colspan' => 2,),
        ),
        'rows' => array(),
    );
    $data = multilingual_get_vocabulary_values($type, TRUE);
    if (!empty($data)) {
        foreach ($data as $row) {
            $table['rows'][] = array($row['en'] . '<br/><small>' . $row['location'] . '</small>', $row['es'], $row['fr'], l('edit', "admin/config/regional/translate/edit/" . $row['lid'] . "?destination=admin/islandora/multilingual"), 'delete!');
        }
        return theme('table', $table);
    }
    else {
        return '';
    }

}

function test() {

    $xml_master = new DOMDocument();
    $file = drupal_get_path('module', 'islandora_multilingual') . '/admin/source.xml';
    $xml_master->loadXML(file_get_contents($file));
    $xml_sheep = new DOMDocument();
    $baa = '<?xml version="1.0" encoding="UTF-8"?><mods xmlns="http://www.loc.gov/mods/v3" xmlns:fn="http://www.w3.org/2005/xpath-functions" xmlns:mods="http://www.loc.gov/mods/v3" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xlink="http://www.w3.org/1999/xlink"><name type="personal"><role></role><namePart>PENIS</namePart></name></mods>';

    define('MODS', "http://www.loc.gov/mods/v3");
    $xml_sheep->loadXML($baa);

    $xml_master->formatOutput = TRUE;
    $xml_sheep->formatOutput = TRUE;

    $mods_m = $xml_master->documentElement;
    $mods_s = $xml_sheep->documentElement;

    $xpath_m = new DOMXpath($xml_master);
    $xpath_m->registerNamespace("mods", "http://www.loc.gov/mods/v3");
    $xpath_s = new DOMXpath($xml_sheep);
    $xpath_s->registerNamespace("mods", "http://www.loc.gov/mods/v3");


    // Set names stuff
    $names_list = $xpath_s->query('/mods:mods/mods:name[@type="personal"]');
    foreach ($names_list as $name) $mods_s->removeChild($name);

    $names_list_m = $xpath_m->query('/mods:mods/mods:name[@type="personal"]');
    foreach ($names_list_m as $name) {
        $name_s = $xml_sheep->importNode($name, TRUE); // MUST do a deep-copy.
        $mods_s->appendChild($name_s);
    }
    // Set master stuff
    $test = $xml_sheep->createElementNS(MODS, 'note');
    $mods_s->appendChild($test);
    $test->setAttribute('type', 'master');
    $test->nodeValue = 'yes';


    $synced = $xpath_s->query('/mods:mods/mods:note[@type="master"]', $mods_s);
    if ($synced->length == 0) {
        $synced = $xml_sheep->createElementNS(MODS, 'note');
        $synced->setAttribute('type', 'master');
        $synced->nodeValue = 'no';
        $mods_s->appendChild($synced);
    }
    else {
        $synced->item(0)->nodeValue = 'no';
    }

    // Set label stuff
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


    $output = '';
    $output .= '<h2>Sheep XML</h2>';
    $output .= '<pre>';
    $output .= htmlentities($xml_sheep->saveXML());
    $output .= '</pre>';
    $output .= '<h2>Master XML</h2>';
    $output .= '<pre>';
    $output .= htmlentities($xml_master->saveXML());
    $output .= '</pre>';

    return $output;
}

function multilingual_reset() {
    $data = array(
array( 'type' => 'themes', 'en' => 'ecosystems', 'es' => 'ecosistemas', 'fr' => 'écosystèmes' ),
array( 'type' => 'themes', 'en' => 'climate change', 'es' => 'cambio climático', 'fr' => 'changements climatiques' ),
array( 'type' => 'themes', 'en' => 'enforcement', 'es' => 'aplicación de la legislación', 'fr' => 'application des lois' ),
array( 'type' => 'themes', 'en' => 'green economy', 'es' => 'economía verde', 'fr' => 'économie verte' ),
array( 'type' => 'themes', 'en' => 'pollutants', 'es' => 'contaminantes', 'fr' => 'polluants' ),
array( 'type' => 'doctypes', 'en' => 'Project report', 'es' => 'Informe de proyecto', 'fr' => 'Compte rendu de projet' ),
array( 'type' => 'doctypes', 'en' => 'Background paper', 'es' => 'Documento de referencia', 'fr' => "Document d'information" ),
array( 'type' => 'doctypes', 'en' => 'Brochure', 'es' => 'Folleto', 'fr' => 'Brochure' ),
array( 'type' => 'doctypes', 'en' => 'Executive summary', 'es' => 'Resumen ejecutivo', 'fr' => 'Résumé' ),
array( 'type' => 'doctypes', 'en' => 'Article 13 Secretariat Report', 'es' => 'Informe independiente del Secretariado', 'fr' => "Rapport du Secrétariat en vertu de l'article 13" ),
array( 'type' => 'doctypes', 'en' => 'North American Regional Action Plan (NARAP)', 'es' => 'Plan de Acción Regional de América del Norte (PARAN)', 'fr' => "Plan d'action régional nord-américain (PARNA)" ),
array( 'type' => 'organizations', 'en' => 'North American Bird Conservation Initiative (NABCI)', 'es' => 'Iniciativa para la Conservación de las Aves de América del Norte (ICAAN)', 'fr' => "Initiative de conservation des oiseaux de l'Amérique du Nord (ICOAN)" ),
array( 'type' => 'organizations', 'en' => 'North American Marine Protected Areas Network (NAMPAN)', 'es' => 'Red de Áreas Marinas Protegidas de América del Norte (RAMPAN)', 'fr' => 'Réseau nord-américain d’aires marines protégées (RNAAMP)' ),
array( 'type' => 'organizations', 'en' => 'North American Fund for Environmental Cooperation (NAFEC)', 'es' => 'Fondo de América del Norte para la Cooperación Ambiental (FANCA)', 'fr' => 'Fonds nord-américain pour la coopération environnementale (FNACE)' ),
array( 'type' => 'organizations', 'en' => 'Commission for Environmental Cooperation (CEC)', 'es' => 'Comisión para la Cooperación Ambiental (CCA)', 'fr' => 'Commission de coopération environnementale (CCE)' ),
array( 'type' => 'organizations', 'en' => 'North American Free Trade Agreement (NAFTA)', 'es' => 'Tratado de Libre Comercio de América del Norte (TLCAN)', 'fr' => 'Accord de libre-échange nord-américain (ALÉNA)' ),
array( 'type' => 'keywords', 'en' => 'air', 'es' => 'atmósfera', 'fr' => 'air' ),
array( 'type' => 'keywords', 'en' => 'chemicals', 'es' => 'sustancias químicas', 'fr' => 'produits chimiques' ),
array( 'type' => 'keywords', 'en' => 'energy', 'es' => 'energía', 'fr' => 'énergie' ),
array( 'type' => 'keywords', 'en' => 'trade', 'es' => 'comercio', 'fr' => 'commerce' ),
array( 'type' => 'keywords', 'en' => 'environmental law', 'es' => 'legislación ambiental', 'fr' => "droit de l'environnement" ),
array( 'type' => 'keywords', 'en' => 'health', 'es' => 'salud', 'fr' => 'santé' ),
array( 'type' => 'keywords', 'en' => 'mapping', 'es' => 'cartografía', 'fr' => 'cartographie' ),
array( 'type' => 'keywords', 'en' => 'monitoring', 'es' => 'monitoreo', 'fr' => 'surveillance' ),
array( 'type' => 'keywords', 'en' => 'transportation', 'es' => 'transporte', 'fr' => 'transport' ),
array( 'type' => 'keywords', 'en' => 'waste', 'es' => 'residuos', 'fr' => 'déchets' ),
array( 'type' => 'keywords', 'en' => 'water', 'es' => 'agua', 'fr' => 'eau' ),
array( 'type' => 'keywords', 'en' => 'markets', 'es' => 'mercados', 'fr' => 'marchés' ),
array( 'type' => 'keywords', 'en' => 'electricity', 'es' => 'electricidad', 'fr' => 'électricité' ),
array( 'type' => 'keywords', 'en' => 'power plants', 'es' => 'centrales eléctricas', 'fr' => 'centrales électriques' ),
array( 'type' => 'keywords', 'en' => 'renewable energy', 'es' => 'energía renovable', 'fr' => 'énergies renouvelables' ),
array( 'type' => 'keywords', 'en' => 'carbon', 'es' => 'carbono', 'fr' => 'carbone' ),
array( 'type' => 'keywords', 'en' => 'greenhouse gases', 'es' => 'gases de efecto invernadero', 'fr' => 'gaz à effet de serre' ),
array( 'type' => 'keywords', 'en' => 'air quality', 'es' => 'calidad del aire', 'fr' => "qualité de l'air" ),
array( 'type' => 'keywords', 'en' => 'air pollution', 'es' => 'contaminación atmosférica', 'fr' => "pollution de l'air" ),
array( 'type' => 'keywords', 'en' => 'water pollution', 'es' => 'contaminación del agua', 'fr' => "pollution de l'eau" ),
array( 'type' => 'keywords', 'en' => 'movements of pollutants', 'es' => 'movimientos de contaminantes', 'fr' => 'mouvements de polluants' ),
array( 'type' => 'keywords', 'en' => 'toxic chemicals', 'es' => 'sustancias químicas tóxicas', 'fr' => 'produits chimiques toxiques' ),
array( 'type' => 'keywords', 'en' => 'persistent organic pollutants (POPs)', 'es' => 'contaminantes orgánicos persistentes (COP)', 'fr' => 'polluants organiques persistants (POP)' ),
array( 'type' => 'keywords', 'en' => 'carbon sequestration', 'es' => 'secuestro de carbono', 'fr' => 'séquestration de carbone' ),
array( 'type' => 'keywords', 'en' => 'carbon markets', 'es' => 'mercados de carbono', 'fr' => 'marchés du carbone' ),
array( 'type' => 'keywords', 'en' => 'human health', 'es' => 'salud humana', 'fr' => 'santé humaine' ),
array( 'type' => 'keywords', 'en' => 'children', 'es' => 'población infantil', 'fr' => 'enfants' ),
array( 'type' => 'keywords', 'en' => 'financing', 'es' => 'financiamiento', 'fr' => 'financement' ),
array( 'type' => 'keywords', 'en' => 'green goods and services', 'es' => 'bienes y servicios ecológicos', 'fr' => 'biens et services écologiques' ),
array( 'type' => 'keywords', 'en' => 'product labelling', 'es' => 'etiquetado de productos', 'fr' => 'étiquetage des produits' ),
array( 'type' => 'keywords', 'en' => 'small and medium enterprises (SMEs)', 'es' => 'pequeñas y medianas empresas (pyme)', 'fr' => 'petites et moyennes entreprises (PME)' ),
array( 'type' => 'keywords', 'en' => 'international trade', 'es' => 'comercio internacional', 'fr' => 'commerce international' ),
array( 'type' => 'keywords', 'en' => 'green building', 'es' => 'edificación sustentable', 'fr' => 'bâtiment écologique' ),
array( 'type' => 'keywords', 'en' => 'hazardous waste', 'es' => 'residuos peligrosos', 'fr' => 'déchets dangereux' ),
array( 'type' => 'keywords', 'en' => 'electronic waste', 'es' => 'desechos electrónicos', 'fr' => 'déchets électroniques' ),
array( 'type' => 'keywords', 'en' => 'agriculture industry', 'es' => 'industria agrícola', 'fr' => 'industrie agricole' ),
array( 'type' => 'keywords', 'en' => 'mining industry', 'es' => 'industria minera', 'fr' => 'industrie minière' ),
array( 'type' => 'keywords', 'en' => 'automotive industry', 'es' => 'industria automotriz', 'fr' => 'industrie automobile' ),
array( 'type' => 'keywords', 'en' => 'environmental regulations', 'es' => 'reglamentación ambiental', 'fr' => 'réglementations environnementales' ),
array( 'type' => 'keywords', 'en' => 'risk assessment', 'es' => 'evaluación de riesgos', 'fr' => 'évaluation des risques' ),
array( 'type' => 'keywords', 'en' => 'arctic communities and ecosystems', 'es' => 'comunidades y ecosistemas árticos', 'fr' => "collectivités et écosystèmes de l'Arctique" ),
array( 'type' => 'keywords', 'en' => 'marine communities and ecosystems', 'es' => 'comunidades y ecosistemas marinos', 'fr' => 'communautés et écosystèmes marins' ),
array( 'type' => 'keywords', 'en' => 'grassland communities and ecosystems', 'es' => 'comunidades y ecosistemas de pastizal', 'fr' => 'communautés et écosystèmes des prairies' ),
array( 'type' => 'keywords', 'en' => 'species', 'es' => 'especies', 'fr' => 'espèces' ),
array( 'type' => 'keywords', 'en' => 'invasive species', 'es' => 'especies invasoras', 'fr' => 'espèces envahissantes' ),
array( 'type' => 'keywords', 'en' => 'environmental information', 'es' => 'información ambiental', 'fr' => 'information environnementale' ),
array( 'type' => 'keywords', 'en' => 'green purchasing', 'es' => 'compras verdes', 'fr' => 'achats écologiques' ),
array( 'type' => 'keywords', 'en' => 'sustainable tourism', 'es' => 'turismo sustentable', 'fr' => 'écotourisme' ),
array( 'type' => 'keywords', 'en' => 'sustainable coffee', 'es' => 'café sustentable', 'fr' => 'café produit de façon durable' ),
array( 'type' => 'keywords', 'en' => 'small and medium-size enterprises (SMEs)', 'es' => 'pequeñas y medianas empresas (pyme)', 'fr' => 'petites et moyennes entreprises (PME)' ),
array( 'type' => 'keywords', 'en' => 'polychlorinated biphenyls (PCBs)', 'es' => 'bifenilos policlorados (BPC)', 'fr' => 'biphényles polychlorés (BPC)' ),
array( 'type' => 'keywords', 'en' => 'policy', 'es' => 'políticas', 'fr' => 'politiques' ),
array( 'type' => 'keywords', 'en' => 'mercury', 'es' => 'mercurio', 'fr' => 'mercure' ),
array( 'type' => 'keywords', 'en' => 'corn (maize)', 'es' => 'maíz', 'fr' => 'maïs' ),
array( 'type' => 'keywords', 'en' => 'livestock', 'es' => 'ganadería', 'fr' => 'élevage' ),
array( 'type' => 'keywords', 'en' => 'humpback whale', 'es' => 'ballena jorobada', 'fr' => 'rorqual à bosse' ),
array( 'type' => 'keywords', 'en' => 'groundwater', 'es' => 'agua subterránea', 'fr' => 'eaux souterraines' ),
array( 'type' => 'keywords', 'en' => 'grasslands', 'es' => 'pastizales', 'fr' => 'prairies' ),
array( 'type' => 'keywords', 'en' => 'furans', 'es' => 'furanos', 'fr' => 'furanes' ),
array( 'type' => 'keywords', 'en' => 'freshwater', 'es' => 'agua dulce', 'fr' => 'eau douce' ),
array( 'type' => 'keywords', 'en' => 'feedlots', 'es' => 'corrales de engorda', 'fr' => 'parcs d’engraissement' ),
array( 'type' => 'keywords', 'en' => 'environmental enforcement', 'es' => 'aplicación de la legislación ambiental', 'fr' => 'application des lois en matière d’environnement' ),
array( 'type' => 'keywords', 'en' => 'emissions inventories', 'es' => 'inventarios de emisiones', 'fr' => 'inventaires des émissions' ),
array( 'type' => 'keywords', 'en' => 'electronics industry', 'es' => 'industria electrónica', 'fr' => "secteur de l'électronique" ),
array( 'type' => 'keywords', 'en' => 'education', 'es' => 'educación', 'fr' => 'éducation' ),
array( 'type' => 'keywords', 'en' => 'dioxins', 'es' => 'dioxinas', 'fr' => 'dioxines' ),
array( 'type' => 'keywords', 'en' => 'dichlorodiphenyltrichloroethane (DDT)', 'es' => 'diclorodifeniltricloroetano (DDT)', 'fr' => 'dichlorodiphényltrichloréthane (DDT)' ),
array( 'type' => 'keywords', 'en' => 'conservation', 'es' => 'conservación', 'fr' => 'conservation' ),
array( 'type' => 'keywords', 'en' => 'chlordane', 'es' => 'chlordano', 'fr' => 'chlordane' ),
array( 'type' => 'keywords', 'en' => 'black carbon', 'es' => 'carbono negro', 'fr' => 'carbone noir' ),
array( 'type' => 'keywords', 'en' => 'birds', 'es' => 'aves', 'fr' => 'oiseaux' ),
array( 'type' => 'keywords', 'en' => 'best practices', 'es' => 'mejores práticas', 'fr' => 'meilleures pratiques' ),
array( 'type' => 'keywords', 'en' => 'agriculture', 'es' => 'agricultura', 'fr' => 'agriculture' ),
array( 'type' => 'keywords', 'en' => 'environment management systems', 'es' => 'sistemas de administración ambiental', 'fr' => "systèmes de gestion de l'environnement" ),
array( 'type' => 'keywords', 'en' => 'monarch butterfly', 'es' => 'mariposa monarca', 'fr' => 'monarque' ),
array( 'type' => 'keywords', 'en' => 'municipalities', 'es' => 'municipios', 'fr' => 'municipalités' ),
array( 'type' => 'keywords', 'en' => '', 'es' => 'ozone', 'fr' => 'ozono' ),
array( 'type' => 'keywords', 'en' => 'Silva Reservoir (Mexico)', 'es' => 'Presa Silva (México)', 'fr' => 'Réservoir Silva (Méxique)' ),
array( 'type' => 'keywords', 'en' => 'state of the environment', 'es' => 'estado del medio ambiente', 'fr' => 'état de l’environnement' ),
array( 'type' => 'keywords', 'en' => 'wildlife trafficking', 'es' => 'tráfico de especies silvestres', 'fr' => 'contrebande d’espèces sauvages' ),
array( 'type' => 'keywords', 'en' => 'lead-acid batteries', 'es' => 'bateras de plomo-cido', 'fr' => 'batteries accumulateurs au plombd' ),
array( 'type' => 'keywords', 'en' => 'lead-acid batteries', 'es' => 'baterías de plomo-ácido', 'fr' => 'batteries d’accumulateurs au plomb' ),
array( 'type' => 'keywords', 'en' => 'ozone', 'es' => 'ozono', 'fr' => 'ozone' ),
);
    $i = 1;
    foreach ($data as $keyword) {
        
        //i18n_string_remove('islandora_multilingual' . $keyword['type'] . $i . 'name');
        //i18n_string_remove(array('islandora_multilingual', $keyword['type'], $i , 'name'));
        i18n_string_translation_update(array('islandora_multilingual', $keyword['type'], $i , 'name'), $keyword['es'], 'es', $keyword['en']);
        i18n_string_translation_update(array('islandora_multilingual', $keyword['type'], $i , 'name'), $keyword['fr'], 'fr', $keyword['en']);
        $i += 1;
    }
    drupal_goto('admin/islandora/multilingual');
        
}

