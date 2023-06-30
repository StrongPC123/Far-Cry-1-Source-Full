
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE source code
//	
//	File: IndoorVolumes.h
//
//	History:
//	-August 28,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#ifndef INDOORVOLUMES_H
#define INDOORVOLUMES_H

#if _MSC_VER > 1000
# pragma once
#endif

//////////////////////////////////////////////////////////////////////
#define FLAG_VOLUME_AREA								1
#define FLAG_VOLUME_PORTAL							2
#define FLAG_VOLUME_HULL								4
#define FLAG_VOLUME_FOG									8
#define FLAG_VOLUME_OBJECT							16
#define FLAG_VOLUME_DRAWN								32
#define FLAG_VOLUME_RECURSED						64
#define FLAG_GEOMETRY_SHARED						128
#define FLAG_OBJECT_VISIBLE_BY_PORTALS	256
#define FLAG_OBJECT_PRX									512		//this object is affected by per-pixel lighting by other lights
#define FLAG_OBJECT_PORTAL_ENTITY				1024
#define FLAG_OBJECT_DOUBLE_PORTAL				2048	

//////////////////////////////////////////////////////////////////////
//#define FLAG2_DRAWN		(1L<<17)


// this holds datas for volumes and areas
//////////////////////////////////////////////////////////////////////////
typedef struct s_Container 
{
	s_Container()
	{
		pObj=NULL;
		pEnt=NULL;
		pSV=NULL;
	}

	IStatObj					*pObj;
	IPhysicalEntity		*pEnt;
	class CShadowVolObject	*pSV;
}tContainer;

//////////////////////////////////////////////////////////////////////
typedef std::vector<tContainer> ContainerList;
typedef std::vector<tContainer>::iterator ContainerListIt;

//////////////////////////////////////////////////////////////////////
typedef std::vector<IStatObj*> istatobjlist;
typedef std::vector<IStatObj*>::iterator istatobjit;

//////////////////////////////////////////////////////////////////////
typedef std::vector<IPhysicalEntity*> iphysobjlist;
typedef std::vector<IPhysicalEntity*>::iterator iphysobjit;

//////////////////////////////////////////////////////////////////////
class CVolume : public Cry3DEngineBase
{
public:	
	//constructors/destructors
	//this constructor is only for building
	CVolume()
	{		
		m_dwFlags=0;				
		m_nObjCount=0;
		m_vOrigin(0,0,0);
		m_vMins=SetMaxBB();
		m_vMaxs=SetMinBB();		
  }
/*	
	CVolume() 
	{		
		m_dwFlags=0;	
		m_nObjCount=0;
		m_vOrigin(0,0,0);		
		m_vMins=SetMaxBB();
		m_vMaxs=SetMinBB();
	};
	*/
	virtual ~CVolume();

	int	GetFlags() { return(m_dwFlags); }
	//////////////////////////////////////////////////////////////////////
	//check if a point is inside the volume
	virtual bool	CheckInside(const Vec3d &pos,bool bWorldSpace) { return(false); }

	void		RemoveGeometry(IStatObj *pSource,ContainerListIt i);
	
	//void			SetGeometry(IStatObj *pSource);
	//void	AddGeometry(IStatObj *pSource,IPhysicalEntity *pEnt=NULL);
	void	AddGeometry(tContainer tCont);

	Vec3d	GetBBoxMin(bool bWorldSpace=false)
	{ 	
		if (bWorldSpace)
		{
			Vec3d vWorldPos=m_vOrigin+m_vMins;
			return(vWorldPos);
		}

		return(m_vMins); 
	}

	Vec3d	GetBBoxMax(bool bWorldSpace=false) 
	{ 
		if (bWorldSpace)
		{
			Vec3d vWorldPos=m_vOrigin+m_vMaxs;
			return(vWorldPos);
		}
		return(m_vMaxs); 
	}

	//set new volume's position 
	//////////////////////////////////////////////////////////////////////
	void	SetPos(const Vec3d &vPos); 
	Vec3d	GetPos() { return (m_vOrigin); }
		
	//bounding box
	Vec3d m_vMins;
	Vec3d	m_vMaxs;

	//volume flags
	int m_dwFlags;	

	//! the source geometry
	//IStatObj	*m_pSource;		
	int						m_nObjCount;

	ContainerList	m_lstObjects;
	//istatobjlist	m_lstStatObjs;
	//iphysobjlist	m_lstPhysObjs;

protected:		

	//! volume origin
	Vec3d m_vOrigin;	
	//Vec3d	m_vWorldPos;	
};

#endif