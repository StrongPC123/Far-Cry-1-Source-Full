// XMLDOMNodeListImpl.h: interface for the CXMLDOMNodeListImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLDOMNODELISTIMPL_H__40D45712_6A7A_481A_8621_5273CEE6560E__INCLUDED_)
#define AFX_XMLDOMNODELISTIMPL_H__40D45712_6A7A_481A_8621_5273CEE6560E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "IXMLDOM.h"

#include <list>
#include "xml_string.h"


typedef std::list<XDOM::IXMLDOMNode *> DOMNodeList;
typedef DOMNodeList::iterator DOMNodeListItor;

class CXMLDOMNodeListImpl : public XDOM::IXMLDOMNodeList  
{
public:
	CXMLDOMNodeListImpl()
	{
		m_nRef=0;
		m_itor=m_lstNodes.begin();
	}
	virtual ~CXMLDOMNodeListImpl()
	{
		DOMNodeListItor itor=m_lstNodes.begin();
		while(itor!=m_lstNodes.end())
		{
			(*itor)->Release();
			++itor;
		}
	}
public:
/////////////////////////////////////////////////
//IXMLDOMBase
	int AddRef() { return ++m_nRef; }
	void Release() { if(--m_nRef<=0) delete this; };
/////////////////////////////////////////////////
//IXMLDOMNodeList
	size_t length()
	{
		return m_lstNodes.size();
	}
	void reset()
	{
		m_itor=m_lstNodes.begin();
	}

	XDOM::IXMLDOMNode *nextNode()
	{
		XDOM::IXMLDOMNode *pTemp;
		if(m_itor==m_lstNodes.end())
			return NULL;
		pTemp=(*m_itor);
		++m_itor;
		return pTemp;
	}
//
	void AddNode(XDOM::IXMLDOMNode *pNode)
	{
		m_lstNodes.push_back(pNode);
		pNode->AddRef();
	}
//
	XDOM::IXMLDOMNode *getNamedItem(const XMLCHAR *sName);
private:
	DOMNodeList m_lstNodes;
	DOMNodeListItor m_itor;
	int m_nRef;
};

#endif // !defined(AFX_XMLDOMNODELISTIMPL_H__40D45712_6A7A_481A_8621_5273CEE6560E__INCLUDED_)
