////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   TrackViewKeys.h
//  Version:     v1.00
//  Created:     23/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __trackviewkeys_h__
#define __trackviewkeys_h__

#if _MSC_VER > 1000
#pragma once
#endif

class CTVTrackPropsDialog;

// CTrackViewKeys

struct IAnimTrack;
struct IAnimNode;

enum ETVActionMode
{
	TVMODE_MOVEKEY = 1,
	TVMODE_ADDKEY,
	TVMODE_SLIDEKEY,
	TVMODE_SCALEKEY,
};

/** Base class for TrackView key editing dialogs.
*/
class CTrackViewKeys : public CWnd
{
	DECLARE_DYNAMIC(CTrackViewKeys)
public:
	struct Item
	{
		IAnimTrack *track;
		IAnimNode *node;
		int paramId;
		Item() { track = 0; node = 0; paramId = 0; }
		Item( IAnimNode *pNode,int nParamId,IAnimTrack *pTrack )
		{
			track = pTrack; paramId = nParamId; node = pNode;
		}
	};

	CTrackViewKeys();
	virtual ~CTrackViewKeys();

	void SetTimeScale( float timeScale );
	float GetTimeScale() { return m_timeScale; }

	void SetTimeRange( float start,float end );
	void SetCurrTime( float time );
	void SetStartMarker(float fTime);
	void SetEndMarker(float fTime);

	void DelSelectedKeys();
	void SetMouseActionMode( ETVActionMode mode );

	bool CanCopyPasteKeys();
	bool CopyPasteKeys();

	void SetTrackDialog( CTVTrackPropsDialog *dlg ) { m_wndTrack = dlg; };

	//////////////////////////////////////////////////////////////////////////
	// Tracks access.
	//////////////////////////////////////////////////////////////////////////
	int GetCount() const { return m_tracks.size(); }
	void AddItem( const Item &item );
	const Item& GetItem( int item );
	IAnimTrack* GetTrack( int item );
	int GetCurSel() { return m_selected; };
	int GetHorizontalExtent() { return m_itemWidth; };	
	
	virtual void	SetHorizontalExtent( int min,int max );
	virtual int		ItemFromPoint( CPoint pnt );
	virtual int		GetItemRect( int item,CRect &rect );
	virtual void	ResetContent() { m_tracks.clear(); }
	virtual void	SetCurSel( int sel );

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT /*lpMeasureItemStruct*/);
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);

	//////////////////////////////////////////////////////////////////////////
	// Drawing methods.
	//////////////////////////////////////////////////////////////////////////
	virtual void DrawControl( CDC *dc,const CRect &rcUpdate );
	virtual void DrawTrack( int item,CDC *dc,CRect &rcItem );
	virtual void DrawTicks( CDC *dc,CRect &rc,Range &timeRange );
	virtual void DrawTimeline( CDC *dc,const CRect &rcUpdate );
	virtual void DrawKeys( IAnimTrack *track,CDC *dc,CRect &rc,Range &timeRange );
	virtual void RedrawItem( int item );

	//////////////////////////////////////////////////////////////////////////
	// Must be overriden.
	//////////////////////////////////////////////////////////////////////////
	//! Find a key near this point.
	virtual int KeyFromPoint( CPoint point );
	//! Select keys inside this client rectangle.
	virtual void SelectKeys( const CRect &rc );

	//////////////////////////////////////////////////////////////////////////
	//! Return time snapped to timestep,
	float SnapTime( float time );

	//! Returns visible time range.
	Range GetVisibleRange();
	Range GetTimeRange( CRect &rc );

	//! Return client position for givven time.
	int TimeToClient( float time );

	float TimeFromPoint( CPoint point );
	float TimeFromPointUnsnapped( CPoint point );

	//! Unselect all selected keys.
	void UnselectAllKeys();
	//! Offset all selected keys by this offset.
	void OffsetSelectedKeys( float timeOffset,bool bSlide,bool bSnapKeys );
	//! Scale all selected keys by this offset.
	void ScaleSelectedKeys( float timeOffset,bool bSnapKeys );
	void CloneSelectedKeys();

	bool FindSingleSelectedKey( IAnimTrack* &track,int &key );

	//////////////////////////////////////////////////////////////////////////
	void SetKeyInfo( IAnimTrack *track,int key,bool openWindow=false );

	void UpdateAnimation();
	void SetLeftOffset( int ofs ) { m_leftOffset = ofs; };

	void SetMouseCursor( HCURSOR crs );

	void RecordTrackUndo( const Item &item );

	//////////////////////////////////////////////////////////////////////////
	// FIELDS.
	//////////////////////////////////////////////////////////////////////////

	IAnimTrack *m_track;
	IAnimNode *m_node;
	int m_paramId;
	
	//////////////////////////////////////////////////////////////////////////
	CBrush m_bkgrBrush;
	CBrush m_bkgrBrushEmpty;
	CBrush m_selectedBrush;
	CBrush m_timeBkgBrush;
	CBrush m_timeHighlightBrush;
	CBrush m_visibilityBrush;

	HCURSOR m_currCursor;
	HCURSOR m_crsLeftRight;
	HCURSOR m_crsAddKey;

	CRect m_rcClient;
	CPoint m_scrollOffset;
	CRect m_rcSelect;
	CRect m_rcTimeline;

	CPoint m_mouseDownPos;
	CImageList m_imageList;
	CImageList m_imgMarker;

	CBitmap m_offscreenBitmap;

	bool m_bZoomDrag;
	bool m_bMoveDrag;

	//////////////////////////////////////////////////////////////////////////
	// Time.
	float m_timeScale;
	float m_currentTime;
	Range m_timeRange;
	Range m_realTimeRange;
	Range m_timeMarked;

	//! This is how often to place ticks.
	//! value of 10 means place ticks every 10 second.
	int m_ticksStep;

	//////////////////////////////////////////////////////////////////////////
	int m_mouseMode;
	int m_mouseActionMode;
	int m_actionMode;
	bool m_bAnySelected;
	float m_keyTimeOffset;

	CTVTrackPropsDialog *m_wndTrack;

	CFont *m_descriptionFont;

	//////////////////////////////////////////////////////////////////////////
	// Track list related.
	//////////////////////////////////////////////////////////////////////////
	std::vector<Item> m_tracks;
	int m_itemWidth;
	int m_itemHeight;
	// Current selected item.
	int m_selected;

	int m_leftOffset;
	int m_scrollMin,m_scrollMax;
};

#endif // __trackviewkeys_h__