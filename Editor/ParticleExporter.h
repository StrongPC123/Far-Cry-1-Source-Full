////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   particleexporter.h
//  Version:     v1.00
//  Created:     12/9/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __particleexporter_h__
#define __particleexporter_h__
#pragma once

// forward declarations.
class CPakFile;

/*! Class responsible for exporting particles to game format.
*/
/** Export brushes from specified Indoor to .bld file.
*/
class CParticlesExporter
{
public:
	void ExportParticles( const CString &path,const CString &levelName,CPakFile &pakFile );

	void AddParticleExportItem( std::vector<struct SExportParticleEffect> &effects,class CParticleItem *pItem,int parent );
	void ExportParticleLib( class CParticleLibrary *pLib,CFile &file );
private:
};

#endif // __particleexporter_h__
