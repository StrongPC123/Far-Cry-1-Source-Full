////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   MaterialDialog.cpp
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MaterialDialog.h"

#include "StringDlg.h"
#include "NumberDlg.h"

#include "MaterialManager.h"
#include "MaterialLibrary.h"
#include "Material.h"

#include "Objects\BrushObject.h"
#include "Objects\Entity.h"
#include "Objects\ObjectManager.h"
#include "ViewManager.h"
#include "Clipboard.h"

#include "Controls\PropertyItem.h"

#include <I3DEngine.h>
//#include <IEntityRenderState.h>
#include <IEntitySystem.h>

#define IDC_MATERIAL_TREE AFX_IDW_PANE_FIRST

#define EDITOR_OBJECTS_PATH CString("Objects\\Editor\\")

IMPLEMENT_DYNAMIC(CMaterialDialog,CBaseLibraryDialog);
//////////////////////////////////////////////////////////////////////////
// Material structures.
//////////////////////////////////////////////////////////////////////////
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

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

struct STextureVars
{
	CVariable<CString> texture;
	CVariable<int> amount;
	CVariable<bool> is_tile[2];

  CVariableEnum<int> etcgentype;
  CVariableEnum<int> etcmrotatetype;
  CVariableEnum<int> etcmumovetype;
  CVariableEnum<int> etcmvmovetype;
  CVariableEnum<int> etextype;

  CVariable<bool> is_tcgprojected;
  CVariable<float> tiling[3];
  CVariable<float> rotate[3];
  CVariable<float> offset[3];
  CVariable<float> tcmuoscrate;
  CVariable<float> tcmvoscrate;
  CVariable<float> tcmuoscamplitude;
  CVariable<float> tcmvoscamplitude;
  CVariable<float> tcmuoscphase;
  CVariable<float> tcmvoscphase;
  CVariable<float> tcmrotoscrate[3];
  CVariable<float> tcmrotoscamplitude[3];
  CVariable<float> tcmrotoscphase[3];
  CVariable<float> tcmrotosccenter[3];

  CVariableArray tableTiling;
  CVariableArray tableOscillator;
  CVariableArray tableRotator;
};

/** User Interface definition of material.
*/
struct CMaterialUI
{
	CVariable<CString> shader;
	CVariable<bool> bLighting;
	CVariable<bool> bCastShadow;
	CVariable<bool> bAdditive;
	CVariable<bool> bAdditiveDecal;
	CVariable<bool> bWire;
	CVariable<bool> b2Sided;
	CVariable<float> opacity;
  CVariable<float> alphaTest;
	CVariable<bool> bAlwaysExport;

	//////////////////////////////////////////////////////////////////////////
	// Lighting
	//////////////////////////////////////////////////////////////////////////
	CVariable<Vec3> ambient;
	CVariable<Vec3> diffuse;
	CVariable<Vec3> specular;
	CVariable<Vec3> emissive;
	CVariable<float> shininess; //!< Specular shininess.
	
	//////////////////////////////////////////////////////////////////////////
	// Textures.
	//////////////////////////////////////////////////////////////////////////
	CVariableArray textureVars[EFTT_MAX];
	STextureVars textures[EFTT_MAX];

	CVariableArray tableShader;
	CVariableArray tableOpacity;
	CVariableArray tableLighting;
	CVariableArray tableTexture;

  CVarEnumList<int> enumTexType;
	CVarEnumList<int> enumTexGenType;
  CVarEnumList<int> enumTexModRotateType;
  CVarEnumList<int> enumTexModUMoveType;
  CVarEnumList<int> enumTexModVMoveType;

	//////////////////////////////////////////////////////////////////////////
	int texUsageMask;

	CVarBlockPtr m_vars;
	IVariable::OnSetCallback m_onSetCallback;

	//////////////////////////////////////////////////////////////////////////
	void SetFromMaterial( CMaterial *mtl );
	void SetToMaterial( CMaterial *mtl );
	void SetTextureNames( CMaterial *mtl );

	void SetShaderResources( const SInputShaderResources &sr );
	void GetShaderResources( SInputShaderResources &sr );

	//////////////////////////////////////////////////////////////////////////
	CVarBlock* CreateVars()
	{
		m_vars = new CVarBlock;

		//////////////////////////////////////////////////////////////////////////
		// Init enums.
		//////////////////////////////////////////////////////////////////////////
    enumTexType.AddRef(); // We not using pointer.
    enumTexType.AddItem( "Base",eTT_Base );
    enumTexType.AddItem( "Cube-Map",eTT_Cubemap );
    enumTexType.AddItem( "Auto Cube-Map",eTT_AutoCubemap );
    enumTexType.AddItem( "Bumpmap",eTT_Bumpmap );
    enumTexType.AddItem( "DUDVBumpmap",eTT_DSDTBump );
    enumTexType.AddItem( "Rectangle",eTT_Rectangle );

    enumTexGenType.AddRef(); // We not using pointer.
    enumTexGenType.AddItem( "Stream",ETG_Stream );
		enumTexGenType.AddItem( "World",ETG_World );
		enumTexGenType.AddItem( "Camera",ETG_Camera );
		enumTexGenType.AddItem( "World Environment Map",ETG_WorldEnvMap );
		enumTexGenType.AddItem( "Camera Environment Map",ETG_CameraEnvMap );
		enumTexGenType.AddItem( "Normal Map",ETG_NormalMap );
    enumTexGenType.AddItem( "Sphere Map",ETG_SphereMap );

    enumTexModRotateType.AddRef(); // We not using pointer.
    enumTexModRotateType.AddItem( "No Change",ETMR_NoChange );
    enumTexModRotateType.AddItem( "Fixed Rotation",ETMR_Fixed );
    enumTexModRotateType.AddItem( "Constant Rotation",ETMR_Constant );
    enumTexModRotateType.AddItem( "Oscillated Rotation",ETMR_Oscillated );

    enumTexModUMoveType.AddRef(); // We not using pointer.
    enumTexModUMoveType.AddItem( "No Change",ETMM_NoChange );
    enumTexModUMoveType.AddItem( "Fixed Moving",ETMM_Fixed );
    enumTexModUMoveType.AddItem( "Constant Moving",ETMM_Constant );
    enumTexModUMoveType.AddItem( "Jitter Moving",ETMM_Jitter );
    enumTexModUMoveType.AddItem( "Pan Moving",ETMM_Pan );
    enumTexModUMoveType.AddItem( "Stretch Moving",ETMM_Stretch );
    enumTexModUMoveType.AddItem( "Stretch-Repeat Moving",ETMM_StretchRepeat );

    enumTexModVMoveType.AddRef(); // We not using pointer.
    enumTexModVMoveType.AddItem( "No Change",ETMM_NoChange );
    enumTexModVMoveType.AddItem( "Fixed Moving",ETMM_Fixed );
    enumTexModVMoveType.AddItem( "Constant Moving",ETMM_Constant );
    enumTexModVMoveType.AddItem( "Jitter Moving",ETMM_Jitter );
    enumTexModVMoveType.AddItem( "Pan Moving",ETMM_Pan );
    enumTexModVMoveType.AddItem( "Stretch Moving",ETMM_Stretch );
    enumTexModVMoveType.AddItem( "Stretch-Repeat Moving",ETMM_StretchRepeat );

		//////////////////////////////////////////////////////////////////////////
		// Init tables.
		//////////////////////////////////////////////////////////////////////////
		AddVariable( m_vars,tableShader,"Material Settings",IVariable::DT_SIMPLE );
		AddVariable( m_vars,tableOpacity,"Opacity Settings",IVariable::DT_SIMPLE );
		AddVariable( m_vars,tableLighting,"Lighting Settings",IVariable::DT_SIMPLE );
		AddVariable( m_vars,tableTexture,"Texture Maps",IVariable::DT_SIMPLE );

		//////////////////////////////////////////////////////////////////////////
		// Shader.
		//////////////////////////////////////////////////////////////////////////
		AddVariable( tableShader,shader,"Shader",IVariable::DT_SHADER );
		AddVariable( tableShader,bAdditive,"Additive",IVariable::DT_SIMPLE );
		AddVariable( tableShader,bWire,"Wireframe",IVariable::DT_SIMPLE );
		AddVariable( tableShader,b2Sided,"2 Sided",IVariable::DT_SIMPLE );
		AddVariable( tableShader,bCastShadow,"Cast Shadow",IVariable::DT_SIMPLE );
		AddVariable( tableShader,bAlwaysExport,"Export Always" );

		//////////////////////////////////////////////////////////////////////////
		// Opacity.
		//////////////////////////////////////////////////////////////////////////
		AddVariable( tableOpacity,opacity,"Opacity",IVariable::DT_PERCENT );
		AddVariable( tableOpacity,alphaTest,"AlphaTest",IVariable::DT_PERCENT );

		//////////////////////////////////////////////////////////////////////////
		// Lighting.
		//////////////////////////////////////////////////////////////////////////

		AddVariable( tableLighting,bLighting,"Enable Lighting",IVariable::DT_SIMPLE );
		AddVariable( tableLighting,ambient,"Ambient",IVariable::DT_COLOR );
		AddVariable( tableLighting,diffuse,"Diffuse",IVariable::DT_COLOR );
		AddVariable( tableLighting,specular,"Specular",IVariable::DT_COLOR );
		AddVariable( tableLighting,emissive,"Emissive",IVariable::DT_COLOR );
		AddVariable( tableLighting,shininess,"Shininess",IVariable::DT_SIMPLE );

		shininess.SetLimits( 0,1000 );
		
		//////////////////////////////////////////////////////////////////////////
		// Init texture variables.
		//////////////////////////////////////////////////////////////////////////
		for (int i = 0; i < _countof(sUsedTextures); i++)
		{
			InitTextureVars( sUsedTextures[i].texId,sUsedTextures[i].name );
		}
		return m_vars;
	}

private:
	//////////////////////////////////////////////////////////////////////////
	void InitTextureVars( int id,const CString &name )
	{
    textureVars[id].SetFlags( IVariable::UI_BOLD );
    AddVariable( tableTexture,textureVars[id],name,IVariable::DT_SIMPLE );
    // Add variables from STextureVars structure.
    AddVariable( textureVars[id],textures[id].texture,"Texture",IVariable::DT_TEXTURE );
    if (id == EFTT_BUMP || id == EFTT_CUBEMAP)
    {
      AddVariable( textureVars[id],textures[id].amount,"Amount" );
			textures[id].amount.SetLimits( 0,255 );
    }
    AddVariable( textureVars[id],textures[id].etextype,"TexType",IVariable::DT_SIMPLE );

    if (id == EFTT_DECAL_OVERLAY)
    {
      AddVariable( textureVars[id],bAdditiveDecal,"Additive Decal",IVariable::DT_SIMPLE );
    }

    AddVariable( textureVars[id],textures[id].is_tcgprojected,"IsProjectedTexGen",IVariable::DT_SIMPLE );
    AddVariable( textureVars[id],textures[id].etcgentype,"TexGenType",IVariable::DT_SIMPLE );

    //////////////////////////////////////////////////////////////////////////
    // Tiling table.
    AddVariable( textureVars[id],textures[id].tableTiling,"Tiling" );
    {
      CVariableArray& table = textures[id].tableTiling;
      table.SetFlags( IVariable::UI_BOLD );
      AddVariable( table,textures[id].is_tile[0],"IsTileU" );
      AddVariable( table,textures[id].is_tile[1],"IsTileV" );


      AddVariable( table,textures[id].tiling[0],"TileU" );
      AddVariable( table,textures[id].tiling[1],"TileV" );
      AddVariable( table,textures[id].offset[0],"OffsetU" );
      AddVariable( table,textures[id].offset[1],"OffsetV" );
      AddVariable( table,textures[id].rotate[0],"RotateU" );
      AddVariable( table,textures[id].rotate[1],"RotateV" );
      AddVariable( table,textures[id].rotate[2],"RotateW" );
    }

    //////////////////////////////////////////////////////////////////////////
    // Rotator tables.
    AddVariable( textureVars[id],textures[id].tableRotator,"Rotator" );
    {
      CVariableArray& table = textures[id].tableRotator;
      table.SetFlags( IVariable::UI_BOLD );
      AddVariable( table,textures[id].etcmrotatetype,"Type" );
      AddVariable( table,textures[id].tcmrotoscrate[0],"RateU" );
      AddVariable( table,textures[id].tcmrotoscrate[1],"RateV" );
      AddVariable( table,textures[id].tcmrotoscrate[2],"RateW" );
      AddVariable( table,textures[id].tcmrotoscphase[0],"PhaseU" );
      AddVariable( table,textures[id].tcmrotoscphase[1],"PhaseV" );
      AddVariable( table,textures[id].tcmrotoscphase[2],"PhaseW" );
      AddVariable( table,textures[id].tcmrotoscamplitude[0],"AmplitudeU" );
      AddVariable( table,textures[id].tcmrotoscamplitude[1],"AmplitudeV" );
      AddVariable( table,textures[id].tcmrotoscamplitude[2],"AmplitudeW" );
      AddVariable( table,textures[id].tcmrotosccenter[0],"CenterU" );
      AddVariable( table,textures[id].tcmrotosccenter[1],"CenterV" );
      AddVariable( table,textures[id].tcmrotosccenter[2],"CenterW" );
    }

    //////////////////////////////////////////////////////////////////////////
    // Oscillator table
    AddVariable( textureVars[id],textures[id].tableOscillator,"Oscillator" );
    {
      CVariableArray& table = textures[id].tableOscillator;
      table.SetFlags( IVariable::UI_BOLD );
      AddVariable( table,textures[id].etcmumovetype,"TypeU" );
      AddVariable( table,textures[id].etcmvmovetype,"TypeV" );
      AddVariable( table,textures[id].tcmuoscrate,"RateU" );
      AddVariable( table,textures[id].tcmvoscrate,"RateV" );
      AddVariable( table,textures[id].tcmuoscphase,"PhaseU" );
      AddVariable( table,textures[id].tcmvoscphase,"PhaseV" );
      AddVariable( table,textures[id].tcmuoscamplitude,"AmplitudeU" );
      AddVariable( table,textures[id].tcmvoscamplitude,"AmplitudeV" );
    }

		//////////////////////////////////////////////////////////////////////////
		// Assign enums tables to variable.
		//////////////////////////////////////////////////////////////////////////
    textures[id].etextype.SetEnumList( &enumTexType );
		textures[id].etcgentype.SetEnumList( &enumTexGenType );
    textures[id].etcmrotatetype.SetEnumList( &enumTexModRotateType );
    textures[id].etcmumovetype.SetEnumList( &enumTexModUMoveType );
    textures[id].etcmvmovetype.SetEnumList( &enumTexModVMoveType );
	}
	//////////////////////////////////////////////////////////////////////////
  void AddVariable( CVariableArray &varArray,CVariableBase &var,const char *varName,unsigned char dataType=IVariable::DT_SIMPLE )
	{
		var.AddRef(); // Variables are local and must not be released by CVarBlock.
		if (varName)
			var.SetName(varName);
		var.SetDataType(dataType);
		if (m_onSetCallback)
			var.AddOnSetCallback(m_onSetCallback);
		varArray.AddChildVar(&var);
	}
	//////////////////////////////////////////////////////////////////////////
  void AddVariable( CVarBlock *vars,CVariableBase &var,const char *varName,unsigned char dataType=IVariable::DT_SIMPLE )
	{
		var.AddRef(); // Variables are local and must not be released by CVarBlock.
		if (varName)
			var.SetName(varName);
		var.SetDataType(dataType);
		if (m_onSetCallback)
			var.AddOnSetCallback(m_onSetCallback);
		vars->AddVariable(&var);
	}

	void SetTextureResources( const SInputShaderResources &sr,int texid );
	void GetTextureResources( SInputShaderResources &sr,int texid );
	Vec3 ToVec3( const CFColor &col ) { return Vec3(col.r,col.g,col.b); }
	CFColor ToCFColor( const Vec3 &col ) { return CFColor(col); }
};

//////////////////////////////////////////////////////////////////////////
void CMaterialUI::SetShaderResources( const SInputShaderResources &sr )
{
	opacity = sr.m_Opacity;
	alphaTest = sr.m_AlphaRef;

	ambient = ToVec3(sr.m_LMaterial->Front.m_Ambient);
	diffuse = ToVec3(sr.m_LMaterial->Front.m_Diffuse);
	specular = ToVec3(sr.m_LMaterial->Front.m_Specular);
	emissive = ToVec3(sr.m_LMaterial->Front.m_Emission);
	shininess = sr.m_LMaterial->Front.m_SpecShininess;

	if (!bLighting)
	{
		ambient.SetFlags( ambient.GetFlags()|IVariable::UI_DISABLED );
		diffuse.SetFlags( diffuse.GetFlags()|IVariable::UI_DISABLED );
		specular.SetFlags( specular.GetFlags()|IVariable::UI_DISABLED );
		emissive.SetFlags( emissive.GetFlags()|IVariable::UI_DISABLED );
		shininess.SetFlags( shininess.GetFlags()|IVariable::UI_DISABLED );
	}
	else
	{
		ambient.SetFlags( ambient.GetFlags()&(~IVariable::UI_DISABLED) );
		diffuse.SetFlags( diffuse.GetFlags()&(~IVariable::UI_DISABLED) );
		specular.SetFlags( specular.GetFlags()&(~IVariable::UI_DISABLED) );
		emissive.SetFlags( emissive.GetFlags()&(~IVariable::UI_DISABLED) );
		shininess.SetFlags( shininess.GetFlags()&(~IVariable::UI_DISABLED) );
	}

	for (int i = 0; i < _countof(sUsedTextures); i++)
	{
		SetTextureResources( sr,sUsedTextures[i].texId );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialUI::GetShaderResources( SInputShaderResources &sr )
{
	sr.m_Opacity = opacity;
	sr.m_AlphaRef = alphaTest;

	sr.m_LMaterial->Front.m_Ambient = ToCFColor(ambient);
	sr.m_LMaterial->Front.m_Diffuse = ToCFColor(diffuse);
	sr.m_LMaterial->Front.m_Specular = ToCFColor(specular);
	sr.m_LMaterial->Front.m_Emission = ToCFColor(emissive);
	sr.m_LMaterial->Front.m_SpecShininess = shininess;

	for (int i = 0; i < _countof(sUsedTextures); i++)
	{
		GetTextureResources( sr,sUsedTextures[i].texId );
	}
}

inline float RoundDegree( float val )
{
	//double v = floor(val*100.0f);
	//return v*0.01f;
	return (float)((int)(val*100+0.5f)) * 0.01f;
}

//////////////////////////////////////////////////////////////////////////
void CMaterialUI::SetTextureResources( const SInputShaderResources &sr,int tex )
{
	// Enable/Disable texture map, depending on the mask.
	int flags = textureVars[tex].GetFlags();
	if ((1 << tex) & texUsageMask)
		flags &= ~IVariable::UI_DISABLED;
	else
		flags |= IVariable::UI_DISABLED;
	textureVars[tex].SetFlags( flags );

	CString texFilename = sr.m_Textures[tex].m_Name.c_str();

	textureVars[tex].Set( texFilename );

	textures[tex].texture = texFilename;
	textures[tex].amount = sr.m_Textures[tex].m_Amount;
	textures[tex].is_tile[0] = sr.m_Textures[tex].m_bUTile;
	textures[tex].is_tile[1] = sr.m_Textures[tex].m_bVTile;

	textures[tex].tiling[0] = sr.m_Textures[tex].m_TexModificator.m_Tiling[0];
	textures[tex].tiling[1] = sr.m_Textures[tex].m_TexModificator.m_Tiling[1];
	textures[tex].offset[0] = sr.m_Textures[tex].m_TexModificator.m_Offs[0];
	textures[tex].offset[1] = sr.m_Textures[tex].m_TexModificator.m_Offs[1];
  textures[tex].etextype = sr.m_Textures[tex].m_TU.m_eTexType;
  textures[tex].etcgentype = sr.m_Textures[tex].m_TexModificator.m_eTGType;
  textures[tex].etcmumovetype = sr.m_Textures[tex].m_TexModificator.m_eUMoveType;
  textures[tex].etcmvmovetype = sr.m_Textures[tex].m_TexModificator.m_eVMoveType;
  textures[tex].etcmrotatetype = sr.m_Textures[tex].m_TexModificator.m_eRotType;
  textures[tex].is_tcgprojected = sr.m_Textures[tex].m_TexModificator.m_bTexGenProjected;
  textures[tex].tcmuoscrate = sr.m_Textures[tex].m_TexModificator.m_UOscRate;
  textures[tex].tcmuoscphase = sr.m_Textures[tex].m_TexModificator.m_UOscPhase;
  textures[tex].tcmuoscamplitude = sr.m_Textures[tex].m_TexModificator.m_UOscAmplitude;
  textures[tex].tcmvoscrate = sr.m_Textures[tex].m_TexModificator.m_VOscRate;
  textures[tex].tcmvoscphase = sr.m_Textures[tex].m_TexModificator.m_VOscPhase;
  textures[tex].tcmvoscamplitude = sr.m_Textures[tex].m_TexModificator.m_VOscAmplitude;
  for (int i=0; i<3; i++)
  {
    textures[tex].rotate[i] = RoundDegree(Word2Degr(sr.m_Textures[tex].m_TexModificator.m_Rot[i]));
    textures[tex].tcmrotoscrate[i] = RoundDegree(Word2Degr(sr.m_Textures[tex].m_TexModificator.m_RotOscRate[i]));
    textures[tex].tcmrotoscphase[i] = RoundDegree(Word2Degr(sr.m_Textures[tex].m_TexModificator.m_RotOscPhase[i]));
    textures[tex].tcmrotoscamplitude[i] = RoundDegree(Word2Degr(sr.m_Textures[tex].m_TexModificator.m_RotOscAmplitude[i]));
    textures[tex].tcmrotosccenter[i] = sr.m_Textures[tex].m_TexModificator.m_RotOscCenter[i];
  }
}

//////////////////////////////////////////////////////////////////////////
void CMaterialUI::GetTextureResources( SInputShaderResources &sr,int tex )
{
	CString texName = textures[tex].texture;
	sr.m_Textures[tex].m_Name = (const char*)texName;
	sr.m_Textures[tex].m_Amount = textures[tex].amount;
	sr.m_Textures[tex].m_bUTile = textures[tex].is_tile[0];
	sr.m_Textures[tex].m_bVTile = textures[tex].is_tile[1] ;

  sr.m_Textures[tex].m_TexModificator.m_bTexGenProjected = textures[tex].is_tcgprojected;
	sr.m_Textures[tex].m_TexModificator.m_Tiling[0] = textures[tex].tiling[0];
	sr.m_Textures[tex].m_TexModificator.m_Tiling[1] = textures[tex].tiling[1];
	sr.m_Textures[tex].m_TexModificator.m_Offs[0] = textures[tex].offset[0];
	sr.m_Textures[tex].m_TexModificator.m_Offs[1] = textures[tex].offset[1];
  sr.m_Textures[tex].m_TU.m_eTexType = textures[tex].etextype;
  sr.m_Textures[tex].m_TexModificator.m_eRotType = textures[tex].etcmrotatetype;
  sr.m_Textures[tex].m_TU.m_eTexType = textures[tex].etextype;
  sr.m_Textures[tex].m_TexModificator.m_eTGType = textures[tex].etcgentype;
  sr.m_Textures[tex].m_TexModificator.m_eUMoveType = textures[tex].etcmumovetype;
  sr.m_Textures[tex].m_TexModificator.m_eVMoveType = textures[tex].etcmvmovetype;
  sr.m_Textures[tex].m_TexModificator.m_UOscRate = textures[tex].tcmuoscrate;
  sr.m_Textures[tex].m_TexModificator.m_UOscPhase = textures[tex].tcmuoscphase;
  sr.m_Textures[tex].m_TexModificator.m_UOscAmplitude = textures[tex].tcmuoscamplitude;
  sr.m_Textures[tex].m_TexModificator.m_VOscRate = textures[tex].tcmvoscrate;
  sr.m_Textures[tex].m_TexModificator.m_VOscPhase = textures[tex].tcmvoscphase;
  sr.m_Textures[tex].m_TexModificator.m_VOscAmplitude = textures[tex].tcmvoscamplitude;
  for (int i=0; i<3; i++)
  {
    sr.m_Textures[tex].m_TexModificator.m_Rot[i] = Degr2Word(textures[tex].rotate[i]);
    sr.m_Textures[tex].m_TexModificator.m_RotOscRate[i] = Degr2Word(textures[tex].tcmrotoscrate[i]);
    sr.m_Textures[tex].m_TexModificator.m_RotOscPhase[i] = Degr2Word(textures[tex].tcmrotoscphase[i]);
    sr.m_Textures[tex].m_TexModificator.m_RotOscAmplitude[i] = Degr2Word(textures[tex].tcmrotoscamplitude[i]);
    sr.m_Textures[tex].m_TexModificator.m_RotOscCenter[i] = textures[tex].tcmrotosccenter[i];
  }
}

void CMaterialUI::SetFromMaterial( CMaterial *mtl )
{
	shader = mtl->GetShaderName();

	int mtlFlags = mtl->GetMaterialFlags();
	bLighting = mtlFlags & CMaterial::MF_LIGHTING;
	bCastShadow = !(mtlFlags & CMaterial::MF_NOSHADOW);
	bAdditive = (mtlFlags & CMaterial::MF_ADDITIVE);
	bAdditiveDecal = (mtlFlags & CMaterial::MF_ADDITIVE_DECAL);
	bWire = (mtlFlags & CMaterial::MF_WIRE);
	b2Sided = (mtlFlags & CMaterial::MF_2SIDED);
	bAlwaysExport = mtlFlags & CMaterial::MF_ALWAYS_USED;

	texUsageMask = mtl->GetTexmapUsageMask();
	// Detail and decal textures are always active.
	texUsageMask |= 1 << EFTT_DETAIL_OVERLAY;
	texUsageMask |= 1 << EFTT_DECAL_OVERLAY;
	if ((texUsageMask & (1<<EFTT_BUMP)) || (texUsageMask & (1<<EFTT_NORMALMAP)))
	{
		texUsageMask |= 1 << EFTT_BUMP;
		texUsageMask |= 1 << EFTT_NORMALMAP;
	}

	SetShaderResources( mtl->GetShaderResources() );
}

void CMaterialUI::SetToMaterial( CMaterial *mtl )
{
	int mtlFlags = 0;
	if (bLighting)
		mtlFlags |= CMaterial::MF_LIGHTING;
	if (bAlwaysExport)
		mtlFlags |= CMaterial::MF_ALWAYS_USED;
	if (!bCastShadow)
		mtlFlags |= CMaterial::MF_NOSHADOW;
	if (bAdditive)
		mtlFlags |= CMaterial::MF_ADDITIVE;
	if (bAdditiveDecal)
		mtlFlags |= CMaterial::MF_ADDITIVE_DECAL;
	if (bWire)
		mtlFlags |= CMaterial::MF_WIRE;
	if (b2Sided)
		mtlFlags |= CMaterial::MF_2SIDED;
	mtl->SetMaterialFlags(mtlFlags);

	// If shader name is different reload shader.
	mtl->SetShaderName( shader );
	GetShaderResources( mtl->GetShaderResources() );
	mtl->Update();
}

void CMaterialUI::SetTextureNames( CMaterial *mtl )
{
	SInputShaderResources &sr = mtl->GetShaderResources();
	for (int i = 0; i < _countof(sUsedTextures); i++)
	{
		int tex = sUsedTextures[i].texId;
		CString texFilename = sr.m_Textures[tex].m_Name.c_str();
		textureVars[tex].Set( texFilename );
	}
}

static CMaterialUI gMatVars;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CMtlPickCallback : public IPickObjectCallback
{
public:
	CMtlPickCallback() { m_bActive = true; };
	//! Called when object picked.
	virtual void OnPick( CBaseObject *picked )
	{
		m_bActive = false;
		CMaterial *pMtl = picked->GetMaterial();
		if (pMtl)
			GetIEditor()->OpenDataBaseLibrary( EDB_MATERIAL_LIBRARY,pMtl );
		delete this;
	}
	//! Called when pick mode cancelled.
	virtual void OnCancelPick()
	{
		m_bActive = false;
		delete this;
	}
	//! Return true if specified object is pickable.
	virtual bool OnPickFilter( CBaseObject *filterObject )
	{
		// Check if object have material.
		if (filterObject->GetMaterial())
			return true;
		else
			return false;
	}
	static bool IsActive() { return m_bActive; };
private:
	static bool m_bActive;
};
bool CMtlPickCallback::m_bActive = false;
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// CMaterialDialog implementation.
//////////////////////////////////////////////////////////////////////////
CMaterialDialog::CMaterialDialog( CWnd *pParent )
	: CBaseLibraryDialog(IDD_DB_ENTITY, pParent)
{
	m_pMatManager = GetIEditor()->GetMaterialManager();
	m_pItemManager = m_pMatManager;

	m_bRealtimePreviewUpdate = true;
	m_pGeometry = 0;
	m_pEntityRender = 0;
	m_publicVarsItems = 0;

	m_shaderGenParamsVars = 0;
	m_shaderGenParamsVarsItem = 0;
	
	m_drawType = DRAW_BOX;
	m_geometryFile = EDITOR_OBJECTS_PATH + "MtlBox.cgf";
	m_bOwnGeometry = true;

	m_dragImage = 0;
	m_hDropItem = 0;

	// Immidiatly create dialog.
	Create( IDD_DB_ENTITY,pParent );
}

CMaterialDialog::~CMaterialDialog()
{
}

void CMaterialDialog::DoDataExchange(CDataExchange* pDX)
{
	CBaseLibraryDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMaterialDialog, CBaseLibraryDialog)
	ON_COMMAND( ID_DB_ADD,OnAddItem )
	ON_COMMAND( ID_DB_PLAY,OnPlay )

	ON_COMMAND( ID_DB_MTL_DRAWSELECTED,OnDrawSelection )
	ON_COMMAND( ID_DB_MTL_DRAWBOX,OnDrawBox )
	ON_COMMAND( ID_DB_MTL_DRAWSPHERE,OnDrawSphere )
	ON_COMMAND( ID_DB_MTL_DRAWTEAPOT,OnDrawTeapot )
	ON_UPDATE_COMMAND_UI( ID_DB_PLAY,OnUpdatePlay )

	ON_COMMAND( ID_DB_SELECTASSIGNEDOBJECTS,OnSelectAssignedObjects )
	ON_COMMAND( ID_DB_MTL_ASSIGNTOSELECTION,OnAssignMaterialToSelection )
	ON_COMMAND( ID_DB_MTL_GETFROMSELECTION,OnGetMaterialFromSelection )
	ON_COMMAND( ID_DB_MTL_RESETMATERIAL,OnResetMaterialOnSelection )
	ON_UPDATE_COMMAND_UI( ID_DB_MTL_ASSIGNTOSELECTION,OnUpdateAssignMtlToSelection )
	ON_UPDATE_COMMAND_UI( ID_DB_SELECTASSIGNEDOBJECTS,OnUpdateMtlSelected )
	ON_UPDATE_COMMAND_UI( ID_DB_MTL_GETFROMSELECTION,OnUpdateObjectSelected )
	ON_UPDATE_COMMAND_UI( ID_DB_MTL_RESETMATERIAL,OnUpdateObjectSelected )

	ON_COMMAND( ID_DB_MTL_ADDSUBMTL,OnAddSubMtl )
	ON_COMMAND( ID_DB_MTL_DELSUBMTL,OnDelSubMtl )
	ON_UPDATE_COMMAND_UI( ID_DB_MTL_ADDSUBMTL,OnUpdateMtlSelected )
	ON_UPDATE_COMMAND_UI( ID_DB_MTL_DELSUBMTL,OnUpdateMtlSelected )

	ON_COMMAND( ID_DB_MTL_PICK,OnPickMtl )
	ON_UPDATE_COMMAND_UI( ID_DB_MTL_PICK,OnUpdatePickMtl )

	ON_COMMAND( ID_DB_MTL_GENCUBEMAP,OnGenCubemap )

	ON_NOTIFY(TVN_BEGINDRAG, IDC_MATERIAL_TREE, OnBeginDrag)
	ON_NOTIFY(NM_RCLICK , IDC_MATERIAL_TREE, OnNotifyMtlTreeRClick)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnDestroy()
{
	int temp;
	int HSplitter,VSplitter;
	m_wndHSplitter.GetRowInfo( 0,HSplitter,temp );
	m_wndVSplitter.GetColumnInfo( 0,VSplitter,temp );
	AfxGetApp()->WriteProfileInt("Dialogs\\Materials","HSplitter",HSplitter );
	AfxGetApp()->WriteProfileInt("Dialogs\\Materials","VSplitter",VSplitter );

	ReleaseGeometry();
	CBaseLibraryDialog::OnDestroy();
}

// CTVSelectKeyDialog message handlers
BOOL CMaterialDialog::OnInitDialog()
{
	CBaseLibraryDialog::OnInitDialog();

	InitToolbar();

	CRect rc;
	GetClientRect(rc);
	//int h2 = rc.Height()/2;
	int h2 = 200;

	int HSplitter = AfxGetApp()->GetProfileInt("Dialogs\\Materials","HSplitter",200 );
	int VSplitter = AfxGetApp()->GetProfileInt("Dialogs\\Materials","VSplitter",200 );

	m_wndVSplitter.CreateStatic( this,1,2,WS_CHILD|WS_VISIBLE );
	m_wndHSplitter.CreateStatic( &m_wndVSplitter,2,1,WS_CHILD|WS_VISIBLE );

	//m_imageList.Create(IDB_MATERIAL_TREE, 16, 1, RGB (255, 0, 255));
	CMFCUtils::LoadTrueColorImageList( m_imageList,IDB_MATERIAL_TREE,16,RGB(255,0,255) );

	// TreeCtrl must be already created.
	m_treeCtrl.SetParent( &m_wndVSplitter );
	m_treeCtrl.SetImageList(&m_imageList,TVSIL_NORMAL);

	m_previewCtrl.Create( &m_wndHSplitter,rc,WS_CHILD|WS_VISIBLE );
	m_previewCtrl.SetGrid(true);
	m_previewCtrl.EnableUpdate( true );

	m_propsCtrl.Create( WS_VISIBLE|WS_CHILD|WS_BORDER,rc,&m_wndHSplitter,2 );
	m_vars = gMatVars.CreateVars();
	m_propsCtrl.AddVarBlock( m_vars );
	m_propsCtrl.EnableWindow(FALSE);

	m_wndHSplitter.SetPane( 0,0,&m_previewCtrl,CSize(100,HSplitter) );
	m_wndHSplitter.SetPane( 1,0,&m_propsCtrl,CSize(100,HSplitter) );

	m_wndVSplitter.SetPane( 0,0,&m_treeCtrl,CSize(VSplitter,100) );
	m_wndVSplitter.SetPane( 0,1,&m_wndHSplitter,CSize(VSplitter,100) );

	RecalcLayout();

	ReloadLibs();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
UINT CMaterialDialog::GetDialogMenuID()
{
	return IDR_DB_ENTITY;
};

//////////////////////////////////////////////////////////////////////////
// Create the toolbar
void CMaterialDialog::InitToolbar()
{
	VERIFY( m_toolbar.CreateEx(this, TBSTYLE_FLAT|TBSTYLE_WRAPABLE,
		WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC) );
	VERIFY( m_toolbar.LoadToolBar24(IDR_DB_MATERIAL_BAR) );

	// Resize the toolbar
	CRect rc;
	GetClientRect(rc);
	m_toolbar.SetWindowPos(NULL, 0, 0, rc.right, 70, SWP_NOZORDER);
	CSize sz = m_toolbar.CalcDynamicLayout(TRUE,TRUE);
	m_toolbar.SetButtonStyle( m_toolbar.CommandToIndex(ID_DB_PLAY),TBBS_CHECKBOX );

	CBaseLibraryDialog::InitToolbar();
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnSize(UINT nType, int cx, int cy)
{
	// resize splitter window.
	if (m_wndVSplitter.m_hWnd)
	{
		CRect rc;
		GetClientRect(rc);
		m_wndVSplitter.MoveWindow(rc,FALSE);
	}
	CBaseLibraryDialog::OnSize(nType, cx, cy);
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnNewDocument()
{
	ReleaseGeometry();
	CBaseLibraryDialog::OnNewDocument();
};

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnLoadDocument()
{
	ReleaseGeometry();
	CBaseLibraryDialog::OnLoadDocument();
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnCloseDocument()
{
	ReleaseGeometry();
	CBaseLibraryDialog::OnCloseDocument();
}

//////////////////////////////////////////////////////////////////////////
HTREEITEM CMaterialDialog::InsertItemToTree( CBaseLibraryItem *pItem,HTREEITEM hParent )
{
	CMaterial *pMtl = (CMaterial*)pItem;
	if (pMtl->GetParent())
	{
		if (!hParent || hParent == TVI_ROOT || m_treeCtrl.GetItemData(hParent) == 0)
			return 0;
	}

	HTREEITEM hMtlItem = CBaseLibraryDialog::InsertItemToTree( pItem,hParent );

	for (int i = 0; i < pMtl->GetSubMaterialCount(); i++)
	{
		CMaterial *pSubMtl = pMtl->GetSubMaterial(i);
		CBaseLibraryDialog::InsertItemToTree( pSubMtl,hMtlItem );
	}
	return hMtlItem;
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnAddItem()
{
	if (!m_pLibrary)
		return;

	CStringGroupDlg dlg( _T("New Material Name"),this );
	dlg.SetGroup( m_selectedGroup );
	//dlg.SetString( entityClass );
	if (dlg.DoModal() != IDOK || dlg.GetString().IsEmpty())
	{
		return;
	}

	CString fullName = m_pItemManager->MakeFullItemName( m_pLibrary,dlg.GetGroup(),dlg.GetString() );
	if (m_pItemManager->FindItemByName( fullName ))
	{
		Warning( "Material with name %s already exist",(const char*)fullName );
		return;
	}

	CMaterial *mtl = (CMaterial*)m_pItemManager->CreateItem( m_pLibrary );
	
	// Make prototype name.
	SetItemName( mtl,dlg.GetGroup(),dlg.GetString() );
	mtl->Update();

	ReloadItems();
	SelectItem( mtl );
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::SetMaterialVars( CMaterial *mtl )
{
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::SelectItem( CBaseLibraryItem *item,bool bForceReload )
{
	bool bChanged = item != m_pCurrentItem || bForceReload;
	CBaseLibraryDialog::SelectItem( item,bForceReload );
	
	if (!bChanged)
		return;

	// Empty preview control.
	m_previewCtrl.SetEntity(0);
	m_pMatManager->SetCurrentMaterial( (CMaterial*)item );

	if (!item)
	{
		m_propsCtrl.EnableWindow(FALSE);
		return;
	}

	if (!m_pEntityRender)
	{
		LoadGeometry( m_geometryFile );
	}
	if (m_pEntityRender)
		m_previewCtrl.SetEntity( m_pEntityRender );

	m_propsCtrl.EnableWindow(TRUE);
	m_propsCtrl.EnableUpdateCallback(false);

	// Render preview geometry with current material
	CMaterial *mtl = GetSelectedMaterial();

	AssignMtlToGeometry();

	// Update variables.
	m_propsCtrl.EnableUpdateCallback(false);
	gMatVars.SetFromMaterial( mtl );
	m_propsCtrl.EnableUpdateCallback(true);

	//gMatVars.m_onSetCallback = functor(*this,OnUpdateProperties);
	//////////////////////////////////////////////////////////////////////////
	if (m_publicVarsItems)
	{
		m_propsCtrl.DeleteItem( m_publicVarsItems );
		m_publicVarsItems = 0;
	}

	m_publicVars = mtl->GetPublicVars();
	if (m_publicVars)
	{
		m_publicVarsItems = m_propsCtrl.AddVarBlock( m_publicVars,"Shader Params" );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Set Shader Gen Params.
	//////////////////////////////////////////////////////////////////////////
	if (m_shaderGenParamsVarsItem)
		m_propsCtrl.DeleteItem(m_shaderGenParamsVarsItem);
	m_shaderGenParamsVarsItem = 0;
	m_shaderGenParamsVars = mtl->GetShaderGenParamsVars();
	if (m_shaderGenParamsVars)
		m_shaderGenParamsVarsItem = m_propsCtrl.AddVarBlock( m_shaderGenParamsVars,"Shader Generation Params" );
	//////////////////////////////////////////////////////////////////////////

	m_propsCtrl.ExpandAllChilds( m_propsCtrl.GetRootItem(),false );

	m_propsCtrl.SetUpdateCallback( functor(*this, &CMaterialDialog::OnUpdateProperties) );
	m_propsCtrl.EnableUpdateCallback(true);
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::Update()
{
	if (!m_bRealtimePreviewUpdate)
		return;

	// Update preview control.
	if (m_pEntityRender)
	{
		m_previewCtrl.Update();
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnUpdateProperties( IVariable *var )
{
	CMaterial *mtl = GetSelectedMaterial();
	if (!mtl)
		return;

	bool bLightingOnChanged = &gMatVars.bLighting == var;
	bool bShaderChanged = (var == &gMatVars.shader);
	bool bShaderGenMaskChanged = false;

	//////////////////////////////////////////////////////////////////////////
	// Assign modified Shader Gen Params to shader.
	//////////////////////////////////////////////////////////////////////////
	if (m_shaderGenParamsVarsItem != NULL && m_shaderGenParamsVars != NULL && !bShaderChanged)
	{
		unsigned int mask = mtl->GetShaderGenMask();
		mtl->SetShaderGenParamsVars(m_shaderGenParamsVars);
		if (mask != mtl->GetShaderGenMask())
		{
			// If mask changed new shader have been created.
			bShaderGenMaskChanged = true;
		}
	}
	//////////////////////////////////////////////////////////////////////////

	// Assign new public vars to material.
	if (m_publicVarsItems != NULL && m_publicVars != NULL)
	{
		if (!bShaderChanged && !bShaderGenMaskChanged)
		{
			// No need to change public vars.
			mtl->SetPublicVars( m_publicVars );
		}
	}

	gMatVars.SetToMaterial( mtl );
	if (bShaderChanged || bLightingOnChanged || bShaderGenMaskChanged)
	{
		gMatVars.SetFromMaterial( mtl );
	}
	gMatVars.SetTextureNames( mtl );

	AssignMtlToGeometry();

	// When shader changed.
	if (bShaderChanged || bShaderGenMaskChanged)
	{
		// Delete old public params and add new ones. 
		if (m_publicVarsItems)
		{
			m_propsCtrl.DeleteItem( m_publicVarsItems );
			m_publicVarsItems = 0;
		}
		m_publicVars = mtl->GetPublicVars();
		if (m_publicVars)
		{
			m_publicVarsItems = m_propsCtrl.AddVarBlock( m_publicVars,"Shader Params" );
			m_propsCtrl.Expand( m_publicVarsItems,true );
		}

		//////////////////////////////////////////////////////////////////////////
		// Set Shader Gen Params.
		//////////////////////////////////////////////////////////////////////////
		if (!bShaderGenMaskChanged)
		{
			if (m_shaderGenParamsVarsItem)
				m_propsCtrl.DeleteItem(m_shaderGenParamsVarsItem);
			m_shaderGenParamsVarsItem = 0;
			m_shaderGenParamsVars = mtl->GetShaderGenParamsVars();
			if (m_shaderGenParamsVars)
				m_shaderGenParamsVarsItem = m_propsCtrl.AddVarBlock( m_shaderGenParamsVars,"Shader Generation Params" );
		}
		//////////////////////////////////////////////////////////////////////////
	}
	if (bLightingOnChanged || bShaderGenMaskChanged || bShaderChanged)
		m_propsCtrl.Invalidate();

	GetIEditor()->SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnPlay()
{
	m_bRealtimePreviewUpdate = !m_bRealtimePreviewUpdate;
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnUpdatePlay( CCmdUI* pCmdUI )
{
	if (m_bRealtimePreviewUpdate)
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

//////////////////////////////////////////////////////////////////////////
CMaterial* CMaterialDialog::GetSelectedMaterial()
{
	CBaseLibraryItem *pItem = m_pCurrentItem;
	return (CMaterial*)pItem;
}

//////////////////////////////////////////////////////////////////////////
IStatObj* CMaterialDialog::GetGeometryFromObject( CBaseObject *pObject )
{
	assert( pObject );

	if (pObject->IsKindOf(RUNTIME_CLASS(CBrushObject)))
	{
		CBrushObject *pBrushObj = (CBrushObject*)pObject;
		return pBrushObj->GetPrefabGeom();
	}
	if (pObject->IsKindOf(RUNTIME_CLASS(CEntity)))
	{
		CEntity *pEntityObj = (CEntity*)pObject;
		if (pEntityObj->GetIEntity())
		{
			IEntity *pGameEntity = pEntityObj->GetIEntity();
			for (int i = 0; i < pGameEntity->GetNumObjects(); i++)
			{
				IStatObj *pStatObj = pGameEntity->GetIStatObj(i);
				if (pStatObj)
					return pStatObj;
			}
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
ICryCharInstance* CMaterialDialog::GetCharacterFromObject( CBaseObject *pObject )
{
	if (pObject->IsKindOf(RUNTIME_CLASS(CEntity)))
	{
		CEntity *pEntityObj = (CEntity*)pObject;
		if (pEntityObj->GetIEntity())
		{
			IEntity *pGameEntity = pEntityObj->GetIEntity();
			ICryCharInstance *pCharacter = pGameEntity->GetCharInterface()->GetCharacter(0);
			if (pCharacter)
				return pCharacter;
			pCharacter = pGameEntity->GetCharInterface()->GetCharacter(1);
			if (pCharacter)
				return pCharacter;
			/*
			for (int i = 0; i < pGameEntity->GetNumObjects(); i++)
			{
				ICryCharInstance *pCharacter = pGameEntity->GetCharInterface()->GetCharacter(i);
				if (pCharacter)
					return pCharacter;
			}
			*/
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnDrawSelection()
{
	//if (m_drawType == DRAW_SELECTION)
		//return;
	m_drawType = DRAW_SELECTION;
	
	m_geometryFile = "";
	ReleaseGeometry();

	m_bOwnGeometry = false;
	CSelectionGroup *pSel = GetIEditor()->GetSelection();
	if (!pSel->IsEmpty())
	{
		IStatObj *pGeometry = GetGeometryFromObject( pSel->GetObject(0) );
		if (pGeometry)
		{
			LoadGeometry( pGeometry->GetFileName() );
		}
		AssignMtlToGeometry();
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnDrawBox()
{
	if (m_drawType == DRAW_BOX)
		return;
	m_drawType = DRAW_BOX;
	LoadGeometry( EDITOR_OBJECTS_PATH+"MtlBox.cgf" );
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnDrawSphere()
{
	if (m_drawType == DRAW_SPHERE)
		return;
	m_drawType = DRAW_SPHERE;
	LoadGeometry( EDITOR_OBJECTS_PATH+"MtlSphere.cgf" );
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnDrawTeapot()
{
	if (m_drawType == DRAW_TEAPOT)
		return;
	m_drawType = DRAW_TEAPOT;
	LoadGeometry( EDITOR_OBJECTS_PATH+"MtlTeapot.cgf" );
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::LoadGeometry( const CString &filename )
{
	m_geometryFile = filename;
	ReleaseGeometry();
	m_bOwnGeometry = true;
	m_pGeometry = GetIEditor()->Get3DEngine()->MakeObject( m_geometryFile );
	if (m_pGeometry)
	{
		m_pEntityRender = GetIEditor()->Get3DEngine()->CreateEntityRender();
		m_pEntityRender->SetEntityStatObj( 0,m_pGeometry );
		m_previewCtrl.SetEntity( m_pEntityRender );
		AssignMtlToGeometry();
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::ReleaseGeometry()
{
	m_previewCtrl.SetEntity(0);
	if (m_pEntityRender)
	{
		GetIEditor()->Get3DEngine()->DeleteEntityRender(m_pEntityRender);
		m_pEntityRender = 0;
	}
	if (m_pGeometry)
	{
		// Load test geometry.
		GetIEditor()->Get3DEngine()->ReleaseObject( m_pGeometry );
	}
	m_pGeometry = 0;
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::AssignMtlToGeometry()
{
	if (!m_pEntityRender)
		return;

	CMaterial *mtl = GetSelectedMaterial();
	if (!mtl)
		return;

	mtl->AssignToEntity( m_pEntityRender );
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnAssignMaterialToSelection()
{
	CMaterial *pMtl = GetSelectedMaterial();
	if (!pMtl)
		return;

	// Only assign most parent material.
	if (pMtl->GetParent())
		pMtl = pMtl->GetParent();

	CUndo undo( "Assign Material" );

	CSelectionGroup *pSel = GetIEditor()->GetSelection();
	if (!pSel->IsEmpty())
	{
		for (int i = 0; i < pSel->GetCount(); i++)
		{
			pSel->GetObject(i)->SetMaterial( pMtl );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnSelectAssignedObjects()
{
	CMaterial *pMtl = GetSelectedMaterial();
	if (!pMtl)
		return;

	CBaseObjectsArray objects;
	GetIEditor()->GetObjectManager()->GetObjects( objects );
	for (int i = 0; i < objects.size(); i++)
	{
		CBaseObject *pObject = objects[i];
		if (pObject->GetMaterial() != pMtl)
			continue;
		if (pObject->IsHidden() || pObject->IsFrozen())
			continue;
		GetIEditor()->GetObjectManager()->SelectObject( pObject );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnResetMaterialOnSelection()
{
	CUndo undo( "Reset Material" );

	CSelectionGroup *pSel = GetIEditor()->GetSelection();
	if (!pSel->IsEmpty())
	{
		for (int i = 0; i < pSel->GetCount(); i++)
		{
			pSel->GetObject(i)->SetMaterial( 0 );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnGetMaterialFromSelection()
{
	if (!m_pLibrary)
		return;

	CSelectionGroup *pSel = GetIEditor()->GetSelection();
	if (pSel->IsEmpty())
		return;

	for (int i = 0; i < pSel->GetCount(); i++)
	{
		CMaterial *pMtl = pSel->GetObject(i)->GetMaterial();
		if (pMtl)
		{
			SelectItem( pMtl );
			return;
		}
	}

	IStatObj *pGeometry = GetGeometryFromObject( pSel->GetObject(0) );
	ICryCharInstance* pCharacter = GetCharacterFromObject( pSel->GetObject(0) );

	if (!pGeometry && !pCharacter)
		return;
	
	// If nothing was selected.
	if (IDNO == AfxMessageBox( _T("Selected Object does not have Material\r\nDo you want to create Material for it?"),MB_YESNO|MB_APPLMODAL|MB_ICONQUESTION ))
	{
		return;
	}

  CStringGroupDlg dlg( _T("New Material Name"),this );
	dlg.SetGroup( m_selectedGroup );
	
	CString uniqName;
	if (pCharacter)
	{
		uniqName = m_pItemManager->MakeUniqItemName(Path::GetFileName(pCharacter->GetModel()->GetFileName()));
	}
	else if (pGeometry)
	{
		uniqName = m_pItemManager->MakeUniqItemName(Path::GetFileName(pGeometry->GetFileName()));
	}
	dlg.SetString( uniqName );
	if (dlg.DoModal() != IDOK || dlg.GetString().IsEmpty())
	{
		return;
	}

	// Make new material from this object.
	CMaterial *mtl = (CMaterial*)m_pItemManager->CreateItem( m_pLibrary );
	// Make prototype name.
	SetItemName( mtl,dlg.GetGroup(),dlg.GetString() );
	if (pCharacter)
	{
		mtl->AssignFromGeometry( pCharacter );
	}
	else if (pGeometry)
	{
		mtl->AssignFromGeometry( pGeometry );
	}
	mtl->Update();
	pSel->GetObject(0)->SetMaterial( mtl );
	ReloadItems();
	SelectItem( mtl );
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnAddSubMtl()
{
	CMaterial *pMtl = GetSelectedMaterial();
	if (!pMtl)
		return;

	if (pMtl->GetParent())
		pMtl = pMtl->GetParent();

	CUndo undo( "Add Sub Material" );

	CString mtlName;
	mtlName.Format( "[%d]",pMtl->GetSubMaterialCount()+1 );
	//CMaterial *pSubMtl = m_pItemManager->CreateMaterial( m_selectedLib );
	CMaterial *pSubMtl = new CMaterial;
	pSubMtl->GenerateId();
	pMtl->AddSubMaterial( pSubMtl );
	pSubMtl->SetName( mtlName );
	pSubMtl->Update();

	ReloadItems();
	SelectItem( pSubMtl );
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnDelSubMtl()
{
	CMaterial *pSubMtl = GetSelectedMaterial();
	if (!pSubMtl)
		return;

	CUndo undo( "Remove Sub Material" );
	CMaterial *pMtl = pSubMtl->GetParent();
	if (pMtl)
	{
		pMtl->RemoveSubMaterial(pSubMtl);
		m_pItemManager->DeleteItem( pSubMtl );

		ReloadItems();
		SelectItem( pMtl );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::DeleteItem( CBaseLibraryItem *pItem )
{
	CMaterial* pMtl = (CMaterial*)pItem;
	CMaterial *pParentMtl = pMtl->GetParent();
	if (pParentMtl)
	{
		CUndo undo( "Remove Sub Material" );
		pParentMtl->RemoveSubMaterial(pMtl);
		m_pItemManager->DeleteItem( pMtl );
	}
	else
	{
		CUndo undo( "Remove Material" );
		m_pItemManager->DeleteItem( pItem );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnUpdateMtlSelected( CCmdUI* pCmdUI )
{
	if (GetSelectedMaterial())
	{
		pCmdUI->Enable( TRUE );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnUpdateAssignMtlToSelection( CCmdUI* pCmdUI )
{
	if (GetSelectedMaterial() && !GetIEditor()->GetSelection()->IsEmpty())
	{
		pCmdUI->Enable( TRUE );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnUpdateObjectSelected( CCmdUI* pCmdUI )
{
	if (!GetIEditor()->GetSelection()->IsEmpty())
	{
		pCmdUI->Enable( TRUE );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	HTREEITEM hItem = pNMTreeView->itemNew.hItem;

	CMaterial* pMtl = (CMaterial*)m_treeCtrl.GetItemData(hItem);
	if (!pMtl)
		return;

	m_pDraggedMtl = pMtl;

	m_treeCtrl.Select( hItem,TVGN_CARET );

	m_hDropItem = 0;
	m_dragImage = m_treeCtrl.CreateDragImage( hItem );
	if (m_dragImage)
	{
		m_hDraggedItem = hItem;
		m_hDropItem = hItem;
		m_dragImage->BeginDrag(0, CPoint(-10, -10));

		CRect rc;
		AfxGetMainWnd()->GetWindowRect( rc );
		
		CPoint p = pNMTreeView->ptDrag;
		ClientToScreen( &p );
		p.x -= rc.left;
		p.y -= rc.top;
		
		m_dragImage->DragEnter( AfxGetMainWnd(),p );
		SetCapture();
		GetIEditor()->EnableUpdate( false );
	}
	
	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_dragImage)
	{
		CPoint p;

		p = point;
		ClientToScreen( &p );
		m_treeCtrl.ScreenToClient( &p );

		TVHITTESTINFO hit;
		ZeroStruct(hit);
		hit.pt = p;
		HTREEITEM hHitItem = m_treeCtrl.HitTest( &hit );
		if (hHitItem)
		{
			if (m_hDropItem != hHitItem)
			{
				if (m_hDropItem)
					m_treeCtrl.SetItem( m_hDropItem,TVIF_STATE,0,0,0,0,TVIS_DROPHILITED,0 );
				// Set state of this item to drop target.
				m_treeCtrl.SetItem( hHitItem,TVIF_STATE,0,0,0,TVIS_DROPHILITED,TVIS_DROPHILITED,0 );
				m_hDropItem = hHitItem;
				m_treeCtrl.Invalidate();
			}
		}

		CRect rc;
		AfxGetMainWnd()->GetWindowRect( rc );
		p = point;
		ClientToScreen( &p );
		p.x -= rc.left;
		p.y -= rc.top;
		m_dragImage->DragMove( p );

		SetCursor( m_hCursorDefault );
		// Check if can drop here.
		{
			CPoint p;
			GetCursorPos( &p );
			CViewport* viewport = GetIEditor()->GetViewManager()->GetViewportAtPoint( p );
			if (viewport)
			{
				CPoint vp = p;
				viewport->ScreenToClient(&vp);
				ObjectHitInfo hit( viewport,vp );
				if (viewport->HitTest( vp,hit,0 ))
				{
					if (hit.object)
					{
						SetCursor( m_hCursorReplace );
					}
				}
			}
		}
	}

	CBaseLibraryDialog::OnMouseMove(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	//CXTResizeDialog::OnLButtonUp(nFlags, point);

	if (m_hDropItem)
	{
		m_treeCtrl.SetItem( m_hDropItem,TVIF_STATE,0,0,0,0,TVIS_DROPHILITED,0 );
		m_hDropItem = 0;
	}

	if (m_dragImage)
	{
		CPoint p;
		GetCursorPos( &p );

		GetIEditor()->EnableUpdate( true );

		m_dragImage->DragLeave( AfxGetMainWnd() );
		m_dragImage->EndDrag();
		delete m_dragImage;
		m_dragImage = 0;
		ReleaseCapture();

		CPoint treepoint = p;
		m_treeCtrl.ScreenToClient( &treepoint );

		TVHITTESTINFO hit;
		ZeroStruct(hit);
		hit.pt = treepoint;
		HTREEITEM hHitItem = m_treeCtrl.HitTest( &hit );
		if (hHitItem)
		{
			DropToItem( hHitItem,m_hDraggedItem,m_pDraggedMtl );
			m_hDraggedItem = 0;
			m_pDraggedMtl = 0;
			return;
		}

		CWnd *wnd = WindowFromPoint( p );

		CUndo undo( "Assign Material" );

		CViewport* viewport = GetIEditor()->GetViewManager()->GetViewportAtPoint( p );
		if (viewport)
		{
			CPoint vp = p;
			viewport->ScreenToClient(&vp);
			// Drag and drop into one of views.
			// Start object creation.
			ObjectHitInfo hit( viewport,vp );
			if (viewport->HitTest( vp,hit,0 ))
			{
				if (hit.object)
				{
					// Only parent can be assigned.
					CMaterial *pMtl = m_pDraggedMtl;
					if (pMtl->GetParent())
					{
						pMtl = pMtl->GetParent();
					}
					hit.object->SetMaterial(pMtl);
				}
			}
		}
		m_pDraggedMtl = 0;
	}
	m_hDraggedItem = 0;

	CBaseLibraryDialog::OnLButtonUp(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnNotifyMtlTreeRClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Show helper menu.
	CPoint point;

	CMaterial *pMtl = 0;

	// Find node under mouse.
	GetCursorPos( &point );
	m_treeCtrl.ScreenToClient( &point );
	// Select the item that is at the point myPoint.
	UINT uFlags;
	HTREEITEM hItem = m_treeCtrl.HitTest(point,&uFlags);
	if ((hItem != NULL) && (TVHT_ONITEM & uFlags))
	{
		pMtl = (CMaterial*)m_treeCtrl.GetItemData(hItem);
	}

	if (!pMtl)
		return;

	SelectItem( pMtl );

	// Create pop up menu.
	CMenu menu;
	menu.CreatePopupMenu();
	
	if (pMtl)
	{
		CClipboard clipboard;
		bool bNoPaste = clipboard.IsEmpty();
		int pasteFlags = 0;
		if (bNoPaste)
			pasteFlags |= MF_GRAYED;

		menu.AppendMenu( MF_STRING,ID_DB_CUT,"Cut" );
		menu.AppendMenu( MF_STRING,ID_DB_COPY,"Copy" );
		menu.AppendMenu( MF_STRING|pasteFlags,ID_DB_PASTE,"Paste" );
		menu.AppendMenu( MF_STRING,ID_DB_CLONE,"Clone" ); 
		menu.AppendMenu( MF_SEPARATOR,0,"" );
		menu.AppendMenu( MF_STRING,ID_DB_RENAME,"Rename" );
		menu.AppendMenu( MF_STRING,ID_DB_REMOVE,"Delete" );
		menu.AppendMenu( MF_SEPARATOR,0,"" );
		menu.AppendMenu( MF_STRING,ID_DB_MTL_ASSIGNTOSELECTION,"Assign to Selected Objects" );
		menu.AppendMenu( MF_STRING,ID_DB_SELECTASSIGNEDOBJECTS,"Select Assigned Objects" );
		menu.AppendMenu( MF_STRING,ID_DB_MTL_ADDSUBMTL,"Add Sub Material" );
	}

	GetCursorPos( &point );
	menu.TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,point.y,this );
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnPickMtl()
{
	if (!CMtlPickCallback::IsActive())
		GetIEditor()->PickObject( new CMtlPickCallback,0,"Pick Object to Select Material" );
	else
		GetIEditor()->CancelPick();
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnUpdatePickMtl( CCmdUI* pCmdUI )
{
	if (CMtlPickCallback::IsActive())
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnCopy()
{
	CMaterial *pMtl = GetSelectedMaterial();
	if (pMtl)
	{
		CClipboard clipboard;
		XmlNodeRef node = new CXmlNode( "Material" );
		CBaseLibraryItem::SerializeContext ctx( node,false );
		ctx.bCopyPaste = true;
		pMtl->Serialize( ctx );
		clipboard.Put( node );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnPaste()
{
	if (!m_pLibrary)
		return;

	CClipboard clipboard;
	if (clipboard.IsEmpty())
		return;
	XmlNodeRef node = clipboard.Get();
	if (!node)
		return;

	if (strcmp(node->getTag(),"Material") == 0)
	{
		CMaterial *pParentMtl = 0;
		CMaterial *pSelMtl = GetSelectedMaterial();
		if (pSelMtl)
		{
			pParentMtl = pSelMtl->GetParent();
		}
		// This is material node.
		CBaseLibrary *pLib = m_pLibrary;
		CMaterial *pMtl = m_pMatManager->LoadMaterial( (CMaterialLibrary*)pLib,node,true );
		if (pMtl)
		{
			if (pParentMtl)
			{
				pParentMtl->AddSubMaterial( pMtl );
			}
			ReloadItems();
			SelectItem(pMtl);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::OnGenCubemap()
{
	CMaterial *pMtl = GetSelectedMaterial();
	if (!pMtl)
		return;
	
	CBaseObject *pObject = GetIEditor()->GetSelectedObject();
	if (!pObject)
	{
		AfxMessageBox( "Select One Object to Generate Cubemap",MB_OK|MB_APPLMODAL|MB_ICONWARNING );
		return;
	}
	CString filename;
	if (!CFileUtil::SelectSaveFile( "*.*","dds","Textures",filename ))
		return;
	filename = GetIEditor()->GetRelativePath(filename);
	if (filename.IsEmpty())
	{
		AfxMessageBox( _T("Texture Must be inside MasterCD folder!"),MB_OK|MB_APPLMODAL|MB_ICONWARNING );
		return;
	}

	CNumberDlg dlg( this,256,"Enter Cubemap Resolution" );
	dlg.SetInteger( true );
	if (dlg.DoModal() != IDOK)
		return;

	int res = 1;
	int size = dlg.GetValue();
	// Make size power of 2.
	for (int i = 0; i < 16; i++)
	{
		if (res*2 > size)
			break;
		res *= 2;
	}
	if (res > 4096)
	{
		AfxMessageBox( "Bad texture resolution.\nMust be power of 2 and less or equal to 4096",MB_OK|MB_APPLMODAL|MB_ICONWARNING );
		return;
	}
	// Hide object before Cubemap generation.
	pObject->SetHidden( true );
	GetIEditor()->GetRenderer()->EF_ScanEnvironmentCM( filename,res,pObject->GetWorldPos() );
	pObject->SetHidden( false );
	CString texname = Path::GetFileName(filename);
	CString path = Path::GetPath(filename);
	texname = Path::Make( path,texname+"_posx.jpg" );
	// Assign this texname to current material.

	pMtl->GetShaderResources().m_Textures[EFTT_CUBEMAP].m_Name = texname;
	pMtl->Update();
	// Update variables.
	m_propsCtrl.EnableUpdateCallback(false);
	gMatVars.SetFromMaterial( pMtl );
	m_propsCtrl.EnableUpdateCallback(true);
}

//////////////////////////////////////////////////////////////////////////
void CMaterialDialog::DropToItem( HTREEITEM hItem,HTREEITEM hSrcItem,CMaterial *pMtl )
{
	pMtl->GetLibrary()->SetModified();

	CMaterial* pTargetMtl = (CMaterial*)m_treeCtrl.GetItemData(hItem);
	if (!pTargetMtl)
	{
		// Only root materials can be inserted at group level.
		if (pMtl->GetParent())
			return;

		// Only move material to different group.
		CString groupName = m_treeCtrl.GetItemText(hItem);
		SetItemName( pMtl,groupName,pMtl->GetShortName() );
		//ReloadItems();
		//SelectItem( pMtl );
		m_treeCtrl.DeleteItem( hSrcItem );
		InsertItemToTree( pMtl,hItem );
		return;
	}
	// Ignore itself.
	if (pTargetMtl == pMtl)
		return;

	CMaterial *pTargetParent = pTargetMtl->GetParent();
	CMaterial *pSourceParent = pMtl->GetParent();
	if (pTargetParent && pSourceParent && pTargetParent == pSourceParent)
	{
		int slot = pTargetParent->FindSubMaterial( pTargetMtl );
		assert(slot>=0);
		if (slot < 0)
			return;

		TSmartPtr<CMaterial> pSourceMtl = pMtl; // Make usre its not release while we remove and add it back.

		pTargetParent->RemoveSubMaterial( pMtl );
		pTargetParent->InsertSubMaterial( slot,pMtl );
		
		ReloadItems();
		SelectItem( pMtl );
	}
	if (pTargetParent == pMtl || pSourceParent == pTargetMtl)
	{
		// Swap contents of theose 2 materials...
		pMtl->SwapContent( pTargetMtl );
		SelectItem( pMtl,true );
	}
}