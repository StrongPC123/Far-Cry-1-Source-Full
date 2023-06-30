////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   MaterialEnum.h
//  Version:     v1.00
//  Created:     7/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Enumerate Installed Materials.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __MaterialEnum_h__
#define __MaterialEnum_h__

#if _MSC_VER > 1000
#pragma once
#endif

/*!
 *	CMaterialEnum class enumerates Materials installed on system.
 *	It scans all effector files, and gather from them all defined effectors.
 */
class CMaterialEnum
{
public:
	CMaterialEnum();
	virtual ~CMaterialEnum();

	//! Enumerate Materials installed on system.
	//! @return Number of enumerated Materials.
	int EnumMaterials();

	//! Get number of Materials in system.
	//! @return Number of installed Materials.
	int	GetCount() const { return m_materials.size(); }
	
	//! Get name of Material by index.
	//! index must be between 0 and number returned by EnumMaterials.
	//! @return Name of Material.
	const CString& GetMaterial( int i ) { return m_materials[i]; }

private:
	bool m_bEnumerated;
	//! Array of Material names.
	std::vector<CString> m_materials;
};


#endif // __MaterialEnum_h__
