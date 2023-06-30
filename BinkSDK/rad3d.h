#ifndef __RAD3DH__
#define __RAD3DH__

#ifndef __RADBASEH__
#include "radbase.h"
#endif

#ifdef GEKKO

#define RAD3DSURFACE32    0
#define RAD3DSURFACE32A   1
#define RAD3DSURFACE565   2
#define RAD3DSURFACE4444  3 // actually RGB4A3
#define RADSURFACEYUY2    4
#define RAD3DSURFACECOUNT ( RADSURFACEYUY2 + 1 )

#else

#define RAD3DSURFACE32    0
#define RAD3DSURFACE32A   1
#define RAD3DSURFACE555   2
#define RAD3DSURFACE565   3
#define RAD3DSURFACE5551  4
#define RAD3DSURFACE4444  5

#ifdef _XBOX
  #define RAD3DSURFACEYUY2  6
  #define RAD3DSURFACECOUNT ( RAD3DSURFACEYUY2 + 1 )
#else
  #define RAD3DSURFACE32R   6
  #define RAD3DSURFACE32RA  7
  #define RAD3DSURFACE24    8
  #define RAD3DSURFACE24R   9
  #define RAD3DSURFACECOUNT ( RAD3DSURFACE24R + 1 )
#endif

#endif

#ifdef __cplusplus
  #define RADCFUNC extern "C"
#else
  #define RADCFUNC
#endif


#ifdef GEKKO

#define HRAD3D int

#elif _XBOX

#define HRAD3D LPDIRECT3DDEVICE8

#else

//
// Define the handle types.
//

struct RAD3D;
typedef struct RAD3D* HRAD3D;

//
// Functions to open a RAD 3D handle (to OpenGL or Direct3D).
//

RADCFUNC HRAD3D Open_RAD_3D( HWND window );

RADCFUNC void Close_RAD_3D( HRAD3D rad_3D );

RADCFUNC void Resize_RAD_3D( HRAD3D rad_3d,
                             U32 width,
                             U32 height );

RADCFUNC char* Describe_RAD_3D( void );

#endif

RADCFUNC void Start_RAD_3D_frame( HRAD3D rad_3D );

RADCFUNC void End_RAD_3D_frame( HRAD3D rad_3D
#ifdef GEKKO
                                ,S32 swap
#endif
                               );

//
// Define the handle types.
//

struct RAD3DIMAGE;
typedef struct RAD3DIMAGE* HRAD3DIMAGE;

//
// Functions to open a 3D image handle (GL or D3D texture array).
//

RADCFUNC HRAD3DIMAGE Open_RAD_3D_image( HRAD3D rad_3d,
                                        U32 width,
                                        U32 height,
#if defined( _XBOX ) || defined( GEKKO )
                                        U32 rad3d_surface_format
#else
                                        S32 alpha_pixels,
                                        U32 maximum_texture_size RADDEFAULT( 256 )
#endif
                                        );

RADCFUNC void Close_RAD_3D_image( HRAD3DIMAGE rad_image );

RADCFUNC S32 Lock_RAD_3D_image( HRAD3DIMAGE rad_image,
                                void* out_pixel_buffer,
                                U32* out_buffer_pitch,
                                U32* out_surface_type
#if !(defined( _XBOX ) || defined( GEKKO ))
                                ,U32* src_x
                                ,U32* src_y
                                ,U32* src_w
                                ,U32* src_h
#endif
                                );

RADCFUNC void Unlock_RAD_3D_image( HRAD3DIMAGE rad_image );

RADCFUNC void Blit_RAD_3D_image( HRAD3DIMAGE rad_image,
                                 F32 x_offset,
                                 F32 y_offset,
                                 F32 x_scale,
                                 F32 y_scale,
                                 F32 alpha_level );

RADCFUNC void Draw_lines_RAD_3D_image( HRAD3DIMAGE rad_image,
                                       F32 x_offset,
                                       F32 y_offset,
                                       F32 x_scale,
                                       F32 y_scale );


#endif