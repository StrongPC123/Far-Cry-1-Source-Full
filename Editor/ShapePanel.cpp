////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   shapepanel.cpp
//  Version:     v1.00
//  Created:     28/2/2002 by Timur.
//  Compilers:   Visual C++.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ShapePanel.h"

#include "Viewport.h"
#include "Objects\\ShapeObject.h"
#include "EditTool.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
class CEditShapeObjectTool : public CEditTool
{
public:
	DECLARE_DYNCREATE(CEditShapeObjectTool)

	CEditShapeObjectTool();
	~CEditShapeObjectTool();

	// Ovverides from CEditTool
	bool MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );

	virtual void SetUserData( void *userData );
	
	// Delete itself.
	void Release() { delete this; };

	virtual void BeginEditParams( IEditor *ie,int flags ) {};
	virtual void EndEditParams() {};

	virtual void Display( DisplayContext &dc ) {};
	virtual bool OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags );
	virtual bool OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags ) { return false; };

private:
	CShapeObject *m_shape;
	int m_currPoint;
	bool m_modifying;
	CPoint m_mouseDownPos;
	Vec3 m_pointPos;
};

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CEditShapeObjectTool,CEditTool)

//////////////////////////////////////////////////////////////////////////
CEditShapeObjectTool::CEditShapeObjectTool()
{
	m_shape = 0;
	m_currPoint = -1;
	m_modifying = false;
}

//////////////////////////////////////////////////////////////////////////
void CEditShapeObjectTool::SetUserData( void *userData )
{
	m_shape = (CShapeObject*)userData;
	assert( m_shape != 0 );

	// Modify shape undo.
	if (!CUndo::IsRecording())
	{
		CUndo ("Modify Shape");
		m_shape->StoreUndo( "Shape Modify" );
	}

	m_shape->SelectPoint(-1);
}

//////////////////////////////////////////////////////////////////////////
CEditShapeObjectTool::~CEditShapeObjectTool()
{
	if (m_shape)
	{
		m_shape->SelectPoint(-1);
	}
	if (GetIEditor()->IsUndoRecording())
		GetIEditor()->CancelUndo();
}

bool CEditShapeObjectTool::OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{
	if (nChar == VK_ESCAPE)
	{
		GetIEditor()->SetEditTool(0);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CEditShapeObjectTool::MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	if (!m_shape)
		return false;

	if (event == eMouseLDown)
	{
		m_mouseDownPos = point;
	}

	if (event == eMouseLDown || event == eMouseMove || event == eMouseLDblClick || event == eMouseLUp)
	{
		const Matrix44 &shapeTM = m_shape->GetWorldTM();

		/*
		float fShapeCloseDistance = SHAPE_CLOSE_DISTANCE;
		Vec3 pos = view->ViewToWorld( point );
		if (pos.x == 0 && pos.y == 0 && pos.z == 0)
		{
			// Find closest point on the shape.
			fShapeCloseDistance = SHAPE_CLOSE_DISTANCE * view->GetScreenScaleFactor(pos) * 0.01f;
		}
		else
			fShapeCloseDistance = SHAPE_CLOSE_DISTANCE * view->GetScreenScaleFactor(pos) * 0.01f;
		*/


		float dist;

		Vec3 raySrc,rayDir;
		view->ViewToWorldRay( point,raySrc,rayDir );

		// Find closest point on the shape.
		int p1,p2;
		Vec3 intPnt;
		m_shape->GetNearestEdge( raySrc,rayDir,p1,p2,dist,intPnt );
		
		float fShapeCloseDistance = SHAPE_CLOSE_DISTANCE * view->GetScreenScaleFactor(intPnt) * 0.01f;


		if ((flags & MK_CONTROL) && !m_modifying)
		{
			// If control we are editing edges..
			if (p1 >= 0 && p2 >= 0 && dist < fShapeCloseDistance+view->GetSelectionTolerance())
			{
				// Cursor near one of edited shape edges.
				view->SetObjectCursor(0);
				if (event == eMouseLDown)
				{
					view->CaptureMouse();
					m_modifying = true;
					GetIEditor()->BeginUndo();
					if (GetIEditor()->IsUndoRecording())
						m_shape->StoreUndo( "Make Point" );

					Vec3 newWorldPnt = view->MapViewToCP(point);
					Matrix44 invShapeTM = shapeTM;
					invShapeTM.Invert44();
					Vec3 newLocalPnt = invShapeTM.TransformPointOLD(newWorldPnt);

					// If last edge, insert at end.
					if (p2 == 0)
						p2 = -1;
					// Create new point between nearest edge.
					// Put intPnt into local space of shape.
					//Matrix invShapeTM = shapeTM;
					//invShapeTM.Invert44();
					//intPnt = invShapeTM.TransformPointOLD(intPnt);
					int index = m_shape->InsertPoint( p2,newLocalPnt );
					m_shape->SelectPoint( index );

					// Set construction plance for view.
					m_pointPos = shapeTM.TransformPointOLD( m_shape->GetPoint(index) );
					Matrix44 tm;
					tm.SetIdentity();
					tm.SetTranslationOLD( m_pointPos );
					view->SetConstrPlane( point,tm );
				}
			}
			return true;
		}

		int index = m_shape->GetNearestPoint( raySrc,rayDir,dist );
		if (index >= 0 && dist < fShapeCloseDistance+view->GetSelectionTolerance())
		{
			// Cursor near one of edited shape points.
			view->SetObjectCursor(0);
			if (event == eMouseLDown)
			{
				if (!m_modifying)
				{
					m_shape->SelectPoint( index );
					m_modifying = true;
					view->CaptureMouse();
					GetIEditor()->BeginUndo();
					
					// Set construction plance for view.
					m_pointPos = shapeTM.TransformPointOLD( m_shape->GetPoint(index) );
					Matrix44 tm;
					tm.SetIdentity();
					tm.SetTranslationOLD( m_pointPos );
					view->SetConstrPlane( point,tm );
				}
			}

			//GetNearestEdge

			if (event == eMouseLDblClick)
			{
				m_modifying = false;
				m_shape->RemovePoint( index );
				m_shape->SelectPoint( -1 );
			}
		}

		if (m_modifying && event == eMouseLUp)
		{
			// Accept changes.
			m_modifying = false;
			m_shape->SelectPoint( -1 );
			view->ReleaseMouse();
			m_shape->CalcBBox();

			if (GetIEditor()->IsUndoRecording())
				GetIEditor()->AcceptUndo( "Shape Modify" );
		}

		if (m_modifying && event == eMouseMove)
		{
			// Move selected point point.
			Vec3 p1 = view->MapViewToCP(m_mouseDownPos);
			Vec3 p2 = view->MapViewToCP(point);
			Vec3 v = view->GetCPVector(p1,p2);

			if (m_shape->GetSelectedPoint() >= 0)
			{
				Vec3 wp = m_pointPos;
				Vec3 newp = wp + v;
				if (GetIEditor()->GetAxisConstrains() == AXIS_TERRAIN)
				{
					// Keep height.
					newp = view->MapViewToCP(point);
					//float z = wp.z - GetIEditor()->GetTerrainElevation(wp.x,wp.y);
					//newp.z = GetIEditor()->GetTerrainElevation(newp.x,newp.y) + z;
					//newp.z = GetIEditor()->GetTerrainElevation(newp.x,newp.y) + SHAPE_Z_OFFSET;
					newp.z += SHAPE_Z_OFFSET;
				}

				if (newp.x != 0 && newp.y != 0 && newp.z != 0)
				{
					// Put newp into local space of shape.
					Matrix44 invShapeTM = shapeTM;
					invShapeTM.Invert44();
					newp = invShapeTM.TransformPointOLD(newp);

					if (GetIEditor()->IsUndoRecording())
						m_shape->StoreUndo( "Move Point" );
					m_shape->SetPoint( m_shape->GetSelectedPoint(),newp );
				}
			}
		}

		/*
		Vec3 raySrc,rayDir;
		view->ViewToWorldRay( point,raySrc,rayDir );
		CBaseObject *hitObj = GetIEditor()->GetObjectManager()->HitTest( raySrc,rayDir,view->GetSelectionTolerance() );
		*/
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


// CShapePanel dialog

IMPLEMENT_DYNAMIC(CShapePanel, CDialog)

//////////////////////////////////////////////////////////////////////////
CShapePanel::CShapePanel( CWnd* pParent /* = NULL */)
	: CDialog(CShapePanel::IDD, pParent)
{
	Create( IDD,AfxGetMainWnd() );
}

CShapePanel::~CShapePanel()
{
}

void CShapePanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PICK, m_pickButton);
	DDX_Control(pDX, IDC_SELECT, m_selectButton);
	DDX_Control(pDX, IDC_EDIT_SHAPE, m_editShapeButton);
	DDX_Control(pDX, IDC_ENTITIES, m_entities);
}


BEGIN_MESSAGE_MAP(CShapePanel, CDialog)
	ON_BN_CLICKED(IDC_SELECT, OnBnClickedSelect)
	ON_BN_CLICKED(IDC_REMOVE, OnBnClickedRemove)
	ON_LBN_DBLCLK(IDC_ENTITIES, OnLbnDblclkEntities)
END_MESSAGE_MAP()


// CShapePanel message handlers

BOOL CShapePanel::OnInitDialog()
{
	__super::OnInitDialog();

	m_pickButton.SetPickCallback( this,"Pick Entity" );
	m_entities.SetBkColor( RGB(0xE0,0xE0,0xE0) );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CShapePanel::SetShape( CShapeObject *shape )
{
	assert( shape );
	m_shape = shape;

	if (shape->GetPointCount() > 1)
		m_editShapeButton.SetToolClass( RUNTIME_CLASS(CEditShapeObjectTool),m_shape );
	else
		m_editShapeButton.EnableWindow( FALSE );

	ReloadEntities();

	CString str;
	str.Format( "Num Points: %d",shape->GetPointCount() );
	GetDlgItem(IDC_NUM_POINTS)->SetWindowText( str );
}

//////////////////////////////////////////////////////////////////////////
void CShapePanel::OnPick( CBaseObject *picked )
{
	assert( m_shape );
	CUndo undo("[Shape] Add Entity");
	m_shape->AddEntity( picked );
	ReloadEntities();
//	m_entityName.SetWindowText( picked->GetName() );
}

//////////////////////////////////////////////////////////////////////////
bool CShapePanel::OnPickFilter( CBaseObject *picked )
{
	assert( picked != 0 );
	return picked->GetType() == OBJTYPE_ENTITY;
}

//////////////////////////////////////////////////////////////////////////
void CShapePanel::OnCancelPick()
{
}

//////////////////////////////////////////////////////////////////////////
void CShapePanel::OnBnClickedSelect()
{
	assert( m_shape );
	int sel = m_entities.GetCurSel();
	if (sel != LB_ERR)
	{
		CBaseObject *obj = m_shape->GetEntity(sel);
		if (obj)
		{
			GetIEditor()->ClearSelection();
			GetIEditor()->SelectObject( obj );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CShapePanel::ReloadEntities()
{
	if (!m_shape)
		return;

	m_entities.ResetContent();
	for (int i = 0; i < m_shape->GetEntityCount(); i++)
	{
		CBaseObject *obj = m_shape->GetEntity(i);
		if (obj)
			m_entities.AddString( obj->GetName() );
		else
			m_entities.AddString( "<Null>" );
	}
}

//////////////////////////////////////////////////////////////////////////
void CShapePanel::OnBnClickedRemove()
{
	assert( m_shape );
	int sel = m_entities.GetCurSel();
	if (sel != LB_ERR)
	{
		CUndo undo("[Shape] Remove Entity");
		if (sel < m_shape->GetEntityCount())
			m_shape->RemoveEntity(sel);
		ReloadEntities();
	}
}

void CShapePanel::OnLbnDblclkEntities()
{
	// Select current entity.
	OnBnClickedSelect();
}
