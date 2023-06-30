////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   memoryblock.h
//  Version:     v1.00
//  Created:     10/10/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __memoryblock_h__
#define __memoryblock_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "RefCountBase.h"

/** Encapsulate block of memory.
*/
class CMemoryBlock : public CRefCountBase
{
public:
	CMemoryBlock();
	CMemoryBlock( const CMemoryBlock &mem );
	~CMemoryBlock();

	CMemoryBlock& operator=( const CMemoryBlock &mem );

	//! Allocate or reallocare memory for this block.
	//! @param size Ammount of memory in bytes to allocate.
	//! @return true if allocation successed.
	bool Allocate( int size,int uncompressedSize=0 );

	//! Frees memory allocated in this block.
	void Free();

	//! Attach memory buffer to this block.
	void Attach( void *buffer,int size,int uncompressedSize=0 );
	//! Detach memory buffer that was previously attached.
	void Detach();

	//! Returns ammount of allocated memory in this block.
	int GetSize() const { return m_size; }

	//! Returns ammount of allocated memory in this block.
	int GetUncompressedSize() const { return m_uncompressedSize; }

	void* GetBuffer() const { return m_buffer; };

	//! Copy memory range to memory block.
	void Copy( void *src,int size );

	//! Compress this memory block to specified memory block.
	//! @param toBlock target memory block where compressed result will be stored.
	void Compress( CMemoryBlock &toBlock ) const;

	//! Uncompress this memory block to specified memory block.
	//! @param toBlock target memory block where compressed result will be stored.
	void Uncompress( CMemoryBlock &toBlock ) const;

	//! Serialize memory block to archive.
	void Serialize( CArchive &ar );

	//! Is MemoryBlock is empty.
	bool IsEmpty() const { return m_buffer == 0; }

private:
	void* m_buffer;
	int m_size;
	//! If not 0, memory block is compressed.
	int m_uncompressedSize;
	//! True if memory block owns its memory.
	bool m_owns;
};

SMARTPTR_TYPEDEF( CMemoryBlock );

#endif // __memoryblock_h__