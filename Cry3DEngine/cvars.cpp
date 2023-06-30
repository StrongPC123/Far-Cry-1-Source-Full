////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   cvars.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: console variables used in 3dengine
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "3dEngine.h"
#include "irenderer.h"

// can not be changed by user
#define INIT_CVAR_CHEAT(_var,_def_val,_comment)\
	(_var) = GetConsole()->Register((#_var), &(_var), (_def_val), VF_CHEAT, _comment);

// can be changed in options menu, will be saved into ini file
#define INIT_CVAR_SER__( _var, _def_val,_comment )\
	( _var ) = GetConsole()->Register( (#_var), &(_var), (_def_val), (e_allow_cvars_serialization ? VF_DUMPTODISK : 0), _comment );

// can be changed in options menu, will be saved into ini file, require level restart
#define INIT_CVAR_SER_R( _var, _def_val,_comment )\
	( _var ) = GetConsole()->Register( (#_var), &(_var), (_def_val), (e_allow_cvars_serialization ? VF_DUMPTODISK : 0)|VF_REQUIRE_LEVEL_RELOAD, _comment );

#define INIT_CVAR_SER_R_NET( _var, _def_val,_comment )\
	( _var ) = GetConsole()->Register( (#_var), &(_var), (_def_val), (e_allow_cvars_serialization ? VF_DUMPTODISK : 0)|VF_REQUIRE_LEVEL_RELOAD|VF_REQUIRE_NET_SYNC, _comment );

// can be changed by game or user
#define INIT_CVAR_PUBL_(_var,_def_val,_comment)\
	(_var) = GetConsole()->Register((#_var), &(_var), (_def_val), 0, _comment);

void CVars::Init()
{
  // Ints
	INIT_CVAR_CHEAT(e_allow_cvars_serialization,	1, "If set to zero - will not save cvars to cfg file");
  INIT_CVAR_CHEAT(e_detail_objects,							1, "Activates drawing of detail objects");
  INIT_CVAR_CHEAT(e_fog,												1, "Activates distance based fog");
  INIT_CVAR_CHEAT(e_motion_blur,								0, "Activates motion blur, values from 1 to 7 will change blur type");
  INIT_CVAR_SER_R(e_beach,											1, "Activates drawing of shore on the border of the ocean");
  INIT_CVAR_CHEAT(e_detail_texture,							1, "Activates drawing of detail textures on terrain ground");
	INIT_CVAR_SER_R(e_detail_texture_quality,			1, "0 - use one single texture per entire level, 1 - use multiple textures");
  INIT_CVAR_CHEAT(e_particles,									1, "Activates drawing of particles");
	INIT_CVAR_CHEAT(e_particles_debug,						0, "Debug");
	INIT_CVAR_SER__(e_particles_max_count,				2048, "Maximum number of particles");
  INIT_CVAR_SER__(e_decals,											1, "Activates drawing of decals (marks from bullets and explosions)");
  INIT_CVAR_CHEAT(e_bflyes,											1, "Activates drawing of butterflies around the camera");
  INIT_CVAR_CHEAT(e_vegetation_bending,					2, "Debug");
  INIT_CVAR_CHEAT(e_vegetation,									1, "Activates drawing of distributed objects like trees");
  INIT_CVAR_CHEAT(e_vegetation_sprites,					1, "Activates drawing of sprites instead of distributed objects at far distance");
  INIT_CVAR_CHEAT(e_entities,										1, "Activates drawing of entities and brushes");
  INIT_CVAR_CHEAT(e_entities_debug,							0, "Debug");
  INIT_CVAR_CHEAT(e_sky_box,										1, "Activates drawing of skybox and moving cloud layers");
  INIT_CVAR_CHEAT(e_terrain,										1, "Activates drawing of terain ground");
  INIT_CVAR_CHEAT(e_terrain_debug,							0, "Debug");
  INIT_CVAR_SER_R(e_shadow_maps,								1, "Activates drawing of shadow maps");
  INIT_CVAR_SER_R(e_shadow_maps_from_static_objects,1, "Activates drawing of shadow maps from distributed objects");
  INIT_CVAR_CHEAT(e_water_ocean,								1, "Activates drawing of ocean");
  INIT_CVAR_CHEAT(e_vegetation_debug,						0, "Debug");
  INIT_CVAR_CHEAT(e_shadow_maps_frustums,				0, "Debug");
  INIT_CVAR_CHEAT(e_terrain_occlusion_culling,	1, "heightmap occlusion culling with time coherency 0=off, 1=on(distant occluders not processed),\n"
																				"4=on(distant occluders still processed but with less precision) ]");
  INIT_CVAR_CHEAT(e_terrain_texture_bind,				1, "Debug");
  INIT_CVAR_CHEAT(e_terrain_log,								0, "Debug");
  INIT_CVAR_CHEAT(e_out_space,									0, "Debug");
  INIT_CVAR_CHEAT(e_sun,												1, "Activates sun light source");
  INIT_CVAR_CHEAT(e_terrain_merge_far_sectors,	1, "Render far heightmap sectors as one mesh");
  INIT_CVAR_CHEAT(e_terrain_texture_mipmaps,		0, "Debug");
  INIT_CVAR_CHEAT(e_timedemo_frames,						0, "Will quit appication in X number of frames, r_DisplayInfo must be also enabled");
  INIT_CVAR_CHEAT(e_timedemo_milliseconds,			0, "Will quit appication in X number of milliseconds");
  INIT_CVAR_CHEAT(e_terrain_texture_pool,				0, "Debug");
  
#ifdef _DEBUG 
  INIT_CVAR_CHEAT(e_cbuffer,										0, "Activates usage of software coverage buffer");
#else
  INIT_CVAR_CHEAT(e_cbuffer,										1, "Activates usage of software coverage buffer");
#endif

  INIT_CVAR_SER_R(e_stencil_shadows,						1, "Activates drawing of shadow volumes");
  INIT_CVAR_CHEAT(e_shadow_maps_debug,					0, "Debug");
  INIT_CVAR_CHEAT(e_dynamic_light,							1, "Activates dynamic light sources");
	INIT_CVAR_CHEAT(e_dynamic_light_exact_vis_test,1, "Use more exact visibility test (based on last draw frame)");
  INIT_CVAR_CHEAT(e_dynamic_light_debug,				0, "Debug");
	INIT_CVAR_CHEAT(e_hw_occlusion_culling_water,	1, "Activates usage of HW occlusion test for ocean");
	INIT_CVAR_CHEAT(e_hw_occlusion_culling_objects,0, "Activates usage of HW occlusion test for objects");
	INIT_CVAR_PUBL_(e_hires_screenshoot,					0, "Writes screenshot to disk, X is scale of image resolution relative to current one, big values can cause crash");
	INIT_CVAR_CHEAT(e_portals,										1, "Activates drawing of visareas content (indoors), values 2,3,4 used for debugging");
	INIT_CVAR_SER__(e_max_entity_lights,					4, "Set maximum number of lights affecting object");
	INIT_CVAR_PUBL_(e_max_shadow_map_size,				512, "Set maximum resolution of shadow map, this value is alos limited by screen resolution in OpenGL");
	INIT_CVAR_CHEAT(e_water_volumes,							1, "Activates drawing of water volumes");
	INIT_CVAR_CHEAT(e_bboxes,											0, "Activates drawing of bounding boxes");
	INIT_CVAR_CHEAT(e_register_in_sectors,				1, "Debug, can cause crash");
	INIT_CVAR_CHEAT(e_stream_cgf,									0, "Debug");
  INIT_CVAR_CHEAT(e_stream_for_physics,					1, "Debug"); 
  INIT_CVAR_CHEAT(e_stream_for_visuals,					1, "Debug"); 
	INIT_CVAR_CHEAT(e_scissor_debug,							0, "Debug");
  INIT_CVAR_CHEAT(e_projector_exact_test,				1, "Debug");
  INIT_CVAR_CHEAT(e_ccgf_load,									0, "Load CCGF if found");
  INIT_CVAR_CHEAT(e_ccgf_make_if_not_found,			0, "Make CCGF if not found");
  INIT_CVAR_CHEAT(e_check_number_of_physicalized_polygons,   1,
    "Activates check (during loading) for number of tris in the objects physics representation\n Current maximum is 100 + overall number of tris divided by 2");
  INIT_CVAR_CHEAT(e_materials,									1, "Activates material support for non animated objects");
  INIT_CVAR_CHEAT(e_vegetation_sprites_slow_switch, 1, "Orient 3d vegetations and sprites to much each other");
  INIT_CVAR_CHEAT(e_terrain_single_pass,				1, "Draw all terrain detail layers in single pass");
  INIT_CVAR_CHEAT(e_light_maps,									1, "Use lightmaps on brushes");
  INIT_CVAR_CHEAT(e_stream_areas,								0, "Stream content of terrain and indoor sectors");
	INIT_CVAR_CHEAT(e_stream_preload_textures,		0, "If texture streaming in renderer enabled - dynamicaly preload textures in x neibhour indoor sectors");
	INIT_CVAR_CHEAT(e_area_merging_distance,			0, "Merge sectror geometry in specified range for rendering speedup");
	INIT_CVAR_CHEAT(e_area_merging_draw_merged_geometry_only,  0, "Do not draw not merged objects");
	INIT_CVAR_CHEAT(e_brushes,										1, "Draw brushes");
  INIT_CVAR_CHEAT(e_brushes_merging_debug,			0, "Print debug info of merged brushes");
  INIT_CVAR_SER_R(e_brushes_merging,						1, "Merge marked brushes during loading");
  INIT_CVAR_CHEAT(e_brushes_onlymerged,					0, "Show only merged brushes");
	INIT_CVAR_CHEAT(e_on_demand_physics,					0, "Turns on on-demand physicalization");
	INIT_CVAR_SER_R(e_light_maps_quality,					1, "define quality for lightmaps (0-2)");
	INIT_CVAR_CHEAT(e_sleep,											0, "Sleep X in C3DEngine::Draw");
	INIT_CVAR_CHEAT(e_objects,										1, "Render or not all objects on terrain");
	INIT_CVAR_CHEAT(e_terrain_draw_this_sector_only, 0, "1 - render only sector where camera is and objects registered in sector 00, 2 - render only sector where camera is");
	INIT_CVAR_CHEAT(e_sun_stencil,								0, "Enable stencil shadows from sun light");
	INIT_CVAR_CHEAT(e_terrain_single_pass_vol_fog,0, "Use single pass volumetric fog on terrain where possible");
	INIT_CVAR_CHEAT(e_occlusion_volumes,					1, "Enable occlusion volumes(antiportals)");
	INIT_CVAR_CHEAT(e_terrain_texture_mip_offset, 0, "Allows to reduce terrain texture resolution by selecting more low mips from texture file");
	INIT_CVAR_SER__(e_overlay_geometry,						1, "Enable rendering of overlay geometry");
	INIT_CVAR_CHEAT(e_player,											1, "Draw main player");
	INIT_CVAR_CHEAT(e_vegetation_sprites_texres,	0, "Sprite texture size modifier. 0 - no scale, 1 - scale 2 times down and so on");
	INIT_CVAR_SER__(e_active_shadow_maps_receving,0, "Allow shadow map receiving from all objects");
	INIT_CVAR_PUBL_(e_shadow_maps_fade_from_light_bit, 1, "Fade shadow maps on terrain if caster is in dark area");
	INIT_CVAR_PUBL_(e_capture_frames,							0, "If set to 1 - will save every frame to tga file");
	INIT_CVAR_PUBL_(e_water_ocean_tesselation,		-1, "0 - default tesselation, 2 - high tesselation, -1 - select automatically depends on per pixel projection support");
	INIT_CVAR_PUBL_(e_shadow_maps_size_ratio,			500,"Controls how shadow map resolution depends from distance to object from camera");
	INIT_CVAR_SER__(e_shadow_spots,								0, "Draw shadow spot under entities");
	INIT_CVAR_SER_R(e_use_global_fog_in_fog_volumes,0, "simulate ocean volume fog using global fog");
	INIT_CVAR_CHEAT(e_precache_level,							1, "Pre-render objects right after level loading");
	INIT_CVAR_PUBL_(e_shadow_maps_max_casters_per_object, 8,  "Maximum number of active shadow casters per object");
	INIT_CVAR_PUBL_(e_water_ocean_sun_reflection, 0,  "Draw sun reflection in the ocean");
	INIT_CVAR_CHEAT(e_objects_fade_on_distance,		1,  "Objects fading out on distance");
	INIT_CVAR_PUBL_(e_area_merging_max_tris_in_input_brush, 512, "Merge only objects containing no more than x triangles");
	INIT_CVAR_SER__(e_stencil_shadows_only_from_strongest_light, 0, "Cast no more than one stencil shadow from object");
	INIT_CVAR_SER_R(e_cgf_load_lods,							1, "Load LOD models for CGFs if found");
	INIT_CVAR_CHEAT(e_optimized_render_object,		1, "test");
	INIT_CVAR_CHEAT(e_stencil_shadows_build_on_load,		1, "Build connectivity during level loading");
	INIT_CVAR_PUBL_(e_vegetation_update_shadow_every_frame, 1, "Allow updating vegetations shadow maps every frame");
	INIT_CVAR_CHEAT(e_particles_receive_shadows, 0, "Enable shadow maps receiving for particles");
	INIT_CVAR_CHEAT(e_light_maps_occlusion, 0, "Enable usage of occlusion maps");
	INIT_CVAR_CHEAT(e_shadow_maps_self_shadowing, 0, "Allow self-shadowing with shadow maps");
	INIT_CVAR_CHEAT(e_voxel_build,								0, "Regenerate voxel world");
	INIT_CVAR_CHEAT(e_voxel_debug,								0, "Draw voxel debug info");
	INIT_CVAR_CHEAT(e_voxel_realtime_light_update,0, "Recalculate voxel terrain vertex colors every frame");
	INIT_CVAR_CHEAT(e_widescreen,									0, "Activate wide screen mode");
	INIT_CVAR_PUBL_(e_shadow_maps_receiving,			1, "Allow shadow maps receiving by brushes, vegetation and entities");

  // Floats
  INIT_CVAR_PUBL_(e_vegetation_sprites_min_distance, 8.0f, "Sets minimal distance when distributed object can be replaced with sprite");
  INIT_CVAR_PUBL_(e_rain_amount,           0.0f, "Values between 0 and 1 controls density of the rain");
  INIT_CVAR_SER__(e_vegetation_sprites_distance_ratio, 1.0f, "Allows changing distance on what vegetation switch into sprite");
	INIT_CVAR_SER__(e_obj_lod_ratio,				10.0f, "LOD for vegetation, brushes and entities");
	INIT_CVAR_CHEAT(e_obj_view_dist_ratio,  55.0f, "View distance for vegetation, brushes and entities");
	INIT_CVAR_CHEAT(e_dynamic_ambient_ratio, 1.0f, "Controls how object ambient level dependinds from surrounding lights");
	INIT_CVAR_PUBL_(e_detail_texture_min_fov,0.55f,"If FOV is less - alternative version of terrain detail texturing will be used");
	INIT_CVAR_SER_R_NET(e_vegetation_min_size,	 0.0f, "Minimal size of static object, smaller objects will be not rendered");
	INIT_CVAR_CHEAT(e_terrain_occlusion_culling_precision, 0.15f, "Density of rays");
	INIT_CVAR_PUBL_(e_terrain_lod_ratio,			1.f, "Set heightmap LOD");
	INIT_CVAR_SER__(e_particles_lod,					1.f, "1 - full LOD, 0.5 - scale frequency and count down twice");
	INIT_CVAR_CHEAT(e_debug_lights,							0, "Use different colors for objects affected by different number of lights\n 0 - back, 1 - blue, 2 - green, 3 or more - red");
	INIT_CVAR_SER__(e_shadow_maps_view_dist_ratio, 10, "View dist ratio for shadow maps");
	INIT_CVAR_SER__(e_decals_life_time_scale,	1.f, "Allows to increase or reduce decals life time for different specs");
	INIT_CVAR_CHEAT(e_obj_min_view_dist,			0.f, "Min distance on what far objects will be culled out");
	INIT_CVAR_CHEAT(e_decals_neighbor_max_life_time, 4.f, "If not zero - new decals will force old decals to fade in X seconds");
	INIT_CVAR_CHEAT(e_explosion_scale,		0.6666f, "Scale size of terrain deformations and explosion decals");

	e_capture_folder = GetConsole()->CreateVariable("e_capture_folder", "CaptureOutput",  0, "Set output folder for video capturing");
	e_capture_file_format = GetConsole()->CreateVariable("e_capture_file_format", "JPG",  0, "Set output image file format for video capturing. Can be JPG or TGA");

	e_deformable_terrain = GetConsole()->CreateVariable("e_deformable_terrain","1",VF_REQUIRE_NET_SYNC,
		"Toggles terrain deforming.\n"
		"Usage: e_deformable_terrain [0/1]\n"
		"Default is 1 (on). Terrain is deformed by explosions. Set\n"
		"to 0 to disable terrain deforming.");

	if(e_water_ocean_tesselation<0)
	{
		ICVar * pVar = GetConsole()->GetCVar("e_water_ocean_tesselation");
		if(GetRenderer()->GetFeatures()&RFT_HW_ENVBUMPPROJECTED)
			pVar->Set(0);
		else
			pVar->Set(1);
	}

	// vars validation
	if(ICVar * pVar0 = GetConsole()->GetCVar("r_Quality_BumpMapping"))
	if(pVar0->GetIVal()==0)
	{
		if(ICVar * pVar1 = GetConsole()->GetCVar("e_shadow_maps"))
			pVar1->Set(0);
		if(ICVar * pVar1 = GetConsole()->GetCVar("e_stencil_shadows"))
			pVar1->Set(0);
	}

  int nGPU = GetRenderer()->GetFeatures() & RFT_HW_MASK;
	if(nGPU == RFT_HW_GF2)
	{
		if(ICVar * pVar1 = GetConsole()->GetCVar("e_detail_texture_quality"))
			pVar1->Set(0);
	}

	if(!(GetRenderer()->GetFeatures() & (RFT_DEPTHMAPS|RFT_SHADOWMAP_SELFSHADOW)))
	{
		if(ICVar * pVar1 = GetConsole()->GetCVar("e_shadow_maps_receiving"))
			pVar1->Set(0);
	}
}