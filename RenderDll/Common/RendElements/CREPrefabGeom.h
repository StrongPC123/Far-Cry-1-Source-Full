 
#ifndef __CREPREFABGEOM_H__
#define __CREPREFABGEOM_H__

//=============================================================

struct SREPrefabStat
{
  int NumRendPolys;
  int NumVerts;
  int NumIndices;
};

class CModelCgf;

class CREPrefabGeom : public CRendElement
{
public:
  void *mModel;

  static SREPrefabStat mRS;
  static void mfPrintStat();

public:
  CREPrefabGeom()
  {
    mfSetType(eDATA_Prefab);
    mModel = 0;
    mfSetFlags(FCEF_TRANSFORM);
  }

  virtual ~CREPrefabGeom() {};

  bool mfCullBox(Vec3d vmin, Vec3d vmax);
  virtual CRendElement *mfCopyConstruct(void);
  virtual bool mfCompile(SShader *ef, char *scr);
  virtual void mfPrepare();
  virtual bool mfDraw(SShader *ef, SShaderPass *sl);
};

#endif  // __CREPREFABGEOM_H__
