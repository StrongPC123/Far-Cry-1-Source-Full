//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// mem_ptr_fun_test.cpp
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright(c) 2001 Meridian'93
//  http://www.meridian93.com
//  mailto:info@meridian93.com
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <functional>
#include <memory>

#ifdef MAIN
#define mem_ptr_fun_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

#if defined(_STLP_DONT_RETURN_VOID) && (defined(_STLP_NO_MEMBER_TEMPLATE_CLASSES) && defined(_STLP_NO_CLASS_PARTIAL_SPECIALIZATION))
#  define _STLP_DONT_TEST_RETURN_VOID
#endif /*_STLP_DONT_RETURN_VOID*/
//else there is no workaround for the return void bug

struct S1 { } s1;
struct S2 { } s2;

int f1(S1&);
int f2(S1&, S2&);
int f1c(const S1&);
int f2c(const S1&, const S2&);

void vf1(S1&);
void vf2(S1&, S2&);
void vf1c(const S1&);
void vf2c(const S1&, const S2&);

class Class {
public:
  int f0();
  int f1(const S1&);

  void vf0();
  void vf1(const S1&);

  int f0c() const;
  int f1c(const S1&) const;

  void vf0c() const;
  void vf1c(const S1&) const;
};

int mem_ptr_fun_test(int, char**)
{
  Class obj;
  const Class& objc = obj;

  // ptr_fun

  ptr_fun(f1)(s1);
  ptr_fun(f2)(s1, s2);

  ptr_fun(f1c)(s1);
  ptr_fun(f2c)(s1, s2);

#ifndef _STLP_DONT_TEST_RETURN_VOID
  ptr_fun(vf1)(s1);
  ptr_fun(vf2)(s1, s2);

  ptr_fun(vf1c)(s1);
  ptr_fun(vf2c)(s1, s2);
#endif /* _STLP_DONT_TEST_RETURN_VOID */

  // mem_fun

  mem_fun(&Class::f0)(&obj);
  mem_fun(&Class::f1)(&obj, s1);

#ifndef _STLP_DONT_TEST_RETURN_VOID
  mem_fun(&Class::vf0)(&obj);
  mem_fun(&Class::vf1)(&obj, s1);
#endif /* _STLP_DONT_TEST_RETURN_VOID */

  // mem_fun (const)

  mem_fun(&Class::f0c)(&objc);
  mem_fun(&Class::f1c)(&objc, s1);

#ifndef _STLP_DONT_TEST_RETURN_VOID
  mem_fun(&Class::vf0c)(&objc);
  mem_fun(&Class::vf1c)(&objc, s1);
#endif /* _STLP_DONT_TEST_RETURN_VOID */

  // mem_fun_ref

  mem_fun_ref(&Class::f0)(obj);
  mem_fun_ref(&Class::f1)(obj, s1);

#ifndef _STLP_DONT_TEST_RETURN_VOID
  mem_fun_ref(&Class::vf0)(obj);
  mem_fun_ref(&Class::vf1)(obj, s1);
#endif /* _STLP_DONT_TEST_RETURN_VOID */

  // mem_fun_ref (const)

  mem_fun_ref(&Class::f0c)(objc);
  mem_fun_ref(&Class::f1c)(objc, s1);

#ifndef _STLP_DONT_TEST_RETURN_VOID
  mem_fun_ref(&Class::vf0c)(objc);
  mem_fun_ref(&Class::vf1c)(objc, s1);
#endif /* _STLP_DONT_TEST_RETURN_VOID */

  return 0;
}

int f1(S1&)
{return 1;}

int f2(S1&, S2&)
{return 2;}

int f1c(const S1&)
{return 1;}

int f2c(const S1&, const S2&)
{return 2;}

void vf1(S1&)
{}

void vf2(S1&, S2&)
{}

void vf1c(const S1&)
{}

void vf2c(const S1&, const S2&)
{}

int Class::f0()
{return 0;}

int Class::f1(const S1&)
{return 1;}

void Class::vf0()
{}

void Class::vf1(const S1&)
{}

int Class::f0c() const
{return 0;}

int Class::f1c(const S1&) const
{return 1;}

void Class::vf0c() const
{}

void Class::vf1c(const S1&) const
{}


#ifdef _STLP_DONT_TEST_RETURN_VOID
#  undef _STLP_DONT_TEST_RETURN_VOID
#endif