#ifndef _DEDICATEDSERVER
#define _DEDICATEDSERVER

#include <ISystem.h>					// ISystem, SSystemInitParams	

struct ICVar;


//! This is FarCry.exe authentication function, this code is not for public release!!
void AuthCheckFunction( void *data );
//!
bool InitDedicatedServer_System( const char *sInCmdLine );
//!
bool InitDedicatedServer_Game( const char *sInCmdLine );
//!
void PrintDedicatedServerStatus();
//!
void DeInitDedicatedServer();

//!
void PrintGoodbyeMessage();
//!
void PrintWelcomeMessage();

//! to make use of g_psvDedicatedMaxRate
#if defined(LINUX)
void SleepIfNeeded(bool &bFirstTime);
#else
void SleepIfNeeded();
#endif

//!
ISystem *GetISystem();
//!
void print( const char *insTxt, ... );

//! returns the decimal string representation of the given int
std::string IntToString (int nNumber);

//! returns hexadecimal string representation of the given dword
std::string UIntToHexString(unsigned long dwNumber);

const char *Strip(const char *inszText);


extern SSystemInitParams			g_SystemInitParams;											//!< inital statup parameters system
extern ICVar *								g_psvDedicatedMaxRate;									//!<


#endif // _DEDICATEDSERVER

