//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE source code
//	
//	File: CryModelShadowVolume.cpp
//
//	History:
//	-November 22,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <Cry_Camera.h>
#include <I3DEngine.h>
#include "..\Cry3DEngine\ShadowVolumeEdge.h"			// IEdgeConnectivityBuilder, IEdgeDetector
#include <IEdgeConnectivityBuilder.h>									// IEdgeConnectivityBuilder
#include "IncContHeap.h"
#include "CryModel.h"
#include "CryModelState.h"
#include "CVars.h"
#include "CryCharReShadowVolume.h"

