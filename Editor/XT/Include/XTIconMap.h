// XTIconMap.h: interface for the CXTIconMap class.
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

#if !defined(__XTICONMAP_H__)
#define __XTICONMAP_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: Strucure used by CXTIconMap to hold up to three
//          icons per command that represent normal, disabled
//          and active icon states.
struct XT_MAPENTRY
{
    HICON hIcon;			// Image icon.
    HICON hDisabledIcon;	// Disabled image icon.
    HICON hHotIcon;			// Hot image icon.
};

// Summary: CMap definition for mapping XT_MAPENTRY structures.
typedef	CMap<UINT, UINT, XT_MAPENTRY, XT_MAPENTRY&> CXTMapEntry;

//////////////////////////////////////////////////////////////////////
// Summary: CXTIconMap is a CObject derived class.  This class stores command to
//          icon associations. Icons are owned by this map and will be destroyed
//          once this object is destroyed.
class CXTIconMap : public CObject
{
	CXTMapEntry m_map; // A map of commands to their icons.

public:
	// Summary: Constructs a CXTIconMap object.
	CXTIconMap();

	// Summary: Destroys a CXTIconMap object, handles cleanup and de-allocation. It
	//			destroys the icons currently in the map.
	virtual ~CXTIconMap();
	
	// Summary: This member function removes and destroys all icons in the map.
	void RemoveAll();

	// Input:	nCmdID - A UINT value that represents the command ID associated with the icon.
	//			hIcon - Handle of the icon to associate with 'nCmdID'.
	//			hDisabledIcon - Handle of the disabled icon to associate with 'nCmdID'.
	//			hHotIcon - Handle of the hot icon to associate with 'nCmdID'.
	// Summary:	This member function sets a command to icon association. The existing
	//			icon, if any, will be destroyed.
	void SetAt(UINT nCmdID, HICON hIcon,HICON hDisabledIcon,HICON hHotIcon);

	// Input:	nCmdID - A UINT value that represents the command ID associated with the icon.
	//			hIcon - Reference to an HICON object to receive the handle of the icon associated
	//			with 'nCmdID'.
	//			hDisabledIcon - Reference to an HICON object to receive the handle of the disabled
	//			icon associated with 'nCmdID'.
	//			hHotIcon - Reference to an HICON object to receive the handle of the hot
	//			icon associated with 'nCmdID'.
	// Returns: TRUE if the icon exists, or FALSE if the icon does not exist.
	// Summary:	This member function looks up an icon by its command ID. 
	BOOL Lookup(UINT nCmdID, HICON& hIcon,HICON& hDisabledIcon,HICON& hHotIcon);

private:
	static void DestroyEntry(XT_MAPENTRY& entry);
};

#endif // !defined(__XTICONMAP_H__)
