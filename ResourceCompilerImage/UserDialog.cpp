#include "StdAfx.h"
#include "userdialog.h"							// CUserDialog
#include "resource.h"								// IDD_ ...
#include <assert.h>									// assert()
#include "ImageCompiler.h"					// CImageCompiler
#include "RelativeMouseInput.h"			// CRelativeMouseInput
#include <windowsx.h>								// ComboBox_GetCurSel

#include <commctrl.h>								// TCITEM
#include <vector>
#include "ICfgFile.h"



// Class name for this application's window class.
#define WINDOW_CLASSNAME      "UserDialog"


// constructor
CUserDialog::CUserDialog( void ) :m_iPreviewWidth(256), m_iPreviewHeight(256)
{
	m_pImageCompiler=0;
	m_fShiftX=0.5f;m_fShiftY=0.5f;
	m_iScale16=16;
	m_hTab_Normalmapgen=0;
	m_hTab_Saveformat=0;
	m_hWindow=0;
}

// destructor
CUserDialog::~CUserDialog( void )
{
	assert(!m_pImageCompiler);			// should be 0 outside of DoModal call
}


//**************************************************************************** 
// // FUNCTION: CenterWindow (HWND, HWND) 
// // PURPOSE:  Center one window over another 
// // COMMENTS: 
// //      Dialog boxes take on the screen position that they were designed at, 
//      which is not always appropriate. Centering the dialog over a particular 
//      window usually results in a better position. 
// //***************************************************************************  
void CenterWindow( HWND hwndChild, HWND hwndParent=0 ) 
{
	RECT    rChild, rParent;     
	int     wChild, hChild, wParent, hParent;     
	int     wScreen, hScreen, xNew, yNew;     
	HDC     hdc;      

	// Get the Height and Width of the child window     
	GetWindowRect (hwndChild, &rChild);     
	wChild = rChild.right - rChild.left;     
	hChild = rChild.bottom - rChild.top;      

	// Get the display limits     
	hdc = GetDC (hwndChild);     
	wScreen = GetDeviceCaps (hdc, HORZRES);     
	hScreen = GetDeviceCaps (hdc, VERTRES);     
	ReleaseDC (hwndChild, hdc);      

	// Get the Height and Width of the parent window     
	if(hwndParent)			
	{
		GetWindowRect (hwndParent, &rParent);     
		wParent = rParent.right - rParent.left;     
		hParent = rParent.bottom - rParent.top;      
	}
	else // 0 means it's the screen
	{
		wParent = wScreen;     
		hParent = hScreen;
		rParent.left=0;
		rParent.top=0;
	}

	// Calculate new X position, then adjust for screen     
	xNew = rParent.left + ((wParent - wChild) /2);     
	
	if(xNew<0) xNew = 0;    
		else if((xNew+wChild)>wScreen)         
			xNew = wScreen - wChild;      

	// Calculate new Y position, then adjust for screen     
	yNew = rParent.top  + ((hParent - hChild) /2);    
	if(yNew<0) yNew = 0;     
		else if((yNew+hChild)>hScreen)	yNew = hScreen - hChild;      

	// Set it, and return     
//	SetWindowPos(hwndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER); 
	MoveWindow(hwndChild, xNew, yNew, wChild, hChild, TRUE); 
} 


bool CUserDialog::DoModal( CImageCompiler *inpImageCompiler )
{
	m_pImageCompiler=inpImageCompiler;

	m_fShiftX=0.5f;
	m_fShiftY=0.5f;
	m_iScale16=16;		// 1:1

	// zoom out if image is too big
	{
		int iTexWidth=(int)m_pImageCompiler->GetWidth(), iTexHeight=(int)m_pImageCompiler->GetHeight();

		while((iTexWidth>256 || iTexHeight>256) && m_iScale16!=1)
		{
			iTexWidth>>=1;
			iTexHeight>>=1;
			m_iScale16>>=1;
		}
	}

	m_hWindow=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_USERDIALOG),0,CUserDialog::WndProc);

	if( !m_hWindow ) 
	{
		DWORD h=GetLastError();
		m_pImageCompiler=0;
		return(false);
	}

	// store this pointer	
	SetWindowLongPtr(m_hWindow,DWL_USER,(LONG)(LONG_PTR)this);

//	CenterWindow(m_hWindow);			// here I can't see my tab items

	CreateDialogItems();

	CenterWindow(m_hWindow);				// here it's moving

	SetDataToDialog();

	UpdateWindowTitle();

	UpdatePreview();			// call after IDC_PIXELFORMAT is created and image conversion is done

	ShowWindow(m_hWindow,SW_SHOW);

	// message loop
	{
		MSG Msg;

		while(GetMessage(&Msg, NULL, 0, WM_USER)) 
		{
			if (WM_CLOSE == Msg.message) break;
			else
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
	}

	DestroyWindow(m_hWindow);
	m_hWindow=0;
	m_pImageCompiler=0;

	return(true);
}


void CUserDialog::CreateDialogItems( void )
{
		// tab control
	{
		HWND hwnd=GetDlgItem(m_hWindow,IDC_PROPTAB);assert(hwnd);
		TCITEM tie; 
		RECT wnp;

    tie.mask = TCIF_TEXT | TCIF_IMAGE; 
    tie.iImage = -1; 
    tie.pszText = "Normalmap"; 
    TabCtrl_InsertItem(hwnd,0,&tie);

    tie.pszText = "File Output"; 
    TabCtrl_InsertItem(hwnd,1,&tie);

		TabCtrl_SetCurSel(hwnd,1);

		GetWindowRect(hwnd,&wnp);

		// sub windows
		m_hTab_Normalmapgen=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_TAB_NORMALMAPGEN),m_hWindow,CUserDialog::WndProcRedirect);assert(m_hTab_Normalmapgen);
		SetWindowPos(m_hTab_Normalmapgen, HWND_TOP, wnp.left, wnp.top, 0,0,SWP_NOSIZE); 

		m_hTab_Saveformat=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_TAB_SAVEFORMAT),m_hWindow,CUserDialog::WndProcRedirect); assert(m_hTab_Saveformat);
		SetWindowPos(m_hTab_Saveformat, HWND_TOP, wnp.left, wnp.top, 0,0,SWP_NOSIZE); 

		// make dialog visible and set right tab
		SetPropertyTab(1);
	}

	// templates
	{
		HWND hwnd=GetDlgItem(m_hWindow,IDC_TEMPLATECOMBO);assert(hwnd);

		// Baustelle
		SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)"<CUSTOM>");
		
		for(int i = 1;; i++)
		{
			const char *name = m_pImageCompiler->m_pCC->presets->GetSectionName(i);
			if(!name) break;
			SendMessage(hwnd, CB_ADDSTRING, 0,(LPARAM)name);
		};
		/*
		SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)"Skybox");
		SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)"Normalmap_low");
		SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)"Normalmap_high");
		SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)"Texture_low");
		SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)"Texture_high");
		*/

		SendMessage(hwnd,CB_SETCURSEL,0,0);		// first 0 is setting the used template
	}

	// pixelformats
	{
		HWND hwnd=GetDlgItem(m_hTab_Saveformat,IDC_PIXELFORMAT);assert(hwnd);

		CString sEntry;

		int iCount=m_pImageCompiler->GetPixelFormatCount();

		for(int i=0;i<iCount;i++)
			SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)m_pImageCompiler->GetPixelFormatName(i));

		UpdatePixelFormatDesc();
	}

	// mipmaps
	{
		HWND hwnd=GetDlgItem(m_hTab_Saveformat,IDC_MIPMAPS);assert(hwnd);

		SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)"max(1)");
		SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)"none(0)");
	}

	// reduce resolution
	{
		HWND hwnd=GetDlgItem(m_hTab_Saveformat,IDC_REDUCERES);assert(hwnd);

		SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)"none");
		SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)"1 (1/4 memory)");
		SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)"2 (1/16 memory)");
	}

	// dither mode
	{
		HWND hwnd=GetDlgItem(m_hTab_Saveformat,IDC_DITHERMODE);assert(hwnd);

		SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)"none(0)");
		SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)"simple(1)");
	}
}


void CUserDialog::SetDataToDialog( void )
{
	HWND hwnd;
	bool isnotpreset = m_pImageCompiler->m_Props.m_sPreset=="";

	// preview part in main window

	hwnd=GetDlgItem(m_hWindow,IDC_PREVIEWALPHA);assert(hwnd);
	Button_SetCheck(hwnd,m_pImageCompiler->m_Props.m_bPreviewAlpha?1:0);

	hwnd=GetDlgItem(m_hWindow,IDC_PREVIEWFILTERED);assert(hwnd);
	Button_SetCheck(hwnd,m_pImageCompiler->m_Props.m_bPreviewFiltered?1:0);

	hwnd=GetDlgItem(m_hWindow,IDC_PREVIEWTILED);assert(hwnd);
	Button_SetCheck(hwnd,m_pImageCompiler->m_Props.m_bPreviewTiled?1:0);
	
	// main window:

	hwnd=GetDlgItem(m_hWindow,IDC_SHOWUSERDIALOG);assert(hwnd);
	Button_SetCheck(hwnd,m_pImageCompiler->m_Props.m_bUserDialog?1:0);
	EnableWindow(hwnd, isnotpreset);

	hwnd=GetDlgItem(m_hWindow,IDC_TEMPLATECOMBO);assert(hwnd);
	SendMessage(hwnd,CB_SETCURSEL,m_pImageCompiler->m_pCC->presets->Find(m_pImageCompiler->m_Props.m_sPreset),0);

	// sub window: save format

	hwnd=GetDlgItem(m_hTab_Saveformat,IDC_DITHERMODE);assert(hwnd);
	SendMessage(hwnd,CB_SETCURSEL,m_pImageCompiler->m_Props.m_dwDitherMode,0);
	EnableWindow(hwnd, isnotpreset);

	hwnd=GetDlgItem(m_hTab_Saveformat,IDC_MIPMAPS);assert(hwnd);
	SendMessage(hwnd,CB_SETCURSEL,m_pImageCompiler->m_Props.m_bMipmaps?0:1,0);
	EnableWindow(hwnd, isnotpreset);
	
	hwnd=GetDlgItem(m_hTab_Saveformat,IDC_REDUCERES);assert(hwnd);
	SendMessage(hwnd,CB_SETCURSEL,m_pImageCompiler->m_Props.m_dwReduceResolution,0);
	EnableWindow(hwnd, isnotpreset);

	hwnd=GetDlgItem(m_hTab_Saveformat,IDC_PIXELFORMAT);assert(hwnd);
	SendMessage(hwnd,CB_SETCURSEL,m_pImageCompiler->m_Props.m_iDestPixelFormat,0);
	EnableWindow(hwnd, isnotpreset);
}




void CUserDialog::GetDataFromDialog( void )
{
	HWND hwnd;
	// preview part in main window

	hwnd=GetDlgItem(m_hWindow,IDC_PREVIEWALPHA);assert(hwnd);
	m_pImageCompiler->m_Props.m_bPreviewAlpha=Button_GetCheck(hwnd)!=0;

	hwnd=GetDlgItem(m_hWindow,IDC_PREVIEWFILTERED);assert(hwnd);
	m_pImageCompiler->m_Props.m_bPreviewFiltered=Button_GetCheck(hwnd)!=0;

	hwnd=GetDlgItem(m_hWindow,IDC_PREVIEWTILED);assert(hwnd);
	m_pImageCompiler->m_Props.m_bPreviewTiled=Button_GetCheck(hwnd)!=0;

	// main window:

	hwnd=GetDlgItem(m_hWindow,IDC_SHOWUSERDIALOG);assert(hwnd);
	m_pImageCompiler->m_Props.m_bUserDialog=Button_GetCheck(hwnd)!=0;
	
	hwnd=GetDlgItem(m_hWindow,IDC_TEMPLATECOMBO);assert(hwnd);
	const char *pname = m_pImageCompiler->m_pCC->presets->GetSectionName(ComboBox_GetCurSel(hwnd));
	m_pImageCompiler->m_Props.m_sPreset = pname ? pname : "";

	// sub window: save format

	hwnd=GetDlgItem(m_hTab_Saveformat,IDC_PIXELFORMAT);assert(hwnd);
	m_pImageCompiler->m_Props.m_iDestPixelFormat=ComboBox_GetCurSel(hwnd);
	
	hwnd=GetDlgItem(m_hTab_Saveformat,IDC_MIPMAPS);assert(hwnd);
	m_pImageCompiler->m_Props.m_bMipmaps=(ComboBox_GetCurSel(hwnd)==0);

	hwnd=GetDlgItem(m_hTab_Saveformat,IDC_REDUCERES);assert(hwnd);
	m_pImageCompiler->m_Props.m_dwReduceResolution=ComboBox_GetCurSel(hwnd);

	hwnd=GetDlgItem(m_hTab_Saveformat,IDC_DITHERMODE);assert(hwnd);
	m_pImageCompiler->m_Props.m_dwDitherMode=ComboBox_GetCurSel(hwnd);

	m_pImageCompiler->SetPresetSettings();
}



void CUserDialog::UpdateWindowTitle( void )
{
	assert(m_pImageCompiler);

	CString Zoom;		Zoom.Format("  Zoom:%d%%",(100*m_iScale16)/16);

	CString title=m_pImageCompiler->GetSourceFilename() + Zoom;
	
	SetWindowText(m_hWindow,title.GetBuffer());
}


//
void CUserDialog::Draw( HDC inHdc )
{
	int iGap=4;
	int iTexWidth=(int)m_pImageCompiler->GetWidth();
	int iTexHeight=(int)m_pImageCompiler->GetHeight();

	RECT rec;

	rec.left=0;
	rec.top=0;
	rec.right=rec.left+m_iPreviewWidth;
	rec.bottom=rec.top+m_iPreviewHeight;

	// left side = original
	assert(m_pImageCompiler);
	if(!m_pImageCompiler->BlitTo(m_hWindow,rec,m_fShiftX,m_fShiftY,m_iScale16,true))
		FillRect(inHdc,&rec,GetSysColorBrush(COLOR_3DFACE));

	rec.left=m_iPreviewWidth+iGap*2+1;
	rec.right=rec.left+m_iPreviewWidth;

	// right side = destination
	assert(m_pImageCompiler);
	if(!m_pImageCompiler->BlitTo(m_hWindow,rec,m_fShiftX,m_fShiftY,m_iScale16,false))
		FillRect(inHdc,&rec,GetSysColorBrush(COLOR_3DFACE));
}


void CUserDialog::MouseMessage( const DWORD indwButtons, const int iniRelX , const int iniRelY, int iniRelZ )
{
	assert(m_pImageCompiler);

	int iWidth=(int)m_pImageCompiler->GetWidth(),iHeight=(int)m_pImageCompiler->GetHeight(); 

	float fOldShiftX=m_fShiftX,fOldShiftY=m_fShiftY;
	int iOldScale16=m_iScale16;

	m_fShiftX-=iniRelX*(16.0f/m_iScale16/iWidth);
	m_fShiftY-=iniRelY*(16.0f/m_iScale16/iHeight);

	while(iniRelZ)
	{
		if(iniRelZ>0)
		{
			m_iScale16/=2;
			iniRelZ-=WHEEL_DELTA;
		}
		else
		{
			m_iScale16*=2;
			iniRelZ+=WHEEL_DELTA;
		}
	}

	if(m_iScale16<4)			m_iScale16=4;
	if(m_iScale16>16*16)	m_iScale16=16*16;

	bool bNoMovementPossible = m_pImageCompiler->ClampBlitOffset(m_iPreviewWidth,m_iPreviewHeight,m_fShiftX,m_fShiftY,m_iScale16);

	if(fOldShiftX!=m_fShiftX || fOldShiftY!=m_fShiftY || iOldScale16!=m_iScale16)
	{
		UpdateWindowTitle();			// Zoom:%d

		RECT rect;

		GetClientRect(m_hWindow,&rect);
		rect.bottom=m_iPreviewHeight;

		// update window
		InvalidateRect(m_hWindow,&rect,bNoMovementPossible);
		UpdateWindow(m_hWindow);
	}
}



void CUserDialog::UpdatePixelFormatDesc( void )
{
	HWND hwnds=GetDlgItem(m_hTab_Saveformat,IDC_PIXELFORMATDESC);			assert(hwnds);

	SetWindowText(hwnds,m_pImageCompiler->GetPixelFormatDesc(m_pImageCompiler->m_Props.m_iDestPixelFormat));
}


void CUserDialog::UpdatePreview( void )
{
	// preview images
	{
		RECT rect;

		GetClientRect(m_hWindow,&rect);
		rect.bottom=m_iPreviewHeight;

		// do image conversion
		if(m_pImageCompiler->RunWithProperties(false))
			InvalidateRect(m_hWindow,&rect,false);					// don't erase background
		else
			InvalidateRect(m_hWindow,&rect,true);						// erase background
	}

	// info text
	{
		HWND hwndc=GetDlgItem(m_hTab_Saveformat,IDC_PIXELFORMAT);					assert(hwndc);

		int iNo=ComboBox_GetCurSel(hwndc); 

		CString sLeftInfo=m_pImageCompiler->GetInfoString(true);
		CString sRightInfo=m_pImageCompiler->GetInfoString(false);

		HWND hwnd1=GetDlgItem(m_hWindow,IDC_LEFTPREVIEWINFO);			assert(hwnd1);
		HWND hwnd2=GetDlgItem(m_hWindow,IDC_RIGHTPREVIEWINFO);		assert(hwnd2);

		SetWindowText(hwnd1,sLeftInfo.GetBuffer());
		SetWindowText(hwnd2,sRightInfo.GetBuffer());
	}
}


void CUserDialog::SetPropertyTab( const int iniNo )
{
	assert(iniNo>=0 && iniNo<=1);
	assert(m_hTab_Normalmapgen);
	assert(m_hTab_Saveformat);

	ShowWindow(m_hTab_Normalmapgen,iniNo==0 ? SW_SHOW:SW_HIDE);
	ShowWindow(m_hTab_Saveformat,iniNo==1 ? SW_SHOW:SW_HIDE);
}

/*
//-----------------------------------------------------------------------------
// Name: WndProc()
// Desc: Static msg handler which passes messages to the application class.
//-----------------------------------------------------------------------------
LRESULT CALLBACK CUserDialog::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
*/
// WndProc vom Treiber auswahl Dialog
BOOL CALLBACK CUserDialog::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CUserDialog *This=(CUserDialog *)(LONG_PTR)GetWindowLongPtr(hWnd,DWL_USER);

	switch(uMsg)
	{
		case WM_NOTIFY:
			{
				WORD wID=LOWORD(wParam);

				switch(wID)
				{
					case IDC_PROPTAB:
						{
							HWND hwnd=GetDlgItem(hWnd,IDC_PROPTAB);						assert(hwnd);
							int iPage = TabCtrl_GetCurSel(hwnd); 

							This->SetPropertyTab(iPage);
						}
						return(TRUE);
				}
			}
			return(FALSE);

		case WM_COMMAND:											//
			{
				WORD wID=LOWORD(wParam);

				switch(wID)
				{
					case IDOK:
						This->GetDataFromDialog();
						This->m_pImageCompiler->RunWithProperties(true);
						if(!This->m_pImageCompiler->UpdateAndSaveConfigFile())
							MessageBox(0,"Error while saving the config file (write protected?)","ResourceCompiler Image Error",MB_OK);

						SendMessage(hWnd,WM_CLOSE,0,0);
						return(TRUE);

					case IDCANCEL:
						PostQuitMessage(0);
						return(TRUE);

					case IDC_ZOOMIN:
						This->MouseMessage(0,0,0,-WHEEL_DELTA);
						return(TRUE);

					case IDC_ZOOMOUT:
						This->MouseMessage(0,0,0,WHEEL_DELTA);
						return(TRUE);

					case IDC_TEMPLATECOMBO:
					case IDC_DITHERMODE:										// dithering mode has changed
					case IDC_REDUCERES:											// reduce resolution has changed
					case IDC_MIPMAPS:												// mipmaps has changed
					case IDC_PIXELFORMAT:										// pixelformat has changed
						if(HIWORD(wParam)==CBN_SELCHANGE)
						{
							This->GetDataFromDialog();
							This->UpdatePixelFormatDesc();
							This->SetDataToDialog();

							This->UpdatePreview();
							UpdateWindow(This->m_hWindow);
						}
						return(TRUE);

					case IDC_PREVIEWALPHA:
					case IDC_PREVIEWTILED:
					case IDC_PREVIEWFILTERED:
						{
							This->GetDataFromDialog();

							This->UpdatePreview();
							UpdateWindow(This->m_hWindow);
						}
						return(TRUE);
				}
			}
			return(TRUE);

		case WM_PAINT:											//
			{
				assert(This);			if(!This)return(FALSE);

				PAINTSTRUCT	ps;
				HDC hdc = BeginPaint (hWnd, &ps);

				This->Draw(hdc);	//,ps.rcPaint);
//				This->Draw(hdc,rChild.right-rChild.left,rChild.bottom-rChild.top);	//,ps.rcPaint);

				EndPaint (hWnd, &ps);
			}
			return(TRUE);

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			{
				assert(This);			if(!This)return(FALSE);

				int iY=(int)HIWORD(lParam);

				if(iY<=This->m_iPreviewHeight)
				{
					This->m_RelMouse.OnButtonDown(hWnd);

					SetFocus(This->m_hWindow);
				}
			}
			return(TRUE);

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			{
				assert(This);			if(!This)return(FALSE);

				if(wParam==0)
					This->m_RelMouse.OnButtonUp();	
			}
			return(TRUE);

		case WM_MOUSEMOVE:
			{
				assert(This);			if(!This)return(FALSE);

				bool bButtonDown=(wParam&MK_LBUTTON)!=0 || (wParam&MK_MBUTTON)!=0 || (wParam&MK_RBUTTON)!=0;

				int iY=(int)HIWORD(lParam);

				if(iY<=This->m_iPreviewHeight || This->m_RelMouse.IsCaptured())
				{
					int relX,relY;

					This->m_RelMouse.OnMouseMove(hWnd,bButtonDown,relX,relY);
					This->MouseMessage((DWORD)wParam,relX,relY,0);
				}
			}
			return(TRUE);

		case WM_MOUSEWHEEL:
			{
				assert(This);			if(!This)return(FALSE);

				int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

				This->MouseMessage(0,0,0,-zDelta);
			}
			return(TRUE);

		case WM_CLOSE:
			PostQuitMessage(0);
			return (TRUE);

//		case WM_ERASEBKGND:								//
//			return(TRUE);
	}

//	return DefWindowProc( hWnd, uMsg, wParam, lParam );
  return(FALSE);
}


// redirect to the parent window
BOOL CALLBACK CUserDialog::WndProcRedirect( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HWND hParent=GetParent(hWnd);

	if(hParent)
		return(SendMessage(hParent,uMsg,wParam,lParam)!=0?TRUE:FALSE);

//	return DefWindowProc( hWnd, uMsg, wParam, lParam );
  return(FALSE);
}

