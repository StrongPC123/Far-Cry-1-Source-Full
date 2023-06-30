////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   levelinfo.h
//  Version:     v1.00
//  Created:     4/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __levelinfo_h__
#define __levelinfo_h__
#pragma once

/*!	CLevelInfo provides methods for getting information about current level.
 */
class CLevelInfo
{
public:
	CLevelInfo();
	void Validate();

	void SaveLevelResources( const CString &toPath );

private:
	void ValidateObjects();
	void ValidateMaterials();

	CErrorReport *m_pReport;
};

#endif // __levelinfo_h__
