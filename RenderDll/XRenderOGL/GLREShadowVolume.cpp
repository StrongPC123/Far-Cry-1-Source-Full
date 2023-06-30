
//////////////////////////////////////////////////////////////////////
//
//	Crytek RenderDLL source code
//	
//	File: GLREShadowVolume.cpp
//
//	History:
//	-October 18,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#include "RenderPCH.h"
#include "GL_Renderer.h"
#include "I3dengine.h"											// USE_SHADOW_VERSION


///////////////////////////////////////////////////////////////////
//TODO: Move into renderer header
#include "..\..\Cry3DEngine\ShadowVolumeEdge.h"

///////////////////////////////////////////////////////////////////
bool CRETriMeshShadow::mfDraw(SShader *ef, SShaderPass *sfm)
{ 
  if(m_nCurrInst<0 || m_nCurrInst>=MAX_SV_INSTANCES)
  {
    iLog->Log("Warning: CRETriMeshShadow::mfDraw: m_nCurrInst not set");
    return false;
  }

//	assert(m_arrLBuffers[m_nCurrInst].pVB);

  CGLRenderer *rd = gcpOGL;
  CDLight *pDL = NULL;
  if (rd->m_RP.m_DynLMask)
  {
    for (int n=0; n<rd->m_RP.m_DLights[SRendItem::m_RecurseLevel].Num(); n++)
    {
      if (rd->m_RP.m_DynLMask & (1<<n))
      {
        pDL = rd->m_RP.m_DLights[SRendItem::m_RecurseLevel][n];
        break;
      }
    }
  }
  if (pDL && pDL->m_sWidth && pDL->m_sHeight)
    rd->EF_Scissor(true, pDL->m_sX, pDL->m_sY, pDL->m_sWidth, pDL->m_sHeight);

	//disable blending, alpha testing, 
	//z-writing - use the z-fail stencil method
  CGLTexMan::BindNULL(0);
  rd->EF_SetColorOp(eCO_DISABLE, eCO_DISABLE, DEF_TEXARG0, DEF_TEXARG0);

	if (rd->CV_ind_VisualizeShadowVolumes)
	{
		glColor4f(1,1,1,1);
    rd->EF_SetState(GS_NODEPTHTEST | GS_STENCIL);
	}
	else
    rd->EF_SetState(GS_NOCOLMASK | GS_STENCIL);

 	if(m_arrLBuffers[m_nCurrInst].pVB && m_arrLBuffers[m_nCurrInst].pVB->m_pVertexBuffer)
	{     
    if (SUPPORTS_GL_ATI_separate_stencil || SUPPORTS_GL_EXT_stencil_two_side)
    {
      // using 2-sided stencil
      rd->EF_SetStencilState(STENCOP_FAIL(FSS_STENCOP_KEEP) |
                             STENCOP_ZFAIL(FSS_STENCOP_DECR_WRAP) |
                             STENCOP_PASS(FSS_STENCOP_KEEP) |
                             STENC_FUNC(FSS_STENCFUNC_ALWAYS) | 
                             STENCOP_CCW_FAIL(FSS_STENCOP_KEEP) |
                             STENCOP_CCW_ZFAIL(FSS_STENCOP_INCR_WRAP) |
                             STENCOP_CCW_PASS(FSS_STENCOP_KEEP) |
                             STENC_CCW_FUNC(FSS_STENCFUNC_ALWAYS) |
                             FSS_STENCIL_TWOSIDED,
                             0, -1);

      rd->GLSetCull(eCULL_None);

      CVertexBuffer * pVB = m_arrLBuffers[m_nCurrInst].pVB->m_pVertexBuffer;
      SVertexStream *Indices = &m_arrLBuffers[m_nCurrInst].pVB->m_Indices;
      int nIndices;
      if (m_nRendIndices)
        nIndices = m_nRendIndices;
      else
        nIndices = Indices->m_nItems;

      assert(nIndices);

		  rd->DrawBuffer(pVB, Indices, nIndices, 0, R_PRIMV_TRIANGLES);
                              
		  rd->SetFenceCompleted(pVB); // set it explicitly since second DrawBuffer do not set it
                
		  gRenDev->m_nShadowVolumePolys += (nIndices/3);
    }
    else
    {
      rd->EF_SetStencilState(STENCOP_FAIL(FSS_STENCOP_KEEP) |
                              STENCOP_ZFAIL(FSS_STENCOP_INCR) |
                              STENCOP_PASS(FSS_STENCOP_KEEP) |
                              STENC_FUNC(FSS_STENCFUNC_ALWAYS),
                              0, -1);

      rd->GLSetCull(eCULL_Front);

      CVertexBuffer * pVB = m_arrLBuffers[m_nCurrInst].pVB->m_pVertexBuffer;
      SVertexStream *Indices = &m_arrLBuffers[m_nCurrInst].pVB->m_Indices;
      int nIndices;
      if (m_nRendIndices)
        nIndices = m_nRendIndices;
      else
        nIndices = Indices->m_nItems;

      assert(nIndices);

		  rd->DrawBuffer(pVB, Indices, nIndices, 0, R_PRIMV_TRIANGLES);
              
      rd->EF_SetStencilState(STENCOP_FAIL(FSS_STENCOP_KEEP) |
                              STENCOP_ZFAIL(FSS_STENCOP_DECR) |
                              STENCOP_PASS(FSS_STENCOP_KEEP) |
                              STENC_FUNC(FSS_STENCFUNC_ALWAYS),
                              0, -1);
      rd->GLSetCull(eCULL_Back);
  		
		  rd->DrawBuffer(pVB, Indices, nIndices, 0, R_PRIMV_TRIANGLES);
                
		  rd->SetFenceCompleted(pVB); // set it explicitly since second DrawBuffer do not set it
                
		  gRenDev->m_nShadowVolumePolys += (nIndices*2/3);
    }
	}

  m_nCurrInst = -1;

  return (true);
}


/*
//TODO: BackFace culling
//TODO: put into vertex buffer
///////////////////////////////////////////////////////////////////
bool CRETriMeshAdditionalShadow::mfDraw(SShader *ef, SShaderPass *sfm)
{	
	int k;

	/*
	//cull front-facing
	glCullFace(GL_FRONT);	
	
	//first draw all back-facing and set z-buffer
	glColorMask(0,0,0,0);
	glDisable(GL_TEXTURE_2D);
	glDepthFunc(GL_LEQUAL);

	//glDisable(GL_CULL_FACE);

	glBegin(GL_TRIANGLES);
	for (k=0;k<m_nNumFaces;k++)
	{
		CObjFace *pFace=&m_pFacesList[k];
		if (!pFace->m_bLit)
		{
			glVertex3fv(&pFace->m_Vecs[0].x);
			glVertex3fv(&pFace->m_Vecs[1].x);
			glVertex3fv(&pFace->m_Vecs[2].x);
		}
	} //k	
	glEnd();	
	

	glColorMask(1,1,1,1);
	glDepthFunc(GL_EQUAL);
	//glDepthFunc(GL_LEQUAL);
	//glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO,GL_SRC_COLOR);
	//glBlendFunc(GL_DST_COLOR,GL_ZERO);
	
	//if (m_pTexture)
	if (0)
	{
		gRenDev->SetTexture(m_pTexture->m_Bind);
		glEnable(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);		
	}		

	CShadowVolEdge *pEdge;	

	glBegin(GL_TRIANGLES);
	for (k=0;k<m_nNumFaces;k++)
	{
		CObjFace *pFace=&m_pFacesList[k];
		if (pFace->m_bLit || (pFace->m_dwFlags&FLAG_BLACK_FACE))
			continue;
		
		for (int i=0;i<3;i++)
		{
			//glTexCoord2f(pFace->m_fCoords[i][0],pFace->m_fCoords[i][1]);				
			pEdge=pFace->m_pEdges[i];
			glColor3fv(&pFace->m_vColors[i].r);
			glVertex3fv(&pFace->m_Vecs[i].x);
		} //i		
	} //k	
	glEnd();
	
	//return to previous state
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_2D);
	glColorMask(1,1,1,1);
*//*	

	glDepthMask(0);

	glDisable(GL_TEXTURE_2D);
	//glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);	

	//glEnable(GL_LINE_SMOOTH);
	//glLineWidth(8.0f);

	//glPushMatrix();
	//glTranslatef(m_vOrigin.x,m_vOrigin.y,m_vOrigin.z);

	//glDisable(GL_BLEND);
	glEnable(GL_BLEND);		
	//glBlendFunc(GL_ZERO,GL_SRC_COLOR);
	glBlendFunc(GL_DST_COLOR,GL_ZERO);

	float fDarkness=0.0f;

	//glPolygonOffset(1.1f,4.0f);
	//glEnable(GL_POLYGON_OFFSET_FILL);

	//glDisable(GL_DEPTH_TEST);

	glBegin(GL_QUADS);			
	for (k=0;k<m_nNumEdges;k++)
	{		
		CShadowVolEdge *pEdge=&m_pShadowVolEdgesList[k];		
		if ((!(pEdge->m_pAddEdge->m_dwFlags & FLAG_FALSE_EDGE_SILOHUETTE)) && (pEdge->m_pFace1->m_bLit!=pEdge->m_pFace2->m_bLit))
		{
			//glBegin(GL_LINE_LOOP);		
			glColor3f(fDarkness,fDarkness,fDarkness);		
			glVertex3fv(&pEdge->m_vecV0.x);							
			glColor3f(1,1,1);			
			glVertex3fv(&pEdge->m_pAddEdge->m_vExtrOut[0].x);			
			glColor3f(1,1,1);			
			glVertex3fv(&pEdge->m_pAddEdge->m_vExtrOut[1].x);			
			glColor3f(fDarkness,fDarkness,fDarkness);			
			glVertex3fv(&pEdge->m_vecV1.x);			
      gRenDev->m_nPolygons += 2;
			//glEnd();
		}
	}
	glEnd();	
	
	if (gRenDev->CV_ind_DrawBorderEdges)
	{
		//glDisable(GL_DEPTH_TEST);
		//glPolygonOffset(1.1f,4.0f);
		//glEnable(GL_POLYGON_OFFSET_FILL);
		glDepthFunc(GL_LEQUAL);

		glColor3f(fDarkness,fDarkness,fDarkness);
		//glColor3f(1,0,0);
		glBegin(GL_QUADS);			
		for (k=0;k<m_nNumEdges;k++)
		{		
			CShadowVolEdge *pEdge=&m_pShadowVolEdgesList[k];		
			if ((!(pEdge->m_pAddEdge->m_dwFlags & FLAG_FALSE_EDGE_SILOHUETTE)) && (pEdge->m_pFace1->m_bLit!=pEdge->m_pFace2->m_bLit))
			{				
				glColor3f(fDarkness,fDarkness,fDarkness);
				glVertex3fv(&pEdge->m_vecV0.x);							
				glColor3f(1,1,1);
				glVertex3fv(&pEdge->m_pAddEdge->m_vExtrIn[0].x);			
				glColor3f(1,1,1);
				glVertex3fv(&pEdge->m_pAddEdge->m_vExtrIn[1].x);			
				glColor3f(fDarkness,fDarkness,fDarkness);
				glVertex3fv(&pEdge->m_vecV1.x);							
        gRenDev->m_nPolygons += 2;
			}
		}
		glEnd();
	}	
	//glPopMatrix();	

	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glDepthMask(1);
		
	return (true);
}
*/