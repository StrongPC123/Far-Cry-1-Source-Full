// ---------------------------------------------------------------------------------------------
//	Crytek CryENGINE source code
//	History:
//	- Created by Michael Glueck
// ---------------------------------------------------------------------------------------------
#pragma once

class GLMInfo
{
protected:
	//info for one particular patch 
	typedef struct GLMInfoPatch
	{
		unsigned int uiOffsetX;
		unsigned int uiOffsetY;
		unsigned int uiWidth;
		unsigned int uiHeight;
	}GLMInfoPatch;

	//info struct for one GLM
	typedef struct GLMInfoMesh
	{
		CString GLMName;
		unsigned int uiLMIndex;
		std::vector<GLMInfoPatch> vPatchInfos;
	}GLMInfoMesh;



};



