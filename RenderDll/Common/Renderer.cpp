//////////////////////////////////////////////////////////////////////
//
//  Crytek CryENGINE Source code
//  
//  File:Renderer.cpp
//  Descriptin: Abstract renderer API
//
//  History:
//  -Feb 05,2001:Originally Created by Marco Corbetta
//  -: taken over by Andrey Khonich
//
//////////////////////////////////////////////////////////////////////

#include "RenderPCH.h"

#include "shadow_renderer.h"
#include "IStatObj.h"
#include "I3dengine.h"
#include <CREPolyMesh.h>

#if defined(LINUX)
	#include "ILog.h"
	#include "WinBase.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Globals.
//////////////////////////////////////////////////////////////////////////
CRenderer *gRenDev = NULL;
bool g_bProfilerEnabled = false;
int g_CpuFlags;
double g_SecondsPerCycle;

#ifndef _XBOX
#include <CrtDebugStats.h>
#endif

// per-frame profilers: collect the infromation for each frame for 
// displaying statistics at the beginning of each frame
//#define PROFILER(ID,NAME) DEFINE_FRAME_PROFILER(ID,NAME)
//#include "FrameProfilers-list.h"
//#undef PROFILER

// Common render console variables
int CRenderer::CV_r_vsync;
int CRenderer::CV_r_stats;
int CRenderer::CV_r_log;
int CRenderer::CV_r_logTexStreaming;
int CRenderer::CV_r_logVBuffers;
int CRenderer::CV_r_syncvbuf;
int CRenderer::CV_r_flush;

int CRenderer::CV_r_fsaa;
int CRenderer::CV_r_fsaa_samples;
int CRenderer::CV_r_fsaa_quality;

int CRenderer::CV_r_texturesstreaming;
int CRenderer::CV_r_texturesstreamingonlyvideo;
int CRenderer::CV_r_texturesstreamingsync;
int CRenderer::CV_r_texturesstreampoolsize;
int CRenderer::CV_r_texturespixelsize;
int CRenderer::CV_r_texturesbasemip;

int CRenderer::CV_r_supportpalettedtextures;
int CRenderer::CV_r_supportcompressedtextures;
#ifdef USE_3DC
int CRenderer::CV_r_texnormalmapcompressed;
#endif
int CRenderer::CV_r_texmaxsize;
int CRenderer::CV_r_texminsize;
int CRenderer::CV_r_texbumpquality;
int CRenderer::CV_r_texbumpresolution;
int CRenderer::CV_r_texquality;
int CRenderer::CV_r_texresolution;
int CRenderer::CV_r_texlmresolution;
int CRenderer::CV_r_texskyquality;
int CRenderer::CV_r_texskyresolution;
int CRenderer::CV_r_texforcesquare;
int CRenderer::CV_r_texquantizedither;
int CRenderer::CV_r_texnormalmaptype;
int CRenderer::CV_r_texgrayoverage;
int CRenderer::CV_r_texsimplemips;
int CRenderer::CV_r_texhwmipsgeneration;
int CRenderer::CV_r_texhwdxtcompression;

#ifdef USE_HDR
int CRenderer::CV_r_hdrrendering;
int CRenderer::CV_r_hdrdebug;
#endif
int CRenderer::CV_r_hdrfake;
float CRenderer::CV_r_hdrlevel;
float CRenderer::CV_r_hdrbrightoffset;
float CRenderer::CV_r_hdrbrightthreshold;

int CRenderer::CV_r_geominstancing;

int CRenderer::CV_r_bumpselfshadow;
int CRenderer::CV_r_selfshadow;
int CRenderer::CV_r_shadowtype;
int CRenderer::CV_r_shadowblur;
int CRenderer::CV_r_nobumpmap;
int CRenderer::CV_r_noloadtextures;
int CRenderer::CV_r_debuglights;
int CRenderer::CV_r_cullgeometryforlights;
//int CRenderer::CV_r_lightsscissor;
ICVar *CRenderer::CV_r_showlight;

int CRenderer::CV_r_nops20;
int CRenderer::CV_r_nops30;
int CRenderer::CV_r_nops2x;
int CRenderer::CV_r_sm30path;
int CRenderer::CV_r_sm2xpath;
int CRenderer::CV_r_offsetbump;
int CRenderer::CV_r_offsetbumpforce;
int CRenderer::CV_r_nopreprocess;
int CRenderer::CV_r_shaderssave;
int CRenderer::CV_r_shadersprecache;
int CRenderer::CV_r_precachemesh;
ICVar *CRenderer::CV_r_shaderdefault;

int CRenderer::CV_r_rb_merge;

int CRenderer::CV_r_accurateparticles;

int CRenderer::CV_r_unifytangentnormals;
int CRenderer::CV_r_smoothtangents;
int CRenderer::CV_r_indexingWithTangents;

int CRenderer::CV_r_measureoverdraw;
int CRenderer::CV_r_printmemoryleaks;

int CRenderer::CV_r_checkSunVis;
float CRenderer::CV_r_embm;
int CRenderer::CV_r_sse;

int CRenderer::CV_r_fullbrightness;

// tiago added
int CRenderer::CV_r_debugscreenfx;  
int CRenderer::CV_r_flipscreen;
int CRenderer::CV_r_dof;
float CRenderer::CV_r_doffocaldist;
float CRenderer::CV_r_doffocaldist_tag;
float CRenderer::CV_r_doffocaldist_entity;
float CRenderer::CV_r_doffocalarea;
float CRenderer::CV_r_doffocalareacurr;
float CRenderer::CV_r_dofbluramount;
int CRenderer::CV_r_doffocalsource;

int CRenderer::CV_r_resetscreenfx;
int CRenderer::CV_r_rendermode;  
int CRenderer::CV_r_glare;
int CRenderer::CV_r_glarequality;  
float CRenderer::CV_r_glaretransition;  

float CRenderer::CV_r_pp_contrast;  
float CRenderer::CV_r_pp_brightness;  
float CRenderer::CV_r_pp_gamma;  
float CRenderer::CV_r_pp_saturation;  
float CRenderer::CV_r_pp_sharpenamount;  
float CRenderer::CV_r_pp_glareintensity;  
float CRenderer::CV_r_pp_cmyk_c;  
float CRenderer::CV_r_pp_cmyk_m;  
float CRenderer::CV_r_pp_cmyk_y;  
float CRenderer::CV_r_pp_cmyk_k; 

int   CRenderer::CV_r_subsurface_type;  

float CRenderer::CV_r_maxtexlod_bias;  

int CRenderer:: CV_r_motionblur;
int CRenderer:: CV_r_motionblurdisplace;
float CRenderer:: CV_r_motionbluramount;

int CRenderer:: CV_r_heathaze;  
int CRenderer:: CV_r_cryvision;
int CRenderer:: CV_r_cryvisiontype;
int CRenderer:: CV_r_enhanceimage;
float CRenderer::CV_r_enhanceimageamount;
float CRenderer:: CV_r_fadeamount;
int CRenderer:: CV_r_scopelens_fx;  
int CRenderer:: CV_r_disable_sfx;  

int CRenderer::CV_r_heatsize;
int CRenderer::CV_r_lightsourcesasheatsources;
int CRenderer::CV_r_heatmapsave;
int CRenderer::CV_r_heatmapmips;
int CRenderer::CV_r_heattype;

int CRenderer::CV_r_flashbangsize;
int CRenderer::CV_r_screenrefract;

int CRenderer::CV_r_nightsize;
int CRenderer::CV_r_nightmapsave;
int CRenderer::CV_r_nighttype;

int CRenderer::CV_r_rainmapsize;

int CRenderer::CV_r_shaderdetailobjectsbending;
int CRenderer::CV_r_shaderterraindot3;
int CRenderer::CV_r_shaderterrainspecular;

int CRenderer::CV_r_portals;
int CRenderer::CV_r_portalsrecursive;

int CRenderer::CV_r_SunStyleCoronas;

int CRenderer::CV_r_stripmesh;

int CRenderer::CV_r_shownormals;
int CRenderer::CV_r_showlines;
float CRenderer::CV_r_normalslength;
int CRenderer::CV_r_showtangents;
int CRenderer::CV_r_showtimegraph;
int CRenderer::CV_r_showtextimegraph;
int CRenderer::CV_r_graphstyle;
 
int CRenderer::CV_r_hwlights;
int CRenderer::CV_r_flares;
int CRenderer::CV_r_procflares;
int CRenderer::CV_r_beams;
float CRenderer::CV_r_flaresize;

int CRenderer::CV_r_specantialias;
float CRenderer::CV_r_shininess;
float CRenderer::CV_r_wavescale;

int CRenderer::CV_r_texprocedures;
int CRenderer::CV_r_envlighting;
int CRenderer::CV_r_envlightcmdebug;
int CRenderer::CV_r_envlightcmsize;
int CRenderer::CV_r_envcmwrite;
int CRenderer::CV_r_envcmresolution;  // 0-64,1-128,2-256
int CRenderer::CV_r_envtexresolution; // 0-64,1-128,2-256
float CRenderer::CV_r_waterupdateDeltaAngle;
float CRenderer::CV_r_waterupdateFactor;
float CRenderer::CV_r_waterupdateDistance;
float CRenderer::CV_r_envlcmupdateinterval;
float CRenderer::CV_r_envcmupdateinterval;
float CRenderer::CV_r_envtexupdateinterval;
int CRenderer::CV_r_waterreflections;
int CRenderer::CV_r_waterrefractions;
int CRenderer::CV_r_waterbeachrefractions;
int CRenderer::CV_r_selfrefract;
int CRenderer::CV_r_texture_anisotropic_level;

int CRenderer::CV_r_oceanrendtype;
int CRenderer::CV_r_oceanloddist;
int CRenderer::CV_r_oceantexupdate;
int CRenderer::CV_r_oceanmaxsplashes;
int CRenderer::CV_r_oceansectorsize;
int CRenderer::CV_r_oceanheightscale;
float CRenderer::CV_r_oceansplashscale;

ICVar *CRenderer::CV_r_glossdefault;
ICVar *CRenderer::CV_r_detaildefault;
ICVar *CRenderer::CV_r_opacitydefault;

int CRenderer::CV_r_reloadshaders;

int CRenderer::CV_r_cullbyclipplanes;
int CRenderer::CV_r_detailtextures;
int CRenderer::CV_r_decaltextures;

int CRenderer::CV_r_detailnumlayers;
float CRenderer::CV_r_detailscale;
float CRenderer::CV_r_detaildistance;
int CRenderer::CV_r_logloadshaders;
int CRenderer::CV_r_usehwshaders;
int CRenderer::CV_r_nolightcalc;
int CRenderer::CV_r_texbindmode;
int CRenderer::CV_r_nodrawshaders;
int CRenderer::CV_r_nodrawnear;
int CRenderer::CV_r_profileshaders;
ICVar *CRenderer::CV_r_excludeshader;
ICVar *CRenderer::CV_r_showonlyshader;
int CRenderer::CV_r_logusedtextures;
int CRenderer::CV_r_logusedshaders;
float CRenderer::CV_r_gamma;
float CRenderer::CV_r_contrast;
float CRenderer::CV_r_brightness;
int CRenderer::CV_r_nohwgamma;
int CRenderer::CV_r_noswgamma;

int CRenderer::CV_r_scissor;

int CRenderer::CV_r_coronas;
float CRenderer::CV_r_coronafade;
float CRenderer::CV_r_coronasizescale;
float CRenderer::CV_r_coronacolorscale;

int CRenderer::CV_r_efmultitex;
int CRenderer::CV_r_noparticles;

int CRenderer::CV_ind_VisualizeShadowVolumes;
int CRenderer::CV_ind_ReverseShadowVolumes;     
int CRenderer::CV_ind_DrawBorderEdges;

int CRenderer::CV_r_PolygonMode;
int CRenderer::CV_r_GetScreenShot;

int CRenderer::CV_r_VolumetricFog;
int CRenderer::CV_r_vpfog;

int CRenderer::CV_r_DisplayInfo;

float CRenderer::CV_r_character_lod_bias;
int CRenderer::CV_r_character_nodeform;
int CRenderer::CV_r_character_debug;
int CRenderer::CV_r_character_noanim;
int CRenderer::CV_r_character_shadow_volume;
int CRenderer::CV_r_character_nophys;

int CRenderer::CV_r_shadow_maps_debug;
int CRenderer::CV_r_draw_phys_only;

int CRenderer::CV_r_ReplaceCubeMap;

int CRenderer::CV_r_VegetationSpritesAlphaBlend;
int CRenderer::CV_r_VegetationSpritesNoBend;
int CRenderer::CV_r_Vegetation_IgnoreVertColors;
int CRenderer::CV_r_Vegetation_PerpixelLight;

int CRenderer::CV_r_Quality_BumpMapping;
int CRenderer::CV_r_Quality_Reflection;

int CRenderer::CV_r_ShowVideoMemoryStats;

//////////////////////////////////////////////////////////////////////
bool CRenderer::m_showfps=false;  

//////////////////////////////////////////////////////////////////////


CRenderer::CRenderer()
{ 
  if (!gRenDev)
    gRenDev = this;

  //_controlfp(_EM_INEXACT, _MCW_EM);

#ifdef USE_HDR
  iConsole->Register("r_HDRRendering", &CV_r_hdrrendering, 0, VF_DUMPTODISK,
    "Toggles HDR rendering.\n"
    "Usage: r_HDRRendering [0/1]\n"
    "Default is 1 (on). Set to 0 to disable HDR rendering.");
  iConsole->Register("r_HDRDebug", &CV_r_hdrdebug, 0, 0,
    "Toggles HDR debugging info.\n"
    "Usage: r_HDRDebug [0/1]\n"
    "Default is 0 (off). Set to 1 to show HDR textures on the screen.");
#endif
  iConsole->Register("r_HDRFake", &CV_r_hdrfake, 1, 0,
    "Toggles HDR fake.\n"
    "Usage: r_HDRFake [0/1]\n"
    "Default is 1 (off). Set to 0 to disable.");
  iConsole->Register("r_HDRLevel", &CV_r_hdrlevel, 0.6f, VF_DUMPTODISK,
    "HDR rendering level.\n"
    "Usage: r_HDRLevel [Value]\n"
    "Default is 0.6f");
  iConsole->Register("r_HDRBrightThreshold", &CV_r_hdrbrightthreshold, 3.0f, VF_DUMPTODISK,
    "HDR rendering bright threshold.\n"
    "Usage: r_HDRBrightThreshold [Value]\n"
    "Default is 3.0f");
  iConsole->Register("r_HDRBrightOffset", &CV_r_hdrbrightoffset, 6.0f, VF_DUMPTODISK,
    "HDR rendering bright offset.\n"
    "Usage: r_HDRBrightOffset [Value]\n"
    "Default is 6.0f");
  iConsole->Register("r_GeomInstancing", &CV_r_geominstancing, 1, 0,
    "Toggles HW geometry instancing.\n"
    "Usage: r_GeomInstancing [0/1]\n"
    "Default is 0 (off). Set to 0 to disable geom. instanicng.");
  iConsole->Register("r_NoBumpmap", &CV_r_nobumpmap, 0,0,
    "Disables bump perpixel lighting in shaders.\n"
    "Usage: r_NoBumpmap [0/1]\n"
    "Default is 0 (bump per pixel lighting enabled).\n"
    "The shader should support the appropriate condition.");
  iConsole->Register("r_BumpSelfShadow", &CV_r_bumpselfshadow, 0,VF_REQUIRE_LEVEL_RELOAD,
    "Enables per-pixel shadows with bumpmapping.\n"
    "Usage: r_BumpSelfShadow [0/1]\n"
    "Default is 0 (bump per pixel shadows disabled).\n"
    "The shader should support the appropriate condition.");
  iConsole->Register("r_SelfShadow", &CV_r_selfshadow, 0, VF_DUMPTODISK|VF_REQUIRE_LEVEL_RELOAD,
    "Enables per-pixel shadows depth comparing.\n"
    "Usage: r_SelfShadow [0/1]\n"
    "The shader should support the appropriate condition.");
  iConsole->Register("r_ShadowType", &CV_r_shadowtype, 0,VF_REQUIRE_APP_RESTART,
    "Selected shadow map type.\n"
    "Default is 0 (most appropriate shadows dependent on hardware:\n"
    "(2D shadow maps or Depth maps or Depth comparing on ATI).\n"
    "1: using only 2d shadows\n"
    "Usage: r_ShadowType [0/1]\n"
    "The shader should support the appropriate condition.");
  iConsole->Register("r_ShadowBlur", &CV_r_shadowblur, 1, VF_DUMPTODISK,
    "Selected shadow map blur.\n"
    "Usage: r_ShadowBlur [0/1]\n"
    "The shader should support the appropriate condition.");
  iConsole->Register("r_DebugLights", &CV_r_debuglights, 0,0,
    "Display dynamic lights for debugging.\n"
    "Usage: r_DebugLights [0/1/2]\n"
    "Default is 0 (off). Set to 1 to display centres of light sources,\n"
    "or set to 2 to display light centres and attenuation spheres.");
//  iConsole->Register("r_LightsScissor", &CV_r_lightsscissor, 1);
  iConsole->Register("r_CullGeometryForLights", &CV_r_cullgeometryforlights, 0,0,
    "Rendering optimization for lights.\n"
    "Usage: r_CullGeometryForLights [0/1/2]\n"
    "Default is 0 (off). Set to 1 to cull geometry behind\n"
    "light sources. Set to 2 to cull geometry behind static\n"
    "lights only.");
  CV_r_showlight = iConsole->CreateVariable("r_ShowLight", "0", NULL,
    "Display a light source by name.\n"
    "Usage: r_ShowLight lightname\n"
    "Default is 0. Set to 'lightname' to show only the light\n"
    "from the source named 'lightname'.");


  // tiago: added

  // tiago: added  
  iConsole->Register("r_DebugScreenEffects", &CV_r_debugscreenfx, 0, NULL,
    "Debugs screen effects textures.\n"
    "Usage: r_DebugScreenEffects #\n"
    "Where # represents:\n"
    " 0: disabled (default)\n"
    " 1: enabled\n");

  iConsole->Register("r_FlipScreen", &CV_r_flipscreen, 0, NULL,
    "Flips screen texture. Only works on hi/med-spec\n"
    "Usage: r_FlipScreen #\n"
    "Where # represents:\n"
    " 0: disabled (default)\n"
    " 1: enabled\n");

  iConsole->Register("r_Dof", &CV_r_dof, 0, NULL,
    "Activates Depth of field\n"
    "Usage: r_Dof #\n"
    "Where # represents:\n"
    " 0: disabled (default)\n"
    " 1: enabled\n");

  iConsole->Register("r_DofFocalDist", &CV_r_doffocaldist, 20.0f, NULL,
    "Sets Depth of field focal distance\n"
    "Usage: r_DofFocalDist #\n"
    "Where # represents:\n"
    " Number: focal plane distance (default 20)\n");

  iConsole->Register("r_DofFocalDist_tag", &CV_r_doffocaldist_tag, 20.0f, NULL,
    "Internal. Do not use.\n");

  iConsole->Register("r_DofFocalDist_entity", &CV_r_doffocaldist_entity, 20.0f, NULL,
    "Internal. Do not use.\n");

  iConsole->Register("r_DofFocalArea", &CV_r_doffocalarea, 5.0f, NULL,
    "Sets Depth of field focal area\n"
    "Usage: r_DofFocalArea #\n"
    "Where # represents:\n"
    " Number: focal plane area (default 5.0)\n");

  iConsole->Register("r_DofFocalAreaCurr", &CV_r_doffocalareacurr, 5.0f, NULL,
    "Sets Depth of field focal area (internal)\n");

  iConsole->Register("r_DofBlurAmount", &CV_r_dofbluramount, 1.0f, NULL,
    "Sets Depth of field bluring amount\n"
    "Usage: r_DofBlurAmount #\n"
    "Where # represents:\n"
    " 0.0: disabled \n"
    " 1.0: maximum amount (default)\n");

  iConsole->Register("r_DofFocalSource", &CV_r_doffocalsource, 0, NULL,
    "Sets Depth of field focal source\n"
    "Usage: r_DofFocalSource #\n"
    "Where # represents:\n"
    " 0: r_DofFocalDist (default)\n"
    " 1: Camera Focus Tag Point\n"
    " 2: Camera Focus Entity\n");
    
  iConsole->Register("r_ResetScreenFx", &CV_r_resetscreenfx, 0, NULL,
    "Resets screen effects state.\n"
    "Usage: r_ResetScreenFx #\n"
    "Where # represents:\n"
    " 0: Normal (default)\n"
    " 1: Reset\n");

  iConsole->Register("r_RenderMode", &CV_r_rendermode, 0, VF_DUMPTODISK,
    "Defines type of post-processing rendering mode.\n"
    "Usage: r_RenderMode #\n"
    "Where # represents:\n"
    " 0: Default rendering (default)\n"
    " 1: Improved\n"
    " 2: Paradisiacal\n"
    " 3: Cold realism\n"
    " 4: Cartoon\n"
    " 5: Cinematic\n");
  
  iConsole->Register("r_Glare", &CV_r_glare, 0, VF_DUMPTODISK,
    "Activates the glare effect.\n"
    "Usage: r_Glare #\n"
    "Where # represents:\n"
    " 0: Off\n"
    " 1: On\n");

  iConsole->Register("r_GlareQuality", &CV_r_glarequality, 2, VF_DUMPTODISK,
    "Sets glare effect blur quality.\n"
    "Usage: r_GlareQuality #\n"
    "Where # represents:\n"
    " 0: Min quality\n"
    " 1: Med quality\n"
    " 2: Max quality\n");

  iConsole->Register("r_GlareTransition", &CV_r_glaretransition, 10, 0,
    "Set glare transition speed.\n"
    "Usage: r_GlareTransition #\n"
    "Where # represents:\n"
    " 0: Imediate\n"
    " Number: (default 6)\n");


  iConsole->Register("r_pp_Contrast", &CV_r_pp_contrast, 1, 0,
    "Set contrast for post processing render modes that require it.\n"
    "Usage: r_pp_Constrast (default is 1)n\n"
    "Where n represents a number: eg: 0.2\n");

  iConsole->Register("r_pp_Brightness", &CV_r_pp_brightness, 0, 0,
    "Set brightness for post processing render modes that require it.\n"
    "Usage: r_pp_Brightness n (default is 0)\n"
    "Where n represents a number : eg: 0.2\n");

  iConsole->Register("r_pp_Gamma", &CV_r_pp_gamma, 1, 0,
    "Set gamma for post processing render modes that require it.\n"
    "Usage: r_pp_Gamma n (default is 1)\n"
    "Where n represents a number: eg: 0.2\n");

  iConsole->Register("r_pp_Saturation", &CV_r_pp_saturation, 1, 0,
    "Set saturation for post processing render modes that require it.\n"
    "Usage: r_pp_Saturation n (default is 1)\n"
    "Where n represents a number: eg: 0.2\n");

  iConsole->Register("r_pp_SharpenAmount", &CV_r_pp_sharpenamount, 1, 0,
    "Set sharpening amount for post processing render modes that require it.\n"
    "Usage: r_pp_SharpenAmount n (default is 1)\n"
    "Where n represents a number: eg: 0.2\n");

  iConsole->Register("r_pp_GlareIntensity", &CV_r_pp_glareintensity, 1, 0,
    "Set glare intensity amount for post processing render modes that require it.\n"
    "Usage: r_pp_GlareIntensity n (default is 1)\n"
    "Where n represents a number: eg: 0.2\n");

  iConsole->Register("r_pp_CMYK_C", &CV_r_pp_cmyk_c, 0, 0,
    "Set CMYK changes in Cyan channel, for post processing render modes that require it.\n"
    "Usage: r_pp_CMYK_C n (default is 0)\n"
    "Where n represents a number: eg: 0.2\n");

  iConsole->Register("r_pp_CMYK_M", &CV_r_pp_cmyk_m, 0, 0,
    "Set CMYK changes in Magenta channel, for post processing render modes that require it.\n"
    "Usage: r_pp_CMYK_M n (default is 0)\n"
    "Where n represents a number: eg: 0.2\n");

  iConsole->Register("r_pp_CMYK_Y", &CV_r_pp_cmyk_y, 0, 0,
    "Set CMYK changes in yellow channel, for post processing render modes that require it.\n"
    "Usage: r_pp_CMYK_Y n (default is 0)\n"
    "Where n represents a number: eg: 0.2\n");

  iConsole->Register("r_pp_CMYK_K", &CV_r_pp_cmyk_k, 0, 0,
    "Set CMYK changes in luminance channel, for post processing render modes that require it.\n"
    "Usage: r_pp_CMYK_K n (default is 0)\n"
    "Where n represents a number: eg: 0.2\n");

  iConsole->Register("r_MaxTexLodBias", &CV_r_maxtexlod_bias, 0, 0,
    "Set max texture lod.\n"
    "Usage: r_MaxTexLodBias #\n"
    "Where # represents:\n"
    " 0: (default)\n"
    " Number: (negative decreases lod, positive increases lod)\n");

  iConsole->Register("r_HeatHaze", &CV_r_heathaze, 1, VF_DUMPTODISK,
    "Enables heat haze effect.\n"
    "Usage: r_HeatHaze #\n"
    "Where # represents:\n"
    " 0: Off\n"
    " 1: On (default)\n");

  iConsole->Register("r_Cryvision", &CV_r_cryvision, 0, 0,
          "Sets cryvision effect.\n"
          "Usage: r_Cryvision #\n"
          "Where # represents:\n"
          " 0: (off default)\n"
          " 1: (on)c \n");

  iConsole->Register("r_CryvisionType", &CV_r_cryvisiontype, 1, VF_DUMPTODISK,
    "Sets cryvision type effect.\n"
    "Usage: r_CryvisionType #\n"
    "Where # represents:\n"
    " 0: hi-spec (default)\n"
    " 1: lo-spec \n");
    
  iConsole->Register("r_EnhanceImage", &CV_r_enhanceimage, 1, VF_DUMPTODISK,
    "Enables image post-processing enhancing.\n"
    "Usage: r_EnhanceImage #\n"
    "Where # represents:\n"
    " 0: Off\n"
    " 1: On (default)\n");

  iConsole->Register("r_EnhanceImageAmount", &CV_r_enhanceimageamount, 1, VF_DUMPTODISK,
    "Defines image post-processing enhancing amount.\n"
    "Usage: r_EnhanceImage #\n"
    "Where # represents:\n"
    " value in [0.0f, 1.0f] range\n"
    " Default is 1.0f\n");  

  iConsole->Register("r_SubSurfaceType", &CV_r_subsurface_type, 1, 0,
    "Sets subsurface scatering type\n"
    "0: per vertex (default)\n"
    "1: per pixel\n");

  iConsole->Register("r_FadeAmount", &CV_r_fadeamount, 1, 0,
    "\n"
    "Usage: \n"
    "");

  iConsole->Register("r_MotionBlur", &CV_r_motionblur, 0, VF_DUMPTODISK,
    "Toggles the motion blur effect.\n"
    "Usage: r_MotionBlur [0/1]\n"
    "Default is 0 (off). Set to 1 to enable motion blur effect.");
  iConsole->Register("r_MotionBlurAmount", &CV_r_motionbluramount, 0.9f,0,
    "Sets the strength of motion blur effect.\n"
    "Usage: r_MotionBlurAmount 0.9\n"
    "Default is 0.9.");
  iConsole->Register("r_MotionBlurDisplace", &CV_r_motionblurdisplace, 4,0,
    "Sets the motion blur displacement.\n"
    "Usage: r_MotionBlurDisplace 4\n"
    "Default is 4.");
  
  iConsole->Register("r_ScopeLens_fx", &CV_r_scopelens_fx, 1, VF_DUMPTODISK,
    "Enables/disables scope lens fx.\n"
    "Usage: r_ScopeLens_fx [0/1]\n"
    "Default is 1 (enabled).");  


  iConsole->Register("r_DisableSfx", &CV_r_disable_sfx, 0, VF_DUMPTODISK,
    "Disables Sfx.\n"
    "Usage: r_DisableSfx [0/1]\n"
    "Default is 0 (Sfx enabled).");  

  iConsole->Register("r_HeatSize", &CV_r_heatsize, 256);
  iConsole->Register("r_LightSourcesAsHeatSources", &CV_r_lightsourcesasheatsources, 0);
  iConsole->Register("r_HeatMapSave", &CV_r_heatmapsave, 0);
  iConsole->Register("r_HeatMapMips", &CV_r_heatmapmips, 0);
  iConsole->Register("r_HeatType", &CV_r_heattype, 1);

  iConsole->Register("r_FlashBangSize", &CV_r_flashbangsize, 128);
  iConsole->Register("r_ScreenRefract", &CV_r_screenrefract, 0);
  
  iConsole->Register("r_NightSize", &CV_r_nightsize, 512);
  iConsole->Register("r_NightMapSave", &CV_r_nightmapsave, 0);
  iConsole->Register("r_NightType", &CV_r_nighttype, 1);

  iConsole->Register("r_RainMapSize", &CV_r_rainmapsize, 256);

  iConsole->Register("r_Portals", &CV_r_portals, 1, 0,
    "Toggles renderer support for portals."
    "Usage: r_Portals [0/1]"
    "Default is 1 (on).");
  iConsole->Register("r_PortalsRecursive", &CV_r_portalsrecursive, 0, 0,
    "Toggles support for recursive portals.\n"
    "Usage: r_PortalsRecursive [0/1]\n"
    "Default is 0 (off). Up to 4 recursions are supported.");

  iConsole->Register("r_ShaderDetailObjectsBending", &CV_r_shaderdetailobjectsbending, 0);
  iConsole->Register("r_ShaderTerrainDOT3", &CV_r_shaderterraindot3, 0);
  iConsole->Register("r_ShaderTerrainSpecular", &CV_r_shaderterrainspecular, 0);

  iConsole->Register("r_SupportPalettedTextures", &CV_r_supportpalettedtextures, 1);
  iConsole->Register("r_SupportCompressedTextures", &CV_r_supportcompressedtextures, 1);
#ifdef USE_3DC
  iConsole->Register("r_TexNormalMapCompressed", &CV_r_texnormalmapcompressed, 0, 0,
    "Toggles support for normal-maps compression.\n"
    "3Dc for ATI and V8U8 for NVidia"
    "Usage: r_TexNormalMapCompressed [0/1]\n"
    "Default is 1 (on).");
#endif
  iConsole->Register("r_TexMaxSize", &CV_r_texmaxsize, 0);
  iConsole->Register("r_TexMinSize", &CV_r_texminsize, 64);
  iConsole->Register("r_TexBumpQuality", &CV_r_texbumpquality, 0);  

  // changing default settings to reduce the insane amount of texture memory
  iConsole->Register("r_TexBumpResolution", &CV_r_texbumpresolution, 0, VF_DUMPTODISK );
  //iConsole->Register("r_TexBumpResolution", &CV_r_texbumpresolution, 1, VF_DUMPTODISK );

  iConsole->Register("r_TexQuality", &CV_r_texquality, 0, VF_DUMPTODISK );
  //iConsole->Register("r_TexQuality", &CV_r_texquality, 2, VF_DUMPTODISK );

  iConsole->Register("r_TexResolution", &CV_r_texresolution, 0, VF_DUMPTODISK,
    "Halves or doubles texture resolution.\n"
    "Usage: r_TexResolution [0/1]\n"
    "When 0 (default) texture resolution is halved.");
  //iConsole->Register("r_TexResolution", &CV_r_texresolution, 1, VF_DUMPTODISK );

  iConsole->Register("r_TexLMResolution", &CV_r_texlmresolution, 0, VF_DUMPTODISK,
    "Halves or doubles texture resolution.\n"
    "Usage: r_TexResolution [0/1]\n"
    "When 0 (default) texture resolution is halved.");

  iConsole->Register( "r_TexSkyResolution", &CV_r_texskyresolution, 0, VF_DUMPTODISK );
  //iConsole->Register( "r_TexSkyResolution", &CV_r_texskyresolution, 1, VF_DUMPTODISK );

  iConsole->Register("r_TexSkyQuality", &CV_r_texskyquality, 0);
  iConsole->Register("r_TexForceSquare", &CV_r_texforcesquare, 0);
  iConsole->Register("r_Texture_Anisotropic_Level", &CV_r_texture_anisotropic_level, 1, VF_DUMPTODISK);
  iConsole->Register("r_TexQuantizeDither", &CV_r_texquantizedither, 0);
  iConsole->Register("r_TexNormalMapType", &CV_r_texnormalmaptype, 1, VF_REQUIRE_LEVEL_RELOAD);
  iConsole->Register("r_TexGrayOverage", &CV_r_texgrayoverage, 1);
  iConsole->Register("r_TexSimpleMips", &CV_r_texsimplemips, 1);
  iConsole->Register("r_TexHWMipsGeneration", &CV_r_texhwmipsgeneration, 1);
  iConsole->Register("r_TexHWDXTCompression", &CV_r_texhwdxtcompression, 1);

  iConsole->Register("r_TexturesStreamPoolSize", &CV_r_texturesstreampoolsize, 0, VF_DUMPTODISK );
  iConsole->Register("r_TexturesStreamingSync", &CV_r_texturesstreamingsync, 0);
  iConsole->Register("r_TexturesStreamingOnlyVideo", &CV_r_texturesstreamingonlyvideo, 0);
  iConsole->Register("r_TexturesStreaming", &CV_r_texturesstreaming, 0, VF_REQUIRE_APP_RESTART,
    "Enables direct streaming of textures from disk during game.\n"
    "Usage: r_TexturesStreaming [0/1]\n"
    "Default is 0 (off). All textures save in native format with mips in a\n"
    "cache file. Textures are then loaded into texture memory from the cache.");
  iConsole->Register("r_TexturesPixelSize", &CV_r_texturespixelsize, 32);
  iConsole->Register("r_TexturesBaseMip", &CV_r_texturesbasemip, 0);

  iConsole->Register("r_FSAA", &CV_r_fsaa, 0, VF_DUMPTODISK | VF_REQUIRE_APP_RESTART);
  iConsole->Register("r_FSAA_samples", &CV_r_fsaa_samples, 4, VF_DUMPTODISK | VF_REQUIRE_APP_RESTART);
  iConsole->Register("r_FSAA_quality", &CV_r_fsaa_quality, 0, VF_DUMPTODISK | VF_REQUIRE_APP_RESTART);

  iConsole->Register("r_RB_Merge", &CV_r_rb_merge, 0, 0 );

  iConsole->Register("r_ShowNormals", &CV_r_shownormals, 0, VF_CHEAT,
    "Toggles visibility of normal vectors.\n"
    "Usage: r_ShowNormals [0/1]"
    "Default is 0 (off).");
  iConsole->Register("r_ShowLines", &CV_r_showlines, 0, VF_CHEAT,
    "Toggles visibility of wireframe overlay.\n"
    "Usage: r_ShowLines [0/1]"
    "Default is 0 (off).");
  iConsole->Register("r_NormalsLength", &CV_r_normalslength, 0.1f, 0,
    "Sets the length of displayed vectors.\n"
    "r_NormalsLength 0.1\n"
    "Default is 0.1 (metres). Used with r_ShowTangents and r_ShowNormals.");
  iConsole->Register("r_ShowTangents", &CV_r_showtangents, 0, VF_CHEAT,
    "Toggles visibility of three tangent space vectors.\n"
    "Usage: r_ShowTangents [0/1]\n"
    "Default is 0 (off).");
  iConsole->Register("r_ShowTimeGraph", &CV_r_showtimegraph, 0, 0,
    "Configures graphic display of frame-times.\n"
    "Usage: r_ShowTimeGraph [0/1/2]\n"
    " 1: Graph displayed as points."
    " 2: Graph displayed as lines."
    "Default is 0 (off).");
  iConsole->Register("r_ShowTexTimeGraph", &CV_r_showtextimegraph, 0, 0,
    "Configures graphic display of frame-times.\n"
    "Usage: r_ShowTexTimeGraph [0/1/2]\n"
    " 1: Graph displayed as points."
    " 2: Graph displayed as lines."
    "Default is 0 (off).");
  iConsole->Register("r_UnifyTangentNormals", &CV_r_unifytangentnormals, 1, VF_REQUIRE_LEVEL_RELOAD,
    "Enables use of vertex normals as tangent space normals.\n"
    "Usage: r_UnifyTangentNormals [0/1]\n"
    "Default is 1 (on).");
  iConsole->Register("r_SmoothTangents", &CV_r_smoothtangents, 1, VF_REQUIRE_LEVEL_RELOAD,
    "Enables interpolation of tangent vectors between non-shared vertices.\n"
    "Usage: r_SmoothTangents [0/1]\n"
    "Default is 1 (on). Very useful for surfaces with light-maps.");
  iConsole->Register("r_IndexingWithTangents", &CV_r_indexingWithTangents, 1, VF_REQUIRE_LEVEL_RELOAD,
    "Checks the angle between tangent space vectors during indexing.\n"
    "Usage: r_IndexingWithTangents [0/1]\n"
    "Default is 1 (on). If the angle between tangent space vectors\n"
    "is more than 60 degrees, the vertices will not be shared.");
  iConsole->Register("r_GraphStyle", &CV_r_graphstyle, 0);

  iConsole->Register("r_LogUsedTextures", &CV_r_logusedtextures, 0, 0,
    "Configures texture information logging.\n"
    "Usage: r_LogUsedTextures #\n"
    "where # represents:\n"
    " 0: Texture logging off\n"
    " 1: All loaded textures logged to 'UsedTextures.txt'\n"
    " 2: Texture information logged to screen\n"
    " 3: Missing textures logged to 'MissingTextures.txt");
  iConsole->Register("r_LogUsedShaders", &CV_r_logusedshaders, 0);
  iConsole->Register("r_LogVBuffers", &CV_r_logVBuffers, 0, 0,
    "Logs vertex buffers in memory to 'LogVBuffers.txt'.\n"
    "Usage: r_LogVBuffers [0/1]\n"
    "Default is 0 (off).");
  iConsole->Register("r_LogTexStreaming", &CV_r_logTexStreaming, 0);
  iConsole->Register("r_SyncVBuf", &CV_r_syncvbuf, 1);
  iConsole->Register("r_Flush", &CV_r_flush, 1); // this causes the game to freeze (infinite loop) - do not use

  iConsole->Register("r_HWLights", &CV_r_hwlights, 1, VF_CHEAT,
    "Toggles hardware vertex lighting.\n"
    "Usage: r_HWLights [0/1]"
    "Default is 1 (on).");

  iConsole->Register("r_NoLoadTextures", &CV_r_noloadtextures, 0);
  iConsole->Register("r_NoPreprocess", &CV_r_nopreprocess, 0);
  iConsole->Register("r_NoPS20", &CV_r_nops20, 0, VF_REQUIRE_APP_RESTART);
  iConsole->Register("r_NoPS30", &CV_r_nops30, 0, 0);
  iConsole->Register("r_NoPS2X", &CV_r_nops2x, 0, 0);
  iConsole->Register("r_SM30PATH", &CV_r_sm30path, 1, 0);
  iConsole->Register("r_SM2XPATH", &CV_r_sm2xpath, 1, 0);
  iConsole->Register("r_OffsetBump", &CV_r_offsetbump, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("r_OffsetBumpForce", &CV_r_offsetbumpforce, 0, VF_REQUIRE_APP_RESTART);
  iConsole->Register("r_ShadersSave", &CV_r_shaderssave, 0, VF_DUMPTODISK);
  iConsole->Register("r_ShadersPrecache", &CV_r_shadersprecache, 1, VF_DUMPTODISK);
  iConsole->Register("r_PrecacheMesh", &CV_r_precachemesh, 1);

  iConsole->Register("r_SpecAntialias", &CV_r_specantialias, 0, 0,
    "Toggles specular antialiasing.\n"
    "Usage: r_SpecAntialias [0/1]\n"
    "Default is 0.");
  iConsole->Register("r_Shininess", &CV_r_shininess, 15.0f, 0,
    "Specular highlight modifier.\n"
    "Usage: r_Shininess 15.0\n"
    "Default is 15. This modifier changes the strength of the specular highlight.");
  iConsole->Register("r_WaveScale", &CV_r_wavescale, 1.0f, 0,
    "Wave scaling modifier for functions using the wave-table."
    "Usage: r_WaveScale 1.0\n"
    "Default is 1.0.");

#ifdef _DEBUG
  iConsole->Register("r_StripMesh", &CV_r_stripmesh, STRIPTYPE_NONE);
#else
  iConsole->Register("r_StripMesh", &CV_r_stripmesh, STRIPTYPE_ONLYLISTS);
#endif
  
  iConsole->Register("r_FullBrightness", &CV_r_fullbrightness, 0, VF_CHEAT,
    "Overrides diffuse light or color properties, by using white color instead.\n"
    "Usage: r_FullBrightness [0/1]\n"
    "Default is 0 (off).");

  iConsole->Register("r_SunStyleCoronas", &CV_r_SunStyleCoronas, 0);

  CV_r_excludeshader = iConsole->CreateVariable("r_ExcludeShader", "0", VF_CHEAT,
    "Exclude the named shader from the render list.\n"
    "Usage: r_ExcludeShader ShaderName\n"
    "Sometimes this is useful when debugging.");
  CV_r_showonlyshader = iConsole->CreateVariable("r_ShowOnlyShader", "0", VF_CHEAT,
    "Render only the named shader, ignoring all others.\n"
    "Usage: r_ShowOnlyShader ShaderName");
  iConsole->Register("r_LogLoadShaders", &CV_r_logloadshaders, 0, 0,
    "Enables logging of missing shaders.\n"
    "Usage: r_LogLoadShaders [0/1]\n"
    "Default is 0 (off).");
  iConsole->Register("r_ProfileShaders", &CV_r_profileshaders, 0, 0,
    "Enables display of render profiling information.\n"
    "Usage: r_ProfileShaders [0/1]\n"
    "Default is 0 (off). Set to 1 to display profiling\n"
    "of rendered shaders.");
  iConsole->Register("r_TexProcedures", &CV_r_texprocedures, 1, 0,
    "Enables rendering of procedural textures.\n"
    "Usage: r_TexProcedures [0/1]\n"
    "Default is 1 (on).");
  iConsole->Register("r_EnvLighting", &CV_r_envlighting, 0, VF_DUMPTODISK,
    "Use low-res. scanned env. cube-map for ambient lighting on characters.\n"
    "Usage: r_EnvLighting [0/1]\n"
    "Default is 0 (off).\n"
    "Shader should support this technique.\n");
  iConsole->Register("r_EnvLightCMSize", &CV_r_envlightcmsize, 8, VF_DUMPTODISK,
    "Size of cube-map for radiocity.\n"
    "Usage: r_EnvLightCMSize [Size]\n"
    "Default is 8 (8 pixels).\n");
  iConsole->Register("r_EnvLightCMDebug", &CV_r_envlightcmdebug, 0, 0,
    "Draw debug cube for env radiocity.\n"
    "Usage: r_EnvLightCMDebug [0/1]\n"
    "Default is 0 (off).\n");
  iConsole->Register("r_EnvCMWrite", &CV_r_envcmwrite, 0, 0,
    "Writes cube-map textures to disk.\n"
    "Usage: r_EnvCMWrite [0/1]\n"
    "Default is 0 (off). The textures are written to 'Cube_posx.jpg'\n"
    "'Cube_negx.jpg',...,'Cube_negz.jpg'. At least one of the real-time\n"
    "cube-map shaders should be present in the current scene.\n");
  iConsole->Register("r_EnvCMResolution", &CV_r_envcmresolution, 2, VF_DUMPTODISK,
    "Sets resolution for target environment cubemap, in pixels.\n"
    "Usage: r_EnvCMResolution #\n"
    "where # represents:\n"
    " 0: 64\n"
    " 1: 128\n"
    " 2: 256\n"
    "Default is 2 (256 by 256 pixels).");
  iConsole->Register("r_EnvTexResolution", &CV_r_envtexresolution, 3, VF_DUMPTODISK,
    "Sets resolution for 2d target environment texture, in pixels.\n"
    "Usage: r_EnvTexResolution #\n"
    "where # represents:\n"
    " 0: 64\n"
    " 1: 128\n"
    " 2: 256\n"
    " 3: 512\n"
    "Default is 3 (512 by 512 pixels).");
  iConsole->Register("r_WaterUpdateDistance", &CV_r_waterupdateDistance, 2.0f, 0,"");
  iConsole->Register("r_WaterUpdateDeltaAngle", &CV_r_waterupdateDeltaAngle, 5.0f, 0,
    "Delta angle for detecting of the water reflected texture updating.\n"
    "Usage: r_WaterUpdateDeltaAngle 5\n"
    "Default is 5.");
  iConsole->Register("r_WaterUpdateFactor", &CV_r_waterupdateFactor, 0.01f, VF_DUMPTODISK,
    "Distance factor for water reflected texture updating.\n"
    "Usage: r_WaterUpdateFactor 0.01\n"
    "Default is 0.01.");
  iConsole->Register("r_EnvLCMupdateInterval", &CV_r_envlcmupdateinterval, 0.1f, VF_DUMPTODISK,
    "Sets the interval between environmental cube map texture updates.\n"
    "Usage: r_EnvCMupdateInterval 0.1\n"
    "Default is 0.1.");
  iConsole->Register("r_EnvCMupdateInterval", &CV_r_envcmupdateinterval, 0.1f, VF_DUMPTODISK,
    "Sets the interval between environmental cube map texture updates.\n"
    "Usage: r_EnvCMupdateInterval 0.1\n"
    "Default is 0.1.");
  iConsole->Register("r_EnvTexUpdateInterval", &CV_r_envtexupdateinterval, 0.001f, VF_DUMPTODISK,
    "Sets the interval between environmental 2d texture updates.\n"
    "Usage: r_EnvTexUpdateInterval 0.001\n"
    "Default is 0.001.");
  iConsole->Register("r_WaterReflections", &CV_r_waterreflections, 1, VF_DUMPTODISK,
    "Toggles water reflections.\n"
    "Usage: r_WaterReflections [0/1]\n"
    "Default is 1 (water reflects).");
  iConsole->Register("r_WaterRefractions", &CV_r_waterrefractions, 0, VF_DUMPTODISK,
    "Toggles water refractions.\n"
    "Usage: r_WaterRefractions [0/1]\n"
    "Default is 1 (water refracts).");
  iConsole->Register("r_WaterBeachRefractions", &CV_r_waterbeachrefractions, 1, 0,
    "Toggles water beach refractions.\n"
    "Usage: r_WaterBeachRefractions [0/1]\n"
    "Default is 1 (water beach distorted).");
  iConsole->Register("r_SelfRefract", &CV_r_selfrefract, 0, 0,
    "Allows to draw refracted object to itself during refracted preprocess.\n"
    "Usage: r_SelfRefract [0/1]\n");

  iConsole->Register("r_AccurateParticles", &CV_r_accurateparticles, 1, 0,
    "Toggles more advanced (but slower) lighting calculations for particles.\n"
    "Usage: r_AccurateParticles [0/1]\n"
    "Default is 1 (on).");

  CV_r_shaderdefault  = iConsole->CreateVariable("r_ShaderDefault", "0",NULL,
    "Name of default shader.\n"
    "Usage: r_ShaderDefault shadername\n");

  CV_r_glossdefault  = iConsole->CreateVariable("r_GlossDefault", "Defaults/gloss",NULL,
    "Name of default gloss map.\n"
    "Usage: r_GlossDefault filename\n"
    "The texture 'defaults/gloss' is used by default. If you don’t specify a gloss map\n"
    "this is the texture that will be used.");
  CV_r_detaildefault  = iConsole->CreateVariable("r_DetailDefault", "Textures/Detail/rock",NULL,
    "Name of default detail texture.\n"
    "Usage: r_DetailDefault filename\n"
    "The texture 'Textures/Detail/rock' is used by default. If you don’t\n"
    "specify a detail texture, this is the texture that will be used.");
  CV_r_opacitydefault  = iConsole->CreateVariable("r_OpacityDefault", "Textures/white",NULL,
    "Name of default opacity mask.\n"
    "Usage: r_OpacityDefault filename\n"
    "The texture 'Textures/white' is used by default. If you don’t\n"
    "specify an opacity mask, this is the mask that will be used.");

  iConsole->Register("r_DetailTextures", &CV_r_detailtextures, 1, VF_DUMPTODISK,
    "Toggles detail texture overlays.\n"
    "Usage: r_DetailTextures [0/1]\n"
    "Default is 1 (detail textures on).");
  iConsole->Register("r_DecalTextures", &CV_r_decaltextures, 1, 0,
    "Toggles decal texture overlays.\n"
    "Usage: r_DecalTextures [0/1]\n"
    "Default is 1 (decal textures on).");

  iConsole->Register("r_ReloadShaders", &CV_r_reloadshaders, 0, 0,
    "Reloads shaders.\n"
    "Usage: r_ReloadShaders [0/1]\n"
    "Default is 0. Set to 1 to reload shaders.");

  iConsole->Register("r_DetailNumLayers", &CV_r_detailnumlayers, 1, VF_DUMPTODISK,
    "Sets the number of detail layers per surface.\n"
    "Usage: r_DetailNumLayers 1\n"
    "Default is 1.");
  iConsole->Register("r_DetailScale", &CV_r_detailscale, 8.0f, 0,
    "Sets the default scaling for detail overlays.\n"
    "Usage: r_DetailScale 8\n"
    "Default is 8. This scale applies only if the object's\n"
    "detail scale was not previously defined (in MAX).");
  iConsole->Register("r_DetailDistance", &CV_r_detaildistance, 4.0f, VF_DUMPTODISK,
    "Distance used for per-pixel detail layers blending.\n"
    "Usage: r_DetailDistance 4\n"
    "Default is 4.");
  
  iConsole->Register("r_UseHWShaders", &CV_r_usehwshaders, 2, VF_REQUIRE_APP_RESTART,
    "Enables loading of hardware dependent shaders.\n"
    "Usage: r_UseHWShaders [0/1]\n"
    "Default is 1 (HW shaders enabled).");
  iConsole->Register("r_NoLightCalc", &CV_r_nolightcalc, 0, 0,
    "Toggles software real-time light calculations.\n"
    "Usage: r_NoLightCalc [0/1]\n"
    "Default is 0 (software light calculations enabled). Set\n"
    "to 1 to disable real-time light calculations.");
  iConsole->Register("r_TexBindMode", &CV_r_texbindmode, 0, 0, "");
  iConsole->Register("r_NoDrawShaders", &CV_r_nodrawshaders, 0, 0,
    "Disable entire render pipeline.\n"
    "Usage: r_NoDrawShaders [0/1]\n"
    "Default is 0 (render pipeline enabled). Used for debugging and profiling.\n");
  iConsole->Register("r_NoDrawNear", &CV_r_nodrawnear, 0, 0,
    "Disable drawing of a near objects.\n"
    "Usage: r_NoDrawNear [0/1]\n"
    "Default is 0 (near objects are drawn).");

  iConsole->Register("r_Flares", &CV_r_flares, 1, VF_DUMPTODISK,
    "Toggles sunlight lens flare effect.\n"
    "Usage: r_Flares [0/1]\n"
    "Default is 1 (on).");
  iConsole->Register("r_ProcFlares", &CV_r_procflares, 1, VF_DUMPTODISK,
    "Toggles procedural flares effect.\n"
    "Usage: r_ProcFlares [0/1]\n"
    "Default is 1 (on).");
  iConsole->Register("r_Beams", &CV_r_beams, 1, VF_DUMPTODISK,
    "Toggles procedural beams effect.\n"
    "Usage: r_Beams [0/1]\n"
    "Default is 1 (on).");
  iConsole->Register("r_FlareSize", &CV_r_flaresize, 1.0f, 0,"");
  iConsole->Register("r_Gamma", &CV_r_gamma, 1.0f, VF_DUMPTODISK,
    "Sets gamma level.\n"
    "Usage: r_Gamma 1.0\n"
    "Default is 1.0.");
  iConsole->Register("r_Brightness", &CV_r_brightness, 0.5f, VF_DUMPTODISK,
    "Sets the diplay brightness.\n"
    "Usage: r_Brightness 0.5\n"
    "Default is 0.5.");
  iConsole->Register("r_Contrast", &CV_r_contrast, 0.5f, VF_DUMPTODISK,
    "Sets the diplay contrast.\n"
    "Usage: r_Contrast 0.5\n"
    "Default is 0.5.");
  iConsole->Register("r_NoHWGamma", &CV_r_nohwgamma, 0, 0,
    "Sets renderer to ignore hardware gamma correction.\n"
    "Usage: r_NoHWGamma [0/1]\n"
    "Default is 0 (allow hardware gamma correction).");
  iConsole->Register("r_NoSWGamma", &CV_r_noswgamma, 1, 0,
    "Sets renderer to ignore software gamma correction.\n"
    "Usage: r_NoSWGamma [0/1]\n"
    "Default is 0 (allow software gamma correction).");

  iConsole->Register("r_checkSunVis", &CV_r_checkSunVis, 1, VF_DUMPTODISK,
    "Sets the algorithm for sun checking and drawing.\n"
    "Usage: r_checkSunVis #\n"
    "where # represents\n"
    " 1: Simple algorithm of visibility checking by reading pixel from Z-buffer.\n"
    " 2:  Slower but impressive method of blurring sun texture outwards, using\n"
    "     the original visibility mask from the Z-buffer.\n"
    " 3:  Checking of average visibility of the sun, without blurring, using\n"
    "     Z-buffer mask and bilinear filter to draw this as a single pixel.");
  iConsole->Register("r_EMBM", &CV_r_embm, 1.0f, 0,
    "Environment Bump-mapping matrix scaling.\n"
    "Usage: r_EMBM 1.0"
    "Default is 1 (unscaled). Used to scale the bump mapping amount for embm\n"
    "shaders, such as water, ocean, glass and others.");
  iConsole->Register("r_SSE", &CV_r_sse, 1, 0,
    "Enables additional SSE instructions for advanced processors.\n"
    "Usage: r_SSE [0/1]"
    "Default is 1 (on). Disabling this can slow down some calculations\n"
    "but is useful for debugging.");
  iConsole->Register("r_Scissor", &CV_r_scissor, 1, 0, "Enables scissor test");
  iConsole->Register("r_Coronas", &CV_r_coronas, 1, VF_DUMPTODISK,
    "Toggles light coronas around light sources.\n"
    "Usage: r_Coronas [0/1]"
    "Default is 1 (on).");
  iConsole->Register("r_CoronaFade", &CV_r_coronafade, 0.125f, VF_DUMPTODISK,
    "Time fading factor of the light coronas.\n"
    "Usage: r_CoronaFade 0.5"
    "Default is 0.5.");
  iConsole->Register("r_CoronaSizeScale", &CV_r_coronasizescale, 1.0f, VF_DUMPTODISK );
  iConsole->Register("r_CoronaColorScale", &CV_r_coronacolorscale, 1.0f, 0,"");

  iConsole->Register("r_CullByClipPlanes", &CV_r_cullbyclipplanes, 1);

  iConsole->Register("r_OceanHeightScale", &CV_r_oceanheightscale, 4) ;
  iConsole->Register("r_OceanSectorSize", &CV_r_oceansectorsize, 128) ;
  iConsole->Register("r_OceanRendType", &CV_r_oceanrendtype, 0) ;
  iConsole->Register("r_OceanLodDist", &CV_r_oceanloddist, 180) ;
  iConsole->Register("r_OceanTexUpdate", &CV_r_oceantexupdate, 1) ;
  iConsole->Register("r_OceanMaxSplashes", &CV_r_oceanmaxsplashes, 8) ;
  iConsole->Register("r_OceanSplashScale", &CV_r_oceansplashscale, 1.0f) ;

  iConsole->Register("r_EfMultiTex", &CV_r_efmultitex, 1);
  iConsole->Register("r_NoParticles", &CV_r_noparticles, 0);

  iConsole->Register("CV_ind_VisualizeShadowVolumes", &CV_ind_VisualizeShadowVolumes, 0);
  iConsole->Register("CV_ind_ReverseShadowVolumes", &CV_ind_ReverseShadowVolumes, 0);
  iConsole->Register("CV_ind_DrawBorderEdges", &CV_ind_DrawBorderEdges, 0);

  iConsole->Register("r_PolygonMode", &CV_r_PolygonMode, 1, VF_CHEAT);
  iConsole->Register("r_GetScreenShot", &CV_r_GetScreenShot, 0);
  iConsole->Register("r_VolumetricFog", &CV_r_VolumetricFog, 2, VF_DUMPTODISK );
  iConsole->Register("r_VPFog", &CV_r_vpfog, 1,VF_CHEAT);

  iConsole->Register("r_Character_LOD_Bias", &CV_r_character_lod_bias, 0.5f);
  iConsole->Register("r_Character_NoDeform", &CV_r_character_nodeform, 0);
  iConsole->Register("r_Character_Debug", &CV_r_character_debug, 0);
  iConsole->Register("r_Character_NoAnim", &CV_r_character_noanim, 0);
  iConsole->Register("r_shadow_maps_debug", &CV_r_shadow_maps_debug, 0);  
  iConsole->Register("r_draw_phys_only", &CV_r_draw_phys_only, 0);
  iConsole->Register("r_Character_Shadow_Volume", &CV_r_character_shadow_volume, 0);
  iConsole->Register("r_Character_NoPhys", &CV_r_character_nophys, 0);

  iConsole->Register("r_DisplayInfo", &CV_r_DisplayInfo, 0);
  iConsole->Register("r_Log", &CV_r_log, 0, 0,
    "Logs rendering information to Direct3DLog.txt.\n"
    "Usage: r_Log [0/1/2/3/4]\n"
    " 1: Logs a list of all shaders without profile info.\n"
    " 2: Log contains a list of all shaders with profile info.\n"
    " 3: Logs all API function calls.\n"
    " 4: Highly detailed pipeline log, including all passes,\n"
    "     states, lights and pixel/vertex shaders.\n"
    "Default is 0 (off). Use this function carefully, because\n"
    "log files grow very quickly.");
  iConsole->Register("r_Stats", &CV_r_stats, 0, 0,
    "Toggles render statistics.\n"
    "Usage: r_Stats [0/1/2/3/4/5]\n");
  iConsole->Register("r_VSync", &CV_r_vsync, 0, VF_DUMPTODISK,
    "Toggles vertical sync.\n"
    "Usage: r_VSync [0/1]\n");
  iConsole->Register("r_MeasureOverdraw", &CV_r_measureoverdraw, 0);
  iConsole->Register("r_PrintMemoryLeaks", &CV_r_printmemoryleaks, 0);

  iConsole->Register("r_ReplaceCubemap",&CV_r_ReplaceCubeMap,1);
  iConsole->Register("r_VegetationSpritesAlphaBlend",&CV_r_VegetationSpritesAlphaBlend,0);
  iConsole->Register("r_VegetationSpritesNoBend",&CV_r_VegetationSpritesNoBend,2);
  iConsole->Register("r_Vegetation_IgnoreVertColors",&CV_r_Vegetation_IgnoreVertColors,0);
  iConsole->Register("r_Vegetation_PerpixelLight",&CV_r_Vegetation_PerpixelLight,1,VF_DUMPTODISK | VF_REQUIRE_APP_RESTART);

  iConsole->Register( "r_Quality_BumpMapping", &CV_r_Quality_BumpMapping, 2, VF_DUMPTODISK | VF_REQUIRE_APP_RESTART);
  iConsole->Register( "r_Quality_Reflection", &CV_r_Quality_Reflection, 0, VF_DUMPTODISK );

  iConsole->Register("r_ShowVideoMemoryStats", &CV_r_ShowVideoMemoryStats, 0);

  m_WorldColor = CFColor(0.5f, 0.5f, 0.5f, 1.0f);
  m_vClearColor = Vec3d(0,0,0);
  m_LogFile = NULL;
  m_nShadowVolumePolys=0;
  m_TexGenID = 1;
  m_VSync = -1;
  m_Features = 0;

  CName::mfInitSubsystem();
  init_math();
  
  m_nFrameID = (unsigned short)-2;

  // tiago added
  m_bRefraction=0;
  m_bPauseTimer=0;
  m_fPrevTime=-1.0f;

  m_CurFontColor = Col_White;
  m_fXFontScale = 0.5f;
  m_fYFontScale = 1.0f;

  m_FontColorTable[0] = CFColor(0, 0, 0);
  m_FontColorTable[1] = CFColor(1, 1, 1);
  m_FontColorTable[2] = CFColor(1, 0, 0);
  m_FontColorTable[3] = CFColor(0, 1, 0);
  m_FontColorTable[4] = CFColor(0, 0, 1);
  m_FontColorTable[5] = CFColor(1, 1, 0);
  m_FontColorTable[6] = CFColor(0, 1, 1);
  m_FontColorTable[7] = CFColor(1, 0, 1);
  m_FontColorTable[8] = CFColor(0, 0.5f, 1);

  m_bSwapBuffers = true;
  m_FS.m_bEnable = true;

#if defined(_DEBUG) && !defined(LINUX)
#ifndef _XBOX
  if (CV_r_printmemoryleaks)
  {
    //_controlfp( _EM_INEXACT,_MCW_EM );
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  }
#endif
#endif

  m_nGlobalShaderTemplateId = -2; // not set
}

CRenderer::~CRenderer()
{
  if (m_TexMan)
  {
    delete m_TexMan;
    m_TexMan = NULL;
  }
  gRenDev = NULL;
}

void CRenderer::Release()
{
  delete this;
}

//////////////////////////////////////////////////////////////////////
/*bool CRenderer::FindImage(CImage *image)
{
  for (ImageIt i=m_imageList.begin();i!=m_imageList.end();i++)
  {
    CImage *ci=(*i);

    if (ci==image) 
      return (true);
  } //i

  return (false);
} */

//////////////////////////////////////////////////////////////////////
/*CImage *CRenderer::FindImage(const char *filename)
{

  ImageIt istart=m_imageList.begin();
  ImageIt iend=m_imageList.end();

  for (ImageIt i=m_imageList.begin();i!=iend;i++)
  {
    CImage *ci=(*i);

    if (stricmp(ci->GetName(),filename)==0) 
      return (ci);
  } //i

  return (NULL);
} */

//////////////////////////////////////////////////////////////////////
/*void CRenderer::AddImage(CImage *image)
{
  m_imageList.push_back(image);
} */

//////////////////////////////////////////////////////////////////////
//void CRenderer::ShowFps(const char *command/* =NULL */)
/*{
  if (!command) 
    return;
  if (stricmp(command,"true")==0) 
    m_showfps=true;
  else
  if (stricmp(command,"false")==0) 
    m_showfps=false;  
  else
    iConsole->Help("ShowFps");
} */

//////////////////////////////////////////////////////////////////////
void CRenderer::TextToScreenColor(int x, int y, float r, float g, float b, float a, const char * format, ...)
{
//  if(!cVars->e_text_info)
  //  return;

  char buffer[512];
  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

  WriteXY(iConsole->GetFont(),
    (int)(0.01f*800*x), (int)(0.01f*600*y), 0.5f, 1, 
    r,g,b,a, buffer);
}

//////////////////////////////////////////////////////////////////////
void CRenderer::TextToScreen(float x, float y, const char * format, ...)
{
//  if(!cVars->e_text_info)
  //  return;

  char buffer[512];
  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

  WriteXY(iConsole->GetFont(),
    (int)(0.01f*800*x), (int)(0.01f*600*y), 0.5f, 1, 
    1,1,1,1, buffer);
}

//////////////////////////////////////////////////////////////////////////
void CRenderer::Draw2dText( float posX,float posY,const char *szText,SDrawTextInfo &ti )
{
  // Check for the presence of a D3D device
  if (!iSystem)
    return;

  ICryFont *pCryFont = iSystem->GetICryFont();
  if (!pCryFont)
    return;

  IFFont *pFont = pCryFont->GetFont("Default");
  if (!pFont)
    return;

  float r = CLAMP( ti.color[0], 0.0f, 1.0f);
  float g = CLAMP( ti.color[1], 0.0f, 1.0f);
  float b = CLAMP( ti.color[2], 0.0f, 1.0f);
  float a = CLAMP( ti.color[3], 0.0f, 1.0f);

  pFont->SetColor(color4f(r,g,b,a));
  pFont->SetCharWidthScale(1);

  if (ti.flags & eDrawText_FixedSize)
  {
    pFont->UseRealPixels(true);
    pFont->SetSize( vector2f(12*ti.xscale,12*ti.yscale) );
    pFont->SetSameSize(false);
    posX = ScaleCoordX(posX);
    posY = ScaleCoordY(posY);
  }
  else
  {
    //    pFont->SetSameSize(true);
    //  pFont->SetSize( vector2f(16*ti.xscale,12*ti.yscale) );

    // vlad: changed to make text at least somehow visible
    pFont->UseRealPixels(false);
    pFont->SetSameSize(true);
    pFont->SetCharWidthScale(2.0f / 3.0f);
    pFont->SetSize(vector2f(15,15));
  }

  pFont->SetColor(color4f(r,g,b,a));
  //pFont->SetEffect("buttonfocus");
  //pFont->SetCharWidthScale(2.0f/3.0f);
  pFont->DrawString( posX,posY,szText);
}

void CRenderer::PrintToScreen(float x, float y, float size, const char *buf)
{
  SDrawTextInfo ti;
  ti.xscale = size*0.5f/8;
  ti.yscale = size*1.f/8;
  ti.color[0] = 1; ti.color[1] = 1; ti.color[2] = 1; ti.color[3] = 1;
  ti.xfont = iConsole->GetFont();
  Draw2dText( x,y,buf,ti );
}

void CRenderer::WriteXY(CXFont *currfont, int x, int y, float xscale, float yscale, float r, float g, float b, float a, const char *message, ...)
{
  //////////////////////////////////////////////////////////////////////
  // Write a string to the screen
  //////////////////////////////////////////////////////////////////////

  va_list ArgList;
  char szStringBuffer[4096];

  // Check for the presence of a D3D device
  // Format the string
  va_start(ArgList, message);
  vsprintf(szStringBuffer, message, ArgList);
  va_end(ArgList);

  SDrawTextInfo ti;
  ti.xscale = xscale;
  ti.yscale = yscale;
  ti.color[0] = r;
  ti.color[1] = g;
  ti.color[2] = b;
  ti.color[3] = a;
  ti.xfont = currfont;
  Draw2dText( (float)x,(float)y,szStringBuffer,ti );
}



//////////////////////////////////////////////////////////////////////
void CRenderer::DrawLabel(Vec3d pos, float font_size, const char * label_text, ...)
{
  if (label_text && m_listMessages.Count()<1000)
  {
    char buffer[512];
    va_list args;
    va_start(args, label_text);
    vsprintf(buffer, label_text, args);
    va_end(args);

    text_info_struct m;
    strcpy(m.mess,buffer);
    m.pos = pos;
    m.font_size = font_size;

    m.fColor[0] = m.fColor[1] = m.fColor[2] = m.fColor[3] = 1;
    m.bFixedSize = false;
    m.bCenter = false;
    m.b2D = false;

    m.nTextureId=-1;

    m_listMessages.Add(m);
  }
}

//////////////////////////////////////////////////////////////////////
void CRenderer::DrawLabelEx(Vec3d pos, float font_size, float * pfColor, bool bFixedSize, bool bCenter, const char * label_text, ...)
{
  if(m_listMessages.Count()<1000)
  {
    char buffer[512];
    va_list args;
    va_start(args, label_text);
    vsprintf(buffer, label_text, args);
    va_end(args);

    text_info_struct m;
    strncpy(m.mess,buffer,sizeof(m.mess));
    m.pos = pos;
    m.font_size = font_size;

    memcpy(m.fColor, pfColor, sizeof(m.fColor));
    m.bFixedSize = bFixedSize;
    m.bCenter = bCenter;
    m.b2D = false;

    m.nTextureId=-1;

    m_listMessages.Add(m);
  }
}

//////////////////////////////////////////////////////////////////////
void CRenderer::Draw2dLabel( float x,float y, float font_size, float * pfColor,bool bCenter, const char * label_text, ...)
{
  if (m_listMessages.Count()<1000)
  {
    char buffer[512];
    va_list args;
    va_start(args, label_text);
    vsprintf(buffer, label_text, args);
    va_end(args);

    text_info_struct m;
    strcpy(m.mess,buffer);
    m.pos.x = x;
    m.pos.y = y;
    m.pos.z = 0.5f;
    m.font_size = font_size;

    memcpy(m.fColor, pfColor, sizeof(m.fColor));
    m.bFixedSize = true;
    m.bCenter = bCenter;
    m.b2D = true;

    m.nTextureId=-1;

    m_listMessages.Add(m);
  }
}

//////////////////////////////////////////////////////////////////////
void CRenderer::DrawLabelImage(const Vec3d &vPos,float fImageSize,int nTextureId)
{
  if (m_listMessages.Count()<1000)
  {
    text_info_struct m;    

    memset(m.mess,0,32);
    m.pos = vPos;
    m.font_size = fImageSize;

    m.fColor[0] = m.fColor[1] = m.fColor[2] = m.fColor[3] = 1;
    m.bFixedSize = false;
    m.bCenter = false;
    m.b2D = false;

    m.nTextureId=nTextureId;

    m_listMessages.Add(m);
  }
}

//////////////////////////////////////////////////////////////////////
void CRenderer::FlushTextMessages()
{
  EnableFog(false);
  int vx,vy,vw,vh;
  GetViewport( &vx,&vy,&vw,&vh );

  for(int i=0; i<m_listMessages.Count(); i++)
  {
    text_info_struct * pTextInfo = &m_listMessages[i];

    float sx,sy,sz;

    if (!pTextInfo->b2D)
    {
      float fDist = GetDistance(pTextInfo->pos,GetCamera().GetPos());
      
      float K = GetCamera().GetZMax()/fDist;
      if(fDist>GetCamera().GetZMax()*0.5)
        pTextInfo->pos = GetCamera().GetPos() + K*(pTextInfo->pos - GetCamera().GetPos());

      ProjectToScreen( pTextInfo->pos.x, pTextInfo->pos.y, pTextInfo->pos.z, 
        &sx, &sy, &sz );
    }
    else
    {
      sx = (pTextInfo->pos.x) / vw * 100;
      sy = (pTextInfo->pos.y) / vh * 100;
      sz = pTextInfo->pos.z;
    }

    if(sx>0 && sx<100)
    if(sy>0 && sy<100)
    if(sz>0 && sz<1)
    {
      // calculate size
      float sizeX;
      float sizeY;
      if (pTextInfo->bFixedSize)
      {
        sizeX = pTextInfo->font_size;
        sizeY = pTextInfo->font_size;
        //sizeX = pTextInfo->font_size * 800.0f/vw;
        //sizeY = pTextInfo->font_size * 500.0f/vh;
      }
      else
      {
        sizeX = sizeY = (1.0f-(float)(sz))*32.f*pTextInfo->font_size;
        sizeX *= 0.5f;
      }

      // center
      if (pTextInfo->bCenter && pTextInfo->mess[0])
      {
        float len = (float)strlen(pTextInfo->mess);      
        CXFont *cf=iConsole->GetFont();
        if(cf)
        {
          sx-=(len/2*cf->m_charsize*0.5f*sizeX/800*100);///100.0f;    
          sy-=(      cf->m_charsize*0.5f*sizeY/600*100);///100.0f;    
        }
      }
      /*
      else 
      {       
        //DrawBall(pTextInfo->pos, 0.025f);
      }
      */

      if (pTextInfo->mess[0])
      {
        // print
        SDrawTextInfo ti;
        ti.color[0] = pTextInfo->fColor[0];
        ti.color[1] = pTextInfo->fColor[1];
        ti.color[2] = pTextInfo->fColor[2];
        ti.color[3] = pTextInfo->fColor[3];
        ti.xfont = iConsole->GetFont();
        if (pTextInfo->bFixedSize)
          ti.flags |= eDrawText_FixedSize;
        ti.xscale = sizeX;
        ti.yscale = sizeY;
        Draw2dText( 0.01f*800*sx,0.01f*600*sy,pTextInfo->mess,ti );
        //[Timur] 
        /*
        WriteXY(iConsole->GetFont(),
          (int)(0.01f*vw*sx),(int)(0.01f*vh*sy), sizeX, 1.f*sizeY,
          pTextInfo->fColor[0],
          pTextInfo->fColor[1],
          pTextInfo->fColor[2],
          pTextInfo->fColor[3],
          pTextInfo->mess);
        */
      }     

      if (pTextInfo->nTextureId>=0)
      {
        SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
        SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
        SetTexture(pTextInfo->nTextureId);        

        Matrix44  mat;
        GetModelViewMatrix(mat.GetData());
        Vec3d   vRight,vUp,vFront;  

        //CELL_CHANGED_BY_IVO
        //vRight(mat.cell(0), mat.cell(4), mat.cell(8));
        //vUp   (mat.cell(1), mat.cell(5), mat.cell(9));        
        vRight( mat(0,0), mat(1,0), mat(2,0) );
        vUp   ( mat(0,1), mat(1,1), mat(2,1) );         

        //DrawQuad(vRight,vUp,pTextInfo->pos,true);   
        DrawQuad(vRight,vUp,pTextInfo->pos,2);    
        SetState(GS_DEPTHWRITE);
      }
    }
  }

  m_listMessages.Clear();
}

ShadowMapFrustum * CRenderer::MakeShadowMapFrustum(ShadowMapFrustum * lof, ShadowMapLightSource * pLs, const Vec3d & obj_pos, list2<IStatObj*> * pStatObjects, int shadow_type)
{  
//  soft2d = cVars->e_2dshadows>0;

//  memset(lof,0,sizeof(ShadowMapFrustum));

//  Vec3d vCenter = ((*pStatObjects)[0]->GetBoxMin() + (*pStatObjects)[0]->GetBoxMax())*0.5;
  
  lof->target = obj_pos;// + vCenter;
  Vec3d dir = lof->target - pLs->vSrcPos;

  float distance = dir.Length();
//  if(distance > pLs->radius+pModel->radius)
  //  return 0; // too far

//  dir.Normalize();

  float radius = 1;
  if(pStatObjects && pStatObjects->Count())
    radius = (*pStatObjects)[0]->GetRadius();

  lof->FOV = RAD2DEG(radius/distance)*1.8f;

  if(lof->FOV>90)
    lof->FOV=90;

  lof->min_dist = distance - radius;
  if(lof->min_dist<0.1f)
    lof->min_dist=0.1f;
  
  lof->max_dist = distance + radius;

  lof->pLs = pLs;
  lof->shadow_type = (EShadowType)shadow_type;

  if(pStatObjects)
  {
    if(pStatObjects->Count() && !lof->pModelsList)  
    {
      assert(0);
      lof->pModelsList = new list2<IStatObj*>;
    }

    for(int o=0; o<pStatObjects->Count(); o++)
    {
      IStatObj* pStatObj = (*pStatObjects)[o];
      lof->pModelsList->Add(pStatObj);
    }
  }

  return lof;
}

void CRenderer::GetViewport(int *x, int *y, int *width, int *height)
{
  *x = m_VX;
  *y = m_VY;
  *width = m_VWidth;
  *height = m_VHeight;
}

#pragma pack (push)
#pragma pack (1)
typedef struct
{
  unsigned char  id_length, colormap_type, image_type;
  unsigned short colormap_index, colormap_length;
  unsigned char  colormap_size;
  unsigned short x_origin, y_origin, width, height;
  unsigned char  pixel_size, attributes;
} TargaHeader_t;
#pragma pack (pop)

bool  CRenderer::SaveTga(unsigned char *sourcedata,int sourceformat,int w,int h,const char *filename,bool flip)
{
  //assert(0);
//  return CImage::SaveTga(sourcedata,sourceformat,w,h,filename,flip);
  
  if (flip)
  {
    int size=w*(sourceformat/8);
    unsigned char *tempw=new unsigned char [size];
    unsigned char *src1=sourcedata;   
    unsigned char *src2=sourcedata+(w*(sourceformat/8))*(h-1);
    for (int k=0;k<h/2;k++)
    {
      memcpy(tempw,src1,size);
      memcpy(src1,src2,size);
      memcpy(src2,tempw,size);
      src1+=size;
      src2-=size;
    }
    delete [] tempw;
  }
  

  unsigned char *oldsourcedata=sourcedata;

  if (sourceformat==FORMAT_8_BIT)
  {

    unsigned char *desttemp=new unsigned char [w*h*3];
    memset(desttemp,0,w*h*3);

    unsigned char *destptr=desttemp;
    unsigned char *srcptr=sourcedata;

    unsigned char col;

    for (int k=0;k<w*h;k++)
    {     
      col=*srcptr++;
      *destptr++=col;   
      *destptr++=col; 
      *destptr++=col;   
    }
    
    sourcedata=desttemp;

    sourceformat=FORMAT_24_BIT;
  }

  TargaHeader_t header;

  memset(&header, 0, sizeof(header));
  header.image_type = 2;
  header.width = w;
  header.height = h;
  header.pixel_size = sourceformat;
    
  unsigned char *data = new unsigned char[w*h*(sourceformat>>3)];
  unsigned char *dest = data;
  unsigned char *source = sourcedata;

  //memcpy(dest,source,w*h*(sourceformat>>3));
  
  for (int ax = 0; ax < h; ax++)
  {
    for (int by = 0; by < w; by++)
    {
      unsigned char r, g, b, a;
      r = *source; source++;
      g = *source; source++;
      b = *source; source++;
      if (sourceformat==FORMAT_32_BIT) 
      {
        a = *source; source++;
      }       
      *dest = b; dest++;
      *dest = g; dest++;
      *dest = r; dest++;
      if (sourceformat==FORMAT_32_BIT) 
      {
        *dest = a; dest++;
      }
    }
  }
  

  FILE *f = fxopen(filename,"wb");
  if (!f)
  {   
    //("Cannot save %s\n",filename);
    delete [] data;
    return (false);
  }

  if (!fwrite(&header, sizeof(header),1,f))
  {   
    //CLog::LogToFile("Cannot save %s\n",filename);
    delete [] data;
    fclose(f);
    return (false);
  }

  if (!fwrite(data, w*h*(sourceformat>>3),1,f))
  {   
    //CLog::LogToFile("Cannot save %s\n",filename);
    delete [] data;
    fclose(f);
    return (false);
  }

  fclose(f);  

  delete [] data;
  if (sourcedata!=oldsourcedata)
    delete [] sourcedata;

  return (true);    
}

//================================================================


/*
int CRenderer::LoadAnimatedTexture(const char * filename, int * tex_type)
{
  char name_begin[1024];
  strcpy(name_begin,filename);
  name_begin[strlen(name_begin)-4]=0;

  while(name_begin[strlen(name_begin)-1]=='0')
    name_begin[strlen(name_begin)-1]=0;

  // count files
  for(int a=0; a<MAX_ANIM_TEX_NUM; a++)
  { 
    char fname[1024];
    sprintf(fname, "%s%0004d.jpg",name_begin, a);
    FILE * f = fxopen(fname, "rb");
    if(f)
      fclose(f);
    else
    { // if no jpg found - try cct
      sprintf(fname, "%s%0004d.cct",name_begin, a);
      f = fxopen(fname, "rb");
      if(f)
        fclose(f);
      else
        break;
    }
  }

  int file_count = a;

  for( a=0; a<file_count; a++)
  { // count files
    char fname[1024];
    sprintf(fname, "%s%0004d.jpg", name_begin, a);
    p_bind_ids[a] = MakeTexture(fname, tex_type); // animated
  }

  anim_tex_count = file_count;
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

//#include "../Common/Character/CryModel.h"

void CRenderer::FreeResources(int nFlags)
{
  iLog->Log("*** Clearing render resources ***");

#if defined(LINUX)
	NotifySystemOnQuit();//tell linux that we are about to quit, on some situation it crashed and this will force a abort call in case of a crash
#endif

  int i;

  if (nFlags & FRR_SHADERS)
    gRenDev->m_cEF.mfClearAll();
  if (nFlags & FRR_TEXTURES)
  {
    if (m_TexMan)
      m_TexMan->ClearAll(nFlags);

    // clear anim texture info
    for(int i=0; i<m_LoadedAnimatedTextures.Count(); i++)
    {
      delete [] m_LoadedAnimatedTextures[i]->pBindIds;
      delete m_LoadedAnimatedTextures[i];
    }
    m_LoadedAnimatedTextures.Reset();
  }
  if (nFlags & FRR_SYSTEM)
  {
    SAFE_DELETE(m_TexMan);
    for (i=0; i<m_RP.m_Objects.GetSize(); i++)
    {
      CCObject *obj = m_RP.m_Objects[i];
      if (!obj)
        continue;
      delete obj;
      m_RP.m_Objects[i] = NULL;
    }
    m_RP.m_Objects.Free();
    SAFE_DELETE_ARRAY(m_RP.m_ObjectsPool);
    for (i=0; i<m_RP.m_TempObjects.GetSize(); i++)
    {
      CCObject *obj = m_RP.m_TempObjects[i];
      if (!obj)
        continue;
      if (i >= m_RP.m_nNumObjectsInPool)
        delete obj;
      m_RP.m_TempObjects[i] = NULL;
    }
    m_RP.m_TempObjects.Free();
    EF_PipelineShutdown();
  }
  if ((nFlags & FRR_RESTORE) && !(nFlags & FRR_SYSTEM))
  {
    gRenDev->m_cEF.mfInit();
  }
  if (nFlags & FRR_REINITHW)
  {
#if !defined(PS2) && !defined (GC) && !defined (NULL_RENDERER) 
    for (int i=0; i<CPShader::m_PShaders.Num(); i++)
    {
      CPShader *ps = CPShader::m_PShaders[i];
      if (!ps)
        continue;
      ps->mfReset();
    }
    for (int i=0; i<CVProgram::m_VPrograms.Num(); i++)
    {
      CVProgram *vp = CVProgram::m_VPrograms[i];
      if (!vp)
        continue;
      vp->mfReset();
    }
#endif
  }
}

/*
CryModel *CRenderer::CreateModel()
{
  return new CryModel();
}

void CRenderer::DeleteModel(CryModel * pCryModel)
{
  delete pCryModel;
} 

void CRenderer::DeleteModelState(CryModelState * pCryModelState)
{
  delete pCryModelState;
}*/

void CRenderer::WriteTGA(byte *dat, int wdt, int hgt, const char *name, int bits)
{
  ::WriteTGA((byte*)dat, wdt, hgt, (char*)name, 24);
}

const char *sourceFile;
unsigned int sourceLine;


void CRenderer::WriteDDS(byte *dat, int wdt, int hgt, int Size, const char *nam, EImFormat eF, int NumMips)
{
  byte *data = NULL;
  if (Size == 3)
  {
    data = new byte[wdt*hgt*4];
    for (int i=0;  i<wdt*hgt; i++)
    {
      data[i*4+0] = dat[i*3+0];
      data[i*4+1] = dat[i*3+1];
      data[i*4+2] = dat[i*3+2];
      data[i*4+3] = 255;
    }
    dat = data;
  }
  char name[256];
  StripExtension(nam, name);
  strcat(name, ".dds");

  bool bMips = false;
  if (NumMips != 1)
    bMips = true;
  STexPic ti;
  ti.m_Width = wdt;
  ti.m_Height = hgt;
  int nMips;
  int DxtSize;
  byte *dst = m_TexMan->GenerateDXT_HW(&ti, eF, dat, &nMips, &DxtSize, bMips);
  if (dst)
  {
    ::WriteDDS(dst,wdt,hgt,DxtSize,name,eF,nMips);
    delete [] dst;
  }
  if (data)
    delete [] data;
}

IShader *CRenderer::EF_LoadShader (const char *name, EShClass Class, int flags, uint64 nMaskGen)
{
#ifdef NULL_RENDERER
  return m_cEF.m_DefaultShader;
#else
  return gRenDev->m_cEF.mfForName(name, Class, flags, NULL, nMaskGen);
#endif
}

SShaderItem CRenderer::EF_LoadShaderItem (const char *name, EShClass Class, bool bShare, const char *templName, int flags, SInputShaderResources *Res, uint64 nMaskGen)
{
#ifdef NULL_RENDERER
  return m_cEF.m_DefaultShaderItem;
#else
  return gRenDev->m_cEF.mfShaderItemForName(name, Class, bShare, templName, flags, Res, nMaskGen);
#endif
}

bool CRenderer::EF_ReloadFile (const char *szFileName)
{
  char nmf[512];
  char drn[512];
  char drv[16];
  char dirn[512];
  char fln[128];
  char extn[16];
  _splitpath(szFileName, drv, dirn, fln, extn);
  strcpy(drn, drv);
  strcat(drn, dirn);
  strcpy(nmf, fln);
  strcat(nmf, extn);
  if (!strnicmp(extn, ".csl", 4) || !stricmp(extn, ".csi") || !stricmp(extn, ".crycg") || !stricmp(extn, ".cryvp")  || !stricmp(extn, ".cryps"))
    return gRenDev->m_cEF.mfReloadFile(drn, nmf, FRO_SHADERS);
  if (!stricmp(extn, ".tga") || !stricmp(extn, ".pcx") || !stricmp(extn, ".dds") || !stricmp(extn, ".jpg") || !stricmp(extn, ".jpeg") || !stricmp(extn, ".gif") || !stricmp(extn, ".bmp"))
    return m_TexMan->ReloadFile(szFileName, FRO_TEXTURES);
  return false;
}

void CRenderer::EF_ReloadShaderFiles (int nCategory)
{
  gRenDev->m_cEF.mfLoadFromFiles(nCategory);
}

void CRenderer::EF_ReloadTextures ()
{
  m_TexMan->ReloadTextures();
}

bool CRenderer::EF_ScanEnvironmentCM (const char *name, int size, Vec3d& Pos)
{
  return m_TexMan->ScanEnvironmentCM(name, size, Pos);
}

int CRenderer::EF_LoadLightmap (const char *name)
{
  STexPic *tp = (STexPic *)EF_LoadTexture(name, FT_CLAMP | FT_NOMIPS, 0, eTT_Base);
  if (tp->IsTextureLoaded())
    return tp->m_Id;
  else
    return -1;
}

ITexPic *CRenderer::EF_GetTextureByID(int Id)
{
  if (Id >= TX_FIRSTBIND)
  {
    STexPic *tp = m_TexMan->GetByID(Id);
    if (tp)
      return tp;
  }
  return m_TexMan->m_Textures[Id];
}

ITexPic *CRenderer::EF_LoadTexture(const char* nameTex, uint flags, uint flags2, byte eTT, float fAmount1, float fAmount2, int Id, int BindId)  
{
  if (m_type == R_NULL_RENDERER && m_TexMan->m_Text_NoTexture)
    return m_TexMan->m_Text_NoTexture;
  else
  {
    STexPic *tx = NULL;
    bool bValid = true;

    if (!nameTex || !nameTex[0])
    {
      nameTex = "<DEFAULT>";
      bValid = false;
    }

    bool bAbsolute = false;
    if (!strnicmp(nameTex, "Textures", 8) || !strnicmp(nameTex, "Objects", 7) || nameTex[1] == ':')
      bAbsolute = true;

    bool bTryed = false;
    if (!bAbsolute && bValid && !(flags2 & FT2_RELOAD))
    {
      if (strchr(nameTex, '/') || strchr(nameTex, '\\'))
      {
        if (eTT == eTT_Cubemap)
          tx = m_TexMan->LoadCubeTex(nameTex, flags, flags2, 0, eTT, 0, Id, BindId, fAmount1);
        else
          tx = m_TexMan->LoadTexture(nameTex, flags, flags2, eTT, fAmount1, fAmount2, BindId, Id);
        bTryed = true;
      }
      if (!tx || (tx->m_Flags & FT_NOTFOUND))
      {
        if (tx)
          tx->Release(false);

        string str;
        str = string("Textures/") + string(nameTex);
        if (eTT == eTT_Cubemap)
          tx = m_TexMan->LoadCubeTex(str.c_str(), flags, flags2, 0, eTT, 0, Id, BindId, fAmount1);
        else
          tx = m_TexMan->LoadTexture(str.c_str(), flags, flags2, eTT, fAmount1, fAmount2, BindId, Id);
      }
    }

    if (!bTryed && (!tx || (tx->m_Flags & FT_NOTFOUND)))
    {
      if (tx)
        tx->Release(false);

      if (eTT == eTT_Cubemap)
        tx = m_TexMan->LoadCubeTex(nameTex, flags, flags2, 0, eTT, 0, Id, BindId, fAmount1);
      else
        tx = m_TexMan->LoadTexture(nameTex, flags, flags2, eTT, fAmount1, fAmount2, BindId, Id);
    }
    return tx;
  }
}

int CRenderer::EF_ReadAllImgFiles(IShader *ef, SShaderTexUnit *tl, STexAnim *ta, char *name)
{
  return gRenDev->m_cEF.mfReadAllImgFiles((SShader *)ef, tl, ta, name);
}

bool CRenderer::EF_RegisterTemplate(int nTemplId, char *Name, bool bReplace)
{
  return gRenDev->m_cEF.mfRegisterTemplate(nTemplId, Name, bReplace);
}

char **CRenderer::EF_GetShadersForFile(const char *File, int num)
{
  int i;
  
  char file[256];
  ConvertDOSToUnixName(file, File);
  if (num >= 0)
  {
    for (i=0; i<MAX_EF_FILES; i++)
    {
      if (!gRenDev->m_cEF.m_FileNames[num][i].empty() && !stricmp(file, gRenDev->m_cEF.m_FileNames[num][i].c_str()))
      {
        char *pFinalScript = gRenDev->m_cEF.mfScriptForFileName(gRenDev->m_cEF.m_FileNames[num][i].c_str(), NULL, 0);
        char **Efs = gRenDev->m_cEF.mfListInScript(pFinalScript);
        delete [] pFinalScript;
        return Efs;
        
      }
    }
  }
  else
  {
    for (num=0; num<2; num++)
    {
      for (i=0; i<MAX_EF_FILES; i++)
      {
        if (!gRenDev->m_cEF.m_FileNames[num][i].empty() && !stricmp(file, gRenDev->m_cEF.m_FileNames[num][i].c_str()))
        {
          char *pFinalScript = gRenDev->m_cEF.mfScriptForFileName(gRenDev->m_cEF.m_FileNames[num][i].c_str(), NULL, 0);
          char **Efs = gRenDev->m_cEF.mfListInScript(pFinalScript);
          delete [] pFinalScript;
          return Efs;
        }
      }
    }
  }
  return NULL;
}

SLightMaterial *CRenderer::EF_GetLightMaterial(char *Str)
{
  return SLightMaterial::mfGet(Str);
}

IShader *CRenderer::EF_CopyShader(IShader *ef)
{
  return (SShader *)gRenDev->m_cEF.mfCopyShader((SShader *)ef);
}

void CRenderer::EF_StartEf ()
{
  int i;
  if (!SRendItem::m_RecurseLevel)
  {
    if (CRenderer::CV_r_shaderdefault->GetString()[0] != '0')
      m_pDefaultShader = (SShader *)EF_LoadShader(CRenderer::CV_r_shaderdefault->GetString(), eSH_World, 0);
    else
      m_pDefaultShader = NULL;

    for (i=0; i<NUMRI_LISTS; i++)
    {
      SRendItem::m_RendItems[i].SetUse(0);
      SRendItem::m_StartRI[SRendItem::m_RecurseLevel][i] = 0;
    }
    m_RP.m_NumVisObjects = 1;
    m_RP.m_TempObjects.SetUse(1);
    CCObject::m_Waves.SetUse(1);
    CCObject::m_ObjMatrices.resize(1);
    m_RP.m_RejectedObjects.SetUse(0);
  }

  for (i=0; i<NUMRI_LISTS; i++)
  {
    SRendItem::m_StartRI[SRendItem::m_RecurseLevel][i] = SRendItem::m_RendItems[i].Num();
  }

  EF_RemovePolysFromScene();
  SRendItem::m_RecurseLevel++;
  EF_ClearLightsList();
}

// Hide shader template (exclude from list)
bool CRenderer::EF_HideTemplate(const char *name)
{
  int i;

  CName nm = CName(name);

  for (i=0; i<m_HidedShaderTemplates.Num(); i++)
  {
    if (m_HidedShaderTemplates[i] == nm)
      return false;
  }
  m_HidedShaderTemplates.AddElem(nm);
  return true;
}

// UnHide shader template (include in list)
bool CRenderer::EF_UnhideTemplate(const char *name)
{
  int i;

  CName nm = CName(name);

  for (i=0; i<m_HidedShaderTemplates.Num(); i++)
  {
    if (m_HidedShaderTemplates[i] == nm)
    {
      m_HidedShaderTemplates.Remove(i, 1);
      return true;
    }
  }
  return false;
}

// UnHide all shader templates (include in list)
bool CRenderer::EF_UnhideAllTemplates()
{
  m_HidedShaderTemplates.Free();
  return true;
}

void CRenderer::EF_InitFogVolumes()
{
  m_RP.m_FogVolumes.Free();
  SMFog Fog;
  memset(&Fog,0,sizeof(Fog));
  m_RP.m_FogVolumes.AddElem(Fog); // register fake zero element
}

void CRenderer::PreLoad (void)
{
  EF_InitFogVolumes();
}
void CRenderer::PostLoad (void)
{
  m_nFrameLoad++;
  if (!m_bEditor)
  {
    if(m_TexMan)
    {
      m_TexMan->PreloadScreenFxMaps();
    }

    Reset();
  }
  m_bTemporaryDisabledSFX = false;
//  m_TexMan->PreloadTextures(-1);
}

void CRenderer::EF_AddEf (int NumFog, CRendElement *re, IShader *ef, SRenderShaderResources *sr, CCObject *obj, int nTempl, IShader *efState, int nSort)
{
  EF_AddEf_NotVirtual (NumFog, re, ef, sr, obj, nTempl, efState, nSort);
}

#ifdef _DEBUG
static float sMatIdent[16] = 
{
  1,0,0,0,
  0,1,0,0,
  0,0,1,0,
  0,0,0,1
};
#endif

void CRenderer::EF_AddEf_NotVirtual (int NumFog, CRendElement *re, IShader *ef, SRenderShaderResources *sr, CCObject *obj, int nTempl, IShader *efState, int nSort)
{
  assert(nSort>=0);
  assert(NumFog>=0);
  if (re && ef)
  {
    SShader *eft = (SShader *)ef->GetTemplate(nTempl);
    if (eft->m_Flags3 & EF3_NODRAW)
      return;
    if (m_pDefaultShader/* && (eft->m_Flags & EF_HASDIFFUSEMAP)*/)
      eft = m_pDefaultShader;
    if (m_HidedShaderTemplates.Num())
    {
      CName cn = CName(eft->m_Name.c_str(), eFN_Find);
      if (cn.GetIndex())
      {
        for (int i=0; i<m_HidedShaderTemplates.Num(); i++)
        {
          if (m_HidedShaderTemplates[i] == cn)
            return;
        }
      }
    }
    if (!CV_r_envlighting && obj)
      obj->m_ObjFlags &= ~FOB_ENVLIGHTING;
    if (eft->m_nPreprocess)
    {
      if (!(eft->m_nPreprocess & FSPR_SCANLCM) || (obj && (obj->m_ObjFlags & FOB_ENVLIGHTING)))
        SRendItem::mfAdd(re, obj, eft, sr ? sr->m_Id : 0, NULL, NumFog, nTempl, eS_PreProcess | EFSLIST_PREPROCESS);
    }

    if (obj)
    {
#ifdef _DEBUG
      if (memcmp(sMatIdent, obj->m_Matrix.GetData(), 4*4*4))
      {
        if (!(obj->m_ObjFlags & FOB_TRANS_MASK))
        {
          assert(0);
        }
      }
#endif
      if (obj->m_Color.a != 1.0f)
        obj->m_ObjFlags |= FOB_HASALPHA;
      if (obj->m_fBending)
        obj->m_ObjFlags |= FOB_BENDED;
    }

    int nS = (nSort & 0x1f) ? (nSort & 0x1f) : eft->m_eSort;
    switch(nS)
    {
      case eS_FogShader:
      case eS_Decal:
        SRendItem::mfAdd(re, obj, eft, sr ? sr->m_Id : 0, (SShader *)efState, NumFog, nTempl, (nSort & ~EFSLIST_MASK) | EFSLIST_GENERAL);
        return;
      case eS_FogShader_Trans:
      case eS_Glare:
      case eS_HeatVision:
        SRendItem::mfAdd(re, obj, eft, sr ? sr->m_Id : 0, (SShader *)efState, NumFog, nTempl, (nSort & ~EFSLIST_MASK) | EFSLIST_LAST);
        return;
      case eS_PreProcess:
        SRendItem::mfAdd(re, obj, eft, sr ? sr->m_Id : 0, (SShader *)efState, NumFog, nTempl, eS_Glare | EFSLIST_PREPROCESS);
        return;
    }
    if ((nSort & EFSLIST_MASK) != EFSLIST_LAST)
    {
      int nFlags2 = eft->GetFlags2();
      if (!(nFlags2 & EF2_DONTSORTBYDIST))
      {
        if (!(nFlags2&EF2_OPAQUE) || eft->m_eSort == eS_Water || (obj && obj->m_Color.a!=1.0f) || (sr && sr->m_Opacity!=1.f))
          SRendItem::mfAdd(re, obj, eft, sr ? sr->m_Id : 0, (SShader *)efState, NumFog, nTempl, (nSort & ~EFSLIST_MASK) | EFSLIST_DISTSORT);
        else
          SRendItem::mfAdd(re, obj, eft, sr ? sr->m_Id : 0, (SShader *)efState, NumFog, nTempl, nSort);
      }
      else
        SRendItem::mfAdd(re, obj, eft, sr ? sr->m_Id : 0, (SShader *)efState, NumFog, nTempl, nSort);
    }
    else
      SRendItem::mfAdd(re, obj, eft, sr ? sr->m_Id : 0, (SShader *)efState, NumFog, nTempl, nSort);
  }
}

int CRenderer::EF_SelectHWTechnique(SShader *ef)
{
  int nHW = 0;
  int n;

  // Select technique
  if (ef->m_HWTechniques.Num() > 1)
  {
    // Check conditions
    int nLights = -1;
    bool bDirect;
    bool bProjected;
    CCObject *obj = m_RP.m_pCurObject;
    for (nHW=0; nHW<ef->m_HWConditions.Num(); nHW++)
    {
      SHWConditions *hc = &ef->m_HWConditions[nHW];
      if (hc->m_Flags)
      {
        int fl = hc->m_Flags;
        int flc;
        if ((flc=(fl & FOB_MASKCONDITIONS)) && ((m_RP.m_ObjFlags & flc) != flc))
          continue;
        if (fl & SHCF_HASDOT3LM)
        { // also support separate stream for lightmaps from object
          if ((!m_RP.m_pShaderResources || !m_RP.m_pShaderResources->m_Textures[EFTT_LIGHTMAP_DIR]) && !m_RP.m_pCurObject->m_nLMDirId)
            continue;
        }
        else
        if (fl & SHCF_HASLM)
        {
          if ((!m_RP.m_pShaderResources || !m_RP.m_pShaderResources->m_Textures[EFTT_LIGHTMAP]) && !m_RP.m_pCurObject->m_nLMId)
            continue;
        }
        if ((fl & SHCF_INFOGVOLUME) && !m_RP.m_pFogVolume)
          continue;
        if ((fl & SHCF_HASRESOURCE) && !m_RP.m_pShaderResources)
          continue;
        if (fl & SHCF_RETEXBIND_MASK)
        {
          if (!m_RP.m_pRE || m_RP.m_pRE->m_CustomTexBind[(fl & SHCF_RETEXBIND_MASK)>>12] <= 0)
            continue;
        }
        if (fl & (SHCF_HASVCOLORS | SHCF_HASALPHATEST | SHCF_HASALPHABLEND))
        {
          if ((fl & SHCF_HASVCOLORS) && !(ef->m_Flags3 & EF3_HASVCOLORS))
            continue;
          if ((fl & SHCF_HASALPHATEST) && !(ef->m_Flags3 & EF3_HASALPHATEST))
          {
            if (!m_RP.m_pShaderResources || !m_RP.m_pShaderResources->m_AlphaRef)
              continue;
          }
          if ((fl & SHCF_HASALPHABLEND) && !(ef->m_Flags3 & EF3_HASALPHABLEND))
            continue;
        }
        if (fl & SHCF_LIGHT)
        {
          if (nLights < 0)
          {
            bDirect = false;
            bProjected = false;
            nLights = 0;
            for (n=0; n<m_RP.m_NumActiveDLights; n++)
            {
              CDLight *dl = m_RP.m_pActiveDLights[n];
              if ((dl->m_Flags & DLF_LIGHTTYPE_MASK) == DLF_DIRECTIONAL)
              {
                if (ef->m_Flags3 & EF3_IGNOREDIRECTIONALLIGHT)
                  continue;
                bDirect = true;
              }
              else
              if ((dl->m_Flags & DLF_LIGHTTYPE_MASK) == DLF_PROJECT)
                bProjected = true;
              nLights++;
            }
            if (m_RP.m_fCurOpacity != 1.0f && nLights > 1)
              nLights = 1;
            else
            if (nLights < 2)
            {
              if ((m_RP.m_ObjFlags & FOB_LIGHTPASS) || (CV_r_bumpselfshadow && m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_BUMP] && m_RP.m_pShaderResources->m_Textures[EFTT_BUMP]->m_TU.m_TexPic && m_RP.m_pShaderResources->m_Textures[EFTT_BUMP]->m_TU.m_TexPic->m_pSH))
                nLights = 2;
            }
          }

          int lm;
          if ((lm = (fl & (SHCF_SINGLELIGHT | SHCF_MULTIPLELIGHTS))))
          {
            if (lm == (SHCF_SINGLELIGHT | SHCF_MULTIPLELIGHTS) && nLights < 1)
              continue;
            if ((fl & SHCF_ONLYDIRECTIONAL) && !bDirect) 
              continue;
            if ((fl & SHCF_HASPROJECTEDLIGHTS) && !bProjected) 
              continue;
            if (lm == SHCF_SINGLELIGHT && nLights != 1)
              continue;
            if (lm == SHCF_MULTIPLELIGHTS && nLights <= 1)
              continue;
          }
          else
          if ((fl & SHCF_NOLIGHTS) && nLights != 0)
            continue;
        }
      }
      for (n=0; n<hc->m_NumVars; n++)
      {
        CVarCond *vc = &hc->m_Vars[n];
        if (vc->m_Var->GetFVal() != vc->m_Val)
          break;
      }
      if (n == hc->m_NumVars)
        break;
    }
    if (nHW == ef->m_HWTechniques.Num())
    {
      static int nMessageCount=0;
      if(nMessageCount<20)
      { // print only first 20 messages to avoid log overflow
        iLog->Log("Warning: Couldn't find suitable render technique for shader '%s' (skipped)\n", ef->m_Name.c_str());
        nMessageCount++;
      }

      return -1;
    }
  }
  else
    nHW = 0;

  return nHW;
}

CRendElement *CRenderer::EF_CreateRE (EDataType edt)
{
  CRendElement *re = NULL;

  switch(edt)
  {
    case eDATA_OcLeaf:
      re = new CREOcLeaf;
      break;

    case eDATA_HDRProcess:
      re = new CREHDRProcess;
      break;

    case eDATA_OcclusionQuery:
      re = new CREOcclusionQuery;
      break;

    case eDATA_Ocean:
#ifdef DEBUGALLOC
#undef new
#endif
      re = new CREOcean;
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
      break;

    case eDATA_Flare:
      re = new CREFlare;
      break;

    case eDATA_Sky:
      re = new CRESky;
      break;

    case eDATA_Beam:
      re = new CREBeam;
      break;

    case eDATA_Poly:
      re = new CREPolyMesh;
      break;

    case eDATA_Glare:
      re = new CREGlare;
      break;

    case eDATA_Prefab:
      re = new CREPrefabGeom;
      break;

    case eDATA_TerrainSector:
      re = new CRECommon;
      break;

    case eDATA_2DQuad:
      re = new CRE2DQuad;
      break;

    case eDATA_TriMeshShadow:
      re = new CRETriMeshShadow;
      break;

    case eDATA_FarTreeSprites:
      re = new CREFarTreeSprites;
      break;

    case eDATA_Dummy:
      re = new CREDummy;
      break;

    // tiago: added
    case eDATA_FlashBang:
      re = new CREFlashBang;
      break;

    case eDATA_ScreenProcess:
      re = new CREScreenProcess;
      m_pREScreenProcess = (CREScreenProcess *)re;
      break;

    case eDATA_ShadowMapGen:
      re = new CREShadowMapGen;
      break;
  
    case eDATA_TerrainDetailTextureLayers:
      re = new CRETerrainDetailTextureLayers;
      break;

    case eDATA_TerrainParticles:
      re = new CRETerrainParticles;
      break;
      
    case eDATA_ClearStencil:
      re = new CREClearStencil;
      break;
  }
  return re;
}


void CCObject::Init()
{
  //m_ObjFlags = FOB_TRANS_MASK;
  m_ObjFlags = 0;
  if (m_ShaderParams && m_bShaderParamCreatedInRenderer)
  {
    m_bShaderParamCreatedInRenderer = false;
    delete m_ShaderParams;
  }
  m_ShaderParams = NULL;
  m_nLMId=m_nLMDirId=0;
  m_InvMatrixId = -1;
  m_VPMatrixId = -1;
  m_RE = NULL;
  m_EF = NULL;
  m_CustomData = NULL;
  m_DynLMMask = 0;
  m_fDistanceToCam = -1.0f;
  m_RenderState = 0;
  m_fHeatFactor = 1.0f;
  m_NumCM = -1;
  m_SortId = 0;
  m_NumWFX = 0;
  m_NumWFY = 0;
  m_fLightFadeTime = 0;
  m_pShadowCasters = NULL;
  //m_pShadowFrustum = NULL;
  m_bVisible = false;
  m_AmbColor = Vec3d(1.0f, 1.0f, 1.0f);
  m_Color = CFColor(1.0f);
  m_pCharInstance = NULL;
  m_pLightImage = NULL;
  m_pLMTCBufferO = NULL;

  m_nScissorX1=m_nScissorX2=m_nScissorY1=m_nScissorY2=0;
}

CCObject::~CCObject()
{
  if (m_ShaderParams && m_bShaderParamCreatedInRenderer)
  {
    m_bShaderParamCreatedInRenderer = false;
    delete m_ShaderParams;
  }
}

void CCObject::SetShaderFloat(const char *Name, float Val)
{
  string name;
  int i;

  name = Name;
  std::transform( name.begin(), name.end(), name.begin(), tolower );
  if (!m_ShaderParams)
    m_ShaderParams = new TArray<SShaderParam>;
  for (i=0; i<m_ShaderParams->Num(); i++)
  {
    if (!strcmp(name.c_str(), m_ShaderParams->Get(i).m_Name))
      break;
  }
  if (i == m_ShaderParams->Num())
  {
    SShaderParam pr;
    strncpy(pr.m_Name, name.c_str(), 32);
    m_ShaderParams->AddElem(pr);
  }
  SShaderParam *pr = &m_ShaderParams->Get(i);
  pr->m_Type = eType_FLOAT;
  pr->m_Value.m_Float = Val;
  m_bShaderParamCreatedInRenderer = true;
}

TArray<SWaveForm2> CCObject::m_Waves;
MatrixArray16 CCObject::m_ObjMatrices;

void CCObject::RemovePermanent()
{
  for (int i=0; i<gRenDev->m_RP.m_Objects.Num(); i++)
  {
    if (gRenDev->m_RP.m_Objects[i] == this)
    {
      m_ObjFlags |= FOB_REMOVED;
      if (m_RE && m_RE->mfGetType() == eDATA_OcclusionQuery)
      {
        m_RE->Release();
        m_RE = NULL;
      }
      break;
    }
  }
}

void CCObject::AddWaves(SWaveForm2 **pWF)
{
  int n1, n2;
  SWaveForm2 *wf;
  if (!m_NumWFX)
  {
    n1 = m_Waves.Num();
    m_Waves.AddIndex(1);
    wf = &m_Waves[n1];
    m_NumWFX = n1;
    wf->m_Amp = 0;
    wf->m_Freq = 0;
    wf->m_Level = 0;
    wf->m_Phase = 0;
    wf->m_eWFType = eWF_Sin;
  }
  if (!m_NumWFY)
  {
    n2 = m_Waves.Num();
    m_Waves.AddIndex(1);
    wf = &m_Waves[n2];
    m_NumWFY = n2;
    wf->m_Amp = 0;
    wf->m_Freq = 0;
    wf->m_Level = 0;
    wf->m_Phase = 0;
    wf->m_eWFType = eWF_Sin;
  }
  if (pWF)
  {
    pWF[0] = &m_Waves[n1];
    pWF[1] = &m_Waves[n2];
  }
}

CCObject *CRenderer::EF_GetObject (bool bTemp, int num)
{
  CCObject *obj;
  bool bOverflow = false;
  static int sFrameWarn;
  int i;

  if (num >= 0)
  {
    obj = m_RP.m_Objects[num];
    if (m_RP.m_NumVisObjects >= MAX_REND_OBJECTS)
      bOverflow = true;
  }
  else
  {
    TArray <CCObject *> *Objs;
    if (bTemp)
      Objs = &m_RP.m_TempObjects;
    else
      Objs = &m_RP.m_Objects;
    int n = Objs->Num();
    if (m_RP.m_NumVisObjects >= MAX_REND_OBJECTS)
    {
      if (sFrameWarn != m_nFrameID)
      {
        sFrameWarn = m_nFrameID;
        iLog->Log("Error: CRenderer::EF_GetObject: Too many objects (> %d)\n", MAX_REND_OBJECTS);
      }
      obj = (*Objs)[1];
      bOverflow = true;
      n = 1;
    }
    else
    {
      if (bTemp && m_RP.m_RejectedObjects.Num())
      {
        int n = m_RP.m_RejectedObjects.Num()-1;
        obj = m_RP.m_RejectedObjects[n];
        m_RP.m_RejectedObjects.SetUse(n);
        obj->Init();
        return obj;
      }
      else
      {
        if (!bTemp)
        {
          for (i=1; i<Objs->Num(); i++)
          {
            if (!Objs->Get(i) || (Objs->Get(i)->m_ObjFlags & FOB_REMOVED))
              break;
          }
          if (i != Objs->Num())
            n = i;
          else
          {
            if (Objs->Num() != MAX_REND_OBJECTS-1)
              Objs->AddIndex(1);
            else
              assert(false);
          }
        }
        else
          Objs->AddIndex(1);
        obj = (*Objs)[n];
        if (!obj)
        {
          obj = new CCObject;
          (*Objs)[n] = obj;
        }
        //cryPrecacheSSE(obj, sizeof(*obj));
        {
          byte *pB = (byte *)obj;
          cryPrefetchNTSSE(pB);
          cryPrefetchNTSSE(pB+64);
          cryPrefetchNTSSE(pB+128);
        }
        CCObject *objNext;
        if (Objs->GetSize() > n+1 && (objNext=(*Objs)[n+1]))
        {
          byte *pB = (byte *)objNext;
          cryPrefetchT0SSE(pB);
          cryPrefetchT0SSE(pB+64);
          cryPrefetchT0SSE(pB+128);
        }
      }
      obj->m_Id = n;
    }
    obj->Init();
  }
  if (!bOverflow)
  {
    m_RP.m_VisObjects[m_RP.m_NumVisObjects] = obj;
    obj->m_VisId = m_RP.m_NumVisObjects;
    if (SRendItem::m_RecurseLevel == 1)
      obj->m_Counter++;
    m_RP.m_NumVisObjects++;
  }
  else
    obj->m_VisId = 1;

  return obj;
}

void CRenderer::EF_AddSplash(Vec3d Pos, eSplashType eST, float fForce, int Id)
{
  int i;

  fForce *= CRenderer::CV_r_oceansplashscale;
  SSplash *spl = NULL;
  //Id = 0;
  if (Id >= 0)
  {
    for (i=0; i<m_RP.m_Splashes.Num(); i++)
    {
      spl = &m_RP.m_Splashes[i];
      if (spl->m_Id == Id)
        break;
    }
    if (i == m_RP.m_Splashes.Num())
      spl = NULL;
  }
  if (!spl)
  {
    SSplash sp;
    sp.m_Id = Id;
    m_RP.m_Splashes.AddElem(sp);
    spl = &m_RP.m_Splashes[m_RP.m_Splashes.Num()-1];
    spl->m_fStartTime = iTimer->GetCurrTime();
  }
  spl->m_Pos = Pos;
  spl->m_fForce = fForce;
  spl->m_eType = eST;
  spl->m_fLastTime = iTimer->GetCurrTime();
}

void CRenderer::EF_UpdateSplashes(float fCurTime)
{
  int i;

  for (i=0; i<m_RP.m_Splashes.Num(); i++)
  {
    SSplash *spl = &m_RP.m_Splashes[i];
    float fScaleFactor = 1.0f / (m_RP.m_RealTime - spl->m_fLastTime + 1.0f);
    if (fScaleFactor*spl->m_fForce < 0.1f)
    {
      m_RP.m_Splashes.Remove(i);
      i--;
    }
    spl->m_fCurRadius = (fCurTime-spl->m_fLastTime)*10.0f*4.0f;
  }
}

float CRenderer::EF_GetWaterZElevation(float fX, float fY)
{
  if (CREOcean::m_pStaticOcean)
    return CREOcean::m_pStaticOcean->GetWaterZElevation(fX, fY);
  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  if (!eng)
    return 0;
  return eng->GetWaterLevel();
}

void CRenderer::EF_RemovePolysFromScene(void)
{
  CREClientPoly::mPolysStorage[SRendItem::m_RecurseLevel].SetUse(0);
  CREClientPoly2D::mPolysStorage.SetUse(0);
  m_RP.m_Sprites.SetUse(0);
}

void CRenderer::EF_AddPolyToScene2D(SShaderItem si, int nTempl, int numPts, SColorVert2D *verts)
{
  int num = CREClientPoly2D::mPolysStorage.Num();
  CREClientPoly2D::mPolysStorage.GrowReset(1);

  SShader *sh = (SShader *)si.m_pShader;
  sh = sh->mfGetTemplate(nTempl);

  CREClientPoly2D *pl = CREClientPoly2D::mPolysStorage[num];
  if (!pl)
  {
    pl = new CREClientPoly2D;
    CREClientPoly2D::mPolysStorage[num] = pl;
  }

  pl->mEf = sh;
  pl->m_pShaderResources = si.m_pShaderResources;
  pl->mNumVerts = numPts;

  SColorVert2D *vt = pl->mVerts;
  int i;
  for (i=0; i<numPts; i++, vt++)
  {
    Vector2Copy(verts[i].vert, vt->vert);
    Vector2Copy(verts[i].dTC, vt->dTC);
    vt->color.dcolor = verts[i].color.dcolor;
  }

  byte *vrtind = pl->mIndices;

  for (i=0; i<numPts-2; i++, vrtind+=3)
  {
    vrtind[0] = 0;
    vrtind[1] = i+1;
    vrtind[2] = i+2;
  }
  pl->mNumIndices = (numPts-2) * 3;
}

void CRenderer::EF_AddPolyToScene2D(int Ef, int numPts, SColorVert2D *verts)
{
  int num = CREClientPoly2D::mPolysStorage.Num();
  CREClientPoly2D::mPolysStorage.GrowReset(1);

  CREClientPoly2D *pl = CREClientPoly2D::mPolysStorage[num];
  if (!pl)
  {
    pl = new CREClientPoly2D;
    CREClientPoly2D::mPolysStorage[num] = pl;
  }

  SShader *sh = SShader::m_Shaders_known[Ef]->mfGetTemplate(-1);

  pl->mEf = sh;
  pl->mNumVerts = numPts;

  SColorVert2D *vt = pl->mVerts;
  int i;
  for (i=0; i<numPts; i++, vt++)
  {
    Vector2Copy(verts[i].vert, vt->vert);
    Vector2Copy(verts[i].dTC, vt->dTC);
    vt->color.dcolor = verts[i].color.dcolor;
  }

  byte *vrtind = pl->mIndices;

  for (i=0; i<numPts-2; i++, vrtind+=3)
  {
    vrtind[0] = 0;
    vrtind[1] = i+1;
    vrtind[2] = i+2;
  }
  pl->mNumIndices = (numPts-2) * 3;
}

//================================================================================================================

CCObject *CRenderer::EF_AddSpriteToScene(int Ef, int numPts, SColorVert *verts, CCObject *obj, byte *inds, int ninds, int nFogID)
{
  int i;

  if (obj)
  {
    SRefSprite *rs = NULL;
    for (i=0; i<m_RP.m_Sprites.Num(); i++)
    {
      rs = &m_RP.m_Sprites[i];
      if (obj == rs->m_pObj)
        break;
    }
    if (i == m_RP.m_Sprites.Num())
    {
      for (i=0; i<m_RP.m_Sprites.Num(); i++)
      {
        rs = &m_RP.m_Sprites[i];
        if (rs->m_pObj->m_NumCM == obj->m_NumCM && rs->m_pObj->m_DynLMMask == obj->m_DynLMMask && rs->m_pObj->m_RenderState == obj->m_RenderState && rs->m_pObj->m_SortId == obj->m_SortId)
        {
          m_RP.m_RejectedObjects.AddElem(obj);
          obj = rs->m_pObj;
          break;
        }
      }
      if (i == m_RP.m_Sprites.Num())
      {
        SRefSprite s;
        s.m_pObj = obj;
        //obj->m_Matrix.SetIdentity();
        m_RP.m_Sprites.AddElem(s);
        rs = &m_RP.m_Sprites[m_RP.m_Sprites.Num()-1];
      }
    }
  } 
  int num = CREClientPoly::mPolysStorage[SRendItem::m_RecurseLevel].Num();
  CREClientPoly::mPolysStorage[SRendItem::m_RecurseLevel].GrowReset(1);

  CREClientPoly *pl = CREClientPoly::mPolysStorage[SRendItem::m_RecurseLevel][num];
  if (!pl)
  {
    pl = new CREClientPoly;
    CREClientPoly::mPolysStorage[SRendItem::m_RecurseLevel][num] = pl;
  }

  pl->mEf = SShader::m_Shaders_known[Ef];
  pl->mNumVerts = numPts;
  pl->m_pObject = obj;
  pl->m_nFogID = nFogID;

  SColorVert *vt = pl->mVerts;
  for (i=0; i<numPts; i++, vt++)
  {
    vt->vert = verts[i].vert;
    Vector2Copy(verts[i].dTC, vt->dTC);
    vt->color.dcolor = verts[i].color.dcolor;
  }
  if (!(m_Features & RFT_RGBA))
  {
    vt = pl->mVerts;
    for (i=0; i<numPts; i++, vt++)
    {
      Exchange(vt->color.bcolor[0], vt->color.bcolor[2]);
    }
  }

  byte *vrtind = pl->mIndices;

  if (inds && ninds)
  {
    assert(ninds <= (16-2)*3);
    memcpy(vrtind, inds, ninds);
    pl->mNumIndices = ninds;
  }
  else
  {
    for (i=0; i<numPts-2; i++, vrtind+=3)
    {
      vrtind[0] = 0;
      vrtind[1] = i+1;
      vrtind[2] = i+2;
    }
    pl->mNumIndices = (numPts-2) * 3;
  }
  return obj;
}

void CRenderer::EF_AddSprite(SShader *pSH, Vec3d vOrigin, float fRadius)
{
  SColorVert vert[4];

  Vec3d VecX = gRenDev->m_RP.m_CamVecs[1] * fRadius;
  Vec3d VecY = gRenDev->m_RP.m_CamVecs[2] * fRadius;
  Vec3d v;

  v = vOrigin + VecX + VecY;
  vert[0].vert = v;
  vert[0].dTC[0] = 0;
  vert[0].dTC[1] = 0;
  vert[0].color.dcolor = -1;

  v = vOrigin - VecX + VecY;
  vert[1].vert = v;
  vert[1].dTC[0] = 1;
  vert[1].dTC[1] = 0;
  vert[1].color.dcolor = -1;

  v = vOrigin - VecX - VecY;
  vert[2].vert = v;
  vert[2].dTC[0] = 1;
  vert[2].dTC[1] = 1;
  vert[2].color.dcolor = -1;

  v = vOrigin + VecX - VecY;
  vert[3].vert = v;
  vert[3].dTC[0] = 0;
  vert[3].dTC[1] = 1;
  vert[3].color.dcolor = -1;

  EF_AddSpriteToScene(pSH->m_Id, 4, vert, NULL);
}

void CRenderer::EF_AddPolyToScene3D(int Ef, int numPts, SColorVert *verts, CCObject *obj, int nFogID)
{
  int num = CREClientPoly::mPolysStorage[SRendItem::m_RecurseLevel].Num();
  CREClientPoly::mPolysStorage[SRendItem::m_RecurseLevel].GrowReset(1);

  CREClientPoly *pl = CREClientPoly::mPolysStorage[SRendItem::m_RecurseLevel][num];
  if (!pl)
  {
    pl = new CREClientPoly;
    CREClientPoly::mPolysStorage[SRendItem::m_RecurseLevel][num] = pl;
  }

  pl->mEf = SShader::m_Shaders_known[Ef];

  pl->mNumVerts = numPts;
  pl->m_pObject = obj;
  pl->m_nFogID = nFogID;

  SColorVert *vt = pl->mVerts;
  int i;
  for (i=0; i<numPts; i++, vt++)
  {
    vt->vert = verts[i].vert;
    Vector2Copy(verts[i].dTC, vt->dTC);
    vt->color.dcolor = verts[i].color.dcolor;
  }

  byte *vrtind = pl->mIndices;

  for (i=0; i<numPts-2; i++, vrtind+=3)
  {
    vrtind[0] = 0;
    vrtind[1] = i+1;
    vrtind[2] = i+2;
  }
  pl->mNumIndices = (numPts-2) * 3;
}

void CRenderer::EF_AddClientPolys3D(void)
{
  int i;
  CREClientPoly *pl;

  for (i=0; i<CREClientPoly::mPolysStorage[SRendItem::m_RecurseLevel].Num(); i++)
  {
    pl = CREClientPoly::mPolysStorage[SRendItem::m_RecurseLevel][i];
    pl->m_fDistance = -1;
    
    if (pl->m_pObject)
    {
      if (pl->mEf->m_nPreprocess)
        SRendItem::mfAdd(pl, pl->m_pObject, pl->mEf, 0, NULL, pl->m_nFogID, 0, eS_PreProcess | EFSLIST_PREPROCESS);
      SRendItem::mfAdd(pl, pl->m_pObject, pl->mEf, 0, NULL, pl->m_nFogID, 0, EFSLIST_DISTSORT);
    }
  }
}

//================================================================================================================
// 2d interface for shaders

void CRenderer::EF_AddClientPolys2D(void)
{
  int i;
  CREClientPoly2D *pl;

  for (i=0; i<CREClientPoly2D::mPolysStorage.Num(); i++)
  {
    pl = CREClientPoly2D::mPolysStorage[i];
    
    SShader *eft = pl->mEf;
    if (pl->m_pShaderResources)
    {
      if (eft->m_nPreprocess)
        SRendItem::mfAdd(pl, 0, eft, pl->m_pShaderResources->m_Id, NULL, 0, 0, eS_PreProcess);
      SRendItem::mfAdd(pl, 0, eft, pl->m_pShaderResources->m_Id, NULL, 0, 0);
    }
    else
    {
      if (eft->m_nPreprocess)
        SRendItem::mfAdd(pl, 0, eft, 0, NULL, 0, 0, eS_PreProcess);
      SRendItem::mfAdd(pl, 0, eft, 0, NULL, 0, 0);
    }
  }
}

bool CRenderer::EF_DrawEf(IShader *e, float x, float y, float width, float height, CFColor& col, int nTempl)
{
  SShader *ef = (SShader *)e;
  if (!ef)
  {
    iLog->Log("Warning: CRenderer::EF_DrawEf: NULL shader\n");
    return false;
  }

  SColorVert2D Points[4];

  DWORD dcl = col.GetTrue();
  //dcl = COLCONV(dcl);

  float fx = (float)x / 800.0f * m_width;
  float fy = (float)y / 600.0f * m_height;
  float fwdt = (float)width / 800.0f * m_width;
  float fhgt = (float)height / 600.0f * m_height;

  float s = 1.0f;
  float t = 1.0f;
  if (ef->m_Flags3 & EF3_SCREENTEXTURE)
  {
    s = fwdt;
    t = fhgt;
  }

  Points[0].vert[0] = fx;
  Points[0].vert[1] = fy;
  Points[0].dTC[0]    = 0;
  Points[0].dTC[1]    = t;
  Points[0].color.dcolor = dcl;

  Points[1].vert[0] = fx + fwdt;
  Points[1].vert[1] = fy;
  Points[1].dTC[0]    = s;
  Points[1].dTC[1]    = t;
  Points[1].color.dcolor = dcl;

  Points[2].vert[0] = fx + fwdt;
  Points[2].vert[1] = fy + fhgt;
  Points[2].dTC[0]    = s;
  Points[2].dTC[1]    = 0;
  Points[2].color.dcolor = dcl;

  Points[3].vert[0] = fx;
  Points[3].vert[1] = fy + fhgt;
  Points[3].dTC[0]    = 0;
  Points[3].dTC[1]    = 0;
  Points[3].color.dcolor   = dcl;

  ef = ef->mfGetTemplate(nTempl);
  EF_AddPolyToScene2D(ef->m_Id, 4, Points);

  return true;
}

bool CRenderer::EF_DrawEf(SShaderItem si, float x, float y, float width, float height, CFColor& col, int nTempl)
{
  SShader *ef = (SShader *)si.m_pShader;
  if (!ef)
  {
    iLog->Log("Warning: CRenderer::EF_DrawEf: NULL shader\n");
    return false;
  }

  SColorVert2D Points[4];

  DWORD dcl = col.GetTrue();
  //dcl = COLCONV(dcl);

  float fx = (float)x / 800.0f * m_width;
  float fy = (float)y / 600.0f * m_height;
  float fwdt = (float)width / 800.0f * m_width;
  float fhgt = (float)height / 600.0f * m_height;

  float s = 1.0f;
  float t = 1.0f;
  if (ef->m_Flags3 & EF3_SCREENTEXTURE)
  {
    s = fwdt;
    t = fhgt;
  }

  Points[0].vert[0] = fx;
  Points[0].vert[1] = fy;
  Points[0].dTC[0]    = 0;
  Points[0].dTC[1]    = t;
  Points[0].color.dcolor = dcl;

  Points[1].vert[0] = fx + fwdt;
  Points[1].vert[1] = fy;
  Points[1].dTC[0]    = s;
  Points[1].dTC[1]    = t;
  Points[1].color.dcolor = dcl;

  Points[2].vert[0] = fx + fwdt;
  Points[2].vert[1] = fy + fhgt;
  Points[2].dTC[0]    = s;
  Points[2].dTC[1]    = 0;
  Points[2].color.dcolor = dcl;

  Points[3].vert[0] = fx;
  Points[3].vert[1] = fy + fhgt;
  Points[3].dTC[0]    = 0;
  Points[3].dTC[1]    = 0;
  Points[3].color.dcolor   = dcl;

  EF_AddPolyToScene2D(si, nTempl, 4, Points);

  return true;
}

bool CRenderer::EF_DrawEfForName(char *name, float x, float y, float width, float height, CFColor& col, int nTempl)
{
  IShader *ef = EF_LoadShader(name, eSH_Screen, 0);
  bool bResult = EF_DrawEf(ef, x, y, width, height, col, nTempl);
  ef->Release();

  return bResult;
}

bool CRenderer::EF_DrawEfForNum(int num, float x, float y, float width, float height, CFColor& col, int nTempl)
{
  if(SShader::m_Shaders_known[num])
    return EF_DrawEf(SShader::m_Shaders_known[num], x, y, width, height, col, nTempl);
  else
    return EF_DrawEf(gRenDev->m_cEF.m_DefaultShader, x, y, width, height, col, nTempl);
}


bool CRenderer::EF_DrawPartialEf(IShader *e, SVrect *vr, SVrect *pr, CFColor &col, float iwdt, float ihgt)
{
  SShader *ef = (SShader *)e;
  if (!ef)
  {
    iLog->Log("Warning: CRenderer::EF_DrawPartialEf: NULL shader\n");
    return false;
  }

  SColorVert2D Points[4];

  DWORD dcl = col.GetTrue();
  //dcl = COLCONV(dcl);

  int wdt, hgt;
  int nPass, nTU;
  float tx1, ty1, tx2, ty2;

  if (!iwdt)
  {
    STexPic *tx = (STexPic *)ef->GetBaseTexture(&nPass, &nTU);
    if (tx)
    {
      wdt = tx->m_Width;
      hgt = tx->m_Height;
    }
    else
    if (ef->m_Passes[0].m_TUnits.Num() && ef->m_Passes[0].m_TUnits[0].m_TexPic)
    {
      wdt = ef->m_Passes[0].m_TUnits[0].m_TexPic->m_Width;
      hgt = ef->m_Passes[0].m_TUnits[0].m_TexPic->m_Height;
    }
    else
      wdt = hgt = 64;

    if (pr->width < 0)
      pr->width = wdt;
    if (pr->height < 0)
      pr->height = hgt;

    tx1 = (float)pr->x / (float)wdt;
    ty1 = (float)pr->y / (float)hgt;
    tx2 = (float)(pr->x+pr->width) / (float)wdt;
    ty2 = (float)(pr->y+pr->height) / (float)hgt;
  }
  else
  {
    if (pr->width < 0)
      pr->width = (int)(1.0f / iwdt);
    if (pr->height < 0)
      pr->height = (int)(1.0f / ihgt);
    tx1 = (float)pr->x * iwdt;
    ty1 = (float)pr->y * ihgt;
    tx2 = (float)(pr->x+pr->width) * iwdt;
    ty2 = (float)(pr->y+pr->height) * ihgt;
  }

  float x = (float)vr->x;
  float y = (float)vr->y;
  float width = (float)vr->width;
  float height = (float)vr->height;

  Points[0].vert[0] = x;
  Points[0].vert[1] = y;
  Points[0].dTC[0]    = tx1;
  Points[0].dTC[1]    = ty1;
  Points[0].color.dcolor = dcl;

  Points[1].vert[0] = x + width;
  Points[1].vert[1] = y;
  Points[1].dTC[0]    = tx2;
  Points[1].dTC[1]    = ty1;
  Points[1].color.dcolor = dcl;

  Points[2].vert[0] = x + width;
  Points[2].vert[1] = y + height;
  Points[2].dTC[0]    = tx2;
  Points[2].dTC[1]    = ty2;
  Points[2].color.dcolor = dcl;

  Points[3].vert[0] = x;
  Points[3].vert[1] = y + height;
  Points[3].dTC[0]    = tx1;
  Points[3].dTC[1]    = ty2;
  Points[3].color.dcolor = dcl;

  EF_AddPolyToScene2D(ef->m_Id, 4, Points);

  return true;
}

bool CRenderer::EF_DrawPartialEfForNum(int num, SVrect *vr, SVrect *pr, CFColor& col)
{
  return EF_DrawPartialEf(SShader::m_Shaders_known[num], vr, pr, col);
}

bool CRenderer::EF_DrawPartialEfForName(char *name, SVrect *vr, SVrect *pr, CFColor& col)
{
  IShader *ef;

  ef = EF_LoadShader(name, eSH_Screen, 0);
  bool bResult = EF_DrawPartialEf(ef, vr, pr, col);
  ef->Release();

  return bResult;
}

void CRenderer::EF_EnableHeatVision(bool bEnable)
{
  if (bEnable == m_bHeatVision)
    return;

  if (bEnable)
  {
    m_SavedWorldColor = m_WorldColor;
    m_WorldColor = CFColor(0.1f, 0.1f, 0.1f, 1.0f);
  }
  else
    m_WorldColor = m_SavedWorldColor;

  m_bHeatVision = bEnable;
}

bool CRenderer::EF_GetHeatVision()
{
  return m_bHeatVision;
}

// Dynamic lights
bool CRenderer::EF_IsFakeDLight(CDLight *Source)
{
  if (!Source)
  {
    iLog->Log("Warning: EF_IsFakeDLight: NULL light source\n");
    return true;
  }

  bool bIgnore = false;
  if (Source->m_Flags & DLF_FAKE)
    bIgnore = true;
  else
  if (Source->m_pShader!=0 && (Source->m_pShader->GetLFlags() & LMF_DISABLE))
    bIgnore = true;
  else
  if (m_bHeatVision && !(Source->m_Flags & DLF_HEATSOURCE) && !CV_r_lightsourcesasheatsources)
    bIgnore = true;
  else
  if (Source->m_Flags & DLF_HEATSOURCE)
  {
    if (!m_bHeatVision && !(Source->m_Flags & DLF_LIGHTSOURCE))
      bIgnore = true;
  }
  return bIgnore;
}

void CRenderer::EF_ADDDlight(CDLight *Source)
{
  if (!Source)
  {
    iLog->Log("Warning: EF_ADDDlight: NULL light source\n");
    return;
  }

  bool bIgnore = EF_IsFakeDLight(Source);
  //Source->m_Flags &= ~DLF_POINT;
  //Source->m_Flags |= DLF_DIRECTIONAL;

  if (bIgnore)
    Source->m_Id = -1;
  else
  {
    assert((Source->m_Flags & DLF_LIGHTTYPE_MASK) != 0);
    Source->m_Id = m_RP.m_DLights[SRendItem::m_RecurseLevel].Num();
    if (Source->m_Id >= 32)
    {
      //iLog->Log("Warning: EF_ADDDlight: Too many light sources (Ignored)\n");
      Source->m_Id = -1;
      return;
    }
    m_RP.m_DLights[SRendItem::m_RecurseLevel].AddElem(Source);
  }
  EF_PrecacheResource(Source, (m_cam.GetPos()-Source->m_Origin).Length(), 0.1f, 0);

  // Add light coronas, lens flares, beams and so on (depends on shader)
  if (Source->m_pShader!=0 && Source->m_pShader->GetREs()->Num())
  {
    I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
    int rl = SRendItem::m_RecurseLevel;
    float fWaterLevel = eng->GetWaterLevel();
    float fCamZ = m_cam.GetPos().z;
    for (int nr=0; nr<Source->m_pShader->GetREs()->Num(); nr++)
    {
      //get the permanent object
      if (!Source->m_pObject[rl][nr])
      {
        Source->m_pObject[rl][nr] = EF_GetObject(false, -1);
        Source->m_pObject[rl][nr]->m_Color.a = 0;
        Source->m_pObject[rl][nr]->m_AmbColor = Vec3d(0,0,0);
        Source->m_pObject[rl][nr]->m_Angs2[0] = 0;
        Source->m_pObject[rl][nr]->m_Angs2[1] = 0;
        Source->m_pObject[rl][nr]->m_Angs2[2] = 0;
        Source->m_pObject[rl][nr]->m_TempVars[3] = 0;
        Source->m_pObject[rl][nr]->m_TempVars[4] = 0;
        Source->m_pObject[rl][nr]->m_fDistanceToCam = 0;
      }
      else
        EF_GetObject(false, Source->m_pObject[rl][nr]->m_Id); 
      CRendElement *pRE = Source->m_pShader->GetREs()->Get(nr);
      float fCustomSort = 0;
      if (pRE->mfGetType() == eDATA_Flare)
      {
        Source->m_pObject[rl][nr]->m_Color.r = Source->m_Color.r;
        Source->m_pObject[rl][nr]->m_Color.g = Source->m_Color.g;
        Source->m_pObject[rl][nr]->m_Color.b = Source->m_Color.b;
        fCustomSort = 4000.0f;
      }
      else
      {
        Source->m_pObject[rl][nr]->m_Color = Source->m_Color;
      }

      if (Source->m_Flags & DLF_SUN)
        Source->m_pObject[rl][nr]->m_ObjFlags |= FOB_DRSUN;
      mathCalcMatrix(Source->m_pObject[rl][nr]->m_Matrix, Source->m_Origin, Source->m_ProjAngles, Vec3d(1,1,1), g_CpuFlags);
      Source->m_pObject[rl][nr]->m_pLight = Source;
      Source->m_pObject[rl][nr]->m_DynLMMask = 1<<Source->m_Id;
      Source->m_pObject[rl][nr]->m_ObjFlags |= FOB_TRANS_MASK;
      Source->m_pObject[rl][nr]->m_InvMatrixId = -1;
      Source->m_pObject[rl][nr]->m_VPMatrixId = -1;
      if((fCamZ-fWaterLevel)*(Source->m_Origin.z-fWaterLevel)>0)
        Source->m_pObject[rl][nr]->m_SortId = -1000000 - fCustomSort;
      else
        Source->m_pObject[rl][nr]->m_SortId = 1000000 - fCustomSort;
      EF_AddEf(0, pRE, Source->m_pShader, NULL, Source->m_pObject[rl][nr], -1, NULL, EFSLIST_DISTSORT);
    }
  }
  //Source->m_Flags &= ~DLF_LIGHTTYPE_MASK;
  //Source->m_Flags |= DLF_DIRECTIONAL;
}

void CRenderer::EF_ClearLightsList()
{
  m_RP.m_DLights[SRendItem::m_RecurseLevel].SetUse(0);
}

bool CRenderer::EF_UpdateDLight(CDLight *dl)
{
  bool bRes = false;
  if (!dl)
    return bRes;

  float fTime = iTimer->GetCurrTime();

  bRes = true;

  SShader *lsh = (SShader *)((IShader*)dl->m_pShader);
  int nStyle = dl->m_nLightStyle;
  if (lsh && lsh->m_EvalLights && dl->m_fLastTime < fTime)
  {
    SLightEval *le = lsh->m_EvalLights;
    float fDelta = fTime - dl->m_fLastTime;
    // Calculating of the projection orientation
    if (dl->m_Flags & DLF_PROJECT)
    {
      float *Angs = le->m_ProjRotate.mfGet();
      Vec3d fa(Angs[0],Angs[1],Angs[2]);
      dl->m_ProjAngles = fa + dl->m_BaseProjAngles;
    }
    // Evaluate light position
  //  if (dl->m_OrigLight && le->m_LightOffset != Vec3d(0,0,0))
    if (IsEquivalent(le->m_LightOffset,Vec3d(0,0,0)))
    {
      Vec3d Angs = le->m_LightRotate * fTime;


      Matrix44 m=ViewMatrix(Angs*gf_DEGTORAD);
      m=GetTranslationMat(le->m_LightOffset)*m;


      dl->m_Origin[0] = dl->m_BaseOrigin[0] + m[3][0];
      dl->m_Origin[1] = dl->m_BaseOrigin[1] + m[3][1];
      dl->m_Origin[2] = dl->m_BaseOrigin[2] + m[3][2];
      dl->m_Flags |= DLF_POSITIONCHANGED;
    }
    if (le->m_LightMove)
    {
      switch (le->m_LightMove->m_eLMType)
      {
        case eLMT_Wave:
          {
            float wf = SEvalFuncs::EvalWaveForm(&le->m_LightMove->m_Wave);
            dl->m_Origin = dl->m_BaseOrigin + le->m_LightMove->m_Dir * wf;
            dl->m_Flags |= DLF_POSITIONCHANGED;
          }
          break;

        case eLMT_Patch:
          {
          }
          break;
      }
    }
    if (!nStyle)
      nStyle = le->m_LightStyle;

    // Update light styles
    if (nStyle>0 && nStyle<CLightStyle::m_LStyles.Num() && CLightStyle::m_LStyles[nStyle])
    {
      CLightStyle *ls = CLightStyle::m_LStyles[nStyle];
      ls->mfUpdate(fTime);
      switch (le->m_EStyleType)
      {
        case eLS_Intensity:
        default:
          dl->m_Color = dl->m_BaseColor * ls->m_fIntensity;
          dl->m_SpecColor = dl->m_BaseSpecColor * ls->m_fIntensity;
          break;
        case eLS_RGB:
          dl->m_Color = ls->m_Color;
          break;
      }
      dl->m_Color.a = 1.0f;
    }

    dl->m_fLastTime = fTime;
  }
  else
  if (nStyle>0 && nStyle<CLightStyle::m_LStyles.Num() && CLightStyle::m_LStyles[nStyle])
  {
    CLightStyle *ls = CLightStyle::m_LStyles[nStyle];
    ls->mfUpdate(fTime);
    dl->m_Color = dl->m_BaseColor * ls->m_fIntensity;
    dl->m_SpecColor = dl->m_BaseSpecColor * ls->m_fIntensity;
  }
  
  if ((dl->m_Flags & DLF_PROJECT))
  {
    //we need to rotate the cubemap to account for the spotlights orientation
    //convert the orienations ortho normal basis (ONB) into XYZ space, and then
    //into the base direction space (using ONB prevents having to calculate angles)
    dl->m_Orientation.m_vForward = Vec3d(1,0,0);
    dl->m_Orientation.m_vUp = Vec3d(0,1,0);
    dl->m_Orientation.m_vRight = Vec3d(0,0,1);
    dl->m_Orientation.rotate(Vec3d(1,0,0), dl->m_ProjAngles.x);
    dl->m_Orientation.rotate(Vec3d(0,1,0), dl->m_ProjAngles.y);
    dl->m_Orientation.rotate(Vec3d(0,0,1), dl->m_ProjAngles.z);

    Matrix44 m = dl->m_Orientation.matrixBasisToXYZ();

    //scale the cubemap to adjust the default 45 degree 1/2 angle fustrum to 
    //the desired angle (0 to 90 degrees)
    float scaleFactor = cry_tanf((90.0f-dl->m_fLightFrustumAngle)*PI/180.0f);

    m=Matrix33::CreateScale( Vec3d(1,scaleFactor,scaleFactor) ) * m;

    m.Transpose();
    dl->m_TextureMatrix = m;      
    
    //translate the vertex relative to the light position

    dl->m_TextureMatrix = GetTranslationMat(-dl->m_Origin) * dl->m_TextureMatrix;      


  }
  return false;
}

void CRenderer::EF_SetWorldColor(float r, float g, float b, float a)
{
  if (!m_bHeatVision)
  {
    m_WorldColor.r = r;
    m_WorldColor.g = g;
    m_WorldColor.b = b;
    m_WorldColor.a = a;
  }
  else
  {
    m_SavedWorldColor.r = r;
    m_SavedWorldColor.g = g;
    m_SavedWorldColor.b = b;
    m_SavedWorldColor.a = a;
  }

//  m_WorldColor = CFColor(1, 1, 1, 1.0f);
}

#ifdef WIN64
#pragma warning( push )             //AMD Port
#pragma warning( disable : 4312 )       // 'type cast' : conversion from 'int' to 'void *' of greater size
#endif
 
static int snLostDevice;

void *CRenderer::EF_Query(int Query, int Param)
{
  switch (Query)
  {
    case EFQ_NUMEFS:
      return (void *)gRenDev->m_cEF.m_Nums;
    case EFQ_LOADEDEFS:
      return (void *)&SShader::m_Shaders_known[0];
    case EFQ_NUMTEXTURES:
      return (void *)m_TexMan->m_Textures.Num();
    case EFQ_LOADEDTEXTURES:
      return (void *)&m_TexMan->m_Textures[0];
    case EFQ_NUMEFFILES0:
      return (void *)gRenDev->m_cEF.m_NumFiles[0];
    case EFQ_NUMEFFILES1:
      return (void *)gRenDev->m_cEF.m_NumFiles[1];
    case EFQ_EFFILENAMES0:
      return (void *)&gRenDev->m_cEF.m_FileNames[0][0];
    case EFQ_EFFILENAMES1:
      return (void *)&gRenDev->m_cEF.m_FileNames[1][0];
    case EFQ_SunFlares:
      return (void *)&CSunFlares::m_SunFlares;
    case EFQ_CurSunFlare:
      return (void *)&CSunFlares::m_CurFlares;
    case EFQ_LightMaterials:
      return (void *)&SLightMaterial::known_materials;
    case EFQ_Pointer2FrameID:
      return (void *)&m_nFrameID;
#if !defined(PS2) && !defined (GC) && !defined (NULL_RENDERER)
    case EFQ_VProgramms:
      return (void *)&CVProgram::m_VPrograms;
#endif      
    case EFQ_D3DDevice:
      return gGet_D3DDevice();
    case EFQ_glReadPixels:
      return gGet_glReadPixels();
    case EFQ_DeviceLost:
      snLostDevice = (int)CheckDeviceLost();
      return (void *)&snLostDevice;
    case EFQ_Orients:
      return (void *)gRenDev->m_cEF.m_Orients;
    case EFQ_NumOrients:
      return (void *)&gRenDev->m_cEF.m_NumOrients;
    case EFQ_RegisteredTemplates:
      return (void *)&gRenDev->m_cEF.m_KnownTemplates;
    case EFQ_RecurseLevel:
      return (void *)SRendItem::m_RecurseLevel;
#if !defined(PS2) && !defined (GC) && !defined (NULL_RENDERER)
    case EFQ_PShaders:
      return (void *)&CPShader::m_PShaders;
#endif      
    case EFQ_LightSource:
      {
        if (m_RP.m_DLights[SRendItem::m_RecurseLevel].Num() > Param)
          return m_RP.m_DLights[SRendItem::m_RecurseLevel][Param];
        return NULL;
      }
      break;
    case EFQ_NumRenderItems:
      {
        int nElements = 0;
        for (int i=0; i<NUMRI_LISTS; i++)
        {
          nElements += SRendItem::m_RendItems[i].Num();
        }
        return (void *)nElements;
      }
      break;
    default:
      assert(0);
  }
  return NULL;
}

#ifdef WIN64
#pragma warning( pop )              //AMD Port
#endif

void CRenderer::EF_ConstructEf(IShader *Ef)
{
  gRenDev->m_cEF.mfConstruct((SShader *)Ef);
}

//================================================================================================================

CLeafBuffer * CRenderer::CreateLeafBuffer(bool bDynamic, const char *szSource, CIndexedMesh * pIndexedMesh)
{
  // make material table with clean elements
  CLeafBuffer * pLeafBuffer = new CLeafBuffer(szSource);
  pLeafBuffer->m_bDynamic = bDynamic;
  pLeafBuffer->m_Indices.m_bDynamic = bDynamic;

  if(pIndexedMesh)
  {
    pLeafBuffer->m_pMats = new list2<CMatInfo>;
    while(pLeafBuffer->m_pMats->Count() < 1/*m_pMesh->m_Materials.Num()*/)
    {
      CMatInfo tmp;
      pLeafBuffer->m_pMats->Add(tmp);
    }

    pLeafBuffer->CreateBuffer(pIndexedMesh, false);
  }
  pLeafBuffer->m_nVertexFormat = VERTEX_FORMAT_P3F;

  return pLeafBuffer;
}

void CRenderer::DeleteLeafBuffer(CLeafBuffer * pLBuffer)
{
  if(pLBuffer)
    delete pLBuffer;
}

// Creates the leaf buffer with the materials, secondary buffer (system buffer)
// indices and perhaps some other stuff initialized.
// NOTE: if the pVertBuffer is NULL, the system buffer doesn't get initialized with any values
// (trash may be in it)
CLeafBuffer * CRenderer::CreateLeafBufferInitialized(
  void * pVertBuffer, int nVertCount, int nVertFormat, 
  ushort * pIndices, int nIndices,
  int nPrimetiveType, const char *szSource, EBufferType eBufType,
  int nMatInfoCount, int nClientTextureBindID,
  bool (*PrepareBufferCallback)(CLeafBuffer *, bool),
  void *CustomData,bool bOnlyVideoBuffer, bool bPrecache)
{
  CLeafBuffer * pLeafBuffer = new CLeafBuffer(szSource);
 
  // make mats info list
  pLeafBuffer->m_pMats = new list2<CMatInfo>;
  pLeafBuffer->m_pMats->PreAllocate(nMatInfoCount);
  pLeafBuffer->m_bMaterialsWasCreatedInRenderer=true;
  pLeafBuffer->m_bDynamic = (eBufType == eBT_Dynamic);
  pLeafBuffer->m_Indices.m_bDynamic = pLeafBuffer->m_bDynamic;
  pLeafBuffer->m_nVertexFormat = nVertFormat;
  pLeafBuffer->m_bOnlyVideoBuffer = bOnlyVideoBuffer;
  pLeafBuffer->m_NumIndices = nIndices;

  pLeafBuffer->m_SecVertCount   = nVertCount;

  // copy vert buffer
  if (pVertBuffer && !PrepareBufferCallback && !bOnlyVideoBuffer)
  {
    pLeafBuffer->m_pSecVertBuffer = new CVertexBuffer(CreateVertexBuffer(nVertFormat, nVertCount),nVertFormat);
    pLeafBuffer->m_pSecVertBuffer->m_NumVerts = nVertCount;
    gRenDev->UpdateBuffer(pLeafBuffer->m_pSecVertBuffer,pVertBuffer,nVertCount,true);
  }
  else
  if (pVertBuffer && bOnlyVideoBuffer && nVertCount)
  {
    pLeafBuffer->CreateVidVertices(nVertCount, nVertFormat);
    pLeafBuffer->UpdateVidVertices(pVertBuffer, nVertCount);
  }

  pLeafBuffer->m_pCustomData = CustomData;
  pLeafBuffer->m_nVertexFormat = nVertFormat;

  pLeafBuffer->PrepareBufferCallback = PrepareBufferCallback;

  if (pIndices)
    pLeafBuffer->UpdateSysIndices(pIndices, nIndices);
  pLeafBuffer->m_nPrimetiveType = nPrimetiveType;

  pLeafBuffer->m_nClientTextureBindID = nClientTextureBindID;

  // Precache for static buffers
  if (CV_r_precachemesh && pLeafBuffer->m_SecVertCount && bPrecache)
  {
    pLeafBuffer->CheckUpdate(nVertFormat, 0, false);
    assert (!pLeafBuffer->m_pVertexBuffer || pLeafBuffer->m_pVertexBuffer->m_NumVerts == pLeafBuffer->m_SecVertCount);
  }
  
  return pLeafBuffer;
}
/*
void CRenderer::AddMatInfoIntoLeafBuffer( CLeafBuffer * pLeafBuffer, 
                                          const char * szEfName, 
                                          int nFirstVertId, int nVertCount, 
                                          int nFirstIndexId, int nIndexCount)
{
  if(!nIndexCount || !nVertCount)
    return;

  // make single chunk
  CMatInfo matinfo;
  matinfo.pShader = EF_LoadShader((char*)szEfName, -1, eEF_World, 0);
  matinfo.pRE = (CREOcLeaf*)EF_CreateRE(eDATA_OcLeaf);
  matinfo.nFirstIndexId = nFirstIndexId;
  matinfo.nNumIndices  = nIndexCount;
  matinfo.nFirstVertId = nFirstVertId;
  matinfo.nNumVerts = nVertCount;
  
  // register this chunk
  pLeafBuffer->m_pMats->Add(matinfo);
  pLeafBuffer->m_pMats->Last().pRE->m_pChunk = &(pLeafBuffer->m_pMats->Last());
  pLeafBuffer->m_pMats->Last().pRE->m_pBuffer = pLeafBuffer;
}*/
/*
CLeafBuffer * CRenderer::CreateLeafBufferInitialized(
  const char * szEfName,
  PipVertex * pVertBuffer, int nVertCount,
  list2<ushort> * pIndices,
  int nPrimetiveType)
{
  CLeafBuffer * pLeafBuffer = new CLeafBuffer();

  // make single chunk
  CMatInfo matinfo;
  matinfo.pShader = EF_LoadShader((char*)szEfName, -1, eEF_World, 0);
  matinfo.pRE = (CREOcLeaf*)EF_CreateRE(eDATA_OcLeaf);
  matinfo.nFirstIndexId = 0;
  matinfo.nNumIndices  = pIndices->Count();
  matinfo.nFirstVertId = 0;
  matinfo.nNumVerts = nVertCount;
  
  // register this chunk
  pLeafBuffer->m_pMats = new list2<CMatInfo>;
  pLeafBuffer->m_pMats->Add(matinfo);

  // copy vert buffer
  pLeafBuffer->m_SecVertCount   = nVertCount;
  pLeafBuffer->m_pSecVertBuffer = new CVertexBuffer;
  pLeafBuffer->m_pSecVertBuffer->m_vertexformat = VERTEX_FORMAT_P3F_N3F_COL4UB_COL4UB_TEX2F_TEX2F_FOG1F_TANGENT_TANNORM;
  pLeafBuffer->m_pSecVertBuffer->m_data = new PipVertex[nVertCount];
  memcpy(pLeafBuffer->m_pSecVertBuffer->m_data, pVertBuffer, sizeof(PipVertex)*nVertCount);

  pLeafBuffer->GetIndices()        = *pIndices;
  pLeafBuffer->m_nPrimetiveType = nPrimetiveType;
  
  return pLeafBuffer;
}*/

void CRenderer::RenderToViewport(const CCamera &cam, float x, float y, float width, float height)
{
  int pX, pY, pWidth, pHeight;

  GetViewport(&pX, &pY, &pWidth, &pHeight);
  ClearDepthBuffer();

  float fx = x / 800.0f * m_width;
  float fy = y / 600.0f * m_height;
  float fwdt = width / 800.0f * m_width;
  float fhgt = height / 600.0f * m_height;

  SetViewport((int)fx, (int)(m_height-fy-fhgt), (int)fwdt, (int)fhgt);

  CCamera prevCamera = GetCamera();  
  iSystem->SetViewCamera((CCamera&)cam);
  gRenDev->SetCamera(cam);

  I3DEngine *eng = (I3DEngine *)iSystem->GetIProcess();
  eng->SetCamera(cam);
  eng->Update();
  eng->DrawLowDetail(0);
  
  iSystem->SetViewCamera(prevCamera);
  SetCamera(prevCamera);

  SetViewport(pX, pY, pWidth, pHeight);
}

AnimTexInfo * CRenderer::GetAnimTexInfoFromId(int nId)
{
  nId--;

  if(nId>=0 && nId<m_LoadedAnimatedTextures.Count())
    return m_LoadedAnimatedTextures[nId];

  return 0;
}

//=======================================================================

void CRenderer::SetWhiteTexture()
{
  m_TexMan->m_Text_White->Set();
}

// used for sprite generation
void CRenderer::SetTextureAlphaChannelFromRGB(byte * pMemBuffer, int nTexSize)
{
  // set alpha channel
  for(int y=0; y<nTexSize; y++)
    for(int x=0; x<nTexSize; x++)
    {
      int t = (x+nTexSize*y)*4;
      if( abs(pMemBuffer[t+0]-pMemBuffer[0+0])<2 && 
          abs(pMemBuffer[t+1]-pMemBuffer[0+1])<2 && 
          abs(pMemBuffer[t+2]-pMemBuffer[0+2])<2 )
        pMemBuffer[t+3] = 0;
      else
        pMemBuffer[t+3] = 255;

      // set border alpha to 0
      if( x==0 || y == 0 || x == nTexSize-1 || y == nTexSize-1 )
        pMemBuffer[t+3] = 0;
    }
}

void CRenderer::Graph(byte *g, int x, int y, int wdt, int hgt, int nC, int type, char *text, CFColor& color, float fScale)
{
  CFColor col;
  Vec3d vp[2048];
  int i;

  Set2DMode(true, m_width, m_height);

  col = Col_Blue;

  if (wdt > 1024)
    wdt = 1024;

  int num = gRenDev->m_TexMan->m_Text_White->GetTextureID();

  float fy = (float)y;
  float fx = (float)x;
  float fwdt = (float)wdt;
  float fhgt = (float)hgt;

  DrawImage(fx, fy, fwdt, 2, num, 0, 0, 1, 1, col.r, col.g, col.b, col.a);
  DrawImage(fx, fy+fhgt, fwdt, 2, num, 0, 0, 1, 1, col.r, col.g, col.b, col.a);
  DrawImage(fx, fy, 2, fhgt, num, 0, 0, 1, 1, col.r, col.g, col.b, col.a);
  DrawImage(fx+fwdt-2, fy, 2, fhgt, num, 0, 0, 1, 1, col.r, col.g, col.b, col.a);

  float fGround = CV_r_graphstyle ? fy+fhgt : -1;

  for (i=0; i<wdt; i++)
  {
    vp[i][0] = (float)i+fx;
    vp[i][1] = fy + (float)(g[i])*fhgt/255.0f;
    vp[i][2] = 0;
  }
  if (type == 1)
  {
    col = color;
    DrawPoints(&vp[0], nC, col, 3);
    col = CFColor(1.0f) - col;
    col[3] = 1;
    DrawPoints(&vp[nC], wdt-nC, col, 3);
  }
  else
  if (type == 2)
  {
    col = color;
    DrawLines(&vp[0], nC, col, 3, fGround);
    col = CFColor(1.0f) - col;
    col[3] = 1;
    DrawLines(&vp[nC], wdt-nC, col, 3, fGround);
  }
  else
  if (type == 3)
  {
    col = color;
    DrawLines(&vp[0], wdt, col, 3, fGround);
  }

  if (text)
  {
    CXFont  *cf = iConsole->GetFont();
    WriteXY(cf,4,y-18, 0.5f,1,1,1,1,1, text);
    WriteXY(cf,wdt-260,y-18, 0.5f,1,1,1,1,1, "%d ms", (int)(1000.0f*fScale));
  }

  Set2DMode(false, 0, 0);
}

//=============================================================================
// Precaching

bool CRenderer::EF_PrecacheResource(CLeafBuffer *pPB, float fDist, float fTimeToReady, int Flags)
{
  int i;
  if (!gRenDev->m_TexMan->m_Streamed)
    return true;

  for (i=0; i<pPB->m_pMats->Count(); i++)
  {
    CMatInfo *mi = &pPB->m_pMats->GetAt(i);
    SRenderShaderResources *pSR = mi->shaderItem.m_pShaderResources;
    if (!pSR)
      continue;
    if (pSR->m_nFrameLoad != m_nFrameID)
    {
      pSR->m_nFrameLoad = m_nFrameID;
      pSR->m_fMinDistanceLoad = 999999.0f;
    }
    else
    if (fDist >= pSR->m_fMinDistanceLoad || gRenDev->m_TexMan->m_Streamed == 2)
      continue;
    pSR->m_fMinDistanceLoad = fDist;
    for (int j=0; j<=pSR->m_nLastTexture; j++)
    {
      if (!pSR->m_Textures[j])
        continue;
      STexPic *tp = pSR->m_Textures[j]->m_TU.m_TexPic;
      if (!tp)
        continue;
      tp->PrecacheAsynchronously(fDist, Flags);
    }
  }
  return true;
}

bool CRenderer::EF_PrecacheResource(CDLight *pLS, float fDist, float fTimeToReady, int Flags)
{
  if (!gRenDev->m_TexMan->m_Streamed)
    return true;

  if (pLS->m_pLightImage)
    pLS->m_pLightImage->PrecacheAsynchronously(fDist, Flags);
  return true;
}

bool CRenderer::EF_PrecacheResource(ITexPic *pTP, float fDist, float fTimeToReady, int Flags)
{
  if (!gRenDev->m_TexMan->m_Streamed)
    return true;

  if (pTP)
    pTP->PrecacheAsynchronously(fDist, Flags);
  return true;
}

bool CRenderer::EF_PrecacheResource(IShader *pSH, float fDist, float fTimeToReady, int Flags)
{
  if (!gRenDev->m_TexMan->m_Streamed)
    return true;

  SShader *pS = (SShader *)pSH->GetTemplate(-1);
  for (int i=0; i<pS->m_Passes.Num(); i++)
  {
    SShaderPass *pSP = &pS->m_Passes[i];
    for (int j=0; j<pSP->m_TUnits.Num(); j++)
    {
      SShaderTexUnit *pSTU = &pSP->m_TUnits[j];
      if (pSTU->m_TexPic && pSTU->m_TexPic->m_Bind > TX_FIRSTBIND)
        pSTU->m_TexPic->PrecacheAsynchronously(fDist, Flags);
    }
  }
  if (pS->m_Sky)
  {
    for (int i=0; i<3; i++)
    {
      if (pS->m_Sky->m_SkyBox[i] && pS->m_Sky->m_SkyBox[i]->m_Bind > TX_FIRSTBIND)
        pS->m_Sky->m_SkyBox[i]->PrecacheAsynchronously(fDist, Flags);
    }
  }

  return true;
}

#include "Textures/dxtlib.h"
#include "Textures/Image/dds.h"
#define DDSD_CAPS   0x00000001l // default
#define DDSD_PIXELFORMAT  0x00001000l
#define DDSD_WIDTH    0x00000004l
#define DDSD_HEIGHT   0x00000002l
#define DDSD_LINEARSIZE   0x00080000l

#define FOURCC_DXT1  (MAKEFOURCC('D','X','T','1'))
#define FOURCC_DXT2  (MAKEFOURCC('D','X','T','2'))
#define FOURCC_DXT3  (MAKEFOURCC('D','X','T','3'))
#define FOURCC_DXT4  (MAKEFOURCC('D','X','T','4'))
#define FOURCC_DXT5  (MAKEFOURCC('D','X','T','5'))

extern  byte *sData;
/*void ReadDTXnFile (DWORD count, void *buffer)
{
  cryMemcpy(buffer, sData, count);
  sData += count;
}*/

bool CRenderer::DXTDecompress(byte *srcData,byte *dstData,int nWidth,int nHeight,ETEX_Format eSrcTF, bool bUseHW, int nDstBytesPerPix)
{
#if !defined(WIN64) && !defined(NULL_RENDERER)
  if (nDstBytesPerPix != 3 && nDstBytesPerPix != 4)
    return false;

  // NOTE: AMD64 port: implement
  if (!bUseHW)
  {
    DDS_HEADER *ddsh;
    int blockSize = (eSrcTF == eTF_DXT1) ? 8 : 16;
    int DXTSize = ((nWidth+3)/4)*((nHeight+3)/4)*blockSize;
    byte *dd = new byte [DXTSize + sizeof(DDS_HEADER) + sizeof(DWORD)];

    DWORD dwMagic = MAKEFOURCC('D','D','S',' ');
    *(DWORD *)dd = dwMagic;
    ddsh = (DDS_HEADER *)&dd[sizeof(DWORD)];
    memset(ddsh, 0, sizeof(DDS_HEADER));
    cryMemcpy(&dd[sizeof(DWORD)+sizeof(DDS_HEADER)], srcData, DXTSize);

    ddsh->dwSize = sizeof(DDS_HEADER);
    ddsh->dwWidth = nWidth;
    ddsh->dwHeight = nHeight;
    ddsh->dwHeaderFlags = DDSD_CAPS | DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_LINEARSIZE;
    ddsh->dwPitchOrLinearSize = nWidth*nHeight*4/blockSize;
    if (eSrcTF == eTF_DXT1)
      ddsh->ddspf.dwFourCC = FOURCC_DXT1;
    else
    if (eSrcTF == eTF_DXT3)
      ddsh->ddspf.dwFourCC = FOURCC_DXT3;
    else
    if (eSrcTF == eTF_DXT5)
      ddsh->ddspf.dwFourCC = FOURCC_DXT5;
    ddsh->ddspf.dwSize = sizeof(ddsh->ddspf);
    ddsh->ddspf.dwFlags = DDS_FOURCC;
    ddsh->dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE;

    sData = dd;
    int planes;
    int lTotalWidth;
    int rowBytes;
    int width;
    int height;
    int src_format;
    byte *_data = nvDXTdecompress(width, height, planes, lTotalWidth, rowBytes, src_format);
    if (planes != nDstBytesPerPix)
    {
      int n = width * height;
      if (planes == 4)
      {
        assert (nDstBytesPerPix == 3);
        byte *data1 = _data;
        byte *dd1 = dstData;

        for (int i=0; i<n; i++)
        {
          dd1[0] = data1[0];
          dd1[1] = data1[1];
          dd1[2] = data1[2];
          dd1   += 3;
          data1 += 4;
        }
      }
      else
      if (planes == 3)
      {
        assert (nDstBytesPerPix == 4);
        byte *data1 = _data;
        byte *dd1 = dstData;

        for (int i=0; i<n; i++)
        {
          dd1[0] = data1[0];
          dd1[1] = data1[1];
          dd1[2] = data1[2];
          dd1[3] = 255;
          dd1   += 4;
          data1 += 3;
        }
      }
    }
    else
      cryMemcpy(dstData, _data, width*height*planes);

    CRTDeleteArray(_data);

    return true;
  }
  else
#endif
  {
    return false;
  }
}

bool CRenderer::DXTCompress( byte *raw_data,int nWidth,int nHeight,ETEX_Format eTF, bool bUseHW, bool bGenMips, int nSrcBytesPerPix, MIPDXTcallback callback)
{
  if(IsBadReadPtr(raw_data, nWidth*nHeight*nSrcBytesPerPix))
  {
    assert(0);
    iLog->Log("Warning: CRenderer::DXTCompress: invalid data passed to the function");
    return false;
  }

#if !defined(WIN64) && !defined(NULL_RENDERER)
  // NOTE: AMD64 port: implement
  if (!bUseHW)
  {
    CompressionOptions opt;
    switch(eTF)
    {
      case eTF_8888:
        opt.TextureFormat = k8888;
        break;
      case eTF_DXT1:
        opt.TextureFormat = kDXT1;
        break;
      case eTF_DXT3:
        opt.TextureFormat = kDXT3;
        break;
      case eTF_DXT5:
        opt.TextureFormat = kDXT5;
        break;
      default:
        assert(0);
        return false;
    }
    opt.MIPFilterType = kMIPFilterQuadratic;
    if (callback)
      opt.MipMapType = dGenerateMipMaps;
    else
      opt.MipMapType = dNoMipMaps;
    nvDXTcompress(raw_data,nWidth,nHeight,nWidth*nSrcBytesPerPix,&opt,nSrcBytesPerPix,(MIPcallback)callback);
  }
  else
#endif
  {
    STexPic ti;
    ti.m_Width = nWidth;
    ti.m_Height = nHeight;
    int i;
    byte *nDst = raw_data;
    if (nSrcBytesPerPix >= 3)
    {
      nDst = new byte[nWidth*nHeight*4];
      for (i=0; i<nWidth*nHeight; i++)
      {
        nDst[i*4+0] = raw_data[i*nSrcBytesPerPix+0];
        nDst[i*4+1] = raw_data[i*nSrcBytesPerPix+1];
        nDst[i*4+2] = raw_data[i*nSrcBytesPerPix+2];
        nDst[i*4+3] = 255;
      }
    }
    int nMips = 0;
    int DXTSize = 0;
    EImFormat eF;
    switch(eTF)
    {
      case eTF_DXT1:
        eF = eIF_DXT1;
        break;
      case eTF_DXT3:
        eF = eIF_DXT3;
        break;
      case eTF_DXT5:
        eF = eIF_DXT5;
        break;
      default:
        assert(0);
        return false;
    }
    byte *data = m_TexMan->GenerateDXT_HW(&ti, eF, nDst, &nMips, &DXTSize, bGenMips);
    if (callback)
    {
      int blockSize = (eTF == eTF_DXT1) ? 8 : 16;
      int nOffs = 0;
      int wdt = nWidth;
      int hgt = nHeight;
      for (i=0; i<nMips; i++)
      {
        if (!wdt)
          wdt = 1;
        if (!hgt)
          hgt = 1;
        int nSize = ((wdt+3)/4)*((hgt+3)/4)*blockSize;
        assert(nSize+nOffs <= DXTSize);
        (*callback)(&data[nOffs], i, nSize, wdt, hgt, NULL);
        nOffs += nSize;
        wdt >>= 1;
        hgt >>= 1;
      }
    }
    delete [] data;
    if (nDst != raw_data)
      delete [] nDst;
  }
  return true;
}

void CRenderer::RemoveAnimatedTexture(AnimTexInfo * pInfo)
{
  if(m_LoadedAnimatedTextures.Find(pInfo)<0)
  {
    assert(!"Attempt to remove invalid animated texture");
    return;
  }

  pInfo->nRefCounter--;

  if(pInfo->nRefCounter)
    return;

  if(m_LoadedAnimatedTextures.Delete(pInfo))
  {
    for(int t=0; t<pInfo->nFramesCount; t++)
      RemoveTexture(pInfo->pBindIds[t]);
    delete [] pInfo->pBindIds;
    delete pInfo;
  }
  else
    assert(0); // texture not found
}

void CRenderer::WriteJPG(byte *dat, int wdt, int hgt, char *name)
{
#ifndef WIN64
  ::WriteJPG(dat, wdt, hgt, name);
#endif
}
