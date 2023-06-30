#include "Stdafx.h"
#include "CryAnimationBase.h"
#include "CryCharInstanceRenderParams.h"

// creates a new CCObject with the most common parameters
CCObject* CryCharInstanceRenderParams::NewCryCharCCObject(const struct SRendParams & RendParams, const Matrix44& mtxObjMatrix, IDeformableRenderMesh* pCharInstance)
{
	IRenderer* pRenderer = g_GetIRenderer();
	CCObject * pObj = pRenderer->EF_GetObject(true);

  pObj->m_ObjFlags |= FOB_TRANS_MASK;

	// set scissor
	//pObj->m_nScissorX1 = RendParams.nScissorX1;
	//pObj->m_nScissorY1 = RendParams.nScissorY1;
	//pObj->m_nScissorX2 = RendParams.nScissorX2;
	//pObj->m_nScissorY2 = RendParams.nScissorY2;

	//checl if it should be drawn close to the player
  if ((RendParams.dwFObjFlags & FOB_NEAREST) || (m_nFlags & CS_FLAG_DRAW_NEAR))
	{
		pObj->m_ObjFlags|=FOB_NEAREST;
	}
	else
		pObj->m_ObjFlags&=~FOB_NEAREST;

	pObj->m_Color = m_Color;
	pObj->m_Color.a *= RendParams.fAlpha;

	pObj->m_AmbColor = RendParams.vAmbientColor;

	if (pRenderer->EF_GetHeatVision())
		pObj->m_ObjFlags |= FOB_HOTAMBIENT;

	pObj->m_ObjFlags |= RendParams.dwFObjFlags;

	int nTemplID = RendParams.nShaderTemplate;

	pObj->m_pShadowCasters = RendParams.pShadowMapCasters;

	if(RendParams.pShadowMapCasters && RendParams.pShadowMapCasters->size())
		pObj->m_ObjFlags |= FOB_INSHADOW;
	else
		pObj->m_ObjFlags &= ~FOB_INSHADOW;

	pObj->m_nTemplId = nTemplID;

	pObj->m_DynLMMask = RendParams.nDLightMask;
	pObj->m_SortId = RendParams.fCustomSortOffset;
	pObj->m_fDistanceToCam = RendParams.fSQDistance;


	pObj->m_Matrix = mtxObjMatrix;
//	pObj->m_Matrix = mtxObjMatrix;
	pObj->m_pCharInstance = pCharInstance;

	return pObj;
}
