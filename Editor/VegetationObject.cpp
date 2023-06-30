// StaticObjParam.cpp: implementation of the CVegetationObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VegetationObject.h"
#include "Material\Material.h"
#include "Material\MaterialManager.h"
#include "ErrorReport.h"

#include "Heightmap.h"
#include "VegetationMap.h"

#include "I3DEngine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVegetationObject::CVegetationObject( int id,CVegetationMap *pVegMap )
{
	m_vegetetionMap = pVegMap;
	m_id = id;

	m_statObj = 0;
	m_objectSize = 1;
	m_numInstances = 0;
	m_bSelected = false;
	m_bHidden = false;
	m_index = 0;
	m_bInUse = true;

	m_bVarIgnoreChange = false;
	
	m_category = _T("Default");

	// Int vars.
	mv_size = 1;
	mv_hmin = GetIEditor()->GetHeightmap()->GetWaterLevel();
	mv_hmax = 255;
	mv_slope_min = 0;
	mv_slope_max = 255;

	mv_density = 1;
	mv_bending = 0;
	mv_sizevar = 0;
	mv_castShadows = false;
	mv_recvShadows = false;
	mv_precalcShadows = false;
	mv_PhysNonColl = false;
	mv_alphaBlend = false;
//	mv_useLightBit = false;
	mv_hideable = false;
	mv_SpriteDistRatio = 1;
	mv_MaxViewDistRatio = 1;
	mv_ShadowDistRatio = 1;
	mv_brightness = 1;
	mv_realtimeShadow = false;
//	mv_AmbScale = 1;
	mv_SpriteTexRes = 0;
  mv_BackSideLevel = 1;
  mv_CalcLighting = true;
  mv_UseSprites = true;
	mv_FadeSize = true;

	CoCreateGuid(&m_guid);

	AddVariable( mv_size,"Size",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_sizevar,"SizeVar",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_bending,"Bending",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_hideable,"Hideable",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_PhysNonColl,"PhysNonColl",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_brightness,"Brightness",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_density,"Density",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_hmin,"ElevationMin",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_hmax,"ElevationMax",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_slope_min,"SlopeMin",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_slope_max,"SlopeMax",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_castShadows,"CastShadow",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_recvShadows,"RecvShadow",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_precalcShadows,"PrecalcShadow",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_realtimeShadow,"RealTimeShadow",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_alphaBlend,"AlphaBlend",functor(*this,&CVegetationObject::OnVarChange) );
//	AddVariable( mv_useLightBit,"UseLigthBit",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_SpriteDistRatio,"SpriteDistRatio",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_ShadowDistRatio,"ShadowDistRatio",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_MaxViewDistRatio,"MaxViewDistRatio",functor(*this,&CVegetationObject::OnVarChange) );
//	AddVariable( mv_AmbScale,"AmbScale",functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_SpriteTexRes, "SpriteTexRes", functor(*this,&CVegetationObject::OnVarChange) );
  AddVariable( mv_material, "Material", functor(*this,&CVegetationObject::OnMaterialChange) );
  AddVariable( mv_BackSideLevel, "BackSideLevel", functor(*this,&CVegetationObject::OnVarChange) );
  AddVariable( mv_CalcLighting, "CalcLighting", functor(*this,&CVegetationObject::OnVarChange) );
  AddVariable( mv_UseSprites, "UseSprites", functor(*this,&CVegetationObject::OnVarChange) );
	AddVariable( mv_FadeSize, "FadeSize", functor(*this,&CVegetationObject::OnVarChange) );
}

CVegetationObject::~CVegetationObject()
{
	if (m_statObj)
	{
		GetIEditor()->GetSystem()->GetI3DEngine()->ReleaseObject( m_statObj );
	}
	if (m_id >= 0)
	{
		IStatInstGroup grp;
		GetIEditor()->GetSystem()->GetI3DEngine()->SetStatInstGroup( m_id,grp );
	}
}

void CVegetationObject::SetFileName( const CString &strFileName )
{
	if (m_strFileName != strFileName)
	{
		m_strFileName = strFileName;
		UnloadObject();
		LoadObject();
	}
	SetEngineParams();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationObject::SetCategory( const CString &category )
{
	m_category = category;
};

//////////////////////////////////////////////////////////////////////////
void CVegetationObject::UnloadObject()
{
	if (m_statObj)
	{
		GetIEditor()->GetSystem()->GetI3DEngine()->ReleaseObject( m_statObj );
	}
	m_statObj = 0;
	m_objectSize = 1;

	SetEngineParams();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationObject::LoadObject()
{
	m_objectSize = 1;
	if (m_statObj == 0 && !m_strFileName.IsEmpty())
	{
		GetIEditor()->GetErrorReport()->SetCurrentFile( m_strFileName );
		m_statObj = GetIEditor()->GetSystem()->GetI3DEngine()->MakeObject( m_strFileName, 0, evs_ShareAndSortForCache, false );
		if (m_statObj)
		{
			Vec3 min = m_statObj->GetBoxMin();
			Vec3 max = m_statObj->GetBoxMax();
			m_objectSize = __max( max.x-min.x,max.y-min.y );

			Validate( *GetIEditor()->GetErrorReport() );
		}
		GetIEditor()->GetErrorReport()->SetCurrentFile( "" );
	}
	SetEngineParams();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationObject::SetHidden( bool bHidden )
{
	m_bHidden = bHidden;
	SetInUse( !bHidden );

	GetIEditor()->SetModifiedFlag();
	/*
	for (int i = 0; i < GetObjectCount(); i++)
	{
		CVegetationObject *obj = GetObject(i);
		obj->SetInUse( !bHidden );
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CVegetationObject::CopyFrom( const CVegetationObject &o )
{
	CopyVariableValues( const_cast<CVegetationObject*>(&o) );

	m_strFileName = o.m_strFileName;
	m_bInUse = o.m_bInUse;
	m_bHidden = o.m_bHidden;
	m_category = o.m_category;

	LoadObject();

	GetIEditor()->SetModifiedFlag();
	SetEngineParams();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationObject::OnVarChange( IVariable *var )
{
	if (m_bVarIgnoreChange)
		return;
	
	SetEngineParams();
	GetIEditor()->SetModifiedFlag();

	if (var == &mv_hideable || var == &mv_PhysNonColl)
	{
		// Reposition this object on vegetation map.
		m_vegetetionMap->RepositionObject( this );
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationObject::OnMaterialChange( IVariable *var )
{
	if (m_bVarIgnoreChange)
		return;

	m_pMaterial = 0;
	CString mtlName = mv_material;
	if (!mtlName.IsEmpty())
		m_pMaterial = (CMaterial*)GetIEditor()->GetMaterialManager()->FindItemByName( mv_material );
	if (m_pMaterial)
	{
		m_pMaterial->SetUsed();
	}
	SetEngineParams();
	GetIEditor()->SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationObject::SetEngineParams()
{
	I3DEngine *engine = GetIEditor()->GetSystem()->GetI3DEngine();
	if (!engine)
		return;

	IStatInstGroup grp;
	grp.pStatObj = m_statObj;
	grp.bHideability = mv_hideable;
	grp.bPhysNonColl = mv_PhysNonColl;
	grp.fBending = mv_bending;
	grp.bCastShadow = mv_castShadows;
	grp.bRecvShadow = mv_recvShadows;
	grp.bPrecShadow = mv_precalcShadows;
	grp.bUseAlphaBlending = mv_alphaBlend;
//	grp.bTakeBrightnessFromLightBit = mv_useLightBit;
	grp.fSpriteDistRatio = mv_SpriteDistRatio;
	grp.fShadowDistRatio = mv_ShadowDistRatio;
	grp.fMaxViewDistRatio =mv_MaxViewDistRatio;
	grp.fBrightness = mv_brightness;
	grp.bUpdateShadowEveryFrame = mv_realtimeShadow;
//	grp.fAmbScale = mv_AmbScale;
	grp.nSpriteTexRes = mv_SpriteTexRes;
	grp.pMaterial = 0;
	if (m_pMaterial)
		grp.pMaterial = m_pMaterial->GetMatInfo();
  grp.fBackSideLevel = mv_BackSideLevel;
  grp.bCalcLighting = mv_CalcLighting;
  grp.bUseSprites = mv_UseSprites;
	grp.bFadeSize = mv_FadeSize;

	engine->SetStatInstGroup( m_id,grp );
}

//////////////////////////////////////////////////////////////////////////
void CVegetationObject::Serialize( XmlNodeRef &node,bool bLoading )
{
	m_bVarIgnoreChange = true;
	CVarObject::Serialize( node,bLoading );
	m_bVarIgnoreChange = false;
	if (bLoading)
	{
		// Loading
		CString fileName;
		node->getAttr( "FileName",fileName );
		node->getAttr( "GUID",m_guid );
		node->getAttr( "Hidden",m_bHidden );
		node->getAttr( "Category",m_category );

		SetFileName( fileName );

		CString mtlName = mv_material;
		if (!mtlName.IsEmpty())
			m_pMaterial = (CMaterial*)GetIEditor()->GetMaterialManager()->FindItemByName( mv_material );
		if (m_pMaterial)
		{
			m_pMaterial->SetUsed();
		}
		SetEngineParams();
	}
	else
	{
		// Save.
		node->setAttr( "FileName",m_strFileName );
		node->setAttr( "GUID",m_guid );
		node->setAttr( "Hidden",m_bHidden );
		node->setAttr( "Index",m_index );
		node->setAttr( "Category",m_category );
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationObject::Validate( CErrorReport &report )
{
	if (m_statObj && m_statObj->IsDefaultObject())
	{
		// File Not found.
		CErrorRecord err;
		err.error.Format( "Geometry file %s for Vegetation Object not found",(const char*)m_strFileName );
		err.file = m_strFileName;
		err.severity = CErrorRecord::ESEVERITY_WARNING;
		err.flags = CErrorRecord::FLAG_NOFILE;
		report.ReportError(err);
	}
	if (m_statObj)
	{
		m_statObj->CheckValidVegetation();
	}
}