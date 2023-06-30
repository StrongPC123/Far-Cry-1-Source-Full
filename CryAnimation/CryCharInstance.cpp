//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:CryCharInstance.cpp
//  Implementation of CryCharInstance class
//
//	History:
//	August 16, 2002: Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
//#include "CryAnimation.h"
#include <CryAnimationScriptCommands.h>
#include <Cry_Camera.h>
#include <I3DEngine.h>
#include <IEntityRenderState.h>

#include "CryModelState.h"
#include "CryCharInstance.h"
#include "CryCharManager.h"
#include "ControllerManager.h"
#include "cvars.h"
#include "CryCharBody.h"
#include "StringUtils.h"
#include "DebugUtils.h"

#include "MathUtils.h"
#include "CryModelSubmesh.h"

#include "CryCharAnimationParams.h"
#include "CryCharMorphParams.h"

#include "CryCharFxTrail.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace CryStringUtils;

CryCharInstance::CryCharInstance (CryCharBody * pBody):
	m_pCryCharBody        (pBody),
	m_fLastAnimUpdateTime (0),
	m_bEnableStartAnimation (true)
{
	m_sShaderTemplateName[0][0] = 0;
	m_sShaderTemplateName[1][0] = 0;
	m_nShaderTemplateFlags = 0;
  m_Color = CFColor(1.0f);

	m_pModelState = m_pCryCharBody->GetModel()->m_pDefaultModelState->MakeCopy();

	if(!m_pModelState)
	{
		g_GetLog()->LogToFile("  Unable To Copy DefaultModelState For Model [%s]", pBody->GetFilePathCStr());
		return;
	}

	m_v3pvSpitFirePos(0,0,0);
	m_v3pvSpitFirePosTranslated(0,0,0);

	ResetAnimations();

	m_nFlags = 0;

	m_fAnimSpeedScale = 1;

	//  m_matAttachedObjectMatrix.Clear();


	pBody->RegisterInstance (this);

}

CryCharInstance::~CryCharInstance()
{ 
	// GetRenderer()->DeleteModelState(m_pModelState);
	delete m_pModelState;
	m_pModelState = NULL;

	m_pCryCharBody->UnregisterInstance(this);
}


std::vector<String> AnimStrings;
std::vector<uint32> FrameID;
std::vector<uint32> LayerID;

bool CryCharInstance::StartAnimation (const char* szAnimName, const struct CryCharAnimationParams& Params)
{

	FUNCTION_PROFILER( g_GetISystem(),PROFILE_ANIMATION );
/*
	float fColor[4] = {0,1,0,1};
	String TestName = "objects\\characters\\story_characters\\valerie\\valeri.cgf";
	String ModelName = GetBody()->GetFileName();
	if (TestName==ModelName) {

		float fColor[4] = {1,0,1,1};
		//	String TestName = "awalkfwd";
		String TestName = "aidle";
		String AnimName = szAnimName;
		//if (TestName==AnimName) {
			FrameID.push_back(g_nFrameID);
			LayerID.push_back(Params.nLayerID);
			AnimStrings.push_back(AnimName);
		//}

	}*/



	if (!m_bEnableStartAnimation)
	{
		// this error happens extremely rarely, and it leads to unpleasant results: dead characters standing etc.
		// so we try to find all places where it was called from
		g_GetLog()->LogError ("\002%d. %p->StartAnimation(file=%s): %s, Layer=%u, blending:in=%.2f,out=%.2f  -  called while StartAnimation is disabled", g_nFrameID, this, m_pCryCharBody->GetNameCStr(), szAnimName, Params.nLayerID, Params.fBlendInTime, Params.fBlendOutTime);
		//GetSystem()->LogCallStack();
	}

	if(g_GetCVars()->ca_Debug())
		g_GetLog()->Log("\002%p->StartAnimation(file=%s): %s, Layer=%u, blending:in=%.2f,out=%.2f", this, m_pCryCharBody->GetNameCStr(), szAnimName, Params.nLayerID, Params.fBlendInTime, Params.fBlendOutTime);

	if (Params.nLayerID & ~0xF)
	{
		g_GetLog()->LogError("\002%p->StartAnimation(file=%s): %s, Layer=%u, blending:in=%.2f,out=%.2f: Layer is out of range", this, m_pCryCharBody->GetNameCStr(), szAnimName, Params.nLayerID, Params.fBlendInTime, Params.fBlendOutTime);
		return false;
	}

	bool bOk = false;
	if (m_pModelState->RunAnimation(szAnimName, Params, m_fAnimSpeedScale * g_GetCVars()->ca_UpdateSpeed()))
	{
		strncpy (m_sCurAnimation,szAnimName,sizeof(m_sCurAnimation));
		bOk = true;
	}

	if (Params.nFlags & Params.FLAGS_RECURSIVE)
	{
		for (BindArray::iterator it = m_arrBinds.begin(); it != m_arrBinds.end(); ++it)
			if ((*it)->pObj->StartAnimation (szAnimName, Params))
				bOk = true;
	}
	return true;
}


// FOR TEST ONLY enables/disables StartAnimation* calls; puts warning into the log if StartAnimation* is called while disabled
void CryCharInstance::EnableStartAnimation (bool bEnable)
{
  m_bEnableStartAnimation = bEnable;
}

//! Start the specified by parameters morph target
void CryCharInstance::StartMorph (const char* szMorphTarget,const CryCharMorphParams& Params)
{
	m_pModelState->RunMorph (szMorphTarget, Params);
	if (Params.nFlags & CryCharMorphParams::FLAGS_RECURSIVE)
	{
		for (BindArray::iterator it = m_arrBinds.begin(); it != m_arrBinds.end(); ++it)
			(*it)->pObj->StartMorph(szMorphTarget, Params);
	}
}

//! Start the specified by parameters morph target
void CryCharInstance::StartMorph (int nMorphTargetId, const CryCharMorphParams& Params)
{
	m_pModelState->GetSubmesh(0)->StartMorph (nMorphTargetId, Params);
}

//! Finds the morph with the given id and sets its relative time.
//! Returns false if the operation can't be performed (no morph)
bool CryCharInstance::SetMorphTime (int nMorphTargetId, float fTime)
{
	return m_pModelState->GetSubmesh(0)->SetMorphTime (nMorphTargetId, fTime);
}

//! Stops the animation at the specified layer. Returns true if there was some animation on that layer, and false otherwise
bool CryCharInstance::StopAnimation (int nLayer)
{
	if (g_GetCVars()->ca_OverrideLayer())
		nLayer = g_GetCVars()->ca_OverrideLayer();

	if (nLayer & ~0xF)
	{
		g_GetLog()->LogError("\002%d. %p->StopAnimation(file=%s): Layer=%u: Layer is out of range", g_nFrameID, this, m_pCryCharBody->GetNameCStr(), nLayer);
		return false;
	}

	if(g_GetCVars()->ca_Debug())
		g_GetLog()->Log("\004%d. %p->StopAnimation(layer=%u): model %s", g_nFrameID, this, nLayer, m_pCryCharBody->GetNameCStr());

	return m_pModelState->StopAnimation (nLayer);
}


bool CryCharInstance::StopMorph (int nMorphTargetId)
{
	return m_pModelState->GetSubmesh(0)->StopMorph(nMorphTargetId);
}

void CryCharInstance::StopAllMorphs()
{
	m_pModelState->StopAllMorphs();
	for (BindArray::iterator it = m_arrBinds.begin(); it != m_arrBinds.end(); ++it)
		(*it)->pObj->StopAllMorphs();
}

//! freezes all currently playing morphs at the point they're at
void CryCharInstance::FreezeAllMorphs()
{
	m_pModelState->FreezeAllMorphs();
	for (BindArray::iterator it = m_arrBinds.begin(); it != m_arrBinds.end(); ++it)
		(*it)->pObj->FreezeAllMorphs();
}


//! Enables/Disables the Default Idle Animation restart.
//! If this feature is enabled, then the last looped animation will be played back after the current (non-loop) animation is finished.
//! Only those animations started with the flag bTreatAsDefaultIdleAnimation == true will be taken into account
void CryCharInstance::EnableLastIdleAnimationRestart (unsigned nLayer, bool bEnable)
{
	m_pModelState->EnableIdleAnimationRestart (nLayer, bEnable);
}

// sets the given aniimation to the given layer as the default
void CryCharInstance::SetDefaultIdleAnimation (unsigned nLayer, const char* szAnimName)
{
	m_pModelState->SetDefaultIdleAnimation(nLayer, szAnimName);
}

// Checks if the animation with the given name exists, returns true if it does
bool CryCharInstance::IsAnimationPresent(const char* szAnimName)
{
	return m_pCryCharBody->GetModel()->findAnimation(szAnimName) >= 0;
}


const char * CryCharInstance::GetCurAnimation()
{
	if(m_pModelState->IsAnimStopped())
		return 0;

	return m_sCurAnimation;
}

//! Returns the current animation in the layer or -1 if no animation is being played 
//! in this layer (or if there's no such layer)
int CryCharInstance::GetCurrentAnimation (unsigned nLayer)
{
	return m_pModelState->GetCurrentAnimation (nLayer);
}


const Vec3d CryCharInstance::GetCenter() 
{ 
	return m_pModelState->GetCenter();
}

const float CryCharInstance::GetRadius() 
{ 
	Vec3d vSize = m_pModelState->m_BBox.getSize();
	if(vSize.z>=32)
		g_GetLog()->LogWarning ("CryCharInstance::GetRadius: bbox is very big: %s (%.2f)", 
		m_pCryCharBody->GetNameCStr(), vSize.z);
	return max(vSize.x,max(vSize.y,vSize.z))*0.5f;
}


/////////////////////////////////////////////////////////////////////////////////
// Attaches a static object to the bone (given the bone name) or detaches previous object from the bone.
// Only one static object can be attached to one bone at a time.
// If pWeaponModel is NULL, detaches any object from the given bone
// Does nothing if there is no bone with the given name.
ICryCharInstance::ObjectBindingHandle CryCharInstance::AttachObjectToBone(IBindable * pWeaponModel, const char * szBoneName, bool bUseRelativeToDefPoseMatrix, unsigned nFlags  )
{
	if(!szBoneName)
	{
		// if you find this assert, this means someone passed here a model and NO bone to attach it to. What should it mean anyway??
		assert (!pWeaponModel);
		// just detach everything
		DetachAll();
		return nInvalidObjectBindingHandle;
	}

	int nBone = m_pCryCharBody->GetModel()->findBone (szBoneName);
	if(nBone < 0)
	{
		if (!pWeaponModel)
		{
			// this is a severe bug if the bone name is invalid and someone tries to detach the model
			// if we find such a situation, we should try to detach the model, as it might be destructed already
			
			// but now we just detach everything, as it's simpler
			//m_arrBoundObjects.clear();
			//m_arrBoundObjectIndices.clear();

			g_GetLog()->LogError ("\002AttachObjectToBone is called for bone \"%s\", which is not in the model \"%s\". Ignoring, but this may cause a crash because the corresponding object won't be detached after it's destroyed", szBoneName, m_pCryCharBody->GetFilePathCStr());
#ifdef _DEBUG
			// this assert will only happen if the ca_NoAttachAssert is off
			// assert (g_GetCVars()->ca_NoAttachAssert());
#endif
		}
		return nInvalidObjectBindingHandle; // bone not found, do nothing
	}

	// detach all objects from this bone before creating a new one
	DetachAllFromBone(nBone);

	if(pWeaponModel == NULL)
	{
		// we didn't create a new binding, so return invalid handle
		return nInvalidObjectBindingHandle;
	}
	else
	{
		return AttachToBone(pWeaponModel, nBone, nFlags);
	}
}


//////////////////////////////////////////////////////////////////////////
// detaches all objects from bones; returns the nubmer of bindings deleted
unsigned CryCharInstanceBase::DetachAll()
{
	unsigned numDetached = (unsigned)m_arrBinds.size();
	for (BindArray::iterator it = m_arrBinds.begin(); it != m_arrBinds.end(); ++it)
		delete *it;
	m_arrBinds.clear();

	return numDetached;
}


//////////////////////////////////////////////////////////////////////////
// detach all bindings to the given bone; returns the nubmer of bindings deleted
unsigned CryCharInstanceBase::DetachAllFromBone(unsigned nBone)
{
	unsigned numDetached = 0;
	for (BindArray::iterator it = m_arrBinds.begin(); it != m_arrBinds.end(); )
		if ((*it)->nBone == nBone)
		{
			delete *it;
			it = m_arrBinds.erase (it);
			++numDetached;
		}
		else
			++it; // keep it

	return numDetached;
}


//////////////////////////////////////////////////////////////////////////
// attaches the object to the given bone. The returned value is the handle of the binding,
// that can be used to examine the binding and delete it
CryCharInstanceBase::ObjectBindingHandle CryCharInstanceBase::AttachToBone (IBindable*pObj, unsigned nBone, unsigned nFlags)
{
	// create a new binding
	if (pObj)
	{
		StatObjBind* pNewBind = new StatObjBind(pObj, nBone,nFlags);
		m_arrBinds.push_back(pNewBind);
		return (ObjectBindingHandle)pNewBind;
	}
	else
	{
		return nInvalidObjectBindingHandle;
	}
}


//////////////////////////////////////////////////////////////////////////
// detaches the given binding; if it returns false, the binding handle is invalid
// the binding becomes invalid immediately after detach
bool CryCharInstanceBase::Detach (ObjectBindingHandle nHandle)
{
	assert (IsHeapValid());
	for (BindArray::iterator it = m_arrBinds.begin(); it != m_arrBinds.end(); )
		if (nHandle == (ObjectBindingHandle)*it)
		{
			delete *it;
			it = m_arrBinds.erase (it);
			assert (IsHeapValid());
			return true;
		}
		else
			++it;
	return false;
}


//////////////////////////////////////////////////////////////////////////
// checks if the given binding is valid
bool CryCharInstanceBase::IsBindingValid (ObjectBindingHandle nHandle)
{
	for (BindArray::iterator it = m_arrBinds.begin(); it != m_arrBinds.end(); ++it)
		if (nHandle == (ObjectBindingHandle)*it)
			return true;
	return false;
}



//////////////////////////////////////////////////////////////////////////
//! attach a light to a bone
ICryCharInstance::LightHandle CryCharInstance::AttachLight (CDLight* pDLight, unsigned nBone, bool bCopyLight)
{
	if (nBone < m_pCryCharBody->GetModel()->numBoneInfos())
		return (LightHandle)m_pModelState->AddDynBoundLight(this, pDLight, nBone, bCopyLight);
	return InvalidLightHandle;
}

//////////////////////////////////////////////////////////////////////////
//! detach the light from the bone
void CryCharInstance::DetachLight (CDLight* pDLight)
{
	m_pModelState->RemoveDynBoundLight (pDLight);
}

//////////////////////////////////////////////////////////////////////////
//! Attach a light (copying the light actually) to the bone
//! Returns the handle identifying the light. With this handle, you can either
//! Retrieve the light information or detach it.
ICryCharInstance::LightHandle CryCharInstance::AttachLight (const CDLight& rDLight, const char* szBoneName)
{
	int nBone = m_pCryCharBody->GetModel()->findBone (szBoneName);
	if (nBone >= 0)
		// the rDLight doesn't get modified because we pass true for copy light; it gets copied
		return (LightHandle)m_pModelState->AddDynBoundLight(this, (CDLight*)&rDLight, nBone, true);
	else
		return InvalidLightHandle;
}

//////////////////////////////////////////////////////////////////////////
//! Detaches the light by the handle retuned by AttachLight
void CryCharInstance::DetachLight (LightHandle nHandle)
{
  m_pModelState->RemoveDynBoundLight ((CDLight*)nHandle);
}

//////////////////////////////////////////////////////////////////////////
//! Returns the light by the light handle; returns NULL if no such light found
CDLight* CryCharInstance::GetLight(LightHandle nHandle)
{
	if (m_pModelState->IsDynLightBound ((CDLight*)nHandle))
		return (CDLight*)nHandle;
	else
		return NULL;
}

ICryCharInstance::LightHandle CryCharInstance::GetLightHandle (CDLight* pLight)
{
	if (m_pModelState->IsDynLightBound(pLight))
		return (LightHandle)pLight;
	else
		return InvalidLightHandle;
}

void CryCharInstance::ResetAnimations() 
{  
	if (g_GetCVars()->ca_Debug())
		g_GetLog()->Log ("\002%d. %p->ResetAnimations (file=%s)", g_nFrameID, this, m_pCryCharBody->GetNameCStr());
	m_sCurAnimation[0] = 0;
	m_pModelState->ResetAllAnimations();
	m_fLastAnimUpdateTime = -1;

	for (BindArray::iterator it = m_arrBinds.begin(); it != m_arrBinds.end(); ++it)
		(*it)->pObj->ResetAnimations();
}

ICryBone * CryCharInstance::GetBoneByName(const char * szName)
{
	return m_pModelState->GetBoneByName(szName);
}

CLeafBuffer * CryCharInstance::GetLeafBuffer()
{
	return m_pModelState->GetLeafBuffer();
}


//! Returns position of specified helper ( exported into cgf file )
//! Actually returns the given bone's position
//! Default implementation: 000
Vec3d CryCharInstance::GetHelperPos(const char * szHelperName)
{
	int nBone = m_pCryCharBody->GetModel()->findBone(szHelperName);

	if (nBone < 0)
		return Vec3d(0,0,0);

	return m_pModelState->getBoneMatrixGlobal(nBone).GetTranslationOLD();
}
//! Returns the matrix of the specified helper ( exported into cgf file )
//! Actually returns the given bone's matrix
const Matrix44 * CryCharInstance::GetHelperMatrixByName(const char * szHelperName)
{
	int nBone = m_pCryCharBody->GetModel()->findBone(szHelperName);

	if (nBone < 0)
#ifdef _DEBUG
		return NULL;
#else
	{
		static Matrix44 mtxDefault;
		// for reliability, in release/profile builds, we return a valid pointer for now; in the future, it may change
		return &mtxDefault;
	}
#endif

	return &m_pModelState->getBoneMatrixGlobal(nBone);
}

Vec3d CryCharInstance::GetTPVWeaponHelper(const char * szHelperName, ObjectBindingHandle nHandle)
{ 
	if (!IsBindingValid(nHandle))
	{
		if (nHandle!= nInvalidObjectBindingHandle)
			g_GetLog()->LogWarning ("\003CryCharInstance::GetTPVWeaponHelper(%s,0x%0X): Invalid binding handle", szHelperName, nHandle);
		return Vec3d(0,0,0);
	}

	IBindable* pBoundObject = ((StatObjBind*)nHandle)->pObj;
	unsigned nBone = ((StatObjBind*)nHandle)->nBone;
	assert (nBone < m_pModelState->numBones());
	assert (pBoundObject);

	Vec3d vPos = pBoundObject->GetHelperPos (szHelperName);

	//Matrix t (m_matTranRotMatrix);

	const Matrix44& matAttachedObjectMatrix = m_pModelState->getBoneMatrixGlobal(nBone); // *t
	Matrix34 m34=Matrix34(GetTransposed44(matAttachedObjectMatrix));

	if (g_GetCVars()->ca_Debug()){
		CryAABB caabb;
		pBoundObject->GetBBox(caabb.vMin, caabb.vMax);
		debugDrawBBox (m34, caabb, 4);
	}

	return m34*vPos;
}


bool CryCharInstance::GetTPVWeaponHelperMatrix(const char * szHelperName, ObjectBindingHandle nHandle, Matrix44& matOut)
{ 
	if (!IsBindingValid(nHandle))
	{
		assert (0);// the binding handle is invalid!
		return false;
	}

	IBindable* pBoundObject = ((StatObjBind*)nHandle)->pObj;
	unsigned nBone = ((StatObjBind*)nHandle)->nBone;
	assert (nBone < m_pModelState->numBones());
	assert (pBoundObject);

	const Matrix44* pHelperMatrix = pBoundObject->GetHelperMatrixByName(szHelperName);

	const Matrix44& matAttachedObjectMatrix = m_pModelState->getBoneMatrixGlobal(nBone); // *t

	if (g_GetCVars()->ca_Debug()){
		Matrix34 m34=Matrix34(GetTransposed44(matAttachedObjectMatrix));
		CryAABB caabb;
		pBoundObject->GetBBox(caabb.vMin, caabb.vMax);
		debugDrawBBox (m34, caabb, 4);
	}
	matOut = *pHelperMatrix * matAttachedObjectMatrix;
	return true;
}


//////////////////////////////////////////////////////////////////////
void CryCharInstance::RenderShadowVolumes(const SRendParams *rParams, int nLimitLOD)
{
	DEFINE_PROFILER_FUNCTION();
	if (g_GetCVars()->ca_NoDrawShadowVolumes())
		return;

  if (m_pModelState && g_GetCVars()->ca_EnableCharacterShadowVolume())
	//if (m_pModelState && (!(rParams->dwFlags & RPF_NOANIMSHADOWS)) && g_GetCVars()->ca_EnableCharacterShadowVolume())
		m_pModelState->RenderShadowVolumes(rParams, nLimitLOD);

	if (!m_arrBinds.empty())
	{
		DEFINE_PROFILER_SECTION("BoundObjectShadowVolumes");
		//render binded object's shadow volumes
		for (BindArray::iterator it = m_arrBinds.begin(); it != m_arrBinds.end(); ++it)
		{
			unsigned nBone = (*it)->nBone;
			assert (nBone < m_pCryCharBody->GetModel()->numBoneInfos());
			IBindable* pBoundObject = (*it)->pObj;
			assert (pBoundObject);

			if (pBoundObject)
			{ // get weapon position


				Matrix44 t = Matrix34::CreateRotationXYZ( Deg2Rad(rParams->vAngles), rParams->vPos + m_pModelState->m_vOffset );
				t	=	GetTransposed44(t); //TODO: remove this after E3 and use Matrix34 instead of Matrix44


				Matrix44 matAttachedObjectMatrix = m_pModelState->getBoneMatrixGlobal(nBone) * t;

				SRendParams rBindParms (*rParams);
				rBindParms.pMatrix = &matAttachedObjectMatrix;

				if (g_GetCVars()->ca_DrawBBox()) {
					Matrix34 m34=Matrix34(GetTransposed44(matAttachedObjectMatrix));
					CryAABB caabb;
					pBoundObject->GetBBox(caabb.vMin, caabb.vMax);
					debugDrawBBox (m34, caabb, g_GetCVars()->ca_DrawBBox());
				}
				pBoundObject->RenderShadowVolumes (&rBindParms);
			}  
		} //i
	}
}

void CryCharInstance::GetBBox(Vec3d& vMin, Vec3d& vMax)
{
	m_pModelState->GetBoundingBox(vMin, vMax);
}

/*
void CryCharInstance::SetAnimationSinkForInstance (const char * szAnimName, ICharInstanceSink * pCharInstanceSink) 
{
	m_pModelState->SetAnimationSinkInstance (szAnimName, pCharInstanceSink);
}
*/

void CryCharInstance::BuildPhysicalEntity(IPhysicalEntity *pent,float mass,int surface_idx,float stiffness_scale, int nLod)
{
	m_pModelState->BuildPhysicalEntity(pent,mass,surface_idx,stiffness_scale, 1.0f,m_pModelState->m_vOffset, nLod);
}

IPhysicalEntity *CryCharInstance::CreateCharacterPhysics(IPhysicalEntity *pHost, float mass,int surface_idx,float stiffness_scale, int nLod)
{
	return m_pModelState->CreateCharacterPhysics(pHost,mass,surface_idx,stiffness_scale, 1.0f,m_pModelState->m_vOffset, nLod);
}

int CryCharInstance::CreateAuxilaryPhysics(IPhysicalEntity *pHost, int nLod)
{
	return m_pModelState->CreateAuxilaryPhysics(pHost,1.0f,m_pModelState->m_vOffset,nLod);
}

void CryCharInstance::SynchronizeWithPhysicalEntity(IPhysicalEntity *pent, const Vec3& posMaster,const Quat& qMaster)
{
#if defined(LINUX)
	if(!pent || !m_pModelState)
		return;
#endif
	m_pModelState->SynchronizeWithPhysicalEntity(pent, posMaster,qMaster,m_pModelState->m_vOffset);
	m_pModelState->UpdateBBox();
}

IPhysicalEntity *CryCharInstance::RelinquishCharacterPhysics() 
{
	return m_pModelState->RelinquishCharacterPhysics();
}

void CryCharInstance::SetCharacterPhysParams(float mass,int surface_idx)
{
	m_pModelState->SetCharacterPhysParams(mass,surface_idx,1.0f);
}

void CryCharInstance::SetLimbIKGoal(int limbid, vectorf ptgoal, int ik_flags, float addlen, vectorf goal_normal)
{
	m_pModelState->SetLimbIKGoal(limbid,1.0f, ptgoal,ik_flags,addlen,goal_normal);
}

vectorf CryCharInstance::GetLimbEndPos(int limbid)
{
	return m_pModelState->GetLimbEndPos(limbid, 1.0f);
}

void CryCharInstance::AddImpact(int partid, vectorf point,vectorf impact)
{
	m_pModelState->AddImpact(partid,point,impact, 1.0f);
}

int CryCharInstance::TranslatePartIdToDeadBody(int partid)
{
	return m_pModelState->TranslatePartIdToDeadBody(partid);
}

bool CryCharInstance::IsAnimStopped()
{
	return m_pModelState->IsAnimStopped();
}

vectorf CryCharInstance::GetOffset()
{
	return (vectorf)m_pModelState->m_vOffset;
}

void CryCharInstance::SetOffset(vectorf offset)
{
	m_pModelState->m_vOffset = (Vec3d)offset;
}

void CryCharInstance::SetTwiningMode (AnimTwinMode eTwinMode)
{
}

int CryCharInstance::GetDamageTableValue(int nId)
{
	return m_pModelState->GetDamageTableValue(nId);
}

//! Enable object animation time update. If the bUpdate flag is false, subsequent calls to Update will not animate the character
void CryCharInstance::EnableTimeUpdate (bool bUpdate)
{
	if (bUpdate)
		m_fAnimSpeedScale = 1;
	else
		m_fAnimSpeedScale = 0;
}

//! Set the current time of the given layer, in seconds
void CryCharInstance::SetLayerTime (int nLayer, float fTimeSeconds)
{
	m_pModelState->SetLayerTime (nLayer, fTimeSeconds);
}
//////////////////////////////////////////////////////////////////////////
float CryCharInstance::GetLayerTime (int nLayer)
{
	return m_pModelState->GetLayerTime(nLayer);
}

// calculates the mask ANDed with the frame id that's used to determine whether to skin the character on this frame or not.
int CryCharInstance::GetUpdateFrequencyMask(Vec3 vPos, float fRadius)
{
	
	//float red[4] = {1,0,0,1};
	//debugDrawSphere(Matrix34::CreateTranslationMat(vPos), fRadius*4,red);

	// on dedicated server, this path will always be taken;
	// on normal clients, this will always be rejected
  if(g_bUpdateBonesAlways)
		return 0;

	float fZoomFactor = 0.01f+0.99f*(RAD2DEG(GetViewCamera().GetFov())/90.f);  

	float fScaledDist = GetDistance(vPos,GetViewCamera().GetPos())*fZoomFactor;

	int iMask = 7;  //don't update bones

	if( fRadius==0  ||  (fScaledDist<(64.f+fRadius) && GetViewCamera().IsSphereVisibleFast( Sphere(vPos,fRadius*4) )))
		iMask = 0;  //if close to camera AND is frustum
	else 
		if(fScaledDist<64.f)	iMask = 3; //close to camera but outside of frustum

//	float col[4] = {0,1,0,1};
//	extern float g_YLine;
//	g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, col, false,"fZoomFactor:%15.10f  fScaledDist:%15.10f  iMask: %d",fZoomFactor,fScaledDist,iMask );	g_YLine+=16.0f;
//	Matrix44 m44 = m_pModelState->m_ModelMatrix44;
//	Vec3 t=m44.GetTranslationOLD();
//	g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, col, false,"vPos:(%15.10f,%15.10f,%15.10f)  t:(%15.10f,%15.10f,%15.10f)",vPos.x,vPos.y,vPos.z, t.x,t.y,t.z);	g_YLine+=16.0f;

	return iMask;
}

//! Updates the bones and the bounding box. Should be called if animation update
//! cycle in EntityUpdate has already passed but you need the result of new animatmions
//! started after Update right now.
void CryCharInstance::ForceUpdate()
{
	m_pModelState->ResetBBoxCache();
	m_pModelState->ProcessAnimations(0, true,this);
}

void CryCharInstance::Update(Vec3d vPos, float fRadius, unsigned uFlags)
{
	DEFINE_PROFILER_FUNCTION();

	SelfValidate();
	// the current time
	float fAnimUpdateTime = g_GetTimer()->GetCurrTime();
	// the current delta time to add to the current animation time
	float fFrameTime;
	
	// for the first-time call, pretend that the last time it was called with exactly the same time
	if (m_fLastAnimUpdateTime <= 0)
	{
		// the first update must be 0-time-range
		fFrameTime = 0;
	}
	else
	{
		// calculate the delta 
		fFrameTime = (fAnimUpdateTime - m_fLastAnimUpdateTime) * m_fAnimSpeedScale;

		// the time should actually go forward
		//assert(fFrameTime >= 0);

		if (fFrameTime <= 0) {
			//just in case we reset the timer
			//in the next frame everything will be ok
			m_fLastAnimUpdateTime = fAnimUpdateTime;
			return; // no need to update twice
		}
	}


	// calculate dampering factors for optimization: depending on how far the character from the player is,
	// the bones may be updated once per several frames


#ifndef _DEBUG
	int nFrameID = g_GetIRenderer()->GetFrameID();
	int nUFM = GetUpdateFrequencyMask(vPos,fRadius);

	bool update=(nFrameID & nUFM)==(m_pModelState->getInstanceNumber()&nUFM);

	//float fColor[4] = {0,1,0,1};
	//g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"InstanceNum:%d  nFrameID:%d (%01d %01d) fRadius:%15.10f ",m_pModelState->getInstanceNumber(), nFrameID, update,nUFM, fRadius );	g_YLine+=16.0f;
	


	if ( update || (fFrameTime>0.25f) )
#endif
	{
		m_pModelState->ProcessAnimations(fFrameTime * g_GetCVars()->ca_UpdateSpeed(), (uFlags & flagDontUpdateBones) == 0, this);
/*
	{
			char str[256];
			sprintf(str,"%p m_fLastAnimUpdateTime=%.2f \n",this,fAnimUpdateTime);
			OutputDebugString(str);
		}
*/
		m_fLastAnimUpdateTime = fAnimUpdateTime;
	}

	if (0==(uFlags & flagDontUpdateAttachments))
	{
		for (BindArray::iterator it = m_arrBinds.begin(); it != m_arrBinds.end(); ++it)
			(*it)->pObj->Update(vPos, fRadius, uFlags);
	}

}

void CryCharInstance::UpdatePhysics( float fScale )
{
	//PROFILE_FRAME(CharacterPhysicsUpdate);
	if (fabsf(g_GetTimer()->GetCurrTime()-m_fLastAnimUpdateTime)<0.001f)	// animation was updated this frame, so update physics as well
	{
		m_pModelState->ProcessPhysics(0.01f, (int)m_pModelState->m_arrAnimationLayers.size());
		m_pModelState->UpdateBBox();
	}
}


// checks for possible memory corruptions in this object and its children
void CryCharInstance::SelfValidate ()const
{
#ifdef _DEBUG
	m_pModelState->SelfValidate();
#endif
}

//////////////////////////////////////////////////////////////////////
// todo: get rid of it or implement it
bool CryCharInstance::SetAnimationFrame(const char * szString, int nFrame)
{
	return m_pModelState->SetAnimationFrame(szString, nFrame);
}

bool CryCharInstance::IsCharacterActive()
{
	return m_pModelState->IsCharacterActive();
}

void CryCharInstance::SetShaderFloat(const char *Name, float Val, const char *ShaderName)
{
	m_pModelState->SetShaderFloat(Name, Val, ShaderName);
}

void CryCharInstance::SetColor(float fR, float fG, float fB, float fA)
{
  m_Color.r = fR;
  m_Color.g = fG;
  m_Color.b = fB;
  m_Color.a = fA;
}

const char * CryCharInstance::GetShaderTemplateName()
{
	list2<CMatInfo>* pMats = m_pModelState->getLeafBufferMaterials();
  
  if(pMats && pMats->size())
  {
    SShaderItem si = (*pMats)[0].shaderItem;
		if (si.m_pShader)
			return si.m_pShader->GetTemplate(0)->GetName();
  }

  return ""; 
}

bool CryCharInstance::SetShaderTemplateName (const char *TemplName, int Id, const char *ShaderName,IMatInfo *pCustomMaterial,unsigned nFlags)
{
  assert (Id <= 1);

  if(TemplName)
  {
    if (!strcmp (m_sShaderTemplateName[Id], TemplName))
      return true;
    strncpy (m_sShaderTemplateName[Id], TemplName, sizeof(m_sShaderTemplateName[0]));
  }
  else
    m_sShaderTemplateName[Id][0] = 0;

  assert (Id <= 1);

	m_nShaderTemplateFlags = nFlags;

//--------------------------------------------------------------------

	bool val=m_pModelState->SetShaderTemplateName (TemplName, Id, ShaderName, pCustomMaterial,nFlags);

//--------------------------------------------------------------------

	if(nFlags & FLAGS_SET_SHADER_RECURSIVE)
	{
		for (BindArray::iterator it = m_arrBinds.begin(); it != m_arrBinds.end(); ++it)
			(*it)->pObj->SetShaderTemplateName(TemplName, Id, ShaderName, pCustomMaterial, nFlags);
	}
	
	return val;

}

	//! Sets shader template for rendering
bool CryCharInstance::SetShaderTemplate(int nTemplate, const char *TemplName, const char *ShaderName, bool bOnlyRegister, int * pnNewTemplateId)
{
	// This gets called when character are attached recursively to each other's bones
	// What to do?
	return true;
}

//! returns the leaf buffer materials in this character (as they are used in the renderer)
const list2<CMatInfo>*CryCharInstance::getLeafBufferMaterials()
{
	return m_pModelState->getLeafBufferMaterials();
}







void CryCharInstance::Draw(const SRendParams& RendParams, const Vec3& translation)
{
	//Vec3 trans=RendParams.vPos;
	//float fColor[4] = {0,1,0,1};
	//extern float g_YLine;
	//g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"draw: %15.10f %15.10f %15.10f %08x",trans.x,trans.y,trans.z,RendParams.pMatrix );	g_YLine+=16.0f;
	//g_YLine+=16.0f;
	Render (RendParams,translation,0);
}


//! Render object ( register render elements into renderer )
void CryCharInstance::Render(const struct SRendParams& RendParams, const Vec3& translation, int nLodLevel)
{

	m_pModelState->CalculateLOD(RendParams.fDistance);
	Vec3d vPos = RendParams.vPos;
	Vec3d vAngles = RendParams.vAngles;

/*{
	//debug code --- debug code --- debug code --- debug code ---
	String TestName = "objects\\characters\\animals\\greatwhiteshark\\greatwhiteshark_dead.cgf";
//	String TestName = "objects\\characters\\story_characters\\valerie\\valeri.cgf";
	String ModelName = GetBody()->GetFileName();
	if (TestName==ModelName) {
		float fColor[4] = {1,0,1,1};
	}
	float fColor[4] = {0,1,0,1};
	extern float g_YLine;
	g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"vPos:(%15.10f %15.10f %15.10f) ModelName: %s",vPos.x,vPos.y,vPos.z,ModelName.c_str() );	g_YLine+=16.0f;
}*/


/*	
	float fColor[4] = {0,1,0,1};
	extern float g_YLine;
	g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"ModelName: %s",ModelName );	g_YLine+=16.0f;
	g_YLine+=16.0f;
*/

	

	// make tran&rot matrix		
	if (RendParams.pMatrix)
		m_matTranRotMatrix = *RendParams.pMatrix;
	else
	{
		//OPTIMIZED_BY_IVO
		m_matTranRotMatrix = Matrix34::CreateRotationXYZ( Deg2Rad(vAngles), vPos + m_pModelState->m_vOffset );
		m_matTranRotMatrix =	GetTransposed44(m_matTranRotMatrix); //TODO: remove this after E3 and use Matrix34 instead of Matrix44
	}

	if (m_pModelState)
	{
		Matrix44 mtxObjMatrix;
		//check for a ready to be used matrix
		if (RendParams.pMatrix)
			mtxObjMatrix = *RendParams.pMatrix;
		else
			mathCalcMatrix(mtxObjMatrix, RendParams.vPos+m_pModelState->m_vOffset, vAngles, Vec3d(1,1,1), g_CpuFlags);


//		Vec3 trans=mtxObjMatrix.GetTranslationOLD();
/*		Vec3 trans=RendParams.vPos+m_pModelState->m_vOffset;
		float fColor[4] = {0,1,0,1};
		extern float g_YLine;
		g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"render trans: %15.10f %15.10f %15.10f %08x",trans.x,trans.y,trans.z,RendParams.pMatrix );	g_YLine+=16.0f;
		g_YLine+=16.0f;*/

		m_pModelState->Render (RendParams, mtxObjMatrix, *this, translation );

		if (g_GetCVars()->ca_DrawBones())
			m_pModelState->debugDrawBones (&m_matTranRotMatrix);

		if (int nBBoxSegments = g_GetCVars()->ca_DrawBBox())
			m_pModelState->debugDrawBoundingBox(&m_matTranRotMatrix, nBBoxSegments);

		// draw weapon and binded objects
		DrawBoundObjects(RendParams,m_matTranRotMatrix, /*FIXME:nLodLevel*/m_pModelState->m_nLodLevel);
	}

	if (g_GetCVars()->ca_DebugShaders())
		g_GetIRenderer()->DrawLabel (m_matTranRotMatrix.GetRow(3), 4, "Shaders: \"%s\", \"%s\"",m_sShaderTemplateName[0], m_sShaderTemplateName[1]);

}




//! marks all LODs as needed to be reskinned
void CryCharInstance::ForceReskin ()
{
	m_pModelState->ForceReskin();
}

// Interface for the renderer - returns the CDLight describing the light in this character;
// returns NULL if there's no light with such index
const CDLight* CryCharInstance::GetBoundLight (int nIndex)
{
	CDLight *pDL = NULL;
	if (m_pModelState  && nIndex >= 0)
  {
		unsigned numGlobalLights = m_pCryCharBody->GetModel()->numGlobalBoneLights();
		if ((unsigned)nIndex < numGlobalLights)
		{
			pDL = (CDLight *)m_pModelState->getGlobalBoundLight (nIndex);
		}
/*		else
		{
			unsigned numDynLights = m_pModelState->numDynBoundLights();
			if (nIndex-numGlobalLights < numDynLights)
			{
				pDL = m_pModelState->getDynBoundLight(nIndex-numGlobalLights);
			}
		}*/
  }
	return pDL;
}

void CryCharInstance::DrawBoundObjects(const SRendParams & rRendParams, Matrix44 &inmatTranRotMatrix, int nLOD)
{
	static const float fColorBBoxAttached[4] = {0.5,1,1,0.75};

	if (g_GetCVars()->ca_NoDrawBound())
		return;

	BindArray::iterator it, itEnd = m_arrBinds.end();
	for (it = m_arrBinds.begin(); it != itEnd; ++it)
	{
		int nBone =	(*it)->nBone;
		CryBone& rBone = m_pModelState->getBone(nBone);
		IBindable* pBoundObject = (*it)->pObj;

		if (!pBoundObject)
			continue;
		
    Matrix44 matAttachedObjectMatrix = rBone.GetGlobalMatrix() * inmatTranRotMatrix;
		if ((*it)->nFlags & FLAGS_ATTACH_ZOFFSET && !(rRendParams.dwFObjFlags&FOB_RENDER_INTO_SHADOWMAP))
		{
			// this is the offset to apply
			Vec3 vDelta = g_GetISystem()->GetViewCamera().GetPos() - matAttachedObjectMatrix.GetTranslationOLD();
			matAttachedObjectMatrix.AddTranslationOLD(vDelta * g_GetCVars()->ca_BoundZOffset());
		}

    SRendParams rParams (rRendParams);
    rParams.pMatrix = &matAttachedObjectMatrix;
		// this is required to avoid the attachments using the parent character material (this is the material that overrides the default material in the attachment)
		rParams.pMaterial = NULL;

		if (m_nFlags & CS_FLAG_DRAW_NEAR)
		{
			rParams.dwFObjFlags |= FOB_NEAREST;
		}

		if (g_GetCVars()->ca_DrawBBox()>1) {
			Matrix34 m34=Matrix34(GetTransposed44(matAttachedObjectMatrix));
			CryAABB caabb;
			pBoundObject->GetBBox(caabb.vMin, caabb.vMax);
			debugDrawBBox (m34, caabb, g_GetCVars()->ca_DrawBBox()-1,fColorBBoxAttached);
		}
		if(	rParams.nShaderTemplate<0 && // if this is not special rendering pass like render into shadow map
				m_sShaderTemplateName[0][0] && // if some shader template is set for this character
				m_nShaderTemplateFlags & FLAGS_SET_SHADER_RECURSIVE) // if it allowed to be set also to attachments
		{ // register this template into bound object and use for rendering
			int nTemplateId=rParams.nShaderTemplate;
			pBoundObject->SetShaderTemplate( rParams.nShaderTemplate, m_sShaderTemplateName[0] , 0, false, &nTemplateId );
			rParams.nShaderTemplate = nTemplateId;
		}

    pBoundObject->Render(rParams,Vec3(zero), nLOD);
  }  
}

IPhysicalEntity *CryCharInstance::GetCharacterPhysics() 
{ 
	return m_pModelState->GetCharacterPhysics(); 
}
IPhysicalEntity *CryCharInstance::GetCharacterPhysics(const char *pRootBoneName) 
{	
	return m_pModelState->GetCharacterPhysics(pRootBoneName); 
}
IPhysicalEntity *CryCharInstance::GetCharacterPhysics(int iAuxPhys) 
{ 
	return m_pModelState->GetCharacterPhysics(iAuxPhys); 
}
void CryCharInstance::DestroyCharacterPhysics(int iMode) 
{ 
	m_pModelState->DestroyCharacterPhysics(iMode); 
}

// Spawn decal on the character
void CryCharInstance::CreateDecal(CryEngineDecalInfo& DecalLCS)
{
	if (g_GetCVars()->ca_EnableDecals() && DecalLCS.fSize > 0.001f)
	{
		// The decal info in the local coordinate system
		CryEngineDecalInfo DecalLocal = DecalLCS;
		DecalLocal.fSize *= g_GetCVars()->ca_DecalSizeMultiplier() * float(0.4 + (rand()&0x7F)*0.001953125);
		DecalLocal.fAngle = (rand()&0xFF)*float(2*gPi/256.0f);
	/*		
#if 1
		Matrix matCharacter;
		matCharacter.Identity();
		matCharacter.SetTranslation (Decal.pDecalOwner->GetPos());
		Rotate (matCharacter, m_vAngles);
		Matrix matInvCharacter;
		//matInvCharacter.AssignInverseOf(matCharacter);
		matInvCharacter=GetInverted44(matCharacter);

#else
		Matrix matInvCharacter;
		matInvCharacter.Identity();
		RotateInv (matInvCharacter, m_vAngles);
		Vec3d ptWCS = Decal.pDecalOwner->GetPos();

		DecalLCS.vPos -= ptWCS;
		DecalLCS.vHitDirection -= ptWCS;
#endif
		DecalLCS.vPos = matInvCharacter.TransformPoint(DecalLCS.vPos);
		DecalLCS.vHitDirection = matInvCharacter.TransformVector(DecalLCS.vHitDirection);
		*/
		m_pModelState->AddDecal (DecalLocal);
	}
}

//! Returns the model interface
ICryCharModel* CryCharInstance::GetModel()
{
	return m_pCryCharBody;
}


//! Enables receiving OnStart/OnEnd of all animations from this character instance
//! THe specified sink also receives the additional animation events specified through AddAnimationEvent interface
void CryCharInstance::AddAnimationEventSink(ICharInstanceSink * pCharInstanceSink)
{
	// not supported yet
}

// removes the event sink for all animations
void CryCharInstance::RemoveAnimationEventSink(ICharInstanceSink * pCharInstanceSink)
{
  m_pModelState->removeAnimationEventSink(pCharInstanceSink);
}

void CryCharInstance::RemoveAllAnimationEvents()
{
	m_pModelState->removeAllAnimationEvents();
}


//////////////////////////////////////////////////////////////////////////
//! Enables receiving OnStart/OnEnd of specified animation from this character instance
//! The specified sink also receives the additional animation events specified through AddAnimationEvent interface for this animation
void CryCharInstance::AddAnimationEventSink(const char* szAnimName, ICharInstanceSink * pCharInstanceSink)
{
	int nAnimId = m_pCryCharBody->GetModel()->findAnimation(szAnimName);
	if (nAnimId < 0)
	{
		if (g_GetCVars()->ca_AnimWarningLevel() >= 2)
			g_GetLog()->LogWarning("CryCharInstance(0x%p)::AddAnimationEventSink(\"%s\",0x%p) - animation not found (model \"%s\")", this, szAnimName, pCharInstanceSink, m_pCryCharBody->GetFilePathCStr());
	}
	else
	{
		//g_GetLog()->LogToFile("\005CryCharInstance(0x%p)::AddAnimationEventSink(\"%s\",0x%p) - animation not found (model \"%s\")", this, szAnimName, pCharInstanceSink, m_pCryCharBody->GetFilePathCStr());
		m_pModelState->setAnimationEventSink (nAnimId, pCharInstanceSink);
	}
}

void CryCharInstance::RemoveAnimationEventSink(const char* szAnimName, ICharInstanceSink * pCharInstanceSink)
{
	int nAnimId = m_pCryCharBody->GetModel()->findAnimation(szAnimName);
	if (nAnimId < 0)
		g_GetLog()->LogError("CryCharInstance(0x%p)::RemoveAnimationEventSink(\"%s\",0x%p) - animation not found (model \"%s\")", this, szAnimName, pCharInstanceSink, m_pCryCharBody->GetFilePathCStr());
	else
	{
		//g_GetLog()->LogToFile("\005CryCharInstance(0x%p)::RemoveAnimationEventSink(\"%s\",0x%p) - animation not found (model \"%s\")", this, szAnimName, pCharInstanceSink, m_pCryCharBody->GetFilePathCStr());
		assert (m_pModelState->getAnimationEventSink (nAnimId) == pCharInstanceSink);
		m_pModelState->setAnimationEventSink (nAnimId, NULL);
	}
}


//! Adds an animation event; whenever the character plays the specified frame of the specified animation,
//! it calls back the animation event sinkreceiving OnEvent notification of specified animation for all instances using this model
bool CryCharInstance::AddAnimationEvent(const char * szAnimName, int nFrameID, AnimSinkEventData UserData)
{
	int nAnimId = m_pCryCharBody->GetModel()->findAnimation(szAnimName);
	m_pModelState->addAnimEvent(nAnimId, nFrameID, UserData);
	return true;
}

//! Deletes the animation event; from now on the sink won't be receiving the animation this event
bool CryCharInstance::RemoveAnimationEvent (const char* szAnimName, int nFrameID, AnimSinkEventData UserData)
{
	int nAnimId = m_pCryCharBody->GetModel()->findAnimation(szAnimName);

	m_pModelState->removeAnimEvent(nAnimId, nFrameID,UserData);
	return false;
}


// Returns true if this character was created from the file the path refers to.
// If this is true, then there's no need to reload the character if you need to change its model to this one.
bool CryCharInstance::IsModelFileEqual (const char* szFileName)
{
	string strPath = szFileName;
	UnifyFilePath(strPath);
#if defined(LINUX)
  return !stricmp (strPath.c_str(), m_pCryCharBody->GetFilePathCStr());
#else
  return !strcmp (strPath.c_str(), m_pCryCharBody->GetFilePathCStr());
#endif
}

void CryCharInstance::GetMemoryUsage(ICrySizer* pSizer)const
{
#if ENABLE_GET_MEMORY_USAGE
	SIZER_SUBCOMPONENT_NAME(pSizer, "Characters");
	if (!pSizer->Add (*this))
		return;
	pSizer->AddContainer (m_arrBinds);
	m_pModelState->GetSize (pSizer);
#endif
}
	
//! Sets up particle spawning. After this funtion is called, every subsequenc frame,
//! During the character deformation, particles will be spawned in the given characteristics.
//! The returned handle is to be used to stop particle spawning
int CryCharInstance::AddParticleEmitter(ParticleParams& rInfo, const CryParticleSpawnInfo& rSpawnInfo)
{
	return m_pModelState->getParticleManager()->add (rInfo, rSpawnInfo);
}

//! Stops particle spawning started with StartParticleSpawn that returned the parameter
//! Returns true if the particle spawn was stopped, or false if the handle is invalid
bool CryCharInstance::RemoveParticleEmitter(int nHandle)
{
	return m_pModelState->getParticleManager()->remove (nHandle);
}

void CryCharInstance::SetAnimationSpeed(int nLayer, float fSpeed)
{
	m_pModelState->setAnimationSpeed (nLayer, fSpeed);
}

//! Set morph speed scale
//! Finds the morph target with the given id, sets its morphing speed and returns true;
//! if there's no such morph target currently playing, returns false
bool CryCharInstance::SetMorphSpeed (int nMorphTargetId, float fSpeed)
{
	return m_pModelState->GetSubmesh(0)->SetMorphSpeed(nMorphTargetId, fSpeed);
}

//! Sets the character scale relative to the model
void CryCharInstance::SetScale (const Vec3d& vScale)
{
	m_pModelState->setScale (vScale);
}

//! cleans up all decals in this character
void CryCharInstance::ClearDecals()
{
	m_pModelState->ClearDecals();
}

//Executes a per-character script command
bool CryCharInstance::ExecScriptCommand (int nCommand, void* pParams, void* pResult)
{
	switch (nCommand)
	{
	case CASCMD_CLEAR_DECALS:
		m_pModelState->ClearDecals();
		break;

	case CASCMD_DUMP_DECALS:
		g_GetLog()->Log("\001Instance 0x%p", this);
		m_pModelState->DumpDecals();
		break;

	case CASCMD_DUMP_STATES:
		g_GetLog()->LogToFile("\001 Instance 0x%p, state 0x%p", this, m_pModelState);
#if defined(LINUX)
		g_GetLog()->LogToFile("\001  %d binds, speed %f, matrix:{%f,%f,%f,%f}{%f,%f,%f,%f}{%f,%f,%f,%f}{%f,%f,%f,%f}", m_arrBinds.size(), m_fAnimSpeedScale, m_matTranRotMatrix.GetData());
#else
		g_GetLog()->LogToFile("\001  %d binds, speed %f, matrix:{%g,%g,%g,%g}{%g,%g,%g,%g}{%g,%g,%g,%g}{%g,%g,%g,%g}", m_arrBinds.size(), m_fAnimSpeedScale, m_matTranRotMatrix);
#endif
		m_pModelState->DumpState();
		break;

	case CASCMD_START_MANY_ANIMS:
		{
			const CASCmdStartMultiAnims& ma = *(CASCmdStartMultiAnims*)(pParams);
			for (int i = 0; i < ma.numAnims; ++i)
			{
				const CASCmdStartAnim& sa = ma.pAnims[i];
				g_GetLog()->LogToConsole("\001Starting \"%s\" @ %d", sa.szAnimName, sa.nLayer);
				StartAnimation (sa.szAnimName, CryCharAnimationParams(ma.fBlendTime, ma.fBlendTime, sa.nLayer));
			}
		}
		break;

	case CASCMD_DEBUG_DRAW:
		m_pModelState->debugDrawBones();
		m_pModelState->debugDrawBoundingBox(NULL, 4);
		break;
	}
	return true;
}

// notifies the renderer that the character will soon be rendered
void CryCharInstance::PreloadResources ( float fDistance, float fTime, int nFlags )
{
	FUNCTION_PROFILER( g_GetISystem(),PROFILE_3DENGINE );
	if (m_pModelState)
		m_pModelState->PreloadResources(fDistance, fTime, nFlags);

	CryCharInstanceBase::PreloadResources (fDistance, fTime, nFlags);
}

void CryCharInstanceBase::PreloadResources ( float fDistance, float fTime, int nFlags )
{
	for (BindArray::iterator it = m_arrBinds.begin(); it != m_arrBinds.end(); ++it)
	{
		StatObjBind * pBind = (*it);
		if(pBind && pBind->pObj)
			pBind->pObj->PreloadResources(fDistance, fTime, nFlags);
	}
}

// adds a submesh, returns handle to it which can be used to delete the submesh
// submesh is created either visible or invisible
// submesh creation/destruction is heavy operations, so the clients must use they rarely,
// and set visible/invisible when they need to turn them on/off
// But creating many submeshes is memory-consuming so the number of them must be kept low at all times
ICryCharSubmesh* CryCharInstance::NewSubmesh (ICryCharModel* pModel, bool bVisible)
{
	return m_pModelState->AddSubmesh (pModel, bVisible);
}

// adds submesh to the specified slot; replaces submesh if there's some there
ICryCharSubmesh* CryCharInstance::NewSubmesh (unsigned nSlot, ICryCharModel* pModel, bool bVisible)
{
	return m_pModelState->SetSubmesh(nSlot, pModel, bVisible);
}


// removes submesh from the character
void CryCharInstance::RemoveSubmesh (ICryCharSubmesh* pSubmesh)
{
	return m_pModelState->RemoveSubmesh(pSubmesh);
}

void CryCharInstance::RemoveSubmesh (unsigned nSlot)
{
	return m_pModelState->RemoveSubmesh(nSlot);
}

// enumeration of submeshes
size_t CryCharInstance::NumSubmeshes()
{
	return m_pModelState->NumSubmeshes();
}
ICryCharSubmesh* CryCharInstance::GetSubmesh(unsigned i)
{
	return m_pModelState->GetSubmesh(i);
}

ICryCharFxTrail* CryCharInstance::NewFxTrail (unsigned nSlot, const struct CryCharFxTrailParams& rParams)
{
	return m_pModelState->NewFxTrail(nSlot, rParams);
}

void CryCharInstance::RemoveFxTrail(unsigned nSlot)
{
	m_pModelState->RemoveFxTrail(nSlot);
}

// returns the number of bindings; valid until the next attach/detach operation
size_t CryCharInstance::GetBindingCount()
{
	return m_arrBinds.size();
}

// fills the given array with  GetBindingCount() pointers to IBindable
void CryCharInstance::EnumBindables(IBindable** pResult)
{
	for (size_t i = 0; i < m_arrBinds.size(); ++i)
		pResult[i] = m_arrBinds[i]->pObj;
}
