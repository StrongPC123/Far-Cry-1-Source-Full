// XMLDOMNodeImpl.h: interface for the CXMLDOMNodeImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLDOMNODEIMPL_H__01846B14_1503_4BDC_AA0F_EAB85F638E90__INCLUDED_)
#define AFX_XMLDOMNODEIMPL_H__01846B14_1503_4BDC_AA0F_EAB85F638E90__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IXMLDOM.h"
#include "XMLDOMNodeListImpl.h"
#include "xml_string.h"

class CXMLDOMNodeImpl :
	public XDOM::IXMLDOMNode
{
public:
	CXMLDOMNodeImpl(XDOM::_DOMNodeType type);
	virtual ~CXMLDOMNodeImpl();
//////////////////////////////////////////////////
//IXMLDOMBase
	int AddRef() { return ++m_nRef; }
	void Release() 
	{ 
		if(--m_nRef<=0) 
		delete this; 
	};
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
public:
	xml_string m_sText;
	xml_string m_sName;
	
	XDOM::_DOMNodeType m_ntNodeType;
	CXMLDOMNodeListImpl *m_pChildNodes;
private:
	int m_nRef;

};

#endif // !defined(AFX_XMLDOMNODEIMPL_H__01846B14_1503_4BDC_AA0F_EAB85F638E90__INCLUDED_)
