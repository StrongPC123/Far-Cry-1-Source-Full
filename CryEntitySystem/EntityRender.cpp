//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:EntityRender.cpp
//  Description: rendering of the entity and other relatated to visials stuff
//
//	History:
//	14.04.2001:Created by Vladimir Kajalin
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <IRenderer.h>
#include "Entity.h"
#include "EntitySystem.h"
#include <ISystem.h>
#include <I3DEngine.h>
#include <ILog.h>
#include <IRenderer.h>
#include "itimer.h"
//#include "list2.h"

#if defined(_DEBUG) && !defined(LINUX)
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

bool CEntity::DrawEntity(const SRendParams & _EntDrawParams) 
{
	FUNCTION_PROFILER( m_pISystem,PROFILE_3DENGINE );

  int nRecursionLevel = (int)m_pISystem->GetIRenderer()->EF_Query(EFQ_RecurseLevel) - 1;

	if(nRecursionLevel==0)
	{ // movement detection (if no recursion)
		m_vPrevDrawCenter = m_center;
		m_vPrevDrawAngles = m_angles;
		m_fPrevDrawScale  = m_fScale;
	}

  if (m_bHidden)
		return 0;

	if(!GetRadius())
		return 0;

	// to check is something was really drawn
  bool bSomethingWasDrawn = false; 

  // some parameters will be modified
	SRendParams rParms = _EntDrawParams;

	// enable lightmaps if alowed
/*	if(GetRndFlags()&ERF_USELIGHTMAPS && HasLightmap())
	{
		rParms.pLightMapInfo = GetLightmap();
		rParms.m_pLMTCBuffer = GetLightmapTexCoord();
	}
	else*/
	{
		rParms.pLightMapInfo = 0;
		rParms.pLMTCBuffer = 0;
	}

	if (!gPrecacheResourcesMode)
	{
		// remember last rendered frames
		if(nRecursionLevel>=0 && nRecursionLevel<=1)
			SetDrawFrame( m_pISystem->GetIRenderer()->GetFrameID(), nRecursionLevel );

		// If entity is drawn and it is set to be update only when visible, awake it for few frames.
		if (m_eUpdateVisLevel == eUT_PhysicsVisible || m_eUpdateVisLevel == eUT_Visible)
		{
			m_awakeCounter = 2;
		}
	}

  // gometry bbox test
	if(!(GetRndFlags()&ERF_DONOTCHECKVIS))
  if(!rParms.pShadowVolumeLightSource && !(rParms.dwFObjFlags & FOB_RENDER_INTO_SHADOWMAP))
	{
		AABB abEntityBBox(m_vBoxMin + m_center, m_vBoxMax + m_center);
		if(m_bForceBBox) 
		{	// if bbox was forced - make it bigger for frustum culling
			Vec3d vSize = m_vBoxMax - m_vBoxMin;
			abEntityBBox = AABB(m_vBoxMin - vSize + m_center, m_vBoxMax + vSize + m_center);
		}

    if(!m_pISystem->GetViewCamera().IsAABBVisibleFast( abEntityBBox ))
			return false;
	}

	// entity do not cast shadows from it own light
	if((m_pDynLight && !GetContainer() && rParms.pShadowVolumeLightSource) || (m_pDynLight && m_pDynLight == rParms.pShadowVolumeLightSource))
		return false;

	if (!gPrecacheResourcesMode)
	{
		// [PETAR] We need to remember the last frame id when this entity was rendered
		m_nLastVisibleFrameID = m_pISystem->GetIRenderer()->GetFrameID();

		// Awake entity on the first time it is seen.
		if (m_bSleeping)
			SetSleep(false);
	}

	// set entity params
  rParms.fScale = GetScale();
	if(!(rParms.dwFObjFlags & FOB_RENDER_INTO_SHADOWMAP))
	  rParms.vPos = GetPos();
  rParms.vAngles  = GetAngles();

	//[Timur][22/3/2003]
	/*
	// set custom materials
	if(m_lstMaterials.Count())
		rParms.pMaterials = &m_lstMaterials;*/

	IMatInfo * pPrevMaterial = rParms.pMaterial;
	rParms.pMaterial = m_pMaterial;

	if(m_bHasEnvLighting)
		rParms.dwFObjFlags |= FOB_ENVLIGHTING;

	if(m_arrShaderParams.Num())
		rParms.pShaderParams = &m_arrShaderParams;

	// calculate entity matrix once for all entity objects
	Matrix44 EntityMatrix;
	
	if(rParms.dwFObjFlags & FOB_RENDER_INTO_SHADOWMAP)
	{
		//EntityMatrix.Identity();
		//EntityMatrix.SetTranslation(rParms.vPos);
		//EntityMatrix	=	GetRotationZYX44(-gf_DEGTORAD*rParms.vAngles)*EntityMatrix; //NOTE: angles in radians and negated 
		//EntityMatrix	=	GetScale33( Vec3d(rParms.fScale,rParms.fScale,rParms.fScale) )*EntityMatrix;

		//OPTIMISED_BY_IVO  
		Matrix33diag diag	=	Vec3(rParms.fScale,rParms.fScale,rParms.fScale);	//use diag-matrix for scaling
		Matrix34 rt34			=	Matrix34::CreateRotationXYZ( Deg2Rad(rParms.vAngles),rParms.vPos );	//set scaling and translation in one function call
		EntityMatrix			=	rt34*diag;	//optimised concatenation: m34*t*diag

	}
	else	
	{
		//EntityMatrix.Identity();
		//EntityMatrix	=	GetTranslationMat(GetPos())*EntityMatrix;
		//EntityMatrix	=	GetRotationZYX44(-gf_DEGTORAD*rParms.vAngles)*EntityMatrix; //NOTE: angles in radians and negated 
		//EntityMatrix	= GetScale33( Vec3d(rParms.fScale,rParms.fScale,rParms.fScale) )*EntityMatrix;

		//OPTIMISED_BY_IVO  
		Matrix33diag diag	=	Vec3(rParms.fScale,rParms.fScale,rParms.fScale);	//use diag-matrix for scaling
		Matrix34 rt34			=	Matrix34::CreateRotationXYZ( Deg2Rad(rParms.vAngles),GetPos() );	//set scaling and translation in one function call
		EntityMatrix			=	rt34*diag;	//optimised concatenation: m34*t*diag

	}

	EntityMatrix	=	GetTransposed44(EntityMatrix);		//TODO: remove this after E3 and use Matrix34 instead of Matrix44


  // lod depends on distance and size and entity settings
  int nLod = max(0,(int)(rParms.fDistance*GetLodRatioNormilized()/(m_pISystem->GetI3DEngine()->GetObjectsLODRatio()*GetRadius())));

	// disable scissoring for entities inside portals
	if(m_pVisArea && ((IVisArea*)m_pVisArea)->IsPortal())
		rParms.dwFObjFlags |= FOB_NOSCISSOR;

  // draw static components
  for (unsigned int k=0; k<m_objects.size(); k++) 
  {	  
    CEntityObject *cb =  &m_objects[k];
    IStatObj * obj = cb->object;

    if (!obj || !(cb->flags & ETY_OBJ_INFO_DRAW))
      continue;

		// render
	  if (cb->flags & ETY_OBJ_USE_MATRIX)
    {
      rParms.pMatrix = &cb->mtx;
			if (!rParms.pShadowVolumeLightSource)			
				obj->Render(rParms,Vec3(zero), nLod);
			else if(GetRndFlags()&ERF_CASTSHADOWVOLUME)
				obj->RenderShadowVolumes(&rParms);
    }
	  else
		{						
			if (rParms.pShadowVolumeLightSource)			
			{
				rParms.pMatrix = &cb->mtx; // use matrix calculated on prev z/ambient pass
				rParms.vPos(0,0,0);
				rParms.vAngles(0,0,0);
				
				if (GetRndFlags()&ERF_CASTSHADOWVOLUME)
					obj->RenderShadowVolumes(&rParms);			
			}
			else
			{

				//Matrix44 mat;
				//mat.Identity();
				//mat=GetTranslationMat(cb->pos)*mat;
				//mat=GetRotationZYX44(-gf_DEGTORAD*cb->angles)*mat; //NOTE: angles in radians and negated 

				//OPTIMISED_BY_IVO  
				Matrix44 mat=Matrix34::CreateRotationXYZ( Deg2Rad(cb->angles),cb->pos);
				mat=GetTransposed44(mat);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44


				mat = mat*EntityMatrix;

				rParms.pMatrix = &mat;
				obj->Render(rParms,Vec3(zero), nLod);

				// cache for shadow volumes and decals
				cb->mtx = mat;
			}
		}

    bSomethingWasDrawn = true;
  }

	//[Timur] Always draw container if its present.
	if (m_pContainer)
  {
		m_pContainer->OnDraw( rParms );
    bSomethingWasDrawn = true;
  }

  // draw animated component 
  for (int i=0; i<MAX_ANIMATED_MODELS; i++)
  {
    ICryCharInstance * cmodel = m_pCryCharInstance[i];    			
    if (cmodel && (cmodel->GetFlags() & CS_FLAG_DRAW_MODEL))
    {
			if (m_pContainer)
			{
				//[Timur] Container Is not used Only to draw characters. This is a general modification of entity.
				//[Timur] m_pContainer->OnDraw( rParms );
			}
			else
      { 
        SRendParams RenderParams = rParms;
        //RenderParams.vAngles = m_physic && m_physic->GetType()==PE_ARTICULATED ? Vec3d(0,0,0) : m_angles;
				//RenderParams.vAngles = m_angles;

				if (rParms.pShadowVolumeLightSource)
				{
					if(GetRndFlags()&ERF_CASTSHADOWVOLUME)
						cmodel->RenderShadowVolumes(&RenderParams, (GetRndFlags()&ERF_SELFSHADOW) ? 0 : 10);
				}
				else
					cmodel->Draw(RenderParams,Vec3(123,123,123));
      }

	    bSomethingWasDrawn = true;
    }
  }

  DrawEntityDebugInfo(rParms);

	// restore material to original.
	rParms.pMaterial = pPrevMaterial;

	return bSomethingWasDrawn;
}	

void CEntity::DrawEntityDebugInfo(const SRendParams & rParms) 
{
  // debug
  if(m_pEntitySystem->m_pEntityBBoxes->GetIVal())
  if (!rParms.pShadowVolumeLightSource)			
	{
		Vec3d mins,maxs;
		GetBBox(mins,maxs);
		m_pISystem->GetIRenderer()->Draw3dBBox(mins,maxs);

    /*for (unsigned int k=0;k<m_objects.size();k++) 
		{	  
			CEntityObject *cb =  &m_objects[k];
			IStatObj * obj = cb->object;

			if (!obj)
				continue;

			maxs = obj->GetBoxMax() * m_fScale;
			mins = obj->GetBoxMin() * m_fScale;
			m_pISystem->GetIRenderer()->Draw3dBBox(m_center+mins,m_center+maxs);
		}*/
	}

  // debug
	if(m_pEntitySystem->m_pEntityHelpers->GetIVal())
	if (!m_objects.empty())
	{
		std::vector < CEntityObject>::iterator oi;
		for (oi = m_objects.begin(); oi != m_objects.end(); oi++)
		{
			CEntityObject eo =(*oi);
			if (!eo.object)
				continue;
			{
				CStatObj *so = (CStatObj*)eo.object;
				Vec3d pos;
				//CryQuat	qRot;
				int idx=0;
				char* name;
					while( name=(char*)eo.object->GetHelperById(idx++, pos) )
					{
						Vec3d	hSize = Vec3d( .5f, .5f, .5f );

						//Matrix44 mtx; 
						//mtx.Identity();
						//mtx=GetTranslationMat(m_center)*mtx;
						//mtx=GetRotationZYX44(-gf_DEGTORAD*m_angles)*mtx; //NOTE: angles in radians and negated 

						//OPTIMISED_BY_IVO  
						Matrix44 mtx = Matrix34::CreateRotationXYZ( Deg2Rad(m_angles),m_center);
						mtx=GetTransposed44(mtx);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44


						pos = mtx.TransformPointOLD(pos);

						Vec3d mins,maxs;
						mins = pos - hSize;
						maxs = pos + hSize;
						m_pISystem->GetIRenderer()->Draw3dBBox(mins,maxs);
						m_pISystem->GetIRenderer()->DrawLabel(pos, .73f, name);
					}
			}
		}	
	}		
}

// set bEntityHasLights flag if there are any light sources in entity objects
void CEntity::CheckEntityLightSourcesInEntityObjects() 
{
	m_bEntityHasLights = false;

	// Static objects
  for (unsigned int k=0; k<m_objects.size(); k++) 
  {	  
    CEntityObject *cb =  &m_objects[k];
    IStatObj * obj = cb->object;

    if (!obj)// || !(cb->flags & ETY_OBJ_INFO_DRAW))
      continue;

		if(obj->GetLightSources(0))
			m_bEntityHasLights = true;
	}

  // Animated components
  for (int i=0;i <MAX_ANIMATED_MODELS; i++)
  {
    ICryCharInstance * cmodel = m_pCryCharInstance[i];    			
    if (cmodel && cmodel->GetBoundLight(0))
			m_bEntityHasLights = true;
  }
}

void CEntity::ProcessEntityLightSources() 
{
	//prepare entity matrix
	//Matrix44 EntityMatrix;
	//EntityMatrix.Identity();
	//EntityMatrix=GetTranslationMat(m_center)*EntityMatrix;
	//EntityMatrix=GetRotationZYX44(-gf_DEGTORAD*m_angles)*EntityMatrix; //NOTE: angles in radians and negated 
	//EntityMatrix=GetScale33( Vec3d(m_fScale,m_fScale,m_fScale) )*EntityMatrix;

	FUNCTION_PROFILER( m_pISystem,PROFILE_ENTITY );

	if (!m_pEntitySystem->m_pUpdateCoocooEgg->GetIVal())
		return;
	
	Matrix33diag diag	=	Vec3d(m_fScale,m_fScale,m_fScale);	//use diag-matrix for scaling
	Matrix34 rt34			=	Matrix34::CreateRotationXYZ(Deg2Rad(m_angles),m_center);	//set rotation and translation in one function call
	Matrix44 EntityMatrix	=	rt34*diag;	//optimised concatenation: m34*diag

	EntityMatrix	=	GetTransposed44(EntityMatrix);		//TODO: remove this after E3 and use Matrix34 instead of Matrix44

	// entity light id
	int nLightId=0;

	// Static objects
  for (unsigned int k=0;k<m_objects.size();k++) 
  {	  
    CEntityObject *cb =  &m_objects[k];
    IStatObj * obj = cb->object;

    if (!obj || !(cb->flags & ETY_OBJ_INFO_DRAW))
      continue;

	  if (cb->flags & ETY_OBJ_USE_MATRIX)
    {
			for(int i=0; obj->GetLightSources(i); i++)
				m_pISystem->GetI3DEngine()->AddDynamicLightSource(*obj->GetLightSources(i),this,nLightId++,&cb->mtx);
    }
	  else
		{						

			//Matrix44 mat;
			//mat.Identity();
			//mat=GetTranslationMat(cb->pos)*mat;
			//mat=GetRotationZYX44(-gf_DEGTORAD*cb->angles)*mat; //NOTE: angles in radians and negated 

			//OPTIMISED_BY_IVO  
			Matrix44 mat = Matrix34::CreateRotationXYZ( Deg2Rad(cb->angles),cb->pos);
			mat=GetTransposed44(mat);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44


			mat = EntityMatrix*mat;
	
			for(int i=0; obj->GetLightSources(i); i++)
				m_pISystem->GetI3DEngine()->AddDynamicLightSource(*obj->GetLightSources(i),this,nLightId++,&mat);
		}
  }

	// Animated objects
  for (int i=0;i<MAX_ANIMATED_MODELS;i++)
  {
    ICryCharInstance * cmodel = m_pCryCharInstance[i];    			
    if (cmodel && (cmodel->GetFlags() & CS_FLAG_DRAW_MODEL) && cmodel->GetBoundLight(0))
    {
			assert(!IsStatic());
			Matrix44 LightMatrix;
      if (m_pContainer)
      { // use character angles and entity pos
				//LightMatrix.Identity();
				//LightMatrix=GetTranslationMat(m_center)*LightMatrix;
				//LightMatrix=GetRotationZYX44(-gf_DEGTORAD*GetAngles())*LightMatrix; //NOTE: angles in radians and negated 

				//OPTIMISED_BY_IVO  
				LightMatrix = Matrix34::CreateRotationXYZ( Deg2Rad(GetAngles()),m_center);
				LightMatrix=GetTransposed44(LightMatrix);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44

			
			}
      else
        LightMatrix = EntityMatrix;

      for(int i=0; cmodel->GetBoundLight(i); i++)
        m_pISystem->GetI3DEngine()->AddDynamicLightSource(*cmodel->GetBoundLight(i),this,nLightId++,&LightMatrix);
    }
  }
}

void CEntity::SetEntityStatObj( unsigned int nSlot, IStatObj * pStatObj, Matrix44 * pMatrix )
{
	if(nSlot>=0 /*&& nSlot<m_objects.size()*/)		//PETAR: changed to be able to SET entity objects, not just replace them
	{
		if (nSlot >= m_objects.size())
			m_objects.resize(nSlot+1);

		if(pMatrix)
		{
			m_objects[nSlot].mtx = *pMatrix;
			m_objects[nSlot].flags |=ETY_OBJ_USE_MATRIX;
		}
		else
			m_objects[nSlot].flags &=~ETY_OBJ_USE_MATRIX;

		// Release previous object.
		if (m_objects[nSlot].object != pStatObj && m_objects[nSlot].object != NULL)
			m_pISystem->GetI3DEngine()->ReleaseObject(m_objects[nSlot].object);

		m_objects[nSlot].object = pStatObj;
	}
}

IStatObj * CEntity::GetEntityStatObj( unsigned int nSlot, Matrix44* pMatrix, bool bReturnOnlyVisible)
{ 
	if(nSlot>=0 && nSlot<m_objects.size() && (!bReturnOnlyVisible || m_objects[nSlot].flags & ETY_OBJ_INFO_DRAW))
	{
		if(pMatrix)
		{
			if(m_objects[nSlot].flags & ETY_OBJ_USE_MATRIX)
				*pMatrix = m_objects[nSlot].mtx;
			else
			{ 
				//OPTIMISED_BY_IVO  
				// make object matrix
				Matrix33diag diag	=	Vec3(GetScale(),GetScale(),GetScale());
				Matrix34 rt34			=	Matrix34::CreateRotationXYZ( Deg2Rad(GetAngles()),GetPos() );
				Matrix44 EntityMatrix =	rt34*diag;
				EntityMatrix = GetTransposed44(EntityMatrix);

				Matrix44 ObjMatrix = Matrix34::CreateRotationXYZ( Deg2Rad(m_objects[nSlot].angles),m_objects[nSlot].pos);
				ObjMatrix = GetTransposed44(ObjMatrix);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44

				*pMatrix = ObjMatrix*EntityMatrix;
			}
		}

		return m_objects[nSlot].object;
	}

	return 0; 
}

ICryCharInstance* CEntity::GetEntityCharacter( unsigned int nSlot, Matrix44* pMatrix ) 
{ 
	if(nSlot>=0 && nSlot<MAX_ANIMATED_MODELS)
	{
		if(pMatrix)
			assert(0); // todo: entity should have matrix calculated once in entity Update()

		return m_pCryCharInstance[nSlot];
	}

	return 0; 
}

void CEntity::InitEntityRenderState()
{
	if(!m_pEntityRenderState)
	{
		m_pEntityRenderState = m_pISystem->GetI3DEngine()->MakeEntityRenderState();
		CheckEntityLightSourcesInEntityObjects();
	}
}

bool CEntity::IsEntityHasSomethingToRender()
{
	bool bItHas = false;

  // test static component 
  for (unsigned int k=0; k<m_objects.size(); k++) 
  {	  
    CEntityObject *cb =  &m_objects[k];
    IStatObj * obj = cb->object;

    if (!obj || !(cb->flags & ETY_OBJ_INFO_DRAW))
      continue;

    bItHas = true;
		break;
  }

  // test animated component 
  for (int i=0; i<MAX_ANIMATED_MODELS; i++)
  {
    ICryCharInstance * cmodel = m_pCryCharInstance[i];    			
    if (cmodel && (cmodel->GetFlags() & CS_FLAG_DRAW_MODEL))
    {
	    bItHas = true;
			break;
    }
  }
	
	return bItHas;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::PlayParticleSoundEffects( IParticleEffect *pEffect )
{
}

// vlad's todo: move ParticleEmitter into 3dengine
int CEntity::CreateEntityParticleEmitter(int nSlotId, const ParticleParams & PartParams, 
                                         float fSpawnPeriod,Vec3d vOffSet,Vec3d vDir,
																				 IParticleEffect *pEffect,float fScale)
{
	if(nSlotId>16)
	{
		m_pISystem->GetILog()->Log("Error: CEntity::CreateEntityParticleEmitter: nSlotId=%d is out of range", nSlotId);
		return -1;
	}

	if (!m_pParticleEmitters)
		m_pParticleEmitters = new PatricleEmitters;

	m_bUpdateEmitters = true;

	if ((int)m_pParticleEmitters->size() <= nSlotId)
	{
		m_pParticleEmitters->resize(nSlotId+1);
	}

	EntPartEmitter &emitter = (*m_pParticleEmitters)[nSlotId];

	emitter.pEmitter = m_pISystem->GetI3DEngine()->CreateParticleEmitter();

	if (!pEffect)
	{
		emitter.pEmitter->SetParams( PartParams );
	}
	else
	{
		emitter.pEmitter->SetEffect( pEffect );
	}

	emitter.fScale = fScale;
	emitter.vOffset = vOffSet;
	emitter.vDir = vDir;

	emitter.fSpawnPeriod = fSpawnPeriod;
	emitter.pEffect = pEffect;

	// Override spawn period.
	emitter.pEmitter->SetSpawnPeriod( fSpawnPeriod );
	emitter.pEmitter->SetUnlimitedLife(); // Unlimited lifetime.
	emitter.pEmitter->SetEntity( this );

	// Assign material only if entity doesnt contain any geometry.
	if (GetNumObjects() == 0 && m_nMaxCharNum == 0)
		emitter.pEmitter->SetMaterial( m_pMaterial );

  InitEntityRenderState();

	return nSlotId;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::DeleteParticleEmitter(int nId)
{
	if(m_pParticleEmitters && nId >= 0 && nId < (int)m_pParticleEmitters->size())
	{
		EntPartEmitter &emitter = (*m_pParticleEmitters)[nId];
		if (emitter.pEmitter)
			m_pISystem->GetI3DEngine()->DeleteParticleEmitter( emitter.pEmitter );
		emitter.pEmitter = 0;
		emitter.pEffect = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::UpdateParticleEmitters( SEntityUpdateContext &ctx )
{
	if(!m_pParticleEmitters)
		return;

	FUNCTION_PROFILER( m_pISystem,PROFILE_ENTITY );

	//[Timur] 
	//@FIXME !!!! this must not be here.
	//@HACK
	m_pISystem->GetI3DEngine()->RegisterEntity(this);

//	if(GetEntityVisArea() && !IsEntityAreasVisible())
	//	return;

	//float fDist = GetDistance(m_pISystem->GetViewCamera().GetPos(), GetPos());

	float fCurrTime = ctx.fCurrTime;

	// Keep emitters alive.
	for (int i=0; i < (int)m_pParticleEmitters->size(); i++)
	{
		EntPartEmitter &emitter = (*m_pParticleEmitters)[i];
		if (emitter.pEmitter)
		{
			// calculate entity rotation matrix 
			Matrix44 EntityMatrix;
			EntityMatrix.SetIdentity();

			//EntityMatrix.RotateMatrix_fix(GetAngles());
			EntityMatrix=Matrix44::CreateRotationZYX(-gf_DEGTORAD*GetAngles())*EntityMatrix; //NOTE: angles in radians and negated 

			Vec3 pos = GetPos() + EntityMatrix.TransformVectorOLD( emitter.vOffset );
			Vec3 dir = EntityMatrix.TransformVectorOLD( emitter.vDir );
			//
			emitter.pEmitter->SetPos( pos,dir,emitter.fScale );

			// Assign material only if entity doesnt contain any geometry.
			if (GetNumObjects() == 0 && m_nMaxCharNum == 0)
			{
				if (m_pMaterial)
					emitter.pEmitter->SetMaterial( m_pMaterial );
				else
					emitter.pEmitter->SetMaterial( NULL );
			}
		}
	}

//[kirill] this vould stop some entities with eUT_Always from being updated - so commented it out
  // on first succesfull update - enable potential visibility check
//	  SetUpdateVisLevel(eUT_PotVisible);
}

bool CEntity::IsEntityAreasVisible()
{
	if(!GetEntityRS())
		return false;

  IVisArea * pArea = (IVisArea *)m_pVisArea;

	// test area vis
	if(pArea && pArea->GetVisFrameId() == m_pISystem->GetIRenderer()->GetFrameID())
		return true; // visible

  if(m_pDynLight && !(m_pDynLight->m_Flags & DLF_THIS_AREA_ONLY)) // tmp hack, should be set from the script
  { // test neighbours
    IVisArea * Areas[64];
    int nCount = pArea->GetVisAreaConnections(Areas,64);
    for (int i=0; i<nCount; i++)
      if(Areas[i]->GetVisFrameId() == m_pISystem->GetIRenderer()->GetFrameID())
        return true; // visible
  }

	return false; // not visible
}

bool CEntity::CheckUpdateVisLevel( SEntityUpdateContext &ctx,EEntityUpdateVisLevel eUpdateVisLevel )
{
  switch(eUpdateVisLevel)
  {
    case eUT_Always: // always update
      return true;

    case eUT_InViewRange: // update if distance is less than camera view distance
      {
				bool bVisible = false;
				if (ctx.nFrameID - GetDrawFrame() < MAX_FRAME_ID_STEP_PER_FRAME)
					bVisible = true; // visible
        
				float fDistSquared = GetLengthSquared( ctx.vCameraPos - GetPos() );
				if (m_fUpdateRadius > 0)
				{
					// Within update radius.
					if (fDistSquared < m_fUpdateRadius*m_fUpdateRadius)
						return bVisible; // Update, because it is visible and within update radius.
					else
						return false; // Never update entity outside update radius.
				}

        return bVisible || fDistSquared < ctx.fMaxViewDistSquared;
      }

    case eUT_PotVisible: // update if entity visarea or terrain sector is visible
      {
				if (m_fUpdateRadius > 0)
				{
					// Within update radius.
					float fDistSquared = GetLengthSquared( ctx.vCameraPos - GetPos() );
					if (fDistSquared > m_fUpdateRadius*m_fUpdateRadius)
						return false; // Out of update radius.
				}

				if (ctx.nFrameID - GetDrawFrame() < MAX_FRAME_ID_STEP_PER_FRAME)
					return true; // visible
        
				float fDistSquared = GetLengthSquared( ctx.vCameraPos - GetPos() );
				if (fDistSquared > ctx.fMaxViewDistSquared)
					return false; // not visible

				float fAddRadius = m_fUpdateRadius;
        // tmp hack for lights, will be removed when static light concept will be used
				if (m_pDynLight && !(m_pDynLight->m_Flags & DLF_THIS_AREA_ONLY) && m_pDynLight->m_Origin!=Vec3d(0,0,0))
					fAddRadius += 8;

        return m_pISystem->GetI3DEngine()->IsPotentiallyVisible(this,fAddRadius);
      }

    case eUT_Visible: // update if visible
			if (ctx.nFrameID - GetDrawFrame() < MAX_FRAME_ID_STEP_PER_FRAME)
			{
				if (m_fUpdateRadius > 0)
				{
					float fDistSquared = GetLengthSquared( ctx.vCameraPos - GetPos() );
					// Within update radius.
					if (fDistSquared < m_fUpdateRadius*m_fUpdateRadius)
						return true; // Update, because it is visible and within update radius.
				}
				else
					return true; // Update, because it is visible and no update radius specified.
			}
			break;

    case eUT_Never: // newer update
      return false;

		case eUT_Physics: // Only update due to physics.
		case eUT_PhysicsVisible:
		case eUT_Unconditional:
			return true;
  }

  // Not update.
	return 	false;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::SetMaterial( IMatInfo *pMatInfo )
{
	m_pMaterial = pMatInfo;

	if(m_pMaterial)
		m_pMaterial->SetFlags(m_pMaterial->GetFlags()|MIF_WASUSED);
}

//////////////////////////////////////////////////////////////////////////
IMatInfo* CEntity::GetMaterial() const
{
	return m_pMaterial;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime)
{
	if(!GetEntityStatObj(0) || !GetEntityStatObj(0)->GetLeafBuffer())
		return;

	float fDistance = fPrevPortalDistance + GetPos().GetDistance(vPrevPortalPos);

	float fMaxViewDist = GetMaxViewDist();
	if(fDistance<fMaxViewDist && fDistance<m_pISystem->GetViewCamera().GetZMax())
	{
		for (unsigned int k=0; k<m_objects.size(); k++) 
		{	  
			CEntityObject *cb = &m_objects[k];
			IStatObj * obj = cb->object;
			if (obj && (cb->flags & ETY_OBJ_INFO_DRAW))
				obj->PreloadResources(fDistance,fTime,0);
		}

		for (int i=0; i<MAX_ANIMATED_MODELS; i++)
		{
			ICryCharInstance * cmodel = m_pCryCharInstance[i];    			
			if (cmodel && (cmodel->GetFlags() & CS_FLAG_DRAW_MODEL))
				cmodel->PreloadResources ( fDistance, 1.f, 0 );
		}
	}
}

float CEntity::GetMaxViewDist()
{
	I3DEngine * p3DEngine = m_pISystem->GetI3DEngine();
	return max(p3DEngine->GetObjectsMinViewDist(),
		GetRenderRadius()*p3DEngine->GetObjectsViewDistRatio()*GetViewDistRatioNormilized());
}

void CEntity::SetShaderFloat(const char *Name, float Val)
{
	char name[128];
	int i;

	strcpy(name, Name);
	strlwr(name);
	for (i=0; i<m_arrShaderParams.Num(); i++)
	{
		if (!strcmp(name, m_arrShaderParams[i].m_Name))
			break;
	}
	if (i == m_arrShaderParams.Num())
	{
		SShaderParam pr;
		strncpy(pr.m_Name, name, 32);
		m_arrShaderParams.AddElem(pr);
	}
	m_arrShaderParams[i].m_Type = eType_FLOAT;
	m_arrShaderParams[i].m_Value.m_Float = Val;
}