#ifdef WIN32
#define portable_makepath _makepath 
#else
#ifdef _XBOX
#ifdef __cplusplus
extern "C" void portable_makepath (
        char *path,
        const char *drive,
        const char *dir,
        const char *fname,
        const char *ext
		);
#else // _CPP
void portable_makepath (
        char *path,
        const char *drive,
        const char *dir,
        const char *fname,
        const char *ext
		);
#endif // _CPP
#else // _XBOX
extern "C" void portable_makepath (
        char *path,
        const char *drive,
        const char *dir,
        const char *fname,
        const char *ext
        );
#endif // _XBOX
#endif