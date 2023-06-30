// MltiTree.cpp : implementation file
// Copyright (c) 1999 Richard Hazlewood
// This code is provided as-is.  Use at your own peril.
//
// Multi-selection tree. Based, for the most part, on the
// selection behaviour of the listview control.
// TVN_SELCHANGING/TVN_SELCHANGED notifications are used
//  throughout: itemOld For de-selection, itemNew for selection.
// Note: TVN_SELCHANGING/TVN_SELCHANGED are still sent by default
//       tree processing for focus changes, i.e. a SetItemState passed
//       TVIS_FOCUSED without TVIS_SELECTED will still cause notification
//       (if not already focused)

//Decoding in TVN_SELCHANGED:
//B = IsEmulatedNotify
//O = itemOld.hItem != 0
//N = itemNew.hItem != 0
//
//B  O  N
//~~~~~~~
//0  1  0	A focus loss on itemOld
//0  0  1	A focus/selection gain on itemNew
//0  1  1	A focus loss on itemOld, a focus/selection gain on itemNew
//1  1  0	A selection loss on itemOld
//1  0  1	A selection gain on itemNew
//else undefined

#include "stdafx.h"
#include <windowsx.h>
#include "MltiTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

#ifndef MST_TIMER_PERIOD
#define MST_TIMER_PERIOD	75		//ms
#endif

/////////////////////////////////////////////////////////////////////////////
// CMultiTree

IMPLEMENT_DYNAMIC(CMultiTree, CMultiTree_BASE)

CMultiTree::CMultiTree()
{
	m_bMulti = TRUE;
	m_hSelect = NULL;
	m_bBandLabel = FALSE;
	m_bEmulated = FALSE;
}

CMultiTree::~CMultiTree()
{
}

#define CTreeCtrl	CMultiTree_BASE
BEGIN_MESSAGE_MAP(CMultiTree, CTreeCtrl)
#undef CTreeCtrl
	//{{AFX_MSG_MAP(CMultiTree)
	ON_WM_LBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY_REFLECT_EX(TVN_ITEMEXPANDING, OnItemExpanding)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiTree message handlers

/////////////////////////////////////////////////////////////////////////////
// GetSelectedCount
// - returns number of selection tree items

UINT CMultiTree::GetSelectedCount() const
{
	UINT nCount = 0;
	HTREEITEM hItem = GetFirstSelectedItem();
	while (hItem) {
		nCount++;
		hItem = GetNextSelectedItem(hItem);
	}
	return nCount;
}

/////////////////////////////////////////////////////////////////////////////
// SetMultiSelect
// - allow mode to be turned off

BOOL CMultiTree::SetMultiSelect(BOOL bMulti)
{
	BOOL b = m_bMulti;
	m_bMulti = bMulti;
	if (!m_bMulti) {
		HTREEITEM hItem = GetSelectedItem();
		if (hItem && !IsSelected(hItem))
			hItem = NULL;
		SelectAllIgnore(FALSE, hItem);
		if (hItem)
			SelectItem(hItem);
	}
	return b;
}

/////////////////////////////////////////////////////////////////////////////
// SetItemState
// - replacement to handle TVIS_FOCUSED

BOOL CMultiTree::SetItemState(HTREEITEM hItem, UINT nState, UINT nStateMask)
{
	ASSERT(hItem);

	if (!m_bMulti)
		return CMultiTree_BASE::SetItemState(hItem, nState, nStateMask);

	HTREEITEM hFocus = GetSelectedItem();		//current focus
	BOOL bWasFocus = (hFocus == hItem);
	BOOL bFocusWasSel = hFocus && IsSelected(hFocus);	//selection state of current focus
	BOOL bWasSel = IsSelected(hItem);		//select state of acting item

	UINT nS = nState & ~TVIS_FOCUSED;
	UINT nSM = nStateMask & ~TVIS_FOCUSED;

	if (nStateMask & TVIS_FOCUSED) {
		//wanted to affect focus
		if (nState & TVIS_FOCUSED) {
			//wanted to set focus
			if (!bWasFocus && bFocusWasSel) {
				//because SelectItem would de-select the current 'real' selection
				// (the one with focus), need to make the tree ctrl think there is
				// no 'real' selection but still keep the the old item selected
				//it has to be done before the SelectItem call because
				// otherwise the TVN_SELCHANGING/ED notification handlers
				// wouldn't be able to get the proper list of selected items
				CMultiTree_BASE::SelectItem(NULL);	//will cause notify, but can be taken as focus change
				CMultiTree_BASE::SetItemState(hFocus, TVIS_SELECTED, TVIS_SELECTED);
				UpdateWindow();
			}
			if (!CMultiTree_BASE::SelectItem(hItem))	//set focus (will consequently select, if not already focused)
				return FALSE;
			if (nStateMask & TVIS_SELECTED) {
				//wanted to affect select state
				if (nState & TVIS_SELECTED) {
					//wanted to select, already done if wasn't focused
					if (!bWasFocus || bFocusWasSel) {
						nS &= ~TVIS_SELECTED;
						nSM &= ~TVIS_SELECTED;
					}
				}
				//else wanted to clear, base call will do that
			}
			else {
				//didn't want to affect select state
				if (!bWasSel) {
					//it wasn't previously selected, let base clear (correct)
					nS &= ~TVIS_SELECTED;
					nSM |= TVIS_SELECTED;
				}
				//else was already selected, no harm done
			}
		}
		else {
			//wanted to clear focus
			if (bWasFocus) {
				//it had the focus
				CMultiTree_BASE::SelectItem(NULL);	//clear focus
				if (!(nStateMask & TVIS_SELECTED)) {
					//didn't want to affect select state
					if (bWasSel) {
						//it was selected, so restore
						ASSERT( !(nS & TVIS_SELECTED) );
						ASSERT( !(nSM & TVIS_SELECTED) );
						//set state here, to avoid double-notify
						CMultiTree_BASE::SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
						//let base do other states
					}
				}
				else if (nState & TVIS_SELECTED) {
					//wanted to select (but clear focus)
					if (bWasSel) {
						//if was selected, restore
						CMultiTree_BASE::SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
					}
					//don't want to notify, default did it
					nS &= ~TVIS_SELECTED;
					nSM &= ~TVIS_SELECTED;
				}
			}
		}
	}

	if (!nSM)
		return TRUE;	//no other states to alter

	if (nSM & TVIS_SELECTED) {
		//still need to alter selection state
		NMTREEVIEW nmtv;
		nmtv.hdr.hwndFrom = m_hWnd;
		nmtv.hdr.idFrom = ::GetDlgCtrlID(m_hWnd);
		nmtv.hdr.code = TVN_SELCHANGING;
		nmtv.itemOld.mask = nmtv.itemNew.mask = 0;
		nmtv.itemOld.hItem = nmtv.itemNew.hItem = NULL;
		TVITEM& item = (nS & TVIS_SELECTED) ? nmtv.itemNew : nmtv.itemOld;
		item.mask = TVIF_HANDLE|TVIF_PARAM;
		item.hItem = hItem;
		item.lParam = GetItemData(hItem);
		if (_SendNotify(&nmtv.hdr))
			return FALSE;	//sel-changing stopped
		VERIFY( CMultiTree_BASE::SetItemState(hItem, nS, nSM) );
		nmtv.hdr.code = TVN_SELCHANGED;
		_SendNotify(&nmtv.hdr);
		nS &= ~TVIS_SELECTED;
		nSM &= ~TVIS_SELECTED;
	}
	if (!nSM)
		return TRUE;
	return CMultiTree_BASE::SetItemState(hItem, nS, nSM);
}

UINT CMultiTree::GetItemState(HTREEITEM hItem, UINT nStateMask) const
{
	UINT n = CMultiTree_BASE::GetItemState(hItem, nStateMask & ~TVIS_FOCUSED);
	if (nStateMask & TVIS_FOCUSED)
		if (GetSelectedItem() == hItem)
			n |= TVIS_FOCUSED;
	return n;
}

BOOL CMultiTree::SelectItem(HTREEITEM hItem)
{
	if (m_bMulti) {
		//TRACE(_T("Use SetItemState or FocusItem when in multi-select mode\n"));
		ASSERT(FALSE);
	}
	return CMultiTree_BASE::SelectItem(hItem);
}

BOOL CMultiTree::FocusItem(HTREEITEM hItem)
{
	ASSERT(m_bMulti);

	BOOL bRet = FALSE;
	if (hItem)
		bRet = SetItemState(hItem, TVIS_FOCUSED, TVIS_FOCUSED);
	else {
		hItem = CMultiTree_BASE::GetSelectedItem();
		if (hItem)
			bRet = SetItemState(hItem, 0, TVIS_FOCUSED);
	}
	return bRet;
}

/////////////////////////////////////////////////////////////////////////////
// _SendNotify
// - helper to distinguish between default control generated notifications
//   and this classes emulated ones (so can tell if focus or select notify)

BOOL CMultiTree::_SendNotify(LPNMHDR pNMHDR)
{
	ASSERT(::GetParent(m_hWnd));	//never expected this

	BOOL b = m_bEmulated;
	m_bEmulated = TRUE;
	BOOL bRes = ::SendMessage(::GetParent(m_hWnd), WM_NOTIFY, (WPARAM)pNMHDR->idFrom, (LPARAM)pNMHDR);
	m_bEmulated = b;
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////
// Selection Parsing

HTREEITEM CMultiTree::GetFirstSelectedItem() const
{
	HTREEITEM hItem = GetRootItem();
	while (hItem) {
		if (IsSelected(hItem))
			break;
		hItem = GetNextVisibleItem(hItem);
	}
	return hItem;
}

HTREEITEM CMultiTree::GetNextSelectedItem(HTREEITEM hItem) const
{
	hItem = GetNextVisibleItem(hItem);
	while (hItem) {
		if (IsSelected(hItem))
			break;
		hItem = GetNextVisibleItem(hItem);
	}
	return hItem;
}

void CMultiTree::SelectAll(BOOL bSelect /*= TRUE*/)
{
	bSelect = !!bSelect;	//ensure 0 or 1
	UINT nState = bSelect ? TVIS_SELECTED : 0;
	HTREEITEM hItem = GetRootItem();
	while (hItem) {
		if (IsSelected(hItem) != bSelect)
			SetItemState(hItem, nState, TVIS_SELECTED);
		hItem = GetNextVisibleItem(hItem);
	}
}

void CMultiTree::SelectAllIgnore(BOOL bSelect, HTREEITEM hIgnore)
{
	//special case to avoid multiple notifications for
	// the same item
	bSelect = !!bSelect;	//ensure 0 or 1
	UINT nState = bSelect ? TVIS_SELECTED : 0;
	HTREEITEM hItem = GetRootItem();
	while (hItem) {
		if (hItem != hIgnore)
			if (IsSelected(hItem) != bSelect)
				SetItemState(hItem, nState, TVIS_SELECTED);
		hItem = GetNextVisibleItem(hItem);
	}
}

void CMultiTree::SelectRange(HTREEITEM hFirst, HTREEITEM hLast, BOOL bOnly /*= TRUE*/)
{
	//locate (and select) either first or last
	// (so order is arbitrary)
	HTREEITEM hItem = GetRootItem();
	while (hItem) {
		if ((hItem == hFirst) || (hItem == hLast)) {
			if (hFirst != hLast) {
				if (!IsSelected(hItem))
					SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
				hItem = GetNextVisibleItem(hItem);
			}
			break;
		}

		if (bOnly && IsSelected(hItem))
			SetItemState(hItem, 0, TVIS_SELECTED);
		hItem = GetNextVisibleItem(hItem);
	}
	//select rest of range
	while (hItem) {
		if (!IsSelected(hItem))
			SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
		if ((hItem == hFirst) || (hItem == hLast)) {
			hItem = GetNextVisibleItem(hItem);
			break;
		}
		hItem = GetNextVisibleItem(hItem);
	}
	if (!bOnly)
		return;
	while (hItem) {
		if (IsSelected(hItem))
			SetItemState(hItem, 0, TVIS_SELECTED);
		hItem = GetNextVisibleItem(hItem);
	}
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonDown

#define _bShift	(nFlags & MK_SHIFT)
#define _bCtrl	(nFlags & MK_CONTROL)


void CMultiTree::OnLButtonDown(UINT nFlags, CPoint point) 
{
	OnButtonDown(TRUE, nFlags, point);
}

void CMultiTree::OnRButtonDown(UINT nFlags, CPoint point) 
{
	OnButtonDown(FALSE, nFlags, point);
}

void CMultiTree::OnButtonDown(BOOL bLeft, UINT nFlags, CPoint point)
{
	UINT nHF;
	HTREEITEM hItem;

	//if (::GetFocus() != m_hWnd)
		//::SetFocus(m_hWnd);

	BOOL bBase = !m_bMulti;
	if (!bBase) {
		hItem = HitTest(point, &nHF);
		if (hItem) {
			//base always handles expanding items
			//(doesn't really mean much to right button, but pass anyway)
			bBase = (nHF & (TVHT_ONITEMBUTTON/*|TVHT_ONITEMINDENT*/));
			if (!bBase && bLeft && (GetStyle() & TVS_CHECKBOXES)) {
				//when the tree has check-boxes, the default handler makes
				// a quick selection of the clicked item, then re-selects
				// the previously selected item - to cause a sel-changed
				// notification.  Fortunately it doesn't affect the multi-
				// selection, so can pass on.
				bBase = (nHF & TVHT_ONITEMSTATEICON);

#ifdef _MST_MULTI_CHECK
				//Use the above define if you want all selected items to
				// be checked the same when any one of them is checked
				// - Interestingly this doesn't happen in the listview control
				//  (LVS_EX_CHECKBOXES)
				if (bBase) {
					//the default selection notification would mess
					// the multi-selection up, so generate the notification
					// manually
					// (anyway, this is smoother than the selection flicker
					//  the default gives)
					NMTREEVIEW nmtv;
#ifdef TVN_CHKCHANGE
					nmtv.hdr.code = TVN_CHKCHANGE;
#else
					nmtv.hdr.code = TVN_SELCHANGED;
#endif
					nmtv.hdr.hwndFrom = m_hWnd;
					nmtv.hdr.idFrom = ::GetDlgCtrlID(m_hWnd);
					nmtv.itemOld.hItem = NULL;
					nmtv.itemNew.mask = TVIF_HANDLE|TVIF_PARAM;

					BOOL bChk = !GetCheck(hItem);
					if (IsSelected(hItem)) {
						HTREEITEM h = GetFirstSelectedItem();
						while (h) {
							if (!GetCheck(h) == bChk) {		//! to ensure 0 or 1
								SetCheck(h, bChk);
#ifdef TVN_CHKCHANGE
								//only send multiple check-change
								// notifications (not sel-changed)
								if (h != hItem) {		//clicked item will be done last
									nmtv.itemNew.hItem = h;
									nmtv.itemNew.lParam = GetItemData(h);
									_SendNotify(&nmtv.hdr);
								}
#endif
							}
							h = GetNextSelectedItem(h);
						}
					}
					else
						SetCheck(hItem, bChk);
					//notify clicked item
					nmtv.itemNew.hItem = hItem;
					nmtv.itemNew.lParam = GetItemData(hItem);
					_SendNotify(&nmtv.hdr);
					return;
				}
#endif

			}
		}
	}
	if (bBase) {
		if (bLeft)
			CMultiTree_BASE::OnLButtonDown(nFlags, point);
		else
			CMultiTree_BASE::OnRButtonDown(nFlags, point);
		return;
	}

	if (!hItem || (nHF & (TVHT_ONITEMRIGHT|TVHT_NOWHERE|TVHT_ONITEMINDENT))) {
		//clicked in space, do rubber-banding
		DoBanding(bLeft, nFlags, point);
		return;
	}

	ASSERT(nHF & (TVHT_ONITEM|TVHT_ONITEMSTATEICON) );	//nothing else left

	DoPreSelection(hItem, bLeft, nFlags);
	DoAction(hItem, bLeft, nFlags, point);
}

void CMultiTree::DoPreSelection(HTREEITEM hItem, BOOL bLeft, UINT nFlags)
{
	if (bLeft) {
		//if shift key down, select immediately
		if (_bShift) {
			if (!m_hSelect)
				m_hSelect = GetSelectedItem();	//focus
			SelectRange(m_hSelect, hItem, !_bCtrl);
			SetItemState(hItem, TVIS_FOCUSED, TVIS_FOCUSED);	//focus changes to last clicked
		}
		else {
			if (!_bCtrl) {
				//if ctrl was down, then the selection is delayed until
				// mouse up, otherwise select the one item
				if (!IsSelected(hItem))
					SelectAllIgnore(FALSE, hItem);
				SetItemState(hItem, TVIS_SELECTED|TVIS_FOCUSED, TVIS_SELECTED|TVIS_FOCUSED);
			}
			m_hSelect = NULL;	//reset when a non-shift operation occurs
		}
		return;
	}

	//right mouse
	if (nFlags & (MK_CONTROL|MK_SHIFT)) {
		if (!_bShift)
			m_hSelect = hItem;
		return;		//do nothing if shift or ctrl
	}
	if (!IsSelected(hItem))
		SelectAllIgnore(FALSE, hItem);
	SetItemState(hItem, TVIS_SELECTED|TVIS_FOCUSED, TVIS_SELECTED|TVIS_FOCUSED);
}

void CMultiTree::DoAction(HTREEITEM hItem, BOOL bLeft, UINT nFlags, CPoint point)
{
	::SetCapture(m_hWnd);
	ASSERT(::GetCapture() == m_hWnd);

	MSG msg;
	UINT nDone = 0;
	CPoint pt;
	CSize sizeDrag(::GetSystemMetrics(SM_CXDRAG), ::GetSystemMetrics(SM_CYDRAG));

	while (!nDone && ::GetMessage(&msg, NULL, 0, 0)) {

		if (::GetCapture() != m_hWnd)
			break;

		switch (msg.message) {
			case WM_MOUSEMOVE:
				pt.x = GET_X_LPARAM(msg.lParam);
				pt.y = GET_Y_LPARAM(msg.lParam);
				if ((abs(pt.x - point.x) > sizeDrag.cx)
								|| ((abs(pt.y - point.y) > sizeDrag.cy)) )
					nDone = 2;
					//because we exit loop, button up will still be dispatched
					// which means WM_CONTEXTMENU will be sent after TVN_BEGINRDRAG
					// - this is the same behaviour as original tree
				break;

			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
				nDone = 1;
				break;

			default:
				::DispatchMessage(&msg);
				break;
		}
	}

	::ReleaseCapture();
	ASSERT(::GetCapture() != m_hWnd);

	//construct tree notification info
	NMTREEVIEW nmtv;
	nmtv.hdr.hwndFrom = m_hWnd;
	nmtv.hdr.idFrom = ::GetDlgCtrlID(m_hWnd);
	nmtv.itemNew.mask = TVIF_HANDLE|TVIF_PARAM;
	nmtv.itemNew.hItem = hItem;
	nmtv.itemNew.lParam = GetItemData(hItem);
	DWORD dwStyle = GetStyle();

	if (nDone == 1) {
		//click
		if (!_bShift && bLeft) {
			UINT nState = TVIS_SELECTED;
			if (_bCtrl)
				nState ^= (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED);
			else
				SelectAllIgnore(FALSE, hItem);
			SetItemState(hItem, TVIS_FOCUSED|nState, TVIS_FOCUSED|TVIS_SELECTED);
		}
		if (::GetFocus() != m_hWnd)
			::SetFocus(m_hWnd);
		nmtv.hdr.code = bLeft ? NM_CLICK : NM_RCLICK;
		_SendNotify(&nmtv.hdr);
	}
	else if (nDone == 2) {
		//drag
		SetItemState(hItem, TVIS_FOCUSED|TVIS_SELECTED, TVIS_FOCUSED|TVIS_SELECTED);
		if (!(dwStyle & TVS_DISABLEDRAGDROP)) {
			nmtv.hdr.code = bLeft ? TVN_BEGINDRAG : TVN_BEGINRDRAG;
			nmtv.ptDrag = point;
			_SendNotify(&nmtv.hdr);
		}
	}
}

void CMultiTree::DoBanding(BOOL bLeft, UINT nFlags, CPoint point)
{
	if (::GetFocus() != m_hWnd)
		::SetFocus(m_hWnd);

	::SetCapture(m_hWnd);

	CTreeItemList list;
	if (nFlags & (MK_SHIFT|MK_CONTROL))
		GetSelectedList(list);

	CClientDC dc(this);
	CRect rectCli;
	GetClientRect(&rectCli);

	MSG msg;
	BOOL bDone = FALSE;
	CPoint pt;
	CSize sizeDrag(::GetSystemMetrics(SM_CXDRAG), ::GetSystemMetrics(SM_CYDRAG));
	BOOL bDrag = FALSE;
	CSize sizeEdge(1, 1);

	UINT nTimer = SetTimer(1, MST_TIMER_PERIOD, NULL);	//for scroll
	CPoint ptScr(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
	CRect rect(0, 0, 0, 0);
	UINT h = 0;
	HTREEITEM hItem = GetRootItem();
	if (hItem) {
		GetItemRect(hItem, &rect, FALSE);
		ptScr.y *= (h = rect.Height());		//this assumes equal height items
	}
	point += ptScr;

	while (!bDone && ::GetMessage(&msg, NULL, 0, 0)) {

		if (::GetCapture() != m_hWnd)
			break;

		switch (msg.message) {
			case WM_TIMER:
				pt = msg.pt;
				ScreenToClient(&pt);
				if (rectCli.PtInRect(pt)) {
					::DispatchMessage(&msg);
					break;
				}
				msg.lParam = MAKELPARAM(pt.x, pt.y);
				//fall through to mousemove

			case WM_MOUSEMOVE:
				pt.x = GET_X_LPARAM(msg.lParam);
				pt.y = GET_Y_LPARAM(msg.lParam);
				if (!bDrag) {
					if ((abs(pt.x - point.x) <= sizeDrag.cx)
							&& ((abs(pt.y - point.y) <= sizeDrag.cy)) )
						break;
					bDrag = TRUE;
					if (!(nFlags & (MK_CONTROL|MK_SHIFT)))
						SelectAll(FALSE);
					UpdateWindow();
					rect.SetRect(point, point);
					dc.DrawDragRect(rect, sizeEdge, NULL, sizeEdge);
				}

				dc.DrawDragRect(rect, sizeEdge, NULL, sizeEdge);	//delete

				if (pt.y < rectCli.top)
					::SendMessage(m_hWnd, WM_VSCROLL, SB_LINEUP, 0);
				else if (pt.y >= rectCli.bottom)
					::SendMessage(m_hWnd, WM_VSCROLL, SB_LINEDOWN, 0);
				if (pt.x < rectCli.left)
					::SendMessage(m_hWnd, WM_HSCROLL, SB_LINELEFT, 0);
				else if (pt.x >= rectCli.right)
					::SendMessage(m_hWnd, WM_HSCROLL, SB_LINERIGHT, 0);

				ptScr = point;
				ptScr.x -= GetScrollPos(SB_HORZ);
				ptScr.y -= GetScrollPos(SB_VERT) * h;
				rect.SetRect(ptScr, pt);
				rect.NormalizeRect();
				UpdateSelectionForRect(rect, nFlags, list);
				dc.DrawDragRect(rect, sizeEdge, NULL, sizeEdge);	//draw
				break;

			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
				bDone = TRUE;
				break;

			case WM_KEYDOWN:
				if (LOWORD(msg.wParam) == VK_ESCAPE) {
					SelectAll(FALSE);
					bDone = TRUE;
					break;
				}
				//dispatch

			default:
				::DispatchMessage(&msg);
				break;
		}
	}
	KillTimer(nTimer);
	::ReleaseCapture();

	if (bDrag)
		dc.DrawDragRect(rect, sizeEdge, NULL, sizeEdge);
	else
		if (!(nFlags & (MK_CONTROL|MK_SHIFT)))
			SelectAll(FALSE);

	//construct notification info
	NMHDR hdr;
	hdr.hwndFrom = m_hWnd;
	hdr.idFrom = ::GetDlgCtrlID(m_hWnd);
	hdr.code = bLeft ? NM_CLICK : NM_RCLICK;
	_SendNotify(&hdr);

	//when banding, make sure we receive WM_CONTEXTMENU
	// every time - which is what the listview ctrl does
	::DispatchMessage(&msg);
}

void CMultiTree::UpdateSelectionForRect(LPCRECT pRect, UINT nFlags, CTreeItemList& list)
{
	CRect rect;
	BOOL bSel;
	POSITION pos;

	HTREEITEM hItem = GetRootItem();
	while (hItem) {
		bSel = IsSelected(hItem);
		GetItemRect(hItem, &rect, m_bBandLabel);
		if (rect.IntersectRect(rect, pRect)) {
			//item in rect
			pos = list.Find(hItem);
			if (!bSel && !pos)
				SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
			else if (_bCtrl && pos)
				SetItemState(hItem, 0, TVIS_SELECTED);
			else if (_bShift && pos)
				list.RemoveAt(pos);		//if shift and in rect, don't lock anymore
		}
		else {
			//item not in rect
			pos = list.Find(hItem);
			if (bSel && !pos)
				SetItemState(hItem, 0, TVIS_SELECTED);
			else if (pos)
				SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
		}
		hItem = GetNextVisibleItem(hItem);
	}
	UpdateWindow();
}

void CMultiTree::OnSetFocus(CWnd* pOldWnd) 
{
	CMultiTree_BASE::OnSetFocus(pOldWnd);
	if (m_bMulti) {
		//'emulated' selected items will remain greyed
		// if application gets covered
		HTREEITEM hItem = GetFirstSelectedItem();
		RECT rect;
		while (hItem) {
			GetItemRect(hItem, &rect, TRUE);
			InvalidateRect(&rect);
			hItem = GetNextSelectedItem(hItem);
		}
	}
}

void CMultiTree::OnKillFocus(CWnd* pNewWnd) 
{
	CMultiTree_BASE::OnKillFocus(pNewWnd);
	if (m_bMulti) {
		//'emulated' selected items may not get
		// refreshed to grey
		HTREEITEM hItem = GetFirstSelectedItem();
		RECT rect;
		while (hItem) {
			GetItemRect(hItem, &rect, TRUE);
			InvalidateRect(&rect);
			hItem = GetNextSelectedItem(hItem);
		}
	}
}

BOOL CMultiTree::OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (!m_bMulti)
		return FALSE;

	LPNMTREEVIEW pNMTreeView = (LPNMTREEVIEW)pNMHDR;
	*pResult = 0;
	if ((pNMTreeView->action == TVE_COLLAPSE) || (pNMTreeView->action == TVE_COLLAPSERESET)) {
		//clear selection of children, it would be confusing otherwise
		// - the notifications can be over-ridden to stop the de-selection
		// if required
		//unfortunately, the parent window can't over-ride this functionality
		// because MFC gives this class first crack.  So if changes are required
		// a derived class will have to be used..
		ASSERT(pNMTreeView->itemNew.hItem);

		//if a descendent item has focus the parent will get selected as a
		// consequence of collapsing the tree, so preserve
		// (if the parent was already selected it will gain focus, but
		//  that's acceptable)
		BOOL bWasSel = IsSelected(pNMTreeView->itemNew.hItem);
		BOOL bWasFocus = SelectChildren(pNMTreeView->itemNew.hItem, FALSE, TRUE);
		if (bWasFocus && !bWasSel)
			CMultiTree_BASE::SelectItem(NULL);	//stop parent from gaining selection
	}

	return FALSE;	//pass to parent
}

BOOL CMultiTree::SelectChildren(HTREEITEM hParent, BOOL bSelect /*= TRUE*/, BOOL bRecurse /*= TRUE*/)
{
	UINT nS = bSelect ? TVIS_SELECTED : 0;

	BOOL bFocusWasInHere = FALSE;

	HTREEITEM hItem = GetNextItem(hParent, TVGN_CHILD);
	while (hItem) {
		UINT nState = GetItemState(hItem, TVIS_SELECTED|TVIS_EXPANDED|TVIS_FOCUSED);
		if ((nState & TVIS_SELECTED) != nS)
			SetItemState(hItem, nS, TVIS_SELECTED);
		bFocusWasInHere |= (nState & TVIS_FOCUSED);
		if (bRecurse && (nState & TVIS_EXPANDED))
			bFocusWasInHere |= SelectChildren(hItem, bSelect, bRecurse);
		hItem = GetNextSiblingItem(hItem);
	}
	return bFocusWasInHere;
}

void CMultiTree::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (!m_bMulti) {
		CMultiTree_BASE::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}
		
	BOOL bCtrl = (GetKeyState(VK_CONTROL) & 0x8000);
	BOOL bShift = (GetKeyState(VK_SHIFT) & 0x8000);

	BOOL bDir = FALSE;
	HTREEITEM hSel = NULL;
	switch (nChar) {
		case VK_UP:
			bDir = TRUE;
		case VK_DOWN:
			//common
			hSel = GetSelectedItem();
			if (!m_hSelect)
				m_hSelect = hSel;

			if (!bCtrl && !bShift) {
				m_hSelect = NULL;	//reset
				SelectAll(FALSE);
			}
			break;
//		case VK_SPACE:
//			hSel = GetSelectedItem();
//			if (hSel) {
//				BOOL b = IsSelected(hSel);
//				if (bCtrl)
//					SetItemState(hSel, b ? 0 : TVIS_SELECTED, TVIS_SELECTED);
//				else if (!b)
//					SetItemState(hSel, TVIS_SELECTED, TVIS_SELECTED);
//			}
//			return;		//don't call base class (it would start a search on the char)
//TODO: the text search isn't stopped by this ^.  It may be done in the TranslateMessage,
// so would have to trap PreTranslateMessage to avoid it.  If 'space' selection is
// required then need to implement - I'm not bothered.
	}

	CMultiTree_BASE::OnKeyDown(nChar, nRepCnt, nFlags);
	if (!hSel || (!bCtrl && !bShift) )
		return;

	HTREEITEM hNext = bDir ? GetPrevVisibleItem(hSel) : GetNextVisibleItem(hSel);
	if (!hNext)
		hNext = hSel;
	if (bShift)
		SelectRange(m_hSelect, hNext, TRUE);
	else if (bCtrl)
		SetItemState(hNext, TVIS_FOCUSED, TVIS_FOCUSED);
}

void CMultiTree::GetSelectedList(CTreeItemList& list) const
{
	list.RemoveAll();

	HTREEITEM hItem = GetFirstSelectedItem();
	while (hItem) {
		list.AddTail(hItem);
		hItem = GetNextSelectedItem(hItem);
	}
}
