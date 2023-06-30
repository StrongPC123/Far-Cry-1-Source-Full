#include "stdafx.h"
#include "System.h"
#include "ScriptObjectSystem.h"
#include "ScriptObjectParticle.h"
#include "ScriptObjectAnimation.h"
#include "ScriptObjectSound.h"
#include "ScriptObjectMovie.h"
#include "ScriptObjectScript.h"
#include "ScriptObjectEntity.h"
#include "HTTPDownloader.h"
#include <IEntitySystem.h>


struct CScriptBindings
{

	CScriptBindings();
	
	~CScriptBindings(){}
	bool Init(CSystem *pSystem);
	bool ShutDown();
private:
	CScriptObjectSystem *m_pScriptObjectSystem;
	CScriptObjectParticle *m_pScriptObjectParticle;
	CScriptObjectAnimation *m_pScriptObjectAnimation;
	CScriptObjectSound *m_pScriptObjectSound;
	CScriptObjectMovie *m_pScriptObjectMovie;
	CScriptObjectScript *m_pScriptObjectScript;
};

CScriptBindings::CScriptBindings()
{
	m_pScriptObjectSystem=NULL;
	m_pScriptObjectParticle=NULL;
	m_pScriptObjectAnimation=NULL;
	m_pScriptObjectSound=NULL;
	m_pScriptObjectMovie=NULL;
	m_pScriptObjectScript=NULL;
}

bool CScriptBindings::Init(CSystem *pSystem)
{
	IScriptSystem *pSS=pSystem->GetIScriptSystem();
//SYSTEM
	CScriptObjectSystem::InitializeTemplate(pSS);
	m_pScriptObjectSystem=new CScriptObjectSystem;
	m_pScriptObjectSystem->Init(pSS,pSystem);
//PARTICLE
	CScriptObjectParticle::InitializeTemplate(pSS);
	m_pScriptObjectParticle=new CScriptObjectParticle;
	m_pScriptObjectParticle->Init(pSS,pSystem);
//ANIMATION
	CScriptObjectAnimation::InitializeTemplate(pSS);
	m_pScriptObjectAnimation=new CScriptObjectAnimation;
	m_pScriptObjectAnimation->Init(pSS,pSystem);
//SOUND	
	CScriptObjectSound::InitializeTemplate(pSS);
	m_pScriptObjectSound=new CScriptObjectSound;
	m_pScriptObjectSound->Init(pSS,pSystem);
//MOVIE
	CScriptObjectMovie::InitializeTemplate(pSS);
	m_pScriptObjectMovie=new CScriptObjectMovie;
	m_pScriptObjectMovie->Init(pSS,pSystem);
//SCRIPT
	CScriptObjectScript::InitializeTemplate(pSS);
	m_pScriptObjectScript=new CScriptObjectScript;
	m_pScriptObjectScript->Init(pSS);
//ENTITY
	CScriptObjectEntity::InitializeTemplate(pSS);
//DOWNLOAD
#if !defined(LINUX)
	CHTTPDownloader::InitializeTemplate(pSS);
#endif
	return true;
}

bool CScriptBindings::ShutDown()
{
	CScriptObjectSystem::ReleaseTemplate();
	SAFE_DELETE(m_pScriptObjectSystem);
	CScriptObjectParticle::ReleaseTemplate();
	SAFE_DELETE(m_pScriptObjectParticle);
	CScriptObjectAnimation::ReleaseTemplate();
	SAFE_DELETE(m_pScriptObjectAnimation);
	CScriptObjectSound::ReleaseTemplate();
	SAFE_DELETE(m_pScriptObjectSound);
	CScriptObjectMovie::ReleaseTemplate();
	SAFE_DELETE(m_pScriptObjectMovie);
	CScriptObjectScript::ReleaseTemplate();
	SAFE_DELETE(m_pScriptObjectScript);
	CScriptObjectEntity::ReleaseTemplate();
#if !defined(LINUX)
	CHTTPDownloader::ReleaseTemplate();
#endif
	return true;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
bool CSystem::InitScriptBindings()
{
	m_pScriptBindings=new CScriptBindings;
	return m_pScriptBindings->Init(this);
}

bool CSystem::ShutDownScriptBindings()
{
	if(m_pScriptBindings)
	{
		bool bres=m_pScriptBindings->ShutDown();
		delete m_pScriptBindings;
		return bres;
	}
	return false;
}

void CSystem::CreateEntityScriptBinding(IEntity *ent)
{
	CScriptObjectEntity *pSEntity = new CScriptObjectEntity();
	pSEntity->Create(m_pScriptSystem, this);
	assert(pSEntity->GetScriptObject());
	if (ent->GetContainer()){
		IScriptObject *pObj=ent->GetContainer()->GetScriptObject();
		if(pObj)
			pSEntity->SetContainer(pObj);
	}
	assert(pSEntity->GetScriptObject());
	pSEntity->SetEntity(ent);
	assert(pSEntity->GetScriptObject());
	ent->SetScriptObject(pSEntity->GetScriptObject());
	assert(pSEntity->GetScriptObject());
	assert(ent->GetScriptObject());
}