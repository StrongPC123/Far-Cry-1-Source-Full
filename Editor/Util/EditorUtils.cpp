////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   EditorUtils.cpp
//  Version:     v1.00
//  Created:     30/11/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "EditorUtils.h"
#include <malloc.h>

//////////////////////////////////////////////////////////////////////////
void HeapCheck::Check( const char *file,int line )
{
	#ifdef _DEBUG
	/* Check heap status */
   int heapstatus = _heapchk();
   switch( heapstatus )
   {
   case _HEAPOK:
      break;
   case _HEAPEMPTY:
      break;
   case _HEAPBADBEGIN:
			{
				CString str;
				str.Format( "Bad Start of Heap, at file %s line:%d",file,line );
				MessageBox( NULL,str,"Heap Check",MB_OK );
			}
      break;
   case _HEAPBADNODE:
			{
				CString str;
				str.Format( "Bad Node in Heap, at file %s line:%d",file,line );
				MessageBox( NULL,str,"Heap Check",MB_OK );
			}
      break;
   }
	#endif
}

//////////////////////////////////////////////////////////////////////////
BOOL CMFCUtils::LoadTrueColorImageList( CImageList &imageList,UINT nIDResource,int nIconWidth,COLORREF colMaskColor )
{
	CBitmap bitmap;
	BITMAP bmBitmap;
	if (!bitmap.Attach(LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(nIDResource),IMAGE_BITMAP, 0, 0,LR_DEFAULTSIZE|LR_CREATEDIBSECTION)))
		return FALSE;
	if (!bitmap.GetBitmap(&bmBitmap))
		return FALSE;
	CSize		cSize(bmBitmap.bmWidth, bmBitmap.bmHeight); 
	RGBTRIPLE*	rgb		= (RGBTRIPLE*)(bmBitmap.bmBits);
	int	nCount	= cSize.cx/nIconWidth;
	if (!imageList.Create(nIconWidth, cSize.cy, ILC_COLOR24|ILC_MASK, nCount, 0))
		return FALSE;

	if (imageList.Add(&bitmap,colMaskColor) == -1)
		return FALSE;
	return TRUE;
}