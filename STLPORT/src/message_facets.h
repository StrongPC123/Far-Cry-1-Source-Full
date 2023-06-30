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
#ifndef MESSAGE_FACETS_H
# define MESSAGE_FACETS_H

#include <string>
#include <stl/_messages_facets.h>
#include <stl/_ctype.h>
// #include <istream>
#include <typeinfo>
#include <hash_map>
#include "c_locale.h"

_STLP_BEGIN_NAMESPACE

// Forward declaration of an opaque type.
struct _Catalog_locale_map;

_Locale_messages* __acquire_messages(const char* name); 
void __release_messages(_Locale_messages* cat);

// Class _Catalog_locale_map.  The reason for this is that, internally,
// a message string is always a char*.  We need a ctype facet to convert
// a string to and from wchar_t, and the user is permitted to provide such
// a facet when calling open().

struct _Catalog_locale_map
{
  _Catalog_locale_map() : M(0) {}
  ~_Catalog_locale_map() { if (M) delete M; }

  void insert(int key, const locale& L);
  locale lookup(int key) const;
  void erase(int key);

  hash_map<int, locale, hash<int>, equal_to<int> >* M;

private:                        // Invalidate copy constructor and assignment
  _Catalog_locale_map(const _Catalog_locale_map&);
  void operator=(const _Catalog_locale_map&);
};


class _Messages {
public:
  typedef messages_base::catalog catalog;

  _Messages();

  virtual catalog     do_open(const string& __fn, const locale& __loc) const;
  virtual string do_get(catalog __c, int __set, int __msgid,
			const string& __dfault) const;
# ifndef _STLP_NO_WCHAR_T
  virtual wstring do_get(catalog __c, int __set, int __msgid,
                             const wstring& __dfault) const;
# endif
  virtual void        do_close(catalog __c) const;
  virtual ~_Messages();
  bool _M_delete;
};

class _Messages_impl : public _Messages {
public:

  _Messages_impl(bool);

  _Messages_impl(bool, _Locale_messages*);

  catalog     do_open(const string& __fn, const locale& __loc) const;
  string do_get(catalog __c, int __set, int __msgid,
			const string& __dfault) const;
# ifndef _STLP_NO_WCHAR_T
  wstring do_get(catalog __c, int __set, int __msgid,
		 const wstring& __dfault) const;
# endif
  void        do_close(catalog __c) const;
  
  ~_Messages_impl();

private:
  _Locale_messages* _M_message_obj;
  _Catalog_locale_map* _M_map;
};

_STLP_END_NAMESPACE

#endif
