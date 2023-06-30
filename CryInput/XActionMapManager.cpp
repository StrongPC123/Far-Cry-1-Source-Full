// XActionMapManager.cpp: implementation of the CXActionMapManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Input.h"
#include "XActionMap.h"
#include "XActionMapManager.h"
 
#include <ISystem.h>

#ifdef _DEBUG
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXActionMapManager::CXActionMapManager(CInput *pInput)
{
	m_bEnabled = 1;
	m_pInput=pInput;
	m_pCurrentActionMap=NULL;
	m_pSink=NULL;
	m_bInvertedMouse=false;
	pInput->AddEventListener( this );
}

CXActionMapManager::~CXActionMapManager()
{
	ActionMapMapItor itor;
	itor=m_mapActionMaps.begin();
	while(itor!=m_mapActionMaps.end())
	{
		CXActionMap *pMap=itor->second;
		delete pMap;
		++itor;
	}
}

void CXActionMapManager::SetSink(IActionMapSink *pSink)
{
	m_pSink=pSink;
}

void CXActionMapManager::CreateAction(XACTIONID nActionID,const char *sActionName,XActionActivationMode aam)
{
	ActionIDsMapItor itorID;
	ActionNamesMapItor itorName;
	itorID=m_mapActionIDs.find(nActionID);
	//if the action ID already exists return
	if(itorID!=m_mapActionIDs.end())
		return;
	//if the action Name already exists return
	itorName=m_mapActionNames.find(sActionName);
	if(itorName!=m_mapActionNames.end())
		return;
	m_mapActionIDs.insert(ActionIDsMapItor::value_type(nActionID,aam));
	m_mapActionNames.insert(ActionNamesMapItor::value_type(sActionName,nActionID));
}

XActionActivationMode CXActionMapManager::GetActionActivationMode(XACTIONID nActionID)
{
	ActionIDsMapItor itorID;
	itorID=m_mapActionIDs.find(nActionID);
	//default
	if(itorID==m_mapActionIDs.end())
		return aamOnPress;
	
	return itorID->second;
}

void CXActionMapManager::GetActionMaps(IActionMapDumpSink *pCallback)
{
	ActionMapMapItor itor=m_mapActionMaps.begin();
	while (itor!=m_mapActionMaps.end())
	{
		pCallback->OnElementFound(itor->first.c_str(), itor->second);
		++itor;
	}
}

void CXActionMapManager::RemoveBind(XACTIONID nActionID, XBind &NewBind, XActionActivationMode aam)
{
	ActionMapMapItor itor=m_mapActionMaps.begin();
	while (itor!=m_mapActionMaps.end())
	{
		itor->second->RemoveBind(nActionID, NewBind, aam);
		++itor;
	}
}

IActionMap *CXActionMapManager::CreateActionMap(const char *s)
{
	ActionMapMapItor itor;
	itor=m_mapActionMaps.find(s);
	//if the map already exists return the existing one
	if(itor!=m_mapActionMaps.end())
		return itor->second;

	CXActionMap *pAM=new CXActionMap(this);
	m_mapActionMaps.insert(ActionMapMapItor::value_type(s,pAM));

	return pAM;
}

IActionMap *CXActionMapManager::GetActionMap(const char *s)
{
	ActionMapMapItor itor;
	itor=m_mapActionMaps.find(s);
	
	if(itor!=m_mapActionMaps.end())
		return itor->second;

	return NULL;
}

void CXActionMapManager::Update(unsigned int nTimeMSec)
{
	m_nCurrentTime=nTimeMSec;
	if(m_pCurrentActionMap)
		m_pCurrentActionMap->Update();
}

void CXActionMapManager::ResetAllBindings()
{
	ActionMapMapItor itor;
	itor=m_mapActionMaps.begin();
	
	while(itor!=m_mapActionMaps.end())
	{
		CXActionMap *pAM=itor->second;
		if (pAM)
			pAM->ResetAllBindings();
		++itor;
	}
}

void CXActionMapManager::SetActionMap(const char *s)
{
	// reset keys when changing action map
	//m_pInput->GetIKeyboard()->ClearKeyState();
	

	ActionMapMapItor itor;
	itor=m_mapActionMaps.find(s);
	 
	if(itor!=m_mapActionMaps.end())
	{
		CXActionMap *pNewActionMap = itor->second;

		if(m_pCurrentActionMap)
		{
			// reset the key state for all actions
			std::vector<int> keys;
			std::vector<int>::iterator i;
			m_pCurrentActionMap->GetBindDifferences(pNewActionMap, keys);
			
			for (i=keys.begin(); i != keys.end(); ++i)
				m_pInput->GetIKeyboard()->ClearKey(*i);

			m_pCurrentActionMap->Reset();
		}

		m_pCurrentActionMap = pNewActionMap;
	}
	else
		m_pCurrentActionMap=NULL;
}

bool CXActionMapManager::CheckActionMap(XACTIONID nActionID)
{
	if(!m_pCurrentActionMap)
		return false;
	return m_pCurrentActionMap->CheckActionMap(nActionID);
}

bool CXActionMapManager::CheckActionMap(const char *sActionName)
{
	ActionNamesMapItor itorName;
	itorName=m_mapActionNames.find(sActionName);
	if(itorName==m_mapActionNames.end())
		return false;
	return CheckActionMap(itorName->second);
}

void CXActionMapManager::SetInvertedMouse(bool bEnable)
{
	m_bInvertedMouse=bEnable;
}

bool CXActionMapManager::GetInvertedMouse()
{
	return m_bInvertedMouse;
}

bool CXActionMapManager::CheckBind(XActionBind &bind,float &fVal,XActivationEvent &ae)
{
	int n=0;
	while(n<MAX_BINDS_PER_ACTION)
	{
		//if (bind.keys[n].Bind.nKey==XKEY_MWHEEL_DOWN)
		//	int a=1;

		if(CheckKey(bind.keys[n],ae))
		{
			switch(bind.keys[n].Bind.nKey)
			{
			case XKEY_MAXIS_X:
				fVal=m_pInput->MouseGetDeltaX();
				break;
			case XKEY_MAXIS_Y:
				fVal=m_bInvertedMouse?-m_pInput->MouseGetDeltaY():m_pInput->MouseGetDeltaY();
				break;
			case XKEY_MWHEEL_UP:
			case XKEY_MWHEEL_DOWN:
				fVal=m_pInput->MouseGetDeltaZ();
				break;
			default:
				fVal=0.0;
			}
			
			return true;
		}
			
		n++;
	}
	ae=etPressing;
	return false;
}

bool CXActionMapManager::CheckKey(struct XActionBind::tagKey &key,XActivationEvent &ae)
{
	switch(key.aam)
	{
	case aamOnPress:
		ae=etPressing;
		return CheckPressedKey(key);
		break;
	case aamOnPressAndRelease:
		if(CheckPressedKey(key)){
			ae=etPressing;
			return true;
		}
		ae=etReleasing;
		return CheckReleasedKey(key);
		break;
	case aamOnDoublePress:
		return CheckDoublePressedKey(key);
		ae=etDoublePressing;
		break;
	case aamOnRelease:
		ae=etReleasing;
		return CheckReleasedKey(key);
		break;
	case aamOnHold:
		ae=etHolding;
		return CheckHoldKey(key);
		break;
	default:
		CryError( "<CryInput> (CXActionMapManager::CheckKey) Unknown Key pressed" );
	}
	return false;
}

bool CXActionMapManager::CheckPressedKey(struct XActionBind::tagKey &key)
{
	if(IS_NULL_KEY(key.Bind.nKey))
		return false;
	
	if(IS_KEYBOARD_KEY(key.Bind.nKey))
	{
		return m_pInput->KeyPressed(key.Bind.nKey);
	}
	else if(IS_MOUSE_KEY(key.Bind.nKey))
	{
		return m_pInput->MousePressed(key.Bind.nKey);
	}
#ifndef _XBOX
	else if(IS_JOYPAD_KEY(key.Bind.nKey))
	{
		//<<FIXME>> implement joypad
		return false;
	}
#else // _XBOX
  else if(IS_GAMEPAD_KEY(key.Bind.nKey))
  {
    return m_pInput->GetIGamepad()->KeyPressed(key.Bind.nKey);
  }
#endif //_XBOX

	return false;
}

bool CXActionMapManager::CheckDoublePressedKey(struct XActionBind::tagKey &key)
{
	unsigned int nDeltaTime=m_nCurrentTime-key.nLastPess;
	if(IS_NULL_KEY(key.Bind.nKey))
		return false;
	
	if(IS_KEYBOARD_KEY(key.Bind.nKey))
	{
		if(m_pInput->KeyPressed(key.Bind.nKey))
		{
			if(nDeltaTime<300)
			{
				key.nLastPess=0;
				return true;
			}
			key.nLastPess=m_nCurrentTime;
		}
		return false;
	}
	else if(IS_MOUSE_KEY(key.Bind.nKey))
	{
		if(m_pInput->MousePressed(key.Bind.nKey))
		{
			if(nDeltaTime<300)
			{
				key.nLastPess=0;
				return true;
			}
			key.nLastPess=m_nCurrentTime;
		}
		return false;
	}
#ifndef _XBOX
	else if(IS_JOYPAD_KEY(key.Bind.nKey))
	{
		
		return false;
	}
#else // _XBOX
 	else if(IS_GAMEPAD_KEY(key.Bind.nKey))
	{
		
		return false;
	}
#endif //_XBOX
	return false;
}

bool CXActionMapManager::CheckReleasedKey(struct XActionBind::tagKey &key)
{
	if(IS_NULL_KEY(key.Bind.nKey))
		return false;
	
	if(IS_KEYBOARD_KEY(key.Bind.nKey))
	{
		return m_pInput->KeyReleased(key.Bind.nKey);
	}
	else if(IS_MOUSE_KEY(key.Bind.nKey))
	{
		return m_pInput->MouseReleased(key.Bind.nKey);
	}
#ifndef _XBOX
	else if(IS_JOYPAD_KEY(key.Bind.nKey))
	{
		//<<FIXME>> implement joypad
		return false;
	}
#else //_XBOBX
	else if(IS_GAMEPAD_KEY(key.Bind.nKey))
	{
    return m_pInput->GetIGamepad()->KeyReleased(key.Bind.nKey);

	}
#endif
	return false;
}

bool CXActionMapManager::CheckHoldKey(struct XActionBind::tagKey &key)
{
	if(IS_NULL_KEY(key.Bind.nKey))
		return false;
	
	if(IS_KEYBOARD_KEY(key.Bind.nKey))
	{
		return m_pInput->KeyDown(key.Bind.nKey);
	}
	else if(IS_MOUSE_KEY(key.Bind.nKey))
	{
		return m_pInput->MouseDown(key.Bind.nKey);
	}
#ifndef _XBOX
	else if(IS_JOYPAD_KEY(key.Bind.nKey))
	{
		return false;
	}
#else // _XBOX
  else if(IS_GAMEPAD_KEY(key.Bind.nKey))
  {
    return m_pInput->GetIGamepad()->KeyDown(key.Bind.nKey);
  }
#endif //_XBOX

	return false;
}

void CXActionMapManager::Enable()
{
	m_bEnabled = 1;
}

void CXActionMapManager::Disable()
{
	m_bEnabled = 0;
}

bool CXActionMapManager::IsEnabled()
{
	return m_bEnabled;
}

void CXActionMapManager::Reset()
{
	if(m_pCurrentActionMap)
		m_pCurrentActionMap->Reset();
}

void CXActionMapManager::Release()
{
	delete this;
}

//////////////////////////////////////////////////////////////////////////
bool CXActionMapManager::OnInputEvent( const SInputEvent &event )
{
	if(m_pCurrentActionMap)
		m_pCurrentActionMap->ProcessInputEvent( event );

	return false;
}