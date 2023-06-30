//////////////////////////////////////////////////////////////////////////
// This file defines a few allocators that are optimized for constantly allocating
// (small) pieces of memory that are only freed upon application exit
// I.e. the free/delete requests do nothing, but the memory is only reclaimed upon
// the character manager destruction. These allocators can be used only for
// objects that are for sure persist during the whole session (e.g. skinners)
//////////////////////////////////////////////////////////////////////////

#ifndef _CRY_ANIMATION_INCREMENTAL_CONTINUOUS_HEAP_HDR_
#define _CRY_ANIMATION_INCREMENTAL_CONTINUOUS_HEAP_HDR_

#if defined(LINUX)
# include "platform.h"
#endif

// base class for temporary array of bytes; can be used for any purpose, e.g. as a temporary
// storage between frames
class CTempStorageBase
{
public:
	// the increment in bytes, in which the block size will rise
	enum {g_nBlockIncrement = 0x1000};

	// constructs the whole thing
	void init()
	{
		m_pTemp = NULL;
		m_nSize = 0;
	}

	void* data() {return m_pTemp;}
	const void* data() const {return m_pTemp;}
	unsigned size() const {return m_nSize;}

protected:
	LPVOID m_pTemp;
	unsigned m_nSize;
};

// temporary array of bytes; can be used for any purpose, e.g. as a temporary
// storage between frames. Uses Standard allocator
class CTempStorage: public CTempStorageBase
{
public:
	// frees the tempoorary storage
	void done();

	// reserves at least the given number of bytes
	void reserve (unsigned sizeMin);
protected:
};

extern CTempStorage g_Temp;

#endif