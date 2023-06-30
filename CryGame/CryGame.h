
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#ifndef __crygame_h__
#define __crygame_h__
#pragma once

#ifdef WIN32
	#ifdef CRYGAME_EXPORTS
		#define CRYGAME_API __declspec(dllexport)
	#else
		#define CRYGAME_API __declspec(dllimport)
	#endif
#else
	#define CRYGAME_API
#endif

#endif // __crygame_h__