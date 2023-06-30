////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   pathutil.h
//  Version:     v1.00
//  Created:     5/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Utility functions to simplify working with paths.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __pathutil_h__
#define __pathutil_h__
#pragma once

namespace Path
{
	// compatibility with CPortableString
	inline const char* CStr(const CString& s) {return s.GetString();}
	inline char* CStr(CString& s) {return s.GetBuffer();}


	//! Split full file name to path and filename
	//! @param filepath [IN] Full file name inclusing path.
	//! @param path [OUT] Extracted file path.
	//! @param file [OUT] Extracted file (with extension).
	inline void Split( const CString &filepath,CString &path,CString &file )
	{
		char path_buffer[_MAX_PATH];
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		_splitpath( CStr(filepath),drive,dir,fname,ext );
		_makepath( path_buffer,drive,dir,0,0 );
		path = path_buffer;
		_makepath( path_buffer,0,0,fname,ext );
		file = path_buffer;
	}

	//! Extract extension from full specified file path.
	inline CString GetExt( const CString &filepath )
	{
		char ext[_MAX_EXT];
		_splitpath( CStr(filepath),0,0,0,ext );
		if (ext[0] == '.')
			return ext+1;
		
		return ext;
	}

	//! Extract path from full specified file path.
	inline CString GetPath( const CString &filepath )
	{
		char path_buffer[_MAX_PATH];
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		_splitpath( CStr(filepath),drive,dir,0,0 );
		_makepath( path_buffer,drive,dir,0,0 );
		return path_buffer;
	}

	//! Extract file name with extension from full specified file path.
	inline CString GetFile( const CString &filepath )
	{
		char path_buffer[_MAX_PATH];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		_splitpath( CStr(filepath),0,0,fname,ext );
		_makepath( path_buffer,0,0,fname,ext );
		return path_buffer;
	}

	//! Extract file name without extension from full specified file path.
	inline CString GetFileName( const CString &filepath )
	{
		char fname[_MAX_FNAME];
		_splitpath( CStr(filepath),0,0,fname,0 );
		return fname;
	}

	//! Removes the trailing backslash from a given path.
	inline CString RemoveBackslash( const CString &path )
	{
		if (path[0] && path[path.GetLength()-1] == '\\')
			return path.Mid( 0,path.GetLength()-1 );
		return path;
	}

	//!
	inline CString AddBackslash( const CString &path )
	{
		if(path[0] && path[path.GetLength()-1] != '/' && path[path.GetLength()-1] != '\\')
			return path + "\\";
		return path;
	}

	//! Replace extension for given file.
	inline CString RemoveExtension( const CString &filepath )
	{
		char path_buffer[_MAX_PATH];
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		_splitpath( CStr(filepath),drive,dir,fname,0 );
		_makepath( path_buffer,drive,dir,fname,0 );
		int len = (int)strlen(path_buffer);
		// Remove last dot.
		if (len > 0 && path_buffer[len-1] == '.')
			path_buffer[len-1] = 0;
		return path_buffer;
	}

	//! Replace extension for given file.
	inline CString ReplaceExtension( const CString &filepath,const CString &ext )
	{
		char path_buffer[_MAX_PATH];
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		_splitpath( CStr(filepath),drive,dir,fname,0 );
		_makepath( path_buffer,drive,dir,fname, CStr(ext) );
		return path_buffer;
	}

	//! Makes a fully specified file path from path and file name.
	inline CString Make( const CString &path,const CString &file )
	{
		return AddBackslash(path) + file;
	}	

	//! Replace path for given file.
	//! \param srcpath e.g. c:\mastercd
	//! \param destpath e.g. c:\mastercd\temp
	//! \param filepath relative or absolute e.g. c:\mastercd\objects\sss.cgf
	//! \return e.g. c:\mastercd\temp\objects\sss.cgf
	inline CString ReplacePath( const CString &srcpath, const CString &destpath, const CString &filepath )
	{
		int iLen=(int)srcpath.GetLength();

		if(_strnicmp(CStr(srcpath),CStr(filepath),(size_t)iLen)==0)
		{
			// srcpath is leading part in filepath
			return Make( destpath ,filepath.Right((int)filepath.GetLength()-iLen) );
		}
		else
		{
			// srcpath is not a leading part in filepath
			assert(strstr(CStr(filepath),":")==0);		// the following code assumes a relative path name

			return Make(CStr(destpath),CStr(filepath));
		}
	}
};

#endif // __pathutil_h__
