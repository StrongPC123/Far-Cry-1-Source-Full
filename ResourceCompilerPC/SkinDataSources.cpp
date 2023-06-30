#include "stdafx.h"
#include "RenderMeshBuilder.h"
#include "SkinDataSources.h"
#include "CryBoneDesc.h"

// helper to get order for Vec3d
struct CVec3dOrder: public std::binary_function< Vec3d, Vec3d, bool>
{
	bool operator() ( const Vec3d &a, const Vec3d &b ) const
	{
		// first sort by x
		if(a.x<b.x)return(true);
		if(a.x>b.x)return(false);

		// then by y
		if(a.y<b.y)return(true);
		if(a.y>b.y)return(false);

		// then by z
		if(a.z<b.z)return(true);
		if(a.z>b.z)return(false);

		return(false);
	}
};

typedef std::map<Vec3d,unsigned,CVec3dOrder> VertexWelderMap;


CRCSkinVertexSource::CRCSkinVertexSource (const CRenderMeshBuilder& rRendMesh, const CryChunkedFile::MeshDesc& rMeshDesc, Matrix44* pBones):
	ICrySkinSource(
		&rMeshDesc.arrVertBinds[0],
		rMeshDesc.arrVertBinds.size(),
		NULL, // m_pVertices
		0, // m_numVertices
		&rRendMesh.m_arrExtTangents[0],
		rRendMesh.m_arrExtTangents.size(),
		NULL // m_pExtToIntMapping
	)
{
	// to get the vertices, we'll have to build the temp buffer holding unique vertices
	// this maps the vertex coordinates to the vertex index in the new buffer
	//VertexWelderMap mapVertices;

	m_arrVerts.resize (rMeshDesc.numVertices());
	m_pVertices = &m_arrVerts[0];
	m_numVertices = m_arrVerts.size();

	// skin the 
	for (unsigned nVertex = 0; nVertex < rMeshDesc.numVertices(); ++nVertex)
	{
		Vec3d & p = m_arrVerts[nVertex];
		const CryVertexBinding& arrLinks = rMeshDesc.arrVertBinds[nVertex];

		if (pBones && !arrLinks.empty())
		{
			p.x = p.y = p.z = 0;

			for (CryVertexBinding::const_iterator itLink = arrLinks.begin(); itLink != arrLinks.end(); ++itLink)
			{
				Matrix44& matBoneGlobal = pBones[itLink->BoneID];
				p += matBoneGlobal.TransformPointOLD(itLink->offset) * itLink->Blending;
			}
		}
		else
			p = rMeshDesc.pVertices[nVertex].p;
	}

	m_arrExtToIntMap.resize (rRendMesh.m_arrExtTangMap.size());
	unsigned i;
	for (i = 0; i < rRendMesh.m_arrExtTangMap.size(); ++i)
		m_arrExtToIntMap[i] = rRendMesh.m_arrExtTangMap[i];
	m_pExtToIntMapping = &m_arrExtToIntMap[0];
}

CRCSkinVertexSource::~CRCSkinVertexSource ()
{

}


CRCSkinNormalSource::CRCSkinNormalSource (const CRenderMeshBuilder& rRendMesh, const CryChunkedFile::MeshDesc& rMeshDesc, const std::vector<CryBoneDesc>& arrBones):
	ICrySkinSource(
		NULL, // m_pLinks {&rMeshDesc.arrVertBinds[0]}
		0,// m_numLinks {rMeshDesc.arrVertBinds.size()}
		NULL, // m_pVertices
		0, // m_numVertices
		&rRendMesh.m_arrExtTangents[0],
		rRendMesh.m_arrExtTangents.size(),
		NULL // m_pExtToIntMapping
	)
{
	// to get the vertices, we'll have to build the temp buffer holding unique vertices
	// this maps the vertex coordinates to the vertex index in the new buffer
	//VertexWelderMap mapVertices;

	m_arrNormals.resize (rMeshDesc.numVertices());
	for (unsigned nVertex = 0; nVertex < rMeshDesc.numVertices(); ++nVertex)
		m_arrNormals[nVertex] = rMeshDesc.pVertices[nVertex].n;

	m_pVertices = &m_arrNormals[0];
	m_numVertices = m_arrNormals.size();
	m_arrExtToIntMap.resize (rRendMesh.m_arrExtTangMap.size());
	unsigned i;
	for (i = 0; i < rRendMesh.m_arrExtTangMap.size(); ++i)
		m_arrExtToIntMap[i] = rRendMesh.m_arrExtTangMap[i];
	m_pExtToIntMapping = &m_arrExtToIntMap[0];

	m_numLinks = rMeshDesc.arrVertBinds.size();
	m_arrLinks.resize (m_numLinks);
	for (unsigned i = 0; i < m_numVertices; ++i)
	{
		m_arrLinks[i].resize (1);
		unsigned nBone = m_arrLinks[i][0].BoneID   = rMeshDesc.arrVertBinds[i][0].BoneID;
		m_arrLinks[i][0].Blending	= 1;//getLink(0)[0].Blending;
		m_arrLinks[i][0].offset   = arrBones[nBone].getInvDefGlobal().TransformVectorOLD(rMeshDesc.arrNormals[i]);
	}

	m_pLinks = &m_arrLinks[0];
}

CRCSkinNormalSource::~CRCSkinNormalSource()
{
}