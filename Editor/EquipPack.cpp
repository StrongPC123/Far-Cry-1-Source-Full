#include "StdAfx.h"
#include "equippack.h"
#include "equippacklib.h"

CEquipPack::CEquipPack(CEquipPackLib *pCreator)
{
	m_pCreator=pCreator;
	m_sPrimary="";
	XmlNodeRef templ=GetIEditor()->FindTemplate("WeaponAmmo");
	if (templ)
		m_AmmoXMLNode=templ->clone();
}

CEquipPack::~CEquipPack()
{
	m_pCreator=NULL;
	Release();
}

void CEquipPack::Release()
{
	if (m_pCreator)
	{
		if (!m_pCreator->RemoveEquipPack(m_sName))
			delete this;
	}
}

bool CEquipPack::AddEquip(const SEquipment &Equip)
{
	TLstEquipmentIt It=std::find(m_lstEquips.begin(), m_lstEquips.end(), Equip);
	if (It!=m_lstEquips.end())
		return false;
	m_lstEquips.push_back(Equip);
	return true;
}

bool CEquipPack::RemoveEquip(const SEquipment &Equip)
{
	TLstEquipmentIt It=std::find(m_lstEquips.begin(), m_lstEquips.end(), Equip);
	if (It==m_lstEquips.end())
		return false;
	m_lstEquips.erase(It);
	return true;
}

bool CEquipPack::AddAmmo(const SAmmo &Ammo)
{
	XmlNodeRef Node=m_AmmoXMLNode->findChild(Ammo.sName);
	if (Node)
		Node->setAttr("value", Ammo.nAmount);
	return true;
}

bool CEquipPack::SetPrimary(CString &sPrimary)
{
	if (sPrimary=="")
	{
		m_sPrimary="";
		return true;
	}
	SEquipment Equip;
	Equip.sName=sPrimary;
	TLstEquipmentIt It=std::find(m_lstEquips.begin(), m_lstEquips.end(), Equip);
	// return false if an equipment with that name doesnt exist
	if (It==m_lstEquips.end())
		return false;
	m_sPrimary=sPrimary;
	return true;
}

CString CEquipPack::GetPrimary()
{
	return m_sPrimary;
}

void CEquipPack::Clear()
{
	m_lstEquips.clear();
}

void CEquipPack::Load(XmlNodeRef Node)
{
	if (!Node->getAttr("primary", m_sPrimary))
		m_sPrimary="";
	for (int i=0;i<Node->getChildCount();i++)
	{
		XmlNodeRef Category=Node->getChild(i);
		if (strcmp(Category->getTag(), "Items")==0)
		{
			for (int i=0;i<Category->getChildCount();i++)
			{
				SEquipment Equip;
				XmlNodeRef Item=Category->getChild(i);
				Equip.sName=Item->getTag();
				if (!Item->getAttr("type", Equip.sType))
					Equip.sName="";
				if (!Item->getAttr("id", Equip.nId))
					Equip.nId=-1;
				AddEquip(Equip);
			}
		}
		if (strcmp(Category->getTag(), "Ammo")==0)
		{
			const char *key = "";
			const char *value = "";
			int numAttr = Category->getNumAttributes();
			for (int j = 0; j < numAttr; j++)
			{
				if (Category->getAttributeByIndex(j,&key,&value))
				{
					SAmmo Ammo;
					Ammo.sName = key;
					Ammo.nAmount = atoi(value);
					AddAmmo(Ammo);
				}
			}
		}
	}
}

bool CEquipPack::Save(XmlNodeRef RootNode)
{
	XmlNodeRef PackNode = RootNode->newChild("EquipPack");
	PackNode->setAttr("name", m_sName);
	if (m_sPrimary!="")
		PackNode->setAttr("primary", m_sPrimary);

	XmlNodeRef ItemsNode = PackNode->newChild("Items");
	for (TLstEquipmentIt It=m_lstEquips.begin();It!=m_lstEquips.end();++It)
	{
		SEquipment &Equip=(*It);
		XmlNodeRef EquipNode = ItemsNode->newChild(Equip.sName);
		EquipNode->setAttr("type", Equip.sType);
		if (Equip.nId!=-1)
			EquipNode->setAttr("id", Equip.nId);
	}
	XmlNodeRef AmmoNode = PackNode->newChild("Ammo");
	for (int i=0;i<m_AmmoXMLNode->getChildCount();i++)
	{
		XmlNodeRef &Ammo=m_AmmoXMLNode->getChild(i);
		CString sVal;
		Ammo->getAttr("value", sVal);
		AmmoNode->setAttr(Ammo->getTag(), sVal);
	}
	return true;
}