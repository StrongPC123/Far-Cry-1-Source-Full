#ifndef _SYNCHED2DTABLE_H_
#define _SYNCHED2DTABLE_H_

#include "GameObject.h"
#include <IEntitySystem.h>
#include <map>											// STL map<,>
#include <list>											// STL list<>
#include <vector>										// STL vector<>
#include <float.h>									// FLT_MAX


class CXGame;

//! for max 256 players
//! server table is synced to all clients
class CSynched2DTable : public CGameObject
{
public:

	//! constructor
	CSynched2DTable( CXGame *pGame );
	//! destructor
	virtual ~CSynched2DTable() {}

	//! call this only on the server
	void SetEntryXYFloat( const uint32 uiX, const uint32 uiY, const float fValue );
	//! call this only on the server
	void SetEntriesYFloat( const uint32 uiY, const float fValue );
	//! call this only on the server
	void SetEntryXYString( const uint32 uiX, const uint32 uiY, const string &sValue );
	//! call this only on the server
	void SetEntriesYString( const uint32 uiY, const string &sValue );
	//!
	uint32 GetLineCount() const;
	//!
	uint32 GetColumnCount() const;

	// interface IEntityContainer -------------------------------

	virtual bool Init();
	virtual void Update();
	virtual bool Write( CStream &stm, EntityCloneState *cs=NULL );
	virtual bool Read( CStream &stm );
	virtual IScriptObject *GetScriptObject( void );	
	virtual void SetScriptObject(IScriptObject *object);
	virtual void OnSetAngles( const Vec3 &ang );
	virtual void OnDraw( const SRendParams & RendParams ) {}
	virtual bool QueryContainerInterface( ContainerInterfaceType desired_interface, void **pInterface);
	virtual void GetEntityDesc( CEntityDesc &desc ) const;
	void PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime) {};
	virtual void OnEntityNetworkUpdate( const EntityId &idViewerEntity, const Vec3d &v3dViewer, uint32 &inoutPriority,
		EntityCloneState &inoutCloneState ) const;

private: // -----------------------------------------------------------------------

	struct STableEntry
	{
		//! constructor
		STableEntry( const char *szString ) 
			:m_fValue(FLT_MAX), m_sValue(szString)
		{
		}

		//! constructor
		STableEntry( const float fValue ) 
			:m_fValue(fValue)
		{
		}

		//! default constructor
		STableEntry() 
			:m_fValue(FLT_MAX)
		{
		}

		//! copy constructor
		STableEntry( const STableEntry &Rhs )
		{
			m_fValue = Rhs.m_fValue;
			m_sValue = Rhs.m_sValue;
		}
		
		//! assignment operator
		STableEntry &operator=( const STableEntry &Rhs )
		{
			if(this==&Rhs)
				return *this;

			m_fValue = Rhs.m_fValue;
			m_sValue = Rhs.m_sValue;

			return *this;
		}

		//! only if IsFloat() returns true
		float GetFloat() const
		{
			assert(IsFloat());

			return m_fValue;
		}

		//! only if IsFloat() returns false
		string GetString() const
		{
			assert(!IsFloat());

			return m_sValue;
		}
		//!
		void SetFloat( const float fValue )
		{
			assert(fValue!=FLT_MAX);

			m_fValue=fValue;
		}

		//!
		bool IsFloat() const
		{
			return m_fValue!=FLT_MAX;
		}

		//!
		bool Write( CStream &stm );
		//!
		bool Read( CStream &stm );

	private: //! ----------------------------------------------

		float				m_fValue;					//!< currently we only support floats - we should extend this for strings as well
		string			m_sValue;					//!< only used if m_fValue is FLT_MAX
	};

	// -----------------

	struct STableLine
	{
		std::vector<STableEntry>		m_Entries;				//!<

		//!
		void EnsureSize( const uint32 uiX )
		{
			if(uiX>=m_Entries.size())
				m_Entries.resize(uiX+1);
		}

		//!
		void SetX( const uint32 uiX, const STableEntry &rValue )
		{
			if(uiX==0xff)
				{ assert(0); return; }	// (0xff is used to mark the whole line dirty)

			EnsureSize(uiX);
			m_Entries[uiX]=rValue;
		}

		//!
		STableEntry GetX( const uint32 uiX )
		{
			if(uiX==0xff)
				{ assert(0); return STableEntry(); }	// (0xff is used to mark the whole line dirty)

			if(uiX>=m_Entries.size())
				return STableEntry();

			return m_Entries[uiX];
		}

		uint32 GetColumnCount() const
		{
			return m_Entries.size();
		}
	};

	// -----------------

	struct STable
	{
		std::vector<STableLine>				m_Lines;									//!<

		void EnsureSize( const uint32 uiY )
		{
			if(uiY>=m_Lines.size())
				m_Lines.resize(uiY+1);
		}

		void SetXY( const uint32 uiX, const uint32 uiY, const STableEntry &rValue )
		{
			if(uiX==0xff)
				{ assert(0); return; }	// (0xff is used to mark the whole line dirty)

			EnsureSize(uiY);
			m_Lines[uiY].SetX(uiX,rValue);
		}

		STableEntry GetXY( const uint32 uiX, const uint32 uiY )
		{
			if(uiX==0xff)
				{ assert(0); return STableEntry(); }	// (0xff is used to mark the whole line dirty)

			if(uiY>=m_Lines.size())
				return STableEntry();

			return m_Lines[uiY].GetX(uiX);
		}

		uint32 GetColumnCountY( const uint32 uiY ) const
		{
			assert(uiY<m_Lines.size());

			return m_Lines[uiY].GetColumnCount();
		}

		uint32 GetColumnCount() const
		{
			uint32 dwRet=0;
			std::vector<STableLine>::const_iterator it;

			for(it=m_Lines.begin();it!=m_Lines.end();++it)
			{
				const STableLine &line = *it;

				uint32 dwCnt = line.GetColumnCount();

				dwRet = max(dwRet,dwCnt);
			}

			return dwRet;
		}
	};

	// -----------------

	//!
	struct SDirtyItem 
	{
		//! constructor (one entry)
		SDirtyItem( const uint8 ucX, const uint8 ucY ) :m_ucX(ucX), m_ucY(ucY)
		{
		}

		//! constructor (whole line)
		SDirtyItem( const uint8 ucY ) :m_ucX(0xff), m_ucY(ucY)
		{
		}

		//! comparison
		bool operator==( const SDirtyItem &Rhs ) const
		{
			return m_ucX==Rhs.m_ucX && m_ucY==Rhs.m_ucY;
		}

		uint8				m_ucX;									//!< coloumn (0xff is used to mark the whole line dirty)
		uint8				m_ucY;									//!< row
	};

	typedef std::list<SDirtyItem>		TDirtyLists;

	//! one per serverslot
	struct SDirtylist
	{
		//! constructor
		SDirtylist() :m_bFrameType(false), m_bPendingPacket(false)
		{
		}

		TDirtyLists													m_DirtyList;					//!< should be quite small
		bool																m_bPendingPacket;			//!< m_DirtyList.front() was already sent but no acknowledged
		bool																m_bFrameType;					//!< alternating frame type is used as sliding window with size 1
	};

	typedef std::map<uint8,SDirtylist>		TDirtyListsMap;

	// ------------------------------------------------------------------------------

	STable						m_EntryTable;							//!< current form of the table
	IScriptObject *		m_pScriptObject;					//!< to fulfull the IEntityContainer interface
	CXGame *					m_pGame;									//!< pointer to the game where we are, must not be 0
	TDirtyListsMap		m_ServerslotDirtylist;		//!< [player id] = DirtyList

	// ------------------------------------------------------------------------------

	//! for one player, whole table
	void MarkDirty( const uint8 ucPlayerId );

	//! for all players, one entry
	void MarkDirtyXY( const uint8 ucX, const uint8 ucY );

	//! for one player, one entry
	void MarkDirtyXY( const uint8 ucX, const uint8 ucY, const uint8 ucPlayerId );

	//! for all players, whole line
	void MarkDirtyY( const uint8 ucY );

	//! for one player, whole line
	void MarkDirtyY( const uint8 ucY, const uint8 ucPlayerId );

	//!
	void InsertServerSlot( const uint8 ucId );

	friend class CScriptObjectSynched2DTable;
};

#endif // _SYNCHED2DTABLE_H_