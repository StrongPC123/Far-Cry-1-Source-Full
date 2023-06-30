#ifndef _GEOM_COMMON_HDR_
#define _GEOM_COMMON_HDR_

#include "TFace.h"
#pragma pack(push,1)
typedef TFace<unsigned short> GeomFace;
#pragma pack(pop)
typedef unsigned char GeomMtlID;
#endif