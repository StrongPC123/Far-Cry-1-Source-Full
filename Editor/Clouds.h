// Clouds.h: Interface of the class CClouds.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HEIGHTMAP_H__747B5795_AE67_4CA6_AF55_73A7659597FE__INCLUDED_)
#define AFX_HEIGHTMAP_H__747B5795_AE67_4CA6_AF55_73A7659597FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <time.h>
#include "Noise.h"

class CXmlArchive;

class CClouds : public CObject
{
	DECLARE_SERIAL (CClouds)

public:
	bool GenerateClouds(SNoiseParams *sParam, CWnd *pWndStatus);
	void DrawClouds(CDC *pDC, LPRECT prcPosition);
	void Serialize(CArchive& ar);
	void Serialize( CXmlArchive &xmlAr );

	SNoiseParams* GetLastParam() { return &m_sLastParam; };
	CClouds();
	virtual ~CClouds();

protected:
	void CleanUp();
	float CloudExpCurve(float v, unsigned int iCover, float fSharpness);

	unsigned int m_iWidth;
	unsigned int m_iHeight;

	CBitmap m_bmpClouds;
	CDC m_dcClouds;

	// Used to hold the last used generation parameters
	SNoiseParams m_sLastParam;

private:

	void CalcCloudsPerlin(SNoiseParams *sParam, CWnd *pWndStatus);
};

#endif // !defined(AFX_HEIGHTMAP_H__747B5795_AE67_4CA6_AF55_73A7659597FE__INCLUDED_)
