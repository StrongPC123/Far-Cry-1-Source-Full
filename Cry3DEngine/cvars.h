////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   cvars.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _3DENGINE_CVARS_H_
#define _3DENGINE_CVARS_H_

// console variables
struct CVars : public Cry3DEngineBase
{
  CVars() 
  { Init(); }

  void Init();

  int 
		e_allow_cvars_serialization,
    e_shadow_maps,
    e_shadow_maps_debug,
    e_shadow_maps_from_static_objects,
		e_shadow_maps_self_shadowing,
		e_shadow_maps_receiving,
    e_detail_objects,
    e_fog,
    e_motion_blur,
    e_beach,
    e_detail_texture,
		e_detail_texture_quality,
    e_particles,
    e_particles_debug,
		e_particles_max_count,
		e_particles_receive_shadows,
    e_decals,
    e_bflyes,
    e_vegetation_bending,
    e_vegetation,
    e_vegetation_sprites,
    e_entities,
    e_entities_debug,
    e_sky_box,
    e_terrain,
    e_terrain_debug,
    e_water_ocean,
    e_vegetation_debug,
    e_shadow_maps_frustums,
    e_terrain_occlusion_culling,
    e_terrain_texture_pool,
    e_terrain_texture_bind,
    e_terrain_log,
    e_out_space,
    e_video_buffer_stats,
    e_sun,
    e_terrain_merge_far_sectors,
    e_terrain_texture_mipmaps,
		e_terrain_texture_mip_offset,
		
		// if set to non-0, then after the specified number of frames rendered,
		// the game exits printing the average FPS value.
		// NOTE:
		//  for this to work, the r_DisplayInfo must be "1"
    e_timedemo_frames,

		// if set to non-0, then after the specified number of milliseconds of rendering,
		// the game exists unconditionally. This is used for automatic multipass profiling with VTune 6.x
		e_timedemo_milliseconds,
    
		e_cbuffer,
		e_dynamic_light,
		e_dynamic_light_exact_vis_test,
    e_stencil_shadows,
		e_dynamic_light_debug,
		e_hw_occlusion_culling_water,
		e_hw_occlusion_culling_objects,
		e_hires_screenshoot,
		e_portals,
		e_max_entity_lights,
		e_max_shadow_map_size,
		e_water_volumes,
		e_bboxes,
		e_brushes,
    e_brushes_merging_debug,
    e_brushes_merging,
    e_brushes_onlymerged,
		e_register_in_sectors,
    e_stream_cgf,
    e_stream_for_physics,
    e_stream_for_visuals,
    e_stream_areas,
		e_scissor_debug,
    e_projector_exact_test,
    e_ccgf_load,
    e_ccgf_make_if_not_found,
    e_check_number_of_physicalized_polygons,
    e_materials,
    e_vegetation_sprites_slow_switch,
    e_terrain_single_pass,
		e_stream_preload_textures,
		e_area_merging_distance,
		e_area_merging_draw_merged_geometry_only,
		e_area_merging_max_tris_in_input_brush,
		e_on_demand_physics,
		e_light_maps,
		e_light_maps_quality,
		e_light_maps_occlusion,
		e_sleep,
		e_objects,
		e_terrain_draw_this_sector_only,
		e_sun_stencil,
		e_terrain_single_pass_vol_fog,
		e_occlusion_volumes,
		e_overlay_geometry,
		e_player,
		e_vegetation_sprites_texres,
		e_active_shadow_maps_receving,
		e_shadow_maps_fade_from_light_bit,
		e_capture_frames,
		e_water_ocean_tesselation,
		e_shadow_maps_size_ratio,
		e_shadow_spots,
		e_use_global_fog_in_fog_volumes,
		e_precache_level,
		e_shadow_maps_max_casters_per_object,
		e_water_ocean_sun_reflection,
		e_objects_fade_on_distance,
		e_stencil_shadows_only_from_strongest_light,
		e_cgf_load_lods,
		e_optimized_render_object,
		e_stencil_shadows_build_on_load,
		e_vegetation_update_shadow_every_frame,
		e_voxel_build,
		e_voxel_debug,
		e_voxel_realtime_light_update,
		e_widescreen;

  float 
    e_vegetation_sprites_min_distance,
    e_rain_amount,
    e_vegetation_sprites_distance_ratio,
		e_obj_lod_ratio,
		e_obj_view_dist_ratio,
		e_dynamic_ambient_ratio,
		e_detail_texture_min_fov,
		e_terrain_occlusion_culling_precision,
		e_vegetation_min_size,
		e_terrain_lod_ratio,
		e_particles_lod,
		e_debug_lights,
		e_shadow_maps_view_dist_ratio,
		e_decals_life_time_scale,
		e_decals_neighbor_max_life_time,
		e_obj_min_view_dist,
		e_explosion_scale;

	ICVar 
		* e_capture_folder,
		* e_capture_file_format,
		* e_deformable_terrain;	
};
  
#endif // _3DENGINE_CVARS_H_
