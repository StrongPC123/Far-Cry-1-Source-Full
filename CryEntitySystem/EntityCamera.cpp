////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   EntityCamera.cpp
//  Version:     v1.00
//  Created:     14/8/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Entity Camera implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "EntityCamera.h"
#include "Entity.h"

#include <ISystem.h>

#ifdef _DEBUG
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

void CEntityCamera::Init(IPhysicalWorld * pIPhysWorld, UINT iWidth, UINT iHeight, IConsole *pConsole)
{
	m_camera.Init(iWidth, iHeight);
	m_pIPhysWorld = pIPhysWorld;
//	m_sParam.m_cam_kstiffness = 150;
	m_sParam.m_cam_kstiffness = 15;
	m_sParam.m_cam_kdamping = 2*(float)(cry_sqrtf(m_sParam.m_cam_kstiffness));
//	m_sParam.m_cam_dir(0,17.0f,0.1f); 
//	m_sParam.m_cam_dir(0,2,0.5); 
	m_sParam.m_cam_dir(0,1,0.1f); 
	m_sParam.m_cam_dir.Normalize();
	m_sParam.m_cam_dist = 1.5f;
	m_sParam.m_cur_cam_dist = m_sParam.m_cur_cam_dangle = 0;
	m_sParam.m_cam_angle_flags = 5;
	m_sParam.m_cam_angle_kstiffness = 20; 
	m_sParam.m_cam_angle_kdamping = 2*(float)(cry_sqrtf(m_sParam.m_cam_angle_kstiffness));
	m_sParam.m_cur_cam_dangle_vel = m_sParam.m_cur_cam_vel = 0;
	m_sParam.m_1pcam_butt_pos(0,0,0); m_sParam.m_1pcam_eye_pos(0,0,0);
	m_sParam.m_camoffset(0,0,0);
	m_sParam.m_viewoffset = 0;

	m_pConsole = pConsole;
	/*
	m_pOne = pConsole->CreateVariable("zed","0",0);
	m_pTwo = pConsole->CreateVariable("why","0",0);
	m_pThree = pConsole->CreateVariable("eks","0",0);
	*/
	m_fTimeIdle = 10.0f;
	m_fDeltaDist = 0.0f;
}

void CEntityCamera::SetThirdPersonMode( const Vec3d &center,const Vec3d &angles0,int mode,float frameTime,float range,int dangleAmmount,
									   IPhysicalEntity *physic, IPhysicalEntity *physicMore,
									   I3DEngine* p3DEngine, float safeRange)
{
	ICVar *pVar = m_pConsole->GetCVar("es_PiercingCamera");
	int bPiercing = pVar && pVar->GetIVal();


  if (mode == CAMERA_3DPERSON1) 
  { 

//remove this after vehicle camera tweaking is done
// put the final value in constructor
pVar = m_pConsole->GetCVar("ThirdPersonAngle");
float	dirZ = pVar->GetFVal();
m_sParam.m_cam_dir(0,1,dirZ); 
m_sParam.m_cam_dir.Normalize();

		vectorf tview, dir, camdir, rotax,camdiry, campos;
		tview.x=center.x;
		tview.y=center.y;
		tview.z=center.z;
		camdir.x=m_sParam.m_cam_dir.x;
		camdir.y=m_sParam.m_cam_dir.y;
		camdir.z=m_sParam.m_cam_dir.z;
		float cam_dist, cam_kstiffness,cam_kdamping,cam_angle_kstiffness,cam_angle_kdamping;
		matrix3x3bf mtx;
		int b1p=0;
		if (range<3 && GetLengthSquared((m_sParam.m_1pcam_eye_pos-m_sParam.m_1pcam_butt_pos))>0) {

			//note: order of angles is flipped!
			//quaternionf qobj = quaternionf(angles.z*(PI/180.0f),angles.y*(PI/180.0f),angles.x*(PI/180.0f));
			quaternionf qobj=GetRotationXYZ<float>( angles0*(gf_PI/180.0f) );
			
			//Q2M_IVO
			//qobj.getmatrix(mtx);
			mtx=matrix3x3f(qobj);

			tview += mtx*vectorf(m_sParam.m_1pcam_butt_pos);
			camdir = mtx*vectorf(m_sParam.m_1pcam_eye_pos-m_sParam.m_1pcam_butt_pos);
			cam_dist = camdir.len();
			camdir /= cam_dist;
			cam_kstiffness=cam_kdamping=0;
			cam_angle_kstiffness=cam_angle_kdamping=0;
			b1p = 1;
		} else {
			pe_status_living status;
  		if (physic && physic->GetStatus(&status))
  			tview += status.camOffset;
			camdir = m_sParam.m_cam_dir;
			cam_dist = range;//m_cam_dist;
			cam_kstiffness=m_sParam.m_cam_kstiffness; cam_kdamping=m_sParam.m_cam_kdamping;
			cam_angle_kstiffness=m_sParam.m_cam_angle_kstiffness; cam_angle_kdamping=m_sParam.m_cam_angle_kdamping;
		}
		dir=vectorf(GetPos())-tview;

		Quat q(1, 0,0,0);
		Vec3 angles = angles0;
		if (m_sParam.m_cam_angle_flags & 4) q = GetRotationAA(angles.x*(gf_PI/180.0f),vectorf(1,0,0));
		if (m_sParam.m_cam_angle_flags & 2) q = GetRotationAA(angles.y*(gf_PI/180.0f),vectorf(0,1,0))*q;
		
		ray_hit hit;
		if (!b1p && safeRange>0)
		{
			static float g_fYawFixups[] = { 0,180,90,-90,0 };
			Quat qYaw;
			int i=0;
			do 
			{
				qYaw = GetRotationAA((angles0.z+g_fYawFixups[i])*(gf_PI/180.0f),vectorf(0,0,1));
			}	
			while (m_pIPhysWorld->RayWorldIntersection(tview,qYaw*q*camdir*safeRange, ent_all&~ent_independent, rwi_stop_at_pierceable,&hit,1, physic, physicMore) &&
						 ++i<sizeof(g_fYawFixups)/sizeof(g_fYawFixups[0])-1);
			angles.z = angles0.z + g_fYawFixups[i];
		}
	if (m_sParam.m_cam_angle_flags & 1) q = GetRotationAA(angles.z*(gf_PI/180.0f),vectorf(0,0,1))*q;
  	
	float dt=frameTime, dist=dir.len(), dangle;//a,newdist;
	float e,v0,C1,C2;
	if (dt>0.1f) dt=0.1f;	
	if (dist<0.001f) dist=0.001f,dir(0,0,1);
	else dir/=dist;
	if (!b1p) {
		//Q2M_IVO 
		//q.getmatrix(mtx);
		mtx=matrix3x3f(q);
  	camdir = mtx*vectorf(m_sParam.m_cam_dir);
	}

  	rotax = (camdir^dir).normalize();	camdiry = rotax^camdir;
  	dangle = cry_acosf(min(1.0f,max(-1.0f,camdir*dir)));
  	Vec3 camdirTmp = camdir*(float)(cry_cosf(dangle))+camdiry*(float)(cry_sinf(dangle));
	Vec3 camposTmp = tview+camdirTmp*dist;
	if (!b1p) {
//		dir = (camposTmp-tview).normalize()*0.5;	camposTmp += dir;
		if (!bPiercing && m_pIPhysWorld->RayWorldIntersection(tview,camposTmp-tview,
			ent_static | ent_sleeping_rigid | ent_rigid | ent_terrain, 
//			rwi_stop_at_pierceable,
			rwi_ignore_noncolliding,
			&hit,1, physic, physicMore))
		{
			if(cam_dist<hit.dist)
				m_fTimeIdle = 0.0f;
			m_fDeltaDist = dist - hit.dist;
			dist = hit.dist;
//			m_sParam.m_cur_cam_vel = 20.0f;
//			campos = hit.pt;
		}
		else
			m_fDeltaDist = 0.0f;
//		if ((campos-tview).len2()>sqr(0.75))
//			campos -= dir;
	}

  	if (dangle>gf_PI/2)
  		dangle = gf_PI/2;
	if (cam_angle_kdamping==0) 
		dangle = 0;
	else {
		e = (float) cry_expf(-cam_angle_kdamping*0.5f*dt);
		v0 = m_sParam.m_cur_cam_dangle_vel;// + (dangle-m_sParam.m_cur_cam_dangle)/dt;
		C1 = dangle;
		C2 = v0 + C1*cam_angle_kdamping*0.5f;
		m_sParam.m_cur_cam_dangle = dangle = C1*e + C2*dt*e;
		m_sParam.m_cur_cam_dangle_vel = -C1*cam_angle_kdamping*0.5f*e + C2*(e-cam_angle_kdamping*0.5f*dt*e);
	}
	m_fTimeIdle += dt;
	if (cam_kstiffness==0 || dist>50)
	  	dist = cam_dist;
	else {
		if(m_fDeltaDist>2.0f)
			m_sParam.m_cur_cam_vel = 20.0f;
		C1 = dist-cam_dist;
		if(m_fTimeIdle>1.5f || m_fDeltaDist>.1f || C1>2.0f)
		{
			e = (float) cry_expf(-cam_kdamping*0.5f*dt);
			v0 = m_sParam.m_cur_cam_vel;// - (dist-m_sParam.m_cur_cam_dist)/dt;
//			C1 = dist-cam_dist;
			C2 = v0 + C1*cam_kdamping*0.5f;
			m_sParam.m_cur_cam_dist = dist = cam_dist + C1*e + C2*dt*e;
			m_sParam.m_cur_cam_vel = -C1*cam_kdamping*0.5f*e + C2*(e-cam_kdamping*0.5f*dt*e);
		}
		else
		{
			cam_dist = dist;
		}
	}
  	camdir = camdir*(float)(cry_cosf(dangle))+camdiry*(float)(cry_sinf(dangle));
  	campos = tview+camdir*dist;

//*
	if (!b1p && m_fDeltaDist==0.0f) {
		dir = (campos-tview).normalize()*0.5;	campos += dir;
		if (!bPiercing && m_pIPhysWorld->RayWorldIntersection(tview,campos-tview,
			ent_static | ent_sleeping_rigid | ent_rigid | ent_terrain, 
//			rwi_stop_at_pierceable,
			rwi_ignore_noncolliding,
			&hit,1, physic, physicMore))
		{
			m_fTimeIdle = 0.0f;
			campos = hit.pt;
		}
		if ((campos-tview).len2()>sqr(0.75))
			campos -= dir;
	}
//*/
  	SetPos((Vec3d) campos);
//	m_camera.m_OldPosition = campos;
  	Vec3d cam_angles;	
	if (b1p) {
		cam_angles.z = angles.z-180;
		cam_angles.x = -angles.x;
		cam_angles.y = -angles.y;
	} else {
  		cam_angles.z = (float)(-cry_atan2f(camdir.x,camdir.y));
  		cam_angles.x = (float)(cry_atan2f(camdir.z,cry_sqrtf(sqr(camdir.x)+sqr(camdir.y))));
  		cam_angles.y = 0;
  		cam_angles *= 180.0f/gf_PI;
	}

  	SetAngles(cam_angles);
		
  }	
  else 
  if (mode == CAMERA_3DPERSON2)
  {
		CryError( "<EntitySystem> Camera in CAMERA_3DPERSON2 mode" );

    Vec3d tview=center;
	  pe_status_living status;
	  if (physic && physic->GetStatus(&status))
		  tview += (Vec3d)status.camOffset;
    Vec3d ang = GetAngles();
		int dangle = dangleAmmount;
		dangle = 90*(dangle-1&3); if (dangle>180) dangle-=360;
    tview.x+= - range*cry_sinf(DEG2RAD(ang[ROLL]+dangle));
    tview.y+= range*cry_cosf(DEG2RAD(ang[ROLL]+dangle));
    tview.z+=1.0f;
    ang.z=angles0.z+90;
		SetAngles(ang);
    SetPos(tview);
  }
	else
	{
		//center is where we want to look
		// angles is where our target looks


		Vec3d mycenter=center;
		mycenter.z+=1.7f;
		Vec3d offset;
		Vec3d myangles;

		//Matrix44 mtx;
		//mtx.Identity();
		//mtx=GetTranslationMat(center)*mtx;
		//mtx=GetRotationZYX44(-gf_DEGTORAD*angles)*mtx; //NOTE: angles in radians and negated 
    
		//OPTIMISED_BY_IVO  
		Matrix44 mtx=Matrix34::CreateRotationXYZ( Deg2Rad(angles0),center);
		mtx=GetTransposed44(mtx);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44


		Vec3d vBpos=mtx.TransformPointOLD(offset);		


		vectorf start, end;
		ray_hit hit;
		
		// so this pos is where we want to be
		// lets move toward it slowly

		Vec3d currpos = GetPos();

		start.Set(mycenter.x,mycenter.y,mycenter.z);
		end.Set(vBpos.x,vBpos.y,vBpos.z);

		if (!bPiercing && m_pIPhysWorld->RayWorldIntersection(start,end-start,ent_terrain|ent_static, rwi_stop_at_pierceable,&hit,1, physic))
				vBpos = (Vec3d)hit.pt;
		
		Vec3d finalpos;

		finalpos = currpos + (vBpos - currpos)*0.1f;

		SetPos(finalpos);
		

		myangles = mycenter - finalpos;
		myangles=ConvertVectorToCameraAngles(myangles);

	
		SetAngles(myangles);
	}
	
}


void CEntityCamera::SetCameraMode(const Vec3d &lookat, const Vec3d &lookat_angles, IPhysicalEntity *physic)
{

		Vec3d mycenter=lookat;
		mycenter.z+=1.7f;
		Vec3d offset = m_vCameraOffset;
		Vec3d myangles;


		//Matrix44 mtx;
		//mtx.Identity();
		//mtx=GetTranslationMat(lookat)*mtx;
		//mtx=GetRotationZYX44(-gf_DEGTORAD*lookat_angles)*mtx; //NOTE: angles in radians and negated 

		//OPTIMISED_BY_IVO  
		Matrix44 mtx=Matrix34::CreateRotationXYZ( Deg2Rad(lookat_angles),lookat);
		mtx=GetTransposed44(mtx);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44


		Vec3d vBpos=mtx.TransformPointOLD(offset);		


		vectorf start, end;
		ray_hit hit;
		
		// so this pos is where we want to be
		// lets move toward it slowly

		Vec3d currpos = GetPos();

		start.Set(mycenter.x,mycenter.y,mycenter.z);
		end.Set(vBpos.x,vBpos.y,vBpos.z);

		if (m_pIPhysWorld->RayWorldIntersection(start,end-start,ent_terrain | ent_static, rwi_stop_at_pierceable,&hit,1, physic))
				vBpos = (Vec3d)hit.pt + (mycenter - ((Vec3d)hit.pt))*0.1f;
		
		Vec3d finalpos;

		finalpos = currpos + (vBpos - currpos)*0.1f;

		SetPos(finalpos);
		

		myangles = mycenter - finalpos;
		myangles=ConvertVectorToCameraAngles(myangles);

	
		SetAngles(myangles);
}
