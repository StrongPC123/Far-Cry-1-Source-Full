////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   IPreferencesPage.h
//  Version:     v1.00
//  Created:     28/10/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __IPreferencesPage_h__
#define __IPreferencesPage_h__
#pragma once

#include "Plugin.h"
/*
 *	IPreferencePage is the interface class for preferences pages.
 */
struct IPreferencesPage
{
	virtual void Release() = 0;

	//! Return category where this preferences page belongs.
	virtual const char* GetCategory() = 0;

	//! Title of this preferences page.
	virtual const char* GetTitle() = 0;

	//! Returns Page window.
	virtual CWnd* GetWindow() = 0;

	//! Called by the editor when the Apply Now button is clicked. 
	virtual void OnApply() = 0;
	
	//! Called by the editor when the Cancel button is clicked. 
	virtual void OnCancel() = 0;

	//! Called by the editor when the Cancel button is clicked, and before the cancel has taken place.
	//! @return true to perform Cancel operation, false to abort Cancel.
	virtual bool OnQueryCancel() = 0;

	//! Called by the editor when the preferences page is made the active page or is not longer the active page. 
	//! @param bActive true when page become active, false when page deactivated.
	virtual void OnSetActive( bool bActive ) = 0;

	//! Called by the editor when the OK, Apply Now, or Close button is clicked. 
	virtual void OnOK() = 0;

	//! Access to variable block for this preferences page.
	virtual CVarBlock* GetVars() = 0;
};

/*!	Interface used to create new preferences pages.
	You can query this interface from any IClassDesc interface with ESYSTEM_CLASS_PREFERENCE_PAGE system class Id.
*/
struct __declspec( uuid("{D494113C-BF13-4171-9171-0333DF10EAFC}") ) IPreferencesPageCreator
{
	//! Get number of preferences page hosted by this class.
	virtual int GetPagesCount() = 0;
	//! Creates a new preferences page by page index.
	//! @param index must be within 0 <= index < GetPagesCount().
	virtual IPreferencesPage* CreatePage( int index,const CRect &rc,CWnd *pParentWnd ) = 0;
};


/*
 *	IPreferencesPageClassDesc is a plugin class description for all IPreferencesPage derived classes.
 */
struct IPreferencesPageClassDesc :  public IClassDesc
{
	//////////////////////////////////////////////////////////////////////////
	// IClassDesc implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual ESystemClassID SystemClassID() { return ESYSTEM_CLASS_PREFERENCE_PAGE; };

	//! This method returns the human readable name of the class.
	virtual const char* ClassName() { return "Preferences Page"; };

	//! This method returns Category of this class, Category is specifing where this plugin class fits best in
	//! create panel.
	virtual const char* Category() { return "Preferences"; };
	//////////////////////////////////////////////////////////////////////////

	//! Show a modal about dialog / message box for the plugin.
	virtual void ShowAbout() {};

	virtual bool CanExitNow() { return true; };

	//! The plugin should write / read its data to the passed stream. The data is saved to or loaded
	//! from the editor project file. This function is called during the usual save / load process of
	//! the editor's project file
	virtual void Serialize( CXmlArchive &ar ) {};

	//! Editor can send to class various events.
	virtual void Event( EClassEvent event ) {};

};

#endif // __IPreferencesPage_h__

