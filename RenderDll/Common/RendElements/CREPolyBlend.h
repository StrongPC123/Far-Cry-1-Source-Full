
#ifndef __CREPOLYBLEND_H__
#define __CREPOLYBLEND_H__

//=============================================================

#define FCEFPB_TRACE  0x10000
#define FCEFPB_ORTHO  0x20000
#define FCEFPB_SCALE  0x40000
#define FCEFPB_PLANET 0x80000

enum ePBColStyle
{
  ePBCS_None,
  ePBCS_Decay,
};

enum ePBType
{
  ePBT_Sprite,
  ePBT_Beam,
};

struct SPolyBlendStat
{
  int NumPolys;
  int NumRendPolys;
  int NumAnimPolys;
  int NumAnimRendPolys;
  int NumVerts;
  int NumIndices;
};

class CREPolyBlend_Base : public CRendElement
{
public:
  float ScaleX, ScaleY;
  int NumOrients;
  SOrient *Orients[16];
  ePBColStyle eColStyle;
  ePBColStyle eAlphaStyle;
  ePBType eType; 
  float LiveTime;
  float LiveTimeA;
  float Val0;
  float Val1;
  float ValA0;
  float ValA1;

public:
  CREPolyBlend_Base()
  {
    ScaleX = ScaleY = 0;
    eColStyle = ePBCS_None;
    eType = ePBT_Sprite;
    LiveTime = 0;
  }

  virtual bool mfCompile(SShader *ef, char *scr);

protected:
  bool mfPrepareRB(CCObject *obj, Vec3d& orgo, CFColor& col);
  void mfSetVerts(CCObject *obj, Vec3d& orgo, uint c, SOrient *ori);

  void mfCompileOrients(SShader *ef, int *nums, SOrient *Orients[], char *scr);
};

class CREPolyBlend : public CREPolyBlend_Base
{
public:

  static SPolyBlendStat mRS;
  static void mfPrintStat();

public:
  CREPolyBlend()
  {
    mfSetType(eDATA_PolyBlend);
    Val0 = Val1 = 0;
    mfSetFlags(FCEF_NEEDFILLBUF);
  }

  virtual ~CREPolyBlend() {};

  virtual void mfPrepare();
  virtual bool mfCull(CCObject *obj);
  virtual CRendElement *mfCopyConstruct(void)
  {
    CREPolyBlend *pb = new CREPolyBlend;
    *pb = *this;
    return pb;
  }
  virtual bool mfIsValidTime(SShader *ef, CCObject *obj, float curtime);
};


class CREAnimPolyBlend : public CREPolyBlend_Base
{
public:
  int curNum;

public:
  CREAnimPolyBlend() : CREPolyBlend_Base()
  {
    mfSetType(eDATA_AnimPolyBlend);
    mfSetFlags(FCEF_NEEDFILLBUF);
  }
  virtual ~CREAnimPolyBlend() {};

  virtual void mfPrepare();
  virtual bool mfCull(CCObject *obj);
  virtual CRendElement *mfCopyConstruct(void)
  {
    CREAnimPolyBlend *apb = new CREAnimPolyBlend;
    *apb = *this;
    return apb;
  }
  virtual bool mfIsValidTime(SShader *ef, CCObject *obj, float curtime);
};

#endif  // __CREPOLYBLEND_H__
