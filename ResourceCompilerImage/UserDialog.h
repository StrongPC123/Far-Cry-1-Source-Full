#pragma once

class CImageCompiler;

#include "RelativeMouseInput.h"						// CRelativeMouseInput


class CUserDialog
{
public:

	//! constructor
	CUserDialog( void );
	//! destructor
	virtual ~CUserDialog( void );

	//!
	bool DoModal( CImageCompiler *inpImageCompiler );

	// ---------------------------------------------------------------------------------------

private:
	CImageCompiler *			m_pImageCompiler;		//!<
	float									m_fShiftX;					//!< in texture coordinates [0..1] initially 0.5
	float									m_fShiftY;					//!< in texture coordinates [0..1] initially 0.5
	int										m_iScale16;					//!< 16 is the default

	HWND									m_hWindow;					//!< main window handle

	// sub windows (in the tab control)
	HWND									m_hTab_Normalmapgen;//!< window handle
	HWND									m_hTab_Saveformat;	//!< window handle

	const int							m_iPreviewWidth;		//!<
	const int							m_iPreviewHeight;		//!<

	CRelativeMouseInput		m_RelMouse;					//!< for relative mouse movement

	//!
	static BOOL CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	//! redirect to the parent window
	static BOOL CALLBACK WndProcRedirect( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	//!
	void Draw( HDC inHdc );

	//!
	void MouseMessage( const DWORD indwButtons, const int iniRelX , const int iniRelY, int iniRelZ );

	//!
	void UpdateWindowTitle( void );

	void UpdatePixelFormatDesc( void );

	void UpdatePreview( void );

	void CreateDialogItems( void );

	void GetDataFromDialog( void );

	//! makes the dialog visible too
	void SetDataToDialog( void );

	void SetPropertyTab( const int iniNo ); 
};
