#include "stdafx.h"
#include "CryAnimationBase.h"
#include "DebugUtils.h"
#include "MathUtils.h"

Matrix44 debugGetMatrix (CCObject*pObj)
{
	return pObj->m_Matrix;
}

void debugDrawBBoxWCS (const Matrix44& matModel, const Vec3& vMin, const Vec3& vMax, const float pColor[4] = NULL)
{
	Vec3 vMinW, vMaxW;
	vMinW = vMaxW = matModel.TransformPointOLD(vMin);
	AddToBounds(matModel.TransformPointOLD(vMax), vMinW, vMaxW);
	AddToBounds(matModel.TransformPointOLD(Vec3(vMin.x, vMin.y,vMax.z)),vMinW, vMaxW);
	AddToBounds(matModel.TransformPointOLD(Vec3(vMin.x, vMax.y,vMin.y)),vMinW, vMaxW);
	AddToBounds(matModel.TransformPointOLD(Vec3(vMin.x, vMax.y,vMax.z)),vMinW, vMaxW);
	AddToBounds(matModel.TransformPointOLD(Vec3(vMax.x, vMin.y,vMin.z)),vMinW, vMaxW);
	AddToBounds(matModel.TransformPointOLD(Vec3(vMax.x, vMin.y,vMax.z)),vMinW, vMaxW);
	AddToBounds(matModel.TransformPointOLD(Vec3(vMax.x, vMax.y,vMin.y)),vMinW, vMaxW);

	IRenderer* pRenderer = g_GetIRenderer();
	pRenderer->Draw3dPrim(vMinW, vMaxW, DPRIM_WHIRE_BOX, pColor);
}

void debugDrawLine (const Vec3& a, const Vec3& b, const float pColor[4])
{
	IRenderer* pRenderer = g_GetIRenderer();
	pRenderer->Draw3dPrim (a,b,DPRIM_LINE, pColor);
}

void debugDrawLine (const Matrix44& matLCS, const Vec3& a, const Vec3& b, const float pColor[4])
{
	debugDrawLine(matLCS.TransformPointOLD(a), matLCS.TransformPointOLD(b), pColor);
}

void debugDrawCircle (const Matrix34& m34, float fRadius, int nAxis, const float* pColor = NULL)
{
	IRenderer* pRenderer = g_GetIRenderer();
	Vec3 ptPrev (0,0,0);
	ptPrev[(nAxis+1)%3] = fRadius;
	ptPrev = m34*ptPrev;

	const int nSteps = 16;

	for (int i = 1; i <= nSteps; ++i)
	{
		Vec3 ptNext(0,0,0);
		ptNext[(nAxis+1)%3] = float(cos (2*gPi*i/nSteps) * fRadius);
		ptNext[(nAxis+2)%3] = float(sin (2*gPi*i/nSteps) * fRadius);
		ptNext = m34*ptNext;

		pRenderer->Draw3dPrim (ptPrev, ptNext, DPRIM_LINE, pColor);
		ptPrev = ptNext;
	}
}

void debugDrawLines (const Vec3& a, const Vec3& b, const Vec3& d, int nSubdiv, const float* pColor = NULL)
{
	IRenderer* pRenderer = g_GetIRenderer();
	pRenderer->Draw3dPrim(a,b,DPRIM_LINE,pColor);
	for (int i = 1; i < nSubdiv; ++i)
		pRenderer->Draw3dPrim(a + (d-a)*float(double(i)/nSubdiv), b + (d-a)*float(double(i)/nSubdiv), DPRIM_LINE, pColor);
}

void debugDrawOctahedron (const Matrix44& matModel, float fSize, const float pColor[4])
{
	Vec3 pt[3][2];
	for (int nAxis = 0; nAxis < 3; ++nAxis)
	{
		Vec3 p(0,0,0);
		p[nAxis] = fSize;
		pt[nAxis][0] = matModel.TransformPointOLD(-p);
		pt[nAxis][1] = matModel.TransformPointOLD(p);
	}
	debugDrawLine (pt[0][0], pt[1][0], pColor);
	debugDrawLine (pt[0][0], pt[1][1], pColor);
	debugDrawLine (pt[0][0], pt[2][0], pColor);
	debugDrawLine (pt[0][0], pt[2][1], pColor);
	debugDrawLine (pt[0][1], pt[1][0], pColor);
	debugDrawLine (pt[0][1], pt[1][1], pColor);
	debugDrawLine (pt[0][1], pt[2][0], pColor);
	debugDrawLine (pt[0][1], pt[2][1], pColor);
}

void debugDrawBBox (const Matrix34& matModel, const CryAABB& BBox, int nSubdiv, const float* pColor)
{
/*	IRenderer* pRenderer = g_GetIRenderer();
	float fColor[4] = {0,1,0,1};
	extern float g_YLine;
	g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"%15.10f %15.10f %15.10f %15.10f",matModel(0,0),matModel(0,1),matModel(0,2),matModel(0,3) );	g_YLine+=16.0f;
	g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"%15.10f %15.10f %15.10f %15.10f",matModel(1,0),matModel(1,1),matModel(1,2),matModel(1,3) );	g_YLine+=16.0f;
	g_pIRenderer->Draw2dLabel( 1,g_YLine, 1.3f, fColor, false,"%15.10f %15.10f %15.10f %15.10f",matModel(2,0),matModel(2,1),matModel(2,2),matModel(2,3) );	g_YLine+=16.0f;
	g_YLine+=16.0f; */

	Vec3 v000 = matModel*BBox.vMin;
	Vec3 v111 = matModel*BBox.vMax;
	Vec3 v100 = matModel*Vec3(BBox.vMin.x, BBox.vMin.y,BBox.vMax.z);
	Vec3 v010 = matModel*Vec3(BBox.vMin.x, BBox.vMax.y,BBox.vMin.z);
	Vec3 v110 = matModel*Vec3(BBox.vMin.x, BBox.vMax.y,BBox.vMax.z);
	Vec3 v001 = matModel*Vec3(BBox.vMax.x, BBox.vMin.y,BBox.vMin.z);
	Vec3 v101 = matModel*Vec3(BBox.vMax.x, BBox.vMin.y,BBox.vMax.z);
	Vec3 v011 = matModel*Vec3(BBox.vMax.x, BBox.vMax.y,BBox.vMin.z);

	// around X
	debugDrawLines(v000, v001, v010, nSubdiv, pColor);
	debugDrawLines(v010, v011, v110, nSubdiv, pColor);
	debugDrawLines(v110, v111, v100, nSubdiv, pColor);
	debugDrawLines(v100, v101, v000, nSubdiv, pColor);
	// around Y
	debugDrawLines(v000, v010, v001, nSubdiv, pColor);
	debugDrawLines(v001, v011, v101, nSubdiv, pColor);
	debugDrawLines(v101, v111, v100, nSubdiv, pColor);
	debugDrawLines(v100, v110, v000, nSubdiv, pColor);
	// around Z
	debugDrawLines(v000, v100, v001, nSubdiv, pColor);
	debugDrawLines(v001, v101, v011, nSubdiv, pColor);
	debugDrawLines(v011, v111, v010, nSubdiv, pColor);
	debugDrawLines(v010, v110, v000, nSubdiv, pColor);
	/*
	pRenderer->Draw3dBBox(v000, v010, true);
	pRenderer->Draw3dBBox(v000, v100, true);

	pRenderer->Draw3dBBox(v111, v011, true);
	pRenderer->Draw3dBBox(v111, v101, true);
	pRenderer->Draw3dBBox(v111, v110, true);

	pRenderer->Draw3dBBox(v011, v001, true);
	pRenderer->Draw3dBBox(v011, v010, true);

	pRenderer->Draw3dBBox(v101, v001, true);
	pRenderer->Draw3dBBox(v101, v100, true);

	pRenderer->Draw3dBBox(v110, v100, true);
	pRenderer->Draw3dBBox(v110, v010, true);
	*/
}


void debugDrawSphere (const Matrix34& m34, float fRadius, const float pColor[4])
{
	for (int nAxis = 0; nAxis < 3; ++nAxis)
		debugDrawCircle(m34, fRadius, nAxis, pColor);

	debugDrawLine (m34*Vec3(-fRadius/2,0,0),m34*Vec3(fRadius,0,0), pColor);
}

void debugDrawRootBone (const Matrix44& rBone, float fSize, const float pColor[4])
{
	Matrix34 m34=Matrix34(GetTransposed44(rBone));
	debugDrawSphere (m34, fSize, pColor);

	debugDrawOctahedron(rBone, fSize*2, pColor);
}


void debugDrawBone (const Matrix44& rParent, const Matrix44& rBone, const float pColor[4])
{
	Vec3 vBone = rParent.GetTranslationOLD() - rBone.GetTranslationOLD();
	float fBoneSize = vBone.Length();

	Matrix34 m34=Matrix34(GetTransposed44(rBone));
	debugDrawSphere(m34, fBoneSize / 15, pColor);

	if (fBoneSize < 1e-4)
		return;

	debugDrawLine (rParent.GetTranslationOLD(), rBone.GetTranslationOLD(), pColor);

	Matrix44 matBone2;
	float q = fBoneSize/20;
	BuildMatrixFromFwd (vBone/fBoneSize, rBone.GetTranslationOLD(), matBone2);
	debugDrawLine (rBone.GetTranslationOLD(), matBone2.TransformPointOLD(Vec3(q,q,q)), pColor);
	debugDrawLine (rBone.GetTranslationOLD(), matBone2.TransformPointOLD(Vec3(-q,q,q)), pColor);
	debugDrawLine (rBone.GetTranslationOLD(), matBone2.TransformPointOLD(Vec3(q,-q,q)), pColor);
	debugDrawLine (rBone.GetTranslationOLD(), matBone2.TransformPointOLD(Vec3(-q,-q,q)), pColor);

	debugDrawLine (matBone2.TransformPointOLD(Vec3( q, q,q)), matBone2.TransformPointOLD(Vec3(-q,q,q)), pColor);
	debugDrawLine (matBone2.TransformPointOLD(Vec3(-q, q,q)), matBone2.TransformPointOLD(Vec3(-q,-q,q)), pColor);
	debugDrawLine (matBone2.TransformPointOLD(Vec3(-q,-q,q)), matBone2.TransformPointOLD(Vec3(q,-q,q)), pColor);
	debugDrawLine (matBone2.TransformPointOLD(Vec3( q,-q,q)), matBone2.TransformPointOLD(Vec3(q,q,q)), pColor);

	debugDrawLine (rParent.GetTranslationOLD(), matBone2.TransformPointOLD(Vec3(q,q,q)), pColor);
	debugDrawLine (rParent.GetTranslationOLD(), matBone2.TransformPointOLD(Vec3(-q,q,q)), pColor);
	debugDrawLine (rParent.GetTranslationOLD(), matBone2.TransformPointOLD(Vec3(q,-q,q)), pColor);
	debugDrawLine (rParent.GetTranslationOLD(), matBone2.TransformPointOLD(Vec3(-q,-q,q)), pColor);
}

bool IsValidString(const char*szMorphTarget)
{
	if (!szMorphTarget)
		return false;
#ifdef WIN32
	return !IsBadStringPtr(szMorphTarget, 0x80000000);
#else
	return true;
#endif

}