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

# ifndef LOCALE_nonclassic_H
#  define  LOCALE_nonclassic_H

# include "locale_impl.h"

_STLP_BEGIN_NAMESPACE

class _STLP_CLASS_DECLSPEC _Locale : public _Locale_impl, public _Refcount_Base
{
public:
  _Locale(size_t n, const char* s) : _Locale_impl(s), _Refcount_Base(1), 
    facets_vec(n, (void*)0 ) { facets = (locale::facet**)&facets_vec[0]; _M_size = n; }
  _Locale(const _Locale_impl&);
  ~_Locale();

  virtual void incr() { this->_M_incr(); }
  virtual void decr() { this->_M_decr(); if (!this->_M_ref_count) delete this;}

  void remove(size_t index);
  locale::facet* insert(locale::facet*, size_t index, bool do_incr);
  void insert(_Locale_impl* from, const locale::id& n);

// Helper functions for byname construction of locales.
  void insert_ctype_facets(const char* name);
  void insert_numeric_facets(const char* name);
  void insert_time_facets(const char* name);
  void insert_collate_facets(const char* name);
  void insert_monetary_facets(const char* name);
  void insert_messages_facets(const char* name);
  
  vector<void*> facets_vec;

  static void _STLP_CALL _M_throw_bad_cast();

private:
  void operator=(const _Locale_impl&);
};

_STLP_END_NAMESPACE

#endif
