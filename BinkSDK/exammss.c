//############################################################################
//##                                                                        ##
//##  EXAMMSS.C                                                             ##
//##                                                                        ##
//##  Example of using Bink with Miles and BinkBuffers on Win32             ##
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


#include "bink.h"

#include "mss.h"

//
// Example globals
//

static HBINK Bink = 0;
static HBINKBUFFER Bink_buffer = 0;


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

      //
      // Redraw the frame (or the color mask for overlays).
      //

      if ( Bink_buffer )
        BinkBufferBlit( Bink_buffer, 0, 1 );

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

        if ( Bink_buffer )
        {
          S32 x,y;

          //
          // Yup, it's being moved - ask the BinkBuffer API to align the
          //   coordinates to a fast boundary.
          //

          x = ( ( WINDOWPOS* )lparam )->x;
          y = ( ( WINDOWPOS* )lparam )->y;
          BinkBufferCheckWinPos( Bink_buffer, &x, &y );
          ( ( WINDOWPOS* )lparam )->x = x;
          ( ( WINDOWPOS* )lparam )->y = y;
        }

      }
      break;

    case WM_WINDOWPOSCHANGED:
      //
      // Tell the BinkBuffer API when the window moves.
      //

      if ( Bink_buffer )
        BinkBufferSetOffset( Bink_buffer, 0, 0 );
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
//## Build_window_handle - creates a window class and window handle.        ##
//##                                                                        ##
//############################################################################

static HWND Build_window_handle( HINSTANCE instance,
                                 HINSTANCE previous_instance )
{
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
    wc.hCursor = LoadCursor( 0, IDC_ARROW );;
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
  // Return the new window with a tiny initial default size (it is resized
  //   later on with the help of the BinkBuffer API).
  //

  return( CreateWindow( "BinkExam",
                        "Bink Example Player",
			   WS_CAPTION|WS_POPUP|WS_CLIPCHILDREN|
                        WS_SYSMENU|WS_MINIMIZEBOX,
                        64, 64,
                        64, 64,
                        0, 0, instance,0 ) );
}


//############################################################################
//##                                                                        ##
//## Show_next_frame - advances to the next Bink frame.                     ##
//##                                                                        ##
//############################################################################

static void Show_next_frame( HBINK bink,
                             HBINKBUFFER bink_buffer,
                             HWND window )
{
  //
  // Decompress the Bink frame.
  //

  BinkDoFrame( bink );

  //
  // Lock the BinkBuffer so that we can copy the decompressed frame into it.
  //

  if ( BinkBufferLock( bink_buffer ) )
  {
    //
    // Copy the decompressed frame into the BinkBuffer (this might be on-screen).
    //

    BinkCopyToBuffer( bink,
                      bink_buffer->Buffer,
                      bink_buffer->BufferPitch,
                      bink_buffer->Height,
                      0,0,
                      bink_buffer->SurfaceType);

    //
    // Unlock the BinkBuffer.
    //

    BinkBufferUnlock( bink_buffer );
  }

  //
  // Tell the BinkBuffer to blit the pixels onto the screen (if the
  //   BinkBuffer is using an off-screen blitting style).
  //

  BinkBufferBlit( bink_buffer,
                  bink->FrameRects,
                  BinkGetRects( bink, bink_buffer->SurfaceType ) );

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
  MSG msg;

  //
  // Miles locals
  //

  HDIGDRIVER digital=0;


  //
  // Try to create our window.
  //

  window = Build_window_handle( instance,
                                previous_instance );
  if ( !window )
  {
    MessageBox( 0,
                "Error creating window.",
                "Windows",
                MB_OK | MB_ICONSTOP );
    return( 1 );
  }

  //
  // Open the Miles Sound System.

  AIL_startup();

  digital = AIL_open_digital_driver( 44100, 16, 2, 0 );
  if ( !digital )
  {
    MessageBox( 0,
                AIL_last_error( ),
                "Error opening Miles",
                MB_OK | MB_ICONSTOP );
    return( 2 );
  }


  //
  //
  // Tell Bink to use the Miles Sound System (must be before BinkOpen)!
  //

  BinkSoundUseMiles( digital );

  //
  // Try to open the Bink file.
  //

  Bink = BinkOpen( cmd_line, 0 );
  if ( !Bink )
  {
    MessageBox( 0,
                BinkGetError( ),
                "Bink Error",
                MB_OK | MB_ICONSTOP );

    DestroyWindow( window );
    return( 3 );
  }


  //
  // Try to open the Bink buffer.
  //

  Bink_buffer = BinkBufferOpen( window, Bink->Width, Bink->Height, 0 );
  if ( !Bink_buffer )
  {
    MessageBox( 0,
                BinkBufferGetError( ),
                "Bink Error",
                MB_OK | MB_ICONSTOP );

    DestroyWindow( window );
    BinkClose( Bink );

    return( 4 );
  }

  //
  // Size the window such that its client area exactly fits our Bink movie.
  //

  SetWindowPos( window, 0,
                0, 0,
                Bink_buffer->WindowWidth,
                Bink_buffer->WindowHeight,
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
                         Bink_buffer,
                         window );
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
  // Close the Bink buffer.
  //

  if ( Bink_buffer )
  {
    BinkBufferClose( Bink_buffer );
    Bink_buffer = 0;
  }

  //
  // Close Miles.
  //

  if ( digital )
  {
    AIL_close_digital_driver( digital );
    digital = 0;
  }

  AIL_shutdown( );

  //
  // And exit.
  //

  return( 0 );
}

// some stuff for the RAD build utility
// @cdep pre $DefaultsWinEXE
// @cdep pre $set(INCs, $INCs -I..\..\..\..\mss\ship\sdk\win32\include )
// @cdep pre $requiresbinary($BuildDir/binkw32.lib)
// @cdep pre $requiresbinary(..\..\mss\ship\sdk\win32\lib\mss32.lib)
// @cdep post $BuildWinEXE( , )
