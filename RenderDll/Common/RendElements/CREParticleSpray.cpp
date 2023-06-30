/*=============================================================================
	CParticleSpray.cpp : implementation of the particle effects RE.
	Copyright 2001 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Honitch Andrey

=============================================================================*/

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#include "RenderPCH.h"


//==================================================================
// Particle Spray
//==================================================================

CREParticleSpray::~CREParticleSpray()
{
  if (mParticlePntr[0])
    delete [] mParticlePntr[0];
  if (mParticlePntr[1])
    delete [] mParticlePntr[1];
}

/// Global Definitions ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CREParticleSpray& CREParticleSpray::operator = (const CREParticleSpray& Orig)
{
  mfSetType(eDATA_ParticleSpray);
  mfSetFlags(Orig.m_Flags);
  mFrame = 0;

  mParticlePool[0] = new SParticle[Orig.mEmitter.totalParticles[0]];
  mParticlePntr[0] = mParticlePool[0];
  // THIS IS A LINKED LIST OF PARTICLES, SO I NEED TO ESTABLISH LINKS
  int loop;
  for (loop=0; loop<Orig.mEmitter.totalParticles[0]-1; loop++)
  {
    mParticlePool[0][loop].next = &mParticlePool[0][loop + 1];
  } 
  // SET THE LAST PARTICLE TO POINT TO NULL
  mParticlePool[0][Orig.mEmitter.totalParticles[0]-1].next = NULL;

  if (Orig.mEmitter.totalParticles[1])
  {
    mParticlePool[1] = new SParticle[Orig.mEmitter.totalParticles[1]];
    mParticlePntr[1] = mParticlePool[1];
    // THIS IS A LINKED LIST OF PARTICLES, SO I NEED TO ESTABLISH LINKS
    for (loop=0; loop<Orig.mEmitter.totalParticles[1]-1; loop++)
    {
      mParticlePool[1][loop].next = &mParticlePool[1][loop + 1];
    } 
    // SET THE LAST PARTICLE TO POINT TO NULL
    mParticlePool[1][Orig.mEmitter.totalParticles[1]-1].next = NULL;
  }
  else
  {
    mParticlePntr[1] = mParticlePool[1] = NULL;
  }
  memcpy(&mEmitter, &Orig.mEmitter, sizeof(SEmitter));
  mEmitter.particle = NULL;

  return *this;
}

bool CREParticleSpray::mfInitParticleSystem(void)
{
  int loop;

  // SO I DON'T NEED TO DYNAMICALLY ALLOC THE PARTICLES IN THE RUNTIME
  // I WANT TO PULL ALREADY CREATED PARTICLES FROM A GLOBAL POOL.
  mEmitter.totalParticles[1] = mEmitter.totalParticles[0] * mEmitter.NumSparks;
  mParticlePool[0] = new SParticle[mEmitter.totalParticles[0]];
  mParticlePntr[0] = mParticlePool[0];
  // THIS IS A LINKED LIST OF PARTICLES, SO I NEED TO ESTABLISH LINKS
  for (loop=0; loop<mEmitter.totalParticles[0]-1; loop++)
  {
    mParticlePool[0][loop].next = &mParticlePool[0][loop + 1];
  } 
  // SET THE LAST PARTICLE TO POINT TO NULL
  mParticlePool[0][mEmitter.totalParticles[0]-1].next = NULL;


  if (mEmitter.totalParticles[1])
  {
    mParticlePool[1] = new SParticle[mEmitter.totalParticles[1]];
    mParticlePntr[1] = mParticlePool[1];
    // THIS IS A LINKED LIST OF PARTICLES, SO I NEED TO ESTABLISH LINKS
    for (loop=0; loop<mEmitter.totalParticles[1]-1; loop++)
    {
      mParticlePool[1][loop].next = &mParticlePool[1][loop + 1];
    } 
    // SET THE LAST PARTICLE TO POINT TO NULL
    mParticlePool[1][mEmitter.totalParticles[1]-1].next = NULL;
  }
  else
  {
    mParticlePool[1] = mParticlePntr[1] = NULL;
  }

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// Function:  CREParticleSpray::mfInitEmitter
// Purpose:   Initialize an emitter in the system
// Arguments: The emitter to initialize
///////////////////////////////////////////////////////////////////////////////
bool CREParticleSpray::mfInitEmitter(SEmitter *emitter)
{
  mfSetDefaultEmitter(emitter);
  emitter->particle = NULL;         // NULL TERMINATED LINKED LIST
  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// Function:  CREParticleSpray::mfSetDefaultEmitter
// Purpose:   Set up some default settings
// Arguments: The emitter to setup
///////////////////////////////////////////////////////////////////////////////
bool CREParticleSpray::mfSetDefaultEmitter(SEmitter *emitter)
{
  emitter->eCollisionType = ePCollision_None;
  emitter->pi.yaw = 0;
  emitter->pi.yawVar = 360.0f;
  emitter->pi.pitch = 90.0f;
  emitter->pi.pitchVar = 40.0f;
  emitter->pi.speed = 0.05f;
  emitter->pi.speedVar = 0.01f;

  emitter->totalParticles[0] = MAX_PARTICLES;
  emitter->totalParticles[1] = 0;
  emitter->particleCount[0] = emitter->particleCount[1] = 0;
  emitter->emitsPerFrame  = 100;
  emitter->emitVar  = 15;
  emitter->pi.life = 60;
  emitter->pi.lifeVar = 15;

  emitter->pi.startColor = CFColor(0.6f, 0.6f, 0.8f, 1.0f);
  emitter->pi.startColorVar = CFColor(0.1f, 0.1f, 0.1f, 0.1f);
  emitter->pi.endColor = CFColor(0.0f, 0.0f, 0.8f, 0.1f);
  emitter->pi.endColorVar = CFColor(0.1f, 0.1f, 0.2f, 0.1f);

  emitter->pi.force = Vec3d(0.0f, 0.0f, -0.001f);

  emitter->Life = 0;

  return true;
}

bool CREParticleSpray::mfCull(CCObject *obj)
{
  mfUpdateEmitter(&mEmitter);

  if (CRenderer::CV_r_noparticles)
    return true;

  CREParticleSpray::mRS.NumSprays++;

  SParticle *particle;

  // IF THERE IS AN EMITTER
  particle = mEmitter.particle;

  if (!particle)
    return false;

  Vec3d mins, maxs;
  mins = Vec3d( 999999.0f, 999999.0f, 999999.0f);
  maxs = Vec3d(-999999.0f,-999999.0f,-999999.0f);
  // GO THROUGH THE PARTICLES AND UPDATE THEM

  if (mEmitter.pi.ePT == ePTPoly || mEmitter.pi.ePT == ePTPoint || mEmitter.pi.ePT == ePTBeam)
  {
    while (particle)
    {
      mins.CheckMin(particle->pos);
      maxs.CheckMax(particle->pos);
      particle = particle->next;
    }
  }
  else
  {
    while (particle)
    {
      mins.CheckMin(particle->pos);
      maxs.CheckMax(particle->pos);
      particle = particle->next;
    }
  }
  if (mfGetFlags() & FCEF_TRANSFORM)
  {
    Vec3d org = Vec3d(gRenDev->m_RP.m_pCurObject->GetTranslation());
    mins += org;
    maxs += org;
  }
  //if (gfCullBox(mins, maxs))
  //  return true;
  return false;
}

bool CREParticleSpray::mfIsValidTime(SShader *ef, CCObject *obj, float curtime)
{
  if (obj->m_TempVars[0] && !mEmitter.particle)
  {
    return false;
  }
  return true;
}

static _inline void sMoveParticle(SParticle *particle, SParticleInfo *pi)
{
  int i;
  Vec3d v(0,0,0);
  float wf;
  float f;

  for (i=0; i<pi->mNumMoves; i++)
  {
    SPartMoveStage *pm = &pi->mMoves[i];

    switch(pm->eMoveType)
    {
      case eMTWave:
        wf = SEvalFuncs::EvalWaveForm(&pm->WaveMove);
        particle->realPos += particle->moveDir * wf;
        break;

      case eMTWhirl:
        wf = SEvalFuncs::EvalWaveForm(&pm->WaveMove);
        //RotatePointAroundVector(&v[0], &particle->moveDir[0], &particle->realPos[0], wf);
        particle->realPos = v;
        break;

      case eMTSqueeze:
        f = (float)particle->life / particle->startLife;
        wf = SEvalFuncs::EvalWaveForm2(&pm->WaveMove, f);
        f = particle->realPos[2];
        particle->realPos *= wf;
        particle->realPos[2] = f;
        break;
    }
  }

}

static void sUpdatePart(SParticle *particle, SParticleInfo *pi, bool bSpark)
{
  // SAVE ITS OLD POS FOR ANTI ALIASING

  if (pi->ePT != ePTBeam)
  {
    if (pi->StackSize == 1)
    {
      particle->prevPos[0] = particle->realPos;
    }
    else
    {
      int n = pi->StackSize - 1;
      while(n>=0)
      {
        particle->prevPos[n] = particle->prevPos[n-1];
        n--;
      }
      particle->prevPos[0] = particle->realPos;
    }
    // CALCULATE THE NEW
    particle->pos = particle->pos + particle->dir;
    particle->realPos = particle->pos;
  }
  if (pi->mNumMoves)
    sMoveParticle(particle, pi);
  if (pi->Squeeze)
  {
    float frac = (float)particle->life / particle->startLife;
    frac *= pi->Squeeze;
    particle->realPos[0] *= frac;
    particle->realPos[1] *= frac;
  }

  // APPLY GLOBAL FORCE TO DIRECTION
  particle->dir = particle->dir + pi->force;

  // SAVE THE OLD COLOR
  particle->prevColor = particle->color;

  // GET THE NEW COLOR
  particle->color += particle->deltaColor;

  particle->curSize += particle->deltaSize;

  particle->life--; // IT IS A CYCLE OLDER
}

STrace (*Trace) (Vec3d& start, Vec3d& mins, Vec3d& maxs, Vec3d& end, CCObject *passobj, int contentmask);

_inline void LerpVector (Vec3d& a, Vec3d& b, float f, Vec3d& res)
{
  for (int i=0; i<3; i++)
  {
    res[i] = LERP(a[i], b[i], f);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Function:  CREParticleSpray::mfUpdateParticle
// Purpose:   mfUpdateParticle settings
// Arguments: The particle to update and the emitter it came from
///////////////////////////////////////////////////////////////////////////////
bool CREParticleSpray::mfUpdateParticle(SParticle *particle,SEmitter *emitter)
{
  // IF THIS IS AN VALID PARTICLE
  if (particle != NULL && particle->life > 0)
  {
    if (!particle->bSpark)
      sUpdatePart(particle, &emitter->pi, false);
    else
      sUpdatePart(particle, &emitter->Spark, true);

    if (emitter->eCollisionType && !particle->bSpark)
    {
      if (emitter->eCollisionType == ePCollision_Plane)
      {
        Vec3d v, v0, v1;
        Vec3d org = gRenDev->m_RP.m_pCurObject->GetTranslation();
      
        v = org + emitter->PlaneOffs;
        float planeDist = v | emitter->PlaneAxis;
        v0 = org + particle->prevPos[0];
        v1 = org + particle->realPos;
        float d0 = (v0 | emitter->PlaneAxis) - planeDist;
        float d1 = (v1 | emitter->PlaneAxis) - planeDist;

        if (d0>=0 && d1<=0)
        {
          if (d0-d1)
          {
            float f = (d0 - 0.1f) / (d0 - d1);
            LerpVector(particle->realPos, particle->prevPos[0], f, particle->realPos);
          }
          particle->life = 0;
          mfEmitSparks(particle, emitter);
        }
      }
      else
      if (emitter->eCollisionType == ePCollision_True)
      {
        Vec3d v0, v1, org;
        org = gRenDev->m_RP.m_pCurObject->GetTranslation();
        v0 = org + particle->prevPos[0];
        v1 = org + particle->realPos;
        /*STrace trace = Trace (&v0[0], Vec3d.Clear(), Vec3d.Clear(), &v1[0], (CCObject *)gRenDev->m_RP.m_pCurObject, MASK_OPAQUE);
        if (trace.fraction < 0.99f)
        {
          float f = trace.fraction;
          LerpVector(&particle->prevPos[0][0], &particle->realPos[0], f, &particle->realPos[0]);
          particle->life = 0;
          mfEmitSparks(particle, emitter);
        }*/
      }
    }

    return TRUE;
  }
  else
  if (particle != NULL && particle->life == 0)
  {
    // FREE THIS SUCKER UP BACK TO THE MAIN POOL
    if (particle->prev != NULL)
      particle->prev->next = particle->next;
    else
      emitter->particle = particle->next;
    // FIX UP THE NEXT'S PREV POINTER IF THERE IS A NEXT
    if (particle->next != NULL)
      particle->next->prev = particle->prev;
    particle->next = mParticlePool[particle->bSpark];
    mParticlePool[particle->bSpark] = particle; // NEW POOL POINTER
    emitter->particleCount[particle->bSpark]--; // ADD ONE TO POOL
  }
  return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Function:  sRotationToDirection
// Purpose:   Convert a Yaw and Pitch to a direction vector
///////////////////////////////////////////////////////////////////////////////
static void sRotationToDirection(float pitch,float yaw,Vec3d& direction)
{
  direction[0] = (float)(-sin_tpl(yaw) * cos_tpl(pitch));
  direction[1] = (float)sin_tpl(pitch);
  direction[2] = (float)(cos_tpl(pitch) * cos_tpl(yaw));
}

static Vec3d sOrg;

void CREParticleSpray::mfEmitSparks(SParticle *p, SEmitter *emitter)
{
  sOrg = p->realPos;
  for (int i=0; i<emitter->NumSparks; i++)
  {
    mfAddParticle(emitter, &emitter->Spark);
  }
}


inline void AxisFromAngles(const Vec3d& v, Vec3d* forward, Vec3d* right, Vec3d* up)
{
float    angle;
float    sr, sp, sy, cr, cp, cy;

angle = (v.y * ((float)PI / 180.0f));
sy = (float)sin_tpl(angle);
cy = (float)cos_tpl(angle);
angle = (v.x * ((float)PI / 180.0f));
sp = (float)sin_tpl(angle);
cp = (float)cos_tpl(angle);
angle = (v.z * ((float)PI / 180.0f));
sr = (float)sin_tpl(angle);
cr = (float)cos_tpl(angle);

if (forward)
{
forward->x = cp*cy;
forward->y = cp*sy;
forward->z = -sp;
}
if (right)
{
right->x = (-1*sr*sp*cy+-1*cr*-sy);
right->y = (-1*sr*sp*sy+-1*cr*cy);
right->z = -1*sr*cp;
}
if (up)
{
up->x = (cr*sp*cy+-sr*-sy);
up->y = (cr*sp*sy+-sr*cy);
up->z = cr*cp;
}
}


///////////////////////////////////////////////////////////////////////////////
// Function:  CREParticleSpray::mfAddParticle
// Purpose:   add a particle to an emitter
// Arguments: The emitter to add to
///////////////////////////////////////////////////////////////////////////////
bool CREParticleSpray::mfAddParticle(SEmitter *emitter, SParticleInfo *pi)
{
/// Local Variables ///////////////////////////////////////////////////////////
  SParticle *particle;
  CFColor  colend;
  float yaw, pitch, speed;
  ///////////////////////////////////////////////////////////////////////////////
  // IF THERE IS AN EMITTER AND A PARTICLE IN THE POOL
  // AND I HAVEN'T EMITTED MY MAX
  bool bSpark = (pi == &emitter->Spark);
  if (emitter != NULL && mParticlePool[bSpark] != NULL && emitter->particleCount[bSpark] < emitter->totalParticles[bSpark])
  {
    particle = mParticlePool[bSpark];   // THE CURRENT PARTICLE 
    mParticlePool[bSpark] = mParticlePool[bSpark]->next;  // FIX THE POOL POINTERS

    if (emitter->particle != NULL)
      emitter->particle->prev = particle; // SET BACK LINK
    particle->bSpark = bSpark;
    particle->next = emitter->particle; // SET ITS NEXT POINTER
    particle->prev = NULL;        // IT HAS NO BACK POINTER
    emitter->particle = particle;   // SET IT IN THE EMITTER

    if (pi->ePT != ePTBeam)
    {
      if (!bSpark)
      {
        Vec3d var = Vec3d(emitter->startPosVar.x*RandomNum(),emitter->startPosVar.y*RandomNum(),emitter->startPosVar.z*RandomNum());
        particle->pos = emitter->startPos + var;
      }
      else
      {
        particle->pos = sOrg;
      }

      particle->realPos = particle->pos;
      int n = pi->StackSize;
      while (n>=0)
      {
        particle->prevPos[n-1] = particle->pos; // USED FOR ANTI ALIAS
        n--;
      }

      // CALCULATE THE STARTING DIRECTION VECTOR
      yaw = pi->yaw + (pi->yawVar * RandomNum());
      pitch = pi->pitch + (pi->pitchVar * RandomNum());

      // CONVERT THE ROTATIONS TO A VECTOR
      Vec3d v;
      v[PITCH] = -pitch;
      v[YAW] = yaw;
      v[ROLL] = 0;
      AxisFromAngles( v, &particle->dir, NULL, NULL);
    }
    else
    {
      particle->pos = gRenDev->m_RP.m_pCurObject->GetTranslation();
      particle->realPos = particle->pos;
      particle->prevPos[0] = Vec3d(gRenDev->m_RP.m_pCurObject->m_Trans2[0], gRenDev->m_RP.m_pCurObject->m_Trans2[1], gRenDev->m_RP.m_pCurObject->m_Trans2[2]);
    }

    Vec3d var = Vec3d(pi->moveDirVar.x*RandomNum(), pi->moveDirVar.y*RandomNum(),pi->moveDirVar.z*RandomNum());
    particle->moveDir = pi->moveDir + var;

    // MULTIPLY IN THE SPEED FACTOR
    speed = pi->speed + (pi->speedVar * RandomNum());
    particle->dir *= speed;

    // CALCULATE THE COLORS
    CFColor c;
    c = CFColor(RandomNum(), RandomNum(), RandomNum(), RandomNum());
    particle->color = pi->startColor + (pi->startColorVar * c);
    particle->color.Clamp();
    c = CFColor(RandomNum(), RandomNum(), RandomNum(), RandomNum());
    colend = pi->endColor + (pi->endColorVar * c);
    colend.Clamp();

    // CALCULATE THE LIFE SPAN
    particle->life = pi->life + (int)((float)pi->lifeVar * RandomNum());
    particle->startLife = particle->life;

    // CREATE THE COLOR DELTA
    particle->deltaColor = (colend - particle->color) / (float)particle->life;

    particle->curSize = pi->startSize + (RandomNum() * pi->startSizeVar);
    float endSize = pi->endSize + (RandomNum() * pi->endSizeVar);
    particle->deltaSize = (endSize - particle->curSize) / particle->life;

    emitter->particleCount[bSpark]++; // A NEW PARTICLE IS BORN

    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Function:  CREParticleSpray::mfUpdateEmitter
// Purpose:   Update Emitter setting
// Arguments: The Emitter to update
// Notes:   This is called once per frame to update the emitter
///////////////////////////////////////////////////////////////////////////////
bool CREParticleSpray::mfUpdateEmitter(SEmitter *emitter)
{
/// Local Variables ///////////////////////////////////////////////////////////
  int loop,emits;
  SParticle *particle, *next;
///////////////////////////////////////////////////////////////////////////////
  if (mFrame == gRenDev->m_RP.m_RenderFrame)
    return TRUE;

  mFrame = gRenDev->m_RP.m_RenderFrame;

  // IF THERE IS AN EMITTER
  if (emitter != NULL)
  {
    if (emitter->particle != NULL)
    {
      // GO THROUGH THE PARTICLES AND UPDATE THEM
      particle = emitter->particle;
      while (particle)
      {
        next = particle->next;  // SAVE THIS BECAUSE IT MAY CHANGE UNDER ME
        mfUpdateParticle(particle,emitter);
        particle = next;
      }
    }

    // EMIT PARTICLES FOR THIS FRAME
    emits = emitter->emitsPerFrame + (int)((float)emitter->emitVar * RandomNum());
    
    if (emitter->Life < 0)
    {
      if (gRenDev->m_RP.m_pCurObject->m_TempVars[0] + emits < emitter->totalParticles[0])
      {
        gRenDev->m_RP.m_pCurObject->m_TempVars[0] += emits;
      }
      else
      {
        emits = (int)(emitter->totalParticles[0] - gRenDev->m_RP.m_pCurObject->m_TempVars[0]);
        gRenDev->m_RP.m_pCurObject->m_TempVars[0] = (float)emitter->totalParticles[0];
      }
    }
    for (loop = 0; loop < emits; loop++)
      mfAddParticle(emitter, &emitter->pi);
    
    return true;
  }
  return false;
}

void CREParticleSpray::mfPrepare()
{
  static int inds[] = {3, 0, 2, 2, 0, 1};

  int n;
  int np;

  CREParticleSpray::mRS.NumRendSprays++;

  if (mEmitter.pi.ePT == ePTPoint || mEmitter.pi.ePT == ePTLine)
  {
    //gRenDev->mfRenderSimpleParticleEmitter(&mEmitter, mfGetFlags());
    return;
  }

  if (mEmitter.pi.ePT != ePTPoly && mEmitter.pi.ePT != ePTPolySegs && mEmitter.pi.ePT != ePTBeam)
    return;

  SEmitter *em = &mEmitter;
  SParticle *pt;
  Vec3d vc1, vc2, vc3, vc4;
  SShader *ef = gRenDev->m_RP.m_pShader;

  int savev = gRenDev->m_RP.m_RendNumVerts;
  int savei = gRenDev->m_RP.m_RendNumIndices;

  Vec3d vecVel;
  if (em != NULL)
  {
    if (em->particle != NULL)
    {
      pt = em->particle;
      np = 0;

      Vec3d VecX, VecY;
      VecX = gRenDev->m_RP.m_CamVecs[1];
      VecY = gRenDev->m_RP.m_CamVecs[2];
      vc1 = VecX + VecY;
      vc2 = -VecX + VecY;
      vc3 = -VecX - VecY;
      vc4 = VecX - VecY;

      n = gRenDev->m_RP.m_RendNumVerts;
      int steps;
      float oldDist = -9999.0f;
      UPipeVertex ptr = gRenDev->m_RP.m_NextPtr;
      byte *cols;
      if (!(gRenDev->m_RP.m_FT & FLT_COL))
        cols = (byte *)(&gRenDev->m_RP.m_pClientColors[n]);
      else
        cols = gRenDev->m_RP.m_NextPtr.PtrB+gRenDev->m_RP.m_OffsD;
      while (pt)
      {
        CREParticleSpray::mRS.NumParticles++;

        Vec3d vecTL, vecBL, vecBR, vecTR;

        float dist;
        EParticleType ePT;
        float so;
        int ms, ss;

        dist = pt->curSize * 0.5f;
        if (pt->bSpark)
        {
          ePT = em->Spark.ePT;
          ms = em->Spark.segmMax;
          so = em->Spark.segmOffs;
          ss = em->Spark.StackSize;
        }
        else
        {
          ePT = em->pi.ePT;
          ms = em->pi.segmMax;
          so = em->pi.segmOffs;
          ss = em->pi.StackSize;
        }
        if (dist != oldDist)
        {
          oldDist = dist;
          vecTL = vc1 * dist;
          vecBL = vc2 * dist;
          vecBR = vc3 * dist;
          vecTR = vc4 * dist;
        }
        Vec3d vecPos;
        if (ePT == ePTPoly)
        {
          steps = 1;
          vecPos = (pt->realPos);
        }
        else
        {
          vecPos = pt->prevPos[ss-1];
          vecVel = pt->realPos - vecPos;


          float fL2 = GetLengthSquared(vecVel);
          if (!fL2)
            steps = 1;
          else
          {
            fL2 = cry_sqrtf(fL2);
            float fst = fL2 / (pt->curSize * so);
            if (fst <= 1)
              steps = 1;
            else
            {
              vecVel /= fst;
              steps = QRound(fst);
              if (steps > ms)
                steps = ms;
            }
          }
        }
        bvec4 c;
        if ((gRenDev->m_RP.m_FT & FLT_COL) && gbRgb)
        {
          c[2] = (byte)QRound(pt->color[0] * 255.0f);
          c[1] = (byte)QRound(pt->color[1] * 255.0f);
          c[0] = (byte)QRound(pt->color[2] * 255.0f);
          c[3] = (byte)QRound(pt->color[3] * 255.0f);
        }
        else
        {
          c[0] = (byte)QRound(pt->color[0] * 255.0f);
          c[1] = (byte)QRound(pt->color[1] * 255.0f);
          c[2] = (byte)QRound(pt->color[2] * 255.0f);
          c[3] = (byte)QRound(pt->color[3] * 255.0f);
        }


        for(int i=0; i<steps; i++)
        {
          if (n+4 >= gRenDev->m_RP.m_MaxVerts || gRenDev->m_RP.m_RendNumIndices+2*3 >= gRenDev->m_RP.m_MaxTris*3)
          {
            gRenDev->m_RP.m_RendNumVerts = n;
            gRenDev->m_RP.m_pRenderFunc();
            gRenDev->EF_Start(gRenDev->m_RP.m_pShader, gRenDev->m_RP.m_pStateShader, NULL, gRenDev->m_RP.m_pFogVolume ? (gRenDev->m_RP.m_pFogVolume-&gRenDev->m_RP.m_FogVolumes[0]) : 0, this);
            n = 0;
            ptr = gRenDev->m_RP.m_NextPtr;
          }
          for (int ii=0; ii<6; ii++)
          {
            gRenDev->m_RP.m_RendIndices[gRenDev->m_RP.m_RendNumIndices++] = inds[ii]+n;
          }

          /*if (!(gRenDev->m_RP.mFT & FLT_COL))
          {
            int nn = gRenDev->m_RP.m_Stride;

            ptr.Ptr_D_2T->xyz[0] = vecPos.x + vecTL.x;
            ptr.Ptr_D_2T->xyz[1] = vecPos.y + vecTL.y;
            ptr.Ptr_D_2T->xyz[2] = vecPos.z + vecTL.z;
            *(uint *)(&cols[0]) = *(uint *)c;
            ptr.Ptr_D_2T->st[0][0] = 0;
            ptr.Ptr_D_2T->st[0][1] = 0;
            ptr.PtrB += nn;

            ptr.Ptr_D_2T->xyz[0] = vecPos.x + vecBL.x;
            ptr.Ptr_D_2T->xyz[1] = vecPos.y + vecBL.y;
            ptr.Ptr_D_2T->xyz[2] = vecPos.z + vecBL.z;
            *(uint *)(&cols[4]) = *(uint *)c;
            ptr.Ptr_D_2T->st[0][0] = 1.0f;
            ptr.Ptr_D_2T->st[0][1] = 0;
            ptr.PtrB += nn;

            ptr.Ptr_D_2T->xyz[0] = vecPos.x + vecBR.x;
            ptr.Ptr_D_2T->xyz[1] = vecPos.y + vecBR.y;
            ptr.Ptr_D_2T->xyz[2] = vecPos.z + vecBR.z;
            *(uint *)(&cols[8]) = *(uint *)c;
            ptr.Ptr_D_2T->st[0][0] = 1.0f;
            ptr.Ptr_D_2T->st[0][1] = 1.0f;
            ptr.PtrB += nn;

            ptr.Ptr_D_2T->xyz[0] = vecPos.x + vecTR.x;
            ptr.Ptr_D_2T->xyz[1] = vecPos.y + vecTR.y;
            ptr.Ptr_D_2T->xyz[2] = vecPos.z + vecTR.z;
            *(uint *)(&cols[12]) = *(uint *)c;
            ptr.Ptr_D_2T->st[0][0] = 0;
            ptr.Ptr_D_2T->st[0][1] = 1.0f;
            ptr.PtrB += nn;

            cols += 16;
          }
          else
          {
            int nn = gRenDev->m_RP.m_Stride;

            ptr.Ptr_D_2T->xyz[0] = vecPos.x + vecTL.x;
            ptr.Ptr_D_2T->xyz[1] = vecPos.y + vecTL.y;
            ptr.Ptr_D_2T->xyz[2] = vecPos.z + vecTL.z;
            *(uint *)(&cols[0]) = *(uint *)c;
            ptr.Ptr_D_2T->st[0][0] = 0;
            ptr.Ptr_D_2T->st[0][1] = 0;
            ptr.PtrB += nn;
            cols += nn;

            ptr.Ptr_D_2T->xyz[0] = vecPos.x + vecBL.x;
            ptr.Ptr_D_2T->xyz[1] = vecPos.y + vecBL.y;
            ptr.Ptr_D_2T->xyz[2] = vecPos.z + vecBL.z;
            *(uint *)(&cols[0]) = *(uint *)c;
            ptr.Ptr_D_2T->st[0][0] = 1.0f;
            ptr.Ptr_D_2T->st[0][1] = 0;
            ptr.PtrB += nn;
            cols += nn;

            ptr.Ptr_D_2T->xyz[0] = vecPos.x + vecBR.x;
            ptr.Ptr_D_2T->xyz[1] = vecPos.y + vecBR.y;
            ptr.Ptr_D_2T->xyz[2] = vecPos.z + vecBR.z;
            *(uint *)(&cols[0]) = *(uint *)c;
            ptr.Ptr_D_2T->st[0][0] = 1.0f;
            ptr.Ptr_D_2T->st[0][1] = 1.0f;
            ptr.PtrB += nn;
            cols += nn;

            ptr.Ptr_D_2T->xyz[0] = vecPos.x + vecTR.x;
            ptr.Ptr_D_2T->xyz[1] = vecPos.y + vecTR.y;
            ptr.Ptr_D_2T->xyz[2] = vecPos.z + vecTR.z;
            *(uint *)(&cols[0]) = *(uint *)c;
            ptr.Ptr_D_2T->st[0][0] = 0;
            ptr.Ptr_D_2T->st[0][1] = 1.0f;
            ptr.PtrB += nn;
            cols += nn;
          }*/

          n += 4;
          vecPos += vecVel;
        }
        pt = pt->next;
        np++;
      }
      gRenDev->m_RP.m_RendNumVerts = n;
      gRenDev->m_RP.m_NextPtr = ptr;
    }
  }
  if (em->Life)
  {
    if (em->Life < 0)
    {
      if (!np)
      {
        //gRenDev->m_RP.m_pCurObject->m_ObjFlags |= FOB_NOTVISIBLE;
      }
      return;
    }
    else
    if (gRenDev->m_RP.m_RealTime - gRenDev->m_RP.m_pCurObject->m_StartTime >= em->Life)
    {
      //gRenDev->m_RP.m_pCurObject->m_ObjFlags |= FOB_NOTVISIBLE;
      return;
    }
  }
  CREParticleSpray::mRS.NumVerts += gRenDev->m_RP.m_RendNumVerts - savev; 
  CREParticleSpray::mRS.NumIndices += gRenDev->m_RP.m_RendNumIndices - savei; 
}

//======================================================================

SParticleStat CREParticleSpray::mRS;

void CREParticleSpray::mfPrintStat()
{
/*  char str[1024];

  *gpCurPrX = 4;
  sprintf(str, "Num Indices: %i\n", mRS.NumIndices);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Verts: %i\n", mRS.NumVerts);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Render Particles: %i\n", mRS.NumParticles);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Render Particle Sprays: %i\n", mRS.NumRendSprays);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  sprintf(str, "Num Added Particle Sprays: %i\n", mRS.NumSprays);
  gRenDev->mfPrintString (str, PS_TRANSPARENT | PS_UP);

  *gpCurPrX = 4;
  gRenDev->mfPrintString ("\nParticles status info:\n", PS_TRANSPARENT | PS_UP);*/
}

//=================================================================================

// Parsing

bool CREParticleSpray::mfCompileMoveTypeWave(SShader *ef, SPartMoveStage *pm, char *scr)
{
  char* name;
  long cmd;
  char *params;

  enum {eWaveForm=1};
  static tokenDesc commands[] =
  {
    {eWaveForm, "WaveForm"},

    {0,0}
  };

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case eWaveForm:
        gRenDev->m_cEF.mfCompileWaveForm(&pm->WaveMove, params);
        break;
    }
  }

  return true;
}

bool CREParticleSpray::mfCompileMoveTypeWhirl(SShader *ef, SPartMoveStage *pm, char *scr)
{
  char* name;
  long cmd;
  char *params;

  enum {eWaveForm=1};
  static tokenDesc commands[] =
  {
    {eWaveForm, "WaveForm"},

    {0,0}
  };

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case eWaveForm:
        gRenDev->m_cEF.mfCompileWaveForm(&pm->WaveMove, params);
        break;
    }
  }

  return true;
}

bool CREParticleSpray::mfCompileMoveTypeSqueeze(SShader *ef, SPartMoveStage *pm, char *scr)
{
  char* name;
  long cmd;
  char *params;

  enum {eWaveForm=1};
  static tokenDesc commands[] =
  {
    {eWaveForm, "WaveForm"},

    {0,0}
  };

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case eWaveForm:
        gRenDev->m_cEF.mfCompileWaveForm(&pm->WaveMove, params);
        break;
    }
  }

  return true;
}

bool CREParticleSpray::mfCompileMove(SShader *ef, SPartMoveStage *pm, SParticleInfo *pi, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eType=1, eDir, eVarDir};
  static tokenDesc commands[] =
  {
    {eType, "Type"},
    {eDir, "Dir"},
    {eVarDir, "VarDir"},

    {0,0}
  };

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    data = NULL;
    if (name)
      data = name;
    else
    if (params)
      data = params;

    switch (cmd)
    {
      case eType:
        if (!data || !data[0])
        {
          Warning( 0,0,"missing parameters for Move Type in Particle Client Shader '%s' (Skipping)\n", ef->m_Name.c_str());
          return false;
        }
        if (!stricmp(data, "Wave"))
        {
          if (mfCompileMoveTypeWave(ef, pm, params))
            pm->eMoveType = eMTWave;
          else
            return false;
        }
        else
        if (!stricmp(data, "Whirl"))
        {
          if (mfCompileMoveTypeWhirl(ef, pm, params))
            pm->eMoveType = eMTWhirl;
          else
            return false;
        }
        else
        if (!stricmp(data, "Squeeze"))
        {
          if (mfCompileMoveTypeSqueeze(ef, pm, params))
            pm->eMoveType = eMTSqueeze;
          else
            return false;
        }
        else
        {
          Warning( 0,0,"Unknown move type '%s' in Shader '%s' (skipping)\n", data, ef->m_Name.c_str());
          return false;
        }
        break;

      case eDir:
        shGetVector(data, pi->moveDir);
        break;

      case eVarDir:
        shGetVector(data, pi->moveDirVar);
        break;
    }
  }

  return true;
}

void CREParticleSpray::mfCompileCollision(SShader *ef, SEmitter *em, char *scr, char *Collide)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {ePlaneAxis=1, ePlaneOffs, eSparks, eNumSparks};
  static tokenDesc commands[] =
  {
    {ePlaneAxis, "PlaneAxis"},
    {ePlaneOffs, "PlaneOffs"},
    {eSparks, "Sparks"},
    {eNumSparks, "NumSparks"},

    {0,0}
  };

  if (!stricmp(Collide, "True"))
    em->eCollisionType = ePCollision_True;
  else
  if (!stricmp(Collide, "Plane"))
    em->eCollisionType = ePCollision_Plane;
  else
  {
    Warning( 0,0,"Can't determine collision type '%s' in Shader '%s' (skipping)\n", Collide, ef->m_Name.c_str());
    em->eCollisionType = ePCollision_None;
    return;
  }
  em->NumSparks = 4;
  em->PlaneAxis = Vec3d(0, 0, -1);
  em->PlaneOffs.Set(0.0f,0.0f,0.0f);
  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    data = NULL;
    if (name)
      data = name;
    else
    if (params)
      data = params;

    switch (cmd)
    {
      case ePlaneAxis:
        if (!data || !data[0])
        {
          Warning( 0,0,"missing parameters for PlaneAxis in Particle Client Shader '%s' (Use default)\n", ef->m_Name.c_str());
          break;
        }
        shGetVector(data, em->PlaneAxis);
        break;

      case ePlaneOffs:
        if (!data || !data[0])
        {
          Warning( 0,0,"missing parameters for PlaneOffs in Particle Client Shader '%s' (Use default)\n", ef->m_Name.c_str());
          break;
        }
        shGetVector(data, em->PlaneOffs);
        break;

      case eSparks:
        mfCompileParticleInfo(ef, &em->Spark, params);
        break;

      case eNumSparks:
        em->NumSparks = shGetInt(data);
        break;
    }
  }
}

void CREParticleSpray::mfCompileParticleInfo(SShader *ef, SParticleInfo *pi, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eAntiAlias=1, ePoints, eLines, ePolySegs, ePolygons, eBeam, eYaw, eVarYaw, ePitch, eVarPitch, eSpeed, eVarSpeed, eLife, eVarLife, eStartColor, eVarStartColor, eEndColor, eVarEndColor, eForce, eSize, eStartSize, eEndSize, eVarStartSize, eVarEndSize, eSegmOffs, eSegmsMax, eMove, eStackSize, eSqueeze};
  static tokenDesc commands[] =
  {
    {eYaw, "Yaw"},
    {eVarYaw, "VarYaw"},
    {ePitch, "Pitch"},
    {eVarPitch, "VarPitch"},
    {eSpeed, "Speed"},
    {eVarSpeed, "VarSpeed"},
    {eLife, "Life"},
    {eVarLife, "VarLife"},

    {eSize, "Size"},
    {eStartSize, "StartSize"},
    {eEndSize, "EndSize"},
    {eVarStartSize, "VarStartSize"},
    {eVarEndSize, "VarEndSize"},

    {eSegmOffs, "SegmOffs"},
    {eSegmsMax, "SegmsMax"},

    {ePoints, "Points"},
    {eLines, "Lines"},
    {ePolygons, "Polygons"},
    {ePolySegs, "PolySegs"},
    {eBeam, "Beam"},

    {eStartColor, "StartColor"},
    {eVarStartColor, "VarStartColor"},

    {eEndColor, "EndColor"},
    {eVarEndColor, "VarEndColor"},

    {eForce, "Force"},

    {eMove, "Move"},

    {eStackSize, "StackSize"},

    {eSqueeze, "Squeeze"},

    {0,0}
  };

  pi->mNumMoves = 0;
  pi->StackSize = 1;
  pi->Squeeze = 0;

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    data = NULL;
    if (name)
      data = name;
    else
    if (params)
      data = params;

    switch (cmd)
    {
      case eAntiAlias:
        pi->Flags |= FP_ANTIALIAS;
        break;

      case ePoints:
        pi->ePT = ePTPoint;
        break;

      case eLines:
        pi->ePT = ePTLine;
        break;

      case ePolygons:
        pi->ePT = ePTPoly;
        break;

      case ePolySegs:
        pi->ePT = ePTPolySegs;
        break;

      case eBeam:
        pi->ePT = ePTBeam;
        break;

      case eYaw:
        pi->yaw = shGetFloat(data);
        break;

      case eVarYaw:
        pi->yawVar = shGetFloat(data);
        break;

      case ePitch:
        pi->pitch = shGetFloat(data);
        break;

      case eVarPitch:
        pi->pitchVar = shGetFloat(data);
        break;

      case eSpeed:
        pi->speed = shGetFloat(data);
        break;

      case eVarSpeed:
        pi->speedVar = shGetFloat(data);
        break;

      case eSqueeze:
        pi->Squeeze = shGetFloat(data);
        break;

      case eLife:
        pi->life = shGetInt(data);
        break;

      case eVarLife:
        pi->lifeVar = shGetInt(data);
        break;

      case eStartColor:
        shGetColor(data, pi->startColor);
        break;

      case eVarStartColor:
        shGetColor(data, pi->startColorVar);
        break;

      case eEndColor:
        shGetColor(data, pi->endColor);
        break;

      case eVarEndColor:
        shGetColor(data, pi->endColorVar);
        break;

      case eForce:
        shGetVector(data, pi->force);
        break;

      case eSize:
        pi->startSize = shGetFloat(data);
        pi->endSize = pi->startSize;
        pi->startSizeVar = pi->endSizeVar = 0;
        break;

      case eStartSize:
        pi->startSize = shGetFloat(data);
        break;

      case eEndSize:
        pi->endSize = shGetFloat(data);
        break;

      case eVarStartSize:
        pi->startSizeVar = shGetFloat(data);
        break;

      case eVarEndSize:
        pi->endSizeVar = shGetFloat(data);
        break;

      case eSegmOffs:
        pi->segmOffs = shGetFloat(data);
        break;

      case eSegmsMax:
        pi->segmMax = shGetInt(data);
        break;

      case eStackSize:
        pi->StackSize = shGetInt(data);
        break;

      case eMove:
        if (pi->mNumMoves == MAX_PART_MOVE_STAGES)
        {
          Warning( 0,0,"MAX_PART_MOVE_STAGES hit in Shader '%s'\n", ef->m_Name.c_str());
        }
        else
        if (mfCompileMove(ef, &pi->mMoves[pi->mNumMoves], pi, params))
        {
          pi->mNumMoves++;
        }
        break;
    }
  }
}

bool CREParticleSpray::mfCompile(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  enum {eParticle = 1, eTotalParticles, eEmits, eVarEmits, eStartPos, eVarStartPos, eCollide, eLife, eAllowMerge};
  static tokenDesc commands[] =
  {
    {eTotalParticles, "TotalParticles"},
    {eAllowMerge, "AllowMerge"},

    {eEmits, "Emits"},
    {eVarEmits, "VarEmits"},

    {eParticle, "Particle"},

    {eStartPos, "StartPos"},
    {eVarStartPos, "VarStartPos"},

    {eCollide, "Collide"},

    {eLife, "Life"},

    {0,0}
  };

  SEmitter *em = mfGetEmitter();

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    data = NULL;
    if (name)
      data = name;
    else
    if (params)
      data = params;

    switch (cmd)
    {
      case eTotalParticles:
        em->totalParticles[0] = shGetInt(data);
        break;

      case eAllowMerge:
        mfClearFlags(FCEF_TRANSFORM);
        break;

      case eEmits:
        em->emitsPerFrame = shGetInt(data);
        break;

      case eVarEmits:
        em->emitVar = shGetInt(data);
        break;

      case eStartPos:
        shGetVector(data, em->startPos);
        break;

      case eVarStartPos:
        shGetVector(data, em->startPosVar);
        break;

      case eLife:
        em->Life = shGetFloat(data);
        break;

      case eParticle:
        mfCompileParticleInfo(ef, &em->pi, params);
        break;

      case eCollide:
        if (!data || !data[0])
        {
          Warning( 0,0,"Missing Collide arg in Particle Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        mfCompileCollision(ef, em, params, data);
        break;
    }
  }
  mfInitParticleSystem();

  return true;
}

