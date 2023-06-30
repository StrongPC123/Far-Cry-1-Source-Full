//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:IBindable.h
//  Interface for IBindable class, which must be implemented by all
//  objects that can be bound to a character bone
//
//	History:
//	April 07, 2003: Created by Sergiy Migdalskiy
//
//////////////////////////////////////////////////////////////////////

#ifndef _CRY_COMMON_BINDABLE_INTERFACE_HDR_
#define _CRY_COMMON_BINDABLE_INTERFACE_HDR_

// Description:
//     This interface define a way to allow an object to be bound to a 
//     character bone. An IBindable should usually be attached to a bone using 
//     member functions of ICryCharInstance.
struct IBindable
{
	// Summary:
	//     Get the bounding box
	// Arguments:
	//     Mins - Position of the bottom left close corner of the bounding box
	//     Maxs - Position of the top right far corner of the bounding box
	virtual void GetBBox(Vec3& Mins, Vec3& Maxs)=0;

	// Description:
	//     Will return the position of the helper named in the argument. The 
	//     helper should have been specified during the exporting process of 
	//     the cgf file.
	// Arguments:
	//     szHelperName - A null terminated string holding the name of the helper
	// Return Value:
	//     A Vec3 object which contains the position.
	// Summary:
	//     Gets the position of a specified helper
	virtual Vec3 GetHelperPos(const char * szHelperName) = 0;

	// Description:
	//     Will return the matrix of the helper named in the argument. The 
	//     helper should have been specified during the exporting process of 
	//     the cgf file.
	// Arguments:
	//     szHelperName - A null terminated string holding the name of the helper
	// Return Value:
	//     A Matrix44 of the object
	// Summary:
	//     Gets the matrix of a specified helper
	virtual const Matrix44 * GetHelperMatrixByName(const char * szHelperName) = 0;

	// renders the shadow volumes of this whole object (together with attachments if any)
	// the implementation may or may not implement the limit lod functionality: if it does,
	// it will render the specified or lower LOD
	virtual void RenderShadowVolumes(const struct SRendParams *pParams, int nLimitLod = -1)=0;

	//! Sets shader template for rendering
	virtual bool SetShaderTemplate(int nTemplate, const char *TemplName, const char *ShaderName, bool bOnlyRegister=false, int * pnNewTemplateId=NULL)=0;

	// Description:
	//     Values for the nFlags parameter of SetShaderTemplateName.
	// Summary:
	//     Flags used for SetShaderTemplateName
	enum ECharShaderFlags 
	{
		FLAGS_SET_SHADER_RECURSIVE = 1
	};

	//! Set shader template to be used with character
	virtual bool SetShaderTemplateName(const char *TemplName, int Id, const char *ShaderName=0,struct IMatInfo *pCustomMaterial=0,unsigned nFlags = 0)
	{
		// [Sergiy] please ask Tiago about details: this function maps to SetShaderTemplate because some of the derived classes (IStatObj)
		// don't implement it, but do implement SetShaderTemplate. The mapping is exactly like in ScriptObjectEntity::SetShader function
		return SetShaderTemplate(-1,TemplName,NULL);
	}

	//there must be only one function

	// Description:
	//     Registers the object elements into the renderer.
	// Arguments:
	//     rParams   - Render parameters
	//     nLogLevel - Level of the LOD
	// Summary:
	//     Renders the object
	virtual void Render(const struct SRendParams & rParams,const Vec3& t, int nLodLevel)=0;

	//! Start the specified animation with the given parameters, if the bindable is an animatable object
	virtual bool StartAnimation (const char* szAnimName, const struct CryCharAnimationParams& params){return false;}

	//! Start the specified by parameters morph target, if the bindable is a morphable object
	virtual void StartMorph (const char* szMorphTarget, const struct CryCharMorphParams& params) {}

	//! Resets all animation layers ( stops all animations )
	virtual void ResetAnimations() {}

	//! Stops all morphs
	virtual void StopAllMorphs() {}

	//! freezes all currently playing morphs at the point they're at
	virtual void FreezeAllMorphs(){}

	//! Processes skining (call this function every frame to animate character)
	//! dwFlags	is a bitwise OR of one of the flags specified in the UpdateEnum enumeration
	virtual void Update (Vec3 vPos = Vec3(0,0,0), float fRadius=0, unsigned uFlags = 0) {}

	//! start preloading of object resources
	virtual void PreloadResources(float fDist, float fTime, int dwFlags) = 0;

	// Summary:
	//     Get the character instance, if valid
	// Return Value:
	//     A pointer to an IStatObj object if the IBindable represent a static 
	//     object, else the NULL value will be returned.
	virtual struct IStatObj* GetIStatObj() {return NULL;}

	// Summary:
	//     Get the character instance, if valid
	// Return Value:
	//     A pointer to an ICryCharInstance object if the IBindable represent 
	//     a character instance, else the NULL value will be returned.
	virtual struct ICryCharInstance* GetICryCharInstance() {return NULL;}
};

#endif