
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#ifndef _SPECTATOR_H_
#define _SPECTATOR_H_

#include "GameObject.h"
#include <IEntitySystem.h>
#include "xarea.h"

class CXGame;

/*!implements the specatator container
	this class is mainly used to parse the input of a the remote client
*/
class CSpectator :public CGameObject
{
public:

	//! constructor
	CSpectator(CXGame *pGame);
	//! destructor
	virtual ~CSpectator();

	//!
	void OnSetAngles( const Vec3 &ang );
	//!
	IScriptObject *GetScriptObject();	
	//!
	void SetScriptObject(IScriptObject *object);
	//!
	bool IsMySpectator() const;
	//!
	void GetEntityDesc( CEntityDesc &desc ) const;
	//!
	void ProcessKeys(CXEntityProcessingCmd &epc);
	//!
	void PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime) {};
	//!
	EntityId GetHostId() const;
	//! free the camera (no host following)
	void ResetHostId();

	// interface IEntityContainer ------------------------------------------------------------

	virtual bool Init();
	virtual void Update();
	virtual bool Read(CStream &stm);
	virtual bool Write(CStream &stm,EntityCloneState *cs=NULL);
	virtual void OnDraw(const SRendParams & RendParams){};
	virtual bool QueryContainerInterface( ContainerInterfaceType desired_interface, void **pInterface);
	virtual void OnEntityNetworkUpdate( const EntityId &idViewerEntity, const Vec3d &v3dViewer, uint32 &inoutPriority, 
		EntityCloneState &inoutCloneState ) const;

private: // ---------------------------------------------------------------------------

	EntityId					m_eiHost;							//!< 0 or the entity if we are currently spectating
	IScriptObject *		m_pScriptObject;			//!<
	CXGame *					m_pGame;							//!<
	ITimer *					m_pTimer;							//!<
	float							m_roll;								//!< roll angle
	Vec3d							m_vAngles;						//!< in degrees
	float							m_fLastTargetSwitch;	//!< 

	friend class CScriptObjectSpectator;

	CXAreaUser				m_AreaUser;						//!<
};

#endif