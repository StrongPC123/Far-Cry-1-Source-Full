// TopRendererWnd.h: interface for the CTopRendererWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TOPRENDERERWND_H__5E850B78_3791_40B5_BD0A_850682E8CAF9__INCLUDED_)
#define AFX_TOPRENDERERWND_H__5E850B78_3791_40B5_BD0A_850682E8CAF9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "2DViewport.h"

// Predeclare because of friend declaration
class CTopRendererWnd : public C2DViewport
{
	DECLARE_DYNCREATE(CTopRendererWnd)
public:
	CTopRendererWnd();
	virtual ~CTopRendererWnd();

	/** Get type of this viewport.
	*/
	virtual EViewportType GetType() { return ET_ViewportMap; }
	virtual void SetType( EViewportType type );

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTopRendererWnd)
	//}}AFX_VIRTUAL
	
	virtual void ResetContent();
	virtual void UpdateContent( int flags );

	//! Map viewport position to world space position.
	virtual Vec3d	ViewToWorld( CPoint vp,bool *collideWithTerrain=0,bool onlyTerrain=false );

	virtual void OnTitleMenu( CMenu &menu );
	virtual void OnTitleMenuCommand( int id );

protected:
	void ResetSurfaceTexture();
	void UpdateSurfaceTexture( int flags );

	// Draw everything.
	virtual void Draw( DisplayContext &dc );

	void DrawStaticObjects();
	
	//{{AFX_MSG(CTopRendererWnd)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
private:
	bool m_bContentsUpdated;

	//CImage* m_pSurfaceTexture;
	int m_terrainTextureId;

	CSize m_textureSize;

	// Size of heightmap in meters.
	CSize m_heightmapSize;

	CImage m_terrainTexture;

	// Display options.
	bool m_bDisplayLabels;
	bool m_bShowHeightmap;
	
	bool m_bShowStatObjects;

	CImage m_vegetationTexture;
	CPoint m_vegetationTexturePos;
	CSize m_vegetationTextureSize;
	int		m_vegetationTextureId;
};

#endif // !defined(AFX_TOPRENDERERWND_H__5E850B78_3791_40B5_BD0A_850682E8CAF9__INCLUDED_)
