////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   propertyitem.h
//  Version:     v1.00
//  Created:     5/6/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __propertyitem_h__
#define __propertyitem_h__

#if _MSC_VER > 1000
#pragma once
#endif

//! All possible property types.
enum PropertyType
{
	ePropertyInvalid = 0,
	ePropertyTable = 1,
	ePropertyBool = 2,
	ePropertyInt,
	ePropertyFloat,
	ePropertyVector,
	ePropertyString,
	ePropertyColor,
	ePropertyAngle,
	ePropertyFile,
	ePropertyTexture,
	ePropertySound,
	ePropertyModel,
	ePropertySelection,
	ePropertyList,
	ePropertyShader,
	ePropertyMaterial,
	ePropertyAiBehavior,
	ePropertyAiAnchor,
	ePropertyAiCharacter,
	ePropertyEquip,
	ePropertySoundPreset,
	ePropertyEAXPreset,
	ePropertyLocalString,
};

// forward declarations.
class CNumberCtrl;
class CPropertyCtrl;
class CInPlaceEdit;
class CInPlaceComboBox;
class CInPlaceButton;
struct IVariable;

/** Item of CPropertyCtrl.
		Every property item reflects value of single XmlNode.
*/
class CPropertyItem : public CRefCountBase
{
public:
  // Variables.
  // Constructors.
  CPropertyItem( CPropertyCtrl* pCtrl );
	virtual ~CPropertyItem();

	//! Set xml node to this property item.
	virtual void SetXmlNode( XmlNodeRef &node );

	//! Set variable. 
	virtual void SetVariable( IVariable *var );

	//! Get Variable.
	IVariable* GetVariable() const { return m_pVariable; }

	/** Get type of property item.
	*/
	virtual int GetType() { return m_type; }
  
	/** Get name of property item.
	*/
	virtual CString GetName() const { return m_name; };

	/** Set name of property item.
	*/
	virtual void SetName( const char *sName ) { m_name = sName; };

	/** Called when item becomes selected.
	*/
	virtual void SetSelected( bool selected );

	/** Get if item is selected.
	*/
	bool IsSelected() const { return m_bSelected; };

	/** Get if item is currently expanded.
	*/
	bool IsExpanded() const { return m_bExpanded; };

	/** Get if item can be expanded (Have children).
	*/
	bool IsExpandable() const { return m_bExpandable; };

	/** Check if item cannot be category.
	*/
	bool IsNotCategory() const { return m_bNoCategory; };

	/** Check if item must be bold.
	*/
	bool IsBold() const;

	/** Check if item must be disabled.
	*/
	bool IsDisabled() const;

	/** Get height of this item.
	*/
	virtual int	GetHeight() { return 14; }
	
	/** Called by PropertyCtrl to draw value of this item.
	*/
	virtual void DrawValue( CDC *dc,CRect rect );

	/** Called by PropertyCtrl when item selected to creare in place editing control.
	*/
	virtual void CreateInPlaceControl( CWnd* pWndParent,CRect& rect );
	/** Called by PropertyCtrl when item deselected to destroy in place editing control.
	*/
	virtual void DestroyInPlaceControl();

	/** Move in place control to new position.
	*/
	virtual void MoveInPlaceControl( const CRect& rect );

	/** Set Focus to inplace control.
	*/
	virtual void SetFocus();

	/** Set data from InPlace control to Item value.
	*/
	virtual void SetData( CWnd* pWndInPlaceControl ){};

	//////////////////////////////////////////////////////////////////////////
	// Mouse notifications.
	//////////////////////////////////////////////////////////////////////////
	virtual void OnLButtonDown( UINT nFlags,CPoint point );
	virtual void OnRButtonDown( UINT nFlags,CPoint point ) {};
	virtual void OnLButtonDblClk( UINT nFlags,CPoint point );
	virtual void OnMouseWheel( UINT nFlags,short zDelta,CPoint point );

	/** Changes value of item.
	*/
	virtual void SetValue( const char* sValue,bool bRecordUndo=true );

	/** Returns current value of property item.
	*/
	virtual const char* GetValue() const;
	
	/** Get Item's XML node.
	*/
	XmlNodeRef&	GetXmlNode() { return m_node; };

	//////////////////////////////////////////////////////////////////////////
	//! Get description of this item.
	CString GetTip() const;

	//! Return image index of this property.
	int GetImage() const { return m_image; };

	//! Return true if this property item is modified.
	bool IsModified() const { return m_modified; }

	//////////////////////////////////////////////////////////////////////////
	// Childs.
	//////////////////////////////////////////////////////////////////////////
	//! Expand child nodes.
	virtual void SetExpanded( bool expanded );
	
	//! Reload Value from Xml Node (hierarchicaly reload children also).
	virtual void ReloadValues();

	//! Get number of child nodes.
	int GetChildCount() const { return m_childs.size(); };
	//! Get Child by id.
	CPropertyItem* GetChild( int index ) const { return m_childs[index]; };

	//! Parent of this item.
	CPropertyItem* GetParent() const { return m_parent; };

	//! Add Child item.
	void AddChild( CPropertyItem *item );

	//! Delete child item.
	void RemoveChild( CPropertyItem *item );

	//! Find item that reference specified property.
	CPropertyItem* FindItemByVar( IVariable *pVar );
	//! Get full name, including names of all parents.
	virtual CString GetFullName() const;
	//! Find item by full specified item.
	CPropertyItem* FindItemByFullName( const CString &name );

protected:
	//////////////////////////////////////////////////////////////////////////
	// Private methods.
	//////////////////////////////////////////////////////////////////////////
	void ReceiveFromControl();
  void SendToControl();

	void OnChildChanged( CPropertyItem *child );

	void OnEditChanged();
	void OnNumberCtrlUpdate( CNumberCtrl *ctrl );
	void OnNumberCtrlBeginUpdate( CNumberCtrl *ctrl ) {};
	void OnNumberCtrlEndUpdate( CNumberCtrl *ctrl ) {};

	void OnComboSelection();

	void OnColorBrowseButton();
	void OnFileBrowseButton();
	void OnShaderBrowseButton();
	void OnAIBehaviorBrowseButton();
	void OnAIAnchorBrowseButton();
	void OnAICharacterBrowseButton();
	void OnEquipBrowseButton();
	void OnSoundPresetBrowseButton();
	void OnEAXPresetBrowseButton();
	void OnMaterialBrowseButton();

	void ParseXmlNode( bool bRecursive=true );

	//! String to color.
	COLORREF StringToColor( const CString &value );
	//! String to boolean.
	bool GetBoolValue();

	//! Convert variable value to value string.
	CString VarToValue( IVariable *var );

	//! Convert from value to variable.
	void ValueToVar( const CString &value,IVariable *var );

	//! Release used variable. 
	void ReleaseVariable();
	//! Callback called when variable change.
	void OnVariableChange( IVariable *var );
	
private:
	CString m_name;
	PropertyType m_type;

	CString m_value;
	CString m_prevValue;

	//////////////////////////////////////////////////////////////////////////
	// Flags for this property item.
	//////////////////////////////////////////////////////////////////////////
	//! True if item selected.
	unsigned int m_bSelected : 1;
	//! True if item currently expanded
	unsigned int m_bExpanded : 1;
	//! True if item can be expanded
	unsigned int m_bExpandable : 1;
	//! True if item can not be category.
	unsigned int m_bNoCategory : 1;
	//! If tru ignore update that comes from childs.
	unsigned int m_bIgnoreChildsUpdate : 1;
	//! True if item modified.
	unsigned int m_modified : 1;

	// Used for number controls.
	float m_rangeMin;
	float m_rangeMax;
	int		m_iInternalPrecision;		//!< m.m. internal precision (digits behind the comma, only used for floats)

	// Xml node.
	XmlNodeRef m_node;

	//! Pointer to the variable for this item.
	TSmartPtr<IVariable> m_pVariable;

	//////////////////////////////////////////////////////////////////////////
	// InPlace controls.
	CNumberCtrl* m_cNumber;
	CInPlaceEdit* m_cEdit;
	CInPlaceComboBox* m_cCombo;
	CInPlaceButton* m_cButton;
	//////////////////////////////////////////////////////////////////////////

	//! Owner property control.
	CPropertyCtrl* m_propertyCtrl;

	//! Parent item.
	CPropertyItem* m_parent;

	// Enum.
	IVarEnumListPtr m_enumList;

	CString m_tip;
	int m_image;

	//! Last modified time in seconds.
	float m_lastModified;

	float m_valueMultiplier;

	// Childs.
	typedef std::vector<TSmartPtr<CPropertyItem> > Childs;
	Childs m_childs;
};


#endif // __propertyitem_h__
