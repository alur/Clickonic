<?xml version="1.0" encoding="UTF-8"?>

<!DOCTYPE xsl:stylesheet [
  <!ENTITY nbsp "&#160;"><!ENTITY copy "&#169;"><!ENTITY emdash "&#x2014;">
]>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<!--
		LSMD v0.3p (XML) -> ruDocs (HTML)
		Copyright(c) Seg@, October 2004
		
		It isn't an universal convertor like a RabidCow's one.
		I'm a really pure HTML and XSLT coder, so here can
		be much more mistakes that you can expect...
		
		By the way it is a ripped RabidCow's XSL scheme =)
	-->

	<!-- Entry point -->
	
	<xsl:template match="/lsmd">
		<html>
			<head>
				<link rel="StyleSheet" href="style.css" type="text/css"/>
				<title>
					<xsl:for-each select="module">
						<xsl:choose>
							<xsl:when test="@friendlyname">
								<xsl:value-of select="@friendlyname"/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:value-of select="concat(@basename,' ',@version)"/>
							</xsl:otherwise>
						</xsl:choose>
						<xsl:if test="position() != last()">
							<xsl:text>, </xsl:text>
						</xsl:if>
					</xsl:for-each>
				</title>
			</head>
			<body>
				<xsl:apply-templates select="module" mode="doc"/>
			</body>
		</html>
	</xsl:template>


	<!-- Section header -->
	
	<xsl:template name="section-header">
		<xsl:param name="title"/>
		<xsl:param name="id"/>
		<xsl:text>&#13;</xsl:text>
		<div class="midhead">
			<xsl:element name="a">
				<xsl:attribute name="name">
					<xsl:value-of select="$id"/>
				</xsl:attribute>
				<xsl:value-of select="$title"/>
			</xsl:element>
		</div>
	</xsl:template>
	
	
	<!-- Show default value of the specified data object-->
	

	<xsl:key name="action-search" match="action" use="@id"/>
	<xsl:key name="bool-search" match="bool" use="@id"/>
	<xsl:key name="color-search" match="color" use="@id"/>
	<xsl:key name="coordinate-search" match="coordinate" use="@id"/>
	<xsl:key name="file-search" match="file" use="@id"/>
	<xsl:key name="flags-search" match="flags" use="@id"/>
	<xsl:key name="folder-search" match="folder" use="@id"/>
	<xsl:key name="command-search" match="command" use="@id"/>
	<xsl:key name="group-search" match="group" use="@id"/>
	<xsl:key name="font-search" match="font" use="@id"/>
	<xsl:key name="image-search" match="image" use="@id"/>
	<xsl:key name="integer-search" match="integer" use="@id"/>
	<xsl:key name="option-search" match="option" use="@id"/>
	<xsl:key name="key-search" match="key" use="@id"/>
	<xsl:key name="sound-search" match="sound" use="@id"/>
	<xsl:key name="string-search" match="string" use="@id"/>
	<xsl:key name="url-search" match="url" use="@id"/>

	<xsl:template name="default-value">
		<xsl:param name="datatype"/>
		<xsl:param name="dataid"/>
		<xsl:for-each select="/lsmd/module/data">
			<xsl:for-each select="key(concat($datatype,'-search'), $dataid)">
				<br /><u>Default</u> : <xsl:value-of select="@default"/>
			</xsl:for-each>
		</xsl:for-each>
	</xsl:template>
	
	<!-- Show information about developers -->

	<xsl:template name="show-developers">
		<xsl:for-each select="author">
			<div class="lasttext">
				Handle:
				<b>
				<xsl:if test="@name">
					<xsl:value-of select="@name"/> a.k.a.
				</xsl:if>
				<xsl:value-of select="text()"/>
				</b>
				<br/>
				<xsl:if test="@email">
					E-Mail : <b>
					<xsl:element name="a">
						<xsl:attribute name="href">
							<xsl:value-of select="concat('mailto:', @email)"/>
						</xsl:attribute>
						<xsl:value-of select="@email"/>
					</xsl:element>
					</b>
					<br />
				</xsl:if>
				<xsl:if test="@web">
					Web : <b>
					<xsl:element name="a">
						<xsl:attribute name="href">
							<xsl:value-of select="@web"/>
						</xsl:attribute>
						<xsl:value-of select="@web"/>
					</xsl:element>
					</b>
					<br />
				</xsl:if>
				<xsl:if test="@icq">
					ICQ: <xsl:value-of select="@icq"/>
				</xsl:if>
			</div>
			<br />
		</xsl:for-each>
	</xsl:template>
	

	<!-- Section footer -->
	
	<xsl:template name="section-footer">
		<div class="totop"><a href="#top">up</a></div>
	</xsl:template>


	<!-- Author (print) -->
	
	<xsl:template match="author" mode="print">
		<xsl:if test="position() = last() and last() != 1">and </xsl:if>
		<xsl:choose>
			<xsl:when test="@email">
				<!-- we have an emal address -->
				<xsl:element name="a">
					<xsl:attribute name="href">
						<xsl:value-of select="concat('mailto:',@email)"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</xsl:element>
			</xsl:when>
			<xsl:otherwise>
				<!-- we don't have an email address -->
				<xsl:value-of select="text()"/>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:if test="position() != last() and last() > 2">,</xsl:if>
		<xsl:if test="position() != last()">
			<xsl:text> </xsl:text>
		</xsl:if>
	</xsl:template>


	<!-- Copyright (nolink) -->
	
	<xsl:template match="copyright" mode="nolink">
		<xsl:if test="position() = last() and last() != 1">and </xsl:if>
		<xsl:value-of select="@year"/>
		<xsl:text> </xsl:text>
		<xsl:value-of select="text()"/>
		<xsl:if test="position() != last() and last() > 2">,</xsl:if>
		<xsl:if test="position() != last()">
			<xsl:text> </xsl:text>
		</xsl:if>
	</xsl:template>
	
	
	<!-- Module (doc) -->
	<xsl:template match="module" mode="doc">
	
		<!-- Module name -->
		<div id="top" class="head">
			<xsl:choose>
				<xsl:when test="@friendlyname">
					<xsl:value-of select="@friendlyname"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="concat(@basename,' ',@version)"/>
				</xsl:otherwise>
			</xsl:choose>
		</div>
		
		<!-- Copyright -->
		<div class="copyright">
			<xsl:text>Copyright &copy; </xsl:text>
			<xsl:apply-templates select="/lsmd/copyright" mode="nolink"/>
		</div>
		
		<!-- Description -->
		<xsl:call-template name="section-header">
			<xsl:with-param name="id">about</xsl:with-param>
			<xsl:with-param name="title">About:</xsl:with-param>
		</xsl:call-template>
		<div class="text">
			<xsl:value-of select="documentation/description"/>
			<xsl:if test="link">
				<br/>
				<ul class="link">
					<xsl:apply-templates select="link" mode="list"/>
				</ul>
			</xsl:if>
		</div>
		
		<!-- The Table of Contents -->
		<xsl:if test="not(//section/@id='toc')">
			<xsl:call-template name="section-header">
				<xsl:with-param name="id">toc</xsl:with-param>
				<xsl:with-param name="title">The Table of Contents:</xsl:with-param>
			</xsl:call-template>
			<xsl:apply-templates select="documentation" mode="toc"/>
			<xsl:call-template name="section-footer"/>
		</xsl:if>
		
		<!-- Main documentation -->
		<xsl:apply-templates select="documentation" mode="print"/>
		
		<!-- History -->
		<xsl:if test="history and not(//section/@id='changes')">
			<xsl:call-template name="section-header">
				<xsl:with-param name="id">changes</xsl:with-param>
				<xsl:with-param name="title">History</xsl:with-param>
			</xsl:call-template>
			<xsl:apply-templates select="history/revision" mode="print"/>
			<xsl:call-template name="section-footer"/>
		</xsl:if>
		
		<!-- Developers -->
		<xsl:if test="author">
			<xsl:call-template name="section-header">
				<xsl:with-param name="id">dev</xsl:with-param>
				<xsl:with-param name="title">Developers</xsl:with-param>
			</xsl:call-template>
			<xsl:call-template name="show-developers"/>
		</xsl:if>
		
	</xsl:template>
	
	
	<!-- Documentation (The Table of Contents) -->
	
	<xsl:template match="documentation" mode="toc">
		<div class="text">
		<xsl:for-each select="section">
			<li>
			<xsl:element name="a">
				<xsl:attribute name="href"><xsl:value-of select="concat('#',@id)"/></xsl:attribute>
				<xsl:value-of select="@name"/>
			</xsl:element>
			</li>
		</xsl:for-each>
		<xsl:if test="../history and not(//section/@id='changes')">
			<li>
				<xsl:element name="a">
					<xsl:attribute name="href">#changes</xsl:attribute>History</xsl:element>
			</li>
		</xsl:if>
		<li>
			<xsl:element name="a"><xsl:attribute name="href">#dev</xsl:attribute>Developers</xsl:element>
		</li>
		</div>
	</xsl:template>
	
	<!-- Documentation (Complete) -->
	
	<xsl:template match="documentation" mode="print">
		<xsl:choose>
			<xsl:when test="section">
				<xsl:for-each select="section">
					<xsl:variable name="section">
						<xsl:value-of select="concat(' ',@id,' ')"/>
					</xsl:variable>
					<xsl:call-template name="section-header">
						<xsl:with-param name="id">
							<xsl:value-of select="@id"/>
						</xsl:with-param>
						<xsl:with-param name="title" select="@name"/>
					</xsl:call-template>
					<div class="text">
						<xsl:apply-templates select="description" mode="print-section-header"/>
						<xsl:apply-templates select="../..//*[contains(concat(' ',@section,' '),$section)]" mode="print"/>
						<xsl:text> </xsl:text>
						<xsl:apply-templates select="description" mode="print-section-footer"/>
					</div>
					<xsl:call-template name="section-footer"/>
				</xsl:for-each>
			</xsl:when>
			<xsl:otherwise>
				<xsl:if test="../..//single|../..//multiple">
					<xsl:call-template name="section-header">
						<xsl:with-param name="id">config</xsl:with-param>
						<xsl:with-param name="title">Configuration</xsl:with-param>
					</xsl:call-template>
					<div class="text">
						<xsl:element name="p"/>
						<xsl:apply-templates select="../..//single|../..//multiple" mode="print"/>
						<xsl:text> </xsl:text>
					</div>
				</xsl:if>
				<xsl:if test="../..//bang">
					<xsl:call-template name="section-header">
						<xsl:with-param name="id">control</xsl:with-param>
						<xsl:with-param name="title">Bang Commands</xsl:with-param>
					</xsl:call-template>
					<div class="text">
						<xsl:element name="p"/>
						<xsl:apply-templates select="../..//bang" mode="print"/>
						<xsl:text> </xsl:text>
					</div>
				</xsl:if>
				<xsl:if test="../..//var">
					<xsl:call-template name="section-header">
						<xsl:with-param name="id">vars</xsl:with-param>
						<xsl:with-param name="title">Variables</xsl:with-param>
					</xsl:call-template>
					<div class="text">
						<xsl:element name="p"/>
						<xsl:apply-templates select="../..//var" mode="print"/>
						<xsl:text> </xsl:text>
					</div>
				</xsl:if>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	<xsl:template match="description" mode="print-section-header">
		<xsl:if test="position()=1">
			<xsl:apply-templates select="." mode="print-desc"/>
		</xsl:if>
	</xsl:template>
	
	<xsl:template match="description" mode="print-section-footer">
		<xsl:if test="position()=2">
			<xsl:apply-templates select="." mode="print-desc"/>
		</xsl:if>
	</xsl:template>

<!-- the comment of truth -->

	<xsl:key name="prefix" match="key" use="@id"/>
	
	<xsl:template match="multiple" mode="print">
		<li>
			<b>
				<xsl:element name="a">
					<xsl:attribute name="name"><xsl:call-template name="anchor-name"><xsl:with-param name="link"><xsl:choose><xsl:when test="@star='false'"/><xsl:otherwise>*</xsl:otherwise></xsl:choose><xsl:for-each select="ancestor-or-self::*/@prefix"><xsl:choose><xsl:when test="key('prefix',.)/@default"><xsl:value-of select="key('prefix',.)/@default"/></xsl:when><xsl:otherwise><xsl:value-of select="key('prefix',.)/@id"/></xsl:otherwise></xsl:choose></xsl:for-each><xsl:value-of select="@name"/></xsl:with-param></xsl:call-template></xsl:attribute>
					<xsl:choose>
						<xsl:when test="@star='false'"/>
						<xsl:otherwise>*</xsl:otherwise>
					</xsl:choose>
					<xsl:for-each select="ancestor-or-self::*/@prefix">
						<xsl:choose>
							<xsl:when test="key('prefix',.)/@default">
								<xsl:value-of select="key('prefix',.)/@default"/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:value-of select="key('prefix',.)/@id"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:for-each>
					<xsl:value-of select="@name"/>
				</xsl:element>
				<xsl:apply-templates select="child::*" mode="params"/>
			</b><br/>
			<xsl:if test="description">
				<xsl:apply-templates select="description" mode="print-desc"/>
			</xsl:if>
		</li>
	</xsl:template>
	
	<xsl:template match="single" mode="print">
		<li>
			<b>
				<xsl:element name="a">
					<xsl:attribute name="name"><xsl:call-template name="anchor-name"><xsl:with-param name="link"><xsl:for-each select="ancestor-or-self::*/@prefix"><xsl:choose><xsl:when test="key('prefix',.)/@default"><xsl:value-of select="key('prefix',.)/@default"/></xsl:when><xsl:otherwise><span class="prefix"><xsl:value-of select="key('prefix',.)/@id"/></span></xsl:otherwise></xsl:choose></xsl:for-each><xsl:value-of select="@name"/></xsl:with-param></xsl:call-template></xsl:attribute>
					<xsl:for-each select="ancestor-or-self::*/@prefix">
						<xsl:choose>
							<xsl:when test="key('prefix',.)/@default">
								<xsl:value-of select="key('prefix',.)/@default"/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:value-of select="key('prefix',.)/@id"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:for-each>
					<xsl:value-of select="@name"/>
				</xsl:element>
				<xsl:apply-templates select="child::*" mode="params"/>
			</b>
			<br/>
			<xsl:if test="description">
				<xsl:apply-templates select="description" mode="print-desc"/>
			</xsl:if>
			<!--
				now let's write default value... it is really hellish, but I
				don't know how to implement this in a proper way
			-->
			<xsl:for-each select="action">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">action</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
			<xsl:for-each select="bool">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">bool</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
			<xsl:for-each select="command">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">command</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
			<xsl:for-each select="color">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">color</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
			<xsl:for-each select="coordinate">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">coordinate</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
			<xsl:for-each select="file">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">file</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
			<xsl:for-each select="flags">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">flags</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
			<xsl:for-each select="folder">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">folder</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
			<xsl:for-each select="font">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">font</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
			<xsl:for-each select="image">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">image</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
			<xsl:for-each select="integer">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">integer</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
			<xsl:for-each select="option">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">option</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
			<xsl:for-each select="key">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">key</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
			<xsl:for-each select="sound">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">sound</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
			<xsl:for-each select="string">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">string</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
			<xsl:for-each select="url">
				<xsl:call-template name="default-value">
					<xsl:with-param name="datatype">url</xsl:with-param>
					<xsl:with-param name="dataid"><xsl:value-of select="@data"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
		</li>
	</xsl:template>
	
	<xsl:template match="bang" mode="print">
		<li>
			<b>
				<xsl:element name="a">
					<xsl:attribute name="name"><xsl:call-template name="anchor-name"><xsl:with-param name="link">!<xsl:for-each select="ancestor-or-self::*/@prefix"><xsl:choose><xsl:when test="key('prefix',.)/@default"><xsl:value-of select="key('prefix',.)/@default"/></xsl:when><xsl:otherwise><span class="prefix"><xsl:value-of select="key('prefix',.)/@id"/></span></xsl:otherwise></xsl:choose></xsl:for-each><xsl:value-of select="@name"/></xsl:with-param></xsl:call-template></xsl:attribute>!<xsl:for-each select="ancestor-or-self::*/@prefix">
						<xsl:choose>
							<xsl:when test="key('prefix',.)/@default">
								<xsl:value-of select="key('prefix',.)/@default"/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:value-of select="key('prefix',.)/@id"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:for-each>
					<xsl:value-of select="@name"/>
				</xsl:element>
				<xsl:apply-templates select="child::*" mode="params"/>
			</b>
			<br/>
			<xsl:if test="description">
				<xsl:apply-templates select="description" mode="print-desc"/>
			</xsl:if>
		</li>
	</xsl:template>
	
	<xsl:template match="var" mode="print">
		<xsl:element name="dt">
			<xsl:if test="description">
				<xsl:attribute name="class">var</xsl:attribute>
			</xsl:if>
			<xsl:if test="not(description)">
				<xsl:attribute name="class">var-short</xsl:attribute>
			</xsl:if>
			<xsl:element name="a">
				<xsl:attribute name="name">
					<xsl:call-template name="anchor-name">
						<xsl:with-param name="link">
							<xsl:for-each select="ancestor-or-self::*/@prefix">
								<xsl:choose>
									<xsl:when test="key('prefix',.)/@default">
										<xsl:value-of select="key('prefix',.)/@default"/>
									</xsl:when>
									<xsl:otherwise>
										<xsl:value-of select="key('prefix',.)/@id"/>
									</xsl:otherwise>
								</xsl:choose>
							</xsl:for-each>
							<xsl:value-of select="@name"/>
						</xsl:with-param>
					</xsl:call-template>
				</xsl:attribute>
				<xsl:choose>
					<xsl:when test="ancestor-or-self::*/@delim-begin">
						<xsl:value-of select="ancestor-or-self::*/@delim-begin"/>
					</xsl:when>
					<xsl:otherwise><xsl:text>$</xsl:text></xsl:otherwise>
				</xsl:choose>
				<xsl:for-each select="ancestor-or-self::*/@prefix">
					<xsl:choose>
						<xsl:when test="key('prefix',.)/@default">
							<xsl:value-of select="key('prefix',.)/@default"/>
						</xsl:when>
						<xsl:otherwise>
							<xsl:value-of select="key('prefix',.)/@id"/>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:for-each>
				<xsl:value-of select="@name"/>
				<xsl:choose>
					<xsl:when test="ancestor-or-self::*/@delim-end">
						<xsl:value-of select="ancestor-or-self::*/@delim-end"/>
					</xsl:when>
					<xsl:otherwise><xsl:text>$</xsl:text></xsl:otherwise>
				</xsl:choose>
			</xsl:element>
			<xsl:apply-templates select="child::*" mode="params"/>
		</xsl:element>
		<xsl:if test="description">
			<xsl:apply-templates select="description" mode="print-desc"/>
		</xsl:if>
	</xsl:template>

	<xsl:template match="group" mode="print">
		<xsl:apply-templates select="*" mode="print"/>
		<xsl:if test="description">
			<xsl:apply-templates select="description" mode="print-desc"/>
		</xsl:if>
	</xsl:template>
	<xsl:template name="anchor-name">
		<xsl:param name="link"/>
		<xsl:choose>
			<xsl:when test="starts-with($link,'*')">
<!-- not enough for non-ascii case conversion, but I don't care. -->
				<xsl:value-of select="concat('star_',translate(substring($link,2),'ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz'))"/>
			</xsl:when>
			<xsl:when test="starts-with($link,'!')">
				<xsl:value-of select="concat('bang_',translate(substring($link,2),'ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz'))"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="translate($link,'ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz')"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<xsl:template match="p" mode="print">
	 <!-- some browsers have baggage preventing p nesting -->
		<xsl:apply-templates select="."/>
	</xsl:template>
	
	<xsl:template match="example" mode="print">
		<pre class="code">
			<xsl:apply-templates select="."/>
		</pre>
	</xsl:template>
	
	<xsl:template match="table" mode="print">
		<xsl:element name="a">
			<xsl:attribute name="name"><xsl:value-of select="@id"/></xsl:attribute>
			<xsl:if test="@caption">
				<div class="table-caption"><xsl:value-of select="@caption"/></div>
			</xsl:if>
			<xsl:apply-templates select="item" mode="print-table"/>
		</xsl:element>
	</xsl:template>
	
	<xsl:template match="item" mode="print-table">
		<dt><xsl:value-of select="@name"/></dt>
		<dd><xsl:apply-templates select="."/></dd>
	</xsl:template>
	
	<xsl:template match="code">
		<xsl:choose>
			<xsl:when test="contains(.,'&#10;')">
				<!--If the code block contains a newline, preserve all whitespace.-->
				<pre class="code">
					<xsl:apply-templates select="." mode="in-code"/>
				</pre>
			</xsl:when>
			<xsl:otherwise>
				<!--Otherwise collapse it, so code can be used inline.-->
				<tt>
					<xsl:apply-templates select="." mode="in-code"/>
				</tt>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<xsl:template match="xref" mode="in-code">
		<xsl:choose>
			<!-- @iscode is meaningless inside a code element -->
			<xsl:when test="@link">
				<span class="xref">
					<xsl:element name="a">
						<xsl:attribute name="href">#<xsl:call-template name="anchor-name">
				<xsl:with-param name="link"><xsl:value-of select="@link"/></xsl:with-param>
			</xsl:call-template></xsl:attribute>
						<xsl:attribute name="title"><xsl:value-of select="@link"/></xsl:attribute>
						<xsl:value-of select="."/>
					</xsl:element>
				</span>
			</xsl:when>
			<xsl:otherwise>
				<tt class="xref">
					<xsl:element name="a">
						<xsl:attribute name="href">#<xsl:call-template name="anchor-name">
				<xsl:with-param name="link"><xsl:value-of select="."/></xsl:with-param>
			</xsl:call-template></xsl:attribute>
						<xsl:value-of select="."/>
					</xsl:element>
				</tt>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<xsl:template match="xref">
		<xsl:choose>
			<xsl:when test="@link and @iscode='true'">
				<tt class="xref">
					<xsl:element name="a">
						<xsl:attribute name="href">#<xsl:call-template name="anchor-name">
				<xsl:with-param name="link"><xsl:value-of select="@link"/></xsl:with-param>
			</xsl:call-template></xsl:attribute>
						<xsl:attribute name="title"><xsl:value-of select="@link"/></xsl:attribute>
						<xsl:value-of select="."/>
					</xsl:element>
				</tt>
			</xsl:when>
			<xsl:when test="@link">
				<span class="xref">
					<xsl:element name="a">
						<xsl:attribute name="href">#<xsl:call-template name="anchor-name">
				<xsl:with-param name="link"><xsl:value-of select="@link"/></xsl:with-param>
			</xsl:call-template></xsl:attribute>
						<xsl:attribute name="title"><xsl:value-of select="@link"/></xsl:attribute>
						<xsl:value-of select="."/>
					</xsl:element>
				</span>
			</xsl:when>
			<xsl:when test="@iscode='false'">
				<span class="xref">
					<xsl:element name="a">
						<xsl:attribute name="href">#<xsl:call-template name="anchor-name">
				<xsl:with-param name="link"><xsl:value-of select="."/></xsl:with-param>
			</xsl:call-template></xsl:attribute>
						<xsl:value-of select="."/>
					</xsl:element>
				</span>
			</xsl:when>
			<xsl:otherwise>
				<tt class="xref">
					<xsl:element name="a">
						<xsl:attribute name="href">#<xsl:call-template name="anchor-name">
				<xsl:with-param name="link"><xsl:value-of select="."/></xsl:with-param>
				</xsl:call-template></xsl:attribute>
						<xsl:value-of select="."/>
					</xsl:element>
				</tt>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<xsl:template match="strong">
		<strong><xsl:apply-templates select="text()"/></strong>
	</xsl:template>
	
	<xsl:template match="em">
		<em><xsl:apply-templates select="text()"/></em>
	</xsl:template>

	<xsl:template match="multiple" mode="summary">
		<li>
			<xsl:choose>
				<xsl:when test="@star='false'"/>
				<xsl:otherwise>*</xsl:otherwise>
			</xsl:choose>
			<xsl:for-each select="ancestor-or-self::*/@prefix">
				<xsl:choose>
					<xsl:when test="key('prefix',.)/@default">
						<xsl:value-of select="key('prefix',.)/@default"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="key('prefix',.)/@id"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:for-each>
			<xsl:value-of select="@name"/>
			<xsl:apply-templates select="child::*" mode="params"/>
		</li>
	</xsl:template>
	
	<xsl:template match="single" mode="summary">
		<li>
			<xsl:for-each select="ancestor-or-self::*/@prefix">
				<xsl:choose>
					<xsl:when test="key('prefix',.)/@default">
						<xsl:value-of select="key('prefix',.)/@default"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="key('prefix',.)/@id"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:for-each>
			<xsl:value-of select="@name"/>
			<xsl:apply-templates select="child::*" mode="params"/>
		</li>
	</xsl:template>
	
	<xsl:template match="bang" mode="summary">
		<li>!<xsl:for-each select="ancestor-or-self::*/@prefix">
				<xsl:choose>
					<xsl:when test="key('prefix',.)/@default">
						<xsl:value-of select="key('prefix',.)/@default"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="key('prefix',.)/@id"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:for-each>
			<xsl:value-of select="@name"/>
			<xsl:apply-templates select="child::*" mode="params"/>
		</li>
	</xsl:template>
	
	<xsl:key name="prefix" match="key" use="@id"/>
	
	<xsl:template match="action" mode="params">
		<xsl:text> [action]</xsl:text>
	</xsl:template>
	
	<xsl:template match="bool" mode="params">
		<xsl:text> [false/true]</xsl:text>
	</xsl:template>
	
	<xsl:template match="tbool" mode="params">
		<xsl:text> [false/true/toggle]</xsl:text>
	</xsl:template>
	
	<xsl:template match="color" mode="params">
		<xsl:text> [color]</xsl:text>
	</xsl:template>
	
	<xsl:template match="coordinate" mode="params">
		<xsl:text> [coordinate]</xsl:text>
	</xsl:template>
	
	<xsl:template match="file" mode="params">
		<xsl:text> [file]</xsl:text>
	</xsl:template>
	
	<xsl:template match="flags" mode="params">
		<xsl:text> [#flags]</xsl:text>
	</xsl:template>
	
	<xsl:template match="folder" mode="params">
		<xsl:text> [folder]</xsl:text>
	</xsl:template>
	
	<xsl:template match="font" mode="params">
		<xsl:text> [font]</xsl:text>
	</xsl:template>
	
	<xsl:template match="group" mode="params">
		<xsl:text> [group]</xsl:text>
	</xsl:template>
	
	<xsl:template match="evar" mode="params">
		<xsl:text> [evar]</xsl:text>
	</xsl:template>
	
	<xsl:template match="deprecated" mode="params">
		<span style="color: red"><xsl:text> DEPRECATED!</xsl:text></span>
	</xsl:template>
	
	<xsl:template match="command" mode="params">
		<xsl:text> [command]</xsl:text>
	</xsl:template>
	
	<xsl:template match="image" mode="params">
		<xsl:text> [image]</xsl:text>
	</xsl:template>
	
	<xsl:template match="integer" mode="params">
		<xsl:text> [integer]</xsl:text>
	</xsl:template>
	
	<xsl:template match="option" mode="params">
		<xsl:text> [option]</xsl:text>
	</xsl:template>
	
	<xsl:template match="key" mode="params">
		<xsl:text> [name]</xsl:text>
	</xsl:template>
	
	<xsl:template match="sound" mode="params">
		<xsl:text> [sound]</xsl:text>
	</xsl:template>
	
	<xsl:template match="string" mode="params">
		<xsl:text> [string]</xsl:text>
	</xsl:template>
	
	<xsl:template match="url" mode="params">
		<xsl:text> [url]</xsl:text>
	</xsl:template>
	
	<xsl:template match="repeat" mode="params">
		<xsl:text> </xsl:text>
		<span class="repeat">
			<xsl:apply-templates select="child::*" mode="params"/>
			<span class="again">
				<xsl:apply-templates select="child::*" mode="params"/>
			</span>...</span>
	</xsl:template>
	
	<xsl:template match="description" mode="params"/>
	<xsl:template match="description" mode="print"/>
	<xsl:template match="description" mode="print-desc">
		<xsl:apply-templates select="*" mode="print"/>
	</xsl:template>
	
	<xsl:template match="fixed" mode="changes">
		<xsl:text>&#13;</xsl:text>
		<li><b>FIXED</b>: <xsl:apply-templates select="."/></li>
	</xsl:template>
	
	<xsl:template match="added" mode="changes">
		<xsl:text>&#13;</xsl:text>
		<li><b>ADDED</b>: <xsl:apply-templates select="."/></li>
	</xsl:template>
	
	<xsl:template match="changed" mode="changes">
		<xsl:text>&#13;</xsl:text>
		<li><b>CHANGED</b>: <xsl:apply-templates select="."/></li>
	</xsl:template>
	
	<xsl:template match="removed" mode="changes">
		<xsl:text>&#13;</xsl:text>
		<li><b>REMOVED</b>: <xsl:apply-templates select="."/></li>
	</xsl:template>
	
	<!-- Version info -->
	
	<xsl:template match="revision" mode="print">
		<div class="lasttext">
			<xsl:text>Version </xsl:text>
			<xsl:choose>
				<xsl:when test="@version">
					<xsl:value-of select="@version"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:text>_unknown_</xsl:text>
				</xsl:otherwise>
			</xsl:choose>
			<xsl:if test="@date">
				<xsl:text>, </xsl:text>
				<xsl:value-of select="@date"/>
			</xsl:if>
			<xsl:if test="@by"> (<xsl:value-of select="@by"/>)</xsl:if>
			<br/>
			<xsl:apply-templates select="child::*" mode="changes"/>
		</div>
	</xsl:template>
	
	<xsl:preserve-space elements="code"/>
	
</xsl:stylesheet>

