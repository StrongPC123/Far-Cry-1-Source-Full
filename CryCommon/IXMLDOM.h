#ifndef _IXMLDOM_H_
#define _IXMLDOM_H_

#include <stdio.h>
#if !defined _XBOX && !defined(LINUX)
#ifdef CRYXMLDOM_EXPORTS
#define CRYXMLDOM_API __declspec(dllexport)
#else
#define CRYXMLDOM_API __declspec(dllimport)
#endif
#else
#define CRYXMLDOM_API
#endif

#define XMLCHAR char

#define __MSXML_LIBRARY_DEFINED__
//////////////////////////////////////////////////////////////////
#define USE_NAMESPACE
#include <string>
#ifdef USE_NAMESPACE
namespace XDOM{
#endif
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
struct IXMLDOMNodeList;

enum _DOMNodeType
{
	NODE_ELEMENT,
		NODE_ATTRIBUTE,
		NODE_TEXT
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
struct IXMLDOMBase
{
	virtual int AddRef() = 0;
	virtual void Release() = 0;
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
struct IXMLDOMNode :
public IXMLDOMBase
{
	virtual _DOMNodeType getNodeType() = 0;
	virtual const char *getText() = 0;
	virtual const char *getName() = 0;
	virtual IXMLDOMNodeList *getChildNodes() = 0;

	virtual void setText(const char *sText) = 0;
	virtual void setName(const char *sName) = 0;
	
	virtual bool hasChildNodes() = 0;
	virtual bool appendChild(IXMLDOMNode *pNode) = 0;

	virtual IXMLDOMNode *getAttribute(const XMLCHAR *sName) = 0;
	virtual IXMLDOMNodeList *getElementsByTagName(const XMLCHAR *sName) = 0;
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
struct IXMLDOMNodeList :
public IXMLDOMBase
{
	virtual size_t length() = 0;
	virtual void reset() = 0;
	virtual IXMLDOMNode *nextNode() = 0;
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
struct IXMLDOMDocument :
public IXMLDOMNode
{
	virtual bool load(const XMLCHAR *sSourceFile) = 0;
	virtual bool loadXML(const XMLCHAR *szString) = 0;
	virtual IXMLDOMNode *getRootNode() = 0;
	virtual IXMLDOMNode *createNode(_DOMNodeType Type, const char *name) = 0;
	//virtual const XMLCHAR *getXML() = 0;
	virtual const XMLCHAR *getErrorString() = 0;
	virtual unsigned short getCheckSum() = 0;
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
#ifdef USE_NAMESPACE
}
#endif


#include "smartptr.h"
//////////////////////////////////////////////////////////////////
#include "Cry_Math.h"
#if defined(LINUX)
inline Vec3 StringToVector(std::string str)
#else
inline Vec3 StringToVector(string str)
#endif
{
	Vec3 vTemp(0,0,0);
	float x,y,z;
	if (sscanf( str.c_str(),"%f,%f,%f",&x,&y,&z ) == 3)
	{
		vTemp(x,y,z);
	}
	else
	{
		vTemp(0,0,0);
	}
	return vTemp;
}


#ifdef USE_NAMESPACE
namespace XDOM{
#endif
	
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
typedef _smart_ptr <XDOM::IXMLDOMDocument>	IXMLDOMDocumentPtr;
typedef _smart_ptr <XDOM::IXMLDOMNode>			IXMLDOMNodePtr;
typedef _smart_ptr <XDOM::IXMLDOMNodeList>	IXMLDOMNodeListPtr;

#ifdef USE_NAMESPACE
}
#endif

/*
extern "C"{
//hack (think if this can be solved in some clean way)
CRYXMLDOM_API XDOM::IXMLDOMDocument *CreateDOMDocument();
typedef XDOM::IXMLDOMDocument *( *CREATEDOMDOCUMENT_FNCPTR)();
}
*/

#endif //_IXMLDOM_H_
