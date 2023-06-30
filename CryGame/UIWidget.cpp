//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - Base Widget
//
// History:
//  - [3/6/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#include "StdAfx.h"
#include "UIWidget.h"
#include "UISystem.h"
#include "UIScreen.h"
#include <algorithm>



//------------------------------------------------------------------------------------------------- 
CUIWidget::CUIWidget()
: m_iFlags(UIFLAG_DEFAULT),
	m_iStyle(0),
	m_pUISystem(0),
	m_pScreen(0),
	m_bMoving(0),
  m_cColor(color4f(1.0, 1.0f, 1.0f, 1.0f)),
	m_iMouseCursor(-1),
	m_iTabStop(0),
	m_cGreyedColor(0.3f, 0.3f, 0.3f, 0.5f),
	m_iGreyedBlend(0)
{
	m_vVisibleWidgetList.reserve(16);
	m_vVisibleWidgetList.resize(0);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetName(const string &szName)
{
	m_szName = szName;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
string &CUIWidget::GetName()
{
	return m_szName;
}

//------------------------------------------------------------------------------------------------- 
CUIWidgetList *CUIWidget::GetChildList()
{
	return &m_pChildList;
}

//------------------------------------------------------------------------------------------------- 
CUIWidget *CUIWidget::GetChild(int iIndex)
{
	return m_pChildList[iIndex];
}

//------------------------------------------------------------------------------------------------- 
CUIWidget *CUIWidget::GetChild(const string &szName)
{
	for (CUIWidgetItor pItor = m_pChildList.begin(); pItor != m_pChildList.end(); pItor++)
	{
		if ((*pItor)->m_szName == szName)
		{
			return *pItor;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetChildCount()
{
	return m_pChildList.size();
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::AddChild(CUIWidget *pWidget)
{
	for (CUIWidgetItor pItor = m_pChildList.begin(); pItor != m_pChildList.end(); pItor++)
	{
		if (*pItor == pWidget)
		{
			return 0;
		}
	}

	m_pChildList.push_back(pWidget);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::DelChild(CUIWidget *pWidget)
{
	for (CUIWidgetItor pItor = m_pChildList.begin(); pItor != m_pChildList.end(); pItor++)
	{
		if (*pItor == pWidget)
		{
			m_pChildList.erase(pItor);

			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::DelChild(int iIndex)
{
	return DelChild(m_pChildList[iIndex]);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::DelChild(const string &szName)
{
	for (CUIWidgetItor pItor = m_pChildList.begin(); pItor != m_pChildList.end(); pItor++)
	{
		if ((*pItor)->m_szName == szName)
		{
			m_pChildList.erase(pItor);

			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetFlags()
{
	return m_iFlags;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetFlags(int iFlags)
{
	m_iFlags = iFlags;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetStyle()
{
	return m_iStyle;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetStyle(int iStyle)
{
	m_iStyle = iStyle;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetRect(UIRect *pRect)
{
	if (m_pParent)
	{
		m_pUISystem->GetAbsoluteXY(&pRect->fLeft, &pRect->fTop, m_pRect.fLeft, m_pRect.fTop, m_pParent);
	}
	else
	{
		pRect->fLeft = m_pRect.fLeft;
		pRect->fTop = m_pRect.fTop;
	}
	
	pRect->fWidth = m_pRect.fWidth;
	pRect->fHeight = m_pRect.fHeight;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetRect(const UIRect &pRect, bool bRelative)
{
	if ((bRelative) || (!m_pParent))
	{
		memcpy(&m_pRect, &pRect, sizeof(UIRect));
	}
	else
	{
		m_pUISystem->GetRelativeXY(&m_pRect.fLeft, &m_pRect.fTop, pRect.fLeft, pRect.fTop, m_pParent);
		m_pRect.fWidth = pRect.fWidth;
		m_pRect.fHeight = pRect.fHeight;
	}
	
	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetBorder(UIBorder *pBorder)
{
	memcpy(pBorder, &m_pBorder, sizeof(UIBorder));

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetBorder(const UIBorder &pBorder)
{
	memcpy(&m_pBorder, &pBorder, sizeof(UIBorder));

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetZ()
{
	return m_iZ;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetZ(int iZ)
{
	if (m_iZ != iZ)
	{
		m_pUISystem->OnZChanged(this);
	}

	m_iZ = iZ;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetTabStop()
{
	return m_iTabStop;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetTabStop(int iTabStop)
{
	m_iTabStop = iTabStop;

	m_pUISystem->OnTabStopChanged(this);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetToolTip(const float fX, const float fY, wstring &szwToolTip)
{
	szwToolTip = m_szwToolTip;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::Release()
{
	CUIWidgetItor pItor = m_pChildList.begin();

	while (GetChildCount())
	{
		GetChild(GetChildCount()-1)->Release();
	}

	if (m_pParent)
	{
		m_pParent->DelChild(this);
	}

	m_pUISystem->DestroyWidget(this);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
// Normal Events
//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnInit()
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnInit", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnInit", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnRelease()
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnRelease", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnRelease", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
// Keyboard Events
//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnKeyDown(int iKeyCode)
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnKeyDown", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnKeyDown", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->PushFuncParam(iKeyCode);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnKeyUp(int iKeyCode)
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnKeyUp", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnKeyUp", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->PushFuncParam(iKeyCode);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnKeyPressed(int iKeyCode)
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnKeyPressed", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnKeyPressed", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->PushFuncParam(iKeyCode);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
// Mouse Events
//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnMouseEnter()
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnMouseEnter", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnMouseEnter", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnMouseLeave()
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnMouseLeave", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnMouseLeave", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnMouseDown(int iKeyCode, float fX, float fY)
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnMouseDown", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnMouseDown", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->PushFuncParam(iKeyCode);
	pScriptSystem->PushFuncParam(fX);
	pScriptSystem->PushFuncParam(fY);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnMouseUp(int iKeyCode, float fX, float fY)
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnMouseUp", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnMouseUp", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->PushFuncParam(iKeyCode);
	pScriptSystem->PushFuncParam(fX);
	pScriptSystem->PushFuncParam(fY);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnMouseClick(int iKeyCode, float fX, float fY)
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnMouseClick", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnMouseClick", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->PushFuncParam(iKeyCode);
	pScriptSystem->PushFuncParam(fX);
	pScriptSystem->PushFuncParam(fY);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnMouseDblClick(int iKeyCode, float fX, float fY)
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnMouseDblClick", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnMouseDblClick", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->PushFuncParam(iKeyCode);
	pScriptSystem->PushFuncParam(fX);
	pScriptSystem->PushFuncParam(fY);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnMouseOver(float fNewX, float fNewY, float fOldX, float fOldY)
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnMouseOver", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnMouseOver", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->PushFuncParam(fNewX);
	pScriptSystem->PushFuncParam(fNewY);
	pScriptSystem->PushFuncParam(fOldX);
	pScriptSystem->PushFuncParam(fOldY);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnMouseMove(float fNewX, float fNewY, float fOldX, float fOldY)
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnMouseMove", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnMouseMove", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->PushFuncParam(fNewX);
	pScriptSystem->PushFuncParam(fNewY);
	pScriptSystem->PushFuncParam(fOldX);
	pScriptSystem->PushFuncParam(fOldY);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnGotFocus()
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnGotFocus", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnGotFocus", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnLostFocus()
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnLostFocus", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnLostFocus", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnSized()
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnSized", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnSized", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnMoved()
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnMoved", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnMoved", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnChanged()
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnChanged", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnChanged", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::OnCommand()
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);
	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject)
	{
		return 1;
	}

	if (!pScriptObject->GetValue("OnCommand", pScriptFunction))
	{
		if (pScriptObject->GetValueType("skin") != svtObject)
		{
			return 1;
		}

		_SmartScriptObject	pSkinScriptObject(pScriptSystem, true);

		if (!pScriptObject->GetValue("skin", pSkinScriptObject))
		{
			return 1;
		}

		if (!pSkinScriptObject->GetValue("OnCommand", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
int CUIWidget::SortChildrenByZ()
{
	std::sort(m_pChildList.begin(), m_pChildList.end(), SortCallback);

	return 1;
}

//-------------------------------------------------------------------------------------------------
int CUIWidget::DrawChildren()
{
	// container to hold the visible widget list
	m_vVisibleWidgetList.resize(0);

	// draw first pass and gater the visible widgets
	for (CUIWidgetItor pItor = m_pChildList.begin(); pItor != m_pChildList.end(); pItor++)
	{
		CUIWidget *pWidget = *pItor;

		if (pWidget->GetFlags() & UIFLAG_VISIBLE)
		{
			m_pUISystem->SendMessage(pWidget, UIM_DRAW, 0, 0);

			m_vVisibleWidgetList.push_back(pWidget);
		}
	}

	// draw next passes
	for (int i = 1; i < UI_DEFAULT_PASSES; i++)
	{
		for (CUIWidgetItor pItor = m_vVisibleWidgetList.begin(); pItor != m_vVisibleWidgetList.end(); pItor++)
		{
			CUIWidget *pWidget = *pItor;

			m_pUISystem->SendMessage(pWidget, UIM_DRAW, i, 0);
		}
	}

	return 1;
}

//-------------------------------------------------------------------------------------------------
// Common Script Functions
//------------------------------------------------------------------------------------------------- 

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetName(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetName, 0);

	return pH->EndFunction(GetName().c_str());
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetScreen(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetScreen, 0);

	if (m_pScreen)
	{
		return pH->EndFunction(m_pScreen->GetScriptObject());
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetChildCount(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetChildCount, 0);

	return pH->EndFunction(GetChildCount());
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetChild(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetChild, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetChild, 1, svtString, svtNumber);

	CUIWidget *pWidget = 0;

	if (pH->GetParamType(1) == svtString)
	{
		char *szName;

		pH->GetParam(1, szName);

		pWidget = GetChild(szName);
	}
	else
	{
		int iIndex;

		pH->GetParam(1, iIndex);

		if (iIndex < GetChildCount())
		{
			pWidget = GetChild(iIndex);
		}
	}

	if (pWidget)
	{
		return pH->EndFunction(m_pUISystem->GetWidgetScriptObject(pWidget));
	}
	else
	{
		return pH->EndFunctionNull();
	}
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::Release(IFunctionHandler *pH)
{
	Release();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetRect(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pUISystem->GetIScriptSystem(), GetName().c_str(), SetRect, 1, 4);

	if ((pH->GetParamCount() == 1) && (pH->GetParamType(1) == svtString))
	{
		UIRect pRect;
		char *szRect;

		pH->GetParam(1, szRect);

		if (m_pUISystem->RetrieveRect(&pRect, szRect))
		{
			m_pRect = pRect;
		}
	}
	else if ((pH->GetParamCount() == 4) && (pH->GetParamType(1) == svtNumber) && (pH->GetParamType(4) == svtNumber))
	{
		UIRect pRect;

		pH->GetParam(1, pRect.fLeft);
		pH->GetParam(2, pRect.fTop);
		pH->GetParam(3, pRect.fWidth);
		pH->GetParam(4, pRect.fHeight);

		m_pRect = pRect;
	}
	else
	{
		m_pUISystem->GetIScriptSystem()->RaiseError("%s:SetRect() Wrong type in parameter 1! Expected 'String' or 'Number', but found '%s'!", GetName().c_str(), GET_SCRIPT_TYPE_STRING(pH->GetParamType(1)));

		return 0;
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetRect(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetRect, 0);

	char szRect[64];

	m_pUISystem->ConvertToString(szRect, m_pRect);

	return pH->EndFunction(szRect);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetColor(IFunctionHandler *pH)
{
	RETURN_COLOR_FROM_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), SetColor, m_cColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetColor(IFunctionHandler *pH)
{
	RETURN_COLOR_TO_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetColor, m_cColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetBorderColor(IFunctionHandler *pH)
{
	RETURN_COLOR_FROM_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), SetFontColor, m_pBorder.cColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetBorderColor(IFunctionHandler *pH)
{
	RETURN_COLOR_TO_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetBorderColor, m_pBorder.cColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetBorderSize(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), SetBorderSize, m_pBorder.fSize);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetBorderSize(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetBorderSize, m_pBorder.fSize);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetBorderStyle(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), SetBorderStyle, m_pBorder.iStyle);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetBorderStyle(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetBorderStyle, m_pBorder.iStyle);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetFontName(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), SetFontName, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pUISystem->GetIScriptSystem(), GetName().c_str(), SetFontName, 1, svtString);

	char *szFontName;

	pH->GetParam(1, szFontName);

	m_pFont.szFaceName = szFontName;

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetFontName(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetFontName, 0);

	return pH->EndFunction(m_pFont.szFaceName.c_str());
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetFontEffect(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), SetFontEffect, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pUISystem->GetIScriptSystem(), GetName().c_str(), SetFontEffect, 1, svtString);

	char *szFontEffect;

	pH->GetParam(1, szFontEffect);

	m_pFont.szEffectName = szFontEffect;

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetFontEffect(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetFontEffect, 0);

	return pH->EndFunction(m_pFont.szEffectName.c_str());
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetFontColor(IFunctionHandler *pH)
{
	RETURN_COLOR_FROM_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), SetFontColor, m_pFont.cColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetFontColor(IFunctionHandler *pH)
{
	RETURN_COLOR_TO_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetFontColor, m_pFont.cColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetFontSize(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), SetFontSize, m_pFont.fSize);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetFontSize(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetFontSize, m_pFont.fSize);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetMouseCursor(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), SetMouseCursor, m_iMouseCursor);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetMouseCursor(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetMouseCursor, m_iMouseCursor);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetToolTip(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), SetToolTip, 0);

	m_pUISystem->ConvertToWString(m_szwToolTip, pH, 1);

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetGreyedColor(IFunctionHandler *pH)
{
	RETURN_COLOR_FROM_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), SetGreyedColor, m_cGreyedColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetGreyedColor(IFunctionHandler *pH)
{
	RETURN_COLOR_TO_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetGreyedColor, m_cGreyedColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::SetGreyedBlend(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), SetGreyedBlend, m_iGreyedBlend);
}

//------------------------------------------------------------------------------------------------- 
int CUIWidget::GetGreyedBlend(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pUISystem->GetIScriptSystem(), GetName().c_str(), GetGreyedBlend, m_iGreyedBlend);
}