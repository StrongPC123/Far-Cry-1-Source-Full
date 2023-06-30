
#ifndef __CRESKYZONE_H__
#define __CRESKYZONE_H__

//=============================================================
class CRESkyZone : public CRendElement
{
public:
  Vec3d mViewPos;
  Vec3d mMins;
  Vec3d mMaxs;
  Vec3d mCenter;
  float mRadius;
  byte *mPVS;

  CRESkyZone()
  {
    mfSetType(eDATA_SkyZone);
    mfUpdateFlags(FCEF_TRANSFORM | FCEF_NODEL);
    mPVS = NULL;
  }

  virtual ~CRESkyZone()
  {
    if (mPVS)
      delete [] mPVS;
  }

  virtual bool mfCompile(SShader *ef, char *scr);
};

#endif  // __CRESKYZONE_H__
