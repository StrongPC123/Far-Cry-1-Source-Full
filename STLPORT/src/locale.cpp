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
# include "stlport_prefix.h"

#include <locale>
#include <stdexcept>
#include <stl/_algobase.h>

#include "locale_nonclassic.h"

_STLP_BEGIN_NAMESPACE

_Locale::_Locale(const _Locale_impl& L)
  : _Locale_impl(L), _Refcount_Base(1), facets_vec((void**)L.facets, (void**)L.facets+L.size())
{
  for (size_t i = 1; i < L.size(); ++i) {
    locale::facet* f = L.facets[i];
    if (f && f->_M_delete)
      f->_M_incr();
  }
  facets = (locale::facet**)&facets_vec[0];
  _M_size = facets_vec.size();
}

_Locale::~_Locale() {
  size_t sz = facets_vec.size();
  for (size_t i = 1; i < sz ; ++i)
    this->remove(i);
}

void _Locale::remove(size_t index) {
  if (index > 0 && index < facets_vec.size()) {
    locale::facet* old = (locale::facet*)facets_vec[index];
    if (old && old->_M_delete) {
      old->_M_decr(); 
      if (old->_M_ref_count == 0)
        delete old;
    }
    facets_vec[index] = 0;
  }
}

locale::facet*
_Locale::insert(locale::facet* f, size_t index, bool do_incr) {
  if (f != 0 && index != 0) {
    if (index >= facets_vec.size()) {
      facets_vec.insert(facets_vec.end(),
                        index - facets_vec.size() + 1, (void*) 0);
      facets = (locale::facet**)&facets_vec[0];
      _M_size = facets_vec.size();
    }
    if (do_incr)
      f->_M_incr();
    
    remove(index);
    facets_vec[index] = (void*)f;
    return f;
  }
  else
    return 0;
}

void _Locale::insert(_Locale_impl* from, const locale::id& n) {
  size_t index = n._M_index;
  this->remove(index);
  if (index > 0 && index < from->size())
    this->insert(from->facets[index], index, true);
}


static _STLP_STATIC_MUTEX _Index_lock _STLP_MUTEX_INITIALIZER;

// Takes a reference to a locale::id, and returns its numeric index.
// If no numeric index has yet been assigned, assigns one.  The return
// value is always positive.
static size_t _Stl_loc_get_index(locale::id& id)
{
  if (id._M_index == 0) {
    _STLP_auto_lock sentry(_Index_lock);
    size_t new_index = locale::id::_S_max++;
    id._M_index = new_index;
  }
  return id._M_index;
}

void locale::_M_insert(facet* f, locale::id& n)
{
  if (f)
    ((_Locale*)_M_impl)->insert(f, _Stl_loc_get_index(n), false);
}


// Make a locale directly from a _Locale_impl object.  If the second argument
// is true, we clone the _Locale_impl.  If false, we just twiddle pointers.
locale::locale(_Locale_impl* impl, bool do_copy)
  : _M_impl(0)
{
  if (do_copy) {
    _M_impl = new _Locale(*impl);
    _M_impl->name = "*";
  } else
    _M_impl = _S_copy_impl(impl);
}

_STLP_END_NAMESPACE
