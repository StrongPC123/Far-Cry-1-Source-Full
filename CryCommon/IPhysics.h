#ifndef cryphysics_h
#define cryphysics_h
#pragma once

#ifdef WIN32
	#ifdef PHYSICS_EXPORTS
		#define CRYPHYSICS_API __declspec(dllexport)
	#else
		#define CRYPHYSICS_API __declspec(dllimport)
	#endif
#else
	#define CRYPHYSICS_API
#endif
#define vector_class Vec3_tpl


#ifndef GAMECUBE
#include <CrySizer.h>
#endif



//#include "utils.h"
#include "Cry_Math.h"
#include "primitives.h"
#include "physinterface.h"

extern "C" CRYPHYSICS_API IPhysicalWorld *CreatePhysicalWorld(struct ISystem *pLog);

#endif