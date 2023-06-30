#include "stdafx.h"
#include "XVehicleSystem.h"
#include <algorithm>


CVehicleSystem::CVehicleSystem()
{
}

CVehicleSystem::~CVehicleSystem()
{
}

void CVehicleSystem::AddVehicleClass( const EntityClassId classid)
{
		m_vVehicleClasses.push_back(classid);
}

bool CVehicleSystem::IsVehicleClass(const EntityClassId classid)
{
	return ( m_vVehicleClasses.end() != std::find(m_vVehicleClasses.begin(),m_vVehicleClasses.end(), classid) );
}