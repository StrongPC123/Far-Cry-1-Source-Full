#ifndef _CRY_ANIMATION_CRY_CHAR_INSTANCE_RENDER_PARAMS_HDR_
#define _CRY_ANIMATION_CRY_CHAR_INSTANCE_RENDER_PARAMS_HDR_

struct CryCharInstanceRenderParams
{
	CFColor m_Color;
	int m_nFlags; 
	CDLight m_ambientLight;

	// creates a new CCObject with the most common parameters
	CCObject* NewCryCharCCObject(const struct SRendParams & RendParams, const Matrix44& mtxObjMatrix, IDeformableRenderMesh* pCharInstance);
};


#endif