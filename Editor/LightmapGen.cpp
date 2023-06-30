////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   lightmapgen.cpp
//  Version:     v1.00
//  Created:     18/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"

#include "LightmapGen.h"
#include "LightmapGen.h"

#include "LightmapCompiler/IndoorLightPatches.h"

#include <I3DEngine.h>
#include "IEntitySystem.h"
#include "ITimer.h"

#include "Objects\ObjectManager.h"
#include "Objects\BrushObject.h"

#include "GameExporter.h"
#include "GameEngine.h"

#include "Util\PakFile.h"


#include <list2.h>
#include <ICryPak.h>

LMGenParam CLightmapGen::m_sParam;

//////////////////////////////////////////////////////////////////////////
class CEntityRenderPred : public std::binary_function<std::pair<IEntityRender*, CBrushObject*>, std::pair<IEntityRender*, CBrushObject*>, bool>
{
public:
	bool operator () ( const std::pair<IEntityRender*, CBrushObject*>& rpIEtyRnd1,const std::pair<IEntityRender*, CBrushObject*>& rpIEtyRnd2 )
	{ 
		IEntityRender* pIEtyRnd1 = rpIEtyRnd1.first;
		IEntityRender* pIEtyRnd2 = rpIEtyRnd2.first;

		if((DWORD_PTR) pIEtyRnd1->GetEntityVisArea() < (DWORD_PTR) pIEtyRnd2->GetEntityVisArea()) return(true);
		if((DWORD_PTR) pIEtyRnd1->GetEntityVisArea() > (DWORD_PTR) pIEtyRnd2->GetEntityVisArea()) return(false);

		Vec3 vMin1,vMin2,vMax1,vMax2;

		pIEtyRnd1->GetBBox(vMin1,vMax1);
		pIEtyRnd2->GetBBox(vMin2,vMax2);

		if(vMin1.x<vMin2.x) return(true);
		if(vMin1.x>vMin2.x) return(false);

		if(vMin1.y<vMin2.y) return(true);
		if(vMin1.y>vMin2.y) return(false);
		
		if(vMin1.z<vMin2.z) return(true);
		if(vMin1.z>vMin2.z) return(false);

		return(false);
	};
};

const bool CLightmapGen::Generate( ISystem *pSystem, std::vector<std::pair<IEntityRender*, CBrushObject*> >& nodes, 
	ICompilerProgress *pICompilerProgress, const ELMMode Mode, const std::set<const CBrushObject*>& vSelectionIndices )
{
	//////////////////////////////////////////////////////////////////////////
	// Put exported files in pak.
	//////////////////////////////////////////////////////////////////////////
	CString levelPath = Path::AddBackslash(GetIEditor()->GetGameEngine()->GetLevelPath());
	CString pakFilename = levelPath + "LevelLM.pak";
	CString filenameLM = levelPath + "Dot3LM.dat";
	CString filenameLights = levelPath + "StatLights.dat";

	if (!CFileUtil::OverwriteFile( pakFilename ))
		return false;

	if (!GetIEditor()->GetSystem()->GetIPak()->ClosePack( pakFilename ))
	{ 
		Error( "Cannot Close Pak File %s",(const char*)pakFilename );
		return false;
	}
	//////////////////////////////////////////////////////////////////////////
	IndoorBaseInterface IndInterface;
	IndInterface.m_pSystem   = pSystem;
	IndInterface.m_p3dEngine = pSystem->GetI3DEngine();
	IndInterface.m_pLog      = pSystem->GetILog();
	IndInterface.m_pRenderer = pSystem->GetIRenderer();
	IndInterface.m_pConsole  = pSystem->GetIConsole();

	CLMLightCollection cLights;

	SEntityUpdateContext ctx;
	ctx.pScriptUpdateParams = NULL;
	ctx.nFrameID = GetIEditor()->GetRenderer()->GetFrameID();
	ctx.pCamera = &GetIEditor()->GetSystem()->GetViewCamera();

	IEntityIt *pIIT = pSystem->GetIEntitySystem()->GetEntityIterator();
	IEntity *pIEty = NULL;
	while (pIEty = pIIT->Next())
	{
		const char * sName = pIEty->GetEntityClassName();
		if (strstr(sName, "DynamicLight"))
		{
			CDLight *pLight = pIEty->GetLight();
			int dwOldFlags = pIEty->GetRndFlags();
			pIEty->SetRndFlags(ERF_UPDATE_IF_PV, false);
 
			 EEntityUpdateVisLevel eOldLevel = pIEty->GetUpdateVisLevel();
			 pIEty->SetUpdateVisLevel(eUT_Always);

			// Required for waking up all lights
			pIEty->SetNeedUpdate(true);
			ctx.fFrameTime = 0.01f;
			pIEty->Update(ctx); 
			if (pLight && !IsEquivalent(pLight->m_Origin, Vec3d(0,0,0),1))
			{
				const bool cbCastShadow = (pIEty->GetRndFlags() & ERF_CASTSHADOWINTOLIGHTMAP);
				if(cbCastShadow || (pLight->m_Flags & DLF_LMOCCL))
				{
					pLight->m_fStartTime = pSystem->GetITimer()->GetCurrTime();
					pSystem->GetIRenderer()->EF_UpdateDLight(pLight);
					// Create LMCompLight from CDLight
					cLights.AddLight(pLight, pIEty->GetName(), pIEty->GetId(), cbCastShadow);
				}
			}
			pIEty->SetNeedUpdate(false); 
			pIEty->SetRndFlags(dwOldFlags);
			pIEty->SetUpdateVisLevel(eOldLevel);
		}
	}
	CLightScene cLightScene;

	std::sort(nodes.begin(), nodes.end(), CEntityRenderPred());

	bool bErrorsOccured = false;
	if (!cLightScene.CreateFromEntity(IndInterface, m_sParam, nodes, cLights, 
		pICompilerProgress, Mode, m_pSharedData, vSelectionIndices, bErrorsOccured))
	{
		Error("CLightmapGen::Generate failed");
	}
	return bErrorsOccured;
}

const bool CLightmapGen::GenerateSelected(IEditor *pIEditor, ICompilerProgress *pICompilerProgress)
{
	std::vector<std::pair<IEntityRender*,CBrushObject*> > nodes;
	CSelectionGroup *pCurSel = pIEditor->GetObjectManager()->GetSelection();
	UINT iNumObjects = pCurSel->GetCount();
	UINT iCurObj;
	//set to help checking for selection 
	//first the selected objects
	std::set<const CBrushObject*> vSelection;
	for (iCurObj=0; iCurObj<iNumObjects; iCurObj++)
	{
		CBaseObject *pCurObject = pCurSel->GetObject(iCurObj);
		if (pCurObject->GetType() != OBJTYPE_BRUSH)
			continue;
		CBrushObject *pBrushObj = (CBrushObject *) pCurObject;
		IEntityRender *pIEtyRend = pBrushObj->GetEngineNode();
		if (pIEtyRend != NULL && IsLM_GLM(pIEtyRend))
		{
			vSelection.insert(pBrushObj); 
			nodes.push_back(std::pair<IEntityRender*,CBrushObject*>(pIEtyRend,pBrushObj));
		}
	} 
	//now the others
	std::vector<CBaseObject*> objects;
	pIEditor->GetObjectManager()->GetObjects( objects );
	for (int i = 0; i < objects.size(); i++)
	{
		if (objects[i]->GetType() != OBJTYPE_BRUSH)
			continue;
		// Cast to brush.
		CBrushObject *obj = (CBrushObject*)objects[i];
		if(vSelection.find(obj) != vSelection.end())
			continue;
		IEntityRender *node = obj->GetEngineNode();
		if (node && IsLM_GLM(node))
		{
			nodes.push_back(std::pair<IEntityRender*,CBrushObject*>(node,obj));
		}
	}

	const bool cbErrorsOccured = Generate(pIEditor->GetSystem(), nodes, pICompilerProgress,ELMMode_SEL, vSelection);
	if (pICompilerProgress)
		pICompilerProgress->Output("\r\nLightmap compilation finished");
	return cbErrorsOccured;
}


const bool CLightmapGen::GenerateChanged( IEditor *pIEditor, ICompilerProgress *pICompilerProgress )
{
	std::vector<std::pair<IEntityRender*,CBrushObject*> > nodes;
	UINT i;

	std::vector<CBaseObject*> objects;
	pIEditor->GetObjectManager()->GetObjects( objects );
	for (i = 0; i < objects.size(); i++)
	{
		if (objects[i]->GetType() != OBJTYPE_BRUSH)
			continue;

		// Cast to brush.
		CBrushObject *obj = (CBrushObject*)objects[i];
		IEntityRender *node = obj->GetEngineNode();
		if (node && IsLM_GLM(node))
		{
			nodes.push_back(std::pair<IEntityRender*,CBrushObject*>(node,obj));
		}
	}
	std::set<const CBrushObject*> vSelectionIndices;
	const bool cbErrorsOccured = Generate(pIEditor->GetSystem(), nodes, pICompilerProgress, ELMMode_CHANGES, vSelectionIndices);

	// Export brush.lst
	if (pICompilerProgress)
		pICompilerProgress->Output("Full rebuild, Exporting 'brush.lst'...\r\n");
	CGameExporter cExporter(pIEditor->GetSystem());
	cExporter.ExportBrushes(pIEditor->GetGameEngine()->GetLevelPath());
	if (pICompilerProgress)
		pICompilerProgress->Output("\r\nLightmap compilation finished");
	return cbErrorsOccured;
}

bool CLightmapGen::IsLM_GLM(struct IEntityRender *pIEtyRend)
{
	if (pIEtyRend->GetRndFlags() & ERF_USELIGHTMAPS)
		return true;
	if (pIEtyRend->GetRndFlags() & ERF_CASTSHADOWINTOLIGHTMAP)
		return true;
	return false;
}

const bool CLightmapGen::GenerateAll(IEditor *pIEditor, ICompilerProgress *pICompilerProgress)
{
	std::vector<std::pair<IEntityRender*,CBrushObject*> > nodes;
	UINT i;

	std::vector<CBaseObject*> objects;
	pIEditor->GetObjectManager()->GetObjects( objects );
	for (i = 0; i < objects.size(); i++)
	{
		if (objects[i]->GetType() != OBJTYPE_BRUSH)
			continue;

		// Cast to brush.
		CBrushObject *obj = (CBrushObject*)objects[i];
		IEntityRender *node = obj->GetEngineNode();
		if (node && IsLM_GLM(node))
		{
			nodes.push_back(std::pair<IEntityRender*,CBrushObject*>(node,obj));
		}
	}
	std::set<const CBrushObject*> vSelectionIndices;
	const bool cbErrorsOccured = Generate(pIEditor->GetSystem(), nodes, pICompilerProgress,ELMMode_ALL, vSelectionIndices);

	// Export brush.lst
	if (pICompilerProgress)
		pICompilerProgress->Output("Full rebuild, Exporting 'brush.lst'...\r\n");
	CGameExporter cExporter(pIEditor->GetSystem());
	cExporter.ExportBrushes(pIEditor->GetGameEngine()->GetLevelPath());
	if (pICompilerProgress)
		pICompilerProgress->Output("\r\nLightmap compilation finished");
	return cbErrorsOccured;
}

void CLightmapGen::Serialize(CXmlArchive xmlAr)
{
	// Serialize lightmap compiler settings
	if (xmlAr.bLoading)
	{
		XmlNodeRef xmlLMCompiler = xmlAr.root->findChild("LMCompiler");

		if (!xmlLMCompiler)
			return;

		xmlLMCompiler->getAttr("TextureResolution", CLightmapGen::m_sParam.m_iTextureResolution);
		xmlLMCompiler->getAttr("TexelSize", CLightmapGen::m_sParam.m_fTexelSize);
		xmlLMCompiler->getAttr("DebugBorders", CLightmapGen::m_sParam.m_bDebugBorders);
		xmlLMCompiler->getAttr("DontMergePolys", CLightmapGen::m_sParam.m_bDontMergePolys);
		xmlLMCompiler->getAttr("SubSampling", CLightmapGen::m_sParam.m_iSubSampling);
		xmlLMCompiler->getAttr("ComputeShadows", CLightmapGen::m_sParam.m_bComputeShadows);
		xmlLMCompiler->getAttr("UseSunLight", CLightmapGen::m_sParam.m_bUseSunLight);
		xmlLMCompiler->getAttr("OverrideSpotlight", CLightmapGen::m_sParam.m_bSpotAsPointlight);
		xmlLMCompiler->getAttr("SmoothingAngle", CLightmapGen::m_sParam.m_uiSmoothingAngle);
		xmlLMCompiler->getAttr("GenOcclMaps", CLightmapGen::m_sParam.m_bGenOcclMaps);
	}
	else
	{
		XmlNodeRef xmlLMCompiler = xmlAr.root->newChild("LMCompiler");

		xmlLMCompiler->setAttr("TextureResolution", CLightmapGen::m_sParam.m_iTextureResolution);
		xmlLMCompiler->setAttr("TexelSize", CLightmapGen::m_sParam.m_fTexelSize);
		xmlLMCompiler->setAttr("DebugBorders", CLightmapGen::m_sParam.m_bDebugBorders);
		xmlLMCompiler->setAttr("DontMergePolys", CLightmapGen::m_sParam.m_bDontMergePolys);
		xmlLMCompiler->setAttr("SubSampling", CLightmapGen::m_sParam.m_iSubSampling);
		xmlLMCompiler->setAttr("ComputeShadows", CLightmapGen::m_sParam.m_bComputeShadows);
		xmlLMCompiler->setAttr("UseSunLight", CLightmapGen::m_sParam.m_bUseSunLight);
		xmlLMCompiler->setAttr("OverrideSpotlight", CLightmapGen::m_sParam.m_bSpotAsPointlight);
		xmlLMCompiler->setAttr("SmoothingAngle", CLightmapGen::m_sParam.m_uiSmoothingAngle);
		xmlLMCompiler->setAttr("GenOcclMaps", CLightmapGen::m_sParam.m_bGenOcclMaps);
	}
}