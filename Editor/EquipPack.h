#pragma once

#include <list>

struct SEquipment
{
	SEquipment()
	{
		sName="";
		nId=-1;
		sType="";
	}
	bool operator==(const SEquipment &e)
	{
		if (sName!=e.sName)
			return false;
		return true;
	}
	bool operator<(const SEquipment &e)
	{
		if (sName<e.sName)
			return true;
		if (sName!=e.sName)
			return false;
		return false;
	}
	CString sName;
	int nId;
	CString sType;
};

struct SAmmo
{
	bool operator==(const SAmmo &e)
	{
		if (sName!=e.sName)
			return false;
		return true;
	}
	bool operator<(const SAmmo &e)
	{
		if (sName<e.sName)
			return true;
		if (sName!=e.sName)
			return false;
		return false;
	}
	CString sName;
	int nAmount;
};

typedef std::list<SEquipment>		TLstEquipment;
typedef TLstEquipment::iterator	TLstEquipmentIt;

class CEquipPackLib;

class CEquipPack
{
private:
	CEquipPackLib *m_pCreator;
	CString m_sName;
	CString m_sPrimary;
	TLstEquipment m_lstEquips;
	XmlNodeRef m_AmmoXMLNode;
private:
	friend CEquipPackLib;
	CEquipPack(CEquipPackLib *pCreator);
	~CEquipPack();
	void SetName(const CString &sName) { m_sName=sName; }
public:
	void Release();
	CString GetName() { return m_sName; }
	bool AddEquip(const SEquipment &Equip);
	bool RemoveEquip(const SEquipment &Equip);
	bool AddAmmo(const SAmmo &Ammo);
	bool SetPrimary(CString &sPrimary);
	CString GetPrimary();
	void Clear();
	void Load(XmlNodeRef Node);
	bool Save(XmlNodeRef RootNode);
	int Count() { return (int)(m_lstEquips.size()); }
	TLstEquipment& GetEquip() { return m_lstEquips; }
	XmlNodeRef& GetAmmo() { return m_AmmoXMLNode; }
};
