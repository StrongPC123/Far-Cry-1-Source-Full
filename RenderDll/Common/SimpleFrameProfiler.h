//////////////////////////////////////////////////////////////////////////
// A simple profiler useful for collecting multiple call times per frame
// and displaying their different average statistics.
// For usage, see the bottom of the file
//////////////////////////////////////////////////////////////////////////


#ifndef _SIMPLE_FRAME_PROFILER_
#define _SIMPLE_FRAME_PROFILER_

// set #if 0 here if you don't want profiling to be compiled in the code
#if ENABLE_FRAME_PROFILER
#define PROFILE_FRAME(id) FRAME_PROFILER_FAST( "Renderer:" #id,iSystem,PROFILE_RENDERER,g_bProfilerEnabled )
#else
#define PROFILE_FRAME(id)
#endif


#endif