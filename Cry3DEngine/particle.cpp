////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   particle.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: move, fill vertex buffer
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "partman.h"
#include "objman.h"
#include "3dengine.h"

#define OT_RIGID_PARTICLE 10

void CParticle::FillBuffer(const PartProcessParams & PPP)
{
	float t = (PPP.fCurTime-m_fSpawnTime)/m_fLifeTime;
  float alpha = (1.f - t);//*2;
  if(alpha<0)
    alpha=0;

  Vec3d vColor = m_pParams->vColorStart*(1.f-t) + m_pParams->vColorEnd*t;

  if (m_pParams->eBlendType != ParticleBlendType_AlphaBased)
  {
    vColor.x*=alpha;
    vColor.y*=alpha;
    vColor.z*=alpha;
  }

  Vec3d v1 = m_vPos + PPP.vUp    *m_fSize;
  Vec3d v2 = m_vPos + PPP.vRight *m_fSize;
  Vec3d v3 = m_vPos - PPP.vRight *m_fSize;
  Vec3d v4 = m_vPos - PPP.vUp    *m_fSize;

  UCol col;
  col.bcolor[0] = FtoI(vColor.x*255.f);
  col.bcolor[1] = FtoI(vColor.y*255.f);
  col.bcolor[2] = FtoI(vColor.z*255.f);
  col.bcolor[3] = FtoI(alpha*255.f);

  PPP.pVertBufChunk[0].xyz = v1;
  PPP.pVertBufChunk[0].color = col;
  PPP.pVertBufChunk[0].st[0] = 0; PPP.pVertBufChunk[0].st[1] = 1;

  PPP.pVertBufChunk[1].xyz = v2;
  PPP.pVertBufChunk[1].color = col;
  PPP.pVertBufChunk[1].st[0] = 0; PPP.pVertBufChunk[0].st[1] = 0;

  PPP.pVertBufChunk[2].xyz = v3;
  PPP.pVertBufChunk[2].color = col;
  PPP.pVertBufChunk[2].st[0] = 1; PPP.pVertBufChunk[0].st[1] = 1;

  PPP.pVertBufChunk[3].xyz = v4;
  PPP.pVertBufChunk[3].color = col;
  PPP.pVertBufChunk[3].st[0] = 1; PPP.pVertBufChunk[0].st[1] = 0;
}

bool CParticle::Update(const PartProcessParams & PPP)
{
////////////////////////////////////////////////////////////////////////////////////////////////
// Move
////////////////////////////////////////////////////////////////////////////////////////////////
	Vec3 prevPos = m_vPos;
	float fAge = PPP.fCurTime - m_fSpawnTime;

	if(m_pParams->nParticleFlags & PART_FLAG_BIND_EMITTER_TO_CAMERA && m_pEmitter)
	{
		float fLen = m_pEmitter->GetParams().vSpaceLoopBoxSize.GetLength();
		Vec3d vDir = PPP.pCamera->GetVCMatrixD3D9().GetOrtZ();
		m_pEmitter->m_pos = PPP.pCamera->GetPos()-vDir*fLen*0.75;
	}

	if(m_pParams->nParticleFlags & PART_FLAG_SPACELOOP && m_pEmitter)
	{ // process space loop
		Vec3d vSpaceFocusPos = m_pEmitter->m_pos;
		float fMaxX = m_pEmitter->GetParams().vSpaceLoopBoxSize.x;
		float fMaxY = m_pEmitter->GetParams().vSpaceLoopBoxSize.y;
		float fMaxZ = m_pEmitter->GetParams().vSpaceLoopBoxSize.z;
		
		// X
		while((m_vPos.x-vSpaceFocusPos.x)>fMaxX)
			m_vPos.x-=fMaxX*2;
		while((vSpaceFocusPos.x-m_vPos.x)>fMaxX)
			m_vPos.x+=fMaxX*2;

		// Y
		while((m_vPos.y-vSpaceFocusPos.y)>fMaxY)
			m_vPos.y-=fMaxY*2;
		while((vSpaceFocusPos.y-m_vPos.y)>fMaxY)
			m_vPos.y+=fMaxY*2;

		// Z
		while((m_vPos.z-vSpaceFocusPos.z)>fMaxZ)
			m_vPos.z-=fMaxZ*2;
		while((vSpaceFocusPos.z-m_vPos.z)>fMaxZ)
			m_vPos.z+=(fMaxZ*2-rnd());
	}

	if(m_pParams->nParticleFlags & PART_FLAG_NO_INDOOR)
	{
		IVisArea * pArea = PPP.p3DEngine->GetVisAreaFromPos(m_vPos);
		if(pArea)
			return false;
	}

	if((m_pParams->nParticleFlags & PART_FLAG_SPACELIMIT) && m_pEmitter)
	{
		Vec3d vBoxMin = m_pEmitter->GetParams().vPosition - m_pEmitter->GetParams().vSpaceLoopBoxSize;
		Vec3d vBoxMax = m_pEmitter->GetParams().vPosition + m_pEmitter->GetParams().vSpaceLoopBoxSize;
		
		vBoxMin.CheckMin(m_pEmitter->GetParams().vPosition + m_pEmitter->GetParams().vSpaceLoopBoxSize);
		vBoxMax.CheckMax(m_pEmitter->GetParams().vPosition - m_pEmitter->GetParams().vSpaceLoopBoxSize);

		Vec3d vNextPos = m_vPos + m_vDelta * PPP.fFrameTime;

		if(	vNextPos.x < vBoxMin.x ||
				vNextPos.y < vBoxMin.y ||
				vNextPos.z < vBoxMin.z ||
				vNextPos.x > vBoxMax.x ||
				vNextPos.y > vBoxMax.y ||
				vNextPos.z > vBoxMax.z )
			return false;
	}

  if(m_pPhysEnt)
  { // use phys engine to control particle
    pe_status_pos status_pos;
    m_pPhysEnt->GetStatus(&status_pos);
    m_vPos = status_pos.pos;


		//CHANGED_BY_IVO (NOTE: order of angles is flipped!!!!)
		//status_pos.q.get_Euler_angles_xyz(PPP.pPartSpray->m_vAngles.z,PPP.pPartSpray->m_vAngles.y,PPP.pPartSpray->m_vAngles.x);
    //EULER_IVO
		//Vec3 TempAng;	PPP.pPartSpray->m_vAngles = status_pos.q.GetEulerAngles_XYZ(TempAng);
		m_vAngles = Ang3::GetAnglesXYZ( matrix3x3f(status_pos.q) );
		m_vAngles *= 180/gf_PI;
  }
	else
  { // process movement, size and rotation
    if(!(m_pParams->nParticleFlags & PART_FLAG_LINEPARTICLE))
    {
			Vec3 vDelta = m_vDelta;

			if (m_pParams->fSpeedFadeOut != 0)
			{
				// Fade out speed.
				float t = m_fLifeTime - fAge;
				if (t < m_pParams->fSpeedFadeOut)
				{
					float speedScale = (t/m_pParams->fSpeedFadeOut);
					if (speedScale < 0) speedScale = 0;
					if (speedScale > 1) speedScale = 1;
					vDelta = vDelta*speedScale;
				}
			}

      m_vPos += vDelta * PPP.fFrameTime;

			if (m_pParams->fTurbulenceSize)
			{
				m_vPos += Vec3d(	cry_cosf( fAge*m_pParams->fTurbulenceSpeed),
													cry_sinf( fAge*m_pParams->fTurbulenceSpeed),
													0)* PPP.fFrameTime * (max(1.f,fAge))*
													m_pParams->fTurbulenceSize;
			}

      m_vDelta  += m_pParams->vGravity*m_fScale * PPP.fFrameTime;

			if (m_pParams->fSpeedAccel != 0)
			{
				if (!m_vDelta.IsZero())
				{
					Vec3 heading = GetNormalized(m_vDelta);
					m_vDelta += heading*m_pParams->fSpeedAccel*m_fScale * PPP.fFrameTime;
				}
			}
			if (m_pParams->fAirResistance != 0)
			{
				float fSpeed = GetLength(m_vDelta);
				if (fSpeed != 0)
				{
					Vec3 heading = m_vDelta * (1.0f/fSpeed);
					float fResistance = (fSpeed*fSpeed) * m_pParams->fAirResistance * PPP.fFrameTime; //D = Cd * A * .5 * r * V^2 
					if (fResistance > fSpeed)
						fResistance = fSpeed;
					m_vDelta -= heading*fResistance;
				}
			}

			if(m_pParams->pStatObj)
				m_vAngles += m_vRotation*PPP.fFrameTime*(180.0f/gf_PI);
			else
				m_vAngles.z += m_vRotation.z*PPP.fFrameTime*(180.0f/gf_PI);
    }

		if (m_pParams->fSizeSpeed != 0)
		{
			if(m_pParams->nParticleFlags & PART_FLAG_SIZE_LINEAR)
				m_fSize += (m_pParams->fSizeSpeed*m_fScale) * PPP.fFrameTime;
			else
			{
				float inc = (m_pParams->fSizeSpeed*m_fScale) * PPP.fFrameTime;;
				if (m_fSize > 1)
					inc = inc / m_fSize;
				m_fSize += inc;
			}
		}
  }

	//////////////////////////////////////////////////////////////////////////
	if (m_pParams->nParticleFlags & PART_FLAG_BIND_POSITION_TO_EMITTER)
	{
		m_vPos = m_pEmitter->m_pos;
	}

	//bool bScaleFadeInOut = false;
	float scaleFade = 1.0f;
	//////////////////////////////////////////////////////////////////////////
	// Process Scale FadeIn/FadeOut of particle.
	// Override m_fSize member.
	//////////////////////////////////////////////////////////////////////////
	if (m_pParams->fSizeFadeIn != 0)
	{
		// Fade in scale
		float t = fAge;
		if (t <= m_pParams->fSizeFadeIn)
		{
			scaleFade = (t/m_pParams->fSizeFadeIn);
			if (scaleFade < 0) scaleFade = 0;
			if (scaleFade > 1) scaleFade = 1;
			//bScaleFadeInOut = true;
			m_fSize = m_fSizeOriginal * scaleFade;
		}
	}
	if (m_pParams->fSizeFadeOut != 0)
	{
		// Fade out scale.
		float t = m_fLifeTime - fAge;
		if (t < m_pParams->fSizeFadeOut)
		{
			if (t < 0) t = 0;
			scaleFade *= (t/m_pParams->fSizeFadeOut);
			if (scaleFade < 0) scaleFade = 0;
			if (scaleFade > 1) scaleFade = 1;
			//m_fSize = m_fSizeOriginal * scaleOut;
			//bScaleFadeInOut = true;
			m_fSize = m_fSizeOriginal * scaleFade;
		}
	}

	// Restrict to minimal size.
	if (m_fSize < 0.001f)
		m_fSize = 0.0001f;

////////////////////////////////////////////////////////////////////////////////////////////////
// Simple collision with terrain simulation
////////////////////////////////////////////////////////////////////////////////////////////////

  bool bContact = false;
  if(m_pPhysEnt)
  { // use real physics
    pe_status_collisions status;
		coll_history_item item;
		status.pHistory = &item;
		status.age=1.f;
		status.bClearHistory=1;
    bContact = m_pPhysEnt->GetStatus(&status)!=0;
  }
	else if(m_vDelta.z<0)
  { 
    const float fFriction = 0.15f;

		/*
    if (m_pVisArea)
    { // if inside the building
      if(pCVars->e_particles_indoor_collisions)
      {
        IndoorRayIntInfo tRayInfo;
        bool bRes = pObjManager->m_pBuildingManager->RayIntersection(m_vPos-(m_vDelta*fFrameTime), m_vPos, tRayInfo, nBuildingID);
        if(bRes && m_vDelta.Dot(tRayInfo.vNormal)<0)
        { // if collides
          float fLen = m_vDelta.Length()-fFriction;

          if(fLen>fFriction)
          {
            m_vDelta = m_vDelta.Reflect(tRayInfo.vNormal);
            m_vDelta.SetLen(fLen*PPP.pPartSpray->m_fBouncenes);
          }
          else
            m_vDelta.Clear();

          bContact = true;
        }
      }
    }
    else*/
		if( PPP.pTerrain && _finite(m_vPos.x) && _finite(m_vPos.y)
				&& m_vPos.z < PPP.pTerrain->GetZApr(m_vPos.x,m_vPos.y)
				&& m_vPos.z > PPP.pTerrain->GetZApr(m_vPos.x,m_vPos.y)-0.1f
				&& !(m_pParams->nParticleFlags & PART_FLAG_SPACELOOP))
    { // collide with terrain
      int x = (int)m_vPos.x;
      int y = (int)m_vPos.y;
      if(!PPP.pTerrain->GetHoleSafe(x,y))
      if(!PPP.pTerrain->GetHoleSafe(x+CTerrain::GetHeightMapUnitSize(),y+CTerrain::GetHeightMapUnitSize()))
      if(!PPP.pTerrain->GetHoleSafe(x-CTerrain::GetHeightMapUnitSize(),y+CTerrain::GetHeightMapUnitSize()))
      if(!PPP.pTerrain->GetHoleSafe(x+CTerrain::GetHeightMapUnitSize(),y-CTerrain::GetHeightMapUnitSize()))
      if(!PPP.pTerrain->GetHoleSafe(x-CTerrain::GetHeightMapUnitSize(),y-CTerrain::GetHeightMapUnitSize()))
      {
        float fLen = (m_vDelta - m_pParams->vGravity*m_fScale*PPP.fFrameTime).Length()-fFriction;

        if(fLen>fFriction)
        {
          m_vDelta.z = -m_vDelta.z;
          m_vDelta.SetLen( fLen*m_pParams->fBouncenes);
        }
        else
        {
          m_vDelta(0,0,0);
          m_vRotation(0,0,0);
        }

        bContact = true;
      }
    }

    if(bContact && !IsEquivalent(m_vDelta,Vec3(0,0,0), 0.001f) )
    {
      Vec3d vVec(m_vDelta.x,m_vDelta.y,0);

      if(vVec.Normalize())
      {
				vVec = vVec.Cross(Vec3d(0,0,1));
        quaternionf q;
			  q=quaternionf(0,  vVec.x,vVec.y,vVec.z );

			  //CHANGED_BY_IVO (NOTE: order of angles is flipped!!!!)
			  //q.get_Euler_angles_xyz( PPP.pPartSpray->m_vRotation.z,PPP.pPartSpray->m_vRotation.y,PPP.pPartSpray->m_vRotation.x );
			  //EULER_IVO
			  //Vec3 TempAng;	PPP.pPartSpray->m_vRotation = q.GetEulerAngles_XYZ( TempAng );	
			  m_vRotation = Ang3::GetAnglesXYZ( matrix3x3f(q) );	
        m_vRotation *= m_pParams->fBouncenes*2;
      }
    }
  }

  if(bContact)
  { // Run child process on collision
    if (m_pParams->pChild && m_pParams->pChild->nCount > 0 && m_pParams->fChildSpawnPeriod < 0)
    {
			m_pParams->pChild->vPosition = m_vPos;
      //m_pParams->pChild->pStatObj = 0;
			m_pParams->pChild->vDirection = Vec3(0,0,1); // Up direction.
			PPP.pPartManager->Spawn( m_pEmitter,true );
      return false;
    }
  }
  
////////////////////////////////////////////////////////////////////////////////////////////////
// Run child process every X sec
////////////////////////////////////////////////////////////////////////////////////////////////

	if (m_pParams->pChild && m_pParams->pChild->nCount > 0 &&
			m_pParams->fChildSpawnPeriod >= 0 &&
			PPP.fCurTime > m_fChildSpawnLastTime + m_pParams->fChildSpawnPeriod)
	{
		if (m_pParams->fChildSpawnTime <= 0 || fAge < m_pParams->fChildSpawnTime)
		{
			// Calc current movement direction.
			if (m_vPos != prevPos)
				m_pParams->pChild->vDirection = GetNormalized(m_vPos - prevPos);
			m_pParams->pChild->vPosition = m_vPos;
			//PPP.pPartSpray->m_ChildParams.pChild=0;
			//PPP.pPartSpray->m_ChildParams.pStatObj=0;
			//    PPP.pPartSpray->m_pChildPartEmitter->Spawn( m_vPos );

			PPP.pPartManager->Spawn( m_pEmitter,true );
			m_fChildSpawnLastTime = PPP.fCurTime;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Remember prev positions for trail
	////////////////////////////////////////////////////////////////////////////////////////////////

	if (m_pArrvPosHistory)
	{
		//float fTailLength = (m_pParams->fTailLenght/m_nTailSteps) /  m_fScale; // Here we must divide by scale because speed is scaled.
		//m_fTrailCurPos += min(1.f, PPP.fFrameTime/fTailLength);
		//m_pArrvPosHistory[ ((unsigned int)FtoI(m_fTrailCurPos))%m_nTailSteps ] = m_vPos;

		/*
		float t = PPP.fCurTime-m_fLastTailPosTime;
		float dt = 0.3f;
		float fTailLength = (m_pParams->fTailLenght/PART_HISTORY_ELEMENTS)*m_fScale;
		if (t > dt)
		{
			//m_fTrailCurPos += min(1.f, PPP.fFrameTime/fTailLength);
			//m_pArrvPosHistory[int(m_fTrailCurPos) & PART_HISTORY_ID_MASK] = m_vPos;
			for (int i = 0; i < PART_HISTORY_ELEMENTS-1; i++)
			{
				m_pArrvPosHistory[i] = m_pArrvPosHistory[i+1];
			}
			m_pArrvPosHistory[PART_HISTORY_ELEMENTS-1] = m_vPos;
			m_fLastTailPosTime = PPP.fCurTime;
		}
		else
		{
			float a = t / dt; // from 0-1.
			a *= 0.05f;
			// Smooth last to prev.
			for (int i = 0; i < PART_HISTORY_ELEMENTS-1; i++)
			{
				Vec3 &last = m_pArrvPosHistory[i];
				Vec3 &prev = m_pArrvPosHistory[i+1];
//				//last = last*(1.0f-a) + (prev+Vec3(0,0,0))*a;
			}
		}
		*/
	}

////////////////////////////////////////////////////////////////////////////////////////////////
// Deactivate if too far or too int
////////////////////////////////////////////////////////////////////////////////////////////////

  if( (PPP.fCurTime/*+PPP.fFrameTime*/) >= (m_fSpawnTime + m_fLifeTime) )
    return false;
	else
	{
		if(m_pParams->nParticleFlags & PART_FLAG_UNDERWATER && m_pEmitter->m_fWaterLevel>WATER_LEVEL_UNKNOWN)
			if(m_vPos.z > m_pEmitter->m_fWaterLevel-m_fSize*0.5)
				return false;
	}

	if((m_pParams->nParticleFlags & PART_FLAG_SPACELIMIT) && m_pEmitter)
	{
		Vec3d vBoxMin = m_pEmitter->GetParams().vPosition - m_pEmitter->GetParams().vSpaceLoopBoxSize;
		Vec3d vBoxMax = m_pEmitter->GetParams().vPosition + m_pEmitter->GetParams().vSpaceLoopBoxSize;

		vBoxMin.CheckMin(m_pEmitter->GetParams().vPosition + m_pEmitter->GetParams().vSpaceLoopBoxSize);
		vBoxMax.CheckMax(m_pEmitter->GetParams().vPosition - m_pEmitter->GetParams().vSpaceLoopBoxSize);

		float fRadius = m_fSize;
		if(m_pParams->pStatObj)
		{ // get world space radius of the object
			float fObjScale = m_fSize;
			if (m_pParams->fObjectScale)
				fObjScale *= m_pParams->fObjectScale;
			fRadius = m_pParams->pStatObj->GetRadius()*fObjScale;
		}

		Vec3d vHitPos = m_vPos + m_vDelta.GetNormalized()*fRadius;

		// kill particle if position is out of bounds
		if(	vHitPos.x < vBoxMin.x ||
			vHitPos.y < vBoxMin.y ||
			vHitPos.z < vBoxMin.z ||
			vHitPos.x > vBoxMax.x ||
			vHitPos.y > vBoxMax.y ||
			vHitPos.z > vBoxMax.z )
			return false;
	}

	return true;
}

void CParticle::DeActivateParticle(IPhysicalWorld * pPhysicalWorld)
{
  if(m_pPhysEnt)
	{
    pPhysicalWorld->DestroyPhysicalEntity(m_pPhysEnt);
		m_pPhysEnt=0;
	}

	if (m_pEmitter)
	{
		m_pEmitter->Release();
		m_pEmitter = 0;
	}

	if (m_pArrvPosHistory)
	{
		delete []m_pArrvPosHistory;
		m_pArrvPosHistory = 0;
	}
	m_pSpawnerEntity = 0;
}

char GetMinAxis(const Vec3d & vVec)
{
	float x = fabs(vVec.x);
	float y = fabs(vVec.y);
	float z = fabs(vVec.z);

	if(x<y && x<z)
		return 'x';
	
	if(y<x && y<z)
		return 'y';
	
	return 'z';
}

void CParticle::Physicalize(  ParticleParams &Params,IPhysicalWorld * pPhysicalWorld)
{
	if (Params.nParticleFlags & PART_FLAG_RIGIDBODY)
	{
		if (!Params.pStatObj)
			return;

		float fObjScale = m_fSize;
		if (m_pParams->fObjectScale)
			fObjScale *= m_pParams->fObjectScale;

		// Make Physical Rigid Body.
		pe_params_pos par_pos;
		par_pos.pos = m_vPos;
		par_pos.q.SetRotationXYZ( m_vAngles*(gf_PI/180.0f) );
		m_pPhysEnt = pPhysicalWorld->CreatePhysicalEntity(PE_RIGID,&par_pos,0,OT_RIGID_PARTICLE );

		pe_params_flags pf;
		pf.flagsOR = pef_never_affect_triggers;
		m_pPhysEnt->SetParams(&pf);

		pe_geomparams partpos;
		//partpos.pos.Set(0,0,0);
		partpos.density = Params.fThickness;
		partpos.scale = fObjScale;
		partpos.flags &= ~geom_colltype3; // don't collide with vehicles
		partpos.flagsCollider = geom_colltype4;
		m_pPhysEnt->AddGeometry( Params.pStatObj->GetPhysGeom(),&partpos,0 );
		
		pe_simulation_params symparams;
		//symparams.damping = 0.3f;
		//symparams.dampingFreefall = 0.3f;
		//symparams.gravity = Params.vGravity*m_fScale;
		symparams.minEnergy = (0.2f)*(0.2f);
		//symparams.softness = symparams.softnessGroup = 0.003f;
		//symparams.softnessAngular = symparams.softnessAngularGroup = 0.01f;
		//symparams.maxTimeStep = 0.05f;
		m_pPhysEnt->SetParams(&symparams);

		pe_action_set_velocity velparam;
		velparam.v = m_vDelta;
		velparam.w = m_vRotation;
		m_pPhysEnt->Action(&velparam);
	}
	else
	{
		// Make Physical Particle.
		pe_params_pos par_pos;
		par_pos.pos = m_vPos;
		m_pPhysEnt = pPhysicalWorld->CreatePhysicalEntity(PE_PARTICLE,&par_pos);
		pe_params_particle params;

		// Take particles size from bounding box of piece.

		float fPartSize = 1.0f;
		if (Params.pStatObj)
		{
			Vec3 vSize = Params.pStatObj->GetBoxMax() - Params.pStatObj->GetBoxMin();
			fPartSize = max(max(vSize.x,vSize.y),vSize.z);
		}

		params.mass = 0.1f;
		params.size = m_fSize*fPartSize;
		if (Params.pStatObj)
		{
			params.size = Params.pStatObj->GetRadius()+0.05f;
			if(m_pParams->fObjectScale)
				params.size *= Params.fObjectScale;
		}

		params.thickness = Params.fThickness ? Params.fThickness : params.size*0.5f;
		params.heading = GetNormalized(m_vDelta);
		params.velocity = m_vDelta.Length();
		params.wspin = m_vRotation*0.25f;
		params.q0.SetRotationXYZ( m_vAngles*(gf_PI/180.0f) );
		
		if(!Params.vNormal.IsZero())
			params.normal = Params.vNormal;
		else if(Params.pStatObj)
		{
			Vec3d vSize = Params.pStatObj->GetBoxMax() - Params.pStatObj->GetBoxMin();
			char cMinAxis = GetMinAxis(vSize);
			params.normal = Vec3d((cMinAxis=='x')?1.f:0,(cMinAxis=='y')?1.f:0,(cMinAxis=='z')?1.f:0);
		}

		params.surface_idx = Params.iPhysMat;
		params.gravity = Params.vGravity*m_fScale;
		params.flags = particle_no_roll|particle_no_path_alignment;
		//	params.acc_lift=0;
		//	params.k_air_resistance=0;
		//	params.acc_thrust=0;
		//	params.acc_lift=0;
		//	params.wspin = PPP.pPartSpray->m_vRotation;

		//	params.q0.set(0,0,0);
		//	params.collider_to_ignore=0;

		m_pPhysEnt->SetParams(&params);
	}
}
