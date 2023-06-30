// XMLDOMDocumentImpl.h: interface for the CXMLDOMDocumentImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLDOMDOCUMENTIMPL_H__EC6A1661_AC49_4D84_8EEE_6FD7B25AAC0F__INCLUDED_)
#define AFX_XMLDOMDOCUMENTIMPL_H__EC6A1661_AC49_4D84_8EEE_6FD7B25AAC0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "xml_string.h"
#include "IXMLDOM.h"
#include "XMLDOMNodeListImpl.h"


#include "_XMLDOMParserImpl.h"

class CXMLDOMDocumentImpl : 
	public XDOM::IXMLDOMDocument
{
public:
	CXMLDOMDocumentImpl();
	virtual ~CXMLDOMDocumentImpl();
//////////////////////////////////////////////////
//IXMLDOMBase
	int AddRef() { return ++m_nRef; }
	void Release() { if(--m_nRef<=0) delete this; };
//////////////////////////////////////////////////
//IXMLDOMNode
	XDOM::_DOMNodeType getNodeType();
	const char *getText();
	const char *getName();
	XDOM::IXMLDOMNodeList *getChildNodes();
	
	void setText(const char *sText);
	void setName(const char *sName);

	bool hasChildNodes();
	bool appendChild(IXMLDOMNode *pNode);

	XDOM::IXMLDOMNode *getAttribute(const XMLCHAR *sName);
	XDOM::IXMLDOMNodeList *getElementsByTagName(const XMLCHAR *sName);
//////////////////////////////////////////////////
//IXMLDOMDocument
	virtual bool load(const char *sSource);
	virtual bool loadXML(const char *sString);
	XDOM::IXMLDOMNode *getRootNode();
	XDOM::IXMLDOMNode *createNode(XDOM::_DOMNodeType Type,const char *name);
	const XMLCHAR *getXML();
	const XMLCHAR *getErrorString();
	unsigned short getCheckSum();

public:
	xml_string m_sText;
	xml_string m_sName;
	string m_sErrorString;
	XDOM::_DOMNodeType m_ntNodeType;
	CXMLDOMNodeListImpl *m_pChildNodes;

	void PrintNode(XDOM::IXMLDOMNode *pNode,string &string,int &level);
private:
	int m_nRef;
	string m_sXml;
	unsigned short m_wCheckSum;
};

#endif // !defined(AFX_XMLDOMDOCUMENTIMPL_H__EC6A1661_AC49_4D84_8EEE_6FD7B25AAC0F__INCLUDED_)
