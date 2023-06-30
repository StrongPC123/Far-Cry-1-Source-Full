////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   AVI_Writer.h
//  Version:     v1.00
//  Created:     13/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __AVI_Writer_h__
#define __AVI_Writer_h__
#pragma once

//////////////////////////////////////////////////////////////////////////
// Can save sequence of images to the AVI file.
//////////////////////////////////////////////////////////////////////////
class CAVI_Writer
{
public:
	CAVI_Writer();
	~CAVI_Writer();

	bool OpenFile( const char *filename,int width,int height );
	bool CloseFile();
	bool AddFrame( CImage &image );
private:
	class CAVIGenerator *m_pAVI;
};

#endif // __AVI_Writer_h__

