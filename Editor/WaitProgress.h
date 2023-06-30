////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   waitprogress.h
//  Version:     v1.00
//  Created:     10/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __waitprogress_h__
#define __waitprogress_h__

#if _MSC_VER > 1000
#pragma once
#endif

/** CWaitProgress class adds information about lengthy process.
		use it like this:
		CWaitProgress wait;
		wait.SetText( "Long" );
		wait.SetProgress( 35 ); // 35 percent.
*/
class CWaitProgress
{
public:
	CWaitProgress( UINT nIDText, bool bStart = true);
	CWaitProgress( LPCTSTR lpszText, bool bStart = true );
	~CWaitProgress();

	void Start();
	void Stop();
	//! @return true to continue, false to abort lengthy operation.
	bool Step(int nPercentage = -1);
	void SetText(LPCTSTR lpszText);

	static void CancelCurrent() { m_bCancel = true; };

protected:
	CString m_strText;
	bool m_bStarted;
	bool m_bIgnore;
	int m_percent;
	HWND m_hwndProgress;
	CCustomButton m_cancelButton;

	static bool m_bInProgressNow;
	static bool m_bCancel;

	void CreateProgressControl();
};

#endif // __waitprogress_h__