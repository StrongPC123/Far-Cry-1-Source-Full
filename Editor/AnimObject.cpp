#include "StdAfx.h"
#include "AnimObject.h"
#include "ASE\ASEParser.h"

#include <I3DEngine.h>
#include <IMovieSystem.h>
#include <ITimer.h>

//////////////////////////////////////////////////////////////////////////
CAnimObject::CAnimObject()
{
	m_3dEngine = GetIEditor()->Get3DEngine();

	m_bbox[0].SetMax();
	m_bbox[1].SetMin();

	m_angles.Set(0,0,0);

	m_bNoUpdate = false;
}

//////////////////////////////////////////////////////////////////////////
CAnimObject::~CAnimObject()
{
	ReleaseNodes();
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::ReleaseNodes()
{
	for (int i = 0; i < m_nodes.size(); i++)
	{
		delete m_nodes[i];
	}
	m_nodes.clear();
}

//////////////////////////////////////////////////////////////////////////
CAnimObject::Node* CAnimObject::CreateNode( const char *szNodeName )
{
	// Try to create object.
	IStatObj *obj = m_3dEngine->MakeObject( m_fileName.c_str(),szNodeName );
	if (!obj)
		return 0;

	obj->GetBoxMin().AddToBounds( m_bbox[0],m_bbox[1] );
	obj->GetBoxMax().AddToBounds( m_bbox[0],m_bbox[1] );

	Node *node = new Node;
	node->m_name = szNodeName;
	node->m_object = obj;

	m_nodes.push_back(node);

	return node;
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::Draw( const SRendParams &rp )
{
	SRendParams nodeRP = rp;
	for (int i = 0; i < m_nodes.size(); i++)
	{
		Node *node = m_nodes[i];
		if (node->m_object)
		{
			Matrix tm = GetNodeMatrix(node);
			tm = node->m_invOrigTM * tm;
			nodeRP.pMatrix = &tm;
			m_nodes[i]->m_object->Render(nodeRP);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::Animate( float time )
{
	for (int i = 0; i < m_nodes.size(); i++)
	{
		Node *node = m_nodes[i];
		if (node->m_object)
		{
			if (node->m_posTrack)
			{
				Vec3 pos(0,0,0);
				node->m_posTrack->GetValue( time,pos );
				node->m_pos = pos;
				node->m_bMatrixValid = false;
			}
			if (node->m_rotTrack)
			{
				Quat q(1,0,0,0);
				node->m_rotTrack->GetValue( time,q );
				node->m_rotate = q;
				node->m_bMatrixValid = false;
			}
			if (node->m_scaleTrack)
			{
				Vec3 scale(1,1,1);
				node->m_scaleTrack->GetValue( time,scale );
				node->m_scale = scale;
				node->m_bMatrixValid = false;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
Matrix& CAnimObject::GetNodeMatrix( Node *node )
{
	// fixme.
	node->m_bMatrixValid = false;


	assert(node);
	if (!node->m_bMatrixValid)
	{
		// Make local matrix.
		node->m_rotate.GetMatrix(node->m_tm);
		node->m_tm.ScaleMatrix( node->m_scale.x,node->m_scale.y,node->m_scale.z );
		node->m_tm.SetTranslation( node->m_pos );
		node->m_bMatrixValid = true;

		if (node->m_parent)
		{
			// Combine with parent matrix.
			Matrix &parentTM = GetNodeMatrix(node->m_parent);
			node->m_tm = node->m_tm * parentTM;
		}
	}

	return node->m_tm;
}

//////////////////////////////////////////////////////////////////////////
bool CAnimObject::Load( const char *fileName,const char *aseFile )
{
	m_fileName = fileName;
	CASEParser aseParser;
	if (aseParser.Parse( this,aseFile ))
	{
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
const CDLight* CAnimObject::GetBoundLight (int nIndex)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//! Draw the character shadow volumes into the stencil buffer using a set of specified 
//! rendering parameters ( for indoors )
void CAnimObject::RenderShadowVolumes(const SRendParams *rParams)
{
}

//! Draw the character without shaders for shadow mapping
void CAnimObject::DrawForShadow(const Vec3d & vTranslationPlus)
{
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::Update( Vec3d vPos, float fRadius, unsigned uFlags)
{
	float time = GetIEditor()->GetSystem()->GetITimer()->GetCurrTime();
	Animate( time );
}

//////////////////////////////////////////////////////////////////////////
bool CAnimObject::IsModelFileEqual (const char* szFileName)
{
	return stricmp(szFileName,m_fileName.c_str()) == 0;
}