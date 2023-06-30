/*=============================================================================
  GLVPrograms.h : OpenGL vertex programming interface declaration.
  Copyright 1999 K&M. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#ifndef __GLVPROGRAMS_H__
#define __GLVPROGRAMS_H__

#define VPF_TRACK_MATRIX 1

#define VPVST_FOGGLOBAL  1
#define VPVST_CLIPPLANES 2
#define VPVST_NOFOG      4

#define VPFI_NOFOG      1
#define VPFI_UNIFIEDPOS 2
#define VPFI_SUPPORTMULTVLIGHTS 4

class CVProgram_GLBase : public CVProgram
{
public:
  CVProgram_GLBase(GLenum tgt)
  {
    m_Target = tgt;
    for (int i=0; i<8; i++)
    {
      m_bValid[i] = false;
      m_Program[i] = 0;
    }
  }
  
  virtual ~CVProgram_GLBase()
  {
    for (int i=0; i<8; i++)
    {
      mfDel(i);
    }
  }
  
  void mfBind(int Num)
  {
    if(!m_bValid[Num])
      mfGen(Num);
    glBindProgramNV(m_Target, m_Program[Num]);
  }
  
  void mfUnbind()
  {
    glBindProgramNV(m_Target, 0);
  }
  
  void mfLoad(GLuint size, const GLubyte * prog_text, int Num)
  {
    if(!m_bValid[Num])
      mfGen(Num);
    glLoadProgramNV(m_Target, m_Program[Num], size, prog_text);
    GLint errpos;
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_NV, &errpos);
    if(errpos != -1)
    {
      iLog->Log("program error:\n");
      int bgn = errpos - 10;
      bgn < 0 ? 0 : bgn;
      const char * c = (const char *)(prog_text + bgn);
      for(int i = 0; i < 30; i++)
      {
        if(bgn+i >= int(size-1))
          break;
        iLog->Log("%c", *c++);
      }
      iLog->Log("\n");
    }
  }

  void mfLoad(GLuint size, const char * prog_text, int Num)
  {
    if(!m_bValid[Num])
      mfGen(Num);
    glLoadProgramNV(m_Target, m_Program[Num], size, (const GLubyte *) prog_text);
    GLint errpos;
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_NV, &errpos);
    if(errpos != -1)
    {
      char name[128];
      sprintf(name, "%s.vps", m_Name);
      FILE *fp = fopen(name, "w");
      fprintf(fp, prog_text);
      fclose(fp);
      iLog->Log("program error: (see file '%s')\n", name);
      int bgn = errpos - 10;
      bgn < 0 ? 0 : bgn;
      const char * c = (const char *)(prog_text + bgn);
      for(int i = 0; i < 30; i++)
      {
        if(bgn+i >= int(size-1))
          break;
        iLog->Log("%c", *c++);
      }
      iLog->Log("\n");
    }
  }

  void mfLoad(const char * prog_text, int Num)
  {
    mfLoad(strlen(prog_text), prog_text, Num);
  }

  // convenience methods
  void mfParameter(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
  {
    glProgramParameter4fNV(m_Target, index, x, y, z, w);
  }

  void mfParameter(GLuint index, GLfloat * v)
  {
    glProgramParameter4fvNV(m_Target, index, v);
  }
  static void mfParameter(GLenum target, GLuint index, GLfloat * v)
  {
    glProgramParameter4fvNV(target, index, v);
  }
  void mfParameter3(GLuint index, const Vec3d& v)
  { 
    GLfloat f[4];
    f[0] = v[0];
    f[1] = v[1];
    f[2] = v[2];
    f[3] = 1;
    glProgramParameter4fvNV(m_Target, index, f);
  }
  void mfParameter4(GLuint index, const vec4_t v)
  {
    glProgramParameter4fvNV(m_Target, index, v);
  }

  void mfTrackMatrix(GLuint index, GLenum matrix, GLenum op)
  {
    glTrackMatrixNV(m_Target, index, matrix, op);
  }

  void mfEnable() { glEnable(m_Target); }
  void mfDisable() { glDisable(m_Target); }

  void mfDel(int Num)
  {
    if(m_bValid[Num])
      glDeleteProgramsNV(1, &m_Program[Num]);
    m_bValid[Num] = false;
  }
  
  bool mfIsValid(int Num) const { return m_bValid[Num]; }
  
protected:
  
  void mfGen(int Num)
  {
    glGenProgramsNV(1, &m_Program[Num]);
    m_bValid[Num] = true;
  }
  
  bool m_bValid[8];
  GLenum m_Target;
  GLuint m_Program[8];
};

struct STrackMatrix
{
  GLuint m_Address;
  GLenum m_Matrix;
  GLenum m_Transform;
};


class CVProgram_GL : public CVProgram_GLBase
{
  friend class CShader;

  uint m_Flags;
  char *m_Script;
  TArray<STrackMatrix> m_TrackMatrix;
  TArray<SParam> m_Params;
  TArray<SArrayPointer *> m_Pointers;

  void mfSetPointers(int nType);
  bool mfActivate(int Num);

public:
  char *mfCreateAdditionalVP(int Num);

  CVProgram_GL() : CVProgram_GLBase(GL_VERTEX_PROGRAM_NV)
  {
    m_bCGType = false;
    m_Flags = 0;
    m_Script = NULL;
  }
  virtual int Size()
  {
    int nSize = sizeof(*this);
    if (m_Script)
      nSize += strlen(m_Script)+1;
    nSize += m_Params.GetSize() * sizeof(SParam);
    nSize += m_TrackMatrix.GetSize() * sizeof(STrackMatrix);
    nSize += m_Pointers.GetSize() * sizeof(SArrayPointer *);

    return nSize;
  }

  virtual ~CVProgram_GL();
  virtual bool mfCompile(char *scr, SShader *ef);
  virtual bool mfSet(bool bStat, int nSetPointers=1);
  virtual void mfSetVariables(TArray<SParam>* Vars);
  virtual void mfReset();
  virtual bool mfHasPointer(EPointer ePtr);
};    



#endif  // __GLVPROGRAMS_H__
