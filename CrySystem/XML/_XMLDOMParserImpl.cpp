// _XMLDOMParserImpl.cpp: implementation of the _XMLDOMParserImpl class.
//
//////////////////////////////////////////////////////////////////////

//#ifndef _XBOX
#include "stdafx.h"
//#endif

#include "_XMLDOMParserImpl.h"

#if !defined(LINUX)
#include <assert.h>
#endif


#define XMLPARSEAPI(type) type
#include "expat\expat.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

_XMLDOMParserImpl::_XMLDOMParserImpl(XDOM::IXMLDOMDocument *pDoc)
{
	m_pDoc=pDoc;
	m_nFilePos=0;
}

_XMLDOMParserImpl::~_XMLDOMParserImpl()
{

}

void _XMLDOMParserImpl::StartDocument(bool unicode)
{
	m_bUnicode=unicode;
}

void _XMLDOMParserImpl::StartElement( const char *name)
{
	XDOM::IXMLDOMNode *pParent=NULL;
	XDOM::IXMLDOMNode *pNode=NULL;

	if (!nodeStack.empty()) {
		
		pParent = nodeStack.back();
		pNode=m_pDoc->createNode(XDOM::NODE_ELEMENT,name);
	} else {
		m_pDoc->setName(name);
		pNode=m_pDoc;
	}
	
	nodeStack.push_back(pNode);

	if (pParent) {
		pParent->appendChild( pNode );
	}
}

void _XMLDOMParserImpl::EndElement()
{
	assert( !nodeStack.empty() );
	if (!nodeStack.empty()) {
		nodeStack.pop_back();
	}
}

void _XMLDOMParserImpl::Data( const char *data )
{
	assert( !nodeStack.empty() );
	if (!nodeStack.empty())
	{
		XDOM::IXMLDOMNode *pNode = nodeStack.back();
		const char *sPrevText = pNode->getText();
		if (strlen(sPrevText) == 0)
			pNode->setText(data);
		else
		{
			std::string addstr = sPrevText;
			addstr += data;
			pNode->setText(addstr.c_str());
		}
	}
}

void _XMLDOMParserImpl::Attribute( const char *name,const char *value)
{
	XDOM::IXMLDOMNode *pNode=NULL;
	if (!nodeStack.empty())
	{
		pNode=nodeStack.back();
	}
	else
	{
		pNode=m_pDoc;
	}

	XDOM::IXMLDOMNode *pAttr=NULL;
	pAttr=m_pDoc->createNode(XDOM::NODE_ATTRIBUTE,name);
	pAttr->setText(value);
	pNode->appendChild(pAttr);
}

void _XMLDOMParserImpl::EndDocument(const char *error)
{
	if(error){
		m_bErrorState=true;
		m_sErrorString=error;
	}
	m_sErrorString="";
	m_bErrorState=false;
}

unsigned int _XMLDOMParserImpl::GetByte()
{
	if(m_nFilePos>=(int)(m_pBuffer->size()))return -1;
	return ((unsigned int)(*m_pBuffer)[m_nFilePos++]);
}

//////////////////////////////////////////////////////////////////////////
// Expat Callbacks.
//////////////////////////////////////////////////////////////////////////
static void EXPAT_XML_StartElement(void *userData, const char *name, const char **atts)
{
	((_XMLDOMParserImpl*)userData)->OnStartElement( name,atts );
}
static void EXPAT_XML_EndElement(void *userData, const char *name )
{
	((_XMLDOMParserImpl*)userData)->OnEndElement( name );
}
static void EXPAT_XML_CharacterData( void *userData, const char *s, int len )
{
	((_XMLDOMParserImpl*)userData)->OnCharacterData( s,len );
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Processing of Expat callbacks.
//////////////////////////////////////////////////////////////////////////
void _XMLDOMParserImpl::OnStartElement( const char *name, const char **atts )
{
	StartElement( name );
	int i = 0;
	while (atts[i] != 0)
	{
		Attribute( atts[i],atts[i+1] );
		i += 2;
	}
}

//////////////////////////////////////////////////////////////////////////
void _XMLDOMParserImpl::OnEndElement( const char *name )
{
	EndElement();
}

//////////////////////////////////////////////////////////////////////////
// Local string trimming functions.
//////////////////////////////////////////////////////////////////////////
namespace
{
	void string_ltrim( String &str, const char* whitespace )
	{
		size_t idx = str.find_first_not_of(whitespace);
		if (idx == 0)
			return;
		else if( idx != String::npos)
			str = str.substr(idx);
		else
			str = "";
	}

	void string_rtrim( String &str, const char* whitespace )
	{
		size_t idx = str.find_last_not_of(whitespace);
		if( idx != String::npos )
			str = str.substr(0,idx+1);
	}

	void string_trim( String &str, const char* whitespace )
	{
		string_ltrim( str,whitespace );
		string_rtrim( str,whitespace );
	}
}

//////////////////////////////////////////////////////////////////////////
void _XMLDOMParserImpl::OnCharacterData( const char *s, int len )
{
	String tempstr( s,len );
	// Trim whitespaces from both sides.
	string_trim( tempstr,"\t\n\r " );
	Data( tempstr.c_str() );
}

//////////////////////////////////////////////////////////////////////////
bool _XMLDOMParserImpl::parse( std::vector<unsigned char> &buffer,string &errorString )
{
	/*
	m_nFilePos=0;
	m_pBuffer=&buffer;
	m_bUnicode=false;
	m_bErrorState=false;

	NanoXML nx;
	nx.Parse(this);
	m_pBuffer=NULL;
	m_sErrorString=errorString;
	return !m_bErrorState;
	*/

	m_bUnicode=false;
	m_bErrorState=false;

	XML_Memory_Handling_Suite memHandler;
	memHandler.malloc_fcn = CryModuleMalloc;
	memHandler.realloc_fcn = CryModuleRealloc;
	memHandler.free_fcn = CryModuleFree;

	XML_Parser parser = XML_ParserCreate_MM(NULL,&memHandler,NULL);

	XML_SetUserData( parser, this );
	XML_SetElementHandler( parser, EXPAT_XML_StartElement,EXPAT_XML_EndElement );
	XML_SetCharacterDataHandler( parser,EXPAT_XML_CharacterData );

	const char *sBuffer = (char*)&buffer[0];
	if (!XML_Parse( parser,sBuffer,(int)buffer.size(),1 ))
	{
		char *str = new char [32768];
		sprintf( str,"XML Error: %s at line %d",XML_ErrorString(XML_GetErrorCode(parser)),XML_GetCurrentLineNumber(parser) );
		errorString = str;
		delete [] str;
	}
	XML_ParserFree( parser );

	return !m_bErrorState;
}
