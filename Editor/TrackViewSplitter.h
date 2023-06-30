////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   trackviewsplitter.h
//  Version:     v1.00
//  Created:     24/4/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: CTrackViewSplitter class.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __trackviewsplitter_h__
#define __trackviewsplitter_h__

#if _MSC_VER > 1000
#pragma once
#endif


// CTrackViewSplitter

class CTrackViewSplitter : public CSplitterWnd
{
	DECLARE_DYNAMIC(CTrackViewSplitter)

	virtual CWnd* GetActivePane(int* pRow = NULL, int* pCol = NULL)
	{
		return GetFocus();
	}

	void SetPane( int row,int col,CWnd *pWnd,SIZE sizeInit );
	// Ovveride this for flat look.
	void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rectArg);
	
public:
	CTrackViewSplitter();
	virtual ~CTrackViewSplitter();

protected:
	DECLARE_MESSAGE_MAP()
};


#endif // __trackviewsplitter_h__