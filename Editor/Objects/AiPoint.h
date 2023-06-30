////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   AIPoint.h
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: AIPoint object definition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __AIPoint_h__
#define __AIPoint_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "BaseObject.h"

enum EAIPointType
{
	EAIPOINT_WAYPOINT = 0,	//!< AI Graph node, waypoint.
	EAIPOINT_HIDE,			//!< Good hiding point.
	EAIPOINT_ENTRY,			//!< Entry point to indoors.
	EAIPOINT_EXIT,			//!< Exit point from indoors.
};

struct IVisArea;

/*!
 *	CAIPoint is an object that represent named AI waypoint in the world.
 *
 */
class CAIPoint : public CBaseObject
{
public:
	DECLARE_DYNCREATE(CAIPoint)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	void Done();
	void Display( DisplayContext &disp );

	virtual CString GetTypeDescription() const;

	//////////////////////////////////////////////////////////////////////////
	virtual void SetPos( const Vec3d &pos );
	virtual void SetAngles( const Vec3d &angles );
	virtual void SetScale( const Vec3d &angles );

	void BeginEditParams( IEditor *ie,int flags );
	void EndEditParams( IEditor *ie );

	//! Called when object is being created.
	int MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );

	bool HitTest( HitContext &hc );

	void GetBoundBox( BBox &box );
	void GetLocalBounds( BBox &box );

	void OnEvent( ObjectEvent event );

	void Serialize( CObjectArchive &ar );
	XmlNodeRef Export( const CString &levelPath,XmlNodeRef &xmlNode );

	//! Invalidates cached transformation matrix.
	virtual void InvalidateTM();

	virtual void SetHelperScale( float scale ) { m_helperScale = scale; };
	virtual float GetHelperScale() { return m_helperScale; };
	//////////////////////////////////////////////////////////////////////////

	//! Retreive number of link points.
	int GetLinkCount() const { return m_links.size(); }
	CAIPoint*	GetLink( int index );
	void AddLink( CAIPoint* obj,bool bNeighbour=false );
	void RemoveLink( CAIPoint* obj,bool bNeighbour=false );
	bool IsLinkSelected( int iLink ) const { m_links[iLink].selected; };
	void SelectLink( int iLink,bool bSelect ) { m_links[iLink].selected = bSelect; };

	void SetAIType( EAIPointType type );
	EAIPointType GetAIType() const { return m_aiType; };

	// Enable picking of AI points.
	void StartPick();

	//////////////////////////////////////////////////////////////////////////
	void Validate( CErrorReport *report );

protected:
	//! Dtor must be protected.
	CAIPoint();
	void DeleteThis() { delete this; };

	// Update links in AI graph.
	void UpdateLinks();
	float GetRadius();

	//! Ids of linked waypoints.
	struct Link
	{
		CAIPoint *object;
		GUID id;
		bool selected; // True if link is currently selected.
		Link() { object = 0; id = GUID_NULL; selected = false; }
	};
	std::vector<Link> m_links;
	EAIPointType m_aiType;

	//! True if this waypoint is indoors.
	bool m_bIndoors;
	IVisArea *m_pArea;

	//////////////////////////////////////////////////////////////////////////
	bool m_bLinksValid;
	bool m_bIgnoreUpdateLinks;

	// AI graph node.
	struct GraphNode *m_aiNode;
	bool m_bIndoorEntrance;

	Vec3 m_currHidePos;
	Vec3 m_currHideDir;

	static int m_rollupId;
	static class CAIPointPanel* m_panel;
	static float m_helperScale;
};

/*!
 * Class Description of AIPoint.	
 */
class CAIPointClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {07303078-B211-40b9-9621-9910A0271AB7}
		static const GUID guid = { 0x7303078, 0xb211, 0x40b9, { 0x96, 0x21, 0x99, 0x10, 0xa0, 0x27, 0x1a, 0xb7 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_AIPOINT; };
	const char* ClassName() { return "AIPoint"; };
	const char* Category() { return "AI"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CAIPoint); };
	int GameCreationOrder() { return 110; };
};

#endif // __AIPoint_h__