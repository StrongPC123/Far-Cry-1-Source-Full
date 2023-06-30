/*=============================================================================
  GLPBuffer.cpp : implementation of the NVidia Pixel Buffers.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honich Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "GL_Renderer.h"
#include "GLPBuffer.h"


CPBuffer::CPBuffer( int w, int h, unsigned int mode ) : m_Width(w), m_Height(h), m_Mode(mode), m_MyDC(NULL), m_MyGLctx(NULL), m_Buffer(NULL)
{
}

CPBuffer::~CPBuffer()
{
  if (m_Buffer)
  {
    pwglDeleteContext(m_MyGLctx);
    wglReleasePbufferDCARB(m_Buffer, m_MyDC);
    wglDestroyPbufferARB(m_Buffer);
  }
}

// Check to see if the pbuffer was lost.
// If it was lost, destroy it and then recreate it.
void CPBuffer::mfHandleModeSwitch()
{
  int lost = 0;

  wglQueryPbufferARB(m_Buffer, WGL_PBUFFER_LOST_ARB, &lost);

  if ( lost )
  {
    this->~CPBuffer();
    mfInitialize();
  }
}

// This function actually does the creation of the p-buffer.
// It can only be called once a window has already been created.
bool CPBuffer::mfInitialize(bool bShare)
{
  HDC hdc = pwglGetCurrentDC();
  HGLRC hglrc = pwglGetCurrentContext();

  // Query for a suitable pixel format based on the specified mode.
  int   iattributes[2*MAX_ATTRIBS];
  float fattributes[2*MAX_ATTRIBS];
  int nfattribs = 0;
  int niattribs = 0;

  // Attribute arrays must be "0" terminated - for simplicity, first
  // just zero-out the array entire, then fill from left to right.
  for ( int a = 0; a < 2*MAX_ATTRIBS; a++ )
  {
    iattributes[a] = 0;
    fattributes[a] = 0;
  }

  // Since we are trying to create a pbuffer, the pixel format we
  // request (and subsequently use) must be "p-buffer capable".
  iattributes[2*niattribs  ] = WGL_DRAW_TO_PBUFFER_ARB;
  iattributes[2*niattribs+1] = true;
  niattribs++;

  if (m_Mode & FPB_DRAWTOTEXTURE)
  {
    iattributes[2*niattribs  ] = WGL_BIND_TO_TEXTURE_RGB_ARB;
    iattributes[2*niattribs+1] = true;
    niattribs++;
  }
  else
  {
    if ( m_Mode & FPB_INDEX )
    {
      iattributes[2*niattribs  ] = WGL_PIXEL_TYPE_ARB;
      iattributes[2*niattribs+1] = WGL_TYPE_COLORINDEX_ARB;  // Yikes!
      niattribs++;
    }
    else
    {
      iattributes[2*niattribs  ] = WGL_PIXEL_TYPE_ARB;
      iattributes[2*niattribs+1] = WGL_TYPE_RGBA_ARB;
      niattribs++;
    }

    if ( m_Mode & FPB_DOUBLE )
    {
      iattributes[2*niattribs  ] = WGL_DOUBLE_BUFFER_ARB;
      iattributes[2*niattribs+1] = true;
      niattribs++;
    }

    if ( m_Mode & FPB_DEPTH )
    {
      iattributes[2*niattribs  ] = WGL_DEPTH_BITS_ARB;
      iattributes[2*niattribs+1] = 1;
      niattribs++;
    }

    if ( m_Mode & FPB_STENCIL )
    {
      iattributes[2*niattribs  ] = WGL_STENCIL_BITS_ARB;
      iattributes[2*niattribs+1] = 1;
      niattribs++;
    }

    if ( m_Mode & FPB_ACCUM )
    {
      iattributes[2*niattribs  ] = WGL_ACCUM_BITS_ARB;
      iattributes[2*niattribs+1] = 1;
      niattribs++;
    }

    iattributes[2*niattribs  ] = WGL_SUPPORT_OPENGL_ARB;
    iattributes[2*niattribs+1] = true;
    niattribs++;
  }

  int format;
  int pformat[MAX_PFORMATS];
  unsigned int nformats;
  if ( !wglChoosePixelFormatARB( hdc, iattributes, fattributes, MAX_PFORMATS, pformat, &nformats ) )
  {
    iLog->Log("Warning: PBuffer creation error:  Couldn't find a suitable pixel format.\n" );
    return false;
  }
  format = pformat[0];

  // Create the p-buffer.
  iattributes[0] = 0;
  niattribs = 0;
  if (m_Mode & FPB_DRAWTOTEXTURE)
  {
    iattributes[2*niattribs  ] = WGL_TEXTURE_FORMAT_ARB;
    iattributes[2*niattribs+1] = WGL_TEXTURE_RGBA_ARB;
    niattribs++;

    iattributes[2*niattribs  ] = WGL_TEXTURE_TARGET_ARB;
    iattributes[2*niattribs+1] = WGL_TEXTURE_2D_ARB;
    niattribs++;

    iattributes[2*niattribs  ] = WGL_MIPMAP_TEXTURE_ARB;
    iattributes[2*niattribs+1] = 0;
    niattribs++;

    iattributes[2*niattribs  ] = WGL_PBUFFER_LARGEST_ARB;
    iattributes[2*niattribs+1] = 0;
    niattribs++;

    iattributes[2*niattribs  ] = 0;
  }
  m_Buffer = wglCreatePbufferARB( hdc, format, m_Width, m_Height, iattributes );
  if ( !m_Buffer )
  {
    DWORD err = GetLastError();
    iLog->Log("Warning: PBuffer creation error:  wglCreatePbufferARB() failed\n" );
    if ( err == ERROR_INVALID_PIXEL_FORMAT )
    {
      iLog->Log("Warning: ERROR_INVALID_PIXEL_FORMAT\n" );
    }
    else
    if ( err == ERROR_NO_SYSTEM_RESOURCES )
    {
      iLog->Log("Warning: ERROR_NO_SYSTEM_RESOURCES\n" );
    }
    else
    if ( err == ERROR_INVALID_DATA )
    {
      iLog->Log("Warning: ERROR_INVALID_DATA\n" );
    }
    return false;
  }

  // Get the device context.
  m_MyDC = wglGetPbufferDCARB( m_Buffer );
  if ( !m_MyDC )
  {
    iLog->Log("Warning: PBuffer creation error:  wglGetPbufferDCARB() failed\n" );
    return false;
  }

  // Create a gl context for the p-buffer.
  m_MyGLctx = pwglCreateContext( m_MyDC );
  if ( !m_MyGLctx )
  {
    iLog->Log("Warning: PBuffer creation error:  wglCreateContext() failed\n" );
    return false;
  }

  if( bShare )
  {
    if( !pwglShareLists(hglrc, m_MyGLctx) )
    {
      iLog->Log("Warning: PBuffer: wglShareLists() failed\n" );
      return false;
    }
  }

  // Determine the actual width and height we were able to create.
  wglQueryPbufferARB( m_Buffer, WGL_PBUFFER_WIDTH_ARB, &m_Width );
  wglQueryPbufferARB( m_Buffer, WGL_PBUFFER_HEIGHT_ARB, &m_Height );

#ifdef _DEBUG
  iLog->Log("Created a %d x %d PBuffer\n", m_Width, m_Height );
#endif

  return true;
}

bool CPBuffer::mfMakeCurrent()
{
  if ( !pwglMakeCurrent( m_MyDC, m_MyGLctx ) )
  {
    iLog->Log("Warning: CPBuffer::mfMakeCurrent() failed.\n" );
    return false;
  }
  return true;
}

bool CPBuffer::mfMakeMainCurrent()
{
  CGLRenderer *rd = gcpOGL;
  if ( !pwglMakeCurrent( rd->m_CurrContext->m_hDC, rd->m_CurrContext->m_hRC ) )
  {
    iLog->Log("Warning: CPBuffer::mfMakeMainCurrent() failed.\n" );
    return false;
  }
  return true;
}

BOOL CPBuffer::mfTextureBind()
{
  return wglBindTexImageARB(m_Buffer, WGL_FRONT_LEFT_ARB);
}

BOOL CPBuffer::mfReleaseFromTexture()
{
  return wglReleaseTexImageARB(m_Buffer, WGL_FRONT_LEFT_ARB);
}
