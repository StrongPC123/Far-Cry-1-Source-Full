/*=============================================================================
	CrtOverrides.h: missing C RunTime overrides implementation.
	Copyright 2001 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Honitch Andrey

=============================================================================*/

#ifdef _XBOX

#ifndef stricmp
inline int stricmp(const char *dst, const char *src)
{
  int f,l;
  do
  {
    if ( ((f=(unsigned char)(*(dst++))) >= 'A') && (f<='Z'))
      f -= ('A' - 'a');
    
    if ( ((l=(unsigned char)(*(src++))) >= 'A') && (l<='Z'))
      l -= ('A' - 'a');
  } while ( f && (f == l) );

  return(f - l);
}
#endif

#ifndef strnicmp
inline int strnicmp (const char * first, const char * last, size_t count)
{
  int f,l;
  if ( count )
  {
    do
    {
      if ( ((f=(unsigned char)(*(first++))) >= 'A') && (f<='Z') )
        f -= 'A' - 'a';

      if ( ((l=(unsigned char)(*(last++))) >= 'A') && (l<='Z'))
        l -= 'A' - 'a';
    } while ( --count && f && (f == l) );

    return( f - l );
  }

  return 0;
}
#endif

#ifndef strdup
inline char * strdup (const char * str)
{
  char *memory;

  if (!str)
    return(NULL);

  memory = (char *)malloc(strlen(str) + 1);
  if (memory)
    return(strcpy(memory,str));

  return(NULL);
}
#endif

#ifndef strlwr
inline char * strlwr (char * str)
{
  unsigned char *dst = NULL;  /* destination string */
  char *cp;               /* traverses string for C locale conversion */

  for (cp=str; *cp; ++cp)
  {
    if ('A' <= *cp && *cp <= 'Z')
      *cp += 'a' - 'A';
  }
  return str;
}
#endif

#endif // _XBOX

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
