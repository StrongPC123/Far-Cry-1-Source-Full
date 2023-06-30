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
#include <\devel\libs\dx7\include\d3d.h>

#include "rad3d.h"


//############################################################################
//##                                                                        ##
//## This function is called as D3D enumerates over the texture formats.    ##
//##   It watches for the best possible non-alpha and alpha texture formats ##
//##   and saves them in both D3D format and RAD_3D surface style format.   ##
//##                                                                        ##
//############################################################################

typedef struct TEXTUREFIND
{
  DDPIXELFORMAT d3d_alpha_surface;
  DDPIXELFORMAT d3d_surface;
  S32 rad_alpha_surface;
  S32 rad_surface;
  U32 alpha_pixel_size;
  U32 pixel_size;
  U32 screen_depth;
} TEXTUREFIND;

static HRESULT CALLBACK Search_d3d_textures( LPDDPIXELFORMAT pixel_format,
                                             LPVOID texture_find )
{
  TEXTUREFIND* tf = ( TEXTUREFIND* ) texture_find;

  //
  // Find a vanilla RGB or BGRA texture (D3D returns some crazy formats).
  //

  if ( ! ( pixel_format->dwFlags &
          ( DDPF_LUMINANCE | DDPF_BUMPLUMINANCE | DDPF_BUMPDUDV |
            DDPF_ALPHA | DDPF_ALPHAPREMULT | DDPF_ZBUFFER |
            DDPF_ZPIXELS | DDPF_YUV | DDPF_STENCILBUFFER |
            DDPF_PALETTEINDEXED1 | DDPF_PALETTEINDEXED2 |
            DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED8 |
            DDPF_PALETTEINDEXEDTO8 | DDPF_FOURCC ) ) )
  {
    switch( pixel_format->dwRGBBitCount )
    {
      //
      // Check for 16-bit modes supported.
      //

      case 16:
      {
        if ( ( pixel_format->dwRGBAlphaBitMask == 0xf000 ) &&
             ( pixel_format->dwRBitMask == 0x0f00 ) &&
             ( pixel_format->dwGBitMask == 0x00f0 ) &&
             ( pixel_format->dwBBitMask == 0x000f ) )
        {
          //
          // Use 4444 if we haven't had any alphas, or if we've
          //   only seen 5551.
          //

          if ( ( tf->rad_alpha_surface == -1 ) ||
               ( tf->rad_alpha_surface == RAD3DSURFACE5551 ) )
          {
            tf->rad_alpha_surface = RAD3DSURFACE4444;
            tf->alpha_pixel_size = 2;
            memcpy( &tf->d3d_alpha_surface,
                    pixel_format,
                    sizeof( tf->d3d_alpha_surface ) );
          }
        }
        else if ( ( pixel_format->dwRBitMask == 0xf800 ) &&
                  ( pixel_format->dwGBitMask == 0x07e0 ) &&
                  ( pixel_format->dwBBitMask == 0x001f ) )
        {
          //
          // Use 565 if we haven't had any textures, or if the only
          //   one we've seen is 555.
          //

          if ( ( tf->rad_surface == -1 ) || ( tf->rad_surface == RAD3DSURFACE555 ) )
          {
            tf->rad_surface = RAD3DSURFACE565;
            tf->pixel_size = 2;
            memcpy( &tf->d3d_surface,
                    pixel_format,
                    sizeof( tf->d3d_surface ) );
          }
        }
        else if ( ( pixel_format->dwRBitMask == 0x7c00 ) &&
                  ( pixel_format->dwGBitMask == 0x03e0 ) &&
                  ( pixel_format->dwBBitMask == 0x001f ) )
        {
          if ( pixel_format->dwRGBAlphaBitMask == 0x8000 )
          {
            //
            // Use 5551 only if we've never seen any other alpha formats.
            //

            if ( tf->rad_alpha_surface == -1 )
            {
              tf->rad_alpha_surface = RAD3DSURFACE5551;
              tf->alpha_pixel_size = 2;
              memcpy( &tf->d3d_alpha_surface,
                      pixel_format,
                      sizeof( tf->d3d_alpha_surface ) );
            }
          }
          else if ( pixel_format->dwRGBAlphaBitMask == 0x0000 )
          {
            //
            // Use 555 only if we've never seen any other formats.
            //

            if ( tf->rad_surface == -1 )
            {
              tf->rad_surface = RAD3DSURFACE555;
              tf->pixel_size = 2;
              memcpy( &tf->d3d_surface,
                      pixel_format,
                      sizeof( tf->d3d_surface ) );
            }
          }
        }
      }
      break;

      //
      // Check for 24-bit modes supported.
      //

      case 24:
      {
        if ( ( pixel_format->dwRBitMask == 0xff0000 ) &&
             ( pixel_format->dwGBitMask == 0x00ff00 ) &&
             ( pixel_format->dwBBitMask == 0x0000ff ) )
        {
          //
          // Use 24, if we've only seen 16-bit textures and the screen color
          //   depth is great than 16.
          //

          if ( ( ( tf->rad_surface != RAD3DSURFACE32 ) ||
                 ( tf->rad_surface != RAD3DSURFACE32R ) ) &&
               ( tf->screen_depth > 16 ) )
          {
            tf->rad_surface = RAD3DSURFACE24;
            tf->pixel_size = 3;
            memcpy( &tf->d3d_surface,
                    pixel_format,
                    sizeof( tf->d3d_surface ) );
          }
        }
      }
      break;

      //
      // Check for 32-bit modes supported.
      //

      case 32:
      {
        //
        // Always use 32 if we find it, if screen color depth is great than 16.
        //

        if ( tf->screen_depth > 16 )
        {
          if ( ( pixel_format->dwRBitMask == 0x00ff0000 ) &&
               ( pixel_format->dwGBitMask == 0x0000ff00 ) &&
               ( pixel_format->dwBBitMask == 0x000000ff ) )
          {
            if ( pixel_format->dwRGBAlphaBitMask == 0xff000000 )
            {
              tf->rad_alpha_surface = RAD3DSURFACE32A;
              tf->alpha_pixel_size = 4;
              memcpy( &tf->d3d_alpha_surface,
                      pixel_format,
                      sizeof( tf->d3d_alpha_surface ) );
            }
            else
            {
              tf->rad_surface = RAD3DSURFACE32;
              tf->pixel_size = 4;
              memcpy( &tf->d3d_surface,
                      pixel_format,
                      sizeof( tf->d3d_surface ) );
            }
          }
          else if ( ( pixel_format->dwRBitMask == 0x000000ff ) &&
                    ( pixel_format->dwGBitMask == 0x0000ff00 ) &&
                    ( pixel_format->dwBBitMask == 0x00ff0000 ) )
          {
            if ( pixel_format->dwRGBAlphaBitMask == 0xff000000 )
            {
              tf->rad_alpha_surface = RAD3DSURFACE32RA;
              tf->alpha_pixel_size = 4;
              memcpy( &tf->d3d_alpha_surface,
                      pixel_format,
                      sizeof( tf->d3d_alpha_surface ) );
            }
            else
            {
              tf->rad_surface = RAD3DSURFACE32R;
              tf->pixel_size = 4;
              memcpy( &tf->d3d_surface,
                      pixel_format,
                      sizeof( tf->d3d_surface ) );
            }
          }
        }
      }
      break;

    }
  }

  //
  // Returning DDENUMRET_OK so that we keep getting textures.
  //

  return( DDENUMRET_OK );
}


static S32 Find_texture_formats( LPDIRECT3DDEVICE7 d3d_device,
                                 U32 screen_color_depth,
                                 TEXTUREFIND* out_texture_find )
{
  out_texture_find->rad_alpha_surface = -1;
  out_texture_find->rad_surface = -1;
  out_texture_find->screen_depth = screen_color_depth;

  //
  // Enumerate all texture formats supported by the current video card.
  //

  d3d_device->EnumTextureFormats( Search_d3d_textures,
                                  (LPVOID)out_texture_find );

  //
  // Return true if we've matched an normal and an alpha format.
  //

  return( ( ( out_texture_find->rad_alpha_surface != -1 ) &&
            ( out_texture_find->rad_surface != -1 ) ) );
}


//
// Direct3D RAD3D structure.
//

typedef struct RAD3D
{
  LPDIRECT3D7 direct_3d;
  LPDIRECTDRAWSURFACE7 primary_surface;
  LPDIRECT3DDEVICE7 direct_3d_device;
  LPDIRECTDRAWSURFACE7 back_buffer;
  LPDIRECTDRAW7 render_direct_draw;
  HWND window;
  DDPIXELFORMAT d3d_alpha_surface;
  DDPIXELFORMAT d3d_surface;
  U32 alpha_pixel_size;
  U32 pixel_size;
  S32 rad_alpha_surface;
  S32 rad_surface;
} RAD3D;


//############################################################################
//##                                                                        ##
//## Call this function to actually open Direct3D.                          ##
//##                                                                        ##
//############################################################################

RADCFUNC HRAD3D Open_RAD_3D( HWND window )
{
  LPDIRECT3D7 direct_3d;
  LPDIRECTDRAWSURFACE7 primary_surface;
  LPDIRECT3DDEVICE7 direct_3d_device;
  HRAD3D rad_3d;
  LPDIRECTDRAWSURFACE7 back_buffer;
  LPDIRECTDRAW7 render_direct_draw;

  //
  // First, try to open DirectDraw.
  //

  LPDIRECTDRAW7 direct_draw;

  if ( !SUCCEEDED( DirectDrawCreateEx( 0,
                                       (VOID**)&direct_draw,
                                       IID_IDirectDraw7,
                                       0 ) ) )
  {
    return( 0 );
  }

  //
  // Now set the cooperative level.
  //

  if ( !SUCCEEDED( direct_draw->SetCooperativeLevel( window,
                                                     DDSCL_NORMAL ) ) )
  {
    return( 0 );
  }

  //
  // Now make sure we are in at least 16-bit color mode.
  //

  DDSURFACEDESC2 display_mode = { sizeof( display_mode ) };

  direct_draw->GetDisplayMode( &display_mode );

  if ( display_mode.ddpfPixelFormat.dwRGBBitCount < 16 )
  {
    direct_draw->Release( );
    return( 0 );
  }

  //
  // Now try to obtain the primary surface handle.
  //

  DDSURFACEDESC2 surface_description = { sizeof( DDSURFACEDESC2 ) };

  surface_description.dwFlags = DDSD_CAPS;
  surface_description.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;

  if ( !SUCCEEDED( direct_draw->CreateSurface( &surface_description,
                                               &primary_surface,
                                               0 ) ) )
  {
    direct_draw->Release( );
    return( 0 );
  }

  //
  // Now get a clipper.
  //

  LPDIRECTDRAWCLIPPER clipper;
  
  if ( !SUCCEEDED( direct_draw->CreateClipper( 0, &clipper, 0 ) ) )
  {
    primary_surface->Release( );
    direct_draw->Release( );
    return( 0 );
  }

  //
  // Point the clipper to our window.
  //

  clipper->SetHWnd( 0, window );
  primary_surface->SetClipper( clipper );
  clipper->Release( );

  //
  // Now get a Direct3D handle.
  //

  if ( !SUCCEEDED( direct_draw->QueryInterface( IID_IDirect3D7,
                                               ( VOID** ) &direct_3d ) ) )
  {
    primary_surface->Release( );
    direct_draw->Release( );
    return( 0 );
  }

  //
  // Now try to find a Direct3D device to use.
  //

  if ( !SUCCEEDED( direct_3d->CreateDevice( IID_IDirect3DHALDevice,
                                            primary_surface,
                                            &direct_3d_device ) ) )
  {

    //
    // If the HAL doesn't find anything, try straight RGB.
    //

    if ( !SUCCEEDED( direct_3d->CreateDevice( IID_IDirect3DRGBDevice,
                                              primary_surface,
                                              &direct_3d_device ) ) )
    {
      primary_surface->Release( );
      direct_draw->Release( );
      direct_3d->Release( );
      return( 0 );
    }
  }

  //
  // We don't need our own DirectDraw object anymore.
  //

  direct_draw->Release( );

  //
  // Try to find some texture formats that we can use.
  //

  TEXTUREFIND texture_find;

  if ( Find_texture_formats( direct_3d_device,
                             display_mode.ddpfPixelFormat.dwRGBBitCount,
                             &texture_find ) == 0 )
  {
    primary_surface->Release( );
    direct_3d_device->Release( );
    direct_3d->Release( );
    return( 0 );
  }

  //
  // Get the current render target and its DirectDraw interface because
  //   we need to create the new render target off the old DirectDraw and
  //   because we need to release the old render target. Sigh.
  //

  LPDIRECTDRAWSURFACE7 current_render_target;

  direct_3d_device->GetRenderTarget( &current_render_target );
  current_render_target->GetDDInterface( ( VOID** ) &render_direct_draw );

  //
  // Get the client area so that we know how big to make our back buffer.
  //

  RECT c;
  GetClientRect( window, &c );

  //
  // Now try to create the back buffer.
  //

  DDSURFACEDESC2 back_description = { sizeof( back_description ) };

  back_description.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
  back_description.ddsCaps.dwCaps =  DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
  back_description.dwWidth = c.right - c.left;
  back_description.dwHeight = c.bottom - c.top;

  if ( !SUCCEEDED( render_direct_draw->CreateSurface( &back_description,
                                                      &back_buffer,
                                                      0 ) ) )
  {
    primary_surface->Release( );
    direct_3d_device->Release( );
    direct_3d->Release( );
    current_render_target->Release( );
    render_direct_draw->Release( );
    return( 0 );
  }

  //
  // Now that we have a new render target, point our device to it.
  //

  direct_3d_device->SetRenderTarget( back_buffer, 0 );

  //
  // Release the old target and the DirectDraw interface that we're done with.
  //

  current_render_target->Release( );

  //
  // try to create a RAD3D structure
  //

  rad_3d = ( HRAD3D ) malloc( sizeof( RAD3D ) );
  if ( rad_3d == 0 )
  {
    render_direct_draw->Release( );
    primary_surface->Release( );
    direct_3d_device->Release( );
    direct_3d->Release( );
    back_buffer->Release( );
    return( 0 );
  }

  //
  // Now set the structure values.
  //

  rad_3d->direct_3d = direct_3d;
  rad_3d->primary_surface = primary_surface;
  rad_3d->direct_3d_device = direct_3d_device;
  rad_3d->back_buffer = back_buffer;
  rad_3d->window = window;
  rad_3d->d3d_alpha_surface = texture_find.d3d_alpha_surface;
  rad_3d->d3d_surface = texture_find.d3d_surface;
  rad_3d->alpha_pixel_size = texture_find.alpha_pixel_size;
  rad_3d->pixel_size = texture_find.pixel_size;
  rad_3d->rad_alpha_surface = texture_find.rad_alpha_surface;
  rad_3d->rad_surface = texture_find.rad_surface;
  rad_3d->render_direct_draw = render_direct_draw;
  //
  // Set the initial viewport.
  //

  Resize_RAD_3D( rad_3d, c.right - c.left, c.bottom - c.top );

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
    if ( rad_3d->back_buffer )
    {
      rad_3d->back_buffer->Release( );
      rad_3d->back_buffer = 0;
    }

    if ( rad_3d->direct_3d )
    {
      rad_3d->direct_3d->Release( );
      rad_3d->direct_3d = 0;
    }

    if ( rad_3d->primary_surface )
    {
      rad_3d->primary_surface->Release( );
      rad_3d->primary_surface = 0;
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
  return( "Direct3D" );
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

    rad_3d->direct_3d_device->Clear(0, 0, D3DCLEAR_TARGET , 0, 1.0f, 0);

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
  RECT client_rect;
  
  if ( rad_3d )
  {
    //
    // End the rendering.
    //

    rad_3d->direct_3d_device->EndScene();

    GetClientRect( rad_3d->window, &client_rect );
    ClientToScreen( rad_3d->window, ( LPPOINT ) &client_rect.left );
    ClientToScreen( rad_3d->window, ( LPPOINT ) &client_rect.right );

    //
    // Blit into our window.
    //

    rad_3d->primary_surface->Blt( &client_rect,
                                  rad_3d->back_buffer,
                                  0, DDBLT_WAIT, 0 );
  }
}


//############################################################################
//##                                                                        ##
//## Resize the Direct3D viewport.                                          ##
//##                                                                        ##
//############################################################################

RADCFUNC void Resize_RAD_3D( HRAD3D rad_3d,
                             U32 width,
                             U32 height )
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
  D3DVIEWPORT7 vp = { 0, 0, width, height, 0.0f, 1.0f };

  rad_3d->direct_3d_device->SetViewport( &vp );
}


//############################################################################
//##                                                                        ##
//## Simple function to round up to the next power of 2.                    ##
//##                                                                        ##
//############################################################################

static U32 Round_up_to_next_2_power( U32 value )
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

LPDIRECTDRAWSURFACE7 Create_texture( LPDIRECT3DDEVICE7 d3d_device,
                                     LPDIRECTDRAW7 direct_draw,
                                     DDPIXELFORMAT* d3d_surface_type,
                                     U32 pitch,
                                     U32 pixel_size,
                                     U32 width,
                                     U32 height )
{
  LPDIRECTDRAWSURFACE7 texture_surface;

  //
  // Create a description for our texture.
  //

  DDSURFACEDESC2 texture_description = { sizeof( texture_description ) };
  texture_description.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT |
                                DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE;
  texture_description.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
  texture_description.dwWidth = width;
  texture_description.dwHeight = height;

  //
  // Ask Direct3D to describe the types of textures the current card
  // can support to set the texture caps.
  //

  D3DDEVICEDESC7 d3d_description;
  if ( !SUCCEEDED( d3d_device->GetCaps( &d3d_description ) ) )
  {
    if ( ( d3d_description.deviceGUID == IID_IDirect3DHALDevice ) ||
         ( d3d_description.deviceGUID == IID_IDirect3DTnLHalDevice ) )
    {
      //
      // We are running on some sort of hardware, so we need to
      // turn on texture management.
      //

      texture_description.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
    }
    else
    {
      //
      // We're not running on hardware, so we can keep all our
      // textures in system memory.
      //
      texture_description.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
    }
  }

  //
  // Copy the pixel format.
  //

  memcpy( &texture_description.ddpfPixelFormat,
          d3d_surface_type,
          sizeof( texture_description.ddpfPixelFormat ) );

  //
  // Create a surface for our texture with the DirectDraw handle.
  //

  if ( SUCCEEDED( direct_draw->CreateSurface( &texture_description,
                                              &texture_surface,
                                              0 ) ) )
  {
    return( texture_surface );
  }

  return( 0 );
}


//############################################################################
//##                                                                        ##
//## Structure to contain a RAD 3D image.                                   ##
//##                                                                        ##
//############################################################################

typedef struct RAD3DIMAGE
{
  U32 total_textures;
  U32 pitch;
  U32 width;
  U32 height;
  S32 alpha_pixels;
  U32 pixel_size;
  U32 textures_across;
  U32 textures_down;
  U32 remnant_width;
  U32 remnant_height;
  U32 remnant_input_width;
  U32 remnant_input_height;
  U32 maximum_texture_size;
  U32 rad_surface_type;
  LPDIRECT3DDEVICE7 direct_3d_device;
  DDPIXELFORMAT d3d_surface_type;
  LPDIRECTDRAWSURFACE7* d3d_textures;
  U32 texture_count;
} RAD3DIMAGE;


//############################################################################
//##                                                                        ##
//## Open a RAD 3D image (a data structure to blit an image through D3D).   ##
//##                                                                        ##
//############################################################################

RADCFUNC HRAD3DIMAGE Open_RAD_3D_image( HRAD3D rad_3d,
                                        U32 width,
                                        U32 height,
                                        S32 alpha_pixels,
                                        U32 maximum_texture_size )
{
  HRAD3DIMAGE rad_image;
  U32 remnant_width, remnant_height;
  U32 buffer_pitch, buffer_height;
  U32 pitch, pixel_size;
  U32 total_textures;
  LPDIRECTDRAWSURFACE7* textures;
  U32 x, y, i;

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

  //
  // The textures come after the structure.
  //

  rad_image->d3d_textures = ( LPDIRECTDRAWSURFACE7 * ) ( rad_image + 1 );

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
  rad_image->texture_count = 0;

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
                                    rad_3d->render_direct_draw,
                                    &rad_image->d3d_surface_type,
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
                                    rad_3d->render_direct_draw,
                                    &rad_image->d3d_surface_type,
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
                                    rad_3d->render_direct_draw,
                                    &rad_image->d3d_surface_type,
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
                                    rad_3d->render_direct_draw,
                                    &rad_image->d3d_surface_type,
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

  return( rad_image );
}



//############################################################################
//##                                                                        ##
//## Closes a RAD 3D image (frees textures and memory).                     ##
//##                                                                        ##
//############################################################################

RADCFUNC void Close_RAD_3D_image( HRAD3DIMAGE rad_image )
{
  if ( rad_image )
  {
    if ( rad_image->d3d_textures )
    {
      U32 i;

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
}


//############################################################################
//##                                                                        ##
//## Lock a RAD 3D image and return the buffer address and pitch.           ##
//##                                                                        ##
//############################################################################

RADCFUNC S32 Lock_RAD_3D_image( HRAD3DIMAGE rad_image,
                                void * out_pixel_buffer,
                                U32 * out_buffer_pitch,
                                U32 * out_surface_type,
                                U32 * src_x,
                                U32 * src_y,
                                U32 * src_w,
                                U32 * src_h )
{
  if ( rad_image == 0 )
  {
    return( 0 );
  }

  if ( rad_image->texture_count < rad_image->total_textures )
  {
    //
    // Lock the DirectX texture.
    //

    DDSURFACEDESC2 surface_description = { sizeof( DDSURFACEDESC2 ) };

    if ( ! SUCCEEDED( rad_image->d3d_textures[ rad_image->texture_count ]->Lock( 0,
                                     &surface_description,
                                     DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR |
                                     DDLOCK_WRITEONLY | DDLOCK_NOSYSLOCK,
                                     0 ) ) )
    {
      goto err;
    }

    //
    // Fill the variables that were requested.
    //

    if ( out_pixel_buffer )
    {
      *( void** )out_pixel_buffer = surface_description.lpSurface;
    }

    if ( out_buffer_pitch )
    {
      *out_buffer_pitch = surface_description.lPitch;
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

RADCFUNC void Unlock_RAD_3D_image( HRAD3DIMAGE rad_image )
{
  if ( rad_image == 0)
  {
    return;
  }

  //
  // Unlock the DirectX texture.
  //

  rad_image->d3d_textures[ rad_image->texture_count - 1 ]->Unlock( 0 );
}


//############################################################################
//##                                                                        ##
//## Submit the vertices for one texture.                                   ##
//##                                                                        ##
//############################################################################

static void Submit_vertices( LPDIRECT3DDEVICE7 d3d_device,
                             F32 dest_x,
                             F32 dest_y,
                             F32 scale_x,
                             F32 scale_y,
                             S32 width,
                             S32 height,
                             F32 alpha_level )
{
  D3DTLVERTEX vertices[ 4 ];

  //
  // Setup up the vertices.
  //

  vertices[ 0 ].sx = dest_x;
  vertices[ 0 ].sy = dest_y;
  vertices[ 0 ].sz = 0.0F;
  vertices[ 0 ].rhw = 1.0F;
  vertices[ 0 ].color = ( ( S32 ) ( ( alpha_level * 255.0F ) ) << 24 ) | 0xffffff;
  vertices[ 0 ].specular = vertices[ 0 ].color;
  vertices[ 0 ].tu = 0.0F;
  vertices[ 0 ].tv = 0.0F;

  vertices[ 1 ] = vertices[ 0 ];

  vertices[ 1 ].sx = dest_x + ( scale_x * ( F32 ) width );
  vertices[ 1 ].tu = 1.0F;

  vertices[ 2 ] = vertices[0];

  vertices[ 2 ].sy = dest_y + ( scale_y * ( F32 ) height );
  vertices[ 2 ].tv = 1.0F;

  vertices[ 3 ] = vertices[ 1 ];

  vertices[ 3 ].sy = vertices[ 2 ].sy;
  vertices[ 3 ].tv = 1.0F;

  //
  // Draw the vertices.
  //

  d3d_device->DrawPrimitive( D3DPT_TRIANGLESTRIP,
                             D3DFVF_TLVERTEX,
                             vertices,
                             4,
                             0 );
}


//############################################################################
//##                                                                        ##
//## Submit the lines to outline a texture.                                 ##
//##                                                                        ##
//############################################################################

static void Submit_lines( LPDIRECT3DDEVICE7 d3d_device,
                          F32 dest_x,
                          F32 dest_y,
                          F32 scale_x,
                          F32 scale_y,
                          S32 width,
                          S32 height )
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

  points[ 1 ].sx = dest_x + ( scale_x * ( F32 ) width );
  points[ 1 ].sy = dest_y ;

  points[ 2 ] = points[ 1 ];
  points[ 3 ] = points[ 1 ];

  points[ 3 ].sy = dest_y + ( scale_y * ( F32 ) height );

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

  d3d_device->DrawPrimitive( D3DPT_LINELIST,
                             D3DFVF_TLVERTEX,
                             points,
                             12,
                             0 );
}


//############################################################################
//##                                                                        ##
//## Blit a 3D image onto the render target.                                ##
//##                                                                        ##
//############################################################################

RADCFUNC void Blit_RAD_3D_image( HRAD3DIMAGE rad_image,
                                 F32 x_offset,
                                 F32 y_offset,
                                 F32 x_scale,
                                 F32 y_scale,
                                 F32 alpha_level )
{
  LPDIRECTDRAWSURFACE7* textures;
  U32 x, y;
  F32 dest_x, dest_y;
  F32 adjust_x, adjust_y;

  if ( rad_image == 0 )
  {
    return;
  }

  //
  // If alpha is disabled and there is no texture alpha, turn alpha off.
  //

  if ( ( alpha_level >= (1.0F-0.0001) ) && ( ! rad_image->alpha_pixels ) )
  {
    rad_image->direct_3d_device->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, 0 );
    rad_image->direct_3d_device->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    rad_image->direct_3d_device->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    rad_image->direct_3d_device->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
  }
  else
  {
    rad_image->direct_3d_device->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, 1 );
    rad_image->direct_3d_device->SetRenderState( D3DRENDERSTATE_SRCBLEND,D3DBLEND_SRCALPHA );
    rad_image->direct_3d_device->SetRenderState( D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCALPHA );
    rad_image->direct_3d_device->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    rad_image->direct_3d_device->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    rad_image->direct_3d_device->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    rad_image->direct_3d_device->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
  }

  textures = rad_image->d3d_textures;

  adjust_x = ( x_scale * ( F32 ) rad_image->maximum_texture_size );
  adjust_y = ( y_scale * ( F32 ) rad_image->maximum_texture_size );

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

RADCFUNC void Draw_lines_RAD_3D_image( HRAD3DIMAGE rad_image,
                                       F32 x_offset,
                                       F32 y_offset,
                                       F32 x_scale,
                                       F32 y_scale  )
{
  U32 x, y;
  F32 dest_x, dest_y;
  F32 adjust_x, adjust_y;

  if ( rad_image == 0 )
  {
    return;
  }

  adjust_x = ( x_scale * ( F32 ) rad_image->maximum_texture_size );
  adjust_y = ( y_scale * ( F32 ) rad_image->maximum_texture_size );

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

// @cdep pre $requiresbinary(d3dx.lib)
// @cdep pre $requiresbinary(ddraw.lib)
