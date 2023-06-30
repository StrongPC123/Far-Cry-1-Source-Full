
//////////////////////////////////////////////////////////////////////
//
//	Crytek Indoor Engine DLL source code
//	
//	File: IndoorShadowVolumes.h
//
//	History:
//	-August 28,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#ifndef INDOORSHADOWVOLUMES_H
#define INDOORSHADOWVOLUMES_H

#if _MSC_VER > 1000
# pragma once
#endif

//////////////////////////////////////////////////////////////////////
#define FLAG_SKIP_SHADOWVOLUME 1

//forward declarations
//////////////////////////////////////////////////////////////////////
#include "IndoorVolumes.h"
#include "I3DEngine.h"								// USE_SHADOW_VERSION

class CDLight;
class CRETriMeshShadow;
class IStencilShadowConnectivity;

//an entire shadow volume object containing many edges and faces
//////////////////////////////////////////////////////////////////////
class CShadowVolObject : public CVolume
{
public:	

	// constructor
	CShadowVolObject()
	{						
//		m_pFaceList=NULL;		
		m_pReMeshShadow=NULL;
		m_pSystemVertexBuffer=NULL;
		m_pEdgeConnectivity=0;
		m_nNumVertices=0;
//		m_nNumFaces=0;
	};

	//! destructor
	~CShadowVolObject();

	//!
	bool	CheckInside(const Vec3d &pos,bool bWorldSpace=true)
	{
		//temporary return false...will be a cool AI game play feature to know 
		//if we are in shadows
		return (false);
	}

	//! precalculate the connectivity infos to build the object silouhette
	bool CreateConnectivityInfo( void );	

	//! create/update a vertex buffer containing the shadow volume (for static lights)
	void	RebuildShadowVolumeBuffer( const CDLight &lSource, float fExtent );

  Vec3d * GetSysVertBufer() { return m_pSystemVertexBuffer; }

  int		GetNumVertices() { return(m_nNumVertices); }
//  int		GetNumFaces() { return(m_nNumFaces); }

	// Shader RenderElements for stencil
	CRETriMeshShadow *						m_pReMeshShadow;								//!<
  
  void CheckUnload();

  IStencilShadowConnectivity * & GetEdgeConnectivity() { return m_pEdgeConnectivity; }

protected:

//! free the shadow volume buffers
	void	FreeVertexBuffers();

	//list of faces from source objects, its always shared from the stat obj
//	int														m_nNumFaces;										//!<
//  CObjFace *										m_pFaceList;										//!< pointer to MeshIdx faces [0..m_nNumFaces-1]

	//list of edges...can be shared from another shadow vol object
	IStencilShadowConnectivity *	m_pEdgeConnectivity;						//!< stored edge connectivity for fast shadow edge extraction, could be 0, call ->Release() to free it

	TFixedArray<unsigned short>		m_arrIndices;										//!<
	unsigned											m_nNumVertices;									//!< number of vertices in SystemBuffer

	//!
	//! /param nNumIndices
	//! /param nNumVertices
	void PrepareShadowVolumeVertexBuffer( unsigned nNumIndices, unsigned nNumVertices );

	//shadow volume renderer vertex buffer
  Vec3d * m_pSystemVertexBuffer;
};

struct ItShadowVolume;
//////////////////////////////////////////////////////////////////////
struct tShadowVolume : public ItShadowVolume
{
	tShadowVolume()
	{
		pSvObj=NULL;
	}

	CShadowVolObject *GetShadowVolume()
	{
		return (pSvObj);
	}

	void SetShadowVolume(CShadowVolObject *pParmSvObj)
	{
		pSvObj=pParmSvObj;
	}

	void	Release()
	{
		if (pSvObj)
		{
			delete pSvObj;
			pSvObj=NULL;
		}

		delete this;
	}

	//! get/set position
	Vec3d GetPos()
	{
		return (pSvObj->GetPos());
	}

	void SetPos(const Vec3d &vPos)
	{
		pSvObj->SetPos(vPos);
	}

	//! recalculate the object's silouhette, mostly for dynamic lights
	//! the light should always be in object space
	void	RebuildShadowVolumeBuffer( const CDLight &lSource, float fExtent )
	{
		pSvObj->RebuildShadowVolumeBuffer(lSource, fExtent);
	}

  Vec3d * GetSysVertBufer() { return pSvObj->GetSysVertBufer(); }

  void CheckUnload() { pSvObj->CheckUnload(); }

	CShadowVolObject *pSvObj;	
};

#endif