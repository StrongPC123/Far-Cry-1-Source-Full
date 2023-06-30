#ifndef __XML_HEADER__
#define __XML_HEADER__

#include <vector>
#include <set>
#include <string>

//#include "ixml.h"

/************************************************************************/
/* XmlParser class, Parse xml and return root xml node if success.      */
/************************************************************************/
class XmlParser
{
public:
	//! Parse xml file.
	XmlNodeRef parse( const char *fileName );
	
	//! Parse xml from memory buffer.
	XmlNodeRef parseBuffer( const char *buffer );

	const char* getErrorString() const { return m_errorString; }
private:
	XmlString m_errorString;
};

//////////////////////////////////////////////////////////////////////////
class XmlAttribute {
public:
	const char* key;
	const char* value;

	XmlAttribute() { m_bOwnKey = false; m_bOwnValue = false; }
	//XmlAttribute( const char *k,const char *v ) : key(k),value(v) {}
	//explicit XmlAttribute( const char *k ) : key(k) {}
	XmlAttribute( const XmlAttribute &attr ) { m_bOwnKey = false; m_bOwnValue = false; *this = attr; }
	XmlAttribute& operator=( const XmlAttribute &attr )
	{
		if (m_bOwnKey) free( (void*)key );
		if (m_bOwnValue) free( (void*)value );

		int keylen = strlen(attr.key);
		int valuelen = strlen(attr.value);
		if (keylen < sizeof(m_key))
		{
			m_bOwnKey = false;
			strcpy(m_key,attr.key);
			key = m_key;
		}
		else
		{
			m_bOwnKey = true;
			char *sKey = (char*)malloc(keylen+1);
			strcpy( sKey,attr.key );
			key = sKey;
		}
		if (valuelen < sizeof(m_value))
		{
			m_bOwnValue = false;
			strcpy(m_value,attr.value);
			value = m_value;
		}
		else
		{
			m_bOwnValue = true;
			char *sValue = (char*)malloc(valuelen+1);
			strcpy( sValue,attr.value );
			value = sValue;
		}
		return *this;
	}
	~XmlAttribute() {
		if (m_bOwnKey) free( (void*)key );
		if (m_bOwnValue) free( (void*)value );
	}
	bool operator<( const XmlAttribute &attr ) const { return stricmp( key,attr.key ) < 0; }
	bool operator>( const XmlAttribute &attr ) const { return stricmp( key,attr.key ) > 0; }
	bool operator==( const XmlAttribute &attr ) const { return stricmp( key,attr.key ) == 0; }
	bool operator!=( const XmlAttribute &attr ) const { return stricmp( key,attr.key ) != 0; }
private:
	char m_key[16];
	char m_value[16];
	unsigned m_bOwnKey : 1;
	unsigned m_bOwnValue : 1;
};

//! Xml node attributes class.
typedef std::set<XmlAttribute>	XmlAttributes;

/**
 ******************************************************************************
 * CXmlNode class
 * Never use CXmlNode directly instead use reference counted XmlNodeRef.
 ******************************************************************************
 */

class CXmlNode : public IXmlNode
{
public:
	//! Constructor.
	CXmlNode( const char *tag );
	//! Destructor.
	~CXmlNode();

	//////////////////////////////////////////////////////////////////////////
	//! Reference counting.
	void AddRef() { m_refCount++; };
	//! When ref count reach zero XML node dies.
	void Release() { if (--m_refCount <= 0) delete this; };

	//! Create new XML node.
	XmlNodeRef createNode( const char *tag );

	//! Get XML node tag.
	const char *getTag() const { return m_tag; };
	void	setTag( const char *tag ) { m_tag = tag; }

	//! Return true if givven tag equal to node tag.
	bool isTag( const char *tag ) const;

	//! Get XML Node attributes.
	//! Get XML Node attributes.
	virtual int getNumAttributes() const { return m_attributes.size(); };
	//! Return attribute key and value by attribute index.
	virtual bool getAttributeByIndex( int index,const char **key,const char **value );
	virtual void copyAttributes( XmlNodeRef fromNode );

	//! Get XML Node attribute for specified key.
	const char* getAttr( const char *key ) const;
	//! Check if attributes with specified key exist.
	bool haveAttr( const char *key ) const;
	
	//! Adds new child node.
	void addChild( XmlNodeRef &node );

	//! Creates new xml node and add it to childs list.
	XmlNodeRef newChild( const char *tagName );

	//! Remove child node.
	void removeChild( XmlNodeRef &node );

	//! Remove all child nodes.
	void removeAllChilds();

	//! Get number of child XML nodes.
	int	getChildCount() const { return m_childs.size(); };
	
	//! Get XML Node child nodes.
	XmlNodeRef getChild( int i ) const;

	//! Find node with specified tag.
	XmlNodeRef findChild( const char *tag ) const;

	//! Get parent XML node.
	XmlNodeRef	getParent() const { return m_parent; }

	//! Returns content of this node.
	const char* getContent() const { return m_content; };
	void setContent( const char *str ) { m_content = str; };
	void addContent( const char *str ) { m_content += str; };

	XmlNodeRef	clone();

	//! Returns line number for XML tag.
	int getLine() const { return m_line; };
	//! Set line number in xml.
	void setLine( int line ) { m_line = line; };

	//! Returns XML of this node and sub nodes.
	XmlString getXML( int level=0 ) const;
	bool saveToFile( const char *fileName );

	//! Set new XML Node attribute (or override attribute with same key).
	void setAttr( const char* key,const char* value );
	void setAttr( const char* key,int value );
	void setAttr( const char* key,unsigned int value );
	void setAttr( const char* key,float value );
	void setAttr( const char* key,const Vec3 &value );
	void setAttr( const char* key,const Quat &value );

	//! Delete attrbute.
	void delAttr( const char* key );
	//! Remove all node attributes.
	void removeAllAttributes();

	//! Get attribute value of node.
	bool getAttr( const char *key,int &value ) const;
	bool getAttr( const char *key,unsigned int &value ) const;
	bool getAttr( const char *key,float &value ) const;
	bool getAttr( const char *key,Vec3 &value ) const;
	bool getAttr( const char *key,Quat &value ) const;
	bool getAttr( const char *key,bool &value ) const;
	bool getAttr( const char *key,XmlString &value ) const { assert(0); return false; };

//	bool getAttr( const char *key,CString &value ) const { XmlString v; if (getAttr(key,v)) { value = (const char*)v; return true; } else return false; }
private:
	void AddToXmlString( CString &xml,int level ) const;

private:
	//! Ref count itself, its zeroed on node creation.
	int m_refCount;

	//! Line in XML file where this node firstly appeared (usefull for debuggin).
	int m_line;
	//! Tag of XML node.
	XmlString m_tag;

	//! Content of XML node.
	XmlString m_content;
	//! Parent XML node.
	CXmlNode *m_parent;
	//! Next XML node in same hierarchy level.
	
	typedef std::vector<XmlNodeRef>	XmlNodes;
	XmlNodes m_childs;
	//! Xml node attributes.
	XmlAttributes m_attributes;

	static XmlAttribute tempAttr;
};

#endif // __XML_HEADER__