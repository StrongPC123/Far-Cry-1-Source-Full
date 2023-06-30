
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// History:
// 02/12/2002 * created by M.M. (modified version of CSpectator)
//
//////////////////////////////////////////////////////////////////////

#ifndef _ADVCAMSYSTEM_H_
#define _ADVCAMSYSTEM_H_

#include "GameObject.h"
#include <IEntitySystem.h>

class CXGame;

/*!implements the AdvancedCameraSystem container
*/
class CAdvCamSystem : public CGameObject
{
public:

	//! constructor
	CAdvCamSystem( CXGame *pGame );

	//! destructor
	virtual ~CAdvCamSystem( void ) {}

	//! process input
	void ProcessKeys( CXEntityProcessingCmd &epc );

	// interface IEntityContainer -------------------------------

	virtual bool Init( void );
	virtual void Update( void );
	virtual bool Write( CStream &stm, EntityCloneState *cs=NULL );
	virtual bool Read( CStream &stm );
	virtual IScriptObject *GetScriptObject( void );	
	virtual void SetScriptObject(IScriptObject *object);
	virtual void OnSetAngles( const Vec3 &ang );
	virtual void OnDraw( const SRendParams & RendParams ) {}
	virtual bool QueryContainerInterface( ContainerInterfaceType desired_interface, void **pInterface);
	virtual void GetEntityDesc( CEntityDesc &desc ) const;

	//! Set the radius at which player a will follow player b
	void SetMaxRadius(float radius);
	//! Set the minimum distance the players can be together
	void SetMinRadius(float radius);
private:
	/*! Calculate the movement of a player based on the input received by the specified controller.

			@param	matCamera orientation of camera
			@param	joyID number of joystick used for player input
			@param  linked 

			@return	movement direction of the player (not normalized)
	*/
	Vec3 CalcPlayerMoveDirection(const Matrix33 &matCamera, unsigned int joyID) const;

	/*!	Calculate the source and target vectors of the camera based on the script-side parameter table, which is passed
			to the function via the script object.

			@param	scriptObject ScriptObject which contains the parameters for the camera setup (see AdvCamSystem.lua)
			@param	vSrcPos	the source vector returned by the function (reset to zero vector internally)
			@param	vDstPos	the target vector returned by the function (reset to zero vector internally)
	*/
	void CalcCameraVectors(_SmartScriptObject &scriptObject, Vec3& vSrcPos, Vec3& vDstPos);

	void SetMoveDirection(CPlayer &player, const IEntity &entity, CXEntityProcessingCmd &epc, const Vec3 &vMoveDir, bool linked) const;

	void PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime) {};

private:
	IScriptObject *	m_pScriptObject;					//!< to fulfull the IEntityContainer interface
	CXGame *				m_pGame;									//!< pointer to the game where we are, must not be 0

	EntityId				m_eiPlayerA;							//!< entity id for player a, may be 0=not assigned
	EntityId				m_eiPlayerB;							//!< entity id for player b, may be 0=not assigned

	float						m_fMaxRadius;					//!< Radius at which player A will start following player B, default 5, must be positive
	float						m_fMinRadius;					//!< Player A and Player B can not get closer than this

	friend class CScriptObjectAdvCamSystem;
};

#endif // _ADVCAMSYSTEM_H_