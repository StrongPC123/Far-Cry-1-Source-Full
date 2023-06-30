#include "StdAfx.h"
#include "equippacklib.h"
#include "equippack.h"

#include "IGame.h"

CEquipPackLib::CEquipPackLib()
{
}

CEquipPackLib::~CEquipPackLib()
{
	Reset();
}

CEquipPack* CEquipPackLib::CreateEquipPack(const CString &sName)
{
	TMapEquipPackIt It=m_mapEquipPacks.find(sName);
	if (It!=m_mapEquipPacks.end())
		return NULL;
	CEquipPack *pNew=new CEquipPack(this);
	if (!pNew)
		return NULL;
	pNew->SetName(sName);
	m_mapEquipPacks.insert(TMapEquipPackIt::value_type(sName, pNew));
	return pNew;
}

bool CEquipPackLib::RemoveEquipPack(const CString &sName)
{
	TMapEquipPackIt It=m_mapEquipPacks.find(sName);
	if (It==m_mapEquipPacks.end())
		return false;
	delete It->second;
	m_mapEquipPacks.erase(It);
	return true;
}

CEquipPack* CEquipPackLib::FindEquipPack(const CString &sName)
{
	TMapEquipPackIt It=m_mapEquipPacks.find(sName);
	if (It==m_mapEquipPacks.end())
		return NULL;
	return It->second;
}

bool CEquipPackLib::RenameEquipPack(const CString &sName, const CString &sNewName)
{
	TMapEquipPackIt It=m_mapEquipPacks.find(sName);
	if (It==m_mapEquipPacks.end())
		return false;
	CEquipPack *pPack=It->second;
	pPack->SetName(sNewName);
	m_mapEquipPacks.erase(It);
	m_mapEquipPacks.insert(TMapEquipPackIt::value_type(sNewName, pPack));
	return true;
}
/*
bool CEquipPackLib::LoadFromPath(CString sPath, CString sWildcard)
{
//	ILog *pLog=GetIEditor()->GetSystem()->GetILog();
//	pLog->Log("Loading EquipPacks from disk...");
	std::vector<CString> vecFiles;
	CFileEnum::ScanDirectory(sPath, sWildcard, vecFiles);
	XmlParser XMLParser;
	for (int i=0;i<vecFiles.size();i++)
	{
		XmlNodeRef EquipXMLNode=XMLParser.parse(sPath+vecFiles[i]);
		if (!EquipXMLNode)
			continue;
		for (int i=0;i<EquipXMLNode->getChildCount();i++)
		{
			XmlNodeRef Node=EquipXMLNode->getChild(i);
			if (Node->getTag()!="EquipPack")
				continue;
			CString sPackName;
			if (!Node->getAttr("name", sPackName))
				sPackName="Unnamed";
			CEquipPack *pCurrpack=CreateEquipPack(sPackName);
			if (!pCurrpack)
				continue;
//			pLog->Log("Loading EquipPack \"%s\" from file %s...", sPackName, sPath+vecFiles[i]);
			for (int i=0;i<Node->getChildCount();i++)
			{
				XmlNodeRef Category=Node->getChild(i);
				if (Category->getTag()=="Items")
				{
					for (int i=0;i<Category->getChildCount();i++)
					{
						SEquipment Equip;
						XmlNodeRef Item=Category->getChild(i);
						Equip.sName=Item->getTag();
						if (!Item->getAttr("type", Equip.sType))
							Equip.sName="";
						pCurrpack->AddEquip(Equip);
					}
				}
				if (Category->getTag()=="Ammo")
				{
					XmlAttributes AttrSet=Category->getAttributes();
					for (XmlAttributes::iterator It=AttrSet.begin();It!=AttrSet.end();++It)
					{
						XmlAttribute &Attr=(*It);
						SAmmo Ammo;
						Ammo.sName=Attr.key;
						Ammo.nAmount=atoi(Attr.value);
						pCurrpack->AddAmmo(Ammo);
					}
				}
			}
		}
	}
	return true;
}
*/
void CEquipPackLib::Serialize(XmlNodeRef &xmlNode, bool bLoading, bool bResetWhenLoad)
{
	if (!xmlNode)
		return;
	if (bLoading)
	{
		if (bResetWhenLoad)
			Reset();
		XmlNodeRef EquipXMLNode=xmlNode->findChild("EquipPacks");
		if (EquipXMLNode)
		{
			for (int i=0;i<EquipXMLNode->getChildCount();i++)
			{
				XmlNodeRef Node=EquipXMLNode->getChild(i);
				if (strcmp(Node->getTag(), "EquipPack"))
					continue;
				CString sPackName;
				if (!Node->getAttr("name", sPackName))
				{
					CLogFile::FormatLine("Warning: Unnamed EquipPack found !");
					sPackName="Unnamed";
				}
				CEquipPack *pCurrpack=CreateEquipPack(sPackName);
				if (!pCurrpack)
				{
					CLogFile::FormatLine("Warning: Unable to create EquipPack %s !", sPackName);
					continue;
				}
				pCurrpack->Load(Node);
			}

#ifndef _ISNOTFARCRY
				GetIXGame( GetIEditor()->GetGame() )->AddEquipPack( EquipXMLNode->getXML() );
#endif
			
		}
	}else
	{
		XmlNodeRef EquipXMLNode = xmlNode->newChild("EquipPacks");
		for (TMapEquipPackIt It=m_mapEquipPacks.begin();It!=m_mapEquipPacks.end();++It)
		{
			It->second->Save(EquipXMLNode);
		}
	}
}

void CEquipPackLib::Reset()
{
	// release all remaining instances
	for (TMapEquipPackIt It=m_mapEquipPacks.begin();It!=m_mapEquipPacks.end();++It)
	{
		delete It->second;
	}
	m_mapEquipPacks.clear();
}