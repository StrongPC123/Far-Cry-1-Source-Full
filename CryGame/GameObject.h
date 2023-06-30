
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#ifndef _GAMEOBJECT_H_
#define _GAMEOBJECT_H_

struct IScriptObject;

/*!@see IEntityContainer
*/
class CGameObject : 
public IEntityContainer
{
public:
	//! constructor
	CGameObject() {/* m_refCount = 0;*/m_pEntity = NULL; }
	//! destructor
	virtual ~CGameObject() {}

	//! Return entity used by this game object.
	IEntity* GetEntity() const { return m_pEntity; }

	//! Forwards name calls to entity.
	void	SetName( const char *s ) { m_pEntity->SetName(s); };
	//!@return the name of the entity
	const char *GetName() const { return m_pEntity->GetName(); };

	// interface IEntityContainer ------------------------------------------------------------

	virtual void SetEntity( IEntity *e ) { m_pEntity = e; }
	virtual void OnSetAngles( const Vec3 &ang ){};
	virtual Vec3 CalcSoundPos() { if (m_pEntity) return m_pEntity->GetPos(); return Vec3(0.0f, 0.0f, 0.0f); }
	virtual void Release() {  delete this; };
	virtual bool QueryContainerInterface(ContainerInterfaceType desired_interface, void** ppInterface) { *ppInterface=0; return false;}
	virtual void OnDraw(const SRendParams & RendParams) {}
	virtual bool IsSaveable() { return(true); }
	virtual void OnEntityNetworkUpdate( const EntityId &idViewerEntity, const Vec3d &v3dViewer, uint32 &inoutPriority, 
		EntityCloneState &inoutCloneState ) const {}

protected: // ----------------------------------------------------------------------------------------

	IEntity *					m_pEntity;						//!< Entity attached to object.
};

#endif //_GAMEOBJECT_H_