////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   AVI_Writer.cpp
//  Version:     v1.00
//  Created:     13/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AVI_Writer.h"
#include "AVIGenerator.h"

#include "Settings.h"

//////////////////////////////////////////////////////////////////////////
CAVI_Writer::CAVI_Writer()
{
	m_pAVI = 0;
}

//////////////////////////////////////////////////////////////////////////
CAVI_Writer::~CAVI_Writer()
{
	CloseFile();
}

//////////////////////////////////////////////////////////////////////////
bool CAVI_Writer::OpenFile( const char *filename,int width,int height )
{
	m_pAVI = new CAVIGenerator;
	m_pAVI->SetFileName( filename );
	m_pAVI->SetRate(20);
	
	BITMAPINFOHEADER bmi;
	ZeroStruct(bmi);
	bmi.biSize = sizeof(bmi);
	bmi.biWidth = width;
	bmi.biHeight = height;
	bmi.biPlanes = 1;
	bmi.biBitCount = 32;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = 0;
	m_pAVI->SetBitmapHeader( &bmi );
	m_pAVI->SetRate( gSettings.aviSettings.nFrameRate );
	if (FAILED(m_pAVI->InitEngine( gSettings.aviSettings.codec )))
	{
		Warning( "AVI Engine Initialization Failed (%s)",filename );
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CAVI_Writer::CloseFile()
{
	if (m_pAVI)
	{
		m_pAVI->ReleaseEngine();
		delete m_pAVI;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CAVI_Writer::AddFrame( CImage &image )
{
	if (m_pAVI)
	{
		if (!FAILED(m_pAVI->AddFrame( (BYTE*)image.GetData() )))
			return true;
	}
	return false;
}