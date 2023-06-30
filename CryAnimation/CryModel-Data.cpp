/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Oscar Blasco
//	Taken over by Vladimir Kajalin, Andrey Honich
//  Taken over by Sergiy Migdalskiy
//
//  This file contains definitions of static variables and tables used by the CryModel class
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CryModel.h"

// the static array of shadow volume vertices
// this holds the vertices of a deformed characters between calls to Deform()
// and functions that use its output; this avoids necessity to reallocate the array many times each frame
//TFixedArray<float> CryModel::g_arrShadowVolumeVerts ("CryModel.ShadowVolumeVerts");

// the following string tables are for recognition of different parts of the body
// by the bone name. depending on this recognition, the damage will be calculated.
const char *CryModel::g_szDamageBonesHead[]	=
{
	"Bip01 Neck",
	"Bip01 Neck1",
	"Bip01 Head",
	NULL
};

// the following string tables are for recognition of different parts of the body
// by the bone name. depending on this recognition, the damage will be calculated.
const char *CryModel::g_szDamageBonesTorso[]	=
{
	"Bip01 Pelvis",
	"Bip01 Spine",
	"Bip01 Spine1",
	"Bip01 Spine2",
	NULL
};

// the following string tables are for recognition of different parts of the body
// by the bone name. depending on this recognition, the damage will be calculated.
const char *CryModel::g_szDamageBonesArmL[]	=
{
	"Bip01 L Clavicle",	
	"Bip01 L UpperArm",
	"Bip01 L Forearm",
	"Bip01 L Hand",

	"Bip01 L Finger0",
	"Bip01 L Finger01",
	"Bip01 L Finger1",
	"Bip01 L Finger11",
	"Bip01 L Finger2",
	"Bip01 L Finger21",
	NULL
};

// the following string tables are for recognition of different parts of the body
// by the bone name. depending on this recognition, the damage will be calculated.
const char *CryModel::g_szDamageBonesArmR[]	=
{
	"Bip01 R Clavicle",
	"Bip01 R UpperArm",
	"Bip01 R Forearm",
	"Bip01 R Hand",

	"Bip01 R Finger0",
	"Bip01 R Finger01",
	"Bip01 R Finger1",
	"Bip01 R Finger11",
	"Bip01 R Finger2",
	"Bip01 R Finger21",
	NULL
};


// the following string tables are for recognition of different parts of the body
// by the bone name. depending on this recognition, the damage will be calculated.
const char *CryModel::g_szDamageBonesLegL[]	=
{
	"Bip01 L Thigh",
	"Bip01 L Calf",
	"Bip01 L Foot",
	"Bip01 L Toe0",
	NULL
};

// the following string tables are for recognition of different parts of the body
// by the bone name. depending on this recognition, the damage will be calculated.
const char *CryModel::g_szDamageBonesLegR[]	=
{
	"Bip01 R Thigh",
	"Bip01 R Calf",
	"Bip01 R Foot",
	"Bip01 R Toe0",
	NULL
};
