////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   thumbnailgenerator.cpp
//  Version:     v1.00
//  Created:     18/3/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "thumbnailgenerator.h"

#include <io.h>
#include <I3DEngine.h>

CThumbnailGenerator::CThumbnailGenerator(void)
{
}

CThumbnailGenerator::~CThumbnailGenerator(void)
{
}

// Get directory contents.
static bool scan_directory( const CString &root,const CString &path,const CString &file,std::vector<CString> &files, bool recursive )
{
	struct __finddata64_t c_file;
	intptr_t hFile;

	CString fullPath = root + path + file;
	if( (hFile = _findfirst64( fullPath, &c_file )) == -1L ) {
		return false;
	}	else  {
		// Find the rest of the .c files.
		do {
			if ((c_file.attrib & _A_SUBDIR) && recursive) {
				// If recursive.
				if (c_file.name[0] != '.') {
					scan_directory( root,path + c_file.name + "\\",file,files,recursive );
				}
				continue;
			}
			files.push_back( path + c_file.name );
			//FileInfo fi;
			//fi.attrib = c_file.attrib;
			//fi.name = path + c_file.name;
/*			
			// Add . after file name without extension.
			if (fi.name.find('.') == CString::npos) {
				fi.name.append( "." );
			}
*/
			//fi.size = c_file.size;
			//fi.time = c_file.time_write;
			//files.push_back( fi );

		}	while (_findnext64( hFile, &c_file ) == 0);
		_findclose( hFile );
	}
	return true;
}

inline void GetThumbFileTime( const char *fileName,FILETIME &time )
{
	HANDLE hFile = CreateFile( fileName, GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL );
	if (INVALID_HANDLE_VALUE != hFile )
	{
		GetFileTime( hFile,NULL,NULL,&time );
		CloseHandle( hFile );
	}
}

inline void SetThumbFileTime( const char *fileName,FILETIME &time )
{
	HANDLE hFile = CreateFile( fileName, GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL );
	if (INVALID_HANDLE_VALUE != hFile )
	{
		SetFileTime( hFile,NULL,NULL,&time );
		CloseHandle( hFile );
	}
}

void CThumbnailGenerator::GenerateForDirectory( const CString &path )
{
	//////////////////////////////////////////////////////////////////////////
	std::vector<CString> files;
	//CString dir = GetIEditor()->GetMasterCDFolder();
	CString dir = path;
	scan_directory( dir,"","*.*",files,true );

	I3DEngine *engine = GetIEditor()->Get3DEngine();
	
	int thumbSize = 128;
	CImage image;
	image.Allocate( thumbSize,thumbSize );

  char fdir[_MAX_DIR];
  char fname[_MAX_FNAME];
  char fext[_MAX_EXT];
	char bmpFile[1024];

	GetIEditor()->ShowConsole( true );
	CWaitProgress wait( "Generating CGF Thumbnails" );
	for (int i = 0; i < files.size(); i++)
	{
		CString file = dir+files[i];
		_splitpath( file,0,fdir,fname,fext );
		
		//if (stricmp(fext,".cgf") != 0 && stricmp(fext,".bld") != 0)
		if (stricmp(fext,".cgf") != 0)
			continue;

		if (!wait.Step( 100*i/files.size() ))
			break;

		_makepath( bmpFile,0,fdir,fname,".tmb" );
		FILETIME ft1,ft2;
		GetThumbFileTime( file,ft1 );
		GetThumbFileTime( bmpFile,ft2 );
		// Both cgf and bmp have same time stamp.
		if (ft1.dwHighDateTime == ft2.dwHighDateTime && ft1.dwLowDateTime == ft1.dwLowDateTime)
			continue;

		//CLogFile::FormatLine( "Generating thumbnail for %s...",file );

		IStatObj *obj = engine->MakeObject( file );
		if (obj)
		{
			obj->MakeObjectPicture( (unsigned char*)image.GetData(),thumbSize );
			
			CImageUtil::SaveBitmap( bmpFile,image,false );
			SetThumbFileTime( bmpFile,ft1 );
			SetFileAttributes( bmpFile,FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_NOT_CONTENT_INDEXED );
			engine->ReleaseObject(obj);
		}
	}
	//GetIEditor()->ShowConsole( false );
}

void CThumbnailGenerator::GenerateForFile( const CString &fileName )
{
	I3DEngine *engine = GetIEditor()->Get3DEngine();
	
	int thumbSize = 128;
	CImage image;
	image.Allocate( thumbSize,thumbSize );

  char fdir[_MAX_DIR];
  char fname[_MAX_FNAME];
  char fext[_MAX_EXT];
	char bmpFile[1024];

	_splitpath( fileName,0,fdir,fname,fext );
		
	_makepath( bmpFile,0,fdir,fname,".tmb" );
	FILETIME ft1,ft2;
	GetThumbFileTime( fileName,ft1 );
	GetThumbFileTime( bmpFile,ft2 );
	// Both cgf and bmp have same time stamp.
	if (ft1.dwHighDateTime == ft2.dwHighDateTime && ft1.dwLowDateTime == ft1.dwLowDateTime)
		return;

	IStatObj *obj = engine->MakeObject( fileName,NULL,evs_ShareAndSortForCache );
	if (obj)
	{
		obj->MakeObjectPicture( (unsigned char*)image.GetData(),thumbSize );
		
		CImageUtil::SaveBitmap( bmpFile,image,false );
		SetThumbFileTime( bmpFile,ft1 );
		SetFileAttributes( bmpFile,FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_NOT_CONTENT_INDEXED );
		engine->ReleaseObject(obj);
	}
}