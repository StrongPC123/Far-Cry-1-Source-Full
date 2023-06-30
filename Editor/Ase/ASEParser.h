#ifndef __ASEParser_h__
#define __ASEParser_h__
#pragma once

#include "AnimObject.h"

//////////////////////////////////////////////////////////////////////////
class CASEParser
{
public:
	CASEParser();

	bool ParseString( CAnimObject *object,const char *buffer );
	bool Parse( CAnimObject *object,const char *filename );

private:
	void ParseGeomNode();
	void ParseTMAnimation();
	void ParseSceneParams();

	CAnimObject::Node* FindNode( const char *sNodeName );
	//! Convert ticks to time in seconds.
	float TickToTime( int ticks );
	Vec3 ScalePosition( const Vec3 &pos );

	// Get a next token from the stream.
	char* GetToken();
	bool SkipBlock();
	// Do a string comparison
	bool Compare( const char* token, const char* id );

	//! Get vector from the stream.
	Vec3d GetVec3();
	//! Get quaternion from the stream.
	Quat GetQuat();
	//! Get float from the stream.
	float GetFloat();
	//! Get integer from the stream.
	int GetInt();
	// Get a string from the stream and strip leading and trailing quotes.
	const char* GetString();
	
	//! Get next character in stream.
	char GetChar() { return m_buffer[m_currentPos++]; }
	//! Check if reached end of input buffer.
	bool IsEof() { return m_currentPos == m_buffer.size(); }

	//! Input buffer.
	std::string m_buffer;
	//! Current position in buffer.
	int m_currentPos;

	//! Current animation object.
	CAnimObject *m_animObject;
	//! Current node.
	CAnimObject::Node* m_node;

	typedef std::map<std::string,CAnimObject::Node*> NodeMap; 
	NodeMap m_nodeMap;

	// Stores global object parameters.
	struct SceneParams
	{
		int firstFrame;
		int lastFrame;
		int frameSpeed;
		int ticksPerFrame;
	};
	SceneParams m_scene;
};

#endif __ASEParser_h__
