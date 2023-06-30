// Scale the direction vsector based on fDirVecScale
//  v3SysDir *= fDirVecScale;// ScriptObjectParticle.cpp: implementation of the CScriptObjectParticle class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptObjectParticle.h"
#include "ScriptObjectVector.h"
#include <ISystem.h>
#include <I3DEngine.h>
#include <IEntitySystem.h>
#include <CryParticleSpawnInfo.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

_DECLARE_SCRIPTABLEEX(CScriptObjectParticle)

CScriptObjectParticle::CScriptObjectParticle()
{
}

CScriptObjectParticle::~CScriptObjectParticle()
{
}

void CScriptObjectParticle::Init(IScriptSystem *pScriptSystem, ISystem *pSystem)
{
	m_pSystem=pSystem;
	m_p3DEngine =m_pSystem->GetI3DEngine();
	InitGlobal(pScriptSystem,"Particle",this);
	
}

void CScriptObjectParticle::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectParticle>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectParticle,CreateParticle);
	REG_FUNC(CScriptObjectParticle,CreateParticleLine);
	REG_FUNC(CScriptObjectParticle,SpawnEffect);
	REG_FUNC(CScriptObjectParticle,CreateDecal);
	REG_FUNC(CScriptObjectParticle,Attach);
	REG_FUNC(CScriptObjectParticle,Detach);

	pSS->SetGlobalValue("CRYPARTICLE_ONE_TIME_SPAWN",CryParticleSpawnInfo::FLAGS_ONE_TIME_SPAWN);
	pSS->SetGlobalValue("CRYPARTICLE_RAIN_MODE",CryParticleSpawnInfo::FLAGS_RAIN_MODE);
	
}

/*!create a particle source
	@param v3Pos a table with the x,y,z fileds specifing the origin of the particle source
	@param v3SysDir a table with the x,y,z fileds specifing the direction of the particle source(in degrees)
	@param pObj a table specifing the particle parameters
		particle={
			focus
			start_color
			end_color
			speed
			rotation
			count
			size
			size_speed
			gravity
			lifetime
			fadeintime
			frames
			tid
			particle_type
			tail_length
			physics
			draw_las
			color_based_blending
			blend_type
			bouncyness
			init_angles
			dir_vec_scale
			stat_obj_entity_id
			stat_obj_slot
			stat_obj_count
			//for child process
			ChildProcess={
      		//all parameters as a normal particle source
			}
			ChildSpawnPeriod
		}
*/
int CScriptObjectParticle::CreateParticle(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
	_SmartScriptObject  pObj(m_pScriptSystem,true);
	_SmartScriptObject  pChildObj(m_pScriptSystem,true);
	CScriptObjectVector oVec(m_pScriptSystem,true);
	Vec3 v3Pos,v3SysDir,v3Offset(0,0,0);
	static ParticleParams sParam;

	if(!pH->GetParam(1,*oVec))
		m_pScriptSystem->RaiseError( "<CreateParticles> parameter 1 not specified or nil(pos)" );
	v3Pos=oVec.Get();
	if(!pH->GetParam(2,*oVec))
		m_pScriptSystem->RaiseError( "<CreateParticles> parameter 2 not specified or nil(normal)" );
	v3SysDir=oVec.Get(); 
	if(!pH->GetParam(3,*pObj))
		m_pScriptSystem->RaiseError( "<CreateParticles> parameter 3 not specified or nil(perticle struct)" );


	ReadParticleTable(*pObj, sParam);
	sParam.vPosition = v3Pos;
	sParam.vDirection = v3SysDir;
	pObj->BeginSetGetChain();
	if ((sParam).fChildSpawnPeriod && pObj->GetValueChain("ChildProcess", *pChildObj))
	{
		ParticleParams sChildParams;
		ReadParticleTable(*pChildObj, sChildParams);
		sParam.pChild = &sChildParams;
		if(!pObj->GetValueChain( "ChildSpawnPeriod",sParam.fChildSpawnPeriod ))            
			sParam.fChildSpawnPeriod=0;
	}
	else
		sParam.pChild = NULL;


	//STATIC OBJECT BASED PARTICLES////////////////////////////////////////////////////

	sParam.pStatObj = NULL;
	INT_PTR nValue=0;
	int nCookie=0;
	if(pObj->GetUDValueChain("geometry",nValue,nCookie) && (nCookie==USER_DATA_OBJECT))
	{
		sParam.pStatObj=(IStatObj *)nValue;
	}
	sParam.vPosition = v3Pos;
	m_p3DEngine->SpawnParticles(sParam);

	pObj->EndSetGetChain();

	return pH->EndFunction();
}
/*!create a decal
	@param v3Pos a table with the x,y,z fileds specifing the origin of the particle source
	@param v3SysDir a table with the x,y,z fileds specifing the direction of the particle source(in degrees)
	@param size the size of the decal
	@param tid the texture
	@param fAngle rotation in degrees[optional]
*/
int CScriptObjectParticle::CreateDecal(IFunctionHandler *pH)
{

	if (pH->GetParamCount() < 5)
	{
		m_pScriptSystem->RaiseError("CreateDecal: Need at least 5 params");
		return pH->EndFunction();;
	}
	
	//CHECK_PARAMETERS(5);
	CScriptObjectVector oVec(m_pScriptSystem,true);

	CryEngineDecalInfo Decal;
	int nCookie=0;
	USER_DATA nUD=0;
	IStatObj *obj=NULL;
	pH->GetParam(1,*oVec);
	Decal.vPos=oVec.Get();
	pH->GetParam(2,*oVec);
	Decal.vNormal=oVec.Get();
	pH->GetParam(3,Decal.fSize);
	pH->GetParam(4,Decal.fLifeTime);
	pH->GetParamUDVal(5,Decal.nTid,nCookie);
	if(pH->GetParamCount()>=6)
	{
		if(pH->GetParamUDVal(6,nUD,nCookie))
		{
			if(nCookie==USER_DATA_OBJECT)
			{
				obj=(IStatObj *)nUD;
			}
		}
	}
	if(pH->GetParamCount()>=7)
	{
		pH->GetParam(7, Decal.fAngle);
	}

	if(pH->GetParamCount()>=8)
	{
		pH->GetParam(8,*oVec);
		Decal.vHitDirection = oVec.Get();
	}

	int ownerID = 0;
	if(pH->GetParamCount()>=9)
	{
		pH->GetParam(9, ownerID);
	}
	IEntity* owner=m_pSystem->GetIEntitySystem()->GetEntity( ownerID );

	if(pH->GetParamCount()>=10)
	{
		pH->GetParam(10, Decal.nPartID);
	}

	Decal.pDecalOwner = owner;
	Decal.pStatObj = obj;

	// get IEntityRender of static object decal owner
	if(pH->GetParamCount()>=11)
	{
	ULONG_PTR ptr = 0;
	int nCookie;
		if (!pH->GetParamUDVal(11, ptr, nCookie))
			ptr = 0;
		if (nCookie != USER_DATA_POINTER)
			ptr = 0;
		Decal.pDecalOwner = (IEntityRender*)ptr;
	}

	m_p3DEngine->CreateDecal(Decal);
	
	return pH->EndFunction();
}

int CScriptObjectParticle::CreateParticleLine(IFunctionHandler *pH)
{
CHECK_PARAMETERS(4);
	_SmartScriptObject  pObj(m_pScriptSystem,true);
	CScriptObjectVector oVec(m_pScriptSystem,true);
	CScriptObjectColor  oCol(m_pScriptSystem,true);
	Vec3 v3Pos,v3SysDir,v3Offset(0,0,0), vSpaceLoopBoxSize(0,0,0);

	float fDensity;

	pH->GetParam(1,*oVec);
	v3Pos=oVec.Get();
	pH->GetParam(2,*oVec);
	v3SysDir=oVec.Get(); 
	pH->GetParam(3,*pObj);
	pH->GetParam(4,fDensity);

	float   focus = 0;
  Vec3   col;
	Vec3  vcol(0,0,0);//dummy
    
  float speed    = 0;
  int	  count    = 0;

  float size       = 1;
  float size_speed = 0;
  
  float gravity    = 0;	
  float lifetime   = 0;
  
  int	  tid      = 0;
  int	  frames   = 0;
  
  int	  color_based_blending = 0;	

  int iParticleType = 0;

  float fTailLength = 0.0f;

  int bRealPhys = 0;
	
	if(!pObj->GetValue( "color",*oCol ))            
    m_pScriptSystem->RaiseError( "<CreateParticles> color field not specified" );
	vcol = oCol.Get();
	if(!pObj->GetValue( "focus",focus ))            
    m_pScriptSystem->RaiseError( "<CreateParticles> focus field not specified" );
	if(!pObj->GetValue( "speed",speed ))            
    m_pScriptSystem->RaiseError( "<CreateParticles> speed field not specified" );
	if(!pObj->GetValue( "count",count ))            
    m_pScriptSystem->RaiseError( "<CreateParticles> count field not specified" );
	if(!pObj->GetValue( "size" ,size  ))            
    m_pScriptSystem->RaiseError( "<CreateParticles> size field not specified"  );

	pObj->GetValue( "size_speed",size_speed ) ;
  
	if(!pObj->GetValue( "gravity",gravity ))        
    m_pScriptSystem->RaiseError( "<CreateParticles> gravity field not specified" );
	if(!pObj->GetValue( "lifetime",lifetime )){      
    //m_pScriptSystem->RaiseError( "<CreateParticles> lifetime field not specified" );
		lifetime=0;
	}
	pObj->GetValue( "frames",frames );
   
	if(!pObj->GetValue( "color_based_blending",color_based_blending )) 
    m_pScriptSystem->RaiseError( "<CreateParticles> color_based_blending field not specified" );

	int nCookie=0;
	pObj->GetValue( "tid",tid );

	if(!pObj->GetValue( "particle_type", iParticleType ))
		iParticleType = PART_FLAG_BILLBOARD;
	if(!pObj->GetValue( "tail_length", fTailLength))
		fTailLength = 0.0f;
	if(!pObj->GetValue( "physics", bRealPhys ))
		bRealPhys = 0;
	if(pObj->GetValue("offset",*oVec))
		v3Offset=oVec.Get();
	if(pObj->GetValue("space_box",*oVec))
		vSpaceLoopBoxSize=oVec.Get();


	float fBouncenes;
  if(!pObj->GetValue( "bouncyness", fBouncenes))
		fBouncenes = 0.5f;

	color_based_blending = (color_based_blending == 3) ? 1 : 0;	

	col = vcol;
	Vec3 currentpos;
	v3SysDir-=v3Pos;
	float finc = 1.f / (v3SysDir.Length() / fDensity) ;
	float t = 0;
	while (t<1.0f)
	{

		currentpos = (v3Pos + t*v3SysDir)+v3Offset;
		t+=finc;

		ParticleParams Params;
		Params.vPosition = currentpos;
		Params.vDirection = Vec3(0,0,1);
		Params.fFocus = focus;
		Params.vColorStart = col;
		Params.vColorEnd = col;
		Params.fSpeed = speed;
		Params.nCount = count;
		Params.fSize = size;
		Params.fSizeSpeed = size_speed;
		Params.vGravity = Vec3 (0,0,-gravity);
		Params.fLifeTime = lifetime;
		Params.nTexId = tid;
		Params.nTexAnimFramesCount = frames;
		Params.eBlendType = color_based_blending>0 ? ParticleBlendType_ColorBased : ParticleBlendType_AlphaBased;
		Params.nParticleFlags = iParticleType;
		Params.bRealPhysics = 0;
		Params.pChild = NULL;
		Params.fChildSpawnPeriod = 0;
		Params.fTailLenght = fTailLength;
		Params.bRealPhysics = bRealPhys != 0;
		Params.fBouncenes = fBouncenes;
		Params.vSpaceLoopBoxSize = vSpaceLoopBoxSize;

		m_p3DEngine->SpawnParticles(Params);
	}

	return pH->EndFunction();
}


bool CScriptObjectParticle::ReadParticleTable(IScriptObject *pITable, ParticleParams &sParamOut)
{
	CScriptObjectColor  oCol(m_pScriptSystem,true);
	Vec3 v3Pos,v3Offset(0,0,0);
	CScriptObjectVector oVec(m_pScriptSystem,true);

	//default params
	float   focus = 0;
	Vec3  vStartColor(1,1,1);
	Vec3  vEndColor(1,1,1);
	Vec3  vRotation(0,0,0);
	Vec3  vGravity(0,0,0);
  float speed    = 0;
  int	  count    = 1;
	float size       = 0.05f;
  float size_speed = 0;
  float gravity    = 0;	
  float lifetime   = 0;
  float fadeintime = 0;
  INT_PTR tid      = 0;
  int	  frames   = 0;
  int draw_last  = 0;
  int  blendType = ParticleBlendType_AlphaBased;
  int	 color_based_blending = 0;	
  int iParticleType = 0;
  float fTailLength = 0.0f;
  int bRealPhys = 0;
  float fDirVecScale = 1.0f;
	int nEntityID=0;
	if(!pITable->BeginSetGetChain())
		return false;
//FOCUS////////////////////////////////////
	if(!pITable->GetValueChain( "focus",focus ))            
    m_pScriptSystem->RaiseError( "<CreateParticles> focus field not specified" );
//START COLOR////////////////////////////////
	if (pITable->GetValueChain( "start_color",oCol ))
		vStartColor = oCol.Get();
//END COLOR////////////////////////////////
	if (pITable->GetValueChain( "end_color",oCol ))
		vEndColor = oCol.Get();
//SPEED////////////////////////////////
	if(!pITable->GetValueChain( "speed",speed ))            
    m_pScriptSystem->RaiseError( "<CreateParticles> speed field not specified" );
//ROTATION////////////////////////////////
	if (pITable->GetValueChain( "rotation",oVec ))
		vRotation = oVec.Get();
//COUNT////////////////////////////////
	if(!pITable->GetValueChain( "count",count ))            
    m_pScriptSystem->RaiseError( "<CreateParticles> count field not specified" );
//SIZE////////////////////////////////
	if(!pITable->GetValueChain( "size" ,size  ))            
    m_pScriptSystem->RaiseError( "<CreateParticles> size field not specified"  );
//SIZE SPEED////////////////////////////////
	if(!pITable->GetValueChain( "size_speed",size_speed ))  
    size_speed=0;
//GRAVITY////////////////////////////////
	if (pITable->GetValueChain( "gravity",oVec ))        
    vGravity = oVec.Get();
//LIFETIME////////////////////////////////
	if(!pITable->GetValueChain( "lifetime",lifetime ))      
    m_pScriptSystem->RaiseError( "<CreateParticles> lifetime field not specified" );
//FADEINTIME////////////////////////////////
	if(!pITable->GetValueChain( "fadeintime",fadeintime ))      
    fadeintime=0;//m_pScriptSystem->RaiseError( "<CreateParticles> fadeintime field not specified" );
//FRAMES////////////////////////////////
	if(!pITable->GetValueChain( "frames",frames ))          
    frames=0;
//TID////////////////////////////////
	int nCookie=0;
	if(!pITable->GetUDValueChain( "tid",tid,nCookie)) 
		tid=0;
//PARTICLE TYPE////////////////////////////////
	if(!pITable->GetValueChain( "particle_type", iParticleType ))
		iParticleType = PART_FLAG_BILLBOARD;
//TAIL LENGHT////////////////////////////////
	if(!pITable->GetValueChain( "tail_length", fTailLength))
		fTailLength = 0.0f;
//PHYSICS////////////////////////////////
  if(!pITable->GetValueChain( "physics", bRealPhys ))
		bRealPhys = 0;
//DRAW LAST////////////////////////////////
	if(!pITable->GetValueChain( "draw_last",draw_last ))
    draw_last=0;
//COLOR BASED BLENDING (legacy)////////////////////////////////
	if (pITable->GetValueChain( "color_based_blending",color_based_blending ))
	{
		// This for backward compatability.
		if (color_based_blending == 3)
			blendType = ParticleBlendType_ColorBased;
	}
//BLEND TYPE////////////////////////////////
	// Read particles blending type.
	pITable->GetValueChain( "blend_type",blendType );
//BOUNCENES/////////////////////////////////
	float fBouncenes;
  if(!pITable->GetValueChain( "bouncyness", fBouncenes))
		fBouncenes = 0.5f;
//INIT ANGLE/////////////////////////////////
	Vec3  vAngles(0,0,0);
	if (pITable->GetValue( "init_angles",oVec ))        
    vAngles = oVec.Get();
//bounding box size/////////////////////////////////
	Vec3  vSpaceLoopBoxSize(0,0,0);
	if (pITable->GetValue("space_box",*oVec))
		vSpaceLoopBoxSize=oVec.Get();

	pITable->GetValueChain( "dir_vec_scale", fDirVecScale );

	// turbulence
	float fTurbulenceSize=0;
  pITable->GetValueChain( "turbulence_size", fTurbulenceSize);
	float fTurbulenceSpeed=0;
  pITable->GetValueChain( "turbulence_speed", fTurbulenceSpeed);
	int nLinearSizeSpeed=0;
  pITable->GetValueChain( "bLinearSizeSpeed", nLinearSizeSpeed);

	sParamOut.fPosRandomOffset=0;
	pITable->GetValueChain( "fPosRandomOffset", sParamOut.fPosRandomOffset);

	float fChildSpawnPeriod=0;
  pITable->GetValueChain( "ChildSpawnPeriod", fChildSpawnPeriod);

	float fAirResistance=0;
	pITable->GetValueChain( "AirResistance", fAirResistance);

	// after this line GetValueChain will crash
	pITable->EndSetGetChain();

//////////////////////////////////////////////////////////////////////////////////////
  sParamOut.fFocus = focus;
  sParamOut.vColorStart = vStartColor;
  sParamOut.vColorEnd = vEndColor;
  sParamOut.fSpeed = speed;
	sParamOut.fSpeed.variation = 0.25f;
  sParamOut.nCount = count;
  sParamOut.fSize = size;
  sParamOut.fSizeSpeed = size_speed;
  sParamOut.vGravity = vGravity;
	sParamOut.fLifeTime = lifetime;
	sParamOut.fFadeInTime = fadeintime;

  sParamOut.nTexId = tid;
  sParamOut.nTexAnimFramesCount = frames;
  sParamOut.eBlendType = (ParticleBlendType)blendType;

  sParamOut.nParticleFlags = iParticleType;
	
	if(nLinearSizeSpeed)
		sParamOut.nParticleFlags |= PART_FLAG_SIZE_LINEAR;
      
  sParamOut.bRealPhysics = bRealPhys != 0;
  sParamOut.pChild = NULL;
  sParamOut.fChildSpawnPeriod = fChildSpawnPeriod;
  sParamOut.fTailLenght = fTailLength;

  sParamOut.nDrawLast = !draw_last;

  sParamOut.vRotation = vRotation;

  sParamOut.fBouncenes = fBouncenes;

  sParamOut.vInitAngles = vAngles;

  // Scale the direction vsector based on fDirVecScale
  sParamOut.vDirection *= fDirVecScale;
  
	sParamOut.fTurbulenceSize=fTurbulenceSize;
	sParamOut.fTurbulenceSpeed=fTurbulenceSpeed;  
	sParamOut.vSpaceLoopBoxSize = vSpaceLoopBoxSize;

	sParamOut.fAirResistance = fAirResistance;

	return true;
}

int CScriptObjectParticle::Attach(IFunctionHandler * pH)
{
	_SmartScriptObject  pObj(m_pScriptSystem,true);
	_SmartScriptObject  pChildObj(m_pScriptSystem,true);
	CScriptObjectVector oVec(m_pScriptSystem,true);
	Vec3 v3Pos,v3SysDir,v3Offset(0,0,0);
	int nID,nFlags=0;
	float fSpawnRate = 0;
	const char *szBoneName;
	ParticleParams sParam;
	CryParticleSpawnInfo cpsi;
	
	if(!pH->GetParam(1,nID))
		m_pScriptSystem->RaiseError( "<Particle::Attach> parameter 1 not specified or nil (entity id to attach to)" );
	if(!pH->GetParam(2,*pObj))
		m_pScriptSystem->RaiseError( "<Particle::Attach> parameter 2 not specified or nil(particle stuct)" );
	if(!pH->GetParam(3,fSpawnRate))
		m_pScriptSystem->RaiseError( "<Particle::Attach> spawn rate not specified" );


	IEntity *pEntity = m_pSystem->GetIEntitySystem()->GetEntity(nID);
	if (!pEntity)
	{
		m_pScriptSystem->RaiseError( "<Particle::Attach> specified entity ID does not exist)" );
		return pH->EndFunctionNull();
	}

	cpsi.fSpawnRate = fSpawnRate;

	ICryCharInstance *pInstance = pEntity->GetCharInterface()->GetCharacter(0);

	if (pH->GetParam(4,szBoneName))
	{
		if (pInstance)
		{
			cpsi.nBone = pInstance->GetModel()->GetBoneByName(szBoneName);
		}

		cpsi.nFlags |= CryParticleSpawnInfo::FLAGS_SPAWN_FROM_BONE;

		if(pH->GetParam(5,*oVec))
			cpsi.vBonePos = oVec.Get();
		else
			cpsi.vBonePos.Set(0,0,0);

		if (pH->GetParam(6,nFlags))
			cpsi.nFlags = nFlags;
	}

	ReadParticleTable(*pObj, sParam);
	/*sParam.vPosition = v3Pos;
	sParam.vDirection = v3SysDir;
	*/
/*	pObj->BeginSetGetChain();
	if ((sParam).fChildSpawnPeriod && pObj->GetValueChain("ChildProcess", *pChildObj))
	{
		ParticleParams sChildParams;
		ReadParticleTable(*pChildObj, sChildParams);
		sParam.pChild = &sChildParams;
		if(!pObj->GetValueChain( "ChildSpawnPeriod",sParam.fChildSpawnPeriod ))            
			sParam.fChildSpawnPeriod=0;
	}
	else
		sParam.pChild = NULL;
		*/
	return pH->EndFunction(pInstance->AddParticleEmitter(sParam,cpsi));
}

int CScriptObjectParticle::Detach(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(2);
	int nID,nHandle;

	pH->GetParam(1,nID);
	pH->GetParam(2,nHandle);

	IEntity *pEntity = m_pSystem->GetIEntitySystem()->GetEntity(nID);
	if (!pEntity)
	{
		m_pScriptSystem->RaiseError( "<Particle::Detach> specified entity ID does not exist)" );
		return pH->EndFunction();
	}

	ICryCharInstance *pInstance = pEntity->GetCharInterface()->GetCharacter(0);

	pInstance->RemoveParticleEmitter(nHandle);

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectParticle::SpawnEffect(IFunctionHandler *pH)
{
	CScriptObjectVector oVec(m_pScriptSystem,true);
	Vec3 pos,dir;
	const char *sEffectName = 0;
	float fScale = 1.0f;

	if(!pH->GetParam(1,*oVec))
		m_pScriptSystem->RaiseError( "<SpawnEffect> parameter 1 not specified or nil(pos)" );
	pos = oVec.Get();
	if(!pH->GetParam(2,*oVec))
		m_pScriptSystem->RaiseError( "<SpawnEffect> parameter 2 not specified or nil(normal)" );
	dir = oVec.Get();
	if(!pH->GetParam(3,sEffectName))
		m_pScriptSystem->RaiseError( "<SpawnEffect> parameter 3 not specified or nil(Effect Name)" );
	// Optional argument for scale.
//	pH->GetParam(4,fScale);
	if(pH->GetParamCount()>3)
		pH->GetParam(4,fScale);

	if (sEffectName)
	{
		IParticleEffect *pEffect = m_p3DEngine->FindParticleEffect( sEffectName );
		if (pEffect)
			pEffect->Spawn( pos,dir,fScale );
	}

	return pH->EndFunction();
}