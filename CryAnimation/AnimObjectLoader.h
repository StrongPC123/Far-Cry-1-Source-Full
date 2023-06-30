#ifndef __AnimObjectLoader_h__
#define __AnimObjectLoader_h__
#pragma once

#include "AnimObject.h"
#include "Controller.h"

class CChunkFileReader;

//////////////////////////////////////////////////////////////////////////
// Loads AnimObject from CGF/CAF files.
//////////////////////////////////////////////////////////////////////////
class CAnimObjectLoader
{
public:
	// Load animation object from cgf or caf.
	bool Load( CAnimObject* animObject,const char *geomName,const char *animFile );

private:
	void LoadChunks( CChunkFileReader* pReader,bool bMakeNodes );
	void LoadNodeChunk( int chunkID,const void* pChunk,bool bMakeNode );
	void LoadControllerChunk( int chunkID,const void* pChunk );
	void InitNodes();

	// Load all animations for this object.
	void LoadAnimations( const char *cgaFile );
	bool LoadAnimation( const char *animFile );
	
	// Convert controller ticks to time in seconds.
	float TickToTime( int ticks )
	{
		return (float)ticks * m_secsPerTick;
	}

	// Internal description of node.
	struct NodeDesc
	{
		int parentID;
		int pos_cont_id;	// position controller chunk id
		int	rot_cont_id;	// rotation controller chunk id
		int	scl_cont_id;	// scale controller chunk id
		Vec3 pos;
		CryQuat rotate;
		Vec3 scale;
		CAnimObject::Node* node;
	};
	
	//////////////////////////////////////////////////////////////////////////
	typedef std::map<int,NodeDesc> NodeDescMap;
	NodeDescMap m_nodeMap;

	//////////////////////////////////////////////////////////////////////////
	typedef std::map<string,CAnimObject::Node*> NodeNamesMap;
	NodeNamesMap m_nodeNameMap;


	// Array of controllers.
	IController** m_controllers;
	int m_numChunks;

	// ticks per one max frame.
	int m_ticksPerFrame;
	//! controller ticks per second.
	float m_secsPerTick;
	float m_animStart;
	float m_animEnd;

	// Created animation object
	CAnimObject* m_animObject;
	CAnimObject::Animation* m_currAnimation;
};

#endif //__AnimObjectLoader_h__