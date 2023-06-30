 
#ifndef __CREPOLYMESH_H__
#define __CREPOLYMESH_H__

//=============================================================

struct SPolyStat
{
  int NumOccPolys;
  int NumRendPolys;
  int NumDetails;
  int NumRendDetails;
  int NumVerts;
  int NumIndices;
};

class CREFlareGeom;

struct SMTriVert
{
  Vec3d vert;
  float dTC[2];
  float lmTC[2];
  UCol color;
};

class CREPolyMesh : public CRendElement
{
private:
  
public:
  Plane m_Plane;
  void *Srf;
  int NumVerts;
  int NumIndices;
  SMTriVert *TriVerts;
  bool *bNoDeform;
  ushort *Indices;

  int NumLightRE;
  int mFrameBr;

  static SPolyStat mRS;
  static void mfPrintStat();

public:
  CREPolyMesh()
  {
    mfSetType(eDATA_Poly);
    mfSetFlags(FCEF_TRANSFORM | FCEF_NEEDFILLBUF);
    NumLightRE = 0;
    bNoDeform = NULL;
    TriVerts = NULL;
    Indices = NULL;
  }
  virtual ~CREPolyMesh();
  
  void mfCheckSun(SShader *ef);
  bool mfCullFace(ECull cl);

  virtual CRendElement *mfCopyConstruct(void);
  virtual void mfCenter(Vec3d& centr, CCObject*pObj);
  virtual int mfTransform(Matrix44& ViewMatr, Matrix44& ProjMatr, vec4_t *verts, vec4_t *vertsp, int Num);
  virtual void mfPrepare(void);
  virtual void mfGetPlane(Plane& pl);
};


#endif  // __CREPOLYMESH_H__
