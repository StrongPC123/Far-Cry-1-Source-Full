/*=============================================================================
  GLPBuffer.h : OpenGL PBuffer interface declaration.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

    Revision history:
    * Created by Honitch Andrey
  
=============================================================================*/

#ifndef PBUFFERS_H
#define PBUFFERS_H

#define MAX_PFORMATS 256
#define MAX_ATTRIBS  32

#define FPB_SINGLE  1
#define FPB_INDEX   2
#define FPB_DOUBLE  4
#define FPB_DEPTH   8
#define FPB_STENCIL 0x10
#define FPB_ACCUM   0x20
#define FPB_DRAWTOTEXTURE 0x40

class CPBuffer
{
private:
  HDC          m_MyDC;      // Handle to a device context.
  HGLRC        m_MyGLctx;   // Handle to a GL context.
  HPBUFFERARB  m_Buffer;    // Handle to a pbuffer.
  unsigned int m_Mode;      // Flags indicating the type of pbuffer.
public:
  int          m_Width;
  int          m_Height;
  CPBuffer( int width, int height, unsigned int mode );
  ~CPBuffer();
  void  mfHandleModeSwitch();
  bool mfMakeCurrent();
  bool mfMakeMainCurrent();
  BOOL mfTextureBind();
  BOOL mfReleaseFromTexture();
  bool mfInitialize(bool share = false);
};

#endif