//****************************************************************************
//*   Author:  Franc Hauselmann fhauselmann@ubisoft.com
//*   Date:    04/06/02 11:25:27
 /*!  \file   DataContainer.h
  *   \brief  This class allows a user to create a data container and/or
  *            modify it.
  *
  *   This class allows a user to create a data container and/or modify it.
  *
  */
//****************************************************************************

/*!
\mainpage gs-sdk-datacontainer
\section intro Introduction

 This SDK gives a solution to exchange game configuration parameter (or data)
 between games, game service or others.
 In fact, through this SDK it's easy to share information between several
 application instances by following a common data format understandable by
 all data partenaires.

\section description Description

 To exchange game configuration parameters between games, game services or others
 application we should have a common data format understandable by all parternaires.

 The main target of this SDK is to allow users to create or/and read this common
 data format named: data container format.
 The data container format can be transported between games, game services or
 others in a simple data buffer named: data container buffer.

 Each information inserted in a data container buffer must have:

	- An unique key ID (unsigned integer with 4 bytes) : used to identified the information.
	- A data type : used to specify if it's a string type, integer type, unknow type, etc.
	- A size : for some data types who use unknown size.

 For instance if a game should publish its map name it will use a key ID = 100 with,
 a string type "DCTYPE_CHAR8" containing the name of the map "New York City".

 There is no restriction on the key name, information size. However for the data type
 it's necessary to make a choice between:

   - DCTYPE_UNKNOW	: Identifies any kind of data like pictures, sounds or others.<br>
   - DCTYPE_CHAR8   : Identifies a string data with a NULL terminator (8 bits per characters).<br>
   - DCTYPE_CHAR16  : Identifies a string data with a NULL terminator (16 bits per characters).<br>
   - DCTYPE_INT32   : Identifies a signed integer data (32 bits).<br>
   - DCTYPE_UINT32  : Identifies an unsigned integer data.<br>
   - DCTYPE_BOOL    : Identifies a boolean data (32 bits).<br>
   - DCTYPE_DOUBLE  : Identifies a floating-point variable (64 bits).<br>

 All services available in this SDK is in this version given by the CDataContainer
 class. Through this class you can handle data, key ID, data container buffer.
 The CDataContainer allows to:

   - Adding data in a data container buffer.<br>
   - Getting data from a data container buffer.<br>
   - Removing data in a data container buffer.<br>
   - Merging a data container buffer to another.<br>
   - Copying a data container buffer in another.<br>

 The GSDataContainer class allows to work on a fraught or empty data container.
 In any time you can attach a fraught data container buffer to an CDataContainer
 intance to handle it or you can detach the current data container buffer managing by
 a CDataContainer instance.

 For more information see the SDK API documentation.

\section see_also See Also
<a href=[relative URL here]>- EMPTY -</a><br>
*/


#if !defined(AFX_DATACONTAINER_H__608AB0A4_0EAD_4B20_A6AC_71C12B96547F__INCLUDED_)
#ifndef DOX_SKIP_THIS
	#define AFX_DATACONTAINER_H__608AB0A4_0EAD_4B20_A6AC_71C12B96547F__INCLUDED_
#endif // DOX_SKIP_THIS

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef GS_WIN32
#pragma warning (disable : 4786)
#endif GS_WIN32

#include "GSTypes.h"
#include "GSErrors.h"

// Usable only under WIN32 platform
#ifdef GS_WIN32
#include <windows.h>
#include <Oaidl.h>
#endif GS_WIN32


//! the data container namespace
namespace GSDataContainer
{
const GSuint DCTYPE_UNKNOW	= 0;	/*!< Identifies an unknow data */
const GSuint DCTYPE_CHAR8	= 1; 	/*!< Identifies a string data with a NULL terminator (8 bits per characters) */
const GSuint DCTYPE_CHAR16	= 2;	/*!< Identifies a string data with a NULL terminator (16 bits per characters) */
const GSuint DCTYPE_INT32	= 3;	/*!< Identifies a signed integer data (32 bits) */
const GSuint DCTYPE_UINT32	= 4;	/*!< Identifies an unsigned integer data */
const GSuint DCTYPE_BOOL	= 5;	/*!< Identifies a boolean data (32 bits) */
const GSuint DCTYPE_DOUBLE  = 6;	/*!< Identifies a floating-point variable (64 bits) */

//! The data container class
/*!
The data container class is the common data structure to be used when sending or receiving information
about a room, a lobby, a player, etc. The purpose of this structure is to make sure all parties involved
in the match-making process (the game, the Game Service Client, the ubi.com servers) can unpack and
understand the content of the data container.
*/
class CDataContainer
{
public:
/*! @defgroup group1 Construction/Destruction
\brief The construction/destruction methods
    @{
*/

	//============================================================================
	// Function CDataContainer
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:13:54
	/*!
	 \brief	 Defaut constructor.
	 \par       Description:
	 Defaut constructor. Initializes the instance and keeps the instance ready to 
	 construct a new data container or attach a valid data container.
	 \return    No return value
	*/
	//============================================================================
	CDataContainer();

	//============================================================================
	// Function CDataContainer
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:13:54
	/*!
	 \brief	 Constructor used to initialize an instance with a data container buffer.
	 \par       Description:
	 Constructor used to initialize an instance with a data container buffer. You can
	 create an instance of the CDataContainer directly with a raw data container buffer received by
	 an internet connection, GSClientSPW or others.

	 \return    No return value

     \param		pBuffer	The buffer used to initialize the data container

	*/
    //============================================================================
	CDataContainer(const GSvoid* pBuffer);

// Available only on a WIN32 platform
#ifdef GS_WIN32

	//============================================================================
	// Function CDataContainer
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 06/06/02 14:25:21
	/*!
	 \brief	 Constructor used to initialize an instance with a data container buffer
	         stored in a VARIANT (This method is only available for WIN32 platform).
	 \par       Description:
	 Constructor used to initialize an instance with a data container buffer stored
	 in a VARIANT. You can directly use the VARIANT returned by a "Game Config
	 Service".
	 The data container buffer must be stored in a SAFEARRAY stored in a VARIANT
	 with the following types: VT_ARRAY | VT_UI1.
	 THIS METHOD IS ONLY AVAILABLE ON PLATEFORM WIN32.

	 \return    No return value

     \param		pVarDataContainer [in] The VARIANT with a data container buffer
									   used to initialize the current instance.

	*/
    //============================================================================
	CDataContainer(const VARIANT* pVarDataContainer);

#endif // GS_WIN32

	//============================================================================
	// Function CDataContainer
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:13:54
	/*!
	 \brief	 Destructor.
	 \par       Description:
	 Destructor. Releases all memory allocated by one instance.

	 \return    No return value
	*/
	//============================================================================
	virtual ~CDataContainer();

/*! @} end of group1 */

/*! @defgroup group2 Managing Data Container
\brief Managin the data container instance (attach buffer, reset data, etc).
    @{
*/

	//============================================================================
	// Function Attach
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Attaches a data container buffer to the current instance.
	 \par       Description:
	 Resets the current instance and attaches a data container buffer to the current instance.
	 The buffer in input will NOT be delete !!
	 This method support buffer compressed or not (See Detach for more information).

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>


	 \retval	GSE_INVALIDARG		pBuffer is invalid (NULL).
	 \retval	GSE_BUFFERNOTVALID	pBuffer points on an invalid data container buffer.
	 \retval	GSE_OUTOFMEMORY		out of memory to complete the operation.
	 \retval	GSS_OK				Success.

	 \param	pBuffer	[in] Must points on a valid data container buffer (row data).
	                     The current instance will not modify data pointed by pBuffer.
						 The instance works on a copy.
	*/
	//============================================================================
	GSRESULT Attach(const GSvoid* pBuffer);

// Available only on a WIN32 platform
#ifdef GS_WIN32

	//============================================================================
	// Function CDataContainer
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 06/06/02 14:25:21
	/*!
	 \brief	 Attaches a data container buffer stored in a VARIANT to the current
	         instance. (This method is only available for WIN32 platform).
	 \par       Description:
	 Resets the current instance and copy a data container buffer stored in 
	 a variant to the current instance. 
	 The data container must be stored in a SAFEARRAY stored in
	 a VARIANT with the following types: VT_ARRAY | VT_UI1.
	 The Variant in input will not be delete !!
	 THIS METHOD IS ONLY AVAILABLE ON PLATEFORM WIN32.
	 This method support buffer compressed or not (See Detach for more information).

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>

     \param		pVarDataContainer [in] The VARIANT with a data container buffer.

	 \retval	GSE_INVALIDARG		pVarDataContainer is invalid (NULL) or is invalid.
	 \retval	GSE_OUTOFMEMORY		out of memory to complete the operation.
	 \retval	GSS_OK				Success.

	*/
    //============================================================================
	GSRESULT Attach(const VARIANT* pVarDataContainer);

#endif // GS_WIN32

	//============================================================================
	// Function CDataContainer
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 06/06/02 14:25:21
	/*!
	 \brief	 Returns a pointer on the internal byte buffer used to store the
	 data container buffer. YOU MUST NOT CHANGE THE CONTENT OF THIS BUFFER.
	 (This buffer can be compressed or not)

	 \par       Description:
	 Returns a pointer on the internal byte buffer used to store the
	 data container buffer. YOU MUST NOT CHANGE THE CONTENT OF THIS BUFFER.
	 The internal byte buffer use the data container format to store the
	 data container content. You can use the buffer returned to make a copy, send it, 
	 attach it in a new datacontainer, etc.
	 
	 \return
	 An internal buffer. PLEASE don't modify the buffer content (never release it or 
	 write in it).
	 This pointer can become invalid if you call another method of the data container
	 class.

	 \retval	NULL	If an error.

	 \param		bCompressData [in] If bCompressData is GS_TRUE, the method returns 
	                               a pointer on the internal compressed buffer. A buffer
								   compressed can be very usful if you send it 
								   (reduce the tranmission time and the bandwith). 
								   By default the buffer is compressed.

	*/
    //============================================================================
	const GSvoid* GetByteBuffer(GSbool bCompressData = GS_TRUE);

	//============================================================================
	// Function CDataContainer
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 06/06/02 14:25:21
	/*!
	 \brief	 Returns the size of the internal byte buffer used to store the
	 data container buffer. 

	 \par       Description:
	 Returns the size of the internal buffer used to store the
	 data container buffer. To get the internal buffer or a copy, you can use 
	 the following functions: GetByteBuffer(), Detach() or GetCopy().
	 The size returned is in bytes.

	 \return
	 Returns the size in byte of the internal buffer.

	 \retval	0 if an error or empty.

	 \param		bCompressData [in] If bCompressData is GS_TRUE, the method returns 
	                               the size of the compressed buffer. A buffer
								   compressed can be very usful if you send it 
								   (reduce the tranmission time and the bandwith). 
								   By default the buffer is compressed.


	*/
    //============================================================================
	GSuint GetByteBufferSize(GSbool bCompressedData = GS_TRUE);

	//============================================================================
	// Function Detach
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Detaches the current data container buffer from this current instance.
	 \par       Description:
	 Fills in a buffer with the current data container buffer managed by the instance.
	 After the operation, the current instance will be reseted. The data container
	 will be empty and ready to receive a new data container or new data.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>

	 \retval	GSE_INVALIDARG		pulBufferSize is invalid (NULL).
	 \retval	GSE_FAIL			If the buffer is too small.
	 \retval	GSS_OK				Success.

	\param	pBuffer			[out] Points on a buffer filled in by the instance. The buffer
	                              must be enough large to receive all data from the data 
								  container.
								  This parameter can be NULL if you want just to know 
								  the size of all data in the data container (same
								  as GetByteBufferSize()).

	\param	pulBufferSize	[in/out] In input this parameter has the size of the buffer pointed
									 by pBuffer. In output this parameter returns the number of 
									 bytes filled in the buffer pointed by pBuffer.
									 This parameter cannot be NULL.

	 \param		bCompressData [in] If bCompressData is GS_TRUE, the method 
								   fills in the pBuffer with a compressed content.
								   A buffer compressed can be very usful if you send it 
								   (reduce the tranmission time and the bandwith). 
								   By default the buffer is compressed.
	*/
	//============================================================================
	GSRESULT Detach(GSvoid* pBuffer, GSuint* pulBufferSize, GSbool bCompressData = GS_TRUE);

// Available only on a WIN32 platform
#ifdef GS_WIN32

	//============================================================================
	// Function GetCopy
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 06/06/02 14:25:21
	/*!
	 \brief	 Copies the current data container in a variant. 
			 (This method is only available for WIN32 platform).

	 \par       Description:
	 Fills in a VARIANT with the current data container buffer managed by the instance.
	 The VARIANT filled in can be directly used with the "Game Config Service".
	 The data container will be stored in a SAFEARRAY stored in
	 a VARIANT with the following types: VT_ARRAY | VT_UI1.
	 Don't forget to call ::VariantClear() when the VARIANT will not be used anymore.
	 THIS METHOD IS ONLY AVAILABLE ON PLATEFORM WIN32.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>

	 \retval	GSE_INVALIDARG		pVarDataContainer is invalid (NULL).
	 \retval	GSE_OUTOFMEMORY		No more memory to complete the operation.
	 \retval	GSE_FAIL			If the buffer is too small.
	 \retval	GSS_OK				Success.

     \param		pVarDataContainer [in,out] The VARIANT will be cleared and filled
										   in with a SAFEARRAY filled with the
										   data container buffer content.

	 \param		bCompressData [in] Compress, if possible, data returned in the VARIANT.
	                               A buffer compressed can be very usful if you send it 
								   (reduce the tranmission time and the bandwith). 
								   However it's take some CPU resources to 
								   compress it.
								   By default the buffer is compressed.
	*/
    //============================================================================
	GSRESULT GetCopy(VARIANT* pVarDataContainer, GSbool bCompressData = GS_TRUE);

#endif // GS_WIN32

	//============================================================================
	// Function Reset
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Resets the current instance.
	 \par       Description:
	 Resets the current instance. Deletes all data stored in the current instance and
	 keeps the instance ready to work with a new data container or new data.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>

	 \retval	GSS_OK	Success.
	*/
	//============================================================================
	GSRESULT Reset();

/*! @} end of group2 */

/*! @defgroup group3 Keys information
\brief Functions to get info about the keys
    @{
*/

	//============================================================================
	// Function GetDataKeyNumber
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Returns the number of key stored in the current instance.
	 \par       Description:
	 Returns the number of key stored in the current instance.

	 \return    Returns the number of key stored in the current instance.

	 \retval	0..N :	Number of key stored in the current instace.

	*/
	//============================================================================
	GSuint GetDataKeyNumber() const;

	//============================================================================
	// Function EnumDataKey
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Enumerates the keys stored in the current data container.
	 \par Description:
	 Enumerates the keys stored in the current data container. This enumerator use
	 a zero index (0 = first key, 1 = second key, ...). <br>
	 It's also possible to know the data type associated with each key. <br>
	 Types available: <br>

	 DCTYPE_UNKNOW	=> Unknow data type. <br>
	 DCTYPE_CHAR8	=> String with NULL character (8 bits per characters) type. <br>
	 DCTYPE_CHAR16	=> String with NULL character (16 bits per characters) type. <br>
	 DCTYPE_INT32	=> Integer (signed 32 bits) type. <br>
	 DCTYPE_UINT32	=> Unsigned integer (32 bits) type. <br>
	 DCTYPE_BOOL	=> Boolean type type. <br>
	 DCTYPE_DOUBLE	=> Floating-point variable (64 bits). <br>

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>


	 \retval	GSE_INVALIDARG			If pulDataType is NULL and pulKeyBufferSize is NULL.
	 \retval	GSE_INVALIDINDEX		If ulIndex is too big !
	 \retval	GSS_KEYBUFFERTOOSMALL	If key name buffer is too small.
	 \retval	GSS_OK					Success

	 \param	ulIndex				[in] Zero index used to select a specific key. To know
	                                 the number of key available you can use GetDataKeyNumber().

	 \param	punKey			   [out] A pointer on a GSuint used to read the Key. The function 
	                                 fill-in the (*punKey) variable.
									 Can be NULL if the call will just read the data type.
	 
	 \param	pulDataType			[out] Returns the data type of the value. Can be NULL.
	*/
	//============================================================================
	GSRESULT EnumDataKey(GSuint ulIndex, GSuint* punKey, GSuint* pulDataType) const;


	//============================================================================
	// Function FindKey
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 04/07/02 17:12:00
	/*!
	 \brief	 Checks if a specific key is in the current instance. 	 
	 \par Description:
	 Checks if a specific key is in the current instance. If yes, the find function
	 can return the data type.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>

	 \retval	GSE_INVALIDKEY			The key was not found.
	 \retval	GSS_OK					Success. The key was found

	 \param	unKey			[in]  The key value.
	 \param	pulDataType		[out] Returns the data type associated to the key.
								  Can be NULL.

	*/
	//============================================================================
	GSRESULT FindKey(GSuint unKey, GSuint* pulDataType) const;

/*! @} end of group3 */

/*! @defgroup group4 Adding data
\brief Methods to add data
    @{
*/
	//============================================================================
	// Function Add
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Adds a string in the current data container.
	 \par       Description:
	 Adds a string in the current data container. The data type used to store the 
	 string will be DCTYPE_CHAR8.
	 If a data with a same key already exist this method will replace the data by the
	 current one.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>

	 \retval	GSE_INVALIDARG		unKey is NULL or czData is NULL.
	 \retval	GSE_OUTOFMEMORY		Out of memory !
	 \retval	GSS_OK				Success.

	 \param	unKey	[in] The key value. If the key already exist, the old value will be replaced by the new one.
	 \param	czData	[in] The string to store (1 byte per character with a null character
						 at the end).
	*/
	//============================================================================
	GSRESULT Add(GSuint unKey, const GSchar* czData);

	//============================================================================
	// Function Add
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Adds an integer in the current data container.
	 \par Description:
	 Adds an integer (signed and 32 bits) in the current data container. The data type used
	 to store the integer will be DCTYPE_INT32.
	 If a data with a same key already exist this method will replace the data by the
	 current one.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value 
	 (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>

	 \retval	GSE_INVALIDARG		unKey is NULL.
	 \retval	GSE_OUTOFMEMORY		Out of memory !
	 \retval	GSS_OK				Success.

	 \param	unKey	[in] The key value. If the key already exist, the old value 
						 will be replaced by the new one.
	 \param	lData	[in] The integer value (32 bits and signed).
	*/
	//============================================================================
	GSRESULT Add(GSuint unKey, const GSint lData);

	//============================================================================
	// Function Add
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Adds an unsigned integer in the current data container.
	 \par Description:
	 Adds an unsigned (32 bits) in the current data container. The data type used
	 to store the unsigned integer will be DCTYPE_UINT32.
	 If a data with a same key already exist this method will replace the data by the
	 current one.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>


	 \retval	GSE_INVALIDARG	unKey is NULL.
	 \retval	GSE_OUTOFMEMORY	Out of memory !
	 \retval	GSS_OK			Success.

	 \param	unKey	[in] The key value. If the key already
	                     exist, the old value will be replaced by the new one.
	 \param	ulData	[in] The unsigned integer value (32 bits).
	*/
	//============================================================================
	GSRESULT Add(GSuint unKey, const GSuint ulData);

	//============================================================================
	// Function Add
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Adds a boolean in the current data container.
	 \par Description:
	 Adds a boolean in the current data container. The data type used to store 
	 the boolean will be DCTYPE_BOOL.
	 If a data with a same key already exist this method will replace the data by the
	 current one.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>

	 \retval	GSE_INVALIDARG	unKey is NULL.
	 \retval	GSE_OUTOFMEMORY Out of memory !
	 \retval	GSS_OK			Success.

	 \param	unKey	[in] The key value. If the key already
	                     exist, the old value will be replaced by the new one.
	 \param	bData	[in] The boolean value.
	*/
	//============================================================================
	GSRESULT Add(GSuint unKey, const GSbool bData);

	//============================================================================
	// Function Add
	// Author: Jose Covatta jcovatta@ubisoft.com
	// Date: 19/07/02 14:21:24
	/*!
	 \brief	 Adds a floating-point variable (64 bits) in the current data container.
	 \par       Description:
	 Adds a floating-point variable (64 bits) in the current data container. The 
	 data type used to store the boolean will be DCTYPE_DOUBLE.
	 If a data with a same key already exist this method will replace the data by the
	 current one.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>

	 \retval	GSE_INVALIDARG	unKey is NULL.
	 \retval	GSE_OUTOFMEMORY Out of memory !
	 \retval	GSS_OK			Success.

	 \param	unKey	[in] The key value. If the key already exist, the old value 
					will be replaced by the new one.
	 \param	dData	[in] The floating-point value (64 bits).
	*/
	//============================================================================
	GSRESULT Add(GSuint unKey, const GSdouble dData);

	//============================================================================
	// Function Add
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Adds a data buffer in the current data container.
	 \par       Description:
	 Adds a data buffer in the current data container. The data type used
	 to store the boolean will be DCTYPE_UNKNOW.
	 If a data with a same key already exist this method will replace the data by the
	 current one.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>

	 \retval	GSE_INVALIDARG	czKey is NULL or pBuffer is NULL.
	 \retval	GSE_OUTOFMEMORY	Out of memory !
	 \retval	GSS_OK			Success.

	 \param	unKey			[in] The key value. If the key already
								 exist, the old value will be replaced by the new one.
	 \param	pBuffer			[in] The data buffer to store (32 bits).
	 \param	ulBufferSize	[in] The data buffer size (in byte).
	*/
	//============================================================================
	GSRESULT Add(GSuint unKey, const GSvoid* pBuffer, const GSuint ulBufferSize);

	//============================================================================
	// Function Add
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Adds a value identified by a key in the current data container.
	 \par       Description:
	 Adds a value identified by a key in the current instance. The value can be
	 of the following type : DCTYPE_UNKNOW, DCTYPE_CHAR8, DCTYPE_CHAR16,
	 DCTYPE_INT32, DCTYPE_UINT32, DCTYPE_BOOL, DCTYPE_DOUBLE. <br>
	 If a data with a same key already exist this method will replace the data by the
	 current one.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>


	 \retval	GSE_INVALIDARG	czKey or pBuffer are NULL.
	 \retval	GSE_OUTOFMEMORY Out of memory !
	 \retval	GSS_OK			Success.

	 \param	unKey			[in] The key value. If the key already
							     exist, the old value will be replaced by the new one.
	 \param	pBuffer			[in] The data buffer to store (32 bits).
	 \param	ulBufferSize	[in] The data buffer size (in byte). If you use the DCTYPE_CHAR8 or
	                             or DCTYPE_CHAR16 type you need to include the NULL in the size.
	 \param	ulDataType		[in] The data type. (DCTYPE_UNKNOW, DCTYPE_CHAR8,
		                         DCTYPE_CHAR16, DCTYPE_INT32, DCTYPE_UINT32, DCTYPE_BOOL,
								 DCTYPE_DOUBLE).
	*/
	//============================================================================
	GSRESULT Add(GSuint unKey, const GSvoid* pBuffer, const GSuint ulBufferSize, const GSuint ulDataType);

/*! @} end of group4 */


/*! @defgroup group5 Getting data
\brief methods to retrieve data from the data container.
    @{
*/

	//============================================================================
	// Function Get
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	Gets an integer value associated by a key in the current data container.
	 \par Description:
	 Get an integer value identified by a key in the current instance. For more
	 information see Add() method.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>

	 \retval	GSE_INVALIDARG	czKey is NULL.
	 \retval	GSE_INVALIDKEY	The key is invalid (not found)
	 \retval	GSS_OK			Success

	 \param	unKey	[in] The key who identifies the value to return.
	 \param	plData	[out] Buffer to fill in. (32 bits signed).

	*/
	//============================================================================
	GSRESULT Get(GSuint unKey, GSuint* plData) const;

	//============================================================================
	// Function Get
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Gets an unsigned integer value identified by a key in the current instance.
	 \par Description:
	 Gets an unsigned integer value identified by a key in the current instance. For more
	 information see Add() method.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>

	 \retval	GSE_INVALIDARG	czKey is NULL.
	 \retval	GSE_INVALIDKEY	The key is invalid (not found)
	 \retval	GSS_OK			Success

	 \param	unKey	[in] The key who identifies the value to return.
	 \param	pulData	[out] Buffer to fill in (32 bits unsigned).
	*/
	//============================================================================
	GSRESULT Get(GSuint unKey, GSint* pulData) const;

	//============================================================================
	// Function Get
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Gets a boolean value identified by a key in the current instance.
	 \par       Description:
	 Gets a boolean value identified by a key in the current instance. For more
	 information see Add() method.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>

	 \retval	GSE_INVALIDARG	czKey is NULL.
	 \retval	GSE_INVALIDKEY	The key is invalid (not found)
	 \retval	GSS_OK			Success

	 \param	unKey	[in] The key who identifies the value to return.
	 \param	pbData	[out] Buffer to fill in (32 bits).
	*/
	//============================================================================
	GSRESULT Get(GSuint unKey, GSbool* pbData) const;

	//============================================================================
	// Function Get
	// Author: Jose Covatta jcovatta@ubisoft.com
	// Date: 19/07/02 14:25:34
	/*!
	 \brief	 Gets a floating-point value (64 bits) identified by a key in the current instance.
	 \par       Description:
	 Gets a floating-point value (64 bits) identified by a key in the current instance. For more
	 information see Add() method.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>

	 \retval	GSE_INVALIDARG	czKey is NULL.
	 \retval	GSE_INVALIDKEY	The key is invalid (not found)
	 \retval	GSS_OK			Success

	 \param	unKey	[in] The key who identifies the value to return.
	 \param	pdData	[out] Buffer to fill in (64 bits).
	*/
	//============================================================================
	GSRESULT Get(GSuint unKey, GSdouble* pdData) const;

	//============================================================================
	// Function Get
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Gets an string or a data buffer identified by a key in the current instance.
	 \par       Description:
	 Gets an string or a data buffer identified by a key in the current instance.
	 For more information see Add() method.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>


	 \retval	GSE_INVALIDARG		czKey is NULL.
	 \retval	GSE_INVALIDKEY		The key is invalid (not found)
	 \retval	GSS_BUFFERTOOSMALL	No error. The buffer used to receive data
	                                is too small.
 	 \retval	GSS_OK (SUCCESS CODE): Success.

	 \param	unKey			[in] The key who identifies the value to return.
	 \param	pBuffer			[out] Buffer to fill in. You can set this pointer to
	                              NULL if you want just to know the size of the value
								  (returned by pulBufferSize).
	 \param	pulBufferSize	[in/out] You have to specify the size of the buffer
						             pointed by pBuffer. If you want to know the size
									 required to store the data you can also used the
									 value returned by pulBufferSize and set pBuffer
									 to NULL.
	*/
	//============================================================================
	GSRESULT Get(GSuint unKey, GSvoid* pBuffer, GSuint* pulBufferSize) const;

	//============================================================================
	// Function Get
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Gets a data buffer identified by a key in the current instance.
	 \par       Description:
	 Gets a data buffer identified by a key in the current instance.
	 For more information see Add() method.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>


	 \retval	GSE_INVALIDARG		czKey is NULL.
	 \retval	GSE_INVALIDKEY		The key is invalid (not found)
	 \retval	GSS_BUFFERTOOSMALL	No error. The buffer used to receive data
	                                is too small.
 	 \retval	GSS_OK				Success.

	 \param	unKey			[in] The key who identifies the value to return.
	 \param	pBuffer			[out] Buffer to fill in. You can set this pointer to
								  NULL if you want just to know the size of the value
								  (returned by pulBufferSize).
	 \param	pulBufferSize	[in/out] You have to specify the size of the buffer pointed
	                                 by pBuffer. If you want to know the size required
									 to store the data you can also used the value
									 returned by pulBufferSize and set pBuffer to NULL.
									 This parameter can be NULL only if pBuffer is NULL.
	 \param	pulDataType	[out] Returns the data type of the data identified by the key name
	                          (DCTYPE_UNKNOW, DCTYPE_CHAR8, DCTYPE_CHAR16, DCTYPE_INT32,
	                          DCTYPE_UINT32, DCTYPE_BOOL, DCTYPE_DOUBLE).
							  You can set this pointer to NULL.
	*/
	//============================================================================
	GSRESULT Get(GSuint unKey, GSvoid* pBuffer, GSuint* pulBufferSize, GSuint* pulDataType) const;

/*! @} end of group5 */

/*! @defgroup group6 Removing data
\brief Methods to remove data from the data container.
    @{
*/

	//============================================================================
	// Function Remove
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Removes a value identified by a key in the current instance.
	 \par       Description:
	 Removes a value identified by a key in the current instance. All data linked with
	 the key (include the key) will be deleted.

	 \return
	 GET_GSRESULTID(ReturnValue) macro extracts the operation code value (error code or success code). <br>
	 GSFAILED(ReturnValue) macro returns true if it's an error. <br>
	 GSSUCCEEDED(ReturnValue) macro returns true if it's a success. <br>

	 \retval	GSE_INVALIDARG	czKey is NULL.
	 \retval	GSE_INVALIDKEY	The key is invalid (not found)
	 \retval	GSS_OK			Success

	 \param		unKey	[in] The key who identifies the value to delete.

	*/
	//============================================================================
	GSRESULT Remove(GSuint unKey);

/*! @} end of group6 */

/*! @defgroup group7 Operators
\brief Operators available to manipulate data containers.
    @{
*/

	//============================================================================
	// Function operator+=
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Adds CDataContainer in the current instance.
	 \par       Description:
	 Adds CDataContainer in the current instance. If a key already
	 exist in the current instance, the value will be replaced.

	 \return    Returns a copy of the current CDataContainer instance.

	 \param DataContainer	[in] a CDataContainer instance
	*/
	//============================================================================
	CDataContainer& operator+=(const CDataContainer& DataContainer);

	//============================================================================
	// Function operator+=
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Adds row data container buffer in the current instance.
	 \par       Description:
	 Adds row data container buffer in the current instance. If a key already
	 exist in the current instance, the value will be replaced.

	 \return    Returns a copy of the current CDataContainer instance.

	 \param pBuffer		[in] A raw data container buffer.
	*/
	//============================================================================
	CDataContainer& operator+=(const GSvoid* pBuffer);

	//============================================================================
	// Function operator=
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Makes a copy of a CDataContainer instance.
	 \par       Description:
	 Makes a copy of a CDataContainer instance.

	 \return    Returns a copy of the current CDataContainer instance.

	 \param DataContainer	[in] A CDataContainer instance.
	*/
	//============================================================================
	CDataContainer& operator=(const CDataContainer& DataContainer);

	//============================================================================
	// Function operator==
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Checks if a container has the same content as another.
	 \par       Description:
	 Checks if a container has the same content as another.

	 \return    Returns true if the DataContainer have the same content.

	 \param DataContainer	[in] A CDataContainer instance.
	*/
	//============================================================================
	bool operator==(CDataContainer& DataContainer);

	//============================================================================
	// Function operator!=
	// Author: Franc Hauselmann fhauselmann@ubisoft.com
	// Date: 03/06/02 17:22:41
	/*!
	 \brief	 Checks if a container has not the same content as another.
	 \par       Description:
	 Checks if a container has not the same content as another.

	 \return    Returns true if the DataContainer haven't the same content.

	 \param DataContainer	[in] A CDataContainer instance.
	*/
	//============================================================================
	bool operator!=(CDataContainer& DataContainer);


/*! @} end of group7 */



#ifndef DOX_SKIP_THIS // skip documenting protected and private methods

// Protected methods
protected:

	GSRESULT SerializeWithStreamCompression();
	GSRESULT UnserializeFromhStreamCompression();

	GSRESULT ResetBuffer();
	GSRESULT ResetZBuffer();
	GSRESULT ResetInternalStructure();

	GSRESULT GetNextChunk(GSubyte** ppCurrentChunk);
	GSRESULT GetChunk(GSubyte** ppCurrentChunk, GSuint ChunkID);

	GSRESULT IsBufferCompressed(const GSvoid* pBuffer);
	GSRESULT CompressBuffer();
	GSRESULT UnCompressBuffer(const GSvoid* pvBufferToCompress);

	GSRESULT ReadStreamHeader(GSubyte** ppCurrentStreamPosition,
							  GSubyte* pCurrentBitPosition,
							  GSuint* punStreamSize, 
							  GSuint* punKeyAvailable, 
							  GSubyte* pucVersion,
							  GSuint* punSmallestKeyValue);
// Protected members
protected:

	// Used to manage a dynamically map.
	// We don't use a STL map because we want not to have problem between
	// STL implementations.
	GSvoid*			m_pDataMap;

	// Buffer to Serialize/UnSerialize
	GSvoid*			m_pBuffer;
	GSuint			m_ulBufferSize;

	// Flag to indicate if m_pBuffer is uptodate or note.
	GSbool			m_bUptoDate;

	// Buffer used to compress/Uncompress m_pBuffer.
	GSvoid*			m_pZBuffer;
	GSuint			m_ulZBufferSize;
	GSvoid*			m_pZBufferStartCursor;

	// Flag to indicate if m_pZBuffer is uptodate or note.
	GSbool			m_bBufferSynchronizedWithZBuffer;

#endif // DOX_SKIP_THIS protected and private methods
};


} // namespace GSDataContainer

#ifndef DOX_SKIP_THIS
	#endif // !defined(AFX_DATACONTAINER_H__608AB0A4_0EAD_4B20_A6AC_71C12B96547F__INCLUDED_)
#endif // DOX_SKIP_THIS
