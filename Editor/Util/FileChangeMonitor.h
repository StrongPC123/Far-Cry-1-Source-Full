////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   filechangemonitor.h
//  Version:     v1.00
//  Created:     15/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __filechangemonitor_h__
#define __filechangemonitor_h__
#pragma once

//! This message sent to main application window when file change detected.
#define WM_FILEMONITORCHANGE WM_APP + 10

//////////////////////////////////////////////////////////////////////////
// Monitors directory for any changed files.
//////////////////////////////////////////////////////////////////////////
class CFileChangeMonitor
{
public:
	CFileChangeMonitor();
	~CFileChangeMonitor();

	void MonitorDirectories( const std::vector<CString> &dirs );
	void StopMonitor();
	
	//! Check if any files where modified.
	//! This is a polling function, call it every frame or so.
	bool HaveModifiedFiles() const;
	//! Get next modified file, this file will be delete from list after calling this function.
	//! Call it until HaveModifiedFiles return true.
	CString GetModifiedFile();

	struct SFileEnumInfo
	{
		CString filename;
		FILETIME ftLastWriteTime;
	};
private:
	bool IsDirectory(const char* sFileName);

	//! Pointer to implementation class.
	struct CFileChangeMonitorThread *m_thread;
};

#endif // __filechangemonitor_h__
