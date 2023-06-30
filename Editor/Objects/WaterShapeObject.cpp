////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   watershapeobject.cpp
//  Version:     v1.00
//  Created:     10/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "WaterShapeObject.h"
#include "Material\MaterialManager.h"

#include <I3DEngine.h>

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CWaterShapeObject,CShapeObject)

//////////////////////////////////////////////////////////////////////////
CWaterShapeObject::CWaterShapeObject()
{
	m_waterVolume = 0;

	ZeroStruct( m_materialGUID );
	
	mv_waterShader = "WaterVolume";
	mv_waterStreamSpeed = 0;
	mv_waterTriMinSize = 8.f;
	mv_waterTriMaxSize = 8.f;
	mv_bAffectToVolFog = false;

	AddVariable( mv_waterShader,"WaterShader", "Shader", functor(*this,&CWaterShapeObject::OnWaterChange),IVariable::DT_SHADER );
	AddVariable( mv_waterStreamSpeed, "WaterSpeed", "Speed", functor(*this,&CWaterShapeObject::OnWaterChange) );
	AddVariable( mv_waterTriMinSize, "TriMinSize", "TriMinSize", functor(*this,&CWaterShapeObject::OnWaterChange) );
	AddVariable( mv_waterTriMaxSize, "TriMaxSize", "TriMaxSize", functor(*this,&CWaterShapeObject::OnWaterChange) );
	AddVariable( mv_bAffectToVolFog, "AffectToVolFog", "AffectToVolFog", functor(*this,&CWaterShapeObject::OnWaterChange) );
}

//////////////////////////////////////////////////////////////////////////
bool CWaterShapeObject::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	bool res = CShapeObject::Init( ie,prev,file );
	if (prev)
	{
		m_materialGUID = ((CWaterShapeObject*)prev)->m_materialGUID;
	}
	if (IsCreateGameObjects())
	{
		m_waterVolume = GetIEditor()->Get3DEngine()->CreateWaterVolume();
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////
void CWaterShapeObject::SetName( const CString &name )
{
	bool bChanged = name != GetName();
	CShapeObject::SetName( name );
	if (bChanged)
	{
		UpdateGameArea();
	}
}

//////////////////////////////////////////////////////////////////////////
void CWaterShapeObject::Done()
{
	if (m_waterVolume)
	{
		GetIEditor()->Get3DEngine()->DeleteWaterVolume(m_waterVolume);
	}
	CShapeObject::Done();
}

//////////////////////////////////////////////////////////////////////////
void CWaterShapeObject::OnWaterChange( IVariable *var )
{
	if (m_waterVolume)
	{
		m_waterVolume->SetFlowSpeed( mv_waterStreamSpeed );
		m_waterVolume->SetAffectToVolFog(mv_bAffectToVolFog);	
		CString strShader = mv_waterShader;
		m_waterVolume->SetShader( strShader );
		m_waterVolume->SetTriSizeLimits( mv_waterTriMinSize, mv_waterTriMaxSize );
	}
}

//////////////////////////////////////////////////////////////////////////
void CWaterShapeObject::SetMaterial( CMaterial *mtl )
{
	StoreUndo( "Assign Material" );
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
	m_bAreaModified = true;
	UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
void CWaterShapeObject::Serialize( CObjectArchive &ar )
{
	CShapeObject::Serialize( ar );
	if (ar.bLoading)
	{
		// Loading.
		ZeroStruct(m_materialGUID);
		if (ar.node->getAttr( "MaterialGUID",m_materialGUID ))
		{
			m_pMaterial = (CMaterial*)GetIEditor()->GetMaterialManager()->FindItem( m_materialGUID );
			if (!m_pMaterial)
			{
				CErrorRecord err;
				err.error.Format( _T("Material %s for WaterVolume %s not found."),GuidUtil::ToString(m_materialGUID),(const char*)GetName() );
				err.severity = CErrorRecord::ESEVERITY_WARNING;
				err.pObject = this;
				ar.ReportError(err);

				//Warning( "Material %s for WaterVolume %s not found,",GuidUtil::ToString(m_materialGUID),(const char*)GetName() );
			}
			else
			{
				if (m_pMaterial->GetParent())
					SetMaterial( m_pMaterial->GetParent() );
				m_pMaterial->SetUsed();
			}
		}
		else
			m_pMaterial = 0;
	}
	else
	{
		// Saving.
		if (!GuidUtil::IsEmpty(m_materialGUID))
		{
			ar.node->setAttr( "MaterialGUID",m_materialGUID );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CWaterShapeObject::Export( const CString &levelPath,XmlNodeRef &xmlNode )
{
	XmlNodeRef objNode = CShapeObject::Export( levelPath,xmlNode );
	if (objNode)
	{
		if (m_pMaterial)
		{
			objNode->setAttr( "Material",m_pMaterial->GetFullName() );
		}
	}
	return objNode;
}

//////////////////////////////////////////////////////////////////////////
void CWaterShapeObject::UpdateGameArea( bool bRemove )
{
	if (bRemove)
		return;

	if (m_bIgnoreGameUpdate)
		return;

	if (m_waterVolume)
	{
		std::vector<Vec3> points;
		if (GetPointCount() > 3)
		{
			const Matrix44 &wtm = GetWorldTM();
			points.resize(GetPointCount());
			for (int i = 0; i < GetPointCount(); i++)
			{
				points[i] = wtm.TransformPointOLD( GetPoint(i) );
			}
			m_waterVolume->UpdatePoints( &points[0],points.size(), mv_height );
			
			CString strShader = mv_waterShader;
			m_waterVolume->SetName( GetName() );
			m_waterVolume->SetFlowSpeed( mv_waterStreamSpeed );
			m_waterVolume->SetAffectToVolFog(mv_bAffectToVolFog);	
			m_waterVolume->SetTriSizeLimits( mv_waterTriMinSize, mv_waterTriMaxSize );
			m_waterVolume->SetShader( strShader );
			if (m_pMaterial)
				m_waterVolume->SetMaterial( m_pMaterial->GetMatInfo() );
			else
				m_waterVolume->SetMaterial( 0 );
		}
	}
	m_bAreaModified = false;
}