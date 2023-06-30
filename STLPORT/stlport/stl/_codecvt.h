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
// WARNING: This is an internal header file, included by other C++
// standard library headers.  You should not attempt to use this header
// file directly.


#ifndef _STLP_INTERNAL_CODECVT_H
#define _STLP_INTERNAL_CODECVT_H

# ifndef _STLP_C_LOCALE_H
#  include <stl/c_locale.h>
# endif
# ifndef _STLP_INTERNAL_LOCALE_H
#  include <stl/_locale.h>
# endif

_STLP_BEGIN_NAMESPACE

class _STLP_CLASS_DECLSPEC codecvt_base {
public:
  enum result {ok, partial, error, noconv};
};

template <class _InternT, class _ExternT, class _StateT>
class codecvt : public locale::facet, public codecvt_base {
  typedef _InternT intern_type;
  typedef _ExternT extern_type;
  typedef _StateT state_type;
};
 
template <class _InternT, class _ExternT, class _StateT>
class codecvt_byname : public codecvt<_InternT, _ExternT, _StateT> {};

_STLP_TEMPLATE_NULL
class _STLP_CLASS_DECLSPEC codecvt<char, char, mbstate_t>
  : public locale::facet, public codecvt_base 
{
  friend class _Locale;
public:
  typedef char       intern_type;
  typedef char       extern_type;
  typedef mbstate_t  state_type;

  explicit codecvt(size_t __refs = 0) : _BaseFacet(__refs) {}

  result out(state_type&  __state,
             const char*  __from,
             const char*  __from_end,
             const char*& __from_next,
             char*        __to,
             char*        __to_limit, 
             char*&       __to_next) const {
    return do_out(__state, 
                  __from, __from_end, __from_next,
                  __to,   __to_limit, __to_next);
  }

  result unshift(mbstate_t& __state,
                 char* __to, char* __to_limit, char*& __to_next) const
    { return do_unshift(__state, __to, __to_limit, __to_next); }
    
  result in(state_type&   __state,
            const char*  __from,
            const char*  __from_end,  
            const char*& __from_next,
            char*        __to, 
            char*        __to_limit, 
            char*&       __to_next) const {
    return do_in(__state,
                 __from, __from_end, __from_next,
                 __to,   __to_limit, __to_next);
  }

  int encoding() const _STLP_NOTHROW { return do_encoding(); }

  bool always_noconv() const _STLP_NOTHROW { return do_always_noconv(); }

  int length(const state_type& __state,
             const char* __from, const char* __end,
             size_t __max) const
    { return do_length(__state, __from, __end, __max); }
  
  int max_length() const _STLP_NOTHROW { return do_max_length(); }

  _STLP_STATIC_MEMBER_DECLSPEC static locale::id id;

protected:
  ~codecvt();

  virtual result do_out(mbstate_t&   /* __state */,
                        const char*  __from,
                        const char*  /* __from_end */,
                        const char*& __from_next,
                        char*        __to,
                        char*        /* __to_limit */,
                        char*&       __to_next) const;

  virtual result do_in (mbstate_t&   /* __state */ , 
                        const char*  __from,
                        const char*  /* __from_end */,
                        const char*& __from_next,
                        char*        __to,
                        char*        /* __to_end */,
                        char*&       __to_next) const;

  virtual result do_unshift(mbstate_t& /* __state */,
                            char*      __to,
                            char*      /* __to_limit */,
                            char*&     __to_next) const;

  virtual int do_encoding() const _STLP_NOTHROW;
  virtual bool do_always_noconv() const _STLP_NOTHROW;
  virtual int do_length(const mbstate_t&         __state,
                        const  char* __from, 
                        const  char* __end,
                        size_t __max) const;
  virtual int do_max_length() const _STLP_NOTHROW;
private:
  codecvt(const codecvt<char, char, mbstate_t>&);
  codecvt<char, char, mbstate_t>& operator =(const codecvt<char, char, mbstate_t>&); 
};

# ifndef _STLP_NO_WCHAR_T
 
_STLP_TEMPLATE_NULL
class _STLP_CLASS_DECLSPEC codecvt<wchar_t, char, mbstate_t>
  : public locale::facet, public codecvt_base
{
  friend class _Locale;
public:
  typedef wchar_t    intern_type;
  typedef char       extern_type;
  typedef mbstate_t  state_type;

  explicit codecvt(size_t __refs = 0) : _BaseFacet(__refs) {}

  result out(mbstate_t       __state,
             const wchar_t*  __from,
             const wchar_t*  __from_end,
             const wchar_t*& __from_next,
             char*           __to,
             char*           __to_limit,
             char*&          __to_next) const {
    return do_out(__state,
                  __from, __from_end, __from_next, 
                  __to,   __to_limit, __to_next);
  }

  result unshift(mbstate_t& __state,
                 char*  __to, char*  __to_limit, char*& __to_next) const {
    return do_unshift(__state, __to, __to_limit, __to_next);
  }
    
  result in(mbstate_t    __state,
            const char*  __from,
            const char*  __from_end,  
            const char*& __from_next,
            wchar_t*     __to, 
            wchar_t*     __to_limit, 
            wchar_t*&    __to_next) const {
    return do_in(__state, 
                 __from, __from_end, __from_next,
                 __to,  __to_limit, __to_next);
  }

  int encoding() const _STLP_NOTHROW { return do_encoding(); }

  bool always_noconv() const _STLP_NOTHROW { return do_always_noconv(); }

  int length(const mbstate_t&        __state,
             const char* __from,
             const char* __end,
             size_t             __max) const
    { return do_length(__state, __from, __end, __max); }
  
  int max_length() const _STLP_NOTHROW { return do_max_length(); }

  _STLP_STATIC_MEMBER_DECLSPEC static locale::id id;

protected:
  ~codecvt();

  virtual result do_out(mbstate_t&         __state,
                        const wchar_t*  __from,
                        const wchar_t*  __from_end,
                        const wchar_t*& __from_next,
                        char*        __to,
                        char*        __to_limit,
                        char*&       __to_next) const;

  virtual result do_in (mbstate_t&         __state,
                        const char*  __from,
                        const char*  __from_end,
                        const char*& __from_next,
                        wchar_t*        __to,
                        wchar_t*        __to_limit,
                        wchar_t*&       __to_next) const;

  virtual result do_unshift(mbstate_t&   __state,
                            char*  __to, 
                            char*  __to_limit,
                            char*& __to_next) const;

  virtual int do_encoding() const _STLP_NOTHROW;

  virtual bool do_always_noconv() const _STLP_NOTHROW;
  
  virtual int do_length(const mbstate_t& __state,
                        const  char* __from, 
                        const  char* __end,
                        size_t __max) const;

  virtual int do_max_length() const _STLP_NOTHROW;

private:
  codecvt(const codecvt<wchar_t, char, mbstate_t>&);
  codecvt<wchar_t, char, mbstate_t>& operator = (const codecvt<wchar_t, char, mbstate_t>&);  
};

# endif

_STLP_TEMPLATE_NULL
class _STLP_CLASS_DECLSPEC codecvt_byname<char, char, mbstate_t>
  : public codecvt<char, char, mbstate_t> {
public:
  explicit codecvt_byname(const char* __name, size_t __refs = 0);
  ~codecvt_byname();
private:
  codecvt_byname(const codecvt_byname<char, char, mbstate_t>&);
  codecvt_byname<char, char, mbstate_t>& operator =(const codecvt_byname<char, char, mbstate_t>&);  
};

# ifndef _STLP_NO_WCHAR_T
_STLP_TEMPLATE_NULL
class codecvt_byname<wchar_t, char, mbstate_t>
  : public codecvt<wchar_t, char, mbstate_t> 
{
public:
  explicit codecvt_byname(const char * __name, size_t __refs = 0);    

protected:
  ~codecvt_byname();

  virtual result do_out(mbstate_t&         __state,
                        const wchar_t*  __from,
                        const wchar_t*  __from_end,
                        const wchar_t*& __from_next,
                        char*        __to,
                        char*        __to_limit,
                        char*&       __to_next) const;

  virtual result do_in (mbstate_t&         __state,
                        const char*  __from,
                        const char*  __from_end,
                        const char*& __from_next,
                        wchar_t*        __to,
                        wchar_t*        __to_limit,
                        wchar_t*&       __to_next) const;

  virtual result do_unshift(mbstate_t&   __state,
                            char*  __to, 
                            char*  __to_limit,
                            char*& __to_next) const;

  virtual int do_encoding() const _STLP_NOTHROW;

  virtual bool do_always_noconv() const _STLP_NOTHROW;
  
  virtual int do_length(const mbstate_t&         __state,
                        const  char* __from, 
                        const  char* __end,
                        size_t __max) const;

  virtual int do_max_length() const _STLP_NOTHROW;

private:
  _Locale_ctype* _M_ctype;
  codecvt_byname(const codecvt_byname<wchar_t, char, mbstate_t>&);
  codecvt_byname<wchar_t, char, mbstate_t>& operator =(const codecvt_byname<wchar_t, char, mbstate_t>&);  
};

# endif

_STLP_END_NAMESPACE

#endif /* _STLP_INTERNAL_CODECVT_H */

// Local Variables:
// mode:C++
// End:

