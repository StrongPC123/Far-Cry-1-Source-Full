////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   resourcecompiler.h
//  Version:     v1.00
//  Created:     4/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __resourcecompiler_h__
#define __resourcecompiler_h__
#pragma once

#include "IRCLog.h"
#include "IResCompiler.h"
#include "Config.h"
#include "ExtensionManager.h"

#include <map>										// stl multimap<>
#include <string>									// stl string




/** Implementation of IResCompiler interface.
*/
class ResourceCompiler :
	public IResourceCompiler,
	public IRCLog
{
public:
	ResourceCompiler();
	~ResourceCompiler();

	//! e.g. print dependencies
	void PostBuild();

	// interface IResourceCompiler ---------------------------------------------

	virtual void RegisterConvertor( IConvertor *conv );
	virtual FILE*	OpenFile( const char *filename,const char *mode );
	bool GetFileTime( const char *filename,FILETIME *ftimeModify, FILETIME*ftimeCreate =NULL );

	// returns the file unix time - the latest of modification and creation times
	DWORD GetFileUnixTimeMax (const char* filename);

	// returns the file unix time - the earliest of modification and creation times
	DWORD GetFileUnixTimeMin (const char* filename);

	virtual bool CompileFile( const char *filename, const char *outroot, const char *outpath );

	//! Load and parse the Crytek Chunked File into the universal (very big) structure
	//! The caller should then call Release on the structure to free the mem
	//! @param filename Full filename including path to the file
	virtual CryChunkedFile* LoadCryChunkedFile (const char* szFileName);
	
	virtual IRCLog *GetIRCLog();

	virtual void AddDependencyMaterial( const char *inszSrcFilename, const char *inszMatName, const char *inszScriptName );

	virtual void AddDependencyFile( const char *inszSrcFilename, const char *inszPathFileName );


	// interface IRCLog ---------------------------------------------

	void LogLine( const ELogType ineType, const char* szText );

	//////////////////////////////////////////////////////////////////////////
	// Resource compiler implementation.
	//////////////////////////////////////////////////////////////////////////
	bool Compile( Platform platform,IConfig* config,const char *filespec );
	const char* GetSectionName( Platform platform ) const;

	//! call this if user asks for help
	void show_help();

	void EnsureDirectoriesPresent(const char *path);

	//! Returns the main application window
	HWND GetHWnd();
	HWND GetEmptyWindow();

	IPhysicalWorld* GetPhysicalWorld();

	Config 												m_MainConfig;					//!<

private:

	// private structures

	typedef std::multimap<string,string>		CFileDepMap;
	typedef std::pair<string,string>				CFileDepPair;

	class CMatDep
	{
		public:
			string				m_sMatName;				//!<
			string				m_sScriptName;		//!<

			bool operator==( const CMatDep &inRhS ) const
			{
				return(inRhS.m_sMatName==m_sMatName && inRhS.m_sScriptName==m_sScriptName);
			}
	};
		
	// helper to get order for CMatDep
	struct CMatDepOrder: public std::binary_function< CMatDep, CMatDep, bool>
	{
		bool operator() ( const CMatDep &a, const CMatDep &b ) const
		{
			// first sort by m_sScriptName (neccessary for printout)
			if(a.m_sScriptName<b.m_sScriptName)return(true);
			if(a.m_sScriptName>b.m_sScriptName)return(false);

			// then by m_sMatName
			if(a.m_sMatName<b.m_sMatName)return(true);
			if(a.m_sMatName>b.m_sMatName)return(false);

			return(false);
		}
	};

	typedef std::multimap<CMatDep,string,CMatDepOrder>		CMatDepMap;
	typedef std::pair<CMatDep,string>										CMatDepPair;

	// ------------------------------------------------



	IConfig *								m_config;								//!< Current global configuration settings.
	ICfgFile *							m_presets;
	Platform								m_platform;							//!< Current compilation platform.

	ExtensionManager				m_extensionManager;			//!<

	IPhysicalWorld*					m_pIPhysicalWorld;			//!<


	CFileDepMap							m_FileDependencies;			//!< key=dependency e.g. nm.dds   value=file that is using it e.g. ball.cgf
	CMatDepMap							m_MaterialDependencies;	//!< key=materialname+scriptmaterialname  value=file that is using it

	// log files
	FILE *									m_hLogFile;							//!< for all messages, might be 0 (no file logging)
	FILE *									m_hWarningLogFile;			//!< for all warnigns only, might be 0 (no file logging)
	FILE *									m_hErrorLogFile;				//!< for all errors only, might be 0 (no file logging)

	//
	bool										m_bWarningHeaderLine;		//!< true= header was already printed, false= header needs to be printed
	bool										m_bErrorHeaderLine;			//!< true= header was already printed, false= header needs to be printed
	bool										m_bStatistics;					//!< true= show statistics.
	bool										m_bQuiet;								//!< true= don't log anything to console, otherwise false
	string									m_sHeaderLine;					//!<

	HWND m_hEmptyWindow;
	//!
	void InitPhysics();

	//!
	void ShowFileDependencies();

	//!
	void ShowMaterialDependencies();

	//! to remove old files for less confusion
	void RemoveOutputFiles();

	void SetHeaderLine( const char *inszLine );
};

#endif // __resourcecompiler_h__
