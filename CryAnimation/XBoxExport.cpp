#include "stdafx.h"

#include "CryGeometryInfo.h"
//#include "CryAnimation.h"

/*
#include "CryCharManager.h"
#include "CryCharInstance.h"
#include "StringUtils.h"
#include "CryCharBody.h"
#include "CryModelState.h"
#include "CryModelLoader.h"
#include "I3DEngine.h"

#include "CryModel.h"
#include "CryGeomMorphTarget.h"
#include "CryCharBody.h"
#include "CryModelState.h"
#include "ChunkFileReader.h"
#include "CryModelGeometryLoader.h"
#include "CryBoneHierarchyLoader.h"
#include "StringUtils.h"
#include <StlUtils.h>
#include "CVars.h"

*/


//-------------------------------------------------------------

void exportTestModel(CryGeometryInfo* pGeometry, CLeafBuffer* pLeafBuffer)
{

#if 1

	FILE* stream = fopen( "c:\\output.txt", "w+b" );

	//---------------------------------------------------------------------------
	//---	 write vertices
	//---------------------------------------------------------------------------

	unsigned long numVertices			=	pGeometry->numExtTangents();

	unsigned long numExtTangents	=	pGeometry->numExtTangents();
	TangData* pExtTangents				=	pGeometry->getExtTangents();

	unsigned long numExtUVs				=	pGeometry->numExtUVs();
	CryUV* pExtUVs								=	pGeometry->getExtUVs();

	for (unsigned long x=0; x<numVertices; x++) {

		unsigned nIntVert = pGeometry->getExtToIntMapEntry(x);
		Vec3* pVertex		= &pGeometry->getVertex(nIntVert);

		fprintf(stream, "%20.12ff,%20.12ff,%20.12ff,   %15.12ff,%15.12ff,%15.12ff,    %15.12ff,%15.12ff,%15.12ff,    %15.12ff,%15.12ff,%15.12ff,        %15.12ff,%15.12ff, //%04x \r\n",  
				pVertex->x,									pVertex->y,										pVertex->z,      
				pExtTangents[x].tangent.x,	pExtTangents[x].tangent.y,		pExtTangents[x].tangent.z,      
				pExtTangents[x].binormal.x,	pExtTangents[x].binormal.y,		pExtTangents[x].binormal.z,     
				pExtTangents[x].tnormal.x,	pExtTangents[x].tnormal.y,		pExtTangents[x].tnormal.z,    
				pExtUVs[x].u,								pExtUVs[x].v, 
				x
			);

	}

	//---------------------------------------------------------------------------
	//---	 write indices for Leafbuffers
	//---------------------------------------------------------------------------
	//unsigned int numIndices				= pLeafBuffer->GetIndices().Count();
	unsigned short *pIndices			= pLeafBuffer->GetIndices(NULL);

	int mcount=pLeafBuffer->m_pMats->Count();

	//loop over all meterials
	for (int i=0; i<mcount; i++)	{	

		CMatInfo *mi		= pLeafBuffer->m_pMats->Get(i);
    int FirstIndex	=	mi->nFirstIndexId;
		int nNumIndices	=	mi->nNumIndices;

		fprintf(stream, "//----------------------------------------------------------------------------\r\n"); 

		for(int i=0; i<nNumIndices; i=i+3 ) { 
				fprintf(stream, "0x%04x,0x%04x,0x%04x,  //0x%04x \r\n", 
					pIndices[FirstIndex+i+0], 
					pIndices[FirstIndex+i+1], 
					pIndices[FirstIndex+i+2],  
					i/3 
				);
		}
	}
	
	//------------------------------------------------------------------------------------------------------

	fprintf(stream, "SMaterial MaterialList[] = {\r\n"); 

	//loop over all meterials
	for (int i=0; i<mcount; i++)	{	

		CMatInfo *mi		= pLeafBuffer->m_pMats->Get(i);
		int FirstIndex	=	mi->nFirstIndexId;
		int nNumIndices	=	mi->nNumIndices;

		fprintf(stream, "{ 0x%04x,0x%04x,0x%04x },  //0x%04x \r\n", 
				FirstIndex, 
				nNumIndices, 
				0,  
				i 
			);

	}

	fprintf(stream, "}\r\n"); 

	//------------------------------------------------------------------------------------------------------



	fclose( stream );
#endif

}

