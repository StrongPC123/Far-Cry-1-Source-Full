////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   trackviewnodes.h
//  Version:     v1.00
//  Created:     29/4/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: TrackView's tree control.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __trackviewnodes_h__
#define __trackviewnodes_h__

#include "Controls\TreeCtrlEx.h"

#if _MSC_VER > 1000
#pragma once
#endif


// forward declarations.
struct IAnimNode;
struct IAnimTrack;
struct IAnimSequence;
class CTrackViewKeys;

// CTrackViewNodes

class CTrackViewNodes : public CTreeCtrlEx
{
	DECLARE_DYNAMIC(CTrackViewNodes)

public:
	struct SItemInfo
	{
	//		int type;
		IAnimNode *node;
		CBaseObject *baseObj;
		int paramId;
		IAnimTrack *track;
	};

	CTrackViewNodes();
	virtual ~CTrackViewNodes();

	void SetSequence( IAnimSequence *seq );
	void SetKeyListCtrl( CTrackViewKeys *keysCtrl );
	SItemInfo* GetSelectedNode();
	//! Synchronize visual appearance of nodes view with track view.
	void SyncKeyCtrl();
	void ExpandNode( IAnimNode *node );
	void AddSelectedNodes();
	void AddSceneNodes();
	void CreateAnimNode( int type,const char *sName );

protected:
	DECLARE_MESSAGE_MAP()

	void RefreshNodes();

	HTREEITEM GetChildNode( HTREEITEM hItem,IAnimNode *node );
	bool ExpandChildNode( HTREEITEM hItem,IAnimNode *node );

	void AddNode( IAnimNode *node );
	void AddNodeItem( IAnimNode *node,HTREEITEM root );

	IAnimSequence *m_sequence;
	CTrackViewKeys* m_keysCtrl;

	// Must not be vector, vector may invalidate pointers on add/remove.
	typedef std::list<SItemInfo> ItemInfos;
	ItemInfos m_itemInfos;

	CImageList m_cImageList;
	
	afx_msg void OnTvnItemexpanded(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#endif // __trackviewnodes_h__