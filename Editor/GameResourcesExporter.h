////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   GameResourcesExporter.h
//  Version:     v1.00
//  Created:     31/10/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __GameResourcesExporter_h__
#define __GameResourcesExporter_h__
#pragma once

/*! Implements exporting of al loaded resources to specified directory.
 *	
 */
class CGameResourcesExporter
{
public:
	void ChooseDirectoryAndSave();
	void Save( const CString &outputDirectory );

private:
	typedef std::vector<CString> Files;
	static Files m_files;

	//////////////////////////////////////////////////////////////////////////
	// Fucntions that gather files from editor subsystems.
	//////////////////////////////////////////////////////////////////////////
	void GetFilesFromObjects();
	void GetFilesFromVarBlock( CVarBlock *pVB );
	void GetFilesFromVariable( IVariable *pVar );
	void GetFilesFromMaterials();
	void GetFilesFromParticles();
	void GetFilesFromMusic();

	static void EnumRecordedFiles( const char *filename );
};

#endif // __GameResourcesExporter_h__

