////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   fileutil.h
//  Version:     v1.00
//  Created:     13/9/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __fileutil_h__
#define __fileutil_h__

#if _MSC_VER > 1000
#pragma once
#endif

/* File types used for File Open dialogs.
 *	
 */
enum ECustomFileType
{
	EFILE_TYPE_ANY,
	EFILE_TYPE_GEOMETRY,
	EFILE_TYPE_TEXTURE,
	EFILE_TYPE_SOUND,
	EFILE_TYPE_LAST,
};

//////////////////////////////////////////////////////////////////////////
class SANDBOX_API CFileUtil
{
public:
	struct FileDesc
	{
		CString filename;
		unsigned int attrib;
		time_t  time_create;    //! -1 for FAT file systems
		time_t  time_access;    //! -1 for FAT file systems
		time_t  time_write;
		int64 size;
	};
	enum ETextFileType
	{
		FILE_TYPE_SCRIPT,
		FILE_TYPE_SHADER
	};

	typedef std::vector<FileDesc> FileArray;

	static bool ScanDirectory( const CString &path,const CString &fileSpec,FileArray &files, bool recursive=true );
	//static bool ScanDirectory( const CString &startDirectory,const CString &searchPath,const CString &fileSpecZ,FileArray &files, bool recursive=true );

	static bool CompileLuaFile( const char *luaFilename );
	static void EditTextFile( const char *txtFile,int line=0,ETextFileType fileType=FILE_TYPE_SCRIPT );

	//! Open file selection dialog.
	static bool SelectFile( const CString &fileSpec,const CString &searchFolder,CString &fullFileName );
	//! Open file selection dialog.
	static bool SelectFiles( const CString &fileSpec,const CString &searchFolder,std::vector<CString> &files );
	
	//! Display OpenFile dialog and allow to select multiple files.
	//! @return true if selected, false if canceled.
	//! @outputFile Inputs and Outputs filename.
	static bool SelectSingleFile( ECustomFileType fileType,CString &outputFile,const CString &filter="",const CString &initialDir="" );

	//! Display OpenFile dialog and allow to select multiple files.
	//! @return true if selected, false if canceled.
	static bool SelectMultipleFiles( ECustomFileType fileType,std::vector<CString> &files,const CString &filter="",const CString &initialDir="" );

	static bool SelectSaveFile( const CString &fileFilter,const CString &defaulExtension,const CString &startFolder,CString &fileName );
	
	//! If file is read-only ask user if he wants to overwrite it.
	//! If yes file is deleted.
	//! @return True if file was deleted.
	static bool OverwriteFile( const char *filename );

	//////////////////////////////////////////////////////////////////////////
	// Interface to Source safe.
	//////////////////////////////////////////////////////////////////////////
	//! Checks out the file from source safe.
	static bool CheckoutFile( const char *filename );

	//! Checks in the file to source safe.
	static bool CheckinFile( const char *filename );

	//! Creates this directory.
	static void CreateDirectory( const char *dir );

	//! Makes a backup file.
	static void BackupFile( const char *filename );
};

#endif // __fileutil_h__
