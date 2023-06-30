//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XArea.h
//  Description: 2D area class. Area is in XY plane. Area has enteties attached to it.
//	Area has fade width (m_Proximity) - distance from border to inside at wich fade coefficient
//	changes linearly from 0(on the border) to 1(inside distance to border more than m_Proximity). 
//	
//	Description: 2D areas manager. Checks player for entering/leaving areas. Updates fade 
//	coefficient for areas player is in
//
//  History:
//  - Feb 24, 2002: Created by Kirill Bulatsev
//
//////////////////////////////////////////////////////////////////////



#if !defined(_XAREA2D_H__INCLUDED_)
#define _XAREA2D_H__INCLUDED_


#pragma once


#include <IMarkers.h>
#include <Cry_Math.h>

class CXGame;

class CXAreaUser
{
friend class CXArea;
friend class CXAreaMgr;

	CXGame				*m_pGame;
	std::vector<int>	m_HostedAreasIdx;
	Vec3				m_vPos;
	IEntity*			m_pEntity;

public:
	void	SetGame( CXGame* pGame )		{ m_pGame = pGame; }
	void	SetEntity( IEntity* pEntity )	{ m_pEntity = pEntity; }
};

class CXArea : public IXArea
{
public:
	typedef enum {
		ATP_SHAPE = 0,
		ATP_SPHERE,
		ATP_SECTOR,
		ATP_BOX,
	}	tAreaType;
	struct a2DPoint
	{
		float x, y;
		a2DPoint(void):x(0.0f),y(0.0f) { }
		a2DPoint( const Vec3& pos3D ) { x=pos3D.x; y=pos3D.y; }
		float	DistSqr( const struct a2DPoint& point ) const
		{
		float xx = x-point.x;
		float yy = y-point.y;
			return (xx*xx + yy*yy);
		}
		float	DistSqr( const float px, const float py ) const
		{
		float xx = x-px;
		float yy = y-py;
			return (xx*xx + yy*yy);
		}

	};
	struct a2DBBox
	{
		a2DPoint	min;	// 2D BBox min 
		a2DPoint	max;	// 2D BBox max
		bool	PointOutBBox2D (const a2DPoint& point) const
		{
			return (point.x<min.x || point.x>max.x || point.y<min.y || point.y>max.y);
		}
		bool	PointOutBBox2DVertical (const a2DPoint& point) const
		{
			return (point.y<=min.y || point.y>max.y || point.x>max.x);
		}
		bool	BBoxOutBBox2D (const a2DBBox& box) const
		{
			return (box.max.x<min.x || box.min.x>max.x || box.max.y<min.y || box.min.y>max.y);
		}

	};
	struct a2DSegment
	{
		bool	isHorizontal;			//horizontal flag
		float	k, b;			//line parameters y=kx+b
		a2DBBox	bbox;		// segment's BBox 
		bool IntersectsXPos( const a2DPoint& point ) const
		{
			return ( point.x < (point.y - b)/k );
		}

		float GetIntersectX( const a2DPoint& point ) const
		{
			return (point.y - b)/k;
		}

		bool IntersectsXPosVertical( const a2DPoint& point ) const
		{
			if(k==0.0f)
				return ( point.x < b );
			return false;
		}
	};

	CXArea(void);
	~CXArea(void);

	void	SetPoints(const Vec3* const vPoints, const int count);
	void	SetID( const int id ) { m_AreaID = id; }
	int		GetID() const { return m_AreaID; }

	void	SetGroup( const int id) { m_AreaGroupID = id; } 
	int		GetGroup( ) const { return m_AreaGroupID; } 

	void	SetAreaType( const tAreaType type) { m_AreaType = type; } 
	tAreaType GetAreaType( ) const { return m_AreaType; } 

//	void	SetBuilding( const int nBuilding ) { m_Building = nBuilding; }
//	void	SetSector( const int nSector ) { m_Sector = nSector; }
//	int		GetBuilding( ) { return m_Building; }
//	int		GetSector( ) { return m_Sector; }

	void	SetCenter( const Vec3& center ) { m_Center=center; }
	void	SetRadius( const float rad ) { m_Radius=rad; m_Radius2=m_Radius*m_Radius; }

	void	SetMin( const Vec3& min ) { m_Min=min; }
	void	SetMax( const Vec3& max ) { m_Max=max; }
	void	SetTM( const Matrix44& TM );

	void	SetVOrigin( float org ) { m_VOrigin = org; }
	void	SetVSize( float sz=0.0f )		{ m_VSize = sz; }

	void	AddEntity( const char* const clsName );
	void	AddEntites( const std::vector<string> &names );
	void	AddEntity( const EntityId entId );
	void	AddEntites( const std::vector<EntityId> &entIDs );

	void	ClearEntities( );

	void	SetProximity( float prx ) {m_Proximity = prx;}
	float	GetProximity( ) { return m_Proximity;}

	float	IsPointWithinDist(const a2DPoint& point) const;
//	bool	IsPointWithin(const a2DPoint& point) const;
//	bool	IsPointWithin(const Vec3& point) const;
	bool	IsPointWithin(const Vec3& point3d) const;
	float	CalcDistToPoint( const a2DPoint& point ) const;

	void	UpdateIDs( ISystem * pSystem );
	void	EnterArea( CXAreaUser& user );
//fixme - do it all with IEntity, not CPlayer
	void	EnterArea( IEntity* const pEntity, ISystem *pSystem );
	void	LeaveArea( CXAreaUser& user ) ;
	void	UpdateArea( CXAreaUser& user );
	float	CalculateFade( const Vec3& pos3D );
	void	ProceedFade( CXAreaUser& user, const float fadeValue );


	void	Draw(const ISystem * const pSystem, const int idx);

	int		m_stepID;
	bool	m_bIsActive;

	unsigned MemStat();

private:

	void	AddSegment(const a2DPoint& p0, const a2DPoint& p1);
	void	CalcBBox();
	void	ClearPoints();

	float	m_Proximity;

	//	attached entities names list
	std::vector<string>	m_vsEntityName;
	//	attached entities IDs list
	std::vector<EntityId>			m_vEntityID;

	float	m_PrevFade;

	int			m_AreaID;
	int			m_AreaGroupID;

	Matrix44	m_InvMatrix;

	tAreaType	m_AreaType;


	// for shape areas ----------------------------------------------------------------------
	// area's bbox
	a2DBBox	m_BBox;
	// the area segments
	std::vector<a2DSegment*>	m_vpSegments;

	// for sector areas ----------------------------------------------------------------------
//	int	m_Building;
//	int	m_Sector;
//	IVisArea *m_Sector;

	// for box areas ----------------------------------------------------------------------
	Vec3 m_Min;
	Vec3 m_Max;

	// for sphere areas ----------------------------------------------------------------------
	Vec3 m_Center;
	float	m_Radius;
	float	m_Radius2;


	//	area vertical origin - the lowest point of area
	float	m_VOrigin;
	//	area height (vertical size). If (m_VSize<=0) - not used, only 2D check is done. Otherwise 
	//	additional check for Z to be in [m_VOrigin, m_VOrigin + m_VSize] range is done
	float	m_VSize;

};

struct IVisArea;
struct IXAreaMgr;

//Areas manager
class CXAreaMgr : public IXAreaMgr
{
public:
	CXAreaMgr(void);
	virtual ~CXAreaMgr(void);

	void Init( ISystem * pSystem );
	// adding shape area
	CXArea*	AddArea(const Vec3* const vPoints, const int count, const std::vector<string> &names, const int id, const int groupId=-1, const float width=0.0f);
	// adding sector area
//	CXArea*	AddArea(const int nBuilding, const int nSectorId, const EntityId entityID, const float width=0.0f);
	// adding box area
	CXArea*	AddArea(const Vec3& min, const Vec3& max, const Matrix44& TM, const std::vector<string> &names, const int id, const int groupId=-1, const float width=0.0f);
	// adding sphere area
	CXArea*	AddArea(const Vec3& center, const float radius, const std::vector<string> &names, const int id, const int groupId=-1, const float width=0.0f);


	void	ReTriggerArea(IEntity* pEntity,const Vec3 &vPos,bool bIndoor);

	void	UpdatePlayer( CXAreaUser& user );
	bool	ProceedExclusiveEnter( CXAreaUser& user, unsigned int curIdx );
	bool	ProceedExclusiveLeave( CXAreaUser& user, unsigned int curIdx );
	void	ProceedExclusiveUpdate( CXAreaUser& user, unsigned int curIdx );
//	int		FindHighestHostedArea( CPlayer* const player, unsigned int curIdx );
typedef std::vector<int>	intVector;
	int		FindHighestHostedArea( intVector& hostedAreas, unsigned int curIdx );
	void	ExitAllAreas( CXAreaUser& user );
	CXArea*	GetArea( const Vec3& point );
//	CXArea*	GetArea(const int nBuilding, const int nSectorId, const EntityId entityID);
	void	DeleteArea( const		IXArea* aPtr );
	
//	void	DeleteEntity();

	void	Clear();

	void	DrawAreas(const ISystem * const pSystem);
	unsigned MemStat();

	void RetriggerAreas();

	IXArea *CreateArea( const Vec3d *vPoints, const int count, const std::vector<string>	&names, 
		const int type, const int groupId, const float width=0.0f, const float height=0.0f);
	IXArea *CreateArea( const Vec3d& min, const Vec3d& max, const Matrix44& TM, const std::vector<string>	&names, 
		const int type, const int groupId, const float width);
	IXArea *CreateArea( const Vec3d& center, const float radius, const std::vector<string>	&names, 
		const int type, const int groupId, const float width);

	IVisArea *m_pPrevArea,*m_pCurrArea;

private:
	std::vector<CXArea*>	m_vpAreas;
	int m_sCurStep;
	ISystem * m_pSystem;	
	Vec3 m_lastUpdatePos;
};

#endif //!defined(_XAREA2D_H__INCLUDED_)