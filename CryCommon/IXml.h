////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   ixml.h
//  Version:     v1.00
//  Created:     16/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ixml_h__
#define __ixml_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "platform.h"
#include <vector>
#include <set>

#ifdef  _AFX
#include "Util\GuidUtil.h"
#endif //_AFX

/**

	This is wrapper arround expat library to provide DOM type of access for xml.
	Do not use IXmlNode class directly instead always use XmlNodeRef wrapper that
	takes care of memory managment issues.

	Usage Example:
	-------------------------------------------------------
	void testXml()
	{
		XmlParser xml;
		XmlNodeRef root = xml.parse( "test.xml" );

		if (root) {
			for (int i = 0; i < root->getChildCount(); i++) {
				XmlNodeRef child = root->getChild(i);
				if (child->isTag("world")) {
					if (child->getAttr("name") == "blah") {
					}
				}
			}
		}
	};
*/

// Special string wrapper for xml nodes.
class XmlString : public string
{
public:
	XmlString() {};
	XmlString( const char *str ) : string(str) {};
#ifdef  _AFX
	XmlString( const CString &str ) : string( (const char*)str ) {};
#endif // _AFX

	operator const char*() const { return c_str(); }
};


class IXmlNode;

/**
 ******************************************************************************
 * XmlNodeRef, wrapper class implementing reference counting for IXmlNode.
 ******************************************************************************
 */
class XmlNodeRef {
private:
  IXmlNode* p;
public:
  XmlNodeRef() : p(NULL) {}
	XmlNodeRef( int Null ) : p(NULL) {}
  XmlNodeRef( IXmlNode* p_ );
  XmlNodeRef( const XmlNodeRef &p_ );
	//explicit XmlNodeRef( const char *tag,IXmlNode *node );
  ~XmlNodeRef();
	
  operator IXmlNode*() const { return p; }
  operator const IXmlNode*() const { return p; }
  IXmlNode& operator*() const { return *p; }
  IXmlNode* operator->(void) const { return p; }
  
  XmlNodeRef&  operator=( IXmlNode* newp );
	XmlNodeRef&  operator=( const XmlNodeRef &newp );
	
  operator bool() const { return p != NULL; };
	bool operator !() const { return p == NULL; };
	
 	// Misc compare functions.
  bool  operator == ( const IXmlNode* p2 ) const { return p == p2; };
	bool  operator == ( IXmlNode* p2 ) const { return p == p2; };
  bool  operator != ( const IXmlNode* p2 ) const { return p != p2; };
  bool  operator != ( IXmlNode* p2 ) const { return p != p2; };
  bool  operator <  ( const IXmlNode* p2 ) const { return p < p2; };
  bool  operator >  ( const IXmlNode* p2 ) const { return p > p2; };

	bool  operator == ( const XmlNodeRef &n ) const { return p == n.p; };
  bool  operator != ( const XmlNodeRef &n ) const { return p != n.p; };
  bool  operator <  ( const XmlNodeRef &n ) const { return p < n.p; };
  bool  operator >  ( const XmlNodeRef &n ) const { return p > n.p; };
	
	friend bool operator == ( const XmlNodeRef &p1,int null );
  friend bool operator != ( const XmlNodeRef &p1,int null );
	friend bool operator == ( int null,const XmlNodeRef &p1 );
  friend bool operator != ( int null,const XmlNodeRef &p1 );
};

/**
 ******************************************************************************
 * IXmlNode class
 * Never use IXmlNode directly instead use reference counted XmlNodeRef.
 ******************************************************************************
 */

class IXmlNode {
public:
	virtual ~IXmlNode() {};

	//! Create new XML node.
	virtual XmlNodeRef createNode( const char *tag ) = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Reference counting.
	virtual void AddRef() = 0;
	//! When ref count reach zero XML node dies.
	virtual void Release() = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Get XML node tag.
	virtual const char *getTag() const = 0;
	virtual void	setTag( const char *tag ) = 0;

	//! Return true if givven tag equal to node tag.
	virtual bool isTag( const char *tag ) const = 0;

	//! Get XML Node attributes.
	virtual int getNumAttributes() const = 0;
	//! Return attribute key and value by attribute index.
	virtual bool getAttributeByIndex( int index,const char **key,const char **value ) = 0;
	
	//! Copy attributes to this node from givven node.
	virtual void copyAttributes( XmlNodeRef fromNode ) = 0;

	//! Get XML Node attribute for specified key.
	virtual const char* getAttr( const char *key ) const = 0;
	//! Check if attributes with specified key exist.
	virtual bool haveAttr( const char *key ) const = 0;
	
	//! Adds new child node.
	virtual void addChild( XmlNodeRef &node ) = 0;

	//! Creates new xml node and add it to childs list.
	virtual XmlNodeRef newChild( const char *tagName ) = 0;

	//! Remove child node.
	virtual void removeChild( XmlNodeRef &node ) = 0;

	//! Remove all child nodes.
	virtual void removeAllChilds() = 0;

	//! Get number of child XML nodes.
	virtual int	getChildCount() const = 0;
	
	//! Get XML Node child nodes.
	virtual XmlNodeRef getChild( int i ) const = 0;

	//! Find node with specified tag.
	virtual XmlNodeRef findChild( const char *tag ) const = 0;

	//! Get parent XML node.
	virtual XmlNodeRef getParent() const = 0;

	//! Returns content of this node.
	virtual const char* getContent() const = 0;
	virtual void setContent( const char *str ) = 0;
	virtual void addContent( const char *str ) = 0;

	//! Deep clone of this and all child xml nodes.
	virtual XmlNodeRef clone() = 0;

	//! Returns line number for XML tag.
	virtual int getLine() const = 0;
	//! Set line number in xml.
	virtual void setLine( int line ) = 0;

	//! Returns XML of this node and sub nodes.
	virtual XmlString getXML( int level=0 ) const = 0;
	virtual bool saveToFile( const char *fileName ) = 0;

	//! Set new XML Node attribute (or override attribute with same key).
	virtual void setAttr( const char* key,const char* value ) = 0;
	virtual void setAttr( const char* key,int value ) = 0;
	virtual void setAttr( const char* key,unsigned int value ) = 0;
	virtual void setAttr( const char* key,float value ) = 0;
	virtual void setAttr( const char* key,const Vec3 &value ) = 0;
	virtual void setAttr( const char* key,const Quat &value ) = 0;
	//////////////////////////////////////////////////////////////////////////
	// Inline Helpers.
	void setAttr( const char* key,unsigned long value ) { setAttr( key,(unsigned int)value ); };
	void setAttr( const char* key,long value ) { setAttr( key,(int)value ); };
	//////////////////////////////////////////////////////////////////////////
	

	//! Delete attrbute.
	virtual void delAttr( const char* key ) = 0;
	//! Remove all node attributes.
	virtual void removeAllAttributes() = 0;

	//! Get attribute value of node.
	virtual bool getAttr( const char *key,int &value ) const = 0;
	virtual bool getAttr( const char *key,unsigned int &value ) const = 0;
	virtual bool getAttr( const char *key,float &value ) const = 0;
	virtual bool getAttr( const char *key,Vec3 &value ) const = 0;
	virtual bool getAttr( const char *key,Quat &value ) const = 0;
	virtual bool getAttr( const char *key,bool &value ) const = 0;
	virtual bool getAttr( const char *key,XmlString &value ) const = 0;
	//////////////////////////////////////////////////////////////////////////
	// Inline Helpers.
	bool getAttr( const char *key,long &value ) const { int v; if (getAttr(key,v)) { value = v; return true; } else return false; }
	bool getAttr( const char *key,unsigned long &value ) const { unsigned int v; if (getAttr(key,v)) { value = v; return true; } else return false; }
	bool getAttr( const char *key,unsigned short &value ) const { unsigned int v; if (getAttr(key,v)) { value = v; return true; } else return false; }
	bool getAttr( const char *key,unsigned char &value ) const { unsigned int v; if (getAttr(key,v)) { value = v; return true; } else return false; }
	bool getAttr( const char *key,short &value ) const { int v; if (getAttr(key,v)) { value = v; return true; } else return false; }
	bool getAttr( const char *key,char &value ) const { int v; if (getAttr(key,v)) { value = v; return true; } else return false; }

	#ifdef  _AFX
	//////////////////////////////////////////////////////////////////////////
	// Get CString attribute.
	//////////////////////////////////////////////////////////////////////////
	bool getAttr( const char *key,CString &value ) const
	{
		if (!haveAttr(key))
			return false;
		value = getAttr(key);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Set GUID attribute.
	//////////////////////////////////////////////////////////////////////////
	void setAttr( const char* key,REFGUID value )
	{
		const char *str = GuidUtil::ToString(value);
		setAttr( key,str );
	};

	//////////////////////////////////////////////////////////////////////////
	// Get GUID from attribute.
	//////////////////////////////////////////////////////////////////////////
	bool getAttr( const char *key,GUID &value ) const
	{
		if (!haveAttr(key))
			return false;
		const char *guidStr = getAttr(key);
		value = GuidUtil::FromString( guidStr );
		if (value.Data1 == 0)
		{
			memset( &value,0,sizeof(value) );
			// If bad GUID, use old guid system.
			value.Data1 = atoi(guidStr);
		}
		return true;
	}
	#endif //_AFX

	//! Lets be friendly to him.
	friend class XmlNodeRef;
};

/*
///////////////////////////////////////////////////////////////////////////////
// Inline Implementation of XmlNodeRef
inline XmlNodeRef::XmlNodeRef( const char *tag,IXmlNode *node )
{
	if (node)
		p = node->createNode( tag );
	else
		p = new XmlNode( tag );
	p->AddRef();
}
*/

//////////////////////////////////////////////////////////////////////////
inline XmlNodeRef::XmlNodeRef( IXmlNode* p_ ) : p(p_)
{
	if (p) p->AddRef();
}

inline XmlNodeRef::XmlNodeRef( const XmlNodeRef &p_ ) : p(p_.p)
{
	if (p) p->AddRef();
}

inline XmlNodeRef::~XmlNodeRef()
{
	if (p) p->Release();
}

inline XmlNodeRef&  XmlNodeRef::operator=( IXmlNode* newp )
{
	if (newp) newp->AddRef();
	if (p) p->Release();
	p = newp;
	return *this;
}

inline XmlNodeRef&  XmlNodeRef::operator=( const XmlNodeRef &newp )
{
	if (newp.p) newp.p->AddRef();
	if (p) p->Release();
	p = newp.p;
	return *this;
}

inline bool operator == ( const XmlNodeRef &p1,int null )	{
	return p1.p == 0;
}

inline bool operator != ( const XmlNodeRef &p1,int null )	{
	return p1.p != 0;
}

inline bool operator == ( int null,const XmlNodeRef &p1 )	{
	return p1.p == 0;
}

inline bool operator != ( int null,const XmlNodeRef &p1 )	{
	return p1.p != 0;
}

#endif // __ixml_h__
