// ViewManager.h: interface for the CViewManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIEWMANAGER_H__CE538069_BE63_4E85_93B4_74D3F72BB112__INCLUDED_)
#define AFX_VIEWMANAGER_H__CE538069_BE63_4E85_93B4_74D3F72BB112__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Grid.h"
#include "Viewport.h"

// forward declaration.
class CLayoutWnd;

/** Description of viewport.
*/
class CViewportDesc : public CRefCountBase
{
public:
	EViewportType type;	//!< Type of viewport.
	CString name;				//!< Name of viewport.
	CRuntimeClass *rtClass; //!< Runtime class of view.

	CViewportDesc( EViewportType _type,const CString &_name,CRuntimeClass* _rtClass )
	{
		type = _type;
		name = _name;
		rtClass = _rtClass;
	}
};

/** Manages set of viewports.
*/
class CViewManager
{
public:
	//! Create viewport.
	CViewport* CreateView( EViewportType type,CWnd *pParentWnd );
	void ReleaseView( CViewport *pViewport );
	
	CViewport* GetViewport( EViewportType type ) const;
	CViewport* GetViewport( const CString &name ) const;

	// Description:
	//		Returns currently active viewport.
	CViewport* GetActiveViewport() const;

	//! Find viewport at Screen point.
	CViewport* GetViewportAtPoint( CPoint point ) const;

	void	SetAxisConstrain( int axis );

	void	SetZoomFactor( float zoom );
	float	GetZoomFactor() const { return m_zoomFactor; }

	//! Reset all views.
	void	ResetViews();
	//! Update all views.
	void	UpdateViews( int flags=0xFFFFFFFF );
	
	void	Update();

	void SetUpdateRegion( const BBox &updateRegion ) { m_updateRegion = updateRegion; };
	const BBox& GetUpdateRegion() { return m_updateRegion; };

	/** Retrieve Grid used for viewes.
	*/
	CGrid* GetGrid() { return &m_grid; };

	/** Get 2D viewports origin.
	*/
	Vec3 GetOrigin2D() const { return m_origin2D; }

	/** Assign 2D viewports origin.
	*/
	void SetOrigin2D( const Vec3 &org ) { m_origin2D = org; };

	/** Assign zoom factor for 2d viewports.
	*/
	void SetZoom2D( float zoom ) { m_zoom2D = zoom; };

	/** Get zoom factor of 2d viewports.
	*/
	float GetZoom2D() const { return m_zoom2D; };

	//////////////////////////////////////////////////////////////////////////
	//! Get currently active camera object id.
	REFGUID GetCameraObjectId() const { return m_cameraObjectId; };
	//! Sets currently active camera object id.
	void SetCameraObjectId( REFGUID cameraObjectId ) { m_cameraObjectId = cameraObjectId; };

	//////////////////////////////////////////////////////////////////////////
	//! Add new viewport description to view manager.
	virtual void RegisterViewportDesc( CViewportDesc *desc );
	//! Get viewport descriptions.
	virtual void GetViewportDescriptions( std::vector<CViewportDesc*> &descriptions );

	//////////////////////////////////////////////////////////////////////////
	//! Get number of currently existing viewports.
	virtual int GetViewCount() { return m_viewports.size(); };
	//! Get viewport by index.
	//! @param index 0 <= index < GetViewportCount()
	virtual CViewport* GetView( int index ) { return m_viewports[index]; }

	//////////////////////////////////////////////////////////////////////////
	//! Get current layout window.
	//! @return Pointer to the layout window, can be NULL.
	virtual CLayoutWnd* GetLayout() const;

	//! Cycyle between different 2D viewports type on same view pane.
	virtual void Cycle2DViewport();

private:
	friend class CEditorImpl;
	CViewManager();
	~CViewManager();
	//////////////////////////////////////////////////////////////////////////
	//FIELDS.
	float	m_zoomFactor;

	BBox m_updateRegion;

	CGrid m_grid;
	//! Origin of 2d viewports.
	Vec3 m_origin2D;
	//! Zoom of 2d viewports.
	float m_zoom2D;

	//! Id of camera object.
	GUID m_cameraObjectId;

	//! Array of viewport descriptions.
	std::vector<TSmartPtr<CViewportDesc> > m_viewportDesc;
	//! Array of currently existing viewports.
	std::vector<CViewport*> m_viewports;
	CViewport* m_pPerspectiveViewport;
};

#endif // !defined(AFX_VIEWMANAGER_H__CE538069_BE63_4E85_93B4_74D3F72BB112__INCLUDED_)
