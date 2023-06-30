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

// #include <locale>
#include <hash_map>
#include "locale_impl.h"
#include "c_locale.h"

#include "locale_nonclassic.h"


#include <stl/_codecvt.h>
#include <stl/_collate.h>
#include <stl/_ctype.h>
#include <stl/_monetary.h>
#include <stl/_time_facets.h>
#include <stl/_messages_facets.h>
#include <stl/_istream.h>
#include <stl/_num_get.h>
#include <stl/_num_put.h>


_STLP_BEGIN_NAMESPACE

// those wrappers are needed to avoid extern "C"

 void* _Loc_ctype_create(const char * s)
  { return (void*)_Locale_ctype_create(s); }
 void* _Loc_numeric_create(const char * s)
  { return (void*)_Locale_numeric_create(s); }
 void* _Loc_time_create(const char * s)
  { return (void*)_Locale_time_create(s); }
 void* _Loc_collate_create(const char * s)
  { return (void*)_Locale_collate_create(s); }
 void* _Loc_monetary_create(const char * s)
  { return (void*)_Locale_monetary_create(s); }
 void* _Loc_messages_create(const char * s)
  { return (void*)_Locale_messages_create(s); }

 char* _Loc_ctype_name(const void* l, char* s)
  { return _Locale_ctype_name(l, s); }
 char* _Loc_numeric_name(const void* l, char* s)
  { return _Locale_numeric_name(l, s); }
 char* _Loc_time_name(const void* l, char* s)
  { return _Locale_time_name(l,s); }
 char* _Loc_collate_name( const void* l, char* s)
  { return _Locale_collate_name(l,s); }
 char* _Loc_monetary_name(const void* l, char* s)
  { return _Locale_monetary_name(l,s); }
 char* _Loc_messages_name(const void* l, char* s)
  { return _Locale_messages_name(l,s); }

 const char* _Loc_ctype_default(char* p)    { return _Locale_ctype_default(p); }
 const char* _Loc_numeric_default(char * p) { return _Locale_numeric_default(p); }
 const char* _Loc_time_default(char* p)     { return _Locale_time_default(p); }
 const char* _Loc_collate_default(char* p)  { return _Locale_collate_default(p); }
 const char* _Loc_monetary_default(char* p) { return _Locale_monetary_default(p); }
 const char* _Loc_messages_default(char* p) { return _Locale_messages_default(p); }

 void _Loc_ctype_destroy(void* p)    {_Locale_ctype_destroy(p); }
 void _Loc_numeric_destroy(void* p)  {_Locale_numeric_destroy(p); }
 void _Loc_time_destroy(void* p)     {_Locale_time_destroy(p);}
 void _Loc_collate_destroy(void* p)  {_Locale_collate_destroy(p);}
 void _Loc_monetary_destroy(void* p) {_Locale_monetary_destroy(p);}
 void _Loc_messages_destroy(void* p) {_Locale_messages_destroy(p);}

typedef void* (*loc_create_func_t)(const char *);
typedef char* (*loc_name_func_t)(const void* l, char* s);
typedef void (*loc_destroy_func_t)(void* l);
typedef const char* (*loc_default_name_func_t)(char* s);

//----------------------------------------------------------------------
// Acquire and release low-level category objects.  The whole point of
// this is so that we don't allocate (say) four different _Locale_ctype
// objects for a single locale.

struct __eqstr {
  bool operator()(const char* s1, const char* s2) const
    { return strcmp(s1, s2) == 0; }
};

struct __ptr_hash {
  size_t operator()(const void* p) const
    { return __REINTERPRET_CAST(size_t,p); }
};

template <class _Category_ptr>
struct __destroy_fun {
  typedef void (*_fun_type)(_Category_ptr);
  _fun_type _M_fun;
  __destroy_fun(_fun_type __f) : _M_fun(__f) {}
  void operator()(_Category_ptr __c) { _M_fun(__c); }  
};

// Global hash tables for category objects.
typedef hash_map<const char*, pair<void*, size_t>, hash<const char*>, __eqstr> Category_Map;

// Look up a category by name
static hash_map<const char*, pair<void*, size_t>, hash<const char*>, __eqstr>* ctype_hash;
static hash_map<const char*, pair<void*, size_t>, hash<const char*>, __eqstr>* numeric_hash;
static hash_map<const char*, pair<void*, size_t>, hash<const char*>, __eqstr>* time_hash;
static hash_map<const char*, pair<void*, size_t>, hash<const char*>, __eqstr>* collate_hash;
static hash_map<const char*, pair<void*, size_t>, hash<const char*>, __eqstr>* monetary_hash;
static hash_map<const char*, pair<void*, size_t>, hash<const char*>, __eqstr>* messages_hash;

// We have a single lock for all of the hash tables.  We may wish to 
// replace it with six different locks.
/* REFERENCED */
_STLP_STATIC_MUTEX __category_hash_lock _STLP_MUTEX_INITIALIZER;

static void*
__acquire_category(const char* name, loc_create_func_t create_obj,
                   loc_default_name_func_t default_obj, Category_Map ** M)
{
  typedef Category_Map::iterator Category_iterator;
  pair<Category_iterator, bool> result;
  _STLP_auto_lock sentry(__category_hash_lock);

  typedef const char* key_type; 
  pair<const key_type, pair<void*,size_t> > __e(name, pair<void*,size_t>((void*)0,size_t(0)));

  if (!*M)
    *M = new Category_Map();

#if defined(__SC__)		//*TY 06/01/2000 - added workaround for SCpp
  if(!*M) delete *M;	//*TY 06/01/2000 - it forgets to generate dtor for Category_Map class. This fake code forces to generate one.
#endif					//*TY 06/01/2000 - 

  // Find what name to look for.  Be careful if user requests the default.
  char buf[_Locale_MAX_SIMPLE_NAME];
  if (name == 0 || name[0] == 0)
    name = default_obj(buf);
  if (name == 0 || name[0] == 0)
    name = "C";

  // Look for an existing entry with that name.

  result = (*M)->insert_noresize(__e);

  // There was no entry in the map already.  Create the category.
  if (result.second) 
    (*result.first).second.first = create_obj(name);

  // Increment the reference count.
  ++((*result.first).second.second);

  return (*result.first).second.first;
}


static void 
__release_category(void* cat,
                 loc_destroy_func_t destroy_fun,
                 loc_name_func_t get_name,
                 Category_Map* M)
{
  _STLP_auto_lock sentry(__category_hash_lock);

  if (cat && M) {
    // Find the name of the category object.
    char buf[_Locale_MAX_SIMPLE_NAME + 1];
    char* name = get_name(cat, buf);

    if (name != 0) {
      Category_Map::iterator it = M->find(name);
      if (it != M->end()) {
        // Decrement the ref count.  If it goes to zero, delete this category
        // from the map.
        if (--((*it).second.second) == 0) {
          void* cat1 = (*it).second.first;
          destroy_fun(cat1);
          M->erase(it);
        }
      }
    }
  }
}

_Locale_ctype* _STLP_CALL __acquire_ctype(const char* name)
{ return __REINTERPRET_CAST(_Locale_ctype*,
                            __acquire_category(name, _Loc_ctype_create, _Loc_ctype_default, &ctype_hash)); }
_Locale_numeric* _STLP_CALL __acquire_numeric(const char* name)
{ return __REINTERPRET_CAST(_Locale_numeric*,
                            __acquire_category(name, _Loc_numeric_create, _Loc_numeric_default, &numeric_hash)); }
_Locale_time* _STLP_CALL __acquire_time(const char* name)
{ return __REINTERPRET_CAST(_Locale_time*,
                            __acquire_category(name, _Loc_time_create, _Loc_time_default, &time_hash)); }
_Locale_collate* _STLP_CALL __acquire_collate(const char* name)
{ return __REINTERPRET_CAST(_Locale_collate*,
                            __acquire_category(name, _Loc_collate_create, _Loc_collate_default, &collate_hash)); }
_Locale_monetary* _STLP_CALL __acquire_monetary(const char* name)
{ return __REINTERPRET_CAST(_Locale_monetary*,
                            __acquire_category(name, _Loc_monetary_create, _Loc_monetary_default, &monetary_hash)); }
_Locale_messages* _STLP_CALL __acquire_messages(const char* name)
{ return __REINTERPRET_CAST(_Locale_messages*,
                            __acquire_category(name, _Loc_messages_create, _Loc_messages_default, &messages_hash)); }

void  _STLP_CALL __release_ctype(_Locale_ctype* cat) {
  __release_category(cat, _Loc_ctype_destroy, _Loc_ctype_name, ctype_hash);
}
void _STLP_CALL __release_numeric(_Locale_numeric* cat) {
  __release_category(cat, _Loc_numeric_destroy, _Loc_numeric_name, numeric_hash);
}
void _STLP_CALL __release_time(_Locale_time* cat) {
  __release_category(cat, _Loc_time_destroy, _Loc_time_name, time_hash);
}
void _STLP_CALL __release_collate(_Locale_collate* cat) {
  __release_category(cat, _Loc_collate_destroy, _Loc_collate_name, collate_hash);
}
void _STLP_CALL __release_monetary(_Locale_monetary* cat) {
  __release_category(cat, _Loc_monetary_destroy, _Loc_monetary_name, monetary_hash);
}
void _STLP_CALL __release_messages(_Locale_messages* cat) {
  __release_category(cat, _Loc_messages_destroy, _Loc_messages_name, messages_hash);
}


//
// <locale> content which is dependent on the name 
//

template <class Facet>
inline locale::facet* 
_Locale_insert(_Locale* __that, Facet* f) {
  return __that->insert(f, Facet::id._M_index, false);
}

// Give L a name where all facets except those in category c
// are taken from name1, and those in category c are taken from name2.
void _Stl_loc_combine_names(_Locale* L,
                   const char* name1, const char* name2,
                   locale::category c)
{
  if ((c & locale::all) == 0 || strcmp(name1, name2) == 0)
    L->name = name1;
  else if ((c & locale::all) == locale::all)
    L->name = name2;
  else {
    // Decompose the names.
    char ctype_buf[_Locale_MAX_SIMPLE_NAME];
    char numeric_buf[_Locale_MAX_SIMPLE_NAME];
    char time_buf[_Locale_MAX_SIMPLE_NAME];
    char collate_buf[_Locale_MAX_SIMPLE_NAME];
    char monetary_buf[_Locale_MAX_SIMPLE_NAME];
    char messages_buf[_Locale_MAX_SIMPLE_NAME];

    _Locale_extract_ctype_name((c & locale::ctype) ? name2 : name1,
                               ctype_buf); 
    _Locale_extract_numeric_name((c & locale::numeric) ? name2 : name1,
                                 numeric_buf); 
    _Locale_extract_time_name((c & locale::time) ? name2 : name1,
                              time_buf); 
    _Locale_extract_collate_name((c & locale::collate) ? name2 : name1,
                                 collate_buf); 
    _Locale_extract_monetary_name((c & locale::monetary) ? name2 : name1,
                                  monetary_buf); 
    _Locale_extract_messages_name((c & locale::messages) ? name2 : name1,
                                  messages_buf); 
    
    // Construct a new composite name.
    char composite_buf[_Locale_MAX_COMPOSITE_NAME];
    _Locale_compose_name(composite_buf,
                         ctype_buf, numeric_buf, time_buf,
                         collate_buf, monetary_buf, messages_buf,
                         name1);
    L->name = composite_buf;
  }
}


// Create a locale from a name.
locale::locale(const char* name)
  : _M_impl(0)
{
  if (!name)
    _M_throw_runtime_error(0);

  _Locale* impl = 0;

  _STLP_TRY {
    impl = new _Locale(locale::id::_S_max, name);

    // Insert categories one at a time.
    impl->insert_ctype_facets(name);
    impl->insert_numeric_facets(name);
    impl->insert_time_facets(name);
    impl->insert_collate_facets(name);
    impl->insert_monetary_facets(name);
    impl->insert_messages_facets(name);
    // reassign impl
    _M_impl = impl;
  }
  _STLP_UNWIND(delete impl);
}

// Create a locale that's a copy of L, except that all of the facets
// in category c are instead constructed by name.
locale::locale(const locale& L, const char* name, locale::category c)
  : _M_impl(0)
{
  if (name == 0 || strcmp(name, "*") == 0)
    _M_throw_runtime_error(name);

  _Locale* impl = 0;

  _STLP_TRY {
    impl = new _Locale(*L._M_impl);
    _Stl_loc_combine_names(impl, L._M_impl->name.c_str(), name, c);

    if (c & locale::ctype)
      impl->insert_ctype_facets(name);
    if (c & locale::numeric)
      impl->insert_numeric_facets(name);
    if (c & locale::time)
      impl->insert_time_facets(name);
    if (c & locale::collate)
      impl->insert_collate_facets(name);
    if (c & locale::monetary)
      impl->insert_monetary_facets(name);
    if (c & locale::messages)
      impl->insert_messages_facets(name);
    _M_impl = impl;
  }
  _STLP_UNWIND(delete impl)

}

// Contruct a new locale where all facets that aren't in category c
// come from L1, and all those that are in category c come from L2.
locale::locale(const locale& L1, const locale& L2, category c)
  : _M_impl(0)
{
  _Locale* impl = new _Locale(*L1._M_impl);
  
  _Locale_impl* i2 = L2._M_impl;

  static string nameless("*");
  if (L1.name() != nameless && L2.name() != nameless)
    _Stl_loc_combine_names(impl,
                  L1._M_impl->name.c_str(), L2._M_impl->name.c_str(),
                  c);
  else {
    impl->name = "*";
  }

  if (c & collate) {
    impl->insert( i2, _STLP_STD::collate<char>::id);
# ifndef _STLP_NO_WCHAR_T
    impl->insert( i2, _STLP_STD::collate<wchar_t>::id);
# endif
  }
  if (c & ctype) {
    impl->insert( i2, _STLP_STD::ctype<char>::id);
    impl->insert( i2, _STLP_STD::codecvt<char, char, mbstate_t>::id);
# ifndef _STLP_NO_WCHAR_T
    impl->insert( i2, _STLP_STD::ctype<wchar_t>::id);
    impl->insert( i2, _STLP_STD::codecvt<wchar_t, char, mbstate_t>::id);
# endif
  }
  if (c & monetary) {
    impl->insert( i2, _STLP_STD::moneypunct<char, true>::id);
    impl->insert( i2, _STLP_STD::moneypunct<char, false>::id);
    impl->insert( i2, _STLP_STD::money_get<char, istreambuf_iterator<char, char_traits<char> > >::id);
    impl->insert( i2, _STLP_STD::money_put<char, ostreambuf_iterator<char, char_traits<char> > >::id);
# ifndef _STLP_NO_WCHAR_T
    impl->insert( i2, _STLP_STD::moneypunct<wchar_t, true>::id);
    impl->insert( i2, _STLP_STD::moneypunct<wchar_t, false>::id);
    impl->insert( i2, _STLP_STD::money_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > >::id);
    impl->insert( i2, _STLP_STD::money_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > >::id);
# endif
  }
  if (c & numeric) {
    impl->insert( i2, _STLP_STD::numpunct<char>::id);
    impl->insert( i2, _STLP_STD::num_get<char, istreambuf_iterator<char, char_traits<char> > >::id);
    impl->insert( i2, _STLP_STD::num_put<char, ostreambuf_iterator<char, char_traits<char> > >::id);
# ifndef _STLP_NO_WCHAR_T
    impl->insert( i2, _STLP_STD::numpunct<wchar_t>::id);
    impl->insert( i2, num_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > >::id);
    impl->insert( i2, num_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > >::id);
# endif
  }
  if (c & time) {
    impl->insert( i2, _STLP_STD::time_get<char, istreambuf_iterator<char, char_traits<char> > >::id);
    impl->insert( i2, _STLP_STD::time_put<char, ostreambuf_iterator<char, char_traits<char> > >::id);
# ifndef _STLP_NO_WCHAR_T
    impl->insert( i2, _STLP_STD::time_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > >::id);
    impl->insert( i2, _STLP_STD::time_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > >::id);
# endif
  }
  if (c & messages) {
    impl->insert( i2, _STLP_STD::messages<char>::id);
# ifndef _STLP_NO_WCHAR_T
    impl->insert( i2, _STLP_STD::messages<wchar_t>::id);
# endif
  }
  _M_impl = impl;
}

// Six functions, one for each category.  Each of them takes a 
// _Locale* and a name, constructs that appropriate category
// facets by name, and inserts them into the locale.  

void _Locale::insert_ctype_facets(const char* pname)
{
  char buf[_Locale_MAX_SIMPLE_NAME];
  _Locale_impl* i2 = locale::classic()._M_impl;

  if (pname == 0 || pname[0] == 0)
    pname = _Locale_ctype_default(buf);

  if (pname == 0 || pname[0] == 0 || strcmp(pname, "C") == 0) {
    this->insert(i2, ctype<char>::id);
# ifndef _STLP_NO_MBSTATE_T
    this->insert(i2, codecvt<char, char, mbstate_t>::id);
# endif
# ifndef _STLP_NO_WCHAR_T
    this->insert(i2, ctype<wchar_t>::id);
# ifndef _STLP_NO_MBSTATE_T
    this->insert(i2, codecvt<wchar_t, char, mbstate_t>::id);
# endif
# endif
  }
  else {
    ctype<char>*    ct                      = 0;
# ifndef _STLP_NO_MBSTATE_T
    codecvt<char, char, mbstate_t>*    cvt  = 0;
# endif
# ifndef _STLP_NO_WCHAR_T
    ctype<wchar_t>* wct                     = 0;
    codecvt<wchar_t, char, mbstate_t>* wcvt = 0;
# endif
    _STLP_TRY {
      ct   = new ctype_byname<char>(pname);
# ifndef _STLP_NO_MBSTATE_T
      cvt  = new codecvt_byname<char, char, mbstate_t>(pname);
# endif
# ifndef _STLP_NO_WCHAR_T
      wct  = new ctype_byname<wchar_t>(pname); 
      wcvt = new codecvt_byname<wchar_t, char, mbstate_t>(pname);
# endif
    }
    
# ifndef _STLP_NO_WCHAR_T
#  ifdef _STLP_NO_MBSTATE_T
    _STLP_UNWIND(delete ct; delete wct; delete wcvt);
#  else
    _STLP_UNWIND(delete ct; delete wct; delete cvt; delete wcvt);
#  endif
# else
#  ifdef _STLP_NO_MBSTATE_T
    _STLP_UNWIND(delete ct);
#  else
    _STLP_UNWIND(delete ct; delete cvt);
#  endif
# endif    
    _Locale_insert(this, ct);
#  ifndef _STLP_NO_MBSTATE_T
    _Locale_insert(this, cvt);
#  endif
#  ifndef _STLP_NO_WCHAR_T
    _Locale_insert(this, wct);
    _Locale_insert(this, wcvt);
#  endif
  }
}

void _Locale::insert_numeric_facets(const char* pname)
{
  _Locale_impl* i2 = locale::classic()._M_impl;

  numpunct<char>*    punct  = 0;
  num_get<char, istreambuf_iterator<char, char_traits<char> > >*     get    = 0;
  num_put<char, ostreambuf_iterator<char, char_traits<char> > >*     put    = 0;
# ifndef _STLP_NO_WCHAR_T
  numpunct<wchar_t>* wpunct = 0;
  num_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > >*  wget   = 0;
  num_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > >*  wput   = 0;
# endif

  char buf[_Locale_MAX_SIMPLE_NAME];
  if (pname == 0 || pname[0] == 0)
    pname = _Locale_numeric_default(buf);

  if (pname == 0 || pname[0] == 0 || strcmp(pname, "C") == 0) {
    this->insert(i2, numpunct<char>::id);
    this->insert(i2, 
		 num_put<char, ostreambuf_iterator<char, char_traits<char> >  >::id);
    this->insert(i2, 
		 num_get<char, istreambuf_iterator<char, char_traits<char> > >::id);
# ifndef _STLP_NO_WCHAR_T
    this->insert(i2, numpunct<wchar_t>::id);
    this->insert(i2, 
		 num_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> >  >::id);
    this->insert(i2, 
		 num_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > >::id);
# endif
  }
  else {
    _STLP_TRY {
      punct  = new numpunct_byname<char>(pname);
      get    = new num_get<char, istreambuf_iterator<char, char_traits<char> > >;
      put    = new num_put<char, ostreambuf_iterator<char, char_traits<char> > >;
# ifndef _STLP_NO_WCHAR_T
      wpunct = new numpunct_byname<wchar_t>(pname);
      wget   = new num_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > >;
      wput   = new num_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > >;
# endif
    }
# ifndef _STLP_NO_WCHAR_T
    _STLP_UNWIND(delete punct; delete wpunct; delete get; delete wget;
    delete put; delete wput);
# else
    _STLP_UNWIND(delete punct; delete get;delete put);
# endif
    
  _Locale_insert(this,punct);
  _Locale_insert(this,get);
  _Locale_insert(this,put);

# ifndef _STLP_NO_WCHAR_T
  _Locale_insert(this,wpunct);
  _Locale_insert(this,wget);
  _Locale_insert(this,wput);
# endif
  }
}

void _Locale::insert_time_facets(const char* pname)
{
  _Locale_impl* i2 = locale::classic()._M_impl;
  time_get<char, istreambuf_iterator<char, char_traits<char> > >*    get  = 0;
  time_put<char, ostreambuf_iterator<char, char_traits<char> > >*    put  = 0;
# ifndef _STLP_NO_WCHAR_T
  time_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > >* wget = 0;
  time_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > >* wput = 0;
# endif

  char buf[_Locale_MAX_SIMPLE_NAME];
  if (pname == 0 || pname[0] == 0)
    pname = _Locale_time_default(buf);
  
  if (pname == 0 || pname[0] == 0 || strcmp(pname, "C") == 0) {

    this->insert(i2, 
		 time_get<char, istreambuf_iterator<char, char_traits<char> > >::id);
    this->insert(i2, 
		 time_put<char, ostreambuf_iterator<char, char_traits<char> > >::id);
# ifndef _STLP_NO_WCHAR_T
    this->insert(i2,
		 time_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > >::id);
    this->insert(i2,
		 time_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > >::id);
# endif
  }
  else {
    _STLP_TRY {
      get  = new time_get_byname<char, istreambuf_iterator<char, char_traits<char> > >(pname);
      put  = new time_put_byname<char, ostreambuf_iterator<char, char_traits<char> > >(pname);
# ifndef _STLP_NO_WCHAR_T
      wget = new time_get_byname<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > >(pname);
      wput = new time_put_byname<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > >(pname);
# endif
    }
# ifndef _STLP_NO_WCHAR_T
    _STLP_UNWIND(delete get; delete wget; delete put; delete wput);
# else
    _STLP_UNWIND(delete get; delete put);
# endif
    _Locale_insert(this,get);
    _Locale_insert(this,put);
# ifndef _STLP_NO_WCHAR_T
    _Locale_insert(this,wget);
    _Locale_insert(this,wput);
# endif
  }
}

void _Locale::insert_collate_facets(const char* nam)
{
  _Locale_impl* i2 = locale::classic()._M_impl;

  collate<char>*    col  = 0;
# ifndef _STLP_NO_WCHAR_T
  collate<wchar_t>* wcol = 0;
# endif

  char buf[_Locale_MAX_SIMPLE_NAME];
  if (nam == 0 || nam[0] == 0)
    nam = _Locale_collate_default(buf);

  if (nam == 0 || nam[0] == 0 || strcmp(nam, "C") == 0) {
    this->insert(i2, collate<char>::id);
# ifndef _STLP_NO_WCHAR_T
    this->insert(i2, collate<wchar_t>::id);
# endif
  }
  else {
    _STLP_TRY {
      col   = new collate_byname<char>(nam);
# ifndef _STLP_NO_WCHAR_T
      wcol  = new collate_byname<wchar_t>(nam); 
# endif
    }
# ifndef _STLP_NO_WCHAR_T
    _STLP_UNWIND(delete col; delete wcol);
# else
    _STLP_UNWIND(delete col);
# endif
    _Locale_insert(this,col);
# ifndef _STLP_NO_WCHAR_T
    _Locale_insert(this,wcol);
# endif
  }
}

void _Locale::insert_monetary_facets(const char* pname)
{
  _Locale_impl* i2 = locale::classic()._M_impl;

  moneypunct<char,    false>* punct   = 0;
  moneypunct<char,    true>*  ipunct  = 0;
  money_get<char, istreambuf_iterator<char, char_traits<char> > >*            get     = 0;
  money_put<char, ostreambuf_iterator<char, char_traits<char> > >*            put     = 0;

# ifndef _STLP_NO_WCHAR_T
  moneypunct<wchar_t, false>* wpunct  = 0;
  moneypunct<wchar_t, true>*  wipunct = 0;
  money_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > >*         wget    = 0;
  money_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > >*         wput    = 0;
# endif

  char buf[_Locale_MAX_SIMPLE_NAME];
  if (pname == 0 || pname[0] == 0)
    pname = _Locale_monetary_default(buf);

  if (pname == 0 || pname[0] == 0 || strcmp(pname, "C") == 0) {
    this->insert(i2, moneypunct<char, false>::id);
    this->insert(i2, moneypunct<char, true>::id);
    this->insert(i2, money_get<char, istreambuf_iterator<char, char_traits<char> > >::id);
    this->insert(i2, money_put<char, ostreambuf_iterator<char, char_traits<char> > >::id);
# ifndef _STLP_NO_WCHAR_T
    this->insert(i2, moneypunct<wchar_t, false>::id);
    this->insert(i2, moneypunct<wchar_t, true>::id);
    this->insert(i2, money_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > >::id);
    this->insert(i2, money_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > >::id);
# endif
  }
  else {    
    _STLP_TRY {
      punct   = new moneypunct_byname<char, false>(pname);
      ipunct  = new moneypunct_byname<char, true>(pname);
      get     = new money_get<char, istreambuf_iterator<char, char_traits<char> > >;
      put     = new money_put<char, ostreambuf_iterator<char, char_traits<char> > >;
# ifndef _STLP_NO_WCHAR_T
      wpunct  = new moneypunct_byname<wchar_t, false>(pname);
      wipunct = new moneypunct_byname<wchar_t, true>(pname);
      wget    = new money_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > >;
      wput    = new money_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > >;
# endif
    }
# ifndef _STLP_NO_WCHAR_T
    _STLP_UNWIND(delete punct; delete ipunct; delete wpunct; delete wipunct;
    delete get; delete wget; delete put; delete wput);
# else
    _STLP_UNWIND(delete punct; delete ipunct; delete get; delete put);
# endif
    _Locale_insert(this,punct);
    _Locale_insert(this,ipunct);
    _Locale_insert(this,get);
    _Locale_insert(this,put);
# ifndef _STLP_NO_WCHAR_T
    _Locale_insert(this,wget);
    _Locale_insert(this,wpunct);
    _Locale_insert(this,wipunct);
    _Locale_insert(this,wput);
# endif
  }
}


void _Locale::insert_messages_facets(const char* pname)
{
  _Locale_impl* i2 = locale::classic()._M_impl;
  messages<char>*    msg  = 0;
# ifndef _STLP_NO_WCHAR_T
  messages<wchar_t>* wmsg = 0;
# endif

  char buf[_Locale_MAX_SIMPLE_NAME];
  if (pname == 0 || pname[0] == 0)
    pname = _Locale_messages_default(buf);

  if (pname == 0 || pname[0] == 0 || strcmp(pname, "C") == 0) {
    this->insert(i2, messages<char>::id);
# ifndef _STLP_NO_WCHAR_T
    this->insert(i2, messages<wchar_t>::id);
# endif
  }
  else {
    _STLP_TRY {
      msg  = new messages_byname<char>(pname);
# ifndef _STLP_NO_WCHAR_T
      wmsg = new messages_byname<wchar_t>(pname);
# endif
    }
# ifndef _STLP_NO_WCHAR_T
    _STLP_UNWIND(delete msg; delete wmsg);
# else
    _STLP_UNWIND(delete msg);
# endif
    _Locale_insert(this,msg);
# ifndef _STLP_NO_WCHAR_T
    _Locale_insert(this,wmsg);
# endif
  }
}

_STLP_END_NAMESPACE



