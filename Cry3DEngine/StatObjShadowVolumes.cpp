////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjshadow.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: shadow maps
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "meshidx.h"
#include <IEdgeConnectivityBuilder.h>									// IEdgeConnectivityBuilder
#include "StencilShadowConnectivity.h"
#include "IndoorVolumes.h"
#include "IndoorShadowVolumes.h"
#include "CRETriMeshShadow.h"
#include "3dengine.h"

ItShadowVolume * CStatObj::MakeConnectivityInfo(CIndexedMesh * pMesh, const Vec3d & vOrigin, CStatObj * pStatObj)
{
  //if it doestn exists yet, create one
  tShadowVolume * ptSvObj = new tShadowVolume;
  ptSvObj->pSvObj = new CShadowVolObject( );

  for (int i=0; i<pMesh->m_nFaceCount; i++)
  {
    CObjFace *cf=&pMesh->m_pFaces[i];		

    for (int v=0; v<3; v++)
    {			
      cf->m_Vecs[v].x=pMesh->m_pVerts[pMesh->m_pFaces[i].v[v]].x;
      cf->m_Vecs[v].y=pMesh->m_pVerts[pMesh->m_pFaces[i].v[v]].y;
      cf->m_Vecs[v].z=pMesh->m_pVerts[pMesh->m_pFaces[i].v[v]].z;			
    } //v					

    //calc plane equation
    cf->m_Plane.CalcPlane(cf->m_Vecs[2],cf->m_Vecs[1],cf->m_Vecs[0]);			
  } //i

  //set the same light area geometry
  tContainer tCont;
  tCont.pObj = pStatObj;
  tCont.pSV=NULL;
  //ptSvObj->pSvObj->AddGeometry(this);
  ptSvObj->pSvObj->AddGeometry(tCont);
  //mark the geometry as shared
  ptSvObj->pSvObj->m_dwFlags|=FLAG_GEOMETRY_SHARED; 
  //set the position of the shadow volumes
  ptSvObj->pSvObj->SetPos(/*pParams->*/vOrigin);
  //precalc edges
  if(!ptSvObj->pSvObj->CreateConnectivityInfo())
	{ // delete empty objects
		ptSvObj->Release();
		ptSvObj=NULL;
	}


  //assign to the istatobj
  return (ptSvObj);
}

ItShadowVolume * CStatObj::MakeConnectivityInfoFromCompiledData(void * pStream, int & nPos, CStatObj * pStatObj)
{
  //if it doestn exists yet, create one
  tShadowVolume * ptSvObj = new tShadowVolume;
  ptSvObj->pSvObj = new CShadowVolObject( );

  tContainer tCont;
  tCont.pObj = pStatObj;
  tCont.pSV=NULL;
  //ptSvObj->pSvObj->AddGeometry(this);
  ptSvObj->pSvObj->AddGeometry(tCont);
  //mark the geometry as shared
  ptSvObj->pSvObj->m_dwFlags|=FLAG_GEOMETRY_SHARED; 
  //set the position of the shadow volumes
  ptSvObj->pSvObj->SetPos(Vec3d(0,0,0));

  IEdgeConnectivityBuilder * iBuilder = Get3DEngine()->GetNewConnectivityBuilder();
  ptSvObj->pSvObj->GetEdgeConnectivity() = iBuilder->ConstructConnectivity();
  nPos += ptSvObj->pSvObj->GetEdgeConnectivity()->Serialize(false,&((byte*)pStream)[nPos],1000000, NULL);//GetLog());

  return (ptSvObj);
}

// NOTE: Current implementation doesn't take the limit LOD parameter into account
void CStatObj::RenderShadowVolumes(const SRendParams *pParams, int nLimitLod)
{
//  assert(pParams->lSource);

	CDLight tTempLight;
	tTempLight = *(pParams->pShadowVolumeLightSource);

	Matrix44 tInvRot;

	if(pParams->pMatrix)
	{		
		tInvRot=*pParams->pMatrix;
	} 
	else
	{	
		//tInvRot.Identity();
		//tInvRot=GetTranslationMat(pParams->vPos)*tInvRot; 
		//tInvRot=GetRotationZYX44(-pParams->vAngles*gf_DEGTORAD)*tInvRot; //NOTE: angles in radians and negated 

		//OPTIMISED_BY_IVO  
		tInvRot	=	Matrix34::CreateRotationXYZ(pParams->vAngles*gf_DEGTORAD, pParams->vPos );	//set scaling and translation in one function call
		tInvRot	=	GetTransposed44(tInvRot);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44

	}
	tInvRot.Invert44();

	Vec3d vPos=tInvRot.TransformPointOLD(tTempLight.m_Origin);
	tTempLight.m_vObjectSpacePos=vPos;

/*
	{	
		char str[256];

		snprintf(str,"'%s' %p (%.2f %.2f %.2f) (%.2f %.2f %.2f)\n",pParams->lSource->m_Name,this,
			tTempLight.m_Origin.x,tTempLight.m_Origin.y,tTempLight.m_Origin.z,
			vPos.x,vPos.y,vPos.z);

		OutputDebugString(str);
	}
*/
	
	//FIXME: move this code in the conn. info calculation
	//assign vertices and calc planes
	CIndexedMesh *pMesh = GetTriData();

	if (!GetShadowVolume())
  {
    if(pMesh)
		{
			SetShadowVolume(CStatObj::MakeConnectivityInfo(pMesh, Vec3d(0,0,0)/* pParams->vOrigin*/, this));
			FreeTriData(); // source geometry is needed only for stencil shadows
		}
    else
      return;
  }

	ItShadowVolume *ptSvObj=GetShadowVolume();	
	CShadowVolObject *pSvObj=ptSvObj->GetShadowVolume();

	IStencilShadowConnectivity* pConnectivity = pSvObj->GetEdgeConnectivity();
  if(!pParams->pShadowVolumeLightSource || !pConnectivity)
    return;

	if (/*!pSvObj->GetNumFaces() && */!pConnectivity->IsStandalone())
		return;

	if(!m_bOpenEdgesTested && pConnectivity->numOrphanEdges())
	{ 
#if !defined(LINUX)
		Warning(0,m_szFileName,"%d open edges found during shadow volume calculations for %s %s", 
			pConnectivity->numOrphanEdges(), m_szFileName, m_szGeomName	);
#endif
		m_bOpenEdgesTested = true;
	}

	//GetRenderer()->Draw3dBBox(Vec3d(vPos.x-0.5f,vPos.y-0.5f,vPos.z-0.5f),Vec3d(vPos.x+0.5f,vPos.y+0.5f,vPos.z+0.5f));
	
	////////////////////////////////////////////////////////////////////////
	//this will be done in RE right before rendering
	//pSvObj->CalculateDynamicShadowVolume(tTempLight);
	//pSvObj->CreateDynamicShadowVolumeBuffer();				

  if(!pSvObj->m_pReMeshShadow)
    pSvObj->m_pReMeshShadow = (CRETriMeshShadow *)GetRenderer()->EF_CreateRE(eDATA_TriMeshShadow);

	// draw shadow volumes						
	if(pSvObj->m_pReMeshShadow)
	{	
		// get the object
		CCObject *pObj;
		pObj = GetRenderer()->EF_GetObject(true);
	  pObj->m_DynLMMask = pParams->nDLightMask;
/*		if(!m_nRenderStackLevel)
		{
			pObj->m_nScissorX1 = pParams->nScissorX1;
			pObj->m_nScissorY1 = pParams->nScissorY1;
			pObj->m_nScissorX2 = pParams->nScissorX2;
			pObj->m_nScissorY2 = pParams->nScissorY2;
		}*/

		// the translation is in the object, otherwise we get z-fighting
		// with the shaders
    // hh unused		pSvObj->m_pReMeshShadow->m_vOrigin = Vec3d(0,0,0); 

		if (pParams->pMatrix)
			pObj->m_Matrix = *pParams->pMatrix;
		else
      mathCalcMatrix(pObj->m_Matrix, pParams->vPos, pParams->vAngles, Vec3d(1,1,1), Cry3DEngineBase::m_CpuFlags);
    pObj->m_ObjFlags |= FOB_TRANS_MASK;

		//assign shadow volume resources
		//allow  the calculation to be done right before rendering
		pSvObj->m_pReMeshShadow->m_pSvObj = ptSvObj;
    pSvObj->m_pReMeshShadow->m_nCurrInst = -1;
    pObj->m_CustomData = pParams->pCaller;
    pObj->m_TempVars[0] = pParams->fShadowVolumeExtent;

		IShader * pEff = ((C3DEngine*)Get3DEngine())->m_pSHStencil;
		GetRenderer()->EF_AddEf(0, (CRendElement *)pSvObj->m_pReMeshShadow , pEff, NULL, pObj, -1, NULL, pParams->nSortValue);							
	}
}

CVolume::~CVolume()
{
	for (ContainerListIt i=m_lstObjects.begin();i!=m_lstObjects.end();)
	{
		RemoveGeometry(NULL,i);
		i=m_lstObjects.begin();
	} //i

	m_lstObjects.clear(); //for the sake of clarity
}	

bool CShadowVolObject::CreateConnectivityInfo( void )
{
  //IStatObj *ob=m_lstStatObjs[0];							assert(ob);
	IStatObj *ob=m_lstObjects[0].pObj;
	assert(ob);
	CIndexedMesh *pMesh=ob->GetTriData();

	if (!pMesh)
		return false;				// to prevent crash, this could be if the file versions are changing)

	//list of faces is shared from statobj
  CObjFace * pFaceList = pMesh->m_pFaces;
	int nNumFaces=pMesh->m_nFaceCount;
	Vec3d *pVert=pMesh->m_pVerts;
 
	IEdgeConnectivityBuilder * iBuilder = Get3DEngine()->GetNewStaticConnectivityBuilder();

	assert(iBuilder);

	iBuilder->ReserveForTriangles(nNumFaces,pMesh->m_nVertCount);

	for(int i=0;i<nNumFaces;i++)
	{
		CObjFace *cf = &pFaceList[i];

		if(cf->m_dwFlags & FLAG_SKIP_SHADOWVOLUME)
			continue;

    // with welding
		unsigned short a=cf->v[0],b=cf->v[1],c=cf->v[2];
		iBuilder->AddTriangleWelded(a,b,c,pVert[a],pVert[b],pVert[c]);
	}

	m_pEdgeConnectivity=iBuilder->ConstructConnectivity();

	DWORD dwVertCount=0,dwTriCount=0;
	if(m_pEdgeConnectivity)
	{
		m_pEdgeConnectivity->GetStats(dwVertCount,dwTriCount);
/*
#ifdef _DEBUG
		char str[256];
		snprintf(str, sizeof(str), "StencilEdgeConnectivity Indoor Stats %p: %d/%d Vertices %d/%d Faces\n",m_pEdgeConnectivity,dwVertCount,pMesh->m_nVertCount,dwTriCount,nNumFaces);
		OutputDebugString(str);
#endif*/
	}

	return dwVertCount && dwTriCount;
}

void CVolume::SetPos(const Vec3d &vPos)
{
	m_vOrigin=vPos;

	pe_params_pos par_pos;
	par_pos.pos=vPos;

	//for (iphysobjit i=m_lstPhysObjs.begin();i!=m_lstPhysObjs.end();i++)
	for (ContainerListIt i=m_lstObjects.begin();i!=m_lstObjects.end();i++)
	{
		IPhysicalEntity *pEnt=(*i).pEnt;
		if (pEnt)
			pEnt->SetParams(&par_pos);
	} //i
}

void CVolume::AddGeometry(tContainer tCont)
{ 		
	m_vMins.CheckMin(tCont.pObj->GetBoxMin());
	m_vMaxs.CheckMax(tCont.pObj->GetBoxMax());

	/*
	m_lstStatObjs.push_back(pSource);	

	if (pEnt)
		m_lstPhysObjs.push_back(pEnt);
	*/

	m_lstObjects.push_back(tCont);

	m_nObjCount++;
}


CShadowVolObject::~CShadowVolObject()
{

	/* //do not delete 'cause the source is a cstatobj
	if (m_pFaceList)
	{
		delete [] m_pFaceList;
		m_pFaceList=NULL;		
	}
	*/

	if(m_pEdgeConnectivity)
	{
		m_pEdgeConnectivity->Release();
		m_pEdgeConnectivity=0;
	}

	FreeVertexBuffers();
}

void CShadowVolObject::FreeVertexBuffers()
{
	if (m_pSystemVertexBuffer)
	{
    delete [] m_pSystemVertexBuffer;
		m_pSystemVertexBuffer=0;
	}

	if (m_pReMeshShadow)
	{						
		m_pReMeshShadow->Release();
		m_pReMeshShadow=NULL;
	}

	/*if (m_pReMeshAdditionalShadow)
	{		
		//FIXME:Move deallocation into renderelements
		if (m_pReMeshAdditionalShadow->m_pShadowVolEdgesList)
		{
			//those edges are shared!
			//delete [] m_pReMeshAdditionalShadow->m_pShadowVolEdgesList;
			m_pReMeshAdditionalShadow->m_pShadowVolEdgesList=NULL;
		}

		m_pReMeshAdditionalShadow->Release();	
		m_pReMeshAdditionalShadow=NULL;
	}*/

	m_nNumVertices=0;
}

void CShadowVolObject::CheckUnload()
{
  if(m_pReMeshShadow)
    m_pReMeshShadow->mfCheckUnload();
}

void CShadowVolObject::RebuildShadowVolumeBuffer( const CDLight &lSource, float fReadyShadowVolumeExtent )		// lSource has to be object relative
{

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4311 )
#endif

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

	if(!m_pEdgeConnectivity || (/*!m_nNumFaces && */!m_pEdgeConnectivity->IsStandalone()))
	{
//		PrepareShadowVolumeVertexBuffer(0,0);
		return;
	}

	//! M.M. for debugging stencil shadows (0 is default, use !=0 to force reacalculation of indoor stencil shadow volumes)

	IEdgeDetector * iEdgeDetector = Get3DEngine()->GetEdgeDetector();

	Vec3d vObjectSpaceLight=lSource.m_vObjectSpacePos;
	Vec3d vWorldSpaceLight=lSource.m_Origin;

	// baustelle
	IStatObj *ob=m_lstObjects[0].pObj;
	CIndexedMesh *pMesh=ob->GetTriData();

	assert(m_pEdgeConnectivity->IsStandalone());

	if (m_pEdgeConnectivity->IsStandalone())
	{
		iEdgeDetector->BuildSilhuetteFromPos(m_pEdgeConnectivity, vObjectSpaceLight, NULL);
	}
	/*else
	{
		unsigned *pTriOriBitfield=iEdgeDetector->getOrientationBitfield(m_nNumFaces);

		for(int nFace = 0; nFace < m_nNumFaces;)
		{	
			unsigned nOrientBit = 1;

			do
			{
				CObjFace *cf=&m_pFaceList[nFace];

				if (cf->m_dwFlags & FLAG_SKIP_SHADOWVOLUME)//! transparent surfaces don't cast shadows
				{ 
					nFace++; 
					continue; 
				}

				float dist1=cf->m_Plane.DistFromPlane(vObjectSpaceLight);

				if(dist1>0)
					*pTriOriBitfield |= nOrientBit;
				nOrientBit <<= 1;
			}
			while (((++nFace) < m_nNumFaces) && (nOrientBit) != 0);

			++pTriOriBitfield;
		}
		if(!pMesh)
			return;				// to prevent crash, this could be if the file versions are changing
	
		iEdgeDetector->BuildSilhuetteFromBitfield(m_pEdgeConnectivity,pMesh->m_pVerts);
	}*/

	unsigned nNumIndices = iEdgeDetector->numShadowVolumeIndices();
	unsigned nNumVertices = iEdgeDetector->numShadowVolumeVertices();

//	float fShadowVolumeExtent=20.0f;



	/*
	// [5/12/2002] Marco's NOTE: removed this part, because to allow shadows to cross
	// portals, we should not check for the object being inside/outside the 
	// area / so the check is now unified like for outdoors

	// extend as little as possible to save fill rate M.M. *********************************************************************** Start
	// I have to be in indoor otherwise shadow volumes doesn't make sense
	// to prevent crash, but this should never be - without this info I assume a big value would do it
	if(inpArea)
	{
		Vec3d vMinObjBBox=GetBBoxMin(true),vMaxObjBBox=GetBBoxMax(true);

		// debugging
//		GetRenderer()->Draw3dBBox(vMinObjBBox,vMaxObjBBox);

		// if the lightsource is inside the object, use big value 
		// - it's dangerous to extrude too much (althougth extrude to infinity is theoretical possible)

		// calculate the minimum extrusion level (to get the object bounding box extruded outside of the room)
		fShadowVolumeExtent = inpArea->CalculateMinimumShadowExtrusion(vWorldSpaceLight,vMinObjBBox,vMaxObjBBox);
		
//		assert(fShadowVolumeExtent>=1.0f); 
	}
	else
	*/	
	{ 
		// calculate optimal extent for outdoor entity
		// todo take into account object scale
		//Vec3d vMinObjBBox=GetBBoxMin(true),vMaxObjBBox=GetBBoxMax(true);
/*		Vec3d vMinObjBBox=GetBBoxMin(),vMaxObjBBox=GetBBoxMax();

		Vec3d vObjCenter = (vMinObjBBox+vMaxObjBBox)*0.5;
		float fObjRadius = (vMaxObjBBox-vMinObjBBox).Length()*0.5f;
		float fObjLightDist = GetDistance(vObjectSpaceLight, vObjCenter) - fObjRadius;

		if(fObjLightDist<0.01f)
			fObjLightDist=0.01f;		

		fShadowVolumeExtent = lSource.m_fRadius/fObjLightDist;

		if(fShadowVolumeExtent<1.f)
			fShadowVolumeExtent=1.f;
		else if(fShadowVolumeExtent>20.f)
			fShadowVolumeExtent=20.f;*/
	}
	// extend as little as possible to save fill rate M.M. *********************************************************************** End

	PrepareShadowVolumeVertexBuffer(nNumIndices, nNumVertices);

	if (nNumVertices > 1 && nNumIndices > 1)
	{
//    m_arrIndices.reinit(n)
		iEdgeDetector->meshShadowVolume (vObjectSpaceLight, fReadyShadowVolumeExtent, (Vec3d *)m_pSystemVertexBuffer, &m_arrIndices[0]);
  
    CRETriMeshShadow::ShadVolInstanceInfo * pSVI = &m_pReMeshShadow->m_arrLBuffers[m_pReMeshShadow->m_nCurrInst];

		if(	pSVI->pVB && pSVI->pVB->m_SecVertCount == (int)m_nNumVertices && 
			pSVI->pVB->m_Indices.m_nItems == (int)nNumIndices )
		{
			pSVI->pVB->UpdateSysIndices(nNumIndices ? &m_arrIndices[0] : 0, nNumIndices);
			pSVI->pVB->UpdateSysVertices(m_pSystemVertexBuffer,m_nNumVertices);
		}
		else
		{
			if(pSVI->pVB)
				GetRenderer()->DeleteLeafBuffer(pSVI->pVB);

			// hack: todo: make m_arrIndices list2
			pSVI->pVB = GetRenderer()->CreateLeafBufferInitialized(
				m_pSystemVertexBuffer, m_nNumVertices, VERTEX_FORMAT_P3F,
				&m_arrIndices[0], nNumIndices, R_PRIMV_TRIANGLES, "ShadowVolume", eBT_Dynamic, 1 , 0, 0, this);
			assert(m_nNumVertices && nNumIndices);
			pSVI->pVB->SetChunk(0,0,m_nNumVertices,0,nNumIndices);
			assert((*(pSVI->pVB)).m_pMats->Count());
		}

/*
    if(!pSVI->pVB)
      pSVI->pVB = GetRenderer()->CreateBuffer(m_nNumVertices, VERTEX_FORMAT_P3F, 
      &pSVI->plstIndices->GetAt(0), pSVI->plstIndices->Count(), "InstanceShadowVolume");

    GetRenderer()->UpdateBuffer(pSVI->pVB, m_pSystemVertexBuffer, m_nNumVertices,true);		
    GetRenderer()->UpdateIndices(pSVI->pVB, &m_arrIndices[0], m_arrIndices.size());		
  */
/*
#ifdef _DEBUG
		{
			for (unsigned i = 0; i < nNumIndices; ++i)
				assert(m_arrIndices[i] < nNumVertices);
		}
#endif
*/
		/*
		m_pReMeshShadow->m_vOrigin=rParams->vPos;	
		m_pReMeshShadow->m_vAngles=rParams->vAngles;	


		//get the effect object from the renderer
		CCObject *pObj = renderer->EF_GetObject(true);

		pObj->m_Trans = rParams->vPos;
		pObj->m_Angs  = rParams->vAngles;
		pObj->m_Scale = 1.0f;

		m_pReMeshShadow->m_nNumIndices = nNumIndices;	
    m_pReMeshShadow->mfCheckUpdate();
		//renderer->UpdateBuffer (m_pRenShadowVolumeBuffer, m_pMemShadowVolumeBuffer, nNumVertices, true, 0);
  	renderer->UpdateBuffer(m_pRenShadowVolumeLBuffer->m_pVertexBuffer,m_pMemShadowVolumeBuffer,nNumVertices,true);		
	  renderer->UpdateIndices(m_pRenShadowVolumeLBuffer->m_pVertexBuffer,&m_arrIndices[0],nNumIndices);		
		renderer->EF_AddEf(0, (CRendElement *)m_pReMeshShadow , rParams->pEff, pObj, -1, NULL, rParams->nSortValue);
*/

//		m_pReMeshShadow->m_nNumIndices = nNumIndices;	
//    m_pReMeshShadow->mfCheckUpdate(0);			// call before update, to make sure video buffer is there

//		assert(m_pRenShadowVolumeLBuffer);
//		assert(m_pRenShadowVolumeLBuffer->m_pVertexBuffer);

//		GetRenderer()->UpdateBuffer(m_pRenShadowVolumeLBuffer->m_pVertexBuffer,pSystemBuffer,nNumVertices,true);		
	//	GetRenderer()->UpdateIndices(m_pRenShadowVolumeLBuffer->m_pVertexBuffer,&m_arrIndices[0],nNumIndices);		
//    m_pRenShadowVolumeLBuffer->m_UpdateVBufferMask &= ~1;
	}
  else
  {
    int i=0;
  }
}

void CVolume::RemoveGeometry(IStatObj *pSource,ContainerListIt it/* =NULL */)
{
	ContainerListIt i = m_lstObjects.end();
	if (it!=m_lstObjects.end())
		i=it;
	else
	{
		for (ContainerListIt it2=m_lstObjects.begin();it2!=m_lstObjects.end();it2++)
		{
			tContainer tempCont=(*it2);
			if (tempCont.pObj==pSource)
			{
				i=it2;
				break;
			}
		} //it2
	}

	if (i!=m_lstObjects.end())
	{
		tContainer tempCont=(*i);

		IStatObj *pSource=tempCont.pObj;
		if (pSource && (!(m_dwFlags & FLAG_GEOMETRY_SHARED)))
			Get3DEngine()->ReleaseObject(pSource);

		IPhysicalEntity *pEnt=(tempCont.pEnt);
		if (pEnt)
			GetPhysicalWorld()->DestroyPhysicalEntity(pEnt);

		m_lstObjects.erase(i);
	}
}

// Prepare the resources for rendering shadow volumes
// This include:
//   index and vertex in-memory arrays
//   render object (m_pReMeshShadow, m_pRenShadowVolumeBuffer, whatever support they need)
void CShadowVolObject::PrepareShadowVolumeVertexBuffer( unsigned nNumIndices, unsigned nNumVertices )
{
	if (!nNumIndices || !nNumVertices)
		return;

	bool bRecreate = false;

	// Realloc index in-mem buffer
	if (m_arrIndices.size() < nNumIndices)
	{
		m_arrIndices.reinit(nNumIndices);
		bRecreate = true;
	}

	// Realloc vertex in-mem buffer
	if (m_nNumVertices < nNumVertices)
	{
		m_nNumVertices = nNumVertices;
		bRecreate = true;
	}

	if (bRecreate)
	{
		if (m_pSystemVertexBuffer)
		{
			delete [] m_pSystemVertexBuffer;
			m_pSystemVertexBuffer=NULL;
		}
	}

	assert(m_pReMeshShadow); // should be called from inside this object
//		m_pReMeshShadow = (CRETriMeshShadow *)GetRenderer()->EF_CreateRE(eDATA_TriMeshShadow);

	if(!m_pSystemVertexBuffer)
	{
    assert(m_nNumVertices);
		m_pSystemVertexBuffer = new Vec3d[m_nNumVertices];
		assert(m_pSystemVertexBuffer);
	}
}
