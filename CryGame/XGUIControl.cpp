#include "stdafx.h"
#include "xguicontrol.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CXGUIControl::CXGUIControl()
{
	m_pGUIScriptSystem=NULL;
	m_hFunc=INVALID_SCRIPT_FUNCTION;
	m_bUpdated=false;
	m_bFocus=false;
	m_bEnabled=true;
}
CXGUIControl::~CXGUIControl()
{
	TlstCtrlEffectIt It=m_lstEffects.begin();
	while (It!=m_lstEffects.end())
	{
		SCtrlEffect &Effect=(*It);
		if (Effect.pParams)
			Effect.pParams->Release();
		++It;
	}
	m_lstEffects.clear();
	SetScriptCallback(INVALID_SCRIPT_FUNCTION);
}


/*! Search for a control-effect.
		@param nEvent EventID to find the effect for
		@param nEffect EffectID to find the effect for
		@return struct of SCtrlEffect describing the effect or NULL if no effect was found
*/
SCtrlEffect* CXGUIControl::FindEffect(int nEvent, int nEffect)
{
	TlstCtrlEffectIt It=m_lstEffects.begin();
	while (It!=m_lstEffects.end())
	{
		SCtrlEffect &Effect=(*It);
		if ((Effect.nEvent==nEvent) && (Effect.nEffect==nEffect))
			return &Effect;
		++It;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Call the callback-function in the script for notification.
		@param pThisScriptObject ScriptObject of calling control
		@param nEvent EventID for this callback
		@param pUserData Optional user-data and extra-parameters send to the callback
*/
bool CXGUIControl::Callback(IScriptObject *pThisScriptObject, int nEvent, IScriptObject *pUserData)
{
	if (!m_pGUIScriptSystem)
		return false;
	if (m_hFunc == INVALID_SCRIPT_FUNCTION)
		return false;
	if (!pThisScriptObject)
		return false;
	m_pGUIScriptSystem->BeginCall(m_hFunc);
	m_pGUIScriptSystem->PushFuncParam(pThisScriptObject);
	m_pGUIScriptSystem->PushFuncParam(nEvent);
	if (pUserData)
		m_pGUIScriptSystem->PushFuncParam(pUserData);
	m_pGUIScriptSystem->EndCall();
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Call the callback-function in the script for effect-processing.
		@param pThisScriptObject ScriptObject of calling control
		@param pEffect Pointer to effect
*/
bool CXGUIControl::ExecuteEffect(IScriptObject *pThisScriptObject, SCtrlEffect *pEffect)
{
	if (!pThisScriptObject)
		return false;
	if (pEffect->bCalled && pEffect->bSingleCall)
		return true;
	if (pEffect->hCallback)
	{
		m_pGUIScriptSystem->BeginCall(pEffect->hCallback);
		m_pGUIScriptSystem->PushFuncParam(pThisScriptObject);
		m_pGUIScriptSystem->PushFuncParam(pEffect->nEffect);
		m_pGUIScriptSystem->PushFuncParam(pEffect->nEvent);
		m_pGUIScriptSystem->PushFuncParam(pEffect->pParams);
		m_pGUIScriptSystem->EndCall();
	}
	pEffect->bCalled=true;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Reset the CALLED-flag for single-call-effects so the callback gets called again next-time.
		@param pThisScriptObject ScriptObject of calling control
		@param pEffect Pointer to effect
*/
bool CXGUIControl::ResetEffect(IScriptObject *pThisScriptObject, SCtrlEffect *pEffect)
{
	pEffect->bCalled=false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Executes all effects which are connected to a specific event.
		@param pThisScriptObject ScriptObject of calling control
		@param nEvent EventID of which all effects should be executed
*/
bool CXGUIControl::ExecuteEffectEvent(IScriptObject *pThisScriptObject, int nEvent)
{
	TlstCtrlEffectIt It=m_lstEffects.begin();
	while (It!=m_lstEffects.end())
	{
		SCtrlEffect &Effect=(*It);
		if (Effect.nEvent==nEvent)
			ExecuteEffect(pThisScriptObject, &Effect);
		++It;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Reset the CALLED-flag for single-call-effects for all effects of a specific event so the callback gets called again next-time.
		@param pThisScriptObject ScriptObject of calling control
		@param nEvent EventID of which all effects should be resetted
*/
bool CXGUIControl::ResetEffectEvent(IScriptObject *pThisScriptObject, int nEvent)
{
	TlstCtrlEffectIt It=m_lstEffects.begin();
	while (It!=m_lstEffects.end())
	{
		SCtrlEffect &Effect=(*It);
		if (Effect.nEvent==nEvent)
			ResetEffect(pThisScriptObject, &Effect);
		++It;
	}
	return true;
}

void CXGUIControl::SetScriptCallback(HSCRIPTFUNCTION hFunc)
{
	if (m_hFunc != INVALID_SCRIPT_FUNCTION)
		m_pGUIScriptSystem->ReleaseFunc(m_hFunc);
	m_hFunc = hFunc;
}


//////////////////////////////////////////////////////////////////////////
// Standard method implementations

/*! Script-function to set the event-callback-function.
@param hFunc FunctionHandle to new callback-function
*/
int CXGUIControl::SetCallback(IFunctionHandler *pH)
{
	CHECK_PARAMETERS_SS(m_pGUIScriptSystem,1);
	HSCRIPTFUNCTION hFunc;
	if (!pH->GetParam(1, hFunc, 1))
	{
		if (pH->GetParamType(1) == svtNull)
			hFunc = INVALID_SCRIPT_FUNCTION;
		else
		{
			m_pGUIScriptSystem->RaiseError("CXButton::SetCallback. Wrong argument type %s #d (function expected)", ScriptVarTypeAsCStr(pH->GetParamType(1)), pH->GetParamType(1));
			return 0;
		}
	}
	CXGUIControl::SetScriptCallback(hFunc);
	return pH->EndFunction();
}
