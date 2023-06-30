// XMLDOMDocumentImpl.cpp: implementation of the CXMLDOMDocumentImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XMLDOMDocumentImpl.h"
#include "XMLDOMNodeImpl.h"
#include "nanoxml.h"
#include "ilog.h"
#include <ICryPak.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef XML_UNICODE
#define _T(s) L##s
#define xfopen _wfopen 
#else
#define _T(s) s
#define xfopen fxopen
#endif

CXMLDOMDocumentImpl::CXMLDOMDocumentImpl()
{
	m_nRef=0;
	m_pChildNodes=new CXMLDOMNodeListImpl;
	m_pChildNodes->AddRef();
	m_ntNodeType=XDOM::NODE_ELEMENT;
	m_wCheckSum=0;
}

CXMLDOMDocumentImpl::~CXMLDOMDocumentImpl()
{
	m_pChildNodes->Release();	
}

bool CXMLDOMDocumentImpl::load(const XMLCHAR *sSource)
{
	m_sErrorString = "";
	_XMLDOMParserImpl xml(this);
	std::vector<unsigned char> buf;
	FILE *file = GetISystem()->GetIPak()->FOpen( sSource,_T("rb") );
	if (file) {
		GetISystem()->GetIPak()->FSeek( file,0,SEEK_END );
		int fileSize = GetISystem()->GetIPak()->FTell(file);
		GetISystem()->GetIPak()->FSeek( file,0,SEEK_SET );
		buf.resize( fileSize );
		GetISystem()->GetIPak()->FRead( &*buf.begin(),fileSize,1,file );
		GetISystem()->GetIPak()->FClose(file);
		//calculate the checksum of the file
		//that's used by to verify that a clien has the same level data
		m_wCheckSum=0;
		for(std::vector<unsigned char>::iterator itr=buf.begin();itr!=buf.end();++itr)
		{
			m_wCheckSum+=(*itr);
		}
		
		if(!xml.parse( buf,m_sErrorString ))
		{
			CryLog("XMLDOM : ",m_sErrorString.c_str());
			return false;
		}
		return true;
	} else {
		return false;
	}
}

bool CXMLDOMDocumentImpl::loadXML(const char *szString)
{
	m_sErrorString = "";
	_XMLDOMParserImpl xml(this);
	std::vector<unsigned char> vBuf;
	vBuf.resize(strlen(szString));
	memcpy(&vBuf[0], szString, strlen(szString));
	if(!xml.parse(vBuf, m_sErrorString))
	{
		CryLog("XMLDOM : ",m_sErrorString.c_str());
		return false;
	}
	return true;
}

XDOM::IXMLDOMNode *CXMLDOMDocumentImpl::getRootNode()
{
	return NULL;
}

XDOM::IXMLDOMNode *CXMLDOMDocumentImpl::createNode(XDOM::_DOMNodeType Type,const XMLCHAR *name)
{
	XDOM::IXMLDOMNode *pNode;
	pNode=new CXMLDOMNodeImpl(Type);
	pNode->setName(name);
	
	return pNode;
}

inline void AddTabs( string &str,int count )
{
	for (int i = 0; i < count; i++)
	{
		str += _T("\t");
	}
}

void CXMLDOMDocumentImpl::PrintNode(XDOM::IXMLDOMNode *pNode,string &rString,int &level)
{
	if(pNode->getNodeType()==XDOM::NODE_ELEMENT)
	{
		AddTabs(rString,level);
		rString+=string(_T("<"))+string(pNode->getName());

		if(!pNode->hasChildNodes()){
			rString+=string(_T("/>"));
			return;
		}
		bool bChildElements=false;
		bool bChildAttributes=false;

		XDOM::IXMLDOMNodeList *pChilds=pNode->getChildNodes();
		//checks if there are childs attributes or elements or both
		XDOM::IXMLDOMNode *pChild=NULL;
		pChilds->reset();
		
		while(pChild=pChilds->nextNode())
		{
			if(pChild->getNodeType()==XDOM::NODE_ELEMENT)
				bChildElements=true;
			if(pChild->getNodeType()==XDOM::NODE_ATTRIBUTE)
				bChildAttributes=true;
		}
		//print attributes
		if(bChildAttributes){
			pChilds->reset();
			while(pChild=pChilds->nextNode())
			{
				if(pChild->getNodeType()==XDOM::NODE_ATTRIBUTE)
				{
					rString+=string(_T(" "))+string(pChild->getName())+string(_T("=\""))+string(pChild->getText())+string(_T("\""));
				}
			}
		}

		if(bChildElements){
			rString+=string(_T(">\n"));
			level++;
			pChilds->reset();
			while(pChild=pChilds->nextNode())
			{
				if(pChild->getNodeType()==XDOM::NODE_ELEMENT){
					PrintNode(pChild,rString,level);
				}
					
			}
			level--;
			AddTabs(rString,level);
			rString+=string(_T("</"))+string(pNode->getName())+string(_T(">\n"));
		}
		else
		{
			rString+=string(_T(" />\n"));
		}
	}
}

const XMLCHAR *CXMLDOMDocumentImpl::getXML()
{
	int level=0;
	m_sXml="";
	PrintNode(this,m_sXml,level);
	return m_sXml.c_str();

}

const XMLCHAR *CXMLDOMDocumentImpl::getErrorString()
{
	return m_sErrorString.c_str();
}

unsigned short CXMLDOMDocumentImpl::getCheckSum()
{
	return m_wCheckSum;
}

///////////////////////////////////////////////////
//NODE IMPL
XDOM::_DOMNodeType CXMLDOMDocumentImpl::getNodeType()
{
	return m_ntNodeType;
} 
 
const char *CXMLDOMDocumentImpl::getText()
{
	return m_sText.c_str();
}

const char *CXMLDOMDocumentImpl::getName()
{
	return m_sName.c_str();
} 

XDOM::IXMLDOMNodeList *CXMLDOMDocumentImpl::getChildNodes()
{
	return m_pChildNodes;
}
	
void CXMLDOMDocumentImpl::setText(const char *sText)
{
	m_sText=sText; 
#if defined(LINUX)
	RemoveCRLF(m_sText);
#endif
}

void CXMLDOMDocumentImpl::setName(const char *sName)
{
	m_sName=sName;
}

bool CXMLDOMDocumentImpl::hasChildNodes()
{
	return m_pChildNodes->length()?true:false;
}

bool CXMLDOMDocumentImpl::appendChild(XDOM::IXMLDOMNode *pNode)
{
	m_pChildNodes->AddNode(pNode);
	return true;
}

XDOM::IXMLDOMNode *CXMLDOMDocumentImpl::getAttribute(const XMLCHAR *sName)
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

XDOM::IXMLDOMNodeList *CXMLDOMDocumentImpl::getElementsByTagName(const XMLCHAR *sName)
{
	XDOM::IXMLDOMNode *pNode;
	CXMLDOMNodeListImpl *pNodeList;
	pNodeList=new CXMLDOMNodeListImpl;
	m_pChildNodes->reset();
	while((pNode=m_pChildNodes->nextNode())!=NULL)
	{
#if defined(LINUX)
		if(compareTextFileStrings(pNode->getName(),sName)==0)
#else
		if(strcmp(pNode->getName(),sName)==0)
#endif
			pNodeList->AddNode(pNode);
	}
//	if(pNodeList->length()==0)
//		return NULL;
	return pNodeList;
}

XDOM::IXMLDOMDocument *CreateDOMDocument()
{
	return new CXMLDOMDocumentImpl;
}