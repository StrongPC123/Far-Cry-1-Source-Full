#pragma once

class CShadowVolObject //: public CVolume
{
public:	

  // constructor
  CShadowVolObject(ILog * pSystem)
  {						
    m_pFaceList=NULL;		
    m_pReMeshShadow=NULL;
    m_pSystemVertexBuffer=NULL;
    m_pEdgeConnectivity=0;
    m_nNumVertices=0;
    m_nNumFaces=0;
    m_pSystem = pSystem;
  };

  ILog * m_pSystem;

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
  void  CreateConnectivityInfo( CIndexedMesh * pIndexedMesh, ILog * pLog );	

  //! create/update a vertex buffer containing the shadow volume (for static lights)
  void	RebuildDynamicShadowVolumeBuffer( const CDLight &lSource, struct IVisArea * pVisArea );// lSource has to be object relative

  Vec3d * GetSysVertBufer() { return m_pSystemVertexBuffer; }

  int		GetNumVertices() { return(m_nNumVertices); }
  int		GetNumFaces() { return(m_nNumFaces); }

  // Shader RenderElements for stencil
  CRETriMeshShadow * m_pReMeshShadow;											//!<

  void CheckUnload();

  class IStencilShadowConnectivity * GetEdgeConnectivity() { return m_pEdgeConnectivity; }

protected:

  //! free the shadow volume buffers
  void	FreeVertexBuffers();

  //list of faces from source objects, its always shared from the stat obj
  int				m_nNumFaces;																			//
  CObjFace  *m_pFaceList;																			// pointer to MeshIdx faces [0..m_nNumFaces-1]

  //list of edges...can be shared from another shadow vol object
  IStencilShadowConnectivity *m_pEdgeConnectivity;						//!< stored edge connectivity for fast shadow edge extraction, could be 0, call ->Release() to free it

  TFixedArray<unsigned short> m_arrIndices;										//!<
  unsigned										m_nNumVertices;									//!< number of vertices in SystemBuffer

  //!
  //! /param nNumIndices
  //! /param nNumVertices
  void PrepareShadowVolumeVertexBuffer( unsigned nNumIndices, unsigned nNumVertices );

  //shadow volume renderer vertex buffer
  Vec3d * m_pSystemVertexBuffer;
};

class CStatCGFShadVol
{
public:
  CStatCGFShadVol(ILog * pSystem, CIndexedMesh * pMesh);
  ~CStatCGFShadVol(void);
  CShadowVolObject * m_pShadowVolObject;
  void Serialize(int & nPos, void * pStream, bool bSave);
};
