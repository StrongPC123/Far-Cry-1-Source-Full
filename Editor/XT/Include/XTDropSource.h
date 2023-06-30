// XTDropSource.h : header file
//
// This file is a part of the Xtreme Toolkit for MFC.
// ©1998-2003 Codejock Software, All Rights Reserved.
//
// This source code can only be used under the terms and conditions 
// outlined in the accompanying license agreement.
//
// support@codejock.com
// http://www.codejock.com
//
//////////////////////////////////////////////////////////////////////

#ifndef __XTDROPSOURCE_H__
#define __XTDROPSOURCE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// NUMBERED LIST:

//////////////////////////////////////////////////////////////////////
// Summary: CXTDropSource class is derived from IDropSource.  It is used by the
//			shell list control to provide drag-and-drop operations in your application. 
//
//			It contains methods used in any application used as a data source in
//			a drag-and-drop operation. The data source application in a drag-and-drop
//			operation is responsible for: 
//			[ol]
//			[li]Determining the data being dragged based on the user's selection.[/li]
//			[li]Initiating the drag-and-drop operation based on the user's mouse actions. [/li]
//			[li]Generating some of the visual feedback during the drag-and-drop operation, such
//			as setting the cursor and highlighting the data selected for the
//			drag-and-drop operation.[/li]
//			[li]Canceling or completing the drag-and-drop operation based on the
//			user's mouse actions.[/li]
//			[li]Performing any action on the original data caused by the drop operation,
//			such as deleting the data on a drag move.[/li]
//			[/ol]
//			CXTDropSource contains the methods for generating visual feedback to
//			the end user and for canceling or completing the drag-and-drop operation.
//			You also need to call the DoDragDrop, RegisterDragDrop, and RevokeDragDrop
//			functions in drag-and-drop operations.
class _XT_EXT_CLASS CXTDropSource : public IDropSource
{
protected:
	
	UINT m_uiRefCount; // Indicates the current reference count.

public: 
	// Summary: Constructs a CXTDropSource object.
	CXTDropSource();
	
	// Summary: Destroys a CXTDropSource object, handles clean up and de-allocation.
	~CXTDropSource();

	//////////////////////////////////////////////////////////////////////
	// IUnknown Interface Members
	//////////////////////////////////////////////////////////////////////

	// Input:	riid - Identifier of the interface being requested. 
	//			ppReturn - Address of the pointer variable that receives the interface pointer
	//			requested in 'riid'. Upon successful return, 'ppReturn' contains
	//			the requested interface pointer to the object. If the object does
	//			not support the interface specified in 'riid', 'ppReturn' is set to NULL. 
	// Returns: A pointer to an interface if successful, otherwise returns NULL.
	// Summary:	The QueryInterface method gives a client access to other interfaces
	//			on an object. For any one object, a specific query for the IUnknown
	//			interface on any of the object's interfaces must always return the
	//			same pointer value. This allows a client to determine whether two pointers
	//			point to the same component by calling QueryInterface on both, and comparing
	//			the results. It is specifically not the case that queries for interfaces
	//			(even the same interface through the same pointer) must return the
	//			same pointer value.
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppReturn);

	// Returns: A ULONG value that represents the new reference count.  The return value can be an 
	//			integer from 1 to n, the value of the new reference count. This information
	//			is meant to be used for diagnostic and testing purposes only, because, in certain 
	//			situations, the value may be unstable.
	// Summary:	The AddRef method increments the reference count for an interface on
	//			an object.  It should be called for every new copy of a pointer to
	//			an interface on a given object.  
	STDMETHODIMP_(ULONG) AddRef(void);

	// Returns: A ULONG value that represents the resulting value of the reference count,
	//			which is used for diagnostic and testing purposes only. 
	// Summary:	This method decrements the reference count for the calling interface
	//			on a object. If the reference count on the object falls to 0, the object
	//			is freed from memory. 
	STDMETHODIMP_(ULONG) Release(void);

	//////////////////////////////////////////////////////////////////////
	// IDropSource Interface Members
	//////////////////////////////////////////////////////////////////////

	// BULLETED LIST:

	// Input:	bEsc - Specifies whether the ESC key has been pressed since the previous
	//			call to QueryContinueDrag, or to DoDragDrop if this is the first call
	//			to QueryContinueDrag.  A TRUE value indicates the end user has pressed
	//			the escape key.  A FALSE value indicates it has not been pressed. 
	//			dwKeyState - Current state of the keyboard modifier keys on the keyboard. Valid
	//			values can be a combination of any of the flags MK_CONTROL, MK_SHIFT,
	//			MK_ALT, MK_BUTTON, MK_LBUTTON, MK_MBUTTON, and MK_RBUTTON. 
	// Returns: An HRESULT value.
	// Summary:	This method determines whether a drag-and-drop operation should be
	//			continued, canceled, or completed.  You do not call this method directly.
	//			The OLE DoDragDrop function calls this method during a drag-and-drop
	//			operation.  This method supports the standard return values E_UNEXPECTED
	//			and E_OUTOFMEMORY, as well as the following:
	//			[ul]
	//			[li]<b>S_OK</b> Drag operation should continue. This result occurs if no
	//			errors are detected, the mouse button starting the drag-and-drop
	//			operation has not been released, and the ESC key has not been detected.[/li]
	//			[li]<b>DRAGDROP_S_DROP</b> Drop operation should occur completing the drag
	//			operation. This result occurs if DWORD indicates that the key that
	//			started the drag-and-drop operation has been released.[/li]
	//			[li]<b>DRAGDROP_S_CANCEL</b> Drag operation should be canceled with no drop
	//			operation occurring. This result occurs if BOOL is TRUE, indicating
	//			the ESC key has been pressed.[/li]
	//			[/ul]
	STDMETHODIMP QueryContinueDrag(BOOL bEsc,DWORD dwKeyState);
	
	// BULLETED LIST:

	// Input:	DWORD - Effect of a drop operation.
	// Returns: An HRESULT value.
	// Summary:	This method enables a source application to give visual feedback to
	//			the end user during a drag-and-drop operation by providing the DoDragDrop
	//			function with an enumeration value specifying the visual effect. This
	//			method supports the standard return values E_INVALIDARG, E_UNEXPECTED,
	//			and E_OUTOFMEMORY, as well as the following: 
	//			[ul]
	//			[li]<b>S_OK</b> Method completed its task successfully, using the cursor
	//			set by the source application.[/li]
	//			[li]<b>DRAGDROP_S_USEDEFAULTCURSORS</b> Successful completion of the method,
	//			and requests OLE to update the cursor using the OLE-provided default
	//			cursors.[/li]
	//			[/ul]
	STDMETHODIMP GiveFeedback(DWORD);
};

//////////////////////////////////////////////////////////////////////

#endif // __XTDROPSOURCE_H__