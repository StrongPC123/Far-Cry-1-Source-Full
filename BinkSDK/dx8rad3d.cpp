//############################################################################
//##                                                                        ##
//##  D3DRAD3D.CPP                                                          ##
//##                                                                        ##
//##  API to use Direct3D for video textures                                ##
//##                                                                        ##
//##  Author: Jeff Roberts                                                  ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  For technical support, contact RAD Game Tools at 425 - 893 - 4300.    ##
//##                                                                        ##
//############################################################################

#define D3D_OVERLOADS
#include <windows.h>
#include <\devel\libs\dx81\include\d3d8.h> // change to your 8.0 or up path

#include "rad3d.h"

//
// Direct3D RAD3D structure.
//

typedef struct RAD3D
{
  LPDIRECT3D8 direct_3d;
  LPDIRECT3DDEVICE8 direct_3d_device;
  HWND window;
  D3DFORMAT d3d_alpha_surface;
  D3DFORMAT d3d_surface;
  u32 alpha_pixel_size;
  u32 pixel_size;
  s32 rad_alpha_surface;
  s32 rad_surface;

  // Device can use non-POW2 textures if:
  //  1) D3DTEXTURE_ADDRESS is set to CLAMP for this texture's stage
  //  2) D3DRS_WRAP(N) is zero for this texture's coordinates
  //  3) mip mapping is not enabled (use magnification filter only)
  u32 nonpow2_textures_supported;
} RAD3D;

//############################################################################
//##                                                                        ##
//## Verify that a texture format is compatible with a specific             ##
//##    back buffer format.                                                 ##
//##                                                                        ##
//############################################################################

inline u32 is_valid_texture_format(LPDIRECT3D8 direct_3d,
    D3DFORMAT TextureFormat, D3DFORMAT AdapterFormat )
{
    return SUCCEEDED( direct_3d->CheckDeviceFormat( D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL, AdapterFormat, 0, D3DRTYPE_TEXTURE, TextureFormat) );
}


//############################################################################
//##                                                                        ##
//## Return the appropriate texture format given a backbuffer d3dformat.    ##
//##                                                                        ##
//############################################################################

static s32 get_texture_formats(LPDIRECT3D8 direct_3d,
    D3DFORMAT adapter_format, HRAD3D rad_3d )
{
  struct
  {
    D3DFORMAT d3d_format;
    s32 rad_surface;
    u32 pixel_size;
    u32 alpha_pixels;
  } format_list[] =
  {
    { D3DFMT_X8R8G8B8,  RAD3DSURFACE32,   4, 0 },
    { D3DFMT_R8G8B8,    RAD3DSURFACE24,   3, 0 },
    { D3DFMT_R5G6B5,    RAD3DSURFACE565,  2, 0 },
    { D3DFMT_X1R5G5B5,  RAD3DSURFACE555,  2, 0 },
  
    { D3DFMT_A8R8G8B8,  RAD3DSURFACE32A,  4, 1 },
    { D3DFMT_A4R4G4B4,  RAD3DSURFACE4444, 2, 1 },
    { D3DFMT_A1R5G5B5,  RAD3DSURFACE5551, 2, 1 },
  };
  u32 num_formats = sizeof(format_list) / sizeof(format_list[0]);
  u32 format;
  
  //
  // Go through the list of texture formats searching for
  // the chosen one. Or at least one that works.
  //

  for ( format = 0; format < num_formats; format++ )
  {
    if ( is_valid_texture_format(direct_3d,
            format_list[format].d3d_format, adapter_format ) )
    {
      rad_3d->d3d_surface = format_list[format].d3d_format;
      rad_3d->rad_surface = format_list[format].rad_surface;
      rad_3d->pixel_size = format_list[format].pixel_size;
      break;
    }
  }
  
  if( format == num_formats)
    return( 0 );

  //
  // Go through the list of texture formats searching for
  // a supported alpha format.
  //
  
  for ( format = 0; format < num_formats; format++ )
  {
    if ( format_list[format].alpha_pixels )
    {
      if ( is_valid_texture_format ( direct_3d,
              format_list[format].d3d_format, adapter_format ) )
      {
        rad_3d->d3d_alpha_surface = format_list[format].d3d_format;
        rad_3d->rad_alpha_surface = format_list[format].rad_surface;
        rad_3d->alpha_pixel_size = format_list[format].pixel_size;
        break;
      }
    }
  }
  
  if( format == num_formats)
    return( 0 );
  
  return( 1 );
}

//############################################################################
//##                                                                        ##
//## Call this function to actually open Direct3D.                          ##
//##                                                                        ##
//############################################################################

RADCFUNC HRAD3D Open_RAD_3D( HWND window )
{
  D3DDISPLAYMODE d3ddm;
  D3DPRESENT_PARAMETERS d3dpp = {0};
  LPDIRECT3D8 direct_3d;
  LPDIRECT3DDEVICE8 direct_3d_device;
  HRAD3D rad_3d;

  if( !( direct_3d = Direct3DCreate8( D3D_SDK_VERSION ) ) )
  {
    return( 0 );
  }

  if( FAILED( direct_3d->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
  {
    direct_3d->Release();
    return( 0 );
  }

  //
  // try to create a RAD3D structure
  //

  rad_3d = ( HRAD3D ) malloc( sizeof( RAD3D ) );
  if ( rad_3d == 0 )
  {
    direct_3d->Release( );
    return( 0 );
  }

  //
  // get the appropriate texture formats
  //

  if ( !get_texture_formats( direct_3d, d3ddm.Format, rad_3d ) )
  {
    direct_3d->Release( );
    return( 0 );
  }

  d3dpp.hDeviceWindow = window;
  d3dpp.Windowed = TRUE;
  d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  d3dpp.BackBufferFormat = d3ddm.Format;

  if( FAILED( direct_3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,
                                       D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                       &d3dpp, &direct_3d_device ) ) )
  {
    free( rad_3d );
    direct_3d->Release();
    return( 0 );
  }

  //
  // Check if linear (non power 2) textures are supported.
  //

  D3DCAPS8 Caps;

  direct_3d_device->GetDeviceCaps(&Caps);
  rad_3d->nonpow2_textures_supported = !!(Caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL);

  //
  // Now set the structure values.
  //

  rad_3d->direct_3d = direct_3d;
  rad_3d->direct_3d_device = direct_3d_device;
  rad_3d->window = window;

  //
  // And return the handle.
  //

  return( rad_3d );
}


//############################################################################
//##                                                                        ##
//## Call this function to close Direct3D.                                  ##
//##                                                                        ##
//############################################################################

RADCFUNC void Close_RAD_3D( HRAD3D rad_3d )
{
  if ( rad_3d )
  {
    if ( rad_3d->direct_3d )
    {
      rad_3d->direct_3d->Release( );
      rad_3d->direct_3d = 0;
    }

    if ( rad_3d->direct_3d_device )
    {
      rad_3d->direct_3d_device->Release( );
      rad_3d->direct_3d_device = 0;
    }

    free( rad_3d );
  }
}


//############################################################################
//##                                                                        ##
//## Return a string describing this 3D provider.                           ##
//##                                                                        ##
//############################################################################

RADCFUNC char* Describe_RAD_3D( void )
{
  return( "Direct3D8" );
}


//############################################################################
//##                                                                        ##
//## Begin a D3D frame.                                                     ##
//##                                                                        ##
//############################################################################

RADCFUNC void Start_RAD_3D_frame( HRAD3D rad_3d )
{
  if ( rad_3d )
  {
    //
    // Clear the screen.
    //

    rad_3d->direct_3d_device->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );

    //
    // Start the scene
    //

    rad_3d->direct_3d_device->BeginScene();

  }
}


//############################################################################
//##                                                                        ##
//## End a D3D frame.                                                       ##
//##                                                                        ##
//############################################################################

RADCFUNC void End_RAD_3D_frame( HRAD3D rad_3d )
{
  if ( rad_3d )
  {
    //
    // End the rendering.
    //

    rad_3d->direct_3d_device->EndScene();

    rad_3d->direct_3d_device->Present( NULL, NULL, NULL, NULL );
  }
}


//############################################################################
//##                                                                        ##
//## Resize the Direct3D viewport.                                          ##
//##                                                                        ##
//############################################################################

RADCFUNC void Resize_RAD_3D( HRAD3D rad_3d,
                             u32 width,
                             u32 height )
{
  //
  // Make sure there is at least a pixel.
  //

  if ( width < 1 )
  {
    width = 1;
  }

  if ( height < 1 )
  {
    height = 1;
  }

  // Create the viewport
  D3DVIEWPORT8 vp = { 0, 0, width, height, 0.0f, 1.0f };

  rad_3d->direct_3d_device->SetViewport( &vp );
}


//############################################################################
//##                                                                        ##
//## Simple function to round up to the next power of 2.                    ##
//##                                                                        ##
//############################################################################

static u32 Round_up_to_next_2_power( u32 value )
{
  if ( value > 16 )
    if ( value > 64 )
      if ( value > 128 )
        if ( value > 256 )
          if ( value > 512 )
            return( 1024 );
          else
            return( 512 );
        else
          return( 256 );
      else
        return( 128 );
    else
      if ( value > 32 )
        return( 64 );
      else
        return( 32 );
  else
    if ( value > 4 )
      if ( value > 8 )
        return( 16 );
      else
        return( 8 );
    else
      if ( value > 2 )
        return( 4 );
  return( value );
}


//############################################################################
//##                                                                        ##
//## Create a new D3D texture.                                              ##
//##                                                                        ##
//############################################################################

LPDIRECT3DTEXTURE8 Create_texture( LPDIRECT3DDEVICE8 d3d_device,
                                   D3DFORMAT d3d_surface_type,
                                   u32 pitch,
                                   u32 pixel_size,
                                   u32 width,
                                   u32 height )
{
  LPDIRECT3DTEXTURE8 texture = NULL;

  //
  // try to create a dynamic texture first
  //

  if ( SUCCEEDED( d3d_device->CreateTexture(width, height, 1,
                                            D3DUSAGE_DYNAMIC, d3d_surface_type,
                                            D3DPOOL_DEFAULT,
                                            &texture) ) )
  {
    goto gotit;
  }

  //
  // if that fails, create a regular texture
  //

  if ( SUCCEEDED( d3d_device->CreateTexture(width, height, 1,
                                            0, d3d_surface_type,
                                            D3DPOOL_MANAGED,
                                            &texture) ) )
  {
    D3DLOCKED_RECT locked_rect;

   gotit:

    //
    // Clear the texture to black.
    //

    if ( SUCCEEDED( texture->LockRect( 0, &locked_rect, NULL, 0 ) ) )
    {
      //
      // Copy over the pixels.
      //

      u8* dest = ( u8* ) locked_rect.pBits;
      u32 bytes = width * pixel_size;

      for ( u32 y = 0 ; y < height ; y++ )
      {
        memset( dest, 0, bytes );
        dest += locked_rect.Pitch;
      }

      //
      // Unlock the DirectX texture.
      //

      texture->UnlockRect( 0 );
    }
  }

  return texture;
}

//############################################################################
//##                                                                        ##
//## Structure to contain a RAD 3D image.                                   ##
//##                                                                        ##
//############################################################################

typedef struct RAD3DIMAGE
{
  // members used by linear and pow2 routines
  LPDIRECT3DDEVICE8 direct_3d_device;
  D3DFORMAT d3d_surface_type;
  u32 rad_surface_type;
  s32 alpha_pixels;
  u32 pixel_size;
  u32 width;
  u32 height;
  u32 texture_count;
  s32 lin_texture;

  // members used by pow2 routines only
  u32 pitch;
  u32 total_textures;
  u32 textures_across;
  u32 textures_down;
  u32 remnant_width;
  u32 remnant_height;
  u32 remnant_input_width;
  u32 remnant_input_height;
  u32 maximum_texture_size;
  LPDIRECT3DTEXTURE8* d3d_textures;

  // member used by linear routines only
  LPDIRECT3DTEXTURE8 texture;

} RAD3DIMAGE;

/*
 * Vertex structure
 */

typedef struct _D3DTLVERTEX {
    float    sx;            /* Screen coordinates */
    float    sy;
    float    sz;
    float    rhw;           /* Reciprocal of homogeneous w */
    D3DCOLOR color;         /* Vertex color */
    D3DCOLOR specular;      /* Specular component of vertex */
    float    tu;            /* Texture coordinates */
    float    tv;
} D3DTLVERTEX, *LPD3DTLVERTEX;

#define D3DFVF_TLVERTEX ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 )

//############################################################################
//##                                                                        ##
//## Submit the vertices for one texture.                                   ##
//##                                                                        ##
//############################################################################

static void Submit_vertices( LPDIRECT3DDEVICE8 d3d_device,
                             f32 dest_x,
                             f32 dest_y,
                             f32 scale_x,
                             f32 scale_y,
                             s32 width,
                             s32 height,
                             f32 alpha_level )
{
  D3DTLVERTEX vertices[ 4 ];

  //
  // Setup up the vertices.
  //

  vertices[ 0 ].sx = dest_x;
  vertices[ 0 ].sy = dest_y;
  vertices[ 0 ].sz = 0.0F;
  vertices[ 0 ].rhw = 1.0F;
  vertices[ 0 ].color = ( ( s32 ) ( ( alpha_level * 255.0F ) ) << 24 ) | 0xffffff;
  vertices[ 0 ].specular = vertices[ 0 ].color;
  vertices[ 0 ].tu = 0.0F;
  vertices[ 0 ].tv = 0.0F;

  vertices[ 1 ] = vertices[ 0 ];

  vertices[ 1 ].sx = dest_x + ( scale_x * ( f32 ) width );
  vertices[ 1 ].tu = 1.0F;

  vertices[ 2 ] = vertices[0];

  vertices[ 2 ].sy = dest_y + ( scale_y * ( f32 ) height );
  vertices[ 2 ].tv = 1.0F;

  vertices[ 3 ] = vertices[ 1 ];

  vertices[ 3 ].sy = vertices[ 2 ].sy;
  vertices[ 3 ].tv = 1.0F;

  //
  // Draw the vertices.
  //

  d3d_device->SetVertexShader(D3DFVF_TLVERTEX);
  d3d_device->DrawPrimitiveUP(
    D3DPT_TRIANGLESTRIP,
    2,
    vertices,
    sizeof(vertices[0]));
}

//############################################################################
//##                                                                        ##
//## Submit the lines to outline a texture.                                 ##
//##                                                                        ##
//############################################################################

static void Submit_lines( LPDIRECT3DDEVICE8 d3d_device,
                          f32 dest_x,
                          f32 dest_y,
                          f32 scale_x,
                          f32 scale_y,
                          s32 width,
                          s32 height )
{
  D3DTLVERTEX points[ 12 ];

  //
  // Setup the line coordinates.
  //

  points[ 0 ].sx = dest_x;
  points[ 0 ].sy = dest_y;
  points[ 0 ].sz = 0.0F;
  points[ 0 ].rhw = 1.0F;
  points[ 0 ].color = 0xffffffff;
  points[ 0 ].specular = 0xffffffff;
  points[ 0 ].tu = 0.0F;
  points[ 0 ].tv = 0.0F;

  points[ 1 ] = points[ 0 ];

  points[ 1 ].sx = dest_x + ( scale_x * ( f32 ) width );
  points[ 1 ].sy = dest_y ;

  points[ 2 ] = points[ 1 ];
  points[ 3 ] = points[ 1 ];

  points[ 3 ].sy = dest_y + ( scale_y * ( f32 ) height );

  points[ 4 ] = points[ 3 ];
  points[ 5 ] = points[ 3 ];

  points[ 5 ].sx = dest_x;

  points[ 6 ] = points[ 5 ];
  points[ 7 ] = points[ 0 ];

  points[ 8 ] = points[ 0 ];
  points[ 9 ] = points[ 3 ];

  points[ 10 ] = points[ 2 ];
  points[ 11 ] = points[ 6 ];

  points[ 8 ].color = 0xffff0000;
  points[ 8 ].specular = 0xffff0000;
  points[ 9 ].color = 0xffff0000;
  points[ 9 ].specular = 0xffff0000;
  points[ 10 ].color = 0xffff0000;
  points[ 10 ].specular = 0xffff0000;
  points[ 11 ].color = 0xffff0000;
  points[ 11 ].specular = 0xffff0000;

  //
  // Turn off texturing.
  //

  d3d_device->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
  d3d_device->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

  //
  // Draw the lines.
  //

  d3d_device->SetVertexShader(D3DFVF_TLVERTEX);
  d3d_device->DrawPrimitiveUP(
    D3DPT_LINELIST,
    6,
    points,
    sizeof(points[0]));
}

//############################################################################
//##                                                                        ##
//## Set up the appropriate renderstates for texture.                       ##
//##                                                                        ##
//############################################################################

static void Setup_Renderstate( HRAD3DIMAGE rad_image, f32 alpha_level )
{

  //
  // If alpha is disabled and there is no texture alpha, turn alpha off.
  //

  if ( ( alpha_level >= (1.0F-0.0001) ) && ( ! rad_image->alpha_pixels ) )
  {
    rad_image->direct_3d_device->SetRenderState( D3DRS_ALPHABLENDENABLE, 0 );
    rad_image->direct_3d_device->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    rad_image->direct_3d_device->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    rad_image->direct_3d_device->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
  }
  else
  {
    rad_image->direct_3d_device->SetRenderState( D3DRS_ALPHABLENDENABLE, 1 );
    rad_image->direct_3d_device->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
    rad_image->direct_3d_device->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
    rad_image->direct_3d_device->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    rad_image->direct_3d_device->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    rad_image->direct_3d_device->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    rad_image->direct_3d_device->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
  }
}

/*
 * Power 2 routines for devices that only support pow2 textures.
 */

//############################################################################
//##                                                                        ##
//## Open a RAD 3D image (a data structure to blit an image through D3D).   ##
//##                                                                        ##
//############################################################################

RADCFUNC HRAD3DIMAGE Open_RAD_3D_image_pow2( HRAD3D rad_3d,
                                             u32 width,
                                             u32 height,
                                             s32 alpha_pixels,
                                             u32 maximum_texture_size )
{
  HRAD3DIMAGE rad_image;
  u32 remnant_width, remnant_height;
  u32 buffer_pitch, buffer_height;
  u32 pitch, pixel_size;
  u32 total_textures;
  LPDIRECT3DTEXTURE8* textures;
  u32 x, y, i;

  //
  // Calculate the pixel size and the pitch
  //

  pixel_size = ( alpha_pixels ) ? rad_3d->alpha_pixel_size : rad_3d->pixel_size;
  pitch=( ( width * pixel_size ) + 15 ) & ~15;

  //
  // Calculate the remnant size (for the width and the height)
  //

  remnant_width = Round_up_to_next_2_power( width % maximum_texture_size );
  remnant_height = Round_up_to_next_2_power( height % maximum_texture_size );

  //
  // The buffer_pitch is the greater of the remnant size and the input pitch.
  //

  buffer_pitch = remnant_width * pixel_size;
  if ( buffer_pitch < pitch )
  {
    buffer_pitch = pitch;
  }

  //
  // The buffer_height is the greater of the remnant size and the input height.
  //

  buffer_height = ( height > remnant_height) ? height : remnant_height;

  //
  // Calculate the total number of textures we'll need.
  //

  total_textures = ( ( width + ( maximum_texture_size - 1 ) ) / maximum_texture_size ) *
                   ( ( height + ( maximum_texture_size - 1 ) ) / maximum_texture_size );

  //
  // Allocate enough memory for a RAD image, a list of textures and a buffer.
  //

  rad_image = ( HRAD3DIMAGE ) malloc( sizeof( RAD3DIMAGE ) +
                                      ( total_textures * 4 ) );
  if ( rad_image == 0 )
  {
    return( 0 );
  }

  memset( rad_image, 0, sizeof( RAD3DIMAGE ) );

  //
  // The textures come after the structure.
  //

  rad_image->d3d_textures = ( LPDIRECT3DTEXTURE8 * ) ( rad_image + 1 );

  //
  // Set all the variables in our new structure.
  //

  rad_image->direct_3d_device = rad_3d->direct_3d_device;
  rad_image->total_textures = total_textures;
  rad_image->pitch = pitch;
  rad_image->width = width;
  rad_image->height = height;
  rad_image->alpha_pixels = alpha_pixels;
  rad_image->pixel_size = pixel_size;
  rad_image->textures_across = width / maximum_texture_size;
  rad_image->textures_down = height / maximum_texture_size;
  rad_image->remnant_width = remnant_width;
  rad_image->remnant_height = remnant_height;
  rad_image->remnant_input_width = width % maximum_texture_size;
  rad_image->remnant_input_height = height % maximum_texture_size;
  rad_image->maximum_texture_size = maximum_texture_size;
  rad_image->rad_surface_type = ( alpha_pixels ) ?
                                  rad_3d->rad_alpha_surface :
                                  rad_3d->rad_surface;
  rad_image->d3d_surface_type = ( alpha_pixels ) ?
                                  rad_3d->d3d_alpha_surface :
                                  rad_3d->d3d_surface;

  //
  // Loop through and init each texture (setting each of their sizes).
  //

  textures = rad_image->d3d_textures;

  for (y = 0; y < rad_image->textures_down ; y++ )
  {
    for ( x = 0 ; x < rad_image->textures_across ; x++ )
    {
      //
      // Create the texture.
      //

      *textures++ = Create_texture( rad_3d->direct_3d_device,
                                    rad_image->d3d_surface_type,
                                    rad_image->pitch,
                                    rad_image->pixel_size,
                                    rad_image->maximum_texture_size,
                                    rad_image->maximum_texture_size );
    }

    //
    // Do the rememnant texture at the end of the scanline.
    //

    if ( rad_image->remnant_width )
    {
      //
      // Create the texture.
      //

      *textures++ = Create_texture( rad_3d->direct_3d_device,
                                    rad_image->d3d_surface_type,
                                    rad_image->pitch,
                                    rad_image->pixel_size,
                                    rad_image->remnant_width,
                                    rad_image->maximum_texture_size );
    }
  }

  //
  // Do the remnants along the bottom edge (if any).
  //

  if ( rad_image->remnant_height )
  {
    for ( x = 0 ; x < rad_image->textures_across ; x++ )
    {
      //
      // Create the texture.
      //

      *textures++ = Create_texture( rad_3d->direct_3d_device,
                                    rad_image->d3d_surface_type,
                                    rad_image->pitch,
                                    rad_image->pixel_size,
                                    rad_image->maximum_texture_size,
                                    rad_image->remnant_height );

    }
    if ( rad_image->remnant_width )
    {
      //
      // Create the texture.
      //

      *textures++ = Create_texture( rad_3d->direct_3d_device,
                                    rad_image->d3d_surface_type,
                                    rad_image->pitch,
                                    rad_image->pixel_size,
                                    rad_image->remnant_width,
                                    rad_image->remnant_height );

    }
  }

  //
  // Now make sure all of the textures were successfully created,
  //  if not, just free them all.
  //

  for ( i = 0 ; i < rad_image->total_textures ; i++ )
  {
    if ( rad_image->d3d_textures[ i ] == 0 )
    {
      //
      // One of the textures wasn't created, so hose them all.
      //

      for ( i = 0 ; i < rad_image->total_textures ; i++ )
      {
        if ( rad_image->d3d_textures[ i ] )
        {
          rad_image->d3d_textures[ i ]->Release();
          rad_image->d3d_textures[ i ] = 0;
        }
      }

      //
      // Free our memory and return.
      //

      free( rad_image );
      return( 0 );

    }

  }

  rad_image->lin_texture = 0;

  return( rad_image );
}

//############################################################################
//##                                                                        ##
//## Closes a RAD 3D image (frees textures and memory).                     ##
//##                                                                        ##
//############################################################################

RADCFUNC void Close_RAD_3D_image_pow2( HRAD3DIMAGE rad_image )
{
  if ( rad_image->d3d_textures )
  {
    u32 i;

    //
    // Hose all our textures.
    //

    for ( i = 0 ; i < rad_image->total_textures ; i++ )
    {
      if ( rad_image->d3d_textures[ i ] )
      {
        rad_image->d3d_textures[ i ]->Release();
      }
    }

    rad_image->d3d_textures = 0;

    //
    // Free our memory.
    //

    free( rad_image );
  }
}

//############################################################################
//##                                                                        ##
//## Lock a RAD 3D image and return the buffer address and pitch.           ##
//##                                                                        ##
//############################################################################

RADCFUNC s32 Lock_RAD_3D_image_pow2( HRAD3DIMAGE rad_image,
                                     void* out_pixel_buffer,
                                     u32* out_buffer_pitch,
                                     u32* out_surface_type,
                                     u32* src_x,
                                     u32* src_y,
                                     u32* src_w,
                                     u32* src_h )
{
  //
  // Fill the variables that were requested.
  //

  D3DLOCKED_RECT locked_rect;

  if ( rad_image->texture_count < rad_image->total_textures )
  {
    //
    // Lock the DirectX texture.
    //

    if ( ! SUCCEEDED( rad_image->d3d_textures[ rad_image->texture_count ]->LockRect( 0, &locked_rect, NULL, 0 ) ) )
    {
      goto err;
    }

    if ( out_pixel_buffer )
    {
      *( void** )out_pixel_buffer = locked_rect.pBits;
    }

    if ( out_buffer_pitch )
    {
      *out_buffer_pitch = locked_rect.Pitch;
    }

    if ( out_surface_type )
    {
      *out_surface_type = rad_image->rad_surface_type;
    }

    U32 acc = rad_image->textures_across + ( rad_image->remnant_width ? 1 : 0 );
    U32 y = rad_image->texture_count / acc;
    U32 x = rad_image->texture_count - ( acc * y );

    if ( src_x )
    {
      *src_x = ( x * rad_image->maximum_texture_size );
    }

    if ( src_y )
    {
      *src_y = ( y * rad_image->maximum_texture_size );
    }

    if ( src_w )
    {
      *src_w = ( x >= rad_image->textures_across ) ?
                 rad_image->remnant_width : rad_image->maximum_texture_size;
    }

    if ( src_h )
    {
      *src_h = ( y >= rad_image->textures_down ) ?
                 rad_image->remnant_height : rad_image->maximum_texture_size;
    }

    ++( rad_image->texture_count );

    return( 1 );
  }
  else
  {
   err:
    rad_image->texture_count = 0;

    return( 0 );
  }
}

//############################################################################
//##                                                                        ##
//## Unlock a RAD 3D image (mark it for redownloading next frame).          ##
//##                                                                        ##
//############################################################################

RADCFUNC void Unlock_RAD_3D_image_pow2( HRAD3DIMAGE rad_image )
{
  //
  // Unlock the DirectX texture.
  //

  rad_image->d3d_textures[ rad_image->texture_count - 1 ]->UnlockRect( 0 );
}

//############################################################################
//##                                                                        ##
//## Blit a 3D image onto the render target.                                ##
//##                                                                        ##
//############################################################################

RADCFUNC void Blit_RAD_3D_image_pow2( HRAD3DIMAGE rad_image,
                                      f32 x_offset,
                                      f32 y_offset,
                                      f32 x_scale,
                                      f32 y_scale,
                                      f32 alpha_level )
{
  LPDIRECT3DTEXTURE8* textures;
  u8* pixels;
  u32 x, y;
  f32 dest_x, dest_y;
  s32 x_skip, y_skip;
  f32 adjust_x, adjust_y;

  //
  // Setup appropriate texture renderstates.
  //

  Setup_Renderstate( rad_image, alpha_level );

  //
  // Now loop through all of our textures, submitting them.
  //

  textures = rad_image->d3d_textures;

  //
  // Calculate how many bytes to move to the next texture block in X.
  //

  x_skip = ( rad_image->maximum_texture_size * rad_image->pixel_size );

  //
  // Calculate how many bytes to move to the next texture block in Y.
  //

  y_skip = ( rad_image->pitch *
             rad_image->maximum_texture_size ) -
           ( rad_image->textures_across *
             rad_image->maximum_texture_size *
             rad_image->pixel_size );

  adjust_x = ( x_scale * ( f32 ) rad_image->maximum_texture_size );
  adjust_y = ( y_scale * ( f32 ) rad_image->maximum_texture_size );

  dest_y = y_offset;

  for ( y = 0 ; y < rad_image->textures_down ; y++ )
  {

    dest_x = x_offset;

    for ( x = 0 ; x < rad_image->textures_across ; x++ )
    {
      //
      // Select the proper texture.
      //

      rad_image->direct_3d_device->SetTexture( 0, *textures );

      ++textures;

      //
      // Submit the vertices.
      //

      Submit_vertices( rad_image->direct_3d_device,
                       dest_x,
                       dest_y,
                       x_scale,
                       y_scale,
                       rad_image->maximum_texture_size,
                       rad_image->maximum_texture_size,
                       alpha_level );

      dest_x += adjust_x;

      pixels += x_skip;
    }

    //
    // Handle the right side remnant (if any).
    //

    if ( rad_image->remnant_width )
    {
      //
      // Select the proper texture.
      //

      rad_image->direct_3d_device->SetTexture( 0, *textures );

      ++textures;

      //
      // Submit the vertices.
      //

      Submit_vertices( rad_image->direct_3d_device,
                       dest_x,
                       dest_y,
                       x_scale,
                       y_scale,
                       rad_image->remnant_width,
                       rad_image->maximum_texture_size,
                       alpha_level );


    }

    dest_y += adjust_y;

    pixels += y_skip;
  }

  //
  // Handle the bottom row remnants (if any).
  //

  if ( rad_image->remnant_height )
  {
    dest_x = x_offset;

    for ( x = 0 ; x < rad_image->textures_across ; x++ )
    {
      //
      // Select the proper texture.
      //

      rad_image->direct_3d_device->SetTexture( 0, *textures );

      ++textures;

      //
      // Submit the vertices.
      //

      Submit_vertices( rad_image->direct_3d_device,
                       dest_x,
                       dest_y,
                       x_scale,
                       y_scale,
                       rad_image->maximum_texture_size,
                       rad_image->remnant_height,
                       alpha_level );

      dest_x += adjust_x;

      pixels += x_skip;
    }

    if ( rad_image->remnant_width )
    {
      //
      // Select the proper texture.
      //

      rad_image->direct_3d_device->SetTexture( 0, *textures );

      ++textures;

      //
      // Submit the vertices.
      //

      Submit_vertices( rad_image->direct_3d_device,
                       dest_x,
                       dest_y,
                       x_scale,
                       y_scale,
                       rad_image->remnant_width,
                       rad_image->remnant_height,
                       alpha_level );
    }
  }
}


//############################################################################
//##                                                                        ##
//## Draw the edges of each texture for debugging purposes.                 ##
//##                                                                        ##
//############################################################################

RADCFUNC void Draw_lines_RAD_3D_image_pow2( HRAD3DIMAGE rad_image,
                                            f32 x_offset,
                                            f32 y_offset,
                                            f32 x_scale,
                                            f32 y_scale  )
{
  u32 x, y;
  f32 dest_x, dest_y;
  f32 adjust_x, adjust_y;

  adjust_x = ( x_scale * ( f32 ) rad_image->maximum_texture_size );
  adjust_y = ( y_scale * ( f32 ) rad_image->maximum_texture_size );

  dest_y = y_offset;

  for ( y = 0 ; y < rad_image->textures_down ; y++ )
  {

    dest_x = x_offset;

    for ( x = 0 ; x < rad_image->textures_across ; x++ )
    {
      //
      // Submit the lines.
      //

      Submit_lines( rad_image->direct_3d_device,
                    dest_x,
                    dest_y,
                    x_scale,
                    y_scale,
                    rad_image->maximum_texture_size,
                    rad_image->maximum_texture_size );

      dest_x += adjust_x;
    }

    //
    // Handle the right side remnant (if any).
    //

    if ( rad_image->remnant_width )
    {
      //
      // Submit the lines.
      //

      Submit_lines( rad_image->direct_3d_device,
                    dest_x,
                    dest_y,
                    x_scale,
                    y_scale,
                    rad_image->remnant_width,
                    rad_image->maximum_texture_size );

    }

    dest_y += adjust_y;
  }

  //
  // Handle the bottom row remnants (if any).
  //

  if ( rad_image->remnant_height )
  {
    dest_x = x_offset;

    for ( x = 0 ; x < rad_image->textures_across ; x++ )
    {
      //
      // Submit the lines.
      //

      Submit_lines( rad_image->direct_3d_device,
                    dest_x,
                    dest_y,
                    x_scale,
                    y_scale,
                    rad_image->maximum_texture_size,
                    rad_image->remnant_height );

      dest_x += adjust_x;
    }

    if ( rad_image->remnant_width )
    {
      //
      // Submit the lines.
      //

      Submit_lines( rad_image->direct_3d_device,
                    dest_x,
                    dest_y,
                    x_scale,
                    y_scale,
                    rad_image->remnant_width,
                    rad_image->remnant_height );
    }
  }
}

/*
 * Linear routines for devices that support non-power 2 textures.
 */

//############################################################################
//##                                                                        ##
//## Blit a 3D image onto the render target.                                ##
//##                                                                        ##
//############################################################################

RADCFUNC void Blit_RAD_3D_image_lin( HRAD3DIMAGE rad_image,
                                     f32 x_offset,
                                     f32 y_offset,
                                     f32 x_scale,
                                     f32 y_scale,
                                     f32 alpha_level )
{

  //
  // Setup appropriate texture renderstates.
  //

  Setup_Renderstate( rad_image, alpha_level );

  //
  // Select our texture.
  //
  rad_image->direct_3d_device->SetTexture( 0, rad_image->texture );

  //
  // Submit the vertices.
  //

  Submit_vertices( rad_image->direct_3d_device,
                   x_offset,
                   y_offset,
                   x_scale,
                   y_scale,
                   rad_image->width,
                   rad_image->height,
                   alpha_level );
}

//############################################################################
//##                                                                        ##
//## Draw the edges of the texture for debugging purposes.                  ##
//##                                                                        ##
//############################################################################

RADCFUNC void Draw_lines_RAD_3D_image_lin( HRAD3DIMAGE rad_image,
                                           f32 x_offset,
                                           f32 y_offset,
                                           f32 x_scale,
                                           f32 y_scale  )
{
  //
  // Submit the lines.
  //

  Submit_lines( rad_image->direct_3d_device,
                x_offset,
                y_offset,
                x_scale,
                y_scale,
                rad_image->width,
                rad_image->height );
}

//############################################################################
//##                                                                        ##
//## Lock a RAD 3D image and return the buffer address and pitch.           ##
//##                                                                        ##
//############################################################################

RADCFUNC s32 Lock_RAD_3D_image_lin( HRAD3DIMAGE rad_image,
                                    void * out_pixel_buffer,
                                    u32 * out_buffer_pitch,
                                    u32 * out_surface_type,
                                    u32 * src_x,
                                    u32 * src_y,
                                    u32 * src_w,
                                    u32 * src_h )
{
  //
  // Fill the variables that were requested.
  //

  if ( rad_image->texture_count == 0 )
  {
    if ( out_pixel_buffer || out_buffer_pitch )
    {
      D3DLOCKED_RECT locked_rect;

      if ( FAILED( rad_image->texture->LockRect( 0, &locked_rect, NULL, 0 ) ) )
      {
        return 0;
      }

      if ( out_pixel_buffer )
      {
        *( void** )out_pixel_buffer = locked_rect.pBits;
      }

      if ( out_buffer_pitch )
      {
        *out_buffer_pitch = locked_rect.Pitch;
      }

      if ( src_x )
      {
        *src_x = 0;
      }

      if ( src_y )
      {
        *src_y = 0;
      }

      if ( src_w )
      {
        *src_w = rad_image->width;
      }

      if ( src_h )
      {
        *src_h = rad_image->height;
      }
    }

    if ( out_surface_type )
    {
      *out_surface_type = rad_image->rad_surface_type;
    }

    rad_image->texture_count = 1;

    return( 1 );
  }
  else
  {
    rad_image->texture_count = 0;

    return( 0 );
  }
}

//############################################################################
//##                                                                        ##
//## Unlock a RAD 3D image (mark it for redownloading next frame).          ##
//##                                                                        ##
//############################################################################

RADCFUNC void Unlock_RAD_3D_image_lin( HRAD3DIMAGE rad_image )
{
  //
  // Unlock our texture.
  //

  rad_image->texture->UnlockRect( 0 );
}

//############################################################################
//##                                                                        ##
//## Open a RAD 3D image (a data structure to blit an image through D3D).   ##
//##                                                                        ##
//############################################################################

RADCFUNC HRAD3DIMAGE Open_RAD_3D_image_lin( HRAD3D rad_3d,
                                            u32 width,
                                            u32 height,
                                            s32 alpha_pixels,
                                            u32 maximum_texture_size )
{
  HRAD3DIMAGE rad_image;
  u32 pixel_size;

  //
  // Allocate enough memory for a RAD image.
  //

  rad_image = ( HRAD3DIMAGE ) malloc( sizeof( RAD3DIMAGE ));
  if ( rad_image == 0 )
  {
    return( 0 );
  }

  memset( rad_image, 0, sizeof( RAD3DIMAGE ) );

  //
  // Calculate the pixel size
  //

  pixel_size = ( alpha_pixels ) ? rad_3d->alpha_pixel_size : rad_3d->pixel_size;

  //
  // Set all the variables in our new structure.
  //

  rad_image->direct_3d_device = rad_3d->direct_3d_device;
  rad_image->width = width;
  rad_image->height = height;
  rad_image->alpha_pixels = alpha_pixels;
  rad_image->pixel_size = pixel_size;
  rad_image->rad_surface_type = ( alpha_pixels ) ?
                                  rad_3d->rad_alpha_surface :
                                  rad_3d->rad_surface;
  rad_image->d3d_surface_type = ( alpha_pixels ) ?
                                  rad_3d->d3d_alpha_surface :
                                  rad_3d->d3d_surface;
  rad_image->texture_count = 0;

  //
  // Create the texture.
  //

  rad_image->texture = Create_texture( rad_3d->direct_3d_device,
                                       rad_image->d3d_surface_type,
                                       rad_image->pitch,
                                       rad_image->pixel_size,
                                       width,
                                       height );

  if ( rad_image->texture == 0 )
  {
      free( rad_image );
      rad_image = 0;
  }

  rad_image->lin_texture = 1;

  return rad_image;
}

//############################################################################
//##                                                                        ##
//## Closes a RAD 3D image (frees textures and memory).                     ##
//##                                                                        ##
//############################################################################

RADCFUNC void Close_RAD_3D_image_lin( HRAD3DIMAGE rad_image )
{
  if ( rad_image->texture )
  {
    rad_image->texture->Release();
  }

  //
  // Free our memory.
  //

  free( rad_image );
}

/*
 * Wrapper routines which call the real pow2 or linear functions.
 */

//############################################################################
//##                                                                        ##
//## Blit a 3D image onto the render target.                                ##
//##                                                                        ##
//############################################################################

RADCFUNC void Blit_RAD_3D_image( HRAD3DIMAGE rad_image,
                                 f32 x_offset,
                                 f32 y_offset,
                                 f32 x_scale,
                                 f32 y_scale,
                                 f32 alpha_level )
{
  if ( rad_image == 0 )
  {
    return;
  }

  if ( rad_image->lin_texture )
  {
    Blit_RAD_3D_image_lin( rad_image, x_offset, y_offset, x_scale,
      y_scale, alpha_level );
  }
  else
  {
    Blit_RAD_3D_image_pow2( rad_image, x_offset, y_offset, x_scale,
      y_scale, alpha_level );
  }
}

//############################################################################
//##                                                                        ##
//## Draw the edges of the texture for debugging purposes.                  ##
//##                                                                        ##
//############################################################################

RADCFUNC void Draw_lines_RAD_3D_image( HRAD3DIMAGE rad_image,
                                       f32 x_offset,
                                       f32 y_offset,
                                       f32 x_scale,
                                       f32 y_scale  )
{
  if ( rad_image == 0 )
  {
    return;
  }

  if ( rad_image->lin_texture )
  {
    Draw_lines_RAD_3D_image_lin( rad_image, x_offset,
        y_offset, x_scale, y_scale );
  }
  else
  {
    Draw_lines_RAD_3D_image_pow2( rad_image, x_offset,
        y_offset, x_scale, y_scale );
  }
}

//############################################################################
//##                                                                        ##
//## Lock a RAD 3D image and return the buffer address and pitch.           ##
//##                                                                        ##
//############################################################################

RADCFUNC s32 Lock_RAD_3D_image( HRAD3DIMAGE rad_image,
                                void * out_pixel_buffer,
                                u32 * out_buffer_pitch,
                                u32 * out_surface_type,
                                u32 * src_x,
                                u32 * src_y,
                                u32 * src_w,
                                u32 * src_h )
{
  if ( rad_image == 0 )
  {
    return( 0 );
  }

  if ( rad_image->lin_texture )
  {
    return Lock_RAD_3D_image_lin( rad_image, out_pixel_buffer,
        out_buffer_pitch, out_surface_type, src_x, src_y, src_w, src_h );
  }
  else
  {
    return Lock_RAD_3D_image_pow2( rad_image, out_pixel_buffer,
        out_buffer_pitch, out_surface_type, src_x, src_y, src_w, src_h );
  }
}

//############################################################################
//##                                                                        ##
//## Unlock a RAD 3D image (mark it for redownloading next frame).          ##
//##                                                                        ##
//############################################################################

RADCFUNC void Unlock_RAD_3D_image( HRAD3DIMAGE rad_image )
{
  if ( rad_image == 0 )
  {
    return;
  }

  if ( rad_image->lin_texture )
  {
    Unlock_RAD_3D_image_lin( rad_image );
  }
  else
  {
    Unlock_RAD_3D_image_pow2( rad_image );
  }
}

#define is_pow2( val ) ( Round_up_to_next_2_power( val ) == ( val ) )

//############################################################################
//##                                                                        ##
//## Open a RAD 3D image (a data structure to blit an image through D3D).   ##
//##                                                                        ##
//############################################################################

RADCFUNC HRAD3DIMAGE Open_RAD_3D_image( HRAD3D rad_3d,
                                        u32 width,
                                        u32 height,
                                        s32 alpha_pixels,
                                        u32 maximum_texture_size )
{
  if ( ( is_pow2( width ) && is_pow2( height ) ) || ( rad_3d->nonpow2_textures_supported ) )
  {
    return Open_RAD_3D_image_lin( rad_3d, width, height,
        alpha_pixels, maximum_texture_size );
  }
  else
  {
    return Open_RAD_3D_image_pow2( rad_3d, width, height,
        alpha_pixels, maximum_texture_size );
  }
}

//############################################################################
//##                                                                        ##
//## Closes a RAD 3D image (frees textures and memory).                     ##
//##                                                                        ##
//############################################################################

RADCFUNC void Close_RAD_3D_image( HRAD3DIMAGE rad_image )
{
  if ( rad_image == 0 )
  {
    return;
  }

  if ( rad_image->lin_texture )
  {
    Close_RAD_3D_image_lin( rad_image );
  }
  else
  {
    Close_RAD_3D_image_pow2( rad_image );
  }
}

// @cdep pre $requiresbinary(d3d8.lib)
