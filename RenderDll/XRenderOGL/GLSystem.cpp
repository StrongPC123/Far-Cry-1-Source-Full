/*=============================================================================
  GLSystem.cpp : HW depended OpenGL functions and extensions handling.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;


#include "RenderPCH.h"
#include "GL_Renderer.h"
#include "cg\cgGL.h"
#include "GLCGVProgram.h"
#include "GLCGPShader.h"


#ifdef USE_3DC
#include "../Common/3Dc/CompressorLib.h"
void (*DeleteDataATI)(void *pData);
COMPRESSOR_ERROR (*CompressTextureATI)(DWORD width,
                                                 DWORD height,
                                                 UNCOMPRESSED_FORMAT sourceFormat,
                                                 COMPRESSED_FORMAT destinationFormat,
                                                 void    *inputData,
                                                 void    **dataOut,
                                                 DWORD   *outDataSize);
#endif


///////////////////////////////////////////////////////////////////
// Gamma Functions
///////////////////////////////////////////////////////////////////

void CGLRenderer::CheckGammaSupport(void)
{
  m_Features &= ~RFT_HWGAMMA;

  if (CV_r_nohwgamma)
    return;

  m_Features |= RFT_HWGAMMA;
}

void CGLRenderer::SetDeviceGamma(ushort *r, ushort *g, ushort *b)
{
  ushort gamma[3][256];
  int i;

  if (!(GetFeatures() & RFT_HWGAMMA))
    return;

  if (CV_r_nohwgamma)
    return;

  if (!m_CurrContext->m_hDC)
    return;

  for (i=0; i<256; i++)
  {
    gamma[0][i] = r[i];
    gamma[1][i] = g[i];
    gamma[2][i] = b[i];
  }
  if (SUPPORTS_WGL_3DFX_gamma_control)
    wglSetDeviceGammaRamp3DFX(m_CurrContext->m_hDC, gamma);
  else
    SetDeviceGammaRamp(m_CurrContext->m_hDC, gamma);
}

void CGLRenderer::RestoreDeviceGamma(void)
{
  if (!(GetFeatures() & RFT_HWGAMMA))
    return;

  if (CV_r_nohwgamma)
    return;

  HDC dc;

  struct
  {
    ushort red[256];
    ushort green[256];
    ushort blue[256];
  } Ramp;

  for(int i=0; i<256; i++)
  {
    Ramp.red[i] = Ramp.green[i] = Ramp.blue[i] = i << 8;
  }

  dc = GetDC(GetDesktopWindow());
  if (SUPPORTS_WGL_3DFX_gamma_control)
    wglSetDeviceGammaRamp3DFX(dc, &Ramp);
  else
    SetDeviceGammaRamp(dc, &Ramp);
  ReleaseDC(GetDesktopWindow(), dc);
}


void CGLRenderer::SetGamma(float fGamma, float fBrightness, float fContrast)
{
  int i;

  fGamma = CLAMP(fGamma, 0.5f, 3.0f);
  if (m_fLastGamma == fGamma && m_fLastBrightness == fBrightness && m_fLastContrast == fContrast)
    return;

  //int n;
  for ( i=0; i<256; i++ )
  {
    m_GammaTable[i] = CLAMP((int)((fContrast+0.5f)*cry_powf((float)i/255.f,1.0f/fGamma)*65535.f + (fBrightness-0.5f)*32768.f - fContrast*32768.f + 16384.f), 0, 65535);
    /*if ( fGamma == 1.0f )
      n = i;
    else
    {
      float f = (float)i;
      n = (int)(pow(f/255.0, 1.0/fGamma) * 255.0 + 0.5);
    }
    if ( n < 0 )
      n = 0;
    if ( n > 255 )
      n = 255;
    m_GammaTable[i] = n | (n<<8); */
  }

  m_fLastGamma = fGamma;
  m_fLastBrightness = fBrightness;
  m_fLastContrast = fContrast;

  SetDeviceGamma(m_GammaTable, m_GammaTable, m_GammaTable);
}

bool CGLRenderer::SetGammaDelta(const float fGamma)
{
  m_fDeltaGamma = fGamma;
  SetGamma(CV_r_gamma + fGamma, CV_r_brightness, CV_r_contrast);
  return true;
}

//================================================================================
// Library funcs

void CGLRenderer::FreeLibrary()
{
  if (m_hLibHandle)
  {
    ::FreeLibrary((HINSTANCE)m_hLibHandle);
    m_hLibHandle = NULL;
  }
  if (m_hLibHandleGDI)
  {
    ::FreeLibrary((HINSTANCE)m_hLibHandleGDI);
    m_hLibHandleGDI = NULL;
  }

  // Free OpenGL function.
  #define GL_EXT(name) SUPPORTS##name=0;
  #define GL_PROC(ext,ret,func,parms) func = NULL;
  #include "GLFuncs.h"
  #undef GL_EXT
  #undef GL_PROC
}

bool CGLRenderer::LoadLibrary()
{
  strcpy(m_LibName, "OpenGL32.dll");

  m_hLibHandle = ::LoadLibrary(m_LibName);
  if (!m_hLibHandle)
  {
    iLog->Log("Warning: Couldn't load library <%s>\n", m_LibName);
    return false;
  }
  m_hLibHandleGDI = ::LoadLibrary("GDI32.dll");
  assert(m_hLibHandleGDI);

  return true;
}

typedef const char * (WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC) (HDC hdc);
typedef const char * (WINAPI * PFNWGLGETEXTENSIONSSTRINGEXTPROC) ();
static PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
static PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT;

#define GET_GL_PROC(functype,funcname) \
  static functype funcname = (functype)pwglGetProcAddress(#funcname);\
  assert(funcname);\

bool CGLRenderer::FindExt( const char* Name )
{
  char *str = (char*)glGetString(GL_EXTENSIONS);
  if (strstr(str, Name))
    return true;

  GET_GL_PROC(PFNWGLGETEXTENSIONSSTRINGARBPROC,wglGetExtensionsStringARB);
  if(wglGetExtensionsStringARB)
  {
    const char * wglExt = wglGetExtensionsStringARB(m_RContexts[0]->m_hDC);
    if (wglExt)
      return (strstr(wglExt, Name) != NULL);
  }

  return false;
}

void CGLRenderer::FindProc( void*& ProcAddress, char* Name, char* SupportName, byte& Supports, bool AllowExt )
{
  if (Name[0] == 'p')
    Name = &Name[1];
  if( !ProcAddress )
    ProcAddress = GetProcAddress( (HINSTANCE)m_hLibHandle, Name );
  if( !ProcAddress )
    ProcAddress = GetProcAddress( (HINSTANCE)m_hLibHandleGDI, Name );
  if( !ProcAddress && Supports && AllowExt )
    ProcAddress = pwglGetProcAddress( Name );
  if( !ProcAddress )
  {
    if( Supports )
      iLog->Log("Warning:   Missing function '%s' for '%s' support\n", Name, SupportName );
    Supports = 0;
  }
}

void CGLRenderer::FindProcs( bool AllowExt )
{
  #define GL_EXT(name) if( AllowExt ) SUPPORTS##name = FindExt( TEXT(#name)+1 );
  #define GL_PROC(ext,ret,func,parms) FindProc( *(void**)&func, #func, #ext, SUPPORTS##ext, AllowExt );
  #include "GLFuncs.h"
  #undef GL_EXT
  #undef GL_PROC
}

bool CGLRenderer::CheckOGLExtensions(void)
{
  CTexMan::m_bRGBA = true;

  iLog->Log("\n...Check OpenGL extensions\n");

  FindProcs( true );

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_ARB_texture_compression)
    iLog->Log("  ...GL_ARB_texture_compression not found\n");
  else
  if (CV_gl_arb_texture_compression && CV_r_supportcompressedtextures)
  {
    m_Features |= RFT_COMPRESSTEXTURE;
    iLog->Log("  ...using GL_ARB_texture_compression\n");
  }
  else
  {
    SUPPORTS_GL_ARB_texture_compression = 0;
    iLog->Log("  ...ignoring GL_ARB_texture_compression\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_WGL_EXT_swap_control)
    iLog->Log("  ...WGL_EXT_swap_control not found\n");
  else
  if (CV_gl_ext_swapcontrol)
    iLog->Log("  ...using WGL_EXT_swap_control\n");
  else
  {
    SUPPORTS_WGL_EXT_swap_control = 0;
    wglSwapIntervalEXT = NULL;
    iLog->Log("  ...ignoring WGL_EXT_swap_control\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_WGL_ARB_render_texture)
    iLog->Log("  ...WGL_ARB_render_texture not found\n");
  else
  if (CV_gl_arb_render_texture)
    iLog->Log("  ...using WGL_ARB_render_texture\n");
  else
  {
    SUPPORTS_WGL_ARB_render_texture = 0;
    iLog->Log("  ...ignoring WGL_ARB_render_texture\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_WGL_3DFX_gamma_control)
    iLog->Log("  ...WGL_3DFX_gamma_control not found\n");
  else
  if (CV_gl_3dfx_gamma_control)
    iLog->Log("  ...using WGL_3DFX_gamma_control\n");
  else
  {
    SUPPORTS_WGL_3DFX_gamma_control = 0;
    wglGetDeviceGammaRamp3DFX = NULL;
    wglSetDeviceGammaRamp3DFX = NULL;
    iLog->Log("  ...ignoring WGL_3DFX_gamma_control\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_ARB_multisample)
    iLog->Log("  ...GL_ARB_multisample not found\n");
  else
  if (CV_gl_arb_multisample)
    iLog->Log("  ...using GL_ARB_multisample\n");
  else
  {
    SUPPORTS_GL_ARB_multisample = 0;
    iLog->Log("  ...ignoring GL_ARB_multisample\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_WGL_ARB_pbuffer)
    iLog->Log("  ...WGL_ARB_pbuffer not found\n");
  else
  if (CV_gl_arb_pbuffer)
    iLog->Log("  ...using WGL_ARB_pbuffer\n");
  else
  {
    SUPPORTS_WGL_ARB_pbuffer = 0;
    iLog->Log("  ...ignoring WGL_ARB_pbuffer\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_WGL_ARB_pixel_format)
    iLog->Log("  ...WGL_ARB_pixel_format not found\n");
  else
  if (CV_gl_arb_pixel_format)
    iLog->Log("  ...using WGL_ARB_pixel_format\n");
  else
  {
    SUPPORTS_WGL_ARB_pixel_format = 0;
    iLog->Log("  ...ignoring WGL_ARB_pixel_format\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_WGL_ARB_buffer_region)
    iLog->Log("  ...WGL_ARB_buffer_region not found\n");
  else
  if (CV_gl_arb_buffer_region)
    iLog->Log("  ...using WGL_ARB_buffer_region\n");
  else
  {
    SUPPORTS_WGL_ARB_buffer_region = 0;
    iLog->Log("  ...ignoring WGL_ARB_buffer_region\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_ARB_texture_env_combine)
    iLog->Log("  ...GL_ARB_texture_env_combine not found\n");
  else
  if (CV_gl_arb_texture_env_combine)
    iLog->Log("  ...using GL_ARB_texture_env_combine\n");
  else
  {
    SUPPORTS_GL_ARB_texture_env_combine = 0;
    iLog->Log("  ...ignoring GL_ARB_texture_env_combine\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_SGIX_depth_texture)
  {
    iLog->Log("  ...GL_SGIX_depth_texture not found\n");
  }
  else
  if (CV_gl_sgix_depth_texture)
  {
#ifdef WIN64
    //m_Features |= RFT_DEPTHMAPS;
#else
    m_Features |= RFT_DEPTHMAPS;
#endif
    iLog->Log("  ...using GL_SGIX_depth_texture\n");
  }
  else
  {
    SUPPORTS_GL_SGIX_depth_texture = 0;
    iLog->Log("  ...ignoring GL_SGIX_depth_texture\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_SGIX_shadow)
    iLog->Log("  ...GL_SGIX_shadow not found\n");
  else
  if (CV_gl_sgix_shadow)
    iLog->Log("  ...using GL_SGIX_shadow\n");
  else
  {
    SUPPORTS_GL_SGIX_shadow = 0;
    iLog->Log("  ...ignoring GL_SGIX_shadow\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_SGIS_generate_mipmap)
    iLog->Log("  ...GL_SGIS_generate_mipmap not found\n");
  else
  if (CV_gl_sgis_generate_mipmap)
    iLog->Log("  ...using GL_SGIS_generate_mipmap\n");
  else
  {
    SUPPORTS_GL_SGIS_generate_mipmap = 0;
    iLog->Log("  ...ignoring GL_SGIS_generate_mipmap\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_SGIS_texture_lod)
    iLog->Log("  ...GL_SGIS_texture_lod not found\n");
  else
  if (CV_gl_sgis_texture_lod)
    iLog->Log("  ...using GL_SGIS_texture_lod\n");
  else
  {
    SUPPORTS_GL_SGIS_texture_lod = 0;
    iLog->Log("  ...ignoring GL_SGIS_texture_lod\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_ARB_multitexture)
    iLog->Log("  ...GL_ARB_multitexture not found\n");
  else
  if (CV_gl_arb_multitexture)
  {
    iLog->Log("  ...using GL_ARB_multitexture\n");
    m_Features |= RFT_MULTITEXTURE;
  }
  else
  {
    SUPPORTS_GL_ARB_multitexture = 0;
    iLog->Log("  ...ignoring GL_ARB_multitexture\n");
  }

  if (!(GetFeatures() & RFT_MULTITEXTURE))
  {
    glActiveTextureARB = NULL;
    glClientActiveTextureARB = NULL;
    glMultiTexCoord2fARB = NULL;
    glMultiTexCoord2fvARB = NULL;

    m_MaxActiveTexturesARBFixed = 0;
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_texgen_reflection)
    iLog->Log("  ...GL_NV_texgen_reflection not found\n");
  else
  if (CV_gl_nv_texgen_reflection)
  {
    iLog->Log("  ...using GL_NV_texgen_reflection\n");
    m_Features |= RFT_TEXGEN_REFLECTION;
  }
  else
  {
    SUPPORTS_GL_NV_texgen_reflection = 0;
    iLog->Log("  ...ignoring GL_NV_texgen_reflection\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_HP_occlusion_test)
    iLog->Log("  ...GL_HP_occlusion_test not found\n");
  else
  if (CV_gl_hp_occlusion_test)
  {
    iLog->Log("  ...using GL_HP_occlusion_test\n");
    m_Features |= RFT_OCCLUSIONTEST;
  }
  else
  {
    SUPPORTS_GL_HP_occlusion_test = 0;
    iLog->Log("  ...ignoring GL_HP_occlusion_test\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_occlusion_query)
    iLog->Log("  ...GL_NV_occlusion_query not found\n");
  else
  if (CV_gl_hp_occlusion_test)
  {
    iLog->Log("  ...using GL_NV_occlusion_query\n");
    m_Features |= RFT_OCCLUSIONTEST;
  }
  else
  {
    SUPPORTS_GL_NV_occlusion_query = 0;
    iLog->Log("  ...ignoring GL_NV_occlusion_query\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_multisample_filter_hint)
    iLog->Log("  ...GL_NV_multisample_filter_hint not found\n");
  else
  if (CV_gl_nv_multisample_filter_hint)
    iLog->Log("  ...using GL_NV_multisample_filter_hint\n");
  else
  {
    SUPPORTS_GL_NV_multisample_filter_hint = 0;
    iLog->Log("  ...ignoring GL_NV_multisample_filter_hint\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_fog_distance)
    iLog->Log("  ...GL_NV_fog_distance not found\n");
  else
  if (CV_gl_nv_fog_distance)
  {
    iLog->Log("  ...using GL_NV_fog_distance\n");
  }
  else
  {
    SUPPORTS_GL_NV_fog_distance = 0;
    iLog->Log("  ...ignoring GL_NV_fog_distance\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_texgen_emboss)
    iLog->Log("  ...GL_NV_texgen_emboss not found\n");
  else
  if (CV_gl_nv_texgen_emboss)
  {
    iLog->Log("  ...using GL_NV_texgen_emboss\n");
    m_Features |= RFT_TEXGEN_EMBOSS;
  }
  else
  {
    SUPPORTS_GL_NV_texgen_emboss = 0;
    iLog->Log("  ...ignoring GL_NV_texgen_emboss\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_EXT_separate_specular_color)
    iLog->Log("  ...GL_EXT_separate_specular_color not found\n");
  else
  if (CV_gl_ext_separate_specular_color)
  {
    iLog->Log("  ...using GL_EXT_separate_specular_color\n");
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SEPARATE_SPECULAR_COLOR_EXT);
  }
  else
  {
    SUPPORTS_GL_EXT_separate_specular_color = 0;
    iLog->Log("  ...ignoring GL_EXT_separate_specular_color\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_EXT_texture_env_combine)
    iLog->Log("  ...GL_EXT_texture_env_combine not found\n");
  else
  if (CV_gl_ext_texture_env_combine)
    iLog->Log("  ...using GL_EXT_texture_env_combine\n");
  else
  {
    SUPPORTS_GL_EXT_texture_env_combine = 0;
    iLog->Log("  ...ignoring GL_EXT_texture_env_combine\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_EXT_multi_draw_arrays)
    iLog->Log("  ...GL_EXT_multi_draw_arrays not found\n");
  else
  if (CV_gl_ext_multi_draw_arrays)
  {
    iLog->Log("  ...using GL_EXT_multi_draw_arrays\n");
  }
  else
  {
    SUPPORTS_GL_EXT_multi_draw_arrays = 0;
    iLog->Log("  ...ignoring GL_EXT_multi_draw_arrays\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_texture_env_combine4)
    iLog->Log("  ...GL_NV_texture_env_combine4 not found\n");
  else
  if (CV_gl_nv_texture_env_combine4)
  {
    iLog->Log("  ...using GL_NV_texture_env_combine4\n");
    if (GetFeatures() & RFT_MULTITEXTURE)
      m_Features |= RFT_DETAILTEXTURE;
  }
  else
  {
    SUPPORTS_GL_NV_texture_env_combine4 = 0;
    iLog->Log("  ...ignoring GL_NV_texture_env_combine4\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_point_sprite)
    iLog->Log("  ...GL_NV_point_sprite not found\n");
  else
  if (CV_gl_nv_point_sprite)
  {
    iLog->Log("  ...using GL_NV_point_sprite\n");
  }
  else
  {
    SUPPORTS_GL_NV_point_sprite = 0;
    iLog->Log("  ...ignoring GL_NV_point_sprite\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_vertex_array_range)
    iLog->Log("  ...GL_NV_vertex_array_range not found\n");
  else
  if (CV_gl_nv_vertex_array_range)
    iLog->Log("  ...using GL_NV_vertex_array_range\n");
  else
  {
    SUPPORTS_GL_NV_vertex_array_range = 0;
    iLog->Log("  ...ignoring GL_NV_vertex_array_range\n");
    wglAllocateMemoryNV = NULL;
    glVertexArrayRangeNV = NULL;
    wglFreeMemoryNV = NULL;
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_fence)
    iLog->Log("  ...GL_NV_fence not found\n");
  else
  if (CV_gl_nv_fence)
    iLog->Log("  ...using GL_NV_fence\n");
  else
  {
    SUPPORTS_GL_NV_fence = 0;
    iLog->Log("  ...ignoring GL_NV_fence\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_EXT_compiled_vertex_array)
    iLog->Log("  ...GL_EXT_compiled_vertex_array not found.\n");
  else
  if ( CV_gl_ext_compiled_vertex_array)
    iLog->Log("  ...using GL_EXT_compiled_vertex_array.\n");
  else
  {
    SUPPORTS_GL_EXT_compiled_vertex_array = 0;
    glLockArraysEXT = NULL;
    glUnlockArraysEXT = NULL;
    iLog->Log("  ...ignoring GL_EXT_compiled_vertex_array.\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_EXT_texture_env_add)
    iLog->Log("  ...GL_EXT_texture_env_add not found.\n");
  else
  if ( CV_gl_ext_texture_env_add)
    iLog->Log("  ...using GL_EXT_texture_env_add.\n");
  else
  {
    SUPPORTS_GL_EXT_texture_env_add = 0;
    iLog->Log("  ...ignoring GL_EXT_texture_env_add.\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_EXT_texture_filter_anisotropic)
    iLog->Log("  ...GL_EXT_texture_filter_anisotropic not found.\n");
  else
  if ( CV_gl_ext_texture_filter_anisotropic)
  {
    iLog->Log("  ...using GL_EXT_texture_filter_anisotropic.\n");
    glGetIntegerv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_MaxAnisotropicLevel );
    m_Features |= RFT_ALLOWANISOTROPIC;
    if (!CV_r_texture_anisotropic_level)
      CV_r_texture_anisotropic_level = 1;
  }
  else
  {
    SUPPORTS_GL_EXT_texture_filter_anisotropic = 0;
    iLog->Log("  ...ignoring GL_EXT_texture_filter_anisotropic.\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_EXT_bgra)
    iLog->Log("  ...GL_EXT_bgra not found.\n");
  else
  if ( CV_gl_ext_bgra)
    iLog->Log("  ...using GL_EXT_bgra.\n");
  else
  {
    SUPPORTS_GL_EXT_bgra = 0;
    iLog->Log("  ...ignoring GL_EXT_bgra.\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_EXT_depth_bounds_test)
    iLog->Log("  ...GL_EXT_depth_bounds_test not found.\n");
  else
  if (CV_gl_ext_depth_bounds_test)
    iLog->Log("  ...using GL_EXT_depth_bounds_test.\n");
  else
  {
    SUPPORTS_GL_EXT_depth_bounds_test = 0;
    iLog->Log("  ...ignoring GL_EXT_depth_bounds_test.\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_EXT_secondary_color)
    iLog->Log("  ...GL_EXT_secondary_color not found.\n");
  else
  if (CV_gl_ext_secondary_color)
  {
    iLog->Log("  ...using GL_EXT_secondary_color.\n");
    m_Features |= RFT_ALLOWSECONDCOLOR;
  }
  else
  {
    SUPPORTS_GL_EXT_secondary_color = 0;
    iLog->Log("  ...ignoring GL_EXT_secondary_color.\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_EXT_paletted_texture)
    iLog->Log("  ...GL_EXT_paletted_texture not found.\n");
  else
  if (CV_gl_ext_paletted_texture && CV_r_supportpalettedtextures)
  {
    iLog->Log("  ...using GL_EXT_paletted_texture.\n");
    m_Features |= RFT_PALTEXTURE;
  }
  else
  {
    SUPPORTS_GL_EXT_paletted_texture = 0;
    iLog->Log("  ...ignoring GL_EXT_paletted_texture.\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_EXT_stencil_two_side)
    iLog->Log("  ...GL_EXT_stencil_two_side not found.\n");
  else
  if (CV_gl_ext_stencil_two_side)
    iLog->Log("  ...using GL_EXT_stencil_two_side.\n");
  else
  {
    SUPPORTS_GL_EXT_stencil_two_side = 0;
    iLog->Log("  ...ignoring GL_EXT_stencil_two_side.\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_EXT_stencil_wrap)
    iLog->Log("  ...GL_EXT_stencil_wrap not found.\n");
  else
  if (CV_gl_ext_stencil_wrap)
    iLog->Log("  ...using GL_EXT_stencil_wrap.\n");
  else
  {
    SUPPORTS_GL_EXT_stencil_wrap = 0;
    iLog->Log("  ...ignoring GL_EXT_stencil_wrap.\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_register_combiners)
    iLog->Log("  ...GL_NV_register_combiners not found.\n");
  else
  if (CV_gl_nv_register_combiners)
  {
    iLog->Log("  ...using GL_NV_register_combiners.\n");
    m_Features &= ~RFT_HW_MASK;
    m_Features |= RFT_HW_GF2;
    m_Features |= RFT_FOGVP;
  }
  else
  {
    SUPPORTS_GL_NV_register_combiners = 0;
    iLog->Log("  ...ignoring GL_NV_register_combiners.\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_register_combiners2)
    iLog->Log("  ...GL_NV_register_combiners2 not found.\n");
  else
  if (CV_gl_nv_register_combiners2)
  {
    iLog->Log("  ...using GL_NV_register_combiners2.\n");
    m_Features |= RFT_HW_RC;
    m_Features |= RFT_FOGVP;
    glEnable(GL_PER_STAGE_CONSTANTS_NV);
  }
  else
  {
    SUPPORTS_GL_NV_register_combiners2 = 0;
    iLog->Log("  ...ignoring GL_NV_register_combiners2.\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_vertex_program)
    iLog->Log("  ...GL_NV_vertex_program not found.\n");
  else
  if (CV_gl_nv_vertex_program)
  {
    iLog->Log("  ...using GL_NV_vertex_program.\n");
    m_Features |= RFT_HW_VS;
    m_Features |= RFT_FOGVP;
  }
  else
  {
    SUPPORTS_GL_NV_vertex_program = 0;
    iLog->Log("  ...ignoring GL_NV_vertex_program3.\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_ARB_vertex_program)
    iLog->Log("  ...GL_ARB_vertex_program not found.\n");
  else
  if (CV_gl_arb_vertex_program)
  {
    iLog->Log("  ...using GL_ARB_vertex_program.\n");
    m_Features |= RFT_HW_VS;
    m_Features |= RFT_FOGVP;
  }
  else
  {
    SUPPORTS_GL_ARB_vertex_program = 0;
    iLog->Log("  ...ignoring GL_ARB_vertex_program.\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_texture_shader)
    iLog->Log("  ...GL_NV_texture_shader not found.\n");
  else
  if (CV_gl_nv_texture_shader)
  {
    iLog->Log("  ...using GL_NV_texture_shader.\n");
    m_Features |= RFT_HW_TS;
    m_Features &= ~RFT_HW_MASK;
    m_Features |= RFT_HW_GF3;
    m_Features |= RFT_FOGVP;
  }
  else
  {
    SUPPORTS_GL_NV_texture_shader = 0;
    iLog->Log("  ...ignoring GL_NV_texture_shader.\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_texture_shader2)
    iLog->Log("  ...GL_NV_texture_shader2 not found.\n");
  else
  if (CV_gl_nv_texture_shader2)
  {
    iLog->Log("  ...using GL_NV_texture_shader2.\n");
    m_Features |= RFT_HW_TS;
    m_Features |= RFT_FOGVP;
  }
  else
  {
    SUPPORTS_GL_NV_texture_shader2 = 0;
    iLog->Log("  ...ignoring GL_NV_texture_shader2.\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_texture_shader3)
    iLog->Log("  ...GL_NV_texture_shader3 not found.\n");
  else
  if (CV_gl_nv_texture_shader3)
  {
    iLog->Log("  ...using GL_NV_texture_shader3.\n");
    m_Features |= RFT_HW_TS | RFT_HW_ENVBUMPPROJECTED;
  }
  else
  {
    SUPPORTS_GL_NV_texture_shader3 = 0;
    iLog->Log("  ...ignoring GL_NV_texture_shader3.\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_texture_rectangle)
    iLog->Log("  ...GL_NV_texture_rectangle not found.\n");
  else
  if (CV_gl_nv_texture_rectangle)
  {
    iLog->Log("  ...using GL_NV_texture_rectangle.\n");
    m_Features |= RFT_ALLOWRECTTEX;
  }
  else
  {
    SUPPORTS_GL_NV_texture_rectangle = 0;
    iLog->Log("  ...ignoring GL_NV_texture_rectangle.\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_EXT_texture_rectangle)
    iLog->Log("  ...GL_EXT_texture_rectangle not found.\n");
  else
  if (CV_gl_ext_texture_rectangle && !SUPPORTS_GL_NV_texture_rectangle)
  {
    iLog->Log("  ...using GL_EXT_texture_rectangle.\n");
    m_Features |= RFT_ALLOWRECTTEX;
  }
  else
  {
    SUPPORTS_GL_EXT_texture_rectangle = 0;
    iLog->Log("  ...ignoring GL_EXT_texture_rectangle.\n");
  }

/////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_EXT_texture_cube_map)
    iLog->Log("  ...GL_EXT_texture_cube_map not found.\n");
  else
  if (CV_gl_ext_texture_cube_map)
  {
    iLog->Log("  ...using GL_EXT_texture_cube_map.\n");
  }
  else
  {
    SUPPORTS_GL_EXT_texture_cube_map = 0;
    iLog->Log("  ...ignoring GL_EXT_texture_cube_map.\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_ATI_separate_stencil)
    iLog->Log("  ...GL_ATI_separate_stencil not found.\n");
  else
  if (CV_gl_ati_separate_stencil)
  {
    iLog->Log("  ...using GL_ATI_separate_stencil.\n");
  }
  else
  {
    SUPPORTS_GL_ATI_separate_stencil = 0;
    iLog->Log("  ...ignoring GL_ATI_separate_stencil.\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_ATI_fragment_shader)
    iLog->Log("  ...GL_ATI_fragment_shader not found.\n");
  else
  if (CV_gl_ati_fragment_shader)
  {
    iLog->Log("  ...using GL_ATI_fragment_shader.\n");
    m_Features &= ~RFT_HW_MASK;
    m_Features |= RFT_HW_RADEON;
  }
  else
  {
    SUPPORTS_GL_ATI_fragment_shader = 0;
    iLog->Log("  ...ignoring GL_ATI_fragment_shader.\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_ARB_vertex_buffer_object)
    iLog->Log("  ...GL_ARB_vertex_buffer_object not found.\n");
  else
  if (CV_gl_arb_vertex_buffer_object || !SUPPORTS_GL_NV_fence)
  {
    SUPPORTS_GL_ATI_fragment_shader = 0;
    SUPPORTS_GL_NV_fence = 0;
    SUPPORTS_GL_NV_vertex_array_range = 0;
    iLog->Log("  ...using GL_ARB_vertex_buffer_object.\n");
  }
  else
  {
    SUPPORTS_GL_ARB_vertex_buffer_object = 0;
    iLog->Log("  ...ignoring GL_ARB_vertex_buffer_object.\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_NV_fragment_program)
    iLog->Log("  ...GL_NV_fragment_program not found.\n");
  else
  if (CV_gl_nv_fragment_program)
  {
    iLog->Log("  ...using GL_NV_fragment_program.\n");
    SUPPORTS_GL_ATI_fragment_shader = 0;
    SUPPORTS_GL_ARB_fragment_program = 0;
    m_Features |= RFT_HW_TS | RFT_HW_PS20;
    m_Features &= ~RFT_HW_MASK;
    m_Features |= RFT_HW_GFFX;
    m_Features |= RFT_HW_ENVBUMPPROJECTED;
    //m_MaxActiveTexturesARB = Max(m_MaxActiveTexturesARB, 6);
  }
  else
  {
    SUPPORTS_GL_NV_fragment_program = 0;
    iLog->Log("  ...ignoring GL_NV_fragment_program.\n");
  }


  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_ARB_fragment_program)
    iLog->Log("  ...GL_ARB_fragment_program not found.\n");
  else
  if (CV_gl_arb_fragment_program && !SUPPORTS_GL_NV_fragment_program)
  {
    iLog->Log("  ...using GL_ARB_fragment_program.\n");
    SUPPORTS_GL_ATI_fragment_shader = 0;
    m_Features |= RFT_HW_TS | RFT_HW_PS20;
    if (((m_Features & RFT_HW_MASK) != RFT_HW_RADEON))
    {
      m_Features &= ~RFT_HW_MASK;
      m_Features |= RFT_HW_GFFX;
    }
    m_Features |= RFT_HW_ENVBUMPPROJECTED;
    m_Features |= RFT_FOGVP;
    //m_MaxActiveTexturesARB = Max(m_MaxActiveTexturesARB, 6);
  }
  else
  {
    SUPPORTS_GL_ARB_fragment_program = 0;
    iLog->Log("  ...ignoring GL_ARB_fragment_program.\n");
  }


  if (!SUPPORTS_GL_NV_vertex_program3)
    iLog->Log("  ...GL_NV_vertex_program3 not found.\n");
  else
  if (CV_gl_nv_vertex_program3)
  {
    iLog->Log("  ...using GL_NV_vertex_program3.\n");
    m_Features |= RFT_HW_VS;
    m_Features |= RFT_FOGVP;
    m_Features &= ~RFT_HW_MASK;
    m_Features |= RFT_HW_NV4X;
    if (SUPPORTS_GL_ARB_vertex_program)
      SUPPORTS_GL_NV_vertex_program = 0;
  }
  else
  {
    SUPPORTS_GL_NV_vertex_program3 = 0;
    iLog->Log("  ...ignoring GL_NV_vertex_program3.\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////

  if (!SUPPORTS_GL_ARB_texture_env_combine)
    iLog->Log("  ...GL_ARB_texture_env_combine not found.\n");
  else
  if (CV_gl_arb_fragment_program)
  {
    iLog->Log("  ...using GL_ARB_texture_env_combine.\n");
  }
  else
  {
    SUPPORTS_GL_ARB_texture_env_combine = 0;
    iLog->Log("  ...ignoring GL_ARB_texture_env_combine.\n");
  }

  m_Features |= RFT_BUMP;
  if (SUPPORTS_GL_ARB_fragment_program || (m_Features & RFT_DEPTHMAPS))
    m_Features |= RFT_SHADOWMAP_SELFSHADOW;
  if (CV_r_shadowtype == 1)
    m_Features &= ~(RFT_SHADOWMAP_SELFSHADOW | RFT_DEPTHMAPS);

  iLog->Log("\n");


  if (SUPPORTS_GL_ARB_fragment_program)
  {
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &m_MaxActiveTexturesARB_VP);
    glGetIntegerv(GL_MAX_TEXTURES_UNITS_ARB, &m_MaxActiveTexturesARBFixed);
  }
  else
  {
    glGetIntegerv(GL_MAX_TEXTURES_UNITS_ARB, &m_MaxActiveTexturesARBFixed);
    m_MaxActiveTexturesARB_VP = m_MaxActiveTexturesARBFixed;
  }
  m_MaxActiveTexturesARB_VP = min(m_MaxActiveTexturesARB_VP, MAX_TMU);
  m_MaxActiveTexturesARBFixed = min(m_MaxActiveTexturesARBFixed, MAX_TMU);

  return true;
}

void CGLRenderer::MakeCurrent()
{
  int i;

  if (pwglGetCurrentContext() == m_RContexts[0]->m_hRC)
    return;
  if (pwglMakeCurrent(m_RContexts[0]->m_hDC, m_RContexts[0]->m_hRC) != TRUE)
	{
		if (iLog)
			iLog->LogToFile( "Warning: MakeCurrent of OpenGL context failed!" );
	}
  for (i=0; i<96; i++)
  {
    CCGVProgram_GL::m_CurParams[i][0] = -999999;
    CCGVProgram_GL::m_CurParams[i][1] = -999999;
    CCGVProgram_GL::m_CurParams[i][2] = -999999;
    CCGVProgram_GL::m_CurParams[i][3] = -999999;
  }
  CVProgram::m_LastVP = 0;
  CPShader::m_CurRC = NULL;
}

void CGLRenderer::ShareResources( IRenderer *renderer )
{
  // Ignore share with myself!
  if (renderer == this)
    return;

  // Assume that renderer is of same type.
  CGLRenderer *glRend = (CGLRenderer*)renderer;

  if (pwglShareLists( m_CurrContext->m_hRC, glRend->m_CurrContext->m_hRC ) == FALSE)
  {
    if (iLog)
      iLog->Log("Warning: Sharing of OpenGL resources failed!" );
  }
}


//////////////////////////////////////////////////////////////////////
bool CGLRenderer::SetupPixelFormat(unsigned char colorbits,unsigned char zbits,unsigned char sbits,SRendContext *rc)
{
#ifdef WIN32
  iLog->LogToFile("SetupPixelFormat ...");

  PIXELFORMATDESCRIPTOR pfd;
  int pixelFormat;

  memset(&pfd, 0, sizeof(pfd));

  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
  pfd.dwLayerMask = PFD_MAIN_PLANE;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = colorbits;
  pfd.cAlphaBits = CV_gl_alpha_bits;
  pfd.cDepthBits = zbits;
  pfd.cAccumBits = 0;
  pfd.cStencilBits = sbits;

  BOOL status = FALSE;

  if (m_RContexts.Num() && rc != m_RContexts[0])
    pixelFormat = m_RContexts[0]->m_PixFormat;
  else
    pixelFormat = ChoosePixelFormat(m_CurrContext->m_hDC, &pfd);
  m_CurrContext->m_PixFormat = pixelFormat;

  if (!pixelFormat)
  {
    iLog->LogToFile("Cannot ChoosePixelFormat \n");
    return (false);
  }

  iLog->LogToFile("Selected cColorBits = %d, cDepthBits = %d, Stencilbits=%d\n", pfd.cColorBits, pfd.cDepthBits,pfd.cStencilBits);

  iLog->LogToFile("GL PIXEL FORMAT:\n");

  memset(&pfd, 0, sizeof(pfd));
  if (DescribePixelFormat(m_CurrContext->m_hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd))
  {
    if (pfd.dwFlags & PFD_DRAW_TO_WINDOW)   iLog->LogToFile("PFD_DRAW_TO_WINDOW\n");
    if (pfd.dwFlags & PFD_DRAW_TO_BITMAP)   iLog->LogToFile("PFD_DRAW_TO_BITMAP\n");

    if (pfd.dwFlags & PFD_SUPPORT_GDI)      iLog->LogToFile("PFD_SUPPORT_GDI\n");

    if (pfd.dwFlags & PFD_SUPPORT_OPENGL)   iLog->LogToFile("PFD_SUPPORT_OPENGL\n");
    if (pfd.dwFlags & PFD_GENERIC_ACCELERATED)  iLog->LogToFile("PFD_GENERIC_ACCELERATED\n");

    if (pfd.dwFlags & PFD_NEED_PALETTE)     iLog->LogToFile("PFD_NEED_PALETTE\n");
    if (pfd.dwFlags & PFD_NEED_SYSTEM_PALETTE)  iLog->LogToFile("PFD_NEED_SYSTEM_PALETTE\n");
    if (pfd.dwFlags & PFD_DOUBLEBUFFER)     iLog->LogToFile("PFD_DOUBLEBUFFER\n");
    if (pfd.dwFlags & PFD_STEREO)       iLog->LogToFile("PFD_STEREO\n");
    if (pfd.dwFlags & PFD_SWAP_LAYER_BUFFERS) iLog->LogToFile("PFD_SWAP_LAYER_BUFFERS\n");
    iLog->LogToFile("Pixel format %d:\n",pixelFormat);
    iLog->LogToFile("Pixel Type: %d\n",pfd.iPixelType);
    iLog->LogToFile("Bits: Color=%d R=%d G=%d B=%d A=%d\n", pfd.cColorBits, pfd.cRedBits, pfd.cGreenBits, pfd.cBlueBits, pfd.cAlphaBits);
    iLog->LogToFile("Bits: Accum=%d Depth=%d Stencil=%d\n", pfd.cAccumBits, pfd.cDepthBits, pfd.cStencilBits);

    iLog->LogToFile("Using cColorBits = %d, cDepthBits = %d, Stencilbits=%d\n", pfd.cColorBits, pfd.cDepthBits,pfd.cStencilBits);

    if (pfd.dwFlags & PFD_GENERIC_FORMAT)
    {
      iLog->LogToFile("PFD_GENERIC_FORMAT\n");
      return (false);
    }

    m_abpp = pfd.cAlphaBits;
    m_sbpp = pfd.cStencilBits;
    m_cbpp = pfd.cColorBits;
    m_zbpp = pfd.cDepthBits;
  }
  else
  {
    iLog->LogToFile("Warning: Cannot DescribePixelFormat \n");
    return (false);
  }

  // set the format to closest match
  SetPixelFormat(m_CurrContext->m_hDC, pixelFormat, &pfd);

  return (true);

#else

  return (false);

#endif
}

//! Return all supported by video card video AA formats
int CGLRenderer::EnumAAFormats(TArray<SAAFormat>& Formats, bool bReset)
{
  return 0;
}

int CGLRenderer::EnumDisplayFormats(TArray<SDispFormat>& Formats, bool bReset)
{
  SDispFormat frm;
  Formats.Free();

  if (bReset)
    return 0;

  int i, j;
  for (i=0; i<m_numvidmodes; i++)
  {
    DEVMODE *m = &m_vidmodes[i];
    if (m->dmPelsWidth < 640 || m->dmPelsHeight < 480 || m->dmBitsPerPel < 15)
      continue;
    for (j=0; j<Formats.Num(); j++)
    {
      SDispFormat *f = &Formats[j];
      if (f->m_Width == m->dmPelsWidth && f->m_Height == m->dmPelsHeight && f->m_BPP == m->dmBitsPerPel)
        break;
    }
    if (j == Formats.Num())
    {
      frm.m_Width = m->dmPelsWidth;
      frm.m_Height = m->dmPelsHeight;
      frm.m_BPP = m->dmBitsPerPel;
      if (frm.m_BPP == 24)
        frm.m_BPP = 32;
      Formats.AddElem(frm);
    }
  }
  return Formats.Num();
}

HWND CGLRenderer::SetMode(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen,HINSTANCE hinst, HWND Glhwnd)
{
  ///////////////////////////////////Get Desktop Settings

  HWND temp = GetDesktopWindow();
  RECT trect;

  GetWindowRect(temp, &trect);

  m_deskwidth =trect.right-trect.left;
  m_deskheight=trect.bottom-trect.top;

  HDC hDC = GetDC (NULL);
  m_deskbpp = GetDeviceCaps( hDC, BITSPIXEL );
//  m_deskfreq= GetDeviceCaps( hDC, VREFRESH );
  ReleaseDC (NULL,hDC);


  iLog->Log("Desktop settings: %d x %d x %d",m_deskwidth,m_deskheight,m_deskbpp/*,m_deskfreq*/);
  iLog->Log("Available video modes:\n");


  ///////////////////////////////////Find available video modes

  int nummode;
  for (nummode=0;;nummode++)
  {
    DEVMODE Tmp;
    ZeroMemory(&Tmp,sizeof(Tmp));
    Tmp.dmSize = sizeof(Tmp);
    if (!EnumDisplaySettings(NULL,nummode,&Tmp))
      break;
  } //nummode

  if (nummode>0)
  {
    if (m_vidmodes)
      delete [] m_vidmodes;
    m_vidmodes = new DEVMODE [m_numvidmodes=nummode];
  }

  for (nummode=0;;nummode++)
  {
    DEVMODE Tmp;
    ZeroMemory(&Tmp,sizeof(Tmp));
    Tmp.dmSize = sizeof(Tmp);

    if (!EnumDisplaySettings(NULL,nummode,&Tmp))
      break;

    m_vidmodes[nummode]=Tmp;
  } //nummode

  if (m_numvidmodes==0)
  {
    iLog->Log("Warning: Cannot find at least one video mode available\n");
    return (NULL);
  }

  ///////////////////////////////////Find the best video mode
  if (width < 640)
    width = 640;
  if (height < 480)
    height = 480;
  if (cbpp < 16)
    cbpp = 16;

  int best_mode=0;

  for (int mode = 0; mode < m_numvidmodes; mode++)
  {
    if (((int)(m_vidmodes[mode].dmPelsWidth)  >= (int)(m_vidmodes[best_mode].dmPelsWidth)) &&
      ((int)(m_vidmodes[mode].dmPelsWidth)  <= width) &&
      ((int)(m_vidmodes[mode].dmPelsHeight) >= (int)(m_vidmodes[best_mode].dmPelsHeight)) &&
      ((int)(m_vidmodes[mode].dmPelsHeight) <= height) &&
      ((int)(m_vidmodes[mode].dmBitsPerPel) >= (int)(m_vidmodes[best_mode].dmBitsPerPel)) &&
      ((int)(m_vidmodes[mode].dmBitsPerPel) <= cbpp))
      best_mode = mode;
  }

///////////////////////////////////Create window

  if (width==m_deskwidth && height==m_deskheight)
    fullscreen=true;

	char szWinTitle[80];
	sprintf(szWinTitle,"- Far Cry - %s (%s)",__DATE__, __TIME__);

  if (fullscreen)
  {
    if (!ChangeDisplay(m_vidmodes[best_mode].dmPelsWidth,m_vidmodes[best_mode].dmPelsHeight,m_vidmodes[best_mode].dmBitsPerPel))
      return (false);
    Glhwnd = CreateWindowEx(WS_EX_APPWINDOW,
                      "CryENGINE", szWinTitle,
                      WS_POPUP |  WS_CLIPSIBLINGS,
                      0, 0, width, height,
                      NULL, NULL, hinst, NULL);

    m_width=m_vidmodes[best_mode].dmPelsWidth;
    m_height=m_vidmodes[best_mode].dmPelsHeight;

    SetCursor(NULL);
    ShowCursor(FALSE);
  }
  else
  {
    if(!Glhwnd)
		{
			WNDCLASS wc;

			if (!GetClassInfo(hinst, "CryENGINE", &wc))
			{

				wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
				wc.lpfnWndProc   = DefWindowProc;
				wc.cbClsExtra    = 4;
				wc.cbWndExtra    = 4;
				wc.hInstance     = hinst;
				wc.hIcon         = NULL;
				wc.hCursor       = NULL;
				wc.hbrBackground = (HBRUSH)(BLACK_BRUSH);
				wc.lpszMenuName  = NULL;
				wc.lpszClassName = "CryENGINE";

				RegisterClass(&wc);
			}

      Glhwnd = CreateWindowEx(WS_EX_APPWINDOW,
                  "CryENGINE", szWinTitle,
                  WS_BORDER | WS_DLGFRAME | WS_CLIPSIBLINGS | WS_SYSMENU,
                  (GetSystemMetrics(SM_CXFULLSCREEN)-width)/2,
                  (GetSystemMetrics(SM_CYFULLSCREEN)-height)/2,
                  GetSystemMetrics(SM_CXDLGFRAME)*2 + width,
                  GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXDLGFRAME)*2 + height,
                  NULL, NULL, hinst, NULL);
		}

    DWORD err=GetLastError();

    m_width = width;
    m_height = height;
  }
  m_VX = m_VY = 0;
  m_VWidth = m_width;
  m_VHeight = m_height;

  if (!Glhwnd)
  {
    iLog->Log("Error: CreateWindowEx\n");
    return (false);
  }

  m_FullScreen = fullscreen;

  if (!m_FullScreen)
    SetWindowPos(Glhwnd, HWND_NOTOPMOST, (GetSystemMetrics(SM_CXFULLSCREEN)-width)/2, (GetSystemMetrics(SM_CYFULLSCREEN)-height)/2, GetSystemMetrics(SM_CXDLGFRAME)*2 + width, GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXDLGFRAME)*2 + height, SWP_SHOWWINDOW);

  //SAFE_DELETE_ARRAY(m_vidmodes);

  return Glhwnd;
}

//////////////////////////////////////////////////////////////////////

typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBIVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues);
typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBFVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues);
typedef BOOL (WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef PROC (WINAPI * PFNWGLGETPROCADDRESSPROC) (LPCSTR str);

// Determine if an OpenGL WGL extension is supported.
//
// NOTE:  This routine uses wglGetProcAddress so this routine REQUIRES
// that the calling thread is bound to a hardware-accelerated OpenGL
// rendering context.
int CGLRenderer::_wglExtensionSupported(const char *extension)
{
  // Lazy initialization - We don't know if we've initialized this yet, so do it here.
  wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC) pwglGetProcAddress("wglGetExtensionsStringEXT");
  wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC) pwglGetProcAddress("wglGetExtensionsStringARB");

  if (wglGetExtensionsStringARB || wglGetExtensionsStringEXT)
  {
    static const GLubyte *extensions = NULL;
    const GLubyte *start;
    GLubyte *where, *terminator;

    // Extension names should not have spaces.
    where = (GLubyte *) strchr(extension, ' ');
    if (where || *extension == '\0')
      return 0;

    if (!extensions)
    {
      HDC hdc = GetDC(0);
      if (wglGetExtensionsStringARB)
        extensions = (const GLubyte *) wglGetExtensionsStringARB(hdc);
      else
        extensions = (const GLubyte *) wglGetExtensionsStringEXT();
      ::ReleaseDC(0,hdc);
    }

    // It takes a bit of care to be fool-proof about parsing the
    // OpenGL extensions string.  Don't be fooled by sub-strings,
    // etc.
    start = extensions;
    for (;;)
    {
      where = (GLubyte *) strstr((const char *) start, extension);
      if (!where)
        break;
      terminator = where + strlen(extension);
      if (where == start || *(where - 1) == ' ')
      {
        if (*terminator == ' ' || *terminator == '\0')
          return 1;
      }

      start = terminator;
    }
  }

  return 0;
}

//======================================================================

int nTexSize=0;
int nFrameTexSize=0;
static int curBind=-1;
int BindSizes[TX_LASTBIND];
int BindFrame[TX_LASTBIND];
int TargetTex[TX_LASTBIND];
static int bAutoMips = 0;

static _inline void sUpdateTexSize(int target, int width, int height, int depth, int internalFormat, int nLevel)
{
  if (target > GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT)
    return;

  int size;
  if (!nLevel && bAutoMips)
  {
    int wdt = width;
    int hgt = height;
    int dpt = depth;
    size = 0;
    while (wdt || hgt || dpt)
    {
      if (!wdt)
        wdt = 1;
      if (!hgt)
        hgt = 1;
      if (!dpt)
        dpt = 1;
      size += CGLTexMan::TexSize(wdt, hgt, dpt, internalFormat);
      wdt >>= 1;
      hgt >>= 1;
      dpt >>= 1;
    }
  }
  else
    size = CGLTexMan::TexSize(width, height, depth, internalFormat);

  assert (curBind >= 0 && curBind < TX_LASTBIND);
  if (target == GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT)
    size *= 6;

  if (!nLevel)
  {
    nTexSize -= BindSizes[curBind];
    BindSizes[curBind] = size;
  }
  else
  if (nLevel < 0)
  {
    int wdt = width;
    int hgt = height;
    int dpt = depth;
    int mips = 1-nLevel;
    size = 0;
    for (int i=0; i<mips; i++)
    {
      if (!wdt)
        wdt = 1;
      if (!hgt)
        hgt = 1;
      if (!dpt)
        dpt = 1;
      size += CGLTexMan::TexSize(wdt, hgt, dpt, internalFormat);
      wdt >>= 1;
      hgt >>= 1;
      dpt >>= 1;
    }
    BindSizes[curBind] = size;
  }
  else
    BindSizes[curBind] += size;
  nTexSize += size;
}

static int nPrevBindARB[2] = {-1,-1};
void (__stdcall *nnglBindBufferARB) (GLenum target, GLuint param);
void __stdcall nglBindBufferARB (GLenum target, GLuint param)
{
  if (nPrevBindARB[target-GL_ARRAY_BUFFER_ARB] != param)
  {
    nPrevBindARB[target-GL_ARRAY_BUFFER_ARB] = param;
    nnglBindBufferARB(target, param);
  }
}
void (__stdcall *nnglGenBuffersARB) (GLsizei n, GLuint *buffers);
void __stdcall nglGenBuffersARB (GLsizei n, GLuint *buffers)
{
  nnglGenBuffersARB(n, buffers);
  nPrevBindARB[0] = -1;
  nPrevBindARB[1] = -1;
}
void (__stdcall *nnglDeleteBuffersARB) (GLsizei n, const GLuint *buffers);
void __stdcall nglDeleteBuffersARB (GLsizei n, const GLuint *buffers)
{
  nnglDeleteBuffersARB(n, buffers);
  nPrevBindARB[0] = -1;
  nPrevBindARB[1] = -1;
}

void (__stdcall *nnglTexParameteri) (GLenum target, GLenum pname, GLint param);
void __stdcall nglTexParameteri (GLenum target, GLenum pname, GLint param)
{
  if (pname == GL_GENERATE_MIPMAP_SGIS)
    bAutoMips = param;
  nnglTexParameteri(target, pname, param);
}

void (__stdcall *nnglTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void __stdcall nglTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
  sUpdateTexSize(target, width, height, 1, internalformat, level);
  nnglTexImage2D (target, level, internalformat, width, height, border, format, type, pixels);
}

void (__stdcall *nnglBindTexture) (GLenum Parm0, GLuint Parm1);
void __stdcall nglBindTexture (GLenum Parm0, GLuint Parm1)
{
  /*float fSize = 0;
  for (int i=0; i<TX_LASTBIND; i++)
  {
    fSize += BindSizes[i];
  }
  fSize /= 1024.0f;
  fSize /= 1024.0f;*/
	assert (Parm1>=0 && Parm1 <TX_LASTBIND);

  curBind = Parm1;
  nnglBindTexture (Parm0, Parm1);
  TargetTex[Parm1] = Parm0;
  if (BindFrame[Parm1] != gcpOGL->GetFrameID())
  {
    BindFrame[Parm1] = gcpOGL->GetFrameID();
    nFrameTexSize += BindSizes[Parm1];
  }
  if (!Parm1)
    bAutoMips = 0;
}

void (__stdcall *nnglCompressedTexImage2DARB) (GLenum Parm0, GLint Parm1, GLenum Parm2, GLsizei Parm3, GLsizei Parm4, GLint Parm5, GLsizei Parm6, const GLvoid *Parm7);
void __stdcall nglCompressedTexImage2DARB (GLenum Parm0, GLint Parm1, GLenum Parm2, GLsizei Parm3, GLsizei Parm4, GLint Parm5, GLsizei Parm6, const GLvoid *Parm7)
{
  sUpdateTexSize(Parm0, Parm3, Parm4, 1, Parm2, Parm1);
  nnglCompressedTexImage2DARB (Parm0, Parm1, Parm2, Parm3, Parm4, Parm5, Parm6, Parm7);
}

void (__stdcall *nnglCopyTexImage2D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void __stdcall nglCopyTexImage2D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
  sUpdateTexSize(target, width, height, 1, internalFormat, level);
  nnglCopyTexImage2D (target, level, internalFormat, x, y, width, height, border);
}

void (__stdcall *nnglDeleteTextures) (GLsizei n, const GLuint *textures);
void __stdcall nglDeleteTextures (GLsizei n, const GLuint *textures)
{
  for (int i=0; i<n; i++)
  {
    nTexSize -= BindSizes[textures[i]];
    BindSizes[textures[i]] = 0;
  }
  nnglDeleteTextures (n, textures);
}


#ifndef PS2

WIN_HWND CGLRenderer::Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen,WIN_HINSTANCE hinst, WIN_HWND Glhwnd, WIN_HDC Glhdc, WIN_HGLRC hGLrc, bool bReInit)
{
#ifdef USE_3DC
    CompressTextureATI = 0;
    DeleteDataATI = 0;
#endif

  m_hInst = (HINSTANCE)hinst;
  if (bReInit)
  {
    CLeafBuffer *pLB = CLeafBuffer::m_Root.m_Prev;
    while(pLB != &CLeafBuffer::m_Root)
    {
      CLeafBuffer *Next = pLB->m_Prev;
      pLB->Unload();
      pLB = Next;
    }
    ShutDownVAR();
    FreeResources(FRR_TEXTURES | FRR_REINITHW);
    ShutDown(true);
  }

  if (!Glhdc)
    Glhwnd = SetMode(x, y, width, height, cbpp, zbpp, sbits, fullscreen, (HINSTANCE)hinst, (HWND)Glhwnd);
  else
  {
    m_width = width;
    m_height = height;
    m_FullScreen = fullscreen;
    m_VX = m_VY = 0;
    m_VWidth = m_width;
    m_VHeight = m_height;
  }
  if ( !LoadLibrary() )
  {
    iLog->Log("Error: Could not open OpenGL library\n");
exr:
    FreeLibrary();
    ShutDown();
    return NULL;
  }
  if (!m_RContexts.Num())
  {
    SRendContext *rc = new SRendContext;
    m_RContexts.AddElem(rc);
  }
  SRendContext *rc = m_RContexts[0];
  if (Glhdc)
  {
    rc->m_hDC = (HDC)Glhdc;
  }
  else
  {
    rc->m_hDC = GetDC((HWND)Glhwnd);
  }
  rc->m_Glhwnd = (HWND)Glhwnd;
  m_CurrContext = rc;

  // Find functions.
  SUPPORTS_GL = 1;
  FindProcs( false );
  if( !SUPPORTS_GL )
  {
    iLog->Log("Error: Library <%s> isn't OpenGL library\n", m_LibName);
    goto exr;
  }
  CreateRContext(rc, Glhdc, hGLrc, cbpp, zbpp, sbits, true);

  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_MaxTextureSize);
  if (CV_gl_maxtexsize)
  {
    if (CV_gl_maxtexsize < 128)
      CV_gl_maxtexsize = 128;
    else
    if (CV_gl_maxtexsize > 2048)
      CV_gl_maxtexsize = 2048;
    m_MaxTextureSize = min(CV_gl_maxtexsize, m_MaxTextureSize);
  }

  iLog->Log("****** OpenGL Driver Stats ******\n");
  iLog->Log("Driver: %s\n", m_LibName);
  m_VendorName = glGetString(GL_VENDOR);
  iLog->Log("GL_VENDOR: %s\n", m_VendorName);
  m_RendererName = glGetString(GL_RENDERER);
  iLog->Log("GL_RENDERER: %s\n", m_RendererName);
  m_VersionName = glGetString(GL_VERSION);
  iLog->Log("GL_VERSION: %s\n", m_VersionName);
  m_ExtensionsName = glGetString(GL_EXTENSIONS);
  iLog->LogToFile("GL_EXTENSIONS:\n");
  char ext[8192];
  char *token;
  strcpy(ext, (char *)m_ExtensionsName);
  token = strtok(ext, " ");
  while (token)
  {
    iLog->LogToFile("  %s\n", token);
    token = strtok(NULL, " ");
  }
  iLog->LogToFile("\n");

  GET_GL_PROC(PFNWGLGETEXTENSIONSSTRINGARBPROC,wglGetExtensionsStringARB);
  if(wglGetExtensionsStringARB)
  {
    const char * wglExt = wglGetExtensionsStringARB(m_CurrContext->m_hDC);
    if (wglExt)
    {
      iLog->LogToFile("WGL_EXTENSIONS:\n");

      strcpy(ext, (char *)wglExt);
      token = strtok(ext, " ");
      while (token)
      {
        iLog->LogToFile("  %s\n", token);
        token = strtok(NULL, " ");
      }
      iLog->LogToFile("\n");
    }
  }

  CheckOGLExtensions();
  CheckGammaSupport();
  ChangeLog();

  if (m_Features & RFT_HW_PS20)
  {
    if (CV_r_shadowtype == 0)
      m_Features |= RFT_SHADOWMAP_SELFSHADOW;
  }
  ICVar *var;
  int nGPU = m_Features & RFT_HW_MASK;

  var = iConsole->GetCVar("r_NoPS30");
  if (var)
    var->Set(1);
  m_NoPS30 = 1;
  var = iConsole->GetCVar("r_HDRRenderingForce");
  if (var)
    var->Set(0);
  var = iConsole->GetCVar("r_HDRRendering");
  if (var)
    var->Set(0);

  // Disable per-pixel lighting if pixel-shaders aren't supported
  if (!(m_Features & (RFT_HW_PS20 | RFT_HW_TS | RFT_HW_RC)))
  {
    if (CV_r_Quality_BumpMapping)
    {
      var = iConsole->GetCVar("r_Quality_BumpMapping");
      if (var)
        var->Set(0);
    }
    if (CV_r_checkSunVis >= 2)
    {
      var = iConsole->GetCVar("r_checkSunVis");
      if (var)
        var->Set(1);
    }
  }

  // Allow pixel shaders 2.0 lighting on Radeon only cards with PS.2.0 support
  if (CV_r_Quality_BumpMapping == 3)
  {
    if (!(m_Features & RFT_HW_PS20) || (nGPU == RFT_HW_GFFX && !CV_gl_nv30_ps20))
    {
      var = iConsole->GetCVar("r_Quality_BumpMapping");
      if (var)
        var->Set(2);
    }
  }

  // Shaders remapping for non-PS20 hardware
  if (!(m_Features & RFT_HW_PS20) || (nGPU == RFT_HW_GFFX && !CV_gl_nv30_ps20))
  {
    var = iConsole->GetCVar("r_NoPS20");
    if (var)
      var->Set(1);
  }

  // Disable trees per-pixel lighting from medium and low spec settings
  if (CV_r_Quality_BumpMapping < 2)
  {
    var = iConsole->GetCVar("r_Vegetation_PerpixelLight");
    if (var)
      var->Set(0);
  }

  // Allow offset bump-mapping and parametric shaders system for very high spec settings only
  if (CV_r_Quality_BumpMapping < 3)
  {
    var = iConsole->GetCVar("r_UseHWShaders");
    if (var && var->GetIVal() == 2)
      var->Set(1);
    if (CV_r_offsetbump)
    {
      var = iConsole->GetCVar("r_OffsetBump");
      if (var)
        var->Set(0);
    }
  }

  iLog->Log(" ****** OGL CryRenderer Stats ******\n");
  iLog->Log(" Mode: %d x %d (%s)\n", m_width, m_height, fullscreen ? "FullScreen" : "Windowed");
  iLog->Log(" Gamma: %s\n", (m_Features & RFT_HWGAMMA) ? "Hardware" : "Software");
  if (CV_r_fsaa && SUPPORTS_GL_ARB_multisample && (m_Features & RFT_SUPPORTFSAA))
    iLog->Log(" Full scene AA: Enabled (%d samples)\n", CV_r_fsaa_samples);
  else
    iLog->Log(" Full scene AA: Disabled\n");
  iLog->Log(" Stencil type: %s\n", (SUPPORTS_GL_ATI_separate_stencil || SUPPORTS_GL_EXT_stencil_two_side) ? "Two sided" : "Single sided");
  iLog->Log(" Max Texture size: %d\n", m_MaxTextureSize);
  iLog->Log(" Max Active Textures ARB: %d\n", m_MaxActiveTexturesARBFixed);
  iLog->Log(" Max Active Textures ARB (For ARB_fragment_program): %d\n", m_MaxActiveTexturesARB_VP);
  iLog->Log(" Projective EMBM: %s\n", (m_Features & RFT_HW_ENVBUMPPROJECTED) ? "enabled" : "disabled");
  iLog->Log(" Multitexture: %s\n", (m_Features & RFT_MULTITEXTURE) ? "enabled" : "disabled");
  iLog->Log(" Compressed textures: %s\n", (m_Features & RFT_COMPRESSTEXTURE) ? "enabled" : "disabled");
  iLog->Log(" Vertex buffers usage: %s\n", SUPPORTS_GL_ARB_vertex_buffer_object ? "VBO" : SUPPORTS_GL_NV_vertex_array_range ? "VAR" : "System memory");
  iLog->Log(" Anisotropic texture filtering: %s (Maximum level: %d)\n", (m_Features & RFT_ALLOWANISOTROPIC) ? "enabled" : "disabled", m_MaxAnisotropicLevel);
  if (m_Features & RFT_BUMP)
    iLog->Log(" Bump mapping: enabled (%s)\n", (m_Features & RFT_BUMP) ? "DOT3" : "EMBM");
  else
    iLog->Log(" Bump mapping: disabled\n");
  iLog->Log(" Detail microtextures: %s\n", (m_Features & RFT_DETAILTEXTURE) ? "enabled" : "disabled");
  iLog->Log(" Paletted textures: %s\n", (m_Features & RFT_PALTEXTURE) ? "enabled" : "disabled");

  if (nGPU == RFT_HW_NV4X)
    iLog->Log(" Use Hardware Shaders for NV4x GPUs\n");
  else
  if (nGPU == RFT_HW_GFFX)
    iLog->Log(" Use Hardware Shaders for NV3x GPUs\n");
  else
  if (nGPU == RFT_HW_GF2)
    iLog->Log(" Use Hardware Shaders for NV1x GPU\n");
  else
  if (nGPU == RFT_HW_GF3)
    iLog->Log(" Use Hardware Shaders for NV2x GPU\n");
  else
  if (nGPU == RFT_HW_RADEON)
    iLog->Log(" Use Hardware Shaders for ATI R300 GPU\n");
  else
    iLog->Log(" Hardware Shaders are not supported\n");

  char *str;
  if (nGPU == RFT_HW_GF2)
    str = "Not using pixel shaders";
  else
  if (CV_r_nops20)
    str = "Replace PS.2.0 to PS.1.1";
  else
  if (CV_r_Quality_BumpMapping == 3)
  {
    if (m_Features & RFT_HW_PS30)
      str = "PS.3.0, PS.2.0 and PS.1.1";
    else
      str = "PS.2.0 and PS.1.1";
  }
  else
    str ="PS1.1 only";
  iLog->Log(" Pixel shaders usage: %s\n", str);

  if (nGPU == RFT_HW_GF2)
    str = "Not using vertex shaders";
  else
  if (CV_r_nops20)
    str = "Replace VS.2.0 to VS.1.1";
  else
  if (CV_r_Quality_BumpMapping == 3)
  {
    if (m_Features & RFT_HW_PS30)
      str = "VS.3.0, VS.2.0 and VS.1.1";
    else
      str = "VS.2.0 and VS.1.1";
  }
  else
    str ="VS1.1 only";
  iLog->Log(" Vertex shaders usage: %s\n", str);

  iLog->Log(" Shadow maps type: %s\n", (m_Features & RFT_DEPTHMAPS) ? "Depth maps" : (m_Features & RFT_SHADOWMAP_SELFSHADOW) ? "Mixed Depth/2D maps" : "2D shadow maps");

  m_numtmus = m_MaxActiveTexturesARB_VP;

  glLoadIdentity();
  glRotatef(60, 0, 1, 0);
  Matrix44 m;
  glGetFloatv(GL_MODELVIEW_MATRIX, m.GetData());

  int parms[4];

  glGetIntegerv(GL_MAX_CLIP_PLANES, &m_MaxClipPlanes);
  iLog->Log(" OGL Max Clip Planes=%d", m_MaxClipPlanes);
  glGetIntegerv(GL_MAX_LIGHTS, &m_MaxLightSources);
  iLog->Log(" OGL Max Lights=%d", m_MaxLightSources);
  glGetIntegerv(GL_MAX_TEXTURE_SIZE,parms);
  iLog->Log(" OGL Max Texture size=%dx%d",parms[0],parms[0]);
  glGetIntegerv(GL_MAX_VIEWPORT_DIMS,parms);
  iLog->Log(" OGL Max Viewport dims=%dx%d",parms[0],parms[1]);
  int nDepth;
  glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &nDepth);
  iLog->Log(" OGL Max ModelView Matrix stack depth=%d", nDepth);
  glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &nDepth);
  iLog->Log(" OGL Max Projection Matrix stack depth=%d", nDepth);
  if (nGPU == RFT_HW_GFFX || nGPU == RFT_HW_GF3)
    m_MaxClipPlanes = 0;

  CheckError("CGLRenderer::Init");

  {
    static bool bSet = false;
    if (!bSet)
    {
      bSet = true;
      nnglTexImage2D = glTexImage2D;
      glTexImage2D = nglTexImage2D;

      nnglBindTexture = glBindTexture;
      glBindTexture = nglBindTexture;

      nnglCompressedTexImage2DARB = glCompressedTexImage2DARB;
      glCompressedTexImage2DARB = nglCompressedTexImage2DARB;

      nnglCopyTexImage2D = glCopyTexImage2D;
      glCopyTexImage2D = nglCopyTexImage2D;

      nnglDeleteTextures = glDeleteTextures;
      glDeleteTextures = nglDeleteTextures;

      nnglTexParameteri = glTexParameteri;
      glTexParameteri = nglTexParameteri;

      nnglBindBufferARB = glBindBufferARB;
      glBindBufferARB = nglBindBufferARB;

      nnglGenBuffersARB = glGenBuffersARB;
      glGenBuffersARB = nglGenBuffersARB;

      nnglDeleteBuffersARB = glDeleteBuffersARB;
      glDeleteBuffersARB = nglDeleteBuffersARB;
    }
  }

  GLSetDefaultState();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  SetPolygonMode(R_SOLID_MODE);

  ::ShowWindow(m_CurrContext->m_Glhwnd,1);
  ::UpdateWindow(m_CurrContext->m_Glhwnd);
  ::SetForegroundWindow(m_CurrContext->m_Glhwnd);
  ::SetFocus(m_CurrContext->m_Glhwnd);

  SetGamma(CV_r_gamma+m_fDeltaGamma, CV_r_brightness, CV_r_contrast);

  m_width = width;
  m_height = height;

  InitVAR();

  if (glVertexArrayRangeNV && IsVarPresent())
    m_RP.m_NumFences = MAX_DYNVBS;

  if (bReInit)
  {
    iLog->Log("Reload textures\n");
    RefreshResources(0);
  }

  iLog->Log("Init Shaders\n");

  gRenDev->m_cEF.mfInit();
  EF_PipelineInit();

  return (m_CurrContext->m_Glhwnd);
}

bool CGLRenderer::CreateRContext(SRendContext *rc, WIN_HDC Glhdc, WIN_HGLRC hGLrc, int cbpp, int zbpp, int sbits, bool bAllowFSAA)
{
  BOOL statusFSAA = false;
  int pixelFormat;

  m_CurrContext = rc;

  int nFSAA = bAllowFSAA ? CV_r_fsaa : 0;
  m_Features &= ~RFT_SUPPORTFSAA;

  if (bAllowFSAA && m_RContexts.Num() && rc == m_RContexts[0])
  {
    WNDCLASS wc;
    wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = ::DefWindowProc;
    wc.cbClsExtra    = 4;
    wc.cbWndExtra    = 4;
    wc.hInstance     = m_hInst;
    wc.hIcon         = 0;
    wc.hCursor       = NULL;
    wc.hbrBackground = (HBRUSH)(BLACK_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "FAKE_WINDOW";

    RegisterClass(&wc);

    HWND hWnd = ::CreateWindow( "FAKE_WINDOW",		// window class name
      "Fake",		            // window caption
      WS_OVERLAPPEDWINDOW,	// window style
      CW_USEDEFAULT,		// initial x pos
      CW_USEDEFAULT,		// initial y pos
      CW_USEDEFAULT,		// initial x size
      CW_USEDEFAULT,		// initial x size
      NULL,			// parent window handle
      NULL,			// window menu handle
      m_hInst,			// program instance handle
      NULL );			// creation params

    if (hWnd)
    {
      PIXELFORMATDESCRIPTOR pfd;
      memset(&pfd, 0, sizeof(pfd));

      pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
      pfd.nVersion = 1;
      pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
      pfd.dwLayerMask = PFD_MAIN_PLANE;
      pfd.iPixelType = PFD_TYPE_RGBA;
      pfd.cColorBits = cbpp;
      pfd.cAlphaBits = CV_gl_alpha_bits;
      pfd.cDepthBits = zbpp;
      pfd.cAccumBits = 0;
      pfd.cStencilBits = sbits;

      BOOL status = FALSE;

      HDC hDC = GetDC(hWnd);

      pixelFormat = ChoosePixelFormat(m_CurrContext->m_hDC, &pfd);

      if (SetPixelFormat(hDC, pixelFormat, &pfd) != FALSE)
      {
        // Create a rendering context
        HGLRC hGLRC = pwglCreateContext(hDC);

        // Make the rendering context current
        pwglMakeCurrent(hDC, hGLRC);

        // Search for best matching pixel format
        pwglGetProcAddress = (PFNWGLGETPROCADDRESSPROC)GetProcAddress( (HINSTANCE)m_hLibHandle, "wglGetProcAddress" );
        if (_wglExtensionSupported("WGL_ARB_multisample"))
        {
          wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)pwglGetProcAddress("wglGetPixelFormatAttribivARB");
          wglGetPixelFormatAttribfvARB = (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)pwglGetProcAddress("wglGetPixelFormatAttribfvARB");
          wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)pwglGetProcAddress("wglChoosePixelFormatARB");

          if (wglChoosePixelFormatARB)
            m_Features |= RFT_SUPPORTFSAA;
        }
        // make no rendering context current
        wglMakeCurrent(NULL, NULL);

        // Destroy the rendering context...
        wglDeleteContext(hGLRC);
      }

      ReleaseDC(hWnd, hDC);
      DestroyWindow(hWnd);
      UnregisterClass("FAKE_WINDOW", m_hInst);
    }
  }

  if (nFSAA && (m_Features & RFT_SUPPORTFSAA))
  {
    // Search for best matching pixel format
    iLog->Log("Tryng to set FSAA pixel format...");
    int   iAttributes[30];
    float fAttributes[] = {0, 0};
    uint  numFormats;

    // Choose a Pixel Format Descriptor (PFD) with multisampling support.
    iAttributes[0] = WGL_DOUBLE_BUFFER_ARB;
    iAttributes[1] = TRUE;
    iAttributes[2] = WGL_COLOR_BITS_ARB;
    iAttributes[3] = cbpp;
    iAttributes[4] = WGL_DEPTH_BITS_ARB;
    iAttributes[5] = zbpp;
    iAttributes[6] = WGL_ALPHA_BITS_ARB;
    iAttributes[7] = CV_gl_alpha_bits;
    iAttributes[8] = WGL_STENCIL_BITS_ARB;
    iAttributes[9] = sbits;
    iAttributes[10] = WGL_SAMPLE_BUFFERS_ARB;
    iAttributes[11] = TRUE;
    iAttributes[12] = WGL_SAMPLES_ARB;
    iAttributes[13] = CV_r_fsaa_samples;
    iAttributes[14] = 0;
    iAttributes[15] = 0;

    // First attempt...
    statusFSAA = wglChoosePixelFormatARB(m_CurrContext->m_hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
    // Failure happens not only when the function fails, but also when no matching pixel format has been found
    if (statusFSAA == FALSE || numFormats == 0)
    {
      iAttributes[6] = 0;
      iAttributes[7] = 0;
      statusFSAA = wglChoosePixelFormatARB(m_CurrContext->m_hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
    }
    if (statusFSAA)
    {
      rc->m_bFSAAWasSet = true;
      iLog->Log("Ok");
    }
    else
      iLog->Log("False");
  }
  if (m_RContexts.Num() && rc != m_RContexts[0])
    statusFSAA = m_RContexts[0]->m_bFSAAWasSet;

  if (!statusFSAA)
    CV_r_fsaa = 0;

  if (statusFSAA)
  {
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));
    DescribePixelFormat(m_CurrContext->m_hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    SetPixelFormat(m_CurrContext->m_hDC, pixelFormat, &pfd);
    rc->m_PixFormat = pixelFormat;

    m_CurrContext->m_hRC = pwglCreateContext(m_CurrContext->m_hDC);
    m_CurrContext->m_bFSAAWasSet = true;

    pwglMakeCurrent(m_CurrContext->m_hDC, m_CurrContext->m_hRC);

    glEnable(GL_MULTISAMPLE_ARB);
  }
  else
  {
    m_CurrContext->m_bFSAAWasSet = false;
    if (!Glhdc && !SetupPixelFormat(cbpp,zbpp,sbits,rc))
    {
      iLog->Log("Warning: SetupPixelFormat failed\n");
		  //try to change alpha bits
		  if (CV_gl_alpha_bits>0)
		  {
			  iLog->Log("Trying to change alpha bits");
			  CV_gl_alpha_bits = 0;
			  //and call pixel format again
			  if (!SetupPixelFormat(cbpp,zbpp,sbits,rc))
				  return (false);
		  }
    }

    if (!hGLrc)
      rc->m_hRC = pwglCreateContext(rc->m_hDC);
    else
    {
      rc->m_hRC = (HGLRC)hGLrc;
      m_bEditor = true;
    }
  }
  if (!rc->m_hRC)
  {
    iLog->Log("Warning: wglCreateContext failed\n");
    return (false);
  }

  pwglMakeCurrent(rc->m_hDC, rc->m_hRC);
  if (m_RContexts[0] != rc)
  {
    pwglShareLists(m_RContexts[0]->m_hRC, rc->m_hRC);
    CV_r_vsync = 1;
  }

  return true;
}

bool CGLRenderer::SetCurrentContext(WIN_HWND hWnd)
{
  int i;

  for (i=0; i<m_RContexts.Num(); i++)
  {
    if (m_RContexts[i]->m_Glhwnd == hWnd)
      break;
  }
  if (i == m_RContexts.Num())
    return false;

  if (m_CurrContext == m_RContexts[i])
    return true;

  m_CurrContext = m_RContexts[i];
  if (pwglGetCurrentContext() != m_RContexts[i]->m_hRC)
  {
    pwglMakeCurrent(m_RContexts[i]->m_hDC, m_RContexts[i]->m_hRC);

    for (i=0; i<96; i++)
    {
      CCGVProgram_GL::m_CurParams[i][0] = -999999;
      CCGVProgram_GL::m_CurParams[i][1] = -999999;
      CCGVProgram_GL::m_CurParams[i][2] = -999999;
      CCGVProgram_GL::m_CurParams[i][3] = -999999;
    }
    CVProgram::m_LastVP = NULL;
    CPShader::m_CurRC = NULL;
  }

  return true;
}

bool CGLRenderer::CreateContext(WIN_HWND hWnd, bool bAllowFSAA)
{
  int i;

  for (i=0; i<m_RContexts.Num(); i++)
  {
    if (m_RContexts[i]->m_Glhwnd == hWnd)
      break;
  }
  if (i != m_RContexts.Num())
    return false;
  SRendContext *rc = new SRendContext;
  m_RContexts.AddElem(rc);
  rc->m_Glhwnd = (HWND)hWnd;
  rc->m_hDC = GetDC((HWND)hWnd);
  rc->m_hRC = NULL;
  int cbpp = m_cbpp;
  int zbpp = m_zbpp;
  int sbpp = m_sbpp;
  bool bRes = CreateRContext(rc, NULL, NULL, cbpp, zbpp, sbpp, bAllowFSAA);
  if (bRes)
  {
    for (i=0; i<96; i++)
    {
      CCGVProgram_GL::m_CurParams[i][0] = -999999;
      CCGVProgram_GL::m_CurParams[i][1] = -999999;
      CCGVProgram_GL::m_CurParams[i][2] = -999999;
      CCGVProgram_GL::m_CurParams[i][3] = -999999;
    }
    CVProgram::m_LastVP = NULL;
    CPShader::m_CurRC = NULL;
  }
  return bRes;
}

bool CGLRenderer::DeleteContext(WIN_HWND hWnd)
{
  int i;

  for (i=0; i<m_RContexts.Num(); i++)
  {
    if (m_RContexts[i]->m_Glhwnd == hWnd)
      break;
  }
  if (i == m_RContexts.Num())
    return false;
  SRendContext *rc = m_RContexts[i];
  pwglMakeCurrent(NULL, NULL);

  if (rc->m_hRC)
  {
    pwglDeleteContext(rc->m_hRC);
    rc->m_hRC = NULL;
  }

  if (rc->m_hDC)
  {
    ReleaseDC(rc->m_Glhwnd, rc->m_hDC);
    rc->m_hDC = NULL;
  }
  delete rc;
  m_RContexts.Remove(i, 1);

  return true;
}


#else //PS2

bool CGLRenderer::Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen)
{
  return false;
}

#endif  //endif

void CGLRenderer::RefreshResources(int nFlags)
{
  if (nFlags & FRO_TEXTURES)
    m_TexMan->ReloadAll(nFlags);
  if (nFlags & (FRO_SHADERS | FRO_SHADERTEXTURES))
    gRenDev->m_cEF.mfReloadAllShaders(nFlags);
}


extern float gFOpenTime;
extern int nRejectFOpen;
extern int nAcceptFOpen;

void CGLRenderer::ShutDown(bool bReInit)
{
  int i;

  //  CRendElement::mfLogAllREs();

  if (CV_r_printmemoryleaks)
  {
    FILE *fp = fxopen("LeafBufferLeaks.txt", "w");
    if (fp)
    {
      CLeafBuffer *pLB = CLeafBuffer::m_RootGlobal.m_NextGlobal;
      while (pLB != &CLeafBuffer::m_RootGlobal)
      {
        CLeafBuffer *Next = pLB->m_NextGlobal;
        float fSize = pLB->Size(0)/1024.0f/1024.0f;
        fprintf(fp, "*** LB %s: %0.3fMb\n", pLB->m_sSource, fSize);
        iLog->Log("WARNING: LB Leak %s: %0.3fMb", pLB->m_sSource, fSize);
        DeleteLeafBuffer(pLB);
        pLB = Next;
      }

      CRendElement *pRE = CRendElement::m_RootGlobal.m_NextGlobal;
      while (pRE != &CRendElement::m_RootGlobal)
      {
        CRendElement *Next = pRE->m_NextGlobal;
        float fSize = pRE->Size()/1024.0f/1024.0f;
        fprintf(fp, "*** LB %s: %0.3fMb\n", pRE->mfTypeString(), fSize);
        iLog->Log("WARNING: RE Leak %s: %0.3fMb", pRE->mfTypeString(), fSize);
        SAFE_RELEASE(pRE);
        pRE = Next;
      }

      fclose(fp);
    }
  }

  FreeResources(FRR_ALL);
  EF_PipelineShutdown();
  GenerateVBLog("VBLeaks.txt");
  ShutDownVAR();
  CName::mfExitSubsystem();

  if (m_CGContext)
  {
#ifndef WIN64
		// TODO: AMD64 port: find 64-bit CG
    cgDestroyContext(m_CGContext);
#endif
    m_CGContext = NULL;
  }


  iLog->LogToFile("Tex. FOpen time: %f sec\n", gFOpenTime);
  iLog->LogToFile("Tex. Accept FOpen: %d\n", nAcceptFOpen);
  iLog->LogToFile("Tex. Reject FOpen: %d\n", nRejectFOpen);

  iLog->LogToFile("GlRenderer Shutdown\n");

  RestoreDeviceGamma();

  glFinish();

  HWND hWnd = m_RContexts[0]->m_Glhwnd;

  for (i=0; i<m_RContexts.Num(); i++)
  {
    DeleteContext(m_RContexts[i]->m_Glhwnd);
  }
  if (hWnd && !bReInit)
  {
    DestroyWindow(hWnd);
  }

  //ChangeDisplay(m_deskwidth,m_deskheight,m_deskbpp);
  ChangeDisplay(0,0,0);

  FreeLibrary();

  if (m_vidmodes)
  {
    delete [] m_vidmodes;
    m_vidmodes=NULL;
  }
}


//=======================================================================

ILog     *iLog;
IConsole *iConsole;
ITimer   *iTimer;
ISystem  *iSystem;
//CVars    *cVars;
int *pTest_int;
//CryCharManager *pCharMan;
IPhysicalWorld *pIPhysicalWorld;

ISystem *GetISystem()
{
	return iSystem;
}

extern "C" DLL_EXPORT IRenderer* PackageRenderConstructor(int argc, char* argv[], SCryRenderInterface *sp);
DLL_EXPORT IRenderer* PackageRenderConstructor(int argc, char* argv[], SCryRenderInterface *sp)
{
  gbRgb = false;

  iConsole  = sp->ipConsole;
  iLog      = sp->ipLog;
  iTimer    = sp->ipTimer;
  iSystem   = sp->ipSystem;
//  cVars     = sp->ipVars;
  pTest_int = sp->ipTest_int;
	pIPhysicalWorld = sp->pIPhysicalWorld;
//  pCharMan = sp->ipCharMan;

#ifdef DEBUGALLOC
#undef new
#endif
  IRenderer *rd = (IRenderer *) (new CGLRenderer());
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif

  g_SecondsPerCycle = iSystem->GetSecondsPerCycle();
  g_CpuFlags = iSystem->GetCPUFlags();

  srand( GetTickCount() );

  return rd;
}

void *gGet_D3DDevice()
{
  return NULL;
}
void *gGet_glReadPixels()
{
  return glReadPixels;
}

