//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:FileMapping
//  Declaration of class CFileMapping
//  USE_FILE_MAPPING must be defined in the project for this class to really
//  use the file mapping. Otherwise it just emulates it
//
//	History:
//	06/26/2002 :Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//////////////////////////////////////////////////////////////////////
#pragma once

////////////////////////////////////////////////////////////////
// class CFileMapping
// Generic file mapping object, is capable of mapping a file with
// a simple call of a method.
//
// NOTES:
// 
// Read-only file mapping is supported only.
//  
// Error handing is performed through examining the getData()
// address: NULL means that no file was open (no file or I/O error)
//
// No exceptions are thrown.
////////////////////////////////////////////////////////////////

class CFileMapping: public _reference_target_t
{
public:
	// Initializes an empty file mapping object
	CFileMapping();
	// initializes the object and tries to open the given file mapping
	CFileMapping (const char* szFileName, unsigned nFlags = 0);
	// closes file mapping
	~CFileMapping();

	// Retuns the size of the mapped file, or 0 if no file was mapped or the file is empty
	unsigned getSize()const;

	typedef
#ifdef USE_FILE_MAPPING
		const
#endif
		void * PData;

	// Returns the pointer to the mapped file start in memory, or NULL if the file
	// wasn't mapped
	PData getData() const;
	
	// Returns the file data at the given offset
	PData getData(unsigned nOffset) const;

#ifndef USE_FILE_MAPPING
	// sets the given (already allocated) buffer to this object
	// the memory must be allocated with malloc()
	void attach (PData pData, unsigned nSize);
#endif

	// initializes the object, opening the given file
	// if file open has failed, subsequent getData() and
	// getSize() will return zeros
	// Returns true if open was successful
	bool open (const char* szFileName, unsigned nFlags = 0);

	// closes file mapping
	void close();

protected:
	// the data of the mapped file.
	PData m_pData;
	// the mapped file size
	unsigned m_nSize;

#ifdef USE_FILE_MAPPING
	// the mapped file handle
	HANDLE m_hFile;
	// the mapped file mapping handle
	HANDLE m_hMapping;
#endif
};

TYPEDEF_AUTOPTR(CFileMapping)
