// EquipPackDialog.cpp : implementation file
//

#include "stdafx.h"
#include "EquipPackDialog.h"
#include "EquipPackLib.h"
#include "EquipPack.h"
#include "StringDlg.h"

#include <IGame.h>

// CEquipPackDialog dialog

IMPLEMENT_DYNAMIC(CEquipPackDialog, CDialog)
CEquipPackDialog::CEquipPackDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CEquipPackDialog::IDD, pParent)
{
	m_sCurrEquipPack="";
}

CEquipPackDialog::~CEquipPackDialog()
{
}

void CEquipPackDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROPERTIES, m_AmmoPropWnd);
	DDX_Control(pDX, IDOK, m_OkBtn);
	DDX_Control(pDX, IDC_EXPORT, m_ExportBtn);
	DDX_Control(pDX, IDC_ADD, m_AddBtn);
	DDX_Control(pDX, IDC_DELETE, m_DeleteBtn);
	DDX_Control(pDX, IDC_RENAME, m_RenameBtn);
	DDX_Control(pDX, IDC_EQUIPPACK, m_EquipPacksList);
	DDX_Control(pDX, IDC_EQUIPAVAILLST, m_AvailEquipList);
	DDX_Control(pDX, IDC_EQUIPUSEDLST, m_EquipList);
	DDX_Control(pDX, IDC_INSERT, m_InsertBtn);
	DDX_Control(pDX, IDC_REMOVE, m_RemoveBtn);
}

void CEquipPackDialog::UpdateEquipPacksList()
{
	int nCurSel=m_EquipPacksList.GetCurSel();
	m_EquipPacksList.ResetContent();
	TMapEquipPack &mapEquipPacks=GetIEditor()->GetEquipPackLib()->Get();
	for (TMapEquipPackIt It=mapEquipPacks.begin();It!=mapEquipPacks.end();++It)
	{
		if (CString(It->first)==m_sCurrEquipPack)
			nCurSel=m_EquipPacksList.AddString(It->first);
		else
			m_EquipPacksList.AddString(It->first);
	}
	int nEquipPacks=GetIEditor()->GetEquipPackLib()->Count();
	if (!nEquipPacks)
		nCurSel=-1;
	else if (nEquipPacks<=nCurSel)
		nCurSel=0;
	else if (nCurSel==-1)
		nCurSel=0;
	m_EquipPacksList.SetCurSel(nCurSel);
	m_DeleteBtn.EnableWindow(nCurSel!=-1);
	m_RenameBtn.EnableWindow(nCurSel!=-1);
	UpdateEquipPackParams();
}

void CEquipPackDialog::UpdateEquipPackParams()
{
	m_AvailEquipList.ResetContent();
	m_EquipList.ResetContent();
	int nItem=m_EquipPacksList.GetCurSel();
	m_OkBtn.EnableWindow(nItem!=-1);
	m_ExportBtn.EnableWindow(nItem!=-1);
	if (nItem==-1)
	{
		m_sCurrEquipPack="";
		return;
	}
	m_EquipPacksList.GetLBText(nItem, m_sCurrEquipPack);
	CString sName;
	m_EquipPacksList.GetLBText(nItem, sName);
	CEquipPack *pEquip=GetIEditor()->GetEquipPackLib()->FindEquipPack(sName);
	if (!pEquip)
		return;
	TLstEquipment &EquipList=pEquip->GetEquip();
	CString sPrimary=pEquip->GetPrimary();
	int nPrimaryItemId=-1;
	for (TLstStringIt It=m_lstAvailEquip.begin();It!=m_lstAvailEquip.end();++It)
	{
		SEquipment &Equip=(*It);
		CString sType=Equip.sType;
		if (sType=="Weapon")
			sType="WPN";
		if (std::find(EquipList.begin(), EquipList.end(), Equip)==EquipList.end())
			m_AvailEquipList.AddString(sType+" - "+Equip.sName);
		else
		{
			int nIdx=m_EquipList.AddString(sType+" - "+Equip.sName);
			if (sPrimary==Equip.sName)
				nPrimaryItemId=nIdx;
		}
	}
	// ...and ammo
	m_AmmoPropWnd.CreateItems(pEquip->GetAmmo());
	m_AmmoPropWnd.SetUpdateCallback(functor(*this, &CEquipPackDialog::AmmoUpdateCallback));
	m_AmmoPropWnd.EnableUpdateCallback(true);
	// update lists
	m_AvailEquipList.SetCurSel(0);
	m_EquipList.SetCurSel(nPrimaryItemId);
	OnLbnSelchangeEquipavaillst();
	OnLbnSelchangeEquipusedlst();
}

void CEquipPackDialog::AmmoUpdateCallback(IVariable *pVar)
{
	m_bChanged=true;
}

SEquipment* CEquipPackDialog::GetEquipment(CString sDesc)
{
	SEquipment Equip;
	Equip.sName=sDesc.Right(sDesc.GetLength()-sDesc.Find("-")-2);//sDesc.Left(sDesc.Find("(")-1);
	TLstStringIt It=std::find(m_lstAvailEquip.begin(), m_lstAvailEquip.end(), Equip);
	if (It==m_lstAvailEquip.end())
		return NULL;
	return &(*It);
}

BEGIN_MESSAGE_MAP(CEquipPackDialog, CDialog)
	ON_CBN_SELCHANGE(IDC_EQUIPPACK, OnCbnSelchangeEquippack)
	ON_BN_CLICKED(IDC_ADD, OnBnClickedAdd)
//	ON_BN_CLICKED(IDC_REFRESH, OnBnClickedRefresh)
	ON_BN_CLICKED(IDC_DELETE, OnBnClickedDelete)
	ON_BN_CLICKED(IDC_RENAME, OnBnClickedRename)
	ON_BN_CLICKED(IDC_INSERT, OnBnClickedInsert)
	ON_BN_CLICKED(IDC_REMOVE, OnBnClickedRemove)
	ON_LBN_SELCHANGE(IDC_EQUIPAVAILLST, OnLbnSelchangeEquipavaillst)
	ON_LBN_SELCHANGE(IDC_EQUIPUSEDLST, OnLbnSelchangeEquipusedlst)
//	ON_WM_DESTROY()
ON_BN_CLICKED(IDC_IMPORT, OnBnClickedImport)
ON_BN_CLICKED(IDC_EXPORT, OnBnClickedExport)
ON_WM_DESTROY()
END_MESSAGE_MAP()


// CEquipPackDialog message handlers

BOOL CEquipPackDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

  m_bChanged=false;
	m_lstAvailEquip.clear();
	// Fill available weapons ListBox with enumerated weapons from game.
	INameIterator *weaponIter;

	weaponIter = NULL;
#ifndef _ISNOTFARCRY
		weaponIter = GetIXGame( GetIEditor()->GetGame() )->GetAvailableWeaponNames();
#endif

	if (weaponIter)
	{
		weaponIter->MoveFirst();
		do
		{
			char szName[1024];
			int size = sizeof(szName);
			if (weaponIter->Get(szName,&size))
			{
				SEquipment Equip;
				Equip.sName=szName;
				Equip.sType="Weapon";
				m_lstAvailEquip.push_back(Equip);
			}
		}while (weaponIter->MoveNext());
		weaponIter->Release();
	}else
		AfxMessageBox("GetAvailableWeaponNames() returned NULL. Unable to retrieve weapons.", MB_ICONEXCLAMATION | MB_OK);
	// get the rest from disk
	XmlParser XMLParser;
	XmlNodeRef EquipXMLNode=XMLParser.parse(CString(GetIEditor()->GetMasterCDFolder())+"Editor/Equip.xml");
	if (!EquipXMLNode)
		AfxMessageBox("Unable to find AvailableEquipment-Description !", MB_OK | MB_ICONEXCLAMATION);
	else
	{
		for (int i=0;i<EquipXMLNode->getChildCount();i++)
		{
			SEquipment Equip;
			XmlNodeRef Node=EquipXMLNode->getChild(i);
			Equip.sName=Node->getTag();
			if (!Node->getAttr("type", Equip.sType))
				Equip.sName="";
			m_lstAvailEquip.push_back(Equip);
		}
	}
	UpdateEquipPacksList();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CEquipPackDialog::OnCbnSelchangeEquippack()
{
	UpdateEquipPackParams();
}

void CEquipPackDialog::OnBnClickedAdd()
{
	CStringDlg Dlg("Enter name for new Equipment-Pack", this);
	if (Dlg.DoModal()!=IDOK)
		return;
	if (Dlg.GetString().GetLength()<=0)
		return;
	if (!GetIEditor()->GetEquipPackLib()->CreateEquipPack(Dlg.GetString()))
		AfxMessageBox("An Equipment-Pack with this name exist already !", MB_OK | MB_ICONEXCLAMATION);
	else
	{
		m_bChanged=true;
		UpdateEquipPacksList();
	}
}

void CEquipPackDialog::OnBnClickedDelete()
{
	if (AfxMessageBox("Are you sure ?", MB_YESNO | MB_ICONEXCLAMATION)==IDNO)
		return;
	CString sName;
	m_EquipPacksList.GetLBText(m_EquipPacksList.GetCurSel(), sName);
	if (!GetIEditor()->GetEquipPackLib()->RemoveEquipPack(sName))
		AfxMessageBox("Unable to delete Equipment-Pack !", MB_OK | MB_ICONEXCLAMATION);
	else
	{
		m_bChanged=true;
		UpdateEquipPacksList();
	}
}

void CEquipPackDialog::OnBnClickedRename()
{
	CString sName;
	m_EquipPacksList.GetLBText(m_EquipPacksList.GetCurSel(), sName);
	CStringDlg Dlg("Enter new name for Equipment-Pack", this);
	if (Dlg.DoModal()!=IDOK)
		return;
	if (Dlg.GetString().GetLength()<=0)
		return;
	if (!GetIEditor()->GetEquipPackLib()->RenameEquipPack(sName, Dlg.GetString()))
		AfxMessageBox("Unable to rename Equipment-Pack ! Probably the new name is already used.", MB_OK | MB_ICONEXCLAMATION);
	else
	{
		m_bChanged=true;
		UpdateEquipPacksList();
	}
}

void CEquipPackDialog::OnBnClickedInsert()
{
	int nItem=m_EquipPacksList.GetCurSel();
	if (nItem==-1)
		return;
	CString sName;
	m_EquipPacksList.GetLBText(nItem, sName);
	CEquipPack *pEquipPack=GetIEditor()->GetEquipPackLib()->FindEquipPack(sName);
	if (!pEquipPack)
		return;
	m_AvailEquipList.GetText(m_AvailEquipList.GetCurSel(), sName);
	SEquipment *pEquip=GetEquipment(sName);
	if (!pEquip)
		return;
	pEquipPack->AddEquip(*pEquip);
	m_bChanged=true;
	UpdateEquipPackParams();
}

void CEquipPackDialog::OnBnClickedRemove()
{
	int nItem=m_EquipPacksList.GetCurSel();
	if (nItem==-1)
		return;
	CString sName;
	m_EquipPacksList.GetLBText(nItem, sName);
	CEquipPack *pEquipPack=GetIEditor()->GetEquipPackLib()->FindEquipPack(sName);
	if (!pEquipPack)
		return;
	m_EquipList.GetText(m_EquipList.GetCurSel(), sName);
	SEquipment *pEquip=GetEquipment(sName);
	if (!pEquip)
		return;
	pEquipPack->RemoveEquip(*pEquip);
	m_bChanged=true;
	UpdateEquipPackParams();
}

void CEquipPackDialog::OnLbnSelchangeEquipavaillst()
{
	m_InsertBtn.EnableWindow(m_AvailEquipList.GetCount()!=0);
}

void CEquipPackDialog::OnLbnSelchangeEquipusedlst()
{
	int nCurSel=m_EquipList.GetCurSel();
	m_RemoveBtn.EnableWindow(nCurSel>=0);
	if (nCurSel>=0)
	{
		CString sName;
		int nItem=m_EquipPacksList.GetCurSel();
		m_EquipPacksList.GetLBText(nItem, sName);
		CString sPrimary;
		CEquipPack *pEquip=GetIEditor()->GetEquipPackLib()->FindEquipPack(sName);
		if (!pEquip)
			return;
		m_EquipList.GetText(nCurSel, sPrimary);
		sPrimary=sPrimary.Right(sPrimary.GetLength()-sPrimary.Find("-")-2);
		pEquip->SetPrimary(sPrimary);
	}
}

void CEquipPackDialog::OnBnClickedImport()
{
	CFileDialog Dlg(TRUE, "eqp", "", OFN_ENABLESIZING|OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR, "Equipment-Pack-Files (*.eqp)|*.eqp||");
	if (Dlg.DoModal()==IDOK)
	{
		if (Dlg.GetPathName().GetLength()>0)
		{
			XmlParser parser;
			XmlNodeRef root=parser.parse(Dlg.GetPathName());
			if (root)
			{
				GetIEditor()->GetEquipPackLib()->Serialize(root, true);
				m_bChanged=true;
				UpdateEquipPacksList();
			}
		}
	}
}

void CEquipPackDialog::OnBnClickedExport()
{
	CFileDialog Dlg(FALSE, "eqp", "", OFN_ENABLESIZING|OFN_EXPLORER|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR, "Equipment-Pack-Files (*.eqp)|*.eqp||");
	if (Dlg.DoModal()==IDOK)
	{
		if (Dlg.GetPathName().GetLength()>0)
		{
			XmlNodeRef root = new CXmlNode("EquipPackDB");
			GetIEditor()->GetEquipPackLib()->Serialize(root, false);
			root->saveToFile(Dlg.GetPathName());
		}
	}
}

void CEquipPackDialog::OnDestroy()
{
	CDialog::OnDestroy();
	if (m_bChanged)
	{
		XmlNodeRef XMLNode = new CXmlNode("Root");
		GetIEditor()->GetEquipPackLib()->Serialize(XMLNode, false);
		XmlNodeRef EquipXMLNode=XMLNode->findChild("EquipPacks");
		if (EquipXMLNode)
		{
#ifndef _ISNOTFARCRY
				GetIXGame( GetIEditor()->GetGame() )->AddEquipPack( EquipXMLNode->getXML() );
#endif
		}
 			
	}
}
