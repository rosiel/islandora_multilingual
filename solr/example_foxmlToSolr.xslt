<?xml version="1.0" encoding="UTF-8"?> 
<!-- $Id: foxmlToSolr.xslt $ -->
<xsl:stylesheet version="1.0"
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform"   
	    	xmlns:exts="xalan://dk.defxws.fedoragsearch.server.GenericOperationsImpl"
    		exclude-result-prefixes="exts"
		xmlns:foxml="info:fedora/fedora-system:def/foxml#"
		xmlns:dc="http://purl.org/dc/elements/1.1/"
		xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/"
		xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
		xmlns:myns="http://www.nsdl.org/ontologies/relationships#"
        xmlns:mods="http://www.loc.gov/mods/v3"
        xmlns:encoder="xalan://java.net.URLEncoder">
	<xsl:output method="xml" indent="yes" encoding="UTF-8"/>

<!--
	 This xslt stylesheet generates the Solr doc element consisting of field elements
     from a FOXML record. 
     You must specify the index field elements in solr's schema.xml file,
     including the uniqueKey element, which in this case is set to "PID".
     Options for tailoring:
       - generation of fields from other XML metadata streams than DC
       - generation of fields from other datastream types than XML
         - from datastream by ID, text fetched, if mimetype can be handled.
-->

	<xsl:param name="REPOSITORYNAME" select="repositoryName"/>
	<xsl:param name="FEDORASOAP" select="repositoryName"/>
	<xsl:param name="FEDORAUSER" select="repositoryName"/>
	<xsl:param name="FEDORAPASS" select="repositoryName"/>
	<xsl:param name="TRUSTSTOREPATH" select="repositoryName"/>
	<xsl:param name="TRUSTSTOREPASS" select="repositoryName"/>
	<xsl:variable name="PID" select="/foxml:digitalObject/@PID"/>

	<xsl:variable name="PROT" >http</xsl:variable>
        <xsl:variable name="FEDORAUSERNAME">fedoraAdmin</xsl:variable>
        <xsl:variable name="FEDORAPASSWORD">fedoraAdmin</xsl:variable>
        <xsl:variable name="HOST">localhost</xsl:variable>
        <xsl:variable name="PORT">8080</xsl:variable>

        <!-- ---- ISLANDORA MULTILINGUAL ---- -->
	<xsl:include href="/usr/local/fedora/tomcat/webapps/fedoragsearch/WEB-INF/classes/fgsconfigFinal/index/FgsIndex/MODS_to_solr.xslt" />
        <!-- ---- END ISLANDORA MULTILINGUAL ---- -->

	
	<xsl:template match="/">
		<!-- The following allows only active FedoraObjects to be indexed. -->
		<xsl:if test="foxml:digitalObject/foxml:objectProperties/foxml:property[@NAME='info:fedora/fedora-system:def/model#state' and @VALUE='Active']">
			<xsl:if test="not(foxml:digitalObject/foxml:datastream[@ID='METHODMAP'] or foxml:digitalObject/foxml:datastream[@ID='DS-COMPOSITE-MODEL'])">
				<xsl:if test="starts-with($PID,'')">
					<add> 
					  <doc> 
					    <xsl:apply-templates mode="activeFedoraObject"/>
					  </doc>
					</add>
				</xsl:if>
			</xsl:if>
		</xsl:if>
		<!-- The following allows inactive FedoraObjects to be deleted from the index. -->
		<xsl:if test="foxml:digitalObject/foxml:objectProperties/foxml:property[@NAME='info:fedora/fedora-system:def/model#state' and @VALUE='Inactive']">
			<xsl:if test="not(foxml:digitalObject/foxml:datastream[@ID='METHODMAP'] or foxml:digitalObject/foxml:datastream[@ID='DS-COMPOSITE-MODEL'])">
				<xsl:if test="starts-with($PID,'')">
					<xsl:apply-templates mode="inactiveFedoraObject"/>
				</xsl:if>
			</xsl:if>
		</xsl:if>
	</xsl:template>

  <xsl:template match="/foxml:digitalObject" mode="index_object_datastreams">
        <!-- records that there is a datastream -->
      <xsl:for-each select="foxml:datastream">
        <field name="fedora_datastreams_ms">
            <xsl:value-of select="@ID"/>
        </field>
      </xsl:for-each>
  </xsl:template>
    

	<xsl:template match="/foxml:digitalObject" mode="activeFedoraObject">


		<!-- THIS IS THE START OF THE DOCUMENT -->
			<field name="PID">	
				<xsl:value-of select="$PID"/>
			</field>
            <!-- This is from islandora solr configuration https://github.com/discoverygarden/basic-solr-config/blob/modular/demoFoxmlToSolr.xslt -->
            <xsl:apply-templates select="/foxml:digitalObject" mode="index_object_datastreams"/>


			<field name="REPOSITORYNAME">
				<xsl:value-of select="$REPOSITORYNAME"/>
			</field>
			<field name="REPOSBASEURL">
				<xsl:value-of select="substring($FEDORASOAP, 1, string-length($FEDORASOAP)-9)"/>
			</field>
			<xsl:for-each select="foxml:objectProperties/foxml:property">
				<field>
					<xsl:attribute name="name"> 
						<xsl:value-of select="concat('fgs.', substring-after(@NAME,'#'))"/>
					</xsl:attribute>
					<xsl:value-of select="@VALUE"/>
				</field>
			</xsl:for-each>


			<!-- here it gets the DC -->	
			<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/oai_dc:dc/*">
				<field>
					<xsl:attribute name="name">
						<xsl:value-of select="concat('dc.', substring-after(name(),':'))"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:for-each>



			<!-- Here it gets the date field -->
			<xsl:for-each select="foxml:datastream[@ID='DC']/foxml:datastreamVersion[last()]/foxml:xmlContent/oai_dc:dc/dc:date">
                <xsl:if test="text() != ''">
                    <xsl:if test="string-length(text()) = 10">
			  <field name="date_facet">
		   	     <xsl:value-of select="concat(text(), 'T00:00:00Z')"/>
			  </field>
                </xsl:if>
                <xsl:if test="string-length(text()) = 4">
                    <field name="date_facet">
                        <xsl:value-of select="concat(text(), '-01-01T00:00:00Z')"/>
                    </field>
                </xsl:if>
                </xsl:if>
			</xsl:for-each>

			<!-- get the RELS-INT -->
			<xsl:for-each select="foxml:datastream[@ID='RELS-INT']/foxml:datastreamVersion[last()]/foxml:xmlContent/rdf:RDF/rdf:Description/myns:isExpressionIn">
				<field>
					<xsl:attribute name="name">
						<xsl:value-of select="concat('frbr.expression-', text())"/>
					</xsl:attribute>
					<xsl:value-of select="../@rdf:about"/>
				</field>
			</xsl:for-each>
			
			<xsl:for-each select="foxml:datastream[@ID='RELS-INT']/foxml:datastreamVersion[last()]/foxml:xmlContent/rdf:RDF/rdf:Description/myns:isSummaryIn">
				<field>
					<xsl:attribute name="name">
						<xsl:value-of select="concat('frbr.summary-', text())"/>
					</xsl:attribute>
					<xsl:value-of select="../@rdf:about"/>
				</field>
			</xsl:for-each>

			<!-- get the RELS-EXT -->
			<xsl:for-each select="foxml:datastream[@ID='RELS-EXT']/foxml:datastreamVersion[last()]/foxml:xmlContent/rdf:RDF/rdf:Description/*">
				<field>
					<xsl:attribute name="name">
						<xsl:value-of select="concat('rels.', local-name())"/>
					</xsl:attribute>
					<xsl:if test="@rdf:resource">
						<xsl:value-of select="@rdf:resource"/>
					</xsl:if>
					<xsl:if test="not(@rdf:resource)">
						<xsl:value-of select="text()"/>
					</xsl:if>
				</field>
			</xsl:for-each>
			<!-- -->
			

			<!-- GET MODS STREAMS ("X" inline only) -->
			<xsl:apply-templates select="foxml:datastream[@ID='MODS-EN']/foxml:datastreamVersion[last()]" mode="activeFedoraObject"/>
			<xsl:apply-templates select="foxml:datastream[@ID='MODS-ES']/foxml:datastreamVersion[last()]" mode="activeFedoraObject"/>
			<xsl:apply-templates select="foxml:datastream[@ID='MODS-FR']/foxml:datastreamVersion[last()]" mode="activeFedoraObject"/>
			

            
            <!-- ---- ISLANDORA MULTILINGUAL ---- --> 
            <!-- GET MODS STREAMS ("M" Managed only) --> 
            <xsl:for-each select="foxml:datastream[@ID='MODS-EN' and @CONTROL_GROUP='M']">
				<xsl:variable name="MODS" select="document(concat($PROT, '://', $FEDORAUSERNAME, ':', $FEDORAPASSWORD, '@',
					$HOST, ':', $PORT, '/fedora/objects/', $PID, '/datastreams/', 'MODS-EN', '/content'))"/>
				<xsl:apply-templates select="$MODS/mods:mods">
					<xsl:with-param name="lang">_EN</xsl:with-param>
				</xsl:apply-templates>
			</xsl:for-each>
			<xsl:for-each select="foxml:datastream[@ID='MODS-ES' and @CONTROL_GROUP='M']">
				<xsl:variable name="MODS" select="document(concat($PROT, '://', $FEDORAUSERNAME, ':', $FEDORAPASSWORD, '@',
					$HOST, ':', $PORT, '/fedora/objects/', $PID, '/datastreams/', 'MODS-ES', '/content'))"/>
				<xsl:apply-templates select="$MODS/mods:mods">
					<xsl:with-param name="lang">_ES</xsl:with-param>
				</xsl:apply-templates>
			</xsl:for-each>
			<xsl:for-each select="foxml:datastream[@ID='MODS-FR' and @CONTROL_GROUP='M']">
				<xsl:variable name="MODS" select="document(concat($PROT, '://', $FEDORAUSERNAME, ':', $FEDORAPASSWORD, '@',
					$HOST, ':', $PORT, '/fedora/objects/', $PID, '/datastreams/', 'MODS-FR', '/content'))"/>
				<xsl:apply-templates select="$MODS/mods:mods">
					<xsl:with-param name="lang">_FR</xsl:with-param>
				</xsl:apply-templates>
			</xsl:for-each>
            <!-- ---- ISLANDORA MULTILINGUAL ---- --> 

			

            <!-- INDEX FULL TEXT [NEW] -->
            <xsl:for-each select="foxml:datastream">
                <xsl:choose>
                    <xsl:when test="@CONTROL_GROUP='X'">
                        <xsl:apply-templates select="foxml:datastreamVersion[last()]" mode="fulltext">
                            <xsl:with-param name="content" select="foxml:datastreamVersion[last()]/foxml:xmlContent"/>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:when test="@CONTROL_GROUP='M' and foxml:datastreamVersion[last()][@MIMETYPE='text/xml' or @MIMETYPE='application/xml' or @MIMETYPE='application/rdf+xml' or @MIMETYPE='text/html']">
                        <xsl:apply-templates select="foxml:datastreamVersion[last()]" mode="fulltext">
                            <xsl:with-param name="content" select="document(concat($PROT, '://', encoder:encode($FEDORAUSER), ':', encoder:encode($FEDORAPASS), '@', $HOST, ':', $PORT, '/fedora/objects/', $PID, '/datastreams/', @ID, '/content'))" />
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:when test="@CONTROL_GROUP='M' and foxml:datastreamVersion[last() and not(starts-with(@MIMETYPE, 'image'))]">
                        <xsl:apply-templates select="foxml:datastreamVersion[last()]" mode="fulltext">
                             <xsl:with-param name="content" select="normalize-space(exts:getDatastreamText($PID, $REPOSITORYNAME, @ID, $FEDORASOAP, $FEDORAUSER, $FEDORAPASS, $TRUSTSTOREPATH, $TRUSTSTOREPASS))" />
                         </xsl:apply-templates>
                     </xsl:when>
                </xsl:choose>
            </xsl:for-each>

            <!-- Extra indexing for OBJ datastreams (i.e. label = filename) -->
            <xsl:for-each select="foxml:datastream[starts-with(@ID, 'OBJ')]">
                <xsl:apply-templates select="foxml:datastreamVersion[last()]" mode="labels">
                </xsl:apply-templates>
            </xsl:for-each>

	</xsl:template>

	<xsl:template match="/foxml:digitalObject" mode="inactiveFedoraObject">
		<delete> 
			<id><xsl:value-of select="$PID"/></id>
		</delete>
    </xsl:template>

    <xsl:template match="foxml:datastream[@ID='DC' or starts-with(@ID, 'MODS')]/foxml:datastreamVersion[last()]" mode="fulltext">
        <xsl:param name="content"/>
        <field name="foxml.all.text">
            <xsl:apply-templates select="$content" mode="index_text_nodes_as_a_single_field" />
        </field>
    </xsl:template>
    <xsl:template match="foxml:datastream[starts-with(@ID, 'OBJ')]/foxml:datastreamVersion[last()]" mode="fulltext">
        <xsl:param name="content"/>
        <field name="foxml.all.text">
            <xsl:value-of select="$content"/>
        </field>
        <field>
            <xsl:attribute name="name">
                <xsl:value-of select="concat('ds.size.', ../@ID, '_tl')"/>
            </xsl:attribute>
            <xsl:value-of select="@SIZE" />
        </field>
    </xsl:template>

    <!-- Template for indexing OBJ datastreams with datstream label -->
    <xsl:template match="foxml:datastream[starts-with(@ID, 'OBJ')]/foxml:datastreamVersion[last()]" mode="labels">
        <field>
            <xsl:attribute name="name">
                <xsl:value-of select="concat('ds.label.OBJ', '')"/>
            </xsl:attribute>
            <xsl:value-of select="@LABEL" />
        </field>
    </xsl:template>

    <xsl:template match="text()" mode="index_text_nodes_as_a_single_field">
        <xsl:variable name="text" select="normalize-space(.)"/>
        <xsl:if test="$text">
            <xsl:value-of select="$text" />
            <xsl:text> </xsl:text>
        </xsl:if>
    </xsl:template>
	
</xsl:stylesheet>	
