SUMMARY
-------

Islandora Multilingual Configuration defines language suffixes for all displayable object parts (datastreams and solr fields). It is based off of the PDF solution pack. 

REQUIREMENTS
------------
* Islandora
* Islandora Solr Search

CONFIGURATION
-------------

Before creating multilingual objects, you must:
* create a collection for them
* alter the MODS-to-Solr transform to append the language suffix to each field, e.g. mods_titleInfo_title_ms_EN
* Create a metadata form for each language, 
 * which contains a textfield called 'label'
 * which contains a (hidden) field specifying the languageOfCataloging as that particular language
 * which is associated with the appropriate datastream (e.g. MODS-DE)
 ** and in the association, the 'label' field above is used for the Fedora label
