#include "stdafx.h"
#include "resource.h"
#include "tipdlg.h"

//#include "misc.h"
// CG: This file added by 'Tip of the Day' component.

//#include <winreg.h>
#include <sys\stat.h>
#include <sys\types.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



#ifndef _WIN32

// supply a replacement for CDC::FillSolidColor for MSVC 1.5

void FillSolidRect(CDC *pDC, LPCRECT lpRect, COLORREF clr)
{
        ASSERT_VALID(pDC);
        ASSERT(pDC->m_hDC != NULL);

        pDC->SetBkColor(clr);
        pDC->ExtTextOut(0, 0, ETO_OPAQUE, lpRect, NULL, 0, NULL);
}

#endif


/////////////////////////////////////////////////////////////////////////////
// CTipDlg dialog

#define MAX_BUFLEN 1000

static const TCHAR szSection[] = _T("Tip");
static const TCHAR szIntFilePos[] = _T("FilePos");
static const TCHAR szTimeStamp[] = _T("TimeStamp");
static const TCHAR szIntStartup[] = _T("StartUp");

CTipDlg::CTipDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_TIP, pParent)
{
	//{{AFX_DATA_INIT(CTipDlg)
	m_bStartup = TRUE;
	//}}AFX_DATA_INIT

	// We need to find out what the startup and file position parameters are
	// If startup does not exist, we assume that the Tips on startup is checked TRUE.
	CWinApp* pApp = AfxGetApp();
	m_bStartup = !pApp->GetProfileInt(szSection, szIntStartup, 1);
	UINT iFilePos = pApp->GetProfileInt(szSection, szIntFilePos, 0);

	// Now try to open the tips file
	const char *p = _pgmptr ;

	CString sPath = "Editor\\Editor.tip";
	
	m_pStream = fopen(sPath, "r");
	if (m_pStream == NULL) 
	{
		m_strTip.LoadString(CG_IDS_FILE_ABSENT);
		return;
	} 

	// If the timestamp in the INI file is different from the timestamp of
	// the tips file, then we know that the tips file has been modified
	// Reset the file position to 0 and write the latest timestamp to the
	// ini file
	struct __stat64 buf;
	_fstat64(_fileno(m_pStream), &buf);
	CString strCurrentTime = _ctime64(&buf.st_ctime);
	strCurrentTime.TrimRight();
	CString strStoredTime = 
		pApp->GetProfileString(szSection, szTimeStamp, NULL);
	if (strCurrentTime != strStoredTime) 
	{
		iFilePos = 0;
		pApp->WriteProfileString(szSection, szTimeStamp, strCurrentTime);
	}

	if (fseek(m_pStream, iFilePos, SEEK_SET) != 0) 
	{
		AfxMessageBox(CG_IDS_FILE_CORRUPT);
	}
	else 
	{
		GetNextTipString(m_strTip);
	}
}

CTipDlg::~CTipDlg()
{
	// This destructor is executed whether the user had pressed the escape key
	// or clicked on the close button. If the user had pressed the escape key,
	// it is still required to update the filepos in the ini file with the 
	// latest position so that we don't repeat the tips! 
    
	// But make sure the tips file existed in the first place....
	if (m_pStream != NULL) 
	{
		CWinApp* pApp = AfxGetApp();
		pApp->WriteProfileInt(szSection, szIntFilePos, ftell(m_pStream));
		fclose(m_pStream);
	}
}
        
void CTipDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTipDlg)
	DDX_Check(pDX, IDC_STARTUP, m_bStartup);
//	DDX_Text(pDX, IDC_TIPSTRING, m_strTip);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTipDlg, CDialog)
	//{{AFX_MSG_MAP(CTipDlg)
	ON_BN_CLICKED(IDC_NEXTTIP, OnNextTip)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTipDlg message handlers

void CTipDlg::OnNextTip()
{
	GetNextTipString(m_strTip);
	UpdateData(FALSE);
	
	CDC *pDC = GetDC() ;
	DoPaint (pDC, TRUE) ;
	VERIFY (ReleaseDC (pDC)) ;
}

void CTipDlg::GetNextTipString(CString& strNext)
{
	LPTSTR lpsz = strNext.GetBuffer(MAX_BUFLEN);

	// This routine identifies the next string that needs to be
	// read from the tips file
	BOOL bStop = FALSE;
	while (!bStop) 
	{
		if (_fgetts(lpsz, MAX_BUFLEN, m_pStream) == NULL) 
		{
			// We have either reached EOF or enocuntered some problem
			// In both cases reset the pointer to the beginning of the file
			// This behavior is same as VC++ Tips file
			if (fseek(m_pStream, 0, SEEK_SET) != 0) 
				AfxMessageBox(CG_IDS_FILE_CORRUPT);
		} 
		else 
		{
			if (*lpsz != ' ' && *lpsz != '\t' && 
				*lpsz != '\n' && *lpsz != ';') 
			{
				// There should be no space at the beginning of the tip
				// This behavior is same as VC++ Tips file
				// Comment lines are ignored and they start with a semicolon
				bStop = TRUE;
			}
		}
	}
	strNext.ReleaseBuffer();
}

HBRUSH CTipDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_TIPSTRING)
		return (HBRUSH)GetStockObject(WHITE_BRUSH);

	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CTipDlg::OnOK()
{
	CDialog::OnOK();
	
    // Update the startup information stored in the INI file
	CWinApp* pApp = AfxGetApp();
	pApp->WriteProfileInt(szSection, szIntStartup, !m_bStartup);
}

BOOL CTipDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// If Tips file does not exist then disable NextTip
	if (m_pStream == NULL)
		GetDlgItem(IDC_NEXTTIP)->EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CTipDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	DoPaint (&dc, FALSE) ;

	// Do not call CDialog::OnPaint() for painting messages
}

//////////////////////////////////////////////////////////////////////////
INT_PTR CTipDlg::DoModal ()
{
  if (m_pStream != NULL)
    return CDialog::DoModal () ;
  else
  {
    MessageBeep (-1) ;
    return IDOK ;
  }
}

void CTipDlg::DoPaint (CDC *pdc, BOOL updatetext)
{
	// Get paint area for the big static control
	CWnd* pStatic ;
	CRect  tiprect, bigrect, 
				 seprect, didyourect, bulbrect;

	CBrush brush;
	brush.CreateStockObject(WHITE_BRUSH);

	pStatic = GetDlgItem(IDC_STATIC1);
	pStatic->GetWindowRect(&bigrect);
	ScreenToClient(&bigrect);
	
	pStatic = GetDlgItem(IDC_TIPSTRING);
	pStatic->GetWindowRect(&tiprect);
	ScreenToClient(&tiprect);
	
	pStatic = (CStatic*)GetDlgItem (IDB_LIGHTBULB) ;
	pStatic->GetWindowRect (bulbrect) ;

	if (!updatetext)
	{	
		// Paint the background white.
	#ifndef _WIN32
		bigrect.InflateRect (-1,-1) ;
	#endif


#ifndef _WIN32
		FillSolidRect(pdc, bigrect, ::GetSysColor(COLOR_WINDOW));	
#else
		pdc->FillSolidRect(bigrect, ::GetSysColor(COLOR_WINDOW));	
#endif

		seprect = bigrect ;
	
		// Paint left side gray
	
		bigrect.right = tiprect.left - 10 ;
		CBrush gbrush;
#if 0
		gbrush.CreateStockObject(GRAY_BRUSH);
#else
		gbrush.CreateSolidBrush( ::GetSysColor(COLOR_BTNSHADOW)) ;
#endif


#ifndef _WIN32
		FillSolidRect(pdc, bigrect, ::GetSysColor(COLOR_BTNSHADOW));	
#else
		pdc->FillSolidRect(bigrect, ::GetSysColor(COLOR_BTNSHADOW));	
#endif
		pdc->SetBkColor(::GetSysColor(COLOR_WINDOW)) ;
	
		// Load bitmap and get dimensions of the bitmap
		CBitmap bmp;
		bmp.LoadBitmap(IDB_LIGHTBULB);
		BITMAP bmpInfo;	
	#ifdef _WIN32	
		bmp.GetBitmap(&bmpInfo);
	#else
		bmp.GetObject(sizeof(BITMAP), &bmpInfo) ;
	#endif	
		// Draw bitmap in top corner and validate only top portion of window
		CDC dcTmp;
		ScreenToClient (bulbrect) ;
		dcTmp.CreateCompatibleDC(pdc);
		dcTmp.SelectObject(&bmp);
		bulbrect.bottom = bmpInfo.bmHeight + bulbrect.top;
		bulbrect.right = bmpInfo.bmWidth + bulbrect.left;
		dcTmp.SelectObject(&gbrush) ;
		dcTmp.ExtFloodFill(0,0, 0x00ffffff, FLOODFILLSURFACE) ;
#if 1
		pdc->BitBlt(bulbrect.left, bulbrect.top, bulbrect.Width(), bulbrect.Height(), 
			&dcTmp, 0, 0, SRCCOPY);
#endif
			
		seprect.top = bulbrect.bottom + 6 ;	
		seprect.bottom = seprect.top + 1 ;

#ifndef _WIN32
		FillSolidRect(pdc, seprect, ::GetSysColor(COLOR_BTNSHADOW));
#else
		pdc->FillSolidRect(seprect, ::GetSysColor(COLOR_BTNSHADOW));
#endif
		pdc->SetBkColor(::GetSysColor(COLOR_WINDOW)) ;
    }

	CFont font, fonttext ;
	BOOL fok = font.CreateFont(22, 0, 0, 0, FW_BOLD, 0, 0, 0,
					  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
					  DEFAULT_PITCH|FF_SWISS, "Times");

	fok = fonttext.CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0,
					  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
					  DEFAULT_PITCH|FF_SWISS, "Arial");

	CFont *pOldFont = pdc->SelectObject(&font);
	// Draw out "Did you know..." message next to the bitmap
	CString strMessage;
	strMessage.LoadString(CG_IDS_DIDYOUKNOW);
	didyourect = tiprect ;
	didyourect.top = bulbrect.top ;
	pdc->DrawText(strMessage, -1, didyourect, DT_SINGLELINE);
	
	pdc->SelectObject(&fonttext) ;


	// efface le rectangle contenant le text


#ifndef _WIN32
	FillSolidRect(pdc, tiprect, ::GetSysColor(COLOR_WINDOW));
#else
	pdc->FillSolidRect(tiprect, ::GetSysColor(COLOR_WINDOW));
#endif
	pdc->DrawText(m_strTip, -1, tiprect, DT_WORDBREAK);
	pdc->SelectObject(pOldFont) ;
}
