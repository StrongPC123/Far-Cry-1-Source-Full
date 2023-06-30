// These are the classes that supply information to the CrySkin builders
#ifndef _SKIN_DATA_SOURCES_HDR_
#define _SKIN_DATA_SOURCES_HDR_

#include "CryChunkedFile.h"
#include "CrySkinBuilderBase.h"

class CRCSkinNormalSource: public ICrySkinSource
{
public:
	CRCSkinNormalSource (const class CRenderMeshBuilder& rRendMesh, const CryChunkedFile::MeshDesc& rMeshDesc, const std::vector<CryBoneDesc>& arrBones);
	~CRCSkinNormalSource ();

protected:
	std::vector<Vec3d> m_arrNormals;
	std::vector<unsigned> m_arrExtToIntMap;
	std::vector<CryVertexBinding> m_arrLinks;
};


class CRCSkinVertexSource: public ICrySkinSource
{
public:
	// constructs the vertex source recomputing the actual vertex coordinates with the bones given if any
	CRCSkinVertexSource (const class CRenderMeshBuilder& rRendMesh, const CryChunkedFile::MeshDesc& rMeshDesc, Matrix44* pBones = NULL);
	~CRCSkinVertexSource ();

protected:
	// the vertex buffer - initial pose, no duplicated vertices
	std::vector<Vec3d> m_arrVerts;
	std::vector<unsigned> m_arrExtToIntMap;
};


#endif
