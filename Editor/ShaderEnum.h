////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ShaderEnum.h
//  Version:     v1.00
//  Created:     7/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Enumerate Installed Shaders.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ShaderEnum_h__
#define __ShaderEnum_h__

#if _MSC_VER > 1000
#pragma once
#endif

/*!
 *	CShaderEnum class enumerates shaders installed on system.
 *	It scans all effector files, and gather from them all defined effectors.
 */
class CShaderEnum
{
public:
	struct ShaderDesc {
		CString name;
		CString file;
	};

	CShaderEnum();
	virtual ~CShaderEnum();

	//! Enumerate shaders installed on system.
	//! @return Number of enumerated shaders.
	int EnumShaders();

	//! Get number of shaders in system.
	//! @return Number of installed shaders.
	int	GetShaderCount() const { return m_shaders.size(); }
	
	//! Get name of shader by index.
	//! index must be between 0 and number returned by EnumShaders.
	//! @return Name of shader.
	const CString& GetShader( int i ) { return m_shaders[i].name; }
	const CString& GetShaderFile( int i ) { return m_shaders[i].file; }

private:
	bool m_bEnumerated;

	//! Array of shader names.
	std::vector<ShaderDesc> m_shaders;
};


#endif // __ShaderEnum_h__
