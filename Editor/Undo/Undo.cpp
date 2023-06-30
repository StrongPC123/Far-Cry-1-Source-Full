////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   Undo.cpp
//  Version:     v1.00
//  Created:     9/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Undo.h"
#include "IUndoObject.h"
#include "Objects\ObjectManager.h"
#include "Settings.h"

//! CUndo is a collection of IUndoObjects instances that forms single undo step.
class CUndoStep
{
public:
	CUndoStep() {};
	virtual ~CUndoStep() { ClearObjects(); }

	//! Set undo object name.
	void SetName( const CString &name ) { m_name = name; };
	//! Get undo object name.
	const CString& GetName() { return m_name; };

	//! Add new undo object to undo step.
	void AddUndoObject( IUndoObject *o )
	{
		m_undoObjects.push_back(o);
	}
	void ClearObjects()
	{
		int i;
		// Release all undo objects.
		for (i = 0; i < m_undoObjects.size(); i++)
		{
			m_undoObjects[i]->Release();
		}
		m_undoObjects.clear();
	};
	virtual int GetSize() const
	{
		int size = 0;
		for (int i = 0; i < m_undoObjects.size(); i++)
		{
			size += m_undoObjects[i]->GetSize();
		}
		return size;
	}
	virtual bool IsEmpty() const { return m_undoObjects.empty(); };
	virtual void Undo( bool bUndo )
	{
		for (int i = m_undoObjects.size()-1; i >= 0; i--)
		{
			m_undoObjects[i]->Undo( bUndo );
		}
	}
	virtual void Redo()
	{
		for (int i = 0; i < m_undoObjects.size(); i++)
		{
			m_undoObjects[i]->Redo();
		}
	}
	
private:
	friend class CUndoManager;
	CString m_name;
	// Undo objects registered for this step.
	std::vector<IUndoObject*> m_undoObjects;
};

//! CSuperUndo objects groups together a block of UndoStepto to allow them to be Undo by single operation.
class CSuperUndoStep : public CUndoStep
{
public:
	//! Add new undo object to undo step.
	void AddUndoStep( CUndoStep *step )
	{
		m_undoSteps.push_back(step);
	}
	virtual int GetSize() const
	{
		int size = 0;
		for (int i = 0; i < m_undoSteps.size(); i++)
		{
			size += m_undoSteps[i]->GetSize();
		}
		return size;
	}
	virtual bool IsEmpty() { return m_undoSteps.empty(); };
	virtual void Undo( bool bUndo )
	{
		for (int i = m_undoSteps.size()-1; i >= 0; i--)
		{
			m_undoSteps[i]->Undo( bUndo );
		}
	}
	virtual void Redo()
	{
		for (int i = 0; i < m_undoSteps.size(); i++)
		{
			m_undoSteps[i]->Redo();
		}
	}
	
private:
	//! Undo steps included in this step.
	std::vector<CUndoStep*> m_undoSteps;
};

//////////////////////////////////////////////////////////////////////////
CUndoManager::CUndoManager()
{
	m_bRecording = false;
	m_bSuperRecording = false;
	
	m_currentUndo = 0;
	m_superUndo = 0;

	m_suspendCount = 0;

	m_bUndoing = false;
	m_bRedoing = false;
}

//////////////////////////////////////////////////////////////////////////
CUndoManager::~CUndoManager()
{
	m_bRecording = false;
	ClearRedoStack();
	ClearUndoStack();

	delete m_superUndo;
	delete m_currentUndo;
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::Begin()
{
	//CLogFile::FormatLine( "<Undo> Begin SuspendCount=%d",m_suspendCount );
	//if (m_bSuperRecording)
		//CLogFile::FormatLine( "<Undo> Begin (Inside SuperSuper)" );
	if (m_bUndoing || m_bRedoing) // If Undoing or redoing now, ignore this calls.
		return;

	assert( m_bRecording == false );

	if (m_bRecording)
	{
		//CLogFile::WriteLine( "<Undo> Begin (already recording)" );
		// Not cancel, just combine.
		return;
	}

	// Begin Creates a new undo object.
	m_currentUndo = new CUndoStep;

	m_bRecording = true;
	//CLogFile::WriteLine( "<Undo> Begin OK" );
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::Restore( bool bUndo )
{
	if (m_bUndoing || m_bRedoing) // If Undoing or redoing now, ignore this calls.
		return;

	if (m_currentUndo)
	{
		Suspend();
		if (bUndo)
			m_currentUndo->Undo(false); // Undo not by Undo command (no need to store Redo)
		Resume();
		m_currentUndo->ClearObjects();
	}
	//CLogFile::FormatLine( "Reset Undo" );
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::Accept( const CString &name )
{
	//CLogFile::FormatLine( "<Undo> Accept, Suspend Count=%d",m_suspendCount );
	if (m_bUndoing || m_bRedoing) // If Undoing or redoing now, ignore this calls.
		return;

	if (!m_bRecording)
	{
		//CLogFile::WriteLine( "<Undo> Accept (Not recording)" );
		return;
	}

	if (!m_currentUndo->IsEmpty())
	{
		GetIEditor()->SetModifiedFlag();
		// If accepting new undo object, must clear all redo stack.
		ClearRedoStack();

		m_currentUndo->SetName(name);
		if (m_bSuperRecording)
		{
			m_superUndo->AddUndoStep( m_currentUndo );
		}
		else
		{
			// Normal recording.
			// Keep max undo steps.
			if (m_undoStack.size() > gSettings.undoLevels)
			{
				delete m_undoStack.front();
				m_undoStack.pop_front();
			}
			m_undoStack.push_back( m_currentUndo );
		}
		CLogFile::FormatLine( "Undo Object Accepted (Undo:%d,Redo:%d, Size=%dKb)",m_undoStack.size(),m_redoStack.size(),GetDatabaseSize()/1024 );
		
		// If undo accepted, document modified.
		GetIEditor()->SetModifiedFlag();
	}
	else
	{
		// If no any object was recorded, Cancel undo operation.
		Cancel();
	}

	m_bRecording = false;
	m_currentUndo = 0;

	//CLogFile::WriteLine( "<Undo> Accept OK" );
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::Cancel()
{
	//CLogFile::WriteLine( "<Undo> Cancel" );
	if (m_bUndoing || m_bRedoing) // If Undoing or redoing now, ignore this calls.
		return;

	if (!m_bRecording)
		return;

	assert( m_currentUndo != 0 );

	m_bRecording = false;

	if (!m_currentUndo->IsEmpty())
	{
		// Restore all objects to the state they was at Begin call and throws out all undo objects.
		Restore( true );
		CLogFile::FormatLine( "Undo Object Canceled (Undo:%d,Redo:%d)",m_undoStack.size(),m_redoStack.size() );
	}
	
	delete m_currentUndo;
	m_currentUndo = 0;
	//CLogFile::WriteLine( "<Undo> Cancel OK" );
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::Redo( int numSteps )
{
	if (m_bUndoing || m_bRedoing) // If Undoing or redoing now, ignore this calls.
		return;

	if (m_bRecording || m_bSuperRecording)
	{
		CLogFile::FormatLine( "Cannot Redo while Recording" );
		return;
	}

	if (!m_redoStack.empty())
	{
		Suspend();
		while (numSteps-- > 0 && !m_redoStack.empty())
		{
			m_bRedoing = true;
			CUndoStep *redo = m_redoStack.back();
			redo->Redo();
			m_redoStack.pop_back();
			// Push undo object to redo stack.
			m_undoStack.push_back( redo );
			m_bRedoing = false;
		}
		Resume();
	}
	if (m_suspendCount == 0)
		GetIEditor()->UpdateViews(eUpdateObjects);
	CLogFile::FormatLine( "Redo (Undo:%d,Redo:%d)",m_undoStack.size(),m_redoStack.size() );
	GetIEditor()->GetObjectManager()->InvalidateVisibleList();
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::Undo( int numSteps )
{
	if (m_bUndoing || m_bRedoing) // If Undoing or redoing now, ignore this calls.
		return;

	if (m_bRecording || m_bSuperRecording)
	{
		CLogFile::FormatLine( "Cannot Undo while Recording" );
		return;
	}

	if (!m_undoStack.empty())
	{
		Suspend();
		while (numSteps-- > 0 && !m_undoStack.empty())
		{
			m_bUndoing = true;
			CUndoStep *undo = m_undoStack.back();
			undo->Undo(true);
			m_undoStack.pop_back();
			// Push undo object to redo stack.
			m_redoStack.push_back( undo );
			m_bUndoing = false;
		}
		Resume();
	}
	// Update viewports.
	if (m_suspendCount == 0)
		GetIEditor()->UpdateViews(eUpdateObjects);
	CLogFile::FormatLine( "Undo (Undo:%d,Redo:%d)",m_undoStack.size(),m_redoStack.size() );
	GetIEditor()->GetObjectManager()->InvalidateVisibleList();
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::RecordUndo( IUndoObject *obj )
{
	//CLogFile::FormatLine( "<Undo> RecordUndo Name=%s",obj->GetDescription() );

	if (m_bUndoing || m_bRedoing) // If Undoing or redoing now, ignore this calls.
	{
		//CLogFile::WriteLine( "<Undo> RecordUndo (Undoing or Redoing)" );
		obj->Release();
		return;
	}

	if (m_bRecording)
	{
		assert( m_currentUndo != 0 );
		m_currentUndo->AddUndoObject( obj );
		//CLogFile::FormatLine( "Undo Object Added: %s",obj->GetDescription() );
	}
	else
	{
		//CLogFile::WriteLine( "<Undo> RecordUndo (Not Recording)" );
		// Ignore this object.
		obj->Release();
	}
}
 
//////////////////////////////////////////////////////////////////////////
void CUndoManager::ClearRedoStack()
{
	for (std::list<CUndoStep*>::iterator it = m_redoStack.begin(); it != m_redoStack.end(); it++)
	{
		delete *it;
	}
	m_redoStack.clear();
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::ClearUndoStack()
{
	for (std::list<CUndoStep*>::iterator it = m_undoStack.begin(); it != m_undoStack.end(); it++)
	{
		delete *it;
	}
	m_undoStack.clear();
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::ClearUndoStack( int num )
{
	int i = num;
	while (i > 0 && !m_undoStack.empty())
	{
		delete m_undoStack.front();
		m_undoStack.pop_front();
		i--;
	}
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::ClearRedoStack( int num )
{
	int i = num;
	while (i > 0 && !m_redoStack.empty())
	{
		delete m_redoStack.back();
		m_redoStack.pop_back();
		i--;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CUndoManager::IsHaveRedo() const
{
	return !m_redoStack.empty();
}

//////////////////////////////////////////////////////////////////////////
bool CUndoManager::IsHaveUndo() const
{
	return !m_undoStack.empty();
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::Suspend()
{
	m_suspendCount++;
	//CLogFile::FormatLine( "<Undo> Suspend %d",m_suspendCount );
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::Resume()
{
	assert( m_suspendCount >= 0 );
	m_suspendCount--;
	//CLogFile::FormatLine( "<Undo> Resume %d",m_suspendCount );
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::SuperBegin()
{
	//CLogFile::FormatLine( "<Undo> SuperBegin (SuspendCount%d)",m_suspendCount );
	if (m_bUndoing || m_bRedoing) // If Undoing or redoing now, ignore this calls.
		return;

	m_bSuperRecording = true;
	m_superUndo = new CSuperUndoStep;
	//CLogFile::WriteLine( "<Undo> SuperBegin OK" );
}
	
//////////////////////////////////////////////////////////////////////////
void CUndoManager::SuperAccept( const CString &name )
{
	//CLogFile::WriteLine( "<Undo> SupperAccept" );
	if (m_bUndoing || m_bRedoing) // If Undoing or redoing now, ignore this calls.
		return;

	if (!m_bSuperRecording)
		return;

	assert( m_superUndo != 0 );

	if (m_bRecording)
		Accept( name );

	if (!m_superUndo->IsEmpty())
	{
		m_superUndo->SetName(name);
		// Keep max undo steps.
		if (m_undoStack.size() > gSettings.undoLevels)
		{
			delete m_undoStack.front();
			m_undoStack.pop_front();
		}
		m_undoStack.push_back( m_superUndo );
	}
	else
	{
		// If no any object was recorded, Cancel undo operation.
		SuperCancel();
	}

	CLogFile::FormatLine( "Undo Object Accepted (Undo:%d,Redo:%d)",m_undoStack.size(),m_redoStack.size() );
	m_bSuperRecording = false;
	m_superUndo = 0;
	//CLogFile::WriteLine( "<Undo> SupperAccept OK" );
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::SuperCancel()
{
	//CLogFile::WriteLine( "<Undo> SuperCancel" );
	if (m_bUndoing || m_bRedoing) // If Undoing or redoing now, ignore this calls.
		return;

	if (!m_bSuperRecording)
		return;

	assert( m_superUndo != 0 );

	if (m_bRecording)
		Cancel();

	Suspend();
	//! Undo all changes already made.
	m_superUndo->Undo(false); // Undo not by Undo command (no need to store Redo)
	Resume();

	m_bSuperRecording = false;
	delete m_superUndo;
	m_superUndo = 0;
	//CLogFile::WriteLine( "<Undo> SuperCancel OK" );
}


//////////////////////////////////////////////////////////////////////////
int	CUndoManager::GetUndoStackLen() const
{
	return m_undoStack.size();
}

//////////////////////////////////////////////////////////////////////////
int	CUndoManager::GetRedoStackLen() const
{
	return m_redoStack.size();
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::GetUndoStackNames( std::vector<CString> &undos )
{
	undos.resize( m_undoStack.size() );
	int i = 0;
	for (std::list<CUndoStep*>::iterator it = m_undoStack.begin(); it != m_undoStack.end(); it++)
	{
		undos[i++] = (*it)->GetName();
	}
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::GetRedoStackNames( std::vector<CString> &redos )
{
	redos.resize( m_redoStack.size() );
	int i = 0;
	for (std::list<CUndoStep*>::iterator it = m_redoStack.begin(); it != m_redoStack.end(); it++)
	{
		redos[i++] = (*it)->GetName();
	}
}

//////////////////////////////////////////////////////////////////////////
int	CUndoManager::GetDatabaseSize()
{
	int size = 0;
	{
		for (std::list<CUndoStep*>::iterator it = m_undoStack.begin(); it != m_undoStack.end(); it++)
		{
			size += (*it)->GetSize();
		}
	}
	{
		for (std::list<CUndoStep*>::iterator it = m_redoStack.begin(); it != m_redoStack.end(); it++)
		{
			size += (*it)->GetSize();
		}
	}
	return size;
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::Flush()
{
	m_bRecording = false;
	ClearRedoStack();
	ClearUndoStack();

	delete m_superUndo;
	delete m_currentUndo;
	m_superUndo = 0;
	m_currentUndo = 0;
}

//////////////////////////////////////////////////////////////////////////
void CUndoManager::SetMaxUndoStep( int steps )
{
	gSettings.undoLevels = steps;
};

//////////////////////////////////////////////////////////////////////////
int CUndoManager::GetMaxUndoStep() const
{
	return gSettings.undoLevels;
}