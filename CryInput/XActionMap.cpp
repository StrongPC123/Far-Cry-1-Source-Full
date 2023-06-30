// XActionMap.cpp: implementation of the CXActionMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Input.h"
#include "XActionMap.h"
#include "XActionMapManager.h"

#ifdef _DEBUG
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXActionMap::CXActionMap(CXActionMapManager *pManager)
{
	m_pManager=pManager;
	
}

CXActionMap::~CXActionMap()
{
}

void CXActionMap::GetBinding(XACTIONID nActionID, int nKeyPos, XBind &Bind)
{
	Bind.nKey=XKEY_NULL;
	Bind.nModifier=XKEY_NULL;
	if ((nKeyPos<0) || (nKeyPos>MAX_BINDS_PER_ACTION))
		return;
	BindsMapItor itor;
	itor=m_mapBinds.find(nActionID);
	if (itor==m_mapBinds.end())
		return;
	Bind=itor->second.keys[nKeyPos].Bind;
}

void CXActionMap::GetBinding(XACTIONID nActionID, int nKeyPos, int &nKey, int &nModifier)
{
	nKey=XKEY_NULL;
	nModifier=XKEY_NULL;
	if ((nKeyPos<0) || (nKeyPos>MAX_BINDS_PER_ACTION))
		return;
	BindsMapItor itor;
	itor=m_mapBinds.find(nActionID);
	if (itor==m_mapBinds.end())
		return;
	nKey=itor->second.keys[nKeyPos].Bind.nKey;
	nModifier=itor->second.keys[nKeyPos].Bind.nModifier;
}

void CXActionMap::GetBinding(XACTIONID nActionID, int nKeyPos, char *pszKey, char *pszModifier)
{
	int nKey;
	int nModifier;
	GetBinding(nActionID, nKeyPos, nKey, nModifier);
	strcpy(pszKey, m_pManager->m_pInput->GetKeyName(nKey));
	strcpy(pszModifier, m_pManager->m_pInput->GetKeyName(nModifier));
}

void CXActionMap::GetBindDifferences(IActionMap *pActionMap, std::vector<int>& keys)
{
	keys.clear();

	for (BindsMapItor itor = m_mapBinds.begin(); itor != m_mapBinds.end(); ++itor)
	{
		XACTIONID action = itor->first;
		XBind			bind;

		pActionMap->GetBinding(action, 0, bind);
		if (bind.nKey==XKEY_NULL)
		{
			for (int i = 0; i < MAX_BINDS_PER_ACTION; ++i)
			{
				if (itor->second.keys[i].Bind.nKey!=XKEY_NULL)
				{
					keys.push_back(itor->second.keys[i].Bind.nKey);
				}
			}
		}
	}
}

void CXActionMap::ResetAllBindings()
{
	BindsMapItor itor;
	itor=m_mapBinds.begin();
	while (itor!=m_mapBinds.end())
	{
		itor->second.RemoveAllBindings();
		++itor;
	}
}

void CXActionMap::ResetBinding(XACTIONID nActionID)
{
	BindsMapItor itor;
	itor=m_mapBinds.find(nActionID);
	if (itor!=m_mapBinds.end())
		itor->second.RemoveAllBindings();
}

void CXActionMap::RemoveBind(XACTIONID nActionID, XBind &NewBind, XActionActivationMode aam)//,int nKey,int nModifier)
{
	BindsMapItor itor;
	itor=m_mapBinds.find(nActionID);
	if (itor!=m_mapBinds.end())
		itor->second.RemoveBind(NewBind, aam);//nKey,nModifier,aam);
}

void CXActionMap::BindAction(XACTIONID nActionID,int nKey, int nModifier, int iKeyPos)//, bool bConfigurable, bool bReplicate)
{
	XBind Bind;
	Bind.nKey=nKey;
	Bind.nModifier=nModifier;
//	Bind.bConfigurable=bConfigurable;
//	Bind.bReplicate=bReplicate;
	BindAction(nActionID, Bind, iKeyPos);
}

void CXActionMap::BindAction(XACTIONID nActionID, XBind &NewBind, int iKeyPos)//,int nKey,int nModifier)
{
	BindsMapItor itor;
	XActionActivationMode aam=m_pManager->GetActionActivationMode(nActionID);
	RemoveBind(nActionID, NewBind, aam);
/*	itor=m_mapBinds.begin();
	XActionActivationMode aam;
	aam=m_pManager->GetActionActivationMode(nActionID);

	while(itor!=m_mapBinds.end())
	{
		itor->second.RemoveBind(NewBind, aam);//nKey,nModifier,aam);
		++itor;
	}
*/
//insert a new bind
	itor=m_mapBinds.find(nActionID);

	if(itor==m_mapBinds.end())
	{
		XActionBind bind;

		if (iKeyPos > -1)
		{
			bind.SetBind(iKeyPos, NewBind, aam);
		}
		else
		{
			bind.PushBind(NewBind, aam);//nKey,nModifier,aam);
		}
		m_mapBinds.insert(BindsMapItor::value_type(nActionID,bind));
	}
	else
	{
		if (iKeyPos > -1)
		{
			itor->second.SetBind(iKeyPos, NewBind, aam);
		}
		else
		{
			itor->second.PushBind(NewBind, aam);//nKey,nModifier,aam);
		}
	}
}

void CXActionMap::BindAction(XACTIONID nActionID, const char *sKey,const char *sModifier, int iKeyPos)
{
	if (!sKey)
	{
		return;
	}

	char sTemp[256];strcpy(sTemp,sKey);
	_strlwr(sTemp);
	XBind NewBind;
	NewBind.nKey=m_pManager->m_pInput->GetKeyID(sTemp);
	NewBind.nModifier=0;
	if(NewBind.nKey ){
		if(sModifier)
		{
			strcpy(sTemp,sModifier);_strlwr(sTemp);
			NewBind.nModifier=m_pManager->m_pInput->GetKeyID(sTemp);
		}
		
		BindAction(nActionID,NewBind, iKeyPos);//nKey,nModifier);
	}
}

bool CXActionMap::CheckActionMap(XACTIONID nActionID)
{
	CurrentActionMapItor itor;
	itor=m_mapCurrentActions.find(nActionID);
	if(itor!=m_mapCurrentActions.end())
	{
		return true;
	}
	return false;
}

void CXActionMap::Reset()
{
	m_mapCurrentActions.clear();
	
}

void CXActionMap::Update()
{
	m_mapCurrentActions.clear();
	
	BindsMapItor itor=m_mapBinds.begin();
	while(itor!=m_mapBinds.end())
	{
		XActionBind &bind=itor->second;
		//optional value send by mouse or pads
		float fValue=0.0;
		XActivationEvent ae;
		if(m_pManager->CheckBind(bind,fValue,ae))
		{
			m_mapCurrentActions.insert(CurrentActionMapItor::value_type(itor->first,XActionData(fValue,ae)));
		}
		++itor;
	}
	//if(!m_mapCurrentActions.empty())
	//{
		CurrentActionMapItor itorA;
		itorA=m_mapCurrentActions.begin();
		while(itorA!=m_mapCurrentActions.end())
		{
			m_pManager->Notify(itorA->first,itorA->second.fParam,itorA->second.aeEvent);
			//avoid to loop forever if the action map is cleaned
			//inside notify(eg SetActionMap())
			if(m_mapCurrentActions.empty())
				return;
			++itorA;
		}
	//}
}

//////////////////////////////////////////////////////////////////////////
void CXActionMap::ProcessInputEvent( const SInputEvent &event )
{
}