//////////////////////////////////////////////////////////////////////
//
//	Crytek Common Source code
//	
//	File:CryEngineDecalInfo.h
//	Description: declaration of struct CryEngineDecalInfo.
//
//	History:
//	-Sep 23, 2002: Created by Sergiy Migdalskiy
//
//	Note:
//    3D Engine and Character Animation subsystems (as well as perhaps
//    some others) transfer data about the decals that need to be spawned
//    via this structure. This is to avoid passing many parameters through
//    each function call, and to save on copying these parameters when just
//    simply passing the structure from one function to another.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CRY_ENGINE_DECAL_INFO_HDR_
#define _CRY_ENGINE_DECAL_INFO_HDR_

// Summary:
//     Structure containing common parameters that describe a decal
struct CryEngineDecalInfo
{
	struct IEntityRender * pDecalOwner; // Owner (decal will be rendered on this entity)
	Vec3_tpl<float> vPos;				// Decal position (world coordinates)
	Vec3_tpl<float> vNormal;			// Decal/face normal
	float fSize;						// Decal size
	float fLifeTime;					// Decal life time (in seconds)
	INT_PTR	nTid;						// Texture Id		//@@ AMD Port
	float fAngle;						// Angle of rotation
	INT_PTR	nPartID;					// Entity part id which determines the bone id in case of a character which has been the hit	//@@AMD Port
	struct IStatObj * pStatObj;			// Decal geometry
	Vec3_tpl<float> vHitDirection;		// Direction from weapon/player position to decal position (bullet direction)
	float m_fGrowTime;					// Used for blood pools
	bool bAdjustPos;					// Place decal on some visible surface

	// the constructor fills in some non-obligatory fields; the other fields must be filled in by the client
	CryEngineDecalInfo ()
	{
		memset(this,0,sizeof(*this));
		nPartID = -1;
		bAdjustPos = true;
	}
};


#endif