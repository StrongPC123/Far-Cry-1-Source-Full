// XTMemFile.h interface for the CXTMemFile class.
//
// This file is a part of the Xtreme Toolkit for MFC.
// ©1998-2003 Codejock Software, All Rights Reserved.
//
// This source code can only be used under the terms and conditions 
// outlined in the accompanying license agreement.
//
// support@codejock.com
// http://www.codejock.com
//---------------------------------------------------------------------------
// Used with permission Copyright © 1999 Maxallion
// XFile@maxallion.8m.com
//---------------------------------------------------------------------------
// ICQ# 32304418
//
// CCJMemFile - Extended Memory File - Beta - w/o a lot of error checking
// - Is used like a normal CFile or CStdioFile or CMemFile object
// - String Functions : ReadString, WriteString
// - Loads physical files into memory on creation and saves them back to disk on destruction
// - Can duplicate itself to other CFile derived objects
// - Has a Search function
// - can be read-accessed like an array
//
// OVERLOADED OPERATORS:
// = Imports from another file or sets file Position
// += Appends another file
// [] reads a byte like an array
//
//////////////////////////////////////////////////////////////////////

#if !defined(__XTMEMFILE_H__)
#define __XTMEMFILE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTMemFile is a CMemFile derived class.  It is used to create a CXTMemFile
//			object to support memory files.
//
//			These memory files behave like disk files except that the file is stored
//			in RAM rather than on disk.  A memory file is useful for fast temporary
//			storage or for transferring raw bytes or serialized objects between
//			independent processes.
//
//			CXTMemFile objects can automatically allocate their own memory, or you
//			can attach your own memory block to the CXTMemFile object by calling
//			Attach. In either case, memory for growing the memory file automatically
//			is allocated in nGrowBytes-sized increments if 'nGrowBytes' is not zero.
//
//			The memory block will automatically be deleted upon destruction of the
//			CXTMemFile object if the memory was originally allocated by the CXTMemFile
//			object. Otherwise, you are responsible for de-allocating the memory you
//			attached to the object.
//
//			You can access the memory block through the pointer supplied when you
//			detach it from the CXTMemFile object by calling Detach.
//
//			The most common use of CXTMemFile is to create a CXTMemFile object and
//			use it by calling CFile member functions. Note that creating a CXTMemFile
//			automatically opens it: you do not call CFile::Open, which is only used
//			for disk files. Because CXTMemFile doesn't use a disk file, the data
//			member CFile::m_hFile is not used and has no meaning. 
//
//			The CFile member functions Duplicate, LockRange, and UnlockRange are
//			not implemented for CXTMemFile. If you call these functions on a CXTMemFile
//			object, you will get a CNotSupportedException.
//
//			CXTMemFile uses the run-time library functions malloc, realloc, and
//			free to allocate, reallocate, and deallocate memory, and the intrinsic
//			memcpy to block copy memory when reading and writing.  If you would like
//			to change this behavior or the behavior when CXTMemFile grows a file,
//			derive your own class from CXTMemFile and override the appropriate functions.
class _XT_EXT_CLASS CXTMemFile : public CMemFile  
{

public:

	// Input:	nGrowBytes  - The memory allocation increment in bytes.
    // Summary:	Constructs a CXTMemFile object.  This overload opens an empty memory
	//			file. Note that the file is opened by the constructor and that you
	//			should not call CFile::Open.
	CXTMemFile(UINT nGrowBytes = 1024 );

	// Input:	lpBuffer - Pointer to the buffer to be attached to CXTMemFile.
	//			nBufferSize - An integer that specifies the size of the buffer in bytes.
	//			nGrowBytes - The memory allocation increment in bytes.
    // Summary:	Constructs a CXTMemFile object.  This overload acts the same as
	//			if you used the first constructor and immediately called Attach with
	//			the same parameters.
	CXTMemFile(BYTE* lpBuffer,UINT nBufferSize,UINT nGrowBytes = 0);

	// Input:	lpstFilename - A string that is the path to the desired file. The path can be relative,
	//			absolute, or a network name (UNC).
	//			uiOpenFlags - A UINT that defines the file’s sharing and access mode. It specifies
	//			the action to take when opening the file. You can combine options
	//			by using the bitwise-OR ( | ) operator. One access permission and
	//			one share option are required. The modeCreate and modeNoInherit modes
	//			are optional. See the CFile constructor for a list of mode options.
    // Summary:	Constructs a CXTMemFile object.
	CXTMemFile(LPCTSTR lpstFilename,UINT uiOpenFlags);

	// Summary: Destroys a CXTMemFile object, handles cleanup and de-allocation.
    virtual ~CXTMemFile();

private:

	UINT			m_uiOpenFlags;
	bool			m_bOpen;
	CFile			m_File;
	CFileException* m_pError;

public:

    // Summary: This member function forces any data remaining in the file buffer
	//			to be written to the file. The use of Flush does not guarantee flushing
	//			of CArchive buffers. If you are using an archive, call CArchive::Flush
	//			first.
	virtual void Flush();
    
#if _MFC_VER >= 0x0700 //MFC 7.0
    using CMemFile::Open;
#endif 

	// Input:	strFilename - Specifies a NULL terminated string that is the path to the desired file.
	//			uiOpenFlags - Specifies a UINT that defines the sharing and access mode in the file.
	//			It specifies the action to take when opening the file. You can combine
	//			options by using the bitwise-OR ( | ) operator. One access permission
	//			and one share option are required. The modeCreate and modeNoInherit
	//			modes are optional. See the CFile constructor for a list of mode options.
	//			pError - Specifies a pointer to an existing file-exception object that receives
	//			the status of a failed operation.
	// Returns: true if successful, or false if it fails.
    // Summary:	This member function opens and loads a physical File into memory.
    virtual bool Open(CString strFilename, UINT uiOpenFlags, CFileException* pError = NULL);

    // Summary: This member function saves the contents of the memory to the disk
	//			and closes it.
    virtual void Close();

	// Input:	rString - A CString reference to an object to receive the string that is read.
	// Returns: TRUE if successful, or FALSE if there is an error.
    // Summary:	This member function reads a string.  
    virtual BOOL ReadString(CString& rString); 

	// Input:	lpsz  - Specifies a pointer to a buffer containing a null-terminated text
	//			string.
    // Summary:	This method writes data from a buffer to the file associated with
	//			the CArchive object. The terminating null character, \0, is not written
	//			to the file, nor is a newline character automatically written.
    virtual void WriteString( LPCTSTR lpsz );

#if _MFC_VER >= 0x0700 //MFC 7.0
    using CMemFile::Duplicate;
#endif //MFC 7.0

	// Input:	fDuplicate - A pointer to a valid CFile object.
	// Returns: true if successful, otherwise returns false.
    // Summary:	This member function will initialize the CXTMemFile object with
	//			the information specified in the 'fDuplicate' object. 
    virtual bool Duplicate(CFile *fDuplicate);

	// Input:	strDup - A NULL terminated string.
	// Returns: true if successful, otherwise returns false.
    // Summary:	This member function will initialize the CXTMemFile object with
	//			the information specified in the 'strDup' string object. 
    virtual bool Duplicate(CString strDup);

	// Returns: true if successful, otherwise returns false.
    // Summary:	This member function discards all changes to file since Open() or
	//			last Flush(). 
    virtual bool Discard();

	// Input:	fSrc - A pointer to a valid CFile object.
	//			dwSourcePos - Represents the source file position.
	//			dwDestPos - Represents the destination file position.
	//			dwBytes - Number of bytes to insert.
	// Returns: A DWORD value that represents the length of the copied bytes.
    // Summary:	This member function inserts any File and returns the length of the actual 
	//			copied bytes. 
    virtual DWORD Insert(CFile* fSrc, DWORD dwSourcePos, DWORD dwDestPos, DWORD dwBytes);

	// Input:	strSrc - Specifies a NULL terminated string that is the path to the desired
	//			file.
	//			dwSourcePos - Represents the source file position.
	//			dwDestPos - Represents the destination file position.
	//			dwBytes - Number of bytes to insert.
	// Returns: A DWORD value that represents the length of the copied bytes.
    // Summary:	This member function inserts any File and returns the length of
	//			the actual copied bytes. 
    virtual DWORD Insert(CString strSrc, DWORD dwSourcePos, DWORD dwDestPos, DWORD dwBytes);

	// Input:	fDest - A pointer to a valid CFile object.
	//			dwStartPos - Represents the starting position.
	//			dwBytes - Number of bytes to extract.
	// Returns: A DWORD value that represents the length of the copied bytes.
    // Summary:	This member function extracts bytes to a file and returns the length
	//			of the actual copied bytes. 
    virtual DWORD Extract(CFile *fDest,DWORD dwStartPos, DWORD dwBytes);

	// Input:	strDest - Specifies a NULL terminated string that is the path to the desired
	//			file.
	//			dwStartPos - Represents the starting position.
	//			dwBytes - Number of bytes to extract.
	// Returns: A DWORD value that represents the length of the copied bytes.
    // Summary:	This member function extracts bytes to a file and returns the length
	//			of the actual copied bytes. 
    virtual DWORD Extract(CString strDest, DWORD dwStartPos, DWORD dwBytes);

	// Input:	pData - Pointer to the buffer to receive the data found.
	//			dwDataLen - Size of the data to find.
	//			lStartPos - Starting position.
	// Returns: A LONG data type.
    // Summary:	This member function finds data in the file. 
    LONG FindData(void* pData, DWORD dwDataLen, LONG lStartPos);

	// Input:	fDup - A pointer to a valid CFile object.
    // Summary:	This member operator will initialize the CXTMemFile object with
	//			the object specified by 'fDup'.
    void operator =(CFile* fDup);

	// Input:	strDup - Specifies a NULL terminated string that is the path to the desired
	//			file.
    // Summary:	This member operator will initialize the CXTMemFile object with
	//			the object specified by 'strDup'.
    void operator =(CString strDup);

	// Input:	dwFilePos - DWORD value that specifies file position.
    // Summary:	This member operator will adjust the file position.
    void operator =(DWORD dwFilePos);

	// Input:	fApp - A pointer to a valid CFile object.
    // Summary:	This member operator will append the CXTMemFile object with the object
	//			specified by 'fApp'.
    void operator +=(CFile *fApp);

	// Input:	strApp - Specifies a NULL terminated string that is the path to the desired
	//			file.
	// Summary:	This member operator will append the CXTMemFile object with the object
	//			specified by 'strApp'.
    void operator +=(CString strApp);

	// Input:	dwFilePos - DWORD value that specifies file position.
    // Summary:	This member operator will perform indexing operations for the CXTMemFile
	//			object. Returns a BYTE data type.
    BYTE operator [](DWORD dwFilePos);

protected:

	// Returns: true if successful, otherwise returns false.
    // Summary:	This member function loads the file into memory. 
    virtual bool Load();  

	// Returns: true if successful, otherwise returns false.
    // Summary:	This member function saves the file to disk. 
    virtual bool Save();

	// Input:	fImp - A pointer to a valid CFile object.
	// Returns: true if successful, otherwise returns false.
    // Summary:	This member function imports the data of a CFile derived object
	//			(operator = ). 
	virtual bool Import(CFile *fImp);

	// Input:	fApp - A pointer to a valid CFile object.
	// Returns: true if successful, otherwise returns false.
    // Summary:	This member function appends a CFile derived object to the file
	//			(operator += ). 
    virtual bool Append(CFile* fApp);
};

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTMEMFILE_H__)
