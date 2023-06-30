// XMLDOMNodeListImpl.cpp: implementation of the CXMLDOMNodeListImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XMLDOMNodeListImpl.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

XDOM::IXMLDOMNode *CXMLDOMNodeListImpl::getNamedItem(const XMLCHAR *sName)
{
	XDOM::IXMLDOMNode *pNode=NULL;
	DOMNodeListItor itor=m_lstNodes.begin();
	while(itor!=m_lstNodes.end())
	{
		pNode=(*itor);
#if defined(LINUX)
		if(compareTextFileStrings(pNode->getName(),sName)==0)
#else
		if(strcmp(pNode->getName(),sName)==0)
#endif
		{
			return pNode;
		}

		++itor;
	}
	return NULL;
}

