////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   lightmapgen.h
//  Version:     v1.00
//  Created:     18/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __lightmapgen_h__
#define __lightmapgen_h__
#pragma once

#include "LMCompStructures.h" 
#include "Objects\BrushObject.h"

class CLightmapGen
{
public:
	CLightmapGen(volatile SSharedLMEditorData *pSharedData = NULL) : m_pSharedData(pSharedData){};
	~CLightmapGen(){};

	//! Generate lightmaps.
	const bool GenerateSelected(IEditor *pIEditor, ICompilerProgress *pICompilerProgress = NULL);
	const bool GenerateAll(IEditor *pIEditor, ICompilerProgress *pICompilerProgress = NULL);
	const bool GenerateChanged(IEditor *pIEditor, ICompilerProgress *pICompilerProgress = NULL);

	static void SetGenParam(LMGenParam sParam) { m_sParam = sParam; };
	static LMGenParam GetGenParam() { return m_sParam; };

	static void Serialize(CXmlArchive xmlAr);
	
protected:
	volatile SSharedLMEditorData* m_pSharedData;

	static LMGenParam m_sParam;

	const bool Generate(ISystem *pSystem, std::vector<std::pair<IEntityRender*, CBrushObject*> >& nodes, ICompilerProgress *pICompilerProgress,
		const ELMMode Mode, const std::set<const CBrushObject*>& vSelectionIndices);

	bool IsLM_GLM(struct IEntityRender *pIEtyRend);


};

#endif // __lightmapgen_h__
