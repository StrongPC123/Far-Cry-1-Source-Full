// XTReBarCtrl.h interface for the CXTReBarCtrl class.
//
// This file is a part of the Xtreme Toolkit for MFC.
// ©1998-2003 Codejock Software, All Rights Reserved.
//
// This source code can only be used under the terms and conditions 
// outlined in the accompanying license agreement.
//
// support@codejock.com
// http://www.codejock.com
//
//////////////////////////////////////////////////////////////////////

#if !defined(__XTREBARCTRL_H__)
#define __XTREBARCTRL_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// BULLETED LIST:

//////////////////////////////////////////////////////////////////////
// Summary:  CXTReBarCtrl is a CWnd derived class.  The CXTReBarCtrl class encapsulates
//			 the functionality of a rebar control, which is a container for a child
//			 window.  The application in which the rebar control resides assigns
//			 the child window contained by the rebar control to the rebar band. 
//			 The child window is usually another common control. 
//
//			 Rebar controls contain one or more bands.  Each band can contain a combination
//			 of a gripper bar, a bitmap, a text label, and a child window.  The band
//			 can contain only one of each of these items.
//
//			 The rebar control can display the child window over a specified background
//			 bitmap.  All rebar control bands can be resized, except those that use
//			 the RBBS_FIXEDSIZE style.  As you reposition or resize a rebar control
//			 band, the rebar control manages the size and position of the child window
//			 assigned to that band.  To resize or change the order of bands within
//			 the control, click and drag a band's gripper bar. A rebar control has
//			 three bands:
//			 [ul] 
//			 [li]A flat, transparent toolbar control.[/li]
//			 [li]Both transparent standard and transparent dropdown buttons.[/li]
//			 [li]A combo box and four standard buttons.[/li]
//			 [/ul]
//			 <b>Rebar control</b>
//
//			 Rebar controls support: 
//			 [ul]
//			 [li]Image lists.[/li]
//			 [li]Message-handling.[/li]
//			 [li]Custom draw functionality.[/li]
//			 [li]A variety of control styles in addition to standard window styles. For
//			 a list of these styles, see Rebar Control Styles in the Platform SDK.[/li]
//			 [/ul]
// See Also: CXTReBarCtrl
class _XT_EXT_CLASS CXTReBarCtrl : public CWnd
{
  DECLARE_DYNAMIC(CXTReBarCtrl)

public:

    // Summary: Constructs a CXTReBarCtrl object.
	CXTReBarCtrl();

	// NUMBERED LIST:

	// Input:	dwStyle - Specifies the combination of rebar control styles applied to the control.
	//			See Rebar Control Styles in the Platform SDK for a list of supported
	//			styles.
	//			rect - A reference to a CRect object or RECT structure, which is the position
	//			and size of the rebar control.
	//			pParentWnd - A pointer to a CWnd object that is the parent window of the rebar
	//			control. It must not be NULL.
	//			nID - Specifies the rebar control's control ID.
	// Example: <pre>
	//			CXTReBarCtrl* pReBarCtrl = new CXTReBarCtrl();
	//			CRect rect;
	//			GetWindowRect(rect);
	//			pReBarCtrl->Create(RBS_BANDBORDERS, rect, this, AFX_IDW_REBAR);</pre>
	//			
	//			 Use ReBar Control.
	//			 ...
	//			<pre>delete pReBarCtrl;</pre>
	// Returns: Nonzero if the object was created successfully, otherwise returns zero.
	// Summary:	Call this member function to create a rebar control.  Create a rebar
	//			control in two steps: 
	//			[ol]
	//			[li]Call CXTReBarCtrl to construct a CXTReBarCtrl object.[/li]
	//			[li]Call this member function, which creates the Windows rebar control
	//			and attaches it to the CXTReBarCtrl object.[/li]
	//			[/ol]
	//			When you call Create, the common controls are initialized.
	BOOL Create(DWORD dwStyle,const RECT& rect,CWnd* pParentWnd,UINT nID);
	
	// Returns: The number of bands assigned to the control.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_GETBANDCOUNT, as described in the Platform SDK.  
	UINT GetBandCount() const;

	// Input:	uBand - Zero-based index of the band for which the information will be retrieved.
	//			prbbi - A pointer to a REBARBANDINFO structure to receive the band information.
	//			You must set the 'cbSize' member of this structure to sizeof(REBARBANDINFO)
	//			and set the 'fMask' member to the items you want to retrieve before
	//			sending this message.
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_GETBANDINFO as described in the Platform SDK.  
	BOOL GetBandInfo(UINT uBand,REBARBANDINFO* prbbi) const;

	// Returns: A value that represents the height, in pixels, of the control.
	// Summary:	Call this member function to retrieve the height of the rebar bar.
	UINT GetBarHeight() const;

	// Input:	prbi - A pointer to a REBARINFO structure that will receive the rebar control
	//			information. You must set the 'cbSize' member of this structure to 
	//			sizeof(REBARINFO) before sending this message.
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_GETBARINFO, as described in the Platform SDK.  
	BOOL GetBarInfo(REBARINFO* prbi) const;

	// Returns: A COLORREF value that represent the current default background color.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_GETBKCOLOR, as described in the Platform SDK.  
	COLORREF GetBkColor() const;

	// Returns: A pointer to an IDropTarget interface.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_GETDROPTARGET, as described in the Platform SDK. 
	IDropTarget* GetDropTarget() const;

	// Input:	uBand - Zero-based index of a band in the rebar control.
	//			prc - A pointer to a RECT structure that will receive the bounds of the
	//			rebar band.
	// Example:	<pre>
	//			CRect rc;
	//			m_wndReBar.GetReBarCtrl().GetRect(0, &rc);
	//			
	//			CString msg;
	//			msg.Format("rect.left = %d, rect.top = %d, rect.right = %d, rect.bottom = %d",
	//			    rc.left, rc.top, rc.right, rc.bottom);
	//			
	//			AfxMessageBox(msg);</pre>
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_GETRECT, as described in the Platform SDK. 
	BOOL GetRect(UINT uBand,LPRECT prc) const;

	// Example: <pre>
	//			UINT nRowCount = m_wndReBar.GetReBarCtrl().GetRowCount();
	//
	//			CString msg;
	//			msg.Format("Row Count is %d", nRowCount);
	//			
	//			AfxMessageBox(msg);</pre>
	// Returns: A UINT value that represents the number of band rows in the control.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_GETROWCOUNT, as described in the Platform SDK. 
	UINT GetRowCount() const;

	// Input:	uRow - Zero-based index of the band that will have its height retrieved.
	// Example: <pre>int nCount = m_wndReBar.GetReBarCtrl().GetRowCount();
	//			int i;
	//			for (i=0; i < nCount; i++)
	//			{
    //			    UINT nHeight = m_wndReBar.GetReBarCtrl().GetRowHeight(i);
	//			    CString msg;
	//			    msg.Format("Height of row %d is %u", i, nHeight);
	//			    AfxMessageBox(msg);
	//			}</pre>
	// Returns: A UINT value that represents the row height, in pixels.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_GETROWHEIGHT, as described in the Platform SDK. 
	UINT GetRowHeight(UINT uRow) const;

	// Returns: A COLORREF value that represents the current default text color.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_GETTEXTCOLOR, as described in the Platform SDK.  
	COLORREF GetTextColor() const;

	// Returns: A pointer to a CToolTipCtrl object.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_GETTOOLTIPS, as described in the Platform SDK.  Note that the MFC
	//			implementation of GetToolTips returns a pointer to a CToolTipCtrl,
	//			rather than an HWND.  
	CToolTipCtrl* GetToolTips() const;

	// Input:	uBandID - The application-defined identifier of the specified band, passed in
	//			the 'wID' member of the REBARBANDINFO structure when the band is inserted.
	// Returns: The zero-based band index if successful, or -1 otherwise.  If duplicate 
	//			band indices exist, the first one is returned.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_IDTOINDEX, as described in the Platform SDK.  
	int IDToIndex(UINT uBandID) const;

	// Input:	uBand - Zero-based index of the band to receive the new settings.
	//			prbbi - Pointer to a REBARBANDINFO structure that defines the band to be inserted.
	//			You must set the 'cbSize' member of this structure to sizeof(REBARBANDINFO)
	//			before sending this message.
	// Example: <pre>
	//			int nCount = m_wndReBar.GetReBarCtrl().GetBandCount();
	//			CString strText = "Band #:";
	//			
	//			int i;
	//			for (i=0; i<nCount; i++)
	//			{
	//			    LPREBARBANDINFO prbbi = (LPREBARBANDINFO)alloca(sizeof(REBARBANDINFO));
	//			    prbbi->cbSize = sizeof(REBARBANDINFO);
	//			
	//			    CString strText;
	//			    strText.Format("Band #: %d", i);
	//			
	//			    LPTSTR lpszText = strText.GetBuffer(strText.GetLength());
	//			    prbbi->lpText = lpszText;
	//			    prbbi->cch = strlen(lpszText) + 1;
	//			    prbbi->fMask = RBBIM_TEXT;
	//			
	//			    m_wndReBar.GetReBarCtrl().SetBandInfo(i, prbbi);
	//			
	//			    strText.ReleaseBuffer();
	//			}</pre>
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_SETBANDINFO, as described in the Platform SDK. 
	BOOL SetBandInfo(UINT uBand,REBARBANDINFO* prbbi);

	// Input:	prbi - A pointer to a REBARINFO structure that contains the information to
	//			be set. You must set the cbSize member of this structure to 
	//			sizeof(REBARINFO) before sending this message
	// Example: <pre>LPREBARINFO prbi = 0;
	//			prbi = (LPREBARINFO)alloca(sizeof(REBARINFO));
	//			if (!prbi)
	//			{
	//			    AfxMessageBox("Couldn't allocate memory for REBARINFO structure!");
	//			    return;
	//			}
	//			
	//			prbi->cbSize = sizeof(REBARINFO);
	//			prbi->fMask = 0;
	//			prbi->himl = 0;
	//			m_wndReBar.GetReBarCtrl().SetBarInfo(prbi);</pre>
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_SETBARINFO, as described in the Platform SDK. 
	BOOL SetBarInfo(REBARINFO* prbi);

	// Input:	clr - The COLORREF value that represents the new default background color.
	// Returns: A COLORREF value that represents the previous default background color.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_SETBKCOLOR, as described in the Platform SDK.  See this topic for
	//			more information about when to set the background color, and how to
	//			set the default.  
	COLORREF SetBkColor(COLORREF clr);

	// Input:	pWnd - A pointer to a CWnd object to set as the owner of the rebar control.
	// Returns: A pointer to a CWnd object that is the current owner of the
	//			rebar control.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_SETPARENT, as described in the Platform SDK. 
	// 
	//			Note that this member function uses pointers to CWnd objects for both
	//			the current and selected owner of the rebar control, rather than handles
	//			to windows.
	// 
	//			<b>Note</b>   This member function does not change the actual parent
	//			that was set when the control was created.  Rather, it sends notification
	//			messages to the window you specify. 
	CWnd* SetOwner(CWnd* pWnd);

	// Input:	clr - A COLORREF value that represents the new text color in the CXTReBarCtrl
	//			object.
	// Returns: The COLORREF value representing the previous text color associated with
	//			the CXTReBarCtrl object.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_SETTEXTCOLOR, as described in the Platform SDK.  It is provided
	//			to support text color flexibility in a rebar control.  
	COLORREF SetTextColor(COLORREF clr);

	// Input:	pToolTip - A pointer to a CToolTipCtrl object.
	// Summary:	Call this member function to associate a tooltip control with a rebar
	//			control.  You must destroy the CToolTipCtrl object when you are done
	//			with it.
	void SetToolTips(CToolTipCtrl* pToolTip);

	// Returns: A pointer to a CImageList object.  Returns NULL if no image
	//			list is set for the control.
	// Summary:	Call this member function to get the CImageList object associated with
	//			a rebar control. This member function uses size and mask information
	//			stored in the REBARINFO structure, as described in the Platform SDK.
	CImageList* GetImageList() const;

	// Input:	pImageList - A pointer to a CImageList object containing the image list to be assigned
	//			to the rebar control.
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	Call this member function to assign an image list to a rebar control.
	BOOL SetImageList(CImageList* pImageList);

	// Input:	uBand - Zero-based index of the band for which the borders will be retrieved.
	//			prc - A pointer to a RECT structure  that will receive the band borders.
	//			If the rebar control has the RBS_BANDBORDERS style, each member of
	//			this structure will receive the number of pixels, on the corresponding
	//			side of the band, that constitute the border.  If the rebar control
	//			does not have the RBS_BANDBORDERS style, only the left member of this
	//			structure receives valid information.  For a description of rebar
	//			control styles, see Rebar Control Styles in the Platform SDK.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_GETBANDBORDERS, as described in the Platform SDK.
	void GetBandBorders(UINT uBand,LPRECT prc) const;

	// Example: <pre>
	//			CPalette* pPalette = m_wndReBar.GetReBarCtrl().GetPalette();
	//			if (pPalette)
	//			{
	//			    int nEntries = pPalette->GetEntryCount();
	//			
	//			    CString msg;
	//			    msg.Format("Number of palette entries: %d", nEntries);
	//			    AfxMessageBox(msg);
	//			}
	//			else
	//			    AfxMessageBox("No palette!");</pre>
	// Returns: A pointer to a CPalette object specifying the rebar control's
	//			current palette.
	// Summary:	Call this member function to retrieve the rebar control's current palette.
	//			Note that this member function uses a CPalette object as its return value, 
	//			rather than an HPALETTE.
	CPalette* GetPalette() const;

	// Input:	hPal - An HPALETTE that specifies the new palette that the rebar control
	//			will use.
	// Returns:	A CPalette object.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_SETPALETTE, as described in the Platform SDK.  Note that this member
	//			function uses a CPalette object as its return value, rather than an
	//			HPALETTE.
	CPalette* SetPalette(HPALETTE hPal);

	// Input:	lpcs - A pointer to a valid tagCOLORSCHEME structure.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function is used to retrieve the current color scheme for
	//			the rebar control. 
	BOOL GetColorScheme(COLORSCHEME* lpcs);

	// Input:	lpcs - A pointer to a valid tagCOLORSCHEME structure.
	// Summary:	This member function is used to set the current color scheme for the
	//			rebar control.
	void SetColorScheme(const COLORSCHEME* lpcs);

	// Input:	uBand - Zero-based index of the band which the drag-and-drop operation will
	//			affect.
	//			dwPos - A DWORD value that contains the starting mouse coordinates.  The horizontal
	//			coordinate is contained in the LOWORD and the vertical coordinate
	//			is contained in the HIWORD.  If you pass (DWORD)-1, the rebar control
	//			will use the position of the mouse the last time the control's thread
	//			called ::GetMessage or ::PeekMessage.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_BEGINDRAG, as described in the Platform SDK.
	void BeginDrag(UINT uBand,DWORD dwPos = (DWORD)-1);

	// Input:	uBand - Zero-based index of the band to be deleted.
	// Example: <pre>
	//			UINT nCount = m_wndReBar.GetReBarCtrl().GetBandCount();
	//		
	//		    if (nCount > 0)
	//		       m_wndReBar.GetReBarCtrl().DeleteBand(nCount - 1);</pre>
	// Returns: Nonzero if the band was deleted successfully, otherwise returns zero.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_DELETEBAND, as described in the Platform SDK. 
	BOOL DeleteBand(UINT uBand);

	// Input:	dwPos - A DWORD value that contains the new mouse coordinates.  The horizontal
	//			coordinate is contained in the LOWORD and the vertical coordinate
	//			is contained in the HIWORD.  If you pass (DWORD)-1, the rebar control
	//			will use the position of the mouse the last time the control's thread
	//			called ::GetMessage or ::PeekMessage.
	// Summary:	This member function implements the behavior of the Win32 message RB_DRAGMOVE,
	//			as described in the Platform SDK.
	void DragMove(DWORD dwPos = (DWORD)-1);

	// Summary: This member function implements the behavior of the Win32 message RB_ENDDRAG,
	//			as described in the Platform SDK.
	void EndDrag();

	// Input:	prbht - A pointer to a RBHITTESTINFO structure.  Before sending the message,
	//			the 'pt' member of this structure must be initialized, in client coordinates,
	//			to the point that will be tested.
	// Returns: The zero-based index of the band at the given point, or -1 if no rebar band 
	//			was at the point.
	// Summary:	This member function implements the behavior of the Win32 message RB_HITTEST,
	//			as described in the Platform SDK.  
	int HitTest(RBHITTESTINFO* prbht);

	// Input:	uIndex - Zero-based index of the location where the band will be inserted.
	//			If you set this parameter to -1, the control will add the new band
	//			at the last location.
	//			prbbi - A pointer to a REBARBANDINFO structure that defines the band to be
	//			inserted. You must set the 'cbSize' member of this structure to 
	//			sizeof(REBARBANDINFO) before calling this function.
	// Example: <pre>
	//			LPREBARBANDINFO prbbi = (LPREBARBANDINFO)alloca(sizeof(REBARBANDINFO));
	//			prbbi->cbSize = sizeof(REBARBANDINFO);
	//			
	//			LPTSTR lpszText = (LPTSTR)alloca(80);
	//			prbbi->lpText = lpszText;
	//			prbbi->cch = 80;
	//			
	//			prbbi->fMask = RBBIM_BACKGROUND | RBBIM_CHILD |
	//			    RBBIM_CHILDSIZE | RBBIM_COLORS | RBBIM_HEADERSIZE |
	//			    RBBIM_IDEALSIZE | RBBIM_ID | RBBIM_IMAGE |
	//			    RBBIM_LPARAM | RBBIM_SIZE | RBBIM_STYLE | RBBIM_TEXT;
	//			
	//			m_wndReBar.GetReBarCtrl().GetBandInfo(1, prbbi);
	//			
	//			m_wndReBar.GetReBarCtrl().InsertBand(2, prbbi);</pre>
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	This member function implements the behavior of the Win32 message RB_INSERTBAND,
	//			as described in the Platform SDK. 
	BOOL InsertBand(UINT uIndex,REBARBANDINFO* prbbi);

	// Input:	uBand - Zero-based index of the band to be maximized.
	// Example: <pre>
	//			UINT nCount = m_pReBarCtrl->GetBandCount();
	//
	//			UINT i;
	//			for (i=0; i<nCount; i++)
	//			    m_pReBarCtrl->MaximizeBand(i);</pre>
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_MAXIMIZEBAND, as described in the Platform SDK. 
	void MaximizeBand(UINT uBand);

	// Input:	uBand - Zero-based index of the band to be minimized.
	// Example: <pre>
	//			UINT nCount = m_pReBarCtrl->GetBandCount();
	//
	//			UINT i;
	//			for (i=0; i<nCount; i++)
	//			    m_pReBarCtrl->MinimizeBand(i);</pre>
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_MINIMIZEBAND, as described in the Platform SDK. 
	void MinimizeBand(UINT uBand);

	// Input:	uBand - Zero-based index of a band in the rebar control.
	//			fShow - Indicates if the band should be shown or hidden.  If this value is
	//			TRUE, the band will be shown.  Otherwise, the band will be hidden.
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_SHOWBAND, as described in the Platform SDK.  
	BOOL ShowBand(UINT uBand,BOOL fShow = TRUE);

	// Input:	rect - A reference to a CRect object that specifies the rectangle that the
	//			rebar control should be sized to.
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_SIZETORECT, as described in the Platform SDK.  Note that this member
	//			function uses a CRect object as a parameter, rather than a RECT structure.
	BOOL SizeToRect(CRect& rect);

	// Input:	uFrom - Zero-based index of the band to be moved.
	//			uTo - Zero-based index of the new band position. This parameter value must
	//			never be greater than the number of bands minus one.  To obtain the
	//			number of bands, call GetBandCount.
	// Returns: Nonzero if successful, otherwise returns zero.
	// Summary:	This member function implements the behavior of the Win32 message 
	//			RB_MOVEBAND, as described in the Platform SDK.  
	BOOL MoveBand(UINT uFrom,UINT uTo);
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE UINT CXTReBarCtrl::GetBandCount() const {
	ASSERT(::IsWindow(m_hWnd)); return (UINT) ::SendMessage(m_hWnd, RB_GETBANDCOUNT, 0, 0L);
}
AFX_INLINE BOOL CXTReBarCtrl::GetBandInfo(UINT uBand, REBARBANDINFO* prbbi) const {
	ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, RB_GETBANDINFO, uBand, (LPARAM)prbbi);
}
AFX_INLINE UINT CXTReBarCtrl::GetBarHeight() const {
	ASSERT(::IsWindow(m_hWnd)); return (UINT) ::SendMessage(m_hWnd, RB_GETBARHEIGHT, 0, 0L);
}
AFX_INLINE BOOL CXTReBarCtrl::GetBarInfo(REBARINFO* prbi) const {
	ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, RB_GETBARINFO, 0, (LPARAM)prbi);
}
AFX_INLINE COLORREF CXTReBarCtrl::GetBkColor() const {
	ASSERT(::IsWindow(m_hWnd)); return (COLORREF) ::SendMessage(m_hWnd, RB_GETBKCOLOR, 0, 0L);
}
AFX_INLINE IDropTarget* CXTReBarCtrl::GetDropTarget() const {
	ASSERT(::IsWindow(m_hWnd)); IDropTarget* pdt; ::SendMessage(m_hWnd, RB_GETDROPTARGET, 0, (LPARAM)&pdt); return pdt;
}
AFX_INLINE BOOL CXTReBarCtrl::GetRect(UINT uBand, LPRECT prc) const {
	ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, RB_GETRECT, uBand, (LPARAM)prc);
}
AFX_INLINE UINT CXTReBarCtrl::GetRowCount() const {
	ASSERT(::IsWindow(m_hWnd)); return (UINT) ::SendMessage(m_hWnd, RB_GETROWCOUNT, 0, 0L);
}
AFX_INLINE UINT CXTReBarCtrl::GetRowHeight(UINT uRow) const {
	ASSERT(::IsWindow(m_hWnd)); return (UINT) ::SendMessage(m_hWnd, RB_GETROWHEIGHT, uRow, 0L);
}
AFX_INLINE COLORREF CXTReBarCtrl::GetTextColor() const {
	ASSERT(::IsWindow(m_hWnd)); return (COLORREF) ::SendMessage(m_hWnd, RB_GETTEXTCOLOR, 0, 0L);
}
AFX_INLINE CToolTipCtrl* CXTReBarCtrl::GetToolTips() const {
	ASSERT(::IsWindow(m_hWnd)); return (CToolTipCtrl*)CWnd::FromHandle((HWND)::SendMessage(m_hWnd, RB_GETTOOLTIPS, 0, 0L));
}
AFX_INLINE int CXTReBarCtrl::IDToIndex(UINT uBandID) const {
	ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, RB_IDTOINDEX, uBandID, 0L);
}
AFX_INLINE BOOL CXTReBarCtrl::SetBandInfo(UINT uBand, REBARBANDINFO* prbbi) {
	ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, RB_SETBANDINFO, uBand, (LPARAM)prbbi);
}
AFX_INLINE BOOL CXTReBarCtrl::SetBarInfo(REBARINFO* prbi) {
	ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, RB_SETBARINFO, 0, (LPARAM)prbi);
}
AFX_INLINE COLORREF CXTReBarCtrl::SetBkColor(COLORREF clr) {
	ASSERT(::IsWindow(m_hWnd)); return (COLORREF) ::SendMessage(m_hWnd, RB_SETBKCOLOR, 0, (LPARAM)clr);
}
AFX_INLINE CWnd* CXTReBarCtrl::SetOwner(CWnd* pWnd) {
	ASSERT(::IsWindow(m_hWnd)); return CWnd::FromHandle((HWND)::SendMessage(m_hWnd, RB_SETPARENT, (WPARAM)pWnd->m_hWnd, 0L));
}
AFX_INLINE COLORREF CXTReBarCtrl::SetTextColor(COLORREF clr) {
	ASSERT(::IsWindow(m_hWnd)); return (COLORREF) ::SendMessage(m_hWnd, RB_SETTEXTCOLOR, 0, (LPARAM)clr);
}
AFX_INLINE void CXTReBarCtrl::SetToolTips(CToolTipCtrl* pToolTip) {
	ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, RB_SETTOOLTIPS, (WPARAM)pToolTip->m_hWnd, 0L);
}
AFX_INLINE void CXTReBarCtrl::GetBandBorders(UINT uBand, LPRECT prc) const {
	ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, RB_GETBANDBORDERS, uBand, (LPARAM)prc);
}
AFX_INLINE CPalette* CXTReBarCtrl::GetPalette() const {
	ASSERT(::IsWindow(m_hWnd)); return CPalette::FromHandle((HPALETTE)::SendMessage(m_hWnd, RB_GETPALETTE, 0, 0L));
}
AFX_INLINE CPalette* CXTReBarCtrl::SetPalette(HPALETTE hPal) {
	ASSERT(::IsWindow(m_hWnd)); return CPalette::FromHandle((HPALETTE)::SendMessage(m_hWnd, RB_SETPALETTE, 0, (LPARAM)hPal));
}
AFX_INLINE void CXTReBarCtrl::BeginDrag(UINT uBand, DWORD dwPos) {
	ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, RB_BEGINDRAG, uBand, dwPos);
}
AFX_INLINE BOOL CXTReBarCtrl::DeleteBand(UINT uBand) {
	ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, RB_DELETEBAND, uBand, 0L);
}
AFX_INLINE void CXTReBarCtrl::DragMove(DWORD dwPos) {
	ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, RB_DRAGMOVE, 0, dwPos);
}
AFX_INLINE void CXTReBarCtrl::EndDrag() {
	ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, RB_ENDDRAG, 0, 0L);
}
AFX_INLINE int CXTReBarCtrl::HitTest(RBHITTESTINFO* prbht) {
	ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, RB_HITTEST, 0, (LPARAM)prbht);
}
AFX_INLINE BOOL CXTReBarCtrl::InsertBand(UINT uIndex, REBARBANDINFO* prbbi) {
	ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, RB_INSERTBAND, uIndex, (LPARAM)prbbi);
}
AFX_INLINE void CXTReBarCtrl::MaximizeBand(UINT uBand) {
	ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, RB_MAXIMIZEBAND, uBand, 0L);
}
AFX_INLINE void CXTReBarCtrl::MinimizeBand(UINT uBand) {
	ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, RB_MINIMIZEBAND, uBand, 0L);
}
AFX_INLINE BOOL CXTReBarCtrl::ShowBand(UINT uBand, BOOL fShow) {
	ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, RB_SHOWBAND, uBand, fShow);
}
AFX_INLINE BOOL CXTReBarCtrl::SizeToRect(CRect& rect) {
	ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, RB_SIZETORECT, 0, (LPARAM)&rect);
}
AFX_INLINE BOOL CXTReBarCtrl::MoveBand(UINT uFrom, UINT uTo) {
	ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, RB_MOVEBAND, uFrom, uTo);
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTREBARCTRL_H__)