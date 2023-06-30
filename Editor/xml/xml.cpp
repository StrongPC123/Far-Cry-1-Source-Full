#include "StdAfx.h"

#include <stdlib.h>

#define XMLPARSEAPI(type) type
#include "expat\expat.h"
#include "xml.h"
#include <string>

/**
 ******************************************************************************
 * CXmlNode implementation.
 ******************************************************************************
 */
XmlAttribute CXmlNode::tempAttr;

CXmlNode::~CXmlNode()
{
	// Clear parent pointer from childs.
	for (XmlNodes::const_iterator it = m_childs.begin(); it != m_childs.end(); ++it)
	{
		IXmlNode *node = *it;
		((CXmlNode*)node)->m_parent = 0;
	}
}

//! The only ctor and private, protect us from deriviation.
CXmlNode::CXmlNode( const char *tag )
{
	m_tag = tag;
	m_parent = 0;
	m_refCount = 0;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CXmlNode::createNode( const char *tag )
{
	return XmlNodeRef( new CXmlNode(tag) );
}

//////////////////////////////////////////////////////////////////////////
bool CXmlNode::isTag( const char *tag ) const
{
	return stricmp( tag,m_tag ) == 0;
}

const char* CXmlNode::getAttr( const char *key ) const
{
	tempAttr.key = key;
	XmlAttributes::const_iterator it = m_attributes.find(tempAttr);
	if (it != m_attributes.end()) {
		return it->value;
	}
	return "";
}

bool CXmlNode::haveAttr( const char *key ) const
{
	tempAttr.key = key;
	XmlAttributes::const_iterator it = m_attributes.find(tempAttr);
	if (it != m_attributes.end()) {
		return true;
	}
	return false;
}

void CXmlNode::delAttr( const char *key )
{
	tempAttr.key = key;
	m_attributes.erase( tempAttr );
}

void CXmlNode::removeAllAttributes()
{
	m_attributes.clear();
}

void CXmlNode::setAttr( const char *key,const char *value )
{
	int i;
	// Filter Bad characters from string.
	char str[1024*16];
	int len = strlen(value);
	assert( len < sizeof(str) );
	if (len >= sizeof(str))
		return;
	for (i = 0; i < len; i++)
	{
		switch (value[i])
		{
		case '"':
			str[i] = '_';
			break;
		case '<':
			str[i] = '_';
			break;
		case '&':
			str[i] = '_';
			break;
		default:
			str[i] = value[i];
		}
	}
	str[i] = 0;
	tempAttr.key = key;
	tempAttr.value = str;
	std::pair<XmlAttributes::iterator,bool> res = m_attributes.insert( tempAttr );
	if (!res.second) {
		// If already exist, ovveride this member.
		m_attributes.erase(res.first);
		m_attributes.insert( tempAttr );
	}
}

void CXmlNode::setAttr( const char *key,int value )
{
	char str[128];
	itoa( value,str,10 );
	setAttr( key,str );
}

void CXmlNode::setAttr( const char *key,unsigned int value )
{
	char str[128];
	ultoa( value,str,10 );
	setAttr( key,str );
}

void CXmlNode::setAttr( const char *key,float value )
{
	char str[128];
	// Optimize float to string storage.
	sprintf( str,"%.8g",value );
	setAttr( key,str );
}

void CXmlNode::setAttr( const char *key,const Vec3 &value )
{
	char str[128];
	sprintf( str,"%.8g,%.8g,%.8g",value.x,value.y,value.z );
	setAttr( key,str );
}

void CXmlNode::setAttr( const char *key,const Quat &value )
{
	char str[128];
	sprintf( str,"%.8g,%.8g,%.8g,%.8g",value.w,value.v.x,value.v.y,value.v.z );
	setAttr( key,str );
}

bool CXmlNode::getAttr( const char *key,int &value ) const
{
	tempAttr.key = key;
	XmlAttributes::const_iterator it = m_attributes.find( tempAttr );
	if (it != m_attributes.end()) {
		value = atoi(it->value);
		return true;
	}
	return false;
}

bool CXmlNode::getAttr( const char *key,unsigned int &value ) const
{
	tempAttr.key = key;
	XmlAttributes::const_iterator it = m_attributes.find( tempAttr );
	if (it != m_attributes.end()) {
		const char *str = it->value;
		char *stopstr;
		value = strtoul(str,&stopstr,10);
		return true;
	}
	return false;
}

bool CXmlNode::getAttr( const char *key,bool &value ) const
{
	tempAttr.key = key;
	XmlAttributes::const_iterator it = m_attributes.find( tempAttr );
	if (it != m_attributes.end())
	{
		value = atoi(it->value);
		return true;
	}
	return false;
}

bool CXmlNode::getAttr( const char *key,float &value ) const
{
	tempAttr.key = key;
	XmlAttributes::const_iterator it = m_attributes.find( tempAttr );
	if (it != m_attributes.end()) {
		value = (float)atof(it->value);
		return true;
	}
	return false;
}

bool CXmlNode::getAttr( const char *key,Vec3 &value ) const
{
	tempAttr.key = key;
	XmlAttributes::const_iterator it = m_attributes.find( tempAttr );
	if (it != m_attributes.end()) {
		float x,y,z;
		if (sscanf( it->value,"%f,%f,%f",&x,&y,&z ) == 3)
		{
			value(x,y,z);
			return true;
		}
	}
	return false;
}

bool CXmlNode::getAttr( const char *key,Quat &value ) const
{
	tempAttr.key = key;
	XmlAttributes::const_iterator it = m_attributes.find( tempAttr );
	if (it != m_attributes.end()) {
		float w,x,y,z;
		if (sscanf( it->value,"%f,%f,%f,%f",&w,&x,&y,&z ) == 4)
		{
			value = Quat(w,x,y,z);
			return true;
		}
	}
	return false;
}

XmlNodeRef CXmlNode::findChild( const char *tag ) const
{
	for (XmlNodes::const_iterator it = m_childs.begin(); it != m_childs.end(); ++it) {
		if ((*it)->isTag(tag))
		{
			return *it;
		}
	}
	return 0;
}

//! Adds new child node.
void CXmlNode::addChild( XmlNodeRef &node )
{
	assert( node != 0 );
	m_childs.push_back(node);
	IXmlNode *n = node;
	((CXmlNode*)n)->m_parent = this;
};

XmlNodeRef CXmlNode::newChild( const char *tagName )
{
	XmlNodeRef node = createNode(tagName);
	addChild(node);
	return node;
}

void CXmlNode::removeChild( XmlNodeRef &node )
{
	XmlNodes::iterator it = std::find(m_childs.begin(),m_childs.end(),(IXmlNode*)node );
	if (it != m_childs.end())
	{
		m_childs.erase(it);
	}
}

void CXmlNode::removeAllChilds()
{
	m_childs.clear();
}

//! Get XML Node child nodes.
XmlNodeRef CXmlNode::getChild( int i ) const
{
	assert( i >= 0 && i < m_childs.size() );
	return m_childs[i];
}

//////////////////////////////////////////////////////////////////////////
void CXmlNode::copyAttributes( XmlNodeRef fromNode )
{
	IXmlNode *n = fromNode;
	m_attributes = ((CXmlNode*)n)->m_attributes;
}

//////////////////////////////////////////////////////////////////////////
bool CXmlNode::getAttributeByIndex( int index,const char **key,const char **value )
{
	XmlAttributes::iterator it = m_attributes.begin();
	if (it != m_attributes.end())
	{
		std::advance( it,index );
		if (it != m_attributes.end())
		{
			*key = it->key;
			*value = it->value;
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CXmlNode::clone()
{
	XmlNodeRef node = createNode(m_tag);
	// Clone attributes.
	IXmlNode *n = node;
	((CXmlNode*)n)->m_attributes = m_attributes;

	// Clone sub nodes.
	for (XmlNodes::const_iterator it = m_childs.begin(); it != m_childs.end(); ++it) {
		XmlNodeRef child = (*it)->clone();
		node->addChild( child);
	}
	
	return node;
}

//////////////////////////////////////////////////////////////////////////
static void AddTabsToString( CString &xml,int level )
{
	static const char *tabs[] = {
		"",
			" ",
			"  ",
			"   ",
			"    ",
			"     ",
			"      ",
			"       ",
			"        ",
			"         ",
			"          ",
			"           ",
	};
	int i;
	// Add tabs.
	if (level < sizeof(tabs)/sizeof(tabs[0]))
	{
		xml += tabs[level];
	}
	else
	{
		for (i = 0; i < level; i++)
			xml += "  ";
	}
}

void CXmlNode::AddToXmlString( CString &xml,int level ) const
{
	AddTabsToString( xml,level );

	// Begin Tag
	if (m_attributes.empty()) {
		xml += "<";
		xml += m_tag;
		xml += ">";
	} else {
		xml += "<";
		xml += m_tag;
		xml += " ";

		// Put attributes.
		for (XmlAttributes::const_iterator it = m_attributes.begin(); it != m_attributes.end(); ++it)
		{
			xml += it->key;
			xml += "=\"";
			xml += it->value;
			xml += "\" ";
		}
		if (m_content.empty() && m_childs.empty())
		{
			// Compact tag form.
			xml += "/>\n";
			return;
		}
		xml += ">";
	}

	// Put node content.
	xml += m_content;

	if (!m_childs.empty()) {
		xml += "\n";
	}

	// Add sub nodes.
	for (XmlNodes::const_iterator it = m_childs.begin(); it != m_childs.end(); ++it)
	{
		IXmlNode *node = *it;
		((CXmlNode*)node)->AddToXmlString( xml,level+1 );
	}

	// End tag.
	
	// Add tabs.
	AddTabsToString( xml,level );

	xml += "</";
	xml += m_tag;
	xml += ">\n";
}

//#ifdef WIN64
//#define WIN64_OLDMFC
//#endif

#ifdef WIN64_OLDMFC
class CPreallocString : public CString
{
public:
	void Preallocate( int nLength )
	{
		Release();
		AllocBuffer( nLength );
		GetData()->nDataLength = 0;
	};
};
#endif // WIN64

XmlString CXmlNode::getXML( int level ) const
{
	//@FIXME: May be some faster solition can be found here.
#ifdef WIN64_OLDMFC
	static CPreallocString xml;
	xml.Preallocate( 10000000 );
#else
	static CString xml;
	xml = "";
	xml.Preallocate( 6000000 );
#endif
	//xml = "";
	//xml.reserve( 10000000 );

	AddToXmlString( xml,level );
	return xml;
}

bool CXmlNode::saveToFile( const char *fileName )
{
	if (!CFileUtil::OverwriteFile( fileName ))
		return false;
	XmlString xml = getXML();
	FILE *file = fopen( fileName,"wt" );
	if (file)
	{
		fwrite( (const char*)xml,xml.size(),1,file );
		fclose(file);
		return true;
	}
	return false;
}

/**
 ******************************************************************************
 * XmlParserImp class.
 ******************************************************************************
 */
class XmlParserImp {
public:
	XmlParserImp();
	XmlNodeRef parse( const char *buffer,int bufLen,XmlString &errorString );

protected:
	void	onStartElement( const char *tagName,const char **atts );
	void	onEndElement( const char *tagName );
	void	onRawData( const char *data );

	static void startElement(void *userData, const char *name, const char **atts) {
		((XmlParserImp*)userData)->onStartElement( name,atts );
	}
	static void endElement(void *userData, const char *name ) {
		((XmlParserImp*)userData)->onEndElement( name );
	}
	static void characterData( void *userData, const char *s, int len ) {
		char str[32768];
		assert( len < 32768 );
		strncpy( str,s,len );
		str[len] = 0;
		((XmlParserImp*)userData)->onRawData( str );
	}

	// First node will become root node.
	std::vector<XmlNodeRef> nodeStack;
	XmlNodeRef m_root;

	XML_Parser m_parser;
};

/**
 ******************************************************************************
 * XmlParserImp
 ******************************************************************************
 */
XmlParserImp::XmlParserImp()
{
	m_root = 0;
}

void	XmlParserImp::onStartElement( const char *tagName,const char **atts )
{
	XmlNodeRef parent;
	XmlNodeRef node = new CXmlNode( tagName );

	if (!nodeStack.empty()) {
		parent = nodeStack.back();
	} else {
		m_root = node;
	}
	nodeStack.push_back(node);

	if (parent) {
		parent->addChild( node );
	}

	node->setLine( XML_GetCurrentLineNumber( (XML_Parser)m_parser ) );
	
	// Call start element callback.
	XmlString key,value;
	int i = 0;
	while (atts[i] != 0) {
		node->setAttr( atts[i],atts[i+1] );
		i += 2;
	}
}

void	XmlParserImp::onEndElement( const char *tagName )
{
	assert( !nodeStack.empty() );
	if (!nodeStack.empty()) {
		nodeStack.pop_back();
	}
}

void	XmlParserImp::onRawData( const char* data )
{
	assert( !nodeStack.empty() );
	if (!nodeStack.empty()) {
		IXmlNode *node = nodeStack.back();
		node->addContent( data );
	}
}

XmlNodeRef XmlParserImp::parse( const char *buffer,int bufLen,XmlString &errorString )
{
	m_parser = XML_ParserCreate(NULL);

  XML_SetUserData( m_parser, this );
  XML_SetElementHandler( m_parser, startElement,endElement );
	XML_SetCharacterDataHandler( m_parser,characterData );

	XmlNodeRef root = 0;

	/*
	int size = file->size();
	char *buf = (char*)memory::malloc( size );
	file->read( buf,size );
	if (!XML_Parse( parser,buf,size,1 ))
	{
		error( "XML Error (%s): %s at line %d\n",filename.c_str(),XML_ErrorString(XML_GetErrorCode(parser)),XML_GetCurrentLineNumber(parser) );
		return false;
	}
	memory::free( buf );

	XMLParser::parse( fileName );
	*/
	if (XML_Parse( m_parser,buffer,bufLen,1 ))
	{
		root = m_root;
	} else {
		char str[32768];
		sprintf( str,"XML Error: %s at line %d",XML_ErrorString(XML_GetErrorCode(m_parser)),XML_GetCurrentLineNumber(m_parser) );
		errorString = str;
		CLogFile::FormatLine( "%s",str );
	}
	
	XML_ParserFree( m_parser );

	m_root = 0;

	return root;
}

//! Parse xml file.
XmlNodeRef XmlParser::parse( const char *fileName )
{
	m_errorString = "";
	XmlParserImp xml;
	std::vector<char> buf;
	ICryPak *pPak = GetISystem()->GetIPak();
	FILE *file = pPak->FOpen(fileName,"rb");
	if (file)
	{
		pPak->FSeek( file,0,SEEK_END );
		int fileSize = pPak->FTell(file);
		pPak->FSeek( file,0,SEEK_SET );
		buf.resize( fileSize );
		pPak->FRead( &(buf[0]),fileSize,1,file );
		pPak->FClose(file);
		return xml.parse( &buf[0],buf.size(),m_errorString );
	} else {
		return XmlNodeRef();
	}
}
	
//! Parse xml from memory buffer.
XmlNodeRef XmlParser::parseBuffer( const char *buffer )
{
	m_errorString = "";
	XmlParserImp xml;
	return xml.parse( buffer,strlen(buffer),m_errorString );
};
