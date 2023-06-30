////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   settings.cpp
//  Version:     v1.00
//  Created:     14/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Settings.h"

//////////////////////////////////////////////////////////////////////////
// Global Instance of Editor settings.
//////////////////////////////////////////////////////////////////////////
SEditorSettings gSettings;

//////////////////////////////////////////////////////////////////////////
SGizmoSettings::SGizmoSettings()
{
	axisGizmoSize = 0.2f;
	axisGizmoText = true;
	axisGizmoMaxCount = 50;
	helpersScale = 1;
}

//////////////////////////////////////////////////////////////////////////
SEditorSettings::SEditorSettings()
{
	undoLevels = 50;

	objectHideMask = 0;
	objectSelectMask = 0xFFFFFFFF; // Initially all selectable.

	autoBackupFilename = "Autobackup";
	autoBackupEnabled = false;
	autoBackupTime = 10;
	autoRemindTime = 0;
	
	viewports.bAlwaysShowRadiuses = false;
	viewports.bAlwaysDrawPrefabBox = true;
	viewports.bAlwaysDrawPrefabInternalObjects = false;
	viewports.bSync2DViews = true;
	viewports.fDefaultAspectRatio = 0.75; // 600/800
	viewports.fDefaultFov = DEG2RAD(90); // 90 degrees.
	viewports.bShowSafeFrame = false;

	cameraMoveSpeed = 1;
	cameraFastMoveSpeed = 2;
	bPreviewGeometryWindow = true;
	bGeometryBrowserPanel = true;
	bBackupOnSave = true;

	// Init source safe params.
	ssafeParams.databasePath = "\\\\Server2\\XISLE\\ArtworkVss";
	ssafeParams.exeFile = "C:\\Program Files\\Microsoft Visual Studio\\VSS\\win32\\ss.exe";
	ssafeParams.project = "$/MasterCD";
	ssafeParams.user = "";

	textEditorForScript = "uedit32.exe";
	textEditorForShaders = "uedit32.exe";
}

//////////////////////////////////////////////////////////////////////////
void SEditorSettings::SaveValue( const char *sSection,const char *sKey,int value )
{
	AfxGetApp()->WriteProfileInt( sSection,sKey,value );
}

//////////////////////////////////////////////////////////////////////////
void SEditorSettings::SaveValue( const char *sSection,const char *sKey,float value )
{
	CString str;
	str.Format( "%g",value );
	AfxGetApp()->WriteProfileString( sSection,sKey,str );
}

//////////////////////////////////////////////////////////////////////////
void SEditorSettings::SaveValue( const char *sSection,const char *sKey,const CString &value )
{
	AfxGetApp()->WriteProfileString( sSection,sKey,value );
}

//////////////////////////////////////////////////////////////////////////
void SEditorSettings::LoadValue( const char *sSection,const char *sKey,int &value )
{
	value = AfxGetApp()->GetProfileInt(sSection,sKey,value );
}

//////////////////////////////////////////////////////////////////////////
void SEditorSettings::LoadValue( const char *sSection,const char *sKey,float &value )
{
	CString defaultVal;
	defaultVal.Format( "%g",value );
	defaultVal = AfxGetApp()->GetProfileString( sSection,sKey,defaultVal );
	value = atof(defaultVal);
}

//////////////////////////////////////////////////////////////////////////
void SEditorSettings::LoadValue( const char *sSection,const char *sKey,bool &value )
{
	value = AfxGetApp()->GetProfileInt(sSection,sKey,value );
}

//////////////////////////////////////////////////////////////////////////
void SEditorSettings::LoadValue( const char *sSection,const char *sKey,CString &value )
{
	value = AfxGetApp()->GetProfileString( sSection,sKey,value );
}

//////////////////////////////////////////////////////////////////////////
void SEditorSettings::Save()
{
	// Save settings to registry.
	SaveValue( "Settings","UndoLevels",undoLevels );
	SaveValue("Settings","AutoBackup",autoBackupEnabled );
	SaveValue("Settings","AutoBackupTime",autoBackupTime );
	SaveValue("Settings","AutoRemindTime",autoRemindTime );
	SaveValue("Settings","CameraMoveSpeed",cameraMoveSpeed );
	SaveValue("Settings","CameraFastMoveSpeed",cameraFastMoveSpeed );
	SaveValue("Settings","PreviewGeometryWindow",bPreviewGeometryWindow );
	SaveValue("Settings","GeometryBrowserPanel",bGeometryBrowserPanel );
	SaveValue("Settings","AutobackupFile",autoBackupFilename );

	SaveValue("Settings","BackupOnSave",bBackupOnSave );

	//////////////////////////////////////////////////////////////////////////
	// Viewports.
	//////////////////////////////////////////////////////////////////////////
	SaveValue("Settings","AlwaysShowRadiuses",viewports.bAlwaysShowRadiuses );
	SaveValue("Settings","AlwaysShowPrefabBox",viewports.bAlwaysDrawPrefabBox );
	SaveValue("Settings","AlwaysShowPrefabObjects",viewports.bAlwaysDrawPrefabInternalObjects );
	SaveValue("Settings","Sync2DViews",viewports.bSync2DViews );
	SaveValue("Settings","PerspectiveFov",viewports.fDefaultFov );
	SaveValue("Settings","PerspectiveAspectRatio",viewports.fDefaultAspectRatio );
	SaveValue("Settings","ShowSafeFrame",viewports.bShowSafeFrame );

	//////////////////////////////////////////////////////////////////////////
	// Gizmos.
	//////////////////////////////////////////////////////////////////////////
	SaveValue( "Settings","AxisGizmoSize",gizmo.axisGizmoSize );
	SaveValue( "Settings","AxisGizmoText",gizmo.axisGizmoText );
	SaveValue( "Settings","AxisGizmoMaxCount",gizmo.axisGizmoMaxCount );
	SaveValue( "Settings","HelpersScale",gizmo.helpersScale );
	//////////////////////////////////////////////////////////////////////////

	SaveValue( "Settings","TextEditorScript",textEditorForScript );
	SaveValue( "Settings","TextEditorShaders",textEditorForShaders );

	// Save source safe settings.
	SaveValue( "Settings","SSafeUser",ssafeParams.user );
	SaveValue( "Settings","SSafeDatabase",ssafeParams.databasePath );
	SaveValue( "Settings","SSafeExe",ssafeParams.exeFile );
	SaveValue( "Settings","SSafeProject",ssafeParams.project );

	SaveValue( "Settings","AVI_FrameRate",aviSettings.nFrameRate );
	SaveValue( "Settings","AVI_Codec",aviSettings.codec );

	/*
	//////////////////////////////////////////////////////////////////////////
	// Save paths.
	//////////////////////////////////////////////////////////////////////////
	for (int id = 0; id < EDITOR_PATH_LAST; id++)
	{
		for (int i = 0; i < searchPaths[id].size(); i++)
		{
			CString path = searchPaths[id][i];
			CString key;
			key.Format( "Paths","Path_%.2d_%.2d",id,i );
			SaveValue( "Paths",key,path );
		}
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void SEditorSettings::Load()
{
	// Load settings from registry.
	SaveValue( "Settings","UndoLevels",undoLevels );
	LoadValue( "Settings","AutoBackup",autoBackupEnabled );
	LoadValue("Settings","AutoBackupTime",autoBackupTime );
	LoadValue("Settings","AutoRemindTime",autoRemindTime );
	LoadValue("Settings","CameraMoveSpeed",cameraMoveSpeed );
	LoadValue("Settings","CameraFastMoveSpeed",cameraFastMoveSpeed );
	LoadValue("Settings","PreviewGeometryWindow",bPreviewGeometryWindow );
	LoadValue("Settings","GeometryBrowserPanel",bGeometryBrowserPanel );
	LoadValue("Settings","AutobackupFile",autoBackupFilename );

	LoadValue("Settings","BackupOnSave",bBackupOnSave );

	//////////////////////////////////////////////////////////////////////////
	// Viewports.
	//////////////////////////////////////////////////////////////////////////
	LoadValue("Settings","AlwaysShowRadiuses",viewports.bAlwaysShowRadiuses );
	LoadValue("Settings","AlwaysShowPrefabBox",viewports.bAlwaysDrawPrefabBox );
	LoadValue("Settings","AlwaysShowPrefabObjects",viewports.bAlwaysDrawPrefabInternalObjects );
	LoadValue("Settings","Sync2DViews",viewports.bSync2DViews );
	LoadValue("Settings","PerspectiveFov",viewports.fDefaultFov );
	LoadValue("Settings","PerspectiveAspectRatio",viewports.fDefaultAspectRatio );
	LoadValue("Settings","ShowSafeFrame",viewports.bShowSafeFrame );

	//////////////////////////////////////////////////////////////////////////
	// Gizmos.
	//////////////////////////////////////////////////////////////////////////
	LoadValue( "Settings","AxisGizmoSize",gizmo.axisGizmoSize );
	LoadValue( "Settings","AxisGizmoText",gizmo.axisGizmoText );
	LoadValue( "Settings","AxisGizmoMaxCount",gizmo.axisGizmoMaxCount );
	LoadValue( "Settings","HelpersScale",gizmo.helpersScale );
	//////////////////////////////////////////////////////////////////////////

	LoadValue( "Settings","TextEditorScript",textEditorForScript );
	LoadValue( "Settings","TextEditorShaders",textEditorForShaders );

	// Load source safe settings.
	LoadValue( "Settings","SSafeUser",ssafeParams.user );
	LoadValue( "Settings","SSafeDatabase",ssafeParams.databasePath );
	LoadValue( "Settings","SSafeExe",ssafeParams.exeFile );
	LoadValue( "Settings","SSafeProject",ssafeParams.project );

	LoadValue( "Settings","AVI_FrameRate",aviSettings.nFrameRate );
	LoadValue( "Settings","AVI_Codec",aviSettings.codec );

	//////////////////////////////////////////////////////////////////////////
	// Load paths.
	//////////////////////////////////////////////////////////////////////////
	for (int id = 0; id < EDITOR_PATH_LAST; id++)
	{
		int i = 0;
		searchPaths[id].clear();
		while (true)
		{
			CString key;
			key.Format( "Path_%.2d_%.2d",id,i );
			CString path;
			LoadValue( "Paths",key,path );
			if (path.IsEmpty())
				break;
			searchPaths[id].push_back( path );
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// Default paths.
	//////////////////////////////////////////////////////////////////////////
	if (searchPaths[EDITOR_PATH_OBJECTS].empty())
		searchPaths[EDITOR_PATH_OBJECTS].push_back( "Objects" );
	if (searchPaths[EDITOR_PATH_TEXTURES].empty())
		searchPaths[EDITOR_PATH_TEXTURES].push_back( "Textures" );
	if (searchPaths[EDITOR_PATH_SOUNDS].empty())
		searchPaths[EDITOR_PATH_SOUNDS].push_back( "Sounds" );
}

//////////////////////////////////////////////////////////////////////////
void SEditorSettings::PostInitApply()
{
}