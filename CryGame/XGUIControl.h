#pragma once

// Events
#define XCTRLEVENT_MOUSEOVER							0x00000001
#define XCTRLEVENT_CLICK									0x00000002

// Effects
#define XCTRLEFFECT_BLINK									0x00010001

struct SMouseState;

struct SCtrlEffect
{
	SCtrlEffect()
	{
		pParams=NULL;
		bCalled=false;
	}
	int nEvent;
	int nEffect;
	HSCRIPTFUNCTION hCallback;
	bool bSingleCall;
	IScriptObject *pParams;
	bool bCalled;
};

typedef std::list<SCtrlEffect> TlstCtrlEffect;
typedef TlstCtrlEffect::iterator TlstCtrlEffectIt;																				

/*! This class is the base-class of all controls and has only very basic functionality.
*/
class CXGUIControl
{
private:
	// TODO: get rid of this, it's also present in the derived controls.
	IScriptSystem *m_pGUIScriptSystem;
	// this is the callback function or an invalid function reference if no callback was defined
	HSCRIPTFUNCTION m_hFunc;
protected:
	bool m_bUpdated;
	bool m_bFocus;
	bool m_bEnabled;
	TlstCtrlEffect m_lstEffects;
protected:
	void AddEffect(SCtrlEffect Effect) { m_lstEffects.push_back(Effect); }
	SCtrlEffect* FindEffect(int nEvent, int nEffect);
	bool ExecuteEffect(IScriptObject *pThisScriptObject, SCtrlEffect *pEffect);
	bool ResetEffect(IScriptObject *pThisScriptObject, SCtrlEffect *pEffect);
	bool ExecuteEffectEvent(IScriptObject *pThisScriptObject, int nEvent);
	bool ResetEffectEvent(IScriptObject *pThisScriptObject, int nEvent);
public:
	CXGUIControl();
	virtual ~CXGUIControl();
	void SetScriptSystem(IScriptSystem *pScriptSystem) { m_pGUIScriptSystem=pScriptSystem; }
	virtual void Draw()=0;
	virtual void Update(SMouseState &MouseState)=0;
	virtual void SetFocus(bool bFocus) { m_bFocus=bFocus; }
	bool HasFocus() { return m_bFocus; }
	virtual void SetEnable(bool bEnable) { m_bEnabled=bEnable; }
	bool GetEnable() { return m_bEnabled; }
	virtual void GetPosition(float &x, float &y, float &w, float &h)=0;
	void SetUpdatedFlag(bool bUpdated) { m_bUpdated=bUpdated; }
	bool GetUpdatedFlag() { return m_bUpdated; }
	// sets the callback, the function needs to be a strong reference
	void SetScriptCallback(HSCRIPTFUNCTION hFunc);
	int  SetCallback(IFunctionHandler *pH);
	bool Callback(IScriptObject *pThisScriptObject, int nEvent, IScriptObject *pUserData);
};
