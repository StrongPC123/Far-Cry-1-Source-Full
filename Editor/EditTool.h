////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   EditTool.h
//  Version:     v1.00
//  Created:     18/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __EditTool_h__
#define __EditTool_h__

#if _MSC_VER > 1000
#pragma once
#endif

class CViewport;
struct IClassDesc;

/*!
 *	CEditTool is an abstract base class for All Editing	Tools supported by Editor.
 *	Edit tools handle specific editing modes in viewports.
 */
class CEditTool : public CObject
{
public:
	DECLARE_DYNAMIC(CEditTool);

	CEditTool() { m_pClassDesc = 0; };
	virtual ~CEditTool() {};

	//! Returns class description for this tool.
	IClassDesc* GetClassDesc() const { return m_pClassDesc; }

	//! Release this tool.
	virtual void Release() = 0;


	//! Status text displayed when this tool is active.
	void SetStatusText( const CString &text ) { m_statusText = text; };
	const char*	GetStatusText() { return m_statusText; };
	
	//! Used to pass user defined data to edit tool from ToolButton.
	virtual void SetUserData( void *userData ) {};

	//! Called when user starts using this tool.
	//! Flags is comnination of ObjectEditFlags flags.
	virtual void BeginEditParams( IEditor *ie,int flags ) = 0;
	//! Called when user ends using this tool.
	virtual void EndEditParams() = 0;

	// Called each frame to display tool for givven viewport.
	virtual void Display( struct DisplayContext &dc ) = 0;

	//! Mouse callback sent from viewport.
	//! Returns true if event processed by callback, and all other processing for this event should abort.
	//! Return false if event was not processed by callback, and other processing for this event should occur.
	//! @param view Viewport that sent this callback.
	//! @param event Indicate what kind of event occured in viewport.
	//! @param point 2D coordinate in viewport where event occured.
	//! @param flags Additional flags (MK_LBUTTON,etc..) or from (MouseEventFlags) specified by viewport when calling callback.
	virtual bool MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags ) = 0;

	//! Called when key in viewport is pressed while using this tool.
	//! Returns true if event processed by callback, and all other processing for this event should abort.
	//! Returns false if event was not processed by callback, and other processing for this event should occur.
	//! @param view Viewport where key was pressed.
	//! @param nChar Specifies the virtual key code of the given key. For a list of standard virtual key codes, see Winuser.h 
	//! @param nRepCnt Specifies the repeat count, that is, the number of times the keystroke is repeated as a result of the user holding down the key.
	//! @param nFlags	Specifies the scan code, key-transition code, previous key state, and context code, (see WM_KEYDOWN)
	virtual bool OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags ) = 0;

	//! Called when key in viewport is released while using this tool.
	//! Returns true if event processed by callback, and all other processing for this event should abort.
	//! Returns false if event was not processed by callback, and other processing for this event should occur.
	//! @param view Viewport where key was pressed.
	//! @param nChar Specifies the virtual key code of the given key. For a list of standard virtual key codes, see Winuser.h 
	//! @param nRepCnt Specifies the repeat count, that is, the number of times the keystroke is repeated as a result of the user holding down the key.
	//! @param nFlags	Specifies the scan code, key-transition code, previous key state, and context code, (see WM_KEYDOWN)
	virtual bool OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags ) = 0;

	//! Called when mouse is moved and give oportunity to tool to set it own cursor.
	//! @return true if cursor changed. or false otherwise.
	virtual bool OnSetCursor( CViewport *vp ) { return false; };

private:
	CString m_statusText;
	IClassDesc* m_pClassDesc;
};

#endif // __EditTool_h__
