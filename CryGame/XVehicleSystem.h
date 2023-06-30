
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//	XVehicleSystem.h
//	- vehicle system class declaration  - 
//	A simple class that takes care of the class id's of the vehicles
//	Created by: Petar Kotevski
//
//////////////////////////////////////////////////////////////////////////

#ifndef _CVEHICLESYSTEM_
#define _CVEHICLESYSTEM_

#include <vector>

typedef std::vector<EntityClassId> VehicleClassVector;


class CVehicleSystem
{

	VehicleClassVector	m_vVehicleClasses;

public:
	CVehicleSystem();
	~CVehicleSystem();

	void AddVehicleClass(const EntityClassId classid);
	bool IsVehicleClass(const EntityClassId classid);
};


#endif // _CVEHICLESYSTEM_






