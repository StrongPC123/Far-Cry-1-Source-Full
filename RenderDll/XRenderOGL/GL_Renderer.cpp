//////////////////////////////////////////////////////////////////////
//
//  Crytek CryENGINE Source code
//  
//  File:GlRenderer.cpp
//  Description: Implementation of the GL renderer API
//
//  History:
//  -Jan 31,2001:Originally created by Marco Corbetta
//	-: taken over by Andrey Khonich
//
//////////////////////////////////////////////////////////////////////

#include "RenderPCH.h"
#include "GL_Renderer.h"
#include "GLCGVProgram.h"
#include "GLCGPShader.h"

// GL functions implement.
#define GL_EXT(name) byte SUPPORTS##name;
#define GL_PROC(ext,ret,func,parms) ret (__stdcall *func)parms;
#include "GLFuncs.h"
#undef GL_EXT
#undef GL_PROC

#include "../common/shadow_renderer.h"
#include "limits.h"

int CGLRenderer::CV_gl_useextensions;
int CGLRenderer::CV_gl_3dfx_gamma_control;
int CGLRenderer::CV_gl_arb_texture_compression;
int CGLRenderer::CV_gl_arb_multitexture;
int CGLRenderer::CV_gl_arb_pbuffer;
int CGLRenderer::CV_gl_arb_pixel_format;
int CGLRenderer::CV_gl_arb_buffer_region;
int CGLRenderer::CV_gl_arb_render_texture;
int CGLRenderer::CV_gl_arb_multisample;
int CGLRenderer::CV_gl_arb_vertex_program;
int CGLRenderer::CV_gl_arb_vertex_buffer_object;
int CGLRenderer::CV_gl_arb_fragment_program;
int CGLRenderer::CV_gl_arb_texture_env_combine;
int CGLRenderer::CV_gl_ext_swapcontrol;
int CGLRenderer::CV_gl_ext_bgra;
int CGLRenderer::CV_gl_ext_depth_bounds_test;
int CGLRenderer::CV_gl_ext_multi_draw_arrays;
int CGLRenderer::CV_gl_ext_compiled_vertex_array;
int CGLRenderer::CV_gl_ext_texture_cube_map;
int CGLRenderer::CV_gl_ext_separate_specular_color;
int CGLRenderer::CV_gl_ext_secondary_color;
int CGLRenderer::CV_gl_ext_texture_env_combine;
int CGLRenderer::CV_gl_ext_paletted_texture;
int CGLRenderer::CV_gl_ext_stencil_two_side;
int CGLRenderer::CV_gl_ext_stencil_wrap;
int CGLRenderer::CV_gl_ext_texture_filter_anisotropic;
int CGLRenderer::CV_gl_ext_texture_env_add;
int CGLRenderer::CV_gl_ext_texture_rectangle;
int CGLRenderer::CV_gl_hp_occlusion_test;
int CGLRenderer::CV_gl_nv_fog_distance;
int CGLRenderer::CV_gl_nv_texture_env_combine4;
int CGLRenderer::CV_gl_nv_point_sprite;
int CGLRenderer::CV_gl_nv_vertex_array_range;
int CGLRenderer::CV_gl_nv_fence;
int CGLRenderer::CV_gl_nv_register_combiners;
int CGLRenderer::CV_gl_nv_register_combiners2;
int CGLRenderer::CV_gl_nv_texgen_reflection;
int CGLRenderer::CV_gl_nv_texgen_emboss;
int CGLRenderer::CV_gl_nv_vertex_program;
int CGLRenderer::CV_gl_nv_vertex_program3;
int CGLRenderer::CV_gl_nv_texture_rectangle;
int CGLRenderer::CV_gl_nv_texture_shader;
int CGLRenderer::CV_gl_nv_texture_shader2;
int CGLRenderer::CV_gl_nv_texture_shader3;
int CGLRenderer::CV_gl_nv_fragment_program;
int CGLRenderer::CV_gl_nv_multisample_filter_hint;
int CGLRenderer::CV_gl_sgix_depth_texture;
int CGLRenderer::CV_gl_sgix_shadow;
int CGLRenderer::CV_gl_sgis_generate_mipmap;
int CGLRenderer::CV_gl_sgis_texture_lod;
int CGLRenderer::CV_gl_ati_separate_stencil;
int CGLRenderer::CV_gl_ati_fragment_shader;

int CGLRenderer::CV_gl_psforce11;
int CGLRenderer::CV_gl_vsforce11;
int CGLRenderer::CV_gl_nv30_ps20;

int CGLRenderer::CV_gl_alpha_bits;

int CGLRenderer::CV_gl_swapOnStart;
int CGLRenderer::CV_gl_clipplanes;

float CGLRenderer::CV_gl_normalmapscale;

float CGLRenderer::CV_gl_offsetfactor;
float CGLRenderer::CV_gl_offsetunits;

float CGLRenderer::CV_gl_pip_allow_var;
float CGLRenderer::CV_gl_pip_buff_size;

int CGLRenderer::CV_gl_rb_verts;
int CGLRenderer::CV_gl_rb_tris;

int CGLRenderer::CV_gl_maxtexsize;
int CGLRenderer::CV_gl_squaretextures;
int CGLRenderer::CV_gl_mipprocedures;
ICVar *CGLRenderer::CV_gl_texturefilter;

//////////////////////////////////////////////////////////////////////
CGLRenderer::CGLRenderer()
{
	if (!gcpOGL)
    gcpOGL = this;

#ifdef DEBUGALLOC
#undef new
#endif
  m_TexMan = new CGLTexMan;
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
  
  m_LogFile = NULL;
  m_Features = RFT_DIRECTACCESSTOVIDEOMEMORY;
  m_numvidmodes=0;
  m_vidmodes=NULL;

  mMinDepth = 0;
  mMaxDepth = 1.0f;
  m_lod_biasSupported=false;
  m_IsDedicated = false;

  m_sbpp=0;

  m_polygon_mode = R_SOLID_MODE;

  m_pip_buffer_size = 0;
  m_AGPbuf=0; m_nPolygons = 0; m_nFrameID = 0;

  iConsole->Register("gl_UseExtensions", &CV_gl_useextensions, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_3DFX_gamma_control", &CV_gl_3dfx_gamma_control, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_ARB_texture_compression", &CV_gl_arb_texture_compression, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_ARB_multitexture", &CV_gl_arb_multitexture, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_ARB_pbuffer", &CV_gl_arb_pbuffer, 0, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_ARB_pixel_format", &CV_gl_arb_pixel_format, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_ARB_buffer_region", &CV_gl_arb_buffer_region, 0, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_ARB_render_texture", &CV_gl_arb_render_texture, 0, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_ARB_multisample", &CV_gl_arb_multisample, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_ARB_vertex_program", &CV_gl_arb_vertex_program, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_ARB_vertex_buffer_object", &CV_gl_arb_vertex_buffer_object, 0, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_ARB_fragment_program", &CV_gl_arb_fragment_program, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_ARB_texture_env_combine", &CV_gl_arb_texture_env_combine, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_EXT_swapcontrol", &CV_gl_ext_swapcontrol, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_EXT_bgra", &CV_gl_ext_bgra, 0, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_EXT_depth_bounds_test", &CV_gl_ext_depth_bounds_test, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_EXT_multi_draw_arrays", &CV_gl_ext_multi_draw_arrays, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_EXT_Compiled_Vertex_Array", &CV_gl_ext_compiled_vertex_array, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_EXT_texture_cube_map", &CV_gl_ext_texture_cube_map, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_EXT_separate_specular_color", &CV_gl_ext_separate_specular_color, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_EXT_secondary_color", &CV_gl_ext_secondary_color, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_EXT_paletted_texture", &CV_gl_ext_paletted_texture, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_EXT_stencil_two_side", &CV_gl_ext_stencil_two_side, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_EXT_stencil_wrap", &CV_gl_ext_stencil_wrap, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_EXT_texture_filter_anisotropic", &CV_gl_ext_texture_filter_anisotropic, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_EXT_texture_env_add", &CV_gl_ext_texture_env_add, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_EXT_texture_env_combine", &CV_gl_ext_texture_env_combine, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_EXT_texture_rectangle", &CV_gl_ext_texture_rectangle, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_HP_occlusion_test", &CV_gl_hp_occlusion_test, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_fog_distance", &CV_gl_nv_fog_distance, 0, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_texture_env_combine4", &CV_gl_nv_texture_env_combine4, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_point_sprite", &CV_gl_nv_point_sprite, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_vertex_array_range", &CV_gl_nv_vertex_array_range, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_fence", &CV_gl_nv_fence, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_register_combiners", &CV_gl_nv_register_combiners, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_register_combiners2", &CV_gl_nv_register_combiners2, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_texgen_reflection", &CV_gl_nv_texgen_reflection, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_texgen_emboss", &CV_gl_nv_texgen_emboss, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_vertex_program", &CV_gl_nv_vertex_program, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_vertex_program3", &CV_gl_nv_vertex_program3, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_texture_rectangle", &CV_gl_nv_texture_rectangle, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_texture_shader", &CV_gl_nv_texture_shader, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_texture_shader2", &CV_gl_nv_texture_shader2, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_texture_shader3", &CV_gl_nv_texture_shader3, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_fragment_program", &CV_gl_nv_fragment_program, 0, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV_multisample_filter_hint", &CV_gl_nv_multisample_filter_hint, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_SGIX_depth_texture", &CV_gl_sgix_depth_texture, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_SGIX_shadow", &CV_gl_sgix_shadow, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_SGIS_generate_mipmap", &CV_gl_sgis_generate_mipmap, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_SGIS_texture_lod", &CV_gl_sgis_texture_lod, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_ATI_separate_stencil", &CV_gl_ati_separate_stencil, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_ATI_fragment_shader", &CV_gl_ati_fragment_shader, 1, VF_REQUIRE_APP_RESTART);

  iConsole->Register("GL_PSforce11", &CV_gl_psforce11, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_VSforce11", &CV_gl_vsforce11, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_NV30_PS20", &CV_gl_nv30_ps20, 1, VF_REQUIRE_APP_RESTART);

  iConsole->Register("GL_alpha_bits", &CV_gl_alpha_bits, 8, VF_REQUIRE_APP_RESTART);

  iConsole->Register("GL_SwapOnStart", &CV_gl_swapOnStart, 0);
  iConsole->Register("GL_ClipPlanes", &CV_gl_clipplanes, 1);

  iConsole->Register("GL_OffsetFactor", &CV_gl_offsetfactor, -1.0f);
  iConsole->Register("GL_OffsetUnits", &CV_gl_offsetunits, -4.0f);

  iConsole->Register("GL_pip_allow_var", &CV_gl_pip_allow_var, 1.0f, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_pip_buff_size", &CV_gl_pip_buff_size, 10.0f, VF_REQUIRE_APP_RESTART);

  iConsole->Register("GL_RB_verts", &CV_gl_rb_verts, 2048, VF_REQUIRE_APP_RESTART);
  iConsole->Register("GL_RB_tris", &CV_gl_rb_tris, 3084, VF_REQUIRE_APP_RESTART);

  iConsole->Register("GL_MaxTexSize", &CV_gl_maxtexsize, 0);
  iConsole->Register("GL_SquareTextures", &CV_gl_squaretextures, 0);
  iConsole->Register("GL_MipProcedures", &CV_gl_mipprocedures, 0);

  CV_gl_texturefilter = iConsole->CreateVariable("GL_TextureFilter", "GL_LINEAR_MIPMAP_LINEAR", VF_DUMPTODISK );

  iConsole->Register("GL_NormalMapScale", &CV_gl_normalmapscale, 0.15f, VF_REQUIRE_LEVEL_RELOAD);

  memset(m_ShadowTexIDBuffer,0,sizeof(m_ShadowTexIDBuffer));
  m_Features |= RFT_RGBA;


  CV_r_envlighting = 0;

  //if (!CV_gl_arb_vertex_buffer_object)
  // CV_gl_psforce11 = 0;

}

CGLRenderer *gcpOGL = NULL;

#include <stdio.h>
//////////////////////////////////////////////////////////////////////
CGLRenderer::~CGLRenderer()
{ 
  ShutDown(); 
  gcpOGL = NULL;
}

void CGLRenderer::GLSetDefaultState()
{
  // Set permanent state.
  CheckError("Set state1");
  glEnable      (GL_DEPTH_TEST);    
  glEnable      (GL_TEXTURE_2D);
  glShadeModel  (GL_SMOOTH);    
  
  glDisable     (GL_ALPHA_TEST);
  glDepthMask   (GL_TRUE);  
  glDisable     (GL_BLEND);
  
//  glEnable    (GL_SCISSOR_TEST);
  //glScissor   (0,0,m_width,m_height);

  glMatrixMode  (GL_PROJECTION);
  glLoadIdentity  (); 

  glClearDepth  (1.0f);
  glDepthFunc   (GL_LEQUAL);  
  glCullFace    (GL_BACK);    
  glDisable     (GL_CULL_FACE);
  
  glBlendFunc   (GL_ZERO,GL_SRC_COLOR); 

  glHint      (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    

  if (CV_r_fsaa && m_CurrContext->m_bFSAAWasSet && SUPPORTS_GL_NV_multisample_filter_hint)
  {
    if (CV_r_fsaa==2)
      glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
    else
      glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_FASTEST);
  }
 
  glPixelStorei (GL_PACK_ALIGNMENT,1);
  glPixelStorei (GL_UNPACK_ALIGNMENT,1);  

  glMatrixMode  (GL_TEXTURE);
  glLoadIdentity  ();   
  
  glDisable   (GL_LIGHTING);
  glDisable   (GL_BLEND);
  glDisable   (GL_LINE_SMOOTH);   
  glDrawBuffer  (GL_BACK);
	glStencilMask(0xff);

  glHint      (GL_FOG_HINT,GL_NICEST);
  
  float globalAmbient[] = {1,1,1,1};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, false);
  float emission[] = {0,0,0,0};
  glMaterialfv(GL_FRONT, GL_EMISSION, emission);
  
  CheckError("Set state2");
  // Set permanent state.

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();

  glEnableClientState(GL_VERTEX_ARRAY);

  m_TexMan->SetFilter(CV_gl_texturefilter->GetString());
}

//////////////////////////////////////////////////////////////////////
void CGLRenderer::EnableTMU(bool enable)
{ 
  if (CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_Target && CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_Target != GL_TEXTURE_2D)
  {
    glDisable(CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_Target);   
    CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_Target = 0;
  }
  if (enable)
  {     
    CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_Target = GL_TEXTURE_2D;
    glEnable(GL_TEXTURE_2D);      
  }
  else
  {     
    glDisable(GL_TEXTURE_2D);   
    CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_Bind = 0;
    CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_Target = 0;
  }   
 
}

extern float glLodCorrection;

void sLogTexture (char *name, int Size);


extern int nFrameTexSize;

void CGLRenderer::ChangeLog()
{
  if (CV_r_log && !m_LogFile)
  {
    if (CV_r_log == 3)
      SetLogFuncs(true);
    m_LogFile = fxopen ("OpenGLLog.txt", "w");
    if (m_LogFile)
    {      
      iLog->Log("OpenGL log file '%s' opened\n", "OpenGLLog.txt");
      char time[128];
      char date[128];

      _strtime( time );
      _strdate( date );

      fprintf(m_LogFile, "\n==========================================\n");
      fprintf(m_LogFile, "OpenGL Log file opened: %s (%s)\n", date, time);
      fprintf(m_LogFile, "==========================================\n");
    }
  }
  else
  if (!CV_r_log && m_LogFile)
  {
    SetLogFuncs(false);

    char time[128];
    char date[128];
    _strtime( time );
    _strdate( date );
    
    fprintf(m_LogFile, "\n==========================================\n");
    fprintf(m_LogFile, "OpenGL Log file closed: %s (%s)\n", date, time);
    fprintf(m_LogFile, "==========================================\n");
    
    fclose(m_LogFile);
    m_LogFile = NULL;
    iLog->Log("OpenGL log file '%s' closed\n", "OpenGLLog.txt");
  }
}

//////////////////////////////////////////////////////////////////////
void CGLRenderer::BeginFrame()
{
  g_bProfilerEnabled = iSystem->GetIProfileSystem()->IsProfiling();

  PROFILE_FRAME(Screen_Begin);

  CheckError("BeginFrame");

  if (CV_r_vsync != m_VSync)
  {
    m_VSync = CV_r_vsync;
		EnableVSync(m_VSync?true:false);
  }

  if (CV_r_reloadshaders)
  {
    gRenDev->m_cEF.mfReloadAllShaders(CV_r_reloadshaders);
    CV_r_reloadshaders = 0;
  }

  if (CV_r_fsaa != m_FSAA && m_CurrContext->m_bFSAAWasSet)
  {
    m_FSAA = CV_r_fsaa;
    if (!m_FSAA)
    {
      glDisable(GL_MULTISAMPLE_ARB);
    }
    else
    {
      glEnable(GL_MULTISAMPLE_ARB);
      if (SUPPORTS_GL_NV_multisample_filter_hint)
      {
        if (m_FSAA == 2)
          glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
        else
          glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_FASTEST);
      }
    }
  }
  ChangeLog ();
    
  if (m_CurrContext->m_hRC != pwglGetCurrentContext())
    pwglMakeCurrent(m_CurrContext->m_hDC, m_CurrContext->m_hRC);

  if (CV_gl_swapOnStart && m_bSwapBuffers)
    SwapBuffers(m_CurrContext->m_hDC);

  if (CV_r_gamma+m_fDeltaGamma != m_fLastGamma || CV_r_brightness != m_fLastBrightness || CV_r_contrast != m_fLastContrast)
    SetGamma(CV_r_gamma+m_fDeltaGamma, CV_r_brightness, CV_r_contrast);

  gRenDev->m_cEF.mfBeginFrame();
    
	if (CV_r_PolygonMode != m_polygon_mode)
		SetPolygonMode(CV_r_PolygonMode);	

  glMatrixMode(GL_MODELVIEW); 

  if (strcmp(CV_gl_texturefilter->GetString(), m_TexMan->m_CurTexFilter) || CV_r_texture_anisotropic_level != m_TexMan->m_CurAnisotropic)
    m_TexMan->SetFilter(CV_gl_texturefilter->GetString());
  
  m_nPolygons = 0;
	m_nShadowVolumePolys=0;
  m_nFrameID++;
  m_nFrameUpdateID++;
  m_bWasCleared = false;
  m_RP.m_RealTime = iTimer->GetCurrTime();
  TArray<CRETempMesh *> *tm = &m_RP.m_TempMeshes[m_nFrameID & 1];
  for (int i=0; i<tm->Num(); i++)
  {
    CRETempMesh *re = tm->Get(i);
    if (!re)
      continue;
    if (re->m_VBuffer)
    {
      ReleaseBuffer(re->m_VBuffer);
      re->m_VBuffer = NULL;
    }
    ReleaseIndexBuffer(&re->m_Inds);
  }
  tm->SetUse(0);
  m_RP.m_CurTempMeshes = tm;

  if (CRenderer::CV_r_log)
    Logv(0, "******************************* BeginFrame ********************************\n");

  glColor4f(1,1,1,1);
  nFrameTexSize = 0;
  glDrawBuffer  (GL_BACK);
  ResetToDefault();

  //  EF_ClearBuffers(true);
}

bool CGLRenderer::ChangeResolution(int nNewWidth, int nNewHeight, int nNewColDepth, int nNewRefreshHZ, bool bFullScreen)
{
  return false;
}

//////////////////////////////////////////////////////////////////////
bool CGLRenderer::ChangeDisplay(unsigned int width,unsigned int height,unsigned int bpp)
{

#ifdef WIN32  
  DWORD Mode = 0;
  DEVMODE dm;

  ZeroMemory(&dm,sizeof(dm));

  EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&dm);
  
  dm.dmFields=0;

  long lRes = 0;

  if (width || height || bpp )
  {
    if (width && height)
    if (width!=dm.dmPelsWidth || height != dm.dmPelsHeight )
    {
      dm.dmPelsWidth = width;
      dm.dmPelsHeight= height;    
      dm.dmFields = dm.dmFields | DM_PELSWIDTH | DM_PELSHEIGHT;
    } 

    if ((bpp) && (bpp != dm.dmBitsPerPel ))
    {
      dm.dmBitsPerPel = bpp;
      dm.dmFields = dm.dmFields | DM_BITSPERPEL;
    }

    if (!dm.dmFields) return (true); //already in this mode

    lRes = ChangeDisplaySettings(&dm, 0);
  } 
  else
  {
    lRes = ChangeDisplaySettings(NULL, 0);
  }

  switch(lRes)
  {
    case DISP_CHANGE_SUCCESSFUL:
      SetViewport(0, 0, width, height);
      m_width = width;
      m_height = height;
      m_cbpp = bpp;
      return (true);
      break;

    case DISP_CHANGE_RESTART:
    case DISP_CHANGE_BADFLAGS:
    case DISP_CHANGE_FAILED:
    case DISP_CHANGE_BADMODE:
    case DISP_CHANGE_NOTUPDATED:
    //ChangeDisplaySettings(NULL, 0);
    return (false);
    iLog->Log("Cannot change display settings\n");    
    break;
  } 

  return (false);
#else  
  return (false);
#endif  
}

//////////////////////////////////////////////////////////////////////
void CGLRenderer::ChangeViewport(unsigned int x,unsigned int y,unsigned int width,unsigned int height)
{
  SetViewport(x, y, width, height);
  m_width = width;
  m_height = height;
}

//////////////////////////////////////////////////////////////////////
extern int gEnvFrame;

//extern double timeFtoI, timeFtoL, timeQRound;
extern int nTexSize;

void CGLRenderer::DrawPoints(Vec3d v[], int nump, CFColor& col, int flags)
{
  int i;
  int st;

  EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  st = GS_NODEPTHTEST;
  if (flags & 1)
    st |= GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA;
  else
  if (flags & 2)
    st = GS_DEPTHWRITE;
  else
    st = flags | GS_NODEPTHTEST;
  EF_SetState(st);

  glColor4fv(&col[0]);

  glBegin(GL_POINTS);

  for (i=0; i<nump; i++)
  {
    glVertex3fv(v[i]);
  }
  glEnd();
}

void CGLRenderer::DrawLines(Vec3d v[], int nump, CFColor& col, int flags, float fGround)
{
  int i;
  int st;

  EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  st = GS_NODEPTHTEST;
  if (flags & 1)
    st |= GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA;
  else
    st = flags | GS_NODEPTHTEST;
  EF_SetState(st);

  glDisableClientState(GL_COLOR_ARRAY);
  glColor4fv(&col[0]);

  if (fGround >= 0)
  {
    glBegin(GL_LINES);

    for (i=0; i<nump; i++)
    {
      glVertex3f(v[i][0], fGround, 0);
      glVertex3fv(v[i]);
    }
    glEnd();
  }
  else
  {
    glBegin(GL_LINE_STRIP);

    for (i=0; i<nump; i++)
    {
      glVertex3fv(v[i]);
    }
    glEnd();
  }
}

static void sDrawImage(float xpos,float ypos,float w,float h,int texture_id,float s0,float t0,float s1,float t1,float r,float g,float b,float a)
{ 
  PROFILE_FRAME(Draw_2DImage);

  gcpOGL->SetCullMode(R_CULL_DISABLE);

  gcpOGL->SelectTMU(0);
  
  if(texture_id>=0)
  {
    gcpOGL->EnableTMU(true);
    gcpOGL->SetTexture(texture_id);
  }
	else
		gcpOGL->EnableTMU(false);

  glBegin(GL_QUADS);
  
  glColor4f(r,g,b,a);
  glTexCoord2f(s0, t1);  
  glVertex2f(xpos, ypos);

  glColor4f(r,g,b,a);
  glTexCoord2f(s1, t1);  
  glVertex2f(xpos+w, ypos);

  glColor4f(r,g,b,a);
  glTexCoord2f(s1, t0);  
  glVertex2f(xpos+w,ypos+h);

  glColor4f(r,g,b,a);
  glTexCoord2f(s0, t0);  
  glVertex2f(xpos, ypos+h);
  glEnd();  
}

//////////////////////////////////////////////////////////////////////
void CGLRenderer::Update()
{
  PROFILE_FRAME(Screen_Update);

  CheckError("Renderer::Update begin");
  m_prevCamera = m_cam;

  if (CV_r_showtimegraph)
  {
    static byte *fg;
    static float fPrevTime = iTimer->GetCurrTime();
    static int sPrevWidth = 0;
    static int sPrevHeight = 0;
    static int nC;

    float fCurTime = iTimer->GetCurrTime();
    float frametime = fCurTime - fPrevTime;
    fPrevTime = fCurTime;
    int wdt = m_width;
    int hgt = m_height;

    if (sPrevHeight != hgt || sPrevWidth != wdt)
    {
      if (fg)
      {
        delete [] fg;
        fg = NULL;
      }
      sPrevWidth = wdt;
      sPrevHeight = hgt;
    }

    if (!fg)
    {
      fg = new byte[wdt];
      memset(fg, -1, wdt);
    }

    int type = CV_r_showtimegraph;
    float f;
    float fScale;
    if (type > 2)
    {
      type = 2;
      fScale = (float)CV_r_showtimegraph / 1000.0f;
    }
    else
      fScale = 0.1f;
    f = frametime / fScale;
    f = 255.0f - CLAMP(f*255.0f, 0, 255.0f);
    if (fg)
    {
      fg[nC] = (byte)f;
      Graph(fg, 0, hgt-280, wdt, 256, nC, type, "Frame Time", Col_Green, fScale);
    }
    nC++;
    if (nC == wdt)
      nC = 0;
  }

  if (CV_r_logVBuffers)
  {
    GenerateVBLog("LogVBuffers.txt");
    CV_r_logVBuffers = 0;
  }

  if (CV_r_measureoverdraw)
  {
    byte *buf = new byte [m_width*m_height];
    glReadPixels(0, 0, m_width, m_height, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, buf);
    
    int size = m_width * m_height;
    int acc = 0;
    for (int i=0; i<size; i++)
    {
      acc += buf[i];
    }
    m_RP.m_PS.m_fOverdraw += (float)acc; 
    if (CV_r_measureoverdraw > 1)
    {
      static uint tBind = 0;
      if (!tBind)
      {
        glGenTextures(1, &tBind);
        if (SUPPORTS_GL_NV_texture_rectangle || SUPPORTS_GL_EXT_texture_rectangle)
        {
          glBindTexture(GL_TEXTURE_RECTANGLE_NV, tBind);
          glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
          glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
      }
      byte *bBuf = new byte[m_width*m_height*3];
      for (int i=0; i<size; i++)
      {
        int b = buf[i]*CV_r_measureoverdraw;
        if (b > 255)
          b = 255;
        bBuf[i*3+0] = b;
        bBuf[i*3+1] = b;
        bBuf[i*3+2] = b;
      }
      EF_SetState(GS_NODEPTHTEST);
      SetCullMode(R_CULL_NONE);
      if (SUPPORTS_GL_NV_texture_rectangle || SUPPORTS_GL_EXT_texture_rectangle)
      {
        glBindTexture(GL_TEXTURE_RECTANGLE_NV, tBind);
        glEnable(GL_TEXTURE_RECTANGLE_NV);
        glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, bBuf);
      }
      Set2DMode(true, m_width, m_height);
      glBegin (GL_QUADS);
      glTexCoord2f(0, 0);
      glVertex3f (0, (float)m_height, 1);
      glTexCoord2f(0, (float)m_height);
      glVertex3f (0, 0, 1);
      glTexCoord2f((float)m_width, (float)m_height);
      glVertex3f ((float)m_width, 0, 1);
      glTexCoord2f((float)m_width, 0);
      glVertex3f ((float)m_width, (float)m_height, 1);
      glEnd ();
      if (SUPPORTS_GL_NV_texture_rectangle || SUPPORTS_GL_EXT_texture_rectangle)
      {
        glDisable(GL_TEXTURE_RECTANGLE_NV);
        glBindTexture(GL_TEXTURE_RECTANGLE_NV, 0);
      }
      Set2DMode(false, m_width, m_height);

      delete [] bBuf;
    }
    delete [] buf;
  }

  if (iConsole && iConsole->GetFont())
  { 
    CRenderer *crend=gRenDev;
    CXFont    *cf=iConsole->GetFont();
    if (CV_r_measureoverdraw)
      crend->WriteXY(cf,550,75, 0.5f,1,1,1,1,1,"Overdraw=%0.3f",m_RP.m_PS.m_fOverdraw / (float)(m_width * m_height));
    switch (CV_r_stats)
    {
      case 1:
        crend->WriteXY(cf,10,270, 0.5f,1,1,1,1,1, "Unique Render Items=%d (%d)",m_RP.m_PS.m_NumRendItems, m_RP.m_PS.m_NumRendBatches);
        crend->WriteXY(cf,10,290, 0.5f,1,1,1,1,1, "Unique VPrograms=%d",m_RP.m_PS.m_NumVShaders);
        crend->WriteXY(cf,10,320, 0.5f,1,1,1,1,1, "Unique PShaders=%d",m_RP.m_PS.m_NumPShaders);
        crend->WriteXY(cf,10,365, 0.5f,1,1,1,1,1,"Unique Textures=%d",m_RP.m_PS.m_NumTextures);
        crend->WriteXY(cf,10,385, 0.5f,1,1,1,1,1,"TexturesSize=%.03f Kb",m_RP.m_PS.m_TexturesSize/1024.0f);
        crend->WriteXY(cf,10,400, 0.5f,1,1,1,1,1,"Mesh update=%.03f Kb",m_RP.m_PS.m_MeshUpdateBytes/1024.0f);
        break;

      case 2:
        {
          crend->WriteXY(cf,550,140, 0.5f,1,1,1,1,1,"FFTTables=%0.3f",CREOcean::m_RS.m_StatsTimeFFTTable);
          crend->WriteXY(cf,550,155, 0.5f,1,1,1,1,1,"FFT=%0.3f",CREOcean::m_RS.m_StatsTimeFFT);
          crend->WriteXY(cf,550,170, 0.5f,1,1,1,1,1,"UpdateVerts=%0.3f",CREOcean::m_RS.m_StatsTimeUpdateVerts);
          crend->WriteXY(cf,550,185, 0.5f,1,1,1,1,1,"BumpTexUpdate=%0.3f",CREOcean::m_RS.m_StatsTimeTexUpdate);
          crend->WriteXY(cf,550,200, 0.5f,1,1,1,1,1,"RendOcean=%0.3f",CREOcean::m_RS.m_StatsTimeRendOcean);
          crend->WriteXY(cf,550,215, 0.5f,1,1,1,1,1,"NumRendSectors=%d",CREOcean::m_RS.m_StatsNumRendOceanSectors);
          crend->WriteXY(cf,550,230, 0.5f,1,1,1,1,1,"NumAllocatedSectors=%d",CREOcean::m_RS.m_StatsAllocatedSectors);
        }
        break;

      case 3:
        {
          CLeafBuffer *pLB = CLeafBuffer::m_Root.m_Prev;
          int nMem = 0;
          while (pLB != &CLeafBuffer::m_Root)
          {
            nMem += pLB->GetAllocatedBytes(true);
            pLB = pLB->m_Prev;
          }
          crend->WriteXY(cf,550,140, 0.5f,1,1,1,1,1,"LeafBuffer Vert. size=%0.3fMb",(float)nMem/1024.0f/1024.0f);
          int num_used = 0, size_used = 0, num_all = 0, size_all = 0;
          for(int i=0; i<m_alloc_info.Count(); i++)
          {
            if(m_alloc_info[i].busy)
            {
              num_used++;
              size_used+=m_alloc_info[i].bytes_num;
            }
            num_all++;
            size_all+=m_alloc_info[i].bytes_num;
          }
          crend->WriteXY(cf,550,160, 0.5f,1,1,1,1,1,"Total Vert. size=%0.3fMb",(float)size_all/1024.0f/1024.0f);
        }
        break;

      case 4:
        {
          int i;

          int nSize = 0;
          int n = 0;
          for (i=0; i<SShader::m_Shaders_known.GetSize(); i++)
          {
            SShader *pSH = SShader::m_Shaders_known[i];
            if (!pSH)
              continue;
            nSize += pSH->Size(0);
            n++;
          }
          crend->WriteXY(cf,550,160, 0.5f,1,1,1,1,1,"Shaders: %d, size=%0.3fMb",n , (float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          for (i=0; i<SShader::m_ShaderResources_known.Num(); i++)
          {
            SRenderShaderResources *pSR = SShader::m_ShaderResources_known[i];
            if (!pSR)
              continue;
            nSize += pSR->Size();
            n++;
          }
          crend->WriteXY(cf,550,175, 0.5f,1,1,1,1,1,"Shader Res: %d, size=%0.3fMb",n , (float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          for (i=0; i<SParamComp::m_ParamComps.Num(); i++)
          {
            nSize += SParamComp::m_ParamComps[i]->Size();
            n++;
          }
          crend->WriteXY(cf,550,190, 0.5f,1,1,1,1,1,"Shader Comps: %d,  size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          for (i=0; i<CVProgram::m_VPrograms.Num(); i++)
          {
            nSize += CVProgram::m_VPrograms[i]->Size();
            n++;
          }
          crend->WriteXY(cf,550,205, 0.5f,1,1,1,1,1,"VProgramms: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          for (i=0; i<CPShader::m_PShaders.Num(); i++)
          {
            nSize += CPShader::m_PShaders[i]->Size();
            n++;
          }
          crend->WriteXY(cf,550,220, 0.5f,1,1,1,1,1,"PShaders: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          for (i=0; i<CLightStyle::m_LStyles.Num(); i++)
          {
            nSize += CLightStyle::m_LStyles[i]->Size();
            n++;
          }
          crend->WriteXY(cf,550,235, 0.5f,1,1,1,1,1,"LStyles: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          for (i=0; i<SLightMaterial::known_materials.Num(); i++)
          {
            nSize += SLightMaterial::known_materials[i]->Size();
            n++;
          }
          crend->WriteXY(cf,550,250, 0.5f,1,1,1,1,1,"LMaterials: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          CLeafBuffer *pLB = CLeafBuffer::m_RootGlobal.m_NextGlobal;
          while (pLB != &CLeafBuffer::m_RootGlobal)
          {
            n++;
            nSize += pLB->Size(0);
            pLB = pLB->m_NextGlobal;
          }
          crend->WriteXY(cf,550,280, 0.5f,1,1,1,1,1,"Mesh: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          CRendElement *pRE = CRendElement::m_RootGlobal.m_NextGlobal;
          while (pRE != &CRendElement::m_RootGlobal)
          {
            n++;
            nSize += pRE->Size();
            pRE = pRE->m_NextGlobal;
          }
          crend->WriteXY(cf,550,295, 0.5f,1,1,1,1,1,"Rend. Elements: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);
        
          nSize = 0;
          n = 0;
          for (i=0; i<gRenDev->m_TexMan->m_Textures.Num(); i++)
          {
            STexPic *tp = gRenDev->m_TexMan->m_Textures[i];
            if (!tp)
              continue;
            n++;
            nSize += tp->Size(0);
          }
          crend->WriteXY(cf,550,310, 0.5f,1,1,1,1,1,"Textures: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);
        }
        break;
    }
  }

  m_TexMan->Update();    

  if (CV_r_profileshaders)
    EF_PrintProfileInfo();

  // draw debug bboxes and lines
	std :: vector<BBoxInfo>::iterator itBBox = m_arrBoxesToDraw.begin(), itBBoxEnd = m_arrBoxesToDraw.end();
	for( ; itBBox != itBBoxEnd; ++itBBox)
  {
		BBoxInfo& rBBox = *itBBox;
    SetMaterialColor( rBBox.fColor[0], rBBox.fColor[1],
                      rBBox.fColor[2], rBBox.fColor[3] );

		// set blend for transparent objects
		if(rBBox.fColor[3]!=1.f)
      EF_SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);

		switch(rBBox.nPrimType)
		{
		case DPRIM_LINE:
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_LINE_LOOP);
			glVertex3f(rBBox.vMins.x,rBBox.vMins.y,rBBox.vMins.z);
			glVertex3f(rBBox.vMaxs.x,rBBox.vMaxs.y,rBBox.vMaxs.z);
			glEnd();
			glEnable(GL_TEXTURE_2D);
			break;

		case DPRIM_WHIRE_BOX:
			Flush3dBBox(rBBox.vMins, rBBox.vMaxs, false);
			break;

		case DPRIM_SOLID_BOX:
			Flush3dBBox(rBBox.vMins, rBBox.vMaxs, true);
			break;

		case DPRIM_SOLID_SPHERE:
			{
				Vec3d vPos = (rBBox.vMins+rBBox.vMaxs)*0.5f;
				float fRadius = (rBBox.vMins-rBBox.vMaxs).Length();
				DrawBall(vPos, fRadius);
			}
			break;
		}
  
		if(rBBox.fColor[3]!=1.f)
		{
      EF_SetState(GS_DEPTHWRITE);
		}
	}

  m_arrBoxesToDraw.clear();

  
  /*if (m_TexMan->m_Text_ScreenLuminosityMap->m_Flags & FT_ALLOCATED)
  {
    EF_SetState(GS_NODEPTHTEST);
    int iTmpX, iTmpY, iTempWidth, iTempHeight;
    GetViewport(&iTmpX, &iTmpY, &iTempWidth, &iTempHeight);   
    Set2DMode(true, 1, 1);

    SetViewport(10, 400, 100, 100);   
    sDrawImage(0, 0, 1, 1, m_TexMan->m_Text_ScreenMap->m_Bind, 0, 0, m_TexMan->m_Text_ScreenMap->m_Width, m_TexMan->m_Text_ScreenMap->m_Height, 1,1,1,1);

    SetViewport(120, 400, 100, 100);   
    sDrawImage(0, 0, 1, 1, m_TexMan->m_Text_ScreenLowMap->m_Bind, 0, 0, m_TexMan->m_Text_ScreenLowMap->m_Width, m_TexMan->m_Text_ScreenLowMap->m_Height, 1,1,1,1);

    SetViewport(230, 400, 100, 100);   
    sDrawImage(0, 0, 1, 1, m_TexMan->m_Text_ScreenAvg1x1->m_Bind, 0, 0, m_TexMan->m_Text_ScreenAvg1x1->m_Width, m_TexMan->m_Text_ScreenAvg1x1->m_Height, 1,1,1,1);

    SetViewport(340, 400, 100, 100);   
    sDrawImage(0, 0, 1, 1, m_TexMan->m_Text_ScreenLuminosityMap->m_Bind, 0, 0, m_TexMan->m_Text_ScreenLuminosityMap->m_Width, m_TexMan->m_Text_ScreenLuminosityMap->m_Height, 1,1,1,1);

    SetViewport(450, 400, 100, 100);   
    sDrawImage(0, 0, 1, 1, m_TexMan->m_Text_Glare->m_Bind, 0, 0, m_TexMan->m_Text_Glare->m_Width, m_TexMan->m_Text_Glare->m_Height, 1,1,1,1);

    SetViewport(560, 400, 100, 100);   
    sDrawImage(0, 0, 1, 1, m_TexMan->m_Text_PrevScreenMap->m_Bind, 0, 0, m_TexMan->m_Text_PrevScreenMap->m_Width, m_TexMan->m_Text_PrevScreenMap->m_Height, 1,1,1,1);

    Set2DMode(false, 1, 1);
    SetViewport(iTmpX, iTmpY, iTempWidth, iTempHeight);   
  }*/

  {
    // print shadow volume stats
    static ICVar *pVar = iConsole->GetCVar("e_stencil_shadows");
    if(pVar && pVar->GetIVal()==3)
      CRETriMeshShadow::PrintStats();
  }

  { // print shadow maps on the screen
    static ICVar *pVar = iConsole->GetCVar("e_shadow_maps_debug");
    if(pVar && pVar->GetIVal()==2)
      DrawAllShadowsOnTheScreen();
  }

  if (!CV_gl_swapOnStart && m_bSwapBuffers)
    SwapBuffers(m_CurrContext->m_hDC);

  if (CRenderer::CV_r_log)
    Logv(0, "******************************* EndFrame ********************************\n");

	if (CV_r_GetScreenShot)
	{
		ScreenShot();
		CV_r_GetScreenShot = 0;
	}
}

extern int BindSizes[];
extern int BindFrame[];

void CGLRenderer::GetMemoryUsage(ICrySizer* Sizer)
{
  int i, nSize;

  assert (Sizer);

  //SIZER_COMPONENT_NAME(Sizer, "GLRenderer");
  {
    SIZER_COMPONENT_NAME(Sizer, "Renderer static");
    Sizer->AddObject(&BindSizes[0], TX_LASTBIND * 4);
    for (i=0; i<NUMRI_LISTS; i++)
    {
      Sizer->AddObject(&SRendItem::m_RendItems[i], SRendItem::m_RendItems[i].GetMemoryUsage());
    }
  }
  {
    SIZER_COMPONENT_NAME(Sizer, "Renderer dynamic");

    Sizer->Add(*this);
    Sizer->AddObject(&CCObject::m_Waves, CCObject::m_Waves.GetMemoryUsage());
    Sizer->AddObject(m_RP.m_VisObjects, MAX_REND_OBJECTS*sizeof(CCObject *));
    Sizer->AddObject(&m_RP.m_TempObjects, m_RP.m_TempObjects.GetMemoryUsage());
    Sizer->AddObject(&m_RP.m_Objects, m_RP.m_Objects.GetMemoryUsage());
    Sizer->AddObject(&m_RP.m_ObjectsPool, m_RP.m_nNumObjectsInPool * sizeof(CCObject));
    Sizer->AddObject(&m_RP.m_DLights, m_RP.m_DLights[0].GetMemoryUsage());
    Sizer->AddObject(m_SysArray, m_SizeSysArray);
    Sizer->AddObject(&m_RP.m_Splashes, m_RP.m_Splashes.GetMemoryUsage());
    nSize = 0;
    for (i=0; i<4; i++)
    {
      nSize += CREClientPoly::mPolysStorage[i].GetMemoryUsage();
    }
    nSize += CREClientPoly2D::mPolysStorage.GetMemoryUsage();
    Sizer->AddObject(&CREClientPoly2D::mPolysStorage, nSize);
    Sizer->AddObject(&m_RP.m_Sprites, m_RP.m_Sprites.GetMemoryUsage());
    nSize = 0;
    for (i=0; i<2; i++)
    {
      nSize += m_RP.m_TempMeshes[i].GetMemoryUsage();
    }
    Sizer->AddObject(&m_RP.m_TempMeshes, nSize);
    Sizer->AddObject(&gCurLightStyles, gCurLightStyles.GetMemoryUsage());
    Sizer->AddObject(&CName::mDuplicate, CName::Size());
  }
  

  {
    SIZER_COMPONENT_NAME(Sizer, "Shaders");
    for (i=0; i<SShader::m_Shaders_known.GetSize(); i++)
    {
      SShader *pSH = SShader::m_Shaders_known[i];
      if (!pSH)
        continue;
      nSize = pSH->Size(0);
      Sizer->AddObject(pSH, nSize);
    }
    {
      SIZER_COMPONENT_NAME(Sizer, "Shader shared components");

      for (i=0; i<SParamComp::m_ParamComps.Num(); i++)
      {
        nSize = SParamComp::m_ParamComps[i]->Size();
        Sizer->AddObject(SParamComp::m_ParamComps[i], nSize);
      }

      for (i=0; i<CVProgram::m_VPrograms.Num(); i++)
      {
        nSize = CVProgram::m_VPrograms[i]->Size();
        Sizer->AddObject(CVProgram::m_VPrograms[i], nSize);
      }

      for (i=0; i<CPShader::m_PShaders.Num(); i++)
      {
        nSize = CPShader::m_PShaders[i]->Size();
        Sizer->AddObject(CPShader::m_PShaders[i], nSize);
      }

      for (i=0; i<CLightStyle::m_LStyles.Num(); i++)
      {
        nSize = CLightStyle::m_LStyles[i]->Size();
        Sizer->AddObject(CLightStyle::m_LStyles[i], nSize);
      }

      for (i=0; i<SLightMaterial::known_materials.Num(); i++)
      {
        nSize = SLightMaterial::known_materials[i]->Size();
        Sizer->AddObject(SLightMaterial::known_materials[i], nSize);
      }
    }
  }
  {
    SIZER_COMPONENT_NAME(Sizer, "Mesh");

    CLeafBuffer *pLB = CLeafBuffer::m_RootGlobal.m_NextGlobal;
    while (pLB != &CLeafBuffer::m_RootGlobal)
    {
      nSize = pLB->Size(0);
      Sizer->AddObject(pLB, nSize);
      pLB = pLB->m_NextGlobal;
    }
  }
  {
    SIZER_COMPONENT_NAME(Sizer, "Render elements");

    CRendElement *pRE = CRendElement::m_RootGlobal.m_NextGlobal;
    while (pRE != &CRendElement::m_RootGlobal)
    {
      nSize = pRE->Size();
      Sizer->AddObject(pRE, nSize);
      pRE = pRE->m_NextGlobal;
    }
  }
  {
    SIZER_COMPONENT_NAME(Sizer, "Texture Objects");

    for (i=0; i<gRenDev->m_TexMan->m_Textures.Num(); i++)
    {
      STexPic *tp = gRenDev->m_TexMan->m_Textures[i];
      if (!tp)
        continue;
      nSize = tp->Size(0);
      Sizer->AddObject(tp, nSize);
    }
    Sizer->AddObject(&gRenDev->m_TexMan->m_Textures[0], gRenDev->m_TexMan->m_Textures.GetMemoryUsage());
    Sizer->AddObject(&gRenDev->m_TexMan->m_FreeSlots[0], gRenDev->m_TexMan->m_FreeSlots.GetMemoryUsage());
    Sizer->AddObject(&gRenDev->m_TexMan->m_TexData[0], 256*256*4);
    Sizer->AddObject(&gRenDev->m_TexMan->m_EnvCMaps[0], sizeof(SEnvTexture)*MAX_ENVCUBEMAPS);
    Sizer->AddObject(&gRenDev->m_TexMan->m_EnvTexts[0], sizeof(SEnvTexture)*MAX_ENVTEXTURES);
    Sizer->AddObject(&gRenDev->m_TexMan->m_CustomCMaps[0], sizeof(SEnvTexture)*16);
    Sizer->AddObject(&gRenDev->m_TexMan->m_CustomTextures[0], sizeof(SEnvTexture)*16);
    Sizer->AddObject(&gRenDev->m_TexMan->m_Templates[0], sizeof(STexPic)*EFTT_MAX);
  }
  {
    SIZER_COMPONENT_NAME(Sizer, "API Texture memory");
    Sizer->AddObject(&nTexSize, nTexSize);
  }
}

WIN_HWND CGLRenderer::GetHWND()
{
  return m_CurrContext ? m_CurrContext->m_Glhwnd : 0;
}

//////////////////////////////////////////////////////////////////////

const char* CGLRenderer::ErrorString( GLenum errorCode )
{
   static char *tess_error[] = {
      "missing gluEndPolygon",
      "missing gluBeginPolygon",
      "misoriented contour",
      "vertex/edge intersection",
      "misoriented or self-intersecting loops",
      "coincident vertices",
      "colinear vertices",
      "intersecting edges",
      "not coplanar contours"
   };
   static char *nurbs_error[] = {
      "spline order un-supported",
      "too few knots",
      "valid knot range is empty",
      "decreasing knot sequence knot",
      "knot multiplicity greater than order of spline",
      "endcurve() must follow bgncurve()",
      "bgncurve() must precede endcurve()",
      "missing or extra geometric data",
      "can't draw pwlcurves",
      "missing bgncurve()",
      "missing bgnsurface()",
      "endtrim() must precede endsurface()",
      "bgnsurface() must precede endsurface()",
      "curve of improper type passed as trim curve",
      "bgnsurface() must precede bgntrim()",
      "endtrim() must follow bgntrim()",
      "bgntrim() must precede endtrim()",
      "invalid or missing trim curve",
      "bgntrim() must precede pwlcurve()",
      "pwlcurve referenced twice",
      "pwlcurve and nurbscurve mixed",
      "improper usage of trim data type",
      "nurbscurve referenced twice",
      "nurbscurve and pwlcurve mixed",
      "nurbssurface referenced twice",
      "invalid property",
      "endsurface() must follow bgnsurface()",
      "misoriented trim curves",
      "intersecting trim curves",
      "UNUSED",
      "unconnected trim curves",
      "unknown knot error",
      "negative vertex count encountered",
      "negative byte-stride encounteed",
      "unknown type descriptor",
      "null control array or knot vector",
      "duplicate point on pwlcurve"
   };

   /* GL Errors */
   if (errorCode==GL_NO_ERROR) {
      return "no error";
   }
   else if (errorCode==GL_INVALID_VALUE) {
      return "invalid value";
   }
   else if (errorCode==GL_INVALID_ENUM) {
      return "invalid enum";
   }
   else if (errorCode==GL_INVALID_OPERATION) {
      return "invalid operation";
   }
   else if (errorCode==GL_STACK_OVERFLOW) {
      return "stack overflow";
   }
   else if (errorCode==GL_STACK_UNDERFLOW) {
      return "stack underflow";
   }
   else if (errorCode==GL_OUT_OF_MEMORY) {
      return "out of memory";
   }
   return NULL;
}

void CGLRenderer::CheckError(const char *comment)
{
  GLenum err = glGetError();
  int errnum=0;
  while (err != GL_NO_ERROR)
  {
    char * pErrText = (char*)ErrorString(err);
    if (comment)
      _text_to_log( "glGetError: %s: %s",  pErrText ? pErrText : "-", comment);
    else
      _text_to_log( "glGetError: %s:",  pErrText ? pErrText : "-");
    
    err = glGetError();
    if (++errnum>10)
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//MISC TEXTURE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void CGLRenderer::SetTexture(int tnum)
{
  if(tnum > INT_MAX/2)
  { // tnum is a shifted pointer to CImage structure
    CImage * pImage = (CImage *)(tnum - INT_MAX/2);
    ASSERT(pImage && pImage->m_valid_test == 123);

    if(!pImage->GetTextureId())
    { // if not loaded
      pImage->SetTextureId(LoadTexture(pImage->GetName(),&pImage->m_type));
    }

    glBindTexture(GL_TEXTURE_2D,pImage->GetTextureId());
    pImage->m_last_time_used = iTimer->GetCurrTime();

    iConsole->Exit("tnum > INT_MAX/2");
  }
  else
  {
    if (tnum>=0 && tnum!=m_lastbind[m_currstage])
    {
      glBindTexture(GL_TEXTURE_2D,tnum);
      m_lastbind[m_currstage]=tnum;             
    }
  }
}*/

//====================================================================

//////////////////////////////////////////////////////////////////////
/*void CGLRenderer::SetTexture(CImage *image)
{
  SetTexture(image->GetTextureId());
} */

void CGLRenderer::SetTexClampMode(bool clamp)
{
  if (!clamp)
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
  else
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
}

//////////////////////////////////////////////////////////////////////
unsigned int CGLRenderer::DownLoadToVideoMemory(unsigned char *data,int w, int h, ETEX_Format eTFSrc, ETEX_Format eTFDst, int nummipmap, bool repeat, int filter, int Id, char *szCacheName, int flags)
{
  unsigned int tnum;

/*  char name[128];
  sprintf(name, "$Auto_%d", m_TexGenID++);
  int flags = nummipmap ? 0 : FT_NOMIPS | FT_NOWORLD;
  int DXTSize = 0;
  int blockSize = 0;
  if (eTFSrc == eTF_DXT1)
  {
    flags |= FT_DXT1;
    blockSize = 8;
  }
  else
  if (eTFSrc == eTF_DXT3)
  {
    flags |= FT_DXT3;
    blockSize = 16;
  }
  else
  if (eTFSrc == eTF_DXT5)
  {
    flags |= FT_DXT5;
    blockSize = 16;
  }
  if (blockSize)
  {
    if (!nummipmap)
      nummipmap = 1;
    int wdt = w;
    int hgt = h;
    for (int i=0; i<nummipmap; i++)
    {
      DXTSize += ((wdt+3)/4)*((hgt+3)/4)*blockSize;
      wdt >>= 1;
      hgt >>= 1;
      if (!wdt)
        wdt = 1;
      if (!hgt)
        hgt = 1;
    }
  }
  if (!repeat)
    flags |= FT_CLAMP;

  if (eTFDst==eTF_8888)
    flags |= FT_HASALPHA;

  STexPic *tp = m_TexMan->DownloadTexture(name, w, h, flags, FT2_NODXT, data, eTT_Base, DXTSize, NULL, 0, eTFSrc);

  return (tp->m_Bind);  */

  tnum = Id;
  ETexType eTT = eTT_Base;
  int nW = ilog2(w);
  int nH = ilog2(h);
  if (w-nW || h-nH)
  {
    if (SUPPORTS_GL_NV_texture_rectangle || SUPPORTS_GL_EXT_texture_rectangle)
      eTT = eTT_Rectangle;
  }
  if (tnum == 0)
    glGenTextures(1,&tnum);
  assert(tnum<14000);
  
  SetTexture(tnum, eTT); 
  int tgt = GL_TEXTURE_2D;
  if (eTT == eTT_Rectangle)
    tgt = GL_TEXTURE_RECTANGLE_NV;

  int srcformat, dstformat;

  if (eTFSrc==eTF_DXT1)
    srcformat=GL_COMPRESSED_RGB_S3TC_DXT1_EXT;

  if (eTFDst==eTF_DXT1)
    dstformat=GL_COMPRESSED_RGB_S3TC_DXT1_EXT;

  if (eTFSrc==eTF_0888)
    srcformat=GL_BGR_EXT;

  if (eTFDst==eTF_0888)
    dstformat=GL_RGB8;

  if (eTFSrc==eTF_8888)
    srcformat=GL_BGRA_EXT;
  
  if (eTFDst==eTF_8888)
    dstformat=GL_RGBA;

  if (eTFSrc==eTF_RGBA)
    srcformat=GL_RGBA;

  if (eTFDst==eTF_RGBA)
    dstformat=GL_RGBA;

  if (eTFSrc==eTF_8000)
    srcformat=GL_ALPHA;
  
  if (eTFDst==eTF_8000)
    dstformat=GL_ALPHA;

	if (eTFSrc==eTF_4444)
    srcformat=GL_RGBA4;
  
  if (eTFDst==eTF_4444)
    dstformat=GL_RGBA4;

  if (repeat)
  {
    glTexParameteri(tgt, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(tgt, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
  else
  {
    glTexParameteri(tgt, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(tgt, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  // filter mode
  if (filter==FILTER_BILINEAR)
  {
    glTexParameteri(tgt, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(tgt, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  else if (filter==FILTER_TRILINEAR)
  {
    glTexParameteri(tgt, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(tgt, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  else if(filter==FILTER_LINEAR)
  {
    glTexParameteri(tgt, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(tgt, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  else
  {
    glTexParameteri(tgt, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(tgt, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }

  if (nummipmap)
  {
    glTexImage2D( tgt, 
                  1-nummipmap,
                  dstformat, 
                  w, 
                  h, 
                  0,
                  srcformat,
                  GL_UNSIGNED_BYTE, data);  
  }
  else
  {
    if (filter == FILTER_TRILINEAR || filter == FILTER_BILINEAR)
    {
      if (SUPPORTS_GL_SGIS_generate_mipmap)
        glTexParameteri(tgt, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    }
    if (eTFDst==eTF_DXT1)
    {
      int blockSize = (eTFDst == eTF_DXT1) ? 8 : 16;
      int size = ((w+3)/4)*((h+3)/4)*blockSize;
      glCompressedTexImage2DARB(tgt,0,GL_COMPRESSED_RGB_S3TC_DXT1_EXT,w,h,0,size,data);
    }
    else
      glTexImage2D(tgt, 
                  0,//1-mlevels,
                  dstformat,
                  w, 
                  h, 
                  0,
                  srcformat,
                  GL_UNSIGNED_BYTE, data);  
  }

  return (tnum);  
}

extern int TargetTex[TX_LASTBIND];

//////////////////////////////////////////////////////////////////////
void CGLRenderer::UpdateTextureInVideoMemory(uint tnum, unsigned char *newdata,int posx,int posy,int w,int h,ETEX_Format eTF)
{
  ETexType eTT = eTT_Base;
  if (TargetTex[tnum] == GL_TEXTURE_RECTANGLE_NV)
    eTT = eTT_Rectangle;

  SetTexture(tnum, eTT); 

  int srcformat;
  if (eTF == eTF_DXT1)
    srcformat=GL_COMPRESSED_RGB_S3TC_DXT1_EXT;

  if (eTF == eTF_0888)
    srcformat=GL_BGR_EXT;// GL_RGB8

	if (eTF==eTF_8888)
    srcformat=GL_BGRA_EXT;// GL_RGBA;

	if (eTF==eTF_4444)
	{
		srcformat=GL_RGBA4;
		assert(0); // not supported in opengl as source format
	}

  if (eTF == eTF_DXT1)
  {
    int blockSize = (eTF == eTF_DXT1) ? 8 : 16;
    int size = ((w+3)/4)*((h+3)/4)*blockSize;
    //glCompressedTexSubImage2DARB(GL_TEXTURE_2D,0,posx,posy,w,h,GL_COMPRESSED_RGB_S3TC_DXT1_EXT,size,newdata);
    //glCompressedTexImage2DARB(GL_TEXTURE_2D,0,GL_COMPRESSED_RGB_S3TC_DXT1_EXT,w,h,0,size,newdata);
    glCompressedTexSubImage2DARB(TargetTex[tnum],0,posx,posy,w,h,GL_COMPRESSED_RGB_S3TC_DXT1_EXT,size,newdata);
  }
  else
  {
    if (TargetTex[tnum] == GL_TEXTURE_2D)
    {
      int nw = ilog2(w);
      if (w != nw)
        return;
      int nh = ilog2(h);
      if (h != nh)
        return;
    }
    glTexSubImage2D(TargetTex[tnum],0,posx,posy,w,h,srcformat,GL_UNSIGNED_BYTE,newdata);
  }
}

void CGLRenderer::RemoveTexture(ITexPic * pTexPic)
{
  if(!pTexPic)
    return;

  STexPic * pSTexPic = (STexPic *)pTexPic;
  pSTexPic->Release(false);
}

void CGLRenderer::RemoveTexture(unsigned int nTextureId)
{
  if(nTextureId)
  {
    STexPic *tp = m_TexMan->GetByID(nTextureId);
    if (tp)
      tp->Release(false);
    else
    {
      CGLTexMan::m_TUState[CGLTexMan::m_CurStage].m_Bind = 0;
      CGLTexMan::m_TUState[CGLTexMan::m_CurStage].m_Target = 0;
      glDeleteTextures(1, &nTextureId);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//IMAGES DRAWING
////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CGLRenderer::Draw2dImage(float xpos,float ypos,float w,float h,int texture_id,float s0,float t0,float s1,float t1,float angle,float r,float g,float b,float a, float z)
{ 
  PROFILE_FRAME(Draw_2DImage);

  xpos=(float)ScaleCoordX(xpos);
  ypos=(float)ScaleCoordY(ypos)-1.0f;
	w=(float)ScaleCoordX(w)+1.0f;
	h=((float)ScaleCoordY(h))+2.0f;

  bool bSaveZTest = ((m_CurState & GS_NODEPTHTEST) == 0);
  SetCullMode(R_CULL_DISABLE);
  
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, m_width, m_height, 0.0, -9999.0, 9999.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  SelectTMU(0);

  ETexType eTT = eTT_Base;
  if (texture_id >= 0)
  {
    if (TargetTex[texture_id] == GL_TEXTURE_RECTANGLE_NV)
      eTT = eTT_Rectangle;
  }

  float fXSc = 1.0f;
  float fYSc = 1.0f;
  if(texture_id>0)
  {
    SetTexture(texture_id, eTT);
    if (eTT == eTT_Rectangle)
    {
      int nW, nH;
      glGetTexLevelParameteriv(TargetTex[texture_id], 0, GL_TEXTURE_WIDTH,  &nW);
      glGetTexLevelParameteriv(TargetTex[texture_id], 0, GL_TEXTURE_HEIGHT, &nH);
      fXSc = (float)nW;
      fYSc = (float)nH;
    }
  }
	else
		EnableTMU(false);

  EF_SetColorOp(255, 255, DEF_TEXARG0, DEF_TEXARG0);

  glBegin(GL_QUADS);
  
  if (angle!=0)
  {
    float xsub=(float)(xpos+w/2);
    float ysub=(float)(ypos+h/2);

    float x,y,x1,y1;
    float mcos=cry_cosf(DEG2RAD(angle));
    float msin=cry_sinf(DEG2RAD(angle));

    x=xpos-xsub;
    y=ypos-ysub;
    x1=x*mcos-y*msin;
    y1=x*msin+y*mcos;
    x1+=xsub;y1+=ysub;
		glColor4f(r,g,b,a);
    glTexCoord2f(s0, 1.f-t0);  
    glVertex3f(x1, y1, z);

    x=xpos+w-xsub;
    y=ypos-ysub;
    x1=x*mcos-y*msin;
    y1=x*msin+y*mcos;
    x1+=xsub;y1+=ysub; 
		glColor4f(r,g,b,a);
    glTexCoord2f(s1, 1.f-t0);  
    glVertex3f(x1, y1, z);

    x=xpos+w-xsub;
    y=ypos+h-ysub;
    x1=x*mcos-y*msin;
    y1=x*msin+y*mcos;
    x1+=xsub;y1+=ysub; 
		glColor4f(r,g,b,a);
    glTexCoord2f(s1, 1.f-t1);  
    glVertex3f(x1,y1,z);

    x=xpos-xsub;
    y=ypos+h-ysub;
    x1=x*mcos-y*msin;
    y1=x*msin+y*mcos;
    x1+=xsub;y1+=ysub; 
		glColor4f(r,g,b,a);
    glTexCoord2f(s0, 1.f-t1);  
    glVertex3f(x1,y1,z);

  }
  else  
  {
    glColor4f(r,g,b,a);
    glTexCoord2f(s0*fXSc, (1.f-t0)*fYSc);  
    glVertex3f((xpos), (ypos), z);

    glColor4f(r,g,b,a);
    glTexCoord2f(s1*fXSc, (1.f-t0)*fYSc);  
    glVertex3f((xpos+w), (ypos), z);

    glColor4f(r,g,b,a);
    glTexCoord2f(s1*fXSc, (1.f-t1)*fYSc);  
    glVertex3f((xpos+w),(ypos+h),z);

    glColor4f(r,g,b,a);
    glTexCoord2f(s0*fXSc, (1.f-t1)*fYSc);  
    glVertex3f((xpos), (ypos+h),z);
  } 
  glEnd();  
  

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);  
}

//////////////////////////////////////////////////////////////////////
void CGLRenderer::DrawImage(float xpos,float ypos,float w,float h,int texture_id,float s0,float t0,float s1,float t1,float r,float g,float b,float a)
{ 
  PROFILE_FRAME(Draw_2DImage);

  SetCullMode(R_CULL_DISABLE);

  SelectTMU(0);
  
  if(texture_id>=0)
  {
    EnableTMU(true);
    SetTexture(texture_id);
  }
	else
		EnableTMU(false);

  glBegin(GL_QUADS);
  
  glColor4f(r,g,b,a);
  glTexCoord2f(s0, 1.f-t0);  
  glVertex2f((float)(xpos), (float)(ypos));

  glColor4f(r,g,b,a);
  glTexCoord2f(s1, 1.f-t0);  
  glVertex2f((float)(xpos+w), (float)(ypos));

  glColor4f(r,g,b,a);
  glTexCoord2f(s1, 1.f-t1);  
  glVertex2f((float)(xpos+w),(float)(ypos+h));

  glColor4f(r,g,b,a);
  glTexCoord2f(s0, 1.f-t1);  
  glVertex2f((float)(xpos), (float)(ypos+h));
  glEnd();  
}


///////////////////////////////////////////
void CGLRenderer::SetCullMode(int mode)
{
  switch (mode)
  {
    case R_CULL_DISABLE:
      GLSetCull(eCULL_None);
      break;
    case R_CULL_BACK:
      GLSetCull(eCULL_Back);
      break;
    case R_CULL_FRONT:
      GLSetCull(eCULL_Front);
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//FOG
////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////
void CGLRenderer::SetFog(float density,float fogstart,float fogend,const float *color,int fogmode)
{
  if (fogmode==R_FOGMODE_LINEAR)
    glFogi(GL_FOG_MODE,GL_LINEAR);
  else
  if (fogmode==R_FOGMODE_EXP2)
    glFogi(GL_FOG_MODE,GL_EXP2);

  //fogend = 200;
  m_FS.m_FogDensity = density;
  m_FS.m_FogStart = fogstart;
  m_FS.m_FogEnd = fogend;
  m_FS.m_nFogMode = fogmode;
  CFColor col = CFColor(color[0], color[1], color[2], 1.0f);
  if (m_bHeatVision)
    col = CFColor(0.0f, 0.0f, 0.0f, 1.0f);
  m_FS.m_FogColor = col;

  glFogf(GL_FOG_DENSITY,density);
  glFogf(GL_FOG_START, fogstart); 
  glFogf(GL_FOG_END, fogend); 

  glFogfv(GL_FOG_COLOR,&col[0]);    

  if (SUPPORTS_GL_NV_fog_distance)
  {
    glFogi(GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);
  }
}

///////////////////////////////////////////
bool CGLRenderer::EnableFog(bool enable)
{
  bool bPrevFog = m_FS.m_bEnable; // remember fog value

  if (enable)
    glEnable(GL_FOG);
  else
    glDisable(GL_FOG);

  m_FS.m_bEnable = enable;

  return bPrevFog;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//TEXGEN 
////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////
void CGLRenderer::EnableTexGen(bool enable)
{
  if(enable)
  {
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);  
//    glEnable(GL_TEXTURE_GEN_R);  
  //  glEnable(GL_TEXTURE_GEN_Q);  
  }
  else
  {
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);  
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_Q);  
  }
}

///////////////////////////////////////////
void CGLRenderer::SetTexgen(float scaleX,float scaleY,float translateX,float translateY)
{        
  GLfloat eyePlaneT[] = { scaleX, 0.0f, 0.0f, translateX };
  GLfloat eyePlaneS[] = { 0.0f, scaleY, 0.0f, translateY };

  glTexGenfv(GL_S, GL_OBJECT_PLANE, eyePlaneS);
  glTexGenfv(GL_T, GL_OBJECT_PLANE, eyePlaneT);

  glTexGenf(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
  glTexGenf(GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);  
}

void CGLRenderer::SetTexgen3D(float x1, float y1, float z1, float x2, float y2, float z2)
{        
  GLfloat eyePlaneS[] = { x1, y1, z1, 0 };
  GLfloat eyePlaneT[] = { x2, y2, z2, 0 };

  glTexGenfv(GL_S, GL_OBJECT_PLANE, eyePlaneS);
  glTexGenfv(GL_T, GL_OBJECT_PLANE, eyePlaneT);

  glTexGenf(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
  glTexGenf(GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//MISC EXTENSIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////
void CGLRenderer::SetLodBias(float value)
{  
  if (m_lod_biasSupported)
    glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, value);
}

///////////////////////////////////////////
void CGLRenderer::EnableVSync(bool enable)
{
  if (wglSwapIntervalEXT)
  {
    if (enable)
      wglSwapIntervalEXT(1);
    else
      wglSwapIntervalEXT(0);
  }
}

//////////////////////////////////////////////////////////////////////
void CGLRenderer::SelectTMU(int tnum)
{
  EF_SelectTMU(tnum);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//MATRIX FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////
void CGLRenderer::PushMatrix()
{
  glPushMatrix();
}

///////////////////////////////////////////
void CGLRenderer::RotateMatrix(float a,float x,float y,float z)
{
  glRotatef(a,x,y,z);
}

void CGLRenderer::RotateMatrix(const Vec3d & angles)
{
  glRotatef(angles.z,0,0,1);
  glRotatef(angles.y,0,1,0);
  glRotatef(angles.x,1,0,0);
}

///////////////////////////////////////////
void CGLRenderer::TranslateMatrix(float x,float y,float z)
{
  glTranslatef(x,y,z);
}

void CGLRenderer::MultMatrix(float * mat)
{
  glMultMatrixf(mat);
}

void CGLRenderer::TranslateMatrix(const Vec3d &pos)
{
  glTranslatef(pos.x,pos.y,pos.z);
}

///////////////////////////////////////////
void CGLRenderer::ScaleMatrix(float x,float y,float z)
{
  glScalef(x,y,z);
}

///////////////////////////////////////////
void CGLRenderer::PopMatrix()
{
  glPopMatrix();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void CGLRenderer::LoadMatrix(const Matrix44 *src)
{
  if(src)
    glLoadMatrixf((float *)src);
  else
    glLoadIdentity();
}

void CGLRenderer::Flush3dBBox(const Vec3d &mins,const Vec3d &maxs,const bool bSolid)
{
	if(bSolid)
	{
    EnableTMU(false);
    SetCullMode(R_CULL_NONE);

		glBegin(GL_QUADS);
		glVertex3f(maxs.x,mins.y,mins.z); //3
		glVertex3f(maxs.x,mins.y,maxs.z); //2
		glVertex3f(mins.x,mins.y,maxs.z); //1
		glVertex3f(mins.x,mins.y,mins.z); //0
		glEnd();

		glBegin(GL_QUADS);
		glVertex3f(mins.x,mins.y,mins.z); //0
		glVertex3f(mins.x,mins.y,maxs.z); //1
		glVertex3f(mins.x,maxs.y,maxs.z); //6
		glVertex3f(mins.x,maxs.y,mins.z); //4
		glEnd();

		glBegin(GL_QUADS);
		glVertex3f(mins.x,maxs.y,mins.z); //4
		glVertex3f(mins.x,maxs.y,maxs.z); //6
		glVertex3f(maxs.x,maxs.y,maxs.z); //7
		glVertex3f(maxs.x,maxs.y,mins.z); //5
		glEnd();

		glBegin(GL_QUADS);
		glVertex3f(maxs.x,maxs.y,mins.z); //54
		glVertex3f(maxs.x,maxs.y,maxs.z); //73
		glVertex3f(maxs.x,mins.y,maxs.z); //22
		glVertex3f(maxs.x,mins.y,mins.z); //31
		glEnd();

    EnableTMU(true);
	}
	else
	{
    EnableTMU(false);
    SetCullMode(R_CULL_NONE);

		glBegin(GL_LINE_LOOP);
		glVertex3f(mins.x,mins.y,mins.z); //0
		glVertex3f(mins.x,mins.y,maxs.z); //1
		glVertex3f(maxs.x,mins.y,maxs.z); //2
		glVertex3f(maxs.x,mins.y,mins.z); //3
		glEnd();

		glBegin(GL_LINE_LOOP);
		glVertex3f(mins.x,mins.y,mins.z); //0
		glVertex3f(mins.x,mins.y,maxs.z); //1
		glVertex3f(mins.x,maxs.y,maxs.z); //6
		glVertex3f(mins.x,maxs.y,mins.z); //4
		glEnd();

		glBegin(GL_LINE_LOOP);
		glVertex3f(mins.x,maxs.y,mins.z); //4
		glVertex3f(mins.x,maxs.y,maxs.z); //6
		glVertex3f(maxs.x,maxs.y,maxs.z); //7
		glVertex3f(maxs.x,maxs.y,mins.z); //5
		glEnd();

		glBegin(GL_LINE_LOOP);
		glVertex3f(maxs.x,mins.y,mins.z); //3
		glVertex3f(maxs.x,mins.y,maxs.z); //2
		glVertex3f(maxs.x,maxs.y,maxs.z); //7
		glVertex3f(maxs.x,maxs.y,mins.z); //5
		glEnd();

    EnableTMU(true);
	}
}

///////////////////////////////////////////
void CGLRenderer::Draw3dBBox(const Vec3d &mins,const Vec3d &maxs, int nPrimType)
{
	Draw3dPrim(mins, maxs, nPrimType);
}

void CGLRenderer::Draw3dPrim(const Vec3d &mins,const Vec3d &maxs, int nPrimType, const float* pRGBA)
{
	BBoxInfo info;
	info.vMins = mins;
	info.vMaxs = maxs;
	info.nPrimType = nPrimType;
	if (pRGBA)
	{
		for (int i = 0; i < 4; ++i)
			info.fColor[i] = pRGBA[i];
	}
	else
		glGetFloatv(GL_CURRENT_COLOR, (float*)info.fColor);
	m_arrBoxesToDraw.push_back(info);
}

///////////////////////////////////////////
int CGLRenderer::SetPolygonMode(int mode)
{
  int prev_mode = m_polygon_mode;
  m_polygon_mode = mode;

  if (mode==R_WIREFRAME_MODE)
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

  return prev_mode;
}


///////////////////////////////////////////
void CGLRenderer::SetPerspective(const CCamera &cam)
{
    gluPerspective(cam.GetFov()/(gf_PI/180.0f)*cam.GetProjRatio(), 1.0f/cam.GetProjRatio(), cam.GetZMin(), cam.GetZMax());    
}

///////////////////////////////////////////
void CGLRenderer::SetCamera(const CCamera &cam)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // camera.fov is for horizontal -> GL needs it vertical
  // projection.ratio is height/width -> GL needs width/height
  gluPerspective(cam.GetFov()/(gf_PI/180.0f)*cam.GetProjRatio(), 1.0f/cam.GetProjRatio(), cam.GetZMin(), cam.GetZMax());
  glMatrixMode(GL_MODELVIEW);

  Matrix44 mat = cam.GetVCMatrixD3D9();
  glLoadMatrixf(mat.GetData());

  EF_SetCameraInfo();
	
  m_cam=cam;
}

void CGLRenderer::SetViewport(int x, int y, int width, int height)
{
  if (!x && !y && !width && !height)
  {
    if(glViewport)
      glViewport(m_VX, m_VY, m_VWidth, m_VHeight);
    return;
  }

  if(glViewport)
    glViewport(x, y, width, height);

  m_VX = x;
  m_VY = y;
  m_VWidth = width;
  m_VHeight = height;
}

void CGLRenderer::SetScissor(int x, int y, int width, int height)
{
  if (!x && !y && !width && !height)
    EF_Scissor(false, x, y, width, height);
  else
    EF_Scissor(true, x, y, width, height);
}

//////////////////////////////////////////////////////////////////////
void CGLRenderer::GetModelViewMatrix(float * mat)
{
  glGetFloatv(GL_MODELVIEW_MATRIX, mat);
}

//////////////////////////////////////////////////////////////////////
void CGLRenderer::GetModelViewMatrix(double *mat)
{
  glGetDoublev(GL_MODELVIEW_MATRIX, mat);
}

//////////////////////////////////////////////////////////////////////
void CGLRenderer::GetProjectionMatrix(double *mat)
{
  glGetDoublev(GL_PROJECTION_MATRIX, mat);
}

//////////////////////////////////////////////////////////////////////
void CGLRenderer::GetProjectionMatrix(float *mat)
{
  glGetFloatv(GL_PROJECTION_MATRIX, mat);
}

//////////////////////////////////////////////////////////////////////
Vec3d CGLRenderer::GetUnProject(const Vec3d &WindowCoords,const CCamera &cam)
{
  /*double sx=WindowCoords.x;
  double sy=WindowCoords.y;
  double sz=WindowCoords.z;

  double px,py,pz;

  int nViewport[4]={0,0,m_width,m_height};

  int res=gluUnProject(sx,sy,sz,
                    cam.m_fViewMatrix,
                    cam.m_fProjMatrix,
                    nViewport,
                    &px,&py,&pz);  

  return (Vec3d((float)(px),(float)(py),(float)(pz)));                    */
  return (Vec3d(0,0,0));
}

void CGLRenderer::DrawQuad(float x0, float y0, float x1, float y1, const CFColor & color, float z)
{
  PROFILE_FRAME(Draw_2DImage);

  glColor4fv(&color[0]);

  glBegin(GL_QUADS);

  glTexCoord2f(0,0);
  glVertex3f(x0, y1, z);

  glTexCoord2f(1,0);
  glVertex3f(x1, y1, z);

  glTexCoord2f(1,1);
  glVertex3f(x1, y0, z);

  glTexCoord2f(0,1);
  glVertex3f(x0, y0, z);

  glEnd();

  m_nPolygons += 2;
}

//////////////////////////////////////////////////////////////////////
void CGLRenderer::DrawQuad(const Vec3d &right, const Vec3d &up, const Vec3d &origin,int nFlipmode/*=0*/)
{
  glBegin(GL_QUADS);

	Vec3d curr;

	if (nFlipmode==1)	
	{
	  curr=origin+(-right-up);
	  glTexCoord2f(1,0);
	  glVertex3fv((float *)&curr);

	  curr=origin+(right-up);
	  glTexCoord2f(0,0);
	  glVertex3fv((float *)&curr);

	  curr=origin+(right+up);
	  glTexCoord2f(0,1);
	  glVertex3fv((float *)&curr);

	  curr=origin+(-right+up);
	  glTexCoord2f(1,1);
	  glVertex3fv((float *)&curr);
	}
	else
	if (nFlipmode==2)	
	{
	  curr=origin+(-right-up);
	  glTexCoord2f(0,1);
	  glVertex3fv((float *)&curr);

	  curr=origin+(right-up);
	  glTexCoord2f(1,1);
	  glVertex3fv((float *)&curr);

	  curr=origin+(right+up);
	  glTexCoord2f(1,0);
	  glVertex3fv((float *)&curr);

	  curr=origin+(-right+up);
	  glTexCoord2f(0,0);
	  glVertex3fv((float *)&curr);
	}
	else
	{
	  curr=origin+(-right-up);
	  glTexCoord2f(0,0);
	  glVertex3fv((float *)&curr);

	  curr=origin+(right-up);
	  glTexCoord2f(1,0);
	  glVertex3fv((float *)&curr);

	  curr=origin+(right+up);
	  glTexCoord2f(1,1);
	  glVertex3fv((float *)&curr);

	  curr=origin+(-right+up);
	  glTexCoord2f(0,1);
	  glVertex3fv((float *)&curr);
	}

  glEnd();
}

//////////////////////////////////////////////////////////////////////
void CGLRenderer::ProjectToScreen( float ptx, float pty, float ptz, 
                                   float *sx, float *sy, float *sz )
{
  float g_ProjectionMatrix[16];
  float g_ModelMatrix[16];
  int   g_Viewport[4];

  glGetFloatv(GL_PROJECTION_MATRIX,g_ProjectionMatrix);
  glGetFloatv(GL_MODELVIEW_MATRIX,g_ModelMatrix);
  glGetIntegerv(GL_VIEWPORT,g_Viewport);

  SGLFuncs::gluProject(ptx,pty,ptz,
              g_ModelMatrix,
              g_ProjectionMatrix,
              g_Viewport,
              sx,sy,sz);
  (*sx) = (*sx) * 100/m_width;
  (*sy) = 100 - (*sy) * 100/m_height;
}

int CGLRenderer::UnProject(float sx, float sy, float sz, 
              float *px, float *py, float *pz,
              const float modelMatrix[16], 
              const float projMatrix[16], 
              const int    viewport[4])
{
  return SGLFuncs::gluUnProject(sx,sy,sz,
                    modelMatrix,
                    projMatrix,
                    viewport,
                    px,py,pz);
}

//////////////////////////////////////////////////////////////////////
int CGLRenderer::UnProjectFromScreen( float  sx, float  sy, float  sz, 
                                      float *px, float *py, float *pz)
{
  float g_ProjectionMatrix[16];
  float g_ModelMatrix[16];
  int    g_Viewport[4];

  glGetFloatv(GL_MODELVIEW_MATRIX,   g_ModelMatrix);
  glGetFloatv(GL_PROJECTION_MATRIX,  g_ProjectionMatrix);
  glGetIntegerv(GL_VIEWPORT,          g_Viewport);

  int res=SGLFuncs::gluUnProject(sx,sy,sz,
                    g_ModelMatrix,
                    g_ProjectionMatrix,
                    g_Viewport,
                    px,py,pz);
  CheckError("unproj");
  return(res);

}

//////////////////////////////////////////////////////////////////////
void CGLRenderer::Draw2dLine	(float x1,float y1,float x2,float y2)
{
	float x1pos=(float)(int)ScaleCoordX(x1);
  float y1pos=(float)(int)ScaleCoordY(y1);
	float x2pos=(float)(int)ScaleCoordX(x2);
  float y2pos=(float)(int)ScaleCoordY(y2);

  SetState(GS_NODEPTHTEST);
  SetCullMode(R_CULL_NONE);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, m_width, m_height, 0.0, -9999.0, 9999.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

	Vec3d Start(x1pos, y1pos, 0.0f);
	Vec3d End(x2pos, y2pos, 0.0f);
  EnableTMU(false);
  glBegin(GL_LINES);
  glVertex3fv(&Start.x);
  glVertex3fv(&End.x);
  glEnd();  
  EnableTMU(true);

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

void CGLRenderer::DrawLine(const Vec3d & vPos1, const Vec3d & vPos2)
{
  SetCullMode(R_CULL_DISABLE);
  EnableTMU(true);
  gRenDev->m_TexMan->m_Text_White->Set();
  glColor4f(1,1,1,1);
  glBegin(GL_LINES);
  glVertex3fv(&vPos1.x);
  glVertex3fv(&vPos2.x);
  glEnd();  
}

void CGLRenderer::DrawLineColor(const Vec3d & vPos1, const CFColor & vColor1, const Vec3d & vPos2, const CFColor & vColor2)
{
  EnableTMU(false);
  glBegin(GL_LINES);
  glColor4fv(&vColor1.r);
  glVertex3fv(&vPos1.x);
  glColor4fv(&vColor2.r);
  glVertex3fv(&vPos2.x);
  glEnd();  
  EnableTMU(true);
}


//////////////////////////////////////////////////////////////////////
void CGLRenderer::ScreenShot(const char *filename)
{
  char scname[512];
  int i;

	FILE *fp;
 
  if (!filename)
  {           
    strcpy(scname,"FarCry00.jpg");
      
    for (i=0 ; i<=99 ; i++) 
    { 
      scname[6] = i/10 + '0'; 
      scname[7] = i%10 + '0'; 
      //if (!CXFile::FileExist(scname)) 
			fp=fxopen(scname,"rb");
			if (!fp)
				break; // file doesn't exist
			fclose(fp);
    }

    if (i==100) 
    {     
      iLog->Log("Cannot save ScreenShot: Too many JPG files\n"); 
      return;
    }   
  }
  else 
    strcpy(scname,filename);

  iLog->Log("ScreenShot %s\n",scname);
  unsigned char *pic=new unsigned char [m_width*m_height*4];
  glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, pic);
  byte *src = pic;
  byte *dst = new byte[m_width*m_height*4];
  for (i=0; i<m_height; i++)
  {
    int ni0 = (m_height-i-1)*m_width*4;
    int ni1 = (i * m_width)*4;
    for (int j=0; j<m_width; j++)
    {
      *(uint *)&dst[ni0+j*4] = *(uint *)&src[ni1+j*4];
    }
  }
  ::WriteJPG(dst, m_width, m_height,scname);
    //CImage::SaveTga(pic,FORMAT_24_BIT,m_width,m_height,scname,false); 
    //unsigned char *pic=new unsigned char [m_width*m_height*FORMAT_8_BIT];
    //glReadPixels(0, 0, m_width, m_height, GL_ALPHA, GL_UNSIGNED_BYTE, pic);
  //CImage::SaveTga(pic,FORMAT_8_BIT,m_width,m_height,scname,false); 
  delete [] pic;    
  delete [] dst;
}

int CGLRenderer::ScreenToTexture()
{
  // for death effects
  unsigned char * pic = new unsigned char [m_width*m_height*3];
  glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, pic);
  uint tid;
  glGenTextures(1,&tid);
  m_TexMan->SetTexture(tid, eTT_Base);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pic);
  delete [] pic;    
  return tid;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// TEXTURE LOADING
////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
unsigned int CGLRenderer::LoadTexture(const char * _filename,int *tex_type,unsigned int def_tid,bool compresstodisk,bool bWarn)
{
  if (def_tid == 0)
    def_tid = -1;

  ITexPic * pPic = EF_LoadTexture(_filename,FT_NOREMOVE,0, eTT_Base, -1, -1, def_tid);
	
  if (pPic->IsTextureLoaded())
    return (pPic->GetTextureID());
	
  return (0);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
//FONT RENDERING
////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////
void CGLRenderer::Print(CXFont *currfont,float x, float y, const char *buf,float xscale,float yscale,float r,float g,float b,float a)
{
  x=ScaleCoordX(x);
  y=ScaleCoordY(y);

  int xsize=(int)(currfont->m_charsize*xscale*(float)(m_width)/800.0f);
  int ysize=(int)(currfont->m_charsize*yscale*(float)(m_height)/600.0f);

  float r1=r+0.25f;if (r1>1) r1=1;
  float g1=g+0.25f;if (g1>1) g1=1;
  float b1=b+0.25f;if (b1>1) b1=1;

  glBegin(GL_QUADS); 
  for (size_t i = strlen(buf); i; i--, x += xsize, buf++)	//AMD Port
  {
    float xot = (float)(*buf & 15)*currfont->m_char_inc;     
    float yot = 1.f-(-(float)(*buf >> 4)*currfont->m_char_inc);     

    glColor4f (r1,g1,b1,a);
    glTexCoord2f(xot, yot);
    glVertex2i  ((int)(x), (int)(y));
    glColor4f (r1,g1,b1,a);
    glTexCoord2f(xot+currfont->m_char_inc , yot);
    glVertex2i  ((int)(x+xsize),(int)(y));
    glColor4f (r,g,b,a);
    glTexCoord2f(xot+currfont->m_char_inc, yot+currfont->m_char_inc);
    glVertex2i  ((int)(x+xsize),(int)(y+ysize));
    glColor4f (r,g,b,a);
    glTexCoord2f(xot, yot+currfont->m_char_inc);    
    glVertex2i  ((int)(x),(int)(y+ysize));
  }
  glEnd();             

}

void CGLRenderer::ResetToDefault()
{
  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, ".... ResetToDefault ....\n");

  EF_Scissor(false, 0, 0, 0, 0);

  if (SUPPORTS_GL_ATI_fragment_shader)
    glDisable(GL_FRAGMENT_SHADER_ATI);
  if (SUPPORTS_GL_ARB_fragment_program)
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
  if (SUPPORTS_GL_NV_register_combiners)
    glDisable(GL_REGISTER_COMBINERS_NV);
  if (SUPPORTS_GL_NV_texture_shader && m_RP.m_ClipPlaneEnabled != 1)
    glDisable(GL_TEXTURE_SHADER_NV);
  if (SUPPORTS_GL_NV_vertex_program)
    glDisable(GL_VERTEX_PROGRAM_NV);
  if (SUPPORTS_GL_ARB_vertex_program)
    glDisable(GL_VERTEX_PROGRAM_ARB);

  if (m_RP.m_ClipPlaneWasOverrided == 1)
  {
    m_RP.m_ClipPlaneWasOverrided = 0;
    m_RP.m_ClipPlaneEnabled = 1;
  }
  for (int i=0; i<m_numtmus; i++)
  {
    if (i >= 4)
      break;
    EF_SelectTMU(i);
    m_eCurColorOp[i] = -1;
    m_eCurAlphaOp[i] = -1;
    m_eCurColorArg[i] = -1;
    m_eCurAlphaArg[i] = -1;
    m_fCurRGBScale[i] = -1.0f;
    CGLTexMan::m_TUState[i].m_Bind = 0;
    SArrayPointer_Texture::m_pLastPointer[i] = NULL;
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
    
    // tiago: added
    SetLodBias(CRenderer::CV_r_maxtexlod_bias);  

    if (!i)
    {
      CGLTexMan::m_TUState[i].m_Target = GL_TEXTURE_2D;
      if (m_RP.m_ClipPlaneEnabled == 1 && SUPPORTS_GL_NV_texture_shader)
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, CGLTexMan::m_TUState[i].m_Target);
      glEnable(GL_TEXTURE_2D);
      glDisable(GL_TEXTURE_CUBE_MAP_EXT);
      if (SUPPORTS_GL_NV_texture_rectangle || SUPPORTS_GL_EXT_texture_rectangle)
        glDisable(GL_TEXTURE_RECTANGLE_NV);
      EnableTexGen(false);    
      EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, 255, 255);
    }
    else
    {
      CGLTexMan::m_TUState[i].m_Target = 0;
      //glDisable(GL_TEXTURE_CUBE_MAP_EXT);
      glDisable(GL_TEXTURE_2D);
      if (SUPPORTS_GL_NV_texture_rectangle || SUPPORTS_GL_EXT_texture_rectangle)
        glDisable(GL_TEXTURE_RECTANGLE_NV);
      if (m_RP.m_ClipPlaneEnabled!=1 || i!=3)
      {
        EnableTexGen(false);    
      }
      else
      if (m_RP.m_ClipPlaneEnabled == 1 && SUPPORTS_GL_NV_texture_shader)
      {
        if (i == 3)
          glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_CULL_FRAGMENT_NV);
        else
        {
          glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_NONE);
          EnableTexGen(false);    
        }
      }
      else
        EnableTexGen(false);    
      EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, 255, 255);
    }
  }
  EF_SelectTMU(0);
  gRenDev->m_TexMan->m_nCurStages = 1;

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_ALPHA_TEST); 
  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glDepthFunc(GL_LEQUAL); 
  glDepthMask(GL_TRUE); 
  SetMaterialColor(1,1,1,1);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_NORMALIZE);

  glDisable(GL_POLYGON_OFFSET_FILL);
  glDisable(GL_POLYGON_OFFSET_LINE);
  glColorMask(1,1,1,1);

  glDisable(GL_POINT_SMOOTH);

  m_CurState = GS_DEPTHWRITE;
  m_RP.m_eCull = (ECull)-1;
  SArrayPointer::m_CurEnabled = PFE_POINTER_VERT;
  SArrayPointer_Vertex::m_pLastPointer = NULL;
  SArrayPointer_Normal::m_pLastPointer = NULL;
  SArrayPointer_Color::m_pLastPointer = NULL;
  SArrayPointer_SecColor::m_pLastPointer = NULL;
  glEnableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);     

  m_RP.m_FlagsPerFlush &= ~(RBSI_ALPHAGEN | RBSI_RGBGEN);
  m_RP.m_PersFlags &= ~(RBPF_PS1NEEDSET | RBPF_PS1NEEDSET | RBPF_TSNEEDSET | RBPF_VSNEEDSET);
  m_RP.m_PersFlags &= ~(RBPF_PS1WASSET  | RBPF_PS2WASSET  | RBPF_TSWASSET  | RBPF_VSWASSET);
  m_RP.m_FlagsModificators = 0;
  m_RP.m_CurrentVLights = 0;
  EF_CommitVLights();

  m_RP.m_PersFlags &= ~RBPF_WASWORLDSPACE;

  if (m_LogFile && CV_r_log == 3)
    Logv(SRendItem::m_RecurseLevel, ".... End ResetToDefault ....\n");

  CPShader::m_CurRC = NULL;
  CVProgram::m_LastVP = NULL;
  CPShader::m_LastVP = NULL;
}

int CGLRenderer::GenerateAlphaGlowTexture(float k)
{
//  float k = 6;
  const int tex_size = 256;
  unsigned char data[tex_size][tex_size];

  for(int x=0; x<tex_size; x++)
  for(int y=0; y<tex_size; y++)
  {
    int _x = x-tex_size/2;
    int _y = y-tex_size/2;

    float val = k*2.f*((float)tex_size/2 - (float)(sqrt(double(_x*_x+_y*_y))));
    if(val>255) 
      val=255;
    if(val<0) 
      val=0;

    data[x][y] = (int)(val);
  }

  return DownLoadToVideoMemory((unsigned char*)data,tex_size,tex_size,eTF_8000,eTF_8000,false,false,FILTER_LINEAR);
}

void CGLRenderer::SetMaterialColor(float r, float g, float b, float a)
{
  EF_SetGlobalColor(r, g, b, a);
}

int CGLRenderer::LoadAnimatedTexture(const char * szFileNameFormat,const int nCount)
{
	if(nCount<1)
		return 0;

	for(int t=0; t<m_LoadedAnimatedTextures.Count(); t++)
	{
		if(strcmp(m_LoadedAnimatedTextures[t]->sName, szFileNameFormat) == 0)
			if(m_LoadedAnimatedTextures[t]->nFramesCount == nCount)
			{
				m_LoadedAnimatedTextures[t]->nRefCounter++;
				return t+1;
			}
	}

	{
		char szFileName[256];
		sprintf(szFileName,szFileNameFormat,0);
		iLog->LogToFile("Loading animated texture %s", szFileName);
	}

	// load new textures
	AnimTexInfo * pInfo = new AnimTexInfo();
	pInfo->pBindIds = new int[nCount];
	memset(pInfo->pBindIds,0,nCount*sizeof(int));
	pInfo->nFramesCount = 0;
	strncpy(pInfo->sName, szFileNameFormat, sizeof(pInfo->sName));

	if (strstr(szFileNameFormat,"%") != 0)
	{
		// Formatted filename.
		for (int i=0; i<nCount; i++) 
		{
			char filename[80]="";
			sprintf(filename, szFileNameFormat, i);
			pInfo->pBindIds[i] = LoadTexture( filename );
			if (!pInfo->pBindIds[i])
				break;
			pInfo->nFramesCount++;
		}
	}
	else
	{
		char szFileName[_MAX_PATH];
		char szFileFormat[_MAX_PATH];
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		_splitpath( szFileNameFormat,drive,dir,fname,ext );

		int numDigits = 0;
		int namelen = strlen(fname);
		int i = namelen-1;
		// Find out number of digits in the end of the filename.
		while (i >= 0 && isdigit(fname[i]))
		{
			numDigits++;
			i--;
		}

		int firstFrame = 0;
		if (numDigits > 0)
		{
			const char *sNum = fname + namelen - numDigits;
			firstFrame = atoi(sNum);

			fname[namelen-numDigits] = 0;
			// Make format string.
			sprintf( fname+strlen(fname),"%%.%dd",numDigits );
		}
		_makepath( szFileFormat,drive,dir,fname,ext );
		for (int i=0; i<nCount; i++) 
		{
			sprintf( szFileName,szFileFormat,firstFrame + i );
			pInfo->pBindIds[i] = LoadTexture( szFileName );
			if (!pInfo->pBindIds[i])
				break;
			pInfo->nFramesCount++;
		}
	}

	iLog->LogToFilePlus(" (%d)", nCount);

	pInfo->nRefCounter=1;

	m_LoadedAnimatedTextures.Add(pInfo);

	return (int)m_LoadedAnimatedTextures.Count();
}

char * CGLRenderer::GetStatusText(ERendStats type)
{
  static char sStatusText[128]="";

  int num_used = 0, size_used = 0, num_all = 0, size_all = 0;

  int i;
  for(i=0; i<m_alloc_info.Count(); i++)
  {
    if(m_alloc_info[i].busy)
    {
      num_used++;
      size_used+=m_alloc_info[i].bytes_num;
    }
    num_all++;
    size_all+=m_alloc_info[i].bytes_num;

//    TextToScreen(1,(float)(10+i*3),"busy=%d bytes=%6d", (int)m_alloc_info[i].busy, (int)m_alloc_info[i].bytes_num);
  }

  sprintf(sStatusText,"Busy=%d(%.1fmb), All=%d(%.1fmb), BuffSize=%.1fmb", 
    num_used, float(size_used/1024)/1024, num_all, float(size_all/1024)/1024, float(m_pip_buffer_size/1024)/1024);

  return sStatusText;

/*
  // calculate resident textures
  int tex_count = m_imageList.size();
  int resident_count = 0;

  for (ImageIt it=m_imageList.begin(); !(it==m_imageList.end()); it++)
  {
    CImage *ci=(*it);

    uint tid = ci->GetTextureId();

    if(tid)
    {
      uchar res=0;
      glAreTexturesResident(1,&tid,&res);
      resident_count += res;
    }
  }

  int loaded_tests=0;
  for(i=0; i<m_texture_registry.Count(); i++)
  {
    if(m_texture_registry[i].bind_id)
    {
      loaded_tests++;
    }
  }

  sprintf(buf, "VAR %.3f/%.1f/%.1f %d TEX %3d/%3d REG %3d/%3d", 
    ((float)size_used)/1024, 
    (float)GetPipWaterLevel()/1024/1024, 
    (float)m_pip_buffer_size/1024/1024, 
    m_alloc_info.Count(),
    resident_count,tex_count,
    loaded_tests,m_texture_registry.Count());

  return buf;*/
}

void CGLRenderer::DrawBall(float x, float y, float z, float radius )
{
//  glColor3f(1,0,0);
  EnableTMU(true);
  gRenDev->m_TexMan->m_Text_White->Set();
  glPushMatrix();
  glTranslatef(x,y,z);
  GLUquadricObj * q = gluNewQuadric();
  gluSphere( q, radius, 16, 16 );
  gluDeleteQuadric(q);
  glPopMatrix();
}

void CGLRenderer::DrawBall(const Vec3d & pos, float radius )
{
  EnableTMU(true);
  gRenDev->m_TexMan->m_Text_White->Set();
  glPushMatrix();
  glTranslatef(pos.x,pos.y,pos.z);
  GLUquadricObj * q = gluNewQuadric();
  gluSphere( q, radius, 16, 16 );
  gluDeleteQuadric(q);
  glPopMatrix();
}

void CGLRenderer::DrawPoint(float x, float y, float z, float fSize)
{
  EnableTMU(true);
  gRenDev->m_TexMan->m_Text_White->Set();
  glPointSize(fSize);
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

  glEnable(GL_POINT_SMOOTH);

  glBegin(GL_POINTS);
    glVertex3f(x, y, z);
  glEnd();

  glDisable(GL_POINT_SMOOTH);
}

void CGLRenderer::SetClipPlane( int id, float * params )
{
  if (params)
    EF_SetClipPlane(true, params, false);
  else
    EF_SetClipPlane(false, NULL, false);
}


struct RGB { uchar r,g,b; };
struct RGBA { uchar r,g,b,a; };

//////////////////////////////////////////////////////////////////////
void CGLRenderer::ClearDepthBuffer()
{
  m_bWasCleared = true;

  EF_SetState(GS_DEPTHWRITE);
  glClear(GL_DEPTH_BUFFER_BIT);
}

void CGLRenderer::ClearColorBuffer(const Vec3d vColor)
{
  m_bWasCleared = true;

  glClearColor(vColor.x,vColor.y,vColor.z,1.f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void CGLRenderer::ReadFrameBuffer(unsigned char * pRGB, int nSizeX, int nSizeY, bool bBackBuffer, bool bRGBA, int nScaledX, int nScaledY)
{
  glFinish();

	if(bBackBuffer)
		glReadBuffer(GL_BACK);
	else
		glReadBuffer(GL_FRONT);

  if (nScaledX <= 0)
  {
    nScaledX = nSizeX;
    nScaledY = nSizeY;
  }

	glReadPixels(0, 0, nScaledX, nScaledY, bRGBA ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pRGB);
  glReadBuffer(GL_BACK);
}

void CGLRenderer::SetFogColor(float * color)
{
  m_FS.m_FogColor = CFColor(color);
  glFogfv(GL_FOG_COLOR,color);
}

void CGLRenderer::TransformTextureMatrix(float x, float y, float angle, float scale)
{
  glMatrixMode(GL_TEXTURE);
  glTranslatef(x,y,0);
  glTranslatef(0.5,0.5,0);
  glRotatef(angle,0,0,1);
  glScalef(scale,scale,scale);
  glTranslatef(-0.5,-0.5,0);
  glMatrixMode(GL_MODELVIEW);
}

void CGLRenderer::ResetTextureMatrix()
{
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
}


void CGLRenderer::DrawQuad(float dy,float dx, float dz, float x, float y, float z)
{
  glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f( -1, 0 ); 
    glVertex3f  (-dx+x, dy+y,-dz+z);
    glTexCoord2f(  0, 0 ); 
    glVertex3f  ( dx+x,-dy+y,-dz+z);  
    glTexCoord2f(  0, 1 ); 
    glVertex3f  ( dx+x,-dy+y, dz+z);  
    glTexCoord2f( -1, 1 ); 
    glVertex3f  (-dx+x, dy+y, dz+z);
  glEnd();                                  

  m_nPolygons+=2;
}


void CGLRenderer::EnableAALines(bool bEnable)
{
  if (bEnable)
  {
    glEnable (GL_LINE_SMOOTH);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
  }
  else
  {
    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_BLEND);
    glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
  }
}

//////////////////////////////////////////////////////////////////////
void CGLRenderer::Set2DMode(bool enable, int ortox, int ortoy)
{ 
  static int bOldFog = 0; 

  if(enable)
  {
    glGetIntegerv(GL_FOG, &bOldFog);
    glDisable(GL_FOG);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    //glOrtho(0.0, ortox, ortoy, 0.0, -19999.0, 19999.0);
		glOrtho(0.0, ortox, ortoy, 0.0, -1e30, 1e30 );
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
  }
  else
  {
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    if(bOldFog)
      glEnable(GL_FOG);
  }
  EF_SetCameraInfo();
}

void CGLRenderer::PrintToScreen(float x, float y, float size, const char *buf)
{
  PushMatrix();
  TranslateMatrix(Vec3d(x,y,0));

  EF_SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);
  SetTexture(iConsole->GetFont()->m_image->GetTextureID());
  EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);

  Print(iConsole->GetFont(),0,0,buf,size*0.5f/8, size*1.f/8, 1,1,1,1);

  PopMatrix();
}

// ogl to create matrix
void CGLRenderer::MakeMatrix(const Vec3d & pos, const Vec3d & angles,const Vec3d & scale, Matrix44 * mat)
{
  // make tran&rot matrix
  PushMatrix();
  LoadMatrix(0);
  TranslateMatrix(pos);  
  RotateMatrix(angles.z,0,0,1);
  RotateMatrix(angles.y,0,1,0);
  RotateMatrix(angles.x,1,0,0);
  ScaleMatrix(scale.x,scale.y,scale.z);
 // GetModelViewMatrix(&(mat->m_values(0,0)));
	GetModelViewMatrix((float*)(&mat));
  PopMatrix();
}

char*	CGLRenderer::GetVertexProfile(bool bSupportedProfile)
{
  CGprofile pr;
#ifndef WIN64
	// TODO: AMD64 port: find 64-bit CG
	pr = cgGLGetLatestProfile(CG_GL_VERTEX);
#else
  if ((m_Features & RFT_HW_MASK) == RFT_HW_GFFX)
    pr = CG_PROFILE_ARBVP1;
  else
    pr = CG_PROFILE_VP20;
#endif
  if (pr != CG_PROFILE_VP20 && SUPPORTS_GL_ARB_vertex_program)
    pr = CG_PROFILE_ARBVP1;
  else
    pr = CG_PROFILE_VP20;
  if (CGLRenderer::CV_gl_vsforce11 && SUPPORTS_GL_NV_vertex_program && !bSupportedProfile)
    pr = CG_PROFILE_VP20;
  if (pr == CG_PROFILE_VP20)
    return "PROFILE_VS_1_1";
  else
    return "PROFILE_VS_2_0";
}

char*	CGLRenderer::GetPixelProfile(bool bSupportedProfile)
{
  CGprofile pr;
#ifndef WIN64
	// TODO: AMD64 port: find 64-bit CG
	pr = cgGLGetLatestProfile(CG_GL_FRAGMENT);
#else
  if ((m_Features & RFT_HW_MASK) == RFT_HW_GFFX)
    pr = CG_PROFILE_ARBFP1;
  else
    pr = CG_PROFILE_FP20;
#endif
	if (pr != CG_PROFILE_FP20 && SUPPORTS_GL_ARB_fragment_program)
    pr = CG_PROFILE_ARBFP1;
  else
    pr = CG_PROFILE_FP20;
  if (CGLRenderer::CV_gl_psforce11 && SUPPORTS_GL_NV_register_combiners && !bSupportedProfile)
    pr = CG_PROFILE_FP20;
  if (pr == CG_PROFILE_FP20)
    return "PROFILE_PS_1_1";
  else
    return "PROFILE_PS_2_0";
}

void CGLRenderer::EF_PolygonOffset(bool bEnable, float fFactor, float fUnits)
{
  if (bEnable)
  {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(fFactor, fUnits);
  }
  else
  {
    glDisable(GL_POLYGON_OFFSET_FILL);
  }
}

//=========================================================================================

#ifdef CRTDBG_MAP_ALLOC

#pragma pack (push,1)
#define nNoMansLandSize 4
typedef struct MyCrtMemBlockHeader
{
	struct MyCrtMemBlockHeader * pBlockHeaderNext;
	struct MyCrtMemBlockHeader * pBlockHeaderPrev;
	char *                      szFileName;
	int                         nLine;
	size_t                      nDataSize;
	int                         nBlockUse;
	long                        lRequest;
	unsigned char               gap[nNoMansLandSize];
	/* followed by:
	*  unsigned char           data[nDataSize];
	*  unsigned char           anotherGap[nNoMansLandSize];
	*/
} MyCrtMemBlockHeader;
#pragma pack (pop)

#define pbData(pblock) ((unsigned char *)((MyCrtMemBlockHeader *)pblock + 1))
#define pHdr(pbData) (((MyCrtMemBlockHeader *)pbData)-1)


void crtdebug( const char *s,... )
{
	char str[32768];
	va_list arg_ptr;
	va_start( arg_ptr,s );
	vsprintf( str,s,arg_ptr );
	va_end( arg_ptr );
	
	FILE *l = fopen( "crtdump.txt","a+t" );
	if (l) {
		fprintf( l,"%s",str );
		fclose( l );
	}
}

int crtAllocHook(int nAllocType, void *pvData, 
      size_t nSize, int nBlockUse, long lRequest, 
      const unsigned char * szFileName, int nLine )
{
	if (nBlockUse == _CRT_BLOCK)
		return( TRUE );
	
	static int total_cnt = 0;
	static int total_mem = 0;
	if (nAllocType == _HOOK_ALLOC)
	{
		//total_mem += nSize;
		//total_cnt++;
		//_CrtMemState mem_state;
		//_CrtMemCheckpoint( &mem_state );
		//total_cnt = mem_state.lCounts[_NORMAL_BLOCK];
		//total_mem = mem_state.lTotalCount;
		if ((total_cnt&0xF) == 0) 
		{
			//_CrtCheckMemory();
		}

		total_cnt++;
		total_mem += nSize;

		//crtdebug( "<CRT> Alloc %d,size=%d,in: %s %d (total size=%d,num=%d)\n",lRequest,nSize,szFileName,nLine,total_mem,total_cnt );
    crtdebug( "Malloc:  Size=%d,	[Total=%d,N=%d]	[%s:%d]\n",nSize,total_mem,total_cnt,szFileName,nLine );
	}
  else
  if (nAllocType == _HOOK_FREE)
  {
		MyCrtMemBlockHeader *pHead;
		pHead = pHdr(pvData);
		
		total_cnt--;
		total_mem -= pHead->nDataSize;

    crtdebug( "Free:    Size=%d,	[Total=%d,N=%d]	[%s:%d]\n",pHead->nDataSize,total_mem,total_cnt,pHead->szFileName,pHead->nLine );
		//crtdebug( "<CRT> Free size=%d,in: %s %d (total size=%d,num=%d)\n",pHead->nDataSize,pHead->szFileName,pHead->nLine,total_mem,total_cnt );
		//total_mem -= nSize;
		//total_cnt--;
	}
  else
  if (nAllocType == _HOOK_REALLOC)
  {
		MyCrtMemBlockHeader *pHead;
		pHead = pHdr(pvData);
		
		//total_cnt++;
		total_mem -= pHead->nDataSize;
		total_mem += nSize;

    crtdebug( "Realloc: Size=%d,	[Total=%d,N=%d]	[%s:%d]\n",nSize,total_mem,total_cnt,pHead->szFileName,pHead->nLine );
		//crtdebug( "<CRT> Free size=%d,in: %s %d (total size=%d,num=%d)\n",pHead->nDataSize,pHead->szFileName,pHead->nLine,total_mem,total_cnt );
		//total_mem -= nSize;
		//total_cnt--;
	}
	return TRUE;
}

int crtReportHook(int nRptType, char *szMsg, int *retVal)
{
	static int gl_num_asserts = 0;
	if (gl_num_asserts != 0) return TRUE;
	gl_num_asserts++;
	switch (nRptType) {
		case _CRT_WARN:
			crtdebug( "<CRT WARNING> %s\n",szMsg );
			break;
		case _CRT_ERROR:
			crtdebug( "<CRT ERROR> %s\n",szMsg );
			break;
		case _CRT_ASSERT:
			crtdebug( "<CRT ASSERT> %s\n",szMsg );
			break;
	}
	gl_num_asserts--;
	return TRUE;
}

void InitCrt() {
	FILE *l = fopen( "crtdump.txt","w" );
	if (l) fclose(l);
	
   //_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
   //_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
   //_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );

	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_WNDW );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_WNDW );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_WNDW );

	//_CrtSetDbgFlag( _CRTDBG_CHECK_ALWAYS_DF|_CRTDBG_CHECK_CRT_DF|_CRTDBG_LEAK_CHECK_DF|_CRTDBG_DELAY_FREE_MEM_DF | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) );
	//_CrtSetDbgFlag( _CRTDBG_CHECK_CRT_DF|_CRTDBG_LEAK_CHECK_DF/*|_CRTDBG_DELAY_FREE_MEM_DF*/ | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) );
	int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	flags &= ~_CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_CRT_DF;

	_CrtSetDbgFlag( flags );

	_CrtSetAllocHook ( crtAllocHook );
	_CrtSetReportHook( crtReportHook );


  void *ptr = malloc(123123);
  ptr = realloc(ptr, 654654);
  free(ptr);
}

void DoneCrt() {
	//_CrtCheckMemory();
	//_CrtDumpMemoryLeaks();
}

// Autoinit CRT.
//struct __autoinit_crt { __autoinit_crt() { InitCrt(); }; ~__autoinit_crt() { DoneCrt(); } } __autoinit_crt_var;
#endif // CRTDBG_MAP_ALLOC
