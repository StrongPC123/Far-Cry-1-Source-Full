////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   i3dengine.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 3dengine interface
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef CRY3DENGINEINTERFACE_H
#define CRY3DENGINEINTERFACE_H

#include "platform.h"

#ifdef WIN32
	#ifdef CRY3DENGINE_EXPORTS
		#define CRY3DENGINEENGINE_API __declspec(dllexport)
	#else
		#define CRY3DENGINEENGINE_API __declspec(dllimport)
	#endif
#else
	#define CRY3DENGINEENGINE_API
#endif

// !!! Do not add any headers here !!!
#include <IProcess.h>
#include <CryEngineDecalInfo.h> 
#include "IStatobj.h"
// !!! Do not add any headers here !!!

struct ISystem;
struct ITexPic;
struct ICryCharInstance;
struct CVars;
struct pe_params_particle;
struct IMatInfo;
struct RenderLMData;
struct AnimTexInfo;
template <class T> class list2;

#if defined(LINUX)
	#include "Splash.h"
#endif


/*! SVariationValue used to specify value, which can have random variance.
		Used by particle system parameters.
 */
template <class T>
struct SVariationValue
{
	T value;
	// Random variation value.
	float variation;

	SVariationValue() { variation = 0; }

	//! Cast to holded type.
	operator T() const { return value; }
	//! Assign operator for variable.
	void operator=( const T& val ) { value = val; }

	T GetVariantValue() const { return value*(1 + variation*GenRand()); }

private:
	//! Generate random value in [-1,+1].
	float GenRand() const { return 2.0f*((float)rand()/RAND_MAX) - 1.0f; }
};

// Specialation for vectors.
inline Vec3 SVariationValue<Vec3>::GetVariantValue() const
{
	Vec3 v;
	v.x = value.x*(1 + variation*GenRand());
	v.y = value.y*(1 + variation*GenRand());
	v.z = value.z*(1 + variation*GenRand());
	return v;
}

//////////////////////////////////////////////////////////////////////////
typedef SVariationValue<float> FloatVariant;
typedef SVariationValue<Vec3> Vec3Variant;


//! Particle Blend Type
enum ParticleBlendType
{
  ParticleBlendType_AlphaBased,
  ParticleBlendType_ColorBased,
  ParticleBlendType_Additive,
  ParticleBlendType_None
};

//! Particle system parameters
struct ParticleParams
{
	ParticleParams() { memset(this,0,sizeof(*this)); }

	Vec3 vPosition; // Spawn position
	Vec3 vDirection; // Initial direction  (normalization not important)
	float fFocus; // If set to 0, the particles go in all directions, but if it's set to more than 20, the particles will mostly go in vDirection
	Vec3 vColorStart; // Initial color
	Vec3 vColorEnd; // Final color
	FloatVariant fSpeed; // Initial speed of a particle ( + or - 25% random factor applyed, m/sec )
	float fSpeedFadeOut; // Time until the end of life during which the speed decreases from normal to 0
	float fSpeedAccel;	// Constant speed acceleration along particle heading.
	float fAirResistance; // Air resistance coefficient
	Vec3Variant vRotation; // Rotation speed (degree/sec)
	Vec3Variant vInitAngles; // Initial rotation
	int nCount; // Number of particles to spawn
	FloatVariant fSize; // Initial size of particles
	float fSizeSpeed; // Speed used to grow the particles
	float fSizeFadeIn; // Time in which at the begning of life time size goes from 0 to fSize.
	float fSizeFadeOut; // Time in which at the end of life time size goes from fSize to 0.
	float fThickness;	// Lying thickness - for physicalized particles only
	FloatVariant fLifeTime; // Time of life of particle
	float fFadeInTime; // Particles will fade in slowly during this time
	// Texture id for body and trail (if used) ( if 0 - will be used default ball/glow texture )
	INT_PTR nTexId; //## AMD Port
	int nTexAnimFramesCount; // Number of frames in animated texture ( set to 0 if there's no animation )
	ParticleBlendType eBlendType; // The blend parameters, set using the ParticleBlendType structure
	float fTailLenght; // Delay of tail ( 0 - no tail, 1 meter if speed is 1 meter/sec )
	int nTailSteps; // How many tail steps particle have (0 will result in the default 8 steps)
	float fStretch;  // Stretch particle into moving direction
	int nParticleFlags; // See EParticleFlags for the possible flags
	bool bRealPhysics; // Uses physics engine to control particles
	IStatObj * pStatObj; // If it isn't set to 0, this object will be used instead sprite
	ParticleParams * pChild; // Child process definition
	float fChildSpawnPeriod; // If higher than 0, it will run the child process every x seconds, but if it's set to 0, run it at collision
	float fChildSpawnTime; // If higher than 0, spawns child process for a maximum of this time
	int nDrawLast; // Add this element into the second list and draw this list last
	float fBouncenes; // If equal to 0, the particles will not bounce from the ground (0.5 is good in most cases)
	float fTurbulenceSize; // Radius of turbulence
	float fTurbulenceSpeed; // Speed of rotation
	float fDirVecScale; // Needed by the game
	struct IEntityRender * pEntity; // Spawner entity
	struct IShader * pShader; // Shader used for the particles
	float fPosRandomOffset; // Maximum distance of random offset from original position
	IMatInfo *pMaterial; // Used to override the material
	
	//DOC-IGNORE-BEGIN
	// Used internally.
	AnimTexInfo *pAnimTex;
	//DOC-IGNORE-END

	//////////////////////////////////////////////////////////////////////////
	// New parameters, used by Particle effects.
	//////////////////////////////////////////////////////////////////////////

	// Spawn Position offset from the effect spawn position
	Vec3 vPositionOffset;
	// Random offset of the particle relative position to the spawn position
	Vec3 vRandomPositionOffset;
	// Delay the actual spawn time by this value
	FloatVariant fSpawnDelay;
	// Life time of the emitter
	FloatVariant fEmitterLifeTime;
	// When using the emitter, this define the spawn time between between 2 particle bursts
	float fSpawnPeriod;

	// Global effect scale (0 is ignored)
	float fScale;
	// Object scale multiplied with fSize to give scale adjustment between object and texture,
	// 0 will not affect fSize
	float fObjectScale;

	Vec3 vNormal; // Lying normal, used for physicalized particles only
	int iPhysMat; // Material for physicalized particles
	Vec3 vGravity; // Gravity vector (wind)
	Vec3 vSpaceLoopBoxSize; // if PART_FLAG_SPACELOOP is set, this box will be used to loop particles in it
};

/*!
IParticleEffect interface.
This Object with this interface is created by CreateParticleEffect method of 3d engine.
*/

// Description:
// This interface is used by I3DEngine::CreateParticleEffect to control a particle effect
struct IParticleEffect : public _i_reference_target_t
{
	// Number of child processes of this effect.
	enum { NUM_PARTICLE_PROCESSES = 2 };

	// Description:
	//     Used by SetSoundParams to specify the sound effect associated with the particle effect.
	// Summary:
	//     Sound effect associated with particles effect
	struct SoundParams
	{
		const char *szSound; // Name of the sound file
		float volume; // The volume specified in the range of 1 to 100
		float minRadius; // The minimum radius
		float maxRadius; // The maximum radius
		bool bLoop; // Specify if the sound effect should be looped
		bool bOnEverySpawn; // If the sound effect should played every time a particle is spawned

		SoundParams()
		{
			szSound = "";
			volume = 0;
			minRadius = 1;
			maxRadius = 10;
			bLoop = true;
		}
	};

	/*
	//! Additional effect parameters.
	struct EffectParams
	{
		//! Multiplies spawn period by this ammount.
		float spawnPeriodMultiplier;

		EffectParams() { spawnPeriodMultiplier = 1; }
	};
	*/

	//////////////////////////////////////////////////////////////////////////
	// Spawn this effect.
	//////////////////////////////////////////////////////////////////////////

	// Summary:
	//     Spawn the particles
	virtual void Spawn( const Vec3 &pos,const Vec3 &dir,float fScale=1.0f ) = 0;

	//! Assign name to this particle effect.

	// Summary:
	//     Set a new name
	// Arguments:
	//     sName - A new name
	virtual void SetName( const char *sName ) = 0;

	//! Returns name of this particle effect.

	// Summary:
	//     Gets the name
	// Return Value:
	//     A null terminated string which hold the name.
	virtual const char* GetName() = 0;

	//! Enable or disable this effect.

	// Summary:
	//     Enable or disable the effect
	// Arguments:
	//     bEnabled - set to true to enable the effect or to false to disable it
	virtual void SetEnabled( bool bEnabled ) = 0;

	// Summary:
	//     Deternime if the effect is already enabled
	// Return Value:
	//     A boolean value which indicate the status of the effect; true if 
	//     enabled or false if disabled.
	virtual bool IsEnabled() const = 0;

	//! Return ParticleParams or specified process.
	//! @param process 0=Primary Process, 1=Child Process.//zdes bil maks

	// Summary:
	//     Gets the particle parameters
	// Arguments:
	//     process - Specify for which process to receives the parameters; 0 is
	//               the primary process and 1 is the the child process
	// Return Value:
	//     An object of the type ParticleParams which contains several parameters.
	virtual ParticleParams& GetParticleParams( int process ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Texture and geometry.
	//////////////////////////////////////////////////////////////////////////

	// Summary:
	//     Gets the particle texture filename
	// Arguments:
	//     process - Specify for which process; 0 is the primary process and 1 is 
	//               the the child process
	// Return Value:
	//     The filename of the texture.
	virtual const char* GetTexture( int process ) const  = 0;
	
	// Summary:
	//     Gets the object filename used
	// Arguments:
	//     process - Specify for which process; 0 is the primary process and 1 is 
	//               the the child process
	// Return Value:
	//     The filename of the object file.
	virtual const char* GetGeometry( int process ) const = 0;

	// Summary:
	//     Set a texture to be used
	// Arguments:
	//     process - Specify for which process; 0 is the primary process and 1 is 
	//               the the child process
	//     s       - filename of the texture
	virtual void SetTexture( int process,const char *s ) = 0;

	// Summary:
	//     Set a texture to be used
	// Arguments:
	//     process - Specify for which process; 0 is the primary process and 1 is 
	//               the the child process
	//     s       - filename of the object file (a cgf)
	virtual void SetGeometry( int process,const char *s ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Material for this effect.
	//////////////////////////////////////////////////////////////////////////

//mat: todo

	virtual void SetMaterial( int process,IMatInfo *pMaterial ) = 0;
	virtual IMatInfo* GetMaterial( int process ) const = 0;
	virtual void SetMaterialName( int process,const char *sMtlName ) = 0;
	virtual const char* GetMaterialName( int process ) const = 0;

	//////////////////////////////////////////////////////////////////////////
	// Sound parameters.
	//////////////////////////////////////////////////////////////////////////

	//! Set Sound parameters for this particle effect.

	// Summary:
	//     Set the sound parameters
	// Arguments:
	//     params - Sound parameters
	virtual void SetSoundParams( const SoundParams &params ) = 0;
	
	//! Get Sound parameters for this particle effect.

	// Summary:
	//     Gets the sound parameters
	// Arguments:
	//     params - Sound parameters
	virtual void GetSoundParams( SoundParams &params ) const = 0;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Spawn rate.
	//virtual EffectParams& GetEffectParams() = 0;

	//////////////////////////////////////////////////////////////////////////
	// Child particle systems.
	//////////////////////////////////////////////////////////////////////////

	//! Get number of sub Particles childs.

	// Summary:
	//     Gets the number of sub particles childs
	// Return Value:
	//     An integer representing the amount of sub particles childs
	virtual int GetChildCount() const = 0;
	
	//! Get sub Particles child by index.

	// Summary:
	//     Gets a specified particles child
	// Arguments:
	//     index - The index of a particle child
	// Return Value:
	//     A pointer to a IParticleEffect derived object.
	virtual IParticleEffect* GetChild( int index ) const = 0;
	
	//! Adds a new sub Particles.

	// Summary:
	//   Adds a child particle effect
	// Arguments:
	//   pEffect - A pointer the particle effect to add as child
	virtual void AddChild( IParticleEffect *pEffect ) = 0;

	//! Remove specific sub Particles

	// Summary:
	//   Removes a sub particle effect
	// Arguments:
	//   pEffect - A pointer to the child particle effect to be removed
	virtual void RemoveChild( IParticleEffect *pEffect ) = 0;

	//! Remove all sub Particles.

	// Summary:
	//   Removes all child particles
	virtual void ClearChilds() = 0;

	//! Insert sub particles in between other child particles.

	// Summary:
	//   Insert a child particle effect at a precise slot
	// Arguments:
	//   slot - An integer value which specify the desired slot
	//   pEffect - A pointer to the particle effect to insert
	virtual void InsertChild( int slot,IParticleEffect *pEffect ) = 0;
	
	//! Find slot where sub Particles stored.
	//! @retun slot index if Particles found, -1 if Particles not found.

	// Summary:
	//   Finds in which slot a child particle effect is stored
	// Arguments:
	//   pEffect - A pointer to the child particle effect
	// Return Value:
	//   An integer representing the slot number.
	virtual int FindChild( IParticleEffect *pEffect ) const = 0;
	
	//! Load particle effect resources

	// Summary:
	//   Load all resources needed for a particle effects
	// Arguments:
	//   bRecursive - Set to true to make sure that all child effects also load their resources
	virtual void LoadResources( bool bRecursive = true ) = 0;
};

//DOC-IGNORE-BEGIN
TYPEDEF_AUTOPTR(IParticleEffect);
//DOC-IGNORE-END

//! Particle emitter interface

// Description:
//     An IParticleEmitter should usually be creater by 
//     I3DEngine::CreateParticleEmitter. Deleting the emitter should be done 
//     using I3DEngine::DeleteParticleEmitter.
// Summary:
//     Interface to a particle effect emitter
struct IParticleEmitter : public _i_reference_target_t
{
	//! Set raw emitter particle parameters (Do not use if you use SetEffect).

	// Summary: Set different parameters concerning the particle emitter
	// NOTE: Do not use this function if you already call SetEffect.
	
	// Description:
	//     Will define the parameters used to spawn the particles from the emitter.
	// Note: 
	//     Never call this function if you already used SetEffect.
	// See Also:
	//     SetEffect
	// Arguments:
	//     params - The parameters used by the emitter
	// Summary:
	//     Set the parameters used by the particle emitter
	virtual void SetParams( const ParticleParams &params ) = 0;

	//! Set particle effect to spawn at this emitter (Do not use if you use SetParams).
	// NOTE: Do not use this function if you already call SetParams.

	// Description:
	//     Will define the effect used to spawn the particles from the emitter.
	// Note: 
	//     Never call this function if you already used SetParams.
	// See Also:
	//     SetParams
	// Arguments:
	//     pEffect - A pointer to an IParticleEffect object
	// Summary:
	//     Set the effects used by the particle emitter
	virtual void SetEffect( IParticleEffect *pEffect ) = 0;

	//! Get current particle params.

	// Summary:
	//     Gets the current emitter's params
	// See Also:
	//     SetParams
	// Return Value:
	//     A structure of the type ParticleParams which hold all the parameters of the particle emitter.
	virtual const ParticleParams& GetParams() const = 0;

	//! Set emitter position and direction and scale.

	// Summary:
	//     Set the position, direction and scale
	// Arguments:
	//     vPos   - A new position
	//     vDir   - A new direction
	//     fScale - a new scale value
	virtual void SetPos( const Vec3 &vPos,const Vec3 &vDir,float fScale ) = 0;
	
	//! Override spawn period (Call after SetParams or SetEffect).

	// Description:
	//     Will override the fSpawnPeriod value from in SetParams or SetEffect.
	// See Also:
	//     SetEffect, SetParams
	// Arguments:
	//     fSpawnPeriod - Number of seconds during which the emitter will spawn particles
	// Summary:
	//     Set the spawn period
	virtual void SetSpawnPeriod( float fSpawnPeriod ) = 0;

	//! Override emitter life time (Call after SetParams or SetEffect).
	//! @param fLifeTime Number of seconds emitter is active (negative values get clamped to 0).
	
	// Description:
	//     Will override the emitter life time value set in SetParams or SetEffect.
	// See Also:
	//     SetUnlimitedLife
	// Arguments:
	//     fLifeTime - Number of seconds to keep the emitter active
	// Summary:
	//     Set the active period
	virtual void SetLifeTime( const float fLifeTime ) = 0;

	//! Override emitter life time (Call after SetParams or SetEffect).

	// Description:
	//     Will override the emitter life time value set in SetParams or SetEffect.
	// See Also:
	//     SetLifeTime
	// Summary:
	//     Set the emitter life to always stay active
	virtual void SetUnlimitedLife() = 0;

	// Summary:
	//     Associates with a spawner entity
	virtual void SetEntity( IEntityRender *pEntity ) = 0;

	// Summary:
	//     Set a new material
	// Arguments:
	//     pMaterial - A pointer to a material object
	virtual void SetMaterial( IMatInfo *pMaterial ) = 0;
};

//DOC-IGNORE-BEGIN
TYPEDEF_AUTOPTR(IParticleEmitter);
//DOC-IGNORE-END

// Summary:
//   Used by ParticleParams to specify options for the particle effect.
enum EParticleFlags
{
	PART_FLAG_BILLBOARD                  =    0, // Usual particle
	PART_FLAG_HORIZONTAL                 =    1, // Flat horisontal rounds on the water
	PART_FLAG_UNDERWATER                 =    2, // Particle will be removed if go out from outdoor water
	PART_FLAG_LINEPARTICLE               =    4, // Draw billboarded line from vPosition to vPosition+vDirection
	PART_FLAG_SWAP_XY                    =    8, // Alternative order of rotation (zxy)
	PART_FLAG_SIZE_LINEAR                =   16, // Change size liner with time
	PART_FLAG_NO_OFFSET                  =   32, // Disable centering of static objects
	PART_FLAG_DRAW_NEAR                  =   64, // Render particle in near space (weapon)
	PART_FLAG_FOCUS_PLANE                =  128, // Focus will spread partices along normal plane instead of direction axis.
	PART_FLAG_NO_DRAW_UNDERWATER         =  256, // Particle will be not visible when it position is under water
	PART_FLAG_RIGIDBODY                  =  512, // If bRealPhysics set to true, physicalizing particles as rigid bodies
	PART_FLAG_SPACELOOP                  = 1024, // Lock paticles in box around emitter position, use ParticleParams::vSpaceLoopBoxSize to set box size
	PART_FLAG_SPACELIMIT                 = 2048, // Limit paticles by box around emitter position, use ParticleParams::vSpaceLoopBoxSize to set box size
	PART_FLAG_SPEED_IN_GRAVITY_DIRECTION = 4096, // Ignores normal passed to particle effect and always uses gravity direction for initial speed direction.
	PART_FLAG_BIND_POSITION_TO_EMITTER   = 0x2000, // Always keep particle position binded to position of the emitter (for rockets etc...)
	PART_FLAG_BIND_EMITTER_TO_CAMERA     = 0x4000, // Attach emitter to camera pos
	PART_FLAG_NO_INDOOR                  = 0x8000, // Kill particle if it enters indoor
};


//! Physics material enumerator, allows for 3dengine to get material id from game code
struct IPhysMaterialEnumerator
{
  virtual int EnumPhysMaterial(const char * szPhysMatName) = 0;
	virtual bool IsCollidable(int nMatId) = 0;
	virtual int	GetMaterialCount() = 0;
	virtual const char* GetMaterialNameByIndex( int index ) = 0;
};

//! flags for DrawLowDetail

// Summary:
//     Flags used by I3DEngine::DrawLowDetail
enum EDrawLowDetailFlags
{
	DLD_DETAIL_OBJECTS            =    1,
	DLD_DETAIL_TEXTURES           =    2,
	DLD_TERRAIN_WATER             =    4,
	DLD_FAR_SPRITES               =    8,
	DLD_STATIC_OBJECTS            =   16,
	DLD_PARTICLES                 =   32,
	DLD_TERRAIN_FULLRES           =   64,
	DLD_TERRAIN_LIGHT             =  128,
	DLD_FIRST_PERSON_CAMERA_OWNER =  256,
	DLD_SHADOW_MAPS               =  512,
	DLD_STENCIL_SHADOWS           = 1024,
	DLD_ENTITIES                  = 2048,
	DLD_TERRAIN                   = 4096,
	DLD_ALLOW_WIDE_SCREEN         = 8192,
};

// phys foreign data flags
#define PFF_HIDABLE 1
#define PFF_EXCLUDE_FROM_STATIC 2

// duplicated definition to avoid including irenderer
#define STRIPTYPE_DEFAULT					4

//! contains current state of oclusion test
/*struct OcclusionTestClient
{
  OcclusionTestClient() { memset(this,0,sizeof(OcclusionTestClient)); bLastResult = true; }
  unsigned char ucOcclusionByTerrainFrames;
  unsigned char ucOcclusionByObjectsFrames;
  bool  bLastResult;
  int   nLastVisibleFrameID;
//	class CREOcclusionQuery * arrREOcclusionQuery[2];
};*/

//! structure to pass statobj group properites
struct IStatInstGroup
{
	IStatInstGroup() 
	{ 
		pStatObj = 0;
		bHideability = 0;
		bPhysNonColl = 0;
		fBending = 0;
		bCastShadow = 0;
		bRecvShadow = 0;
		bPrecShadow = true;
		bUseAlphaBlending = 0;
//		bTakeBrightnessFromLightBit = 0;
		fSpriteDistRatio = 1.f;
		fShadowDistRatio = 1.f;
		fMaxViewDistRatio= 1.f;
		fBrightness = 1.f;
		bUpdateShadowEveryFrame = 0;
//		fAmbScale = 1.f;
		nSpriteTexRes = 0;
		pMaterial = 0;
    fBackSideLevel = 1.f;
    bCalcLighting = true;
    bUseSprites = true;
		bFadeSize = true;
	}

	struct IStatObj * pStatObj;
	bool	bHideability;
	bool	bPhysNonColl;
	float fBending;
	bool	bCastShadow;
	bool	bRecvShadow;
	bool	bPrecShadow;
	bool	bUseAlphaBlending;
//	bool	bTakeBrightnessFromLightBit;
	float fSpriteDistRatio;
	float fShadowDistRatio;
	float fMaxViewDistRatio;
	float	fBrightness;
	bool  bUpdateShadowEveryFrame;
//	float fAmbScale;
	int		nSpriteTexRes;
  float fBackSideLevel;
  bool  bCalcLighting;
  bool  bUseSprites;
	bool  bFadeSize;

	//! Override material for this instance group.
	IMatInfo *pMaterial;

	//! flags similar to entity render flags
	int m_dwRndFlags;
};

//! Interface for water volumes editing from editor


// Description:
//     Water volumes should usually be created by I3DEngine::CreateWaterVolume.
// Summary:
//     Interface to water volumes
struct IWaterVolume
{
//DOC-IGNORE-BEGIN
	virtual void UpdatePoints(const Vec3 * pPoints, int nCount, float fHeight) = 0;
	virtual void SetFlowSpeed(float fSpeed) = 0;
	virtual void SetAffectToVolFog(bool bAffectToVolFog) = 0;
	virtual void SetTriSizeLimits(float fTriMinSize, float fTriMaxSize) = 0;
	virtual void SetShader(const char * szShaderName) = 0;
	virtual void SetMaterial( IMatInfo *pMaterial ) = 0;
	virtual IMatInfo * GetMaterial() = 0;
	virtual void SetName(const char * szName) = 0;
//DOC-IGNORE-END

	// Description:
	//     Used to change the water level. Will assign a new Z value to all 
	//     vertices of the water geometry.
	// Arguments:
	//     vNewOffset - Position of the new water level
	// Summary:
	//     Set a new water level
	virtual void SetPositionOffset(const Vec3d & vNewOffset) = 0;
};

// Summary:
//     Provide information about the different VisArea volumes
struct IVisArea
{
	// Summary:
	//     Gets the last rendered frame id
	// Return Value:
	//     An int which contrain the frame id.
	virtual int GetVisFrameId() = 0;
	
	// Description:
	//     Gets a list of all the VisAreas which are connected to the current one. 
	// Arguments:
	//     pAreas               - Pointer to an array of IVisArea*
	//     nMaxConnNum          - The maximum of IVisArea to write in pAreas
	//     bSkipDisabledPortals - Ignore portals which are disabled
	// Return Value:
	//     An integer which hold the amount of VisArea found to be connected. If 
	//     the return is equal to nMaxConnNum, it's possible that not all 
	//     connected VisAreas were returned due to the restriction imposed by the 
	//     argument.
	// Summary:
	//     Gets all the areas which are connected to the current one
	virtual	int GetVisAreaConnections(IVisArea ** pAreas, int nMaxConnNum, bool bSkipDisabledPortals = false) = 0;

	// Summary:
	//     Determine if it's connected to an outdoor area
	// Return Value:
	//     Return true if the VisArea is connected to an outdoor area.
	virtual bool IsConnectedToOutdoor() = 0;

	// Summary:
	//     Gets the name
	// Note:
	//     The name is always returned in lower case.
	// Return Value:
	//     A null terminated char array containing the name of the VisArea.
	virtual const char * GetName() = 0;

	// Summary:
	//     Determine if this VisArea is a portal
	// Return Value:
	//     true if the VisArea is a portal, or false in the opposite case.
	virtual bool IsPortal() = 0;

	// Description: 
	//     Search for a specified VisArea to see if it's connected to the current 
	//     VisArea.
	// Arguments:
	//     pAnotherArea         - A specified VisArea to find
	//     nMaxRecursion        - The maximum number of recursion to do while searching
	//     bSkipDisabledPortals - Will avoid searching disabled VisAreas
	// Return Value:
	//     true if the VisArea was found.
	// Summary:
	//     Search for a specified VisArea
	virtual bool FindVisArea(IVisArea * pAnotherArea, int nMaxRecursion, bool bSkipDisabledPortals) = 0;

	// Summary:
	//     Determine if it's affected by outdoor lighting
	// Return Value:
	//     Return true if the VisArea if it's affected by outdoor lighting, else
	//     false will be returned.
	virtual bool IsAfectedByOutLights() = 0;
};


enum EVertsSharing
{
  evs_NoSharing=0,
  evs_ShareAndSortForCache=1,
};

// water level unknown
#define WATER_LEVEL_UNKNOWN -1000000

//! Interface to the 3d engine dll

// Summary:
//     Interface to the 3d Engine
struct I3DEngine : public IProcess
{
	//! Enable/Disable the 3d engine update and rendering process

	// Summary:
	//     Enable or disable the 3D Engine
	// Arguments:
	//     bEnable - true indicate the Engine should be enabled, while the 
	//               false would disable it
	virtual void Enable(bool bEnable) = 0;

	//! Initialize 3dengine (call once, after creations)

	// Summary:
	//     Initialize the 3D Engine
	// See Also:
	//     ShutDown
	// Note:
	//     Only call once, after creating the instance.
	virtual bool Init() = 0;

	// Summary:
	//     Set the path used to load levels
	// See Also:
	//     LoadLevel
	// Arguments:
	//     szFolderName - Should contains the folder to be used
	virtual void SetLevelPath( const char * szFolderName ) = 0;

	// Description:
	//     Will load a level from the folder specified with SetLevelPath. If a 
	//     level is already loaded, the resources will be deleted before.
	// See Also:
	//     SetLevelPath
	// Arguments:
	//     szFolderName - Name of the subfolder to load
	//     szMissionName - Name of the mission
	//     bEditorMode - If called from the editor
	// Return Value:
	//     A boolean which indicate the result of the function; true is 
	//     succeed, or false if failled.
	// Summary:
	//     Load a level
	virtual bool LoadLevel(const char * szFolderName, const char * szMissionName, bool bEditorMode = false) = 0;

	// Summary:
	//     Update the 3D Engine 
	// Note:
	//     Should be called for every frame.
	virtual void Update() = 0;

	// Summary:
	//     Set the camera
	// See Also:
	//     Draw
	// Note:
	//     Must be called before Draw.
	// Arguments:
	//     cam - ...
	//     bToTheScreen - ...
	virtual void SetCamera(const CCamera &cam, bool bToTheScreen=true) = 0;

	// Summary:
	//     Draw the world
	// See Also:
	//     SetCamera
	virtual void Draw() = 0;

	// Summary:
	//     Draw the world for rendering into a texture
	// Arguments:
	//     DrawFlags - Define what to draw, use any flags defined in EDrawLowDetailFlags
	virtual void DrawLowDetail(const int & DrawFlags) = 0;

	// Summary:
	//     Shutdown the 3D Engine
	// Arguemnts:
	//     bEditorMode - Indicate if the 3D Engine was used by the editor
	virtual void ShutDown(bool bEditorMode=false) = 0;

	// Summary:
	//     Delete the 3D Engine instance
	virtual void Release() = 0;

//DOC-IGNORE-BEGIN
	//! Activates lighting for indoors (for debug only)
	virtual void ActivateLight(const char *szName,bool bActivate)=0;
//DOC-IGNORE-END

	/*! Load cgf file and create non animated object.
      Returns pointer to already loaded object with same name if found.
      Reference counting used */

	// Summary:
	//     Loads a static object from a cgf file
	// See Also:
	//     IStatObj
	// Arguments:
	//     szFileName -
	//     szGeomName -
	//     eVertsSharing -
	//     bLoadAdditionalInfo -
	// Return Value:
	//     A pointer to an object derived from IStatObj.
	virtual IStatObj * MakeObject(const char * szFileName, const char * szGeomName = 0, 
		EVertsSharing eVertsSharing = evs_ShareAndSortForCache, 
		bool bLoadAdditinalInfo = true,    
		bool bKeepInLocalSpace = false) = 0;

	// Description:
	//     Will reduce the reference count of the static object. If this count 
	//     result in zero, the object will be deleted from memory.
	// Arguments:
	//     A pointer to a static object
	// Return Value:
	//     The value true is return if the object was valid, else false is returned.
	// Summary:
	//     Release the static object
	virtual bool ReleaseObject(IStatObj * pObject)=0;

	// Summary:
	//     Creates an empty static object
	// Return Value:
	//     A pointer to a static object.
	virtual IStatObj* MakeObject() = 0;

	// Summary:
	//     Gets the amount of loaded objects
	// Return Value:
	//     An integer representing the amount of loaded objects.
	virtual int GetLoadedObjectCount() { return 0; }

	// Summary:
	//     Registers an entity to be rendered
	// Arguments:
	//     pEntity - The entity to render
	virtual void RegisterEntity( IEntityRender * pEntity )=0;

	// Summary:
	//     Notices the 3D Engine to stop rendering a specified entity
	// Arguments:
	//     pEntity - The entity to stop render
	virtual bool UnRegisterEntity( IEntityRender * pEntity )=0;

//DOC-IGNORE-BEGIN
	/*! Unregister all entities in all sectors (or only one if specified)
		Returns true if specified entity was found */
	virtual bool UnRegisterInAllSectors(IEntityRender * pEntity = NULL) = 0;
//DOC-IGNORE-END

	/*! Get water level in specified point (taking into account global water level and water volumes)
			Function returns WATER_LEVEL_UNKNOWN if in specified position water was not found */

	// Summary:
	//     Gets the water level for a specified position
	// Note:
	//     This function will take into account both the global water level and any water volume present.
	// Arguments:
	//     pvPos - Desired position to inspect the water level
	//     pvFlowDir - Pointer to return the flow direction (optional)
	// Return Value:
	//     A float value which indicate the water level. In case no water was 
	//     found at the specified location, the value WATER_LEVEL_UNKNOWN will 
	//     be returned.
	virtual float GetWaterLevel(const Vec3 * pvPos = NULL, Vec3 * pvFlowDir = NULL) = 0;

	/*! Get water level in position of specified object taking into account global water level 
      and water volumes. For indoor objects global water level is ignored.
			Function returns WATER_LEVEL_UNKNOWN if in specified position water was not found */

	// Summary:
	//     Gets the water level at the location of a specified entity
	// Note:
	//     This function will take into account both the global water level and any water volume present.
	// Arguments:
	//     pvPos - Desired position to inspect the water level
	//     pvFlowDir - Pointer to return the flow direction (optional)
	// Return Value:
	//     A float value which indicate the water level. In case no water was 
	//     found at the location, the value WATER_LEVEL_UNKNOWN will be returned.
	virtual float GetWaterLevel(IEntityRender * pEntityRender, Vec3 * pvFlowDir = NULL) = 0;

	// Summary:
	//     Spawns particles using information from a ParticleParams struture
	// Arguments:
	//     SpawnParticleParams - Information on how to spawn the particles
	virtual void SpawnParticles( const ParticleParams & SpawnParticleParams ) = 0;

	// Summary:
	//     Removes all particles and decals from the world
	virtual void ResetParticlesAndDecals( ) = 0;

	// Summary:
	//     Creates a new particle emitter
	// Return Value:
	//     A pointer to an object derived from IParticleEmitter
	virtual IParticleEmitter* CreateParticleEmitter() = 0;

	// Summary:
	//     Deletes a specified particle emitter
	// Arguments:
	//     pPartEmitter - Specify the emitter to delete
	virtual void DeleteParticleEmitter(IParticleEmitter * pPartEmitter) = 0;

	//////////////////////////////////////////////////////////////////////////
	// ParticleEffects
	//////////////////////////////////////////////////////////////////////////

	// Summary:
	//     Create a new particle effect object
	// Return Value:
	//     A pointer to a object derived from IParticleEffect.
	virtual IParticleEffect* CreateParticleEffect() = 0;

	// Summary:
	//     Deletes a specified particle effect
	// Arguments:
	//     pEffect - A pointer to the particle effect object to delete
	virtual void DeleteParticleEffect( IParticleEffect* pEffect ) = 0;

	// Summary:
	//     Searches by name one the particle effect
	// Arguments:
	//     sEffectName - The name of the particle effect to search
	// Return Value:
	//     A pointer to a particle effect object matching the specified name. In 
	//     case no effect has been found, the value NULL will be returned.
	virtual IParticleEffect* FindParticleEffect( const char *sEffectName ) = 0;
	
	//////////////////////////////////////////////////////////////////////////

	// Summary:
	//     Creates new decals on the walls, static objects, terrain and entities
	// Arguments:
	//     Decal - Structure describing the decal effect to be applied
	virtual void CreateDecal( const CryEngineDecalInfo & Decal )=0;

	// Summary:
	//     Removes decals in a specified range
	// Arguments:
	//     vBoxMin - Specify the range in which the decals will be removed
	//     vBoxMax - Specify the range in which the decals will be removed
	//     bDeleteBigTerrainDecals - Not used
	virtual void DeleteDecalsInRange( Vec3d vBoxMin, Vec3d vBoxMax, bool bDeleteBigTerrainDecals = true)=0;

	//! Give access to shore geoemtry for AI

	// Summary:
	//     Gets access to the shore geometry
	virtual const void * GetShoreGeometry(int & nPosStride, int & nVertCount, int nSectorX, int nSectorY)=0;

//DOC-IGNORE-BEGIN
  /*! Call back for renderer. 
      Renders detail textures on terrain. Will be removed from here.*/
	virtual void DrawTerrainDetailTextureLayers() = 0; // used by renderer
  /*! Call back for renderer.
      Renders far trees/object as sprites. Will be removed from here.*/
	virtual void DrawFarTrees() = 0; // used by renderer
  /*! Call back for renderer.
      Renders decals, particles, bflyes. Need to remove from here.*/
	virtual void DrawTerrainParticles(struct IShader * pShader) = 0;// used by renderer  
  
  /*! Set render call back to make possible to 
    render user defined objects from outside of 3dengine (used by editor)*/
	virtual void SetRenderCallback(void (*pFunc)(void *pParams), void *pParams) = 0;
//DOC-IGNORE-END

	/*! Load cgf and caf files and creates animated object.
		Returns pointer to already loaded object with same name if found.
		Reference counting used */

    // Summary:
	//     Create a new character instance
	// See Also:
	//     ICryCharManager::MakeCharacter
	// Note:
	//     This function simply call ICryCharManager::MakeCharacter
	virtual ICryCharInstance * MakeCharacter(const char * cid_file_name, unsigned int dwFlags = 0)=0;

	// Summary:
	//     Scan the file and determine if it's a valid animated object
	// Arguments:
	//     szFileName - Filename of the object
	// Return Value:
	//     A boolean value; true if the file is indeed an animated object, or false if it isn't.
	virtual bool IsCharacterFile (const char* szFileName) = 0;

	// Summary:
	//! Reduces reference counter for object and deletes object if counter is 0

	// Summary:
	//     Release a specified character instance
	// See Also:
	//     ICryCharManager::RemoveCharacter
	// Note:
	//     This function simply call ICryCharManager::RemoveCharacter
	virtual void RemoveCharacter(ICryCharInstance * pCryCharInstance)=0;  

	// Summary:
	//     Gets the current world color 
	virtual Vec3 GetWorldColor(bool bScaled=true)=0; // for example red at evening
  
	// Summary:
	//     Set the world color 
	virtual void SetWorldColor(Vec3 vColor)=0;

	// Summary:
	//     Set the current outdoor ambient color 
	virtual void SetOutdoorAmbientColor(Vec3d vColor)=0;
	
	// Summary:
	//     Set the world color ratio
	virtual void SetWorldColorRatio(float fWorldColorRatio) = 0;
	
	// Summary:
	//     Gets world color ratio
	virtual float GetWorldColorRatio() = 0;

	// Summary:
	//     Set to a new sky box
	// Arguments:
	//     szShaderName - Name of the shader used for the sky box
	virtual void SetSkyBox(const char * szShaderName) = 0;

	// Summary:
	//     Set the view distance
	// Arguments:
	//     fMaxViewDistance - Maximum view distance
	virtual void SetMaxViewDistance(float fMaxViewDistance)=0;

	// Summary:
	//     Gets the view distance
	// Return Value:
	//     A float value representing the maximum view distance.
	virtual float GetMaxViewDistance()=0;
  
	//! Set/Get fog params

	// Summary:
	//     Set the fog color
	virtual void SetFogColor(const Vec3& vFogColor)=0;

	// Summary:
	//   Set the intensity of fog at a close distance
	// See Also:
	//   SetFogEnd
	// Arguments:
	//   fFogStart - Intensity at a close distance, by default 50
	virtual void SetFogStart(const float fFogStart)=0;

	// Summary:
	//   Set the intensity of fog at a far distance
	// See Also:
	//   SetFogStart
	// Arguments:
	//   fFogEnd - Intensity at a close distance, by default 1500
	virtual void SetFogEnd(const float fFogEnd)=0;

	// Summary:
	//     Gets the fog color
	virtual Vec3 GetFogColor( )=0;
	
	// Summary:
	//   Get the intensity of fog at a close distance
	// See Also:
	//   GetFogEnd
	virtual float GetFogStart( )=0;

	// Summary:
	//   Set the intensity of fog at a far distance
	// See Also:
	//   GetFogStart
	virtual float GetFogEnd( )=0;

	// Summary:
	//     Gets the interpolated terrain elevation for a specified location
	// Note:
	//     All x,y values are valid
	// Arguments:
	//     x - X coordinate of the location
	//     y - Y coordinate of the location
	// Return Value:
	//     A float which indicate the elevation level.
	virtual float GetTerrainElevation(float x, float y) = 0;

	// Summary:
	//     Gets the terrain elevation for a specified location
	// Note:
	//     Only values between 0 and WORLD_SIZE.
	// Arguments:
	//     x - X coordinate of the location
	//     y - Y coordinate of the location
	// Return Value:
	//     A float which indicate the elevation level.
	virtual float GetTerrainZ(int x, int y) = 0;

	// Summary:
	//     Gets the unit size of the terrain
	// Note:
	//     The value should currently be 2.
	// Return Value:
	//     A int value representing the terrain unit size in meters.
	virtual int   GetHeightMapUnitSize() = 0;

	//! Returns size of terrain in meters ( currently is 2048 )

	// Summary:
	//     Gets the size of the terrain
	// Note:
	//     The value should be 2048 by default.
	// Return Value:
	//     An int representing the terrain size in meters.
	virtual int   GetTerrainSize()=0;


	//! Returns size of terrain sector in meters ( currently is 64 )

	// Summary:
	//     Gets the size of the terrain sectors
	// Note:
	//     The value should be 64 by default.
	// Return Value:
	//     An int representing the size of a sector in meters.
	virtual int   GetTerrainSectorSize()=0;

	//! Maximum view distance in meters ( usualy 512 )

	// Summary:
	//     Gets the maximum view distance
	// Note:
	//     The value should be 512 by default.
	// Return Value:
	//     A float which represent the view distance in meters
	virtual float GetMaxViewDist()=0;

//DOC-IGNORE-BEGIN

	// Internal functions, mostly used by the editor, which won't be documented for now

	//! Places object at specified position (for editor)
	virtual bool AddStaticObject(int nObjectID, const Vec3 & vPos, const float fScale, unsigned char ucBright=255) = 0;
	//! Removes static object from specified position (for editor)
	virtual bool RemoveStaticObject(int nObjectID, const Vec3 & vPos) = 0;
	//! On-demand physicalization of a static object
	virtual bool PhysicalizeStaticObject(void *pForeignData,int iForeignData,int iForeignFlags) = 0;
	//! Removes all static objects on the map (for editor)
	virtual void RemoveAllStaticObjects() = 0;
	//! Allows to set terrain surface type id for specified point in the map (for editor)
	virtual void SetTerrainSurfaceType(int x, int y, int nType)=0; // from 0 to 6 - sur type ( 7 = hole )
	/*! Return terrain surface type id for specified point on the map 
		Return -1 if point is outside of the map or if there is hole in terrain here ) */
    virtual int  GetTerrainSurfaceType(int x, int y)=0; // from 0 to 6 - sur type ( 7 = hole )
  
	//! Updates part of hight map (in terrain units, by default update only elevation)
	virtual void SetTerainHightMapBlock(int x1, int y1, int nSizeX, int nSizeY, unsigned short * TerrainBlock, unsigned short nUpdateMask = (((unsigned short)-1) & (~31))) = 0;
	//! Returns true if game modified terrain hight map since last update by editor
	virtual bool IsTerainHightMapModifiedByGame() = 0;
  
	//! returns terrain sector texture id, texture dimensions and disable streaming on this sector
	//! returns 0 in case of error (wrong SectorOrigin)
	virtual int LockTerrainSectorTexture(int nSectorOriginX, int nSectorOriginY, int & nTexDim) = 0;

	//! Set group parameters
	virtual bool SetStatInstGroup(int nGroupId, const IStatInstGroup & siGroup) = 0;

	//! Get group parameters
	virtual bool GetStatInstGroup(int nGroupId,       IStatInstGroup & siGroup) = 0;

	//! returns dimensions of entire terrain texture
	virtual int GetTerrainTextureDim() = 0;

	//! Set burbed out flag
	virtual void SetTerrainBurnedOut(int x, int y, bool bBurnedOut) = 0;
	//! Get burbed out flag
	virtual bool IsTerrainBurnedOut(int x, int y) = 0;

	//! recalculate shore geometry, save it to disk and reload into engine
	virtual void RecompileBeaches() = 0;

//DOC-IGNORE-END

	// Summary:
	//   Notifies of an explosion, and maybe creates an hole in the terrain
	// Description:
	//   This function should usually make sure that no static objects are near before making the hole.
	// Arguments:
	//   vPos - Position of the explosion
	//   vHitDir - Direction of the explosion
	//   fRadius - Radius of the explosion
	//   nTexID - Texture ID
	//   bDeformTerrain - Allow to deform the terrain
	virtual void OnExplosion(Vec3 vPos, Vec3 vHitDir, float fRadius, int nTexID, bool bDeformTerrain = true) = 0;

//DOC-IGNORE-BEGIN
    // Not used anymore
	//! Makes 3d waves on the water surface
	virtual void AddWaterSplash (Vec3 vPos, enum eSplashType eST, float fForce, int Id=-1) = 0;
//DOC-IGNORE-END

	//! Force to draw quad on entire screen with specified shader (night vision) ( use szShaderName="" to disable drawing )

	// Summary:
	//   Draws a quad on the entire screen using a specified shader
	// Note:
	//   Used for special night vision effect.
	// Arguments:
	//   szShaderName - Name of the shader to use
	virtual void SetScreenShader( const char * szShaderName ) = 0;

	//! Set physics material enumerator

	// Summary:
	//   Set the physics material enumerator
	// Arguments:
	//   pPhysMaterialEnumerator - The physics material enumarator to set
	virtual void SetPhysMaterialEnumerator(IPhysMaterialEnumerator * pPhysMaterialEnumerator) = 0;

	// Summary:
	//   Gets the physics material enumerator
	// Return Value:
	//   A pointer to an IPhysMaterialEnumerator derived object.
	virtual IPhysMaterialEnumerator * GetPhysMaterialEnumerator() = 0;

//DOC-IGNORE-BEGIN
//Internal functions
	
	//! Allows to enable fog in editor
	virtual void SetupDistanceFog() = 0;

	//! Load environment settings for specified mission
	virtual void LoadEnvironmentSettingsFromXML(const char * szMissionName, bool bEditorMode, const char * szMissionXMLString = 0, bool bUpdateLightingOnVegetations = true) = 0;

	//! Load detail texture and detail object settings from XML doc (load from current LevelData.xml if pDoc is 0)
	virtual void	LoadTerrainSurfacesFromXML(void * pDoc = NULL) = 0;

//Unused

	//! Recalculate shore geometry
	virtual void UpdateBeaches() = 0;
//DOC-IGNORE-END

	//! Returns true if point is in water

	// Summary:
	//   Determines if a specified position is in the water
	// Arguments:
	//   vPos - The position to evaluate
	// Return Value:
	//   A boolean with the value true if the position is in the water, else false is returned.
	virtual bool IsPointInWater(Vec3 vPos) = 0;

	//! Returns true if point is in the building
	//virtual bool IsPointInsideIndoors(const Vec3 & vPos) = 0;

	//! Creates new light source in the world to be used during this frame (or longer)

	// Summary:
	//   Creates a new dynamic light source
	// Description:
	//   The new light source will be used during this frame, and maybe longer.
	// Arguments:
	//   LSource - ...
	//   pEnt - ...
	//   nEntityLightId - ...
	//   pMatrix - ...
	virtual void AddDynamicLightSource(const class CDLight & LSource, IEntityRender * pEnt, int nEntityLightId=-1, const Matrix44* pMatrix=NULL) = 0;

	//! Make move/bend vegetations in specified area (not implemented yet)

	// Description:
	//   Physics applied to the area will apply to vegetations and allow it to move/blend.
	// Arguments:
	//   vPos - Center position to apply physics
	//   fRadius - Radius which specify the size of the area to apply physics
	//   fAmountOfForce - The amount of force, should be at least of 1.0f
	// Summary:
	//   Applies physics in a specified area
	virtual void ApplyForceToEnvironment(Vec3 vPos, float fRadius, float fAmountOfForce) = 0;

//DOC-IGNORE-BEGIN
// Internal function used by the 3d engine and renderer
	//! Return sun position (if bMoveUp=true sun will be 30% higher, it makes shadows from objects not so long)
	virtual Vec3 GetSunPosition(bool bMoveUp = true) = 0;
 
// Internal function used by the 3d engine and editor
	//! Returns light mask for this point (valid only during rendering stage)
	virtual unsigned int GetLightMaskFromPosition(const Vec3 & vPos, float fRadius=1.f) = 0;

// Internal function used by the 3d engine
	//! Returns lighting level for this point
	virtual Vec3 GetAmbientColorFromPosition(const Vec3 & vPos, float fRadius=1.f) = 0;

// Never used
	//! Returns fog volume id
	virtual int GetFogVolumeIdFromBBox(const Vec3 & vBoxMin, const Vec3 & vBoxMax) = 0;

	//! Return surface normal at specified position
	//virtual Vec3 GetTerrainSurfaceNormal(Vec3 vPos) = 0;

//Internal function used by 3d engine and editor

	//! Render shadows of objects into frame buffer and read this picture
	virtual bool MakeSectorLightMap(int nSectorOriginX, int nSectorOriginY, unsigned char * pImage, int nImageSize) = 0;

//Internal function used by 3d engine and renderer
	//! get distance to the sector containig ocean water
	virtual float GetDistanceToSectorWithWater() = 0;
//DOC-IGNORE-END

	//! allows to slowly replace skybox with bg(fog) color

	// Summary:
	//   Set an alpha value to apply to the sky box
	// Arguments:
	//   fAlpha - A value between 0.0f to 1.0f
	virtual void SetSkyBoxAlpha(float fAlpha /* from 0 to 1 */) = 0;

//DOC-IGNORE-BEGIN
//Not supported anymore
	//! set/get current number of butterflies and other live particles
	virtual void SetBFCount(int nCount/* from 0 to MAX_BF_COUNT */) = 0;
	virtual int  GetBFCount() = 0;
	virtual void SetGrasshopperCount(int nCount/* from 0 to MAX_BF_COUNT */) = 0;
	virtual int  GetGrasshopperCount() = 0;
	virtual void SetGrasshopperCGF( int nSlot, IStatObj * pStatObj ) = 0;
//DOC-IGNORE-END

	//! get environment ambient color specified in editor

	// Summary:
	//   Gets the environment ambient color
	// Note:
	//   Should have been specified in the editor.
	// Return Value:
	//   An rgb value contained in a Vec3 object.
	virtual Vec3 GetOutdoorAmbientColor() = 0;

	//! get environment sun color specified in editor

	// Summary:
	//   Gets the sun color
	// Note:
	//   Should have been specified in the editor.
	// Return Value:
	//   An rgb value contained in a Vec3 object.
	virtual Vec3 GetSunColor() = 0;

	//! check object visibility taking into account portals and terrain occlusion test
	//  virtual bool IsBoxVisibleOnTheScreen(const Vec3 & vBoxMin, const Vec3 & vBoxMax, OcclusionTestClient * pOcclusionTestClient = NULL)=0;
	//! check object visibility taking into account portals and terrain occlusion test
	//  virtual bool IsSphereVisibleOnTheScreen(const Vec3 & vPos, const float fRadius, OcclusionTestClient * pOcclusionTestClient = NULL)=0;

	//! Clears all rendering resources, should be called before LoadLevel() and before loading of any textures from script

	// Summary:
	//   Clears all rendering resources
	// Note:
	//   Should always be called before LoadLevel, and also before loading textures from a script.
	// Arguments:
	//   bEditor - Set to true if called from the editor
	virtual void ClearRenderResources(bool bEditor=false)=0;

//mat: todo

	//! Alloc entity render info
	virtual struct IEntityRenderState * MakeEntityRenderState() = 0;

	//! Free entity render info
	virtual void FreeEntityRenderState(IEntityRender * pEntity) = 0;

	//! Return pointer to full file name of file in current level folder

	// Summary:
	//   Add the level's path to a specified filename
	// Arguments:
	//   szFileName - The filename for which we need to add the path
	// Return Value:
	//   Full path for the filename; including the level path and the filename appended after.
	virtual const char * GetLevelFilePath(const char * szFileName) = 0;

//DOC-IGNORE-BEGIN
//Internal function for the 3d Engine and Renderers
	//! make smoothed terrain for ocean calculations
	virtual void MakeUnderWaterSmoothHMap(int nWaterUnitSize) = 0;

	//! returns ptr to smoothed terrain for ocean calculations
	virtual unsigned short * GetUnderWaterSmoothHMap(int & nDimensions) = 0;

//Never used
	//! refresh detail objects
	virtual void UpdateDetailObjects() = 0;
//DOC-IGNORE-END

	// Summary:
	//   Displays statistic on the 3d Engine
	// Arguments:
	//   fTextPosX - X position for the text
	//   fTextPosY - Y position for the text
	//   fTextStepY - Amount of pixels to distance each line
	virtual void DisplayInfo(float & fTextPosX, float & fTextPosY, float & fTextStepY) = 0;

	// Summary:
	//   Used for precalculation in ShadowVolumes
	//## added by M.M. (don't copy the interface pointer and don't forget to call Release)
	virtual class IEdgeConnectivityBuilder *GetNewConnectivityBuilder( void )=0;

	//! Creates a connectivity object that can be used to deserialize the connectivity data
	virtual class IStencilShadowConnectivity *NewConnectivity() = 0;

	//! Returns the connectivity builder that's building the connectivity for static objects
	virtual class IEdgeConnectivityBuilder *GetNewStaticConnectivityBuilder (void) = 0;

	// Summary:
	//   Used for precalculation in ShadowVolumes
	//## added by M.M. (don't delete or Release the interface pointer)
	virtual class IEdgeDetector *GetEdgeDetector( void )=0;

	// Summary:
	//   Enable or disable heat vision
	// Arguments:
	//   bEnable - Set to true in order to enable heat vision, or to false to disable it.
	virtual void EnableHeatVision(bool bEnable) = 0;

	//! Enable/Disable portal at specified position

	// Summary:
	//   Enable or disable portal at a specified position
	// Arguments:
	//   vPos - Position to place the portal
	//   bActivate - Set to true in order to enable the portal, or to false to disable
	//   pEntity - A pointer to the entity holding the portal
	virtual void ActivatePortal(const Vec3 &vPos, bool bActivate, IEntityRender *pEntity) = 0;

//DOC-IGNORE-BEGIN
	//! Count memory usage
	virtual void GetMemoryUsage(class ICrySizer * pSizer)=0;
//DOC-IGNORE-END

	//! Create water volume

	// Summary:
	//   Creates a new water volume
	// See Also:
  //   DeleteWaterVolume
	// Return Value:
	//   A newly allocated object derived from IWaterVolume.
	virtual IWaterVolume * CreateWaterVolume() = 0;

	//! Delete water volume

	// Summary:
	//   Deletes a water volume
	// Arguments:
	//   pWaterVolume - A pointer to the water volume
	virtual void DeleteWaterVolume(IWaterVolume * pWaterVolume) = 0;

	// Summary:
	//   Finds a water volume by its name
	// Arguments:
	//   szName - Name to find
	// Return Value:
	//   A pointer to the water volume matching the name, or 0 if the search was unsuccessful.
	virtual IWaterVolume * FindWaterVolumeByName(const char * szName) = 0;

	//! Create visarea

	// Summary:
	//   Creates a new VisArea
	// Return Value:
	//   A pointer to a newly created VisArea object
	virtual IVisArea * CreateVisArea() = 0;

	//! Delete visarea

	// Summary:
	//   Deletes a VisArea
	// Arguments:
	//   pVisArea - A pointer to the VisArea to delete
	virtual void DeleteVisArea(IVisArea * pVisArea) = 0;

	//! Update visarea

//mat: todo

	// Summary:
	//   Updates the VisArea
	// Arguments:
	//   pArea -
	//   pPoints - 
	//   nCount -
	//   szName -
	//   fHeight -
	//   vAmbientColor -
	//   bAfectedByOutLights -
	//   bSkyOnly -
	//   vDynAmbientColor -
	//   fViewDistRatio -
	//   bDoubleSide -
	//   bUseDeepness -
	//   bUseInIndoors -
	virtual void UpdateVisArea(IVisArea * pArea, const Vec3 * pPoints, int nCount, const char * szName, float fHeight, const Vec3 & vAmbientColor, bool bAfectedByOutLights, bool bSkyOnly, const Vec3 & vDynAmbientColor, float fViewDistRatio, bool bDoubleSide, bool bUseDeepness, bool bUseInIndoors) = 0;

	/*! decides if a sound is potentially hearable between vis areas (different sectors, a door block the sounds)
		@param pArea1	  (the sector of one of the source)
		@param pArea2	  (the sector of one of the source)
		@return	true if sound is hearable, false otherwise
	*/

	// Summary:
	//   Determines if two VisAreas are connected
	// Description:
	//   Used to determine if a sound is potentially hearable between two VisAreas.
	// Arguments:
	//   pArea1 - A pointer to a VisArea
	//   pArea2 - A pointer to a VisArea
	//   nMaxRecursion - Maximum number of recursions to be done
	//   bSkipDisabledPortals - Indicate if disabled portals should be skipped
	// Return Value:
	//   A boolean value set to true if the two VisAreas are connected, else false will be returned.
	virtual bool IsVisAreasConnected(IVisArea * pArea1, IVisArea * pArea2, int nMaxRecursion = 1, bool bSkipDisabledPortals = true) = 0;

//mat: todo

	//! Create EntityRender object
	virtual IEntityRender * CreateEntityRender() = 0;

	//! Delete EntityRender object
	virtual void DeleteEntityRender(IEntityRender * pEntityRender) = 0;

	//! draw rain for renderer to render into texture
	virtual void DrawRain() = 0;

	//! set density of rain (0 - no rain, 1.f - max rain)
	virtual void SetRainAmount( float fAmount ) = 0;

	//! set wind direction and force Vec3(0,0,0) = no wind
	virtual void SetWindForce( const Vec3 & vWindForce ) = 0;

	// Summary:
	//   Get the amount of light (form 0.f to 1.f, valid only during entity render)
	// Parameters:
	//   bOnlyVisibleLights - If set to true, only sources with origin in the camera frustum will be taken into account
	virtual float GetLightAmountForEntity(IEntityRender * pEntity, bool bOnlyVisibleLights = false) = 0;

	//! returns amount of ambient light (form 0.f to 1.f)
	virtual float GetAmbientLightAmountForEntity(IEntityRender * pEntity) = 0;

	//! return indoor visibility area containing this position

	// Description:
	//   Gets the VisArea which is present at a specified point.
	virtual	IVisArea * GetVisAreaFromPos(const Vec3 &vPos) = 0;	

	//! enable/disable outdoor water and beaches rendering

	// Summary:
	//   Enable or disable ocean and shores rendering.
	// Arguments:
	//   bOcean - Will enable or disable the rendering of ocean
	//   bShore - Will enable or disable the rendering of the shores
	virtual void EnableOceanRendering(bool bOcean, bool bShore) = 0;

	//! Allocates new material object, and add it to the material manager.

	// Summary:
	//   Creates a new material object and register it with the material manager
	// Return Value:
	//   A newly created object derived from IMatInfo.
	virtual IMatInfo* CreateMatInfo() = 0;

	//! Deletes material object from material manager.
	//! @param pMatInfo Must be a valid pointer to existing material object.

	// Summary:
	//   Deletes a material object
	// Arguments:
	//   pMatInfo - A pointer to the material object to delete
	virtual void DeleteMatInfo(IMatInfo * pMatInfo) = 0;

	//! Rename material object (You should not use IMatInfo::SetName directly).
	//! @param pMtl Must be a valid pointer to existing material object.

	// Summary:
	//   Renames a material object
	// Note: 
	//   Do not use IMatInfo::SetName directly.
	// Arguments:
	//   pMtl - Pointer to a material object
	//   sNewName - New name to assign to the material
	virtual void RenameMatInfo( IMatInfo *pMtl,const char *sNewName ) = 0;

	//! Find loaded material from the library.

	// Summary:
	//   Finds a material
	// Arguments:
	//   A null terminated string which hold the name to search
	// Return Value:
	//   Return a pointer to the material object found.
	virtual IMatInfo* FindMaterial( const char *sMaterialName ) = 0;
	
//DOC-IGNORE-BEGIN
//Internal for the the lightmaps
	//! \brief Create an instance of a lightmap serialization manager
	virtual struct ILMSerializationManager * CreateLMSerializationManager() = 0;
//DOC-IGNORE-END

	//! return true if object can be visible soon

	// Summary:
	//   Determine if the entity is potentially visible
	// Arguments:
	//   pEntityRender - A pointer to the entity
	//   fAdditionRadius - Size of any additional radius which need to be visible (optional)
	// Return Value:
	//   A boolean value which equal to true if the entity is visible, or false if it isn't.
	virtual bool IsPotentiallyVisible(IEntityRender * pEntityRender, float fAdditionRadius=0) = 0;

	//! create new static lsource, returns source id or -1 if it fails

	// Summary:
	//   Creates a new static light source
	// Return Value:
	//   An integer which hold the id of the newly created light.
	virtual INT_PTR AddStaticLightSource(const class CDLight & LSource, IEntityRender *pCreator, ICryCharInstance * pCryCharInstance=NULL, const char * szBoneName=NULL) = 0;	//## AMD Port

	//! delete static lsource (return false if not found)

	// Summary:
	//   Deletes a static light
	// Arguments:
	//   nLightId - Id of the light to delete
	// Return Value:
	//   A boolean which equal to true if the light has been deleted or false if it couldn't be found.
	virtual bool DeleteStaticLightSource(INT_PTR nLightId) = 0;		//## AMD Port

	//! gives access to static lsources list (for lmap generator)

	// Summary:
	//   Gives access to the list holding all static light sources
	// Return Value:
	//   A list2 holding all the CDLight pointers.
	virtual const list2<CDLight*> * GetStaticLightSources() = 0;

  //! Reload heightmap and reset decals and particles, in future will restore deleted vegetations

	// Summary:
	//   Reload the heightmap
	// Description:
	//   Reloading the heightmap will resets all decals and particles.
  virtual void RestoreTerrainFromDisk() = 0;

//DOC-IGNORE-BEGIN
  // tmp
  virtual const char * GetFilePath(const char * szFileName) { return GetLevelFilePath(szFileName); }
//DOC-IGNORE-END

  // tiago: added

  //! set blur mask texture
  virtual void SetBlurMask(ITexPic *pMask)=0;

  //! set screen mask texture
  virtual void SetScreenMask(ITexPic *pMask)=0;

  //! set screen fx type
  virtual void SetScreenFx(const char *pEffectName, int iActive)=0;

  //! set current active camera focus position
  //virtual int SetCameraFocus(const Vec3 &pPos)=0;

  //! set screen fx parameter
  virtual void SetScreenFxParam(const char *pEffectName, const char *pEffectParam, void *pValue)=0;

  //! get screen fx type
  virtual int GetScreenFx(const char *pEffectName)=0;

  //! get screen fx parameter
  virtual int GetScreenFxParam(const char *pEffectName, const char *pEffectParam, void *&pValue)=0;

  //! reset current screen effects
  virtual void ResetScreenFx(void)=0;

  //! physicalize area if not physicalized yet
  virtual void CheckPhysicalized(const Vec3 & vBoxMin, const Vec3 & vBoxMax) = 0;

	//! in debug mode check memory heap and makes assert, do nothing in release
	virtual void CheckMemoryHeap() = 0;

	//! return value of e_obj_lod_ratio cvar(about 10.f), used for entities
	virtual float GetObjectsLODRatio() = 0;

	//! return value of e_obj_view_dist_ratio cvar, used for entities
	virtual float GetObjectsViewDistRatio() = 0;

	//! return value of e_obj_min_view_dist cvar, used for entities
	virtual float GetObjectsMinViewDist() = 0;

	/*!
	Set material parameter
	@param szMatName materal name
	@param nTexSlot text slot id, see EFTT_DIFFUSE for example
	@param nSubMatId submaterial id, -1 means use root material
	@param szParamName can be one of:
		
		m_eTGType
		m_eRotType
		m_eUMoveType
		m_eVMoveType
		m_bTexGenProjected

		m_Tiling[0]
		m_Tiling[1]
    m_Tiling[2]
		m_Offs[0]
		m_Offs[1]
		m_Offs[2]

		m_Rot[0]
		m_Rot[1]
		m_Rot[2]
		m_RotOscRate[0]
		m_RotOscRate[1]
		m_RotOscRate[2]
		m_RotOscAmplitude[0]
		m_RotOscAmplitude[1]
		m_RotOscAmplitude[2]
		m_RotOscPhase[0]
		m_RotOscPhase[1]
		m_RotOscPhase[2]

		m_UOscRate
		m_VOscRate
		m_UOscAmplitude
		m_VOscAmplitude
		m_UOscPhase
		m_VOscPhase

	@param fValue new value
*/
	virtual bool SetMaterialFloat( char * szMatName, int nSubMatId, int nTexSlot, char * szParamName, float fValue ) = 0;

	//! close terrain texture file handle and allows to replace/update it
	virtual void CloseTerrainTextureFile() = 0;
	
	//! remove all decals attached to specified entity
	virtual void DeleteEntityDecals(IEntityRender * pEntity) = 0;

	//! prepare data for rendering
	virtual void OnLevelLoaded() = 0;

	//! disable CGFs unloading
	virtual void LockCGFResources() = 0;

	//! enable CGFs unloading (this is default state), this function will also release all not used CGF's
	virtual void UnlockCGFResources() = 0;

	//! give access to materials library
	virtual class CMatMan * GetMatMan() = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

#if (defined(GAMECUBE) || defined (PS2))
#define __TIMESTAMP__ "Ver1.0"
#endif

// expirimental way to track interface version 
// this value will be compared with value passed from system module
const char g3deInterfaceVersion[32] = __TIMESTAMP__;

// CreateCry3DEngine function type definition
typedef I3DEngine * (*PFNCREATECRY3DENGINE)(ISystem	* pSystem,const char * szInterfaceVersion);

//! Creates 3dengine instance

// Description:
//     Create an instance of the 3D Engine. It should usually be called by 
//     ISystem::Init3DEngine.
// See Also:
//     I3DEngine, I3DEngine::Release
// Arguments:
//     ISystem            - Pointer to the current ISystem instance
//     szInterfaceVersion - String version of with the build date
// Summary:
//     Create an instance of the 3D Engine
CRY3DENGINEENGINE_API I3DEngine * CreateCry3DEngine(ISystem	* pSystem,const char * szInterfaceVersion=g3deInterfaceVersion);

#ifdef __cplusplus
}
#endif




#endif //CRY3DENGINEINTERFACE_H

