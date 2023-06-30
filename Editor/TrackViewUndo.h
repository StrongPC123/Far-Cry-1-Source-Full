////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   trackviewundo.h
//  Version:     v1.00
//  Created:     30/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __trackviewundo_h__
#define __trackviewundo_h__

#if _MSC_VER > 1000
#pragma once
#endif

struct IAnimTrack;
struct IAnimNode;

/** Undo object stored when track is modified.
*/
class CUndoTrackObject : public IUndoObject
{
public:
	CUndoTrackObject( IAnimTrack *track );
protected:
	virtual int GetSize() { return sizeof(*this); }
	virtual const char* GetDescription() { return "Track Modify"; };

	virtual void Undo( bool bUndo );
	virtual void Redo();

private:
	TSmartPtr<IAnimTrack> m_pTrack;
	XmlNodeRef m_undo;
	XmlNodeRef m_redo;
};


/** Undo object stored when sequence is modified.
*/
class CUndoAnimSequenceObject : public IUndoObject
{
public:
	CUndoAnimSequenceObject( IAnimSequence *seq );
protected:
	virtual int GetSize() { return sizeof(*this); }
	virtual const char* GetDescription() { return "Sequence Modify"; };

	virtual void Undo( bool bUndo );
	virtual void Redo();

private:
	TSmartPtr<IAnimSequence> m_pSequence;
	XmlNodeRef m_undo;
	XmlNodeRef m_redo;
};


#endif // __trackviewundo_h__
