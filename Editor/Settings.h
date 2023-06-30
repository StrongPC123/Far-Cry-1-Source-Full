////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   settings.h
//  Version:     v1.00
//  Created:     14/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: General editor settings.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __settings_h__
#define __settings_h__
#pragma once

class CGrid;

enum EEditorPathName
{
	EDITOR_PATH_OBJECTS = 0,
	EDITOR_PATH_TEXTURES = 1,
	EDITOR_PATH_SOUNDS = 2,
	EDITOR_PATH_LAST = 3,
};

//////////////////////////////////////////////////////////////////////////
//! Parameters needed to use Microsoft source safe.
//////////////////////////////////////////////////////////////////////////
struct SSourceSafeInitParams
{
	CString user;
	CString exeFile;
	CString databasePath;
	CString project;
};

struct SGizmoSettings
{
	float axisGizmoSize;
	bool axisGizmoText;
	int axisGizmoMaxCount;

	float helpersScale;

	SGizmoSettings();
};

//////////////////////////////////////////////////////////////////////////
// Settings for AVI encoding.
//////////////////////////////////////////////////////////////////////////
struct SAVIEncodingSettings
{
	SAVIEncodingSettings() : nFrameRate(25),codec("DIVX") {};
	
	int nFrameRate;
	CString codec;
};

//////////////////////////////////////////////////////////////////////////
struct SViewportsSettings
{
	//! If enabled always show entity radiuse.
	bool bAlwaysShowRadiuses;
	//! If enabled always display boxes for prefab entity.
	bool bAlwaysDrawPrefabBox;
	//! If enabled always display objects inside prefab.
	bool bAlwaysDrawPrefabInternalObjects;
	//! True if 2D viewports will be synchronized with same view and origin.
	bool bSync2DViews;
	//! Camera FOV for perspective View.
	float fDefaultFov;
	//! Camera Aspect Ratio for perspective View.
	float fDefaultAspectRatio;
	//! Show safe frame.
	bool bShowSafeFrame;
};

/** Various editor settings.
*/
struct SEditorSettings
{
	SEditorSettings();
	void Save();
	void Load();

	void PostInitApply();

	//////////////////////////////////////////////////////////////////////////
	// Variables.
	//////////////////////////////////////////////////////////////////////////
	int undoLevels;

	//! Speed of camera movement.
	float cameraMoveSpeed;
	float cameraFastMoveSpeed;

	//! Hide mask for objects.
  int objectHideMask;

	//! Selection mask for objects.
	int objectSelectMask;

	//////////////////////////////////////////////////////////////////////////
	// Viewport settings.
	//////////////////////////////////////////////////////////////////////////
	SViewportsSettings viewports;

	//////////////////////////////////////////////////////////////////////////
	// Files.
	//////////////////////////////////////////////////////////////////////////
	bool bBackupOnSave;

	//////////////////////////////////////////////////////////////////////////
	// Autobackup.
	//////////////////////////////////////////////////////////////////////////
	//! Save auto backup file every autoSaveTime minutes.
	int autoBackupTime;
	//! When this variable set to true automatic file backup is enabled.
	bool autoBackupEnabled;
	//! After this amount of minutes message box with reminder to save will pop on.
	int autoRemindTime;
	//! Autobackup filename.
	CString autoBackupFilename;
	//////////////////////////////////////////////////////////////////////////


	//! If true preview windows is displayed when browsing geometries.
	bool bPreviewGeometryWindow;
	//! If true display geometry browser window for brushes and simple entities.
	bool bGeometryBrowserPanel;

	//! Pointer to currently used grid.
	CGrid *pGrid;

	SGizmoSettings gizmo;

	//! Source safe parameters.
	SSourceSafeInitParams ssafeParams;

	//! Text editor.
	CString textEditorForScript;
	CString textEditorForShaders;

	//////////////////////////////////////////////////////////////////////////
	//! Editor data search paths.
	std::vector<CString> searchPaths[EDITOR_PATH_LAST];

	SAVIEncodingSettings aviSettings;

private:
	void SaveValue( const char *sSection,const char *sKey,int value );
	void SaveValue( const char *sSection,const char *sKey,float value );
	void SaveValue( const char *sSection,const char *sKey,const CString &value );

	void LoadValue( const char *sSection,const char *sKey,int &value );
	void LoadValue( const char *sSection,const char *sKey,float &value );
	void LoadValue( const char *sSection,const char *sKey,bool &value );
	void LoadValue( const char *sSection,const char *sKey,CString &value );
};

//! Single instance of editor settings for fast access.
extern SEditorSettings gSettings;


#endif // __settings_h__
