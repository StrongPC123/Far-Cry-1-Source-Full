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

# include "message_facets.h"

_STLP_BEGIN_NAMESPACE

//----------------------------------------------------------------------
// messages<char>

messages<char>::messages(_Messages* imp)  : 
  _BaseFacet(1), _M_impl(imp) { imp->_M_delete = false;  }

messages<char>::~messages()
{ 
  if (_M_impl && _M_impl->_M_delete)  delete _M_impl; 
}

int messages<char>::do_open(const string& filename, const locale& l) const
{
  return _M_impl->do_open(filename, l);
}

string messages<char>::do_get(catalog cat,
                              int set, int p_id, const string& dfault) const
{
  return _M_impl->do_get(cat, set, p_id, dfault);
}

void messages<char>::do_close(catalog cat) const
{
  _M_impl->do_close(cat);
}

_Messages::_Messages()
{ }

_Messages::~_Messages()
{}

int _Messages::do_open(const string&, const locale&) const
{  
  return -1;
}

string _Messages::do_get(catalog,
			 int, int, const string& dfault) const
{
  return dfault;
}

void _Messages::do_close(catalog) const
{}


# ifndef _STLP_NO_WCHAR_T

messages<wchar_t>::messages(_Messages* imp)  : 
  _BaseFacet(1), _M_impl(imp) { imp->_M_delete = false;  }

messages<wchar_t>::~messages()
{ if (_M_impl && _M_impl->_M_delete) delete _M_impl; }

int messages<wchar_t>::do_open(const string& filename, const locale& L) const
{
  return _M_impl->do_open(filename, L);
}

wstring
messages<wchar_t>::do_get(catalog thecat,
                          int set, int p_id, const wstring& dfault) const
{
  return _M_impl->do_get(thecat, set, p_id, dfault);
}

void messages<wchar_t>::do_close(catalog cat) const
{
  _M_impl->do_close(cat);
}

wstring
_Messages::do_get(catalog,
		  int, int, const wstring& dfault) const
{
  return dfault;
}

# endif

_STLP_END_NAMESPACE

// Local Variables:
// mode:C++
// End:

