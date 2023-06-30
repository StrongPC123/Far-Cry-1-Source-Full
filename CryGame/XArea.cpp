//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XArea.cpp
//  Description: 2D area class. Area is in XY plane.
//	2D areas manager
//
//  History:
//  - Feb 24, 2002: Created by Kirill Bulatsev
//
//////////////////////////////////////////////////////////////////////
 
#include "stdafx.h"
#include "xarea.h"
#include "xPlayer.h"
#include "TagPoint.h"

#ifdef _XBOX
#ifdef k2
#undef k2
#endif
#endif

//////////////////////////////////////////////////////////////////////////
CXArea::CXArea(void):
m_VOrigin(0.0f),
m_VSize(0.0f),
m_PrevFade(-1.0f),
m_Proximity(5.0f),
m_AreaGroupID(-1),
m_AreaID(-1),
m_bIsActive(false)
{
	m_InvMatrix.SetIdentity();
}


//////////////////////////////////////////////////////////////////////////
CXArea::~CXArea(void)
{
	ClearPoints();
}

// resets area - clears all segments in area
//////////////////////////////////////////////////////////////////////////
void	CXArea::ClearPoints()
{
	a2DSegment*	carSegment;

	for(unsigned int sIdx=0; sIdx<m_vpSegments.size(); sIdx++)
	{
		carSegment = m_vpSegments[sIdx];
		delete carSegment;
	}
	m_vpSegments.clear();
}

//////////////////////////////////////////////////////////////////////////
unsigned CXArea::MemStat()
{
	unsigned memSize = sizeof *this;

	memSize += m_vpSegments.size()*(sizeof(a2DSegment) + sizeof(a2DSegment*)) ;
	return memSize;	
}

//adds segment to area, calculates line parameters y=kx+b, sets horizontal/vertical flags
//////////////////////////////////////////////////////////////////////////
void	CXArea::AddSegment(const a2DPoint& p0, const a2DPoint& p1)
{
	a2DSegment*	newSegment = new a2DSegment;

	//if this is horizontal line set flag. This segment is needed only for distance calculations
	if(p1.y == p0.y)
		newSegment->isHorizontal = true;
	else
		newSegment->isHorizontal = false;

	if( p0.x < p1.x )
	{
		newSegment->bbox.min.x = p0.x;
		newSegment->bbox.max.x = p1.x;
	}
	else
	{
		newSegment->bbox.min.x = p1.x;
		newSegment->bbox.max.x = p0.x;
	}

	if( p0.y < p1.y )
	{
		newSegment->bbox.min.y = p0.y;
		newSegment->bbox.max.y = p1.y;
	}
	else
	{
		newSegment->bbox.min.y = p1.y;
		newSegment->bbox.max.y = p0.y;
	}

	if(!newSegment->isHorizontal)
	{
		//if this is vertical line - spesial case
		if(p1.x == p0.x)
		{
			newSegment->k = 0;
			newSegment->b = p0.x;
		}
		else
		{
			newSegment->k = (p1.y - p0.y)/(p1.x - p0.x);
			newSegment->b = p0.y - newSegment->k*p0.x;
		}
	}
	else
	{
			newSegment->k = 0;
			newSegment->b = 0;
	}
	m_vpSegments.push_back( newSegment );
}

// calculates min distance from point within area to the border of area
// returns fade coefficient: Distance/m_Proximity
// [0 - on the very border of area,	1 - inside area, distance to border is more than m_Proximity]
//////////////////////////////////////////////////////////////////////////	
float	CXArea::CalcDistToPoint(const a2DPoint& point) const
{
	if (m_Proximity==0.0f)
		return 1.0f;

	float	distMin = m_Proximity*m_Proximity;
	float	curDist;
	a2DBBox	proximityBox;

	proximityBox.max.x = point.x + m_Proximity;
	proximityBox.max.y = point.y + m_Proximity;
	proximityBox.min.x = point.x - m_Proximity;
	proximityBox.min.y = point.y - m_Proximity;

	for(unsigned int sIdx=0; sIdx<m_vpSegments.size(); sIdx++)
	{
		a2DSegment *curSg = m_vpSegments[sIdx];

		if(!m_vpSegments[sIdx]->bbox.BBoxOutBBox2D( proximityBox ))
		{
			if(m_vpSegments[sIdx]->isHorizontal)
			{
				if( point.x<m_vpSegments[sIdx]->bbox.min.x )
					curDist = m_vpSegments[sIdx]->bbox.min.DistSqr( point );
				else if( point.x>m_vpSegments[sIdx]->bbox.max.x )
					curDist = m_vpSegments[sIdx]->bbox.max.DistSqr( point );
				else
					curDist = fabsf( point.y-m_vpSegments[sIdx]->bbox.max.y );
				curDist *= curDist;
			}
			else
			if(m_vpSegments[sIdx]->k==0.0f)
			{
				if( point.y<m_vpSegments[sIdx]->bbox.min.y )
					curDist = m_vpSegments[sIdx]->bbox.min.DistSqr( point );
				else if( point.y>m_vpSegments[sIdx]->bbox.max.y )
					curDist = m_vpSegments[sIdx]->bbox.max.DistSqr( point );
				else
					curDist = fabsf( point.x-m_vpSegments[sIdx]->b );
				curDist *= curDist;
			}
			else
			{
				a2DPoint	intersection;
				float	b2, k2;
				k2 = -1.0f/m_vpSegments[sIdx]->k;
				b2 = point.y - k2*point.x;
				intersection.x = (b2 - m_vpSegments[sIdx]->b)/(m_vpSegments[sIdx]->k - k2);
				intersection.y = k2*intersection.x + b2;

				if( intersection.x<m_vpSegments[sIdx]->bbox.min.x)
					if( m_vpSegments[sIdx]->k<0 )
						curDist = point.DistSqr( m_vpSegments[sIdx]->bbox.min.x, m_vpSegments[sIdx]->bbox.max.y );
					else
						curDist = point.DistSqr( m_vpSegments[sIdx]->bbox.min );
				else if( intersection.x>m_vpSegments[sIdx]->bbox.max.x)
					if( m_vpSegments[sIdx]->k<0 )
						curDist = point.DistSqr( m_vpSegments[sIdx]->bbox.max.x, m_vpSegments[sIdx]->bbox.min.y );
					else
						curDist = point.DistSqr( m_vpSegments[sIdx]->bbox.max );
				else
					curDist = intersection.DistSqr( point );
			}
			if(curDist<distMin)
				distMin = curDist;	
		}
	}
	return cry_sqrtf(distMin)/m_Proximity;
}

// check if the point is within the area
// first BBox check, then count number of intersections for horizontal ray from point and area segments
// if the number is odd - the point is inside
//////////////////////////////////////////////////////////////////////////
bool	CXArea::IsPointWithin(const Vec3d& point3d) const
{
/*
	if( m_AreaType == ATP_SECTOR )
	{
		if( building == m_Building && sector == m_Sector )
			return true;
		return false;
	}
*/

	if( m_AreaType == ATP_SPHERE )
	{
		Vec3d	sPnt = point3d - m_Center;
		if( GetLengthSquared(sPnt) < m_Radius2 )
			return true;
		return false;
	}
	if( m_AreaType == ATP_BOX )
	{
		Vec3d p3d=m_InvMatrix.TransformPointOLD(point3d);
		if (	(p3d.x<m_Min.x) ||
					(p3d.y<m_Min.y) ||
					(p3d.z<m_Min.z) ||
					(p3d.x>m_Max.x) ||
					(p3d.y>m_Max.y) ||
					(p3d.z>m_Max.z))
						return false;
		return true;
	}
	if( m_AreaType == ATP_SHAPE )
	{
		if( m_VSize>0.0f )
			if( point3d.z<m_VOrigin || point3d.z>m_VOrigin + m_VSize )
				return false;

		a2DPoint	*point = (CXArea::a2DPoint*)(&point3d);

		if( m_BBox.PointOutBBox2D( *point ) )
			return false;

		int	cntr=0;	

		for(unsigned int sIdx=0; sIdx<m_vpSegments.size(); sIdx++)
		{
			if(m_vpSegments[sIdx]->isHorizontal)
				continue;
			if(!m_vpSegments[sIdx]->bbox.PointOutBBox2DVertical( *point ))
				if( m_vpSegments[sIdx]->IntersectsXPosVertical( *point ) )
					cntr++;
			else
				if(!m_vpSegments[sIdx]->bbox.PointOutBBox2DVertical( *point ))
					if( m_vpSegments[sIdx]->IntersectsXPos( *point ) )
						cntr++;
		}
		return (cntr & 0x1);
	}

	// to keep the compiler happy and give more robustness to the code with the assert...
	assert(0); // can never reach this part
	return false; // default behavior
}

//	for editor use - if point is withing - returns min hor distance to border
//	if point out - returns -1
//////////////////////////////////////////////////////////////////////////
float	CXArea::IsPointWithinDist(const a2DPoint& point) const
{
int	cntr=0;	
float	minDist=-1;
float	dist;

	if( m_BBox.PointOutBBox2D( point ) )
		return -1;

	for(unsigned int sIdx=0; sIdx<m_vpSegments.size(); sIdx++)
	{
		if(m_vpSegments[sIdx]->isHorizontal)
			continue;
		if( m_vpSegments[sIdx]->k==0 )
			dist = m_vpSegments[sIdx]->b - point.x;
		else
			if(!m_vpSegments[sIdx]->bbox.PointOutBBox2D( point ))
				dist = m_vpSegments[sIdx]->GetIntersectX( point );
			else
				continue;
		if( dist<0 )
			dist = -dist;
		else
			cntr++;
		if( minDist>dist || minDist<0)
			minDist = dist;
	}
	if(cntr & 0x1)
		return	minDist;
	else
		return	-1;
}

//////////////////////////////////////////////////////////////////////////
void	CXArea::SetPoints(const Vec3d* vPoints, const int count)
{
	ClearPoints();
	// at least three points needed to create closed shape
	if(count>2)
	{
		int pIdx;
		for(pIdx=1; pIdx<count; pIdx++)
		{
			AddSegment( *((CXArea::a2DPoint*)(vPoints+pIdx-1)), *((CXArea::a2DPoint*)(vPoints+pIdx)) );
		}
		AddSegment( *((CXArea::a2DPoint*)(vPoints+pIdx-1)), *((CXArea::a2DPoint*)(vPoints)) );
		CalcBBox( );
	}
}

//////////////////////////////////////////////////////////////////////////
void	CXArea::CalcBBox( )
{
	m_BBox.min.x = m_vpSegments[0]->bbox.min.x;
	m_BBox.min.y = m_vpSegments[0]->bbox.min.y;
	m_BBox.max.x = m_vpSegments[0]->bbox.max.x;
	m_BBox.max.y = m_vpSegments[0]->bbox.max.y;
	for(unsigned int sIdx=1; sIdx<m_vpSegments.size(); sIdx++)
	{
		if( m_BBox.min.x>m_vpSegments[sIdx]->bbox.min.x )
			m_BBox.min.x = m_vpSegments[sIdx]->bbox.min.x;
		if( m_BBox.min.y>m_vpSegments[sIdx]->bbox.min.y )
			m_BBox.min.y = m_vpSegments[sIdx]->bbox.min.y;
		if( m_BBox.max.x<m_vpSegments[sIdx]->bbox.max.x )
			m_BBox.max.x = m_vpSegments[sIdx]->bbox.max.x;
		if( m_BBox.max.y<m_vpSegments[sIdx]->bbox.max.y )
			m_BBox.max.y = m_vpSegments[sIdx]->bbox.max.y;
	}
}

//////////////////////////////////////////////////////////////////////////
void	CXArea::SetTM( const Matrix44& TM )
{
	m_InvMatrix=TM;
	m_InvMatrix.Invert44();
}

//////////////////////////////////////////////////////////////////////////
void	CXArea::AddEntity( const char* const clsName )
{
	string name = clsName;
	m_vsEntityName.push_back( name );
	m_vEntityID.push_back( 0 );
}

//////////////////////////////////////////////////////////////////////////
void	CXArea::AddEntites( const std::vector<string> &names )
{
	for(unsigned int i=0; i<names.size(); i++)
		AddEntity(names[i].c_str());
}

//////////////////////////////////////////////////////////////////////////
void	CXArea::AddEntity( const EntityId entId )
{
	m_vEntityID.push_back( entId );
}

//////////////////////////////////////////////////////////////////////////
void	CXArea::AddEntites( const std::vector<EntityId> &entIDs )
{
	for(unsigned int i=0; i<entIDs.size(); i++)
		AddEntity(entIDs[i]);
}

//////////////////////////////////////////////////////////////////////////
void	CXArea::ClearEntities( )
{
	m_vsEntityName.clear();
	m_vEntityID.clear();
	m_PrevFade = -1.0f;
}



// init attached entities IDs list using entites names list
//////////////////////////////////////////////////////////////////////////
void	CXArea::UpdateIDs( ISystem * pSystem )
{
IEntity* pEntity;

	for( unsigned int eIdx=0; eIdx<m_vEntityID.size(); eIdx++ )
	{
		//char* tmp_name = (char *)m_vsEntityName[eIdx].c_str();
		pEntity = pSystem->GetIEntitySystem()->GetEntity(m_vEntityID[eIdx]);
		if(!pEntity)
		{
			pEntity = pSystem->GetIEntitySystem()->GetEntity((char *)m_vsEntityName[eIdx].c_str());
			if(!pEntity)
			{
//FIXME
//				m_vsEntityName.erase( m_vsEntityName.begin() + eIdx );
//				m_vEntityID.erase( m_vEntityID.begin() + eIdx );
//				eIdx--;

				m_vEntityID[eIdx] = 0;
				continue;
			}
			m_vEntityID[eIdx] = pEntity->GetId();
		}
	}
}


// do enter area - player was outside, now is inside
// calls entity OnEnterArea wich calls script OnEnterArea( player, AreaID )	
//////////////////////////////////////////////////////////////////////////
void	CXArea::EnterArea( CXAreaUser& user )
{
	IEntity* pEntity;

	if (user.m_pGame->m_bIsLoadingLevelFromFile)
		return;

	m_PrevFade = -1;
	m_bIsActive = true;
	UpdateIDs( user.m_pGame->GetSystem() );
	for( unsigned int eIdx=0; eIdx<m_vEntityID.size(); eIdx++ )
	{
		pEntity = user.m_pGame->GetSystem()->GetIEntitySystem()->GetEntity(m_vEntityID[eIdx]);
//		ASSERT(pEntity);
		if (pEntity)
		{
			if(user.m_pGame->a_LogArea->GetIVal())
				user.m_pGame->m_pLog->Log("\001 CXArea::EnterArea calling  %s\n", pEntity->GetName() );
			pEntity->OnEnterArea( pEntity, m_AreaID);
		}
	}
}

// do enter area - player was outside, now is inside
// calls entity OnEnterArea wich calls script OnEnterArea( player, AreaID )	
//////////////////////////////////////////////////////////////////////////
void	CXArea::EnterArea( IEntity* const pEntity, ISystem *pSystem )
{
	IEntity* pAreaAttachedEntity;

	m_PrevFade = -1;
	m_bIsActive = true;
	UpdateIDs( pSystem );
	for( unsigned int eIdx=0; eIdx<m_vEntityID.size(); eIdx++ )
	{
		pAreaAttachedEntity = pSystem->GetIEntitySystem()->GetEntity(m_vEntityID[eIdx]);
//		ASSERT(pEntity);
		if (pAreaAttachedEntity)
		{
			pAreaAttachedEntity->OnEnterArea( pEntity, m_AreaID);
//			if(a_LogArea->GetIVal())
//				m_pGame->m_pLog->Log("\001 CXArea::EnterArea reenter calling  %s\n", pAreaAttachedEntity->GetName() );
		}
	}
}


// do leave area - player was inside, now is outside
// calls entity OnLeaveArea wich calls script OnLeaveArea( player, AreaID )	
//////////////////////////////////////////////////////////////////////////
void	CXArea::LeaveArea( CXAreaUser& user )
{
	IEntity* pEntity;

	if (user.m_pGame->m_bIsLoadingLevelFromFile)
		return;

	UpdateIDs( user.m_pGame->GetSystem() );
	for( unsigned int eIdx=0; eIdx<m_vEntityID.size(); eIdx++ )
	{
		pEntity = user.m_pGame->GetSystem()->GetIEntitySystem()->GetEntity(m_vEntityID[eIdx]);
//		ASSERT(pEntity);
		if (pEntity)
		{
			pEntity->OnLeaveArea( user.m_pEntity, m_AreaID);
			if(user.m_pGame->a_LogArea->GetIVal())
				user.m_pGame->m_pLog->Log("\001 CXArea::LeaveArea calling  %s\n", pEntity->GetName() );
		}
	}
	m_PrevFade = -1;
	m_bIsActive = false;
}

//calculate distance to area - proceed fade. player is inside of the area
//////////////////////////////////////////////////////////////////////////
void	CXArea::UpdateArea( CXAreaUser& user )
{
	ProceedFade( user, CalculateFade( user.m_vPos ));
}

//calculate distance to area. player is inside of the area 
//////////////////////////////////////////////////////////////////////////
float	CXArea::CalculateFade( const Vec3d& pos3D )
{
//	a2DPoint pos = *((CXArea::a2DPoint*)(&pos3D));
	a2DPoint pos = CXArea::a2DPoint(pos3D);
	float fadeCoeff;
	
	switch(m_AreaType)
	{
	case	 ATP_SHAPE:
					fadeCoeff = CalcDistToPoint( pos );		
					break;
	case	 ATP_SPHERE:
		{
					if (m_Proximity<=0.0f)
					{
						fadeCoeff=1.0f;
						break;
					}
					Vec3d Delta=pos3D-m_Center;
					fadeCoeff=(m_Radius-Delta.Length())/m_Proximity;
					if (fadeCoeff>1.0f)
						fadeCoeff=1.0f;
					break;
		}
	case	 ATP_BOX:
		{
					if (m_Proximity<=0.0f)
					{
						fadeCoeff=1.0f;
						break;
					}
					Vec3d p3D=m_InvMatrix.TransformPointOLD(pos3D);
					Vec3d MinDelta=p3D-m_Min;
					Vec3d MaxDelta=m_Max-p3D;
					Vec3d EdgeDist=(m_Max-m_Min)/2.0f;
					if ((!EdgeDist.x) ||
							(!EdgeDist.y) ||
							(!EdgeDist.z))
					{
						fadeCoeff=1.0f;
						break;
					}

					float fFadeScale=m_Proximity/100.0f;
					EdgeDist*=fFadeScale;

					//float fFade=MinDelta.x/EdgeDist.x;
					float fMinFade=MinDelta.x/EdgeDist.x;					

					for (int k=0;k<3;k++)
					{
						float fFade1=MinDelta[k]/EdgeDist[k];
						float fFade2=MaxDelta[k]/EdgeDist[k];
						fMinFade=min(fMinFade,min(fFade1,fFade2));
					} //k

					/*
					fFade=MinDelta.y/EdgeDist.y;
					if (fFade<fMinFade)
						fMinFade=fFade;

					fFade=MinDelta.z/EdgeDist.z;
					if (fFade<fMinFade)
						fMinFade=fFade;

					fFade=MaxDelta.x/EdgeDist.x;
					if (fFade<fMinFade)
						fMinFade=fFade;

					fFade=MaxDelta.y/EdgeDist.y;
					if (fFade<fMinFade)
						fMinFade=fFade;

					fFade=MaxDelta.z/EdgeDist.z;
					if (fFade<fMinFade)
						fMinFade=fFade;
					*/

					fadeCoeff=fMinFade;
					if (fadeCoeff>1.0f)
						fadeCoeff=1.0f;
					break;
		}
	}

	return fadeCoeff;
}

//proceed fade. player is inside of the area
//////////////////////////////////////////////////////////////////////////
void	CXArea::ProceedFade( CXAreaUser& user, const float fadeCoeff )
{
	// no update if fade coefficient hasn't changed
	if(m_PrevFade == fadeCoeff)
		return;
	m_PrevFade = fadeCoeff;
	UpdateIDs( user.m_pGame->GetSystem() );

	IEntity* pEntity;
	for( unsigned int eIdx=0; eIdx<m_vEntityID.size(); eIdx++ )
	{
		pEntity = user.m_pGame->GetSystem()->GetIEntitySystem()->GetEntity(m_vEntityID[eIdx]);
//		ASSERT(pEntity);
		if (pEntity)
			pEntity->OnProceedFadeArea( user.m_pEntity, m_AreaID, fadeCoeff);
	}
}


//////////////////////////////////////////////////////////////////////////
void	CXArea::Draw(const ISystem * const pSystem, const int idx) 
{
	CFColor	colorsArray[] = {	CFColor(1.0f, 0.0f, 0.0f, 1.0f ),
														CFColor(0.0f, 1.0f, 0.0f, 1.0f),
														CFColor(0.0f, 0.0f, 1.0f, 1.0f),
														CFColor(1.0f, 1.0f, 0.0f, 1.0f),
														CFColor(1.0f, 0.0f, 1.0f, 1.0f),
														CFColor(0.0f, 1.0f, 1.0f, 1.0f),
														CFColor(1.0f, 1.0f, 1.0f, 1.0f),
													};
	CFColor	color = colorsArray[idx%(sizeof(colorsArray)/sizeof(CFColor))];
	CFColor	color1 = colorsArray[(idx+1)%(sizeof(colorsArray)/sizeof(CFColor))];
	CFColor	color2 = colorsArray[(idx+2)%(sizeof(colorsArray)/sizeof(CFColor))];
	Vec3d	v0, v1;
	float	deltaZ=1.5f;

	if(m_AreaType != ATP_SHAPE)
		return;

	for(unsigned int sIdx=0; sIdx<m_vpSegments.size(); sIdx++)
	{
		if( m_vpSegments[sIdx]->k<0 )
		{
			v0.x = m_vpSegments[sIdx]->bbox.min.x;
			v0.y = m_vpSegments[sIdx]->bbox.max.y;

			v1.x = m_vpSegments[sIdx]->bbox.max.x;
			v1.y = m_vpSegments[sIdx]->bbox.min.y;
		}
		else
		{
			v0.x = m_vpSegments[sIdx]->bbox.min.x;
			v0.y = m_vpSegments[sIdx]->bbox.min.y;

			v1.x = m_vpSegments[sIdx]->bbox.max.x;
			v1.y = m_vpSegments[sIdx]->bbox.max.y;
		}
		if( m_VSize>0.0f )
		{
			v0.z = m_VOrigin;
			v1.z = m_VOrigin;
		}
		else
		{
			v0.z = ((ISystem *)pSystem)->GetI3DEngine()->GetTerrainElevation(v0.x, v0.y) + deltaZ;
			v1.z = ((ISystem *)pSystem)->GetI3DEngine()->GetTerrainElevation(v1.x, v1.y) + deltaZ;
		}

		if(m_vpSegments[sIdx]->isHorizontal)		// horizontal segment
			((ISystem *)pSystem)->GetIRenderer()->DrawLineColor( v0, color2, v1, color2);
		else if(m_vpSegments[sIdx]->k==0)				// vertical segment
			((ISystem *)pSystem)->GetIRenderer()->DrawLineColor( v0, color1, v1, color1);
		else
		{
			((ISystem *)pSystem)->GetIRenderer()->DrawLineColor( v0, color, v1, color);
			if( m_VSize>0.0f )
			{
				v0.z = m_VOrigin + m_VSize;
				v1.z = m_VOrigin + m_VSize;
				((ISystem *)pSystem)->GetIRenderer()->DrawLineColor( v0, color, v1, color);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//*************************************************************************************************************
//
//			AREAs MANEGER
//
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
CXAreaMgr::CXAreaMgr(void):
m_sCurStep(3)
{
	m_lastUpdatePos.Set(0,0,0);
}

//////////////////////////////////////////////////////////////////////////
CXAreaMgr::~CXAreaMgr(void)
{
	Clear();
}

//////////////////////////////////////////////////////////////////////////
void CXAreaMgr::Clear()
{
CXArea	*carArea;

	for(unsigned int sIdx=0; sIdx<m_vpAreas.size(); sIdx++)
	{
		carArea = m_vpAreas[sIdx];
		delete carArea;
	}
	m_vpAreas.clear();
}

//-------------------------------------------------------------------------------------------------------------
//	adding area BOX
// min				- Min of box
// max				- Max of box
// names	- attached entities names
// width			-	fade width (m_Proximity), from border to withing 
//////////////////////////////////////////////////////////////////////////
CXArea*	CXAreaMgr::AddArea(const Vec3d& min, const Vec3d& max, const Matrix44& TM, const std::vector<string> &names, 
													 const int id, const int groupId, const float width)
{
	CXArea	*newArea;

	newArea = new CXArea;

	newArea->SetMin( min );
	newArea->SetMax( max );
	newArea->SetTM( TM );
	newArea->SetID(id);
	newArea->SetGroup(groupId);

	newArea->SetAreaType( CXArea::ATP_BOX );
//	newArea->SetID( id );
	newArea->AddEntites( names );
	newArea->SetProximity( width );

	m_vpAreas.push_back( newArea );
	return newArea;

}

//-------------------------------------------------------------------------------------------------------------
//	adding area SPHERE
// center			- pos of center of sphere
// radius			-	radius of the sphere
// names	- attached entities names
// width			-	fade width (m_Proximity), from border to withing 
//////////////////////////////////////////////////////////////////////////
CXArea*	CXAreaMgr::AddArea(const Vec3d& center, const float radius, const std::vector<string> &names, 
													const int id, const int groupId, const float width)
{
	CXArea	*newArea;

	// no building/sector  must be outside
	if(radius<=0)
		return NULL;

	newArea = new CXArea;

	newArea->SetCenter( center );
	newArea->SetRadius( radius );
	newArea->SetID(id);
	newArea->SetGroup(groupId);

	newArea->SetAreaType( CXArea::ATP_SPHERE );
//	newArea->SetID( id );
	newArea->AddEntites( names );
	newArea->SetProximity( width );

	m_vpAreas.push_back( newArea );
	return newArea;

}

//-------------------------------------------------------------------------------------------------------------
//	adding area SECTOR
// nBuilding	- building number
// nSectorId	-	sector ID
// entityID		-	attached entity id
// width			-	fade width (m_Proximity), from border to withing 
//////////////////////////////////////////////////////////////////////////
/*
CXArea*	CXAreaMgr::AddArea(const int nBuilding, const int nSectorId, const EntityId entityID, const float width)
{
	CXArea	*newArea;

	// no building/sector  must be outside
	if(nBuilding<0 || nSectorId<0)
		return NULL;

	newArea = new CXArea;

	newArea->SetBuilding( nBuilding );
	newArea->SetSector( nSectorId );

	newArea->SetAreaType( CXArea::ATP_SECTOR );
//	newArea->SetID( id );
	newArea->AddEntity( entityID );
	newArea->SetProximity( width );

	m_vpAreas.push_back( newArea );
	return newArea;

}
*/
//	adding area SHAPE
// vPoint - border points
// count	- number of border points
// names	- attached entities names
// id			- area ID, used for exclusive areas
// id			- group area belongs to, used for exclusive areas (exclusive withing group)
// width	- fade width (m_Proximity), from border to withing 
//////////////////////////////////////////////////////////////////////////
CXArea*	CXAreaMgr::AddArea(const Vec3d* const vPoints, const int count, const std::vector<string> &names, 
													 const int id, const int groupId, const float width)
{
	CXArea	*newArea;

	// at least three points needed to create closed shape
	if(count<3)
		return NULL;

	newArea = new CXArea;

	//
	//find the lowest point - to use for vertical origin in 3D area
	float	minZ=vPoints[0].z;
	for( int idx=1; idx<count; idx++ )
		if(minZ>vPoints[idx].z)
			minZ = vPoints[idx].z;
	newArea->SetVOrigin( minZ );

	newArea->SetPoints( vPoints, count );

	newArea->SetAreaType( CXArea::ATP_SHAPE );
	newArea->SetID( id );
	newArea->SetGroup(groupId);
	newArea->AddEntites( names );
	newArea->SetProximity( width );

	m_vpAreas.push_back( newArea );
	return newArea;
}

/*
//	gets area by building/sector/entityID
//////////////////////////////////////////////////////////////////////////
CXArea*	CXAreaMgr::GetArea(const int nBuilding, const int nSectorId, const EntityId entityID)
{
	for(unsigned int aIdx=0; aIdx<m_vpAreas.size(); aIdx++)
	{
		CXArea*	curArea = m_vpAreas[aIdx];
		if( curArea->GetAreaType() == CXArea::ATP_SECTOR )
		{
			if(		curArea->GetBuilding() == nBuilding 
				&& curArea->GetSector() == nSectorId 
//					&&curArea->Get
					)
				return curArea;
		}
	}
	return NULL;
}
*/

//	gets area by point
//////////////////////////////////////////////////////////////////////////
CXArea*	CXAreaMgr::GetArea( const Vec3d& point )
{
	float		dist;
	float		closeDist=-1;
	CXArea*	closeArea=NULL;

	for(unsigned int aIdx=0; aIdx<m_vpAreas.size(); aIdx++)
	{
		if( m_vpAreas[aIdx]->GetAreaType() == CXArea::ATP_SHAPE )
		{
			dist = m_vpAreas[aIdx]->IsPointWithinDist( *((CXArea::a2DPoint*)&point) );
			if(dist>0)
			{
				if( !closeArea || closeDist>dist )
				{
					closeDist = dist;
					closeArea = m_vpAreas[aIdx];
				}
			}
		}
	}
	return closeArea;
}

//////////////////////////////////////////////////////////////////////////
void	CXAreaMgr::DeleteArea( const IXArea* aPtr )
{
	for(unsigned int aIdx=0; aIdx<m_vpAreas.size(); aIdx++)
		if( m_vpAreas[aIdx]== aPtr)
		{
			delete aPtr;
			m_vpAreas.erase( m_vpAreas.begin() + aIdx );
		}
}

//	check player's position for all the areas. Do onEnter, onLeave, updateFade
//////////////////////////////////////////////////////////////////////////
void	CXAreaMgr::UpdatePlayer(CXAreaUser& user)
{
	if (user.m_pGame->m_bIsLoadingLevelFromFile)
		return;

	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	unsigned int aIdx;
//	int	building=0, sector=0;
//	IVisArea *sector=NULL;

//fixme - localplayer check should be earlier
//	if( player->GetEntity() != player->GetGame()->GetMyPlayer() )
//		return;

//	pos = player->GetGame()->GetSystem()->GetViewCamera().GetPos();
	// get check player position - not foot
	user.m_vPos = user.m_pEntity->GetPos() + Vec3d(0,0,1);
	if (user.m_vPos == m_lastUpdatePos)
		return;
	m_lastUpdatePos = user.m_vPos;

//	player->GetGame()->GetSystem()->GetI3DEngine()->GetBuildingManager()->CheckInside( pos, building, sector );
//	sector = (int)player->GetGame()->GetSystem()->GetI3DEngine()->GetVisAreaFromPos(pos);

	m_sCurStep++;

	// check all the areas player is in already
	for(unsigned int inIdx=0; inIdx<user.m_HostedAreasIdx.size(); inIdx++)
	{
		aIdx = user.m_HostedAreasIdx[inIdx];
		//safecheck for editor
		if( aIdx>=m_vpAreas.size() )
			return;
		m_vpAreas[aIdx]->m_stepID = m_sCurStep;
		if(!m_vpAreas[aIdx]->IsPointWithin(user.m_vPos))
		{
			// was inside, now is out - do OnLeaveArea
			ProceedExclusiveLeave( user, aIdx );
			user.m_HostedAreasIdx.erase( user.m_HostedAreasIdx.begin() + inIdx );
			inIdx--;
		}
	}

	// check all the rest areas (player is outside of them)
	for(aIdx=0; aIdx<m_vpAreas.size(); aIdx++)
		if( m_vpAreas[aIdx]->m_stepID != m_sCurStep )
			if(m_vpAreas[aIdx]->IsPointWithin(user.m_vPos))
			{
				// was outside, now inside - do enter area
				if(ProceedExclusiveEnter( user, aIdx ))
					m_vpAreas[aIdx]->EnterArea(user);
				user.m_HostedAreasIdx.push_back( aIdx );
			}

	//
	//update fade. For all hosted areas
	for(unsigned int inIdx=0; inIdx<user.m_HostedAreasIdx.size(); inIdx++)
	{
		aIdx = user.m_HostedAreasIdx[inIdx];
		//safecheck for editor
		if( aIdx>=m_vpAreas.size() )
			return;
		// this one already updated
		if( m_vpAreas[aIdx]->m_stepID == 0 )
			continue;
		// this area is not active - overriden by same groupId area with higher id (exclusive areas)
		if( !m_vpAreas[aIdx]->m_bIsActive )
			continue;
		ProceedExclusiveUpdate(user, aIdx);
	}
}

//	checks for areas in the same group, if entered area is lower priority (areaID) - return false 
//	(don't do enreArea for it)
//////////////////////////////////////////////////////////////////////////
void	CXAreaMgr::ProceedExclusiveUpdate( CXAreaUser& user, unsigned int curIdx )
{
int	maxArea=FindHighestHostedArea( user.m_HostedAreasIdx, curIdx );
CXArea *firstArea=m_vpAreas[curIdx];

	// not grouped or the only one in a group
	if( maxArea<0 )
	{
		firstArea->UpdateArea(user);
		return;
	}

	CXArea *secondArea=m_vpAreas[maxArea];

	if( firstArea->GetID()<secondArea->GetID() )
	{
		firstArea = secondArea;
		int	secondMaxArea=FindHighestHostedArea( user.m_HostedAreasIdx, maxArea );
		secondArea=m_vpAreas[secondMaxArea];
	}
	
	float fadeBase = firstArea->CalculateFade(user.m_vPos);

	if( fadeBase<1.0f )
	{
		if( !secondArea->m_bIsActive )
			secondArea->EnterArea(user);
		float fadeSecond = secondArea->CalculateFade(user.m_vPos);
		fadeSecond *= (1.0f-fadeBase);
		firstArea->ProceedFade(user, fadeBase);
		secondArea->ProceedFade(user, fadeSecond);
	}
	else
	{
		if( secondArea->m_bIsActive )
			secondArea->LeaveArea(user);
	}

	// mark all the areas of this group as updated
	int curGroup = m_vpAreas[curIdx]->GetGroup();
	for(unsigned int inIdx=0; inIdx<user.m_HostedAreasIdx.size(); inIdx++)
	{
		unsigned int aIdx = user.m_HostedAreasIdx[inIdx];
		//safecheck for editor
		if( aIdx>=m_vpAreas.size() )
			continue;
		if(m_vpAreas[curIdx]->GetGroup() != m_vpAreas[aIdx]->GetGroup())
			continue;
		m_vpAreas[aIdx]->m_stepID = 0;
	}
}

//	checks for areas in the same group, if entered area is lower priority (areaID) - return false 
//	(don't do enreArea for it)
//////////////////////////////////////////////////////////////////////////
bool	CXAreaMgr::ProceedExclusiveEnter( CXAreaUser& user, unsigned int curIdx )
{
int	maxArea=FindHighestHostedArea( user.m_HostedAreasIdx, curIdx );

	if(maxArea<0)
		return true;

	int maxID = m_vpAreas[maxArea]->GetID();
	if(m_vpAreas[curIdx]->GetID()<maxID)
		return false;
	return true;
}

//	checks for areas in the same group, if entered area is lower priority (areaID) - return false 
//	(don't do enreArea for it)
//////////////////////////////////////////////////////////////////////////
bool	CXAreaMgr::ProceedExclusiveLeave( CXAreaUser& user, unsigned int curIdx )
{
int	maxArea=FindHighestHostedArea( user.m_HostedAreasIdx, curIdx );

	if( maxArea<0 )
	{
		m_vpAreas[curIdx]->LeaveArea( user );
		return true;
	}

	int maxID = m_vpAreas[maxArea]->GetID();
	if( m_vpAreas[curIdx]->GetID()<maxID )
		return false;

	m_vpAreas[curIdx]->LeaveArea( user );
	if(!m_vpAreas[maxArea]->m_bIsActive)
		m_vpAreas[maxArea]->EnterArea( user );			// new area will owerride this one

	return true;
}

//	find hosted area with highest priority withing the group of curIdx area
//////////////////////////////////////////////////////////////////////////
int	CXAreaMgr::FindHighestHostedArea( intVector& hostedAreas, unsigned int curIdx )
{
unsigned int aIdx;
int	maxID=-1;
int	maxArea=-1;
int curGroup = m_vpAreas[curIdx]->GetGroup();

	if(curGroup<=0)
	{
		return maxArea;
	}
	for(unsigned int inIdx=0; inIdx<hostedAreas.size(); inIdx++)
	{
		aIdx = hostedAreas[inIdx];
		if( aIdx == curIdx )
			continue;
		//safecheck for editor
		if( aIdx>=m_vpAreas.size() )
			continue;
		if(curGroup != m_vpAreas[aIdx]->GetGroup())	// different groups
			continue;
		if(maxID<m_vpAreas[aIdx]->GetID())
		{
			maxID = m_vpAreas[aIdx]->GetID();
			maxArea	= aIdx;
		}
	}
	return maxArea;
}

//	do onexit for all areas player is in - do it before kill player
//////////////////////////////////////////////////////////////////////////
void	CXAreaMgr::ExitAllAreas(CXAreaUser& user)
{

//fixme - is this needed here - should be check before
//
	if( user.m_pEntity != user.m_pGame->GetMyPlayer() )
		return;

	// check all the areas player is in already
	for(unsigned int inIdx=0; inIdx<user.m_HostedAreasIdx.size(); inIdx++)
	{
	unsigned int aIdx;
		aIdx = user.m_HostedAreasIdx[inIdx];
		// was inside, now is out - do OnLeaveArea
		if(aIdx<m_vpAreas.size())
		{
			m_vpAreas[aIdx]->LeaveArea( user );
			user.m_HostedAreasIdx.erase( user.m_HostedAreasIdx.begin() + inIdx );
			inIdx--;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void	CXAreaMgr::DrawAreas(const ISystem * const pSystem) 
{
	for(unsigned int aIdx=0; aIdx<m_vpAreas.size(); aIdx++)
		m_vpAreas[aIdx]->Draw( pSystem, aIdx );
}

//////////////////////////////////////////////////////////////////////////
unsigned CXAreaMgr::MemStat()
{
	unsigned memSize = sizeof *this;

	for(unsigned int aIdx=0; aIdx<m_vpAreas.size(); aIdx++)
		memSize += m_vpAreas[aIdx]->MemStat();

	return memSize;
}

//////////////////////////////////////////////////////////////////////////
void CXAreaMgr::ReTriggerArea(IEntity* pEntity, const Vec3d &vPos,bool bIndoor)
{
intVector	hostedAreasIdx;
intVector	updatedID;
unsigned int aIdx;


	hostedAreasIdx.clear();
	updatedID.clear();
	// check all the rest areas (player is outside of them)
	for(aIdx=0; aIdx<m_vpAreas.size(); aIdx++)
		if(m_vpAreas[aIdx]->IsPointWithin(vPos))
			hostedAreasIdx.push_back(aIdx);

	for(int idxIdx=0; idxIdx<(int)(hostedAreasIdx.size()); idxIdx++  )
	{
		if(std::find(updatedID.begin(), updatedID.end(), m_vpAreas[hostedAreasIdx[idxIdx]]->GetGroup())!=updatedID.end())
			continue;
		int exclIdx = FindHighestHostedArea( hostedAreasIdx, hostedAreasIdx[idxIdx] );
		if(exclIdx<0)
		{
			m_vpAreas[hostedAreasIdx[idxIdx]]->EnterArea(pEntity, m_pSystem);
			continue;
		}
		m_vpAreas[exclIdx]->EnterArea(pEntity, m_pSystem);
		updatedID.push_back( m_vpAreas[hostedAreasIdx[idxIdx]]->GetGroup() );
	}


/*
unsigned int aIdx;
int	sector = 0;		//(int)player->GetGame()->GetSystem()->GetI3DEngine()->GetVisAreaFromPos(pos);
int	building = 0;	 
	// check all the rest areas (player is outside of them)
	for(aIdx=0; aIdx<m_vpAreas.size(); aIdx++)
		if(m_vpAreas[aIdx]->IsPointWithin(vPos, building, sector))
		{
			m_vpAreas[aIdx]->EnterArea(pEntity, m_pSystem);
		}
*/
}

//////////////////////////////////////////////////////////////////////////
void CXAreaMgr::Init( ISystem * pSystem )
{
	m_pSystem = pSystem;
	m_pCurrArea=m_pPrevArea=NULL;
}

// [marco] functions common for game and editor mode
// to be able to retrigger areas etc.
//////////////////////////////////////////////////////////////////////////
void CXGame::CheckSoundVisAreas()
{
	m_XAreaMgr.m_pPrevArea=m_XAreaMgr.m_pCurrArea=NULL;
	
	if (m_pSystem->GetISoundSystem())	
		m_XAreaMgr.m_pPrevArea=m_pSystem->GetISoundSystem()->GetListenerArea();		
}

//////////////////////////////////////////////////////////////////////////
void CXGame::RetriggerAreas()
{
	if (m_pSystem->GetISoundSystem())	
	{
		m_XAreaMgr.m_pCurrArea=m_pSystem->GetISoundSystem()->GetListenerArea();	

		if ((!m_XAreaMgr.m_pCurrArea && m_XAreaMgr.m_pPrevArea) || (m_XAreaMgr.m_pCurrArea && !m_XAreaMgr.m_pPrevArea))
		{
			// if we switched between outdoor and indoor, let's
			// retrigger the area to account for random ambient sounds
			// indoor / outdoor only
	//		m_XAreaMgr.ReTriggerArea(m_pSystem->GetISoundSystem()->GetListenerPos(),false);
			if( GetMyPlayer() )
				m_XAreaMgr.ReTriggerArea(GetMyPlayer(), m_pSystem->GetISoundSystem()->GetListenerPos(),false);
		}
	}	
}

//////////////////////////////////////////////////////////////////////////
IXArea *CXGame::CreateArea( const Vec3d *vPoints, const int count, const std::vector<string>	&names, 
														const int type, const int groupId, const float width, const float height)
{
	IXArea* newArea = m_XAreaMgr.AddArea( vPoints, count, names, type, groupId, width );
	newArea->SetVSize( height );
	return	newArea;
	//	return (IXArea*)m_XAreaMgr.AddArea( vPoints, count, names, type, width );
}

//////////////////////////////////////////////////////////////////////////
IXArea *CXGame::CreateArea( const Vec3d& min, const Vec3d& max, const Matrix44& TM, const std::vector<string>	&names, 
														const int type, const int groupId, const float width)
{
	return m_XAreaMgr.AddArea(min, max, TM, names, type, groupId, width );
}

//////////////////////////////////////////////////////////////////////////
IXArea *CXGame::CreateArea( const Vec3d& center, const float radius, const std::vector<string>	&names, 
														const int type, const int groupId, const float width)
{
	return m_XAreaMgr.AddArea(center, radius, names, type, groupId, width );
}

//////////////////////////////////////////////////////////////////////////
void CXGame::DeleteArea( const IXArea *pArea )
{
	m_XAreaMgr.DeleteArea( (CXArea*) pArea);
}

//finds closest area the point is in
//////////////////////////////////////////////////////////////////////////
IXArea *CXGame::GetArea( const Vec3d &point )
{
	return (IXArea*)m_XAreaMgr.GetArea( point );
}

//////////////////////////////////////////////////////////////////////////
void CXAreaMgr::RetriggerAreas()
{
	if (m_pSystem->GetISoundSystem())	
	{
		m_pCurrArea = m_pSystem->GetISoundSystem()->GetListenerArea();	

		if ((!m_pCurrArea && m_pPrevArea) || ( m_pCurrArea && !m_pPrevArea ))
		{
			// if we switched between outdoor and indoor, let's
			// retrigger the area to account for random ambient sounds
			// indoor / outdoor only
			//		m_XAreaMgr.ReTriggerArea(m_pSystem->GetISoundSystem()->GetListenerPos(),false);
			if ( GetIXGame( m_pSystem->GetIGame() )->GetMyPlayer() )
				ReTriggerArea( GetIXGame( m_pSystem->GetIGame() )->GetMyPlayer(), m_pSystem->GetISoundSystem()->GetListenerPos(), false );
		}
	}	
}

//////////////////////////////////////////////////////////////////////////
IXArea *CXAreaMgr::CreateArea( const Vec3d *vPoints, const int count, const std::vector<string>	&names, 
						   const int type, const int groupId, const float width, const float height)
{
	IXArea* newArea = AddArea( vPoints, count, names, type, groupId, width );
	newArea->SetVSize( height );
	return	newArea;
	//	return (IXArea*)m_XAreaMgr.AddArea( vPoints, count, names, type, width );
}

//////////////////////////////////////////////////////////////////////////
IXArea *CXAreaMgr::CreateArea( const Vec3d& min, const Vec3d& max, const Matrix44& TM, const std::vector<string>	&names, 
						   const int type, const int groupId, const float width)
{
	return AddArea(min, max, TM, names, type, groupId, width );
}

//////////////////////////////////////////////////////////////////////////
IXArea *CXAreaMgr::CreateArea( const Vec3d& center, const float radius, const std::vector<string>	&names, 
						   const int type, const int groupId, const float width)
{
	return AddArea(center, radius, names, type, groupId, width );
}
