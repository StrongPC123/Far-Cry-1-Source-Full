// XActionMap.h: interface for the CXActionMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XACTIONMAP_H__8B377CD3_569A_4B34_A450_351530562078__INCLUDED_)
#define AFX_XACTIONMAP_H__8B377CD3_569A_4B34_A450_351530562078__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <map>
#include <set>
#include <IInput.h>
#include "XActionMapManager.h"

///////////////////////////////////////////////////////////////
typedef std::map<XACTIONID,XActionBind> BindsMap;
typedef BindsMap::iterator BindsMapItor;

struct XActionData{
	XActionData(float p,XActivationEvent e){
		fParam=p;
		aeEvent=e;
	}
	XActionData(const XActionData &o)
	{
		fParam=o.fParam;
		aeEvent=o.aeEvent;
	}
	float fParam;
	XActivationEvent aeEvent;
};
typedef std::map<XACTIONID,XActionData> CurrentActionMap;
typedef CurrentActionMap::iterator CurrentActionMapItor;
///////////////////////////////////////////////////////////////


class CXActionMap :
public IActionMap
{
public:
	CXActionMap(CXActionMapManager *pManager);
	virtual ~CXActionMap();
public:
//IActionMap
	void ResetAllBindings();
	void ResetBinding(XACTIONID nActionID);
	void RemoveBind(XACTIONID nActionID, XBind &NewBind, XActionActivationMode aam);
	void BindAction(XACTIONID nActionID, XBind &NewBind, int iKeyPos = -1);//XACTIONID nActionID,int nKey,int nModifier=XKEY_NULL);
	void BindAction(XACTIONID nActionID,int nKey, int nModifier=XKEY_NULL, int iKeyPos = -1);//, bool bConfigurable=true, bool bReplicate=false);
	void BindAction(XACTIONID nActionID,const char *sKey,const char *sModifier=NULL, int iKeyPos = -1);
	void GetBinding(XACTIONID nActionID, int nKeyPos, XBind &Bind);
	void GetBinding(XACTIONID nActionID, int nKeyPos, int &nKey, int &nModifier);	// nKey==XKEY_NULL if no more keys are bound to this action or action doesnt exist
	void GetBinding(XACTIONID nActionID, int nKeyPos, char *pszKey, char *pszModifier); // pszKey=="" if no more keys are bound to this action or action doesnt exist
	void GetBindDifferences(IActionMap *pActionMap, std::vector<int>& keys);
////////////
	bool CheckActionMap(XACTIONID nActionID);
	void Update();
	void Reset();

	void ProcessInputEvent( const SInputEvent &event );
private:
	CXActionMapManager *m_pManager;
	BindsMap m_mapBinds;
	CurrentActionMap m_mapCurrentActions;
};

#endif // !defined(AFX_XACTIONMAP_H__8B377CD3_569A_4B34_A450_351530562078__INCLUDED_)
