////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   propertyitem.cpp
//  Version:     v1.00
//  Created:     5/6/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "propertyitem.h"
#include "PropertyCtrl.h"
#include "InPlaceEdit.h"
#include "InPlaceComboBox.h"
#include "InPlaceButton.h"

#include "EquipPackDialog.h"
#include "SelectSoundPresetDlg.h"
#include "SelectEAXPresetDlg.h"

#include "ShaderEnum.h"
#include "ShadersDialog.h"

// AI
#include "AIDialog.h"
#include "AICharactersDialog.h"
#include "AIAnchorsDialog.h"
#include "AI\AIBehaviorLibrary.h"
#include "AI\AIGoalLibrary.h"

#include <ITimer.h>

//////////////////////////////////////////////////////////////////////////
#define CMD_ADD_CHILD_ITEM 100
#define CMD_ADD_ITEM 101
#define CMD_DELETE_ITEM 102

#define BUTTON_WIDTH 16

//////////////////////////////////////////////////////////////////////////
//! Undo object for Variable in property control..
class CUndoVariableChange : public IUndoObject
{
public:
	CUndoVariableChange( IVariable *var,const char *undoDescription )
	{
		// Stores the current state of this object.
		assert( var != 0 );
		m_undoDescription = undoDescription;
		m_var = var;
		m_undo = m_var->Clone(false);
	}
protected:
	virtual int GetSize() { 
		int size = sizeof(*this);
		//if (m_var)
			//size += m_var->GetSize();
		if (m_undo)
			size += m_undo->GetSize();
		if (m_redo)
			size += m_redo->GetSize();
		return size;
	}
	virtual const char* GetDescription() { return m_undoDescription; };
	virtual void Undo( bool bUndo )
	{
		if (bUndo)
		{
			m_redo = m_var->Clone(false);
		}
		m_var->CopyValue( m_undo );
	}
	virtual void Redo()
	{
		if (m_redo)
			m_var->CopyValue(m_redo);
	}

private:
	CString m_undoDescription;
	TSmartPtr<IVariable> m_undo;
	TSmartPtr<IVariable> m_redo;
	TSmartPtr<IVariable> m_var;
};

//////////////////////////////////////////////////////////////////////////
namespace {
struct {
	int dataType;
	const char *name;
	PropertyType type;
	int image;
} s_propertyTypeNames[] =
{
	{ IVariable::DT_SIMPLE,"Bool",ePropertyBool,2 },
	{ IVariable::DT_SIMPLE,"Int",ePropertyInt,0 },
	{ IVariable::DT_SIMPLE,"Float",ePropertyFloat,0 },
	{ IVariable::DT_SIMPLE,"Vector",ePropertyVector,10 },
	{ IVariable::DT_SIMPLE,"String",ePropertyString,3 },
	{ IVariable::DT_PERCENT,"Float",ePropertyInt,13 },
	{ IVariable::DT_COLOR,"Color",ePropertyColor,1 },
	{ IVariable::DT_ANGLE,"Angle",ePropertyAngle,0 },
	{ IVariable::DT_FILE,"File",ePropertyFile,7 },
	{ IVariable::DT_TEXTURE,"Texture",ePropertyTexture,4 },
	{ IVariable::DT_SOUND,"Sound",ePropertySound,6 },
	{ IVariable::DT_OBJECT,"Model",ePropertyModel,5 },
	{ IVariable::DT_SIMPLE,"Selection",ePropertySelection,-1 },
	{ IVariable::DT_SIMPLE,"List",ePropertyList,-1 },
	{ IVariable::DT_SHADER,"Shader",ePropertyShader,9 },
	{ IVariable::DT_MATERIAL,"Material",ePropertyMaterial,9 },
	{ IVariable::DT_AI_BEHAVIOR,"AIBehavior",ePropertyAiBehavior,8 },
	{ IVariable::DT_AI_ANCHOR,"AIAnchor",ePropertyAiAnchor,8 },
	{ IVariable::DT_AI_CHARACTER,"AICharacter",ePropertyAiCharacter,8 },
	{ IVariable::DT_EQUIP,"Equip",ePropertyEquip,11 },
	{ IVariable::DT_SOUNDPRESET,"SoundPreset",ePropertySoundPreset,11 },
	{ IVariable::DT_EAXPRESET,"EAXPreset",ePropertyEAXPreset,11 },
	{ IVariable::DT_LOCAL_STRING,"LocalString",ePropertyLocalString,3 },
};
static int NumPropertyTypes = sizeof(s_propertyTypeNames)/sizeof(s_propertyTypeNames[0]);

const char* DISPLAY_NAME_ATTR	= "DisplayName";
const char* VALUE_ATTR = "Value";
const char* TYPE_ATTR	=	"Type";
const char* TIP_ATTR	= "Tip";
const char* FILEFILTER_ATTR = "FileFilters";
};

//////////////////////////////////////////////////////////////////////////
// CPropertyItem implementation.
//////////////////////////////////////////////////////////////////////////
CPropertyItem::CPropertyItem( CPropertyCtrl* pCtrl )
{
	m_propertyCtrl = pCtrl;
	m_parent = 0;
	m_bExpandable = false;
	m_bExpanded = false;
	m_bSelected = false;
	m_bNoCategory = false;

	m_cNumber = 0;
	m_cEdit = 0;
	m_cCombo = 0;
	m_cButton = 0;
	m_image = -1;
	m_bIgnoreChildsUpdate = false;
	m_value = "";
	m_type = ePropertyInvalid;
	
	m_modified = false;
	m_lastModified = 0;
	m_valueMultiplier = 1;
}

CPropertyItem::~CPropertyItem()
{
	if (m_pVariable)
		ReleaseVariable();
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::SetXmlNode( XmlNodeRef &node )
{
	m_node = node;
	// No Undo while just creating properties.
	//GetIEditor()->SuspendUndo();
	ParseXmlNode();
	//GetIEditor()->ResumeUndo();
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::ParseXmlNode( bool bRecursive/* =true  */)
{
	if (!m_node->getAttr( DISPLAY_NAME_ATTR,m_name ))
	{
		m_name = m_node->getTag();
	}

	CString value;
	bool bHasValue=m_node->getAttr( VALUE_ATTR,value );

	CString type;
	m_node->getAttr( TYPE_ATTR,type);

	m_tip = "";
	m_node->getAttr( TIP_ATTR,m_tip );

	m_image = -1;
	m_type = ePropertyInvalid;
	for (int i = 0; i < NumPropertyTypes; i++)
	{
		if (stricmp(type,s_propertyTypeNames[i].name) == 0)
		{
			m_type = s_propertyTypeNames[i].type;
			m_image = s_propertyTypeNames[i].image;
			break;
		}
	}

	m_rangeMin = -10000;
	m_rangeMax = 10000;
	m_iInternalPrecision=2;																			// m.m.
	if (m_type == ePropertyFloat || m_type == ePropertyInt)
	{
		m_node->getAttr( "Min",m_rangeMin );
		m_node->getAttr( "Max",m_rangeMax );
		m_node->getAttr( "Precision",m_iInternalPrecision );			// m.m.
	}

	if (bHasValue)
		SetValue( value );

	m_bNoCategory = false;

	if (m_type == ePropertyVector)
	{
		bRecursive = false;

		m_childs.clear();
		Vec3 vec;
		m_node->getAttr( VALUE_ATTR,vec );
		// Create 3 sub elements.
		XmlNodeRef x = m_node->createNode( "X" );
		XmlNodeRef y = m_node->createNode( "Y" );
		XmlNodeRef z = m_node->createNode( "Z" );
		x->setAttr( TYPE_ATTR,"Float" );
		y->setAttr( TYPE_ATTR,"Float" );
		z->setAttr( TYPE_ATTR,"Float" );

		x->setAttr( VALUE_ATTR,vec.x );
		y->setAttr( VALUE_ATTR,vec.y );
		z->setAttr( VALUE_ATTR,vec.z );

		// Start ignoring all updates comming from childs. (Initializing childs).
		m_bIgnoreChildsUpdate = true;

		CPropertyItem *itemX = new CPropertyItem( m_propertyCtrl );
		itemX->SetXmlNode( x );
		AddChild( itemX );

		CPropertyItem *itemY = new CPropertyItem( m_propertyCtrl );
		itemY->SetXmlNode( y );
		AddChild( itemY );

		CPropertyItem *itemZ = new CPropertyItem( m_propertyCtrl );
		itemZ->SetXmlNode( z );
		AddChild( itemZ );
		
		m_bNoCategory = true;
		m_bExpandable = true;

		m_bIgnoreChildsUpdate = false;
	}
	else if (bRecursive)
	{
		// If recursive and not vector.

		m_bExpandable = false;
		// Create sub-nodes.
		for (i=0; i < m_node->getChildCount(); i++)
		{
			m_bIgnoreChildsUpdate = true;

			XmlNodeRef child = m_node->getChild(i);
			CPropertyItem *item = new CPropertyItem( m_propertyCtrl );
			item->SetXmlNode( m_node->getChild(i) );
			AddChild( item );
			m_bExpandable = true;

			m_bIgnoreChildsUpdate = false;
		}
	}
	m_modified = false;
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::SetVariable( IVariable *var )
{
	// Release previous variable.
	if (m_pVariable)
		ReleaseVariable();

	m_pVariable = var;
	assert( m_pVariable != NULL );

	m_pVariable->AddOnSetCallback( functor(*this,&CPropertyItem::OnVariableChange) );

	m_tip = "";
	m_name = m_pVariable->GetHumanName();

	int dataType = m_pVariable->GetDataType();

	m_image = -1;
	m_type = ePropertyInvalid;
	int i;

	if (dataType != IVariable::DT_SIMPLE)
	{
		for (i = 0; i < NumPropertyTypes; i++)
		{
			if (dataType == s_propertyTypeNames[i].dataType)
			{
				m_type = s_propertyTypeNames[i].type;
				m_image = s_propertyTypeNames[i].image;
				break;
			}
		}
	}

	m_enumList = m_pVariable->GetEnumList();
	if (m_enumList)
	{
		m_type = ePropertySelection;
	}

	if (m_type == ePropertyInvalid)
	{
		switch(m_pVariable->GetType()) 
		{
		case IVariable::INT:
			m_type = ePropertyInt;
			break;
		case IVariable::BOOL:
			m_type = ePropertyBool;
			break;
		case IVariable::FLOAT:
			m_type = ePropertyFloat;
			break;
		case IVariable::VECTOR:
			m_type = ePropertyVector;
			break;
		case IVariable::STRING:
			m_type = ePropertyString;
			break;
		}
		for (i = 0; i < NumPropertyTypes; i++)
		{
			if (m_type == s_propertyTypeNames[i].type)
			{
				m_image = s_propertyTypeNames[i].image;
				break;
			}
		}
	}

	m_valueMultiplier = 1;
	m_rangeMin = -10000;
	m_rangeMax = 10000;
	m_iInternalPrecision=2;  // m.m.

	// Get variable limits.
	m_pVariable->GetLimits( m_rangeMin,m_rangeMax );
	
	// Check if value is percsents.
	if (dataType == IVariable::DT_PERCENT)
	{
		// Scale all values by 100.
		m_valueMultiplier = 100;
		m_rangeMin = 0;
		m_rangeMax = 100;
	}

	//////////////////////////////////////////////////////////////////////////
	CString value = VarToValue(var);

	m_value = value;
	SetValue( value );

	m_bNoCategory = false;

	if (m_type == ePropertyVector)
	{
		m_childs.clear();

		Vec3 vec;
		m_pVariable->Get( vec );
		IVariable *pVX = new CVariable<float>;
		pVX->SetName( "x" );
		pVX->Set( vec.x );
		IVariable *pVY = new CVariable<float>;
		pVY->SetName( "y" );
		pVY->Set( vec.y );
		IVariable *pVZ = new CVariable<float>;
		pVZ->SetName( "z" );
		pVZ->Set( vec.z );

		// Start ignoring all updates comming from childs. (Initializing childs).
		m_bIgnoreChildsUpdate = true;

		CPropertyItem *itemX = new CPropertyItem( m_propertyCtrl );
		itemX->SetVariable( pVX );
		AddChild( itemX );

		CPropertyItem *itemY = new CPropertyItem( m_propertyCtrl );
		itemY->SetVariable( pVY );
		AddChild( itemY );

		CPropertyItem *itemZ = new CPropertyItem( m_propertyCtrl );
		itemZ->SetVariable( pVZ );
		AddChild( itemZ );

		m_bNoCategory = true;
		m_bExpandable = true;

		m_bIgnoreChildsUpdate = false;
	}
	/*
	else if (bRecursive)
	{
		// If recursive and not vector.

		m_bExpandable = false;
		// Create sub-nodes.
		for (i=0; i < m_node->getChildCount(); i++)
		{
			m_bIgnoreChildsUpdate = true;

			XmlNodeRef child = m_node->getChild(i);
			CPropertyItem *item = new CPropertyItem( this,m_propertyCtrl );
			item->SetXmlNode( m_node->getChild(i) );
			m_childs.push_back( item );
			m_bExpandable = true;

			m_bIgnoreChildsUpdate = false;
		}
	}
	*/

	if (m_pVariable->NumChildVars() > 0)
	{
		if (m_type == ePropertyInvalid)
			m_type = ePropertyTable;
		m_bIgnoreChildsUpdate = true;
		for (i = 0; i < m_pVariable->NumChildVars(); i++)
		{
			CPropertyItem *item = new CPropertyItem( m_propertyCtrl );
			item->SetVariable(m_pVariable->GetChildVar(i));
			AddChild( item );
			m_bExpandable = true;
		}
		m_bIgnoreChildsUpdate = false;
	}
	m_modified = false;
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::ReleaseVariable()
{
	if (m_pVariable)
	{
		// Unwire all from variable.
		m_pVariable->Unwire(0);
		m_pVariable->RemoveOnSetCallback(functor(*this, &CPropertyItem::OnVariableChange));
	}
}

//////////////////////////////////////////////////////////////////////////
//! Find item that reference specified property.
CPropertyItem* CPropertyItem::FindItemByVar( IVariable *pVar )
{
	if (m_pVariable == pVar)
		return this;
	for (int i = 0; i < m_childs.size(); i++)
	{
		CPropertyItem* pFound = m_childs[i]->FindItemByVar(pVar);
		if (pFound)
			return pFound;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CString CPropertyItem::GetFullName() const
{
	if (m_parent)
	{
		return m_parent->GetFullName() + "::" + m_name;
	}
	else
		return m_name;
}

//////////////////////////////////////////////////////////////////////////
CPropertyItem* CPropertyItem::FindItemByFullName( const CString &name )
{
	if (GetFullName() == name)
		return this;
	for (int i = 0; i < m_childs.size(); i++)
	{
		CPropertyItem* pFound = m_childs[i]->FindItemByFullName(name);
		if (pFound)
			return pFound;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::AddChild( CPropertyItem *item )
{
	assert(item);
	m_bExpandable = true;
	item->m_parent = this;
	m_childs.push_back(item);
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::RemoveChild( CPropertyItem *item )
{
	// Find item and erase it from childs array.
	for (int i = 0; i < m_childs.size(); i++)
	{
		if (item == m_childs[i])
		{
			m_childs.erase( m_childs.begin() + i );
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnChildChanged( CPropertyItem *child )
{
	if (m_bIgnoreChildsUpdate)
		return;

	if (m_type == ePropertyVector)
	{
		assert( m_childs.size() == 3 );
		// Get values from childs.
		CString x = m_childs[0]->GetValue();
		CString y = m_childs[1]->GetValue();
		CString z = m_childs[2]->GetValue();
		bool prevIgnore = m_bIgnoreChildsUpdate;
		m_bIgnoreChildsUpdate = true;
		SetValue( x+","+y+","+z,false );
		m_bIgnoreChildsUpdate = prevIgnore;
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::SetSelected( bool selected )
{
	m_bSelected = selected;
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::SetExpanded( bool expanded )
{
	if (IsDisabled())
	{
		m_bExpanded = false;
	}
	else
		m_bExpanded = expanded;
}

bool CPropertyItem::IsDisabled() const
{
	if (m_pVariable)
	{
		return m_pVariable->GetFlags() & IVariable::UI_DISABLED;
	}
	return false;
}

bool CPropertyItem::IsBold() const
{
	if (m_pVariable)
	{
		return m_pVariable->GetFlags() & IVariable::UI_BOLD;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::CreateInPlaceControl( CWnd* pWndParent,CRect& ctrlRect )
{
	CRect nullRc(0,0,0,0);
	switch (m_type)
	{
		case ePropertyFloat:
		case ePropertyInt:
		case ePropertyAngle:
			{
				m_cNumber = new CNumberCtrl;
				m_cNumber->SetUpdateCallback( functor(*this, &CPropertyItem::OnNumberCtrlUpdate) );
				//m_cNumber->SetBeginUpdateCallback( functor(*this, &CPropertyItem::OnNumberCtrlBeginUpdate) );
				//m_cNumber->SetEndUpdateCallback( functor(*this, &CPropertyItem::OnNumberCtrlEndUpdate) );

				// (digits behind the comma, only used for floats)
				if(m_type == ePropertyFloat)
					m_cNumber->SetInternalPrecision( m_iInternalPrecision );

				// Only for integers.
				if (m_type == ePropertyInt)
					m_cNumber->SetInteger(true);
				m_cNumber->SetMultiplier( m_valueMultiplier );
				m_cNumber->SetRange( m_rangeMin,m_rangeMax );
				
				//m_cNumber->Create( pWndParent,rect,1,CNumberCtrl::LEFTARROW|CNumberCtrl::NOBORDER );
				m_cNumber->Create( pWndParent,nullRc,1,CNumberCtrl::NOBORDER|CNumberCtrl::LEFTALIGN );
				m_cNumber->SetLeftAlign( true );
			}
			break;

		case ePropertyString:
		case ePropertyVector:
		case ePropertyLocalString:
			m_cEdit = new CInPlaceEdit( m_value,functor(*this, &CPropertyItem::OnEditChanged) );
			m_cEdit->Create( WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|ES_LEFT, nullRc, pWndParent,2 );
			break;

		case ePropertyShader:
		case ePropertyMaterial:
		case ePropertyAiBehavior:
		case ePropertyAiAnchor:
		case ePropertyAiCharacter:
		case ePropertyEquip:
		case ePropertySoundPreset:
		case ePropertyEAXPreset:
			m_cEdit = new CInPlaceEdit( m_value,functor(*this, &CPropertyItem::OnEditChanged) );
			m_cEdit->Create( WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|ES_LEFT, nullRc, pWndParent,2 );
			if (m_type == ePropertyShader)
			{
				m_cButton = new CInPlaceButton( functor(*this, &CPropertyItem::OnShaderBrowseButton) );
				m_cButton->Create( "...",WS_CHILD|WS_VISIBLE,nullRc,pWndParent,4 );
			}
			if (m_type == ePropertyAiBehavior)
			{
				m_cButton = new CInPlaceButton( functor(*this, &CPropertyItem::OnAIBehaviorBrowseButton) );
				m_cButton->Create( "...",WS_CHILD|WS_VISIBLE,nullRc,pWndParent,4 );
			}
			if (m_type == ePropertyAiAnchor)
			{
				m_cButton = new CInPlaceButton( functor(*this, &CPropertyItem::OnAIAnchorBrowseButton) );
				m_cButton->Create( "...",WS_CHILD|WS_VISIBLE,nullRc,pWndParent,4 );
			}
			if (m_type == ePropertyAiCharacter)
			{
				m_cButton = new CInPlaceButton( functor(*this, &CPropertyItem::OnAICharacterBrowseButton) );
				m_cButton->Create( "...",WS_CHILD|WS_VISIBLE,nullRc,pWndParent,4 );
			}
			if (m_type == ePropertyEquip)
			{
				m_cButton = new CInPlaceButton( functor(*this, &CPropertyItem::OnEquipBrowseButton) );
				m_cButton->Create( "...",WS_CHILD|WS_VISIBLE,nullRc,pWndParent,4 );
			}
			if (m_type == ePropertySoundPreset)
			{
				m_cButton = new CInPlaceButton( functor(*this, &CPropertyItem::OnSoundPresetBrowseButton) );
				m_cButton->Create( "...",WS_CHILD|WS_VISIBLE,nullRc,pWndParent,4 );
			}
			if (m_type == ePropertyEAXPreset)
			{
				m_cButton = new CInPlaceButton( functor(*this, &CPropertyItem::OnEAXPresetBrowseButton) );
				m_cButton->Create( "...",WS_CHILD|WS_VISIBLE,nullRc,pWndParent,4 );
			}
			else if (m_type == ePropertyMaterial)
			{
				m_cButton = new CInPlaceButton( functor(*this, &CPropertyItem::OnMaterialBrowseButton) );
				m_cButton->Create( "...",WS_CHILD|WS_VISIBLE,nullRc,pWndParent,4 );
			}
			if (m_cButton)
			{
				m_cButton->SetXButtonStyle( BS_XT_SEMIFLAT,FALSE );
			}
			break;

		//case ePropertyBool:
		case ePropertySelection:
			{
				// Combo box.
				m_cCombo = new CInPlaceComboBox();
				m_cCombo->SetReadOnly( true );
				m_cCombo->Create( NULL,"",WS_CHILD|WS_VISIBLE, nullRc, pWndParent,2 );
				m_cCombo->SetUpdateCallback( functor(*this, &CPropertyItem::OnComboSelection) );

				if (m_type == ePropertySelection)
				{
					if (m_enumList)
					{
						for (int i = 0; i < m_enumList->GetItemsCount(); i++)
						{
							m_cCombo->AddString( m_enumList->GetItemName(i) );
						}
					}

					/*
					CString sel;
					m_node->getAttr( "Selection",sel );
					if (!sel.IsEmpty())
					{
						char *token;
						char str[1024];
						strcpy( str,sel );
						token = strtok( str,"," );
						while (token != NULL)
						{
							m_cCombo->AddString( token );
							token = strtok( NULL,"," );
						}
					}
					*/
				} else if (m_type == ePropertyBool)
				{
					m_cCombo->AddString( "True" );
					m_cCombo->AddString( "False" );
					if (GetBoolValue())
						m_cCombo->SetCurSel(0,false);
					else
						m_cCombo->SetCurSel(1,false);
				}
			}
			break;

		case ePropertyColor:
			{
				m_cEdit = new CInPlaceEdit( m_value,functor(*this, &CPropertyItem::OnEditChanged) );
				m_cEdit->Create( WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|ES_LEFT, nullRc, pWndParent,2 );
				m_cButton = new CInPlaceButton( functor(*this, &CPropertyItem::OnColorBrowseButton) );
				
				m_cButton->Create( "",WS_CHILD|WS_VISIBLE,nullRc,pWndParent,4 );
				m_cButton->SetXButtonStyle( BS_XT_SEMIFLAT,FALSE );
			}
			break;

		case ePropertyFile:
		case ePropertyTexture:
		case ePropertyModel:
		case ePropertySound:
			m_cEdit = new CInPlaceEdit( m_value,functor(*this, &CPropertyItem::OnEditChanged) );
			m_cEdit->Create( WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|ES_LEFT, nullRc, pWndParent,2 );

			m_cButton = new CInPlaceButton( functor(*this, &CPropertyItem::OnFileBrowseButton) );
			m_cButton->Create( "...",WS_CHILD|WS_VISIBLE,nullRc,pWndParent,4 );

			// Use file browse icon.
			m_cButton->SetXButtonStyle( BS_XT_SEMIFLAT,FALSE );
			m_cButton->SetIcon( CSize(16,15),IDI_FILE_BROWSE );
			m_cButton->SetBorderGap(0);
			break;
/*
		case ePropertyList:
			{
				AddButton( "Add", CWDelegate(this,(TDelegate)OnAddItemButton) );
			}
			break;
*/
	}
	
	MoveInPlaceControl( ctrlRect );
	SendToControl();
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::DestroyInPlaceControl()
{
	ReceiveFromControl();

	if (m_cNumber)
	{
		delete m_cNumber;
		m_cNumber = 0;
	}
	if (m_cEdit)
	{
		delete m_cEdit;
		m_cEdit = 0;
	}
	if (m_cCombo)
	{
		delete m_cCombo;
		m_cCombo = 0;
	}
	if (m_cButton)
	{
		delete m_cButton;
		m_cButton = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::MoveInPlaceControl( const CRect& rect )
{
	CRect rc = rect;
	if (m_cButton)
	{
		if (m_type == ePropertyColor)
		{
			rc.right = rc.left + BUTTON_WIDTH + 2;
			m_cButton->MoveWindow( rc,FALSE );
			rc = rect;
			rc.left += BUTTON_WIDTH + 2 + 4;
		}
		else
		{
			rc.left = rc.right - BUTTON_WIDTH;
			m_cButton->MoveWindow( rc,FALSE );
			rc = rect;
			rc.right -= BUTTON_WIDTH;
		}
		m_cButton->SetFont(m_propertyCtrl->GetFont());
	}

	if (m_cNumber)
	{
		CRect rcn = rc;
		//rcn.right = rc.left + 60;
		if (rcn.Width() > 60)
			rcn.right = rc.left + 60;
		m_cNumber->MoveWindow( rcn,FALSE );
		m_cNumber->SetFont(m_propertyCtrl->GetFont());
	}
	if (m_cEdit)
	{
		m_cEdit->MoveWindow( rc,FALSE );
		m_cEdit->SetFont(m_propertyCtrl->GetFont());
	}
	if (m_cCombo)
	{
		m_cCombo->MoveWindow( rc,FALSE );
		m_cCombo->SetFont(m_propertyCtrl->GetFont());
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::SetFocus()
{
	if (m_cNumber)
	{
		m_cNumber->SetFocus();
	}
	if (m_cEdit)
	{
		m_cEdit->SetFocus();
	}
	if (m_cCombo)
	{
		m_cCombo->SetFocus();
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::ReceiveFromControl()
{
	if (m_cEdit)
	{
		CString str;
		m_cEdit->GetWindowText( str );
		SetValue( str );
	}
	if (m_cCombo)
	{
		SetValue( m_cCombo->GetSelectedString() );
	}
	if (m_cNumber)
	{
		SetValue( m_cNumber->GetValueAsString() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::SendToControl()
{
	bool bInPlaceCtrl = false;
	if (m_cButton)
	{
		if (m_type == ePropertyColor)
		{
			COLORREF clr = StringToColor(m_value);
			m_cButton->SetColorFace( clr );
			bInPlaceCtrl = true;
		}
	}
	if (m_cEdit)
	{
		m_cEdit->SetText(m_value);
		bInPlaceCtrl = true;
	}
	if (m_cCombo)
	{
		if (m_type == ePropertyBool)
			m_cCombo->SetCurSel( GetBoolValue()?0:1,false );
		else
			m_cCombo->SelectString(m_value);
		bInPlaceCtrl = true;
	}
	if (m_cNumber)
	{
		m_cNumber->SetValue( atof(m_value) );
		bInPlaceCtrl = true;
	}
	if (!bInPlaceCtrl)
		m_propertyCtrl->Invalidate();
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnComboSelection()
{
	ReceiveFromControl();
	SendToControl();
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::DrawValue( CDC *dc,CRect rect )
{
		// Setup text.
	dc->SetBkMode( TRANSPARENT );
	
	static char str[4096];
	strcpy( str,m_value );
	if (m_valueMultiplier != 1)
	{
		sprintf( str,"%g",atof(m_value)*m_valueMultiplier );
	}
	if (m_type == ePropertyList)
	{
		//sprintf( str,"%d items",(int)Children.Num() );
		return;
	}
	else if (m_type == ePropertyBool)
	{
		CPoint p1( rect.left,rect.top+1 );
		int sz = rect.bottom - rect.top - 1;
		CRect rc(p1.x,p1.y,p1.x+sz,p1.y+sz );
		dc->DrawFrameControl( rc,DFC_BUTTON,DFCS_BUTTON3STATE );
		if (GetBoolValue())
		{
			strcpy( str,"True" );
			m_propertyCtrl->m_icons.Draw( dc,12,p1,ILD_TRANSPARENT );
		}
		else
		{
			strcpy( str,"False" );
		}
		CRect textRc;
		textRc = rect;
		textRc.left += 15;
		::DrawTextEx( dc->GetSafeHdc(),str, strlen(str), textRc, DT_END_ELLIPSIS|DT_LEFT|DT_SINGLELINE|DT_VCENTER, NULL );
		strcpy( str,"" );
	}
	else if (m_type == ePropertyFile || m_type == ePropertyTexture || 
						m_type == ePropertySound || m_type == ePropertyModel)
	{
		// Any file.
		// Check if file name fits into the designated rectangle.
		CSize textSize = dc->GetTextExtent( str,strlen(str) );
		if (textSize.cx > rect.Width())
		{
			// Cut file name...
			CString file = Path::GetFile(str);
			strcpy( str,"...\\" );
			strcat( str,file );
		}
	}

	if (m_type == ePropertyColor)
	{
		//CRect rc( CPoint(rect.right-BUTTON_WIDTH,rect.top),CSize(BUTTON_WIDTH,rect.bottom-rect.top) );
		CRect rc( CPoint(rect.left,rect.top+1),CSize(BUTTON_WIDTH+2,rect.bottom-rect.top-2) );
		//CPen pen( PS_SOLID,1,RGB(128,128,128));
		CPen pen( PS_SOLID,1,RGB(0,0,0));
		CBrush brush( StringToColor(m_value) );
		CPen *pOldPen = dc->SelectObject( &pen );
		CBrush *pOldBrush = dc->SelectObject( &brush );
		dc->Rectangle( rc );
		//COLORREF col = StringToColor(m_value);
		//rc.DeflateRect( 1,1 );
		//dc->FillSolidRect( rc,col );
		dc->SelectObject( pOldPen );
		dc->SelectObject( pOldBrush );
		rect.left = rect.left + BUTTON_WIDTH + 2 + 4;
	}

	CRect textRc;
	textRc = rect;
	::DrawTextEx( dc->GetSafeHdc(),str, strlen(str), textRc, DT_END_ELLIPSIS|DT_LEFT|DT_SINGLELINE|DT_VCENTER, NULL );
}

COLORREF CPropertyItem::StringToColor( const CString &value )
{
	float r,g,b;
	int res = 0;
	if (res != 3)
		res = sscanf( value,"%f,%f,%f",&r,&g,&b );
	if (res != 3)
		res = sscanf( value,"R:%f,G:%f,B:%f",&r,&g,&b );
	if (res != 3)
		res = sscanf( value,"R:%f G:%f B:%f",&r,&g,&b );
	if (res != 3)
		res = sscanf( value,"%f %f %f",&r,&g,&b );
	if (res != 3)
	{
		sscanf( value,"%f",&r );
		return r;
	}
	int ir = r;
	int ig = g;
	int ib = b;

	return RGB(ir,ig,ib);
}

bool CPropertyItem::GetBoolValue()
{
	if (stricmp(m_value,"true")==0 || atoi(m_value) != 0)
		return true;
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////
const char* CPropertyItem::GetValue() const
{
	return m_value;
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::SetValue( const char* sValue,bool bRecordUndo )
{
	CString value = sValue;
	if (m_type == ePropertyBool)
	{
		if (stricmp(value,"true")==0 || atof(value) != 0)
			value = "1";
		else
			value = "0";
	}

	bool bModified = false;
	if (m_value.Compare(value) != 0)
	{
		bModified = true;
	}
	m_value = value;

	if (m_pVariable)
	{
		if (bModified)
		{
			float currTime = GetIEditor()->GetSystem()->GetITimer()->GetCurrTime();
			if (currTime > m_lastModified+1) // At least one second between undo stores.
			{
				if (bRecordUndo && !CUndo::IsRecording())
				{
					CUndo undo( CString(m_pVariable->GetName()) + " Changed" );
					undo.Record( new CUndoVariableChange(m_pVariable,"PropertyChange") );
				}
				else if (bRecordUndo)
				{
					GetIEditor()->RecordUndo( new CUndoVariableChange(m_pVariable,"PropertyChange") );
				}
			}

			ValueToVar( m_value,m_pVariable );
			m_lastModified = currTime;
		}
		// Return now, (OnVariableChange was called).
		return;
	}
	

	//////////////////////////////////////////////////////////////////////////
	// DEPRICATED (For XmlNode).
	//////////////////////////////////////////////////////////////////////////
	if (m_node)
		m_node->setAttr( VALUE_ATTR,m_value );
	//CString xml = m_node->getXML();

	if (bModified)
	{
		m_modified = true;
		if (m_type == ePropertyVector && m_childs.size() == 3)
		{
			Vec3 vec;
			if (m_node)
				m_node->getAttr( VALUE_ATTR,vec );
			if (m_pVariable)
				m_pVariable->Get(vec);
			// Get values from childs.
			char str[1024];
			sprintf( str,"%g",vec.x );
			m_childs[0]->SetValue(str,false);
			sprintf( str,"%g",vec.y );
			m_childs[1]->SetValue(str,false);
			sprintf( str,"%g",vec.z );
			m_childs[2]->SetValue(str,false);
		}

		if (m_parent)
			m_parent->OnChildChanged( this );
		// If Value changed mark document modified.
		// Notify parent that this Item have been modified.
		m_propertyCtrl->OnItemChange( this );
	}

	SendToControl();
}

//////////////////////////////////////////////////////////////////////////
CString CPropertyItem::VarToValue( IVariable *var )
{
	assert( var );

	if (m_enumList)
	{
		return m_enumList->GetNameFromVariableValue(var);
	}

	CString value;
	if (m_type == ePropertyColor)
	{
		Vec3 v;
		m_pVariable->Get(v);
		int r = v.x*255;
		int g = v.y*255;
		int b = v.z*255;
		r = max(0,min(r,255));
		g = max(0,min(g,255));
		b = max(0,min(b,255));
    value.Format( "%d,%d,%d",r,g,b );
		return value;
	}

	m_pVariable->Get(value);

	if (m_type == ePropertyAngle)
	{
		// Convert from radians to degrees.
		float f;
		m_pVariable->Get(f);
		f = RAD2DEG(f);
		char str[32];
		sprintf( str,"%g",f );
		value = str;
	}

	return value;
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::ValueToVar( const CString &value,IVariable *var )
{
	assert( m_pVariable != NULL );

	if (m_enumList)
	{
		m_enumList->SetVariableValue( var,value );
		return;
	}

	if (m_type == ePropertyAngle)
	{
		// Convert from degrees to radians.
		float f = (float)atof(m_value);
		m_pVariable->Set(DEG2RAD(f));
	}
	else if (m_type == ePropertyColor)
	{
		COLORREF col = StringToColor(value);
		Vec3 v;
		v.x = GetRValue(col)/255.0f;
		v.y = GetGValue(col)/255.0f;
		v.z = GetBValue(col)/255.0f;
		m_pVariable->Set(v);
	}
	else
	{
		// Any other variable type.
		if (m_type != ePropertyInvalid && m_type != ePropertyTable)
			m_pVariable->Set(m_value);
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnVariableChange( IVariable *var )
{
	// When variable change, invalidate UI.
	m_modified = true;

	m_value = VarToValue(var);

	if (m_type == ePropertyVector && m_childs.size() == 3)
	{
		Vec3 vec;
		m_pVariable->Get(vec);
		// Get values from childs.
		char str[1024];
		sprintf( str,"%g",vec.x );
		m_childs[0]->SetValue(str,false);
		sprintf( str,"%g",vec.y );
		m_childs[1]->SetValue(str,false);
		sprintf( str,"%g",vec.z );
		m_childs[2]->SetValue(str,false);
	}

	SendToControl();
	if (m_parent)
		m_parent->OnChildChanged( this );
	
	// If Value changed mark document modified.
	// Notify parent that this Item have been modified.
	// This may delete this control...
	m_propertyCtrl->OnItemChange( this );
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnMouseWheel( UINT nFlags,short zDelta,CPoint point )
{
	if (m_cCombo)
	{
		int sel = m_cCombo->GetCurSel();
		if (zDelta > 0)
		{
			sel++;
			if (m_cCombo->SetCurSel( sel ) == CB_ERR)
				m_cCombo->SetCurSel( 0 );
		}
		else
		{
			sel--;
			if (m_cCombo->SetCurSel( sel ) == CB_ERR)
				m_cCombo->SetCurSel( m_cCombo->GetCount()-1 );
		}
	}
	else if (m_cNumber)
	{
		if (zDelta > 0)
		{
			m_cNumber->SetValue( m_cNumber->GetValue() + m_cNumber->GetStep() );
		}
		else
		{
			m_cNumber->SetValue( m_cNumber->GetValue() - m_cNumber->GetStep() );
		}
		ReceiveFromControl();
	}
}


//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnLButtonDblClk( UINT nFlags,CPoint point )
{
	if (m_type == ePropertyBool)
	{
		// Swap boolean value.
		if (GetBoolValue())
			SetValue( "0" );
		else
			SetValue( "1" );
	}
	else
	{
		// Simulate button click.
		if (m_cButton)
			m_cButton->Click();
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnLButtonDown( UINT nFlags,CPoint point )
{
	if (m_type == ePropertyBool)
	{
		CRect rect;
		m_propertyCtrl->GetItemRect( this,rect );
		rect = m_propertyCtrl->GetItemValueRect( rect );

		CPoint p( rect.left-2,rect.top+1 );
		int sz = rect.bottom - rect.top;
		rect = CRect(p.x,p.y,p.x+sz,p.y+sz );

		if (rect.PtInRect(point))
		{
			// Swap boolean value.
			if (GetBoolValue())
				SetValue( "0" );
			else
				SetValue( "1" );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnColorBrowseButton()
{
	COLORREF clr = StringToColor(m_value);
  if (GetIEditor()->SelectColor(clr,m_propertyCtrl))
  {
		int r,g,b;
		r = GetRValue(clr);
		g = GetGValue(clr);
		b = GetBValue(clr);
    //val.Format( "R:%d G:%d B:%d",r,g,b );
		CString val;
		val.Format( "%d,%d,%d",r,g,b );
		SetValue( val );
		m_propertyCtrl->Invalidate();
		//RedrawWindow( OwnerProperties->hWnd,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN );
  }
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnFileBrowseButton()
{
	ECustomFileType ftype = EFILE_TYPE_ANY;

	if (m_type == ePropertyTexture)
	{
		// Filters for texture.
		ftype = EFILE_TYPE_TEXTURE;
	}
	else if (m_type == ePropertySound)
	{
		// Filters for sounds.
		ftype = EFILE_TYPE_SOUND;
	}
	else if (m_type == ePropertyModel)
	{
		// Filters for models.
		ftype = EFILE_TYPE_GEOMETRY;
	}
	CString startPath = Path::GetPath(m_value);

	/*
	if (ftype == EFILE_TYPE_ANY)
	{
		CString relativeFilename;
		if (CFileUtil::SelectFile( "All Files (*.*)|*.*",startPath,relativeFilename ))
		{
			SetValue( relativeFilename );
		}
	}
	else
	{
	*/
	CString relativeFilename = m_value;
	if (CFileUtil::SelectSingleFile( ftype,relativeFilename,"",startPath ))
	{
		SetValue( relativeFilename );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnShaderBrowseButton()
{
	CShadersDialog cShaders( m_value );
	if (cShaders.DoModal() == IDOK)
	{
		SetValue( cShaders.GetSelection() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::ReloadValues()
{
	if (m_node)
		ParseXmlNode(false);
	if (m_pVariable)
		SetVariable(m_pVariable);
	for (int i = 0; i < GetChildCount(); i++)
	{
		GetChild(i)->ReloadValues();
	}
}

//////////////////////////////////////////////////////////////////////////
CString CPropertyItem::GetTip() const
{
	if (!m_tip.IsEmpty())
		return m_tip;

	CString type;
	for (int i = 0; i < NumPropertyTypes; i++)
	{
		if (m_type == s_propertyTypeNames[i].type)
		{
			type = s_propertyTypeNames[i].name;
			break;
		}
	}

	CString tip;
	tip = CString("[")+type+"] ";
	tip += m_name + ": " + m_value;
	
	if (m_pVariable)
	{
		CString description = m_pVariable->GetDescription();
		if (!description.IsEmpty())
		{
			tip += CString("\r\n") + description;
		}
	}
	return tip;
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnEditChanged()
{
	ReceiveFromControl();
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnNumberCtrlUpdate( CNumberCtrl *ctrl )
{
	ReceiveFromControl();
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnAIBehaviorBrowseButton()
{
	CAIDialog aiDlg(m_propertyCtrl);
	aiDlg.SetAIBehavior( m_value );
	if (aiDlg.DoModal() == IDOK)
	{
		SetValue( aiDlg.GetAIBehavior() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnAIAnchorBrowseButton()
{
	CAIAnchorsDialog aiDlg(m_propertyCtrl);
	aiDlg.SetAIAnchor(m_value);
	if (aiDlg.DoModal()==IDOK)
	{
		SetValue(aiDlg.GetAIAnchor());
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnAICharacterBrowseButton()
{
	CAICharactersDialog aiDlg(m_propertyCtrl);
	aiDlg.SetAICharacter( m_value );
	if (aiDlg.DoModal() == IDOK)
	{
		SetValue( aiDlg.GetAICharacter() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnEquipBrowseButton()
{
	CEquipPackDialog EquipDlg(m_propertyCtrl);
	EquipDlg.SetCurrEquipPack(m_value);
	if (EquipDlg.DoModal()==IDOK)
	{
		SetValue(EquipDlg.GetCurrEquipPack());
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnSoundPresetBrowseButton()
{
	CSelectSoundPresetDlg PresetDlg;
	PresetDlg.SetCurrPreset(m_value);
	if (PresetDlg.DoModal()==IDOK)
	{
		SetValue(PresetDlg.GetCurrPreset());
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnEAXPresetBrowseButton()
{
	CSelectEAXPresetDlg PresetDlg;
	PresetDlg.SetCurrPreset(m_value);
	if (PresetDlg.DoModal()==IDOK)
	{
		SetValue(PresetDlg.GetCurrPreset());
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertyItem::OnMaterialBrowseButton()
{
	// Open material browser dialog.
}
