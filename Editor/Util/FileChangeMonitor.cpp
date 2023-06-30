////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   filechangemonitor.cpp
//  Version:     v1.00
//  Created:     15/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "FileChangeMonitor.h"

#include "Thread.h"
#include "Util\FileUtil.h"
#include <sys/stat.h>

//////////////////////////////////////////////////////////////////////////
// Directory monitoring thread.
//////////////////////////////////////////////////////////////////////////
struct CFileChangeMonitorThread : public CThread
{
public:
	std::vector<HANDLE> m_handles;
	std::vector<CString> m_dirs;
	MTQueue<CString> m_files;
	HANDLE m_killEvent;
	DWORD m_mainThreadId;


	CFileChangeMonitorThread()
	{
		m_mainThreadId = GetCurrentThreadId();
		m_killEvent = ::CreateEvent( NULL,TRUE,FALSE,NULL );
		// First event in list is KillEvent.
		m_handles.push_back(m_killEvent);
		m_dirs.push_back("");
	}

	~CFileChangeMonitorThread()
	{
		::CloseHandle( m_killEvent );
		m_killEvent = 0;
	}

protected:
	void FindChangedFiles( const CString &dir );

	void Run()
	{
		DWORD dwWaitStatus;

		int numHandles = m_handles.size();

		// If First handle triggers, its quit.
		// Waiting thread.
		while (TRUE) 
		{ 
			// Wait for notification.
			dwWaitStatus = WaitForMultipleObjects( numHandles,&m_handles[0],FALSE,INFINITE );

			if (dwWaitStatus >= WAIT_OBJECT_0 && dwWaitStatus < WAIT_OBJECT_0+numHandles)
			{
				if (dwWaitStatus == WAIT_OBJECT_0)
				{
					// This is Thread Kill event.
					break;
				}
				// One of objects got triggered, find out which.
				int id = dwWaitStatus - WAIT_OBJECT_0;
				CString dir = m_dirs[id];

				FindChangedFiles(dir);

				// Now the intesting part.. we need to find which file have been changed.
				// The fastest way, is scan directoryto take current time

				if (FindNextChangeNotification(m_handles[id]) == FALSE)
				{
					// Error!!!.
					//ExitProcess(GetLastError());
				}
				// Notify main thread that something have changed.
				PostThreadMessage( m_mainThreadId,WM_FILEMONITORCHANGE,0,0 );
			}
		}

		// Close all change notification handles.
		for (int i = 1; i < m_handles.size(); i++)
		{
			FindCloseChangeNotification(m_handles[i]);
		}
	}
};

//////////////////////////////////////////////////////////////////////////
inline void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
}

//////////////////////////////////////////////////////////////////////////
inline bool ScanDirectoryFiles( const CString &root,const CString &path,const CString &fileSpec,std::vector<CFileChangeMonitor::SFileEnumInfo> &files )
{
	bool anyFound = false;
	CString dir = Path::AddBackslash(root + path);

	CString findFilter = Path::Make(dir,fileSpec);
	ICryPak *pIPak = GetIEditor()->GetSystem()->GetIPak();

	// Add all directories.
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

			CFileChangeMonitor::SFileEnumInfo fd;
			fd.filename = dir + finder.GetFileName();
			//fd.nFileSize = finder.GetLength();

			//finder.GetCreationTime( &fd.ftCreationTime );
			//finder.GetLastAccessTime( &fd.ftLastAccessTime );
			finder.GetLastWriteTime( &fd.ftLastWriteTime );

			files.push_back(fd);
		}
	}

	return anyFound;
}

//////////////////////////////////////////////////////////////////////////
// Get directory contents.
//////////////////////////////////////////////////////////////////////////
inline bool ScanDirectoryRecursive( const CString &root,const CString &path,const CString &fileSpec,std::vector<CFileChangeMonitor::SFileEnumInfo> &files, bool recursive )
{
	bool anyFound = false;
	CString dir = Path::AddBackslash(root + path);

	if (ScanDirectoryFiles( root,path,fileSpec,files ))
		anyFound = true;

	if (recursive)
	{
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
	}
	
	return anyFound;
}

//////////////////////////////////////////////////////////////////////////
void CFileChangeMonitorThread::FindChangedFiles( const CString &dir )
{
	//CLogFile::WriteLine( "** Searching file" );
	// Get current file time.
	SYSTEMTIME curSysTime;
	FILETIME curFileTime;
	GetSystemTime( &curSysTime );
	SystemTimeToFileTime( &curSysTime,&curFileTime );

	std::vector<CFileChangeMonitor::SFileEnumInfo> files;
	files.reserve( 1000 );
	ScanDirectoryRecursive( dir,"","*.*",files,true );

	INT64 curftime = *(INT64*)&curFileTime;

	SYSTEMTIME s1,s2;
	FILETIME ft1,ft2;
	ZeroStruct(ft1);
	FileTimeToSystemTime(&ft1,&s1);
	FileTimeToSystemTime(&ft1,&s2);
	
	s2.wSecond = 5; // 5 second.

	SystemTimeToFileTime(&s1,&ft1);
 	SystemTimeToFileTime(&s2,&ft2);
	int deltaT = (*(INT64*)&ft2) - (*(INT64*)&ft1) ; // 1 second.

	for (int i = 0; i < files.size(); i++)
	{
		// now compare time stamps.
		INT64 ftime = *(INT64*)&(files[i].ftLastWriteTime);
		
		INT64 dt = curftime-ftime;
		if (dt < 0)
		{
			dt = ftime - curftime;
		}
		
		if (dt < deltaT)
		{
			// This file was written within deltaT time from now, consider it as modified.
			CString filename = files[i].filename;
			m_files.push( filename );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
CFileChangeMonitor::CFileChangeMonitor()
{
	m_thread = new CFileChangeMonitorThread;
}

//////////////////////////////////////////////////////////////////////////
CFileChangeMonitor::~CFileChangeMonitor()
{
	// Send to thread a kill event.
	StopMonitor();
}

//////////////////////////////////////////////////////////////////////////
bool CFileChangeMonitor::IsDirectory(const char* sFileName)
{
	struct __stat64 my_stat;
	if (_stat64(sFileName, &my_stat) != 0) return false;
	return ((my_stat.st_mode & S_IFDIR) != 0);
}

//////////////////////////////////////////////////////////////////////////
void CFileChangeMonitor::MonitorDirectories( const std::vector<CString> &dirs )
{
	// Watch the C:\WINDOWS directory for file creation and 
	// deletion.
	for (int i = 0; i < dirs.size(); i++)
	{
		if (!IsDirectory( Path::RemoveBackslash(dirs[i]) ))
			continue;
		
		m_thread->m_dirs.push_back(dirs[i]);

		// Create Notification Event.
		HANDLE dwHandle = FindFirstChangeNotification( dirs[i],TRUE,FILE_NOTIFY_CHANGE_LAST_WRITE); // watch file name changes 
		m_thread->m_handles.push_back(dwHandle);
	}

	// Start monitoring thread.
	m_thread->Start();
}

//////////////////////////////////////////////////////////////////////////
void CFileChangeMonitor::StopMonitor()
{
	if (m_thread)
	{
		::SetEvent(m_thread->m_killEvent);
	}
}
	
//////////////////////////////////////////////////////////////////////////
bool CFileChangeMonitor::HaveModifiedFiles() const
{
	return !m_thread->m_files.empty();
}

//////////////////////////////////////////////////////////////////////////
CString CFileChangeMonitor::GetModifiedFile()
{
	CString file;
	if (!m_thread->m_files.empty())
	{
		file = m_thread->m_files.top();
		m_thread->m_files.pop();
	}
	return file;
}