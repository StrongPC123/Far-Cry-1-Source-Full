/* ========================================================================================== */
/* CRYSOUND Main header file. Copyright (c), Firelight Technologies Pty, Ltd 1999-2002.           */
/* ========================================================================================== */

#ifndef _CS_H_
#define _CS_H_

/* ========================================================================================== */
/* DEFINITIONS                                                                                */
/* ========================================================================================== */

#if (!defined(WIN32) && !defined(__WIN32__) && !defined(_WIN32_WCE)) || (defined(__GNUC__) && defined(WIN32))
    #ifndef _cdecl
        #define _cdecl 
    #endif
    #ifndef _stdcall
        #define _stdcall
    #endif
#endif 

#define F_API _stdcall
#define F_CALLBACKAPI _cdecl

#ifdef DLL_EXPORTS
    #define DLL_API __declspec(dllexport)
#else
    #if defined(__LCC__) || defined(__MINGW32__) || defined(__CYGWIN32__)
        #define DLL_API F_API
    #else
        #define DLL_API
    #endif /* __LCC__ ||  __MINGW32__ || __CYGWIN32__ */
#endif /* DLL_EXPORTS */

#define CS_VERSION    3.61f

/* 
    CRYSOUND defined types 
*/
typedef struct CS_SAMPLE    CS_SAMPLE;
typedef struct CS_STREAM    CS_STREAM;
typedef struct CS_DSPUNIT   CS_DSPUNIT;
typedef struct CM_MODULE    CM_MODULE;

/* 
    Callback types
*/
typedef void *      (F_CALLBACKAPI *CS_DSPCALLBACK)    (void *originalbuffer, void *newbuffer, int length, int param);
typedef signed char (F_CALLBACKAPI *CS_STREAMCALLBACK) (CS_STREAM *stream, void *buff, int len, int param);
typedef void        (F_CALLBACKAPI *CM_CALLBACK)       (CM_MODULE *mod, unsigned char param);

typedef unsigned int(F_CALLBACKAPI *CS_OPENCALLBACK)   (const char *name);
typedef void        (F_CALLBACKAPI *CS_CLOSECALLBACK)  (unsigned int handle);
typedef int         (F_CALLBACKAPI *CS_READCALLBACK)   (void *buffer, int size, unsigned int handle);
typedef int         (F_CALLBACKAPI *CS_SEEKCALLBACK)   (unsigned int handle, int pos, signed char mode);
typedef int         (F_CALLBACKAPI *CS_TELLCALLBACK)   (unsigned int handle);

typedef void *      (F_CALLBACKAPI *CS_ALLOCCALLBACK)  (unsigned int size);
typedef void *      (F_CALLBACKAPI *CS_REALLOCCALLBACK)(void *ptr, unsigned int size);
typedef void        (F_CALLBACKAPI *CS_FREECALLBACK)   (void *ptr);


/*
[ENUM]
[
    [DESCRIPTION]   
    On failure of commands in CRYSOUND, use CS_GetError to attain what happened.
    
    [SEE_ALSO]      
    CS_GetError
]
*/
enum CS_ERRORS 
{
    CS_ERR_NONE,             /* No errors */
    CS_ERR_BUSY,             /* Cannot call this command after CS_Init.  Call CS_Close first. */
    CS_ERR_UNINITIALIZED,    /* This command failed because CS_Init or CS_SetOutput was not called */
    CS_ERR_INIT,             /* Error initializing output device. */
    CS_ERR_ALLOCATED,        /* Error initializing output device, but more specifically, the output device is already in use and cannot be reused. */
    CS_ERR_PLAY,             /* Playing the sound failed. */
    CS_ERR_OUTPUT_FORMAT,    /* Soundcard does not support the features needed for this soundsystem (16bit stereo output) */
    CS_ERR_COOPERATIVELEVEL, /* Error setting cooperative level for hardware. */
    CS_ERR_CREATEBUFFER,     /* Error creating hardware sound buffer. */
    CS_ERR_FILE_NOTFOUND,    /* File not found */
    CS_ERR_FILE_FORMAT,      /* Unknown file format */
    CS_ERR_FILE_BAD,         /* Error loading file */
    CS_ERR_MEMORY,           /* Not enough memory or resources */
    CS_ERR_VERSION,          /* The version number of this file format is not supported */
    CS_ERR_INVALID_PARAM,    /* An invalid parameter was passed to this function */
    CS_ERR_NO_EAX,           /* Tried to use an EAX command on a non EAX enabled channel or output. */
    CS_ERR_CHANNEL_ALLOC,    /* Failed to allocate a new channel */
    CS_ERR_RECORD,           /* Recording is not supported on this machine */
    CS_ERR_MEDIAPLAYER,      /* Windows Media Player not installed so cannot play wma or use internet streaming. */
    CS_ERR_CDDEVICE          /* An error occured trying to open the specified CD device */
};


/*
[ENUM]
[
    [DESCRIPTION]   
    These output types are used with CS_SetOutput, to choose which output driver to use.
    
    CS_OUTPUT_DSOUND will not support hardware 3d acceleration if the sound card driver 
    does not support DirectX 6 Voice Manager Extensions.

    CS_OUTPUT_WINMM is recommended for NT and CE.

    [SEE_ALSO]      
    CS_SetOutput
    CS_GetOutput
]
*/
enum CS_OUTPUTTYPES
{
    CS_OUTPUT_NOSOUND,    /* NoSound driver, all calls to this succeed but do nothing. */
    CS_OUTPUT_WINMM,      /* Windows Multimedia driver. */
    CS_OUTPUT_DSOUND,     /* DirectSound driver.  You need this to get EAX2 or EAX3 support, or FX api support. */
    CS_OUTPUT_A3D,        /* A3D driver. */

    CS_OUTPUT_OSS,        /* Linux/Unix OSS (Open Sound System) driver, i.e. the kernel sound drivers. */
    CS_OUTPUT_ESD,        /* Linux/Unix ESD (Enlightment Sound Daemon) driver. */
    CS_OUTPUT_ALSA,       /* Linux Alsa driver. */

    CS_OUTPUT_ASIO,       /* Low latency ASIO driver */
    CS_OUTPUT_XBOX,       /* Xbox driver */
    CS_OUTPUT_PS2,        /* PlayStation 2 driver */
    CS_OUTPUT_MAC,        /* Mac SoundManager driver */
    CS_OUTPUT_GC,         /* Gamecube driver */

    CS_OUTPUT_NOSOUND_NONREALTIME   /* This is the same as nosound, but the sound generation is driven by CS_Update */
};


/*
[ENUM]
[
    [DESCRIPTION]   
    These mixer types are used with CS_SetMixer, to choose which mixer to use, or to act 
    upon for other reasons using CS_GetMixer.
    It is not nescessary to set the mixer.  CRYSOUND will autodetect the best mixer for you.

    [SEE_ALSO]      
    CS_SetMixer
    CS_GetMixer
]
*/
enum CS_MIXERTYPES
{
    CS_MIXER_AUTODETECT,        /* CE/PS2/GC Only - Non interpolating/low quality mixer. */
    CS_MIXER_BLENDMODE,         /* Removed / obsolete. */
    CS_MIXER_MMXP5,             /* Removed / obsolete. */
    CS_MIXER_MMXP6,             /* Removed / obsolete. */

    CS_MIXER_QUALITY_AUTODETECT,/* All platforms - Autodetect the fastest quality mixer based on your cpu. */
    CS_MIXER_QUALITY_FPU,       /* Win32/Linux only - Interpolating/volume ramping FPU mixer.  */
    CS_MIXER_QUALITY_MMXP5,     /* Win32/Linux only - Interpolating/volume ramping P5 MMX mixer.  */
    CS_MIXER_QUALITY_MMXP6,     /* Win32/Linux only - Interpolating/volume ramping ppro+ MMX mixer. */

    CS_MIXER_MONO,              /* CE/PS2/GC only - MONO non interpolating/low quality mixer. For speed*/
    CS_MIXER_QUALITY_MONO,      /* CE/PS2/GC only - MONO Interpolating mixer.  For speed */

    CS_MIXER_MAX
};


/*
[ENUM]
[
    [DESCRIPTION]   
    These definitions describe the type of song being played.

    [SEE_ALSO]      
    CM_GetType  
]
*/
enum CM_TYPES
{
    CM_TYPE_NONE,       
    CM_TYPE_MOD,        /* Protracker / Fasttracker */
    CM_TYPE_S3M,        /* ScreamTracker 3 */
    CM_TYPE_XM,         /* FastTracker 2 */
    CM_TYPE_IT,         /* Impulse Tracker. */
    CM_TYPE_MIDI        /* MIDI file */
};


/*
[DEFINE_START] 
[
    [NAME] 
    CS_DSP_PRIORITIES

    [DESCRIPTION]   
    These default priorities are used by CRYSOUND internal system DSP units.  They describe the 
    position of the DSP chain, and the order of how audio processing is executed.
    You can actually through the use of CS_DSP_GetxxxUnit (where xxx is the name of the DSP
    unit), disable or even change the priority of a DSP unit.

    [SEE_ALSO]      
    CS_DSP_Create
    CS_DSP_SetPriority
    CS_DSP_GetSpectrum
]
*/
#define CS_DSP_DEFAULTPRIORITY_CLEARUNIT        0       /* DSP CLEAR unit - done first */
#define CS_DSP_DEFAULTPRIORITY_SFXUNIT          100     /* DSP SFX unit - done second */
#define CS_DSP_DEFAULTPRIORITY_MUSICUNIT        200     /* DSP MUSIC unit - done third */
#define CS_DSP_DEFAULTPRIORITY_USER             300     /* User priority, use this as reference */
#define CS_DSP_DEFAULTPRIORITY_FFTUNIT          900     /* This reads data for CS_DSP_GetSpectrum, so it comes after user units */
#define CS_DSP_DEFAULTPRIORITY_CLIPANDCOPYUNIT  1000    /* DSP CLIP AND COPY unit - last */
/* [DEFINE_END] */


/*
[DEFINE_START] 
[
    [NAME] 
    CS_CAPS

    [DESCRIPTION]   
    Driver description bitfields.  Use CS_Driver_GetCaps to determine if a driver enumerated
    has the settings you are after.  The enumerated driver depends on the output mode, see
    CS_OUTPUTTYPES

    [SEE_ALSO]
    CS_GetDriverCaps
    CS_OUTPUTTYPES
]
*/
#define CS_CAPS_HARDWARE                0x1     /* This driver supports hardware accelerated 3d sound. */
#define CS_CAPS_EAX2                    0x2     /* This driver supports EAX 2 reverb */
#define CS_CAPS_EAX3                    0x10    /* This driver supports EAX 3 reverb */
/* [DEFINE_END] */


/*
[DEFINE_START] 
[
    [NAME] 
    CS_MODES
    
    [DESCRIPTION]   
    Sample description bitfields, OR them together for loading and describing samples.
    NOTE.  If the file format being loaded already has a defined format, such as WAV or MP3, then 
    trying to override the pre-defined format with a new set of format flags will not work.  For
    example, an 8 bit WAV file will not load as 16bit if you specify CS_16BITS.  It will just
    ignore the flag and go ahead loading it as 8bits.  For these type of formats the only flags
    you can specify that will really alter the behaviour of how it is loaded, are the following.

    CS_LOOP_OFF     
    CS_LOOP_NORMAL  
    CS_LOOP_BIDI    
    CS_HW3D
    CS_2D
    CS_STREAMABLE   
    CS_LOADMEMORY   
    CS_LOADRAW          
    CS_MPEGACCURATE     

    See flag descriptions for what these do.
]
*/
#define CS_LOOP_OFF     0x00000001  /* For non looping samples. */
#define CS_LOOP_NORMAL  0x00000002  /* For forward looping samples. */
#define CS_LOOP_BIDI    0x00000004  /* For bidirectional looping samples.  (no effect if in hardware). */
#define CS_8BITS        0x00000008  /* For 8 bit samples. */
#define CS_16BITS       0x00000010  /* For 16 bit samples. */
#define CS_MONO         0x00000020  /* For mono samples. */
#define CS_STEREO       0x00000040  /* For stereo samples. */
#define CS_UNSIGNED     0x00000080  /* For user created source data containing unsigned samples. */
#define CS_SIGNED       0x00000100  /* For user created source data containing signed data. */
#define CS_DELTA        0x00000200  /* For user created source data stored as delta values. */
#define CS_IT214        0x00000400  /* For user created source data stored using IT214 compression. */
#define CS_IT215        0x00000800  /* For user created source data stored using IT215 compression. */
#define CS_HW3D         0x00001000  /* Attempts to make samples use 3d hardware acceleration. (if the card supports it) */
#define CS_2D           0x00002000  /* Tells software (not hardware) based sample not to be included in 3d processing. */
#define CS_STREAMABLE   0x00004000  /* For a streamimg sound where you feed the data to it. */
#define CS_LOADMEMORY   0x00008000  /* "name" will be interpreted as a pointer to data for streaming and samples. */
#define CS_LOADRAW      0x00010000  /* Will ignore file format and treat as raw pcm. */
#define CS_MPEGACCURATE 0x00020000  /* For CS_Stream_OpenFile - for accurate CS_Stream_GetLengthMs/CS_Stream_SetTime.  WARNING, see CS_Stream_OpenFile for inital opening time performance issues. */
#define CS_FORCEMONO    0x00040000  /* For forcing stereo streams and samples to be mono - needed if using CS_HW3D and stereo data - incurs a small speed hit for streams */
#define CS_HW2D         0x00080000  /* 2D hardware sounds.  allows hardware specific effects */
#define CS_ENABLEFX     0x00100000  /* Allows DX8 FX to be played back on a sound.  Requires DirectX 8 - Note these sounds cannot be played more than once, be 8 bit, be less than a certain size, or have a changing frequency */
#define CS_MPEGHALFRATE 0x00200000  /* For CRYSOUNDCE only - decodes mpeg streams using a lower quality decode, but faster execution */
#define CS_XADPCM       0x00400000  /* For XBOX only - Contents are compressed as XADPCM  */
#define CS_VAG          0x00800000  /* For PS2 only - Contents are compressed as Sony VAG format */
#define CS_NONBLOCKING  0x01000000  /* For CS_Stream_OpenFile - Causes stream to open in the background and not block the foreground app - stream functions only work when ready.  Poll any stream function determine when it IS ready. */
#define CS_GCADPCM      0x02000000  /* For Gamecube only - Contents are compressed as Gamecube DSP-ADPCM format */

#define CS_NORMAL       (CS_16BITS | CS_SIGNED | CS_MONO)      
/* [DEFINE_END] */



/*
[DEFINE_START] 
[
    [NAME] 
    CS_CDPLAYMODES
    
    [DESCRIPTION]   
    Playback method for a CD Audio track, with CS_CD_SetPlayMode

    [SEE_ALSO]    
    CS_CD_SetPlayMode  
    CS_CD_Play
]
*/
#define CS_CD_PLAYCONTINUOUS    0   /* Starts from the current track and plays to end of CD. */
#define CS_CD_PLAYONCE          1   /* Plays the specified track then stops. */
#define CS_CD_PLAYLOOPED        2   /* Plays the specified track looped, forever until stopped manually. */
#define CS_CD_PLAYRANDOM        3   /* Plays tracks in random order */
/* [DEFINE_END] */


/*
[DEFINE_START] 
[
    [NAME] 
    CS_MISC_VALUES
    
    [DESCRIPTION]
    Miscellaneous values for CRYSOUND functions.

    [SEE_ALSO]
    CS_PlaySound
    CS_PlaySoundEx
    CS_Sample_Alloc
    CS_Sample_Load
    CS_SetPan
]
*/
#define CS_FREE             -1      /* value to play on any free channel, or to allocate a sample in a free sample slot. */
#define CS_UNMANAGED        -2      /* value to allocate a sample that is NOT managed by CRYSOUND or placed in a sample slot. */
#define CS_ALL              -3      /* for a channel index , this flag will affect ALL channels available!  Not supported by every function. */
#define CS_STEREOPAN        -1      /* value for CS_SetPan so that stereo sounds are not played at half volume.  See CS_SetPan for more on this. */
#define CS_SYSTEMCHANNEL    -1000   /* special 'channel' ID for all channel based functions that want to alter the global CRYSOUND software mixing output channel */
#define CS_SYSTEMSAMPLE     -1000   /* special 'sample' ID for all sample based functions that want to alter the global CRYSOUND software mixing output sample */

/* [DEFINE_END] */


/*
[STRUCTURE] 
[
    [DESCRIPTION]
    Structure defining a reverb environment.
    For more indepth descriptions of the reverb properties under win32, please see the EAX2 and EAX3
    documentation at http://developer.creative.com/ under the 'downloads' section.
    If they do not have the EAX3 documentation, then most information can be attained from
    the EAX2 documentation, as EAX3 only adds some more parameters and functionality on top of 
    EAX2.
    Note the default reverb properties are the same as the CS_PRESET_GENERIC preset.
    Note that integer values that typically range from -10,000 to 1000 are represented in 
    decibels, and are of a logarithmic scale, not linear, wheras float values are typically linear.
    PORTABILITY: Each member has the platform it supports in braces ie (win32/xbox).  
    Some reverb parameters are only supported in win32 and some only on xbox. If all parameters are set then
    the reverb should product a similar effect on either platform.
    Only Win32 supports the reverb api.
    
    The numerical values listed below are the maximum, minimum and default values for each variable respectively.

    [SEE_ALSO]
    CS_Reverb_SetProperties
    CS_Reverb_GetProperties
    CS_REVERB_PRESETS
    CS_REVERB_FLAGS
]
*/
typedef struct _CS_REVERB_PROPERTIES /* MIN     MAX    DEFAULT   DESCRIPTION */
{                                   
    unsigned int Environment;            /* 0     , 25    , 0      , sets all listener properties (win32/ps2 only) */
    float        EnvSize;                /* 1.0   , 100.0 , 7.5    , environment size in meters (win32 only) */
    float        EnvDiffusion;           /* 0.0   , 1.0   , 1.0    , environment diffusion (win32/xbox) */
    int          Room;                   /* -10000, 0     , -1000  , room effect level (at mid frequencies) (win32/xbox/ps2) */
    int          RoomHF;                 /* -10000, 0     , -100   , relative room effect level at high frequencies (win32/xbox) */
    int          RoomLF;                 /* -10000, 0     , 0      , relative room effect level at low frequencies (win32 only) */
    float        DecayTime;              /* 0.1   , 20.0  , 1.49   , reverberation decay time at mid frequencies (win32/xbox) */
    float        DecayHFRatio;           /* 0.1   , 2.0   , 0.83   , high-frequency to mid-frequency decay time ratio (win32/xbox) */
    float        DecayLFRatio;           /* 0.1   , 2.0   , 1.0    , low-frequency to mid-frequency decay time ratio (win32 only) */
    int          Reflections;            /* -10000, 1000  , -2602  , early reflections level relative to room effect (win32/xbox) */
    float        ReflectionsDelay;       /* 0.0   , 0.3   , 0.007  , initial reflection delay time (win32/xbox) */
    float        ReflectionsPan[3];      /*       ,       , [0,0,0], early reflections panning vector (win32 only) */
    int          Reverb;                 /* -10000, 2000  , 200    , late reverberation level relative to room effect (win32/xbox) */
    float        ReverbDelay;            /* 0.0   , 0.1   , 0.011  , late reverberation delay time relative to initial reflection (win32/xbox) */
    float        ReverbPan[3];           /*       ,       , [0,0,0], late reverberation panning vector (win32 only) */
    float        EchoTime;               /* .075  , 0.25  , 0.25   , echo time (win32 only) */
    float        EchoDepth;              /* 0.0   , 1.0   , 0.0    , echo depth (win32 only) */
    float        ModulationTime;         /* 0.04  , 4.0   , 0.25   , modulation time (win32 only) */
    float        ModulationDepth;        /* 0.0   , 1.0   , 0.0    , modulation depth (win32 only) */
    float        AirAbsorptionHF;        /* -100  , 0.0   , -5.0   , change in level per meter at high frequencies (win32 only) */
    float        HFReference;            /* 1000.0, 20000 , 5000.0 , reference high frequency (hz) (win32/xbox) */
    float        LFReference;            /* 20.0  , 1000.0, 250.0  , reference low frequency (hz) (win32 only) */
    float        RoomRolloffFactor;      /* 0.0   , 10.0  , 0.0    , like CS_3D_SetRolloffFactor but for room effect (win32/xbox) */
    float        Diffusion;              /* 0.0   , 100.0 , 100.0  , Value that controls the echo density in the late reverberation decay. (xbox only) */
    float        Density;                /* 0.0   , 100.0 , 100.0  , Value that controls the modal density in the late reverberation decay (xbox only) */
    unsigned int Flags;                  /* CS_REVERB_FLAGS - modifies the behavior of above properties (win32 only) */
} CS_REVERB_PROPERTIES;


/*
[DEFINE_START] 
[
    [NAME] 
    CS_REVERB_FLAGS
    
    [DESCRIPTION]
    Values for the Flags member of the CS_REVERB_PROPERTIES structure.

    [SEE_ALSO]
    CS_REVERB_PROPERTIES
]
*/
#define CS_REVERB_FLAGS_DECAYTIMESCALE        0x00000001 /* 'EnvSize' affects reverberation decay time */
#define CS_REVERB_FLAGS_REFLECTIONSSCALE      0x00000002 /* 'EnvSize' affects reflection level */
#define CS_REVERB_FLAGS_REFLECTIONSDELAYSCALE 0x00000004 /* 'EnvSize' affects initial reflection delay time */
#define CS_REVERB_FLAGS_REVERBSCALE           0x00000008 /* 'EnvSize' affects reflections level */
#define CS_REVERB_FLAGS_REVERBDELAYSCALE      0x00000010 /* 'EnvSize' affects late reverberation delay time */
#define CS_REVERB_FLAGS_DECAYHFLIMIT          0x00000020 /* AirAbsorptionHF affects DecayHFRatio */
#define CS_REVERB_FLAGS_ECHOTIMESCALE         0x00000040 /* 'EnvSize' affects echo time */
#define CS_REVERB_FLAGS_MODULATIONTIMESCALE   0x00000080 /* 'EnvSize' affects modulation time */
#define CS_REVERB_FLAGS_CORE0                 0x00000100 /* PS2 Only - Reverb is applied to CORE0 (hw voices 0-23) */
#define CS_REVERB_FLAGS_CORE1                 0x00000200 /* PS2 Only - Reverb is applied to CORE1 (hw voices 24-47) */
#define CS_REVERB_FLAGS_DEFAULT              (CS_REVERB_FLAGS_DECAYTIMESCALE |        \
                                                  CS_REVERB_FLAGS_REFLECTIONSSCALE |      \
                                                  CS_REVERB_FLAGS_REFLECTIONSDELAYSCALE | \
                                                  CS_REVERB_FLAGS_REVERBSCALE |           \
                                                  CS_REVERB_FLAGS_REVERBDELAYSCALE |      \
                                                  CS_REVERB_FLAGS_DECAYHFLIMIT |          \
                                                  CS_REVERB_FLAGS_CORE0 |                 \
                                                  CS_REVERB_FLAGS_CORE1 )
/* [DEFINE_END] */




/*
[DEFINE_START] 
[
    [NAME] 
    CS_REVERB_PRESETS
    
    [DESCRIPTION]   
    A set of predefined environment PARAMETERS, created by Creative Labs
    These are used to initialize an CS_REVERB_PROPERTIES structure statically.
    ie 
    CS_REVERB_PROPERTIES prop = CS_PRESET_GENERIC;

    [SEE_ALSO]
    CS_Reverb_SetProperties
]
*/
/*                                     Env  Size    Diffus  Room   RoomHF  RmLF DecTm   DecHF  DecLF   Refl  RefDel  RefPan               Revb  RevDel  ReverbPan           EchoTm  EchDp  ModTm  ModDp  AirAbs  HFRef    LFRef  RRlOff Diffus  Densty  FLAGS */
#define CS_PRESET_OFF              {0,	7.5f,	1.00f, -10000, -10000, 0,   1.00f,  1.00f, 1.0f,  -2602, 0.007f, { 0.0f,0.0f,0.0f },   200, 0.011f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f,   0.0f,   0.0f, 0x33f }
#define CS_PRESET_GENERIC          {0,	7.5f,	1.00f, -1000,  -100,   0,   1.49f,  0.83f, 1.0f,  -2602, 0.007f, { 0.0f,0.0f,0.0f },   200, 0.011f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
#define CS_PRESET_PADDEDCELL       {1,	1.4f,	1.00f, -1000,  -6000,  0,   0.17f,  0.10f, 1.0f,  -1204, 0.001f, { 0.0f,0.0f,0.0f },   207, 0.002f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
#define CS_PRESET_ROOM 	           {2,	1.9f,	1.00f, -1000,  -454,   0,   0.40f,  0.83f, 1.0f,  -1646, 0.002f, { 0.0f,0.0f,0.0f },    53, 0.003f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
#define CS_PRESET_BATHROOM 	       {3,	1.4f,	1.00f, -1000,  -1200,  0,   1.49f,  0.54f, 1.0f,   -370, 0.007f, { 0.0f,0.0f,0.0f },  1030, 0.011f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f,  60.0f, 0x3f }
#define CS_PRESET_LIVINGROOM       {4,	2.5f,	1.00f, -1000,  -6000,  0,   0.50f,  0.10f, 1.0f,  -1376, 0.003f, { 0.0f,0.0f,0.0f }, -1104, 0.004f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
#define CS_PRESET_STONEROOM        {5,	11.6f,	1.00f, -1000,  -300,   0,   2.31f,  0.64f, 1.0f,   -711, 0.012f, { 0.0f,0.0f,0.0f },    83, 0.017f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
#define CS_PRESET_AUDITORIUM       {6,	21.6f,	1.00f, -1000,  -476,   0,   4.32f,  0.59f, 1.0f,   -789, 0.020f, { 0.0f,0.0f,0.0f },  -289, 0.030f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
#define CS_PRESET_CONCERTHALL      {7,	19.6f,	1.00f, -1000,  -500,   0,   3.92f,  0.70f, 1.0f,  -1230, 0.020f, { 0.0f,0.0f,0.0f },    -2, 0.029f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
#define CS_PRESET_CAVE             {8,	14.6f,	1.00f, -1000,  0,      0,   2.91f,  1.30f, 1.0f,   -602, 0.015f, { 0.0f,0.0f,0.0f },  -302, 0.022f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x1f }
#define CS_PRESET_ARENA            {9,	36.2f,	1.00f, -1000,  -698,   0,   7.24f,  0.33f, 1.0f,  -1166, 0.020f, { 0.0f,0.0f,0.0f },    16, 0.030f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
#define CS_PRESET_HANGAR           {10,	50.3f,	1.00f, -1000,  -1000,  0,   10.05f, 0.23f, 1.0f,   -602, 0.020f, { 0.0f,0.0f,0.0f },   198, 0.030f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
#define CS_PRESET_CARPETTEDHALLWAY {11,	1.9f,	1.00f, -1000,  -4000,  0,   0.30f,  0.10f, 1.0f,  -1831, 0.002f, { 0.0f,0.0f,0.0f }, -1630, 0.030f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
#define CS_PRESET_HALLWAY          {12,	1.8f,	1.00f, -1000,  -300,   0,   1.49f,  0.59f, 1.0f,  -1219, 0.007f, { 0.0f,0.0f,0.0f },   441, 0.011f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
#define CS_PRESET_STONECORRIDOR    {13,	13.5f,	1.00f, -1000,  -237,   0,   2.70f,  0.79f, 1.0f,  -1214, 0.013f, { 0.0f,0.0f,0.0f },   395, 0.020f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
#define CS_PRESET_ALLEY 	       {14,	7.5f,	0.30f, -1000,  -270,   0,   1.49f,  0.86f, 1.0f,  -1204, 0.007f, { 0.0f,0.0f,0.0f },    -4, 0.011f, { 0.0f,0.0f,0.0f }, 0.125f, 0.95f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
#define CS_PRESET_FOREST 	       {15,	38.0f,	0.30f, -1000,  -3300,  0,   1.49f,  0.54f, 1.0f,  -2560, 0.162f, { 0.0f,0.0f,0.0f },  -229, 0.088f, { 0.0f,0.0f,0.0f }, 0.125f, 1.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f,  79.0f, 100.0f, 0x3f }
#define CS_PRESET_CITY             {16,	7.5f,	0.50f, -1000,  -800,   0,   1.49f,  0.67f, 1.0f,  -2273, 0.007f, { 0.0f,0.0f,0.0f }, -1691, 0.011f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f,  50.0f, 100.0f, 0x3f }
#define CS_PRESET_MOUNTAINS        {17,	100.0f, 0.27f, -1000,  -2500,  0,   1.49f,  0.21f, 1.0f,  -2780, 0.300f, { 0.0f,0.0f,0.0f }, -1434, 0.100f, { 0.0f,0.0f,0.0f }, 0.250f, 1.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f,  27.0f, 100.0f, 0x1f }
#define CS_PRESET_QUARRY           {18,	17.5f,	1.00f, -1000,  -1000,  0,   1.49f,  0.83f, 1.0f, -10000, 0.061f, { 0.0f,0.0f,0.0f },   500, 0.025f, { 0.0f,0.0f,0.0f }, 0.125f, 0.70f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
#define CS_PRESET_PLAIN            {19,	42.5f,	0.21f, -1000,  -2000,  0,   1.49f,  0.50f, 1.0f,  -2466, 0.179f, { 0.0f,0.0f,0.0f }, -1926, 0.100f, { 0.0f,0.0f,0.0f }, 0.250f, 1.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f,  21.0f, 100.0f, 0x3f }
#define CS_PRESET_PARKINGLOT       {20,	8.3f,	1.00f, -1000,  0,      0,   1.65f,  1.50f, 1.0f,  -1363, 0.008f, { 0.0f,0.0f,0.0f }, -1153, 0.012f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x1f }
#define CS_PRESET_SEWERPIPE        {21,	1.7f,	0.80f, -1000,  -1000,  0,   2.81f,  0.14f, 1.0f,    429, 0.014f, { 0.0f,0.0f,0.0f },  1023, 0.021f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f,  80.0f,  60.0f, 0x3f }
#define CS_PRESET_UNDERWATER       {22,	1.8f,	1.00f, -1000,  -4000,  0,   1.49f,  0.10f, 1.0f,   -449, 0.007f, { 0.0f,0.0f,0.0f },  1700, 0.011f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 1.18f, 0.348f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }

/* Non I3DL2 presets */

#define CS_PRESET_DRUGGED          {23,	1.9f,	0.50f, -1000,  0,      0,   8.39f,  1.39f, 1.0f,  -115,  0.002f, { 0.0f,0.0f,0.0f },   985, 0.030f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 0.25f, 1.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x1f }
#define CS_PRESET_DIZZY            {24,	1.8f,	0.60f, -1000,  -400,   0,   17.23f, 0.56f, 1.0f,  -1713, 0.020f, { 0.0f,0.0f,0.0f },  -613, 0.030f, { 0.0f,0.0f,0.0f }, 0.250f, 1.00f, 0.81f, 0.310f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x1f }
#define CS_PRESET_PSYCHOTIC        {25,	1.0f,	0.50f, -1000,  -151,   0,   7.56f,  0.91f, 1.0f,  -626,  0.020f, { 0.0f,0.0f,0.0f },   774, 0.030f, { 0.0f,0.0f,0.0f }, 0.250f, 0.00f, 4.00f, 1.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x1f }

/* PlayStation 2 Only presets */

#define CS_PRESET_PS2_ROOM         {1,	0,	    0,         0,  0,      0,   0.0f,   0.0f,  0.0f,     0,  0.000f, { 0.0f,0.0f,0.0f },     0, 0.000f, { 0.0f,0.0f,0.0f }, 0.000f, 0.00f, 0.00f, 0.000f,  0.0f, 0000.0f,   0.0f, 0.0f,   0.0f,   0.0f, 0x31f }
#define CS_PRESET_PS2_STUDIO_A     {2,	0,	    0,         0,  0,      0,   0.0f,   0.0f,  0.0f,     0,  0.000f, { 0.0f,0.0f,0.0f },     0, 0.000f, { 0.0f,0.0f,0.0f }, 0.000f, 0.00f, 0.00f, 0.000f,  0.0f, 0000.0f,   0.0f, 0.0f,   0.0f,   0.0f, 0x31f }
#define CS_PRESET_PS2_STUDIO_B     {3,	0,	    0,         0,  0,      0,   0.0f,   0.0f,  0.0f,     0,  0.000f, { 0.0f,0.0f,0.0f },     0, 0.000f, { 0.0f,0.0f,0.0f }, 0.000f, 0.00f, 0.00f, 0.000f,  0.0f, 0000.0f,   0.0f, 0.0f,   0.0f,   0.0f, 0x31f }
#define CS_PRESET_PS2_STUDIO_C     {4,	0,	    0,         0,  0,      0,   0.0f,   0.0f,  0.0f,     0,  0.000f, { 0.0f,0.0f,0.0f },     0, 0.000f, { 0.0f,0.0f,0.0f }, 0.000f, 0.00f, 0.00f, 0.000f,  0.0f, 0000.0f,   0.0f, 0.0f,   0.0f,   0.0f, 0x31f }
#define CS_PRESET_PS2_HALL         {5,	0,	    0,         0,  0,      0,   0.0f,   0.0f,  0.0f,     0,  0.000f, { 0.0f,0.0f,0.0f },     0, 0.000f, { 0.0f,0.0f,0.0f }, 0.000f, 0.00f, 0.00f, 0.000f,  0.0f, 0000.0f,   0.0f, 0.0f,   0.0f,   0.0f, 0x31f }
#define CS_PRESET_PS2_SPACE        {6,	0,	    0,         0,  0,      0,   0.0f,   0.0f,  0.0f,     0,  0.000f, { 0.0f,0.0f,0.0f },     0, 0.000f, { 0.0f,0.0f,0.0f }, 0.000f, 0.00f, 0.00f, 0.000f,  0.0f, 0000.0f,   0.0f, 0.0f,   0.0f,   0.0f, 0x31f }
#define CS_PRESET_PS2_ECHO         {7,	0,	    0,         0,  0,      0,   0.0f,   0.0f,  0.0f,     0,  0.000f, { 0.0f,0.0f,0.0f },     0, 0.000f, { 0.0f,0.0f,0.0f }, 0.000f, 0.00f, 0.00f, 0.000f,  0.0f, 0000.0f,   0.0f, 0.0f,   0.0f,   0.0f, 0x31f }
#define CS_PRESET_PS2_DELAY        {8,	0,	    0,         0,  0,      0,   0.0f,   0.0f,  0.0f,     0,  0.000f, { 0.0f,0.0f,0.0f },     0, 0.000f, { 0.0f,0.0f,0.0f }, 0.000f, 0.00f, 0.00f, 0.000f,  0.0f, 0000.0f,   0.0f, 0.0f,   0.0f,   0.0f, 0x31f }
#define CS_PRESET_PS2_PIPE         {9,	0,	    0,         0,  0,      0,   0.0f,   0.0f,  0.0f,     0,  0.000f, { 0.0f,0.0f,0.0f },     0, 0.000f, { 0.0f,0.0f,0.0f }, 0.000f, 0.00f, 0.00f, 0.000f,  0.0f, 0000.0f,   0.0f, 0.0f,   0.0f,   0.0f, 0x31f }

/* [DEFINE_END] */


/*
[STRUCTURE] 
[
    [DESCRIPTION]
    Structure defining the properties for a reverb source, related to a CRYSOUND channel.
    For more indepth descriptions of the reverb properties under win32, please see the EAX3
    documentation at http://developer.creative.com/ under the 'downloads' section.
    If they do not have the EAX3 documentation, then most information can be attained from
    the EAX2 documentation, as EAX3 only adds some more parameters and functionality on top of 
    EAX2.
    
    Note the default reverb properties are the same as the CS_PRESET_GENERIC preset.
    Note that integer values that typically range from -10,000 to 1000 are represented in 
    decibels, and are of a logarithmic scale, not linear, wheras float values are typically linear.
    PORTABILITY: Each member has the platform it supports in braces ie (win32/xbox).  
    Some reverb parameters are only supported in win32 and some only on xbox. If all parameters are set then
    the reverb should product a similar effect on either platform.
    Linux and CRYSOUNDCE do not support the reverb api.
    
    The numerical values listed below are the maximum, minimum and default values for each variable respectively.

    [SEE_ALSO]
    CS_Reverb_SetChannelProperties
    CS_Reverb_GetChannelProperties
    CS_REVERB_CHANNELFLAGS
]
*/
typedef struct _CS_REVERB_CHANNELPROPERTIES /* MIN     MAX    DEFAULT */
{                                   
    int    Direct;                              /* -10000, 1000,  0,    direct path level (at low and mid frequencies) (win32/xbox) */
    int    DirectHF;                            /* -10000, 0,     0,    relative direct path level at high frequencies (win32/xbox) */
    int    Room;                                /* -10000, 1000,  0,    room effect level (at low and mid frequencies) (win32/xbox) */
    int    RoomHF;                              /* -10000, 0,     0,    relative room effect level at high frequencies (win32/xbox) */
    int    Obstruction;                         /* -10000, 0,     0,    main obstruction control (attenuation at high frequencies)  (win32/xbox) */
    float  ObstructionLFRatio;                  /* 0.0,    1.0,   0.0,  obstruction low-frequency level re. main control (win32/xbox) */
    int    Occlusion;                           /* -10000, 0,     0,    main occlusion control (attenuation at high frequencies) (win32/xbox) */
    float  OcclusionLFRatio;                    /* 0.0,    1.0,   0.25, occlusion low-frequency level re. main control (win32/xbox) */
    float  OcclusionRoomRatio;                  /* 0.0,    10.0,  1.5,  relative occlusion control for room effect (win32) */
    float  OcclusionDirectRatio;                /* 0.0,    10.0,  1.0,  relative occlusion control for direct path (win32) */
    int    Exclusion;                           /* -10000, 0,     0,    main exlusion control (attenuation at high frequencies) (win32) */
    float  ExclusionLFRatio;                    /* 0.0,    1.0,   1.0,  exclusion low-frequency level re. main control (win32) */
    int    OutsideVolumeHF;                     /* -10000, 0,     0,    outside sound cone level at high frequencies (win32) */
    float  DopplerFactor;                       /* 0.0,    10.0,  0.0,  like DS3D flDopplerFactor but per source (win32) */
    float  RolloffFactor;                       /* 0.0,    10.0,  0.0,  like DS3D flRolloffFactor but per source (win32) */
    float  RoomRolloffFactor;                   /* 0.0,    10.0,  0.0,  like DS3D flRolloffFactor but for room effect (win32/xbox) */
    float  AirAbsorptionFactor;                 /* 0.0,    10.0,  1.0,  multiplies AirAbsorptionHF member of CS_REVERB_PROPERTIES (win32) */
    int    Flags;                               /* CS_REVERB_CHANNELFLAGS - modifies the behavior of properties (win32) */
} CS_REVERB_CHANNELPROPERTIES;


/*
[DEFINE_START] 
[
    [NAME] 
    CS_REVERB_CHANNELFLAGS
    
    [DESCRIPTION]
    Values for the Flags member of the CS_REVERB_CHANNELPROPERTIES structure.

    [SEE_ALSO]
    CS_REVERB_CHANNELPROPERTIES
]
*/
#define CS_REVERB_CHANNELFLAGS_DIRECTHFAUTO  0x00000001 /* Automatic setting of 'Direct'  due to distance from listener */
#define CS_REVERB_CHANNELFLAGS_ROOMAUTO      0x00000002 /* Automatic setting of 'Room'  due to distance from listener */
#define CS_REVERB_CHANNELFLAGS_ROOMHFAUTO    0x00000004 /* Automatic setting of 'RoomHF' due to distance from listener */
#define CS_REVERB_CHANNELFLAGS_DEFAULT       (CS_REVERB_CHANNELFLAGS_DIRECTHFAUTO |   \
                                                  CS_REVERB_CHANNELFLAGS_ROOMAUTO|        \
                                                  CS_REVERB_CHANNELFLAGS_ROOMHFAUTO)
/* [DEFINE_END] */


/*
[ENUM] 
[
    [DESCRIPTION]
    These values are used with CS_FX_Enable to enable DirectX 8 FX for a channel.

    [SEE_ALSO]
    CS_FX_Enable
    CS_FX_Disable
    CS_FX_SetChorus
    CS_FX_SetCompressor
    CS_FX_SetDistortion
    CS_FX_SetEcho
    CS_FX_SetFlanger
    CS_FX_SetGargle
    CS_FX_SetI3DL2Reverb
    CS_FX_SetParamEQ
    CS_FX_SetWavesReverb
]
*/
enum CS_FX_MODES
{
    CS_FX_CHORUS,
    CS_FX_COMPRESSOR,
    CS_FX_DISTORTION,
    CS_FX_ECHO,
    CS_FX_FLANGER,
    CS_FX_GARGLE,
    CS_FX_I3DL2REVERB,
    CS_FX_PARAMEQ,
    CS_FX_WAVES_REVERB,

    CS_FX_MAX
};

/*
[ENUM]
[
    [DESCRIPTION]   
    These are speaker types defined for use with the CS_SetSpeakerMode command.
    Note - Only reliably works with CS_OUTPUT_DSOUND or CS_OUTPUT_XBOX output modes.  Other output modes will only 
    interpret CS_SPEAKERMODE_MONO and set everything else to be stereo.
    Note - Only reliably works with CS_OUTPUT_DSOUND or CS_OUTPUT_XBOX output modes.  Other output modes will only 
    interpret CS_SPEAKERMODE_MONO and set everything else to be stereo.

    Using either DolbyDigital or DTS will use whatever 5.1 digital mode is available if destination hardware is unsure.

    [SEE_ALSO]
    CS_SetSpeakerMode

]
*/
enum CS_SPEAKERMODES
{
    CS_SPEAKERMODE_DOLBYDIGITAL,  /* The audio is played through a speaker arrangement of surround speakers with a subwoofer. */
    CS_SPEAKERMODE_HEADPHONES,    /* The speakers are headphones. */
    CS_SPEAKERMODE_MONO,          /* The speakers are monaural. */
    CS_SPEAKERMODE_QUAD,          /* The speakers are quadraphonic.  */
    CS_SPEAKERMODE_STEREO,        /* The speakers are stereo (default value). */
    CS_SPEAKERMODE_SURROUND,      /* The speakers are surround sound. */
    CS_SPEAKERMODE_DTS            /* (XBOX Only) The audio is played through a speaker arrangement of surround speakers with a subwoofer. */
};


/*
[DEFINE_START] 
[
    [NAME] 
    CS_INIT_FLAGS
    
    [DESCRIPTION]   
    Initialization flags.  Use them with CS_Init in the flags parameter to change various behaviour.
    
    CS_INIT_ENABLEOUTPUTFX Is an init mode which enables the CRYSOUND mixer buffer to be affected by DirectX 8 effects.
    Note that due to limitations of DirectSound, CS_Init may fail if this is enabled because the buffersize is too small.
    This can be fixed with CS_SetBufferSize.  Increase the BufferSize until it works.
    When it is enabled you can use the CS_FX api, and use CS_SYSTEMCHANNEL as the channel id when setting parameters.

    [SEE_ALSO]
    CS_Init
]
*/
#define CS_INIT_USEDEFAULTMIDISYNTH     0x01    /* Causes MIDI playback to force software decoding. */
#define CS_INIT_GLOBALFOCUS             0x02    /* For DirectSound output - sound is not muted when window is out of focus. */
#define CS_INIT_ENABLEOUTPUTFX          0x04    /* For DirectSound output - Allows CS_FX api to be used on global software mixer output! */
#define CS_INIT_ACCURATEVULEVELS        0x08    /* This latency adjusts CS_GetCurrentLevels, but incurs a small cpu and memory hit */
#define CS_INIT_PS2_DISABLECORE0REVERB  0x10    /* PS2 only - Disable reverb on CORE 0 to regain SRAM */
#define CS_INIT_PS2_DISABLECORE1REVERB  0x20    /* PS2 only - Disable reverb on CORE 1 to regain SRAM */
#define CS_INIT_PS2_SWAPDMACORES        0x40    /* PS2 only - By default CRYSOUND uses DMA CH0 for mixing, CH1 for uploads, this flag swaps them around */
/* [DEFINE_END] */




/* ========================================================================================== */
/* FUNCTION PROTOTYPES                                                                        */
/* ========================================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

/* ================================== */
/* Initialization / Global functions. */
/* ================================== */

/* 
    PRE - CS_Init functions. These can't be called after CS_Init is 
    called (they will fail). They set up CRYSOUND system functionality. 
*/

DLL_API signed char     F_API CS_SetOutput(int outputtype);
DLL_API signed char     F_API CS_SetDriver(int driver);
DLL_API signed char     F_API CS_SetMixer(int mixer);
DLL_API signed char     F_API CS_SetBufferSize(int len_ms);
DLL_API signed char     F_API CS_SetHWND(void *hwnd);
DLL_API signed char     F_API CS_SetMinHardwareChannels(int min);
DLL_API signed char     F_API CS_SetMaxHardwareChannels(int max);
DLL_API signed char     F_API CS_SetMemorySystem(void *pool, 
                                                     int poollen, 
                                                     CS_ALLOCCALLBACK   useralloc,
                                                     CS_REALLOCCALLBACK userrealloc,
                                                     CS_FREECALLBACK    userfree);
/* 
    Main initialization / closedown functions.
    Note : Use CS_INIT_USEDEFAULTMIDISYNTH with CS_Init for software override 
           with MIDI playback.
         : Use CS_INIT_GLOBALFOCUS with CS_Init to make sound audible no matter 
           which window is in focus. (CS_OUTPUT_DSOUND only)
*/

DLL_API signed char     F_API CS_Init(int mixrate, int maxsoftwarechannels, unsigned int flags);
DLL_API void            F_API CS_Close();

/* 
    Runtime system level functions 
*/

DLL_API void            F_API CS_Update();   /* This is called to update 3d sound / non-realtime output */

DLL_API void            F_API CS_SetSpeakerMode(unsigned int speakermode);
DLL_API void            F_API CS_SetSFXMasterVolume(int volume);
DLL_API void            F_API CS_SetPanSeperation(float pansep);
DLL_API void            F_API CS_File_SetCallbacks(CS_OPENCALLBACK  useropen,
                                                       CS_CLOSECALLBACK userclose,
                                                       CS_READCALLBACK  userread,
                                                       CS_SEEKCALLBACK  userseek,
                                                       CS_TELLCALLBACK  usertell);

/* 
    System information functions. 
*/

DLL_API int             F_API CS_GetError();
DLL_API float           F_API CS_GetVersion();
DLL_API int             F_API CS_GetOutput();
DLL_API void *          F_API CS_GetOutputHandle();
DLL_API int             F_API CS_GetDriver();
DLL_API int             F_API CS_GetMixer();
DLL_API int             F_API CS_GetNumDrivers();
DLL_API signed char *   F_API CS_GetDriverName(int id);
DLL_API signed char     F_API CS_GetDriverCaps(int id, unsigned int *caps);
DLL_API int             F_API CS_GetOutputRate();
DLL_API int             F_API CS_GetMaxChannels();
DLL_API int             F_API CS_GetMaxSamples();
DLL_API int             F_API CS_GetSFXMasterVolume();
DLL_API int             F_API CS_GetNumHardwareChannels();
DLL_API int             F_API CS_GetChannelsPlaying();
DLL_API float           F_API CS_GetCPUUsage();
DLL_API void            F_API CS_GetMemoryStats(unsigned int *currentalloced, unsigned int *maxalloced);

/* =================================== */
/* Sample management / load functions. */
/* =================================== */

/* 
    Sample creation and management functions
    Note : Use CS_LOADMEMORY   flag with CS_Sample_Load to load from memory.
           Use CS_LOADRAW      flag with CS_Sample_Load to treat as as raw pcm data.
*/

DLL_API CS_SAMPLE * F_API CS_Sample_Load(int index, const char *name_or_data, unsigned int mode, int memlength);
DLL_API CS_SAMPLE * F_API CS_Sample_Alloc(int index, int length, unsigned int mode, int deffreq, int defvol, int defpan, int defpri);
DLL_API void            F_API CS_Sample_Free(CS_SAMPLE *sptr);
DLL_API signed char     F_API CS_Sample_Upload(CS_SAMPLE *sptr, void *srcdata, unsigned int mode);
DLL_API signed char     F_API CS_Sample_Lock(CS_SAMPLE *sptr, int offset, int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2);
DLL_API signed char     F_API CS_Sample_Unlock(CS_SAMPLE *sptr, void *ptr1, void *ptr2, unsigned int len1, unsigned int len2);

/*
    Sample control functions
*/

DLL_API signed char     F_API CS_Sample_SetMode(CS_SAMPLE *sptr, unsigned int mode);
DLL_API signed char     F_API CS_Sample_SetLoopPoints(CS_SAMPLE *sptr, int loopstart, int loopend);
DLL_API signed char     F_API CS_Sample_SetDefaults(CS_SAMPLE *sptr, int deffreq, int defvol, int defpan, int defpri);
DLL_API signed char     F_API CS_Sample_SetMinMaxDistance(CS_SAMPLE *sptr, float min, float max);
DLL_API signed char     F_API CS_Sample_SetMaxPlaybacks(CS_SAMPLE *sptr, int max);

/* 
    Sample information functions
*/

DLL_API CS_SAMPLE * F_API CS_Sample_Get(int sampno);
DLL_API char *          F_API CS_Sample_GetName(CS_SAMPLE *sptr);
DLL_API unsigned int    F_API CS_Sample_GetLength(CS_SAMPLE *sptr);
DLL_API signed char     F_API CS_Sample_GetLoopPoints(CS_SAMPLE *sptr, int *loopstart, int *loopend);
DLL_API signed char     F_API CS_Sample_GetDefaults(CS_SAMPLE *sptr, int *deffreq, int *defvol, int *defpan, int *defpri);
DLL_API unsigned int    F_API CS_Sample_GetMode(CS_SAMPLE *sptr);
  
/* ============================ */
/* Channel control functions.   */
/* ============================ */

/* 
    Playing and stopping sounds.  
    Note : Use CS_FREE as the 'channel' variable, to let CRYSOUND pick a free channel for you.
           Use CS_ALL as the 'channel' variable to control ALL channels with one function call!
*/

DLL_API int             F_API CS_PlaySound(int channel, CS_SAMPLE *sptr);
DLL_API int             F_API CS_PlaySoundEx(int channel, CS_SAMPLE *sptr, CS_DSPUNIT *dsp, signed char startpaused);
DLL_API signed char     F_API CS_StopSound(int channel);

/* 
    Functions to control playback of a channel.
    Note : CS_ALL can be used on most of these functions as a channel value.
*/

DLL_API signed char     F_API CS_SetFrequency(int channel, int freq);
DLL_API signed char     F_API CS_SetVolume(int channel, int vol);
DLL_API signed char     F_API CS_SetVolumeAbsolute(int channel, int vol);
DLL_API signed char     F_API CS_SetPan(int channel, int pan);
DLL_API signed char     F_API CS_SetSurround(int channel, signed char surround);
DLL_API signed char     F_API CS_SetMute(int channel, signed char mute);
DLL_API signed char     F_API CS_SetPriority(int channel, int priority);
DLL_API signed char     F_API CS_SetReserved(int channel, signed char reserved);
DLL_API signed char     F_API CS_SetPaused(int channel, signed char paused);
DLL_API signed char     F_API CS_SetLoopMode(int channel, unsigned int loopmode);
DLL_API signed char     F_API CS_SetCurrentPosition(int channel, unsigned int offset);

/* 
    Channel information functions.
*/

DLL_API signed char     F_API CS_IsPlaying(int channel);
DLL_API int             F_API CS_GetFrequency(int channel);
DLL_API int             F_API CS_GetVolume(int channel);
DLL_API int             F_API CS_GetPan(int channel);
DLL_API signed char     F_API CS_GetSurround(int channel);
DLL_API signed char     F_API CS_GetMute(int channel);
DLL_API int             F_API CS_GetPriority(int channel);
DLL_API signed char     F_API CS_GetReserved(int channel);
DLL_API signed char     F_API CS_GetPaused(int channel);
DLL_API unsigned int    F_API CS_GetLoopMode(int channel);
DLL_API unsigned int    F_API CS_GetCurrentPosition(int channel);
DLL_API CS_SAMPLE * F_API CS_GetCurrentSample(int channel);
DLL_API signed char     F_API CS_GetCurrentLevels(int channel, float *l, float *r);


/* =================== */
/* FX functions.       */
/* =================== */

/* 
    Functions to control DX8 only effects processing.

    - FX enabled samples can only be played once at a time, not multiple times at once.
    - Sounds have to be created with CS_HW2D or CS_HW3D for this to work.
    - CS_INIT_ENABLEOUTPUTFX can be used to apply hardware effect processing to the
      global mixed output of CRYSOUND's software channels.
    - CS_FX_Enable returns an FX handle that you can use to alter fx parameters.
    - CS_FX_Enable can be called multiple times in a row, even on the same FX type,
      it will return a unique handle for each FX.
    - CS_FX_Enable cannot be called if the sound is playing or locked.
    - CS_FX_Disable must be called to reset/clear the FX from a channel.
*/

DLL_API int             F_API CS_FX_Enable(int channel, unsigned int fx);    /* See CS_FX_MODES */
DLL_API signed char     F_API CS_FX_Disable(int channel);    

DLL_API signed char     F_API CS_FX_SetChorus(int fxid, float WetDryMix, float Depth, float Feedback, float Frequency, int Waveform, float Delay, int Phase);
DLL_API signed char     F_API CS_FX_SetCompressor(int fxid, float Gain, float Attack, float Release, float Threshold, float Ratio, float Predelay);
DLL_API signed char     F_API CS_FX_SetDistortion(int fxid, float Gain, float Edge, float PostEQCenterFrequency, float PostEQBandwidth, float PreLowpassCutoff);
DLL_API signed char     F_API CS_FX_SetEcho(int fxid, float WetDryMix, float Feedback, float LeftDelay, float RightDelay, int PanDelay);
DLL_API signed char     F_API CS_FX_SetFlanger(int fxid, float WetDryMix, float Depth, float Feedback, float Frequency, int Waveform, float Delay, int Phase);
DLL_API signed char     F_API CS_FX_SetGargle(int fxid, int RateHz, int WaveShape);
DLL_API signed char     F_API CS_FX_SetI3DL2Reverb(int fxid, int Room, int RoomHF, float RoomRolloffFactor, float DecayTime, float DecayHFRatio, int Reflections, float ReflectionsDelay, int Reverb, float ReverbDelay, float Diffusion, float Density, float HFReference);
DLL_API signed char     F_API CS_FX_SetParamEQ(int fxid, float Center, float Bandwidth, float Gain);
DLL_API signed char     F_API CS_FX_SetWavesReverb(int fxid, float InGain, float ReverbMix, float ReverbTime, float HighFreqRTRatio);  
 
/* =================== */
/* 3D sound functions. */
/* =================== */

/* 
    See also CS_Sample_SetMinMaxDistance (above)
    Note! CS_3D_Update is now called CS_Update.
*/

DLL_API void            F_API CS_3D_SetDopplerFactor(float scale);
DLL_API void            F_API CS_3D_SetDistanceFactor(float scale);
DLL_API void            F_API CS_3D_SetRolloffFactor(float scale);
DLL_API signed char     F_API CS_3D_SetAttributes(int channel, float *pos, float *vel);
DLL_API signed char     F_API CS_3D_GetAttributes(int channel, float *pos, float *vel);

DLL_API void            F_API CS_3D_Listener_SetCurrent(int current, int numlisteners);  /* use this if you use multiple listeners / splitscreen */
DLL_API void            F_API CS_3D_Listener_SetAttributes(float *pos, float *vel, float fx, float fy, float fz, float tx, float ty, float tz);
DLL_API void            F_API CS_3D_Listener_GetAttributes(float *pos, float *vel, float *fx, float *fy, float *fz, float *tx, float *ty, float *tz);
 
/* ========================= */
/* File Streaming functions. */
/* ========================= */

/*
    Note : Use CS_LOADMEMORY   flag with CS_Stream_OpenFile to stream from memory.
           Use CS_LOADRAW      flag with CS_Stream_OpenFile to treat stream as raw pcm data.
           Use CS_MPEGACCURATE flag with CS_Stream_OpenFile to open mpegs in 'accurate mode' for settime/gettime/getlengthms.
           Use CS_FREE as the 'channel' variable, to let CRYSOUND pick a free channel for you.
*/

DLL_API signed char     F_API CS_Stream_SetBufferSize(int ms);      /* call this before opening streams, not after */

DLL_API CS_STREAM * F_API CS_Stream_OpenFile(const char *filename, unsigned int mode, int memlength);
DLL_API CS_STREAM * F_API CS_Stream_Create(CS_STREAMCALLBACK callback, int length, unsigned int mode, int samplerate, int userdata);
DLL_API signed char     F_API CS_Stream_Close(CS_STREAM *stream);

DLL_API int             F_API CS_Stream_Play(int channel, CS_STREAM *stream);
DLL_API int             F_API CS_Stream_PlayEx(int channel, CS_STREAM *stream, CS_DSPUNIT *dsp, signed char startpaused);
DLL_API signed char     F_API CS_Stream_Stop(CS_STREAM *stream);

DLL_API int             F_API CS_Stream_GetOpenState(CS_STREAM *stream);  /* use with CS_NONBLOCKING opened streams */
DLL_API signed char     F_API CS_Stream_SetPosition(CS_STREAM *stream, unsigned int position);
DLL_API unsigned int    F_API CS_Stream_GetPosition(CS_STREAM *stream);
DLL_API signed char     F_API CS_Stream_SetTime(CS_STREAM *stream, int ms);
DLL_API int             F_API CS_Stream_GetTime(CS_STREAM *stream);
DLL_API int             F_API CS_Stream_GetLength(CS_STREAM *stream);
DLL_API int             F_API CS_Stream_GetLengthMs(CS_STREAM *stream);

DLL_API signed char     F_API CS_Stream_SetLoopPoints(CS_STREAM *stream, unsigned int loopstartpcm, unsigned int loopendpcm);
DLL_API CS_SAMPLE * F_API CS_Stream_GetSample(CS_STREAM *stream);
DLL_API CS_DSPUNIT *F_API CS_Stream_CreateDSP(CS_STREAM *stream, CS_DSPCALLBACK callback, int priority, int param);
DLL_API signed char     F_API CS_Stream_SetEndCallback(CS_STREAM *stream, CS_STREAMCALLBACK callback, int userdata);

DLL_API signed char     F_API CS_Stream_SetSynchCallback(CS_STREAM *stream, CS_STREAMCALLBACK callback, int userdata);
DLL_API int             F_API CS_Stream_AddSynchPoint(CS_STREAM *stream, unsigned int pcmoffset, int userdata);
DLL_API signed char     F_API CS_Stream_DeleteSynchPoint(CS_STREAM *stream, int index);
DLL_API int             F_API CS_Stream_GetNumSynchPoints(CS_STREAM *stream);

DLL_API signed char     F_API CS_Stream_SetSubStream(CS_STREAM *stream, int index);     /* For CRYSOUND .FSB bank files.  Console only currently. */
DLL_API int             F_API CS_Stream_GetNumSubStreams(CS_STREAM *stream);            /* For CRYSOUND .FSB bank files.  Console only currently. */

/* =================== */
/* CD audio functions. */
/* =================== */

/*
    Note : 0 = default cdrom.  Otherwise specify the drive letter, for example. 'D'. 
*/

DLL_API signed char     F_API CS_CD_Play(char drive, int track);
DLL_API void            F_API CS_CD_SetPlayMode(char drive, signed char mode);
DLL_API signed char     F_API CS_CD_Stop(char drive);
DLL_API signed char     F_API CS_CD_SetPaused(char drive, signed char paused);
DLL_API signed char     F_API CS_CD_SetVolume(char drive, int volume);
DLL_API signed char     F_API CS_CD_Eject(char drive);

DLL_API signed char     F_API CS_CD_GetPaused(char drive);
DLL_API int             F_API CS_CD_GetTrack(char drive);
DLL_API int             F_API CS_CD_GetNumTracks(char drive);
DLL_API int             F_API CS_CD_GetVolume(char drive);
DLL_API int             F_API CS_CD_GetTrackLength(char drive, int track); 
DLL_API int             F_API CS_CD_GetTrackTime(char drive);

/* ============== */
/* DSP functions. */
/* ============== */

/* 
    DSP Unit control and information functions. 
    These functions allow you access to the mixed stream that CRYSOUND uses to play back sound on.
*/

DLL_API CS_DSPUNIT *F_API CS_DSP_Create(CS_DSPCALLBACK callback, int priority, int param);
DLL_API void            F_API CS_DSP_Free(CS_DSPUNIT *unit);
DLL_API void            F_API CS_DSP_SetPriority(CS_DSPUNIT *unit, int priority);
DLL_API int             F_API CS_DSP_GetPriority(CS_DSPUNIT *unit);
DLL_API void            F_API CS_DSP_SetActive(CS_DSPUNIT *unit, signed char active);
DLL_API signed char     F_API CS_DSP_GetActive(CS_DSPUNIT *unit);

/* 
    Functions to get hold of CRYSOUND 'system DSP unit' handles. 
*/

DLL_API CS_DSPUNIT *F_API CS_DSP_GetClearUnit();
DLL_API CS_DSPUNIT *F_API CS_DSP_GetSFXUnit();
DLL_API CS_DSPUNIT *F_API CS_DSP_GetMusicUnit();
DLL_API CS_DSPUNIT *F_API CS_DSP_GetFFTUnit();
DLL_API CS_DSPUNIT *F_API CS_DSP_GetClipAndCopyUnit();

/* 
    Miscellaneous DSP functions 
    Note for the spectrum analysis function to work, you have to enable the FFT DSP unit with 
    the following code CS_DSP_SetActive(CS_DSP_GetFFTUnit(), TRUE);
    It is off by default to save cpu usage.
*/

DLL_API signed char     F_API CS_DSP_MixBuffers(void *destbuffer, void *srcbuffer, int len, int freq, int vol, int pan, unsigned int mode);
DLL_API void            F_API CS_DSP_ClearMixBuffer();
DLL_API int             F_API CS_DSP_GetBufferLength();      /* Length of each DSP update */
DLL_API int             F_API CS_DSP_GetBufferLengthTotal(); /* Total buffer length due to CS_SetBufferSize */
DLL_API float *         F_API CS_DSP_GetSpectrum();          /* Array of 512 floats - call CS_DSP_SetActive(CS_DSP_GetFFTUnit(), TRUE)) for this to work. */

/* =================================================================================== */
/* Reverb functions. (eax2/eax3 reverb)  (ONLY SUPPORTED ON WIN32 W/ CS_HW3D FLAG) */
/* =================================================================================== */

/*
    See top of file for definitions and information on the reverb parameters.
*/

DLL_API signed char F_API CS_Reverb_SetProperties(CS_REVERB_PROPERTIES *prop);
DLL_API signed char F_API CS_Reverb_GetProperties(CS_REVERB_PROPERTIES *prop);
DLL_API signed char F_API CS_Reverb_SetChannelProperties(int channel, CS_REVERB_CHANNELPROPERTIES *prop);
DLL_API signed char F_API CS_Reverb_GetChannelProperties(int channel, CS_REVERB_CHANNELPROPERTIES *prop);

/* ===================================================== */
/* Recording functions  (ONLY SUPPORTED IN WIN32, WINCE) */
/* ===================================================== */

/*
    Recording initialization functions
*/

DLL_API signed char     F_API CS_Record_SetDriver(int outputtype);
DLL_API int             F_API CS_Record_GetNumDrivers();
DLL_API signed char *   F_API CS_Record_GetDriverName(int id);
DLL_API int             F_API CS_Record_GetDriver();

/*
    Recording functionality.  Only one recording session will work at a time.
*/

DLL_API signed char     F_API CS_Record_StartSample(CS_SAMPLE *sptr, signed char loop);
DLL_API signed char     F_API CS_Record_Stop();
DLL_API int             F_API CS_Record_GetPosition();  

/* ========================================================================================== */
/* CRYMUSIC API (MOD,S3M,XM,IT,MIDI PLAYBACK)                                                   */
/* ========================================================================================== */

/* 
    Song management / playback functions.
*/

DLL_API CM_MODULE * F_API CM_LoadSong(const char *name);
DLL_API CM_MODULE * F_API CM_LoadSongMemory(void *data, int length);
DLL_API signed char     F_API CM_FreeSong(CM_MODULE *mod);
DLL_API signed char     F_API CM_PlaySong(CM_MODULE *mod);
DLL_API signed char     F_API CM_StopSong(CM_MODULE *mod);
DLL_API void            F_API CM_StopAllSongs();

DLL_API signed char     F_API CM_SetZxxCallback(CM_MODULE *mod, CM_CALLBACK callback);
DLL_API signed char     F_API CM_SetRowCallback(CM_MODULE *mod, CM_CALLBACK callback, int rowstep);
DLL_API signed char     F_API CM_SetOrderCallback(CM_MODULE *mod, CM_CALLBACK callback, int orderstep);
DLL_API signed char     F_API CM_SetInstCallback(CM_MODULE *mod, CM_CALLBACK callback, int instrument);

DLL_API signed char     F_API CM_SetSample(CM_MODULE *mod, int sampno, CS_SAMPLE *sptr);
DLL_API signed char     F_API CM_SetUserData(CM_MODULE *mod, unsigned int userdata);
DLL_API signed char     F_API CM_OptimizeChannels(CM_MODULE *mod, int maxchannels, int minvolume);

/*
    Runtime song functions. 
*/

DLL_API signed char     F_API CM_SetReverb(signed char reverb);             /* MIDI only */
DLL_API signed char     F_API CM_SetLooping(CM_MODULE *mod, signed char looping);
DLL_API signed char     F_API CM_SetOrder(CM_MODULE *mod, int order);
DLL_API signed char     F_API CM_SetPaused(CM_MODULE *mod, signed char pause);
DLL_API signed char     F_API CM_SetMasterVolume(CM_MODULE *mod, int volume);
DLL_API signed char     F_API CM_SetMasterSpeed(CM_MODULE *mode, float speed);
DLL_API signed char     F_API CM_SetPanSeperation(CM_MODULE *mod, float pansep);
 
/* 
    Static song information functions.
*/

DLL_API char *          F_API CM_GetName(CM_MODULE *mod);
DLL_API int             F_API CM_GetType(CM_MODULE *mod);
DLL_API int             F_API CM_GetNumOrders(CM_MODULE *mod);
DLL_API int             F_API CM_GetNumPatterns(CM_MODULE *mod);
DLL_API int             F_API CM_GetNumInstruments(CM_MODULE *mod);
DLL_API int             F_API CM_GetNumSamples(CM_MODULE *mod);
DLL_API int             F_API CM_GetNumChannels(CM_MODULE *mod);
DLL_API CS_SAMPLE * F_API CM_GetSample(CM_MODULE *mod, int sampno);
DLL_API int             F_API CM_GetPatternLength(CM_MODULE *mod, int orderno);
 
/* 
    Runtime song information.
*/

DLL_API signed char     F_API CM_IsFinished(CM_MODULE *mod);
DLL_API signed char     F_API CM_IsPlaying(CM_MODULE *mod);
DLL_API int             F_API CM_GetMasterVolume(CM_MODULE *mod);
DLL_API int             F_API CM_GetGlobalVolume(CM_MODULE *mod);
DLL_API int             F_API CM_GetOrder(CM_MODULE *mod);
DLL_API int             F_API CM_GetPattern(CM_MODULE *mod);
DLL_API int             F_API CM_GetSpeed(CM_MODULE *mod);
DLL_API int             F_API CM_GetBPM(CM_MODULE *mod);
DLL_API int             F_API CM_GetRow(CM_MODULE *mod);
DLL_API signed char     F_API CM_GetPaused(CM_MODULE *mod);
DLL_API int             F_API CM_GetTime(CM_MODULE *mod);
DLL_API int             F_API CM_GetRealChannel(CM_MODULE *mod, int modchannel);
DLL_API unsigned int    F_API CM_GetUserData(CM_MODULE *mod);

#ifdef __cplusplus
}
#endif

#endif
