/*
 * Copyright (c) 1999
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1999
 * Boris Fomitchev
 *
 * Written 2000
 * Anton Lapach
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted
 * without fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 */

#include <windows.h>
#include <limits.h>
#if defined (_STLP_MSVC) || defined (__ICL) || defined (__BORLANDC__)
# include <memory.h>
#endif
#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
// _STLP_BEGIN_NAMESPACE
extern "C" {
#endif

/* Framework functions */
/*
  locale :: "lang[_country[.code_page]]"
  | ".code_page"
  | ""
  | NULL

*/

#ifndef _LEADBYTE
/* multibyte leadbyte */
#define _LEADBYTE       0x8000
#endif

typedef struct _LOCALECONV
{
  const char* name;
  const char* abbrev;
} LOCALECONV;

#define MAX_LANG_LEN        64  /* max language name length */
#define MAX_CTRY_LEN        64  /* max country name length */
#define MAX_MODIFIER_LEN    0   /* max modifier name length - n/a */
#define MAX_LC_LEN          (MAX_LANG_LEN+MAX_CTRY_LEN+MAX_MODIFIER_LEN+3)
                                /* max entire locale string length */
#define MAX_CP_LEN          5   /* max code page name length */


// Metrowerks has different define here
#if !defined (LC_MAX)
# if defined (LC_LAST)
#  define LC_MAX LC_LAST
# endif
#endif

//  non-NLS language string table
static LOCALECONV __rg_language[] =
{
  {"american",                    "ENU"},
  {"american english",            "ENU"},
  {"american-english",            "ENU"},
  {"australian",                  "ENA"},
  {"belgian",                     "NLB"},
  {"canadian",                    "ENC"},
  {"chh",                         "ZHH"},
  {"chi",                         "ZHI"},
  {"chinese",                     "CHS"},
  {"chinese-hongkong",            "ZHH"},
  {"chinese-simplified",          "CHS"},
  {"chinese-singapore",           "ZHI"},
  {"chinese-traditional",         "CHT"},
  {"dutch-belgian",               "NLB"},
  {"english-american",            "ENU"},
  {"english-aus",                 "ENA"},
  {"english-belize",              "ENL"},
  {"english-can",                 "ENC"},
  {"english-caribbean",           "ENB"},
  {"english-ire",                 "ENI"},
  {"english-jamaica",             "ENJ"},
  {"english-nz",                  "ENZ"},
  {"english-south africa",        "ENS"},
  {"english-trinidad y tobago",   "ENT"},
  {"english-uk",                  "ENG"},
  {"english-us",                  "ENU"},
  {"english-usa",                 "ENU"},
  {"french-belgian",              "FRB"},
  {"french-canadian",             "FRC"},
  {"french-luxembourg",           "FRL"},
  {"french-swiss",                "FRS"},
  {"german-austrian",             "DEA"},
  {"german-lichtenstein",         "DEC"},
  {"german-luxembourg",           "DEL"},
  {"german-swiss",                "DES"},
  {"irish-english",               "ENI"},
  {"italian-swiss",               "ITS"},
  {"norwegian",                   "NOR"},
  {"norwegian-bokmal",            "NOR"},
  {"norwegian-nynorsk",           "NON"},
  {"portuguese-brazilian",        "PTB"},
  {"spanish-argentina",           "ESS"},
  {"spanish-bolivia",             "ESB"},
  {"spanish-chile",               "ESL"},
  {"spanish-colombia",            "ESO"},
  {"spanish-costa rica",          "ESC"},
  {"spanish-dominican republic",  "ESD"},
  {"spanish-ecuador",             "ESF"},
  {"spanish-el salvador",         "ESE"},
  {"spanish-guatemala",           "ESG"},
  {"spanish-honduras",            "ESH"},
  {"spanish-mexican",             "ESM"},
  {"spanish-modern",              "ESN"},
  {"spanish-nicaragua",           "ESI"},
  {"spanish-panama",              "ESA"},
  {"spanish-paraguay",            "ESZ"},
  {"spanish-peru",                "ESR"},
  {"spanish-puerto rico",         "ESU"},
  {"spanish-uruguay",             "ESY"},
  {"spanish-venezuela",           "ESV"},
  {"swedish-finland",             "SVF"},
  {"swiss",                       "DES"},
  {"uk",                          "ENG"},
  {"us",                          "ENU"},
  {"usa",                         "ENU"}
};

//  non-NLS country string table
static LOCALECONV __rg_country[] =
{
  {"america",                     "USA"},
  {"britain",                     "GBR"},
  {"china",                       "CHN"},
  {"czech",                       "CZE"},
  {"england",                     "GBR"},
  {"great britain",               "GBR"},
  {"holland",                     "NLD"},
  {"hong-kong",                   "HKG"},
  {"new-zealand",                 "NZL"},
  {"nz",                          "NZL"},
  {"pr china",                    "CHN"},
  {"pr-china",                    "CHN"},
  {"puerto-rico",                 "PRI"},
  {"slovak",                      "SVK"},
  {"south africa",                "ZAF"},
  {"south korea",                 "KOR"},
  {"south-africa",                "ZAF"},
  {"south-korea",                 "KOR"},
  {"trinidad & tobago",           "TTO"},
  {"uk",                          "GBR"},
  {"united-kingdom",              "GBR"},
  {"united-states",               "USA"},
  {"us",                          "USA"},
};

typedef struct _Locale_ctype {
  LCID lcid;
  UINT cp;
  unsigned int ctable[257];
} _Locale_ctype_t;

typedef struct _Locale_numeric	{
  LCID lcid;
  char cp[MAX_CP_LEN+1];
  char decimal_point[4];
  char thousands_sep[4];
  char *grouping;
} _Locale_numeric_t;

typedef struct _Locale_time	{
  LCID lcid;
  char cp[MAX_CP_LEN+1];
  char *month[12];
  char *abbrev_month[12];
  char *dayofweek[7];
  char *abbrev_dayofweek[7];
} _Locale_time_t;

typedef struct _Locale_collate	{
  LCID lcid;
  char cp[MAX_CP_LEN+1];
} _Locale_collate_t;

typedef struct _Locale_monetary	{
  LCID lcid;
  char cp[MAX_CP_LEN+1];
  char decimal_point[4];
  char thousands_sep[4];
  char *grouping;
  char int_curr_symbol[5]; // 3+1+1
  char curr_symbol[6];
  char negative_sign[5];
  char positive_sign[5];
  int frac_digits;
  int int_frac_digits;
} _Locale_monetary_t;

typedef struct _Locale_messages	{
  LCID lcid;
  char cp[MAX_CP_LEN+1];
} _Locale_messages_t;

/* Internal function */
static void __FixGrouping(char *grouping);
static const char* __ConvertName(const char* lname, LOCALECONV* ConvTable, int TableSize);
static int __ParseLocaleString(const char* lname, char* lang, char* ctry, char* page);
static int __GetLCID(const char* lang, const char* ctry, LCID* lcid);
static int __GetLCIDFromName(const char* lname, LCID* lcid, char *cp);
static char* __GetLocaleName(LCID lcid, const char* cp, char* buf);
static char* __Extract_locale_name(const char* loc, int category, char* buf);
static char* __TranslateToSystem(const char* lname, char* buf);
static void __ConvertFromACP(char* buf, int buf_size, const char* cp);
static int __intGetACP(LCID lcid);
static int __intGetOCP(LCID lcid);
static int __GetDefaultCP(LCID lcid);
static char* __ConvertToCP(int from_cp, int to_cp, const char *from, size_t size, size_t *ret_buf_size);
static void my_ltoa(long __x, char* buf) ;

void my_ltoa(long __x, char* buf) 
{ 
  char rbuf[64];
  char* ptr = rbuf; 

  if (__x == 0)
    *ptr++ = '0';
  else {
    for (; __x != 0; __x /= 10)
      *ptr++ = (int)(__x % 10) + '0';
  }
  while(ptr > rbuf) *buf++ = *--ptr;
  //psw
  *buf = '\0';
} 

# ifdef __cplusplus
_STLP_BEGIN_NAMESPACE
extern "C" {
# endif

  void* _Locale_ctype_create(const char * name)
  {
    char cname[_Locale_MAX_SIMPLE_NAME];
    char cp_name[MAX_CP_LEN+1];
    int NativeCP;
    unsigned char Buffer[256];
    unsigned char *ptr;
    unsigned short ctable[256];
    CPINFO CPInfo;
    int i;
    wchar_t *wbuffer;
    int BufferSize;

    _Locale_ctype_t *ltype=(_Locale_ctype_t*)malloc(sizeof(_Locale_ctype_t));
    if(!ltype) return ltype;
    memset(ltype, 0, sizeof(_Locale_ctype_t));

    __Extract_locale_name(name, LC_CTYPE, cname);

    if(__GetLCIDFromName(cname, &ltype->lcid, cp_name)==-1)
      { free(ltype); return NULL; }
    ltype->cp = atoi(cp_name);

    NativeCP=__intGetACP(ltype->lcid);
    if(NativeCP == 0)
      NativeCP=__intGetOCP(ltype->lcid);

    /* Make table with all characters. */
    for(i = 0; i < 256; i++) Buffer[i] = i;

    if(!GetCPInfo(NativeCP, &CPInfo)) { free(ltype); return NULL; }

    if(CPInfo.MaxCharSize > 1)
      for(ptr=(unsigned char*)CPInfo.LeadByte; *ptr && *(ptr+1); ptr+=2)
	for(i=*ptr; i <= *(ptr+1); i++) Buffer[i] = 0;

    if(NativeCP != ltype->cp)
      {
    	OSVERSIONINFO ver_info;
        ver_info.dwOSVersionInfoSize = sizeof(ver_info);
    	GetVersionEx(&ver_info);
    	if(ver_info.dwPlatformId == VER_PLATFORM_WIN32_NT)
	  {
	    // Convert character sequence to Unicode.
	    BufferSize = MultiByteToWideChar(ltype->cp, MB_PRECOMPOSED, (const char*)Buffer, 256, NULL, 0);
	    wbuffer = (wchar_t*)malloc(BufferSize*sizeof(wchar_t));
	    if(!MultiByteToWideChar(ltype->cp, MB_PRECOMPOSED, (const char*)Buffer, 256, wbuffer, BufferSize))
	      { free(wbuffer); free(ltype); return NULL; }

	    GetStringTypeW(CT_CTYPE1, wbuffer, 256, ctable);

	    for(i = 0; i < 256; ++i)
	      ltype->ctable[i+1]=(unsigned int)ctable[i];

	    if(CPInfo.MaxCharSize > 1)
	      for(ptr=(unsigned char*)CPInfo.LeadByte; *ptr && *(ptr+1); ptr+=2)
		for(i=*ptr; i <= *(ptr+1); i++) ltype->ctable[i+1] = _LEADBYTE;

	    free(wbuffer);
	  }
    	else
	  {
	    unsigned char TargetBuffer[256];
	    GetStringTypeA(ltype->lcid, CT_CTYPE1, (const char*)Buffer, 256, ctable);

	    // Convert character sequence to target code page.
	    BufferSize = MultiByteToWideChar(NativeCP, MB_PRECOMPOSED, (const char*)Buffer, 256, NULL, 0);
	    wbuffer = (wchar_t*)malloc(BufferSize*sizeof(wchar_t));
	    if(!MultiByteToWideChar(NativeCP, MB_PRECOMPOSED, (const char*)Buffer, 256, wbuffer, BufferSize))
	      { free(wbuffer); free(ltype); return NULL; }
	    if(!WideCharToMultiByte(ltype->cp, WC_COMPOSITECHECK | WC_SEPCHARS, wbuffer, BufferSize, (char*)TargetBuffer, 256, NULL, FALSE))
	      { free(wbuffer); free(ltype); return NULL; }

	    free(wbuffer);

	    // Translate ctype table.
	    for(i = 0; i < 256; ++i)
	      {
		if(!TargetBuffer[i]) continue;
		ltype->ctable[TargetBuffer[i]+1] = ctable[i];
	      }
	    ltype->ctable[0] = 0; // EOF

	    // Mark lead byte.
	    if(!GetCPInfo(ltype->cp, &CPInfo)) { free(ltype); return NULL; }
	    if(CPInfo.MaxCharSize > 1)
	      for(ptr=(unsigned char*)CPInfo.LeadByte; *ptr && *(ptr+1); ptr+=2)
		for(i=*ptr; i <= *(ptr+1); i++) ltype->ctable[i+1] = _LEADBYTE;
	  }
      }
    else
      {
	GetStringTypeA(ltype->lcid, CT_CTYPE1, (const char*)Buffer, 256, ctable);
	for(i = 0; i < 256; ++i)
	  ltype->ctable[i+1]=(unsigned int)ctable[i];

    	if(CPInfo.MaxCharSize > 1)
	  for(ptr=(unsigned char*)CPInfo.LeadByte; *ptr && *(ptr+1); ptr+=2)
	    for(i=*ptr; i <= *(ptr+1); i++) ltype->ctable[i+1] = _LEADBYTE;
      }
    ltype->ctable[0] = 0; // EOF

    return ltype;
  }

  void* _Locale_numeric_create(const char * name)
  {
    char *GroupingBuffer;
    char cname[_Locale_MAX_SIMPLE_NAME];
    int BufferSize;
    _Locale_numeric_t *lnum=(_Locale_numeric_t*)malloc(sizeof(_Locale_numeric_t));
    if(!lnum) return lnum; // MS normal behavior for 'new'

    __Extract_locale_name(name, LC_NUMERIC, cname);

    if(__GetLCIDFromName(cname, &lnum->lcid, lnum->cp)==-1)
      { free(lnum); return NULL; }

    GetLocaleInfoA(lnum->lcid, LOCALE_SDECIMAL, lnum->decimal_point, 4);
    __ConvertFromACP(lnum->decimal_point, 4, lnum->cp);
    GetLocaleInfoA(lnum->lcid, LOCALE_STHOUSAND, lnum->thousands_sep, 4);
    __ConvertFromACP(lnum->thousands_sep, 4, lnum->cp);

    BufferSize=GetLocaleInfoA(lnum->lcid, LOCALE_SGROUPING, NULL, 0);
    GroupingBuffer=(char*)malloc(BufferSize);
    if(!GroupingBuffer) { lnum->grouping=NULL; return lnum; }
    GetLocaleInfoA(lnum->lcid, LOCALE_SGROUPING, GroupingBuffer, BufferSize);
    __FixGrouping(GroupingBuffer);
    lnum->grouping=GroupingBuffer;

    return lnum;
  }

  void* _Locale_time_create(const char * name)
  {
    int size, month, dayofweek;
    char cname[_Locale_MAX_SIMPLE_NAME];

    _Locale_time_t *ltime=(_Locale_time_t*)malloc(sizeof(_Locale_time_t));;
    if(!ltime) return ltime;
    memset(ltime, 0, sizeof(_Locale_time_t));

    __Extract_locale_name(name, LC_TIME, cname);

    if(__GetLCIDFromName(cname, &ltime->lcid, ltime->cp)==-1)
      { free(ltime); return NULL; }

    for(month=LOCALE_SMONTHNAME1; month<=LOCALE_SMONTHNAME12; month++) // Small hack :-)
      {
	size=GetLocaleInfoA(ltime->lcid, month, NULL, 0);
    	ltime->month[month-LOCALE_SMONTHNAME1]=(char*)malloc(size);
        if(!ltime->month[month-LOCALE_SMONTHNAME1]) { _Locale_time_destroy(ltime); return NULL; }
	GetLocaleInfoA(ltime->lcid, month, ltime->month[month-LOCALE_SMONTHNAME1], size);
    	__ConvertFromACP(ltime->month[month-LOCALE_SMONTHNAME1], size, ltime->cp);
      }

    for(month=LOCALE_SABBREVMONTHNAME1; month<=LOCALE_SABBREVMONTHNAME12; month++)
      {
	size=GetLocaleInfoA(ltime->lcid, month, NULL, 0);
	ltime->abbrev_month[month-LOCALE_SABBREVMONTHNAME1]=(char*)malloc(size);
	if(!ltime->abbrev_month[month-LOCALE_SABBREVMONTHNAME1]) { _Locale_time_destroy(ltime); return NULL; }
	GetLocaleInfoA(ltime->lcid, month, ltime->abbrev_month[month-LOCALE_SABBREVMONTHNAME1], size);
	__ConvertFromACP(ltime->abbrev_month[month-LOCALE_SABBREVMONTHNAME1], size, ltime->cp);
      }

    for(dayofweek=LOCALE_SDAYNAME1; dayofweek<=LOCALE_SDAYNAME7; dayofweek++)
      {
	int dayindex= ( dayofweek != LOCALE_SDAYNAME7 ) ?
	  dayofweek-LOCALE_SDAYNAME1+1 : 0;
	size=GetLocaleInfoA(ltime->lcid, dayofweek, NULL, 0);
	ltime->dayofweek[dayindex]=(char*)malloc(size);
	if(!ltime->dayofweek[dayindex] ) { _Locale_time_destroy(ltime); return NULL; }
	GetLocaleInfoA(ltime->lcid, dayofweek, ltime->dayofweek[dayindex], size);
	__ConvertFromACP(ltime->dayofweek[dayindex], size, ltime->cp);
      }

    for(dayofweek=LOCALE_SABBREVDAYNAME1; dayofweek<=LOCALE_SABBREVDAYNAME7; dayofweek++)
      {
	int dayindex= ( dayofweek != LOCALE_SABBREVDAYNAME7 ) ?
	  dayofweek-LOCALE_SABBREVDAYNAME1+1 : 0;
	size=GetLocaleInfoA(ltime->lcid, dayofweek, NULL, 0);
	ltime->abbrev_dayofweek[dayindex]=(char*)malloc(size);
	if(!ltime->abbrev_dayofweek[dayindex]) { _Locale_time_destroy(ltime); return NULL; }
	GetLocaleInfoA(ltime->lcid, dayofweek, ltime->abbrev_dayofweek[dayindex], size);
	__ConvertFromACP(ltime->abbrev_dayofweek[dayindex], size, ltime->cp);
      }

    return ltime;
  }

  void* _Locale_collate_create(const char * name)
  {
    char cname[_Locale_MAX_SIMPLE_NAME];

    _Locale_collate_t *lcol=(_Locale_collate_t*)malloc(sizeof(_Locale_collate_t));
    if(!lcol) return lcol;
    memset(lcol, 0, sizeof(_Locale_collate_t));

    __Extract_locale_name(name, LC_COLLATE, cname);

    if(__GetLCIDFromName(cname, &lcol->lcid, lcol->cp)==-1)
      { free(lcol); return NULL; }

    return lcol;
  }

  void* _Locale_monetary_create(const char * name)
  {
    char cname[_Locale_MAX_SIMPLE_NAME];
    char *GroupingBuffer;
    int BufferSize;
    char FracDigits[3];

    _Locale_monetary_t *lmon=(_Locale_monetary_t*)malloc(sizeof(_Locale_monetary_t));
    if(!lmon) return lmon;
    memset(lmon, 0, sizeof(_Locale_monetary_t));

    __Extract_locale_name(name, LC_MONETARY, cname);

    if(__GetLCIDFromName(cname, &lmon->lcid, lmon->cp)==-1)
      { free(lmon); return NULL; }

    // Extract information about monetary system
    GetLocaleInfoA(lmon->lcid, LOCALE_SDECIMAL, lmon->decimal_point, 4);
    __ConvertFromACP(lmon->decimal_point, 4, lmon->cp);
    GetLocaleInfoA(lmon->lcid, LOCALE_STHOUSAND, lmon->thousands_sep, 4);
    __ConvertFromACP(lmon->thousands_sep, 4, lmon->cp);

    BufferSize=GetLocaleInfoA(lmon->lcid, LOCALE_SGROUPING, NULL, 0);
    GroupingBuffer=(char*)malloc(BufferSize);
    if(!GroupingBuffer) { lmon->grouping=NULL; return lmon; }
    GetLocaleInfoA(lmon->lcid, LOCALE_SGROUPING, GroupingBuffer, BufferSize);
    __FixGrouping(GroupingBuffer);
    lmon->grouping=GroupingBuffer;

    GetLocaleInfoA(lmon->lcid, LOCALE_SCURRENCY, lmon->curr_symbol, 6);
    __ConvertFromACP(lmon->curr_symbol, 6, lmon->cp);

    GetLocaleInfoA(lmon->lcid, LOCALE_SNEGATIVESIGN, lmon->negative_sign, 5);
    __ConvertFromACP(lmon->negative_sign, 5, lmon->cp);

    GetLocaleInfoA(lmon->lcid, LOCALE_SPOSITIVESIGN, lmon->positive_sign, 5);
    __ConvertFromACP(lmon->positive_sign, 5, lmon->cp);

    GetLocaleInfoA(lmon->lcid, LOCALE_ICURRDIGITS, FracDigits, 3);
    lmon->frac_digits=atoi(FracDigits);

    GetLocaleInfoA(lmon->lcid, LOCALE_IINTLCURRDIGITS, FracDigits, 3);
    lmon->int_frac_digits=atoi(FracDigits);

    GetLocaleInfoA(lmon->lcid, LOCALE_SINTLSYMBOL, lmon->int_curr_symbol, 5);

    return lmon;
  }

  void* _Locale_messages_create(const char *name)
  {
    _Locale_messages_t *lmes=(_Locale_messages_t*)malloc(sizeof(_Locale_messages_t));
    if(!lmes) return lmes;
    memset(lmes, 0, sizeof(_Locale_messages_t));

    return lmes;
  }

  const char* _Locale_common_default(char* buf)
  {
    char cp[MAX_CP_LEN+1];
    int CodePage=__intGetACP(LOCALE_USER_DEFAULT);
    if(!CodePage) CodePage=__intGetOCP(LOCALE_USER_DEFAULT);
    my_ltoa(CodePage, cp);
    return __GetLocaleName(LOCALE_USER_DEFAULT, cp, buf);
  }

  const char* _Locale_ctype_default(char* buf)
  {
    return  _Locale_common_default(buf);
  }

  const char* _Locale_numeric_default(char * buf)
  {
    return  _Locale_common_default(buf);
  }

  const char* _Locale_time_default(char* buf)
  {
    return  _Locale_common_default(buf);
  }

  const char* _Locale_collate_default(char* buf)
  {
    return  _Locale_common_default(buf);
  }

  const char* _Locale_monetary_default(char* buf)
  {
    return  _Locale_common_default(buf);
  }

  const char* _Locale_messages_default(char* buf)
  {
    return  _Locale_common_default(buf);
  }

  char* _Locale_ctype_name(const void* loc, char* buf)
  {
    char cp_buf[MAX_CP_LEN+1];
    _Locale_ctype_t* ltype=(_Locale_ctype_t*)loc;
    my_ltoa(ltype->cp, cp_buf);
    return __GetLocaleName(ltype->lcid, cp_buf, buf);
  }

  char* _Locale_numeric_name(const void* loc, char* buf)
  {
    _Locale_numeric_t* lnum=(_Locale_numeric_t*)loc;
    return __GetLocaleName(lnum->lcid, lnum->cp, buf);
  }

  char* _Locale_time_name(const void* loc, char* buf)
  {
    _Locale_time_t* ltime=(_Locale_time_t*)loc;
    return __GetLocaleName(ltime->lcid, ltime->cp, buf);
  }

  char* _Locale_collate_name(const void* loc, char* buf)
  {
    _Locale_collate_t* lcol=(_Locale_collate_t*)loc;
    return __GetLocaleName(lcol->lcid, lcol->cp, buf);
  }

  char* _Locale_monetary_name(const void* loc, char* buf)
  {
    _Locale_monetary_t* lmon=(_Locale_monetary_t*)loc;
    return __GetLocaleName(lmon->lcid, lmon->cp, buf);
  }

  char* _Locale_messages_name(const void* loc, char* buf)
  {
    _Locale_messages_t* lmes=(_Locale_messages_t*)loc;
    return __GetLocaleName(lmes->lcid, lmes->cp, buf);
  }

  void _Locale_ctype_destroy(void* loc)
  {
    _Locale_ctype_t *ltype=(_Locale_ctype_t*)loc;
    if(!ltype) return;

    free(ltype);
  }

  void _Locale_numeric_destroy(void* loc)
  {
    _Locale_numeric_t *lnum=(_Locale_numeric_t *)loc;
    if(!lnum) return;

    if(lnum->grouping) free(lnum->grouping);
    free(lnum);
  }

  void _Locale_time_destroy(void* loc)
  {
    int i;
    _Locale_time_t* ltime=(_Locale_time_t*)loc;
    if(!ltime) return;

    for(i=0; i<12; i++)
      {
    	if(ltime->month[i]) free(ltime->month[i]);
    	if(ltime->abbrev_month[i]) free(ltime->abbrev_month[i]);
      }

    for(i=0; i<7; i++)
      {
    	if(ltime->dayofweek[i]) free(ltime->dayofweek[i]);
    	if(ltime->abbrev_dayofweek[i]) free(ltime->abbrev_dayofweek[i]);
      }

    free(ltime);
  }

  void _Locale_collate_destroy(void* loc)
  {
    _Locale_collate_t* lcol=(_Locale_collate_t*)loc;
    if(!lcol) return;

    free(lcol);
  }

  void _Locale_monetary_destroy(void* loc)
  {
    struct _Locale_monetary *lmon=(struct _Locale_monetary *)loc;
    if(!lmon) return;

    if(lmon->grouping) free(lmon->grouping);
    free(lmon);
  }

  void _Locale_messages_destroy(void* loc)
  {
    _Locale_messages_t* lmes=(_Locale_messages_t*)loc;
    if(!lmes) return;

    free(lmes);
  }

  char* _Locale_extract_ctype_name(const char* cname, char* buf) 
  {
    char lname[_Locale_MAX_SIMPLE_NAME];
    __Extract_locale_name(cname, LC_CTYPE, lname);
    if(lname[0] == 'C' && lname[1] == 0) return strcpy(buf, lname);
    return __TranslateToSystem(lname, buf);
  }

  char* _Locale_extract_numeric_name(const char* cname, char* buf) 
  {
    char lname[_Locale_MAX_SIMPLE_NAME];
    __Extract_locale_name(cname, LC_NUMERIC, lname);
    if(lname[0] == 'C' && lname[1] == 0) return strcpy(buf, lname);
    return __TranslateToSystem(lname, buf);
  }

  char* _Locale_extract_time_name(const char* cname, char* buf) 
  {
    char lname[_Locale_MAX_SIMPLE_NAME];
    __Extract_locale_name(cname, LC_TIME, lname);
    if(lname[0] == 'C' && lname[1] == 0) return strcpy(buf, lname);
    return __TranslateToSystem(lname, buf);
  }

  char* _Locale_extract_collate_name(const char* cname, char* buf) 
  {
    char lname[_Locale_MAX_SIMPLE_NAME];
    __Extract_locale_name(cname, LC_COLLATE, lname);
    if(lname[0] == 'C' && lname[1] == 0) return strcpy(buf, lname);
    return __TranslateToSystem(lname, buf);
  }

  char* _Locale_extract_monetary_name(const char* cname, char* buf) 
  {
    char lname[_Locale_MAX_SIMPLE_NAME];
    __Extract_locale_name(cname, LC_MONETARY, lname);
    if(lname[0] == 'C' && lname[1] == 0) return strcpy(buf, lname);
    return __TranslateToSystem(lname, buf);
  }

  char* _Locale_extract_messages_name(const char* cname, char* buf) 
  {
    if(cname[0] == 'L' && cname[1] == 'C' && cname[2] == '_')
      return strcpy(buf, "C");
    return strcpy(buf, cname);
  }

  char* _Locale_compose_name(char* buf,
			     const char* _ctype, const char* numeric,
			     const char* time, const char* _collate,
			     const char* monetary, const char* messages,
			     const char* default_name)
  {
    (void) default_name;

    if(!strcmp(_ctype, numeric) &&\
       !strcmp(_ctype, time) &&\
       !strcmp(_ctype, _collate) &&\
       !strcmp(_ctype, monetary) &&\
       !strcmp(_ctype, messages))
      return strcpy(buf, _ctype);

    strcpy(buf, "LC_CTYPE="); strcat(buf, _ctype); strcat(buf, ";");
    strcat(buf, "LC_TIME="); strcat(buf, time); strcat(buf, ";");
    strcat(buf, "LC_NUMERIC="); strcat(buf, numeric); strcat(buf, ";");
    strcat(buf, "LC_COLLATE="); strcat(buf, _collate); strcat(buf, ";");
    strcat(buf, "LC_MONETARY="); strcat(buf, monetary); strcat(buf, ";");
    strcat(buf, "LC_MESSAGES="); strcat(buf, messages); strcat(buf, ";");

    return buf;
  }

  /* ctype */

 const  _Locale_mask_t* _Locale_ctype_table(struct _Locale_ctype* ltype)
  {
    return (const _Locale_mask_t*)ltype->ctable;
  }

  int _Locale_toupper(struct _Locale_ctype* ltype, int c)
  {
    char buf[2], out_buf[2];;
    buf[0] = (char)c; buf[1] = 0;
    if(__GetDefaultCP(ltype->lcid) == ltype->cp)
      {
	LCMapStringA(ltype->lcid, LCMAP_LINGUISTIC_CASING | LCMAP_UPPERCASE, buf, 2, out_buf, 2);
	return out_buf[0];
      }
    else
      {
	wchar_t wbuf[2];
    	MultiByteToWideChar(ltype->cp, MB_PRECOMPOSED, buf, 2, wbuf, 2);
	WideCharToMultiByte(__GetDefaultCP(ltype->lcid), WC_COMPOSITECHECK | WC_SEPCHARS, wbuf, 2, buf, 2, NULL, FALSE);

	LCMapStringA(ltype->lcid, LCMAP_LINGUISTIC_CASING | LCMAP_UPPERCASE, buf, 2, out_buf, 2);

    	MultiByteToWideChar(__GetDefaultCP(ltype->lcid), MB_PRECOMPOSED, out_buf, 2, wbuf, 2);
	WideCharToMultiByte(ltype->cp, WC_COMPOSITECHECK | WC_SEPCHARS, wbuf, 2, out_buf, 2, NULL, FALSE);
	return out_buf[0];
      }
  }

  int _Locale_tolower(struct _Locale_ctype* ltype, int c)
  {
    char buf[2], out_buf[2];;
    buf[0] = (char)c; buf[1] = 0;
    if(__GetDefaultCP(ltype->lcid) == ltype->cp)
      {
	LCMapStringA(ltype->lcid, LCMAP_LINGUISTIC_CASING | LCMAP_LOWERCASE, buf, 2, out_buf, 2);
	return out_buf[0];
      }
    else
      {
	wchar_t wbuf[2];
    	MultiByteToWideChar(ltype->cp, MB_PRECOMPOSED, buf, 2, wbuf, 2);
	WideCharToMultiByte(__GetDefaultCP(ltype->lcid), WC_COMPOSITECHECK | WC_SEPCHARS, wbuf, 2, buf, 2, NULL, FALSE);

	LCMapStringA(ltype->lcid, LCMAP_LINGUISTIC_CASING | LCMAP_LOWERCASE, buf, 2, out_buf, 2);

    	MultiByteToWideChar(__GetDefaultCP(ltype->lcid), MB_PRECOMPOSED, out_buf, 2, wbuf, 2);
	WideCharToMultiByte(ltype->cp, WC_COMPOSITECHECK | WC_SEPCHARS, wbuf, 2, out_buf, 2, NULL, FALSE);
	return out_buf[0];
      }
  }

# ifndef _STLP_NO_WCHAR_T
  _Locale_mask_t _Locale_wchar_ctype(struct _Locale_ctype* ltype, wint_t c,
    _Locale_mask_t which_bits)
  {
    wchar_t buf[2];
    WORD out[2];
    buf[0] = c; buf[1] = 0;
    GetStringTypeW(CT_CTYPE1, buf, -1, out);

    return (_Locale_mask_t)out[0] & which_bits;
    //	ltype;
  }

  wint_t _Locale_wchar_tolower(struct _Locale_ctype* ltype, wint_t c)
  {
    wint_t res;

    LCMapStringW(ltype->lcid, LCMAP_LOWERCASE, &c, 1, &res, 1);
    return res;
  }

  wint_t _Locale_wchar_toupper(struct _Locale_ctype* ltype, wint_t c)
  {
    wint_t res;

    LCMapStringW(ltype->lcid, LCMAP_UPPERCASE, &c, 1, &res, 1);
    return res;
  }
# endif

# ifndef _STLP_NO_MBSTATE_T

  int _Locale_mb_cur_max (struct _Locale_ctype * ltype)
  {
    CPINFO CPInfo;
    if(!GetCPInfo(ltype->cp, &CPInfo)) return 0;
    return CPInfo.MaxCharSize;
  }

  int _Locale_mb_cur_min (struct _Locale_ctype *dummy)
  { return 1; }

  int _Locale_is_stateless (struct _Locale_ctype * ltype)
  {
    CPINFO CPInfo;
    GetCPInfo(ltype->cp, &CPInfo);
    return (CPInfo.MaxCharSize == 1) ? 1 : 0;
  }

  wint_t _Locale_btowc(struct _Locale_ctype * ltype, int c)
  {
    wchar_t wc;
    if(c == EOF) return WEOF;

    MultiByteToWideChar(ltype->cp, MB_PRECOMPOSED, (char*)&c, 1, &wc, 1);

    return (wint_t)wc;
  }

  int _Locale_wctob(struct _Locale_ctype * ltype, wint_t wc)
  {
    char c;

    if(WideCharToMultiByte(ltype->cp, WC_COMPOSITECHECK | WC_DEFAULTCHAR, (wchar_t*)&wc, 1, &c, 1, NULL, NULL) == 0)
      return WEOF; // Not single byte or error conversion.

    return (int)c;
  }

  static int __isleadbyte(int c, unsigned int *ctable)
  {
    return (ctable[(unsigned char)(c)] & _LEADBYTE);
  }

  static size_t __mbtowc(struct _Locale_ctype *l, wchar_t *dst, char src, mbstate_t *shift_state)
  {
    int result;

    if(*shift_state == 0)
      {
	if(__isleadbyte(src, l->ctable))
	  {
	    ((unsigned char*)shift_state)[0] = src;
	    return (size_t)-2;
	  }
	else
	  {
	    result = MultiByteToWideChar(l->cp, MB_PRECOMPOSED, &src, 1, dst, 1);
	    if(result == 0) return (size_t)-1;

	    return 1;
	  }
      }
    else
      {
	((unsigned char*)shift_state)[1] = src;
	result = MultiByteToWideChar(l->cp, MB_PRECOMPOSED, (const char*)shift_state, 2, dst, 1);
	*shift_state = 0;
	if(result == 0) return (size_t)-1;

	return 1;
      }
  }

  size_t _Locale_mbtowc(struct _Locale_ctype *ltype,
			wchar_t *to,
			const char *from, size_t n,
			mbstate_t *shift_state)
  {
    CPINFO ci;
    int result;

    GetCPInfo(ltype->cp, &ci);
    if(ci.MaxCharSize == 1) // Single byte encoding.
      {
	*shift_state = (mbstate_t)0;
	result = MultiByteToWideChar(ltype->cp, MB_PRECOMPOSED, from, 1, to, 1);
	if(result == 0) return (size_t)-1;
	return result;
      }
    else // Multi byte encoding.
      {
    	size_t retval, count = 0;
    	while(n--)
	  {
	    retval = __mbtowc(ltype, to, *from, shift_state);
	    if(retval == -2) { from++; count++; }
	    else if(retval == -1) return -1;
	    else return count+retval;
	  }
    	if(retval == -2) return (size_t)-2;

    	return n;
      }
  }

  size_t _Locale_wctomb(struct _Locale_ctype *ltype,
			char *to, size_t n,
			const wchar_t c,
			mbstate_t *shift_state)
  {
    int size = \
      WideCharToMultiByte(ltype->cp,  WC_COMPOSITECHECK | WC_SEPCHARS, &c, 1, NULL, 0, NULL, NULL);

    if(size > n) return (size_t)-2;

    size = \
      WideCharToMultiByte(ltype->cp,  WC_COMPOSITECHECK | WC_SEPCHARS, &c, 1, to, n, NULL, NULL);

    if(size == 0) return (size_t)-1;

    return (size_t)size;
  }

  size_t _Locale_unshift(struct _Locale_ctype *ltype,
			 mbstate_t *st,
			 char *buf, size_t n, char **next)
  {
    if(*st == 0)
      {
	*next = buf;
	return 0;
      }
    else
      {
	if(n < 1) { *next = buf; return (size_t)-2; }

	*next = buf + 1;
	return 1;
      }
  }

# endif /*  _STLP_NO_MBSTATE_T */


# ifndef CSTR_EQUAL /* VC5SP3*/
# define CSTR_EQUAL 2
# endif
# ifndef CSTR_LESS_THAN /* VC5SP3 */
# define CSTR_LESS_THAN 1
# endif

  /* Collate */
  int _Locale_strcmp(struct _Locale_collate* lcol,
		     const char* s1, size_t n1,
		     const char* s2, size_t n2)
  {
    int result;
    if(__GetDefaultCP(lcol->lcid) == atoi(lcol->cp))
      {
	result=CompareStringA(lcol->lcid, 0, s1, n1, s2, n2);
      }
    else
      {
	char *buf1, *buf2;
	size_t size1, size2;
	buf1 = __ConvertToCP(atoi(lcol->cp), __GetDefaultCP(lcol->lcid), s1, n1, &size1);
	buf2 = __ConvertToCP(atoi(lcol->cp), __GetDefaultCP(lcol->lcid), s2, n2, &size2);

	result=CompareStringA(lcol->lcid, 0, buf1, size1, buf2, size2);
	free(buf1); free(buf2);
      }
    return (result == CSTR_EQUAL) ? 0 : (result == CSTR_LESS_THAN) ? -1 : 1;
  }

# ifndef _STLP_NO_WCHAR_T

  int _Locale_strwcmp(struct _Locale_collate* lcol,
		      const wchar_t* s1, size_t n1,
		      const wchar_t* s2, size_t n2)
  {
    int result;
    result=CompareStringW(lcol->lcid, 0, s1, n1, s2, n2);
    return (result == CSTR_EQUAL) ? 0 : (result == CSTR_LESS_THAN) ? -1 : 1;
  }

# endif

  size_t _Locale_strxfrm(struct _Locale_collate* lcol,
			 char* dst, size_t dst_size,
			 const char* src, size_t src_size)
  {
    if(__GetDefaultCP(lcol->lcid) == atoi(lcol->cp))
      return LCMapStringA(lcol->lcid, LCMAP_SORTKEY, src, src_size, dst, dst_size);
    else
      {
	int result;
	char *buf;
	size_t size;
	buf = __ConvertToCP(atoi(lcol->cp), __GetDefaultCP(lcol->lcid), src, src_size, &size);

	result=LCMapStringA(lcol->lcid, LCMAP_SORTKEY, buf, size, dst, dst_size);
	free(buf);
	return result;
      }
  }

# ifndef _STLP_NO_WCHAR_T

  size_t _Locale_strwxfrm(struct _Locale_collate* lcol,
                          wchar_t* dst, size_t dst_size,
                          const wchar_t* src, size_t src_size)
  {
    return LCMapStringW(lcol->lcid, LCMAP_SORTKEY, src, src_size, dst, dst_size);
  }

# endif

  /* Numeric */

  static const char* __true_name="true";
  static const char* __false_name="false";

  char _Locale_decimal_point(struct _Locale_numeric* lnum)
  {
    return lnum->decimal_point[0];
  }

  char _Locale_thousands_sep(struct _Locale_numeric* lnum)
  {
    return lnum->thousands_sep[0];
  }

  const char* _Locale_grouping(struct _Locale_numeric * lnum)
  {
    if(!lnum->grouping) return "";
    else return lnum->grouping;
  }

  const char * _Locale_true(struct _Locale_numeric * lnum)
  {
    return __true_name; // NT does't provide information about this
  }

  const char * _Locale_false(struct _Locale_numeric * lnum)
  {
    return __false_name; // NT does't provide information about this
  }


  /* Monetary */

  const char* _Locale_int_curr_symbol(struct _Locale_monetary * lmon)
  {
    return lmon->int_curr_symbol;
  }

  const char* _Locale_currency_symbol(struct _Locale_monetary * lmon)
  {
    return lmon->curr_symbol;
  }

  char        _Locale_mon_decimal_point(struct _Locale_monetary * lmon)
  {
    return lmon->decimal_point[0];
  }

  char        _Locale_mon_thousands_sep(struct _Locale_monetary * lmon)
  {
    return lmon->thousands_sep[0];
  }

  const char* _Locale_mon_grouping(struct _Locale_monetary * lmon)
  {
    if(!lmon->grouping) return "";
    else return lmon->grouping;
  }

  const char* _Locale_positive_sign(struct _Locale_monetary * lmon)
  {
    return lmon->positive_sign;
  }

  const char* _Locale_negative_sign(struct _Locale_monetary * lmon)
  {
    return lmon->negative_sign;
  }

  char        _Locale_int_frac_digits(struct _Locale_monetary * lmon)
  {
    return (char)lmon->int_frac_digits;
  }

  char        _Locale_frac_digits(struct _Locale_monetary * lmon)
  {
    return (char)lmon->frac_digits;
  }

  int         _Locale_p_cs_precedes(struct _Locale_monetary * lmon)
  {
    char loc_data[2];
    GetLocaleInfoA(lmon->lcid, LOCALE_IPOSSYMPRECEDES, loc_data, 2);
    if(loc_data[0]=='0') return 0;
    else if(loc_data[0]=='1') return 1;
    else return -1;
  }

  int         _Locale_p_sep_by_space(struct _Locale_monetary * lmon)
  {
    char loc_data[2];
    GetLocaleInfoA(lmon->lcid, LOCALE_IPOSSEPBYSPACE, loc_data, 2);
    if(loc_data[0]=='0') return 0;
    else if(loc_data[0]=='1') return 1;
    else return -1;
  }

  int         _Locale_p_sign_posn(struct _Locale_monetary * lmon)
  {
    char loc_data[2];
    GetLocaleInfoA(lmon->lcid, LOCALE_IPOSSIGNPOSN, loc_data, 2);
    return atoi(loc_data);
  }

  int         _Locale_n_cs_precedes(struct _Locale_monetary * lmon)
  {
    char loc_data[2];
    GetLocaleInfoA(lmon->lcid, LOCALE_INEGSYMPRECEDES, loc_data, 2);
    if(loc_data[0]=='0') return 0;
    else if(loc_data[0]=='1') return 1;
    else return -1;
  }

  int          _Locale_n_sep_by_space(struct _Locale_monetary * lmon)
  {
    char loc_data[2];
    GetLocaleInfoA(lmon->lcid, LOCALE_INEGSEPBYSPACE, loc_data, 2);
    if(loc_data[0]=='0') return 0;
    else if(loc_data[0]=='1') return 1;
    else return -1;
  }

  int          _Locale_n_sign_posn(struct _Locale_monetary * lmon)
  {
    char loc_data[2];
    GetLocaleInfoA(lmon->lcid, LOCALE_INEGSIGNPOSN, loc_data, 2);
    return atoi(loc_data);
  }


  /* Time */
  const char * _Locale_full_monthname(struct _Locale_time * ltime, int month)
  {
    const char **names = (const char**)ltime->month;
    return names[month];
  }

  const char * _Locale_abbrev_monthname(struct _Locale_time * ltime, int month)
  {
    const char **names = (const char**)ltime->abbrev_month;
    return names[month];
  }

  const char * _Locale_full_dayofweek(struct _Locale_time * ltime, int day)
  {
    const char **names = (const char**)ltime->dayofweek;
    return names[day];
  }

  const char * _Locale_abbrev_dayofweek(struct _Locale_time * ltime, int day)
  {
    const char **names = (const char**)ltime->abbrev_dayofweek;
    return names[day];
  }

  const char* _Locale_d_t_fmt(struct _Locale_time* ltime)
  {
    // NT doesn't provide this information, and must simulate.
    static char buffer[MAX_PATH];
    strcpy(buffer, _Locale_d_fmt(ltime));
    strcat(buffer, " ");
    strcat(buffer, _Locale_t_fmt(ltime));
    return buffer;
  }

  const char* _Locale_long_d_t_fmt(struct _Locale_time* ltime)
  {
    // NT doesn't provide this information, and must simulate.
    static char buffer[MAX_PATH];
    strcpy(buffer, _Locale_long_d_fmt(ltime));
    strcat(buffer, " ");
    strcat(buffer, _Locale_t_fmt(ltime));
    return buffer;
  }

  const char* __ConvertDate(const char* NTTime)
  {
    static char ansi_date_fmt[MAX_PATH]; // Hack
    const char *cur_char;
    char *cur_output;

    // Correct time format.
    cur_char=NTTime;
    cur_output=ansi_date_fmt;
    while(*cur_char)
      {
    	switch(*cur_char)
	  {
	  case 'd':
            {
	      if(*(cur_char+1)=='d')
                {
		  if(*(cur_char+2)=='d')
		    {
		      if(*(cur_char+3)=='d')
			{ *(cur_output++)='%'; *(cur_output++)='A';  cur_char+=3; }
		      else
			{ *(cur_output++)='%'; *(cur_output++)='a';  cur_char+=2; }
		    }
		  else
		    { *(cur_output++)='%'; *(cur_output++)='d';  cur_char++; }
                }
	      else
		{ *(cur_output++)='%'; *(cur_output++)='#'; *(cur_output++)='d'; }
            }
            break;

	  case 'M':
            {
	      if(*(cur_char+1)=='M')
                {
		  if(*(cur_char+2)=='M')
		    {
		      if(*(cur_char+3)=='M')
			{ *(cur_output++)='%'; *(cur_output++)='B';  cur_char+=3; }
		      else
			{ *(cur_output++)='%'; *(cur_output++)='b';  cur_char+=2; }
		    }
		  else
		    { *(cur_output++)='%'; *(cur_output++)='m';  cur_char++; }
                }
	      else
		{ *(cur_output++)='%'; *(cur_output++)='#'; *(cur_output++)='m'; }
            }
            break;

	  case 'y':
            {
	      if(*(cur_char+1)=='y')
                {
		  if(*(cur_char+2)=='y' && *(cur_char+3)=='y')
		    { *(cur_output++)='%'; *(cur_output++)='Y';  cur_char+=3; }
		  else
		    { *(cur_output++)='%'; *(cur_output++)='y';  cur_char++; }
                }
	      else
		{ *(cur_output++)='%'; *(cur_output++)='#'; *(cur_output++)='y'; }
            }
            break;

	  case '%':
	    {
	      *(cur_output++)='%'; *(cur_output++)='%';
    	    }
            break;

	  case '\'':
	    {
	      cur_char++;
	      while(*cur_char!='\'' && *cur_char!=0) *cur_output++=*cur_char++;
	    }
	    break;

	  default:
	    {
	      *(cur_output++)=*cur_char;
            }
            break;
	  }
        if(*cur_char==0) break;
        cur_char++;
      }

    *cur_output=0;
    return ansi_date_fmt;
  }

  const char* _Locale_d_fmt(struct _Locale_time* ltime)
  {
    static char date_fmt[80];
    GetLocaleInfoA(ltime->lcid, LOCALE_SSHORTDATE, date_fmt, MAX_PATH);
    __ConvertFromACP(date_fmt, 80, ltime->cp);

    return __ConvertDate(date_fmt);
  }

  const char* _Locale_long_d_fmt(struct _Locale_time* ltime)
  {
    static char date_fmt[80];
    GetLocaleInfoA(ltime->lcid, LOCALE_SLONGDATE, date_fmt, MAX_PATH);
    __ConvertFromACP(date_fmt, 80, ltime->cp);

    return __ConvertDate(date_fmt);
  }

  const char* _Locale_t_fmt(struct _Locale_time* ltime)
  {
    char time_fmt[80];
    static char ansi_time_fmt[MAX_PATH]; // Hack
    char *cur_char, *cur_output;
    GetLocaleInfoA(ltime->lcid, LOCALE_STIMEFORMAT, time_fmt, MAX_PATH);
    __ConvertFromACP(time_fmt, 80, ltime->cp);

    // Correct time format.
    cur_char=time_fmt;
    cur_output=ansi_time_fmt;
    while(*cur_char)
      {
    	switch(*cur_char)
	  {
	  case 'h':
            {
	      if(*(cur_char+1)=='h')
		{ *(cur_output++)='%'; *(cur_output++)='I';  cur_char++; }
	      else
		{ *(cur_output++)='%'; *(cur_output++)='#'; *(cur_output++)='I'; }
            }
            break;

	  case 'H':
	    if(*(cur_char+1)=='H')
	      { *(cur_output++)='%'; *(cur_output++)='H'; cur_char++; }
	    else
	      { *(cur_output++)='%'; *(cur_output++)='#'; *(cur_output++)='H'; }
	    break;

	  case 'm':
            {
	      if(*(cur_char+1)=='m')
		{ *(cur_output++)='%'; *(cur_output++)='M';  cur_char++; }
	      else
		{ *(cur_output++)='%'; *(cur_output++)='#'; *(cur_output++)='M'; }
            }
            break;

	  case 's':
            {
	      if(*(cur_char+1)=='s')
		{ *(cur_output++)='%'; *(cur_output++)='S';  cur_char++; }
	      else
		{ *(cur_output++)='%'; *(cur_output++)='#'; *(cur_output++)='S'; }
            }
            break;

	  case 't':
            {
	      if(*(cur_char+1)=='t')
		cur_char++;

	      *(cur_output++)='%'; *(cur_output++)='p';
            }
            break;

	  case '%':
	    {
	      *(cur_output++)='%'; *(cur_output++)='%';
    	    }
            break;

	  case '\'':
	    {
	      cur_char++;
	      while(*cur_char!='\'' && *cur_char!=0) *cur_output++=*cur_char++;
	    }
	    break;

	  default:
	    {
	      *(cur_output++)=*cur_char;
            }
            break;
	  }
        if(*cur_char==0) break;
        cur_char++;
      }

    *cur_output=0;
    return ansi_time_fmt;
  }

  const char* _Locale_am_str(struct _Locale_time* ltime)
  {
    static char buffer[9];
    GetLocaleInfoA(ltime->lcid, LOCALE_S1159, buffer, 9);
    __ConvertFromACP(buffer, 9, ltime->cp);
    return buffer;
  }

  const char* _Locale_pm_str(struct _Locale_time* ltime)
  {
    static char buffer[9];
    GetLocaleInfoA(ltime->lcid, LOCALE_S2359, buffer, 9);
    __ConvertFromACP(buffer, 9, ltime->cp);
    return buffer;
  }

  const char* _Locale_t_fmt_ampm(struct _Locale_time* ltime)
  {
    char time_sep[4];
    static char buffer[17];
    GetLocaleInfoA(ltime->lcid, LOCALE_STIME, time_sep, 4);
    __ConvertFromACP(time_sep, 4, ltime->cp);
    strcpy(buffer, "%H"); strcat(buffer, time_sep);
    strcat(buffer, "%M"); strcat(buffer, time_sep);
    strcat(buffer, "%S %p");
    return buffer;
  }

  /* Messages */

  int _Locale_catopen(struct _Locale_messages* __DUMMY_PAR1, const char* __DUMMY_PAR)
  { return -1; }
  void _Locale_catclose(struct _Locale_messages* __DUMMY_PAR1, int __DUMMY_PAR) {}
  const char* _Locale_catgets(struct _Locale_messages* __DUMMY_PAR1, int __DUMMY_PAR2,
			      int __DUMMY_PAR3, int __DUMMY_PAR4,
			      const char *dfault)
  { return dfault; }

#ifdef __cplusplus    
} /* extern C */
_STLP_END_NAMESPACE
#endif

void __FixGrouping(char *grouping)
{
  /* This converts NT version which uses '0' instead of 0, etc ; to ANSI */
  while (*grouping) {
    if (*grouping >= '0' && *grouping <= '9') {
      *grouping = *grouping - '0'; 
      grouping++;
    } else if (*grouping == ';') {
      char *tmp = grouping;
      /* remove ':' */
      for (tmp = grouping; *tmp; tmp++)
        *tmp = *(tmp+1);
    } else
      grouping++;
  } 
}

const char* __ConvertName(const char* lname, LOCALECONV* ConvTable, int TableSize)
{
  int     i;
  int     cmp = 1;
  int     low = 0;
  int		high=TableSize-1;

  //  typical binary search - do until no more to search or match
  while (low <= high)
    {
      i = (low + high) / 2;

      if ((cmp=lstrcmpiA(lname, (*(ConvTable + i)).name))==0)
	return (*(ConvTable + i)).abbrev;
      else if (cmp < 0)
	high = i - 1;
      else
	low = i + 1;
    }
  return lname;
}

int __ParseLocaleString(const char* lname, char* lang, char* ctry,
char* page)
{
  int param=0;
  size_t len=0;
  char ch=lname[0];

  if(ch==0)
     return 0;
    
  else if(ch=='.') // Only code page provided
    {
      if(strlen(lname+1)>MAX_CP_LEN) return -1; // CP number too long
      strcpy(page, lname+1);
      lang[0] = 0; ctry[0] = 0;  //necessary? calling function does this too
      return 0;
    }

  while (ch!=0) // Parse string
    {
      len=strcspn(lname, "_.,"); // ',' for compability with POSIX

      ch=lname[len];
      if((param==0) && (len<MAX_LANG_LEN))
        {
          if (ch!='.') // hence '_', ',' or '/0'
                strncpy(lang, lname, len);
          else  // no ctry, read lang and skip ctry on next pass
            {
              ctry[0]=0;  
              strncpy(lang, lname, len);
              param++;    
             }
         }
      else if(param==1 && len<MAX_CTRY_LEN && ch!='_')
            strncpy(ctry, lname, len);
      else if(param==2 && len<MAX_CP_LEN && (ch==0 || ch==','))
            strncpy(page, lname, len);
      else
        return -1;
      if(ch==',') break; // Modifier found. In NT not used.
      param++;
      lname+=(len+1);
    }
  return 0;
}

/* Data necesary for find LCID*/
static int __FindFlag;
static LCID __FndLCID;
static const char* __FndLang;
static const char* __FndCtry;

static LCID LocaleFromHex(const char* locale)
{
  unsigned long result=0;
  int digit;
  while(*locale)
    {
      result<<=4;
      digit=(*locale>='0' && *locale<='9')? *locale-'0':
	(*locale>='A' && *locale<='F')?(*locale-'A')+10:(*locale-'a')+10;
      result+=digit;
      locale++;
    }
  return (LCID)result;
}

static BOOL CALLBACK EnumLocalesProcA(LPSTR locale)
{
  LCID lcid=LocaleFromHex(locale);
  int LangFlag=0, CtryFlag=!__FndCtry;
  static char Lang[MAX_LANG_LEN], Ctry[MAX_CTRY_LEN];

  GetLocaleInfoA(lcid, LOCALE_SENGLANGUAGE, Lang, MAX_LANG_LEN);
  if(lstrcmpiA(Lang, __FndLang)!=0)
    {
      GetLocaleInfoA(lcid, LOCALE_SABBREVLANGNAME, Lang, MAX_LANG_LEN);
      if(lstrcmpiA(Lang, __FndLang)==0) LangFlag=1;
    }
  else LangFlag=1;

  if(__FndCtry)
    {
      GetLocaleInfoA(lcid, LOCALE_SENGCOUNTRY, Ctry, MAX_CTRY_LEN);
      if(lstrcmpiA(Ctry, __FndCtry)!=0)
	{
	  GetLocaleInfoA(lcid, LOCALE_SABBREVCTRYNAME, Ctry, MAX_CTRY_LEN);
	  if(lstrcmpiA(Ctry, __FndCtry)==0) CtryFlag=1;
	}
      else CtryFlag=1;
    }

  if(LangFlag && CtryFlag)
    {
      __FindFlag=1;
      __FndLCID=lcid;
      return FALSE;
    }

  return TRUE;
}

int __GetLCID(const char* lang, const char* ctry, LCID* lcid)
{
  __FindFlag=0;
  __FndLang=lang;
  __FndCtry=ctry;
  EnumSystemLocalesA(EnumLocalesProcA, LCID_INSTALLED);

  if(__FindFlag==0) return -1;

  *lcid=__FndLCID;
  return 0;
}

int __GetLCIDFromName(const char* lname, LCID* lcid, char* cp)
{
  char lang[MAX_LANG_LEN+1], ctry[MAX_CTRY_LEN+1], page[MAX_CP_LEN+1];
  int result = 0;
  if(lname==NULL || lname[0]==0)
    {
      *lcid=LOCALE_USER_DEFAULT;
      return 0;
    }

  memset(lang, 0, MAX_LANG_LEN+1);
  memset(ctry, 0, MAX_CTRY_LEN+1);
  memset(page, 0, MAX_CP_LEN+1);
  if(__ParseLocaleString(lname, lang, ctry, page)==-1) return -1;

  if(lang[0] == 0 && ctry[0] == 0)
    *lcid = LOCALE_USER_DEFAULT; // Only code page given.
  else
    {
      if(ctry[0] == 0)
	result = __GetLCID(__ConvertName(lang, __rg_language, sizeof(__rg_language)/sizeof(LOCALECONV)), NULL, lcid);
      else
	result = __GetLCID(
			   __ConvertName(lang, __rg_language, sizeof(__rg_language)/sizeof(LOCALECONV)),
			   __ConvertName(ctry, __rg_country, sizeof(__rg_country)/sizeof(LOCALECONV)),
			   lcid);
    }

  if(result==0)
    {
      // Handling code page
      if(lstrcmpiA(page, "ACP")==0 || page[0]==0)
	my_ltoa(__intGetACP(*lcid), cp);
      else if(lstrcmpiA(page, "OCP")==0)
	my_ltoa(__intGetOCP(*lcid), cp);
      else strncpy(cp, page, 5);
    }
  return result;
}

char* __GetLocaleName(LCID lcid, const char* cp, char* buf)
{
  char lang[MAX_LANG_LEN+1], ctry[MAX_CTRY_LEN+1];
  GetLocaleInfoA(lcid, LOCALE_SENGLANGUAGE, lang, MAX_LANG_LEN);
  GetLocaleInfoA(lcid, LOCALE_SENGCOUNTRY, ctry, MAX_CTRY_LEN);
  strcpy(buf, lang); strcat(buf, "_");
  strcat(buf, ctry); strcat(buf, ".");
  return strcat(buf, cp);
}

static const char* __loc_categories[]=
{
  "LC_ALL", "LC_COLLATE", "LC_CTYPE", "LC_MONETARY", "LC_NUMERIC", "LC_TIME"
    };

char* __Extract_locale_name(const char* loc, int category, char* buf)
{
  char *expr;
  size_t len_name;
  buf[0] = 0;
  if(category < LC_ALL || category > LC_MAX) return NULL;

  if(loc[0]=='L' && loc[1]=='C' && loc[2]=='_')
    {
      expr=strstr((char*)loc, __loc_categories[category]);
      if(expr==NULL) return NULL; // Category not found.
      expr=strchr(expr, '=');
      if(expr==NULL) return NULL;
      expr++;
      len_name=strcspn(expr, ";");
      len_name=len_name>_Locale_MAX_SIMPLE_NAME?\
	_Locale_MAX_SIMPLE_NAME:len_name;
      strncpy(buf, expr, len_name);		buf[len_name]=0;
      return buf;
    }
  else
    return strncpy(buf, loc, _Locale_MAX_SIMPLE_NAME);
}

char* __TranslateToSystem(const char* lname, char* buf)
{
  LCID lcid;
  char cp[MAX_CP_LEN+1];
  if(__GetLCIDFromName(lname, &lcid, cp)!=0) return NULL;

  return __GetLocaleName(lcid, cp, buf);
}

void __ConvertFromACP(char* buf, int buf_size, const char* cp)
{
  wchar_t *Buffer;
  int BufferSize=MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
  Buffer=(wchar_t*)malloc(sizeof(wchar_t)*(BufferSize+1));
  MultiByteToWideChar(CP_ACP, 0, buf, -1, Buffer, BufferSize);
  WideCharToMultiByte(atoi(cp), 0, Buffer, -1, buf, buf_size, NULL, NULL);
  free(Buffer);
}

/* Return 0 if ANSI code page not used */
int __intGetACP(LCID lcid)
{
  char cp[6];
  GetLocaleInfoA(lcid, LOCALE_IDEFAULTANSICODEPAGE, cp, 6);
  return atoi(cp);
}

/* Return 1 if OEM code page not used */
int __intGetOCP(LCID lcid)
{
  char cp[6];
  GetLocaleInfoA(lcid, LOCALE_IDEFAULTCODEPAGE, cp, 6);
  return atoi(cp);
}

int __GetDefaultCP(LCID lcid)
{
  int cp = __intGetACP(lcid);
  if(cp == 0) return __intGetOCP(lcid);
  else return cp;
}

char* __ConvertToCP(int from_cp, int to_cp, const char *from, size_t size, size_t *ret_buf_size)
{
  int wbuffer_size, buffer_size;
  wchar_t *wbuffer;
  char* buffer;

  wbuffer_size = MultiByteToWideChar(from_cp, MB_PRECOMPOSED, from, size, NULL, 0);
  wbuffer = (wchar_t*)malloc(sizeof(wchar_t)*wbuffer_size);
  MultiByteToWideChar(from_cp, MB_PRECOMPOSED, from, size, wbuffer, wbuffer_size);

  buffer_size = WideCharToMultiByte(to_cp, WC_COMPOSITECHECK | WC_SEPCHARS, wbuffer, wbuffer_size, NULL, 0, NULL, FALSE);
  buffer = (char*)malloc(buffer_size);
  WideCharToMultiByte(to_cp, WC_COMPOSITECHECK | WC_SEPCHARS, wbuffer, wbuffer_size, buffer, buffer_size, NULL, FALSE);

  free(wbuffer);
  *ret_buf_size = buffer_size;
  return buffer;
}

#ifdef __cplusplus
}
// _STLP_END_NAMESPACE
#endif
