// XActionMapManager.h: interface for the CXActionMapManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XACTIONMAPMANAGER_H__62E67F42_D4C6_4DE9_AE51_6562CCCDAD04__INCLUDED_)
#define AFX_XACTIONMAPMANAGER_H__62E67F42_D4C6_4DE9_AE51_6562CCCDAD04__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <string>

struct IActionMap;
class CXActionMap;

///////////////////////////////////////////////////////////////
struct XActionBind
{
//
	XActionBind()
	{
		memset(keys,0,sizeof(keys));
	}
	XActionBind( const XActionBind &a ) { *this = a; }
	XActionBind& operator=( const XActionBind &a )
	{
		memcpy(keys,a.keys,sizeof(keys));
		return *this;
	}
	bool IsActivationModeEquivalent(XActionActivationMode aam1,XActionActivationMode aam2)
	{
		if(aam1==aam2)
			return true;
		if((aam1==aamOnPress) && (aam2==aamOnHold) ||
			(aam2==aamOnPress) && (aam1==aamOnHold)
			)return true;
		if((aam1==aamOnPressAndRelease) || (aam2==aamOnPressAndRelease))
			return true;
		return false;

	}
	void RemoveBind(XBind &bind, XActionActivationMode aam)//int nKey,int nModifier,XActionActivationMode aam)
	{
		int n;
		for(n=0;n<MAX_BINDS_PER_ACTION;n++)
		{
			if((keys[n].Bind.nKey==bind.nKey) && (keys[n].Bind.nModifier==bind.nModifier) && IsActivationModeEquivalent(keys[n].aam,aam))
			{
//				char sTemp[1000];
//				sprintf(sTemp,"Bind removed %03d\n",keys[n].nKey);
//				::OutputDebugString(sTemp);
				keys[n].Bind.nKey=0;
				keys[n].Bind.nModifier=0;
				keys[n].aam=aamOnPress;
				keys[n].nLastPess=0;
			}
		}
	}
	void RemoveAllBindings()
	{
		int n;
		for(n=0;n<MAX_BINDS_PER_ACTION;n++)
		{
			keys[n].Bind.nKey=0;
			keys[n].Bind.nModifier=0;
			keys[n].aam=aamOnPress;
			keys[n].nLastPess=0;
		}
	}

	void PushBind(XBind &bind, XActionActivationMode aam)//int nKey,int nModifier,XActionActivationMode aam)
	{
/*		bool bAssigned=false;
		int n;
		for(n=0;n<MAX_BINDS_PER_ACTION;n++)
		{
			if(keys[n].Bind.nKey==0)
			{
				bAssigned=true;
//				keys[n].nKey=bind.nKey;
//				keys[n].nModifier=bind.nModifier;
				keys[n].Bind=bind;
				keys[n].aam=aam;
				keys[n].nLastPess=0;
//				char sTemp[1000];
//				sprintf(sTemp,"Bind added %03d\n",keys[n].nKey);
//				::OutputDebugString(sTemp);

				break;
			}
		}
		if(bAssigned==false)*/
		{
			int n=MAX_BINDS_PER_ACTION-1;
			while(n>=1)
			{
				keys[n]=keys[n-1];
				n--;
			}
//			keys[0].nKey=bind.nKey;
//			keys[0].nModifier=bind.nModifier;
			keys[0].Bind=bind;
			keys[0].aam=aam;
			keys[0].nLastPess=0;
//			char sTemp[1000];
//				sprintf(sTemp,"Bind added %03d\n",keys[n].nKey);
//				::OutputDebugString(sTemp);
		}
	}

	void SetBind(int iKeyPos, XBind &bind, XActionActivationMode aam)
	{
		if (iKeyPos >= 0 && iKeyPos < MAX_BINDS_PER_ACTION)
		{
			keys[iKeyPos].Bind=bind;
			keys[iKeyPos].aam=aam;
			keys[iKeyPos].nLastPess=0;
		}
	}
//
	struct tagKey
	{
		XBind Bind;
//		int nKey;
//		int nModifier;
		XActionActivationMode aam;
		unsigned int nLastPess;
	}keys[MAX_BINDS_PER_ACTION];
	
	
};

typedef std::map<XACTIONID,XActionActivationMode> ActionIDsMap;
typedef ActionIDsMap::iterator ActionIDsMapItor;

typedef std::map<string,XACTIONID> ActionNamesMap;
typedef ActionNamesMap::iterator ActionNamesMapItor;

typedef std::map<string,CXActionMap *> ActionMapMap;
typedef ActionMapMap::iterator ActionMapMapItor;

class CInput;

class CXActionMapManager : public IActionMapManager,public IInputEventListener
{
public:
	CXActionMapManager(CInput *pInput);
	virtual ~CXActionMapManager();
public:
//!IActionMapManager
//@{
	void SetSink(IActionMapSink *pSink);
	void CreateAction(XACTIONID nActionID,const char *sActionName,XActionActivationMode aam=aamOnPress);

	void SetInvertedMouse(bool bEnable);
	bool GetInvertedMouse();

	IActionMap *CreateActionMap(const char *s);
	IActionMap *GetActionMap(const char *s);

	void ResetAllBindings();

	void SetActionMap(const char *s);

	void GetActionMaps(IActionMapDumpSink *pCallback);

	void RemoveBind(XACTIONID nActionID, XBind &NewBind, XActionActivationMode aam);

	bool CheckActionMap(XACTIONID nActionID);
	bool CheckActionMap(const char *sActionName);
	void Reset();
	void Update(unsigned int nTimeMSec);
	void Release();
//@}
///////////////////////////////////////////
	bool CheckBind(XActionBind &bind,float &fVal,XActivationEvent &ae);
	bool CheckKey(struct XActionBind::tagKey &key,XActivationEvent &ae);
	bool CheckPressedKey(struct XActionBind::tagKey &key);
	bool CheckDoublePressedKey(struct XActionBind::tagKey &key);
	bool CheckReleasedKey(struct XActionBind::tagKey &key);
	bool CheckHoldKey(struct XActionBind::tagKey &key);

	void Enable();
	void Disable();
	bool IsEnabled();
	
	XActionActivationMode GetActionActivationMode(XACTIONID nActionID);
	void Notify(XACTIONID nActionID,float fValue,XActivationEvent ae)
	{
		if((m_pSink) && (m_bEnabled))
			m_pSink->OnAction(nActionID,fValue,ae);
	}

	//////////////////////////////////////////////////////////////////////////
	// Implements input event.
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnInputEvent( const SInputEvent &event );

	CInput *m_pInput;
private:
	
	ActionIDsMap m_mapActionIDs;
	ActionNamesMap m_mapActionNames;
	ActionMapMap m_mapActionMaps;
	CXActionMap *m_pCurrentActionMap;
	IActionMapSink *m_pSink;
	unsigned int m_nCurrentTime;
	bool m_bInvertedMouse;
	bool m_bEnabled;
};

#endif // !defined(AFX_XACTIONMAPMANAGER_H__62E67F42_D4C6_4DE9_AE51_6562CCCDAD04__INCLUDED_)
