////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushmtllib.h
//  Version:     v1.00
//  Created:     2/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __brushmtllib_h__
#define __brushmtllib_h__
#pragma once

/** Library of materials used for brush creation.
*/
class CBrushMtlLib
{
public:
private:
	// Array of all available materials.
	std::vector<CBrushMtlPtr> m_materials;
};

#endif // __brushmtllib_h__
