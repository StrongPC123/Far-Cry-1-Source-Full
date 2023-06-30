#include "StdAfx.h"
#include "statcgfshadvol.h"

#include "meshidx.h"
#include "IEdgeConnectivityBuilder.h"
#include "..\StencilShadowConnectivity.h"
#include "..\StencilShadowConnectivityBuilder.h"

CStatCGFShadVol::CStatCGFShadVol(ILog * pLog, CIndexedMesh * pIndexedMesh)
{
  m_pShadowVolObject = new CShadowVolObject( pLog );

  for (int i=0; i<pIndexedMesh->m_nFaceCount; i++)
  {
    CObjFace *cf=&pIndexedMesh->m_pFaces[i];		

    for (int v=0; v<3; v++)
    {			
      cf->m_Vecs[v].x=pIndexedMesh->m_pVerts[pIndexedMesh->m_pFaces[i].v[v]].x;
      cf->m_Vecs[v].y=pIndexedMesh->m_pVerts[pIndexedMesh->m_pFaces[i].v[v]].y;
      cf->m_Vecs[v].z=pIndexedMesh->m_pVerts[pIndexedMesh->m_pFaces[i].v[v]].z;			
    } //v					

    //calc plane equation
    cf->m_Plane.CalcPlane(cf->m_Vecs[2],cf->m_Vecs[1],cf->m_Vecs[0]);			
  } //i

  //precalc edges
  m_pShadowVolObject->CreateConnectivityInfo(pIndexedMesh, pLog);					
}

CStatCGFShadVol::~CStatCGFShadVol()
{
  delete m_pShadowVolObject;
}

#define FLAG_SKIP_SHADOWVOLUME 1 // todo: share this flag

void CShadowVolObject::CreateConnectivityInfo( CIndexedMesh * pIndexedMesh, ILog * pLog )
{
	//list of faces is shared from statobj
  m_pFaceList = pIndexedMesh->m_pFaces;
	m_nNumFaces = pIndexedMesh->m_nFaceCount;
	Vec3d *pVert = pIndexedMesh->m_pVerts;
 
  IEdgeConnectivityBuilder * iBuilder = new CStencilShadowStaticConnectivityBuilder();
  iBuilder->Reinit();

	assert(iBuilder);

	iBuilder->ReserveForTriangles(m_nNumFaces,pIndexedMesh->m_nVertCount);

	for(int i=0;i<m_nNumFaces;i++)
	{
		CObjFace *cf = &m_pFaceList[i];

		if(cf->m_dwFlags & FLAG_SKIP_SHADOWVOLUME)
			continue;

    // with welding
		unsigned short a=cf->v[0],b=cf->v[1],c=cf->v[2];
		iBuilder->AddTriangleWelded(a,b,c,pVert[a],pVert[b],pVert[c]);
	}

	m_pEdgeConnectivity = iBuilder->ConstructConnectivity();

#ifdef _DEBUG
	if(m_pEdgeConnectivity)
	{
		DWORD dwVertCount,dwTriCount;

		m_pEdgeConnectivity->GetStats(dwVertCount,dwTriCount);

		pLog->Log(" StencilEdgeConnectivity Stats:");
		pLog->Log("  %d/%d Vertices %d/%d Faces",dwVertCount,pIndexedMesh->m_nVertCount,dwTriCount,m_nNumFaces);
	}
#endif

	delete iBuilder;
}

CShadowVolObject::~CShadowVolObject()
{
  if(m_pEdgeConnectivity)
  {
    m_pEdgeConnectivity->Release();
    m_pEdgeConnectivity=0;
  }
}

void CStatCGFShadVol::Serialize(int & nPos, void * pStream, bool bSave)
{
	byte* pTarget = pStream ? (byte*)pStream+nPos : NULL;
	IStencilShadowConnectivity* pConnectivity = m_pShadowVolObject->GetEdgeConnectivity();
	// NOTE: passing a big number is not a good practice here, because in debug mode
	// it validates the buffers size and can detect buffer overruns early and painlessly.
	// a good practice is passing the actual number of bytes available in ths target buffer

  nPos += pConnectivity->Serialize(bSave, pTarget, 100000000);
}