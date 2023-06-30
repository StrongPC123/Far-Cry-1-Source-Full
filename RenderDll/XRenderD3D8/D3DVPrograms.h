/*=============================================================================
  D3DVPrograms.h : Direct3D vertex programming interface declaration.
  Copyright 1999 K&M. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#ifndef __D3DVPROGRAMS_H__
#define __D3DVPROGRAMS_H__

#define VPF_TRACK_MATRIX 1

#define VPVST_FOGGLOBAL  1
#define VPVST_CLIPPLANES 2
#define VPVST_NOFOG      4
#define VPVST_USETANGENTS 0x10

#define VPFI_NOFOG      1
#define VPFI_UNIFIEDPOS 2
#define VPFI_SUPPORTMULTVLIGHTS 4

struct STrackMatrix
{
  uint m_Address;
  int m_Matrix;
  int m_Transform;
};

class CVProgram_D3D : public CVProgram
{
  static int m_LastTypeVP;

  void *m_pCode[4];
  char *m_Script;
  int m_CodeSize[4];
  uint m_Flags;

  int m_dwShader[8][4][32]; // different handles for each declaration (VertexFormat)

  TArray<SParam> m_Params;
  TArray<STrackMatrix> m_TrackMatrix;
  TArray<SArrayPointer *> m_Pointers;

  void mfTrackMatrix(int Address, int Matrix, int Transform);

  char *mfCreateAdditionalVP(int Num);
  bool mfActivate(int Num, int Streams, int VertFormat);

public:
  static CVProgram *m_LastVP;

  virtual int Size()
  {
    return 0;
  }
  CVProgram_D3D() : CVProgram()
  {
    m_bCGType = false;
    m_Flags = 0;
    int i, j, n;
    m_Script = NULL;
    for (i=0; i<4; i++)
    {
      m_CodeSize[i] = 0;
    }
    
    m_Flags = 0;
    for (i=0; i<4; i++)
    {
      m_pCode[i] = NULL;
      for (j=0; j<4; j++)
      {
        for (n=0; n<32; n++)
        {
          m_dwShader[i][j][n] = 0;
        }
      }
    }
  }
  void mfParameter(int index, float * v);

  virtual ~CVProgram_D3D();
  virtual bool mfCompile(char *scr, SShader *sh);
  virtual bool mfSet(bool bStat, int nSetPointers=1);
  virtual void mfSetVariables(TArray<SParam>* Vars);
  virtual bool mfHasPointer(EPointer ePtr);
};    

#define MT_MODELVIEW_PROJECTION 1
#define MT_MODELVIEW            2
#define MT_MATRIX0              3
#define MT_MATRIX1              4
#define MT_MATRIX2              5
#define MT_MATRIX3              6
#define MT_MATRIX4              7
#define MT_MATRIX5              8
#define MT_MATRIX6              9
#define MT_MATRIX7              10
#define MT_TEXTURE              11

#define MTR_IDENTITY            1
#define MTR_INVERSE             2
#define MTR_TRANSPOSE           3
#define MTR_INVERSE_TRANSPOSE   4

#endif  // __D3DVPROGRAMS_H__
