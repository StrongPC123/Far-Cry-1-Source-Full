////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   IUndoObject.h
//  Version:     v1.00
//  Created:     5/2/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Interface for implementation of IUndo objects.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __IUndoObject_h__
#define __IUndoObject_h__

#if _MSC_VER > 1000
#pragma once
#endif

//! IUndoObject is a interface of general Undo object.
struct IUndoObject
{
	// Virtual destructor.
	virtual ~IUndoObject() {};

	//! Called to delete undo object.
	virtual void Release() { delete this; };
	//! Return size of this Undo object.
	virtual int GetSize() = 0;
	//! Return description of this Undo object.
	virtual const char* GetDescription() = 0;

	//! Undo this object.
	//! @param bUndo If true this operation called in response to Undo operation.
	virtual void Undo( bool bUndo=true ) = 0;

	//! Redo undone changes on object.
	virtual void Redo() = 0;
};

#endif // __IUndoObject_h__
