// TrackViewNodes.cpp : implementation file
//

#include "stdafx.h"
#include "TrackViewNodes.h"
#include "TrackViewKeyList.h"
#include "TrackViewUndo.h"
#include "StringDlg.h"

#include "IMovieSystem.h"

#include "Objects\ObjectManager.h"

// CTrackViewNodes

IMPLEMENT_DYNAMIC(CTrackViewNodes, CTreeCtrl)
CTrackViewNodes::CTrackViewNodes()
{
	m_keysCtrl = 0;
}

CTrackViewNodes::~CTrackViewNodes()
{
}


BEGIN_MESSAGE_MAP(CTrackViewNodes, CTreeCtrlEx)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnTvnItemexpanded)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(NM_RCLICK, OnNMRclick)
	ON_WM_VSCROLL()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnTvnSelchanged)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void CTrackViewNodes::RefreshNodes()
{
	SetSequence(m_sequence);
}

CTrackViewNodes::SItemInfo* CTrackViewNodes::GetSelectedNode()
{
	HTREEITEM hItem=GetSelectedItem();
	if (!hItem)
		return NULL;
	return (SItemInfo*)GetItemData(hItem);
}

void CTrackViewNodes::AddNode( IAnimNode *node )
{
	AddNodeItem( node,TVI_ROOT );
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewNodes::AddNodeItem( IAnimNode *node,HTREEITEM root )
{
	bool bNeedExpand = false;
	HTREEITEM MyRoot=root;
	CBaseObject *pBaseObject=GetIEditor()->GetObjectManager()->FindAnimNodeOwner(node);
	if (pBaseObject)
	{
		// connect to parent if it is in treeview already
		CBaseObject *pParent=pBaseObject->GetParent();
		if (pParent)
		{
			HTREEITEM hParent=GetChildNode(root, pParent->GetAnimNode());
			if (hParent!=NULL)
				MyRoot=hParent;
		}
	}
	int nNodeImage = 1;
	EAnimNodeType nodeType = node->GetType();
	switch (nodeType)
	{
	case ANODE_ENTITY:	   nNodeImage = 1; break;
	case ANODE_CAMERA:	   nNodeImage = 1; break;
	case ANODE_SCRIPTVAR:	 nNodeImage = 14; break;
	case ANODE_CVAR:       nNodeImage = 15; break;
	case ANODE_MATERIAL:   nNodeImage = 16; break;
	}
	HTREEITEM hItem = InsertItem( node->GetName(),nNodeImage,nNodeImage,MyRoot);
	//SetItemState(MyRoot, TVIS_BOLD, TVIS_BOLD);
	SetItemState(hItem, TVIS_BOLD, TVIS_BOLD);


	SItemInfo *pItemInfo = &(*m_itemInfos.insert(m_itemInfos.begin(),SItemInfo()));
	pItemInfo->node = node;
	pItemInfo->baseObj = pBaseObject;
	pItemInfo->track = 0;
	pItemInfo->paramId = 0;
	SetItemData( hItem,(DWORD_PTR)pItemInfo );

	if (MyRoot != root)
	{
		// Expand root object.
		Expand( MyRoot,TVE_EXPAND );
	}
	if (node->GetFlags()&ANODE_FLAG_EXPANDED)
	{
		bNeedExpand = true;
	}

	IAnimBlock *anim = node->GetAnimBlock();

	if (anim)
	{
		int type;
		IAnimTrack *track;
		IAnimNode::SParamInfo paramInfo;
		for (int i = 0; i < anim->GetTrackCount(); i++)
		{
			if (!anim->GetTrackInfo( i,type,&track ))
				continue;
			if (!track)
				continue;
			if (track->GetFlags() & ATRACK_HIDDEN) 
				continue;

			if (!node->GetParamInfoFromId(type,paramInfo))
				continue;

			int nImage = 13; // Default
			if (type == APARAM_FOV)
				nImage = 2;
			if (type == APARAM_POS)
				nImage = 3;
			if (type == APARAM_ROT)
				nImage = 4;
			if (type == APARAM_SCL)
				nImage = 5;
			if (type == APARAM_EVENT)
				nImage = 6;
			if (type == APARAM_VISIBLE)
				nImage = 7;
			if (type == APARAM_CAMERA)
				nImage = 8;
			if ((type >= APARAM_SOUND1) && (type <= APARAM_SOUND3))
				nImage = 9;
			if ((type >= APARAM_CHARACTER1) && (type <= APARAM_CHARACTER3))
				nImage = 10;
			if (type == APARAM_SEQUENCE)
				nImage = 11;
			if ((type >= APARAM_EXPRESSION1) && (type <= APARAM_EXPRESSION3))
				nImage = 12;
			if (type == APARAM_FLOAT_1)
				nImage = 13;

			HTREEITEM hTrackItem = InsertItem( paramInfo.name,nImage,nImage,hItem );

			SItemInfo *pItemInfo = &(*m_itemInfos.insert(m_itemInfos.begin(),SItemInfo()));
			pItemInfo->node = node;
			pItemInfo->paramId = type;
			pItemInfo->track = track;
			SetItemData( hTrackItem,(DWORD_PTR)pItemInfo );
		}
	}
	if (pBaseObject)
	{
		// connect children if they are in treeview already
		for (int i=0;i<pBaseObject->GetChildCount();i++)
		{
			CBaseObject *pChild=pBaseObject->GetChild(i);
			HTREEITEM hChild=GetChildNode(root, pChild->GetAnimNode());
			if (hChild!=NULL)
			{
				HTREEITEM hNewItem=CopyBranch(hChild, hItem);
				//SetItemState(hItem, TVIS_BOLD, TVIS_BOLD);
				SetItemState(hNewItem, TVIS_BOLD, TVIS_BOLD);
				DeleteItem(hChild);
				// If node have child nodes. expand it.
				bNeedExpand = true;
			}
		}
	}

	if (bNeedExpand)
	{
		// If node have child nodes. expand it.
		Expand( hItem,TVE_EXPAND );
	}
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewNodes::SetKeyListCtrl( CTrackViewKeys *keysCtrl )
{
	m_keysCtrl = keysCtrl;
	//SyncKeyCtrl();
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewNodes::SyncKeyCtrl()
{
	if (!m_keysCtrl)
		return;

	m_keysCtrl->ResetContent();

	if (!m_sequence)
		return;

	HTREEITEM hItem = GetFirstVisibleItem();
	while (hItem)
	{
		SItemInfo *pItemInfo = (SItemInfo*)GetItemData(hItem);
		if (pItemInfo && pItemInfo->track)
		{
			m_keysCtrl->AddItem( CTrackViewKeyList::Item(pItemInfo->node,pItemInfo->paramId,pItemInfo->track) );
		}
		else
		{
			m_keysCtrl->AddItem( CTrackViewKeyList::Item() );
		}

		hItem = GetNextVisibleItem(hItem);
	}
}
// CTrackViewNodes message handlers

void CTrackViewNodes::OnTvnItemexpanded(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	SItemInfo *pItemInfo = (SItemInfo*)GetItemData(pNMTreeView->itemNew.hItem);
	if (pItemInfo && pItemInfo->node)
	{
		IAnimNode *node = pItemInfo->node;
		if ((pNMTreeView->itemNew.state & TVIS_EXPANDED) || (pNMTreeView->itemNew.state & TVIS_EXPANDPARTIAL))
		{
			// Mark node expanded
			node->SetFlags( node->GetFlags()|ANODE_FLAG_EXPANDED );
		}
		else
		{
			// Mark node collapsed.
			node->SetFlags( node->GetFlags()&(~ANODE_FLAG_EXPANDED) );
		}
	}

	SyncKeyCtrl();

	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
int CTrackViewNodes::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create the list
	CMFCUtils::LoadTrueColorImageList( m_cImageList,IDB_TRACKVIEW_NODES,16,RGB(255,0,255) );
	SetImageList( &m_cImageList,TVSIL_NORMAL );

	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewNodes::CreateAnimNode( int type,const char *sName )
{
	IAnimNode *pAnimNode = GetIEditor()->GetSystem()->GetIMovieSystem()->CreateNode( type );
	if (pAnimNode && m_sequence->AddNode(pAnimNode))
	{
		pAnimNode->SetName( sName );
		pAnimNode->CreateDefaultTracks();
	}
	RefreshNodes();
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewNodes::SetSequence( IAnimSequence *seq )
{
	DeleteAllItems();
	m_sequence = seq;

	if (!m_sequence)
		return;

	// Create root item.
	HTREEITEM hRootItem = InsertItem( seq->GetName(),0,0,TVI_ROOT );
	SetItemData(hRootItem,(DWORD_PTR)0);
	SetItemState(hRootItem, TVIS_BOLD, TVIS_BOLD);

	for (int i = 0; i < seq->GetNodeCount(); i++)
	{
		IAnimNode *node = seq->GetNode(i);
		AddNodeItem( node,hRootItem );
	}
	Expand( hRootItem,TVE_EXPAND );
	SyncKeyCtrl();
}

void CTrackViewNodes::OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	CPoint point;

	SItemInfo *pItemInfo = 0;

	if (!m_sequence)
		return;

	// Find node under mouse.
	GetCursorPos( &point );
	ScreenToClient( &point );
	// Select the item that is at the point myPoint.
	UINT uFlags;
	HTREEITEM hItem = HitTest(point,&uFlags);
	if ((hItem != NULL) && (TVHT_ONITEM & uFlags))
	{
		pItemInfo = (SItemInfo*)GetItemData(hItem);
	}

	// Create pop up menu.
	CMenu menu;
	CMenu menuAddTrack;
	
	menu.CreatePopupMenu();

	if (!pItemInfo || !pItemInfo->node)
	{
		menu.AppendMenu( MF_STRING,500,_T("Add Selected Entity Node") );
		menu.AppendMenu( MF_STRING,501,_T("Add Scene Node") );
		menu.AppendMenu( MF_STRING,502,_T("Add Console Variable Node") );
		menu.AppendMenu( MF_STRING,503,_T("Add Script Variable Node") );
		menu.AppendMenu( MF_STRING,504,_T("Add Material Node") );
	}
	else
	{
		if (pItemInfo && pItemInfo->node && pItemInfo->track == 0)
		{
			menu.AppendMenu( MF_STRING,10,"Remove Node" );
		}
		if (pItemInfo && pItemInfo->node != 0)
		{
			if (pItemInfo->node->GetFlags() & ANODE_FLAG_CAN_CHANGE_NAME)
			{
				menu.AppendMenu( MF_STRING,11,"Rename Node" );
			}
		}

		menu.AppendMenu( MF_SEPARATOR,0,"" );
	}

	// add tracks menu
	menuAddTrack.CreatePopupMenu();
	bool bTracksToAdd=false;
	if (pItemInfo && pItemInfo->node != 0)
	{
		IAnimNode::SParamInfo paramInfo;
		// List`s which tracks can be added to animation node.
		for (int i = 0; i < pItemInfo->node->GetParamCount(); i++)
		{
			if (!pItemInfo->node->GetParamInfo( i,paramInfo ))
				continue;
				
			int flags = 0;
			IAnimTrack *track = pItemInfo->node->GetTrack( paramInfo.paramId );
			if (track)
			{
				continue;
				//flags |= MF_CHECKED;
			}

			menuAddTrack.AppendMenu( MF_STRING|flags,1000+paramInfo.paramId,paramInfo.name );
			bTracksToAdd=true;
		}
	}
	if (bTracksToAdd)
		menu.AppendMenu(MF_POPUP,(UINT_PTR)menuAddTrack.m_hMenu,"Add Track");

	// delete track menu
	if (pItemInfo && pItemInfo->node && pItemInfo->track)
	{
		menu.AppendMenu(MF_STRING, 299, "Remove Track");
	}

	if (bTracksToAdd || (pItemInfo && pItemInfo->track))
		menu.AppendMenu( MF_SEPARATOR,0,"" );

	if (pItemInfo && pItemInfo->node != 0)
	{
		CString str;
		str.Format( "%s Tracks",pItemInfo->node->GetName() );
		menu.AppendMenu( MF_STRING|MF_DISABLED,0,str );

		// Show tracks in anim node.
		IAnimBlock *anim = pItemInfo->node->GetAnimBlock();
		if (anim)
		{
			IAnimNode::SParamInfo paramInfo;
			int type;
			IAnimTrack *track;
			for (int i = 0; i < anim->GetTrackCount(); i++)
			{
				if (!anim->GetTrackInfo( i,type,&track ))
					continue;
				if (!pItemInfo->node->GetParamInfo( type,paramInfo ))
					continue;
			
				// change hidden flag for this track.
				int checked = MF_CHECKED;
				if (track->GetFlags() & ATRACK_HIDDEN)
				{
					checked = MF_UNCHECKED;
				}
				menu.AppendMenu( MF_STRING|checked,100+i,CString( "  " ) + paramInfo.name );
			}
		}
	}

	GetCursorPos( &point );
	int cmd = menu.TrackPopupMenu( TPM_RETURNCMD|TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,point.y,this->GetParent()->GetParent() );
	
	//////////////////////////////////////////////////////////////////////////
	// Check Remove Node.
	if (cmd == 10)
	{
		// Remove node under mouse.
		if (pItemInfo)
		{
			CUndo undo("Remove Anim Node");
			CUndo::Record( new CUndoAnimSequenceObject(m_sequence) );

			m_sequence->RemoveNode(pItemInfo->node);
			RefreshNodes();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Add scene node.
	if (cmd == 500)
	{
		AddSelectedNodes();
	}
	//////////////////////////////////////////////////////////////////////////
	// Add scene node.
	if (cmd == 501)
	{
		m_sequence->AddSceneNode();
		RefreshNodes();
	}
	//////////////////////////////////////////////////////////////////////////
	// Add cvar node.
	if (cmd == 502)
	{
		CStringDlg dlg( _T("Console Variable Name") );
		if (dlg.DoModal() == IDOK && !dlg.GetString().IsEmpty())
		{
			CreateAnimNode( ANODE_CVAR,dlg.GetString() );
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// Add script var node.
	if (cmd == 503)
	{
		CStringDlg dlg( _T("Script Variable Name") );
		if (dlg.DoModal() == IDOK && !dlg.GetString().IsEmpty())
		{
			CreateAnimNode( ANODE_SCRIPTVAR,dlg.GetString() );
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// Add Material node.
	if (cmd == 504)
	{
		CStringDlg dlg( _T("Material Name") );
		if (dlg.DoModal() == IDOK && !dlg.GetString().IsEmpty())
		{
			CreateAnimNode( ANODE_MATERIAL,dlg.GetString() );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Check Rename Node.
	if (cmd == 11 && pItemInfo && pItemInfo->node)
	{
		// Rename node under cursor.
		CStringDlg dlg( _T("Rename Node") );
		dlg.SetString(pItemInfo->node->GetName());
		if (dlg.DoModal() == IDOK)
		{
			pItemInfo->node->SetName( dlg.GetString() );
			RefreshNodes();
		}
	}

	if (cmd >= 1000 && cmd < 2000)
	{
		if (pItemInfo)
		{
			IAnimNode *node = pItemInfo->node;
			IAnimBlock *anim = node->GetAnimBlock();
			if (anim)
			{
				CUndo undo("Create Anim Track");
				CUndo::Record( new CUndoAnimSequenceObject(m_sequence) );

				int paramId = cmd-1000;
				node->CreateTrack( paramId );
				ExpandNode(node);
			}
		}
	}

	if (cmd == 299)
	{
		if (pItemInfo)
		{
			IAnimNode *node = pItemInfo->node;
			IAnimBlock *anim = node->GetAnimBlock();
			if (anim)
			{
				if (AfxMessageBox("Are you sure you want to delete this track ? Undo will not be available !", MB_ICONQUESTION | MB_YESNO)==IDYES)
				{
					CUndo undo("Remove Anim Track");
					CUndo::Record( new CUndoAnimSequenceObject(m_sequence) );

					node->RemoveTrack(pItemInfo->track);
					ExpandNode(node);
				}
			}
		}
	}

	if (cmd >= 100 && cmd < 200)
	{
		if (pItemInfo)
		{
			IAnimNode *node = pItemInfo->node;
			IAnimBlock *anim = node->GetAnimBlock();
			if (anim)
			{
				CUndo undo("Modify Sequence");
				CUndo::Record( new CUndoAnimSequenceObject(m_sequence) );

				int type;
				IAnimTrack *track;
				for (int i = 0; i < anim->GetTrackCount(); i++)
				{
					if (!anim->GetTrackInfo( i,type,&track ))
						continue;

					if (cmd-100 == i)
					{
						// change hidden flag for this track.
						if (track->GetFlags() & ATRACK_HIDDEN)
							track->SetFlags( track->GetFlags() & ~ATRACK_HIDDEN );
						else
							track->SetFlags( track->GetFlags() | ATRACK_HIDDEN );
					}
					RefreshNodes();
					ExpandNode(  node );
					break;
				}
			}
		}
	}

	// processed
	*pResult = 1;
}

void CTrackViewNodes::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CTreeCtrl::OnVScroll(nSBCode, nPos, pScrollBar);

	SyncKeyCtrl();
}

void CTrackViewNodes::OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if (m_keysCtrl)
	{
		SItemInfo *pItemInfo = (SItemInfo*)GetItemData(pNMTreeView->itemNew.hItem);
		if (pItemInfo && pItemInfo->track)
		{
			for (int i = 0; i < m_keysCtrl->GetCount(); i++)
			{
				if (m_keysCtrl->GetTrack(i) == pItemInfo->track)
				{
					m_keysCtrl->SetCurSel(i);
					break;
				}
			}
		}
		else
		{
			m_keysCtrl->SetCurSel(-1);
		}
	}
	
	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
HTREEITEM CTrackViewNodes::GetChildNode( HTREEITEM hItem,IAnimNode *node )
{
	// Look at all of the root-level items
	while (hItem != NULL)
	{
		SItemInfo *pItemInfo = (SItemInfo*)GetItemData(hItem);
		if (pItemInfo)
		{
			if (pItemInfo->node == node && pItemInfo->track == 0)
			{
				return hItem;
			}
		}
		HTREEITEM hChild = GetNextItem( hItem, TVGN_CHILD);
		HTREEITEM hRes=GetChildNode(hChild,node);
		if (hRes!=NULL)
			return hRes;
		hItem = GetNextItem( hItem, TVGN_NEXT);
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
bool CTrackViewNodes::ExpandChildNode( HTREEITEM hItem,IAnimNode *node )
{
	// Look at all of the root-level items
	while (hItem != NULL)
	{
		SItemInfo *pItemInfo = (SItemInfo*)GetItemData(hItem);
		if (pItemInfo)
		{
			if (pItemInfo->node == node && pItemInfo->track == 0)
			{
				HTREEITEM hParent = GetParentItem(hItem);
				if (hParent != NULL)
					Expand(hParent, TVE_EXPAND);

				Expand( hItem,TVE_EXPAND );
				EnsureVisible(hItem);
				return true;
			}
		}
		HTREEITEM hChild = GetNextItem( hItem, TVGN_CHILD);
		if (ExpandChildNode(hChild,node))
			return true;
		hItem = GetNextItem( hItem, TVGN_NEXT);
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewNodes::ExpandNode( IAnimNode *node )
{
	// Look at all of the root-level items
	HTREEITEM hCurrent = GetRootItem();
	ExpandChildNode( GetNextItem(GetRootItem(),TVGN_CHILD),node );
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewNodes::AddSelectedNodes()
{
	CUndo undo("Add Anim Node(s)");
	CUndo::Record( new CUndoAnimSequenceObject(m_sequence) );

	// Add selected nodes.
	CSelectionGroup *sel = GetIEditor()->GetSelection();
	for (int i = 0; i < sel->GetCount(); i++)
	{
		CBaseObject *obj = sel->GetObject(i);
		if (obj && obj->GetAnimNode())
		{
			IAnimNode *pAnimNode = obj->GetAnimNode();
			
			if (!m_sequence->AddNode(pAnimNode))
				continue;

			IAnimBlock *pAnimBlock=pAnimNode->GetAnimBlock();
			if (pAnimBlock)		// lets add basic tracks
			{
				pAnimNode->CreateDefaultTracks();
			}
			// Force default parameters.
			pAnimNode->SetPos(0,obj->GetPos());
			Quat rotate;
			rotate.SetRotationXYZ( DEG2RAD(obj->GetAngles()));
			pAnimNode->SetRotate(0,rotate);
			pAnimNode->SetScale(0,obj->GetScale());
		}
	}
	RefreshNodes();
	for (int i = 0; i < sel->GetCount(); i++)
	{
		CBaseObject *obj = sel->GetObject(i);
		if (obj && obj->GetAnimNode())
			ExpandNode(  obj->GetAnimNode() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CTrackViewNodes::AddSceneNodes()
{
	CUndo undo("Add Scene Node");
	CUndo::Record( new CUndoAnimSequenceObject(m_sequence) );

	IAnimNode *pSceneNode=m_sequence->AddSceneNode();
	if (!pSceneNode)
		AfxMessageBox("Scene-track already exists.", MB_ICONEXCLAMATION | MB_OK);
	else
	{
		pSceneNode->CreateDefaultTracks();
		RefreshNodes();
		ExpandNode(pSceneNode);
	}
}

//////////////////////////////////////////////////////////////////////////
// Select object in Editor with Double click.
//////////////////////////////////////////////////////////////////////////
void CTrackViewNodes::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	CPoint point;
	SItemInfo *pItemInfo = 0;

	if (!m_sequence)
		return;

	// Find node under mouse.
	GetCursorPos( &point );
	ScreenToClient( &point );
	// Select the item that is at the point myPoint.
	UINT uFlags;
	HTREEITEM hItem = HitTest(point,&uFlags);
	if ((hItem != NULL) && (TVHT_ONITEM & uFlags))
	{
		pItemInfo = (SItemInfo*)GetItemData(hItem);
	}

	if (pItemInfo && pItemInfo->node)
	{
		// Double Clicked on node or track.
		// Make this object selected in Editor.
		CBaseObject *obj = GetIEditor()->GetObjectManager()->FindAnimNodeOwner(pItemInfo->node);
		if (obj)
		{
			GetIEditor()->ClearSelection();
			GetIEditor()->SelectObject(obj);

			// Disable expanding on dblclick.
			*pResult = TRUE;
		}
	}
}

void CTrackViewNodes::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_DELETE)
	{
		if (!m_sequence)
			return;
		SItemInfo *pItemInfo = GetSelectedNode();
		if (pItemInfo)
		{
			if (pItemInfo->node && pItemInfo->track)
			{
				// Delete track.
				if (AfxMessageBox("Delete Selected Track?", MB_ICONQUESTION|MB_YESNO)==IDYES)
				{
					IAnimNode *node = pItemInfo->node;

					CUndo undo("Remove Anim Track");
					CUndo::Record( new CUndoAnimSequenceObject(m_sequence) );

					node->RemoveTrack( pItemInfo->track );
					ExpandNode( node );
				}
			}
			else if (pItemInfo->node && pItemInfo->track == 0)
			{
				// Delete node.
				if (AfxMessageBox("Delete Selected Animation Node?", MB_ICONQUESTION|MB_YESNO)==IDYES)
				{
					CUndo undo("Remove Anim Node");
					CUndo::Record( new CUndoAnimSequenceObject(m_sequence) );

					m_sequence->RemoveNode(pItemInfo->node);
					RefreshNodes();
				}
			}
		}
	}
	else
    CTreeCtrlEx::OnKeyDown(nChar, nRepCnt, nFlags);
}
