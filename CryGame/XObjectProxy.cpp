
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xobjectproxy.h"
#include "xplayer.h"

CXObjectProxy::CXObjectProxy(IEntity *pEntity, IScriptSystem *pSystem)
{
	m_pScriptSystem = pSystem;
	m_pEntity = pEntity;

	if (m_pEntity->GetContainer())
	{
		if (!m_pEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &m_pPlayer))
			m_pPlayer = 0;
	}
	else
		m_pPlayer = 0;
}

CXObjectProxy::~CXObjectProxy(void)
{
}

// Sets the name of the function that will be called when an incoming signal is intercepted
void CXObjectProxy::SetSignalFuncName(const char * szName)
{
}

int CXObjectProxy::Update(SOBJECTSTATE * state)
{
	while (!state->vSignals.empty())
	{
		AISIGNAL sstruct = state->vSignals.back();
		state->vSignals.pop_back();

		switch(sstruct.nSignal) 
		{
		case 1:
			if (!stricmp(sstruct.strText,"PERCEPTION_RESET"))
				m_pEntity->SendScriptEvent(ScriptEvent_AllClear,0);
			break;
		default:
			break;
		}
	}
	
	return 0;
}

void CXObjectProxy::SendSignal(SOBJECTSTATE * pState)
{
}

bool CXObjectProxy::CustomUpdate(Vec3d &pos, Vec3d &angle)
{
	if (m_pPlayer)
	{
		if (m_pPlayer->m_bFirstPerson && m_pEntity->GetCamera())
		{
			pos = m_pEntity->GetCamera()->GetPos();
			if ( (1.f -fabs(m_pPlayer->m_walkParams.fLeanTarget)) < 0.0001f )
				m_pEntity->GetAI()->SetEyeHeight(0.1f);
			return true;
		}
		angle.z = m_pPlayer->m_LegAngle;
	}
	return false;
}



void CXObjectProxy::DebugDraw(struct IRenderer * pRenderer)
{
}
