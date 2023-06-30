////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   animsequence.h
//  Version:     v1.00
//  Created:     26/4/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Implementation of IAnimSequence interface.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __animsequence_h__
#define __animsequence_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "IMovieSystem.h"

class CAnimSequence : public IAnimSequence
{
public:
	CAnimSequence( IMovieSystem *pMovieSystem );

	// Movie system.
	IMovieSystem* GetMovieSystem() const { return m_pMovieSystem; };

	void SetName( const char *name );
	const char* GetName();

	virtual void SetFlags( int flags );
	virtual int GetFlags() const;

	void SetTimeRange( Range timeRange );
	Range GetTimeRange() { return m_timeRange; };

	void ScaleTimeRange( const Range &timeRange );

	//! Return number of animation nodes in sequence.
	int GetNodeCount() const;
	//! Get specified animation node.
	IAnimNode* GetNode( int index ) const;
	//! Get Animation assigned to node at specified index.
	IAnimBlock* GetAnimBlock( int index ) const;

	void Reset();
	void Pause();
	void Resume();

	//! Add animation node to sequence.
	bool AddNode( IAnimNode *node );
	//! Remove animation node from sequence.
	void RemoveNode( IAnimNode *node );
	//! Add scene node to sequence.
	IAnimNode* AddSceneNode();
	void RemoveAll();

	void Activate();
	void Deactivate();
	void Animate( SAnimContext &ec );

	void Serialize( XmlNodeRef &xmlNode,bool bLoading, bool bLoadEmptyTracks=true );

private:
	void ComputeTimeRange();

	struct ANode
	{
		TSmartPtr<IAnimNode> node;
		TSmartPtr<IAnimBlock> anim;
	};
	typedef std::vector<ANode> AnimNodes;
	AnimNodes m_nodes;

	AnimString m_name;
	Range m_timeRange;

	int m_flags;

	TSmartPtr<IAnimNode> m_pSceneNode;

	//
	IMovieSystem *m_pMovieSystem;
	bool m_bPaused;
};

#endif // __animsequence_h__