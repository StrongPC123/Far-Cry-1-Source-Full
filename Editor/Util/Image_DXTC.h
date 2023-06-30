////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   image_dxtc.h
//  Version:     v1.00
//  Created:     5/9/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __image_dxtc_h__
#define __image_dxtc_h__
#pragma once

class CImage_DXTC  
{
public:
	bool Load( const char *filename,CImage &outImage );		// true if success

	CImage_DXTC();
	~CImage_DXTC();
};

#endif // __image_dxtc_h__
