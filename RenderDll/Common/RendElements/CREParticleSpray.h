
#ifndef __CREPARTICLESPRAY_H__
#define __CREPARTICLESPRAY_H__

//=============================================================

struct SParticle
{
  SParticle *prev,*next;       // LINK
  Vec3d      pos;              // CURRENT POSITION
  Vec3d      prevPos[8];       // PREVIOUS POSITION
  Vec3d      realPos;          // CURRENT RENDER POSITION
  Vec3d      dir;              // CURRENT DIRECTION WITH SPEED
  Vec3d      moveDir;          // CURRENT CHANGE DIRECTION
  int        life;             // HOW LONG IT WILL LAST
  int        startLife;
  
  CFColor    color;            // CURRENT COLOR OF PARTICLE
  CFColor    prevColor;        // LAST COLOR OF PARTICLE
  CFColor    deltaColor;       // CHANGE OF COLOR

  bool bSpark;

  float curSize;
  float deltaSize;
};

//================================================

#define MAX_PART_MOVE_STAGES 4

enum EMoveType
{
  eMTWave,
  eMTWhirl,
  eMTSqueeze,
};

struct SPartMoveStage
{
  EMoveType eMoveType;
  SWaveForm WaveMove;
};

//================================================

enum EParticleType
{
  ePTPoint,
  ePTLine,
  ePTPolySegs,
  ePTPoly,
  ePTBeam,
};

enum EParticleCollision
{
  ePCollision_None,
  ePCollision_True,
  ePCollision_Plane
};

struct SParticleInfo
{
  // TRANSFORMATION INFO
  float   yaw, yawVar;        // YAW AND VARIATION
  float   pitch, pitchVar;    // PITCH AND VARIATION
  float   speed,speedVar;

  int     life, lifeVar;              // LIFE COUNT AND VARIATION
  CFColor    startColor, startColorVar;  // CURRENT COLOR OF PARTICLE
  CFColor    endColor, endColorVar;      // CURRENT COLOR OF PARTICLE

  // Physics
  Vec3d      force;

  EParticleType ePT;
  int Flags;

  // Move info
  SPartMoveStage mMoves[MAX_PART_MOVE_STAGES];
  int mNumMoves;

  Vec3d moveDir;
  Vec3d moveDirVar;

  // Geometry info
  float startSize, startSizeVar;
  float endSize, endSizeVar;

  float segmOffs;
  int segmMax;

  int StackSize;
  float Squeeze;
};

struct SEmitter
{
  SParticleInfo pi;

  Vec3d    startPos;           // XYZ POSITION
  Vec3d    startPosVar;        // XYZ POSITION VARIATION

  // Particle
  SParticle *particle;               // NULL TERMINATED LINKED LIST
  int       totalParticles[2];       // TOTAL EMITTED AT ANY TIME
  int       particleCount[2];        // TOTAL EMITTED RIGHT NOW
  int       emitsPerFrame, emitVar;  // EMITS PER FRAME AND VARIATION

  EParticleCollision eCollisionType;
  Vec3d PlaneAxis;
  Vec3d PlaneOffs;

  SParticleInfo Spark;
  int NumSparks;

  float Life;
};

///////////////////////////////////////////////////////////////////////////////

/// Particle Definitions //////////////////////////////////////////////////////
#define MAX_PARTICLES   4096    // MAXIMUM NUMBER OF PARTICLES
///////////////////////////////////////////////////////////////////////////////

#define FP_ANTIALIAS 0x1

struct SParticleStat
{
  int NumSprays;
  int NumRendSprays;
  int NumParticles;
  int NumVerts;
  int NumIndices;
};

class CREParticleSpray : public CRendElement
{
public:
  SEmitter mEmitter;
  SParticle *mParticlePool[2];
  SParticle *mParticlePntr[2];
  int mFrame;

private:
/// Support Function Definitions //////////////////////////////////////////////
  bool mfInitParticleSystem();
  bool mfSetDefaultEmitter(SEmitter *emitter);
  bool mfInitEmitter(SEmitter *emitter);

  bool mfAddParticle(SEmitter *emitter, SParticleInfo *pi);
  bool mfUpdateParticle(SParticle *particle,SEmitter *emitter);
  void mfEmitSparks(SParticle *p, SEmitter *em);

  bool mfUpdateEmitter(SEmitter *emitter);    // DRAW THE SYSTEM FOR A FRAME
  SEmitter *mfGetEmitter(void) { return &mEmitter; }

  // Parsing
  void mfCompileParticleInfo(SShader *ef, SParticleInfo *pi, char *scr);
  void mfCompileCollision(SShader *ef, SEmitter *em, char *scr, char *Collision);
  bool mfCompileMove(SShader *ef, SPartMoveStage *pm, SParticleInfo *pi, char *scr);
  bool mfCompileMoveTypeSqueeze(SShader *ef, SPartMoveStage *pm, char *scr);
  bool mfCompileMoveTypeWhirl(SShader *ef, SPartMoveStage *pm, char *scr);
  bool mfCompileMoveTypeWave(SShader *ef, SPartMoveStage *pm, char *scr);

public:
  CREParticleSpray()
  {
    mfSetType(eDATA_ParticleSpray);
    mfUpdateFlags(FCEF_TRANSFORM | FCEF_NEEDFILLBUF);
    mfInitEmitter(&mEmitter);
  }
  virtual ~CREParticleSpray();
  //CREParticleSpray(CREParticleSpray *Orig);
  CREParticleSpray& operator = (const CREParticleSpray& src);

  // CRendElement interface
  virtual void mfPrepare();
  virtual bool mfCull(CCObject *obj);

  virtual CRendElement *mfCopyConstruct(void)
  {
    //CREParticleSpray *ps = new CREParticleSpray;
    //*ps = this;
    return this;
  }

  virtual bool mfIsValidTime(SShader *ef, CCObject *obj, float curtime);
  virtual bool mfCompile(SShader *ef, char *scr);

  static SParticleStat mRS;
  static void mfPrintStat();

#ifdef DEBUGALLOC
#undef new
#endif
  void* operator new( size_t Size )
  {
    void *ptr = malloc(Size);
    memset(ptr, 0, Size);
    return ptr;
  }
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
};


#endif  // __RENDELEMENT_H__
