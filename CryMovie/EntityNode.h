////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   entitynode.h
//  Version:     v1.00
//  Created:     23/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __entitynode_h__
#define __entitynode_h__

#if _MSC_VER > 1000
//#pragma once
#endif

#include <set>
#include <string>
#include "AnimNode.h"
#include "SoundTrack.h"

#define ENTITY_SOUNDTRACKS	3
#define ENTITY_EXPRTRACKS		3

typedef std::set<string>	TStringSet;
typedef TStringSet::iterator	TStringSetIt;

class CAnimEntityNode : public CAnimNode, ICharInstanceSink
{
public:
	CAnimEntityNode( IMovieSystem *sys );
	~CAnimEntityNode();

	virtual EAnimNodeType GetType() const { return ANODE_ENTITY; }

	void SetEntity( int Id) { m_EntityId=Id; m_entity=NULL; }
	void SetEntity( IEntity *entity );
	IEntity* GetEntity();

	void SetTarget( IAnimNode *node );
	IAnimNode* GetTarget() const;

	//////////////////////////////////////////////////////////////////////////
	// Overrides from CAnimNode
	//////////////////////////////////////////////////////////////////////////
	virtual void Animate( SAnimContext &ec );
	virtual void CreateDefaultTracks();

	void SetPos( float time,const Vec3 &pos );
	void SetRotate( float time,const Quat &quat );
	void SetScale( float time,const Vec3 &scale );

	Vec3 GetPos() { return m_pos; };
	Quat GetRotate() { return m_rotate; };
	Vec3 GetScale() { return m_scale; };

	void GetWorldTM( Matrix44 &tm );

	// ovverided from IAnimNode
//	IAnimBlock* CreateAnimBlock();

	// Ovveride SetAnimBlock
	void SetAnimBlock( IAnimBlock *block );

	//////////////////////////////////////////////////////////////////////////
	void Serialize( XmlNodeRef &xmlNode,bool bLoading );
	void Reset();
	void Pause();

	//////////////////////////////////////////////////////////////////////////
	virtual int GetParamCount() const;
	virtual bool GetParamInfo( int nIndex, SParamInfo &info ) const;
	virtual bool GetParamInfoFromId( int paramId, SParamInfo &info ) const;

	static void AddSupportedParams( std::vector<IAnimNode::SParamInfo> &nodeParams );

private:
	void CalcLocalMatrix();
	void InvalidateTM();

	void ReleaseSounds();
	void ApplyEventKey( class CEventTrack *track,int keyIndex,IEventKey &key );
	void ApplySoundKey( IAnimTrack *pTrack,int nCurrKey,int nLayer, ISoundKey &key, SAnimContext &ec);
	void AnimateCharacterTrack( class CCharacterTrack* track,SAnimContext &ec,int layer );
	void StopExpressions();
	void AnimateExpressionTrack(class CExprTrack *pTrack, SAnimContext &ec);

	void ReleaseAllAnims();
	virtual void OnStartAnimation(const char *sAnimation) {}
	virtual void OnAnimationEvent(const char *sAnimation, AnimSinkEventData pUserData) {}
	virtual void OnEndAnimation(const char *sAnimation);

	//! Search entity system for specified entityid, return true if entity found and assigned to m_entity.
	bool ResolveEntity();

	//! Reference to game entity.
	IEntity *m_entity;
	EntityId m_EntityId;

	TStringSet m_setAnimationSinks;

	IMovieSystem *m_pMovie;

	//! Pointer to target animation node.
	TSmartPtr<IAnimNode> m_target;

	// Cached parameters of node at givven time.
	float m_time;
	Vec3 m_pos;
	Quat m_rotate;
	Vec3 m_scale;

	bool m_visible;

	//! Last animated key in Entity track.
	int m_lastEntityKey;
	int m_lastCharacterKey[3];
	SSoundInfo m_SoundInfo[ENTITY_SOUNDTRACKS];
	TStringSet m_setExpressions;

	//! World transformation matrix.
	Matrix44 m_worldTM;
	unsigned m_bMatrixValid : 1;	//!< True if current world transformation matrix is valid.
	unsigned m_bMatrixInWorldSpace : 1;	//!< True if current matrix is in world space.
};

#endif // __entitynode_h__
