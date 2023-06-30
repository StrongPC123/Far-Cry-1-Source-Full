////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   GameResourcesExporter.cpp
//  Version:     v1.00
//  Created:     31/10/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GameResourcesExporter.h"
#include "GameEngine.h"

#include "Objects\ObjectManager.h"
#include "Objects\Entity.h"
#include "Material\MaterialManager.h"
#include "Particles\ParticleManager.h"
#include "Music\MusicManager.h"

#include <XTToolkit.h>

//////////////////////////////////////////////////////////////////////////
// Static data.
//////////////////////////////////////////////////////////////////////////
CGameResourcesExporter::Files CGameResourcesExporter::m_files;

//////////////////////////////////////////////////////////////////////////
void CGameResourcesExporter::ChooseDirectoryAndSave()
{
	CString path;
	{
		CXTBrowseDialog dlg;
		dlg.SetTitle( "Choose Target Folder" );
		dlg.SetOptions( BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN  );
		if (dlg.DoModal() == IDOK)
		{
			path = dlg.GetSelPath();
		}
	}
	if (!path.IsEmpty())
		Save( path );
}

//////////////////////////////////////////////////////////////////////////
void CGameResourcesExporter::EnumRecordedFiles( const char *filename )
{
	m_files.push_back( filename );
}

//////////////////////////////////////////////////////////////////////////
void CGameResourcesExporter::Save( const CString &outputDirectory )
{
	m_files.clear();
	m_files.reserve( 100000 );
	
	GetISystem()->GetIPak()->EnumerateRecordedFiles( &EnumRecordedFiles );

	GetFilesFromObjects();
	GetFilesFromMaterials();
	GetFilesFromParticles();
	GetFilesFromMusic();

	CMemoryBlock data;

	int numFiles = m_files.size();

	CLogFile::WriteLine( "===========================================================================" );
	CLogFile::FormatLine( "Exporting Level %s resources, %d files",(const char*)GetIEditor()->GetGameEngine()->GetLevelName(),numFiles );
	CLogFile::WriteLine( "===========================================================================" );

	// Needed files.
	CWaitProgress wait( "Exporting Resources" );
	for (int i = 0; i < numFiles; i++)
	{
		CString srcFilename = m_files[i];
		if (!wait.Step( (i*100)/numFiles ))
			break;
		wait.SetText( srcFilename );

		CLogFile::WriteLine( srcFilename );

		CCryFile file;
		if (file.Open( srcFilename,"rb" ))
		{
			// Save this file in target folder.
			CString trgFilename = Path::Make( outputDirectory,srcFilename );
			int fsize = file.GetLength();
			if (fsize > data.GetSize())
			{
				data.Allocate( fsize + 16 );
			}
			// Read data.
			file.Read( data.GetBuffer(),fsize );
			
			// Save this data to target file.
			CString trgFileDir = Path::GetPath(trgFilename);
			CFileUtil::CreateDirectory( trgFileDir );
			// Create a file.
			FILE *trgFile = fopen( trgFilename,"wb" );
			if (trgFile)
			{
				// Save data to new file.
				fwrite(data.GetBuffer(),fsize,1,trgFile);
				fclose(trgFile);
			}
		}
	}
	CLogFile::WriteLine( "===========================================================================" );
	m_files.clear();
}

#ifdef WIN64
template <class Container1, class Container2>
void Append(Container1& a, const Container2& b)
{
	a.reserve (a.size() + b.size());
	for (Container2::const_iterator it = b.begin(); it != b.end(); ++it)
		a.insert(a.end(),*it);
}
#else
template <class Container1, class Container2>
void Append(Container1& a, const Container2& b)
{
	a.insert (a.end(), b.begin(), b.end());
}
#endif
//////////////////////////////////////////////////////////////////////////
//
// Go through all editor objects and gathers files from thier properties.
//
//////////////////////////////////////////////////////////////////////////
void CGameResourcesExporter::GetFilesFromObjects()
{
	CUsedResources rs;
	GetIEditor()->GetObjectManager()->GatherUsedResources( rs );

	Append(m_files,rs.files);
}

//////////////////////////////////////////////////////////////////////////
void CGameResourcesExporter::GetFilesFromMaterials()
{
	CUsedResources rs;
	GetIEditor()->GetMaterialManager()->GatherUsedResources( rs );
	Append(m_files,rs.files);
}

//////////////////////////////////////////////////////////////////////////
void CGameResourcesExporter::GetFilesFromParticles()
{
	CUsedResources rs;
	GetIEditor()->GetParticleManager()->GatherUsedResources( rs );
	Append(m_files, rs.files);
}

//////////////////////////////////////////////////////////////////////////
void CGameResourcesExporter::GetFilesFromMusic()
{
	CUsedResources rs;
	GetIEditor()->GetMusicManager()->GatherUsedResources( rs );
	Append(m_files,rs.files);
}
