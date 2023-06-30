////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   MatMan.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Material Manager Implementation
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "3dEngine.h"
#include "objman.h"
#include "irenderer.h"

CMatMan::CMatMan( )
{
}
CMatMan::~CMatMan()
{
	int nNotUsed=0, nNotUsedParents=0;

  for (MtlSet::iterator it = m_mtlSet.begin(); it != m_mtlSet.end(); ++it)
  {
    IMatInfo *pMtl = *it;
    SShaderItem Sh = pMtl->GetShaderItem();
		if(Sh.m_pShader)
		{
			Sh.m_pShader->Release();
			Sh.m_pShader = 0;
		}
    if(Sh.m_pShaderResources)
		{
			Sh.m_pShaderResources->Release();
			Sh.m_pShaderResources = 0;
		}
		pMtl->SetShaderItem( Sh );

		if(!(pMtl->GetFlags()&MIF_CHILD))
		if(!(pMtl->GetFlags()&MIF_WASUSED))
		{
			GetLog()->Log("Warning: CMatMan::~CMatMan: Material was loaded but never used: %s", pMtl->GetName());
			nNotUsed += (pMtl->GetSubMtlCount()+1);
			nNotUsedParents++;
		}
		if (pMtl->GetNumRefs() > 1)
		{
			//
			GetLog()->Log("Warning: CMatMan::~CMatMan: Material %s is being referenced", pMtl->GetName());
		}
  }

	if(nNotUsed)
		GetLog()->Log("Warning: CMatMan::~CMatMan: %d(%d) of %d materials was not used in level", 
		nNotUsedParents, nNotUsed, m_mtlSet.size());
}

IMatInfo * CMatMan::CreateMatInfo( const char *sMtlName )
{
	IMatInfo *pMatInfo = new CMatInfo;
	m_mtlSet.insert( pMatInfo );
	if (sMtlName)
	{
		pMatInfo->SetName( sMtlName );
		m_mtlNameMap[sMtlName] = pMatInfo;
	}
	return pMatInfo;
}

void CMatMan::DeleteMatInfo(IMatInfo * pMatInfo)
{
	// Delete sub materials if present.
	/* Not needed for now..
	if (pMatInfo)
	{
		CMatInfo *mtl = (CMatInfo*)pMatInfo;
		if (mtl->pSubMtls)
		{
			// Delete all sub materials.
			for (int i = 0; i < mtl->pSubMtls->size(); i++)
			{
				DeleteMatInfo( mtl->pSubMtls[i] );
			}
		}
	}
	*/
	assert( pMatInfo );
	pMatInfo->SetFlags( pMatInfo->GetFlags()|MIF_INVALID );
	m_mtlNameMap.erase( pMatInfo->GetName() );
	m_mtlSet.erase( pMatInfo );
}

//////////////////////////////////////////////////////////////////////////
void CMatMan::RenameMatInfo( IMatInfo *pMtl,const char *sNewName )
{
	assert( pMtl );
	m_mtlNameMap.erase( pMtl->GetName() );
	pMtl->SetName( sNewName );
	m_mtlNameMap[sNewName] = pMtl;
}

//////////////////////////////////////////////////////////////////////////
IMatInfo* CMatMan::FindMatInfo( const char *sMtlName ) const
{
	IMatInfo *pMtl = stl::find_in_map( m_mtlNameMap,sMtlName, (IMatInfo *)NULL );
	return pMtl;
}

//////////////////////////////////////////////////////////////////////////
void CMatMan::LoadMaterialsLibrary( const char *sMtlFile,XmlNodeRef &levelDataRoot )
{
	GetLog()->UpdateLoadingScreen("\003Loading materials ...");

	// load environment settings
	if (!levelDataRoot)
		return;
	
	XmlNodeRef mtlLibs = levelDataRoot->findChild( "MaterialsLibrary" );
	if (mtlLibs)
	{
		// Enmerate material libraries.
		for (int i = 0; i < mtlLibs->getChildCount(); i++)
		{
			XmlNodeRef mtlLib = mtlLibs->getChild(i);
			XmlString libraryName = mtlLib->getAttr( "Name" );
			for (int j =0; j < mtlLib->getChildCount(); j++)
			{
				XmlNodeRef mtlNode = mtlLib->getChild(j);
				LoadMaterial( mtlNode,libraryName.c_str(),0 );
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Loading from external materials.xml file.
	//////////////////////////////////////////////////////////////////////////
	mtlLibs = GetSystem()->LoadXmlFile( sMtlFile );
	if (mtlLibs)
	{
		// Enmerate material libraries.
		for (int i = 0; i < mtlLibs->getChildCount(); i++)
		{
			XmlNodeRef mtlLib = mtlLibs->getChild(i);
			if (!mtlLib->isTag("Library"))
				continue;
			XmlString libraryName = mtlLib->getAttr( "Name" );
			for (int j =0; j < mtlLib->getChildCount(); j++)
			{
				XmlNodeRef mtlNode = mtlLib->getChild(j);
				LoadMaterial( mtlNode,libraryName.c_str(),0 );
			}
		}
	}

/*
	// Load level data xml.
	if (pDoc->load(Get3DEngine()->GetFilePath("LevelData.xml")))
	{
		XDOM::IXMLDOMNodeListPtr pLibsNode = pDoc->getElementsByTagName("MaterialsLibrary");
		if (pLibsNode)
		{
			pLibsNode->reset();
			XDOM::IXMLDOMNodePtr pLibNode;
			while (pLibNode = pLibsNode->nextNode())
			{
				// For each library.
				const char *sLibraryName = "";
				XDOM::IXMLDOMNodePtr pName = pLibNode->getAttribute("Name");
				if (pName)
					sLibraryName = pName->getText();
				/*
				// Enumerate library.
				XDOM::IXMLDOMNodeListPtr pMtlsListNode = pLibNode->getElementsByTagName("Material");
				XDOM::IXMLDOMNodePtr pMtlNode;
				pMtlsListNode->reset();
				while (pMtlNode = pMtlListNode->nextNode())
				{
					// For each material.
					LoadMaterial( pMtlNode,sLibraryName,0 );
				}
	
			}
		}
	}
				*/

  GetLog()->UpdateLoadingScreenPlus("\003 %d mats loaded", m_mtlSet.size());
}

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
inline CFColor ToCFColor( const Vec3 &col ) { return CFColor(col); }

//////////////////////////////////////////////////////////////////////////
IMatInfo* CMatMan::LoadMaterial( XmlNodeRef node,const char *sLibraryName,IMatInfo* pParent )
{
	XmlString name,mtlName,shaderName,texmap,file;
	int mtlFlags = 0;
	unsigned int nShaderGenMask = 0;
	SInputShaderResources sr;
	SLightMaterial lm;

	// Make new mat info.
	mtlName = node->getAttr( "Name" );

	if (pParent)
	{
		name = XmlString(pParent->GetName());
		name += mtlName;
	}
	else
	{
		// Combine library name with item name to form a fully specified material name.
		name = sLibraryName;
		name += ".";
		name += mtlName;
	}

	IMatInfo* pMtl = CreateMatInfo( name.c_str() );
	if (pParent)
	{
		// Add as sub material if have parent.
		pParent->AddSubMtl(pMtl);
	}
	
	// Loading from Material XML node.
	shaderName = node->getAttr( "Shader" );
	if (shaderName.empty())
	{
		// Replace empty shader with NoDraw shader.
		shaderName = "NoDraw";
	}
	node->getAttr( "MtlFlags",mtlFlags );
	node->getAttr( "GenMask",nShaderGenMask );

	// Load lighting data.
	Vec3 ambient,diffuse,specular,emissive;
	node->getAttr( "Ambient",ambient );
	node->getAttr( "Diffuse",diffuse );
	node->getAttr( "Specular",specular );
	node->getAttr( "Emissive",emissive );
	node->getAttr( "Shininess",lm.Front.m_SpecShininess );
	lm.Front.m_Ambient = ToCFColor(ambient);
	lm.Front.m_Diffuse = ToCFColor(diffuse);
	lm.Front.m_Specular = ToCFColor(specular);
	lm.Front.m_Emission = ToCFColor(emissive);

	node->getAttr( "Opacity",sr.m_Opacity );
	node->getAttr( "AlphaTest",sr.m_AlphaRef );

	// Load material textures.
	XmlNodeRef texturesNode = node->findChild( "Textures" );
	if (texturesNode)
	{
		for (int i = 0; i < texturesNode->getChildCount(); i++)
		{
			texmap = "";
			XmlNodeRef texNode = texturesNode->getChild(i);
			texmap = texNode->getAttr( "Map" );

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
			file = texNode->getAttr( "File" );

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
				modNode->getAttr( "TexType",sr.m_Textures[texId].m_TU.m_eTexType );

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

	// Load sub materials.
	XmlNodeRef childsNode = node->findChild( "SubMaterials" );
	if (childsNode)
	{
		for (int i = 0; i < childsNode->getChildCount(); i++)
		{
			XmlNodeRef mtlNode = childsNode->getChild(i);
			LoadMaterial( mtlNode,sLibraryName,pMtl );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Load public parameters.
	//////////////////////////////////////////////////////////////////////////
	XmlNodeRef publicsNode = node->findChild( "PublicParams" );

	//////////////////////////////////////////////////////////////////////////
	// Reload shader item with new resources and shader.
	//////////////////////////////////////////////////////////////////////////
	LoadMaterialShader( pMtl,shaderName.c_str(),mtlFlags,nShaderGenMask,sr,lm,publicsNode );

	static int nTic=0;
	if((++nTic%10)==0)
		GetConsole()->TickProgressBar();

	return pMtl;
}

//////////////////////////////////////////////////////////////////////////
// Material flags from Editor.
//////////////////////////////////////////////////////////////////////////
enum EMtlFlagsFromXml
{
	MF_WIRE					= 0x0001,
	MF_2SIDED				= 0x0002,
	MF_ADDITIVE			= 0x0004,
	MF_ADDITIVE_DECAL	= 0x0008,
	MF_LIGHTING			= 0x0010,
	MF_NOSHADOW			= 0x0020,
};

//////////////////////////////////////////////////////////////////////////
bool CMatMan::LoadMaterialShader( IMatInfo *pMtl,const char *sShader,int mtlFlags,unsigned int nShaderGenMask,SInputShaderResources &sr,SLightMaterial &lm,XmlNodeRef &publicsNode )
{
	// Mark material invalid by default.
	pMtl->SetFlags( pMtl->GetFlags()|MIF_INVALID );

	if (mtlFlags & MF_LIGHTING)
		sr.m_LMaterial = &lm;
	else
		sr.m_LMaterial = 0;

	if (mtlFlags & MF_NOSHADOW)
		pMtl->SetFlags( pMtl->GetFlags()|MIF_NOCASTSHADOWS );

	sr.m_ResFlags = 0;
	if (mtlFlags & MF_WIRE)
		sr.m_ResFlags |= MTLFLAG_WIRE;
	if (mtlFlags & MF_2SIDED)
		sr.m_ResFlags |= MTLFLAG_2SIDED;
	if (mtlFlags & MF_ADDITIVE)
		sr.m_ResFlags |= MTLFLAG_ADDITIVE;
	if (mtlFlags & MF_ADDITIVE_DECAL)
		sr.m_ResFlags |= MTLFLAG_ADDITIVEDECAL;

	/*
	sr.m_ShaderParams.clear();
	for (int i = 0; i < m_shaderParams.size(); i++)
	{
		sr.m_ShaderParams.push_back( m_shaderParams[i] );
	}
	*/
	IShader *pTemplShader = 0;
	// If we have public parameters, first load shader and parse public parameters.
	if (publicsNode)
	{
		pTemplShader = GetSystem()->GetIRenderer()->EF_LoadShader( sShader,eSH_Misc,0,nShaderGenMask );
		TArray<SShaderParam> &params = pTemplShader->GetPublicParams();
		if (!params.empty())
		{
			// Parse public parameters, and assign them to source shader resources.
			ParsePublicParams( params,publicsNode );
			sr.m_ShaderParams.Reserve( params.size() );
			for (unsigned int i = 0; i < params.size(); i++)
			{
				sr.m_ShaderParams.push_back(params[i]);
			}
		}
	}

	// Shader container name does not support '.', replace it with different character.
	char sContainerName[1024];
	strncpy( sContainerName,pMtl->GetName(),sizeof(sContainerName) );
	sContainerName[sizeof(sContainerName)-1] = 0;
	std::replace( sContainerName,sContainerName+strlen(sContainerName),'.','_' );


  int nQBM = 0;
  ICVar * pIVar = GetConsole()->GetCVar("r_Quality_BumpMapping");
  if(pIVar)
    nQBM = pIVar->GetIVal();

  if ((GetSystem()->GetIRenderer()->GetFeatures() & RFT_HW_MASK) == RFT_HW_GF2 || nQBM == 0)
  {
    SLightMaterial mtl;
    sr.m_LMaterial = &mtl;
  }

	SShaderItem shaderItem = GetSystem()->GetIRenderer()->EF_LoadShaderItem( "MaterialContainer",eSH_Misc,true,sShader,0,&sr,nShaderGenMask );
	if (!shaderItem.m_pShader)
	{
		Warning( 0,0,"Failed to load shader %s in Material %s",sShader,sContainerName );
		return false;
	}
	pMtl->SetShaderItem( shaderItem );
	// If material shader was loaded successfully mark it as valid again.
	pMtl->SetFlags( pMtl->GetFlags()&(~MIF_INVALID) );

	if (pTemplShader)
	{
		// Release templ shader reference.
		pTemplShader->Release();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CMatMan::ParsePublicParams( TArray<SShaderParam> &params,XmlNodeRef paramsNode )
{
	// Load shader params from xml node.
	// Initialize shader params if thier number is changed.
	if (params.empty())
		return;

	for (unsigned int i = 0; i < params.size(); i++)
	{
		SShaderParam *pParam = &params[i];
		switch (pParam->m_Type)
		{
		case eType_BYTE:
			paramsNode->getAttr( pParam->m_Name,pParam->m_Value.m_Byte );
			break;
		case eType_SHORT:
			paramsNode->getAttr( pParam->m_Name,pParam->m_Value.m_Short );
			break;
		case eType_INT:
			paramsNode->getAttr( pParam->m_Name,pParam->m_Value.m_Int );
			break;
		case eType_FLOAT:
			paramsNode->getAttr( pParam->m_Name,pParam->m_Value.m_Float );
			break;
		//case eType_STRING:
			//paramsNode->getAttr( pParam->m_Name,pParam->m_Value.m_String );
			//break;
		case eType_FCOLOR:
			{
				Vec3 v(pParam->m_Value.m_Color[0],pParam->m_Value.m_Color[1],pParam->m_Value.m_Color[2]);
				paramsNode->getAttr( pParam->m_Name,v );
				pParam->m_Value.m_Color[0] = v.x;
				pParam->m_Value.m_Color[1] = v.y;
				pParam->m_Value.m_Color[2] = v.z;
			}
			break;
		case eType_VECTOR:
			{
				Vec3 v(pParam->m_Value.m_Vector[0],pParam->m_Value.m_Vector[1],pParam->m_Value.m_Vector[2]);
				paramsNode->getAttr( pParam->m_Name,v );
				pParam->m_Value.m_Vector[0] = v.x;
				pParam->m_Value.m_Vector[1] = v.y;
				pParam->m_Value.m_Vector[2] = v.z;
			}
			break;
		}
	}
}