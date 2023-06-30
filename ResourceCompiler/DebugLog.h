#ifndef _RC_DEBUG_LOG_HDR_
#define _RC_DEBUG_LOG_HDR_

inline void DebugLog (const char* szFormat, ...)
{
	FILE* f = fopen ("Rc.Debug.log", "wa");
	if (!f)
		return;
	va_list args;
	va_start (args, szFormat);
	vfprintf (f, szFormat, args);
	fprintf (f, "\n");
	vprintf (szFormat, args);
	printf ("\n");
	va_end(args);
	fclose (f);
}

#endif