// NumberCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "NumberCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CNumberCtrl

CNumberCtrl::CNumberCtrl()
{
	m_btnStatus = 0;
	m_btnWidth = 10;
	m_draggin = false;
	m_value = 0;
	m_min = 0;
	m_max = 10000;
	m_step = 0.01f;
	m_enabled = true;
	m_noNotify = false;
	m_integer = false;
	m_iInternalPrecision=2;		// default internal precision for floats
	m_nFlags = 0;
	m_bUndoEnabled = false;
	m_bDragged = false;
	m_multiplier = 1;
}

CNumberCtrl::~CNumberCtrl()
{
}


BEGIN_MESSAGE_MAP(CNumberCtrl, CWnd)
	//{{AFX_MSG_MAP(CNumberCtrl)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_ENABLE()
	ON_WM_SETFOCUS()
	ON_EN_SETFOCUS(IDC_EDIT,OnEditSetFocus)
	ON_EN_KILLFOCUS(IDC_EDIT,OnEditKillFocus)
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNumberCtrl message handlers

void CNumberCtrl::Create( CWnd* parentWnd,CRect &rc,UINT nID,int nFlags )
{
	m_nFlags = nFlags;
	HCURSOR arrowCursor = AfxGetApp()->LoadStandardCursor( IDC_ARROW );
	CreateEx( 0,AfxRegisterWndClass(NULL,arrowCursor,NULL/*(HBRUSH)GetStockObject(LTGRAY_BRUSH)*/),NULL,WS_CHILD|WS_VISIBLE|WS_TABSTOP,rc,parentWnd,nID );
}

void CNumberCtrl::Create( CWnd* parentWnd,UINT ctrlID,int flags )
{
	ASSERT( parentWnd );
	m_nFlags = flags;
	CRect rc;
	CWnd *ctrl = parentWnd->GetDlgItem( ctrlID );
	ctrl->SetDlgCtrlID( ctrlID + 10000 );
	ctrl->ShowWindow( SW_HIDE );
	ctrl->GetWindowRect( rc );
	parentWnd->ScreenToClient(rc);

	HCURSOR arrowCursor = AfxGetApp()->LoadStandardCursor( IDC_ARROW );
	CreateEx( 0,AfxRegisterWndClass(NULL,arrowCursor,NULL/*(HBRUSH)GetStockObject(LTGRAY_BRUSH)*/),NULL,WS_CHILD|WS_VISIBLE|WS_TABSTOP,rc,parentWnd,ctrlID );
}

int CNumberCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_upDownCursor = AfxGetApp()->LoadStandardCursor( IDC_SIZENS );
	m_upArrow = (HICON)LoadImage( AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_UP_ARROW),IMAGE_ICON,5,5,LR_DEFAULTCOLOR );
	m_downArrow = (HICON)LoadImage( AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_DOWN_ARROW),IMAGE_ICON,5,5,LR_DEFAULTCOLOR );
	
	CRect rc;
	GetClientRect( rc );

	if (m_nFlags & LEFTARROW)
	{
		rc.left += m_btnWidth+3;
	}
	else
	{
		rc.right -= m_btnWidth+1;
	}

	DWORD nFlags = WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL;
	if (m_nFlags & LEFTALIGN)
		nFlags |= ES_LEFT;
	else
		nFlags |= ES_RIGHT;

	if (!(m_nFlags & NOBORDER))
		nFlags |= WS_BORDER;
	//m_edit.Create( WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_BORDER|ES_RIGHT|ES_AUTOHSCROLL,rc,this,IDC_EDIT );
	m_edit.Create( nFlags,rc,this,IDC_EDIT );
//	m_edit.Create( WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_BORDER|ES_LEFT|ES_AUTOHSCROLL,rc,this,IDC_EDIT );
	m_edit.SetFont( CFont::FromHandle( (HFONT)::GetStockObject(DEFAULT_GUI_FONT)) );
	m_edit.SetUpdateCallback( functor(*this, &CNumberCtrl::OnEditChanged) );

	float val = m_value;
	m_value = val+1;
	SetInternalValue( val );
	
	return 0;
}

void CNumberCtrl::SetLeftAlign( bool left )
{
	if (m_edit)
	{
		if (left)
			m_edit.ModifyStyle( ES_RIGHT,ES_LEFT );
		else
			m_edit.ModifyStyle( ES_LEFT,ES_RIGHT );
	}
}

void CNumberCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	DrawButtons( dc );
	
	// Do not call CWnd::OnPaint() for painting messages
}

void CNumberCtrl::DrawButtons( CDC &dc )
{
	CRect rc;
	GetClientRect( rc );

	if (!m_enabled)
	{
		//dc.FillRect( rc,CBrush::FromHandle((HBRUSH)GetStockObject(LTGRAY_BRUSH)) );
		dc.FillSolidRect( rc,GetSysColor(COLOR_3DFACE) );
		return;
	}
	int x = 0;
	if (!(m_nFlags & LEFTARROW))
	{
		x = rc.right-m_btnWidth;
	}
	int y = rc.top;
	int w = m_btnWidth;
	int h = rc.bottom;
	int h2 = h/2;
	COLORREF hilight = RGB(255,255,255);
	COLORREF shadow = RGB(100,100,100);
	//dc.Draw3dRect( x,y,w,h/2-1,hilight,shadow );
	//dc.Draw3dRect( x,y+h/2+1,w,h/2-1,hilight,shadow );

	int smallOfs = 0;
	if (rc.bottom <= 18)
		smallOfs = 1;


	//dc.FillRect()
	//dc.SelectObject(b1);
	//dc.Rectangle( x,y, x+w,y+h2 );
	//dc.SelectObject(b2);
	//dc.Rectangle( x,y+h2, x+w,y+h );

	if (m_btnStatus == 1 || m_btnStatus == 3)
	{
		dc.Draw3dRect( x,y,w,h2,shadow,hilight );
	}
	else
	{
		dc.Draw3dRect( x,y,w,h2,hilight,shadow );
		//DrawIconEx( dc,x+1,y+2,m_upArrow,5,5,0,0,DI_NORMAL );
	}

	if (m_btnStatus == 2 || m_btnStatus == 3)
		dc.Draw3dRect( x,y+h2+1,w,h2-1+smallOfs,shadow,hilight );
	else
		dc.Draw3dRect( x,y+h2+1,w,h2-1+smallOfs,hilight,shadow );

	DrawIconEx( dc,x+2,y+2,m_upArrow,5,5,0,0,DI_NORMAL );

	DrawIconEx( dc,x+2,y+h2+3-smallOfs,m_downArrow,5,5,0,0,DI_NORMAL );
}

void CNumberCtrl::GetBtnRect( int btn,CRect &rc )
{
	CRect rcw;
	GetClientRect( rcw );

	int x = 0;
	if (!(m_nFlags & LEFTARROW))
	{
		x = rcw.right-m_btnWidth;
	}
	int y = rcw.top;
	int w = m_btnWidth;
	int h = rcw.bottom;
	int h2 = h/2;

	if (btn == 0)
	{
		rc.SetRect( x,y,x+w,y+h2 );
	}
	else if (btn == 1)
	{
		rc.SetRect( x,y+h2+1,x+w,y+h );
	}
}

int CNumberCtrl::GetBtn( CPoint point )
{
	for (int i = 0; i < 2; i++)
	{
		CRect rc;
		GetBtnRect( i,rc );
		if (rc.PtInRect(point))
		{
			return i;
		}
	}
	return -1;
}

void CNumberCtrl::SetBtnStatus( int s )
{
	m_btnStatus = s;
	RedrawWindow();
}

void CNumberCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (!m_enabled)
		return;

	m_btnStatus = 0;
	int btn = GetBtn(point);
	if (btn >= 0)
	{
		SetBtnStatus( btn+1 );
		m_bDragged = false;

		// Start undo.
		if (m_bUndoEnabled)
			GetIEditor()->BeginUndo();
	}
	m_mousePos = point;
	SetCapture();
	m_draggin = true;
	
	CWnd::OnLButtonDown(nFlags, point);
}

void CNumberCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (!m_enabled)
		return;

	bool bLButonDown = false;
	m_draggin = false;
	if (GetCapture() == this)
	{
		bLButonDown = true;
		ReleaseCapture();
	}
	SetBtnStatus( 0 );

	//if (m_endUpdateCallback)
		//m_endUpdateCallback(this);

	if (m_bUndoEnabled && GetIEditor()->IsUndoRecording())
		GetIEditor()->AcceptUndo( m_undoText );

	if (bLButonDown)
	{
		int btn = GetBtn(point);
		if (!m_bDragged && btn >= 0)
		{
			float prevValue = m_value;
			if (btn == 0)
				SetInternalValue( GetInternalValue() + m_step );
			if (btn == 1)
				SetInternalValue( GetInternalValue() - m_step );

			if (prevValue != m_value)
				NotifyUpdate(false);
		}
		else if (m_bDragged)
		{
			// Send last non tracking update after tracking.
			NotifyUpdate(false);
		}
	}
	
	///CWnd::OnLButtonUp(nFlags, point);

	if (m_edit)
		m_edit.SetFocus();
}

void CNumberCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (!m_enabled)
		return;
	
	if (point == m_mousePos)
		return;

	if (m_draggin)
	{
		m_bDragged = true;
		float prevValue = m_value;

		SetCursor( m_upDownCursor );
		if (m_btnStatus != 3)
			SetBtnStatus(3);
		
		if (!m_integer)
		{
			// Floating control.
			int y = (point.y-m_mousePos.y) * abs((point.y-m_mousePos.y));
			SetInternalValue( GetInternalValue() - m_step*y );
		}
		else
		{
			// Integer control.
			int y = point.y-m_mousePos.y;
			SetInternalValue( GetInternalValue() - m_step*y );
		}

		CPoint cp;
		GetCursorPos( &cp );
		int sX = GetSystemMetrics(SM_CXSCREEN);
		int sY = GetSystemMetrics(SM_CYSCREEN);

		if (cp.y < 20 || cp.y > sY-20)
		{
			// When near end of screen, prevent cursor from moving.
			CPoint p = m_mousePos;
			ClientToScreen(&p);
			SetCursorPos( p.x,p.y );
		}
		else
		{
			m_mousePos = point;
		}

		if (prevValue != m_value)
			NotifyUpdate(true);
	}
	
	CWnd::OnMouseMove(nFlags, point);
}

void CNumberCtrl::SetRange( float min,float max )
{
	m_min = min;
	m_max = max;
}

//////////////////////////////////////////////////////////////////////////
void CNumberCtrl::SetInternalValue( float val )
{
	CString s;
	
	if (val < m_min) val = m_min;
	if (val > m_max) val = m_max;

	if (m_integer)
	{
		if ((int)m_value == (int)val)
			return;
		s.Format( "%d",(int)val );
	}
	else
	{
		double prec = pow(10.0f,m_iInternalPrecision);
		if (fabs(val-m_value) < 0.9f/prec)
			return;

		/*
		if (val == int(val))
		{
			// Optimize float to string storage.
			s.Format( "%i",int(val) );
		}
		else
		{
			int digits = CalculateDecimalPlaces(val,m_iInternalPrecision);		// m.m. calculate the digits right from the comma 
			char fmt[12];

			if (digits < 2)
				digits = 2;
			sprintf(fmt,"%%.%df",digits); // e.g.   "%.2f"
			s.Format( fmt,val );
		}
		*/
		double v = val;
		v = v*(prec);
		int intpart = RoundFloatToInt(v);
		v = (double)intpart / prec;
		// Round it to precision.
		s.Format( "%g",v );
	}
	m_value = val;
	
	if (m_edit)
	{
		m_noNotify = true;
		m_edit.SetText( s );
		m_noNotify = false;
	}
}

//////////////////////////////////////////////////////////////////////////
float CNumberCtrl::GetInternalValue() const
{
	if (!m_enabled)
		return m_value;

	if (m_edit)
	{
		CString str;
		m_edit.GetWindowText(str);
		m_value = atof( (const char*)str );
	}
	return m_value;
}

//////////////////////////////////////////////////////////////////////////
void CNumberCtrl::SetValue( float val,float step )
{
	m_step = step;
	if (m_integer && m_step < 1)
		m_step = 1;
	SetInternalValue( val*m_multiplier );
}

//////////////////////////////////////////////////////////////////////////
float CNumberCtrl::GetValue() const
{
	return GetInternalValue() / m_multiplier;
}

//////////////////////////////////////////////////////////////////////////
float CNumberCtrl::GetStep() const
{
	return m_step;
}

//////////////////////////////////////////////////////////////////////////
CString CNumberCtrl::GetValueAsString() const
{
	CString str;
	/*
	int digits = CalculateDecimalPlaces(GetValue(),m_iInternalPrecision);		// m.m. calculate the digits right from the comma 
	char fmt[12];
	sprintf(fmt,"%%.%df",digits);									// e.g.   "%.2f"
	str.Format( fmt,m_value );
	*/
	str.Format( "%g",m_value/m_multiplier );
	return str;
}

//////////////////////////////////////////////////////////////////////////
void CNumberCtrl::OnEditChanged()
{
	static bool inUpdate = false;

	if (!inUpdate)
	{
		float prevValue = m_value;

		float v = GetInternalValue();
		if (v < m_min) v = m_min;
		if (v > m_max) v = m_max;
		if (v != m_value)
		{
			SetInternalValue( v );
		}

		if (prevValue != m_value)
			NotifyUpdate(false);

		inUpdate = false;
	}
}

//////////////////////////////////////////////////////////////////////////
void CNumberCtrl::OnEnable(BOOL bEnable) 
{
	CWnd::OnEnable(bEnable);
	
	m_enabled = (bEnable == TRUE);
	if (m_edit)
	{
		m_edit.EnableWindow(bEnable);
		RedrawWindow();
	}
}

void CNumberCtrl::NotifyUpdate( bool tracking )
{
	if (m_noNotify)
		return;
	
	if (!tracking && m_bUndoEnabled)
		GetIEditor()->BeginUndo();

	if (m_updateCallback)
		m_updateCallback(this);

	CWnd *parent = GetParent();
	if (parent)
	{
		if (!tracking)
		{
			::SendMessage( parent->GetSafeHwnd(),WM_COMMAND,MAKEWPARAM( GetDlgCtrlID(),EN_CHANGE ),(LPARAM)GetSafeHwnd() );
		}
		::SendMessage( parent->GetSafeHwnd(),WM_COMMAND,MAKEWPARAM( GetDlgCtrlID(),EN_UPDATE ),(LPARAM)GetSafeHwnd() );
	}
	m_lastUpdateValue = m_value;

	if (!tracking && m_bUndoEnabled)
		GetIEditor()->AcceptUndo( m_undoText );
}

void CNumberCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	
	if (m_edit)
	{
		m_edit.SetFocus();
		m_edit.SetSel(0,-1);
	}
}

void CNumberCtrl::SetInteger( bool enable )
{
	m_integer = enable;
	m_step = 1;
	float f = GetInternalValue();
	m_value = f+1;
	SetInternalValue(f);
}

//////////////////////////////////////////////////////////////////////////
void CNumberCtrl::OnEditSetFocus()
{
}

//////////////////////////////////////////////////////////////////////////
void CNumberCtrl::OnEditKillFocus()
{
	//if (m_bUpdating && m_endUpdateCallback)
		//m_endUpdateCallback(this);

	if (m_value != m_lastUpdateValue)
		NotifyUpdate(false);
}


//////////////////////////////////////////////////////////////////////////
// calculate the digits right from the comma 
int CNumberCtrl::CalculateDecimalPlaces( float infNumber, int iniMaxPlaces )
{
	assert(iniMaxPlaces>=0);

	char str[256],*_str=str;

	sprintf(str,"%f",infNumber);

	while(*_str!='.')_str++;											// search the comma

	int ret=0;

	if(*_str!=0)																	// comma?
	{
		int cur=1;

		_str++;																			// jump over comma

		while(*_str>='0' && *_str<='9')
		{
			if(*_str!='0')ret=cur;

			_str++;cur++;
		}

		if(ret>iniMaxPlaces)ret=iniMaxPlaces;				// bound to maximum
	}

	return(ret);
}



//! m.m. (default is 2)
void CNumberCtrl::SetInternalPrecision( int iniDigits )
{
	assert(iniDigits>=0);

	m_iInternalPrecision=iniDigits;
}

//////////////////////////////////////////////////////////////////////////
void CNumberCtrl::EnableUndo( const CString& undoText )
{
	m_undoText = undoText;
	m_bUndoEnabled = true;
}

void CNumberCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if (m_edit.m_hWnd)
	{
		CRect rc;
		GetClientRect( rc );
		if (m_nFlags & LEFTARROW)
		{
			rc.left += m_btnWidth+3;
		}
		else
		{
			rc.right -= m_btnWidth+1;
		}
		m_edit.MoveWindow(rc,FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////
void CNumberCtrl::SetFont( CFont* pFont,BOOL bRedraw )
{
	CWnd::SetFont(pFont,bRedraw);
	if (m_edit.m_hWnd)
		m_edit.SetFont(pFont,bRedraw);
}

//////////////////////////////////////////////////////////////////////////
void CNumberCtrl::SetMultiplier( float fMultiplier )
{
	if (fMultiplier != 0)
		m_multiplier = fMultiplier;
}