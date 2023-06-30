// XMLDOMNodeImpl.cpp: implementation of the CXMLDOMNodeImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XMLDOMNodeImpl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXMLDOMNodeImpl::CXMLDOMNodeImpl(XDOM::_DOMNodeType type)
{
	m_nRef=0;
	m_ntNodeType=type;
	m_pChildNodes=new CXMLDOMNodeListImpl;
	m_pChildNodes->AddRef(); 
}

CXMLDOMNodeImpl::~CXMLDOMNodeImpl()
{
	m_pChildNodes->Release(); 
}

XDOM::_DOMNodeType CXMLDOMNodeImpl::getNodeType()
{
	return m_ntNodeType;
}

const char *CXMLDOMNodeImpl::getText()
{
	return m_sText.c_str();
}

const char *CXMLDOMNodeImpl::getName()
{
	return m_sName.c_str();
}

XDOM::IXMLDOMNodeList *CXMLDOMNodeImpl::getChildNodes()
{
	return m_pChildNodes;
}
	
void CXMLDOMNodeImpl::setText(const char *sText)
{
	m_sText=sText;
#if defined(LINUX)
	RemoveCRLF(m_sText);
#endif
} 

void CXMLDOMNodeImpl::setName(const char *sName)
{
	m_sName=sName;
}

bool CXMLDOMNodeImpl::hasChildNodes()
{
	return m_pChildNodes->length()?true:false;
}

bool CXMLDOMNodeImpl::appendChild(IXMLDOMNode *pNode)
{
	m_pChildNodes->AddNode(pNode);
	return true;
}

XDOM::IXMLDOMNode *CXMLDOMNodeImpl::getAttribute(const XMLCHAR *sName)
{
	XDOM::IXMLDOMNode *pNode;
	if(m_ntNodeType!=XDOM::NODE_ELEMENT)
		return NULL;
	pNode=m_pChildNodes->getNamedItem(sName);
	if(pNode)
	{
		if(pNode->getNodeType()==XDOM::NODE_ATTRIBUTE)
		return pNode;
	}
	return NULL;
}

XDOM::IXMLDOMNodeList *CXMLDOMNodeImpl::getElementsByTagName(const XMLCHAR *sName)
{
	XDOM::IXMLDOMNode *pNode;
	CXMLDOMNodeListImpl *pNodeList;
	pNodeList=new CXMLDOMNodeListImpl;
	m_pChildNodes->reset();
	while(pNode=m_pChildNodes->nextNode())
	{
#if defined(LINUX)
		if((compareTextFileStrings(pNode->getName(),sName)==0) && (pNode->getNodeType()==XDOM::NODE_ELEMENT))
#else
		if((strcmp(pNode->getName(),sName)==0) && (pNode->getNodeType()==XDOM::NODE_ELEMENT))
#endif
			pNodeList->AddNode(pNode);
	}
	if(pNodeList->length()==0)
	{
		pNodeList->Release();
		return NULL;
	}
		
	return pNodeList;
}
