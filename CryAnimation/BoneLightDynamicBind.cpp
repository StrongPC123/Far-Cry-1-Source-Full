#include "stdafx.h"
#include "cvars.h"
#include "DebugUtils.h"
#include "MathUtils.h"
#include "BoneLightDynamicBind.h"

/*
 *	Here is the sample code from Renderer.cpp, line 3000:
 dl->m_Orientation.m_vForward = Vec3d(1,0,0);
 dl->m_Orientation.m_vUp = Vec3d(0,1,0);
 dl->m_Orientation.m_vRight = Vec3d(0,0,1);
 dl->m_Orientation.rotate(Vec3d(1,0,0), dl->m_ProjAngles.x);
 dl->m_Orientation.rotate(Vec3d(0,1,0), dl->m_ProjAngles.y);
 dl->m_Orientation.rotate(Vec3d(0,0,1), dl->m_ProjAngles.z);
 * it converts the light angles into the matrix
 */

// initializes this object out of the given DLight structure and bone index
// this is one-time call that is only required for construction out of the DLight
// to initialize the constant parameters
void CBoneLightDynamicBind::init (ICryCharInstance* pParent, CDLight* pDLight, unsigned nBone, const Matrix44& matBone, bool bCopyLight)
{
	m_pDLight = bCopyLight?new CDLight(*pDLight):pDLight;
	m_bDeleteLight = bCopyLight;
	m_nBone = nBone;
	m_fRadius = pDLight->m_fRadius;
	m_nDLightFlags = pDLight->m_Flags;
	m_matLight.SetRotationXYZ(DEG2RAD(pDLight->m_ProjAngles));
	m_matLight.SetTranslationOLD(pDLight->m_vObjectSpacePos);
	m_pDLight->m_pCharInstance = pParent;

	// at this point, we have the m_matLight in character object coordinates
	// now let's transform it into the bone coordinates
	m_matLight = m_matLight * OrthoUniformGetInverted(matBone);
/*
	pDLight->m_Orientation.m_vForward = Vec3d(1,0,0);
	pDLight->m_Orientation.m_vUp = Vec3d(0,1,0);
	pDLight->m_Orientation.m_vRight = Vec3d(0,0,1);
	pDLight->m_Orientation.rotate(Vec3d(1,0,0), pDLight->m_ProjAngles.x);
	pDLight->m_Orientation.rotate(Vec3d(0,1,0), pDLight->m_ProjAngles.y);
	pDLight->m_Orientation.rotate(Vec3d(0,0,1), pDLight->m_ProjAngles.z);
*/
}

// deletes the light object, if needed
void CBoneLightDynamicBind::done()
{
	m_pDLight->m_pCharInstance = NULL;
	if (m_bDeleteLight)
		delete m_pDLight;
}

// per-frame update of the DLight structure. Updates the light position and radius
void CBoneLightDynamicBind::updateDLight (const Matrix44& matBone, const Matrix44& matModel, float fRadiusMultiplier)
{
	Matrix44 matLightObject = m_matLight * matBone;
	Matrix44 matLightWorld = matLightObject * matModel;
	m_pDLight->m_Origin = matLightWorld.GetTranslationOLD();
	m_pDLight->m_vObjectSpacePos = matLightObject.GetTranslationOLD();
	m_pDLight->m_ProjAngles = RAD2DEG(Ang3::GetAnglesXYZ(Matrix33(matLightWorld)));
	m_pDLight->m_fRadius = m_fRadius * fRadiusMultiplier;

	static const float fColor[3][4] =
	{
		{1,0.8f,0.8f,1},
		{0.8f,1,0.8f,1},
		{0.8f,0.8f,1,1}
	};
	if (g_GetCVars()->ca_DrawLHelpers())
	{
		float fTripodScale = 1;
		debugDrawLine (m_pDLight->m_Origin, matLightWorld.TransformPointOLD(Vec3d(fTripodScale,0,0)), fColor[0]);
		debugDrawLine (m_pDLight->m_Origin, matLightWorld.TransformPointOLD(Vec3d(0,fTripodScale,0)), fColor[1]);
		debugDrawLine (m_pDLight->m_Origin, matLightWorld.TransformPointOLD(Vec3d(0,0,fTripodScale)), fColor[2]);
	}
}


// returns true if this light source is local (affects only the character)
bool CBoneLightDynamicBind::isLocal()const
{
	return (m_nDLightFlags & DLF_LOCAL) != 0;
}

bool CBoneLightDynamicBind::isHeatSource()const
{
	return (m_nDLightFlags & DLF_HEATSOURCE) != 0;
}

bool CBoneLightDynamicBind::isLightSource()const
{
	return (m_nDLightFlags & DLF_LIGHTSOURCE) != 0;
}
