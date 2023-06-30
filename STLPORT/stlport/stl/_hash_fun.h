/*
 * Copyright (c) 1996-1998
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef _STLP_HASH_FUN_H
#define _STLP_HASH_FUN_H

# ifndef _STLP_CSTDDEF
#  include <cstddef>
# endif

_STLP_BEGIN_NAMESPACE

template <class _Key> struct hash { };

inline size_t __stl_hash_string(const char* __s)
{
  _STLP_FIX_LITERAL_BUG(__s)
  unsigned long __h = 0; 
  for ( ; *__s; ++__s)
    __h = 5*__h + *__s;
  
  return size_t(__h);
}

_STLP_TEMPLATE_NULL struct hash<char*>
{
  size_t operator()(const char* __s) const { _STLP_FIX_LITERAL_BUG(__s) return __stl_hash_string(__s); }
};

_STLP_TEMPLATE_NULL struct hash<const char*>
{
  size_t operator()(const char* __s) const { _STLP_FIX_LITERAL_BUG(__s) return __stl_hash_string(__s); }
};

_STLP_TEMPLATE_NULL struct hash<char> {
  size_t operator()(char __x) const { return __x; }
};
_STLP_TEMPLATE_NULL struct hash<unsigned char> {
  size_t operator()(unsigned char __x) const { return __x; }
};
#ifndef _STLP_NO_SIGNED_BUILTINS
_STLP_TEMPLATE_NULL struct hash<signed char> {
  size_t operator()(unsigned char __x) const { return __x; }
};
#endif
_STLP_TEMPLATE_NULL struct hash<short> {
  size_t operator()(short __x) const { return __x; }
};
_STLP_TEMPLATE_NULL struct hash<unsigned short> {
  size_t operator()(unsigned short __x) const { return __x; }
};
_STLP_TEMPLATE_NULL struct hash<int> {
  size_t operator()(int __x) const { return __x; }
};
_STLP_TEMPLATE_NULL struct hash<unsigned int> {
  size_t operator()(unsigned int __x) const { return __x; }
};
_STLP_TEMPLATE_NULL struct hash<long> {
  size_t operator()(long __x) const { return __x; }
};
_STLP_TEMPLATE_NULL struct hash<unsigned long> {
  size_t operator()(unsigned long __x) const { return __x; }
};

# if defined (_STLP_LONG_LONG)
_STLP_TEMPLATE_NULL struct hash<_STLP_LONG_LONG> {
  size_t operator()(long x) const { return x; }
};
_STLP_TEMPLATE_NULL struct hash<unsigned _STLP_LONG_LONG> {
  size_t operator()(unsigned long x) const { return x; }
};
# endif

_STLP_END_NAMESPACE

#endif /* _STLP_HASH_FUN_H */

// Local Variables:
// mode:C++
// End:
