//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:CryCharBody.cpp
//  Implementation of CryCharBody class
//
//	History:
//	August 15, 2002: Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
//#include "CryAnimation.h"
#include "CryAnimationScriptCommands.h"
#include "CryCharManager.h"
#include "CryCharInstance.h"
#include "StringUtils.h"
#include "CryCharBody.h"
#include "CryModelState.h"
#include "CryModelLoader.h"

using namespace CryStringUtils;


///////////////////////////////////////////////////////////////////////////////////////////////////////
// CryCharBody
///////////////////////////////////////////////////////////////////////////////////////////////////////

CryCharBody::CryCharBody (CryCharManager* pManager, const string& strFileName):
	m_pCryModel (NULL),
	m_fAnimationFrameRate (30), // the default animation rate
	m_strFilePath (strFileName), // we're optimistic and think that the object will be loaded successfully
	m_pManager (pManager)
{
	string strGeom = strFileName;
	ReplaceExtension (strGeom, "cgf");

	string strCid = strFileName;
	ReplaceExtension (strCid, "cid");

	CryModelLoader ModelLoader (pManager->GetControllerManager()); //GetRenderer()->CreateModel();

	//m_fAnimationFrameRate = GetConsoleVariable ("FrameRate", strCid, m_fAnimationFrameRate);
	float fScale = g_fDefaultAnimationScale; //GetConsoleVariable ("Scale", strCid, g_fDefaultAnimationScale);

	m_pCryModel = ModelLoader.loadNew(this, strGeom, fScale);
	
#if 0
	// needed for XBox development
	extern void exportTestModel(CryGeometryInfo* pGeometry, CLeafBuffer* pLeafBuffer);
	// on this stage, if m_pCryModel == NULL, it means the loader couldn't load the model
	if (m_pCryModel) 
		exportTestModel( m_pCryModel->getGeometryInfo(), m_pCryModel->m_pDefaultModelState->m_pLeafBuffers[0]);
#endif

	pManager->RegisterBody (this);
}


CryCharBody::~CryCharBody()
{
	if (!m_setInstances.empty())
	{
		g_GetLog()->LogToFile("*ERROR* ~CryCharBody(%s): %u character instances still not deleted. Forcing deletion.", m_strFilePath.c_str(), m_setInstances.size());
		CleanupInstances();
	}

	//GetLog()->LogToFile("Deleting body \"%s\" (%d references)", m_strFilePath.c_str(), m_nRefCounter);
	//GetRenderer()->DeleteModel(m_pCryModel);
	if (m_pCryModel)
	{
		delete m_pCryModel;
		m_pCryModel = NULL;
	}

	m_pManager->UnregisterBody (this);
}

CryModel *CryCharBody::GetModel()
{
	return m_pCryModel;
}

const string& CryCharBody::GetFilePath()const
{
	return m_strFilePath;
}

const char* CryCharBody::GetFilePathCStr()const
{
	return m_strFilePath.c_str();
}

const char* CryCharBody::GetNameCStr()const
{
	return FindFileNameInPath(m_strFilePath.c_str());
}

float CryCharBody::GetFrameRate ()const
{
	return m_fAnimationFrameRate;
}

void CryCharBody::RegisterInstance (CryCharInstance* pInstance)
{
	m_setInstances.insert (pInstance);
}

void CryCharBody::UnregisterInstance (CryCharInstance* pInstance)
{
#ifdef _DEBUG
	if (m_setInstances.find(pInstance) == m_setInstances.end())
	{
		g_GetLog()->LogToFile("*ERROR* CryCharBody::UnregisterInstance(0x%x): twice-deleting the character instance 0x%08X!!!", this, pInstance);
	}
#endif
	m_setInstances.erase(pInstance);
}

// destroys all characters
// THis may (and should) lead to destruction and self-deregistration of this body
void CryCharBody::CleanupInstances()
{
	// since after each instance deletion the body may be destructed itself,
	// we'll lock it for this time
	if (!m_setInstances.empty())
		g_GetISystem()->Warning (VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, 0, "CryCharBody.CleanupInstances", "Forcing deletion of %d instances for body %s. CRASH POSSIBLE because other subsystems may have stored dangling pointer(s).", NumRefs(), m_strFilePath.c_str());
	CryCharBody_AutoPtr pLock = this; // don't remove this line, it's for locking the body in memory untill every instance is finished.
	while (!m_setInstances.empty())
		delete *m_setInstances.begin();
}

// Returns the scale of the model - not used now
float CryCharBody::GetScale() const
{
  return 1.0f;
}

// Returns the interface for animations applicable to this model
ICryAnimationSet* CryCharBody::GetAnimationSet ()
{
	return m_pCryModel->GetAnimationSet();
}

// Return name of bone from bone table, return zero id nId is out of range (the game gets this id from physics)
const char * CryCharBody::GetBoneName(int nId) const
{
	if ((unsigned)nId < m_pCryModel->numBoneInfos())
		return m_pCryModel->getBoneInfo (nId).getNameCStr();
	else
		return ""; // invalid bone id
}

//! Returns the number of bones; all bone ids are in the range from 0 to this number exclusive; 0th bone is the root
int CryCharBody::NumBones() const
{
	return m_pCryModel->numBoneInfos();
}

void CryCharBody::GetSize(ICrySizer* pSizer)
{
#if ENABLE_GET_MEMORY_USAGE
	if (!pSizer->Add (*this))
		return;
	pSizer->AddString (m_strFilePath);
	m_pCryModel->GetSize(pSizer);
	pSizer->AddContainer (m_setInstances);
	CryCharInstanceRegistry::const_iterator it = m_setInstances.begin(), itEnd = m_setInstances.end();
	for (; it != itEnd; ++it)
		(*it)->GetMemoryUsage(pSizer);
#endif
}

// makes all character instances spawn some particles (for test purposes)
void CryCharBody::SpawnTestParticles(bool bStart)
{
	ParticleParams params;
	params.fFocus = 1.5f;
	params.fLifeTime = 0.5f;
	params.fSize = 0.02f;
	params.fSizeSpeed = 0;
	params.fSpeed = 1.f;
	params.vGravity(0,0,-5.f);
	params.nCount = 15;
	params.eBlendType = ParticleBlendType_ColorBased;
	params.nTexId = g_GetIRenderer()->LoadTexture("cloud");
	float fAlpha = 1;
	params.vColorStart(fAlpha,fAlpha,fAlpha);
	params.vColorEnd(fAlpha,fAlpha,fAlpha);
	params.vDirection(0,0,1);
	params.vPosition = Vec3d (1,0.85f,0.75f);
	params.fTailLenght = 0.25;

	CryParticleSpawnInfo spawn;
	spawn.fSpawnRate = 30;
	spawn.nFlags = CryParticleSpawnInfo::FLAGS_SPAWN_FROM_BONE;
	spawn.nBone = GetBoneByName("weapon_bone");
	spawn.vBonePos = Vec3d(0,0,0);

	CryCharInstanceRegistry::iterator it = m_setInstances.begin(), itEnd = m_setInstances.end();
	for (; it != itEnd; ++it)
	{
		if (bStart)
			(*it)->AddParticleEmitter(params, spawn);
		else 
			(*it)->RemoveParticleEmitter (-1);
	}
}

//! returns the file name of the character model
const char* CryCharBody::GetFileName()
{
	return m_strFilePath.c_str();
}

// dumps the model info into the log, one line
void CryCharBody::DumpModel()
{
	g_GetLog()->LogToFile    ("\001%60s  %3d instance%s, %3d animations (%3d unique)", m_strFilePath.c_str(), m_setInstances.size(), m_setInstances.size()==1?"":"s", m_pCryModel->numAnimations(), m_pCryModel->numUniqueAnimations());
	g_GetLog()->LogToConsole ("\001%60s  %3d instance%s, %3d animations (%3d unique)", m_strFilePath.c_str(), m_setInstances.size(), m_setInstances.size()==1?"":"s", m_pCryModel->numAnimations(), m_pCryModel->numUniqueAnimations());
}

//! Returns the index of the bone by its name or -1 if no such bone exists; this is Case-Sensitive
int CryCharBody::GetBoneByName (const char* szName)
{
	return GetModel()->findBone(szName);
}

//Executes a per-body script command
bool CryCharBody::ExecScriptCommand (int nCommand, void* pParams, void* pResult)
{
	if (CASCMD_DUMP_MODELS == nCommand)
	{
		DumpModel();
		return true;
	}

	if (CASCMD_EXPORT_MODELS_ASCII == nCommand)
	{
		m_pCryModel->ExportModelsASC();
		return true;
	}

	switch (nCommand)
	{
	case CASCMD_DUMP_DECALS:
	case CASCMD_DUMP_STATES:
		g_GetLog()->Log    ("\001%60s  %3d instance%s", m_strFilePath.c_str(), m_setInstances.size(), m_setInstances.size()==1?"":"s");
		break;
	}

	CryCharInstanceRegistry::iterator it = m_setInstances.begin(), itEnd = m_setInstances.end();
	for (; it != itEnd; ++it)
	{
		(*it)->ExecScriptCommand(nCommand, pParams, pResult);
	}
	return true;
}

