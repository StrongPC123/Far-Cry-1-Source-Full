////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   fileutil.cpp
//  Version:     v1.00
//  Created:     13/9/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "FileUtil.h"
#include "Settings.h"
#include "CheckOutDialog.h"
#include "SrcSafeSettingsDialog.h"

#include "CustomFileDialog.h"
#include <CryFile.h>
#include <io.h>

//////////////////////////////////////////////////////////////////////////
bool CFileUtil::CompileLuaFile( const char *luaFilename )
{
	// Check if this file is in Archive.
	{
		CCryFile file;
		if (file.Open( luaFilename,"rb" ))
		{
			// Check if in pack.
			if (file.IsInPak())
				return true;
		}
	}
	// First try compiling script and see if it have any errors.
	CString LuaCompiler;
	CString CompilerOutput;

	// Create the filepath of the lua compiler
	 char szExeFileName[_MAX_PATH];
	// Get the path of the executable
	GetModuleFileName( GetModuleHandle(NULL), szExeFileName, sizeof(szExeFileName));
	CString exePath = Path::GetPath( szExeFileName );

	LuaCompiler = Path::AddBackslash(exePath) + "LuaCompiler.exe ";

	CString luaFile = luaFilename;
	luaFile.Replace( '/','\\' );

	// Add the name of the Lua file
	CString cmdLine = LuaCompiler + luaFile;

	// Execute the compiler and capture the output
	if (!GetIEditor()->ExecuteConsoleApp( cmdLine,CompilerOutput ))
	{
		AfxMessageBox("Error while executing 'LuaCompiler.exe', make sure the file is in" \
			" your Master CD folder !");
		return false;
	}

	// Check return string
	if (strlen(CompilerOutput) > 0)
	{
		// Errors while compiling file.

		// Show output from Lua compiler
		if (MessageBox( NULL,(CString("Error output from Lua compiler:\r\n") + CompilerOutput +
			CString("\r\nDo you want to edit the file ?")),"Lua Compiler", MB_ICONERROR | MB_YESNO) == IDYES)
		{
			int line = 0;
			CString cmdLine = luaFile;
			int index = CompilerOutput.Find( "at line" );
			if (index >= 0)
			{
				const char *str = CompilerOutput;
				sscanf( &str[index],"at line %d",&line );
			}
			// Open the Lua file for editing
			EditTextFile( luaFile,line );
			return false;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CFileUtil::EditTextFile( const char *txtFile,int line,ETextFileType fileType )
{
	CString file = txtFile;
	file.Replace( '/','\\' );

	CString cmd;
	if (line != 0)
	{
		cmd.Format( "%s/%d/0",(const char*)file,line );
	}
	else
	{
		cmd = file;
	}

	// Check if this file is in Archive.
	{
		CCryFile cryfile;
		if (cryfile.Open( file,"rb" ))
		{
			// Check if in pack.
			if (cryfile.IsInPak())
			{
				const char *sPakName = cryfile.GetPakPath();
				// Cannot edit file in pack, suggest to extract it for editing.
				CString msg;
				msg.Format( _T("File %s is inside Pak file %s\r\nDo you want it to be extracted for Editing?"),(const char*)file,sPakName );
				if (AfxMessageBox( msg,MB_YESNO|MB_ICONEXCLAMATION) == IDNO)
				{
					return;
				}
				CFileUtil::CreateDirectory( Path::GetPath(file) );
				// Extract it from Pak file.
				CFile diskFile;
				if (diskFile.Open( file,CFile::modeCreate|CFile::modeWrite))
				{
					// Copy data from packed file to disk file.
					char *data = new char[cryfile.GetLength()];
					cryfile.Read( data,cryfile.GetLength() );
					diskFile.Write( data,cryfile.GetLength() );
					delete []data;
				}
				else
				{
					Warning( "Failed to create file %s on disk",(const char*)file );
				}
			}
		}
	}


	CString TextEditor = gSettings.textEditorForScript;
	if (fileType == FILE_TYPE_SHADER)
	{
		TextEditor = gSettings.textEditorForShaders;
	}

	HINSTANCE hInst = ShellExecute( NULL, "open", TextEditor, cmd,	NULL, SW_SHOWNORMAL );
	if ((DWORD_PTR)hInst < 32)
	{
		// Failed.
		file = file.SpanExcluding( "/" );
		// Try standart open.
		ShellExecute( NULL, "open", file, NULL, NULL, SW_SHOWNORMAL );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CFileUtil::SelectFile( const CString &fileSpec,const CString &searchFolder,CString &fullFileName )
{
	CString filter;
	filter.Format( "%s|%s||",(const char*)fileSpec,(const char*)fileSpec );

	CFileDialog dlg(TRUE, NULL,NULL, OFN_ENABLESIZING|OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR, filter );
	dlg.m_ofn.lpstrInitialDir = searchFolder;

	if (dlg.DoModal() == IDOK)
	{
		fullFileName = dlg.GetPathName();
		return true;
		/*
		if (!fileName.IsEmpty())
		{
			relativeFileName = GetIEditor()->GetRelativePath( fileName );
			if (!relativeFileName.IsEmpty())
			{
				return true;
			}
			else
			{
				Warning( "You must select files from %s folder",(const char*)GetIEditor()->GetMasterCDFolder(); );
			}
		}
		*/
	}

//	CSelectFileDlg cDialog;
//	bool res = cDialog.SelectFileName( &fileName,&relativeFileName,fileSpec,searchFolder );
	return false;
}

bool CFileUtil::SelectFiles( const CString &fileSpec,const CString &searchFolder,std::vector<CString> &files )
{
	CString filter;
	filter.Format( "%s|%s||",(const char*)fileSpec,(const char*)fileSpec );

	char filesStr[16768];
	memset( filesStr,0,sizeof(filesStr) );

	CFileDialog dlg(TRUE, NULL,NULL, OFN_ALLOWMULTISELECT|OFN_ENABLESIZING|OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR, filter );
	dlg.m_ofn.lpstrInitialDir = searchFolder;
	dlg.m_ofn.lpstrFile = filesStr;
	dlg.m_ofn.nMaxFile = sizeof(filesStr);

	files.clear();
	if (dlg.DoModal())
	{
		POSITION pos = dlg.GetStartPosition();
		while (pos != NULL)
		{
			CString fileName = dlg.GetNextPathName(pos);
			if (fileName.IsEmpty())
				continue;
			files.push_back( fileName );
		}
	}

	if (!files.empty())
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CFileUtil::SelectSaveFile( const CString &fileFilter,const CString &defaulExtension,const CString &startFolder,CString &fileName )
{
	CFileDialog dlg(FALSE, defaulExtension,NULL, OFN_ENABLESIZING|OFN_EXPLORER|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|OFN_OVERWRITEPROMPT, fileFilter );
	dlg.m_ofn.lpstrInitialDir = startFolder;

	if (dlg.DoModal() == IDOK)
	{
		fileName = dlg.GetPathName();
		if (!fileName.IsEmpty())
		{
			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CFileUtil::SelectSingleFile( ECustomFileType fileType,CString &outputFile,const CString &filter,const CString &initialDir )
{
	CCustomFileDialog::OpenParams op;
	op.bMultiSelection = false;
	op.filetype = fileType;
	op.filter = filter;
	op.initialDir = initialDir;
	op.initialFile = outputFile;
	CCustomFileDialog dlg( op );

	if (dlg.DoModal() == IDOK)
	{
		outputFile = dlg.GetFilePath();
		return true;
	}
	return false;
}

//! Display OpenFile dialog and allow to select multiple files.
bool CFileUtil::SelectMultipleFiles( ECustomFileType fileType,std::vector<CString> &files,const CString &filter,const CString &initialDir )
{
	CCustomFileDialog::OpenParams op;
	op.bMultiSelection = true;
	op.filetype = fileType;
	op.filter = filter;
	op.initialDir = initialDir;
	CCustomFileDialog dlg( op );

	files.clear();
	if (dlg.DoModal() == IDOK)
	{
		for (int i = 0; i < dlg.GetSelectedCount(); i++)
		{
			files.push_back( dlg.GetSelectedFile(i) );
		}
	}
	return !files.empty();
}

//////////////////////////////////////////////////////////////////////////
// Get directory contents.
//////////////////////////////////////////////////////////////////////////
inline bool ScanDirectoryFiles( const CString &root,const CString &path,const CString &fileSpec,std::vector<CFileUtil::FileDesc> &files )
{
	bool anyFound = false;
	CString dir = Path::AddBackslash(root + path);

	CString findFilter = Path::Make(dir,fileSpec);
	ICryPak *pIPak = GetIEditor()->GetSystem()->GetIPak();
	
	// Add all directories.
	_finddata_t fd;
	intptr_t fhandle;

	fhandle = pIPak->FindFirst( findFilter,&fd );
	if (fhandle != -1)
	{
		do {
			// Skip back folders.
			if (fd.name[0] == '.')
				continue;
			if (fd.attrib & _A_SUBDIR) // skip sub directories.
				continue;

			anyFound = true;

			CFileUtil::FileDesc file;
			file.filename = path + fd.name;
			file.attrib = fd.attrib;
			file.size = fd.size;
			file.time_access = fd.time_access;
			file.time_create = fd.time_create;
			file.time_write = fd.time_write;

			files.push_back( file );
		} while (pIPak->FindNext( fhandle,&fd ) == 0);
		pIPak->FindClose(fhandle);
	}

	/*
	CFileFind finder;
	BOOL bWorking = finder.FindFile( Path::Make(dir,fileSpec) );
	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		if (finder.IsDots())
			continue;

		if (!finder.IsDirectory())
		{
			anyFound = true;

			CFileUtil::FileDesc fd;
			fd.filename = dir + finder.GetFileName();
			fd.nFileSize = finder.GetLength();

			finder.GetCreationTime( &fd.ftCreationTime );
			finder.GetLastAccessTime( &fd.ftLastAccessTime );
			finder.GetLastWriteTime( &fd.ftLastWriteTime );

			fd.dwFileAttributes = 0;
			if (finder.IsArchived())
				fd.dwFileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
			if (finder.IsCompressed())
				fd.dwFileAttributes |= FILE_ATTRIBUTE_COMPRESSED;
			if (finder.IsNormal())
				fd.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
			if (finder.IsHidden())
				fd.dwFileAttributes = FILE_ATTRIBUTE_HIDDEN;
			if (finder.IsReadOnly())
				fd.dwFileAttributes = FILE_ATTRIBUTE_READONLY;
			if (finder.IsSystem())
				fd.dwFileAttributes = FILE_ATTRIBUTE_SYSTEM;
			if (finder.IsTemporary())
				fd.dwFileAttributes = FILE_ATTRIBUTE_TEMPORARY;

			files.push_back(fd);
		}
	}
	*/

	return anyFound;
}

//////////////////////////////////////////////////////////////////////////
// Get directory contents.
//////////////////////////////////////////////////////////////////////////
inline bool ScanDirectoryRecursive( const CString &root,const CString &path,const CString &fileSpec,std::vector<CFileUtil::FileDesc> &files, bool recursive )
{
	bool anyFound = false;
	CString dir = Path::AddBackslash(root + path);

	if (ScanDirectoryFiles( root,Path::AddBackslash(path),fileSpec,files ))
		anyFound = true;

	if (recursive)
	{
		/*
		CFileFind finder;
		BOOL bWorking = finder.FindFile( Path::Make(dir,"*.*") );
		while (bWorking)
		{
			bWorking = finder.FindNextFile();

			if (finder.IsDots())
				continue;

			if (finder.IsDirectory())
			{
				// Scan directory.
				if (ScanDirectoryRecursive( root,Path::AddBackslash(path+finder.GetFileName()),fileSpec,files,recursive ))
					anyFound = true;
			}
		}
		*/

		ICryPak *pIPak = GetIEditor()->GetSystem()->GetIPak();

		// Add all directories.
		_finddata_t fd;
		intptr_t fhandle;

		fhandle = pIPak->FindFirst( Path::Make(dir,"*.*"),&fd );
		if (fhandle != -1)
		{
			do {
				// Skip back folders.
				if (fd.name[0] == '.')
					continue;
				if (!(fd.attrib & _A_SUBDIR)) // skip not directories.
					continue;

				// Scan directory.
				if (ScanDirectoryRecursive( root,Path::AddBackslash(path + fd.name),fileSpec,files,recursive ))
					anyFound = true;

			} while (pIPak->FindNext( fhandle,&fd ) == 0);
			pIPak->FindClose(fhandle);
		}
	}

	return anyFound;
}

//////////////////////////////////////////////////////////////////////////
bool CFileUtil::ScanDirectory( const CString &path,const CString &file,std::vector<FileDesc> &files, bool recursive )
{
	return ScanDirectoryRecursive(Path::AddBackslash(path),"",file,files,recursive );
}

/*
bool CFileUtil::ScanDirectory( const CString &startDirectory,const CString &searchPath,const CString &fileSpec,FileArray &files, bool recursive=true )
{
	return ScanDirectoryRecursive(startDirectory,SearchPath,file,files,recursive );
}
*/

//////////////////////////////////////////////////////////////////////////
bool CFileUtil::OverwriteFile( const char *filename )
{
	// check if file exist.
	FILE *file = fopen( filename,"rb" );
	if (!file)
		return true;
	fclose(file);
	
	int res = (GetFileAttributes(filename)&FILE_ATTRIBUTE_READONLY);
	if (res == INVALID_FILE_ATTRIBUTES)
	{
		Warning( "Cannot Save File %s",filename );
		return false;
	}
	if (res != 0)
	{
		CCheckOutDialog dlg( filename,AfxGetMainWnd() );
		if (dlg.DoModal() == IDCANCEL)
		{
			return false;
		}
		if (dlg.GetResult() == CCheckOutDialog::CHECKOUT)
		{
			return CheckoutFile( filename );
		}
		SetFileAttributes( filename,FILE_ATTRIBUTE_NORMAL );
	}
	return true;
}

/*
static bool CheckOutFile( const char *filename )
{
	CString ssafeExe = "C:\\Program Files\\Microsoft Visual Studio\\VSS\\win32\\ss.exe";
	SetEnvironmentVariable( "ssuser","timur" );
	SetEnvironmentVariable( "ssdir","\\\\Server2\\XISLE\\ArtworkVss" );
	//CString SSafeArtwork = "\\\\Server2\\XISLE\\ArtworkVss\\win32\\ss.exe";
	//CString SSafeArtworkProject = "$/MASTERCD";

	CString cmd = ssafeExe + " " + " checkout cg.dll";

	char currDirectory[MAX_PATH];
	GetCurrentDirectory( sizeof(currDirectory),currDirectory  );
	char cmdLine[MAX_PATH];
	strcpy( cmdLine,cmd );

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	memset( &si,0,sizeof(si) );
	si.cb = sizeof(si);
	memset( &pi,0,sizeof(pi) );
	if (CreateProcess( NULL,cmdLine,NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,currDirectory,&si,&pi ))
	{
		// Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );

		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}
}
*/

//////////////////////////////////////////////////////////////////////////
bool CFileUtil::CheckoutFile( const char *filename )
{
	if (gSettings.ssafeParams.user.IsEmpty())
	{
		AfxMessageBox( _T("Source Safe login user name must be configured."),MB_OK|MB_ICONEXCLAMATION );

		// Source safe not configured.
		CSrcSafeSettingsDialog dlg;
		if (dlg.DoModal() != IDOK)
		{
			AfxMessageBox( _T("Checkout canceled"),MB_OK|MB_ICONEXCLAMATION );
			return false;
		}
	}
	SetEnvironmentVariable( "ssuser",gSettings.ssafeParams.user );
	SetEnvironmentVariable( "ssdir",gSettings.ssafeParams.databasePath );

	CString relFile = GetIEditor()->GetRelativePath(filename);
	if (relFile.IsEmpty())
		relFile = filename;

	CString cmd = gSettings.ssafeParams.exeFile + " checkout " + relFile;

	char currDirectory[MAX_PATH];
	GetCurrentDirectory( sizeof(currDirectory),currDirectory  );
	char cmdLine[MAX_PATH];
	strcpy( cmdLine,cmd );

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	memset( &si,0,sizeof(si) );
	si.cb = sizeof(si);
	memset( &pi,0,sizeof(pi) );
	if (CreateProcess( NULL,cmdLine,NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,currDirectory,&si,&pi ))
	{
		// Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );

		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CFileUtil::CheckinFile( const char *filename )
{
	SetEnvironmentVariable( "ssuser",gSettings.ssafeParams.user );
	SetEnvironmentVariable( "ssdir",gSettings.ssafeParams.databasePath );

	CString relFile = GetIEditor()->GetRelativePath(filename);
	if (relFile.IsEmpty())
		relFile = filename;

	CString cmd = gSettings.ssafeParams.exeFile + " checkout " + relFile;

	char currDirectory[MAX_PATH];
	GetCurrentDirectory( sizeof(currDirectory),currDirectory  );
	char cmdLine[MAX_PATH];
	strcpy( cmdLine,cmd );

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	memset( &si,0,sizeof(si) );
	si.cb = sizeof(si);
	memset( &pi,0,sizeof(pi) );
	if (CreateProcess( NULL,cmdLine,NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,currDirectory,&si,&pi ))
	{
		// Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );

		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		return true;
	}
	return false;
}

// Create new directory, check if directory already exist.
static bool CheckAndCreateDirectory( const char *path )
{
	WIN32_FIND_DATA FindFileData;

	HANDLE hFind = FindFirstFile( path,&FindFileData );
	if (hFind == INVALID_HANDLE_VALUE) {
		return ::CreateDirectory( path,NULL ) == TRUE;
	} else {
		DWORD attr = FindFileData.dwFileAttributes;
		FindClose(hFind);
		if (attr & FILE_ATTRIBUTE_DIRECTORY) {
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CFileUtil::CreateDirectory( const char *directory )
{
	CString path = directory;
	CString dir;
	bool res = CheckAndCreateDirectory( path );
	if (!res)
	{
		int iStart = 0;
		CString token = TokenizeString(path,"\\/",iStart );
		dir = token;
		while (token != "")
		{
			CheckAndCreateDirectory( dir );
			token = TokenizeString(path,"\\/",iStart );
			dir += CString("\\") + token;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CFileUtil::BackupFile( const char *filename )
{
	// Make a backup of previous file.
	bool makeBackup = true;

	CString bakFilename = Path::ReplaceExtension( filename,"bak" );

	{
		// Check if backup needed.
		CFile bak;
		if (bak.Open( filename,CFile::modeRead ))
		{
			if (bak.GetLength() <= 0)
				makeBackup = false;
		}
		else
			makeBackup = false;
	}
	if (makeBackup)
	{
		SetFileAttributes( filename,FILE_ATTRIBUTE_NORMAL );
		MoveFileEx( filename,bakFilename,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH );
	}
}