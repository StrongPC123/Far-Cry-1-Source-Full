////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   BrushObject.cpp
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: CBrushObject implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BrushObject.h"

#include "ObjectManager.h"
#include "..\Viewport.h"

#include "..\Brush\BrushPanel.h"
#include "..\Brush\Brush.h"
#include "PanelTreeBrowser.h"

#include "Entity.h"
#include "EdMesh.h"
#include "Material\Material.h"
#include "Material\MaterialManager.h"
#include "Settings.h"

#include <I3Dengine.h>
#include <IEntitySystem.h>

#define MIN_BOUNDS_SIZE 0.01f

//////////////////////////////////////////////////////////////////////////
// CBase implementation.
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CBrushObject,CBaseObject)

namespace
{
	CBrushPanel* s_brushPanel = NULL;
	int s_brushPanelId = 0;

	CPanelTreeBrowser s_treePanel;
	CPanelTreeBrowser* s_treePanelPtr = NULL;
	int s_treePanelId = 0;

}


//////////////////////////////////////////////////////////////////////////
CBrushObject::CBrushObject()
{
	m_prefabGeom = 0;
	m_indoor = 0;
	m_engineNode = 0;

	m_renderFlags = 0;

	AddVariable( mv_prefabName,"Prefab",functor(*this,&CBrushObject::OnPrefabChange),IVariable::DT_OBJECT );

	// Init Variables.
	mv_outdoor = false;
	mv_castShadows = false;
	mv_selfShadowing = false;
  mv_castShadowMaps = false;
  mv_recvShadowMaps = true;
	mv_castLightmap = true;
	mv_recvLightmap = true;
	mv_hideable = false;
	mv_ratioLOD = 100;
	mv_ratioViewDist = 100;
	mv_excludeFromTriangulation = false;
	mv_lightmapQuality = 1;
	mv_lightmapQuality.SetLimits( 0,100 );

	ZeroStruct(m_materialGUID);

	static CString sVarName_OutdoorOnly = "OutdoorOnly";
	static CString sVarName_CastShadows = "CastShadows";
	static CString sVarName_CastShadows2 = _T("CastShadowVolume");
	static CString sVarName_SelfShadowing = "SelfShadowing";
	static CString sVarName_CastShadowMaps = "CastShadowMaps";
	static CString sVarName_RecvShadowMaps = "RecvShadowMaps";
	static CString sVarName_CastLightmap = "CastLightmap";
	static CString sVarName_ReceiveLightmap = "ReceiveLightmap";
	static CString sVarName_Hideable = "Hideable";
	static CString sVarName_LodRatio = "LodRatio";
	static CString sVarName_ViewDistRatio = "ViewDistRatio";
	static CString sVarName_NotTriangulate = "NotTriangulate";
	static CString sVarName_LightmapQuality = "LightmapQuality";

	ReserveNumVariables( 11 );
	AddVariable( mv_outdoor,sVarName_OutdoorOnly,functor(*this, &CBrushObject::OnRenderVarChange) );
	AddVariable( mv_castShadows,sVarName_CastShadows,sVarName_CastShadows2,functor(*this, &CBrushObject::OnRenderVarChange) );
	AddVariable( mv_selfShadowing,sVarName_SelfShadowing,functor(*this, &CBrushObject::OnRenderVarChange) );
	AddVariable( mv_castShadowMaps,sVarName_CastShadowMaps,functor(*this, &CBrushObject::OnRenderVarChange) );
	AddVariable( mv_recvShadowMaps,sVarName_RecvShadowMaps,functor(*this, &CBrushObject::OnRenderVarChange) );

	AddVariable( mv_castLightmap,sVarName_CastLightmap,functor(*this, &CBrushObject::OnRenderVarChange) );
	AddVariable( mv_recvLightmap,sVarName_ReceiveLightmap,functor(*this, &CBrushObject::OnRenderVarChange) );
	AddVariable( mv_hideable,sVarName_Hideable,functor(*this, &CBrushObject::OnRenderVarChange) );
	AddVariable( mv_ratioLOD,sVarName_LodRatio,functor(*this, &CBrushObject::OnRenderVarChange) );
	AddVariable( mv_ratioViewDist,sVarName_ViewDistRatio,functor(*this, &CBrushObject::OnRenderVarChange) );
	AddVariable( mv_excludeFromTriangulation,sVarName_NotTriangulate,functor(*this, &CBrushObject::OnRenderVarChange) );
	AddVariable( mv_lightmapQuality,sVarName_LightmapQuality );

	mv_ratioLOD.SetLimits( 0,255 );
	mv_ratioViewDist.SetLimits( 0,255 );

	m_bIgnoreNodeUpdate = false;
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::Done()
{
	if (m_engineNode)
	{
		GetIEditor()->Get3DEngine()->DeleteEntityRender(m_engineNode);
		m_engineNode = 0;
	}
	// Release Mesh.
	m_prefabGeom = 0;

	//free brush.
	m_brush = 0;

	CBaseObject::Done();
}

//////////////////////////////////////////////////////////////////////////
bool CBrushObject::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	SetColor( RGB(255,255,255) );
	
	if (IsCreateGameObjects())
	{
		if (prev)
		{
			CBrushObject *brushObj = (CBrushObject*)prev;
			if (brushObj->GetBrush())
			{
				SetBrush( brushObj->GetBrush()->Clone(true) );
			}
		}
		else if (!file.IsEmpty())
		{
			// Create brush from prefab.
			mv_prefabName = file;

			CString name = Path::GetFileName( file );
			SetUniqName( name );
		}
		//m_indoor->AddBrush( this );
	}

	// Must be after SetBrush call.
	bool res = CBaseObject::Init( ie,prev,file );
	
	if (prev)
	{
		CBrushObject *brushObj = (CBrushObject*)prev;
		// Copy material GUID.
		m_materialGUID = brushObj->m_materialGUID;
		m_bbox = brushObj->m_bbox;
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////
bool CBrushObject::CreateGameObject()
{
	if (!m_engineNode)
	{
		m_engineNode = GetIEditor()->Get3DEngine()->CreateEntityRender();
		m_engineNode->SetEditorObjectId( GetId().Data1 );
		OnRenderVarChange(0);
		UpdateEngineNode();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::BeginEditParams( IEditor *ie,int flags )
{
	CBaseObject::BeginEditParams( ie,flags );

	if (!s_brushPanel)
	{
		s_brushPanel = new CBrushPanel;
		s_brushPanelId = ie->AddRollUpPage( ROLLUP_OBJECTS,_T("Brush Parameters"),s_brushPanel );
	}

	if (gSettings.bGeometryBrowserPanel)
	{
		CString prefabName = mv_prefabName;
		if (!prefabName.IsEmpty())
		{
			if (!s_treePanelPtr)
			{
				s_treePanelPtr = &s_treePanel;
				//s_treePanel = new CPanelTreeBrowser;
				int flags = CPanelTreeBrowser::NO_DRAGDROP|CPanelTreeBrowser::NO_PREVIEW|CPanelTreeBrowser::SELECT_ONCLICK;
				s_treePanelPtr->Create( functor(*this, &CBrushObject::OnFileChange),GetClassDesc()->GetFileSpec(),AfxGetMainWnd(),flags );
			}
			if (s_treePanelId == 0)
				s_treePanelId = GetIEditor()->AddRollUpPage( ROLLUP_OBJECTS,_T("Prefab"),s_treePanelPtr,false );
		}

		if (s_treePanelPtr)
		{
			s_treePanelPtr->SetSelectCallback( functor(*this, &CBrushObject::OnFileChange) );
			s_treePanelPtr->SelectFile( prefabName );
		}
	}

	if (s_brushPanel)
		s_brushPanel->SetBrush( this );
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::EndEditParams( IEditor *ie )
{
	CBaseObject::EndEditParams( ie );

	if (s_treePanelId != 0)
	{
		GetIEditor()->RemoveRollUpPage( ROLLUP_OBJECTS,s_treePanelId );
		s_treePanelId = 0;
	}

	if (s_brushPanel)
	{
		ie->RemoveRollUpPage( ROLLUP_OBJECTS,s_brushPanelId );
		s_brushPanel = 0;
		s_brushPanelId = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::BeginEditMultiSelParams( bool bAllOfSameType )
{
	CBaseObject::BeginEditMultiSelParams( bAllOfSameType );
	if (bAllOfSameType)
	{
		if (!s_brushPanel)
		{
			s_brushPanel = new CBrushPanel;
			s_brushPanelId = GetIEditor()->AddRollUpPage( ROLLUP_OBJECTS,_T("Brush Parameters"),s_brushPanel );
		}
		if (s_brushPanel)
			s_brushPanel->SetBrush(0);
	}
}
	
//////////////////////////////////////////////////////////////////////////
void CBrushObject::EndEditMultiSelParams()
{
	CBaseObject::EndEditMultiSelParams();
	if (s_brushPanel)
	{
		GetIEditor()->RemoveRollUpPage( ROLLUP_OBJECTS,s_brushPanelId );
		s_brushPanel = 0;
		s_brushPanelId = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::OnFileChange( CString filename )
{
	CUndo undo("Brush Prefab Modify");
	StoreUndo( "Brush Prefab Modify" );
	mv_prefabName = filename;
	if (m_brush)
		m_brush->ResetToPrefabSize();
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::SetScale( const Vec3d &scale )
{
	// Ignore scale;
	CBaseObject::SetScale( scale );
};

//////////////////////////////////////////////////////////////////////////
void CBrushObject::SetSelected( bool bSelect )
{
	CBaseObject::SetSelected( bSelect );

	if (m_engineNode)
	{
		if (bSelect)
			m_renderFlags |= ERF_SELECTED;
		else
			m_renderFlags &= ~ERF_SELECTED;
		m_engineNode->SetRndFlags( m_renderFlags );
	}
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::GetBoundBox( BBox &box )
{
	box = m_bbox;
	box.Transform( GetWorldTM() );
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::GetLocalBounds( BBox &box )
{
	box = m_bbox;
}

//////////////////////////////////////////////////////////////////////////
int CBrushObject::MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	if (event == eMouseMove || event == eMouseLDown)
	{
		Vec3 pos = view->MapViewToCP( point );
		SetPos( pos );
		if (event == eMouseLDown)
			return MOUSECREATE_OK;
		return MOUSECREATE_CONTINUE;
	}
	return CBaseObject::MouseCreateCallback( view,event,point,flags );
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::Display( DisplayContext &dc )
{
	if (dc.flags & DISPLAY_2D)
	{
		int flags = 0;

		if (IsSelected())
		{
			dc.SetLineWidth(2);
	
			flags = 1;
			//dc.SetSelectedColor();
			dc.SetColor( RGB(225,0,0) );
		}
		else
		{
			flags = 0;
			dc.SetColor( GetColor() );
		}

		dc.PushMatrix( GetWorldTM() );
		dc.DrawWireBox( m_bbox.min,m_bbox.max );
		dc.PopMatrix();
		//if (m_brush)
			//dc.view->DrawBrush( dc,m_brush,GetWorldTM(),flags );

		if (IsSelected())
			dc.SetLineWidth(1);
		
		return;
	}

	/*
	if (m_brush != 0 && m_indoor != 0)
	{
		if (!(m_brush->m_flags&BRF_RE_VALID))
		{
			m_indoor->UpdateObject( m_brush->GetIndoorGeom() );
			m_indoor->RecalcBounds();
		}
		Vec3 invCamSrc = dc.camera->GetPos();
		invCamSrc = m_invertTM.TransformPoint(invCamSrc);

		m_brush->Render( dc.renderer,invCamSrc )
	}
	*/
	
	if (IsSelected())
	{
		dc.SetSelectedColor();
		dc.PushMatrix( GetWorldTM() );
		dc.DrawWireBox( m_bbox.min,m_bbox.max );
		//dc.SetColor( RGB(255,255,0),0.1f ); // Yellow selected color.
		//dc.DrawSolidBox( m_bbox.min,m_bbox.max );
		dc.PopMatrix();
	}
  
	
	DrawDefault( dc );
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CBrushObject::Export( const CString &levelPath,XmlNodeRef &xmlNode )
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::Serialize( CObjectArchive &ar )
{
	XmlNodeRef xmlNode = ar.node;
	m_bIgnoreNodeUpdate = true;
	CBaseObject::Serialize( ar );
	m_bIgnoreNodeUpdate = false;
	if (ar.bLoading)
	{
		ZeroStruct(m_materialGUID);
		if (ar.node->getAttr( "MaterialGUID",m_materialGUID ))
		{
			m_pMaterial = (CMaterial*)GetIEditor()->GetMaterialManager()->FindItem( m_materialGUID );
			if (!m_pMaterial)
			{
				CErrorRecord err;
				err.error.Format( "Material %s for Brush %s not found,",GuidUtil::ToString(m_materialGUID),(const char*)GetName() );
				err.pObject = this;
				err.severity = CErrorRecord::ESEVERITY_WARNING;
				ar.ReportError(err);
				//Warning( "Material %s for Brush %s not found,",GuidUtil::ToString(m_materialGUID),(const char*)GetName() );
			}
			else
			{
				if (m_pMaterial->GetParent())
					SetMaterial( m_pMaterial->GetParent() );
				m_pMaterial->SetUsed();
			}
			//if (m_engineNode)
				//UpdateEngineNode();
		}
		else
			m_pMaterial = 0;

			/*
		if (ar.bUndo)
		{
			OnPrefabChange(0);
		}
		*/
		if (!m_prefabGeom)
		{
			CString mesh = mv_prefabName;
			if (!mesh.IsEmpty())
				CreateBrushFromPrefab( mesh );
		}

		if (!m_prefabGeom)
		{
			XmlNodeRef brushNode = xmlNode->findChild( "Brush" );
			if (brushNode)
			{
				SBrush *brush = m_brush;
				if (!brush)
					brush = new SBrush;

				if (!m_prefabGeom)
					brush->Serialize( brushNode,ar.bLoading );

				if (!m_brush)
					SetBrush( brush );
			}
		}
		UpdateEngineNode();
	}
	else
	{
		ar.node->setAttr( "RndFlags",m_renderFlags );
		if (!GuidUtil::IsEmpty(m_materialGUID))
		{
			ar.node->setAttr( "MaterialGUID",m_materialGUID );
		}
		if (m_brush)
		{
			if (!m_prefabGeom)
			{
				XmlNodeRef brushNode = xmlNode->newChild( "Brush" );
				m_brush->Serialize( brushNode,ar.bLoading );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CBrushObject::HitTest( HitContext &hc )
{
	Vec3 pnt;
	
	Vec3 raySrc = hc.raySrc;
	Vec3 rayDir = hc.rayDir;
	WorldToLocalRay( raySrc,rayDir );

	if (m_bbox.IsIntersectRay( raySrc,rayDir,pnt ))
	{
		if (hc.b2DViewport)
		{
			// World space distance.
			hc.dist = GetDistance(hc.raySrc, GetWorldTM().TransformPointOLD(pnt));
			return true;
		}

		IPhysicalEntity *physics = 0;

		if (m_engineNode)
		{
			physics = m_engineNode->GetPhysics();
			if (physics)
			{
				if (physics->GetStatus( &pe_status_nparts() ) == 0)
					physics = 0;
			}
		}

		if (physics)
		{ 
			vectorr origin = hc.raySrc;
			vectorr dir = hc.rayDir*10000.0f;
			ray_hit hit;
			int col = GetIEditor()->GetSystem()->GetIPhysicalWorld()->RayTraceEntity( physics,origin,dir,&hit );
			if (col <= 0)
				return false;

			// World space distance.
			hc.dist = hit.dist;
			return true;
		}
		else
		{
			// World space distance.
			hc.dist = GetDistance(hc.raySrc, GetWorldTM().TransformPointOLD(pnt));
			return true;

			/*
			// No physics collision.
			SBrushFace *face = m_brush->Ray( raySrc,rayDir,&dist );
			//SBrushFace *face = m_brush->Ray( hc.raySrc,hc.rayDir,&dist );
			if (face)
			{
				hc.dist = dist;
				return true;
			}
			*/
		} 
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
int CBrushObject::HitTestAxis( HitContext &hc )
{
	//@HACK Temporary hack.
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::SetBrush( SBrush *brush )
{
	if (m_brush == brush)
		return;

	/*
	if (m_brush)
	{
		m_indoor->RemoveObject( m_brush->GetIndoorGeom() );
	}
	*/

	m_brush = brush;
	if (m_brush)
	{
		m_brush->SetMatrix( GetWorldTM() );
		UpdateEngineNode();
		m_bbox = m_brush->m_bounds;
	}

	// Add brush indoor geometry to indoors.
	//m_indoor->AddObject( m_brush->GetIndoorGeom() );
}
	
//! Retrieve brush assigned to object.
SBrush* CBrushObject::GetBrush()
{
	return m_brush;
}

//////////////////////////////////////////////////////////////////////////
//! Invalidates cached transformation matrix.
void CBrushObject::InvalidateTM()
{
	CBaseObject::InvalidateTM();

	if (m_brush)
	{
		m_brush->SetMatrix( GetWorldTM() );
	}
	if (m_engineNode)
		UpdateEngineNode(true);
	
	m_invertTM = GetWorldTM();
	m_invertTM.Invert44();
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::WorldToLocalRay( Vec3 &raySrc,Vec3 &rayDir )
{
	raySrc = m_invertTM.TransformPointOLD( raySrc );
	rayDir = GetNormalized( m_invertTM.TransformVectorOLD( rayDir ) );
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::SelectBrushSide( const Vec3 &raySrc,const Vec3 &rayDir,bool shear )
{
	if (!m_brush)
		return;

	m_subSelection.Clear();

	Vec3 rSrc = raySrc;
	Vec3 rDir = rayDir;
	Vec3 rTrg;
	WorldToLocalRay( rSrc,rDir );
	rTrg = rSrc + rDir*32768.0f;

	m_brush->SelectSide( rSrc,rDir,shear,m_subSelection );
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::MoveSelectedPoints( const Vec3 &worldOffset )
{
	if (!m_brush)
		return;

	// Store undo.
	StoreUndo( "Stretch Brush" );

	BBox prevBounds = m_brush->m_bounds;

	std::vector<Vec3> prevPoints;
	prevPoints.resize( m_subSelection.points.size() );

	const Matrix44 &tm = GetWorldTM();

	//CHANGED_BY_IVO
	Vec3 ofs = m_invertTM.TransformVectorOLD( worldOffset );

	for (int i = 0; i < m_subSelection.points.size(); i++)
	{
		Vec3 pnt = *m_subSelection.points[i];
		prevPoints[i] = pnt; // Remember previous point.
		pnt = m_invertTM.TransformPointOLD(pnt) + ofs;
		*m_subSelection.points[i] = tm.TransformPointOLD(pnt);
	}
	
	if (m_brush->BuildSolid(false))
	{
		// Now optimize brush if it correctly created.
		// Now move brush. to make position center of the brush again.
		//Vec3 halfSize = (m_brush->m_bounds.max - m_brush->m_bounds.min)/2;
		Vec3 prevMid = (prevBounds.max + prevBounds.min)/2;
		Vec3 mid = (m_brush->m_bounds.max + m_brush->m_bounds.min)/2;
		Vec3 ofs = mid - prevMid;
		m_brush->Move( -ofs );

		SetPos( GetPos() + GetWorldTM().TransformVectorOLD(ofs) );

		m_bbox = m_brush->m_bounds;
	}
	else
	{
		// Restore previous brush.
		for (int i = 0; i < m_subSelection.points.size(); i++)
		{
			*m_subSelection.points[i] = prevPoints[i];
		}
		m_brush->BuildSolid(false);
		CLogFile::WriteLine( "Invalid brush, operation aborted." );
	}
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::OnPrefabChange( IVariable *var )
{
	// Load new prefab model.
	CString objName = mv_prefabName;

	CreateBrushFromPrefab( objName );
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::CreateBrushFromPrefab( const char *meshFilname )
{
	if (m_prefabGeom)
	{
		if (m_prefabGeom->IsSameObject(meshFilname))
		{
			return;
		}
	}

	GetIEditor()->GetErrorReport()->SetCurrentFile( meshFilname );
	m_prefabGeom = CEdMesh::LoadMesh( meshFilname,false );
	GetIEditor()->GetErrorReport()->SetCurrentFile( "" );
	if (m_prefabGeom)
	{
		if (m_brush)
		{
			// If brush already created, assign it with geometry and update IEntityRender
			m_brush->SetPrefabGeom( m_prefabGeom );
			m_brush->ResetToPrefabSize();
			m_bbox = m_brush->m_bounds;
		}
		else
		{
			m_prefabGeom->GetBounds( m_bbox );
			//[Timur] Do not create real brush now.
			// If brush not yet created, make a new brush.
			//SBrush *brush = new SBrush;
			//brush->SetPrefabGeom( m_prefabGeom );
			//SetBrush( brush );
		}
		UpdateEngineNode();
	}
	else if (m_engineNode)
	{
		// Remove this object from engine node.
		m_engineNode->SetEntityStatObj( 0,0,0 );
	}
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::ResetToPrefabSize()
{
	if (m_brush)
	{
		m_brush->ResetToPrefabSize();
		m_bbox = m_brush->m_bounds;
		UpdateEngineNode();
	}
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::ReloadPrefabGeometry()
{
	if (m_prefabGeom)
	{
		m_prefabGeom->ReloadGeometry();
	}
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::OnRenderVarChange( IVariable *var )
{
	UpdateEngineNode();
}

//////////////////////////////////////////////////////////////////////////
IPhysicalEntity* CBrushObject::GetCollisionEntity() const
{
	// Returns physical object of entity.
	if (m_engineNode)
		return m_engineNode->GetPhysics();
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CBrushObject::ConvertFromObject( CBaseObject *object )
{
	CBaseObject::ConvertFromObject( object );
	if (object->IsKindOf(RUNTIME_CLASS(CEntity)))
	{
		CEntity *entity = (CEntity*)object;
		IEntity *pIEntity = entity->GetIEntity();
		if (!pIEntity)
			return false;

		IStatObj *prefab = pIEntity->GetIStatObj(0);
		if (!prefab)
			return false;

		// Copy entity shadow parameters.
		mv_castShadows = entity->IsCastShadow();
		mv_selfShadowing = entity->IsSelfShadowing();
		mv_castShadowMaps = entity->IsCastShadowMaps();
		mv_recvShadowMaps = entity->IsRecvShadowMaps();
		mv_castLightmap = entity->IsCastLightmap();
		mv_recvLightmap = entity->IsRecvLightmap();
		mv_ratioLOD = entity->GetRatioLod();
		mv_ratioViewDist = entity->GetRatioViewDist();

		mv_prefabName = prefab->GetFileName();
	}
	/*
	if (object->IsKindOf(RUNTIME_CLASS(CStaticObject)))
	{
		CStaticObject *pStatObject = (CStaticObject*)object;
		mv_hideable = pStatObject->IsHideable();
	}
	*/
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::UpdateEngineNode( bool bOnlyTransform )
{
	if (m_bIgnoreNodeUpdate)
		return;

	if (!m_engineNode)
		return;

	//////////////////////////////////////////////////////////////////////////
	// Set brush render flags.
	//////////////////////////////////////////////////////////////////////////

	m_renderFlags = 0;
	if (mv_outdoor)
		m_renderFlags |= ERF_OUTDOORONLY;
	if (mv_castShadows)
		m_renderFlags |= ERF_CASTSHADOWVOLUME;
	if (mv_selfShadowing)
		m_renderFlags |= ERF_SELFSHADOW;
	if (mv_castShadowMaps)
		m_renderFlags |= ERF_CASTSHADOWMAPS;
	if (mv_recvShadowMaps)
		m_renderFlags |= ERF_RECVSHADOWMAPS;

	if (mv_castLightmap)
		m_renderFlags |= ERF_CASTSHADOWINTOLIGHTMAP;
	if (mv_recvLightmap)
		m_renderFlags |= ERF_USELIGHTMAPS;
	if (mv_hideable)
		m_renderFlags |= ERF_HIDABLE;
	if (mv_excludeFromTriangulation)
		m_renderFlags |= ERF_EXCLUDE_FROM_TRIANGULATION;

	if (IsSelected())
		m_renderFlags |= ERF_SELECTED;

	int flags = GetRenderFlags();
	m_engineNode->SetRndFlags( m_renderFlags );

	m_engineNode->SetViewDistRatio( mv_ratioViewDist );
	m_engineNode->SetLodRatio( mv_ratioLOD );

	if (m_prefabGeom)
	{
		Matrix44 tm = GetBrushMatrix();
		m_engineNode->SetEntityStatObj( 0,m_prefabGeom->GetGeometry(),&tm );
	}

	// Fast exit if only transformation needs to be changed.
	if (bOnlyTransform)
		return;

	if (m_pMaterial)
	{
		m_pMaterial->AssignToEntity( m_engineNode );
	}
	else
	{
		// Reset all material settings for this node.
		m_engineNode->SetMaterial(0);
	}

	return;
}

//////////////////////////////////////////////////////////////////////////
Matrix44 CBrushObject::GetBrushMatrix() const
{
	if (m_prefabGeom == NULL || m_brush == NULL)
	{
		return GetWorldTM();
	}
	BBox box;
	m_prefabGeom->GetBounds( box );
	Vec3 omin = box.min;
	Vec3 omax = box.max;
	if (omax.x - omin.x == 0)
		omax.x = omin.x + MIN_BOUNDS_SIZE;
	if (omax.y - omin.y == 0)
		omax.y = omin.y + MIN_BOUNDS_SIZE;
	if (omax.z - omin.z == 0)
		omax.z = omin.z + MIN_BOUNDS_SIZE;

	Matrix44 mov1;
	Matrix44 mov2;
	mov1.SetIdentity();
	mov2.SetIdentity();
	mov1.SetTranslationOLD( -omin );
	mov2.SetTranslationOLD( m_brush->m_bounds.min );

	Vec3 bb = (m_brush->m_bounds.max - m_brush->m_bounds.min);
	Vec3 ob = (omax - omin);
	Vec3 scl;
	scl.x = bb.x / ob.x;
	scl.y = bb.y / ob.y;
	scl.z = bb.z / ob.z;

	Matrix44 stm;
	stm.SetIdentity();

	stm=Matrix33::CreateScale( Vec3d(scl.x,scl.y,scl.z) )*stm;

	Matrix44 tm = mov1 * stm * mov2;
	tm = tm * GetWorldTM();

	return tm;
}

//////////////////////////////////////////////////////////////////////////
IStatObj* CBrushObject::GetPrefabGeom() const
{
	if (!m_prefabGeom)
		return 0;
	return m_prefabGeom->GetGeometry();
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::UpdateVisibility( bool visible )
{
	CBaseObject::UpdateVisibility( visible );
	if (m_engineNode)
	{
		if (!visible)
			m_renderFlags |= ERF_HIDDEN;
		else
			m_renderFlags &= ~ERF_HIDDEN;
		m_engineNode->SetRndFlags( m_renderFlags );
	}
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::SetMaterial( CMaterial *mtl )
{
	StoreUndo( "Assign Material" );
	if (m_prefabGeom)
		m_prefabGeom->SetMaterial( mtl );
	m_pMaterial = mtl;
	if (m_pMaterial)
	{
		m_pMaterial->SetUsed();
		m_materialGUID = m_pMaterial->GetGUID();
	}
	else
	{
		ZeroStruct(m_materialGUID);
	}
	if (m_engineNode)
		UpdateEngineNode();
}

//////////////////////////////////////////////////////////////////////////
CMaterial* CBrushObject::GetMaterial() const
{
	/*
	if (!m_pMaterial)
	{
		if (m_prefabGeom != NULL && m_prefabGeom->GetMaterial())
			return m_prefabGeom->GetMaterial();
	}
	*/
	return m_pMaterial;
}

//////////////////////////////////////////////////////////////////////////
void CBrushObject::Validate( CErrorReport *report )
{
	CBaseObject::Validate( report );
	// Checks for invalid values in base object.
	if (!GuidUtil::IsEmpty(m_materialGUID) && m_pMaterial == NULL)
	{
		CErrorRecord err;
		err.error.Format( "Material %s for Brush %s not found,",GuidUtil::ToString(m_materialGUID),(const char*)GetName() );
		err.pObject = this;
		report->ReportError(err);
		//Warning( "Material %s for Entity %s not found,",GuidUtil::ToString(m_materialGUID),(const char*)GetName() );
	}

	if (!m_prefabGeom)
	{
		CString file = mv_prefabName;
		CErrorRecord err;
		err.error.Format( "No Geometry for Brush %s",(const char*)file,(const char*)GetName() );
		err.file = file;
		err.pObject = this;
		report->ReportError(err);
	}
	else if (m_prefabGeom->IsDefaultObject())
	{
		CString file = mv_prefabName;
		CErrorRecord err;
		err.error.Format( "Geometry file %s for Brush %s Failed to Load",(const char*)file,(const char*)GetName() );
		err.file = file;
		err.pObject = this;
		report->ReportError(err);
	}
}

//////////////////////////////////////////////////////////////////////////
bool CBrushObject::IsSimilarObject( CBaseObject *pObject )
{
	if (pObject->GetClassDesc() == GetClassDesc() && GetRuntimeClass() == pObject->GetRuntimeClass())
	{
		CBrushObject *pBrush = (CBrushObject*)pObject;
		if ((CString)mv_prefabName == (CString)pBrush->mv_prefabName)
			return true;
	}
	return false;
}
