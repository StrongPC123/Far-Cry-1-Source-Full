/*
 * Copyright (c) 1999
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1999 
 * Boris Fomitchev
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


#include "stlport_prefix.h"
#include <stl/_time_facets.h>
#include <stl/_istream.h>
#include "c_locale.h"

_STLP_BEGIN_NAMESPACE

char* _STLP_CALL
__write_integer(char* buf, ios_base::fmtflags flags, long x);

// The function copy_cstring is used to initialize a string
// with a C-style string.  Used to initialize the month and weekday
// tables in time_get and time_put.  Called only by _Init_timeinfo
// so its name does not require leading underscores.

static inline void copy_cstring(const char * s, string& v) {
  copy(s, s + strlen(s), back_insert_iterator<string >(v));
}

// default "C" values for month and day names

  const char default_dayname[][14] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
    "Friday", "Saturday"};

  const char default_monthname[][24] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"};

// _Init_time_info: initialize table with
// "C" values (note these are not defined in the C standard, so this
// is somewhat arbitrary).

void _STLP_CALL _Init_timeinfo(_Time_Info& table) {
  int i;
  for (i = 0; i < 14; ++i)
    copy_cstring(default_dayname[i], table._M_dayname[i]);
  for (i = 0; i < 24; ++i)
    copy_cstring(default_monthname[i], table._M_monthname[i]);
  copy_cstring("AM", table._M_am_pm[0]);
  copy_cstring("PM", table._M_am_pm[1]);
  copy_cstring("%H:%M:%S", table._M_time_format);
  copy_cstring("%m/%d/%y",  table._M_date_format);
  copy_cstring("%a %b %e %H:%M:%S %Y", table._M_date_time_format);
}

void _STLP_CALL _Init_timeinfo(_Time_Info& table, _Locale_time * time) {
  int i;
  for (i = 0; i < 7; ++i)
    copy_cstring(_Locale_abbrev_dayofweek(time, i),
		 table._M_dayname[i]);
  for (i = 0; i < 7; ++i)
    copy_cstring(_Locale_full_dayofweek(time, i),
		 table._M_dayname[i+7]); 
  for (i = 0; i < 12; ++i)
    copy_cstring(_Locale_abbrev_monthname(time, i),
		 table._M_monthname[i]);
  for (i = 0; i < 12; ++i)
    copy_cstring(_Locale_full_monthname(time, i),
		 table._M_monthname[i+12]);
  copy_cstring(_Locale_am_str(time),
		 table._M_am_pm[0]);
  copy_cstring(_Locale_pm_str(time),
	       table._M_am_pm[1]);
  copy_cstring(_Locale_t_fmt(time), table._M_time_format);
  copy_cstring(_Locale_d_fmt(time), table._M_date_format);
  copy_cstring(_Locale_d_t_fmt(time), table._M_date_time_format);
  copy_cstring(_Locale_long_d_fmt(time), table._M_long_date_format);
  copy_cstring(_Locale_long_d_t_fmt(time), table._M_long_date_time_format);
}

inline char* __subformat(string format, char*& buf, 
			 const _Time_Info&  table, const tm* t) {
  const char * cp = format.data();
  const char * cp_end = cp + format.size();
  while (cp != cp_end) {
    if (*cp == '%') {
  	  char mod = 0;
      ++cp;
      if(*cp == '#') {
        mod = *cp; ++cp;
      }
      buf = __write_formatted_time(buf, *cp++, mod, table, t);
    } else
      *buf++ = *cp++;
  }
  return buf;
}

/* The number of days from the first day of the first ISO week of this
   year to the year day YDAY with week day WDAY.  ISO weeks start on
   Monday; the first ISO week has the year's first Thursday.  YDAY may
   be as small as YDAY_MINIMUM.  */
#define __ISO_WEEK_START_WDAY 1 /* Monday */
#define __ISO_WEEK1_WDAY 4 /* Thursday */
#define __YDAY_MINIMUM (-366)
#define __TM_YEAR_BASE 1900
static int
__iso_week_days (int yday, int wday)
{
  /* Add enough to the first operand of % to make it nonnegative.  */
  int big_enough_multiple_of_7 = (-__YDAY_MINIMUM / 7 + 2) * 7;
  return (yday
	  - (yday - wday + __ISO_WEEK1_WDAY + big_enough_multiple_of_7) % 7
	  + __ISO_WEEK1_WDAY - __ISO_WEEK_START_WDAY);
}

#define	__is_leap(year)	\
  ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

#define __hour12(hour) \
  (((hour) % 12 == 0) ? (12) : (hour) % 12)

char * __write_formatted_time(char* buf, char format, char modifier,
			      const _Time_Info& table, const tm* t) {
  switch(format) {
    case 'a':
      return copy(table._M_dayname[t->tm_wday].begin(),
	          table._M_dayname[t->tm_wday].end(),
	          buf);

    case 'A':
      return copy(table._M_dayname[t->tm_wday+7].begin(),
	          table._M_dayname[t->tm_wday+7].end(),
	          buf);

    case 'b':
      return copy(table._M_monthname[t->tm_mon].begin(),
	          table._M_monthname[t->tm_mon].end(),
	          buf);

    case 'B':
      return copy(table._M_monthname[t->tm_mon+12].begin(),
	     table._M_monthname[t->tm_mon+12].end(),
	     buf);

    case 'c': {
      const char * cp = (modifier != '#') ? table._M_date_time_format.data():\
      									   table._M_long_date_time_format.data();
      const char* cp_end = (modifier != '#') ? cp + table._M_date_time_format.size():\
      										  cp + table._M_long_date_time_format.size();
      char mod = 0;
      while (cp != cp_end) {
	if (*cp == '%') {
	  ++cp; if(*cp == '#') mod = *cp++; else mod = 0;
	  buf = __write_formatted_time(buf, *cp++, mod, table, t);
	}
	else
	  *buf++ = *cp++;
      }
      return buf;
    }

    case 'd': 
      sprintf(buf, (modifier != '#')?"%.2ld":"%ld", (long)t->tm_mday);
      return ((long)t->tm_mday < 10L && modifier == '#')?buf+1:buf + 2;

    case 'e':
      sprintf(buf, "%2ld", (long)t->tm_mday);
      return buf + 2;

    case 'H':
      sprintf(buf, (modifier != '#')?"%.2ld":"%ld", (long)t->tm_hour);
      return ((long)t->tm_hour < 10L && modifier == '#')?buf+1:buf + 2;

    case 'I':
      sprintf(buf, (modifier != '#')?"%.2ld":"%ld", (long)__hour12(t->tm_hour));
      return ((long)__hour12(t->tm_hour) < 10L && modifier == '#')?buf+1:buf + 2;

    case 'j':
      return __write_integer(buf, 0, (long)((long)t->tm_yday + 1));

    case 'm':
      sprintf(buf, (modifier != '#')?"%.2ld":"%ld", (long)t->tm_mon + 1);
      return ((long)(t->tm_mon + 1) < 10L && modifier == '#')?buf+1:buf + 2;

    case 'M':
      sprintf(buf, (modifier != '#')?"%.2ld":"%ld", (long)t->tm_min);
      return ((long)t->tm_min < 10L && modifier == '#')?buf+1:buf + 2;

    case 'p':
      return copy(table._M_am_pm[t->tm_hour/12].begin(),
	          table._M_am_pm[t->tm_hour/12].end(),
	   	  buf);

    case 'S': // pad with zeros
       sprintf(buf, (modifier != '#')?"%.2ld":"%ld", (long)t->tm_sec);
       return ((long)t->tm_sec < 10L && modifier == '#')?buf+1:buf + 2;

    case 'U':
      return __write_integer(buf, 0, 
			    long((t->tm_yday - t->tm_wday + 7) / 7));
      //      break;

    case 'w':
      return __write_integer(buf, 0, (long)t->tm_wday);
      //      break;

    case 'W':
      return __write_integer(buf, 0,
		             (long)(t->tm_wday == 0       ?
			      (t->tm_yday + 1) / 7 :
			      (t->tm_yday + 8 - t->tm_wday) / 7));

    case'x': {
      const char * cp = (modifier != '#') ? table._M_date_format.data():\
      									   table._M_long_date_format.data();
      const char* cp_end = (modifier != '#') ? cp + table._M_date_format.size():\
      										  cp + table._M_long_date_format.size();
      char mod = 0;
      while (cp != cp_end) {
	if (*cp == '%') {
	  ++cp; if(*cp == '#') mod = *cp++; else mod = 0;
	  buf = __write_formatted_time(buf, *cp++, mod, table, t);
	}
	else
	  *buf++ = *cp++;
      }
      return buf;
    }

    case 'X': {
      const char * cp = table._M_time_format.data();
      const char* cp_end = cp + table._M_time_format.size();
      char mod = 0;
      while (cp != cp_end) {
	if (*cp == '%') {
	  ++cp; if(*cp == '#') mod = *cp++; else mod = 0;
	  buf = __write_formatted_time(buf, *cp++, mod, table, t);
	}
	else
	  *buf++ = *cp++;
      }
      return buf;
    }
    case 'y':
      return __write_integer(buf, 0, (long)((long)(t->tm_year + 1900) % 100));

    case 'Y':
      return __write_integer(buf, 0, (long)((long)t->tm_year + 1900));

    case '%':
      *buf++ = '%';
      return buf;

#ifdef __GNUC__

      // fbp : at least on SUN 
# if defined ( _STLP_UNIX ) && ! defined (__linux__)
#  define __USE_BSD 1
# endif
 
   /*********************************************
    *     JGS, handle various extensions        *
    *********************************************/

    case 'h': /* POSIX.2 extension */
      // same as 'b', abbrev month name
      return copy(table._M_monthname[t->tm_mon].begin(),
	          table._M_monthname[t->tm_mon].end(),
	          buf);

    case 'C': /* POSIX.2 extension */
      // same as 'd', the day 
      sprintf(buf, "%2ld", (long)t->tm_mday);
      return buf + 2;

    case 'D': /* POSIX.2 extension */
      // same as 'x'
      return __subformat(table._M_date_format, buf, table, t);

    case 'k': /* GNU extension */
      sprintf(buf, "%2ld", (long)t->tm_hour);
      return buf + 2;

    case 'l': /* GNU extension */
      sprintf(buf, "%2ld", (long)t->tm_hour % 12);
      return buf + 2;

    case 'n': /* POSIX.2 extension */
      *buf++ = '\n';
      return buf;

    case 'R': /* GNU extension */
      return __subformat("%H:%M", buf, table, t);

    case 'r': /* POSIX.2 extension */
      return __subformat("%I:%M:%S %p", buf, table, t);

    case 'T': /* POSIX.2 extension.  */
      return __subformat("%H:%M:%S", buf, table, t);

    case 't': /* POSIX.2 extension.  */
      *buf++ = '\t';
      return buf;

    case 'u': /* POSIX.2 extension.  */
      return __write_integer(buf, 0, long((t->tm_wday - 1 + 7)) % 7 + 1);

    case 's': {
      time_t __t;
      __t = mktime ((tm*)t);
      return __write_integer(buf, 0, (long)__t );
    }
    case 'g': /* GNU extension */
    case 'G': {
      int year = t->tm_year + __TM_YEAR_BASE;
      int days = __iso_week_days (t->tm_yday, t->tm_wday);
      if (days < 0) {
	  /* This ISO week belongs to the previous year.  */
	  year--;
	  days = __iso_week_days (t->tm_yday + (365 + __is_leap (year)),
				t->tm_wday);
      } else {
	int d = __iso_week_days (t->tm_yday - (365 + __is_leap (year)),
			       t->tm_wday);
	if (0 <= d) {
	  /* This ISO week belongs to the next year.  */
	  year++;
	  days = d;
	}
      }
      switch (format) {
      case 'g':
	return __write_integer(buf, 0, (long)(year % 100 + 100) % 100);
      case 'G':
	return __write_integer(buf, 0, (long)year);
      default:
	return __write_integer(buf, 0, (long)days / 7 + 1);
      }
    }

# if defined ( _STLP_USE_GLIBC  ) && ! defined (__CYGWIN__)
    case 'z':		/* GNU extension.  */
      if (t->tm_isdst < 0)
	break;
      {
	int diff;
#if defined(__USE_BSD) || defined(__BEOS__)
	diff = t->tm_gmtoff;
#else
	diff = t->__tm_gmtoff;
#endif	
	if (diff < 0) {
	  *buf++ = '-';
	  diff = -diff;
	} else
	  *buf++ = '+';
	
	diff /= 60;
	sprintf(buf, "%.4d", (diff / 60) * 100 + diff % 60);
	return buf + 4;
      }
# endif /* __GLIBC__ */
#endif /* __GNUC__ */

    default:
      //      return buf;
      break;
  }
  return buf;
}

time_base::dateorder _STLP_CALL
__get_date_order(_Locale_time* time)
{
  const char * fmt = _Locale_d_fmt(time);
  char first, second, third;

  while (*fmt != 0 && *fmt != '%') ++fmt;
  if (*fmt == 0)
    return time_base::no_order;
  first = *++fmt;
  while (*fmt != 0 && *fmt != '%') ++fmt;
  if (*fmt == 0)
    return time_base::no_order;
  second = *++fmt;
  while (*fmt != 0 && *fmt != '%') ++fmt;
  if (*fmt == 0)
    return time_base::no_order;
  third = *++fmt;

  switch (first) {
    case 'd':
      return (second == 'm' && third == 'y') ? time_base::dmy
					     : time_base::no_order;
    case 'm':
      return (second == 'd' && third == 'y') ? time_base::mdy
					     : time_base::no_order;
    case 'y':
      switch (second) {
	case 'd':
	  return third == 'm' ? time_base::ydm : time_base::no_order;
        case 'm':
	  return third == 'd' ? time_base::ymd : time_base::no_order;
	default:
	  return time_base::no_order;
      }
    default:
      return time_base::no_order;
  }
}

#if !defined(_STLP_NO_FORCE_INSTANTIATE)
template class time_get<char, istreambuf_iterator<char, char_traits<char> > >;
// template class time_get<char, const char*>;
template class time_put<char, ostreambuf_iterator<char, char_traits<char> > >;
// template class time_put<char, char*>;

#ifndef _STLP_NO_WCHAR_T
template class time_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > >;
// template class time_get<wchar_t, const wchar_t*>;
template class time_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > >;
// template class time_put<wchar_t, wchar_t*>;
#endif /* INSTANTIATE_WIDE_STREAMS */

#endif

_STLP_END_NAMESPACE
