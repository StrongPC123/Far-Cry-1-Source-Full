#pragma once

#include <map>

class CEquipPack;

typedef std::map<CString,CEquipPack*>	TMapEquipPack;
typedef	TMapEquipPack::iterator				TMapEquipPackIt;

class CEquipPackLib
{
private:
	TMapEquipPack m_mapEquipPacks;
public:
	CEquipPackLib();
	~CEquipPackLib();
	CEquipPack* CreateEquipPack(const CString &sName);
	bool RemoveEquipPack(const CString &sName);
	bool RenameEquipPack(const CString &sName, const CString &sNewName);
	CEquipPack* FindEquipPack(const CString &sName);
	void Serialize(XmlNodeRef &xmlNode, bool bLoading, bool bResetWhenLoad=true);
	//bool LoadFromPath(CString sPath, CString sWildcard);
	//bool SaveAll(CString sPath);
	void Reset();
	int Count() { return (int)(m_mapEquipPacks.size()); }
	TMapEquipPack& Get() { return m_mapEquipPacks; }
};
