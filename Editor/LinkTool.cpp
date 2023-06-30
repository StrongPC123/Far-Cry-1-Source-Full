#include "StdAfx.h"
#include "linktool.h"
#include "Viewport.h"
#include "objects/entity.h"

IMPLEMENT_DYNAMIC(CLinkTool,CEditTool)

//////////////////////////////////////////////////////////////////////////
CLinkTool::CLinkTool()
{
	m_pChild=NULL;
	SetStatusText("Click on object and drag a link to a new parent");

	m_hLinkCursor = AfxGetApp()->LoadCursor( IDC_POINTER_LINK );
	m_hLinkNowCursor = AfxGetApp()->LoadCursor( IDC_POINTER_LINKNOW );
	m_hCurrCursor = m_hLinkCursor;
}

//////////////////////////////////////////////////////////////////////////
CLinkTool::~CLinkTool()
{
}

//////////////////////////////////////////////////////////////////////////
bool CLinkTool::MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	if (event == eMouseLDown)
	{
		ObjectHitInfo hitInfo(view,point);
		view->HitTest( point,hitInfo );
		CBaseObject *obj = hitInfo.object;
		if (obj)
		{
			if (IsRelevant(obj))
			{
				m_StartDrag = obj->GetWorldPos();
				m_pChild=obj;
			}
		}
	}
	if (event == eMouseLUp)
	{
		ObjectHitInfo hitInfo(view,point);
		view->HitTest( point,hitInfo );
		CBaseObject *obj = hitInfo.object;
		if (obj)
		{
			if (IsRelevant(obj))
			{
				if (m_pChild)
				{
					if (ChildIsValid(obj, m_pChild))
					{
						CUndo undo( "Link Object(s)" );
						obj->AttachChild(m_pChild);
						CString str;
						str.Format("%s attached to %s", m_pChild->GetName(), obj->GetName());
						SetStatusText(str);
					}else
					{
						SetStatusText("Error: Cyclic linking or already linked.");
					}
				}
			}
		}
		m_pChild=NULL;
	}
	if (event == eMouseRDown)
	{
		GetIEditor()->SetEditTool(0);
	}
	m_hCurrCursor = m_hLinkCursor;
	if (event == eMouseMove)
	{
		m_EndDrag = view->ViewToWorld(point);
		
		ObjectHitInfo hitInfo(view,point);
		view->HitTest( point,hitInfo );
		CBaseObject *obj = hitInfo.object;
		if (obj)
		{
			if (IsRelevant(obj))
			{
				// Set Cursors.
				view->SetObjectCursor(obj);
				if (m_pChild)
				{
					if (ChildIsValid(obj, m_pChild))
					{
						m_hCurrCursor = m_hLinkNowCursor;
					}
				}
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CLinkTool::OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{ 
	if (nChar == VK_ESCAPE)
	{
		// Cancel selection.
		GetIEditor()->SetEditTool(0);
	}
	return false; 
}

//////////////////////////////////////////////////////////////////////////
void CLinkTool::Display( DisplayContext &dc )
{
	if (m_pChild)
		dc.DrawLine(m_StartDrag, m_EndDrag, CFColor(1, 0, 0), CFColor(1, 1, 1));
}

//////////////////////////////////////////////////////////////////////////
bool CLinkTool::ChildIsValid(CBaseObject *pParent, CBaseObject *pChild, int nDir)
{
	if (!pParent)
		return false;
	if (!pChild)
		return false;
	if (pParent==pChild)
		return false;
	CBaseObject *pObj;
	if (nDir & 1)
	{
		if (pObj=pChild->GetParent())
		{
			if (!ChildIsValid(pParent, pObj, 1))
			{
				return false;
			}
		}
	}
	if (nDir & 2)
	{
		for (int i=0;i<pChild->GetChildCount();i++)
		{
			if (pObj=pChild->GetChild(i))
			{
				if (!ChildIsValid(pParent, pObj, 2))
				{
					return false;
				}
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CLinkTool::OnSetCursor( CViewport *vp )
{
	SetCursor( m_hCurrCursor );
	return true;
}