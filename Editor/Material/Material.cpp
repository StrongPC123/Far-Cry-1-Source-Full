////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   material.cpp
//  Version:     v1.00
//  Created:     3/2/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Material.h"
#include "BaseLibrary.h"
#include "ErrorReport.h"

#include <I3DEngine.h>
#include "IEntityRenderState.h"
#include <list2.h>
#include <CryHeaders.h>
#include <ICryAnimation.h>

static bool defaultTexMod_Initialized = false;
static SEfTexModificator defaultTexMod;
static SInputShaderResources defaultShaderResource;

//////////////////////////////////////////////////////////////////////////
CMaterial::CMaterial()
{
	if (!defaultTexMod_Initialized)
	{
		ZeroStruct(defaultTexMod);
		defaultTexMod.m_Tiling[0] = 1;
		defaultTexMod.m_Tiling[1] = 1;
		defaultTexMod_Initialized = true;
	}

	m_mtlFlags = 0;

	m_pParentMtl = NULL;
	m_shaderResources.m_LMaterial = &m_lightMaterial;
	m_shaderResources.m_Opacity = 1;

	for (int i = 0; i < EFTT_MAX; i++)
	{
		m_shaderResources.m_Textures[i].m_Amount = 100;
		m_shaderResources.m_Textures[i].m_TexModificator.m_Tiling[0] = 1;
		m_shaderResources.m_Textures[i].m_TexModificator.m_Tiling[1] = 1;
	}
	m_shaderResources.m_Textures[EFTT_BUMP].m_Amount = 50;
	m_shaderResources.m_Textures[EFTT_NORMALMAP].m_Amount = 50;

	ZeroStruct(m_shaderItem);
	ZeroStruct(m_lightMaterial);
	m_lightMaterial.Front.m_Ambient.Set(1,1,1,1);
	m_lightMaterial.Front.m_Diffuse.Set(0.5,0.5,0.5,1);
	m_lightMaterial.Front.m_Specular.Set(0,0,0,1);
	m_lightMaterial.Front.m_Emission.Set(0,0,0,1);

	// Default shader.
	m_shaderName = "Illumination";
	m_nShaderGenMask = 0;

	m_pMatInfo = GetIEditor()->Get3DEngine()->CreateMatInfo();
	m_bInUse = false;
	m_bRegetPublicParams = true;
}

//////////////////////////////////////////////////////////////////////////
CMaterial::~CMaterial()
{
	// Release used shader.
	SAFE_RELEASE( m_shaderItem.m_pShader );
	SAFE_RELEASE( m_shaderItem.m_pShaderResources );

	RemoveAllSubMaterials();
	GetIEditor()->Get3DEngine()->DeleteMatInfo( m_pMatInfo );
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::SetName( const CString &name )
{
	CBaseLibraryItem::SetName( name );
	GetIEditor()->Get3DEngine()->RenameMatInfo( m_pMatInfo,GetFullName() );
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::SetShaderName( const CString &shaderName )
{
	if (m_shaderName != shaderName)
		m_bRegetPublicParams = true;
	m_shaderName = shaderName;
}

//////////////////////////////////////////////////////////////////////////
bool CMaterial::LoadShader( const CString &shaderName )
{
	// Mark material invalid.
	m_pMatInfo->SetFlags( m_pMatInfo->GetFlags()|MIF_INVALID );

	if (m_shaderName != shaderName)
		m_bRegetPublicParams = true;
	m_shaderName = shaderName;

//	if (shaderName.IsEmpty())
//		return false;

	GetIEditor()->GetErrorReport()->SetCurrentValidatorMaterial( this );

	if (m_mtlFlags & MF_LIGHTING)
		m_shaderResources.m_LMaterial = &m_lightMaterial;
	else
		m_shaderResources.m_LMaterial = 0;

	if (m_mtlFlags & MF_NOSHADOW)
		m_pMatInfo->SetFlags( m_pMatInfo->GetFlags()|MIF_NOCASTSHADOWS );
	else
		m_pMatInfo->SetFlags( m_pMatInfo->GetFlags() & (~MIF_NOCASTSHADOWS) );

	m_shaderResources.m_ResFlags = 0;
	if (m_mtlFlags & MF_WIRE)
		m_shaderResources.m_ResFlags |= MTLFLAG_WIRE;
	if (m_mtlFlags & MF_2SIDED)
		m_shaderResources.m_ResFlags |= MTLFLAG_2SIDED;
	if (m_mtlFlags & MF_ADDITIVE)
		m_shaderResources.m_ResFlags |= MTLFLAG_ADDITIVE;
	if (m_mtlFlags & MF_ADDITIVE_DECAL)
		m_shaderResources.m_ResFlags |= MTLFLAG_ADDITIVEDECAL;

	m_shaderResources.m_ShaderParams.clear();
	for (int i = 0; i < m_shaderParams.size(); i++)
	{
		m_shaderResources.m_ShaderParams.push_back( m_shaderParams[i] );
	}

	// Shader name does not support '.'.
	CString fullName = GetFullName();
	fullName.Replace( '.','_' );
	CString sShader = shaderName;
	if (sShader.IsEmpty())
	{
		sShader = "NoDraw";
	}

	SShaderItem newShaderItem = GetIEditor()->GetRenderer()->EF_LoadShaderItem( "MaterialContainer",eSH_Misc,false,sShader,0,&m_shaderResources,m_nShaderGenMask );

	// Release previously used shader (Must be After new shader is loaded, for speed).
	SAFE_RELEASE( m_shaderItem.m_pShader );
	SAFE_RELEASE( m_shaderItem.m_pShaderResources );

	m_shaderItem = newShaderItem;

	m_shaderResources.m_LMaterial = &m_lightMaterial;
	if (!m_shaderItem.m_pShader)
	{
		CErrorRecord err;
		err.error.Format( _T("Failed to Load Shader %s"),(const char*)shaderName );
		err.pMaterial = this;
		GetIEditor()->GetErrorReport()->ReportError( err );
		GetIEditor()->GetErrorReport()->SetCurrentValidatorMaterial( NULL );
		return false;
	}
	m_pMatInfo->SetShaderItem( m_shaderItem );

	// Initialize shader params if thier number is changed.
	IShader *pTemplShader = m_shaderItem.m_pShader->GetTemplate(-1);
	m_nShaderGenMask = pTemplShader->GetGenerationMask();
	TArray<SShaderParam> &params = pTemplShader->GetPublicParams();
	if (m_bRegetPublicParams)
	{
		m_shaderParams.clear();
		m_shaderParams.reserve( params.GetSize() );
		for (int i = 0; i < params.size(); i++)
		{
			m_shaderParams.push_back( params[i] );
		}
	}
	m_bRegetPublicParams = false;

	// Set mat info as valid again.
	m_pMatInfo->SetFlags( m_pMatInfo->GetFlags()&(~MIF_INVALID) );

	GetIEditor()->GetErrorReport()->SetCurrentValidatorMaterial( NULL );
	return true;
}

//////////////////////////////////////////////////////////////////////////
CVarBlock* CMaterial::GetPublicVars()
{
	if (m_shaderParams.empty())
		return 0;

	CVarBlock *pPublicVars = new CVarBlock;
	for (int i = 0; i < m_shaderParams.size(); i++)
	{
		SShaderParam *pParam = &m_shaderParams[i];
		switch (pParam->m_Type)
		{
		case eType_BYTE:
			{
			CVariable<int> *pVar = new CVariable<int>;
			pVar->SetName( pParam->m_Name );
			*pVar = pParam->m_Value.m_Byte;
			pPublicVars->AddVariable( pVar );
			}
			break;
		case eType_SHORT:
			{
			CVariable<int> *pVar = new CVariable<int>;
			*pVar = pParam->m_Value.m_Short;
			pVar->SetName( pParam->m_Name );
			pPublicVars->AddVariable( pVar );
			}
			break;
		case eType_INT:
			{
			CVariable<int> *pVar = new CVariable<int>;
			*pVar = pParam->m_Value.m_Int;
			pVar->SetName( pParam->m_Name );
			pPublicVars->AddVariable( pVar );
			}
			break;
		case eType_FLOAT:
			{
			CVariable<float> *pVar = new CVariable<float>;
			*pVar = pParam->m_Value.m_Float;
			pVar->SetName( pParam->m_Name );
			pPublicVars->AddVariable( pVar );
			}
			break;
	/*
		case eType_STRING:
			pVar = new CVariable<String>;
			*pVar = pParam->m_Value.m_String;
			pVar->SetName( pParam->m_Name.c_str() );
			pPublicVars->AddVariable( pVar );
			break;
			*/
		case eType_FCOLOR:
			{
			CVariable<Vec3> *pVar = new CVariable<Vec3>;
			*pVar = Vec3(pParam->m_Value.m_Color[0],pParam->m_Value.m_Color[1],pParam->m_Value.m_Color[2]);
			pVar->SetName( pParam->m_Name );
			pVar->SetDataType( IVariable::DT_COLOR );
			pPublicVars->AddVariable( pVar );
			}
			break;
		case eType_VECTOR:
			{
			CVariable<Vec3> *pVar = new CVariable<Vec3>;
			*pVar = Vec3(pParam->m_Value.m_Vector[0],pParam->m_Value.m_Vector[1],pParam->m_Value.m_Vector[2]);
			pVar->SetName( pParam->m_Name );
			pPublicVars->AddVariable( pVar );
			}
			break;
		}
	}

	return pPublicVars;
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::SetPublicVars( CVarBlock *pPublicVars )
{
	assert( pPublicVars );
	if (m_shaderParams.empty())
		return;

	int numVars = pPublicVars->GetVarsCount();

	for (int i = 0; i < m_shaderParams.size(); i++)
	{
		if (i >= numVars)
			break;

		IVariable *pVar = pPublicVars->GetVariable(i);
		SShaderParam *pParam = &m_shaderParams[i];
		switch (pParam->m_Type)
		{
		case eType_BYTE:
			if (pVar->GetType() == IVariable::INT)
			{
				int val = 0;
				pVar->Get(val);
				pParam->m_Value.m_Byte = val;
			}
			break;
		case eType_SHORT:
			if (pVar->GetType() == IVariable::INT)
			{
				int val = 0;
				pVar->Get(val);
				pParam->m_Value.m_Short = val;
			}
			break;
		case eType_INT:
			if (pVar->GetType() == IVariable::INT)
			{
				int val = 0;
				pVar->Get(val);
				pParam->m_Value.m_Int = val;
			}
			break;
		case eType_FLOAT:
			if (pVar->GetType() == IVariable::FLOAT)
			{
				float val = 0;
				pVar->Get(val);
				pParam->m_Value.m_Float = val;
			}
			break;
			/*
		case eType_STRING:
			if (pVar->GetType() == IVariable::STRING)
			{
				CString str;
				int val = 0;
				pVar->Get(val);
				pParam->m_Value.m_Byte = val;
			}
			break;
			*/
		case eType_FCOLOR:
			if (pVar->GetType() == IVariable::VECTOR)
			{
				Vec3 val(0,0,0);
				pVar->Get(val);
				pParam->m_Value.m_Color[0] = val.x;
				pParam->m_Value.m_Color[1] = val.y;
				pParam->m_Value.m_Color[2] = val.z;
			}
			break;
		case eType_VECTOR:
			if (pVar->GetType() == IVariable::VECTOR)
			{
				Vec3 val(0,0,0);
				pVar->Get(val);
				pParam->m_Value.m_Vector[0] = val.x;
				pParam->m_Value.m_Vector[1] = val.y;
				pParam->m_Value.m_Vector[2] = val.z;
			}
			break;
		}
	}
	//m_vars->AddVariable( 
}

//////////////////////////////////////////////////////////////////////////
CVarBlock* CMaterial::GetShaderGenParamsVars()
{	
	if (!m_shaderItem.m_pShader)
		return 0;
	IShader *pTemplShader = m_shaderItem.m_pShader->GetTemplate(-1);
	if (!pTemplShader)
		return 0;

	SShaderGen *pShaderGen = pTemplShader->GetGenerationParams();
	if (!pShaderGen)
		return 0;

	CVarBlock *pBlock = new CVarBlock;
	for (int i = 0; i < pShaderGen->m_BitMask.size(); i++)
	{
		SShaderGenBit *pGenBit = pShaderGen->m_BitMask[i];
		if (!pGenBit->m_ParamProp.empty())
		{
			CVariable<bool> *pVar = new CVariable<bool>;
			pBlock->AddVariable( pVar );
			pVar->SetName( pGenBit->m_ParamProp.c_str() );
			*pVar = (pGenBit->m_Mask & m_nShaderGenMask) != 0;
			pVar->SetDescription( pGenBit->m_ParamDesc.c_str() );
		}
	}
	return pBlock;
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::SetShaderGenParamsVars( CVarBlock *pBlock )
{
	if (!m_shaderItem.m_pShader)
		return;
	IShader *pTemplShader = m_shaderItem.m_pShader->GetTemplate(-1);
	if (!pTemplShader)
		return;

	SShaderGen *pShaderGen = pTemplShader->GetGenerationParams();
	if (!pShaderGen)
		return;

	int nGenMask = 0;

	for (int i = 0; i < pShaderGen->m_BitMask.size(); i++)
	{
		SShaderGenBit *pGenBit = pShaderGen->m_BitMask[i];
		if (!pGenBit->m_ParamProp.empty())
		{
			IVariable *pVar = pBlock->FindVariable(pGenBit->m_ParamProp.c_str());
			bool bFlagOn = false;
			pVar->Get(bFlagOn);
			if (bFlagOn)
				nGenMask |= pGenBit->m_Mask;
		}
	}
	if (m_nShaderGenMask != nGenMask)
	{
		m_bRegetPublicParams = true;
		m_nShaderGenMask = nGenMask;
		CWaitCursor wait;
		Update();
	}
}

//////////////////////////////////////////////////////////////////////////
unsigned int CMaterial::GetTexmapUsageMask() const
{
	int mask = 0;
	if (m_shaderItem.m_pShader)
	{
		IShader *pTempl = m_shaderItem.m_pShader->GetTemplate(-1);
		if (pTempl)
			mask = pTempl->GetUsedTextureTypes();
	}
	return mask;
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::Update()
{
	ICVar *pVar = GetIEditor()->GetSystem()->GetIConsole()->GetCVar( "r_ShadersPrecache" );
	int nOld = (pVar)?pVar->GetIVal():0;
	// Reload shader item with new resources and shader.
	LoadShader( m_shaderName );
	if (pVar)
		pVar->Set(nOld);

	// Mark library as modified.
	GetLibrary()->SetModified();
}

namespace
{
	inline Vec3 ToVec3( const CFColor &col ) { return Vec3(col.r,col.g,col.b); }
	inline CFColor ToCFColor( const Vec3 &col ) { return CFColor(col); }
};

static struct
{
	int texId;
	const char *name;
} sUsedTextures[] =
{
	{ EFTT_DIFFUSE,		"Diffuse" },
	{ EFTT_GLOSS,			"Specular" },
	{ EFTT_BUMP,			"Bumpmap" },
	{ EFTT_NORMALMAP,	"Normalmap" },
	{ EFTT_CUBEMAP,		"Cubemap" },
	{ EFTT_DETAIL_OVERLAY,"Detail" },
	{ EFTT_OPACITY,		"Opacity" },
	{ EFTT_DECAL_OVERLAY,	"Decal" },
  { EFTT_SUBSURFACE,	"SubSurface" },
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CMaterial::Serialize( SerializeContext &ctx )
{
	CBaseLibraryItem::Serialize( ctx );
	XmlNodeRef node = ctx.node;
	if (ctx.bLoading)
	{
		m_shaderParams.clear();
		SInputShaderResources &sr = m_shaderResources;

		m_shaderName = "";
		// Loading
		node->getAttr( "Shader",m_shaderName );
		node->getAttr( "MtlFlags",m_mtlFlags );
		node->getAttr( "GenMask",m_nShaderGenMask );

		// Load lighting data.
		if (m_mtlFlags & MF_LIGHTING)
		{
			Vec3 ambient,diffuse,specular,emissive;
			node->getAttr( "Ambient",ambient );
			node->getAttr( "Diffuse",diffuse );
			node->getAttr( "Specular",specular );
			node->getAttr( "Emissive",emissive );
			node->getAttr( "Shininess",m_lightMaterial.Front.m_SpecShininess );
			m_lightMaterial.Front.m_Ambient = ToCFColor(ambient);
			m_lightMaterial.Front.m_Diffuse = ToCFColor(diffuse);
			m_lightMaterial.Front.m_Specular = ToCFColor(specular);
			m_lightMaterial.Front.m_Emission = ToCFColor(emissive);
		}

		node->getAttr( "Opacity",sr.m_Opacity );
		node->getAttr( "AlphaTest",sr.m_AlphaRef );

		CString texmap,file;
		// 
		XmlNodeRef texturesNode = node->findChild( "Textures" );
		if (texturesNode)
		{
			for (int i = 0; i < texturesNode->getChildCount(); i++)
			{
				texmap = "";
				XmlNodeRef texNode = texturesNode->getChild(i);
				texNode->getAttr( "Map",texmap );

				int texId = -1;
				for (int j = 0; j < sizeof(sUsedTextures)/sizeof(sUsedTextures[0]); j++)
				{
					if (stricmp(sUsedTextures[j].name,texmap) == 0)
					{
						texId = sUsedTextures[j].texId;
						break;
					}
				}
				if (texId < 0)
					continue;

				file = "";
				texNode->getAttr( "File",file );
				
				// Correct texid found.
				sr.m_Textures[texId].m_Name = file;
				texNode->getAttr( "Amount",sr.m_Textures[texId].m_Amount );
				texNode->getAttr( "IsTileU",sr.m_Textures[texId].m_bUTile );
				texNode->getAttr( "IsTileV",sr.m_Textures[texId].m_bVTile );
				texNode->getAttr( "TexType",sr.m_Textures[texId].m_TU.m_eTexType );

				XmlNodeRef modNode = texNode->findChild( "TexMod" );
				if (modNode)
				{
					SEfTexModificator &texm = sr.m_Textures[texId].m_TexModificator;

					// Modificators
					modNode->getAttr( "TileU",texm.m_Tiling[0] );
					modNode->getAttr( "TileV",texm.m_Tiling[1] );
					modNode->getAttr( "OffsetU",texm.m_Offs[0] );
					modNode->getAttr( "OffsetV",texm.m_Offs[1] );

          float f;
					modNode->getAttr( "TexMod_bTexGenProjected",texm.m_bTexGenProjected );
					modNode->getAttr( "TexMod_UOscillatorType",texm.m_eUMoveType );
					modNode->getAttr( "TexMod_VOscillatorType",texm.m_eVMoveType );
					modNode->getAttr( "TexMod_RotateType",texm.m_eRotType );
					modNode->getAttr( "TexMod_TexGenType",texm.m_eTGType );

          if (modNode->getAttr( "RotateU",f ))
            texm.m_Rot[0] = Degr2Word(f);
          if (modNode->getAttr( "RotateV",f ))
            texm.m_Rot[1] = Degr2Word(f);
          if (modNode->getAttr( "RotateW",f ))
            texm.m_Rot[2] = Degr2Word(f);
					if (modNode->getAttr( "TexMod_URotateRate",f ))
            texm.m_RotOscRate[0] = Degr2Word(f);
					if (modNode->getAttr( "TexMod_VRotateRate",f ))
            texm.m_RotOscRate[1] = Degr2Word(f);
          if (modNode->getAttr( "TexMod_WRotateRate",f ))
            texm.m_RotOscRate[2] = Degr2Word(f);
					if (modNode->getAttr( "TexMod_URotatePhase",f ))
            texm.m_RotOscPhase[0] = Degr2Word(f);
					if (modNode->getAttr( "TexMod_VRotatePhase",f ))
            texm.m_RotOscPhase[1] = Degr2Word(f);
          if (modNode->getAttr( "TexMod_WRotatePhase",f ))
            texm.m_RotOscPhase[2] = Degr2Word(f);
					if (modNode->getAttr( "TexMod_URotateAmplitude",f ))
            texm.m_RotOscAmplitude[0] = Degr2Word(f);
					if (modNode->getAttr( "TexMod_VRotateAmplitude",f ))
            texm.m_RotOscAmplitude[1] = Degr2Word(f);
          if (modNode->getAttr( "TexMod_WRotateAmplitude",f ))
            texm.m_RotOscAmplitude[2] = Degr2Word(f);
					modNode->getAttr( "TexMod_URotateCenter",texm.m_RotOscCenter[0] );
          modNode->getAttr( "TexMod_VRotateCenter",texm.m_RotOscCenter[1] );
          modNode->getAttr( "TexMod_WRotateCenter",texm.m_RotOscCenter[2] );

					modNode->getAttr( "TexMod_UOscillatorRate",texm.m_UOscRate );
					modNode->getAttr( "TexMod_VOscillatorRate",texm.m_VOscRate );
					modNode->getAttr( "TexMod_UOscillatorPhase",texm.m_UOscPhase );
					modNode->getAttr( "TexMod_VOscillatorPhase",texm.m_VOscPhase );
					modNode->getAttr( "TexMod_UOscillatorAmplitude",texm.m_UOscAmplitude );
					modNode->getAttr( "TexMod_VOscillatorAmplitude",texm.m_VOscAmplitude );
				}
			}
		}

		// Serialize sub materials.
		XmlNodeRef childsNode = node->findChild( "SubMaterials" );
		if (childsNode)
		{
			if (!ctx.bIgnoreChilds)
			{
				for (int i = 0; i < childsNode->getChildCount(); i++)
				{
					XmlNodeRef mtlNode = childsNode->getChild(i);
					CMaterial *pSubMtl = new CMaterial;
					//GetLibrary()->AddItem( pSubMtl );
					AddSubMaterial( pSubMtl );

					SerializeContext childCtx(ctx);
					childCtx.node = mtlNode;
					pSubMtl->Serialize( childCtx );
				}
			}
		}

		m_bRegetPublicParams = true;
		// Reload shader item with new resources and shader.
		LoadShader( m_shaderName );

		m_bRegetPublicParams = false;
		//////////////////////////////////////////////////////////////////////////
		// Load public parameters.
		//////////////////////////////////////////////////////////////////////////
		XmlNodeRef publicsNode = node->findChild( "PublicParams" );
		if (publicsNode)
		{
			CVarBlockPtr pPublicVars = GetPublicVars();
			if (pPublicVars)
			{
				pPublicVars->Serialize( publicsNode,ctx.bLoading );
				SetPublicVars( pPublicVars );
				// Repeat Load shader... to accept new shader params.
				LoadShader( m_shaderName );
			}
		}
	}
	else
	{
		// Saving.
		node->setAttr( "Shader",m_shaderName );
		node->setAttr( "MtlFlags",m_mtlFlags );
		node->setAttr( "GenMask",m_nShaderGenMask );

  	SInputShaderResources &sr = m_shaderResources;
		if (!m_shaderName.IsEmpty() && (stricmp(m_shaderName,"nodraw") != 0))
		{
			// Save lighing data.
			if (m_mtlFlags & MF_LIGHTING)
			{
				node->setAttr( "Ambient",ToVec3(m_lightMaterial.Front.m_Ambient) );
				node->setAttr( "Diffuse",ToVec3(m_lightMaterial.Front.m_Diffuse) );
				node->setAttr( "Specular",ToVec3(m_lightMaterial.Front.m_Specular) );
				node->setAttr( "Emissive",ToVec3(m_lightMaterial.Front.m_Emission) );
				node->setAttr( "Shininess",m_lightMaterial.Front.m_SpecShininess );
			}

			node->setAttr( "Opacity",sr.m_Opacity );
			node->setAttr( "AlphaTest",sr.m_AlphaRef );

			// Save texturing data.
			XmlNodeRef texturesNode = node->newChild( "Textures" );
			for (int i = 0; i < sizeof(sUsedTextures)/sizeof(sUsedTextures[0]); i++)
			{
				int texId = sUsedTextures[i].texId;
				if (!sr.m_Textures[texId].m_Name.empty())
				{
					XmlNodeRef texNode = texturesNode->newChild( "Texture" );
					//			texNode->setAttr( "TexID",texId );
					texNode->setAttr( "Map",sUsedTextures[i].name );
					texNode->setAttr( "File",sr.m_Textures[texId].m_Name.c_str() );

					if (sr.m_Textures[texId].m_Amount != defaultShaderResource.m_Textures[texId].m_Amount)
						texNode->setAttr( "Amount",sr.m_Textures[texId].m_Amount );
					if (sr.m_Textures[texId].m_bUTile != defaultShaderResource.m_Textures[texId].m_bUTile)
						texNode->setAttr( "IsTileU",sr.m_Textures[texId].m_bUTile );
					if (sr.m_Textures[texId].m_bVTile != defaultShaderResource.m_Textures[texId].m_bVTile)
						texNode->setAttr( "IsTileV",sr.m_Textures[texId].m_bVTile );
					if (sr.m_Textures[texId].m_TU.m_eTexType != defaultShaderResource.m_Textures[texId].m_TU.m_eTexType)
						texNode->setAttr( "TexType",sr.m_Textures[texId].m_TU.m_eTexType );

					SEfTexModificator &texm = sr.m_Textures[texId].m_TexModificator;
					if (memcmp(&texm,&defaultTexMod,sizeof(texm)) == 0)
						continue;

					XmlNodeRef modNode = texNode->newChild( "TexMod" );
					//////////////////////////////////////////////////////////////////////////
					// Save texture modificators Modificators
					//////////////////////////////////////////////////////////////////////////
					if (texm.m_Tiling[0] != defaultTexMod.m_Tiling[0])
						modNode->setAttr( "TileU",texm.m_Tiling[0] );
					if (texm.m_Tiling[1] != defaultTexMod.m_Tiling[1])
						modNode->setAttr( "TileV",texm.m_Tiling[1] );
					if (texm.m_Offs[0] != defaultTexMod.m_Offs[0])
						modNode->setAttr( "OffsetU",texm.m_Offs[0] );
					if (texm.m_Offs[1] != defaultTexMod.m_Offs[1])
						modNode->setAttr( "OffsetV",texm.m_Offs[1] );
					if (texm.m_Rot[0] != defaultTexMod.m_Rot[0])
						modNode->setAttr( "RotateU",Word2Degr(texm.m_Rot[0]) );
					if (texm.m_Rot[1] != defaultTexMod.m_Rot[1])
						modNode->setAttr( "RotateV",Word2Degr(texm.m_Rot[1]) );
					if (texm.m_Rot[2] != defaultTexMod.m_Rot[2])
						modNode->setAttr( "RotateW",Word2Degr(texm.m_Rot[2]) );
					if (texm.m_eUMoveType != defaultTexMod.m_eUMoveType)
						modNode->setAttr( "TexMod_UOscillatorType",texm.m_eUMoveType );
					if (texm.m_eVMoveType != defaultTexMod.m_eVMoveType)
						modNode->setAttr( "TexMod_VOscillatorType",texm.m_eVMoveType );
					if (texm.m_eRotType != defaultTexMod.m_eRotType)
						modNode->setAttr( "TexMod_RotateType",texm.m_eRotType );
					if (texm.m_eTGType != defaultTexMod.m_eTGType)
						modNode->setAttr( "TexMod_TexGenType",texm.m_eTGType );

					if (texm.m_RotOscRate[0] != defaultTexMod.m_RotOscRate[0])
						modNode->setAttr( "TexMod_URotateRate",Word2Degr(texm.m_RotOscRate[0]) );
					if (texm.m_RotOscPhase[0] != defaultTexMod.m_RotOscPhase[0])
						modNode->setAttr( "TexMod_URotatePhase",Word2Degr(texm.m_RotOscPhase[0]) );
					if (texm.m_RotOscAmplitude[0] != defaultTexMod.m_RotOscAmplitude[0])
						modNode->setAttr( "TexMod_URotateAmplitude",Word2Degr(texm.m_RotOscAmplitude[0]) );
					if (texm.m_RotOscRate[1] != defaultTexMod.m_RotOscRate[1])
						modNode->setAttr( "TexMod_VRotateRate",Word2Degr(texm.m_RotOscRate[1]) );
					if (texm.m_RotOscPhase[1] != defaultTexMod.m_RotOscPhase[1])
						modNode->setAttr( "TexMod_VRotatePhase",Word2Degr(texm.m_RotOscPhase[1]) );
					if (texm.m_RotOscAmplitude[1] != defaultTexMod.m_RotOscAmplitude[1])
						modNode->setAttr( "TexMod_VRotateAmplitude",Word2Degr(texm.m_RotOscAmplitude[1]) );
					if (texm.m_RotOscRate[2] != defaultTexMod.m_RotOscRate[2])
						modNode->setAttr( "TexMod_WRotateRate",Word2Degr(texm.m_RotOscRate[2]) );
					if (texm.m_RotOscPhase[2] != defaultTexMod.m_RotOscPhase[2])
						modNode->setAttr( "TexMod_WRotatePhase",Word2Degr(texm.m_RotOscPhase[2]) );
					if (texm.m_RotOscAmplitude[2] != defaultTexMod.m_RotOscAmplitude[2])
						modNode->setAttr( "TexMod_WRotateAmplitude",Word2Degr(texm.m_RotOscAmplitude[2]) );
					if (texm.m_RotOscCenter[0] != defaultTexMod.m_RotOscCenter[0])
						modNode->setAttr( "TexMod_URotateCenter",texm.m_RotOscCenter[0] );
					if (texm.m_RotOscCenter[1] != defaultTexMod.m_RotOscCenter[1])
						modNode->setAttr( "TexMod_VRotateCenter",texm.m_RotOscCenter[1] );
					if (texm.m_RotOscCenter[2] != defaultTexMod.m_RotOscCenter[2])
						modNode->setAttr( "TexMod_WRotateCenter",texm.m_RotOscCenter[2] );

					if (texm.m_UOscRate != defaultTexMod.m_UOscRate)
						modNode->setAttr( "TexMod_UOscillatorRate",texm.m_UOscRate );
					if (texm.m_VOscRate != defaultTexMod.m_VOscRate)
						modNode->setAttr( "TexMod_VOscillatorRate",texm.m_VOscRate );
					if (texm.m_UOscPhase != defaultTexMod.m_UOscPhase)
						modNode->setAttr( "TexMod_UOscillatorPhase",texm.m_UOscPhase );
					if (texm.m_VOscPhase != defaultTexMod.m_VOscPhase)
						modNode->setAttr( "TexMod_VOscillatorPhase",texm.m_VOscPhase );
					if (texm.m_UOscAmplitude != defaultTexMod.m_UOscAmplitude)
						modNode->setAttr( "TexMod_UOscillatorAmplitude",texm.m_UOscAmplitude );
					if (texm.m_VOscAmplitude != defaultTexMod.m_VOscAmplitude)
						modNode->setAttr( "TexMod_VOscillatorAmplitude",texm.m_VOscAmplitude );
				}
			}
		}

		if (GetSubMaterialCount() > 0)
		{
			// Serialize sub materials.
			XmlNodeRef childsNode = node->newChild( "SubMaterials" );
			for (int i = 0; i < GetSubMaterialCount(); i++)
			{
				XmlNodeRef mtlNode = childsNode->newChild( "Material" );
				SerializeContext childCtx(ctx);
				childCtx.node = mtlNode;
				GetSubMaterial(i)->Serialize( childCtx );
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Save public parameters.
		//////////////////////////////////////////////////////////////////////////
		CVarBlockPtr pPublicVars = GetPublicVars();
		if (pPublicVars)
		{
			XmlNodeRef publicsNode = node->newChild( "PublicParams" );
			pPublicVars->Serialize( publicsNode,ctx.bLoading );
		}
	}
}

/*
//////////////////////////////////////////////////////////////////////////
void CMaterial::SerializePublics( XmlNodeRef &node,bool bLoading )
{
	if (bLoading)
	{
	}
	else
	{
		if (m_shaderParams.empty())
			return;
		XmlNodeRef publicsNode = node->newChild( "PublicParams" );

		for (int i = 0; i < m_shaderParams.size(); i++)
		{
			XmlNodeRef paramNode = node->newChild( "Param" );
			SShaderParam *pParam = &m_shaderParams[i];
			paramNode->setAttr( "Name",pParam->m_Name );
			switch (pParam->m_Type)
			{
			case eType_BYTE:
				paramNode->setAttr( "Value",(int)pParam->m_Value.m_Byte );
				paramNode->setAttr( "Type",(int)pParam->m_Value.m_Byte );
				break;
			case eType_SHORT:
				paramNode->setAttr( "Value",(int)pParam->m_Value.m_Short );
				break;
			case eType_INT:
				paramNode->setAttr( "Value",(int)pParam->m_Value.m_Int );
				break;
			case eType_FLOAT:
				paramNode->setAttr( "Value",pParam->m_Value.m_Float );
				break;
			case eType_STRING:
				paramNode->setAttr( "Value",pParam->m_Value.m_String );
			break;
			case eType_FCOLOR:
				paramNode->setAttr( "Value",Vec3(pParam->m_Value.m_Color[0],pParam->m_Value.m_Color[1],pParam->m_Value.m_Color[2]) );
				break;
			case eType_VECTOR:
				paramNode->setAttr( "Value",Vec3(pParam->m_Value.m_Vector[0],pParam->m_Value.m_Vector[1],pParam->m_Value.m_Vector[2]) );
				break;
			}
		}
	}
}
*/

//////////////////////////////////////////////////////////////////////////
void CMaterial::AssignToEntity( IEntityRender *pEntity )
{
	assert( pEntity );

	pEntity->SetMaterial( NULL );
	
	// Only material without parent can be assigned to geometry (not sub material).
	if (m_pParentMtl)
	{
		m_pParentMtl->AssignToEntity( pEntity );
	}
	else
	{
		if (!m_shaderItem.m_pShader)
			return;

		pEntity->SetMaterial( m_pMatInfo );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::AssignFromGeometry( IStatObj *pGeometry )
{
	CLeafBuffer *pLeafBuffer = pGeometry->GetLeafBuffer();
	if (!pLeafBuffer)
		return;

	if (!pLeafBuffer->m_pMats)
		return;

	int numGeomMtls = pLeafBuffer->m_pMats->size();
	if (numGeomMtls < 1)
		return;

	CMatInfo *pMatInfo = &(*pLeafBuffer->m_pMats)[0];
	if (pMatInfo)
	{
		SetFromMatInfo( pMatInfo );
	}
	for (int i = 1; i < pLeafBuffer->m_pMats->size(); i++)
	{
		CString name;
		name.Format( "[%d]",i );
		const char *sMtlName = (*pLeafBuffer->m_pMats)[i].sMaterialName;
		if (strlen(sMtlName) > 0)
		{
			name += " ";
			name += sMtlName;
		}
		CMaterial *pSubMtl = new CMaterial;
		pSubMtl->SetName( name );
		//GetLibrary()->AddItem( pSubMtl );
		AddSubMaterial( pSubMtl );
		pSubMtl->SetFromMatInfo( &(*pLeafBuffer->m_pMats)[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::AssignFromGeometry( ICryCharInstance *pCharacter )
{
	assert(pCharacter);
	
	const list2<CMatInfo> *pMats = pCharacter->getLeafBufferMaterials();

	if (!pMats)
		return;

	int numGeomMtls = pMats->size();
	if (numGeomMtls < 1)
		return;

	CMatInfo *pMatInfo = &(*pMats)[0];
	if (pMatInfo)
	{
		SetFromMatInfo( pMatInfo );
	}
	for (int i = 1; i < pMats->size(); i++)
	{
		CString name;
		name.Format( "[%d]",i );
		const char *sMtlName = (*pMats)[i].sMaterialName;
		if (strlen(sMtlName) > 0)
		{
			name += " ";
			name += sMtlName;
		}
		CMaterial *pSubMtl = new CMaterial;
		pSubMtl->SetName( name );
		//GetLibrary()->AddItem( pSubMtl );
		AddSubMaterial( pSubMtl );
		pSubMtl->SetFromMatInfo( &(*pMats)[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::SetFromMatInfo( CMatInfo *pMatInfo )
{
	assert( pMatInfo );

	m_shaderName = "";
	m_mtlFlags = 0;

	if (pMatInfo->shaderItem.m_pShader)
	{
		// Get name of template.
		IShader *pTemplShader = pMatInfo->shaderItem.m_pShader->GetTemplate(-1);
		if (pTemplShader)
			m_shaderName = pTemplShader->GetName();
	}
	if (pMatInfo->shaderItem.m_pShaderResources)
	{
		m_shaderResources = SInputShaderResources((pMatInfo->shaderItem.m_pShaderResources));
		if (pMatInfo->shaderItem.m_pShaderResources->m_LMaterial)
		{
			m_lightMaterial = *pMatInfo->shaderItem.m_pShaderResources->m_LMaterial;
			m_mtlFlags |= MF_LIGHTING;
		}
	}

	if (m_shaderResources.m_ResFlags & MTLFLAG_WIRE)
		m_mtlFlags |= MF_WIRE;
	if (m_shaderResources.m_ResFlags & MTLFLAG_2SIDED)
		m_mtlFlags |= MF_2SIDED;
	if (m_shaderResources.m_ResFlags & MTLFLAG_ADDITIVE)
		m_mtlFlags |= MF_ADDITIVE;
	if (m_shaderResources.m_ResFlags & MTLFLAG_ADDITIVEDECAL)
		m_mtlFlags |= MF_ADDITIVE_DECAL;

	LoadShader( m_shaderName );
}

//////////////////////////////////////////////////////////////////////////
int CMaterial::GetSubMaterialCount() const
{
	return m_subMaterials.size();
}
	
//////////////////////////////////////////////////////////////////////////
CMaterial* CMaterial::GetSubMaterial( int index ) const
{
	assert( index >= 0 && index < m_subMaterials.size() );
	return m_subMaterials[index];
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::AddSubMaterial( CMaterial *mtl )
{
	assert( mtl );
	mtl->m_pParentMtl = this;
	mtl->m_library = m_library;
	m_subMaterials.push_back(mtl);
	m_pMatInfo->AddSubMtl( mtl->m_pMatInfo );
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::RemoveSubMaterial( CMaterial *mtl )
{
	assert( mtl );
	TSmartPtr<CMaterial> refholder = mtl;
	if (stl::find_and_erase( m_subMaterials,mtl ))
	{
		m_pMatInfo->RemoveSubMtl( mtl->m_pMatInfo );
		mtl->m_pParentMtl = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::RemoveAllSubMaterials()
{
	m_pMatInfo->RemoveAllSubMtls();
	// Also delete them from the library.
	for (int i = 0; i < m_subMaterials.size(); i++)
	{
		m_subMaterials[i]->m_pParentMtl = NULL;
	}
	m_subMaterials.clear();
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::InsertSubMaterial( int slot,CMaterial *mtl )
{
	if (slot < 0)
		slot = 0;
	if (slot > m_subMaterials.size())
		slot = m_subMaterials.size();

	assert( mtl );
	mtl->m_pParentMtl = this;
	mtl->m_library = m_library;

	m_subMaterials.insert( m_subMaterials.begin() + slot,mtl );
	m_pMatInfo->RemoveAllSubMtls();
	for (int i = 0; i < m_subMaterials.size(); i++)
	{
		m_pMatInfo->AddSubMtl( m_subMaterials[i]->m_pMatInfo );
	}
}

//////////////////////////////////////////////////////////////////////////
int CMaterial::FindSubMaterial( CMaterial *mtl )
{
	for (int i = 0; i < m_subMaterials.size(); i++)
	{
		if (m_subMaterials[i] == mtl)
		{
			return i;
		}
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
CMaterial* CMaterial::GetParent() const
{
	return m_pParentMtl;
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::GenerateIdRecursively()
{
	GenerateId();
	for (int i = 0; i < m_subMaterials.size(); i++)
	{
		m_subMaterials[i]->GenerateIdRecursively();
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::SwapContent( CMaterial *mtl )
{
	int mtlFlags = m_mtlFlags;
	SInputShaderResources shaderResources = m_shaderResources;
	SLightMaterial lightMaterial = m_lightMaterial;
	ShaderPublicParams shaderParams = m_shaderParams;
	CString shaderName = m_shaderName;

	m_mtlFlags = mtl->m_mtlFlags;
	m_shaderResources = mtl->m_shaderResources;
	m_lightMaterial = mtl->m_lightMaterial;
	m_shaderParams = mtl->m_shaderParams;
	m_shaderName = mtl->m_shaderName;

	mtl->m_mtlFlags = mtlFlags;
	mtl->m_shaderResources = shaderResources;
	mtl->m_lightMaterial = lightMaterial;
	mtl->m_shaderParams = shaderParams;
	mtl->m_shaderName = shaderName;

	Update();
	mtl->Update();
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::Validate()
{
	// Reload shader.
	LoadShader( m_shaderName );

	// Validate sub materials.
	for (int i = 0; i < m_subMaterials.size(); i++)
	{
		m_subMaterials[i]->Validate();
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterial::GatherUsedResources( CUsedResources &resources )
{
	if (!IsUsed())
		return;

	SInputShaderResources &sr = GetShaderResources();
	for (int texid = 0; texid < EFTT_MAX; texid++)
	{
		if (!sr.m_Textures[texid].m_Name.empty())
		{
			resources.Add( sr.m_Textures[texid].m_Name.c_str() );
		}
	}
}