//############################################################################
//##                                                                        ##
//##  EXAMDDP.C                                                             ##
//##                                                                        ##
//##  Example of using Bink to draw onto a DirectDraw primary surface       ##
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
#include <ddraw.h>

#include "bink.h"


//
// Example globals
//

static HBINK Bink = 0;
static S32 Window_x, Window_y;
static S32 Client_offset_x, Client_offset_y;


//############################################################################
//##                                                                        ##
//## Open_directdraw - opens DirectDraw, the surface, and gets the surface  ##
//##   description.                                                         ##
//##                                                                        ##
//############################################################################

static S32 Open_directdraw( HWND                 window,
                            LPDIRECTDRAW*        out_directdraw,
                            LPDIRECTDRAWSURFACE* out_directdraw_surface,
                            DDSURFACEDESC*       out_surface_description,
                            S32*                 out_surface_type )
{
  //
  // Try to open the DirectDraw object, if that fails just return 0.
  //

  if ( DirectDrawCreate( 0, out_directdraw, NULL ) != DD_OK )
  {
    *out_directdraw = 0;
    *out_directdraw_surface = 0;

    return( 0 );
  }

  //
  // Try to set the DirectDraw cooperative level. If we fail, cleanup and exit.
  //

  if ( IDirectDraw_SetCooperativeLevel( *out_directdraw,
                                        window,
                                        DDSCL_NORMAL ) != DD_OK )
  {
    IDirectDraw_Release( *out_directdraw );

    *out_directdraw = 0;
    *out_directdraw_surface = 0;

    return( 0 );
  }

  //
  // Set up the surface description to request the primary surface.
  //

  memset( out_surface_description, 0, sizeof(DDSURFACEDESC) );
  out_surface_description->dwSize = sizeof( DDSURFACEDESC );
  out_surface_description->dwFlags = DDSD_CAPS;
  out_surface_description->ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

  //
  // Now try to create the surface. If we fail, clean up and exit.

  if ( IDirectDraw_CreateSurface( *out_directdraw,
                                   out_surface_description,
                                   out_directdraw_surface, NULL ) != DD_OK )
  {
    IDirectDraw_Release( *out_directdraw );

    *out_directdraw = 0;
    *out_directdraw_surface = 0;

    return( 0 );
  }

  //
  // Get the Bink surface type of the newly create surface.
  //

  *out_surface_type = BinkDDSurfaceType( *out_directdraw_surface );

  //
  // Is it a surface type that we can directly copy into? If not, fail with an error.
  //

  if ( ( *out_surface_type == -1 ) || ( *out_surface_type == BINKSURFACE8P ) )
  {
    MessageBox( window,
                "Unsupported primary surface format.",
                "Error",
                MB_OK | MB_ICONSTOP );

    IDirectDrawSurface_Release( *out_directdraw_surface );
    IDirectDraw_Release( *out_directdraw );

    *out_directdraw = 0;
    *out_directdraw_surface = 0;

    return( 0 );
  }

  return( 1 );
}


//############################################################################
//##                                                                        ##
//## Close_directdraw - frees the surface and shuts down DirectDraw.        ##
//##                                                                        ##
//############################################################################

static void Close_directdraw( LPDIRECTDRAW*        out_directdraw,
                              LPDIRECTDRAWSURFACE* out_directdraw_surface )
{
  //
  // Free the primary surface.
  //

  if ( out_directdraw_surface )
  {
    IDirectDrawSurface_Release( *out_directdraw_surface );
    *out_directdraw_surface = 0;
  }

  //
  // Free the directdraw object.
  //

  if ( out_directdraw )
  {
    IDirectDraw_Release( *out_directdraw );
    *out_directdraw = 0;
  }
}


//############################################################################
//##                                                                        ##
//## Clear_to_black - just fills a window with black pixels.                ##
//##                                                                        ##
//############################################################################

static void Clear_to_black( HWND window )
{
  PAINTSTRUCT ps;
  HDC dc;

  //
  // Get the repaint DC and then fill the window with black.
  //

  dc = BeginPaint( window, &ps );

  PatBlt( dc, 0, 0, 4096, 4096, BLACKNESS );

  EndPaint( window, &ps );
}


//############################################################################
//##                                                                        ##
//## WindowProc - the main window message procedure.                        ##
//##                                                                        ##
//############################################################################

LONG FAR PASCAL WindowProc( HWND   window,
                            UINT   message,
                            WPARAM wparam,
                            LPARAM lparam )
{

  switch( message )
  {
    //
    // Just close the window if the user hits a key.
    //

    case WM_CHAR:
      DestroyWindow( window );
      break;

    //
    // Pause/resume the video when the focus changes.
    //

    case WM_KILLFOCUS:
      BinkPause( Bink, 1 );
      break;

    case WM_SETFOCUS:
      BinkPause( Bink, 0 );
      break;

    //
    // Handle the window paint messages.
    //

    case WM_PAINT:
      Clear_to_black( window );
      return( 0 );

    case WM_ERASEBKGND:
      return( 1 );

    //
    // Handle the window being moved.
    //

    case WM_WINDOWPOSCHANGING:
      //
      // Is the window even being moved?
      //

      if ( ! ( ( ( WINDOWPOS* )lparam )->flags & SWP_NOMOVE ) )
      {
        //
        // Yup, it's being moved - force it to a 4 pixel boundary.
        //

        ( ( WINDOWPOS* )lparam )->x =
          ( ( ( ( WINDOWPOS* )lparam )-> x + Client_offset_x) & ~3 ) -
             Client_offset_x;

        //
        // Now record the final window position.
        //

        Window_x = ( ( WINDOWPOS* ) lparam )->x + Client_offset_x;
        Window_y = ( ( WINDOWPOS* ) lparam )->y + Client_offset_y;
      }
      break;

    //
    // Post the quit message.
    //

    case WM_DESTROY:
      PostQuitMessage( 0 );
      return( 0 );
  }

  //
  // Call the OS default window procedure.
  //

  return( DefWindowProc( window, message, wparam, lparam ) );
}


//############################################################################
//##                                                                        ##
//## Calc_window_values - calculates the X, Y, the X and Y adjustments for  ##
//##   the non-client areas (border, title bar) and the extra width and     ##
//##   height to add to a windows's size so that the video fits.            ##
//##                                                                        ##
//############################################################################

static void Calc_window_values( HWND window,
                                S32* out_window_x,
                                S32* out_window_y,
                                S32* out_client_x,
                                S32* out_client_y,
                                S32* out_extra_width,
                                S32* out_extra_height )
{
  RECT r, c;
  POINT p;

  //
  // Get the position of the upper-left client coordinate (in screen space).
  //

  p.x = 0;
  p.y = 0;
  ClientToScreen( window, &p );

  *out_window_x = p.x;
  *out_window_y = p.y;

  //
  // Get the current window rect (in screen space).
  //

  GetWindowRect( window, &r );

  *out_client_x = p.x - r.left;
  *out_client_y = p.y - r.top;

  //
  // Get the client rectangle of the window.
  //

  GetClientRect( window, &c );

  *out_extra_width = ( r.right - r.left ) - ( c.right - c.left );
  *out_extra_height = ( r.bottom - r.top ) - ( c.bottom - c.top );
}


//############################################################################
//##                                                                        ##
//## Build_window_handle - creates a window class and window handle.        ##
//##                                                                        ##
//############################################################################

static HWND Build_window_handle( HINSTANCE instance,
                                 HINSTANCE previous_instance,
                                 HCURSOR* out_cursor )
{
  //
  // Load a cursor.
  //

  *out_cursor = LoadCursor( 0, IDC_ARROW );

  //
  // Create the window class if this is the first instance.
  //

  if ( !previous_instance )
  {
    WNDCLASS wc;

    wc.style = 0;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = instance;
    wc.hIcon = LoadIcon( instance, MAKEINTRESOURCE( 101 ) );
    wc.hCursor = *out_cursor;
    wc.hbrBackground = 0;
    wc.lpszMenuName = 0;
    wc.lpszClassName = "BinkExam";

    //
    // Try to register the class.
    //

    if ( !RegisterClass( &wc ) )
    {
      return( 0 );
    }
  }

  //
  // Return the new window with a tiny initial default size (we assume this
  //   initial size later on when we are calculating the non-client areas).
  //

  return( CreateWindow( "BinkExam",
                        "Bink Example Player",
			   WS_CAPTION|WS_POPUP|WS_CLIPCHILDREN|
                        WS_SYSMENU|WS_MINIMIZEBOX,
                        64, 64, 64, 64, 0, 0, instance,0 ) );
}


//############################################################################
//##                                                                        ##
//## Next_bink_frame - advances to the next Bink frame.                     ##
//##                                                                        ##
//############################################################################

static void Show_next_frame( HBINK bink,
                             HWND window,
                             LPDIRECTDRAWSURFACE surface,
                             DDSURFACEDESC* surface_description,
                             S32 surface_type,
                             S32 window_x,
                             S32 window_y,
                             S32 software_cursor )
{
  S32 count = 0;

  //
  // Decompress the Bink frame.
  //

  BinkDoFrame( bink );

  //
  // If we have a software cursor, hide it, if we are going to overwrite it.
  //

  if ( software_cursor )
    count = BinkCheckCursor( window,
                             0,0,
                             bink->Width, bink->Height );

  //
  // Try to lock the surface.
  //

  while ( IDirectDrawSurface_Lock( surface,
                                   0,
                                   surface_description,
                                   DDLOCK_WAIT,
                                   0 ) == DDERR_SURFACELOST )
  {
    //
    // Surface was lost, try to restore it. If we can't, skip over the copy.
    //

    if ( IDirectDrawSurface_Restore( surface ) != DD_OK )
      goto unable_to_lock;
  }

  //
  // Copy the decompressed frame onto the screen.
  //

  BinkCopyToBuffer( bink,
                    surface_description->lpSurface,
                    surface_description->lPitch,
                    bink->Height,
                    window_x, window_y,
                    surface_type );

  //
  // Now unlock the surface.
  //

  IDirectDrawSurface_Unlock( surface,
                             surface_description->lpSurface );

 unable_to_lock:

  //
  // Restore the software cursor, if we hid it earlier.
  //

  if ( software_cursor )
    BinkRestoreCursor( count );

  //
  // Are we at the end of the movie?
  //

  if ( bink->FrameNum == bink->Frames )
  {
    //
    // Yup, close the window.
    //

    DestroyWindow( window );
  }
  else
  {
    //
    // Nope, advance to the next frame.
    //

    BinkNextFrame( bink );
  }
}


//############################################################################
//##                                                                        ##
//##  Good_sleep_us - sleeps for a specified number of MICROseconds.        ##
//##    The task switcher in Windows has a latency of 15 ms.  That means    ##
//##    you can ask for a Sleep of one millisecond and actually get a       ##
//##    sleep of 15 ms!  In normal applications, this is no big deal,       ##
//##    however, with a video player at 30 fps, 15 ms is almost half our    ##
//##    frame time!  The Good_sleep_us function times each sleep and keeps  ##
//##    the average sleep time to what you requested.  It also give more    ##
//##    accuracy than Sleep - Good_sleep_us() uses microseconds instead of  ##
//##    milliseconds.                                                       ##
//##                                                                        ##
//############################################################################

static void Good_sleep_us( S32 microseconds )
{
  static S32 total_sleep=0;
  static S32 slept_in_advance=0;
  static U64 frequency=1000;
  static S32 got_frequency=0;

  //
  // If this is the first time called, get the high-performance timer count.
  //

  if ( !got_frequency )
  {
    got_frequency = 1;
    QueryPerformanceFrequency( ( LARGE_INTEGER* )&frequency );
  }

  total_sleep += microseconds;

  //
  // Have we exceeded our reserve of slept microseconds?
  //

  if (( total_sleep - slept_in_advance ) > 1000)
  {
    U64 start, end;
    total_sleep -= slept_in_advance;

    //
    // Do the timed sleep.
    //

    QueryPerformanceCounter( ( LARGE_INTEGER* )&start );
    Sleep( total_sleep / 1000 );
    QueryPerformanceCounter( ( LARGE_INTEGER* )&end );

    //
    // Calculate delta time in microseconds.
    //

    end = ( (end - start) * (U64)1000000 ) / frequency;

    //
    // Keep track of how much extra we slept.
    //

    slept_in_advance = ( U32 )end - total_sleep;
    total_sleep %= 1000;
  }
}


//############################################################################
//##                                                                        ##
//## WinMain - the primary function entry point                             ##
//##                                                                        ##
//############################################################################

int PASCAL WinMain( HINSTANCE instance,
                    HINSTANCE previous_instance,
                    LPSTR cmd_line,
                    int cmd_show )
{
  //
  // Win32 locals.
  //

  HWND window = 0;
  HCURSOR cursor = 0;
  LPDIRECTDRAW directdraw = 0;
  LPDIRECTDRAWSURFACE directdraw_surface = 0;
  MSG msg;
  DDSURFACEDESC surface_description;

  //
  // Example locals.
  //

  S32 surface_type;
  S32 software_cursor;
  S32 extra_width, extra_height;


  //
  // Try to create our window.
  //

  window = Build_window_handle( instance,
                                previous_instance,
                                &cursor );
  if ( !window )
  {
    MessageBox( 0,
                "Error creating window.",
                "Windows",
                MB_OK | MB_ICONSTOP );
    return( 1 );
  }

  //
  // Calculate the initial window positions and the client offsets.
  //

  Calc_window_values( window,
                      &Window_x, &Window_y,
                      &Client_offset_x, &Client_offset_y,
                      &extra_width, &extra_height );

  //
  // Try to open DirectDraw.
  //

  if ( !Open_directdraw( window,
                         &directdraw,
                         &directdraw_surface,
                         &surface_description,
                         &surface_type ) )
  {
    DestroyWindow( window );
    return( 2 );
  }

  //
  // Determine if we have a software cursor.
  //

  software_cursor = BinkIsSoftwareCursor( directdraw_surface, cursor );

  //
  // Tell Bink to use DirectSound (must be before BinkOpen)!
  //

  BinkSoundUseDirectSound( 0 );

  //
  // Try to open the Bink file.
  //

  Bink = BinkOpen( cmd_line, 0 );
  if ( !Bink )
  {
    MessageBox( window,
                BinkGetError( ),
                "Bink Error",
                MB_OK | MB_ICONSTOP );

    DestroyWindow( window );
    return( 3 );
  }

  //
  // Size the window such that its client area exactly fits our Bink movie.
  //

  SetWindowPos( window, 0,
                0, 0,
                Bink->Width+extra_width,
                Bink->Height+extra_height,
                SWP_NOMOVE );

  //
  // Now display the window and start the message loop.
  //

  ShowWindow( window, cmd_show );

  for ( ; ; )
  {
    //
    // Are there any messages to handle?
    //

    if ( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
    {
      //
      // Yup, handle them.
      //

      if ( msg.message == WM_QUIT )
        break;

      TranslateMessage( &msg );
      DispatchMessage( &msg );
    }
    else
    {
      //
      // Is it time for a new Bink frame?
      //

      if ( !BinkWait( Bink ) )
      {
        //
        // Yup, draw the next frame.
        //

        Show_next_frame( Bink,
                         window,
                         directdraw_surface,
                         &surface_description,
                         surface_type,
                         Window_x,
                         Window_y,
                         software_cursor );
      }
      else
      {
        //
        // Nope, give the rest of the system a chance to run (500 MICROseconds).
        //

        Good_sleep_us( 500 );
      }

    }
  }

  //
  // Close the Bink file.
  //

  if ( Bink )
  {
    BinkClose( Bink );
    Bink = 0;
  }

  //
  // Clean up DirectDraw.
  //

  Close_directdraw( &directdraw,
                    &directdraw_surface );

  //
  // And exit.
  //

  return( 0 );
}

// some stuff for the RAD build utility
// @cdep pre $DefaultsWinEXE
// @cdep pre $requiresbinary($BuildDir/binkw32.lib)
// @cdep pre $requiresbinary(ddraw.lib)
// @cdep post $BuildWinEXE( , )
