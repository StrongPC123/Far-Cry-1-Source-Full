// XTDockState.h : header file
//
// This file is a part of the Xtreme Toolkit for MFC.
// ©1998-2003 Codejock Software, All Rights Reserved.
//
// This source code can only be used under the terms and conditions 
// outlined in the accompanying license agreement.
//
// support@codejock.com
// http://www.codejock.com
//
//////////////////////////////////////////////////////////////////////

#ifndef __XTDOCKSTATE_H__
#define __XTDOCKSTATE_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTControlBarInfo is a CControlBarInfo derived class.  It is used to
//			load and save the information about control bars.
class CXTControlBarInfo : public CControlBarInfo
{
public:

	// Input:	lpszProfileName - Points to a null-terminated string that specifies the name of a section
	//			in the initialization file or a key in the Windows registry where
	//			state information is stored.
	//			nIndex - Index of the control bar in the array.
	//			pDockState - A pointer to a valid CDockState object.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this function to retrieve state information from the registry
	//			or .INI file. The profile name is the section of the application's
	//			.INI file or the registry that contains the bars' state information.
	//			You can save control bar state information to the registry or .INI
	//			file with SaveState. 
	BOOL LoadState(LPCTSTR lpszProfileName,int nIndex,CDockState* pDockState);
	
	// Input:	lpszProfileName - Points to a null-terminated string that specifies the name of a section
	//			in the initialization file or a key in the Windows registry where
	//			state information is stored.
	//			nIndex - Index of the control bar in the array.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	Call this function to save the state information to the registry or
	//			.INI file. The profile name is the section of the application's .INI
	//			file or the registry that contains the control bar's state information.
	//			SaveState also saves the current screen size. You can retrieve control
	//			bar information from the registry or .INI file with LoadState. 
	BOOL SaveState(LPCTSTR lpszProfileName,int nIndex);
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTDockState is a serialized CObject derived class.  It loads, unloads,
//			or clears the state of one or more docking control bars in persistent
//			memory (a file).  The dock state includes the size and position of the
//			bar and whether or not it is docked.  When retrieving the stored dock
//			state, CXTDockState checks the bar's position and, if the bar is not
//			visible with the current screen settings, CXTDockState scales the bar's
//			position so that it is visible.  The main purpose of CXTDockState is
//			to hold the entire state of a number of control bars and to allow that
//			state to be saved and loaded either to the registry, the application's
//			.INI file, or in binary form as part of a CArchive object's contents. 
//
//			The bar can be any dockable control bar, including a toolbar, status
//			bar, or dialog bar.  CXTDockState objects are written and read to, or
//			from, a file via a CArchive object. 
//
//			CXTFrameWnd::GetDockState retrieves the state information of all the
//			frame window's CXTControlBar objects and puts it into the CXTDockState
//			object.  You can then write the contents of the CXTDockState object
//			to storage with Serialize or CXTDockState::SaveState. If you later want
//			to restore the state of the control bars in the frame window, you can
//			load the state with Serialize or CXTDockState::LoadState, then use 
//			CXTFrameWnd::SetDockState to apply the saved state to the frame window's
//			control bars.
class CXTDockState : public CDockState
{
	DECLARE_SERIAL(CXTDockState)
public:

	// Input:	lpszProfileName - Points to a null-terminated string that specifies the name of a section
	//			in the initialization file or a key in the Windows registry where
	//			state information is stored.
	// Summary:	Call this function to retrieve state information from the registry
	//			or .INI file. The profile name is the section of the application's
	//			.INI file or the registry that contains the bar's state information.
	//			You can save control bar state information to the registry or .INI
	//			file with SaveState.
	void LoadState(LPCTSTR lpszProfileName);
	
	// Input:	lpszProfileName - Points to a null-terminated string that specifies the name of a section
	//			in the initialization file or a key in the Windows registry where
	//			state information is stored.
	// Summary:	Call this function to save the state information to the registry or
	//			.INI file. The profile name is the section of the application's .INI
	//			file or the registry that contains the control bar's state information.
	//			SaveState also saves the current screen size. You can retrieve control
	//			bar information from the registry or .INI file with LoadState.
	void SaveState(LPCTSTR lpszProfileName);
};

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // __XTDOCKSTATE_H__
