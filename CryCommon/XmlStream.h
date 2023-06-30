//////////////////////////////////////////////////////////////////////
//
//  Crytek (C) 2001
//
//  CryCommon Source Code
//
//  File: XmlStream.h
//  Description: XML Parser based on the code of codeguru.com.
//
//  History:
//  - August 26, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef CRYCOMMON_XMLSTREAM_H
#define CRYCOMMON_XMLSTREAM_H

#include <XmlParserFont.h>

#include <string>
//#include <iostream>

/////////////////////////////////////////////////////////////////////////////////
// XmlNotify
//
// Purpose:		abstract class used to notify about xml tags found. 
struct IXmlNotify
{
public:
	// notify methods
	virtual void FoundElement	( const string & name, const string & value ) = 0;
	virtual void FoundAttribute	( const string & name, const string & value ) = 0;
};


//TiZ #define _SHOWDEBUGINFO

//////////////////////////////////////////////////////////////////////////////
// XmlStream
//
// Purpose: stores a string that contain start,end tag delimited value

class XmlStream :
	public string
{
	IXmlNotify *			_subscriber;	// notification subscriber

public:

	XmlStream (IXmlNotify *pNotify = NULL) :
		string ()
	{
			_subscriber = pNotify;
	}

	virtual ~XmlStream ()
	{ 
		release(); 
	}

	// release resources
	bool create ()
	{
		return true;
	}

	bool create ( char * buffer, long len )
	{
		if ( buffer && len > 0 )
		{
			assign( buffer, len );
			return true;
		}
		else
			return false;
	}

	void release ()
	{
		erase( begin(), end() );
	}

	// parse the attibutes and values
	void parseAttributesValues(string &att)
	{
		if(!att.size())
			return;

		char *p = (char*)att.c_str();
		while(*p)
		{
			string attrib = "";
			string value = "";

			while(*p && (*p == ' '  || *p == '=' || *p == '"'))
			{			
				p++;
			}

			while(*p && *p != ' ' && *p != '=')
			{
				attrib += *p;
				p++;
			}

			while(*p && (*p == ' '  || *p == '=' || *p == '"'))
			{			
				p++;
			}

			while(*p && *p != '"')
			{
				value += *p;
				p++;
			}

			if ( _subscriber )
			{
				if(attrib.size() && value.size())
					_subscriber->FoundAttribute(attrib,value);
			}
		}
	}

	// notify methods
	void foundNode		( string & name, string & attributes )
	{
		// if no name stop
		if ( name.empty() )
			return;

		// tell subscriber
		if ( _subscriber )
		{
			_subscriber->FoundElement(name,string(""));
			parseAttributesValues(attributes);		
		}
	}

	void foundElement	( string & name, string & value, string & attributes )
	{
		// if no name stop
		if ( name.empty() )
			return;

		// tell subscriber
		if ( _subscriber )
		{
			_subscriber->FoundElement(name,value);
			parseAttributesValues(value);
			parseAttributesValues(attributes);
		}
	}

	// save/load stream
	bool save		( char * buffer )
	{
		// show success
		bool success = true;
		//try
		//{
			// save stream to buffer
			memcpy( buffer,(char *) c_str(), size() );
		//}
		//catch(...)
		//{
		//	success = false;
		//}

		return success;
	}

	bool load		( char * buffer )
	{
		// if invalid show failed
		if ( !buffer )
			return false;

		// show success
		bool success = true;
		//try
		//{
			// load stream from buffer
			str() = buffer;
		//}
		//catch(...)
		//{
		//	success = false;
		//}

		return success;
	}

	// parse the current buffer
	bool parse			( char * buffer, long length )
	{
		// if invalid stop
		if ( !buffer || length <= 0 )
			return false;

		// debug info
		assign( buffer, length );

		// debug info
#ifdef _SHOWDEBUGINFO
		{
			cout << "----- parse document -----" << endl;
			cout << buffer << endl;
			cout << endl;
			cout << "----- start parsing -----" << endl;
			cout << endl;
		}
#endif

		// declare parser
		XmlParserFont parser;

		// parse nodes
		bool docParsed = parseNodes(parser,buffer,length);

		return docParsed;
	}

	bool parseNodes		( XmlParserFont & parser, char * buffer, long parseLength )
	{
		// #DGH note
		// need to address a null node within another node
		// i.e.
		// <Contacts>
		//		<Contact/>
		// </Contacts>
		// in this instance Contact will be reported as an
		// element 

		// debug info
		///string::iterator buf = buffer;
#ifdef _SHOWDEBUGINFO
		{
			cout << "parse node: " << parser._id << endl;
			cout << endl;
		}
#endif

		// while parse success, note for the first parser
		// this set the internal state also
		while ( parser.parse(buffer,parseLength) )
		{
			// if the value has a tag marker
			if ( parser.valueHasTag() )
			{
				// show found node
				string   name		 = parser.getName();
				string	 attributes  = parser.getAttributes();

				foundNode( name, attributes );				

				// get the parse state
				long valueLength;
				char * valueBuffer =
				parser.getValueState(valueLength);

				// if parsing the node fails
				XmlParserFont parserNode;
				if ( !parseNodes(parserNode,valueBuffer,valueLength) )
					return false;

				// if new parse cur position is in the last parser
				// last tag position we are done with the node
				char * curPos     = parserNode.getCurPos();
				char * lastCurPos = parser.getLastTagPos();
				if ( curPos >= lastCurPos )
				{
					break;
				}
			}
			else
			{
				// show found element
				string   name		 = parser.getName();
				string   value		 = parser.getValue();
				string	 attributes  = parser.getAttributes();

				foundElement(name,value,attributes);
			}

			// get new parse state
			buffer =
			parser.getParseState(parseLength);
		}

		return true;
	}

	// get/set subscriber
	bool hasSubscriber ()
	{
		if ( _subscriber )
			return true;
		else
			return false;
	}

	IXmlNotify * getSubscriber ()
	{
		return _subscriber;
	}

	void setSubscriber ( IXmlNotify *set )
	{
		_subscriber = set;
	}


	// get ref to tag stream
	XmlStream & getTagStream ()
	{ return *this; }


	// get string ref
	string & str()
	{ return (string &) *this; }
};

#endif // CRYCOMMON_XMLSTREAM_H
