<?xml version="1.0" encoding="UTF-8"?>
<!-- Basic MODS -->
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xlink="http://www.w3.org/1999/xlink"
  xmlns:foxml="info:fedora/fedora-system:def/foxml#"
  xmlns:mods="http://www.loc.gov/mods/v3" 
  	exclude-result-prefixes="xlink mods">
    <!-- ---- ISLANDORA MULTILINGUAL ---- -->
    <!-- Multilingual configuration throughout: -->
    <!-- All fields are suffixed by the language suffix $lang passed in from foxmlToSolr. -->
    <!-- The datastream template requires datastreams to start with 'MODS-' instead of equal 'MODS'. -->
    <!-- ---- END ISLANDORA MULTILINGUAL ---- -->

  <xsl:template match="foxml:datastream[starts-with(@ID, 'MODS-')]/foxml:datastreamVersion[last()]" name="index_MODS" mode="activeFedoraObject">
    <xsl:param name="content">foxml:xmlContent</xsl:param>
    <xsl:param name="prefix">mods_</xsl:param>
    <xsl:param name="lang"><xsl:value-of select="concat('_', substring(./../@ID, 6, 2))"/></xsl:param>
   <xsl:param name="suffix">_ms</xsl:param>
    <xsl:apply-templates select="foxml:xmlContent/mods:mods">
      <xsl:with-param name="prefix" select="$prefix"/>
      <xsl:with-param name="suffix" select="$suffix"/>
      <xsl:with-param name="lang" select="$lang"/>
    </xsl:apply-templates>
  </xsl:template>



  <xsl:template match="mods:mods">
    <xsl:param name="prefix">mods_</xsl:param>
    <xsl:param name="suffix">_ms</xsl:param>
    <xsl:param name="lang"></xsl:param>

    <!-- Index stuff from the auth-module. -->
    <xsl:for-each select=".//*[@authorityURI='info:fedora'][@valueURI]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'related_object', $suffix)"/>
        </xsl:attribute>
        <xsl:value-of select="@valueURI"/>
      </field>
    </xsl:for-each>

    <!--************************************ MODS subset for Bibliographies ******************************************-->

    <!-- Main Title, with non-sorting prefixes -->
    <!-- ...specifically, this avoids catching relatedItem titles -->
    <!-- seems to ignore the second instance of titleInfo -->
    <xsl:for-each select="(mods:titleInfo[not(@type)]/mods:title[normalize-space(text())][1])">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:if test="../mods:nonSort">
          <xsl:value-of select="../mods:nonSort/text()"/>
          <xsl:text> </xsl:text>
        </xsl:if>
        <xsl:value-of select="text()"/>
      </field>
      <!-- bit of a hack so it can be sorted on... --> <!-- this seems to yield exactly the same as above. ??? -->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), '_mlt', $lang)"/>
        </xsl:attribute>
        <xsl:if test="../mods:nonSort">
          <xsl:value-of select="../mods:nonSort/text()"/>
          <xsl:text> </xsl:text>
        </xsl:if>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Sub-title -->
    <xsl:for-each select="mods:titleInfo[not(@type)]/mods:subTitle[normalize-space(text())][1]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), '_mlt', $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Part Name -->
    <xsl:for-each select="mods:titleInfo[not(@type)]/mods:partName[normalize-space(text())][1]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), '_mlt', $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Alternate Title, with non-sorting prefixes -->
    <!-- ...specifically, this avoids catching relatedItem titles -->
    <!-- seems to ignore the second instance of titleInfo -->
    <xsl:for-each select="(mods:titleInfo[@type='alternative']/mods:title[normalize-space(text())][1])">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'title_alternative', $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:if test="../mods:nonSort">
          <xsl:value-of select="../mods:nonSort/text()"/>
          <xsl:text> </xsl:text>
        </xsl:if>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Sub-title -->
    <xsl:for-each select="mods:titleInfo[@type='alternative']/mods:subTitle[normalize-space(text())][1]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'subTitle_alternative', $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>


    <!-- Abstract -->
    <xsl:for-each select="mods:abstract[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Genre (a.k.a. specific doctype) -->
    <xsl:for-each select="mods:genre[normalize-space(text())]">
      <xsl:variable name="authority">
        <xsl:choose>
          <xsl:when test="@authority">
            <xsl:value-of select="concat('_', @authority)"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>_local_authority</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $authority, $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Resource Type (a.k.a. broad doctype) -->
    <xsl:for-each select="mods:typeOfResource[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'resource_type', $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- DOI, ISSN, ISBN, and any other typed IDs -->
    <xsl:for-each select="mods:identifier[@type][normalize-space(text())]">
    <field>
      <xsl:attribute name="name">
        <xsl:choose>
	  <xsl:when test="@type='isbn'">
            <xsl:value-of select="concat($prefix, local-name(), '_', translate(@type, ' ', '_'), '_mtt', $lang)"/>
	  </xsl:when>
	  <xsl:otherwise>
	    <xsl:value-of select="concat($prefix, local-name(), '_', translate(@displayLabel, ' ', '_'), '_mtt', $lang)" />
	  </xsl:otherwise>
	</xsl:choose>
      </xsl:attribute>
      <xsl:value-of select="text()"/>
    </field>
    </xsl:for-each>

    <!-- Names and Roles
    @TODO: examine if formating the names is necessary?-->
      <!-- name type (corporate or personal) is being entirely ignored. -->
    <xsl:for-each select="mods:name[mods:namePart and mods:role]">
      <xsl:variable name="role" select="mods:role/mods:roleTerm[@type='code']/text()"/>
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'name_', $role, $suffix, $lang)"/>
        </xsl:attribute>
        <!-- <xsl:for-each select="../../namePart[@type='given']">-->
        <xsl:for-each select="mods:namePart[@type='given']">
          <xsl:value-of select="text()"/>
          <xsl:if test="string-length(text())=1">
            <xsl:text>.</xsl:text>
          </xsl:if>
          <xsl:text> </xsl:text>
        </xsl:for-each>
        <xsl:for-each select="mods:namePart[not(@type='given')]">
          <xsl:value-of select="text()"/>
          <xsl:if test="position()!=last()">
            <xsl:text> </xsl:text>
          </xsl:if>
        </xsl:for-each>
      </field>
      <!-- The following isn't strictly necessary unless we start using the 'given' name fields.-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'reversed_name_', $role, $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:for-each select="mods:namePart[not(@type='given')]">
          <xsl:value-of select="text()"/>
        </xsl:for-each>
        <xsl:for-each select="mods:namePart[@type='given']">
          <xsl:if test="position()=1">
            <xsl:text>, </xsl:text>
          </xsl:if>
          <xsl:value-of select="text()"/>
          <xsl:if test="string-length(text())=1">
            <xsl:text>.</xsl:text>
          </xsl:if>
          <xsl:if test="position()!=last()">
            <xsl:text> </xsl:text>
          </xsl:if>
        </xsl:for-each>
      </field>
    </xsl:for-each>

    <!-- Notes with no type -->
    <xsl:for-each select="mods:note[not(@type)][normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Notes -->
    <xsl:for-each select="mods:note[@type][normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), '_', translate(@type, ' ', '_'), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Subjects / Keywords without displayLabel -->
    <xsl:for-each select="mods:subject[not(@displayLabel)][normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Immediate children of Subjects / Keywords -->
    <xsl:for-each select="mods:subject[not(@displayLabel)]/*[normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'subject_', local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Subjects / Keywords with displaylabel -->
    <xsl:for-each select="mods:subject[@displayLabel][normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), '_', translate(@displayLabel, ' ', '_'), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Immediate children of Subjects / Keywords with displaylabel -->
    <xsl:for-each select="mods:subject[@displayLabel]/*[normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'subject_', local-name(), '_', translate(../@displayLabel, ' ', '_'), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>
     
     <!-- Grand- children of Subjects / Keywords with displaylabel -->
    <xsl:for-each select="mods:subject[@displayLabel]/*/*[normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'subject_', translate(../../@displayLabel, ' ', '_'), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>


    <!-- Coordinates (lat,long) -->
    <xsl:for-each select="mods:subject/mods:cartographics/mods:coordinates[normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Coordinates (lat,long) -->
    <xsl:for-each select="mods:subject/mods:topic[../cartographics/text()][normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'cartographic_topic', $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Country -->
    <xsl:for-each select="mods:country[normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <xsl:for-each select="mods:province[normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <xsl:for-each select="mods:county[normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <xsl:for-each select="mods:region[normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <xsl:for-each select="mods:city[normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <xsl:for-each select="mods:citySection[normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Host Name (i.e. journal/newspaper name) -->
    <xsl:for-each select="mods:relatedItem[@type='host']/mods:titleInfo/mods:title[normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'host_title', $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Series Name (this is e.g. a publication series and is sometimes used) -->
    <xsl:for-each select="mods:relatedItem[@type='series']/mods:titleInfo/mods:title[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'series_title', $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
      <field>
	<xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'series_volume', $suffix, $lang)"/>
	</xsl:attribute>
	<xsl:value-of select="normalize-space(../../mods:part/mods:detail[@type='volume']/mods:number/text())" />
      </field>
     </xsl:for-each>

    <!-- Referenced / is Referenced By (i.e. background paper and Secretariat Report) -->
    <xsl:for-each select="mods:relatedItem[@type='references']">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'references_title', $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="mods:titleInfo/mods:title[normalize-space(text())]"/>
      </field>
      <field>
	<xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'references_id', $suffix, $lang)"/>
	</xsl:attribute>
    <xsl:value-of select="mods:identifier[normalize-space(text())]" />
      </field>
  </xsl:for-each>

    <xsl:for-each select="mods:relatedItem[@type='isReferencedBy']">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'isReferencedBy_title', $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="mods:titleInfo/mods:title[normalize-space(text())]"/>
      </field>
      <field>
	<xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'isReferencedBy_id', $suffix, $lang)"/>
	</xsl:attribute>
    <xsl:value-of select="mods:identifier[normalize-space(text())]" />
      </field>
     </xsl:for-each>

    <!-- Volume (e.g. journal vol) -->
    <xsl:for-each select="mods:part/mods:detail[@type='volume']/*[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'volume', $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Issue (e.g. journal vol) -->
    <xsl:for-each select="mods:part/mods:detail[@type='issue']/*[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'issue', $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Subject Names -->
    <xsl:for-each select="mods:subject/mods:name/mods:namePart/*[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'subject', $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Physical Description -->
    <xsl:for-each select="mods:physicalDescription/*[not(@displayLabel='File size')][normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Physical Description (note - file size) -->
    <xsl:for-each select="mods:physicalDescription/mods:note[@displayLabel='File size'][normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
            <xsl:value-of select="concat($prefix, 'file_size', $suffix, $lang)"/>
	</xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Physical Description (form) -->
    <xsl:for-each select="mods:physicalDescription/mods:form[@type][normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'physical_description_', local-name(), '_', @type, $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Location -->
    <xsl:for-each select="mods:location[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Location (physical) -->
    <xsl:for-each select="mods:location/mods:physicalLocation[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Location (url) -->
    <xsl:for-each select="mods:location/mods:url[not(@access)][normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'location_', local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>
    <!-- Location (url, object in context) -->
    <xsl:for-each select="mods:location/mods:url[@access='object in context'][normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'location_', local-name(), '_context', $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>
    <!-- Location (url, raw object) -->
    <xsl:for-each select="mods:location/mods:url[@access='raw object'][normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'location_', local-name(), '_raw', $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Place of publication -->
    <xsl:for-each select="mods:originInfo/mods:place/mods:placeTerm[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, 'place_of_publication', $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Publisher's Name -->
    <xsl:for-each select="mods:originInfo/mods:publisher[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Edition (Book) -->
    <xsl:for-each select="mods:originInfo/mods:edition[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Date Issued (i.e. Journal Pub Date) -->
    <xsl:for-each select="mods:originInfo/mods:dateIssued[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
      <xsl:if test="position() = 1"><!-- use the first for a sortable field -->
        <field>
          <xsl:attribute name="name">
            <xsl:value-of select="concat($prefix, local-name(), '_s')"/>
          </xsl:attribute>
          <xsl:value-of select="text()"/>
        </field>
      </xsl:if>
    </xsl:for-each>

    <!-- Date Captured -->
    <xsl:for-each select="mods:originInfo/mods:dateCaptured[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
      <xsl:if test="position() = 1"><!-- use the first for a sortable field -->
        <field>
          <xsl:attribute name="name">
            <xsl:value-of select="concat($prefix, local-name(), '_s')"/>
          </xsl:attribute>
          <xsl:value-of select="text()"/>
        </field>
      </xsl:if>
    </xsl:for-each>

    <!-- Date Captured -->
    <xsl:for-each select="mods:originInfo/mods:dateCreated[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
      <xsl:if test="position() = 1"><!-- use the first for a sortable field -->
        <field>
          <xsl:attribute name="name">
            <xsl:value-of select="concat($prefix, local-name(), '_s')"/>
          </xsl:attribute>
          <xsl:value-of select="text()"/>
        </field>
      </xsl:if>
    </xsl:for-each>

    <!-- Copyright Date (is an okay substitute for Issued Date in many circumstances) -->
    <xsl:for-each select="mods:originInfo/mods:copyrightDate[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Issuance (i.e. ongoing, monograph, etc. ) -->
    <xsl:for-each select="mods:originInfo/mods:issuance[normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Language Term -->
    <xsl:for-each select="mods:language/mods:languageTerm[@authority='iso639-2b' and @type='code'][normalize-space(text())]">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>

    <!-- Access Condition -->
    <xsl:for-each select="mods:accessCondition[normalize-space(text())]">
      <!--don't bother with empty space-->
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat($prefix, local-name(), $suffix, $lang)"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>
    <!--************************************ END MODS subset for Bibliographies ******************************************-->
  </xsl:template>

</xsl:stylesheet>
