////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   animnode.h
//  Version:     v1.00
//  Created:     23/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: Base of all Animation Nodes
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __animnode_h__
#define __animnode_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "IMovieSystem.h"
/*
// Common parameter-bits of animation node.
enum AnimParamTypeBits
{
	APARAMBIT_POS =			0x00000001,	//!< Position parameter id.
	APARAMBIT_ROT =			0x00000002,	//!< Rotation parameter id.
	APARAMBIT_SCL =			0x00000004,	//!< Scale parameter id.
	APARAMBIT_ENTITY =	0x00000008,	//!< Entity parameter id.
	APARAMBIT_VISIBLE =	0x00000010,	//!< Visibilty parameter id.
	APARAMBIT_CAMERA =	0x00000020,	//!< Camera parameter id.
	APARAMBIT_FOV =			0x00000040,	//!< FOV parameter id.
};*/

#define PARAM_BIT(param) (1<<(param))

class CAnimBlock : public IAnimBlock
{
public:
	void SetId( int id ) { m_id = id; };
	int	GetId() const { return m_id; };

	int GetTrackCount() const;
	bool GetTrackInfo( int index,int &paramId,IAnimTrack **pTrack ) const;

	const char* GetParamName( AnimParamType param ) const;

	IAnimTrack* GetTrack( int paramId ) const;
	void SetTrack( int paramId,IAnimTrack *track );
	IAnimTrack* CreateTrack( int paramId,EAnimValue valueType );

	void SetTimeRange( Range timeRange );

	bool RemoveTrack( IAnimTrack *track );

	void Serialize( IAnimNode *pNode,XmlNodeRef &xmlNode,bool bLoading, bool bLoadEmptyTracks=true );

private:
	void AddTrack( int param,IAnimTrack *track );

	struct TrackDesc
	{
		int paramId;           // Track parameter id.
		TSmartPtr<IAnimTrack> track;  // Track pointer.
	};
	std::vector<TrackDesc> m_tracks;
	int m_id;
};

/*!
		Base class for all Animation nodes,
		can host multiple animation tracks, and execute them other time.
		Animation node is reference counted.
 */
class CAnimNode : public IAnimNode
{
public:
	CAnimNode( IMovieSystem *sys );
	~CAnimNode();

	void SetName( const char *name ) { strncpy(m_name,name,sizeof(m_name)-1); int nLen=strlen(name); if (nLen>sizeof(m_name)-1) nLen=sizeof(m_name)-1; m_name[nLen]=0; m_pMovieSystem->Callback(CBR_CHANGENODE); };
	const char* GetName() { return m_name; };

	void SetId( int id ) { m_id = id; };
	int GetId() const { return m_id; };

	void SetEntity( int Id) {}
	void SetEntity( IEntity *entity ) {}
	IEntity* GetEntity() { return NULL; }

	void SetFlags( int flags );
	int GetFlags() const;

	virtual IAnimTrack* CreateTrack( int paramId );
	virtual void CreateDefaultTracks() {};
	virtual bool RemoveTrack( IAnimTrack *pAnimTrack );
	int FindTrack(IAnimTrack *pTrack);

	IMovieSystem*	GetMovieSystem() { return m_pMovieSystem; };

	virtual void Reset() {}
	virtual void Pause() {}
	virtual void Resume() {}

	//////////////////////////////////////////////////////////////////////////
	// Space position/orientation scale.
	//////////////////////////////////////////////////////////////////////////
	void SetPos( float time,const Vec3 &pos ) {};
	void SetRotate( float time,const Quat &quat ) {};
	void SetScale( float time,const Vec3 &scale ) {};

	Vec3 GetPos() { return Vec3(0,0,0); };
	Quat GetRotate() { return Quat(0,0,0,0); };
	Vec3 GetScale() { return Vec3(0,0,0); };

	//////////////////////////////////////////////////////////////////////////
	bool SetParamValue( float time,AnimParamType param,float val );
	bool GetParamValue( float time,AnimParamType param,float &val );

	//////////////////////////////////////////////////////////////////////////
	void GetWorldTM( Matrix44 &tm ) { tm.SetIdentity(); };

	void SetTarget( IAnimNode *node ) {};
	IAnimNode* GetTarget() const { return 0; };

	void Animate( SAnimContext &ec );

	IAnimBlock* GetAnimBlock() const { return m_animBlock; };
	void SetAnimBlock( IAnimBlock *block );
	IAnimBlock* CreateAnimBlock() { return new CAnimBlock; };

	IAnimTrack* GetTrack( int nParamId ) const;
	void SetTrack( int nParamId,IAnimTrack *track );

	//////////////////////////////////////////////////////////////////////////
	// Support Child nodes.
	//////////////////////////////////////////////////////////////////////////

	//! Return true if node have childs.
	virtual bool HaveChilds() const { return !m_childs.empty(); }
	//! Return true if have attached childs.
	virtual int GetChildCount() const { return m_childs.size(); }

	//! Get child by index.
	virtual IAnimNode* GetChild( int i ) const;
	//! Return parent node if exist.
	virtual IAnimNode* GetParent() const { return m_parent; }
	//! Scans hiearachy up to determine if we child of specified node.
	virtual bool IsChildOf( IAnimNode *node );
	
	//! Attach new child node.
	virtual void AttachChild( IAnimNode* child );
	
	//! Detach all childs of this node.
	virtual void DetachAll();
	
	//! Detach this node from parent.
	//! Warning, if node is only referenced from parent, calling this will delete node.
	virtual void DetachThis();

	virtual void Serialize( XmlNodeRef &xmlNode,bool bLoading );

	void RegisterCallback( IAnimNodeCallback *callback ) { m_callback = callback; m_pMovieSystem->Callback(CBR_REGISTERNODECB); };
	void UnregisterCallback( IAnimNodeCallback *callback ) { m_callback = NULL; m_pMovieSystem->Callback(CBR_UNREGISTERNODECB); };

	bool IsParamValid( int paramId ) const;

private:
	// Remove child from our childs list.
	void RemoveChild( CAnimNode *node );

	//! Id of animation node.
	int m_id;

	//! Name of animation node.
	char m_name[64];

	//! Pointer to parent node.
	CAnimNode *m_parent;

	//! Child animation nodes.
	typedef std::vector<TSmartPtr<CAnimNode> > Childs;
	Childs m_childs;

	typedef std::vector<SAnimParam> Params;
	Params m_params;

	TSmartPtr<IAnimBlock> m_animBlock;

protected:
	IAnimNodeCallback* m_callback;
	IMovieSystem* m_pMovieSystem;
	unsigned int m_dwSupportedTracks;
	int m_flags;
};

#endif // __animnode_h__
