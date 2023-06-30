////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PreferencesStdPages.cpp
//  Version:     v1.00
//  Created:     29/10/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "PreferencesPropertyPage.h"
#include "PreferencesStdPages.h"
#include "DisplaySettings.h"
#include "Settings.h"

//////////////////////////////////////////////////////////////////////////
// Specific preferences pages.
//////////////////////////////////////////////////////////////////////////
class CPreferencesPage_General : public CPreferencesPropertyPage
{
public:
	CVariableArray tableGeneral;
	CVariableArray tableUndo;
	CVariable<int> undo_levels;

	CVariable<bool> previewPanel;
	CVariable<bool> treeBrowserPanel;
public:
	virtual const char* GetCategory() { return "General Settings"; }
	virtual const char* GetTitle()  { return "General"; }

	CPreferencesPage_General()
	{
		//////////////////////////////////////////////////////////////////////////
		// File table.
		AddVariable( tableGeneral,"General Settings" );
		AddVariable( tableGeneral,previewPanel,"Show Geometry Preview Panel" );
		AddVariable( tableGeneral,treeBrowserPanel,"Show Geometry Tree Browser Panel" );

		AddVariable( tableUndo,"Undo" );
		AddVariable( tableUndo,undo_levels,"Undo Levels" );
		undo_levels.SetLimits( 0,10000 );

		// Get settings.
		UpdateSettings( true );
	}
	virtual void OnApply() { UpdateSettings( false ); }
	//////////////////////////////////////////////////////////////////////////
	void UpdateSettings( bool bGet )
	{
		CopyVar( undo_levels,gSettings.undoLevels,bGet );
		CopyVar( previewPanel,gSettings.bPreviewGeometryWindow,bGet );
		CopyVar( treeBrowserPanel,gSettings.bGeometryBrowserPanel,bGet );
	}
	//////////////////////////////////////////////////////////////////////////
};

//////////////////////////////////////////////////////////////////////////
// Specific preferences pages.
//////////////////////////////////////////////////////////////////////////
class CPreferencesPage_Files : public CPreferencesPropertyPage
{
public:
	CVariableArray tableFiles;
	CVariable<bool> files_Backup;

	CVariableArray tableTextEditor;
	CVariable<CString> textEditor_Script;
	CVariable<CString> textEditor_Shaders;


	//////////////////////////////////////////////////////////////////////////
	// Autobackup table.
	CVariableArray tableAutoBackup;
	CVariable<int> autobak_Time;
	CVariable<int> autobak_RemindTime;
	CVariable<bool> autobak_Enabled;
	CVariable<CString> autobak_Filename;
	//////////////////////////////////////////////////////////////////////////

public:
	virtual const char* GetCategory() { return "General Settings"; }
	virtual const char* GetTitle()  { return "Files"; }

	CPreferencesPage_Files()
	{
		//////////////////////////////////////////////////////////////////////////
		// File table.
		AddVariable( tableFiles,"Files" );
		AddVariable( tableFiles,files_Backup,"Backup on Save" );

		AddVariable( tableTextEditor,"Text Editors" );
		AddVariable( tableFiles,textEditor_Script,"Scripts Text Editor" );
		AddVariable( tableFiles,textEditor_Shaders,"Shaders Text Editor" );

		//////////////////////////////////////////////////////////////////////////
		// Autobackup table.
		AddVariable( tableAutoBackup,"Auto Backup" );
		AddVariable( tableAutoBackup,autobak_Enabled,"Enable" );
		AddVariable( tableAutoBackup,autobak_Time,"Auto Backup Interval (Minutes)" );
		AddVariable( tableAutoBackup,autobak_Filename,"Auto Backup File Name" );
		AddVariable( tableAutoBackup,autobak_RemindTime,"Auto Remind Every (Minutes)" );
		//////////////////////////////////////////////////////////////////////////

		// Get settings.
		UpdateSettings( true );
	}
	virtual void OnApply() { UpdateSettings( false ); }
	//////////////////////////////////////////////////////////////////////////
	void UpdateSettings( bool bGet )
	{
		CopyVar( files_Backup,gSettings.bBackupOnSave,bGet );
		
		CopyVar( textEditor_Script,gSettings.textEditorForScript,bGet );
		CopyVar( textEditor_Shaders,gSettings.textEditorForShaders,bGet );

		CopyVar( autobak_Enabled,gSettings.autoBackupEnabled,bGet );
		CopyVar( autobak_Time,gSettings.autoBackupTime,bGet );
		CopyVar( autobak_Filename,gSettings.autoBackupFilename,bGet );
		CopyVar( autobak_RemindTime,gSettings.autoRemindTime,bGet );
	}
	//////////////////////////////////////////////////////////////////////////
};

//////////////////////////////////////////////////////////////////////////
// Specific preferences pages.
//////////////////////////////////////////////////////////////////////////
class CPreferencesPage_SSafe : public CPreferencesPropertyPage
{
public:
	CVariableArray tableSSafe;
	CVariable<CString> user;
	CVariable<CString> exeFile;
	CVariable<CString> databasePath;
	CVariable<CString> project;
public:
	virtual const char* GetCategory() { return "General Settings"; }
	virtual const char* GetTitle()  { return "Source Safe"; }

	CPreferencesPage_SSafe()
	{
		//////////////////////////////////////////////////////////////////////////
		// File table.
		AddVariable( tableSSafe,"Source Safe Settings" );
		AddVariable( tableSSafe,user,"User Name" );
		AddVariable( tableSSafe,exeFile,"Source Safe Executable",IVariable::DT_FILE );
		AddVariable( tableSSafe,databasePath,"Path to Source Safe Database" );
		AddVariable( tableSSafe,project,"Source Safe Project" );

		// Get settings.
		UpdateSettings( true );
	}
	virtual void OnApply() { UpdateSettings( false ); }
	//////////////////////////////////////////////////////////////////////////
	void UpdateSettings( bool bGet )
	{
		CopyVar( user,gSettings.ssafeParams.user,bGet );
		CopyVar( exeFile,gSettings.ssafeParams.exeFile,bGet );
		CopyVar( databasePath,gSettings.ssafeParams.databasePath,bGet );
		CopyVar( project,gSettings.ssafeParams.project,bGet );
	}
	//////////////////////////////////////////////////////////////////////////
};

//////////////////////////////////////////////////////////////////////////
// Specific preferences pages.
//////////////////////////////////////////////////////////////////////////
class CPreferencesPage_AVIRecording : public CPreferencesPropertyPage
{
public:
	CVariableArray table;
	CVariable<CString> codec;
	CVariable<int> frameRate;
public:
	virtual const char* GetCategory() { return "General Settings"; }
	virtual const char* GetTitle()  { return "AVI Encoding"; }

	CPreferencesPage_AVIRecording()
	{
		//////////////////////////////////////////////////////////////////////////
		// File table.
		AddVariable( table,"AVI Encoder Settings" );
		AddVariable( table,codec,"Codec (4 chars)" );
		AddVariable( table,frameRate,"Frame rate" );

		// Get settings.
		UpdateSettings( true );
	}
	virtual void OnApply() { UpdateSettings( false ); }
	//////////////////////////////////////////////////////////////////////////
	void UpdateSettings( bool bGet )
	{
		CopyVar( codec,gSettings.aviSettings.codec,bGet );
		CopyVar( frameRate,gSettings.aviSettings.nFrameRate,bGet );
	}
	//////////////////////////////////////////////////////////////////////////
};


//////////////////////////////////////////////////////////////////////////
// Specific preferences pages.
//////////////////////////////////////////////////////////////////////////
class CPreferencesPage_ViewportGeneral : public CPreferencesPropertyPage
{
public:
	CVariableArray tableGeneral;
	CVariableArray tableDisplay;
	CVariableArray tableLabels;

	CVariable<bool> sync2DViews;
	CVariable<float> defaultFOV;
	CVariable<float> defaultAspectRatio;
	CVariable<bool> showSafeFrame;

	CVariable<bool> labelsOn;
	CVariable<float> labelsDistance;
	CVariable<bool> displayTracks;
	CVariable<bool> displayLinks;
	CVariable<bool> alwaysShowRadiuses;
	CVariable<bool> alwaysShowPrefabBox;
	CVariable<bool> alwaysShowPrefabObjects;
	CVariable<bool> showBBoxes;
public:
	virtual const char* GetCategory() { return "Viewports"; }
	virtual const char* GetTitle()  { return "General"; }

	CPreferencesPage_ViewportGeneral()
	{
		//////////////////////////////////////////////////////////////////////////
		// File table.
		AddVariable( tableGeneral,"General Viewport Settings" );
		AddVariable( tableDisplay,"Viewport Displaying" );

		AddVariable( tableGeneral,sync2DViews,"Synchronize 2D Viewports" );
		AddVariable( tableGeneral,defaultFOV,"Perspective View FOV",IVariable::DT_ANGLE );
		AddVariable( tableGeneral,defaultAspectRatio,"Perspective View Aspect Ratio" );
		AddVariable( tableDisplay,showSafeFrame,"Show Safe Frame" );

		AddVariable( tableDisplay,displayLinks,"Display Object Links" );
		AddVariable( tableDisplay,displayTracks,"Display Animation Tracks" );
		AddVariable( tableDisplay,alwaysShowRadiuses,"Always Show Radiuses" );
		AddVariable( tableDisplay,alwaysShowPrefabBox,"Always Show Prefab Bounds" );
		AddVariable( tableDisplay,alwaysShowPrefabObjects,"Always Show Prefab Objects" );
		AddVariable( tableDisplay,showBBoxes,"Show Bounding Boxes" );
		
		//////////////////////////////////////////////////////////////////////////
		AddVariable( tableLabels,"Text Labels" );
		AddVariable( tableLabels,labelsOn,"Enabled" );
		AddVariable( tableLabels,labelsDistance,"Distance" );
		labelsDistance.SetLimits( 0,100000 );
		//////////////////////////////////////////////////////////////////////////

		// Get settings.
		UpdateSettings( true );
	}
	virtual void OnApply() { UpdateSettings( false ); }

	//////////////////////////////////////////////////////////////////////////
	void UpdateSettings( bool bGet )
	{
		CDisplaySettings *ds = GetIEditor()->GetDisplaySettings();
		if (bGet)
		{
			labelsOn = ds->IsDisplayLabels();
			labelsDistance = ds->GetLabelsDistance();
			displayTracks = ds->IsDisplayTracks();
			displayLinks = ds->IsDisplayLinks();
			showBBoxes = (ds->GetRenderFlags() & RENDER_FLAG_BBOX) == RENDER_FLAG_BBOX;
		}
		else
		{
			ds->DisplayLabels( labelsOn );
			ds->SetLabelsDistance( labelsDistance );
			ds->DisplayTracks( displayTracks );
			ds->DisplayLinks( displayLinks );
			if (showBBoxes)
				ds->SetRenderFlags( ds->GetRenderFlags() | RENDER_FLAG_BBOX );
			else
				ds->SetRenderFlags( ds->GetRenderFlags() & (~RENDER_FLAG_BBOX) );
		}
		CopyVar( alwaysShowRadiuses,gSettings.viewports.bAlwaysShowRadiuses,bGet );
		CopyVar( alwaysShowPrefabBox,gSettings.viewports.bAlwaysDrawPrefabBox,bGet );
		CopyVar( alwaysShowPrefabObjects,gSettings.viewports.bAlwaysDrawPrefabInternalObjects,bGet );
		CopyVar( sync2DViews,gSettings.viewports.bSync2DViews,bGet );
		CopyVar( defaultFOV,gSettings.viewports.fDefaultFov,bGet );
		CopyVar( defaultAspectRatio,gSettings.viewports.fDefaultAspectRatio,bGet );
		CopyVar( showSafeFrame,gSettings.viewports.bShowSafeFrame,bGet );
	}
	//////////////////////////////////////////////////////////////////////////
};

//////////////////////////////////////////////////////////////////////////
// Specific preferences pages.
//////////////////////////////////////////////////////////////////////////
class CPreferencesPage_ViewportGizmo : public CPreferencesPropertyPage
{
public:
	CVariableArray tableGizmo;
	CVariableArray tableHelpers;

	CVariable<float> axisGizmoSize;
	CVariable<bool> axisGizmoText;
	CVariable<int> axisGizmoMaxCount;
	CVariable<float> helpersGlobalScale;
public:
	virtual const char* GetCategory() { return "Viewports"; }
	virtual const char* GetTitle()  { return "Gizmos"; }

	CPreferencesPage_ViewportGizmo()
	{
		//////////////////////////////////////////////////////////////////////////
		// File table.
		AddVariable( tableGizmo,"Axis Gizmo" );
		AddVariable( tableGizmo,axisGizmoSize,"Axis Gizmo Size" );
		AddVariable( tableGizmo,axisGizmoText,"Text Labels on Axis Gizmo" );
		AddVariable( tableGizmo,axisGizmoMaxCount,"Max Count of Axis Gizmos" );

		//////////////////////////////////////////////////////////////////////////
		// Helpers table.
		AddVariable( tableHelpers,"Helpers" );
		AddVariable( tableHelpers,helpersGlobalScale,"Helpers Scale" );
		helpersGlobalScale.SetLimits( 0.01f,100 );
		//////////////////////////////////////////////////////////////////////////

		// Get settings.
		UpdateSettings( true );
	}
	virtual void OnApply() { UpdateSettings( false ); }

	//////////////////////////////////////////////////////////////////////////
	void UpdateSettings( bool bGet )
	{
		CopyVar( axisGizmoSize,gSettings.gizmo.axisGizmoSize,bGet );
		CopyVar( axisGizmoText,gSettings.gizmo.axisGizmoText,bGet );
		CopyVar( axisGizmoMaxCount,gSettings.gizmo.axisGizmoMaxCount,bGet );

		CopyVar( helpersGlobalScale,gSettings.gizmo.helpersScale,bGet );
	}
	//////////////////////////////////////////////////////////////////////////
};

//////////////////////////////////////////////////////////////////////////
// Specific preferences pages.
//////////////////////////////////////////////////////////////////////////
class CPreferencesPage_ViewportMovement : public CPreferencesPropertyPage
{
public:
	CVariableArray tableCamera;
	CVariable<float> camMoveSpeed;
	CVariable<float> camFastMoveSpeed;

public:
	virtual const char* GetCategory() { return "Viewports"; }
	virtual const char* GetTitle()  { return "Movement"; }

	CPreferencesPage_ViewportMovement()
	{
		//////////////////////////////////////////////////////////////////////////
		// File table.
		AddVariable( tableCamera,"Camera Movement Settings" );
		AddVariable( tableCamera,camMoveSpeed,"Camera Movement Speed" );
		AddVariable( tableCamera,camFastMoveSpeed,"Fast Movement Scale (holding Shift)" );

		// Get settings.
		UpdateSettings( true );
	}
	virtual void OnApply() { UpdateSettings( false ); }
	//////////////////////////////////////////////////////////////////////////
	void UpdateSettings( bool bGet )
	{
		CopyVar( camMoveSpeed,gSettings.cameraMoveSpeed,bGet );
		CopyVar( camFastMoveSpeed,gSettings.cameraFastMoveSpeed,bGet );
	}
	//////////////////////////////////////////////////////////////////////////
};

//////////////////////////////////////////////////////////////////////////
// Implementation of ClassDesc for standart Editor preferences.
//////////////////////////////////////////////////////////////////////////
HRESULT CStdPreferencesClassDesc::QueryInterface( const IID &riid, void **ppvObj )
{
	if (riid == __uuidof(IPreferencesPageCreator))
	{
		*ppvObj = (IPreferencesPageCreator*)this;
		return S_OK;
	}
	return E_NOINTERFACE;
}

//////////////////////////////////////////////////////////////////////////
ULONG CStdPreferencesClassDesc::AddRef()
{
	m_refCount++;
	return m_refCount;
};

//////////////////////////////////////////////////////////////////////////
ULONG CStdPreferencesClassDesc::Release()
{
	ULONG refs = --m_refCount;
	if (m_refCount <= 0)
		delete this;
	return refs;
}

//////////////////////////////////////////////////////////////////////////
REFGUID CStdPreferencesClassDesc::ClassID()
{
	// {95FE3251-796C-4e3b-82F0-AD35F7FFA267}
	static const GUID guid = { 0x95fe3251, 0x796c, 0x4e3b, { 0x82, 0xf0, 0xad, 0x35, 0xf7, 0xff, 0xa2, 0x67 } };
	return guid;
}

//////////////////////////////////////////////////////////////////////////
int CStdPreferencesClassDesc::GetPagesCount()
{
	return 6;
}

IPreferencesPage* CStdPreferencesClassDesc::CreatePage( int index,const CRect &rc,CWnd *pParentWnd )
{
	CPreferencesPropertyPage *pPage = 0;
	switch (index)
	{
	//////////////////////////////////////////////////////////////////////////
	case 0:	pPage = new CPreferencesPage_General();	break;
	//////////////////////////////////////////////////////////////////////////
	case 1:	pPage = new CPreferencesPage_Files();	break;
	//////////////////////////////////////////////////////////////////////////
	case 2:	pPage = new CPreferencesPage_SSafe();	break;
		//////////////////////////////////////////////////////////////////////////
	case 3:	pPage = new CPreferencesPage_AVIRecording();	break;
		//////////////////////////////////////////////////////////////////////////
	case 4:	pPage = new CPreferencesPage_ViewportGeneral();	break;
	//////////////////////////////////////////////////////////////////////////
	case 5: pPage = new CPreferencesPage_ViewportMovement(); break;
	//////////////////////////////////////////////////////////////////////////
	case 6:	pPage = new CPreferencesPage_ViewportGizmo();	break;
	}
	if (pPage)
	{
		if (pPage->Create( rc,pParentWnd ) != TRUE)
		{
			delete pPage;
			return 0;
		}
	}
	return pPage;
}
