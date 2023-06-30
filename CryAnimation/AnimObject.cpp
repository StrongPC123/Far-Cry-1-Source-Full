#include "StdAfx.h"
#include "AnimObject.h"
#include "StringUtils.h"
#include "cvars.h"
#include "DebugUtils.h"

#include <I3DEngine.h>
#include <IMovieSystem.h>
#include <ITimer.h>
#include "CryCharAnimationParams.h"
#include "Cry_Geo.h"

using namespace CryStringUtils;

//////////////////////////////////////////////////////////////////////////
CAnimObject::CAnimObject()
{
	m_nRefCount = 0;
	m_characterModel.animSet.obj = this;
	m_characterModel.pAnimObject = this;

	m_bbox[0]=SetMaxBB();
	m_bbox[1]=SetMinBB();

	m_angles(0,0,0);

	m_nFlags = CS_FLAG_UPDATE|CS_FLAG_DRAW_MODEL;
	m_bNoTimeUpdate = false;
	m_currAnimation = 0;

	m_bAllNodesValid = false;
	m_bboxValid = false;

	m_time = 0;
	m_animSpeed = 1;
	m_physic = 0;
	m_lastAnimTime = -1;
	m_lastScale = 1.0f;
}

//////////////////////////////////////////////////////////////////////////
CAnimObject::~CAnimObject()
{
	ReleaseNodes();
	ReleaseAnims();
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::ReleaseNodes()
{
	I3DEngine *engine = Get3DEngine();
	for (unsigned int i = 0; i < m_nodes.size(); i++)
	{
		Node *pNode = m_nodes[i];
		if (pNode->m_object)
			engine->ReleaseObject( pNode->m_object );

		delete pNode;
	}
	m_nodes.clear();
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::ReleaseAnims()
{
	for (unsigned int i = 0; i < m_animations.size(); i++)
	{
		delete m_animations[i];
	}
	m_animations.clear();
	m_currAnimation = 0;
}

//////////////////////////////////////////////////////////////////////////
//! set name of geometry for this animated object.
void CAnimObject::SetFileName( const char *szFileName )
{
	m_fileName = szFileName;
	UnifyFilePath( m_fileName );
}

//////////////////////////////////////////////////////////////////////////
CAnimObject::Node* CAnimObject::CreateNode( const char *szNodeName )
{
	// Try to create object.
	I3DEngine *engine = Get3DEngine();
	IStatObj *obj = engine->MakeObject( m_fileName.c_str(),szNodeName,evs_ShareAndSortForCache,true,true );
	if (obj)
	{
		AddToBounds( obj->GetBoxMin(), m_bbox[0],m_bbox[1] );
		AddToBounds( obj->GetBoxMax(), m_bbox[0],m_bbox[1] );
	}

	Node *node = new Node;
	node->m_name = szNodeName;
	node->m_object = obj;
	node->m_id = (int)m_nodes.size();

	m_nodes.push_back(node);

	return node;
}

//! Render object ( register render elements into renderer )
void CAnimObject::Render(const struct SRendParams & rParams,const Vec3& t, int nLodLevel)
{
	Draw (rParams,Vec3(123,123,123));
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::Draw( const SRendParams &rp, const Vec3& t )
{
	SRendParams nodeRP = rp;
	Matrix44 renderTM;
	if (rp.pMatrix)
	{
		renderTM = *rp.pMatrix;
	}
	else
	{
		//renderTM.Identity();
		//renderTM=GetTranslationMat(rp.vPos)*renderTM;
		//renderTM=GetRotationZYX44(-gf_DEGTORAD*rp.vAngles )*renderTM; //NOTE: angles in radians and negated 
		//renderTM=GetScale33(Vec3(rp.fScale,rp.fScale,rp.fScale))*renderTM;

		//OPTIMISED_BY_IVO  
		Matrix33diag diag	=	Vec3(rp.fScale,rp.fScale,rp.fScale);	//use diag-matrix for scaling
		Matrix34 rt34			=	Matrix34::CreateRotationXYZ( Deg2Rad(rp.vAngles),rp.vPos );	//set rotation and translation in one function call
		renderTM					=	rt34*diag;	//optimised concatenation: m34*diag
		renderTM=GetTransposed44(renderTM);			//TODO: remove this after E3 and use Matrix34 instead of Matrix44
	}
	for (unsigned int i = 0; i < m_nodes.size(); i++)
	{
		Node *node = m_nodes[i];
		if (node->m_object)
		{
			Matrix44 tm = GetNodeMatrix(node) * renderTM;
			nodeRP.pMatrix = &tm;
			nodeRP.dwFObjFlags |= FOB_TRANS_MASK;
			m_nodes[i]->m_object->Render(nodeRP,Vec3(zero),0);
		}
	}
	DrawBoundObjects( rp,renderTM,0 );
}

//////////////////////////////////////////////////////////////////////////
//! Draw the character shadow volumes into the stencil buffer using a set of specified 
//! rendering parameters ( for indoors )
void CAnimObject::RenderShadowVolumes(const SRendParams *rParams, int nLimitLOD)
{
	const SRendParams &rp = *rParams;

	SRendParams nodeRP = rp;
	Matrix44 renderTM;
	if (rp.pMatrix)
	{
		renderTM = *rp.pMatrix;
	}
	else
	{
		//renderTM.Identity();
		//renderTM=GetTranslationMat(rp.vPos)*renderTM;
		//renderTM=GetRotationZYX44(-gf_DEGTORAD*rp.vAngles )*renderTM; //NOTE: angles in radians and negated 
		//renderTM=GetScale33(Vec3(rp.fScale,rp.fScale,rp.fScale))*renderTM;

		//OPTIMISED_BY_IVO  
		Matrix33diag diag	=	Vec3(rp.fScale,rp.fScale,rp.fScale);	//use diag-matrix for scaling
		Matrix34 rt34			=	Matrix34::CreateRotationXYZ(Deg2Rad(rp.vAngles),rp.vPos);	//set rotation and translation in one function call
		renderTM					=	rt34*diag;	//optimised concatenation: m34*diag
		renderTM=GetTransposed44(renderTM);			//TODO: remove this after E3 and use Matrix34 instead of Matrix44

	}
	for (unsigned int i = 0; i < m_nodes.size(); i++)
	{
		Node *node = m_nodes[i];
		if (node->m_object)
		{
			Matrix44 tm = GetNodeMatrix(node) * renderTM;
			nodeRP.pMatrix = &tm;
			m_nodes[i]->m_object->RenderShadowVolumes(&nodeRP);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::Animate( float time )
{
	if (!m_currAnimation)
		return;


	if (m_currAnimation->loop)
	{
		float timelen = m_currAnimation->endTime - m_currAnimation->startTime;
		time = m_currAnimation->startTime + cry_fmod( time,timelen );
	}
	else
	{
		time = m_currAnimation->startTime + time;
	}
	if (time < 0)
	{
		// Go negative.
		time = m_currAnimation->endTime + time;
	}

	for (unsigned int i = 0; i < m_nodes.size(); i++)
	{
		Node *node = m_nodes[i];
		NodeAnim *nodeAnim = GetNodeAnim(node);
		if (node->m_object && nodeAnim)
		{
			if (!m_bAllNodesValid)
			{
				node->m_pos = nodeAnim->m_pos;
				node->m_scale = nodeAnim->m_scale;
				node->m_rotate = nodeAnim->m_rotate;
				node->m_bMatrixValid = false;
			}
			if (nodeAnim->m_posTrack)
			{
				node->m_pos = nodeAnim->m_posTrack->GetPosition( time );
				node->m_bMatrixValid = false;
				// Bounding box can change.
				m_bboxValid = false;
			}
			if (nodeAnim->m_rotTrack)
			{
				node->m_rotate = nodeAnim->m_rotTrack->GetOrientation( time );
				node->m_bMatrixValid = false;
				m_bboxValid = false;
			}
			if (nodeAnim->m_scaleTrack)
			{
				node->m_scale = nodeAnim->m_scaleTrack->GetScale( time );
				node->m_bMatrixValid = false;
				m_bboxValid = false;
			}
		}
	}
	//m_bboxValid = false;

	m_bAllNodesValid = true;
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::RecalcBBox()
{
	AABB box;
	m_bbox[0]=SetMaxBB();
	m_bbox[1]=SetMinBB();
	// Re calc bbox.
	for (unsigned int i = 0; i < m_nodes.size(); i++)
	{
		Node *node = m_nodes[i];
		Matrix44 &tm = GetNodeMatrix(node);
		if (node->m_object)
		{
			box.min = node->m_object->GetBoxMin();
			box.max = node->m_object->GetBoxMax();
			box.Transform( tm );
			AddToBounds( box.min, m_bbox[0],m_bbox[1] );
			AddToBounds( box.max, m_bbox[0],m_bbox[1] );
		}
	}
	m_bboxValid = true;
}

//////////////////////////////////////////////////////////////////////////
Matrix44& CAnimObject::GetNodeMatrix( Node *node )
{
	// fixme.
	node->m_bMatrixValid = false;


	assert(node);
	if (!node->m_bMatrixValid)
	{
		// Make local matrix.
		//Q2M_CHANGED_BY_IVO
		//node->m_rotate.GetMatrix(node->m_tm);
		//node->m_tm = GetTransposed44(node->m_rotate);
		node->m_tm = Matrix44(node->m_rotate);


		//SCALE_CHANGED_BY_IVO
		//node->m_tm.ScaleMatrix( node->m_scale.x,node->m_scale.y,node->m_scale.z );
		node->m_tm = Matrix33::CreateScale( Vec3(node->m_scale.x,node->m_scale.y,node->m_scale.z) ) * node->m_tm;

		node->m_tm.SetTranslationOLD( node->m_pos );
		node->m_bMatrixValid = true;

		if (node->m_parent)
		{
			// Combine with parent matrix.
			Matrix44 &parentTM = GetNodeMatrix(node->m_parent);
			node->m_tm = node->m_tm * parentTM;
		}
	}

	return node->m_tm;
}

//////////////////////////////////////////////////////////////////////////
CAnimObject::NodeAnim* CAnimObject::GetNodeAnim( Node *node )
{
	if (!m_currAnimation)
		return 0;
	if (node->m_id >= (int)m_currAnimation->nodeAnims.size())
	{
		return 0;
	}
	return &(m_currAnimation->nodeAnims[node->m_id]);
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::AddAnimation( Animation* anim )
{
	m_animations.push_back(anim);
	if (m_animations.size() == 1)
	{
		// First anim, make current.
		m_currAnimation = anim;
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::RemoveAnimation( Animation* anim )
{
	m_animations.erase( std::remove(m_animations.begin(),m_animations.end(),anim),m_animations.end() );
	if (m_currAnimation = anim)
	{
		if (!m_animations.empty())
			m_currAnimation = m_animations[0];
		else
			m_currAnimation = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
CAnimObject::Animation* CAnimObject::FindAnimation( const char *szAnimationName )
{
	for (unsigned int i = 0; i < m_animations.size(); i++)
	{
		if (stricmp(m_animations[i]->name.c_str(),szAnimationName)==0)
		{
			return m_animations[i];
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
int CAnimObject::FindNodeByName( const char *szNodeName )
{
	for (unsigned int i = 0; i < m_nodes.size(); i++)
	{
		if (stricmp(m_nodes[i]->m_name.c_str(),szNodeName)==0)
		{
			return i;
		}
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
const char* CAnimObject::GetNodeName( int nodeId )
{
	if (nodeId > 0 && nodeId < (int)m_nodes.size())
		return m_nodes[nodeId]->m_name.c_str();
	return "";
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::SetCurrent( Animation *anim )
{
	if (anim != m_currAnimation)
	{
		m_time = 0;
		m_lastAnimTime = -1;
		m_bAllNodesValid = false;
		m_bboxValid = false;
		m_currAnimation = anim;
		if (m_currAnimation)
		{
			Animate(m_currAnimation->startTime);
		}
	}
}

//! Resets all animation layers ( stops all animations )
void CAnimObject::ResetAnimations()
{
	if (m_currAnimation)
	{
		// Animate existing animation to 0.
		Animate(m_currAnimation->startTime);
	}
	SetCurrent(0);
	UpdatePhysics( m_lastScale );
};

//////////////////////////////////////////////////////////////////////////
const CDLight* CAnimObject::GetBoundLight (int nIndex)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
void CAnimObject::Update( Vec3 vPos, float fRadius, unsigned uFlags)
{
	if (!(m_nFlags & CS_FLAG_UPDATE))
		return;

	float m_lastAnimTime = m_time;
	if (!m_bNoTimeUpdate)
	{
		float dt = g_GetTimer()->GetFrameTime();
		m_time = m_time + dt*m_animSpeed;

		if (m_currAnimation)
		{
			if (!m_currAnimation->loop && !m_currAnimation->haveLoopingController)
			{
				if (m_time > m_currAnimation->endTime)
					m_time = m_currAnimation->endTime;
				if (m_time < m_currAnimation->startTime)
					m_time = m_currAnimation->startTime;
			}
		}
	}
	if (m_lastAnimTime != m_time)
		Animate( m_time );
}

//! Set the current time of the given layer, in seconds
void CAnimObject::SetLayerTime (int nLayer, float fTimeSeconds)
{
	m_time = fTimeSeconds;
	m_fAnimTime = fTimeSeconds;

	if (m_lastAnimTime != m_time)
		Animate( m_time );

	m_lastAnimTime = m_time;
};

//////////////////////////////////////////////////////////////////////////
bool CAnimObject::IsModelFileEqual (const char* szFileName)
{
	string strPath = szFileName;
	UnifyFilePath(strPath);

	return stricmp(strPath.c_str(),m_fileName.c_str()) == 0;
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::CreateDecal(CryEngineDecalInfo& Decal)
{
}

//////////////////////////////////////////////////////////////////////////
// Physics.
//////////////////////////////////////////////////////////////////////////
IPhysicalEntity* CAnimObject::CreateCharacterPhysics(IPhysicalEntity *pHost, float mass,int surface_idx,float stiffness_scale, int nLod)
{
	//m_physic = GetPhysicalWorld()->CreatePhysicalEntity(PE_STATIC,&partpos,(IEntity*)this);
	assert( pHost );
	m_physic = pHost;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
int CAnimObject::CreateAuxilaryPhysics(IPhysicalEntity *pHost, int nLod)
{
	//assert( pHost );
	//m_physic = pHost;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::BuildPhysicalEntity( IPhysicalEntity *pent,float mass,int surface_idx,float stiffness_scale,int nLod )
{
	assert( pent );

	m_physic = pent;

	unsigned int i;
	float totalVolume = 0;

	for (i = 0; i < m_nodes.size(); i++)
	{
		Node *node = m_nodes[i];
		if (!node->m_object)
			continue;
		phys_geometry *geom = node->m_object->GetPhysGeom(0);
		if (geom)
		{
			totalVolume += geom->V;
		}
	}
	float density = mass/totalVolume;

	pe_geomparams params;
	for (i = 0; i < m_nodes.size(); i++)
	{
		Node *node = m_nodes[i];
		if (!node->m_object)
			continue;

		params.density = density;

		// Add collision geometry.
		params.flags = geom_collides;
		phys_geometry *geom = node->m_object->GetPhysGeom(0);
		if (geom)
		{
			m_physic->AddGeometry( geom, &params, node->m_id );
			node->bPhysics = true;
		}

		// Add obstruct geometry.
		params.flags = geom_proxy;
		geom = node->m_object->GetPhysGeom(1);
		if (geom)
		{
			m_physic->AddGeometry( geom, &params, node->m_id );
			node->bPhysics = true;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::UpdatePhysics( float fScale )
{
	if (!m_physic)
		return;

	m_lastScale = fScale;

	Matrix44 parentTM;
	parentTM.SetIdentity();

	//SCALE_CHANGED_BY_IVO
	//parentTM.ScaleMatrix( fScale,fScale,fScale );
	parentTM=Matrix33::CreateScale( Vec3(fScale,fScale,fScale) )*parentTM;

	int numNodes = (int)m_nodes.size();
	pe_params_part params;
	for (int i = 0; i < numNodes; i++)
	{
		Node *node = m_nodes[i];
		if (!node->bPhysics)
			continue;

		params.partid = node->m_id;
		params.bRecalcBBox = true;
		//if (i == numNodes-1) // last node.
			//params.bRecalcBBox = true;
		
		Matrix44 tm = GetNodeMatrix(node) * parentTM;
		params.pMtx4x4T = tm.GetData();

		m_physic->SetParams( &params );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::AddImpact(int partid, vectorf point,vectorf impact)
{
	//
}

// Pushes the underlying tree of objects into the given Sizer object for statistics gathering
void CAnimObject::GetMemoryUsage(class ICrySizer* pSizer)const
{
#if ENABLE_GET_MEMORY_USAGE
	if (!pSizer->Add (*this))
		return;
	pSizer->AddString (m_fileName);
#endif
}

void CAnimObject::Node::GetSize (ICrySizer* pSizer)const
{
#if ENABLE_GET_MEMORY_USAGE
	if (!pSizer->Add (*this))
		return;
	pSizer->AddString (m_name);
	m_parent->GetSize(pSizer);

	if (pSizer->GetFlags() & CSF_RecurseSubsystems)
	{
		SIZER_COMPONENT_NAME(pSizer, "3DEngine");
		m_object->GetMemoryUsage(pSizer);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
ICryBone* CAnimObject::GetBoneByName(const char * szName)
{
	for (unsigned int i = 0; i < m_nodes.size(); i++)
	{
		if (m_nodes[i] != NULL && stricmp(m_nodes[i]->m_name.c_str(),szName) == 0)
			return m_nodes[i];
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CAnimObject::ObjectBindingHandle CAnimObject::AttachObjectToBone(IBindable * pWeaponModel, const char *szBoneName, bool bUseRelativeToDefPoseMatrix , unsigned nFlags )
{
	if(!szBoneName)
	{
		// if you find this assert, this means someone passed here a model and NO bone to attach it to. What should it mean anyway??
		assert (!pWeaponModel);
		// just detach everything
		DetachAll();
		return nInvalidObjectBindingHandle;
	}

	int nBone = FindNodeByName(szBoneName);
	if(nBone < 0)
	{
		if (!pWeaponModel)
		{
			// this is a severe bug if the bone name is invalid and someone tries to detach the model
			// if we find such a situation, we should try to detach the model, as it might be destructed already

			// but now we just detach everything, as it's simpler
			//m_arrBoundObjects.clear();
			//m_arrBoundObjectIndices.clear();

			g_GetLog()->LogError ("\002AttachObjectToBone is called for bone \"%s\", which is not in the model \"%s\". Ignoring, but this may cause a crash because the corresponding object won't be detached after it's destroyed", szBoneName, GetFileName() );
#ifdef _DEBUG
			// this assert will only happen if the ca_NoAttachAssert is off
			// assert (GetCVars()->ca_NoAttachAssert());
#endif
		}
		return nInvalidObjectBindingHandle; // bone not found, do nothing
	}

	// detach all objects from this bone before creating a new one
	DetachAllFromBone(nBone);

	if(pWeaponModel == NULL)
	{
		// we didn't create a new binding, so return invalid handle
		return nInvalidObjectBindingHandle;
	}
	else
	{
		return AttachToBone(pWeaponModel, nBone, nFlags);
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::PreloadResources ( float fDistance, float fTime, int nFlags)
{
	CryCharInstanceBase::PreloadResources( fDistance,fTime,nFlags );

	for (unsigned int i = 0; i < m_nodes.size(); i++)
		if (m_nodes[i] != NULL && m_nodes[i]->m_object)
			m_nodes[i]->m_object->PreloadResources(fDistance, fTime, nFlags);
}

//////////////////////////////////////////////////////////////////////////
void CAnimObject::DrawBoundObjects(const SRendParams & rRendParams, Matrix44 &inmatTranRotMatrix, int nLOD)
{
	static const float fColorBBoxAttached[4] = {0.5,1,1,0.75};

	if (g_GetCVars()->ca_NoDrawBound())
		return;

	if (m_arrBinds.empty())
		return;

	Matrix44 matAttachedObjectMatrix;
	SRendParams rParams (rRendParams);
	// this is required to avoid the attachments using the parent character material (this is the material that overrides the default material in the attachment)
	rParams.pMaterial = NULL;

	if (m_nFlags & CS_FLAG_DRAW_NEAR)
	{
		rParams.dwFObjFlags |= FOB_NEAREST;
	}

	BindArray::iterator it, itEnd = m_arrBinds.end();
	for (it = m_arrBinds.begin(); it != itEnd; ++it)
	{
		IBindable* pBoundObject = (*it)->pObj;
		if (!pBoundObject)
			continue;

		int nBone =	(*it)->nBone;
		if (nBone < 0 || nBone >= (int)m_nodes.size())
			continue;

		Matrix44 &boneTM = GetNodeMatrix(m_nodes[nBone]);
		matAttachedObjectMatrix = boneTM * inmatTranRotMatrix;
		rParams.pMatrix = &matAttachedObjectMatrix;

		if (g_GetCVars()->ca_DrawBBox()>1){
			Matrix34 m34=Matrix34(GetTransposed44(matAttachedObjectMatrix));

			CryAABB caabb;
			pBoundObject->GetBBox(caabb.vMin, caabb.vMax);

			debugDrawBBox (m34, caabb, g_GetCVars()->ca_DrawBBox()-1,fColorBBoxAttached);
		}
		//pBoundObject->SetShaderTemplate( rParams.nShaderTemplate, 0] ,0 );
		pBoundObject->Render(rParams,Vec3(zero), nLOD);
	}  
}
