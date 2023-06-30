////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   gizmo.h
//  Version:     v1.00
//  Created:     2/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __gizmo_h__
#define __gizmo_h__

#if _MSC_VER > 1000
#pragma once
#endif

// forward declarations.
struct DisplayContext;
struct HitContext;
struct IGizmoManager;

enum EGizmoFlags
{
	EGIZMO_SELECTABLE = 0x0001, //! If set gizmo can be selected by clicking.
	EGIZMO_HIDDEN = 0x0002,			//! If set gizmo hidden and should not be displayed.
};

/** Any helper object that BaseObjects can use to display some usefull information like tracks.
		Gizmo's life time should be controlled by thier owning BaseObjects.
*/
class CGizmo : public CRefCountBase
{
public:
	CGizmo();
	~CGizmo();

	//! Set gizmo object flags.
	void SetFlags( uint flags ) { m_flags = flags; }
	//! Get gizmo object flags.
	uint GetFlags() const { return m_flags; }

	/** Get bounding box of Gizmo in world space.
		@param bbox Returns bounding box.
	*/
	virtual void GetWorldBounds( BBox &bbox ) = 0;
	
	/** Set transformation matrix of this gizmo.
	*/
	virtual void SetMatrix( const Matrix44 &tm );

	/** Get transformation matrix of this gizmo.
	*/
	virtual const Matrix44& GetMatrix() const { return m_matrix; }

	/** Display Gizmo in the viewport.
	*/
	virtual void Display( DisplayContext &dc ) = 0;

	/** Performs hit testing on gizmo object.
	*/
	virtual bool HitTest( HitContext &hc ) { return false; };

	/** Return gizmo manager that owns this gizmo.
	*/
	IGizmoManager* GetGizmoManager() const;

	//! Is this gizmo need to be deleted?.
	bool IsDelete() const { return m_bDelete; }
	//! Set this gizmo to be deleted.
	void DeleteThis();

protected:
	Matrix44 m_matrix;
	bool m_bDelete; // This gizmo is marked for deletion.
	uint m_flags;
};

// Define CGizmoPtr smart pointer.
SMARTPTR_TYPEDEF(CGizmo);

#endif // __gizmo_h__
