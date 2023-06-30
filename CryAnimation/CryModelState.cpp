#include "stdafx.h"
#include <Cry_Camera.h>
#include <CryCharMorphParams.h>
#include <StlUtils.h>
#include "CryModel.h"
#include "CryModelState.h"
#include "ControllerManager.h"
#include "ChunkFileReader.h"
#include "StringUtils.h"
#include "CVars.h"
#include "CryCharDecalManager.h"
#include "BoneLightDynamicBind.h"
#include "CryModEffMorph.h"
#include "DebugUtils.h"
#include "MathUtils.h"
#include "CryModelSubmesh.h"
#include "CryCharBody.h"
#include "CryCharAnimationParams.h"
#include "CryCharFxTrail.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace CryStringUtils;

#define m_pMesh GetMesh()

CryModelState::ActiveLayerArray CryModelState::g_arrActiveLayers;

unsigned CryModelState::g_nInstanceCount = 0;

CryModelState::CryModelState (CryModel* pMesh):
#ifdef DEBUG_STD_CONTAINERS
	m_arrBones ("CryModelState.Bones"),
	m_arrBoneGlobalMatrices ("CryModelState.arrBoneGlobalMatrices"),
	m_arrAnimationLayers ("CryModelState.AnimationLayers"),
	m_arrMorphEffectors ("CryModelState.MorphTargets"),
	m_arrHeatSources ("CryModelState.HeatSources"),
	m_arrSinks ("CryModelState.Sinks"),
#endif
	m_vRuntimeScale(1,1,1),
	m_nLastSkinBBoxUpdateFrameId (0),
	m_pShaderStateCull(NULL),
	m_pShaderStateShadowCull(NULL)
#ifdef _DEBUG
	,m_bOriginalPose (true)
#endif
{
	m_uFlags = nFlagsNeedReskinAllLODs;
	m_ModelMatrix44.SetIdentity();
	m_nInstanceNumber = g_nInstanceCount++;
	m_nLastTangentsUpdatedFrameId = 0;
	m_nLastTangentsUpdatedLOD     = -1;

	SelfValidate();
	m_arrAnimationLayers.reserve(2);
	//m_pqLast.reset();
	//m_pBoneHead = m_pBoneSpine2 = m_pBoneLeftArm = NULL;
	m_pCharPhysics = 0;
	m_bHasPhysics = 0;
	for (int i = 0; i < 4; ++i) {
    m_pIKEffectors[i] = 0;
		m_IKpos0[i].zero();
	}
	m_vOffset(0,0,0);
	m_fScale = 0.01f;
	m_bPhysicsAwake = 0;
	m_bPhysicsWasAwake = 1;
	m_nAuxPhys = 0;
  m_nLodLevel=0;
	m_fPhysBlendTime = 1E6f;
	m_fPhysBlendMaxTime = m_frPhysBlendMaxTime = 1.0f;

	AddSubmesh(pMesh, true);
}


CryModelState::~CryModelState()
{ 
	m_arrBones.clear();
	m_arrBoneGlobalMatrices.clear();

	if (m_pCharPhysics)
		GetPhysicalWorld()->DestroyPhysicalEntity(m_pCharPhysics);

	int i;
	for(i=0;i<m_nAuxPhys;i++) {
		GetPhysicalWorld()->DestroyPhysicalEntity(m_auxPhys[i].pPhysEnt);
		delete[] m_auxPhys[i].pauxBoneInfo;
	}
	m_nAuxPhys = 0;

	for (i = 0; i < 4; ++i)
		if (m_pIKEffectors[i])
			delete m_pIKEffectors[i];

	m_arrAnimationLayers.clear();
	RemoveAllDynBoundLights();
}




//////////////////////////////////////////////////////////////////////////
// Generate Base Layer and Detail Layer Effectors for animation, and set up initial state
void CryModelState::InitBones(bool bBoneInfoDefPoseNeedInitialization)
{
	SelfValidate();
	AnimationLayer layer;
  layer.pEffector = new CCryModEffAnimation(this);
  m_arrAnimationLayers.push_back(layer);

	InitDefaultPose(bBoneInfoDefPoseNeedInitialization,!bBoneInfoDefPoseNeedInitialization);
}


// sets the default position, getting it from the inverse default matrix in the bone infos
void CryModelState::SetPoseFromBoneInfos()
{
	for (unsigned nBone = 0; nBone  < numBones(); ++nBone)
	{
		Matrix44& matGlobal = getBoneMatrixGlobal (nBone);
		const Matrix44& matInvGlobal = getBoneInfo (nBone)->getInvDefGlobal();
		assert (IsOrthoUniform(matInvGlobal));
		// calculate the global from the inverse
		matGlobal = OrthoUniformGetInverted(matInvGlobal);
		Matrix44& matRelative = getBone(nBone).m_matRelativeToParent;
		if (nBone)
		{
			unsigned nParent = (unsigned)(nBone + getBoneInfo(nBone)->getParentIndexOffset());
			assert (nParent < nBone);
			const Matrix44& matInvGlobalParent = getBoneInfo(nParent)->getInvDefGlobal();
			// this has a parent: calculate the relative-to-parent transform
			matRelative = matGlobal * matInvGlobalParent;
		}
		else
			matRelative = matGlobal;
		getBone(nBone).BuildQPFromRelToParent();
	}

	UpdateBoneMatricesGlobal();
}

CryModel* CryModelState::GetMesh()
{
	return GetCryModelSubmesh(0)->GetCryModel();
}

//////////////////////////////////////////////////////////////////////////
// Puts the bones into default pose, 0th frame
// If there's no default pose animation, then resets the bone to default position/orientation
// Calculates the relative to parent and global matrices
// creates the initial pose, initializes the bones (builds the inverse transform of the global) and IK limb pose
void CryModelState::InitDefaultPose(bool bInitBoneInfos, bool bTakePoseFromBoneInfos)
{
	SelfValidate();

	if (bTakePoseFromBoneInfos)
	{
		SetPoseFromBoneInfos();
	}
	else
	{
		// apply the default animation
		// for those bones that have no controller in the default animation, we can do little
		ApplyAnimationToBones (CAnimationLayerInfo(0,0,1));
	}
	m_uFlags &= ~nFlagNeedBoneUpdate;

	if (bInitBoneInfos)
	{
		// calculate the real inverse-global matrix and put it into the bone info;
		// reset the relative to default matrix (since it's invalid)
		CryBoneInfo* pBoneInfo = m_pMesh->getBoneInfos(), *pBoneInfoEnd = pBoneInfo + m_pMesh->numBoneInfos();
		Matrix44* pmatGlobal = m_arrBoneGlobalMatrices.begin();
		for (; pBoneInfo != pBoneInfoEnd; ++pBoneInfo, ++pmatGlobal)
			//pBoneInfo->m_matInvDefGlobal.AssignInverseOf(*pmatGlobal);
			pBoneInfo->m_matInvDefGlobal=GetInverted44(*pmatGlobal);
	}

	// reset the relative to default matrix (since it's invalid)
	

	for (unsigned i = 0; i < 4; ++i) 
		m_IKpos0[i].zero() = GetLimbEndPos (i, 1);
}

// Creates the bones
void CryModelState::CreateBones()
{
	SelfValidate();
	unsigned numBones = m_pMesh->numBoneInfos();
	m_arrBones.reinit (numBones);
	Matrix44 matDefault;
	matDefault.SetIdentity();
	m_arrBoneGlobalMatrices.reinit (numBones, matDefault);
	//assert (((unsigned)m_arrBoneGlobalMatrices.begin() & 0xF) == 0);
	setBoneParent();
}

void CryModelState::InitBBox()
{
	if (!m_pMesh->getGeometryInfo(0)->computeBBox (m_BBox))
		UpdateBBox();

	if (m_BBox.empty())
		g_GetLog()->LogError("\003CryModel::LoadPostInitialize:Character %s bbox initialized to empty {%g,%g,%g}{%g,%g,%g}", m_pMesh->getFilePathCStr(), m_BBox.vMin.x, m_BBox.vMin.y, m_BBox.vMin.z, m_BBox.vMax.x, m_BBox.vMax.y, m_BBox.vMax.z);
}

//////////////////////////////////////////////////////////////////////////
// out of the bone positions, calculates the bounding box for this character and puts it
// into m_vBoxMin,m_vBoxMax
// NOTE: There is a huge potential for optimizing this for SSE
void CryModelState::UpdateBBox()
{

#if !BBOX_FROM_SKIN
	SelfValidate();
	//PROFILE_FRAME(BBoxUpdate);
	if (m_nLastSkinBBoxUpdateFrameId + 1 >= (unsigned)g_nFrameID && (m_nLastSkinBBoxUpdateFrameId&g_GetCVars()->ca_SkinBasedBBoxMask()))
	{
		ValidateBBox();
		return; // the bbox is being updated by SSE skin calculation routine right now
	}

	if (m_arrBones.empty())
		return;

	// this is the number of bones to take into account when calculating the bbox;
	// and the number of first (root) bones to skip
	// by default, take all bones
	unsigned numSkipBones = 0;
	unsigned numBones = this->numBones();
	if (m_pMesh->getGeometryInfo()->hasGeomSkin())
	{
		CrySkinFull* pSkin = m_pMesh->getGeometryInfo()->getGeomSkin();
		if (pSkin)
		{
			// if there is a skin, it knows which bones don't affect it; use only the bones that do affect it
			numSkipBones = pSkin->numSkipBones();
			numBones = pSkin->numBones();
		}
	}
	const CryBBoxA16* pBoneBBox = m_pMesh->getBoneBBoxes();
#if !defined(LINUX) && defined(_CPU_X86)
	if (g_GetCVars()->ca_SSEEnable() && cpu::hasSSE())
	{
		if (pBoneBBox)
			// if the data about dimension of each bbox is unavalable, use the simlpe algorithm
			getBBoxSSE (&m_arrBoneGlobalMatrices[numSkipBones], pBoneBBox+numSkipBones, numBones-numSkipBones, &m_BBox);
		else
			getBBoxSSE (&m_arrBoneGlobalMatrices[numSkipBones], numBones-numSkipBones, &m_BBox);

	}
	else
#endif
	{
		if (!pBoneBBox)
		{
			// if the data about dimension of each bbox is unavalable, use the simlpe algorithm
			Vec3 c = getBoneMatrixGlobal(numSkipBones).GetTranslationOLD();
			m_BBox = CryAABB(c,c);

			for (unsigned i = numSkipBones+1; i < numBones; ++i)
				m_BBox.include(getBoneMatrixGlobal(i).GetTranslationOLD());
		}
		else
		{
			const Matrix44* pBoneMtx = &getBoneMatrixGlobal(numSkipBones), *pBoneMtxEnd = &getBoneMatrixGlobal(0)+numBones;

			m_BBox = CryAABB(pBoneMtx->GetTranslationOLD(),pBoneMtx->GetTranslationOLD());

			for (; pBoneMtx < pBoneMtxEnd; ++pBoneMtx, ++pBoneBBox)
			{
				for (int i = 0; i < 3; ++i)
				{
					m_BBox.include(pBoneMtx->GetTranslationOLD() + pBoneMtx->GetRow (i)*pBoneBBox->vMin.v[i]);
					m_BBox.include(pBoneMtx->GetTranslationOLD() + pBoneMtx->GetRow (i)*pBoneBBox->vMax.v[i]);
				}
			}
		}
	}
#endif
}


void CryModelState::ValidateBBox()
{
	if (m_BBox.vMax.x < m_BBox.vMin.x
		||m_BBox.vMax.y < m_BBox.vMin.y
		||m_BBox.vMax.z < m_BBox.vMin.z
		||!(m_BBox.vMax.x < 1000)
		||!(m_BBox.vMax.z < 1000)
		||!(m_BBox.vMax.z < 1000)
		||!(m_BBox.vMin.x < 1000)
		||!(m_BBox.vMin.z < 1000)
		||!(m_BBox.vMin.z < 1000)
		||!(m_BBox.vMax.x > -1000)
		||!(m_BBox.vMax.z > -1000)
		||!(m_BBox.vMax.z > -1000)
		||!(m_BBox.vMin.x > -1000)
		||!(m_BBox.vMin.z > -1000)
		||!(m_BBox.vMin.z > -1000)
		)
	{
		g_GetLog()->LogWarning ("\001[Animation] invalid BBox{{%.3f,%.3f,%.3f}-{%.3f,%.3f,%.3f}}. Model state 0x%X, model 0x%X, \"%s\" instance %d, frame %d",
			m_BBox.vMin.x, m_BBox.vMin.y, m_BBox.vMin.z, m_BBox.vMax.x, m_BBox.vMax.y, m_BBox.vMax.z,
			this, m_pMesh, m_pMesh->getFilePathCStr(), m_nInstanceNumber, g_nFrameID);
	}
}

bool CryModelState::IsAnimStopped()
{
	SelfValidate();
	if (m_bPhysicsAwake)
		return false;

	for (unsigned nLayer = 0; nLayer < m_arrAnimationLayers.size(); ++nLayer)
	{
		AnimationLayer& layer = m_arrAnimationLayers[nLayer];
		if (!layer.pEffector)
			continue;
    if (!layer.pEffector->IsStopped())
      return false;
	}

	for (SubmeshArray::iterator it = m_arrSubmeshes.begin(); it != m_arrSubmeshes.end(); ++it)
#if !defined(LINUX64)
		if (*it != NULL && (*it)->NeedMorph())
#else
		if (*it != 0 && (*it)->NeedMorph())
#endif
			return false;

  return true;
}


// updates the *ModEff* - adds the given delta to the current time,
// calls the callbacks, etc. Returns the array describing the updated anim layers,
// it can be applied to the bones
void CryModelState::UpdateAnimatedEffectors (float fDeltaTimeSec, ActiveLayerArray& arrActiveLayers)
{
	DEFINE_PROFILER_FUNCTION();
	SelfValidate();

	for (SubmeshArray::iterator it = m_arrSubmeshes.begin(); it != m_arrSubmeshes.end(); ++it)
		if (*it)(*it)->UpdateMorphEffectors(fDeltaTimeSec);

	// we'll remember each active layer blending
	unsigned numLayers = (unsigned)m_arrAnimationLayers.size();
	arrActiveLayers.reserve(numLayers*2);

	for (unsigned nLayer = 0; nLayer < numLayers; ++nLayer)
	{
		AnimationLayer& layer = m_arrAnimationLayers[nLayer];
		if(!layer.pEffector)
			continue;

#ifdef _DEBUG
		AnimationLayer bakLayer = layer;
		layer = bakLayer;
#endif
		layer.pEffector->Tick (fDeltaTimeSec * g_GetCVars()->ca_UpdateSpeed(nLayer) * getAnimationSpeed(nLayer), m_arrSinks, arrActiveLayers);
		
		if (!layer.pEffector->isActive())
		{
			if (layer.queDelayed.empty())
			{
				if (layer.nDefaultIdleAnimID >= 0 && layer.bEnableDefaultIdleAnimRestart)
				{
					// re-run the default loop animation
					assert ((unsigned)layer.nDefaultIdleAnimID < m_pMesh->numAnimations());
					if (g_GetCVars()->ca_Debug())
						g_GetLog()->Log ("\003Restarting default idle animation %s in layer %d, blend in=%.2f, out=%.2f", m_pMesh->getAnimation(layer.nDefaultIdleAnimID).strName.c_str(), nLayer, layer.fDefaultAnimBlendTime, layer.fDefaultAnimBlendTime);
					// we run the new animation with the current speed
					CryCharAnimationParams Params(layer.fDefaultAnimBlendTime, layer.fDefaultAnimBlendTime, nLayer);

					if (layer.bKeepLayer0Phase)
						Params.nFlags |= CryCharAnimationParams::FLAGS_SYNCHRONIZE_WITH_LAYER_0;

					RunAnimation(layer.nDefaultIdleAnimID, Params, fDeltaTimeSec);
				}
			}
			else
			{
				// run the aligned animation
				AnimationRecord record = layer.queDelayed.front();
				layer.queDelayed.pop_front();
				if (g_GetCVars()->ca_Debug())
					g_GetLog()->Log("\003Autostarting queued animation %s in layer %d, blend in=%.2f, out=%.2f",
					m_pMesh->getAnimation(record.nAnimId).strName.c_str(), nLayer, record.fBlendInTime, record.fBlendOutTime);
				RunAnimation(record.nAnimId, static_cast<CryCharAnimationParams&>(record), record.fSpeed);
			}
		}
		else
		{
			if (!layer.queDelayed.empty() && layer.pEffector->GetTimeTillEnd() <= layer.queDelayed.front().fBlendInTime)
			{
				AnimationRecord record = layer.queDelayed.front();
				layer.queDelayed.pop_front();
				if (g_GetCVars()->ca_Debug())
					g_GetLog()->Log("\003Autostarting queued animation %s in layer %d, blend in=%.2f, out=%.2f",
					m_pMesh->getAnimation(record.nAnimId).strName.c_str(), nLayer, record.fBlendInTime, record.fBlendOutTime);

				RunAnimation(record.nAnimId, static_cast<CryCharAnimationParams&>(record), record.fSpeed, true);
			}
		}
	}
	m_fPhysBlendTime += fDeltaTimeSec;
}


//////////////////////////////////////////////////////////////////////////
// Moves the animations in time by the given time interval, applies the
// animation to bones, sends required notification to the animation sinks
// Calculates bone matrices. Processes physics. Updates BBox.
void CryModelState::ProcessAnimations (float fDeltaTimeSec, bool bUpdateBones, CryCharInstance* instance)
{
	FUNCTION_PROFILER( g_GetISystem(),PROFILE_ANIMATION );

	SelfValidate();

  if (g_GetCVars()->ca_NoAnim())
    return;	

	g_arrActiveLayers.clear();
	UpdateAnimatedEffectors (fDeltaTimeSec, g_arrActiveLayers);

	if (g_GetCVars()->ca_Debug() && *g_GetCVars()->ca_LogAnimation() && stristr(m_pMesh->getFilePathCStr(), g_GetCVars()->ca_LogAnimation()))
	{
		// this test code outputs the blending information every frame
		string strLayers;
		for (AnimationLayerArray::iterator it = m_arrAnimationLayers.begin(); it != m_arrAnimationLayers.end(); ++it)
			if (it->pEffector)
				strLayers += "  " + it->pEffector->dump();
			else
				strLayers += "  <I>";
		string strInfo;
		for (CAnimationLayerInfoArray::iterator it = g_arrActiveLayers.begin(); it != g_arrActiveLayers.end(); ++it)
		{
			const AnimData& anim = getAnimationSet()->getAnimation(it->nAnimId);
			strInfo += " \"" + anim.strName + "\"";
			char szBuf[0x100];
			sprintf (szBuf, " (t=%.2f, b=%.2f)", it->fTime, it->fBlending);
			strInfo += szBuf;
		}
		g_GetLog()->LogToFile ("\003%d.Effectors %s, layer info %s", g_GetIRenderer()->GetFrameID(),strLayers.c_str(), strInfo.c_str());
	}
	
	if (g_GetCVars()->ca_EnableCubicBlending())
	{
		ActiveLayerArray::iterator it, itEnd = g_arrActiveLayers.end();
		for (it = g_arrActiveLayers.begin(); it != itEnd; ++it)
			it->fBlending = SmoothBlendValue(it->fBlending);
	}

	if (!g_arrActiveLayers.empty())
	{

/*
if (g_YLine==16.0f) {

		extern std::vector<String> AnimStrings;
		extern std::vector<u32> FrameID;
		extern std::vector<u32> LayerID;

		u32 size=AnimStrings.size();
		float fColor[4] = {0,1,0,1};
	
		u32 Counter=0;
		if (size>20) Counter=size-20;
		for (u32 x=Counter; x<size; x++) {
			g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"Valerie: FrameID: %04x LayerID: %04x  %s", FrameID[x], LayerID[x],AnimStrings[x] );
			g_YLine+=16.0f;
		} 

	}*/


	/*	
		if (instance) {

			float fColor[4] = {0,1,0,1};
			String TestName = "objects\\characters\\story_characters\\valerie\\valeri.cgf";
			String ModelName = instance->GetBody()->GetFileName();
			if (TestName==ModelName) {
				int i=0;
			}

			g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"model: %1x %s",bUpdateBones, ModelName );
			g_YLine+=16.0f;

			u32 NumLayer=g_arrActiveLayers.size();
			g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"g_arrActiveLayers %d %s",g_arrActiveLayers.size(),m_pMesh->getAnimationInfo(0)->strName );
			g_YLine+=0x10;

			for (ActiveLayerArray::const_iterator it = g_arrActiveLayers.begin(); it != g_arrActiveLayers.end(); ++it)
			{
				float fColor[4] = {1,1,1,1};
				const char* AnimationName = m_pMesh->GetName(it->nAnimId);
				g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"g_arrActiveLayers.nAnimId %d %s",it->nAnimId, AnimationName );
				g_YLine+=0x10;
			}
			g_YLine+=0x10;

		}
		*/

		if (bUpdateBones)
		{
			UpdateBones (g_arrActiveLayers);
			UpdateBBox(); //use vertices for update
			m_uFlags |= nFlagsNeedReskinAllLODs;
		}
		else
			m_uFlags |= nFlagNeedBoneUpdate|nFlagsNeedReskinAllLODs;
	}
}


// updates the bone matrices using the given array of animations -
// applies the animation layers to the bones
void CryModelState::UpdateBones (const ActiveLayerArray& arrActiveLayers)
{
	FUNCTION_PROFILER( g_GetISystem(),PROFILE_ANIMATION );

/*	u32 NumLayer=arrActiveLayers.size();
	float fColor[4] = {0,1,0,1};
	g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"arrActiveLayers %d %s",arrActiveLayers.size(),m_pMesh->getAnimationInfo(0)->strName );
//	g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"arrActiveLayers %d %s",arrActiveLayers.size(),m_pMesh->Animation->getGeometryInfo()-> );
//	g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"arrActiveLayers %d %s",arrActiveLayers.size(),m_pMesh->GetName(0) );
	g_YLine+=0x10;

	for (ActiveLayerArray::const_iterator it = arrActiveLayers.begin(); it != arrActiveLayers.end(); ++it)
	{
		float fColor[4] = {1,1,1,1};
		u32 id=it->nAnimId;
		const CAnimationLayerInfo& rLayer = *it;
		g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"arrActiveLayers.nAnimId %d %s",it->nAnimId, m_pMesh->GetName(it->nAnimId) );
		g_YLine+=0x10;
	}
	g_YLine+=0x10;
*/

	SelfValidate();
	//PROFILE_FRAME(BoneUpdate);
	unsigned numBones = this->numBones();
	// now calculate the actual target PQ for each bone and apply that to it
	if (arrActiveLayers.empty())
		return;

#ifdef _DEBUG
	//if (g_GetCVars()->ca_AnimWarningLevel()>=2)
	{
		string strLayers;
		for (unsigned nLayer = 0; nLayer < arrActiveLayers.size(); ++nLayer)
		{
			const CAnimationLayerInfo& rLayer = arrActiveLayers[nLayer];
			const char* szName = getAnimationSet()->getAnimation (rLayer.nAnimId).strName.c_str();
			char szBuf[0x200];
			sprintf (szBuf, "\"%s\"  @%8.3f,%3d%%", szName, rLayer.fTime/getAnimationSet()->getTicksPerSecond(), int(rLayer.fBlending*100));
			if (!strLayers.empty())
				strLayers += ",  ";
			strLayers += szBuf;
		}

		g_Info ("%2d layers: %s", arrActiveLayers.size(), strLayers.c_str());
	}
#endif

#ifdef _DEBUG
	if (!arrActiveLayers.empty())
	{
		m_bOriginalPose = true;
		for (ActiveLayerArray::const_iterator it = arrActiveLayers.begin(); it != arrActiveLayers.end(); ++it)
		{
			if (it->nAnimId > 0 || (it->nAnimId == 0 && m_pMesh->getAnimationInfo(0)->strName != "default"))
				m_bOriginalPose = false;
		}
	}
#endif

	if (arrActiveLayers.size() == 1)
		ApplyAnimationToBones (arrActiveLayers.back());
	else
		ApplyAnimationsToBones (&arrActiveLayers[0], (unsigned)arrActiveLayers.size());

	m_uFlags &= ~nFlagNeedBoneUpdate;

	for (CryCharFxTrailArray::iterator it = m_arrFxTrails.begin(); it != m_arrFxTrails.end(); ++it)
		if (*it)(*it)->Deform (getBoneGlobalMatrices());
}


////////////////////////////////////////////////////////////////////////////
// Calculates the relative-to-parent position and rotation of the bone
// applies the given set of animations, the last overrides the first
// NOTE:
//  Anim.fWeight - the weight of the change. 0 means no change, 1 - full change,
//             1/3 - the resulting Q/P will be 1/3 of the controller's and 2/3 of the previous bone's Q/P
// calculate the position and orientation of the bone according to the current time
// set the fBlendWeight parameter to 1 in order to disable blending at all. 0 effectively turns animation off
void CryModelState::ApplyAnimationsToBones (const CAnimationLayerInfo* pAnims, unsigned numAnims)
{
	FUNCTION_PROFILER( g_GetISystem(),PROFILE_ANIMATION );
	SelfValidate();
	for (unsigned i = 0; i < numAnims; ++i)
		m_pMesh->OnAnimationApply(pAnims[i].nAnimId);
	assert(numAnims > 1);
	CryBone* pBoneBegin = &m_arrBones[0], *pBone = pBoneBegin;
	CryBone* pBoneEnd = pBone + numBones();
	const CryBoneInfo* pBoneInfoBegin = getBoneInfo(0), *pBoneInfo = pBoneInfoBegin;
	for (; pBone != pBoneEnd; ++pBone, ++pBoneInfo)
	{
		if(!pBone->m_bUseReadyRelativeToParentMatrix || pBone->m_bUseMatPlus)
			// if we use external matrix, we don't build our own
			// if we use Plus matrix (which doesn't allow to use the external matrix), we rebuild internal matrix anyway
		{
			float fMaxBlending = 1;

			IController::PQLog pqTarget = pBone->m_pqTransform;

			// for each layer, blend the layer's pq with the target pq
			// we blend smoothly each layer's animation, according to the blending weight.
			// the algorithm does override the blending of the highest layer, i.e. if the highest layer
			// has blending of 0.8 and the lower layer has 1, then it will be 0.8-0.2 proportion after recalculation
			for (int nLayer = (int)numAnims - 1; nLayer >= 0 && fMaxBlending; --nLayer)
			{
				const CAnimationLayerInfo& rAnim = pAnims[nLayer];
				if ((unsigned)rAnim.nAnimId >= (unsigned)pBoneInfo->m_arrControllers.size() || rAnim.fBlending <= 0)
					continue;
				IController* pController = pBoneInfo->m_arrControllers[rAnim.nAnimId];
				if (!pController)
					continue; // no animation

				// get the underlayer transform and blend it into the target transform
				IController::PQLog pqNewTransform;
				pController->GetValue2 (rAnim.fTime, pqNewTransform);
				AdjustLogRotations (pqTarget.vRotLog, pqNewTransform.vRotLog);
				pqTarget.blendPQ (pqTarget, pqNewTransform, fMaxBlending * rAnim.fBlending);

				// each underlayer gather as little control as the overlay leaves for it;
				// if the overlay animation doesn't leave any (if it's weight is 0.99+) we don't have to continue
				fMaxBlending *= 1 - rAnim.fBlending;
				assert (fMaxBlending >= 0);
				if (fMaxBlending <= 0.001f)
					break;
			}

			// we lock the position/rotation of the bone if it's 100% determined by the animation, so that if
			// the ainmation abruptly stops (which should not happen), the bone doesn't move; if the animation fades out
			// (which happens often), the bone still won't return to its pre-animated position, if ht eanimation reached
			// 100% blend weight during its play
			if (fMaxBlending <= 0.01f)
			{
#ifdef _DEBUG
				if (g_GetCVars()->ca_Debug())
				{
					float fDistance = (pBone->m_pqTransform.vRotLog - pqTarget.vRotLog).Length();
					if (fDistance > 0.3)
						g_GetLog()->Log("\005animation jump: %.5f on %s", fDistance, pBoneInfo->getNameCStr());
				}
#endif
				pBone->m_pqTransform = pqTarget;
			}
			pBone->BuildRelToParentFromQP (pqTarget);
			AddModelOffsets(pBone);
		}
	}


	// finalize: build the global matrices
	// from the already given relative to parent matrix
	UpdateBoneMatricesGlobal();
}



////////////////////////////////////////////////////////////////////////////
// Calculates the relative-to-parent position and rotation of the bone
void CryModelState::ApplyAnimationToBones (CAnimationLayerInfo rAnim)
{
	FUNCTION_PROFILER( g_GetISystem(),PROFILE_ANIMATION );
	SelfValidate();
	m_pMesh->OnAnimationApply(rAnim.nAnimId);
	CryBone* pBoneBegin = &m_arrBones[0], *pBone = pBoneBegin;
	CryBone* pBoneEnd = pBone + numBones();
	const CryBoneInfo* pBoneInfoBegin = getBoneInfo(0), *pBoneInfo = pBoneInfoBegin;
	for (; pBone != pBoneEnd; ++pBone, ++pBoneInfo)
	{
		if ((unsigned)rAnim.nAnimId >= pBoneInfo->m_arrControllers.size() || rAnim.fBlending <= 0)
			continue;
		IController* pController = pBoneInfo->m_arrControllers[rAnim.nAnimId];
		if (!pController)
			continue;
		if (!pBone->m_bUseReadyRelativeToParentMatrix || pBone->m_bUseMatPlus)
		{
			if (rAnim.fBlending >= 1)
			{
				pController->GetValue2(rAnim.fTime, pBone->m_pqTransform);
				pBone->BuildRelToParentFromQP (pBone->m_pqTransform);
			}
			else
			{
				// suppose we rarely get the fWeight 0 so we won't optimize for that case
				IController::PQLog pqNewTransform;
				pController->GetValue2(rAnim.fTime, pqNewTransform);
				AdjustLogRotations (pBone->m_pqTransform.vRotLog, pqNewTransform.vRotLog);
				pqNewTransform.blendPQ (pBone->m_pqTransform, pqNewTransform, rAnim.fBlending);
				pBone->BuildRelToParentFromQP(pqNewTransform);
			}								 
			AddModelOffsets(pBone);
		}
	}
	

	// finalize: build the global matrices
	// from the already given relative to parent matrix
	UpdateBoneMatricesGlobal();
}


void CryModelState::AddModelOffsets(CryBone* pBone)
{
	// apply the model offset to the root bone
	if (&m_arrBones[0] == pBone)
	{
		pBone->AddOffsetRelToParent(m_pMesh->getModelOffset());
		pBone->ScaleRelToParent (m_vRuntimeScale * g_GetCVars()->ca_RuntimeScale());
	}
}



//////////////////////////////////////////////////////////////////////////
// calculates the global matrices
// from relative to parent matrices
void CryModelState::UpdateBoneMatricesGlobal()
{
	// [marco] it crashes here while loading a mission - I added 
	// a check if this should never happens plz fix this properly

	if (m_arrBones.empty())
		return;

	FUNCTION_PROFILER( g_GetISystem(),PROFILE_ANIMATION );

	// update the global matrices and the relative-to-default matrices
	Matrix44* pmatGlobal = &m_arrBoneGlobalMatrices[0];
	const CryBone* pBone = &m_arrBones[0], *pBoneEnd = pBone + m_arrBones.size();
	const CryBoneInfo* pBoneInfo = getBoneInfo(0);
		
	// the first step is for the root bone
	*pmatGlobal = pBone->m_matRelativeToParent;
	
	// go to the 1st bone and go on as if there is a parent for each and every bone
	for (++pBone, ++pBoneInfo, ++pmatGlobal;
		pBone != pBoneEnd;
		   ++pBone, ++pBoneInfo, ++pmatGlobal)
	{
		assert (IsOrthoUniform(pBone->m_matRelativeToParent));
		// calculate the global matrix
		// to calculate the global, we should search for the global matrix of the parent.
		// the global matrix of the parent has the known offset, we apply it directly to the pointer to the child's global matrix
		*pmatGlobal = pBone->m_matRelativeToParent * *(pmatGlobal+pBoneInfo->getParentIndexOffset());
		assert (IsOrthoUniform(*pmatGlobal));
	}
}


//////////////////////////////////////////////////////////////////////////
// based on the distance from camera, determines the best LOD
// for this character and memorizes it in the m_nLodLevel member
void CryModelState::CalculateLOD (float fDistance)
{
	SelfValidate();
	unsigned numLODs = m_pMesh->numLODs();
  // Set LOD depending on distance, lod bias, and size of object
  if(m_pMesh->getStaticBSphereRadius()>0 && ((INT_PTR)g_GetIRenderer()->EF_Query(EFQ_RecurseLevel)==1))
  { 
    float fZoomFactor = RAD2DEG(GetViewCamera().GetFov())/90.f;
    float fVal = fDistance * g_GetCVars()->ca_LodBias() /*0.125f*//*m_pMesh->m_fRadius*/*fZoomFactor;
    
    m_nLodLevel = 0;
		frexp (fVal,&m_nLodLevel);
		/*
    while(fVal>1)
    { 
      m_nLodLevel++;
      fVal*=0.5;
    }
		*/

    if(m_nLodLevel<0)
      m_nLodLevel=0;
		if(m_nLodLevel>=(int)numLODs)
      m_nLodLevel = numLODs-1;
  }
}


bool CryModelState::IsCharacterActive()
{
	SelfValidate();
  return !IsAnimStopped();
}


void CryModelState::Render(const struct SRendParams & RendParams, Matrix44& mtxObjMatrix, CryCharInstanceRenderParams& rCharParams, const Vec3& translation )
{

	for (SubmeshArray::iterator it = m_arrSubmeshes.begin(); it != m_arrSubmeshes.end(); ++it)
		if (*it)(*it)->Render(RendParams, mtxObjMatrix, rCharParams,translation);

	for (CryCharFxTrailArray::iterator it = m_arrFxTrails.begin(); it != m_arrFxTrails.end(); ++it)
		if (*it)(*it)->Render(RendParams, mtxObjMatrix, rCharParams);

	unsigned nFrameId = g_GetIRenderer()->GetFrameID();

	// every 32-th frame, try to shrink the pool of shadow buffers, if there are unused
	// for some time shadow buffers
	if ((nFrameId & 0x1F) == (m_nInstanceNumber & 0x1F))
		for (SubmeshArray::iterator it = m_arrSubmeshes.begin(); it != m_arrSubmeshes.end(); ++it)
			if (*it)(*it)->ShrinkShadowPool();
}

/*
void CryModelState::DrawForShadow()
{
	SelfValidate();
  if(m_pLeafBuffers[m_nLodLevel] && m_pLeafBuffers[m_nLodLevel]->m_pVertexBuffer)
  {
    //g_GetRenderer()->m_RP.mStateIgnore |= (RBSI_ALPHAGEN | RBSI_RGBGEN);
    g_GetRenderer()->DrawBuffer(m_pLeafBuffers[m_nLodLevel]->m_pVertexBuffer, 
      &m_pLeafBuffers[m_nLodLevel]->GetIndices()[0], 
      m_pLeafBuffers[m_nLodLevel]->GetIndices().Count(), 
      m_pLeafBuffers[m_nLodLevel]->m_nPrimetiveType);
  }
}
*/

void CryModelState::setBoneParent()
{
	SelfValidate();
	for (unsigned nBone = 0; nBone < m_arrBones.size(); ++nBone)
		m_arrBones[nBone].setParent (this);
}

//////////////////////////////////////////////////////////////////////////
// This function should be as fast as possible...
CryModelState* CryModelState::MakeCopy()
{
	SelfValidate();
	assert(this && m_pMesh);
  if(!getRootBone() && !m_pMesh->numMorphTargets())
    return NULL;

  CryModelState * pCopy = new CryModelState(m_pMesh);
  assert(pCopy);

	pCopy->m_pShaderStateCull = m_pShaderStateCull;
	pCopy->m_pShaderStateShadowCull = m_pShaderStateShadowCull;

	pCopy->m_mapAnimEvents = m_mapAnimEvents;
	pCopy->m_arrSinks = m_arrSinks;

  pCopy->CreateBones();
	for (unsigned nBone = 0; nBone < numBones(); ++nBone)
	{
		pCopy->m_arrBones[nBone] = m_arrBones[nBone];
		pCopy->m_arrBoneGlobalMatrices[nBone] = m_arrBoneGlobalMatrices[nBone];
	}
	pCopy->setBoneParent();

  int nLod = 0;

	pCopy->m_bHasPhysics = m_bHasPhysics;

  // Generate Base Layer and Detail Layer Effectors for animation, and set up initial state
  AnimationLayer layer;
	layer.pEffector = new CCryModEffAnimation (pCopy);
  pCopy->m_arrAnimationLayers.push_back(layer);

	pCopy->InitDefaultPose(false,true);

  pCopy->m_BBox = m_BBox;

  return pCopy;
}


ICryBone * CryModelState::GetBoneByName(const char * szBoneName)
{
	SelfValidate ();
  int nBone = m_pMesh->findBone(szBoneName);
  if(nBone == -1)
    return NULL;

  return &getBone(nBone);
}


// Enables/Disables the Default Idle Animation restart.
// If this feature is enabled, then the last looped animation will be played back after the current (non-loop) animation is finished.
// Only those animations started with the flag bTreatAsDefaultIdleAnimation == true will be taken into account
void CryModelState::EnableIdleAnimationRestart (unsigned nLayer, bool bEnable)
{
	SelfValidate();
	if (nLayer < 0 || nLayer > 0x400)
		return;
	if (m_arrAnimationLayers.size() <= nLayer)
		m_arrAnimationLayers.resize (nLayer + 1);
	m_arrAnimationLayers[nLayer].bEnableDefaultIdleAnimRestart = bEnable;
}



// returns the animation currently being played in the given layer, or -1
int CryModelState::GetCurrentAnimation (unsigned nLayer)
{
	if (nLayer >= m_arrAnimationLayers.size())
		return -1; // no such layer exists
	const CCryModEffAnimation* pEffector = m_arrAnimationLayers[nLayer].pEffector;
	if (pEffector)
		return pEffector->GetAnyCurrentAnimation();
	return -1; // no effector -> no animation
}


bool CryModelState::RunMorph (const char* szMorphTarget,const CryCharMorphParams&Params)
{
	bool bRes = false;
	for (SubmeshArray::iterator it = m_arrSubmeshes.begin(); it != m_arrSubmeshes.end(); ++it)
		if (*it)
			bRes = (*it)->RunMorph (szMorphTarget, Params, false) || bRes;
	return bRes;
}

bool CryModelState::RunMorph (int nMorphTargetId, float fBlendInTime, float fBlendOutTime)
{
	SelfValidate();

	GetCryModelSubmesh(0)->RunMorph (nMorphTargetId, CryCharMorphParams(fBlendInTime, 0, fBlendOutTime));
	return true;
}


//! Finds the morph with the given id and sets its relative time.
//! Returns false if the operation can't be performed (no morph)

void CryModelState::StopAllMorphs()
{
	for (SubmeshArray::iterator it = m_arrSubmeshes.begin(); it != m_arrSubmeshes.end(); ++it)
		if (*it)(*it)->StopAllMorphs();
}

void CryModelState::FreezeAllMorphs()
{
	for (SubmeshArray::iterator it = m_arrSubmeshes.begin(); it != m_arrSubmeshes.end(); ++it)
		if (*it)(*it)->FreezeAllMorphs();
}


// stops the animation at the given layer, and returns true if the animation was
// actually stopped (if the layer existed and the animation was played there)
bool CryModelState::StopAnimation (int nLayer)
{
	SelfValidate();
	bool bResult = false;
	if ((size_t)nLayer < m_arrAnimationLayers.size())
	{
		AnimationLayer& rLayer = m_arrAnimationLayers[nLayer];
		rLayer.nDefaultIdleAnimID = -1; // forget the default idle animation
		if (rLayer.pEffector)
		{
			bResult = rLayer.pEffector->IsStopped();
			rLayer.pEffector->stop();
		}
	}
	return bResult;
}



// starts playing animation
// returns true if the animation was found and run, false otherwise
bool CryModelState::RunAnimation (const char * szAnimName, const struct CryCharAnimationParams& Params, float fSpeed)
{
	SelfValidate ();
	assert(szAnimName);

	bool bPrintDebugInfo = g_GetCVars()->ca_AnimWarningLevel() > 2;

	int nAnimID = m_pMesh->findAnimation(szAnimName);
	if(nAnimID<0)
	{ 
		int nMorphTargetId = m_pMesh->findMorphTarget(szAnimName);
		if (nMorphTargetId < 0)
		{
			if (m_pMesh->isDummyAnimation(szAnimName))
				return true; // pretend we started it; just ignore the call

			if(bPrintDebugInfo)
			{
				CryWarning (VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, "!Animation \"%s\" Not Found for character \"%s\"", szAnimName, m_pMesh->getFilePathCStr());
				//g_GetLog()->LogWarning ("\002CryModelState::RunAnimation: Animation \"%s\" Not Found for character \"%s\"", szAnimName, m_pMesh->getFilePathCStr());
			}
			return false; 
		}
		else
			return RunMorph (nMorphTargetId, Params.fBlendInTime, Params.fBlendOutTime);
	}
	else
		return RunAnimation(nAnimID, Params, fSpeed);
}

// The fSpeed is the external speed, not taking into account the internal per-animation layer multiplier
bool CryModelState::RunAnimation (int nAnimID, const CryCharAnimationParams& Params, float fSpeed, bool bInternal)
{
	SelfValidate();
	assert (nAnimID >= 0 && (unsigned)nAnimID < m_pMesh->numAnimations());
	const AnimData* pAnim = m_pMesh->getAnimationInfo(nAnimID);
	if (!pAnim)
		return false;

	if (m_arrAnimationLayers.size() <= (size_t)Params.nLayerID)
		m_arrAnimationLayers.resize (Params.nLayerID+1);
	AnimationLayer& rLayer = m_arrAnimationLayers[Params.nLayerID];

	// make effector if not created yet
	if(!rLayer.pEffector)
	{
		rLayer.pEffector = new CCryModEffAnimation(this);
	}

	if ((Params.nFlags&Params.FLAGS_NO_DEFAULT_IDLE)==0 && pAnim->bLoop)
	{
		// if this animation is looped, it becomes the default loop animation
		// for this rLayer. THe settings of the current call become the default settings for it
		rLayer.nDefaultIdleAnimID = nAnimID;
		rLayer.fDefaultAnimBlendTime = Params.fBlendInTime;
		rLayer.bKeepLayer0Phase = (Params.nFlags&Params.FLAGS_SYNCHRONIZE_WITH_LAYER_0)!=0;
	}

	assert(rLayer.pEffector != (CCryModEffAnimation*)NULL);

	if (!bInternal && rLayer.pEffector->isActive() && 0!=((Params.nFlags|rLayer.pEffector->getStartAnimFlags())&CryCharAnimationParams::FLAGS_ALIGNED))
	{
		CryCharAnimationParams NewParams = Params;
		if (rLayer.pEffector->getStartAnimFlags() & Params.FLAGS_ALIGNED)
			NewParams.fBlendInTime = 0;
		// this animation must be queued
		if (/*(Params.nFlags & Params.FLAGS_ALIGNED)!=0 || */rLayer.queDelayed.empty())
			rLayer.queDelayed.push_back (AnimationRecord(nAnimID, NewParams, fSpeed));
		else
			rLayer.queDelayed.back().assign (nAnimID, NewParams, fSpeed);
		rLayer.pEffector->SetNoLoop/*NoBlendOut*/(); // prepare the current loop to stop and give way to the aligned animation
	}
	else
		rLayer.pEffector->StartAnimation (nAnimID, Params.fBlendInTime, Params.fBlendOutTime, (Params.nFlags&Params.FLAGS_SYNCHRONIZE_WITH_LAYER_0)!=0 ? m_arrAnimationLayers[0].pEffector : rLayer.pEffector,fSpeed * g_GetCVars()->ca_UpdateSpeed(Params.nLayerID) * getAnimationSpeed(Params.nLayerID), Params.nFlags);

	return true;
} 



void CryModelState::ResetAllAnimations()
{
	SelfValidate ();

	for (unsigned nLayer = 0; nLayer < m_arrAnimationLayers.size(); ++nLayer)
	{ // TODO: Allow more detail layers
		AnimationLayer& layer = m_arrAnimationLayers[nLayer];
		if(layer.pEffector)
			layer.pEffector->Reset();
		layer.bEnableDefaultIdleAnimRestart = false;
	}
}



bool CryModelState::SetAnimationFrame(const char * szString, int nFrame)
{
	SelfValidate ();
  ResetAllAnimations();
	// we don't know the speed - actually, this set frame is for setting the frame and freezing
	// so we pass 0
	CryCharAnimationParams NullAnim(0, 0);
	NullAnim.nFlags = NullAnim.FLAGS_NO_DEFAULT_IDLE;
	bool bRes = RunAnimation(szString, NullAnim,0);
  ProcessAnimations(0,true,0);
  return bRes;
}


int CryModelState::GetDamageTableValue(int nId)
{
	SelfValidate ();
  if (nId < 0 || nId >= (int)numBones())
    return 0;
  return m_pMesh->m_arrDamageTable[nId];
}

Matrix44 GetModelMatrix (CCObject* pObj)
{

	//Matrix44 matModel =  GetTranslationMat(pObj->m_Trans); //matModel.Translate(pObj->m_Trans);
	//matModel=matModel*Matrix33::GetRotationX33( DEG2RAD(-pObj->m_Angs[0]) );
	//matModel=matModel*Matrix33::GetRotationY33( DEG2RAD(+pObj->m_Angs[1]) ); //IMPORTANT: radian-angle must be negated 
	//matModel=matModel*Matrix33::GetRotationZ33( DEG2RAD(-pObj->m_Angs[2]) ); 
	//if (!isUnit(pObj->m_Scale))
		//matModel=GetScale33( Vec3d(pObj->m_Scale.x,pObj->m_Scale.y,pObj->m_Scale.z) )*matModel;

	//OPTIMISED_BY_IVO
	//Matrix33diag diag	=	pObj->m_Scale;	//use diag-matrix for scaling
	//Matrix34 t				= Matrix34::GetTransMat34(pObj->m_Trans);
	//Matrix33 r33			=	Matrix33::GetRotationXYZ33( Deg2Rad(Ang3(+pObj->m_Angs[0],-pObj->m_Angs[1],+pObj->m_Angs[2])) );	
	//Matrix44 matModel	=	r33*t*diag;	//optimised concatenation: m34*t*diag

	//matModel	=	GetTransposed44( matModel); //TODO: remove this after E3 and use Matrix34 instead of Matrix44

	return pObj->m_Matrix;
}

//////////////////////////////////////////////////////////////////////////
// updates all heat sources (read from CGF, kept in the Model) in the given object
// The global ones can later be extracted via GetBoundLight() member of ICryCharInstance interface
void CryModelState::UpdateHeatSources (CCObject * pObj, const SRendParams & RendParams)
{
	SelfValidate();
	unsigned numBindings = m_pMesh->numBoneLights();

	// construct the DLights for the first time this function is called
	if (m_arrHeatSources.size() != numBindings)
	{
		m_arrHeatSources.reinit(numBindings);
		for (unsigned nBinding = 0; nBinding < numBindings; ++nBinding)
		{
			CBoneLightBindInfo& rBinding = m_pMesh->getBoneLight(nBinding);
			CryBone& rBone = getBone(rBinding.getBone());
			CDLight& rDLight = m_arrHeatSources[nBinding];
			rBinding.initDLight (rDLight);
		}
	}

	// the self-heat intensity of the character.
	// if it's 0, there's no point in adding the heat sources to the renderer
  float fHeatIntensity = 1.0f;
  if (pObj->m_ShaderParams)
    fHeatIntensity = SShaderParam::GetFloat("heatintensity", pObj->m_ShaderParams, -1);

	bool bHeatVisionMode = g_GetIRenderer()->EF_GetHeatVision();


	// the model matrix in the world space
	Matrix44 matModel = GetModelMatrix (pObj);
	
	// update DLight positions and add them to the renerer
	if (!bHeatVisionMode || fHeatIntensity > 0)
	for (unsigned nBinding = 0; nBinding < numBindings; ++nBinding)
	{
		CBoneLightBindInfo& rBinding = m_pMesh->getBoneLight(nBinding);

		if (bHeatVisionMode)
		{
			if (!rBinding.isHeatSource())
				continue;
		}
		else
		{
			if (!rBinding.isLightSource())
				continue;
		}
		
		CryBone& rBone = getBone(rBinding.getBone());
		CDLight& rDLight = m_arrHeatSources[nBinding];

		// unwrap (change #if 0 to #if 1) the following code and pass matModel to the updateDLight
		// instead of the rBone global matrix, in order to have the light position
		// returned in World coordinates
		rDLight.m_fDirectFactor = 0;

		Matrix44 matBone =  rBone.GetGlobalMatrix() * matModel;

		if (g_GetCVars()->ca_Debug()) {
			Matrix34 m34 = Matrix34(GetTransposed44(matBone)); //fix this
			debugDrawBBox (m34, CryAABB(Vec3d(-0.1f,-0.2f,-0.1f), Vec3d(0.1f, 0.2f, 0.1f)), 5);
		}

		rBinding.updateDLight (matBone, fHeatIntensity, rDLight);

		if (rBinding.isLocal())
		{
			g_GetIRenderer()->EF_ADDDlight (&rDLight);
			pObj->m_DynLMMask |= (1<<rDLight.m_Id);
		}
	}
}


// adds a new dynamically bound light
CDLight* CryModelState::AddDynBoundLight (ICryCharInstance* pParent,CDLight* pDLight, unsigned nBone, bool bCopyLight)
{
	// we don't support multiple instances of the same light bound
	// because we use the light pointer as the handle
	if (IsDynLightBound(pDLight) && !bCopyLight)
		return NULL;

	m_arrDynBoundLights.resize (m_arrDynBoundLights.size() + 1);
	m_arrDynBoundLights.back().init(pParent, pDLight, nBone, getBoneMatrixGlobal(nBone), bCopyLight);
	return m_arrDynBoundLights.back().getDLight();
}

// checks if such light is already bound
bool CryModelState::IsDynLightBound (CDLight*pDLight)
{
	for (DynamicBoundLightArray::iterator it = m_arrDynBoundLights.begin(); it != m_arrDynBoundLights.end(); ++it)
		if (it->getDLight() == pDLight)
			return true;
	return false;
}


//////////////////////////////////////////////////////////////////////////
// removes the dynamically bound light
void CryModelState::RemoveDynBoundLight (CDLight* pDLight)
{
	unsigned i;
	// scan through
	for (i = 0; i < m_arrDynBoundLights.size(); )
	{
		CBoneLightDynamicBind& rBinding = m_arrDynBoundLights[i];
		if (rBinding.getDLight() == pDLight)
		{
			rBinding.done();
			m_arrDynBoundLights.erase(m_arrDynBoundLights.begin() + i);
		}
		else
			++i;
	}
}

unsigned CryModelState::numDynBoundLights()const
{
	return (unsigned)m_arrDynBoundLights.size();
}

CDLight* CryModelState::getDynBoundLight(unsigned i)
{
	return m_arrDynBoundLights[i].getDLight();
}


void CryModelState::RemoveAllDynBoundLights()
{
	for (DynamicBoundLightArray::iterator it = m_arrDynBoundLights.begin(); it != m_arrDynBoundLights.end(); ++it)
		it->done();
	m_arrDynBoundLights.clear();
}


//////////////////////////////////////////////////////////////////////////
// updates the dynamically (via ICryCharInstance at runtime) bound lights
void CryModelState::UpdateDynBoundLights (CCObject * pObj, const SRendParams & RendParams)
{
	unsigned i, numDynBoundLights = (unsigned)m_arrDynBoundLights.size();
	if (!numDynBoundLights)
		return;

	Matrix44 matModel = GetModelMatrix (pObj);
	for (i = 0; i < numDynBoundLights; ++i)
	{
		CBoneLightDynamicBind& rBinding = m_arrDynBoundLights[i];

		rBinding.updateDLight (getBoneMatrixGlobal(rBinding.getBone()), matModel, 1);

		if (rBinding.isLocal())
		{
			CDLight& rDLight = *rBinding.getDLight();
			g_GetIRenderer()->EF_ADDDlight (&rDLight);
			pObj->m_DynLMMask |= (1<<rDLight.m_Id);
		}
	}
}


// Interface for the renderer - returns the CDLight describing the light in this character;
// returns NULL if there's no light with such index
const CDLight* CryModelState::getGlobalBoundLight (unsigned nIndex)
{
	SelfValidate();
  return &m_arrHeatSources[nIndex+m_pMesh->numLocalBoneLights()];
}


// Set the current time of the given layer, in seconds
void CryModelState::SetLayerTime (unsigned nLayer, float fTimeSeconds)
{
	AnimationLayer& layer = m_arrAnimationLayers[nLayer];
	if(!layer.pEffector)
		return;
	// sets the current time of the effector, translating seconds into ticks.
	layer.pEffector->SetCurrentTime (fTimeSeconds);
  ProcessAnimations(0,true,0);
}

// Get the current time of the given layer, in seconds
float CryModelState::GetLayerTime (unsigned nLayer) const
{
	const AnimationLayer& layer = m_arrAnimationLayers[nLayer];
	if(!layer.pEffector)
		return 0;
	// sets the current time of the effector, translating seconds into ticks.
	return layer.pEffector->GetCurrentTime();
}

// forgets about all default idle animations
void CryModelState::ForgetDefaultIdleAnimations()
{
	SelfValidate();
	for (AnimationLayerArray::iterator it = m_arrAnimationLayers.begin(); it != m_arrAnimationLayers.end(); ++it)
		it->ForgetDefaultIdleAnimation();
}


// sets the given aniimation to the given layer as the default
void CryModelState::SetDefaultIdleAnimation (unsigned nLayer, const char* szAnimName)
{
	SelfValidate();
	if (nLayer >= m_arrAnimationLayers.size())
		return;

	if (!szAnimName||!szAnimName[0])
	{
		m_arrAnimationLayers[nLayer].ForgetDefaultIdleAnimation();
		return;
	}

	int nAnimID = -1;
	if (szAnimName)
		nAnimID = m_pMesh->findAnimation(szAnimName);

	if (nAnimID >= 0 && m_arrAnimationLayers.size() <= nLayer)
		m_arrAnimationLayers.resize (nLayer+1);

	if (m_arrAnimationLayers.size() > nLayer)
		m_arrAnimationLayers[nLayer].nDefaultIdleAnimID = nAnimID;
}

#ifdef _DEBUG
// checks for possible memory corruptions in this object and its children
void CryModelState::SelfValidate ()const
{
}
#endif

// returns the approximate bounding box for this character in the passed in vectors
void CryModelState::GetBoundingBox (Vec3d& vMin, Vec3d& vMax) const
{
	vMin = m_BBox.vMin;
	vMax = m_BBox.vMax;
}


//////////////////////////////////////////////////////////////////////////
// Adds a decal to the character
void CryModelState::AddDecal (CryEngineDecalInfo& Decal)
{
	for (SubmeshArray::iterator it = m_arrSubmeshes.begin(); it != m_arrSubmeshes.end(); ++it)
		if (*it)(*it)->AddDecal(Decal);
}

void CryModelState::DumpDecals()
{
	//if (m_pDecalManager)
	//	m_pDecalManager->debugDump();
}

// discards all outstanding decal requests - the decals that have not been meshed (realized) yet
// and have no chance to be correctly meshed in the future
void CryModelState::DiscardDecalRequests()
{
	SelfValidate();
	for (SubmeshArray::iterator it = m_arrSubmeshes.begin(); it != m_arrSubmeshes.end(); ++it)
		if (*it)(*it)->DiscardDecalRequests();
}



Vec3d CryModelState::GetCenter ()const
{
	SelfValidate();
	return m_BBox.getCenter();
}





// given the bone index, (INDEX, NOT ID), returns this bone's parent index
int CryModelState::getBoneParentIndex (int nBoneIndex)
{
	assert (nBoneIndex >= 0 && nBoneIndex < (int)numBones());
	return nBoneIndex + getBoneInfo(nBoneIndex)->getParentIndexOffset();
}

CryModelState::AnimationLayer::AnimationLayer ():
	nDefaultIdleAnimID(-1),
	fDefaultAnimBlendTime(0),
	bEnableDefaultIdleAnimRestart (false),
	bKeepLayer0Phase(false)
{}

// forgets about the default idle animation
void CryModelState::AnimationLayer::ForgetDefaultIdleAnimation()
{
	nDefaultIdleAnimID = -1;
}


void CryModelState::initClass()
{
}

void CryModelState::deinitClass()
{
	//assert(g_arrEmptyAnimEventArray.empty());
	g_arrActiveLayers.clear();
}


ICharInstanceSink* CryModelState::getAnimationEventSink (int nAnimId)
{
	if (nAnimId>= 0 && nAnimId < (int)m_arrSinks.size())
		return m_arrSinks[nAnimId];
	else
		return NULL;
}

void CryModelState::setAnimationEventSink (int nAnimId, ICharInstanceSink* pSink)
{
	//g_GetLog()->LogToFile ("AnimDebug: 0x%08X->setAnimationEventSink(anim:%d, sink:%08X)", this, nAnimId, pSink);
	if (nAnimId>= 0 && nAnimId < (int)m_pMesh->numAnimations())
	{
		if ((int) m_arrSinks.size() <= nAnimId)
			m_arrSinks.resize (nAnimId+1, NULL);
		m_arrSinks[nAnimId] = pSink;
	}		
}


void CryModelState::removeAllAnimationEvents ()
{
	m_mapAnimEvents.clear();
}

void CryModelState::removeAnimationEventSink(ICharInstanceSink * pCharInstanceSink)
{
  for (std::vector<ICharInstanceSink*>::iterator it = m_arrSinks.begin(); it != m_arrSinks.end(); ++it)
    if (*it == pCharInstanceSink)
      *it = NULL;
}

// this is an empty array used to return the reference in teh getAnimEvents();
// it's always empty, so doesn't require cleaning and doesn't cause troubles with memory manager
// DEBUG NOTE: this is not a strict array, it's a class that takes care about assigning the parent to the array
static CryModelState::AnimEventArray g_arrEmptyAnimEventArray;

CryModelState::AnimEventArray& CryModelState::getAnimEvents(int nAnimId)
{
	return find_in_map_ref (m_mapAnimEvents, nAnimId, g_arrEmptyAnimEventArray);
}

void CryModelState::addAnimEvent (int nAnimId, int nFrame, AnimSinkEventData UserData)
{
	if (g_GetCVars()->ca_Debug())
		g_GetLog()->LogToFile ("\003AnimDebug: 0x%08X->addAnimEvent(anim:%d, frame:%d, userdata:%p,%d) model %s", this, nAnimId, nFrame, UserData.p, UserData.n, m_pMesh->getFilePathCStr());

	if (nAnimId >= 0 && nAnimId < (int)m_pMesh->numAnimations())
	{
		const AnimData* pAnim = m_pMesh->getAnimationInfo(nAnimId);
		if (!pAnim)
			return;

		AnimEventArray& arrEvents = m_mapAnimEvents[nAnimId];
		AnimEvent event;
		event.UserData = UserData;
		event.fTime    = pAnim->fStart + nFrame * m_pMesh->getSecondsPerFrame();

		AnimEventArray::iterator it, itBegin = arrEvents.begin(), itEnd = itBegin + arrEvents.size();
		//it = std::lower_bound(itBegin, itEnd, event);
		for (it = itBegin; it != itEnd; ++it)
			if (isEqual(*it, event))
				return;
		arrEvents.push_back(event);
	}
}

void CryModelState::removeAnimEvent (int nAnimId, int nFrame, AnimSinkEventData UserData)
{
	//g_GetLog()->LogToFile ("AnimDebug: 0x08X->removeAnimEvent(anim:%d, frame:%d, userdata:%d)", this, nAnimId, nFrame, nUserData);
	if (nAnimId >= 0 && nAnimId < (int)m_pMesh->numAnimations())
	{

	}
}

// draws the skeleton
void CryModelState::debugDrawBones(const Matrix44* pModelMatrix)
{
	static float fColorBones[4] = {1,0.75f,0.75f,0.75};
	static float fColorBoneBBWithPhysics[4] = {0.5f,0.5f,1,0.75};
	static float fColorBoneBBWithoutPhysics[4] = {0.65f,0.65f,0.65f,0.75};

	if (!pModelMatrix)
		pModelMatrix = &m_ModelMatrix44;

	debugDrawRootBone (getBone(0).GetGlobalMatrix() * *pModelMatrix, 0.02f, fColorBones);
	for (unsigned nBone = 1; nBone < numBones(); ++nBone)
	{
		int nParentBone = getBoneParentIndex(nBone);
		Matrix44 matBoneGlobalWCS = getBone(nBone).GetGlobalMatrix() * *pModelMatrix;
		debugDrawBone (matBoneGlobalWCS, getBone(nParentBone).GetGlobalMatrix() * *pModelMatrix, fColorBones);
		
		if (int((g_GetTimer()->GetCurrTime()+m_nInstanceNumber*0.2f)*3)&1)
		{
			fColorBoneBBWithPhysics[3] = 1;
			fColorBoneBBWithoutPhysics[3] = 0.5f;
		}
		else
		{
			fColorBoneBBWithPhysics[3] = 0.75f;
			fColorBoneBBWithoutPhysics[3] = 0.75f;
		}

		const CryBBoxA16* pBoneBBoxes = m_pMesh->getBoneBBoxes ();

		if (pBoneBBoxes)
		{
			const CryBoneInfo &rBoneInfo = m_pMesh->getBoneInfo(nBone);
			Matrix34 m34=Matrix34( GetTransposed44(matBoneGlobalWCS) );

			CryAABB caabb;
			caabb.vMin=pBoneBBoxes[nBone].vMin.v;
			caabb.vMax=pBoneBBoxes[nBone].vMax.v;

			debugDrawBBox (m34, caabb, 1, rBoneInfo.m_PhysInfo[0].pPhysGeom?fColorBoneBBWithPhysics:fColorBoneBBWithoutPhysics);
		}
	}
}

void CryModelState::debugDrawBoundingBox(const Matrix44* pModelMatrix, int nBBoxSegments)
{
	static const float fColorBBox[4] = {1,1,1,1};

	if (!pModelMatrix)
		pModelMatrix = &m_ModelMatrix44;

	// this draws the bounding box around the character
	Matrix34 m34=Matrix34(GetTransposed44(*pModelMatrix));
	debugDrawBBox (m34, m_BBox, nBBoxSegments, fColorBBox);

	int nAxis[3];
	for (nAxis[0] = 0; nAxis[0] < 3; ++nAxis[0])
	{
		nAxis[1] = (nAxis[0]+1)%3;
		nAxis[2] = (nAxis[0]+2)%3;
		Vec3d vDown(0,0,0), vUp(0,0,0);
		vDown[nAxis[0]] = m_BBox.vMin[nAxis[0]];
		float fUp = m_BBox.vMax[nAxis[0]];
		vUp[nAxis[0]] = fUp+0.05f;
		float fColor[4] = {1,1,1,1};
		fColor[nAxis[1]] = fColor[nAxis[2]] = 0.75;
		debugDrawLine (*pModelMatrix, vDown, vUp, fColor);
		Vec3d vA;
		vA[nAxis[0]] = m_BBox.vMax[nAxis[0]]-0.025f;
		for (int i= 0; i < 4; ++i)
		{
			for (int j = 0; j < 2; ++j)
				vA[nAxis[j+1]] = (i&(1<<j))?0.025f:-0.025f;
			debugDrawLine (*pModelMatrix, vUp, vA, fColor);
		}
	}
}



//returns the j-th child of i-th child of the given bone
CryBone* CryModelState::getBoneChild (int nBone, int i)
{
	return &getBone(getBoneChildIndex(nBone, i));
}


//returns the j-th child of i-th child of the given bone
CryBone* CryModelState::getBoneGrandChild (int nBone, int i, int j)
{
	return &getBone(getBoneGrandChildIndex(nBone, i, j));
}

//returns the j-th child of i-th child of the given bone
int CryModelState::getBoneChildIndex (int nBone, int i)
{
	assert (i >= 0 && i < (int)getBoneInfo(nBone)->numChildren());
	return nBone + getBoneInfo(nBone)->getFirstChildIndexOffset() + i;
}


//returns the j-th child of i-th child of the given bone
int CryModelState::getBoneGrandChildIndex (int nBone, int i, int j)
{
	return getBoneChildIndex(getBoneChildIndex(nBone, i), j);
}


/*
CryModel* CryModelState::getAnimationSet()
{
	SelfValidate();
	return m_pMesh;
}
*/

// calculates the mem usage
void CryModelState::GetSize(ICrySizer* pSizer)
{
#if ENABLE_GET_MEMORY_USAGE
	pSizer->AddContainer(g_arrActiveLayers);
	
	unsigned i;
	size_t nSize = sizeof(*this);
	nSize += sizeofArray (m_arrAnimationLayers);
	nSize += sizeofArray (m_arrBoneGlobalMatrices, numBones());
	nSize += sizeofArray (m_arrBones);
	nSize += sizeofArray (m_arrHeatSources);
	//nSize += sizeofArray (m_arrMorphEffectors);
	//nSize += sizeofArray (m_arrShaderTemplates[0]);
	//nSize += sizeofArray (m_arrShaderTemplates[1]);
	nSize += sizeofArray (m_arrSinks);
	nSize += m_mapAnimEvents.size() * sizeof(AnimEventMap::value_type);
	for (i = 0; i < 4; ++i)
	if (m_pIKEffectors[i])
		nSize += sizeof(CCryModEffIKSolver);

	pSizer->AddObject(this, nSize);

	//if (m_pDecalManager)
	//{
	//	SIZER_SUBCOMPONENT_NAME(pSizer, "Decals");
	//	m_pDecalManager->GetMemoryUsage(pSizer);
	//}
	/*
	{
		SIZER_SUBCOMPONENT_NAME(pSizer, "Materials");
		for (i = 0; i < g_nMaxGeomLodLevels; ++i)
		{
			CLeafBuffer *lb = m_pLeafBuffers[i];
			list2<CMatInfo>* pMats;
			if (lb && (pMats = lb->m_pMats))
				pSizer->Add(pMats, pMats->capacity());
		}
	}

	{
		SIZER_SUBCOMPONENT_NAME (pSizer,"Shadows&Particles");
		m_ReShadowManager.GetMemoryUsage (pSizer);
		m_ParticleManager.GetMemoryUsage (pSizer);
	}
	*/
	// TODO: getsize on each submesh
#endif
}

void CryModelState::setAnimationSpeed (unsigned nLayer, float fSpeed)
{
	if (nLayer > 16 || !(fSpeed>=0 && fSpeed < 1e6))
	{
		g_GetLog()->LogError ("\002invalid layer %d speed scale %g (0x%08X)", nLayer, fSpeed, *(unsigned*)&fSpeed);
		return;
	}

	if (m_arrLayerSpeedScale.size() <= nLayer)
		m_arrLayerSpeedScale.resize (nLayer+1, 1);
	m_arrLayerSpeedScale[nLayer] = fSpeed;
}

float CryModelState::getAnimationSpeed (unsigned nLayer)
{
	if (nLayer >= m_arrLayerSpeedScale.size())
		return 1;
	return m_arrLayerSpeedScale[nLayer];
}


void CryModelState::setScale (const Vec3d& vScale)
{
	m_vRuntimeScale = vScale;
#ifdef _DEBUG
	for (int i = 0; i < 3; ++i)
		if (!(m_vRuntimeScale[i] > 1e-3 && m_vRuntimeScale[i] < 1e3))
			g_GetLog()->LogWarning ("\003character scale[%d] is out of range: %g", i, m_vRuntimeScale[i]);
#endif

	bool bParityOdd = vScale.x * vScale.y * vScale.z < 0; // can replace with a XOR
	if (bParityOdd)
	{
		m_pShaderStateCull = g_GetIRenderer()->EF_LoadShader("FrontCull", eSH_World, EF_SYSTEM);
		m_pShaderStateShadowCull = g_GetIRenderer()->EF_LoadShader("StencilState_FrontCull", eSH_World, EF_SYSTEM);
	}
	else
	{
		m_pShaderStateCull = NULL;
		m_pShaderStateShadowCull = NULL;
	}
}

// cleans up all decals
void CryModelState::ClearDecals()
{
	for (SubmeshArray::iterator it = m_arrSubmeshes.begin(); it != m_arrSubmeshes.end(); ++it)
		if (*it)(*it)->ClearDecals();
}

void CryModelState::DumpState()
{
	string strLayers;
	size_t i;
	for (i = 0; i < m_arrAnimationLayers.size(); ++i)
	{
		AnimationLayer& rLayer = m_arrAnimationLayers[i];
		if (rLayer.pEffector)
		{
			strLayers += "  " + rLayer.pEffector->dump();

			if (rLayer.bEnableDefaultIdleAnimRestart)
			{
				strLayers += ":";
				if (rLayer.bKeepLayer0Phase)
					strLayers += " KeepLayer0Phase";

				char szBuf[32];
				sprintf (szBuf, "%g", rLayer.fDefaultAnimBlendTime );

				strLayers += " Idle blend: ";
				strLayers += szBuf;

        if (rLayer.nDefaultIdleAnimID >= 0)
				{
					const AnimData* pAnim = m_pMesh->getAnimationInfo(rLayer.nDefaultIdleAnimID);
					strLayers += " Idle:" + pAnim->strName;

					if (pAnim->bLoop)
						strLayers += " (looped)";
				}

				strLayers += "; ";
			}
			else
				strLayers += "(no def idle)";

			if (i < m_arrLayerSpeedScale.size())
			{
				float fSpeed = m_arrLayerSpeedScale[i];
				if (fSpeed != 1)
				{
					char szBuf[32];
					sprintf (szBuf, "*(%g)", fSpeed);
					strLayers += szBuf;
				}
			}
		}
		else
			strLayers += "  <Idle>";
	}
	g_GetLog()->LogToFile("\001--ModelState %p-----------------------", this);
	g_GetLog()->LogToFile("\001%u layers: %s", m_arrAnimationLayers.size(), strLayers.c_str());

	/*string strMorphs;
	for (i = 0; i < m_arrMorphEffectors.size(); ++i)
	{
		if (i)
			strMorphs += "  ";
		char szBuf[128];
		sprintf (szBuf, "%u. ", i);
		strMorphs += szBuf;
		CryModEffMorph& rMorph = m_arrMorphEffectors[i];
		if (rMorph.isActive())
		{
			sprintf (szBuf, "\"%s\" w=%.2f, t=%.2f", m_pMesh->GetNameMorphTarget(rMorph.getMorphTargetId()), rMorph.getBlending(), rMorph.getTime());
			strMorphs += szBuf;
		}
		else
			strMorphs += "Off";
	}

	g_GetLog()->LogToFile("\001%u morphs: %s", m_arrMorphEffectors.size(), strMorphs.c_str());
	*/
	g_GetLog()->LogToFile ("\002 %d bones", m_arrBones.size());
	for (unsigned i = 0; i < m_arrBones.size(); ++i)
	{
		g_GetLog()->LogToFile ("\003  \"%s\" relative %s, global %s pq %s%s%s",
			m_pMesh->getBoneInfo(i).getNameCStr(),
			toString(getBone(i).m_matRelativeToParent).c_str(),
			toString(getBoneMatrixGlobal(i)).c_str(),
			getBone(i).m_pqTransform.toString().c_str(),
			getBone(i).m_bUseMatPlus?" (Mat+)":"",
			getBone(i).m_bUseReadyRelativeToParentMatrix?" (Use Ready Matrix)":"");
	}
}

// this is the array that's returned from the LeafBuffer
list2<CMatInfo>* CryModelState::getLeafBufferMaterials()
{
	return GetCryModelSubmesh(0)->getLeafBufferMaterials();
}

void CryModelState::PreloadResources(float fDistance, float fTime, int nFlags)
{
	for (SubmeshArray::iterator it = m_arrSubmeshes.begin(); it != m_arrSubmeshes.end(); ++it)
		if (*it)(*it)->PreloadResources(fDistance, fTime, nFlags);
}

// Forces skinning on the next frame
void CryModelState::ForceReskin()
{
	m_uFlags |= nFlagsNeedReskinAllLODs;
	m_bPhysicsWasAwake = true;
}

// the model that's coherent with the current model state: bones etc. are taken from there
CryModel* CryModelState::GetModel()
{
	// TODO: maybe it's faster to cache this pointer
	return m_arrSubmeshes.empty()?NULL:m_arrSubmeshes[0]->GetCryModel();
}
const CryModel* CryModelState::GetModel()const
{
	// TODO: maybe it's faster to cache this pointer
	return m_arrSubmeshes.empty()?NULL:m_arrSubmeshes[0]->GetCryModel();
}

CryModelSubmesh* CryModelState::AddSubmesh (ICryCharModel* pModel, bool bVisible)
{
	if (pModel->GetClass() != ICryCharModel::CLASS_CRYCHARBODY)
		return NULL; // not supported

	return AddSubmesh (static_cast<CryCharBody*>(pModel)->GetModel(), bVisible);
}

CryModelSubmesh* CryModelState::SetSubmesh (unsigned nSlot, ICryCharModel* pModel, bool bVisible)
{
	if (!pModel)
	{
		RemoveSubmesh (nSlot);
		return NULL; 
	}

	if (pModel->GetClass() != ICryCharModel::CLASS_CRYCHARBODY)
		return NULL; // not supported

	return SetSubmesh (nSlot,static_cast<CryCharBody*>(pModel)->GetModel(), bVisible); 
}


CryModelSubmesh* CryModelState::SetSubmesh (unsigned nSlot, CryModel* pCryModel, bool bVisible)
{
	if (nSlot & ~0xFF)
		return NULL; // we don't support more than 256 submeshes

	if (m_arrSubmeshes.size() <= nSlot)
		m_arrSubmeshes.resize (nSlot+1);

	CryModelSubmesh* pNewSubmesh = new CryModelSubmesh(this, pCryModel);
	pNewSubmesh->SetVisible(bVisible);
	m_arrSubmeshes[nSlot] = pNewSubmesh;
	return pNewSubmesh;
}

// adds a submesh, returns handle to it which can be used to delete the submesh
// submesh is created either visible or invisible
// submesh creation/destruction is heavy operations, so the clients must use they rarely,
// and set visible/invisible when they need to turn them on/off
// But creating many submeshes is memory-consuming so the number of them must be kept low at all times
CryModelSubmesh* CryModelState::AddSubmesh (CryModel* pCryModel, bool bVisible)
{	
	// TODO: thorough check for compatibility between the models here
	if (!IsEmpty() && pCryModel->numBoneInfos() != GetCryModelSubmesh(0)->GetCryModel()->numBoneInfos())
	{
		g_GetLog()->LogError ("Trying to add submesh %s to an existing instance with main model %s: incompatible skeletons", pCryModel->getFilePathCStr(), GetCryModelSubmesh(0)->GetCryModel()->getFilePathCStr());
		return NULL;
	}
	
	CryModelSubmesh* pNewSubmesh = new CryModelSubmesh(this, pCryModel);
	pNewSubmesh->SetVisible(bVisible);

	// find a free slot to insert the new submesh
	for (unsigned nSlot = 1; nSlot < m_arrSubmeshes.size(); ++nSlot)
		if (!m_arrSubmeshes[nSlot])
		{
			m_arrSubmeshes[nSlot] = pNewSubmesh;
			return pNewSubmesh;
		}

  m_arrSubmeshes.push_back (pNewSubmesh);
	return pNewSubmesh;
}

// removes submesh from the character
void CryModelState::RemoveSubmesh (ICryCharSubmesh* pSubmesh)
{
	// we may not delete the 0th submesh
	for (SubmeshArray::iterator it = m_arrSubmeshes.begin()+1; it != m_arrSubmeshes.end(); ++it)
		if (*it == pSubmesh)
			*it = NULL;
}

void CryModelState::RemoveSubmesh(unsigned nSlot)
{
	if (nSlot > 1 && nSlot < m_arrSubmeshes.size())
	{
		m_arrSubmeshes[nSlot] = NULL;

		// we may auto-shrink the array
		while (!m_arrSubmeshes.empty() && !m_arrSubmeshes.back())
			m_arrSubmeshes.resize (m_arrSubmeshes.size()-1);
	}
}

ICryCharSubmesh* CryModelState::GetSubmesh(unsigned i)
{
#if defined(LINUX)
	ICryCharSubmesh* pRes( 0 );
	if( i < m_arrSubmeshes.size() )
	{
		pRes = m_arrSubmeshes[i];
	}
	return( pRes );
#else
	return i < m_arrSubmeshes.size() ? m_arrSubmeshes[i]:NULL;
#endif
}

CryModelSubmesh* CryModelState::GetCryModelSubmesh(unsigned i)
{
#if defined(LINUX)
	CryModelSubmesh* pRes( 0 );
	if( i < m_arrSubmeshes.size() )
	{
		pRes = m_arrSubmeshes[i];
	}
	return( pRes );
#else
	return i < m_arrSubmeshes.size() ? m_arrSubmeshes[i]:NULL;
#endif
}

bool CryModelState::SetShaderTemplateName (const char *TemplName, int Id, const char *ShaderName,IMatInfo *pCustomMaterial,unsigned nFlags)
{
	bool bRes = false;
	for (SubmeshArray::iterator it = m_arrSubmeshes.begin(); it != m_arrSubmeshes.end(); ++it)
		if (*it)
			bRes = (*it)->SetShaderTemplateName (TemplName, Id, ShaderName, pCustomMaterial,nFlags) || bRes;
	return bRes;
}

CLeafBuffer* CryModelState::GetLeafBuffer ()
{
	return GetCryModelSubmesh(0)->GetLeafBuffer();
}

void CryModelState::SetShaderFloat(const char *Name, float fVal, const char *ShaderName)
{
	for (SubmeshArray::iterator it = m_arrSubmeshes.begin(); it != m_arrSubmeshes.end(); ++it)
		if (*it)(*it)->SetShaderFloat (Name, fVal, ShaderName);
}

int CryModelState::numLODs() // number of LODs in the 0th submesh
{
	return GetCryModelSubmesh(0)->GetCryModel()->numLODs();
}

//Calculate shadow volumes,fill buffers and render shadow volumes into the stencil buffer
//TODO: Optimize everything
//////////////////////////////////////////////////////////////////////
void CryModelState::RenderShadowVolumes (const SRendParams *rParams, int nLimitLOD)
{
	assert(Get3DEngine());						// should be always there

	for (SubmeshArray::iterator it = m_arrSubmeshes.begin(); it != m_arrSubmeshes.end(); ++it)
		if (*it)(*it)->RenderShadowVolumes(rParams, nLimitLOD);
}


// renders the decals - adds the render element to the renderer
/*
void CryModelState::AddDecalRenderData (CCObject *pObj, const SRendParams & rRendParams)
{
	if (m_nLodLevel == 0) // we don't render the character decals in lod!=0
	{
		for (SubmeshArray::iterator it = m_arrSubmeshes.begin(); it != m_arrSubmeshes.end(); ++it)
			if (*it)(*it)->AddDecalRenderData (pObj, rRendParams);
	}
}
*/


class CryCharFxTrail* CryModelState::NewFxTrail (unsigned nSlot, const struct CryCharFxTrailParams& rParams)
{
	if (m_arrFxTrails.size() <= nSlot)
		m_arrFxTrails.resize (nSlot+1);
	return m_arrFxTrails[nSlot] = new CryCharFxTrail(this, rParams);
}

void CryModelState::RemoveFxTrail (unsigned nSlot)
{
	if (nSlot < m_arrFxTrails.size())
	{
		m_arrFxTrails[nSlot] = NULL;

		// remove unnecessary slots
#if !defined(LINUX64)
		while (!m_arrFxTrails.empty() && m_arrFxTrails.back() == NULL)
#else
		while (!m_arrFxTrails.empty() && m_arrFxTrails.back() == 0)
#endif
			m_arrFxTrails.resize (m_arrFxTrails.size()-1);
	}
}
