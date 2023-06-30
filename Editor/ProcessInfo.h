////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   processinfo.h
//  Version:     v1.00
//  Created:     13/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __processinfo_h__
#define __processinfo_h__

#if _MSC_VER > 1000
#pragma once
#endif

/** Stores information about memory usage of process, retrieved from CProcessInfo class.
		All size values are in bytes.
*/
struct ProcessMemInfo
{
	int WorkingSet;
	int PeakWorkingSet;
	int PagefileUsage;
	int PeakPagefileUsage;
	int PageFaultCount;
};

/** Use this class to query information about current process.
		Like memory usage, pagefile usage etc..
*/
class CProcessInfo
{
public:
	CProcessInfo(void);
	~CProcessInfo(void);

	//! Loads PSAPI.DLL into the editor memoryspace
	static void LoadPSApi();

	//! Unloads PSAPI.DLL from the editor memoryspace
	static void UnloadPSApi();

	/** Retrieve information about memory usage of current process.
			@param meminfo Output parameter where information is saved.
	*/
	static void QueryMemInfo( ProcessMemInfo &meminfo );
};

#endif // __processinfo_h__