////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   splitterwndex.h
//  Version:     v1.00
//  Created:     24/4/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: CSplitterWndEx class.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __splitterwndex_h__
#define __splitterwndex_h__

#if _MSC_VER > 1000
#pragma once
#endif


// CSplitterWndEx

class CSplitterWndEx : public CSplitterWnd
{
public:
	DECLARE_DYNAMIC(CSplitterWndEx)

	CSplitterWndEx();
	~CSplitterWndEx();

	virtual CWnd* GetActivePane(int* pRow = NULL, int* pCol = NULL);
	//! Assign any Window to splitter window pane.
	void SetPane( int row,int col,CWnd *pWnd,SIZE sizeInit );
	// Ovveride this for flat look.
	void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rectArg);

protected:
	DECLARE_MESSAGE_MAP()
};


#endif __splitterwndex_h__