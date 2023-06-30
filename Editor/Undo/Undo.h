////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   Undo.h
//  Version:     v1.00
//  Created:     9/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __Undo_h__
#define __Undo_h__

#if _MSC_VER > 1000
#pragma once
#endif

struct IUndoObject;
class CUndoStep;
class CSuperUndoStep;

/*!
 *	CUndoManager is keeping and operating on CUndo class instances.
 */
class CUndoManager
{
public:
	CUndoManager();
	~CUndoManager();

	//! Begin opretaion requiering Undo.
	//! Undo manager enters holding state.
	void Begin();
	//! Restore all undo objects registered since last Begin call.
	//! @param bUndo if true all Undo object registered up to this point will be undone.
	void Restore( bool bUndo );
	//! Accept changes and registers an undo object with the undo manager.
	//! This will allow the user to undo the operation.
	void Accept( const CString &name );
	//! Cancel changes and restore undo objects.
	void Cancel();

	//! Normally this is NOT needed but in special cases this can be useful.
	//! This allows to group a set of Begin()/Accept() sequences to be undone in one operation.
	void SuperBegin();
	//! When a SuperBegin() used, this method is used to Accept.
	//! This leaves the undo database in its modified state and registers the IUndoObjects with the undo system. 
	//! This will allow the user to undo the operation.
	void SuperAccept( const CString &name );
	//! Cancel changes and restore undo objects.
	void SuperCancel();

	//! Temporarily suspends recording of undo.
	void Suspend();
	//! Resume recording if was suspended.
	void Resume();

	// Undo last operation.
	void Undo( int numSteps=1 );

	//! Redo last undo.
	void Redo( int numSteps=1 );

	//! Check if undo information is recording now.
	bool IsUndoRecording() const { return (m_bRecording || m_bSuperRecording) && m_suspendCount == 0; };

	//! Put new undo object, must be called between Begin and Accept/Cancel methods.
	void RecordUndo( IUndoObject *obj );

	bool IsHaveUndo() const;
	bool IsHaveRedo() const;

	void SetMaxUndoStep( int steps );
	int GetMaxUndoStep() const;

	//! Returns length of undo stack.
	int	GetUndoStackLen() const;
	//! Returns length of redo stack.
	int	GetRedoStackLen() const;

	//! Retreves array of undo objects names.
	void GetUndoStackNames( std::vector<CString> &undos );
	//! Retreves array of redo objects names.
	void GetRedoStackNames( std::vector<CString> &redos );

	//! Get size of current Undo and redo database size.
	int	GetDatabaseSize();

	//! Completly flush all Undo and redo buffers.
	//! Must be done on level reloads or global Fetch operation.
	void Flush();

	void ClearRedoStack();
	void ClearUndoStack();

	void ClearUndoStack( int num );
	void ClearRedoStack( int num );

private:
	bool m_bRecording;
	bool m_bSuperRecording;
	int m_suspendCount;

	bool m_bUndoing;
	bool m_bRedoing;
	
	CUndoStep*	m_currentUndo;
	//! Undo step object created by SuperBegin.
	CSuperUndoStep*	m_superUndo;
	std::list<CUndoStep*> m_undoStack;
	std::list<CUndoStep*> m_redoStack;
};


#endif // __Undo_h__
