////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   imagegif.h
//  Version:     v1.00
//  Created:     15/3/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __imagegif_h__
#define __imagegif_h__

#if _MSC_VER > 1000
#pragma once
#endif

class CImageGif 
{
public:
	bool Load( const CString &fileName,CImage &outImage );
};


#endif // __imagegif_h__
