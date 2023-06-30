//############################################################################
//##                                                                        ##
//##  GLRAD3D.C                                                             ##
//##                                                                        ##
//##  API to use GL for video textures                                      ##
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

#include <windows.h>
#include <gl/gl.h>

#include "rad3d.h"

//############################################################################
//##                                                                        ##
//## Get the desktop color depth in bits.                                   ##
//##                                                                        ##
//############################################################################

static S32 Get_desktop_color_depth( void )
{
  HDC screen_dc;
  S32 desktop_color_bits;

  //
  // Calculate the desktop color depth
  //

  screen_dc = GetDC( 0 );
  desktop_color_bits = ( GetDeviceCaps( screen_dc, BITSPIXEL ) *
                         GetDeviceCaps( screen_dc, PLANES ) );
  ReleaseDC( 0, screen_dc );

  return( desktop_color_bits );
}


//############################################################################
//##                                                                        ##
//## Find a pixel format to use with GL (at least 16-bit).                  ##
//##                                                                        ##
//############################################################################

static S32 Find_good_pixel_format( HDC window_dc,
                                   PIXELFORMATDESCRIPTOR* out_pixel_format )
{
  S32 closest_format_index;
  S32 desktop_color_bits;
  PIXELFORMATDESCRIPTOR desired_format;

  //
  // Setup the description of the pixel format we want.
  //    (at least 16-bit color, 16-bit Z-buffer).
  //

  memset( &desired_format, 0, sizeof( PIXELFORMATDESCRIPTOR ) );
  desired_format.nSize = sizeof( PIXELFORMATDESCRIPTOR );
  desired_format.nVersion = 1;
  desired_format.dwFlags =  PFD_DRAW_TO_WINDOW |
                            PFD_SUPPORT_OPENGL |
                            PFD_DOUBLEBUFFER;
  desired_format.iPixelType = PFD_TYPE_RGBA;
  desired_format.cColorBits = 16;
  desired_format.cDepthBits = 16;

  //
  // Check to see if the desktop is already running in higher than
  //    16-bit color, and if so, use the desktop's color depth.
  //

  desktop_color_bits = Get_desktop_color_depth( );

  if ( desired_format.cColorBits < desktop_color_bits )
  {
    desired_format.cColorBits = ( unsigned char ) desktop_color_bits;
  }

  //
  // Find a suitable pixel format match.
  //

  closest_format_index = ChoosePixelFormat( window_dc, &desired_format );

  if ( closest_format_index )
  {
    //
    // Get a description of the matched format and return.
    //

    if ( DescribePixelFormat( window_dc, closest_format_index,
                              sizeof( PIXELFORMATDESCRIPTOR ),
                              out_pixel_format ) )
    {
      return( closest_format_index );
    }
  }

  //
  // Return nothing.
  //

  return( 0 );
}


//############################################################################
//##                                                                        ##
//## Configure the DC to use a OpenGL pixel format.                         ##
//##                                                                        ##
//############################################################################

static S32 Configure_dc_to_pixel_format( HDC rendering_dc,
                                         S32 format_index,
                                         PIXELFORMATDESCRIPTOR const* pixel_format )
{
  //
  // Ensure that we're running on a non-palettized display.
  //

  if ( ( GetDeviceCaps( rendering_dc, RASTERCAPS ) & RC_PALETTE ) != RC_PALETTE )
  {
    //
    // Set the pixel format.
    //

    if ( SetPixelFormat( rendering_dc,
                         format_index,
                         pixel_format ) )
    {
      //
      // Return success.
      //

      return( 1 );
    }
  }

  //
  // Return failure.
  //

  return( 0 );
}


//############################################################################
//##                                                                        ##
//## Create an OpenGL context for rendering.                                ##
//##                                                                        ##
//############################################################################

static HGLRC Create_gl_context( HDC rendering_dc )
{
  HGLRC context;

  //
  // Create a context
  //

  context = wglCreateContext( rendering_dc );

  //
  // If we got a context, make it current.
  //

  if ( context )
  {
    wglMakeCurrent( rendering_dc, context );
  }

  return( context );
}


//
// Open GL RAD3D structure.
//

typedef struct RAD3D
{
  HWND window;
  HGLRC context;
  HDC rendering_dc;
} RAD3D;


//############################################################################
//##                                                                        ##
//## Call this function to actually open GL.                                ##
//##                                                                        ##
//############################################################################

RADCFUNC HRAD3D Open_RAD_3D( HWND window )
{
  HRAD3D rad_3d;
  PIXELFORMATDESCRIPTOR pixel_format;
  S32 format_index;

  //
  // try to create a RAD3D structure
  //

  rad_3d = ( HRAD3D ) malloc( sizeof( RAD3D ) );
  if ( rad_3d == 0 )
  {
    return( 0 );
  }

  //
  // Get a DC to use.
  //

  rad_3d->rendering_dc = GetDC( window );

  //
  // Try to find a pixel format to use.
  //

  format_index = Find_good_pixel_format( rad_3d->rendering_dc, &pixel_format );

  if ( format_index )
  {

    //
    // Try to set the pixel format of the DC to match.
    //

    if ( Configure_dc_to_pixel_format( rad_3d->rendering_dc,
                                       format_index,
                                       &pixel_format ) )
    {

      //
      // Now try to create an OpenGL context.
      //

      rad_3d->context = Create_gl_context( rad_3d->rendering_dc );
      if ( rad_3d->context )
      {
        RECT c;

        //
        // Get the client size of the window.
        //

        GetClientRect( window, &c );

        //
        // Set GL to use our width and height.
        //

        glViewport( c.left, c.top, c.right - c.left, c.bottom - c.top );

        //
        // Set GL to use simple orthographic mode.
        //

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity( );
        glOrtho( 0, c.right - c.left, c.bottom - c.top, 0, -1.0F, 1.0F );
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity( );

        //
        // Set some of the initial simple GL state.
        //

        glDisable( GL_CULL_FACE );
        glDisable( GL_DEPTH_TEST );
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glEnable( GL_TEXTURE_2D );

        //
        // Set the background color.
        //

        glClearColor( 0, 0, 0, 0 );

        //
        // Return the new DC to use.
        //

        rad_3d->window = window;

        return( rad_3d );
      }
    }
  }

  //
  // Open failed, release the DC, cleanup, and return.
  //

  ReleaseDC( window, rad_3d->rendering_dc );

  free( rad_3d );

  return(0);
}


//############################################################################
//##                                                                        ##
//## Call this function to close GL.                                        ##
//##                                                                        ##
//############################################################################

RADCFUNC void Close_RAD_3D( HRAD3D rad_3d )
{
  //
  // Clean up after ourselves.
  //

  if ( rad_3d )
  {
    wglMakeCurrent( 0, 0 );

    wglDeleteContext( rad_3d->context );

    ReleaseDC( rad_3d->window, rad_3d->rendering_dc );

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
  return( "OpenGL" );
}


//############################################################################
//##                                                                        ##
//## Begin a GL frame.                                                      ##
//##                                                                        ##
//############################################################################

RADCFUNC void Start_RAD_3D_frame( HRAD3D rad_3d )
{
  if ( rad_3d )
  {
    //
    // Clear the screen.
    //

    glClear( GL_COLOR_BUFFER_BIT );
  }
}


//############################################################################
//##                                                                        ##
//## End a GL frame.                                                        ##
//##                                                                        ##
//############################################################################

RADCFUNC void End_RAD_3D_frame( HRAD3D rad_3d )
{
  if ( rad_3d )
  {
    //
    // Swap the buffers and render.
    //

    SwapBuffers( rad_3d->rendering_dc );
  }
}


//############################################################################
//##                                                                        ##
//## Resize the GL viewport.                                                ##
//##                                                                        ##
//############################################################################

RADCFUNC void Resize_RAD_3D( HRAD3D rad_3d,
                             U32 width,
                             U32 height )
{
  glViewport( 0, 0, width, height );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity( );
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
//## Setup the specified GL texture.                                        ##
//##                                                                        ##
//############################################################################

static void Setup_gl_texture( U32 texture,
                              U32 pitch,
                              U32 pixel_size,
                              U32 texture_width,
                              U32 texture_height,
                              U32 gl_surface_type,
                              void* buffer )
{
  //
  // Make the texture current.
  //

  glBindTexture( GL_TEXTURE_2D, texture );

  //
  // Set the texture wrap and filtering options.
  //

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

  //
  // Set the pixel format options.
  //

  glPixelStorei( GL_UNPACK_ROW_LENGTH, pitch / pixel_size );
  glPixelStorei( GL_UNPACK_ALIGNMENT, ( pitch % pixel_size ) + 1 );

  //
  // Upload data into the texture.
  //

  glTexImage2D ( GL_TEXTURE_2D,
                 0,
                 gl_surface_type,
                 texture_width,
                 texture_height,
                 0,
                 gl_surface_type,
                 GL_UNSIGNED_BYTE,
                 buffer );
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
  U32 gl_surface_type;
  U32* gl_textures;
  U8* pixels;
  S32 download_textures;
  U32 row_length;
  U32 texture_count;
} RAD3DIMAGE;


//############################################################################
//##                                                                        ##
//## Open a RAD 3D image (a data structure to blit an image through GL).    ##
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
  U32* textures;
  U32 x, y;

  //
  // Calculate the pixel size and the pitch
  //

  pixel_size = ( alpha_pixels ) ? 4 : 3;
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
                                      ( total_textures * 4 ) +
                                      31 + ( buffer_pitch * buffer_height) );
  if ( rad_image == 0 )
  {
    return( 0 );
  }

  //
  // The textures come after the structure.
  //

  rad_image->gl_textures = ( U32 * ) ( rad_image + 1 );

  //
  // And the buffer comes after the textures (aligned to a 32-byte address).
  //

  rad_image->pixels = ( U8 * ) ( ( ( ( U32 ) ( rad_image->gl_textures +
                                               total_textures ) ) + 8 ) & ~7 );

  //
  // Set all the variables in our new structure.
  //

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
  rad_image->rad_surface_type = ( alpha_pixels ) ? RAD3DSURFACE32RA : RAD3DSURFACE24R;
  rad_image->gl_surface_type = ( alpha_pixels ) ? GL_RGBA : GL_RGB;
  rad_image->download_textures = 0;
  rad_image->row_length = pitch / pixel_size;
  rad_image->texture_count = 0;

  //
  // Clear the buffer.
  //

  memset( rad_image->pixels,
          0,
          buffer_pitch * buffer_height );

  //
  // Call GL to create a bunch of textures (clear the last one as a flag to
  //   see if all of the textures were created).
  //

  rad_image->gl_textures[ rad_image->total_textures - 1 ] = 0;

  glGenTextures( rad_image->total_textures, rad_image->gl_textures );

  if ( rad_image->gl_textures[ rad_image->total_textures - 1 ] == 0 )
  {
    //
    // GL didn't allocate enough textures for us, so just fail.
    //

    free( rad_image );

    return(0);
  }


  //
  // Loop through and init each texture (setting each of their sizes).
  //

  textures = rad_image->gl_textures;

  for (y = 0; y < rad_image->textures_down ; y++ )
  {
    for ( x = 0 ; x < rad_image->textures_across ; x++ )
    {
      //
      // Setup the texture.
      //

      Setup_gl_texture( *textures++,
                        rad_image->pitch,
                        rad_image->pixel_size,
                        rad_image->maximum_texture_size,
                        rad_image->maximum_texture_size,
                        rad_image->gl_surface_type,
                        rad_image->pixels );
    }

    //
    // Do the rememnant texture at the end of the scanline.
    //

    if ( rad_image->remnant_width )
    {
      //
      // Setup the texture.
      //

      Setup_gl_texture( *textures++,
                        rad_image->pitch,
                        rad_image->pixel_size,
                        rad_image->remnant_width,
                        rad_image->maximum_texture_size,
                        rad_image->gl_surface_type,
                        rad_image->pixels );
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
      // Setup the texture.
      //

      Setup_gl_texture( *textures++,
                        rad_image->pitch,
                        rad_image->pixel_size,
                        rad_image->maximum_texture_size,
                        rad_image->remnant_height,
                        rad_image->gl_surface_type,
                        rad_image->pixels );
    }
    if ( rad_image->remnant_width )
    {
      //
      // Setup the texture.
      //
      
      Setup_gl_texture( *textures++,
                        rad_image->pitch,
                        rad_image->pixel_size,
                        rad_image->remnant_width,
                        rad_image->remnant_height,
                        rad_image->gl_surface_type,
                        rad_image->pixels );
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
    if ( rad_image->gl_textures )
    {
      //
      // Ask GL to delete the textures.
      //

      glDeleteTextures( rad_image->total_textures,  rad_image->gl_textures );
      rad_image->gl_textures = 0;

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
                                void* out_pixel_buffer,
                                U32* out_buffer_pitch,
                                U32* out_surface_type,
                                U32* src_x,
                                U32* src_y,
                                U32* src_w,
                                U32* src_h )
{
  if ( rad_image == 0 )
  {
    return( 0 );
  }

  //
  // Fill the variables that were requested.
  //

  if ( rad_image->texture_count < 1 )
  {

    if ( out_pixel_buffer )
    {
      *( void** )out_pixel_buffer = rad_image->pixels;
    }

    if ( out_buffer_pitch )
    {
      *out_buffer_pitch = rad_image->pitch;
    }

    if ( out_surface_type )
    {
      *out_surface_type = rad_image->rad_surface_type;
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

RADCFUNC void Unlock_RAD_3D_image( HRAD3DIMAGE rad_image )
{
  if ( rad_image == 0)
  {
    return;
  }

  //
  // Set the flag to redownload the texture for the next frame.
  //

  rad_image->download_textures = 1;
}


//############################################################################
//##                                                                        ##
//## Submit a new texture.                                                  ##
//##                                                                        ##
//############################################################################

static void Submit_texture( void* pixels,
                            U32 row_length,
                            U32 width,
                            U32 height,
                            U32 surface_type )
{
  //
  // Set the pixel format options.
  //

  glPixelStorei( GL_UNPACK_ROW_LENGTH, row_length );
  glPixelStorei( GL_UNPACK_ALIGNMENT, 8);

  //
  // Use TexSubImage because it is faster on some hardware.
  //

  glTexSubImage2D ( GL_TEXTURE_2D,
                    0, 0, 0,
                    width,
                    height,
                    surface_type,
                    GL_UNSIGNED_BYTE,
                    pixels );
}


//############################################################################
//##                                                                        ##
//## Submit the vertices for one texture.                                   ##
//##                                                                        ##
//############################################################################

static void Submit_vertices( F32 dest_x,
                             F32 dest_y,
                             F32 scale_x,
                             F32 scale_y,
                             U32 width,
                             U32 height,
                             F32 alpha_level )
{
  F32 right, bottom;

  //
  // Start a quad.
  //

  glBegin( GL_QUADS );

  //
  // Set the colors for these vertices.
  //

  glColor4f( 1.0F, 1.0F, 1.0F, alpha_level );

  //
  // Draw around a rectangle.
  //

  right = dest_x + ( scale_x * ( F32 ) width );
  bottom = dest_y + ( scale_y * ( F32 ) height );

  glTexCoord2f( 0, 0 );
  glVertex3f( dest_x, dest_y, 0.0F );

  glTexCoord2f( 1.0f, 0 );
  glVertex3f( right, dest_y, 0.0F );

  glTexCoord2f( 1.0f, 1.0f );
  glVertex3f( right, bottom, 0.0F );

  glTexCoord2f( 0, 1.0f );
  glVertex3f( dest_x, bottom, 0.0F );

  //
  // Done with the vertices.
  //

  glEnd( );
}


//############################################################################
//##                                                                        ##
//## Submit the lines to outline a texture.                                 ##
//##                                                                        ##
//############################################################################

static void Submit_lines( F32 dest_x,
                          F32 dest_y,
                          F32 scale_x,
                          F32 scale_y,
                          U32 width,
                          U32 height )
{
  F32 right, bottom;

  //
  // Start a quad.
  //

  glBegin( GL_LINES );

  glColor4f( 1.0F, 1.0F, 1.0F, 1.0F );

  //
  // Draw around a rectangle.
  //

  right = dest_x + ( scale_x * ( F32 ) width );
  bottom = dest_y + ( scale_y * ( F32 ) height );

  glVertex3f( dest_x, dest_y, 0.0F );
  glVertex3f( right, dest_y, 0.0F );

  glVertex3f( right, dest_y, 0.0F );
  glVertex3f( right, bottom, 0.0F );

  glVertex3f( right, bottom, 0.0F );
  glVertex3f( dest_x, bottom, 0.0F );

  glVertex3f( dest_x, bottom, 0.0F );
  glVertex3f( dest_x, dest_y, 0.0F );

  glColor4f( 1.0F, 0.0F, 0.0F, 1.0F );

  glVertex3f( dest_x, dest_y, 0.0F );
  glVertex3f( right, bottom, 0.0F );

  glVertex3f( right, dest_y, 0.0F );
  glVertex3f( dest_x, bottom, 0.0F );

  //
  // Done with the lines.
  //

  glEnd( );
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
  U32* textures;
  U8* pixels;
  U32 x, y;
  F32 dest_x, dest_y;
  F32 adjust_x, adjust_y;
  U32 x_skip, y_skip;

  if ( rad_image == 0 )
  {
    return;
  }

  //
  // If alpha is disabled and there is no texture alpha, turn alpha off.
  //

  if ( ( alpha_level >= (1.0F-0.0001) ) && ( ! rad_image->alpha_pixels ) )
  {
    glDisable( GL_BLEND );
  }
  else
  {
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }

  //
  // Now loop through all of our textures, submitting them.
  //

  pixels = rad_image->pixels;
  textures = rad_image->gl_textures;

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

      glBindTexture( GL_TEXTURE_2D, *textures++ );

      //
      // If we got new pixels, download them.
      //

      if ( rad_image->download_textures )
      {
        Submit_texture( pixels,
                        rad_image->row_length,
                        rad_image->maximum_texture_size,
                        rad_image->maximum_texture_size,
                        rad_image->gl_surface_type );
      }

      //
      // Submit the vertices.
      //

      Submit_vertices( dest_x,
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

      glBindTexture( GL_TEXTURE_2D, *textures++ );

      //
      // If we got new pixels, download them.
      //

      if ( rad_image->download_textures )
      {
        Submit_texture( pixels,
                        rad_image->row_length,
                        rad_image->remnant_input_width,
                        rad_image->maximum_texture_size,
                        rad_image->gl_surface_type );
      }

      //
      // Submit the vertices.
      //

      Submit_vertices( dest_x,
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

      glBindTexture( GL_TEXTURE_2D, *textures++ );

      //
      // If we got new pixels, download them.
      //

      if ( rad_image->download_textures )
      {
        Submit_texture( pixels,
                        rad_image->row_length,
                        rad_image->maximum_texture_size,
                        rad_image->remnant_input_height,
                        rad_image->gl_surface_type );
      }

      //
      // Submit the vertices.
      //

      Submit_vertices( dest_x,
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

      glBindTexture( GL_TEXTURE_2D, *textures++ );

      //
      // If we got new pixels, download them.
      //

      if ( rad_image->download_textures )
      {
        Submit_texture( pixels,
                        rad_image->row_length,
                        rad_image->remnant_input_width,
                        rad_image->remnant_input_height,
                        rad_image->gl_surface_type );
      }

      //
      // Submit the vertices.
      //

      Submit_vertices( dest_x,
                       dest_y,
                       x_scale,
                       y_scale,
                       rad_image->remnant_width,
                       rad_image->remnant_height,
                       alpha_level );
    }
  }

  //
  // Clear the download texture flag after a blit.
  //

  rad_image->download_textures = 0;
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

  //
  // Disable texturing when drawing lines.
  //

  glDisable(GL_TEXTURE_2D);

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

      Submit_lines( dest_x,
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

      Submit_lines( dest_x,
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

      Submit_lines( dest_x,
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

      Submit_lines( dest_x,
                    dest_y,
                    x_scale,
                    y_scale,
                    rad_image->remnant_width,
                    rad_image->remnant_height );
    }
  }

  //
  // Re-enable texturing.
  //

  glEnable(GL_TEXTURE_2D);

}

// @cdep pre $requiresbinary(opengl32.lib)
