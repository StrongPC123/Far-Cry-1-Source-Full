// vc_warning_disable.h
#pragma warning( disable : 4786 ) // identifier truncated to 255 characters
#pragma warning( disable : 4244 ) // implicit conversion: possible loss of data
#pragma warning( disable : 4800 ) // forcing value to bool 'true' or 'false' (performance warning)
#pragma warning( disable : 4189) // local variable is initialized but not referenced
#pragma warning( disable : 4100) // unreferenced formal parameter
#pragma warning( disable : 4245) // conversion from 'const int' to 'unsigned long', signed/unsigned mismatch
#pragma warning( disable : 4660) // template-class specialization 'basic_filebuf<char,class _STLD::char_traits<char> >' is already instantiated
#pragma warning( disable :  4701) // local variable 'base' may be used without having been initialized
#pragma warning( disable : 4075) // initializers put in unrecognized initialization area
#pragma warning( disable : 4673) // throwing class with private base class
#pragma warning( disable : 4670) // throwing class with private base class
#pragma warning( disable : 4018) // signed/unsigned mismatch
#pragma warning( disable : 4505 ) // unreferenced local function has been removed
#pragma warning( disable : 4146 ) // unary minus applied to unsigned type
#pragma warning( disable : 4244 ) // arithmetic conversion - possible loss of data
#pragma warning( disable : 4290 ) // c++ exception specification ignored

// dwa 1/28/00 - actually I think this may indicate real bugs. We should look
// into these
#pragma warning( disable : 4018 ) // signed/unsigned mismatch
#pragma warning( disable : 4251 ) // DLL interface needed
#pragma warning( disable : 4284 ) // for -> operator
