// for ASCII strings up to 255 character
// character with priority 0 remain available

// References:
//    http://www.howtodothings.com/showarticle.asp?article=313
//    http://dogma.net/markn/articles/bwt/bwt.htm   (this might perform worse for small strings)

#include <vector>						// STL vector<>

//
class CStaticCharCompressor
{
public:
	//! constructor
	CStaticCharCompressor();

	//! \param indwPriority table of the priorites of the character set
	void InitFromStatistics( const DWORD indwPriority[256] );

	//! \return stream error
	bool Write( CStream &outStream, const unsigned char inChar );

	//! \return stream error
	bool Read( CStream &inStream, unsigned char &outChar );


private: // ---------------------------------------------------------

	struct SNode
	{
		//! constructor for a leaf
		SNode( const unsigned char inValue )
			:m_Value(inValue)
		{
			m_dwIndexZero=0xffffffff;m_dwIndexOne=0xffffffff;		// unused
		}

		//! constructor for a node
		SNode( const DWORD indwIndexZero, const DWORD indwIndexOne )
			:m_dwIndexZero(indwIndexZero),m_dwIndexOne(indwIndexOne)
		{
			m_Value=0;																					// unused
		}

		DWORD							m_dwIndexZero;			//!< 0xffffffff or index to the subtree for bit = 0
		DWORD							m_dwIndexOne;				//!< 0xffffffff or index to the subtree for bit = 1

		unsigned char			m_Value;						//!< is only valid for Leafes

		bool IsLeaf()
		{
			return m_dwIndexZero==0xffffffff && m_dwIndexOne==0xffffffff;
		}
	};
	
	// ------------------------------------------------------------------

	struct SPriIndex
	{
		//! constructor
		SPriIndex( const DWORD indwIndex, const DWORD indwPriority ) 
			:m_dwIndex(indwIndex),m_dwPriority(indwPriority)
		{
		}

		DWORD								m_dwIndex;					//!<
		DWORD								m_dwPriority;				//!<

		//! used for sorting (from low to hight values)
		bool operator<( const SPriIndex &inRhs ) const
		{
			return m_dwPriority<inRhs.m_dwPriority;
		}
	};

	// ------------------------------------------------------------------

	struct SBitToChar
	{
		//! constructor
		SBitToChar()
		{
			m_Bitfield=0;m_BitCount=0;
		}

		unsigned short			m_Bitfield;					//!<
		unsigned short			m_BitCount;					//!<
	};

	// ------------------------------------------------------------------

	DWORD									m_dwRootIndex;			//!< 0 if the instance was not initiated - tree is build out of elements from m_NodeMemory
	std::vector<SNode>		m_Nodes;						//!<
	SBitToChar						m_CharToBit[256];		//!<

	bool GetCharFromBits_slow( const unsigned char inVal, const DWORD indwIndex, SBitToChar &outDat );
};

