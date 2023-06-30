// _XMLDOMParserImpl.h: interface for the _XMLDOMParserImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX__XMLDOMPARSERIMPL_H__4035186F_92C8_44D3_A375_4243E8B88AC0__INCLUDED_)
#define AFX__XMLDOMPARSERIMPL_H__4035186F_92C8_44D3_A375_4243E8B88AC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "nanoxml.h"
#include "IXMLDOM.h"
#include <vector>
#include "xml_string.h"

#include <ISystem.h>

#ifdef XML_UNICODE
#define xstrncpy wcsncpy
#else
#define xstrncpy strncpy
#endif


class _XMLDOMParserImpl /* : public NanoXMLSink */
{
public:
	_XMLDOMParserImpl(XDOM::IXMLDOMDocument *pDoc);
	virtual ~_XMLDOMParserImpl();
	bool parse( std::vector<unsigned char> &buffer,string &errorString );

	void OnStartElement( const char *name, const char **atts );
	void OnEndElement( const char *name );
	void OnCharacterData( const char *s, int len );
protected:
	void StartDocument(bool unicode);
	void StartElement( const char *name);
	void EndElement();
	void Data( const char *name);
	void Attribute( const char *name,const char *value);
	void EndDocument(const char *error);
	unsigned int GetByte();

	// First node will become root node.
	std::vector<XDOM::IXMLDOMNode *> nodeStack;
	int m_nFilePos;
	std::vector<unsigned char> *m_pBuffer;
	bool m_bUnicode;
	bool m_bErrorState;
	XDOM::IXMLDOMDocument *m_pDoc;
	string m_sErrorString;
};

#endif // !defined(AFX__XMLDOMPARSERIMPL_H__4035186F_92C8_44D3_A375_4243E8B88AC0__INCLUDED_)
