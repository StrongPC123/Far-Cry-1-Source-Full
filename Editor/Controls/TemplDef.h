#if !defined(AFX_TEMPLDEF_H__742F3281_055B_11D4_B261_00104BB13A66__INCLUDED_)
#define AFX_TEMPLDEF_H__742F3281_055B_11D4_B261_00104BB13A66__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///////////////////////////////////////////////////////////////////////////////
// TemplDef.h: helper macroses for using templates with MFC
//
// Author: Yury Goltsman
// email:   ygprg@go.to
// page:    http://go.to/ygprg
// Copyright © 2000, Yury Goltsman
//
// This code provided "AS IS," without any kind of warranty.
// You may freely use or modify this code provided this
// Copyright is included in all derived versions.
//
// version : 1.0
//

///////////////////////////////////////////////////////////////////////////////
// common definitions for any map:

// use it to specify list of arguments for class and as theTemplArgs
// e.g. BEGIN_TEMPLATE_MESSAGE_MAP(ARGS2(class base_class, int max),
//                                 CMyTClass<ARGS2(base_class, max)>,
//                                 base_class)

#define ARGS2(arg1, arg2)                     arg1, arg2
#define ARGS3(arg1, arg2, arg3)               arg1, arg2, arg3
#define ARGS4(arg1, arg2, arg3, arg4)         arg1, arg2, arg3, arg4
#define ARGS5(arg1, arg2, arg3, arg4, arg5)   arg1, arg2, arg3, arg4, arg5

///////////////////////////////////////////////////////////////////////////////
// definition for MESSAGE_MAP:

#define DECLARE_TEMPLATE_MESSAGE_MAP() DECLARE_MESSAGE_MAP()

#if defined(WIN64) && _MFC_VER < 0x700
#ifdef _AFXDLL
#define BEGIN_TEMPLATE_MESSAGE_MAP(theTemplArgs, theClass, baseClass) \
	template <theTemplArgs> \
	const AFX_MSGMAP* PASCAL theClass::_GetBaseMessageMap() \
			{ return &baseClass::messageMap; } \
			template <theTemplArgs> \
			const AFX_MSGMAP* theClass::GetMessageMap() const \
			{ return &theClass::messageMap; } \
			template <theTemplArgs> \
			/*AFX_COMDAT AFX_DATADEF*/ const AFX_MSGMAP theClass::messageMap = \
		{ &theClass::_GetBaseMessageMap, &theClass::_messageEntries[0] }; \
		template <theTemplArgs> \
		/*AFX_COMDAT */const AFX_MSGMAP_ENTRY theClass::_messageEntries[] = \
		{ 
#else
#define BEGIN_TEMPLATE_MESSAGE_MAP(theTemplArgs, theClass, baseClass) \
	template <theTemplArgs> \
	const AFX_MSGMAP* theClass::GetMessageMap() const \
			{ return &theClass::messageMap; } \
			template <theTemplArgs> \
			/*AFX_COMDAT */AFX_DATADEF const AFX_MSGMAP theClass::messageMap = \
		{ &baseClass::messageMap, &theClass::_messageEntries[0] }; \
		template <theTemplArgs> \
		/*AFX_COMDAT */const AFX_MSGMAP_ENTRY theClass::_messageEntries[] = \
		{ 

#endif // _AFXDLL

#else

#ifdef _AFXDLL
#define BEGIN_TEMPLATE_MESSAGE_MAP(theTemplArgs, theClass, baseClass) \
	template <theTemplArgs> \
		const AFX_MSGMAP* PASCAL theClass::GetThisMessageMap() \
			{ return &theClass::messageMap; } \
	template <theTemplArgs> \
		const AFX_MSGMAP* theClass::GetMessageMap() const \
			{ return &theClass::messageMap; } \
	template <theTemplArgs> \
		/*AFX_COMDAT AFX_DATADEF*/ const AFX_MSGMAP theClass::messageMap = \
		{ &baseClass::GetThisMessageMap, &theClass::_messageEntries[0] }; \
	template <theTemplArgs> \
		/*AFX_COMDAT */const AFX_MSGMAP_ENTRY theClass::_messageEntries[] = \
		{ \

#else
#define BEGIN_TEMPLATE_MESSAGE_MAP(theTemplArgs, theClass, baseClass) \
	template <theTemplArgs> \
		const AFX_MSGMAP* theClass::GetMessageMap() const \
			{ return &theClass::messageMap; } \
	template <theTemplArgs> \
		/*AFX_COMDAT */AFX_DATADEF const AFX_MSGMAP theClass::messageMap = \
		{ &baseClass::messageMap, &theClass::_messageEntries[0] }; \
	template <theTemplArgs> \
		/*AFX_COMDAT */const AFX_MSGMAP_ENTRY theClass::_messageEntries[] = \
		{ \

#endif // _AFXDLL

#endif //WIN64

#define END_TEMPLATE_MESSAGE_MAP() END_MESSAGE_MAP()

#endif // !defined(AFX_TEMPLDEF_H__742F3281_055B_11D4_B261_00104BB13A66__INCLUDED_)
