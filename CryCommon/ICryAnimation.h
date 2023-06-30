////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   iCryAnimation.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: CryAnimation interface
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef ICRY_ANIMATION
#define ICRY_ANIMATION

#if !defined(_XBOX) && !defined(LINUX)
	#ifdef CRYANIMATION_EXPORTS
		#define CRYANIMATION_API __declspec(dllexport)
	#else
		#define CRYANIMATION_API __declspec(dllimport)
	#endif
#else
	#define CRYANIMATION_API
#endif

//DOC-IGNORE-BEGIN
#include "smartptr.h"
#include "Cry_Math.h"

//! Forward declarations
#include "IBindable.h"
struct	IShader;
class   CryMatrix;
struct  SRendParams;
template <class T> class list2;
struct CryEngineDecalInfo;
struct ParticleParams;
struct CryCharMorphParams;
struct IMatInfo;

//#include "CryParticleSpawnInfo.h"

//////////////////////////////////////////////////////////////////////
#include <IPhysics.h>
//DOC-IGNORE-END

//! flags used by game
enum ECharRenderFlags
{
	CS_FLAG_DRAW_MODEL = 1 << 0,
	CS_FLAG_DRAW_NEAR  = 1 << 1,
	CS_FLAG_UPDATE     = 1 << 2
	/*
	CS_FLAG_MIRROR_X   = 1 << 3,// PARTIALLY supported (only for 1stperson weapons)
	CS_FLAG_MIRROR_Y   = 1 << 4,// NOT supported
	CS_FLAG_MIRROR_Z   = 1 << 5 // NOT supported
	*/
};

//! character limb indentifiers used for IK
enum limb_identifiers { LIMB_LEFT_ARM=0,LIMB_RIGHT_ARM=1, LIMB_LEFT_LEG=2,LIMB_RIGHT_LEG=3 };
enum limb_ik_flags { ik_leg=1, ik_arm=2, ik_avoid_stretching=4 };
#define IK_NOT_USED 1E9f



// Description:
//     Provide access to the bone matrices to allow procedural animations
struct ICryBone
{
	// Description:
	//     Can set the relative to default pose matrix to not be calculated. 
	//     In this case, the relative to default must be set externally via 
	//     GetRelativeToDefPoseMatrixPointer.
	//     This feature is used for lip sync.
	// Arguments:
	//     bDoNotCalculate - If set to true, the relative to default pose matrix 
	//                       will not be calculated.
	// Summary:
	//     Can disable the relative to default pose matrix calculation
	virtual void DoNotCalculateBoneRelativeMatrix(bool bDoNotCalculate) = 0;

	// Description:
	//     Will remove the effect done by FixBoneMatrix.
	// See Also:
	//     FixBoneMatrix
	// Summary:
	//     Remove any bone matrix fixing effect
	void UnfixBoneMatrix()
	{
		DoNotCalculateBoneRelativeMatrix(false);
	}
  
	// Description:
	//     Use a supplied matrix (relative to parent) as the bone matrix and 
	//     will prevent the Animation System from changing it afterward. To 
	//     reset this effect, call UnfixBoneMatrix.
	// See Also:
	//     UnfixBoneMatrix
	// Arguments:
	//     mtxBone - The matrix to specify to be the bone's matrix
	// Summary:
	//     Fix the bone's matrix with a specified matrix
	virtual void FixBoneMatrix (const Matrix44& mtxBone) = 0;

	// Description:
	//     WHAT IS THIS:
	//     fixes the bone matrix to the given position in world coordinates,
	//     assuming the character position and orientation are given by the vCharPos and vCharAngles
	//     vCharAngles are the same as in the entity and in the Draw call to ICryCharInstance
	// See Also:
	//     SetBoneOriginInWorld
	// Arguments:
	//     vCharPos      - The character's position
	//     vCharAngles   - The character's angles
	//     vTargetOrigin - The character's target origin
	// Summary:
	//     Fix the bone matrix to a position and orientation in world coordinates
	virtual void FixBoneOriginInWorld (const Vec3& vCharPos, const Vec3& vCharAngles, const Vec3& vTargetOrigin) = 0;
	
	// Description:
	//     WHAT IS THIS:
	//     Sets the bone matrix to the given position in world coordinates only for this frame (until update)
	//     assuming the character position and orientation are given by the vCharPos and vCharAngles
	//     vCharAngles are the same as in the entity and in the Draw call to ICryCharInstance
	// See Also:
	//     FixBoneOriginInWorld
	// Arguments:
	//     vCharPos      - The character's position
	//     vCharAngles   - The character's angles
	//     vTargetOrigin - The character's target origin
	// Summary:
	//     Fix the bone matrix to a position and orientation in world coordinates until the Animation System is updated
	virtual void SetBoneOriginInWorld (const Vec3& vCharPos, const Vec3& vCharAngles, const Vec3& vTargetOrigin) = 0;

	// Description:
	//     Set the plus-matrix rotation components. The plus-matrix is used to 
	//     rotate the upper body following the rotation of the head. This usually
	//     should be set in the game code.
	// Arguments:
	//     x - x angle in degree
	//     y - y angle in degree
	//     z - z angle in degree
	// Summary:
	//     Set the plus-matrix rotation components
	virtual void SetPlusRotation(float x, float y, float z) = 0;

	// Description:
	//     Set the plus-matrix rotation components. The plus-matrix is used to 
	//     rotate the upper body following the rotation of the head. This usually
	//     should be set in the game code.
	// Arguments:
	//     qRotation - Rotation stored as a quaternion
	// Summary:
	//     Set the plus-matrix rotation components
	virtual void SetPlusRotation(const CryQuat& qRotation) = 0;

	// Description:
	//     Reset the plus-matrix to identity. The upper body will be rotated at the same rate as the head.
	// Summary: 
	//     Reset the plus-matrix rotation
	virtual void ResetPlusRotation() = 0;

	// Description:
	//     Get the position of the bone.
	// Return Value:
	//     The position is return in a vec3 structure.
	// Summary:
	//     Get the position of the bone
	virtual Vec3 GetBonePosition() = 0;

	// Description:
	//     Get the bone axis.
	// Arguments:
	//     cAxis - char value containing either x, y or z
	// Return Value:
	//     Return the vector for the requested axis. If cAxis wasn't set to either x, y or z, Vec3d(0,0,0) will be returned.
	// Summary:
	//     Get the bone axis
	virtual Vec3 GetBoneAxis(char cAxis) = 0;

	// returns the parent world coordinate system rotation as a quaternion

	// Description:
	//     Get the parent bone world coordinate rotation as a quaternion.
	// Return Value:
	//     A quaternion holding the world coordinate rotation of the parent bone.
	// Summary:
	//     Get the parent bone world coordinate rotation as a quaternion
	virtual CryQuat GetParentWQuat () = 0;

	// Description:
	//     Get the parent bone.
	// Return Value:
	//     A pointer to a ICryBone derived class representing the parent bone. If there's none, NULL will be returned.
	// Summary:
	//     Get the parent bone
	virtual ICryBone* GetParent() = 0;

	// Description:
	//     Get the matrix relative to the parent bone.
	// Return Value:
	//     A 4x4 matrix holding the relative to the parent bone.
	// Summary:
	//     Get the matrix relative to the parent bone
	virtual const Matrix44& GetRelativeMatrix() = 0;
	
	// Description:
	//     Get the matrix in object coordinates.
	// Return Value:
	//     A 4x4 matrix holding the relative to the parent bone.
	// Summary:
	//     Get the matrix in object coordinates
	virtual const Matrix44& GetAbsoluteMatrix() = 0;
};

struct AnimSinkEventData
{
	AnimSinkEventData (void* _p = NULL):
		p(_p),
		n((INT_PTR)_p)
	{
	}
	
	void* p;
	INT_PTR   n;

	operator void* () {return p;}
	operator const void* ()const {return p;}
	bool operator == (const AnimSinkEventData& that) const {return p == that.p && n == that.n;}
};

// Summary:
//     Callback functions for game code
struct ICharInstanceSink
{
  virtual void OnStartAnimation(const char *sAnimation) = 0;
  virtual void OnAnimationEvent(const char *sAnimation, AnimSinkEventData data) = 0;
  virtual void OnEndAnimation(const char *sAnimation) = 0;
};

// Summary:
//     Types of blending between animation layers
enum AnimTwinMode
{
  AnimTwinMode_Replace,
  AnimTwinMode_Add
};


//DOC-IGNORE-BEGIN
//! This interface is free from concepts specific for CryEngine
//DOC-IGNORE-END

// Description:
//     This interface hold a set of animation in which each animation is described as properties.
// Summary:
//     Hold description of a set of animation
struct IAnimationSet
{
	// Summary:
	//   Retrieves the amount of animation.
	// Return Value: 
	//     An integer holding the amount of animation
	virtual int Count() = 0;

	//! Returns the number of morph targets in the set

	// Summary:
	//   Retrieves the amount of morph target.
	// Return Value: 
	//     An integer holding the amount of morth target
	virtual int CountMorphTargets() {return 0;}

	//! Returns the index of the animation in the set, -1 if there's no such animation

	// Summary:
	//   Searches for the index of an animation using its name.
	// Arguments:
	//   szAnimationName - Null terminated string holding the name of the animation.
	// Return Value:
	//   An integer representing the index of the animation. In case the animation 
	//   couldn't be found, -1 will be returned.
	virtual int Find (const char* szAnimationName) = 0;

	//! Loads the animation data in memory. fWhenRequired is the timeout in seconds from current moment when
	//! the animation data will actually be required

	// Summary:
	//   Loads the animation data in memory.
	// See Also:
  //   <LINK IAnimationSet::StartLoadAnimation@const char*@float, IAnimationSet::StartLoadAnimation>
	// Parameters:
	//   nAnimId       - Index of the animation.
	//   fWhenRequired - Amount of second until the data is loaded. Use
	//                   0.0f to immediately load the data.
	// Description:
	//   This method can allow to delay loading the data from disk
	//   until a specified amount of seconds.                                                         
	virtual void StartLoadAnimation (int nAnimId, float fWhenRequired) {}

	//! Loads the animation data in memory. fWhenRequired is the timeout in seconds from current moment when
	//! the animation data will actually be required

	// Summary:
	//   Loads the animation data in memory.
	// See Also:
	//   <LINK IAnimationSet::StartLoadAnimation@int@float, IAnimationSet::StartLoadAnimation>
	// Parameters:
	//   szAnimationName - Name of the animation
	//   fWhenRequired   - Amount of second until the data is loaded. Use
	//                     0.0f to immediately load the data.
	// Description:
	//   This method can allow to delay loading the data from disk
	//   until a specified amount of seconds.                                                 
	void StartLoadAnimation (const char* szAnimationName, float fWhenRequired)
	{
		StartLoadAnimation(Find (szAnimationName),fWhenRequired);
	}

	//! Unloads animation from memory
	//! The client must take into account that the animation can be shared and, if unloaded, the other
	//! character models (animation sets) will have to load it back to use.

	// Summary:
	//   Unloads the animation data from memory.
	// See Also:
	//   <LINK IAnimationSet::UnloadAnimation@int, IAnimationSet::UnloadAnimation>
	// Parameters:
	//   nAnimId - Id of the animation to unload.
	// Description:
	//   If the animation is used after a cal to this method, the animation data 
	//   will be loaded back automatically.
	virtual void UnloadAnimation (int nAnimId) {}
	
	//! Unloads animation from memory
	//! The client must take into account that the animation can be shared and, if unloaded, the other
	//! character models (animation sets) will have to load it back to use.

	// Summary:
	//   Unloads the animation data from memory.
	// See Also:
	//   <LINK IAnimationSet::UnloadAnimation@const char*, IAnimationSet::UnloadAnimation>
	// Parameters:
	//   nAnimId - Id of the animation to unload.
	// Description:
	//   If the animation is used after a cal to this method, the animation data 
	//   will be loaded back automatically.
	void UnloadAnimation (const char* szAnimationName)
	{
		UnloadAnimation(Find(szAnimationName));
	}

	//! Returns the index of the morph target in the set, -1 if there's no such morph target

	// Summary:
	//   Searches for morph target using a specified name.
	// Arguments:
	//   szMorphTarget - Name of the morph target to find.
	// Return Value:
	//   An integer representing the index of the morph target. The value -1 will 
	//   be returned in case that an appropriate morph target haven't been found.
	virtual int FindMorphTarget (const char* szMorphTarget) {return -1;}

	//! Returns the given animation length, in seconds

	// Summary:
	//   Retrieves the length of an animation.
	// See Also:
	//   <LINK IAnimationSet::GetLength@const char*, IAnimationSet::GetLength>
	// Arguments:
	//   nAnimationId - Id of the animation.
	// Return Value:
	//   A float value representing the length in seconds.
	virtual float GetLength (int nAnimationId) = 0;

	//! Returns the given animation length, in seconds

	// Summary:
	//   Retrieves the length of an animation.
	// See Also:
	//   <LINK IAnimationSet::GetLength@int, IAnimationSet::GetLength>
	// Arguments:
	//   szAnimationName - Name of the animation.
	// Return Value:
	//   A float value representing the length in seconds.
	virtual float GetLength (const char* szAnimationName)
	{
		return GetLength(Find(szAnimationName));
	}

//DOC-IGNORE-BEGIN
	//! Returns the given animation's start, in seconds; 0 if the id is invalid
	virtual float GetStart (int nAnimationId) {return 0;}// default implementation

	//! Returns the given animation's start, in seconds
	virtual float GetStart (const char* szAnimationName)
	{
		return GetStart (Find(szAnimationName));
	}
//DOC-IGNORE-END

	//! Returns the given animation name

	// Summary:
	//   Gets the name of the specified animation.
	// Arguments:
	//   nAnimationId - Id of an animation.
	// Return Value:
	//   A null terminated string holding the name of the animation. In case the 
	//   animation wasn't found, the string "!NEGATIVE ANIMATION ID!" will be 
	//   returned.
	virtual const char* GetName (int nAnimationId) = 0;

	//! Returns the name of the morph target

	// Summary:
	//   Gets the name of a morph target.
	// Arguments:
	//   nMorphTargetId - Id of the morph target.
	// Return Value:
	//   A null terminated string holding the name of the morph target. In case 
	//   the specified id is out of range, the string 
	//   "!MORPH TARGET ID OUT OF RANGE!" is returned.
	virtual const char* GetNameMorphTarget (int nMorphTargetId) {return "!NOT IMPLEMENTED!";}

	//! Retrieves the animation loop flag

	// Summary:
	//   Determines if an animation will be looped.
	// Arguments:
	//   nAnimationId - Id of the animation.
	// Return Value:
	//   A boolean equal to true if the animation is looped, or false if it isn't.
	virtual bool IsLoop (int nAnimationId) = 0;

	//! Retrieves the animation loop flag

	// Summary:
	//   Determines if an animation will be looped.
	// Arguments:
	//   szAnimationName - Name of the animation.
	// Return Value:
	//   A boolean equal to true if the animation is looped, or false if it isn't.
	virtual bool IsLoop (const char* szAnimationName)
	{
		return IsLoop(Find(szAnimationName));
	}
};

//! Interface that describes and manipulates a set of animations
//! Cry... because it's specific to the needs of the CryEngine
struct ICryAnimationSet: public IAnimationSet
{
	//! Modifies the animation loop flag

	// Summary:
	//   Specifies if an animation should be looped.
	// Arguments:
	//   nAnimationId - Id of the animation.
	//   bIsLooped - Should be equal to true to enable looping, or false to disable it.
	virtual void SetLoop (int nAnimationId, bool bIsLooped) = 0;
};

//! Interface to the character instance model
//! A set of characters created out of the same CGF share many properties, and
//! interface to those properties is through the ICryCharModel.
//! The instance of the ICryCharModel is guaranteed to be alive at least as long as
//! all its ICryCharInstance are alive

// Description:
//     This class is used to store in memory the model data usually loaded from
//     a cgf file. Should usually be created while creating a new character 
//     instance with ICryCharManager::MakeCharacter.
struct ICryCharModel: public _i_reference_target_t
{
	// Description:
	//     Return a pointer of the instance of a ICryAnimationSet derived class
	//     applicable for the model.
	// Return Value:
	//     A pointer to a ICryAnimationSet derived class
	// Summary:
	//     Get the Animation Set defined for the model
	virtual ICryAnimationSet* GetAnimationSet () = 0;
	
	// Description:
	//     Return the name of a bone by searching its id.
	// See Also:
	//     GetBoneByName
	// Arguments:
	//     nId - Id of the bone
	// Return Value:
	//     A NULL terminated char array which hold the name. If no bones of the specified id have been found, NULL is returned.
	// Summary:
	//     Get the bone's name from a specified id
	virtual const char * GetBoneName(int nId) const = 0;

	// Description:
	//     Return the index of a bone by searching its name. It's important to 
	//     be aware that the bone name is case sensitive.
	// See Also:
	//     GetBoneName
	// Arguments:
	//     szName - Name of the bone
	// Return Value:
	//     An int value which hold the id. If no bones of the specified name have been found, -1 is returned.
	// Summary:
	//     Get the bone's id from a specified name
	virtual int GetBoneByName (const char* szName) {return -1;}

	// Description:
	//     Return the number of bones included in the model. All bone ids start from 0. The first bone, which has the id 0, is the root bone.
	// Return Value:
	//     An int value which hold the number of bones.
	// Summary:
	//     Get the number of bones
	virtual int NumBones() const = 0;

	// Description:
	//     Currently not implemented. Will only return 1.0f.
	// Return Value:
	//     A float value which hold the scale value.
	// Summary:
	//     Get the scale of the model (not implemented)
	virtual float GetScale() const = 0;

	// Summary:
	//     Possible classes for a model
	enum ClassEnum
	{
		CLASS_UNKNOWN,
		CLASS_ANIMOBJECT,
		CLASS_CRYCHARBODY
	};

	// Description:
	//     Get the class of the model.
	// Return Value:
	//     Return one of the class ids declared in EClassEnum
	// Summary:
	//     Get the class of the model
	virtual ClassEnum GetClass() {return CLASS_UNKNOWN;}

	// Description:
	//     Get a property attached to the model during exporting process.
	// Return Value:
	//     A pointer to a null terminated char string which contain the 
	//     filename of the model.
	// Summary:
	//     Get the filename of the model
	virtual const char* GetFileName() {return "";}

	// Description:
	//     Get a property which was attached to the model during exporting 
	//     process as the scene user properties.
	// Arguments:
	//     szName - Name of the property
	// Return Value:
	//     A pointer to a null terminated char string which contain the value 
	//     of the requested property.
	//     NULL will be returned if there isn't any property attached with the 
	//     specified name.
	// Summary:
	//     Get property which was set for the model
	virtual const char* GetProperty(const char* szName) {return "";}
};


//! This is interface to an object that should be skinned
//! It's used to let the renderer know that an object callback must be called before rendering because
//! the object vertices can be deformed
struct IDeformableRenderMesh
{
	//! Renderer calls this function to allow update the video vertex buffers right before the rendering
	virtual void ProcessSkinning(const Vec3& t,const Matrix44& mtxModel, int nTemplate, int nLod=-1, bool bForceUpdate=false) = 0;
};

struct ICryCharVisualElement: public _reference_target_t
{
	// returns true if the given submesh is visible
	virtual bool IsVisible() = 0;

	// depending on bVisible, either makes the submesh visible or invisible
	virtual void SetVisible(bool bVisible = true) = 0;
};

// this is submesh of the character instance: a piece of geometry that can be
// turned visible/invisible by wish
struct ICryCharSubmesh: public ICryCharVisualElement
{
	// returns the model of the submesh, or NULL in case of failure
	virtual ICryCharModel* GetModel () = 0;

	//! Start the specified by parameters morph target
	virtual void StartMorph (const char* szMorphTarget, const CryCharMorphParams& params) = 0;

	//! Start the specified by parameters morph target
	virtual void StartMorph (int nMorphTargetId, const CryCharMorphParams& params) = 0;

	//! Finds the morph with the given id and sets its relative time.
	//! Returns false if the operation can't be performed (no morph)
	//! The default implementation for objects that don't implement morph targets is to always fail
	virtual bool SetMorphTime (int nMorphTargetId, float fTime) = 0;

	//! Set morph speed scale
	//! Finds the morph target with the given id, sets its morphing speed and returns true;
	//! if there's no such morph target currently playing, returns false
	virtual bool SetMorphSpeed (int nMorphTargetId, float fSpeed) = 0;

	//! Stops morph by target id
	virtual bool StopMorph (int nMorphTargetId) = 0;

	//! Stops all morphs
	virtual void StopAllMorphs() = 0;

	//! freezes all currently playing morphs at the point they're at
	virtual void FreezeAllMorphs(){}

	// Adds a decal to the character
	virtual void AddDecal (CryEngineDecalInfo& Decal) = 0;

	// cleans up the decals on this body part
	virtual void ClearDecals() = 0;

	//! returns the leaf buffer materials in this character (as they are used in the renderer)
	virtual const list2<struct CMatInfo>*getLeafBufferMaterials() = 0;

	virtual bool SetShaderTemplateName (const char *TemplName, int Id, const char *ShaderName=0,IMatInfo *pCustomMaterial=0, unsigned nFlags = 0) = 0;
};

// Description:
//     Implement the sword trail effect
struct ICryCharFxTrail: public ICryCharVisualElement
{

};



//DOC-IGNORE-BEGIN
//! TODO:
//! Split this interface up into a few logical interfaces, starting with the ICryCharModel
//DOC-IGNORE-END

// Description:
//     This interface contains methods for manipulating and querying an animated character
//     Instance. The methods allow modify the animated instance to the certain way,
//     animate it, render, retrieve BBox/etc, control physics, particles and skinning, transform.
// Summary:
//     Interface to character animation
struct ICryCharInstance: public IBindable
{
	virtual ~ICryCharInstance() {};

	// Summary:
	//     Release character instance
	virtual void Release() = 0;

	// Description:
	//     Set rendering flags defined in ECharRenderFlags for this character instance
	// Arguments:
	//     Pass the rendering flags
	// Summary:
	//     Set rendering flags
	virtual void SetFlags(int nFlags)=0;

	// Description:
	//     Get the rendering flags enabled. The valid flags are the ones declared in ECharRenderFlags.
	// Return Value:
	//     Return an integer value which hold the different rendering flags
	// Summary:
	//     Set rendering flags
	virtual int  GetFlags()=0;

	// Description:
	//     Execute a specified animation script command on the characted instance.
	// Arguments:
	//     nCommand - The command to execute, as declared in CryAnimationScriptCommandEnum
	//     pParams  - Any parameters for the command
	//     pResult  - Pointer where to store the result
	// Return Value:
	//     true if the command was executed,
	//     false if it wasn't executed
	// Summary:
	//     Executes a script command
	virtual bool ExecScriptCommand (int nCommand, void* pParams, void* pResult) {return false;}

	// Description:
	//     Set the shader template to be used with the character instance.
	// Arguments:
	//     TemplName       - Name of the template
	//     Id              - Id of the shader
	//     ShaderName      - Name of the shader (optional)
	//     pCustomMaterial - Pointer to a Material class (optional)
	//     nFlags          - Flags, which are defined in ECharShaderFlags (optional)
	// Return Value:
	//     A bolean value which is true if success to set a new shader, otherwise false will be returned.
	// Summary:
	//     Set the shader template to be used
	virtual bool SetShaderTemplateName(const char *TemplName, int Id, const char *ShaderName=0,IMatInfo *pCustomMaterial=0,unsigned nFlags = 0)=0;
	
	// Description:
	//     Get the name of the current shader template used by the character instance.
	// Return Value:
	//     Null terminated char string which contain the name of the shader template
	// Summary:
	//     Get the name of the current shader template
	virtual const char * GetShaderTemplateName() = 0;

	//DOC-IGNORE-BEGIN
	// WHAT: !!!
	//! Set refract coef. for refractive shader
	//DOC-IGNORE-END

	// Description:
	//     Set the refraction coeficient to be used by the refractive shader.
	// Arguments:
	//     Name       - Name of the value
	//     fVal       - value to be passed to the shader
	//     ShaderName - Name of the shader
	// Summary:
	//     Set a float value associated with a shader
	virtual void SetShaderFloat(const char *Name, float fVal, const char *ShaderName=NULL) = 0;

	// Description:
	//     Set the color to be used by the character instance
	// Arguments:
	//     fR - Red value 
	//     fG - Green value
	//     fB - Blue value
	//     fA - Alpha value
	// Summary:
	//     Set the color
	virtual void SetColor(float fR, float fG, float fB, float fA) = 0;

	// Description:
	//     Draw the character using specified rendering parameters.
	// Arguments:
	//     RendParams - Rendering parameters
	// Summary:
	//     Draw the character
	virtual void Draw(const SRendParams & RendParams,const Vec3& t)=0;	

	// Summary:
	//     Mark all LODs to be reskinned
	virtual void ForceReskin () {}

	// Description:
	//     Get the leaf buffer materials in this character as they are used in the renderer
	// Return Value:
	//     List of material info
	// Summary:
	//     Return the leaf buffer materials
	virtual const list2<struct CMatInfo>*getLeafBufferMaterials() {return NULL;}

	//! Interface for the renderer - returns the CDLight describing the light in this character;
	//! returns NULL if there's no light with such index
	//! ICryCharInstance owns this light. This light may be destructed without notice upon next call to 
	//! any other function but GetLight(). Nobody may every modify it.
	virtual const class CDLight* GetBoundLight (int nIndex) = 0;

	// Description:
	//     Draw the character shadow volumes into the stencil buffer. It's 
	//     suggested to only use this function in in-door environment.
	// Arguments:
	//     rParams - Rendering Parameters
	//     nLimitLOD - The maximum LOD to be used
	// Summary:
	//     Draw the character shadow volumes into the stencil buffer
	virtual void RenderShadowVolumes(const SRendParams *rParams, int nLimitLOD = 0)=0;

	//! Draw the character without shaders for shadow mapping
  //virtual void DrawForShadow(const Vec3 & vTranslationPlus = Vec3(0,0,0))=0;
  
  //! Return dynamic bbox of object

	// Description:
	// Arguments:
	// Summary:
	//     Get the bounding box
	virtual void GetBBox(Vec3& Mins, Vec3& Maxs)=0;

	//! Return dynamic center of object

	// Summary:
	//   Gets the dynamic center.
	// Return Value:
	//   Center of the character.
  virtual const Vec3 GetCenter()=0;

  //! Return dynamic radius of object

	// Summary:
	//   Gets the dynamic radius.
	// Return Value:
	//   Radius of the character.
  virtual const float GetRadius()=0;

	// Handle which determines the object binding to a bone in the character.
	typedef ULONG_PTR ObjectBindingHandle;
	
	// Specify an invalid object binding.
	enum {nInvalidObjectBindingHandle = 0};

  //! Attach object to bone (Return invalid handle if bone not found)
	//! Detaches all objects beforehand; if the object is NULL, just detaches all objects from the bone
	//! If the bone name is invalid, detaches everything to avoid using dangle pointers (in case the bone name was misspelled)

	// Summary:
	//   Attach an object to a bone
	// See Also:
	//   AttachToBone
	// Description:
	//   The method AttachToBone should be used instead.
  virtual ObjectBindingHandle AttachObjectToBone(IBindable * pWeaponModel, const char * szBoneName, bool bUseRelativeToDefPoseMatrix = true, unsigned nFlags = 0)=0;   

	// detaches all objects from bones; returns the nubmer of bindings deleted

	// Summary:
	//   Detaches all objects from every bones.
	// Return Value:
	//   An integer representing the number of binding detached.
	virtual unsigned DetachAll() {return 0;}

	// detach all bindings to the given bone; returns the nubmer of bindings deleted

	// Summary:
	//   Detaches all objects from a specified bone.
	// Arguemnts:
	//   nBone - The id of a bone.
	// Return Value:
	//   An integer representing the number of binding detached.
	virtual unsigned DetachAllFromBone(unsigned nBone) {return 0;}

	// Summary:
	//   Flag used to attach bindable an object to a bone.
	enum EAttachFlags
	{
		// Used to make sure a binding is more visible to the camera despite its distance.
		FLAGS_ATTACH_ZOFFSET = 1
	};

	// Summary:
	//   Attaches an bindable object to a specified bone.
	// Arguments:
	//   pObj - A pointer to an object derived from IBindable.
	//   nBone - Id of the bone to attach to.
	//   nFlags - A flag from EAttachFlags (optional).
	// Return Value:
	//   An handle to the binding. In case the call failed, nInvalidObjectBindingHandle will be returned.
	virtual ObjectBindingHandle AttachToBone (IBindable*pObj, unsigned nBone, unsigned nFlags = 0) {return nInvalidObjectBindingHandle;}

	// detaches the given binding; returns true upon successful detach; if it returns false, the binding handle is invalid
	// the binding becomes invalid immediately after detach

  // Summary:
	//   Detaches a specified binded object.
	// Arguments:
	//   nHandle - A binding object, as returned from AttachToBone.
	// Return Value:
	//   A bool with the value true is the binding was detached, or false if the 
	//   binding handle was invalid.
	virtual bool Detach (ObjectBindingHandle nHandle) {return false;}

	// checks if the given binding is valid

	// Summary:
	//   Determines if a specified binding is valid.
	// Arguments:
	//   A binding handle as returned by AttachToBone.
	// Return Value:
	//   A bool which is equal to true if the binding is valid or false if it isn't.
	virtual bool IsBindingValid (ObjectBindingHandle nHandle) {return false;}

	// returns the number of bindings; valid until the next attach/detach operation

	// Summary:
	//   Get the amount of binding on a character instance.
	// Return Value:
	//   An integer representing the amount of binding.
	virtual size_t GetBindingCount() {return 0;}

	// fills the given array with  GetBindingCount() pointers to IBindable

	// Summary:
	//   Gets all pointers of every bindable attached.
	// Note: 
	//   Make sure the array is sized to at least the amount returned by GetBindingCount.
	// See Also:
	//   GetBindingCount
	// Arguments:
	//   pResult - An array of IBindable* which should be already allocated.
	virtual void EnumBindables(IBindable** pResult) {}

	typedef ULONG_PTR LightHandle;
	enum {InvalidLightHandle = 0};

	//! attach a light to a bone
	//! If bCopyLight is true, then this light will be copied and the copy will be managed by the CryCharInstance
	//! You can always get the pointer to it via GetLight, but don't you dare to delete it!


	// Summary:
	//   Attach a light to a bone.
	// Arguments:
	//   pDLight - ...
	//   nBone - ...
	//   bCopyLight - ...
	// See Also:
	//   DetachLight, GetLight
	virtual LightHandle AttachLight (CDLight* pDLight, unsigned nBone, bool bCopyLight = false) {return InvalidLightHandle;}
	
	//! detach the light from the bone

	// Summary:
	//   Detach a light from a bone.
	// See Also:
	//   AttachLight
	virtual void DetachLight (CDLight* pDLight){}

	//! Attach a light (copying the light actually) to the bone
	//! Returns the handle identifying the light. With this handle, you can either
	//! Retrieve the light information or detach it.

	// Summary:
	//   Attach a light to a bone.
	// Arguments:
	//   rDLight - Structure representing a light.
	//   szBoneName - Name of the bone intended to have a light attached to it.
	// See Also:
	//   DetachLight
	virtual LightHandle AttachLight (const CDLight& rDLight, const char* szBoneName){return InvalidLightHandle;}
	//! Detaches the light by the handle retuned by AttachLight
	virtual void DetachLight (LightHandle nHandle) {}
	//! Returns the light by the light handle; returns NULL if no such light found
	virtual CDLight* GetLight(LightHandle nHandle) {return NULL;}
	//! Returns the light handle if the light is attached; returns invalid handle, if this light is not attached
	//! NOTE: if your light was attached with copying, then you won't get the handle for the original light pointer
	//! because the original light might have been attached several times and have several pointers in this case
	virtual LightHandle GetLightHandle (CDLight* pLight) {return InvalidLightHandle;}

	//! Enables/Disables the Default Idle Animation restart.
	//! If this feature is enabled, then the last looped animation will be played back after the current (non-loop) animation is finished.
	//! Only those animations started with the flag bTreatAsDefaultIdleAnimation == true will be taken into account

	// Summary:
	//   Enables or disables the default idle animation.
	// See Also:
	//   SetDefaultIdleAnimation
	// Description:
	//   When enabled, the default idle animation will be played back once the 
	//   current non-looped animation will be finished. The default idle animation
	//   needs to have already been specified to the animation system.
	// Arguments:
	//   nLayer - Specify on which layer to activate the default idle animation.
	//   bEnable - true will enable this feature, false will disable it.
	virtual void EnableLastIdleAnimationRestart (unsigned nLayer, bool bEnable = true) = 0;

	//! Start the specified animation with the given parameters

	// Summary:
	//   Starts an animation with the given parameters.
	// Arguments:
	//   szAnimName - Name of the animation.
	//   Params - Structure holding different options.
	// Return Value:
	//   A bool value set to true if the call was successful, or false if it wasn't.
	virtual bool StartAnimation (const char* szAnimName, const struct CryCharAnimationParams& Params) {return false;}

//DOC-IGNORE-BEGIN
	// FOR TEST ONLY enables/disables StartAnimation* calls; puts warning into the log if StartAnimation* is called while disabled
	virtual void EnableStartAnimation (bool bEnable) {}
//DOC-IGNORE-END

	//! Start the specified by parameters morph target
	virtual void StartMorph (const char* szMorphTarget, const CryCharMorphParams& params) {}

	//! Start the specified by parameters morph target
	virtual void StartMorph (int nMorphTargetId, const CryCharMorphParams& params) {}

	//! Finds the morph with the given id and sets its relative time.
	//! Returns false if the operation can't be performed (no morph)
	//! The default implementation for objects that don't implement morph targets is to always fail
	virtual bool SetMorphTime (int nMorphTargetId, float fTime) {return false;}

	//! freezes all currently playing morphs at the point they're at
	virtual void FreezeAllMorphs() {}

	//! Stops the animation at the specified layer. Returns true if there was some animation on that layer, and false otherwise
	virtual bool StopAnimation (int nLayer) {return false;}

  //! Return current animation name ( Return 0 if animations stoped )
  virtual const char * GetCurAnimation() = 0;

	//! Returns the current animation in the layer or -1 if no animation is being played 
	//! in this layer (or if there's no such layer)
	virtual int GetCurrentAnimation (unsigned nLayer) = 0;
  
	//! Resets all animation layers ( stops all animations )
  virtual void ResetAnimations()=0;
  
	//! Set animations speed scale
	//! This is the scale factor that affects the animation speed of the character.
	//! All the animations are played with the constant real-time speed multiplied by this factor.
	//! So, 0 means still animations (stuck at some frame), 1 - normal, 2 - twice as fast, 0.5 - twice slower than normal.
  virtual void SetAnimationSpeed(float fSpeed) = 0;

  virtual float GetAnimationSpeed() = 0;

	//! Set morph speed scale
	//! Finds the morph target with the given id, sets its morphing speed and returns true;
	//! if there's no such morph target currently playing, returns false
	virtual bool SetMorphSpeed (int nMorphTargetId, float fSpeed) {return false;}

	//! Stops morph by target id
	virtual bool StopMorph (int nMorphTargetId) {return false;}

	//! Stops all morphs
	virtual void StopAllMorphs() {}

	//! This is the same as SetAnimationSpeed, but it sets the speed for layers
	//! NOTE: It does NOT override the overall animation speed, but it's multiplies it
	virtual void SetAnimationSpeed(int nLayer, float fSpeed) {}

	//! Enable object animation time update. If the bUpdate flag is false, subsequent calls to Update will not animate the character
	virtual void EnableTimeUpdate (bool bUpdate) = 0;

	//! Set the current time of the given layer, in seconds
	virtual void SetLayerTime (int nLayer, float fTimeSeconds) = 0;
	//! Return the current time of the given layer, in seconds
	virtual float GetLayerTime (int nLayer) = 0;

	enum UpdateEnum
	{
		flagDontUpdateBones = 1,
		flagDontUpdateAttachments = 1 << 1
	};

  //! Processes skining (call this function every frame to animate character)
	//! dwFlags	is a bitwise OR of one of the flags specified in the UpdateEnum enumeration
	virtual void Update (Vec3 vPos = Vec3(0,0,0), float fRadius=0, unsigned uFlags = 0) {}

	//! Updates the bones and the bounding box. Should be called if animation update
	//! cycle in EntityUpdate has already passed but you need the result of new animatmions
	//! started after Update right now.
	virtual void ForceUpdate() {}

	//! Synchronizes state with character physical animation; should be called after all updates (such as animation, game bones updates, etc.)
	virtual void UpdatePhysics( float fScale=1.0f )=0;

  //! IK (Used by physics engine)
	virtual void BuildPhysicalEntity(IPhysicalEntity *pent,float mass,int surface_idx,float stiffness_scale=1.0f,int nLod=0) = 0;
	virtual IPhysicalEntity *CreateCharacterPhysics(IPhysicalEntity *pHost, float mass,int surface_idx,float stiffness_scale, int nLod=0) = 0;
	virtual int CreateAuxilaryPhysics(IPhysicalEntity *pHost, int nLod=0) = 0;
	virtual IPhysicalEntity *GetCharacterPhysics() = 0;
	virtual IPhysicalEntity *GetCharacterPhysics(const char *pRootBoneName) = 0;
	virtual IPhysicalEntity *GetCharacterPhysics(int iAuxPhys) = 0;
  virtual void SynchronizeWithPhysicalEntity(IPhysicalEntity *pent, const Vec3& posMaster=Vec3(zero),const Quat& qMaster=Quat(1,0,0,0)) = 0;
	virtual IPhysicalEntity *RelinquishCharacterPhysics() = 0;
	virtual void DestroyCharacterPhysics(int iMode=0) = 0;
	virtual void SetCharacterPhysParams(float mass,int surface_idx) = 0;
  virtual void SetLimbIKGoal(int limbid, vectorf ptgoal=vectorf(1E10f,0,0), int ik_flags=0, float addlen=0, vectorf goal_normal=vectorf(zero)) = 0;
  virtual vectorf GetLimbEndPos(int limbid) = 0;
  virtual void AddImpact(int partid, vectorf point,vectorf impact) = 0;
	virtual int TranslatePartIdToDeadBody(int partid) = 0;
	virtual vectorf GetOffset() = 0;
	virtual void SetOffset(vectorf offset) = 0;

  //! Direct access to the specified  bone
  virtual ICryBone * GetBoneByName(const char * szName) = 0;

  //! Pose character bones
  virtual bool SetAnimationFrame(const char * szString, int nFrame)=0;

	//! Callback interface; <<TODO::>> THese should be in a separate interface
  
	
	//! Enables receiving OnStart/OnEnd of all animations from this character instance
	//! THe specified sink also receives the additional animation events specified through AddAnimationEvent interface
  virtual void AddAnimationEventSink(ICharInstanceSink * pCharInstanceSink) = 0;

	//! Counterpart to AddAnimationEventSink
	virtual void RemoveAnimationEventSink(ICharInstanceSink * pCharInstanceSink) = 0;

  //! Enables receiving OnStart/OnEnd of specified animation from this character instance
	//! The specified sink also receives the additional animation events specified through AddAnimationEvent interface for this animation
  virtual void AddAnimationEventSink(const char* szAnimName, ICharInstanceSink * pCharInstanceSink) = 0;

	//! Counterpart to the AddAnimationEventSink
  virtual void RemoveAnimationEventSink(const char* szAnimName, ICharInstanceSink * pCharInstanceSink) = 0;

	//! Adds an animation event; whenever the character plays the specified frame of the specified animation,
	//! it calls back the animation event sinkreceiving OnEvent notification of specified animation for all instances using this model
  virtual bool AddAnimationEvent(const char * szAnimName, int nFrameID, AnimSinkEventData pUserData) = 0;

  //! Deletes the animation event; from now on the sink won't be receiving the animation this event
	virtual bool RemoveAnimationEvent (const char* szAnimName, int nFrameID, AnimSinkEventData pUserData) = 0;

	//! Deletes all animation events
	virtual void RemoveAllAnimationEvents(){}

	//! Returns the model interface
	virtual ICryCharModel* GetModel() = 0;

  //! Return position of helper of the static object which is attached to animated object
  virtual Vec3 GetTPVWeaponHelper(const char * szHelperName, ObjectBindingHandle nHandle) = 0;

	//! Returns position of specified helper ( exported into cgf file )
	//! Actually returns the given bone's position
	//! Default implementation: 000
	virtual Vec3 GetHelperPos(const char * szHelperName) {return Vec3(0,0,0);}
	//! Returns the matrix of the specified helper ( exported into cgf file )
	//! Actually returns the given bone's matrix
	virtual const Matrix44 * GetHelperMatrixByName(const char * szHelperName) {return NULL;}

	//! Returns the matrix of the helper object (in matOut), like GetTPVWeaponHelper returns the position.
	//! When the return value is true, matOut contains the matrix of the helper object
	//! When the return value is false, operation failed - assume the matOut to be undefined
	virtual bool GetTPVWeaponHelperMatrix(const char * szHelperName, ObjectBindingHandle nHandle, Matrix44& matOut) {return false;}

  //! Set the twining type. If replace - animations of second layer will overwrite first, otherwise it will sum
  virtual void SetTwiningMode(AnimTwinMode eTwinMode = AnimTwinMode_Replace) = 0;

  //! Return damage zone id for specified bone id
  virtual int GetDamageTableValue (int nId) = 0;

  //! returns true if the character is playing any animation now (so bbox is changing)
  virtual bool IsCharacterActive() = 0;

  // temporary hack - this is used in XPlayer in CryGame only
	/*
	Vec3 m_vAngles;
	void		SetAngles( const Vec3& angles ) {m_vAngles = angles;}
	Vec3&	GetAngles( ) {return m_vAngles;}
	*/

  //! Spawn decal on the walls, static objects, terrain and entities
	//! The decal hit direction and source point are in the local coordinates of the chracter.
	virtual void CreateDecal(CryEngineDecalInfo& DecalLCS)=0;

	//! cleans up all decals in this character
	virtual void ClearDecals() {}

	//! Returns true if this character was created from the file the path refers to.
	//! If this is true, then there's no need to reload the character if you need to change its model to this one.
	virtual bool IsModelFileEqual (const char* szFileName) = 0;


	//int AddParticleEmitter(ParticleParams& rInfo, float fSpawnRate, bool bFacesUp)
	//{
	//	return AddParticleEmitter(rInfo, CryParticleSpawnInfo(fSpawnRate, bFacesUp?CryParticleSpawnInfo::FLAGS_RAIN_MODE:0));
	//}

	//! Sets up particle spawning. After this funtion is called, every subsequenc frame,
	//! During the character deformation, particles will be spawned in the given characteristics.
	//! The returned handle is to be used to stop particle spawning
	//! -1 means invalid handle value (couldn't add the particle spawn task, or not implemented)
	virtual int AddParticleEmitter(struct ParticleParams& rInfo, const struct CryParticleSpawnInfo& rSpawnInfo) {return -1;}

	//! Stops particle spawning started with StartParticleSpawn that returned the parameter
	//! Returns true if the particle spawn was stopped, or false if the handle is invalid
	//! -1 means remove all particle emitters
	virtual bool RemoveParticleEmitter (int nHandle) {return false;}

	//! sets the scale to the given vector (1,1,1 is the default)
	//! isn't compatible with physicalized objects, use only with weapons
	virtual void SetScale (const Vec3d& vScale) {}

	//! Pushes the underlying tree of objects into the given Sizer object for statistics gathering
	virtual void GetMemoryUsage(class ICrySizer* pSizer) const = 0;

	//! sets the given aniimation to the given layer as the default
	virtual void SetDefaultIdleAnimation(unsigned nLayer, const char* szAnimName) = 0;

	//! notifies the renderer that the character will soon be rendered
	virtual void PreloadResources ( float fDistance, float fTime, int nFlags) {}


	//! adds a submesh, returns handle to it which can be used to delete the submesh
	//! submesh is created either visible or invisible
	//! submesh creation/destruction is heavy operations, so the clients must use they rarely,
	//! and set visible/invisible when they need to turn them on/off
	//! But creating many submeshes is memory-consuming so the number of them must be kept low at all times
	virtual ICryCharSubmesh* NewSubmesh (ICryCharModel* pModel, bool bVisible = false) {return NULL;}
	// adds submesh to the specified slot; replaces submesh if there's some there
	virtual ICryCharSubmesh* NewSubmesh (unsigned nSlot, ICryCharModel* pModel, bool bVisible = false) {return NULL;}
	
	//! removes submesh from the character
	//! does not change slot assignments. That is, removing a submesh from slot 1, you don't shift models from upper slots
	//! you may NOT remove submesh from slot 0
	virtual void RemoveSubmesh (ICryCharSubmesh* pSubmesh){}
	virtual void RemoveSubmesh (unsigned nSlot) {}

	//! enumeration of submeshes
	//! returns the number of currently allocated submesh slots
	virtual size_t NumSubmeshes() {return 0;}
	//! returns submesh from the i-th slot. Some submesh slots may be empty (NULL)
	virtual ICryCharSubmesh* GetSubmesh(unsigned i){return NULL;}


	virtual ICryCharFxTrail* NewFxTrail (unsigned nSlot, const struct CryCharFxTrailParams&) {return NULL;}
	virtual void RemoveFxTrail(unsigned nSlot) {}


	// returns the pointer to this character instance, if it's a character instance
	virtual struct ICryCharInstance* GetICryCharInstance() {return this;}
};

// Description:
//     This class is the main access point for any character anymation 
//     required for a program which uses CryEngine.
// See Also:
//     CreateCharManager
struct ICryCharManager
{
	//DOC-IGNORE-BEGIN
	// Flags used by this interface methods
	enum
	{
		// Temporary flag for AnimObject testing.
		nMakeAnimObject = 0x1000,
	};
	//DOC-IGNORE-END
	
	// Description:
	//     Model keep in memory hints.
	//     The model of a character can be locked in memory. This means, that even if the 
	//     number of characters using this model drops to zero, it'll be kept in memory.
	//     Such model is called Persistent.
	//     A model that's deleted as soon as there are no characters referencing it is called Transient.
	//     The default (when there's no hint) is defined by ca_KeepModels console variable.
	//     If there are both hints, it's up to the character animation system to decide whether to keep it or not.
	// Summary:
	//     Flags to unload a model from memory when it's no longer being used
	enum EModelPersistence
	{
		// Let the Animation System releases the model data when it’s no longer used
		nHintModelTransient  = 1, 
		// Force the model data to stay in memory even if no character instance uses it anymore
		nHintModelPersistent = 2, 
	};

	// Description:
	//     Contains statistics about CryCharManager.
	struct Statistics
	{
		// Number of character instances
		unsigned numCharacters;
		// Number of character models (CGF)
		unsigned numCharModels;
		// Number of animobjects
		unsigned numAnimObjects;
		// Number of animobject models
		unsigned numAnimObjectModels;
	};

	// Description:
	//     Will fill the Statistics parameters with statistic on the instance 
	//     of the Animation System.
	//     It isn't recommanded to call this function often.
	// Arguments:
	//     rStats - Structure which hold the statistics
	// Summary:
	//     Get statistics on the Animation System
    virtual void GetStatistics(Statistics& rStats) = 0;

    // Description:
	//     Gather the memory currently used by the animation. The information
	//     returned is classified according to the flags set in the sizer 
	//     argument.
	// Arguments:
	//     pSizer - Sizer class which will store the memory usage
	// Summary:
	//     Track memory usage
    virtual void GetMemoryUsage(class ICrySizer* pSizer) const = 0;

	// Description:
	//     Create a new instance for a model Load the model file along with any animation file that might be
	//     available.
	// See Also:
	//     RemoveCharacter
	// Arguments:
	//     szFilename - Filename of the model to be loaded
	//     nFlags     - Set how the model will be kept in memory after being 
	//                  used. Uses flags defined with EModelPersistence.
	// Return Value:
	//     A pointer to a ICryCharInstance class if the model could be loaded 
	//     properly.
	//     NULL if the model couldn't be loaded.
	// Summary:
	//     Create a new instance of a model
	virtual ICryCharInstance * MakeCharacter(const char * szFilename, unsigned nFlags = 0)=0;

	// Description:
	//     Load a model file into a ICryCharModel class. Usually, MakeCharacter 
	//     shall be used to ensure that a model is only loaded once even if 
	//     it's used several times.
	// See Also:
	//     MakeCharacter
	// Arguments:
	//     szFilename - Filename of the model to be loaded
	//     nFlags     - Set how the model will be kept in memory after being 
	//                  used. Uses flags defined with EModelPersistence.
	// Return Value:
	//     A pointer to a ICryCharModel class if the model could be loaded 
	//     properly.
	//     NULL if the model couldn't be loaded.
	// Summary:
	//     Load model file
	virtual ICryCharModel* LoadModel(const char* szFileName, unsigned nFlags=0)=0;

	// Description:
	//     Delete an instance of a character. If the corresponding model file
	//     is no longer used by any instances, it might be freed from memory if 
	//     the model was set to nHintModelTransient.
	// See Also:
	//     MakeCharacter
	// Arguments:
	//     pCryCharInstance - Pointer to the character instance class to delete
	//     nFlags - Set how the model will be kept in memory after being 
	//                  used. Uses flags defined with EModelPersistence.
	// Summary:
	//     Free a character instance
	virtual void RemoveCharacter(ICryCharInstance * pCryCharInstance, unsigned nFlags = 0)=0;  

	// Description:
	//     Clear all decals on every characters. The result is the same as calling 
	//     ExecScriptCommand with the command CASCMD_CLEAR_DECALS.
	// Summary:
	//     Clear all decals on every characters
	virtual void ClearDecals() = 0;

	// Description:
	//     Cleans up all resources. Currently deletes all bodies and characters even if there are references on them.
	// Summary:
	//     Cleans up all resources 
	virtual void ClearResources(void) = 0;

	// Description:
	//     Execute a specified animation script command.
	// Arguments:
	//     nCommand - The command to execute, as declared in CryAnimationScriptCommandEnum
	//     pParams  - Any parameters for the command
	//     pResult  - Pointer where to store the result
	// Return Value:
	//     true if the command was executed,
	//     false if it wasn't executed
	// Summary:
	//     Executes a script command
	virtual bool ExecScriptCommand (int nCommand, void* pParams = NULL, void* pResult = NULL) {return false;}

	// Description:
	//     Update the Animation System. It's important to call this function at every frame. This should perform very fast.
	// Summary:
	//     Update the Animation System
	virtual void Update() = 0;

	//! The specified animation will be unloaded from memory; it will be loaded back upon the first invokation (via StartAnimation())

	// Description:
	//     Unload a specific animation from memory. If there's any attempt to 
	//     use it again, with ICryCharInstance::StartAnimation for example, it 
	//     will loaded back automatically.
	// See Also:
	//     StartLoadAnimation, ICryCharInstance::StartAnimation
	// Arguments:
	//     szFileName - Filename of the animation
	// Summary:
	//     Unload animation from memory
	virtual void UnloadAnimation(const char* szFileName) = 0;
	
	// Description:
	//     Start to load an animation file into memory. It allows to specify when the 
	//     animation will be needed in order to let the system delay loading.
	// See Also
	//     UnloadAnimation, ICryCharInstance::StartAnimation
	// Arguments:
	//     szFileName - Filename of the animation
	//     fWhenRequired - Timeout in seconds until the animation is needed
	// Summary:
	//     Start to load an animation file
	virtual void StartLoadAnimation (const char* szFileName, float fWhenRequired) = 0;

	// Description:
	//     Releases any resource allocated by the Animation System and shut it down properly.
	// Summary:
	//     Release the Animation System
	virtual void Release()=0;

	//! Locks all models in memory

	// Description:
	//     Lock all the models to stay loaded in memory.
	// See Also:
	//     UnlockResources
	// Summary:
	//     Lock all the models to stay loaded in memory.
	virtual void LockResources() = 0;

	// Description:
	//     Unlock all the models allow them to be unloaded from memory.
	// See Also:
	//     LockResources
	// Summary:
	//     Unlock all the models allow them to be unloaded from memory.
	virtual void UnlockResources() = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

#if (defined(GAMECUBE) || defined (PS2))
#define __TIMESTAMP__ "Ver1.0"
#endif

//#ifdef PS2
//#define __TIMESTAMP__ "Ver1.0"
//#endif

// expirimental way to track interface version 
// this value will be compared with value passed from system module
const char gAnimInterfaceVersion[64] = __TIMESTAMP__;

// CreateCryAnimation function type definition
typedef ICryCharManager * (*PFNCREATECRYANIMATION)(ISystem	* pSystem,const char * szInterfaceVersion);

// Description:
//     Create an instance of the Animation System. It should usually be called 
//     by ISystem::InitAnimationSystem().
// See Also:
//     ICryCharManager, ICryCharManager::Release
// Arguments:
//     ISystem            - Pointer to the current ISystem instance
//     szInterfaceVersion - String version of with the build date
// Summary:
//     Create an instance of the Animation System
CRYANIMATION_API ICryCharManager * CreateCharManager(ISystem* pSystem, const char * szInterfaceVersion=gAnimInterfaceVersion);

#ifdef __cplusplus
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ICRY_ANIMATION
