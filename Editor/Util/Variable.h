#ifndef __Variable_h__
#define __Variable_h__

#if _MSC_VER > 1000
#pragma once
#endif

//typedef string CString;
inline const char* to_c_str( const char *str ) { return str; }
inline const char* to_c_str( const string &str ) { return str.c_str(); }
inline const char* to_c_str( const CString &str ) { return str; }
#define MAX_VAR_STRING_LENGTH 4096

#define DEFAULT_VARIABLE_MIN -100000
#define DEFAULT_VARIABLE_MAX 100000

struct IVarEnumList;
class CUsedResources;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/** IVariable is the variant variable interface.
 */
struct IVariable : public CRefCountBase
{
	/** Type of data stored in variable.
	*/
	enum EType
	{
		UNKNOWN,	//!< Unknown paremeter type.
		INT,			//!< Integer property.
		BOOL,			//!< Boolean property.
		FLOAT,		//!< Float property.
		VECTOR,		//!< Vector property.
		QUAT,			//!< Quaternion property.
		STRING,		//!< String property.
		ARRAY		//!< Array of parameters.
	};

	//! Type of data holded by variable.
	enum EDataType
	{
		DT_SIMPLE = 0, //!< Standart param type.
		DT_PERCENT,		//!< Percent data type, (Same as simple but value is from 0-1 and UI will be from 0-100).
		DT_COLOR,
		DT_ANGLE,
		DT_FILE,
		DT_TEXTURE,
		DT_SOUND,
		DT_OBJECT,
		DT_SHADER,
		DT_AI_BEHAVIOR,
		DT_AI_ANCHOR,
		DT_AI_CHARACTER,
		DT_LOCAL_STRING,
		DT_EQUIP,
		DT_SOUNDPRESET,
		DT_EAXPRESET,
		DT_MATERIAL
	};

	// Flags that can used with variables.
	enum EFlags
	{
		// User interface related flags.
		UI_DISABLED = 0x01, //!< If Set  this variable will be disabled in UI.
		UI_BOLD = 0x02,			//!<  If set, variable name in properties will be bold.
	};

	typedef Functor1<IVariable*> OnSetCallback;

	//! Get name of parameter.
	virtual	const char*	GetName() const = 0;
	//! Set name of parameter.
	virtual void SetName( const CString &name ) = 0;

	//! Get human readable name of parameter (Normally same as name).
	virtual	const char*	GetHumanName() const = 0;
	//! Set human readable name of parameter (Normall same as name).
	virtual void SetHumanName( const CString &name ) = 0;

	//! Get variable description.
	virtual const char*	GetDescription() const = 0;
	//! Set variable description.
	virtual void SetDescription( const char *desc ) = 0;

	//! Get paremeter type.
	virtual	EType	GetType() const = 0;
	//! Get size of parameter.
	virtual	int	GetSize() const = 0;

	//! Type of data stored in this variable.
	virtual	unsigned char	GetDataType() const = 0;
	virtual void SetDataType( unsigned char dataType ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Flags
	//////////////////////////////////////////////////////////////////////////
	//! Set variable flags, (Limited to 8 flags).
	virtual	void SetFlags( int flags ) = 0;
	virtual	int  GetFlags() const = 0;

	/////////////////////////////////////////////////////////////////////////////
	// Set methods.
	/////////////////////////////////////////////////////////////////////////////
	virtual void Set( int value ) = 0;
	virtual void Set( bool value ) = 0;
	virtual void Set( float value ) = 0;
	virtual void Set( const Vec3 &value ) = 0;
	virtual void Set( const Quat &value ) = 0;
	virtual void Set( const CString &value ) = 0;
	virtual void Set( const char *value ) = 0;

	/////////////////////////////////////////////////////////////////////////////
	// Get methods.
	/////////////////////////////////////////////////////////////////////////////
	virtual void Get( int &value ) const  = 0;
	virtual void Get( bool &value ) const  = 0;
	virtual void Get( float &value ) const  = 0;
	virtual void Get( Vec3 &value ) const  = 0;
	virtual void Get( Quat &value ) const  = 0;
	virtual void Get( CString &value ) const  = 0;

	//////////////////////////////////////////////////////////////////////////
	// For vector parameters.
	//////////////////////////////////////////////////////////////////////////
	virtual int NumChildVars() const = 0;
	virtual IVariable* GetChildVar( int index ) const = 0;
	virtual void AddChildVar( IVariable *var ) = 0;
	virtual void DeleteAllChilds() = 0;

	//! Return cloned value of variable.
	virtual IVariable* Clone( bool bRecursive ) const = 0;

	//! Copy variable value from specified variable.
	//! This method executed always recursively on all sub hierachy of variables,
	//! In Array vars, will never create new variables, only copy values of corresponding childs.
	//! @param fromVar Source variable to copy value from.
	virtual void CopyValue( IVariable *fromVar ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Value Limits.
	//////////////////////////////////////////////////////////////////////////
	//! Set value limits.
	virtual void SetLimits( float min,float max ) = 0;
	//! Get value limits.
	virtual void GetLimits( float &min,float &max ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Wire/Unwire variables.
	//////////////////////////////////////////////////////////////////////////
	//! Wire variable, wired variable will be changed when this var changes.
	virtual void Wire( IVariable *targetVar ) = 0;
	//! Unwire variable.
	virtual void Unwire( IVariable *targetVar ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Assign on set callback.
	//////////////////////////////////////////////////////////////////////////
	virtual void AddOnSetCallback( OnSetCallback func ) = 0;
	virtual void RemoveOnSetCallback( OnSetCallback func ) = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Retrieve pointer to selection list used by variable.
	virtual IVarEnumList* GetEnumList() const = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Serialize variable to XML.
	//////////////////////////////////////////////////////////////////////////
	virtual void Serialize( XmlNodeRef node,bool load ) = 0;
};

// Smart pointer to this parameter.
typedef TSmartPtr<IVariable> IVariablePtr;

/**
 **************************************************************************************
 * CVariableBase implements IVariable interface and provide default implementation
 * for basic IVariable functionality (Name, Flags, etc...)
 * CVariableBase cannot be instantiated directly and should be used as the base class for
 * actual Variant implementation classes.
 ***************************************************************************************
 */
class	CVariableBase : public IVariable
{
public:
	~CVariableBase() {}

	void SetName( const CString &name ) { m_name = name; };
	//! Get name of parameter.
	const char*	GetName() const { return to_c_str(m_name); };

	const char*	GetHumanName() const
	{
		if (!m_humanName.IsEmpty())
			return m_humanName;
		return m_name;
	}
	void SetHumanName( const CString &name ) { m_humanName = name; }

	void SetDescription( const char *desc ) { m_description = desc; };
	//! Get name of parameter.
	const char*	GetDescription() const { return to_c_str(m_description); };

	EType	GetType() const { return  IVariable::UNKNOWN; };
	int	GetSize() const { return sizeof(*this); };

	unsigned char	GetDataType() const { return m_dataType; };
	void SetDataType( unsigned char dataType ) { m_dataType = dataType; }

	void SetFlags( int flags ) { m_flags = flags; }
	int  GetFlags() const { return m_flags; }

	//////////////////////////////////////////////////////////////////////////
	// Set methods.
	//////////////////////////////////////////////////////////////////////////
	void Set( int value )							{ assert(0); }
	void Set( bool value )						{ assert(0); }
	void Set( float value )						{ assert(0); }
	void Set( const Vec3 &value )			{ assert(0); }
	void Set( const Quat &value )			{ assert(0); }
	void Set( const CString &value )	{ assert(0); }
	void Set( const char *value )			{ assert(0); }

	//////////////////////////////////////////////////////////////////////////
	// Get methods.
	//////////////////////////////////////////////////////////////////////////
	void Get( int &value ) const 			{ assert(0); }
	void Get( bool &value ) const 		{ assert(0); }
	void Get( float &value ) const 		{ assert(0); }
	void Get( Vec3 &value ) const 		{ assert(0); }
	void Get( Quat &value ) const 		{ assert(0); }
	void Get( CString &value ) const 	{ assert(0); }

	//////////////////////////////////////////////////////////////////////////
	// Implementation functions.
	//////////////////////////////////////////////////////////////////////////
	int NumChildVars() const { return 0; }
	IVariable* GetChildVar( int index ) const { return 0; }
	void AddChildVar( IVariable *var ) { assert(0); }; // Not supported.
	void DeleteAllChilds() {};

	//////////////////////////////////////////////////////////////////////////
	void Wire( IVariable *var )
	{
		m_wiredVars.push_back(var);
	}
	//////////////////////////////////////////////////////////////////////////
	void Unwire( IVariable *var )
	{
		if (!var)
		{
			// Unwire all.
			m_wiredVars.clear();
		}
		else
		{
			stl::find_and_erase( m_wiredVars,var );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void AddOnSetCallback( OnSetCallback func )
	{
		m_onSetFuncs.push_back(func);
	}

	//////////////////////////////////////////////////////////////////////////
	void RemoveOnSetCallback( OnSetCallback func )
	{
		stl::find_and_erase( m_onSetFuncs,func );
	}

	//////////////////////////////////////////////////////////////////////////
	void Serialize( XmlNodeRef node,bool load )
	{
		if (load)
		{
			// Loading.
			if (node->haveAttr(to_c_str(m_name)))
			{
				const char *str = node->getAttr(to_c_str(m_name));
				Set( CString(str) );
			}
		}
		else
		{
			// Saving.
			CString str;
			Get( str );
			node->setAttr( to_c_str(m_name),to_c_str(str) );
		}
	}

	virtual void SetLimits( float min,float max ) {};
	virtual void GetLimits( float &min,float &max ) {};
	virtual IVarEnumList* GetEnumList() const { return 0; };

protected:
	// Constructor.
	CVariableBase() {
		m_dataType = DT_SIMPLE;
		m_flags = 0;
	};
	// Copy constructor.
	CVariableBase( const CVariableBase &var )
	{
		m_name = var.m_name;
		m_humanName = var.m_humanName;
		m_flags = var.m_flags;
		m_dataType = var.m_dataType;
		// Never copy callback function or wired variables they are private to specific variable,

	};
private:
	// Not allow.
	CVariableBase& operator=( const CVariableBase &var )
	{
		return *this;
	}

protected:
	//////////////////////////////////////////////////////////////////////////
	// Variables.
	//////////////////////////////////////////////////////////////////////////
	typedef std::vector<OnSetCallback> OnSetCallbackList;
	typedef std::vector<IVariablePtr> WiredList;

	CString m_name;
	CString m_humanName;
	CString m_description;
	
	//! Extended data (Extended data is never copied, it's always private to this variable).
	WiredList m_wiredVars;
	OnSetCallbackList m_onSetFuncs;

	// Wired params.
	//! Limited to 8 flags.
	unsigned char m_flags;
	unsigned char m_dataType;
};

/**
 **************************************************************************************
 * CVariableArray implements variable of type array of IVariables.
 ***************************************************************************************
 */
class	CVariableArray : public CVariableBase
{
public:
	CVariableArray() {};
	// Copy Constructor.
	CVariableArray( const CVariableArray &var ) : CVariableBase(var)
	{}

	//! Get name of parameter.
	virtual	EType	GetType() const { return IVariable::ARRAY; };
	virtual	int		GetSize() const { return sizeof(CVariableArray); };

	//////////////////////////////////////////////////////////////////////////
	// Set methods.
	//////////////////////////////////////////////////////////////////////////
	virtual void Set( const char *value )
	{
		if (m_strValue != value)
		{
			m_strValue = value;
			// If have wired variables or OnSet callback, process them.
			// Call on set callback.
			for (OnSetCallbackList::iterator it = m_onSetFuncs.begin(); it != m_onSetFuncs.end(); ++it)
			{
				// Call on set callback.
				(*it)(this);
			}
			// Send value to wired variable.
			for (WiredList::iterator it = m_wiredVars.begin(); it != m_wiredVars.end(); ++it)
			{
				// Set current value on each wired variable.
				(*it)->Set(value);
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// Get methods.
	//////////////////////////////////////////////////////////////////////////
	virtual void Get( CString &value ) const { value = m_strValue; }

	//////////////////////////////////////////////////////////////////////////
	IVariable* Clone( bool bRecursive ) const
	{
		CVariableArray *var = new CVariableArray(*this);
		for (Vars::const_iterator it = m_vars.begin(); it != m_vars.end(); ++it)
		{
			var->m_vars.push_back( (*it)->Clone(bRecursive) );
		}
		return var;
	}

	//////////////////////////////////////////////////////////////////////////
	void CopyValue( IVariable *fromVar )
	{
		assert(fromVar);
		if (fromVar->GetType() != IVariable::ARRAY)
			return;
		int numSrc = fromVar->NumChildVars();
		int numTrg = m_vars.size();
		for (int i = 0; i < numSrc && i < numTrg; i++)
		{
			// Copy Every child variable.
			m_vars[i]->CopyValue( fromVar->GetChildVar(i) );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	int NumChildVars() const { return m_vars.size(); }
	IVariable* GetChildVar( int index ) const
	{
		assert( index >= 0 && index < (int)m_vars.size() );
		return m_vars[index];
	}
	void AddChildVar( IVariable *var )
	{
		m_vars.push_back(var);
	}

	void DeleteAllChilds()
	{
		m_vars.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	void Serialize( XmlNodeRef node,bool load )
	{
		if (load)
		{
			// Loading.
			CString name;
			for (Vars::iterator it = m_vars.begin(); it != m_vars.end(); ++it)
			{
				IVariable *var = *it;
				if (var->NumChildVars())
				{
					XmlNodeRef child = node->findChild(var->GetName());
					if (child)
						var->Serialize( child,load );
				}
				else
					var->Serialize( node,load );
			}
		}
		else
		{
			// Saving.
			for (Vars::iterator it = m_vars.begin(); it != m_vars.end(); ++it)
			{
				IVariable *var = *it;
				if (var->NumChildVars())
				{
					XmlNodeRef child = node->newChild(var->GetName());
					var->Serialize( child,load );
				}
				else
					var->Serialize( node,load );
			}
		}
	}

protected:
	typedef std::vector<IVariablePtr> Vars;
	Vars m_vars;
	//! Any string value displayed in properties.
	CString m_strValue;
};

/** var_type namespace includes type definitions needed for CVariable implementaton.
 */
namespace var_type
{
	//////////////////////////////////////////////////////////////////////////
	template <int TypeID,bool IsStandart,bool IsInteger,bool IsSigned>
		struct type_traits_base 
	{
		static int type() { return TypeID; };
		//! Return true if standart C++ type.
		static bool is_standart() { return IsStandart; };
		static bool is_integer() { return IsInteger; };
		static bool is_signed() { return IsSigned; };
	};
	
	template <class Type>
		struct type_traits : public type_traits_base<IVariable::UNKNOWN,false,false,false> {};
	
	// Types specialization.
	template<> struct type_traits<int>		: public type_traits_base<IVariable::INT,true,true,true> {};
	template<> struct type_traits<bool>		: public type_traits_base<IVariable::BOOL,true,true,false> {};
	template<> struct type_traits<float>	: public type_traits_base<IVariable::FLOAT,true,false,false> {};
	template<> struct type_traits<Vec3>		: public type_traits_base<IVariable::VECTOR,false,false,false> {};
	template<> struct type_traits<Quat>		: public type_traits_base<IVariable::QUAT,false,false,false> {};
	template<> struct type_traits<CString>	: public type_traits_base<IVariable::STRING,false,false,false> {};
	//////////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////////////////
	// Extended stream operatios.
	//////////////////////////////////////////////////////////////////////////
	//! Put CString to out stream operator.
	inline std::ostream&	operator<<( std::ostream &stream,const CString &s )
	{
		stream << to_c_str(s);
		return stream;
	}

	//! Put Vec3 to out stream operator.
	inline std::ostream&	operator<<( std::ostream &stream,const Vec3 &v )
	{
		stream << v.x << "," << v.y << "," << v.z;
		return stream;
	}
	
	//! Put Quat to out stream operator.
	inline std::ostream&	operator<<( std::ostream &stream,const Quat &q )
	{
		stream << q.w << "," << q.v.x << "," << q.v.y << "," << q.v.z;
		return stream;
	}
	
	//! Get CString from input stream operator.
	inline std::istream&	operator>>( std::istream &stream,CString &s )
	{
		// String is limited..
		char str[MAX_VAR_STRING_LENGTH];
		stream >> str;
		s = str;
		return stream;
	}

	//! Get Vec3 from input stream operator.
	inline std::istream&	operator>>( std::istream &stream,Vec3 &v )
	{
		char s[64];
		stream >> s;
		v.x = v.y = v.z = 0;
		sscanf( s,"%f,%f,%f",&v.x,&v.y,&v.z );
		return stream;
	}

	//! Get Quat from input stream operator.
	inline std::istream&	operator>>( std::istream &stream,Quat &q )
	{
		char s[64];
		stream >> s;
		q.v.x = q.v.y = q.v.z = q.w = 0;
		sscanf( s,"%f,%f,%f,%f",&q.w,&q.v.x,&q.v.y,&q.v.z );
		return stream;
	}

	
	//////////////////////////////////////////////////////////////////////////
	// General one type to another type convertor class.
	//////////////////////////////////////////////////////////////////////////
	template <class From,class To>
		struct type_convertor
	{
		void operator()(  const From &value,To &to ) const
		{
			// Use stream conversion.
			std::stringstream ss;
			ss << value; // first insert value to stream
			ss >> to; // write value to result
		};
	};
	
	//////////////////////////////////////////////////////////////////////////
	// Specialized faster conversions.
	//////////////////////////////////////////////////////////////////////////
	template<> void type_convertor<int,int>::operator()( const int &from,int &to ) const { to = from; }
	template<> void type_convertor<int,bool>::operator()( const int &from,bool &to ) const { to = from != 0; }
	template<> void type_convertor<int,float>::operator()( const int &from,float &to ) const { to = (float)from; }
	//////////////////////////////////////////////////////////////////////////
	template<> void type_convertor<bool,int>::operator()( const bool &from,int &to ) const { to = from; }
	template<> void type_convertor<bool,bool>::operator()( const bool &from,bool &to ) const { to = from; }
	template<> void type_convertor<bool,float>::operator()( const bool &from,float &to ) const { to = from; }
	//////////////////////////////////////////////////////////////////////////
	template<> void type_convertor<float,int>::operator()( const float &from,int &to ) const { to = (int)from; }
	template<> void type_convertor<float,bool>::operator()( const float &from,bool &to ) const { to = from != 0; }
	template<> void type_convertor<float,float>::operator()( const float &from,float &to ) const { to = from; }
	
	template<> void type_convertor<Vec3,Vec3>::operator()( const Vec3 &from,Vec3 &to ) const { to = from; }
	template<> void type_convertor<Quat,Quat>::operator()( const Quat &from,Quat &to ) const { to = from; }
	template<> void type_convertor<CString,CString>::operator()( const CString &from,CString &to ) const { to = from; }
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Custom comparasion functions for different variable type's values,.
	//////////////////////////////////////////////////////////////////////////
	template <class Type>
	inline bool compare( const Type &arg1,const Type &arg2 )
	{
		return arg1 == arg2;
	};
	inline bool compare( const Vec3 &v1,const Vec3 &v2 )
	{
		return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
	}
	inline bool compare( const Quat &q1,const Quat &q2 )
	{
		return q1.v.x == q2.v.x && q1.v.y == q2.v.y && q1.v.z == q2.v.z && q1.w == q2.w;
	}
	inline bool compare( const char* s1,const char* s2 )
	{
		return strcmp(s1,s2) == 0;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Custom Initializaton functions for different variable type's values,.
	//////////////////////////////////////////////////////////////////////////
	template <class Type>
	inline void init( Type &val )
	{
		val = 0;
	};
	inline void init( Vec3 &val )
	{
		val.x = 0; val.y = 0;val.z = 0;
	};
	inline void init( Quat &val )
	{
		val.v.x = 0; val.v.y = 0; val.v.z = 0; val.w = 0;
	};
	inline void init( const char* &val )
	{
		val = "";
	};
	inline void init( CString &val )
	{
		// self initializing.
	};
	//////////////////////////////////////////////////////////////////////////
};


//////////////////////////////////////////////////////////////////////////
template <class T>
class	CVariable : public CVariableBase
{
	typedef CVariable<T> Self;
public:
	// Constructor.
	CVariable()
	{
		// Initialize value to zero or empty string.
		var_type::init( m_value );
		m_valueMin = DEFAULT_VARIABLE_MIN;
		m_valueMax = DEFAULT_VARIABLE_MAX;
	}

	//! Get name of parameter.
	virtual	EType	GetType() const { return  (EType)var_type::type_traits<T>::type(); };
	virtual	int		GetSize() const { return sizeof(T); };

	//////////////////////////////////////////////////////////////////////////
	// Set methods.
	//////////////////////////////////////////////////////////////////////////
	virtual void Set( int value )							{ SetValue(value); }
	virtual void Set( bool value )						{ SetValue(value); }
	virtual void Set( float value )						{ SetValue(value); }
	virtual void Set( const Vec3 &value )			{ SetValue(value); }
	virtual void Set( const Quat &value )			{ SetValue(value); }
	virtual void Set( const CString &value )	{ SetValue(value); }
	virtual void Set( const char *value )			{ SetValue(CString(value)); }

	//////////////////////////////////////////////////////////////////////////
	// Get methods.
	//////////////////////////////////////////////////////////////////////////
	virtual void Get( int &value ) const 						{ GetValue(value); }
	virtual void Get( bool &value ) const 					{ GetValue(value); }
	virtual void Get( float &value ) const 					{ GetValue(value); }
	virtual void Get( Vec3 &value ) const 					{ GetValue(value); }
	virtual void Get( Quat &value ) const 					{ GetValue(value); }
	virtual void Get( CString &value ) const 				{ GetValue(value); }

	//////////////////////////////////////////////////////////////////////////
	// Limits.
	//////////////////////////////////////////////////////////////////////////
	virtual void SetLimits( float min,float max )
	{
		m_valueMin = min;
		m_valueMax = max;
	}
	virtual void GetLimits( float &min,float &max )
	{
		min = m_valueMin;
		max = m_valueMax;
	}

	//////////////////////////////////////////////////////////////////////////
	// Access operators.
	//////////////////////////////////////////////////////////////////////////
	//! Cast to holded type.
	operator T() const { return m_value; }

	//! Assign operator for variable.
	void operator=( const T& value ) { SetValue(value); }

	//////////////////////////////////////////////////////////////////////////
	IVariable* Clone( bool bRecursive ) const
	{
		Self *var = new Self(*this);
		return var;
	}

	//////////////////////////////////////////////////////////////////////////
	void CopyValue( IVariable *fromVar )
	{
		assert(fromVar);
		T val;
		fromVar->Get(val);
		SetValue(val);
	}

protected:
	// Copy Constructor.
	CVariable( const CVariable<T> &var ) : CVariableBase(var)
	{
		m_value = var.m_value;
		m_valueMin = var.m_valueMin;
		m_valueMax = var.m_valueMax;
	}
	//////////////////////////////////////////////////////////////////////////
	template <class P>
	void SetValue( const P &value )
	{
		T newValue;
		var_type::type_convertor<P,T> convertor;
		convertor( value,newValue );

		// compare old and new values.
		if (!var_type::compare(m_value,newValue))
		{
			m_value = newValue;
			// If have wired variables or OnSet callback, process them.
			// Call on set callback.
			for (OnSetCallbackList::iterator it = m_onSetFuncs.begin(); it != m_onSetFuncs.end(); ++it)
			{
				// Call on set callback.
				(*it)(this);
			}
			// Send value to wired variable.
			for (WiredList::iterator it = m_wiredVars.begin(); it != m_wiredVars.end(); ++it)
			{
				// Set current value on each wired variable.
				(*it)->Set(value);
			}
		}
	}
	
	//////////////////////////////////////////////////////////////////////////
	template <class P>
	void GetValue( P &value ) const
	{
		var_type::type_convertor<T,P> convertor;
		convertor( m_value,value );
	}

protected:
	T	m_value;
	
	// Min/Max value.
	float m_valueMin;
	float m_valueMax;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Selection list shown in combo box, for enumerated variable.
struct IVarEnumList : public CRefCountBase
{
	//! Get the number of entries in enumeration.
	virtual int GetItemsCount() = 0;
	//! Get the name of specified value in enumeration.
	virtual const CString& GetItemName( int index ) = 0;
	//! Set the value of variable to the value of entry with specified name.
	virtual void SetVariableValue( IVariable *pVar,const CString &name ) = 0;
	//! Gets the name of entry which have same value as variable.
	virtual CString GetNameFromVariableValue( IVariable *pVar ) = 0;
};
typedef TSmartPtr<IVarEnumList> IVarEnumListPtr;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Selection list shown in combo box, for enumerated variable.
template <class T>
class CVarEnumList : public IVarEnumList
{
public:
	struct Item {
		CString name;
		T value;
	};
	int GetItemsCount() { return m_items.size(); }
	const CString& GetItemName( int index )
	{
		assert( index >= 0 && index < m_items.size() );
		return m_items[index].name;
	};
	//////////////////////////////////////////////////////////////////////////
	void SetVariableValue( IVariable *pVar,const CString &name )
	{
		assert(pVar);
		for (int i = 0; i < m_items.size(); i++)
		{
			if (name == m_items[i].name)
			{
				pVar->Set( m_items[i].value );
				break;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
	CString GetNameFromVariableValue( IVariable *pVar )
	{
		assert(pVar);
		for (int i = 0; i < m_items.size(); i++)
		{
			T value;
			pVar->Get(value);
			if (value == m_items[i].value)
			{
				return m_items[i].name;
			}
		}
		return "";
	}
	//! Add new item to the selection.
	void AddItem( const CString &name,const T &value )
	{
		Item item;
		item.name = name;
		item.value = value;
		m_items.push_back(item);
	};

private:
	std::vector<Item> m_items;
};

//////////////////////////////////////////////////////////////////////////////////
// CVariableEnum is the same as CVariable but it display enumerated values in UI
//////////////////////////////////////////////////////////////////////////////////
template <class T>
class	CVariableEnum : public CVariable<T>
{
public:
	//////////////////////////////////////////////////////////////////////////
	CVariableEnum() {};

	//! Cast to holded type.
	operator T() const { return m_value; }
	//! Assign operator for variable.
	void operator=( const T& value ) { SetValue(value); }

	//! Add new item to the enumeration.
	void AddEnumItem( const CString &name,const T &value )
	{
		if (!m_enum)
			m_enum = new CVarEnumList<T>;
		m_enum->Add( name,value );
	};
	void SetEnumList( CVarEnumList<T> *enumList )
	{
		m_enum = enumList;
	}
	IVarEnumList* GetEnumList() const
	{
		return m_enum;
	}
	//////////////////////////////////////////////////////////////////////////
	IVariable* Clone( bool bRecursive ) const
	{
		CVariableEnum<T> *var = new CVariableEnum<T>(*this);
		return var;
	}
protected:
	// Copy Constructor.
	CVariableEnum( const CVariableEnum<T> &var ) : CVariable<T>(var)
	{
		m_enum = var.m_enum;
	}
private:
	TSmartPtr<CVarEnumList<T> > m_enum;
};

//////////////////////////////////////////////////////////////////////////
class CVarBlock : public CRefCountBase
{
public:
	// Dtor.
	~CVarBlock();
	//! Add parameter to block.
	void AddVariable( IVariable *var );

	//! Find variable by name.
	IVariable* FindVariable( const char *name,bool bRecursive=true ) const;

	//! Return true if vriable block is empty (Does not have any vars).
	bool IsEmpty() const { return m_vars.empty(); }
	//! Returns number of variables in block.
	int GetVarsCount() const { return m_vars.size(); }
	//! Get pointer to stored variable by index.
	IVariable* GetVariable( int index )
	{
		assert( index >= 0 && index < m_vars.size() );
		return m_vars[index];
	}
	// Clear all vars from VarBlock.
	void Clear() { m_vars.clear(); };

	//////////////////////////////////////////////////////////////////////////
	//! Clone var block.
	CVarBlock* Clone( bool bRecursive ) const;

	//////////////////////////////////////////////////////////////////////////
	//! Copy variable values from specifed var block.
	//! Do not create new variables, only copy values of existing ones.
	//! Should only be used to copy identical var blocks (eg. not Array type var copied to String type var)
	//! @param fromVarBlock Source variable block that contain copied values, must be identical to this var block.
	void CopyValues( CVarBlock *fromVarBlock );

	//////////////////////////////////////////////////////////////////////////
	//! Copy variable values from specifed var block.
	//! Do not create new variables, only copy values of existing ones.
	//! Can be used to copy slighly different var blocks, matching performed by variable name.
	//! @param fromVarBlock Source variable block that contain copied values.
	void CopyValuesByName( CVarBlock *fromVarBlock );

	//////////////////////////////////////////////////////////////////////////
	// Wire/Unwire other variable blocks.
	//////////////////////////////////////////////////////////////////////////
	//! Wire to other variable block.
	//! Only equialent VarBlocks can be wired (same number of variables with same type).
	//! Recursive wiring of array variables is supported.
	void Wire( CVarBlock *toVarBlock );
	//! Unwire var block.
	void Unwire( CVarBlock *varBlock );

	//! Add this callback to every variable in block (recursively).
	void AddOnSetCallback( IVariable::OnSetCallback func );
	//! Remove this callback from every variable in block (recursively).
	void RemoveOnSetCallback( IVariable::OnSetCallback func );

	//////////////////////////////////////////////////////////////////////////
	void Serialize( XmlNodeRef &vbNode,bool load );

	void ReserveNumVariables( int numVars );

	//////////////////////////////////////////////////////////////////////////
	//! Gather resources in this variable block.
	virtual void GatherUsedResources( CUsedResources &resources );
	
protected:
	IVariable* FindChildVar( const char *name,IVariable *pParentVar ) const;
	void SetCallbackToVar( IVariable::OnSetCallback func,IVariable *pVar,bool bAdd );
	void WireVar( IVariable *src,IVariable *trg,bool bWire );
	void GatherUsedResourcesInVar( IVariable *pVar,CUsedResources &resources );

	typedef std::vector<IVariablePtr> Variables;
	Variables m_vars;
};

typedef TSmartPtr<CVarBlock> CVarBlockPtr;

//////////////////////////////////////////////////////////////////////////
class CVarObject :  public CRefCountBase
{
public:
	typedef IVariable::OnSetCallback VarOnSetCallback;

	CVarObject();
	~CVarObject();

	void Serialize( XmlNodeRef node,bool load );
	CVarBlock* GetVarBlock() const { return m_vars; };

	void AddVariable( CVariableBase &var,const CString &varName,VarOnSetCallback cb=NULL,unsigned char dataType=IVariable::DT_SIMPLE );
	void AddVariable( CVariableBase &var,const CString &varName,const CString &varHumanName,VarOnSetCallback cb=NULL,unsigned char dataType=IVariable::DT_SIMPLE );
	void ReserveNumVariables( int numVars );

protected:
	//! Copy values of variables from other VarObject.
	//! Source object must be of same type.
	void CopyVariableValues( CVarObject *sourceObject );

private:
	CVarBlockPtr m_vars;
};

#endif // __Variable_h__