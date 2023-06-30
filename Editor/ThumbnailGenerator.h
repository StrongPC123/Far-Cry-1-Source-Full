////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   thumbnailgenerator.h
//  Version:     v1.00
//  Created:     18/3/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __thumbnailgenerator_h__
#define __thumbnailgenerator_h__

#if _MSC_VER > 1000
#pragma once
#endif


class CThumbnailGenerator
{
public:
	CThumbnailGenerator(void);
	~CThumbnailGenerator(void);

	void GenerateForDirectory( const CString &path );
	void GenerateForFile( const CString &fileName );
};

#endif // __thumbnailgenerator_h__