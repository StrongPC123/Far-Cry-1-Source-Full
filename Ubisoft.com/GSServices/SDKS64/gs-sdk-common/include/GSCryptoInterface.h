//****************************************************************************
//*   Author:  Guillaume Plante, Philippe Lalande gsdevelopers@ubisoft.com
//*   Date:    2002-05-07 16:18:32
 /*!  \file   GSCryptoInterface.h
  *   \brief  Interface ubi.com's cryptographic library.
  *
  *   This interface provides one-way hash,
  *   synchronous block cipher and pseudo-random number generator
  *   functionality to be use along with the ubi.com crypto interface.
  */
//****************************************************************************

/*!
\mainpage gs-sdk-crypto
\section intro Introduction
 ubi.com's cryptographic interface.

\section description Description
 The cryptographic interface contains one-way hash,
 synchronous block cipher and pseudo-random number generator
 functionality to be use along with the ubi.com interface.
*/

#ifndef __GSCRYPTOINTERFACE_H__
#define __GSCRYPTOINTERFACE_H__

#include "GSTypes.h"

#include "GSCryptoDefines.h"

extern "C" {

//============================================================================
// Function InitializeCrypto
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-05-07 15:49:51
/*!
 \brief	 Initialize library
 \par       Description:
 Initialize library

 \return    Status of the function call

 \retval	GS_TRUE	the operation suceeded.
 \retval	GS_FALSE the operation failed.


*/
//============================================================================
// plalande (01/2004)
// Not necessary. Does no do anything
//GSbool __stdcall InitializeCrypto();

//============================================================================
// Function UninitializeCrypto
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-05-07 15:49:52
/*!
 \brief	 Uninitialize library
 \par       Description:
 Uninitialize library

 \return    void
*/
//============================================================================
// plalande (01/2004)
// Very dangerous to use these functions since they affect all of the
// cryptographic handles existing in the library
//GSvoid __stdcall UninitializeCrypto();

/*! @defgroup groupcrypto1 One-way hash functions
\brief One-way hash functions

These functions are used to get cryptographic checksums
from supplied input buffers.
    @{
*/


//============================================================================
// Function GenerateRawHash
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-05-07 15:49:53
/*!
 \brief	 Generate binary hash
 \par    Description:
 This function generate a hash in binary form
 from a supplied input value. The output buffer must be at
 least (SHA1_DIGESTSIZE) byte long for the SHA1 (E_SHA1) algorithm
 and at leat (MD5_DIGESTSIZE) byte long for the MD5 (E_MD5) algorithm.
 The number of iteration can be modified to change the hashed
 output value, it represent the number of passed the input value 
 is put through the hash algorithm.

 \return    Status of the function call

 \retval	GS_TRUE	the operation suceeded.
 \retval	GS_FALSE the operation failed.

 \param	IN eAlgo	The type of hash algorithm to use.
 \param	IN pucInBuffer	The input buffer to be hashed.
 \param	IN uiBufferSize	The lenght of the input buffer.
 \param	OUT pucOutBuffer	Binary output of the generated hashed value.
 \param	IN uiIterations 	Nuber of time to hash the input value.

*/
//============================================================================
GSbool __stdcall GenerateRawHash(GSCRYPTO_HASH_ALGO eAlgo,
                                 const GSubyte* pucInBuffer,
                                 GSuint uiBufferSize,
                                 GSubyte* pucOutBuffer,
                                 GSuint uiIterations = 1);


//============================================================================
// Function GenerateHexaHash
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-05-07 15:49:53
/*!
 \brief	 Generate hexadecimal hash
 \par    Description:
 This function generate a hash in hexadecimal form
 from a supplied input value. The output buffer must be at
 least (SHA1_HEXASIZE) byte long for the SHA1 (E_SHA1) algorithm
 and at leat (MD5_HEXASIZE) byte long for the MD5 (E_MD5) algorithm.
 It is important to note that this function does not put a \0 at the end
 of the output buffer, so the programmer will have to terminate the output
 string once he gets it.
 The number of iteration can be modified to change the hashed
 output value, it represent the number of passed the input value 
 is put through the hash algorithm.

 \return    Status of the function call

 \retval	GS_TRUE	the operation suceeded.
 \retval	GS_FALSE the operation failed.

 \param	IN eAlgo	The type of hash algorithm to use.
 \param	IN pucInBuffer	The input buffer to be hashed.
 \param	IN uiBufferSize	The lenght of the input buffer.
 \param	OUT pszOutBuffer	Hexadecimal output of the generated hashed value.
 \param	IN uiIterations = 1	Nuber of time to hash the input value.

*/
//============================================================================
GSbool __stdcall GenerateHexaHash(GSCRYPTO_HASH_ALGO eAlgo,
                                  const GSubyte* pucInBuffer,
                                  GSuint uiBufferSize,
                                  GSchar* pszOutBuffer,
                                  GSuint uiIterations = 1);   


//============================================================================
// Function InitializeHash
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-05-07 16:27:15
/*!
 \brief	 Initialize the hash context 
 \par       Description:
 This function will initialize the hash context. This must be called before
 any call to the UpdateHash() function. This function will return GS_FALSE
 if the hash context is not yet terminated.

 \return    Status of the function call

 \retval	Identification of the hash algorithm created
 \retval	NULL if the operation failed

 \param	IN eAlgo	The type of hash algorithm to use.

*/
//============================================================================
GShandle __stdcall InitializeHash(GSCRYPTO_HASH_ALGO eAlgo);


//============================================================================
// Function     UpdateHash
// Author:	  Guillaume Plante gplante@ubisoft.com
// Date:		2002-05-07 16:37:38
/*!
 \brief	 Update the internal hash context
 \par       Description:
 Update the internal hash context with the input value,
 the number of iteration can be modified to change the hashed
 output value, it represent the number of passed the input value 
 is put through the hash algorithm.
 This function will return GS_FALSE if the hash context is already terminated.

 \return    Status of the function call

 \retval	GS_TRUE	the operation suceeded.
 \retval	GS_FALSE the operation failed.

 \param IN hHash        Handle to the hash to be updated
 \param	IN pucInBuffer	The input buffer to be hashed.
 \param	IN uiBufferSize	The lenght of the input buffer.
 \param	IN uiIterations Nuber of time to hash the input value.

*/
//============================================================================
GSbool __stdcall UpdateHash(GShandle hHash, const GSubyte* pucInBuffer, GSuint uiBufferSize,GSuint uiIterations = 1);


//============================================================================
// Function	TerminateRawHash
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-05-07 16:46:15
/*!
 \brief	 Terminate the hash context, output binary hashed result
 \par       Description:
 This function terminate the hash context and output the hashed
 result in binary form. The output buffer must be at
 least (SHA1_DIGESTSIZE) byte long for the SHA1 (E_SHA1) algorithm
 and at leat (MD5_DIGESTSIZE) byte long for the MD5 (E_MD5) algorithm.
 This function will return GS_FALSE if the hash context is already terminated.

 \return    Status of the function call

 \retval	GS_TRUE	the operation suceeded.
 \retval	GS_FALSE the operation failed.

 \param IN  hHash           Handle to the hash to be terminated
 \param	OUT pucOutBuffer	Binary output of the generated hashed value.

*/
//============================================================================
GSbool __stdcall TerminateRawHash(GShandle hHash, GSubyte* pucOutBuffer);

//============================================================================
// Function	TerminateRawHash
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-05-07 16:46:15
/*!
 \brief	 Terminate the hash context, output hexadecimal hashed result
 \par       Description:
 This function terminate the hash context and output the hashed
 result in hexadecimal form. The output buffer must be at
 least (SHA1_HEXASIZE) byte long for the SHA1 (E_SHA1) algorithm
 and at leat (MD5_HEXASIZE) byte long for the MD5 (E_MD5) algorithm.
 It is important to note that this function does not put a \0 at the end
 of the output buffer, so the programmer will have to terminate the output
 string once he gets it.
 This function will return GS_FALSE if the hash context is already terminated.

 \return    Status of the function call

 \retval	GS_TRUE	the operation suceeded.
 \retval	GS_FALSE the operation failed.

 \param IN  hHash           Handle to the hash to be terminated
 \param	OUT pszOutBuffer	Hexadecimal output of the generated hashed value.

*/
//============================================================================
GSbool __stdcall TerminateHexaHash(GShandle hHash, GSchar* pszOutBuffer);


//============================================================================
// Function ResetHash
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-06-03 17:05:49
/*!
 \brief	  Reset the hash module
 \par       Description:
 Reset the hash module

 \return    Status of the function call

 \retval	GS_TRUE	the operation suceeded.
 \retval	GS_FALSE the operation failed.

 \param	hHash	Handle on the hash module

*/
//============================================================================
GSbool __stdcall ResetHash(GShandle hHash);

//============================================================================
// Function UninitializeHash
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-06-03 17:05:49
/*!
 \brief	  Uninitiaze the hash module
 \par       Description:
 Uninitiaze the hash module

 \return    void

 \param	hHash	Handle on the hash module

*/
//============================================================================
GSvoid __stdcall UninitializeHash(GShandle hHash);

/*! @} end of groupcrypto1 */

/*! @defgroup groupcrypto2 Pseudo random number generator functions
\brief PRNG functions

These functions are used to get random pseudo-generated numbers
    @{
*/

//============================================================================
// Function StartNumberGenerator
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-06-03 16:35:30
/*!
 \brief	 Initialize the pseudo-random number generator    
 \par       Description:
 Initialize the pseudo-random number generator

 \return    Status of the function call

 \retval	Identification of the PRNG algorithm created
 \retval	NULL if the operation failed

 \param	eAlgo	The algo to use for the pseudo-random number generator
 \param	eHash	The algo to use for the hash algorithm in the prng
 \param	pucSeed	The seed value
 \param	uiSeedSize	The size of the seed

*/
//============================================================================
GShandle __stdcall StartNumberGenerator(GSCRYPTO_PRNG_ALGO eAlgo,
                                        GSCRYPTO_HASH_ALGO eHash,
                                        const GSubyte *pucSeed,
                                        GSuint uiSeedSize);

//============================================================================
// Function StopNumberGenerator
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-06-03 17:03:37
/*!
 \brief	 Uninitiaze the pseudo-random number generator
 \par       Description:
 Uninitiaze the pseudo-random number generator

 \return    void

 \param	hPRNG		Handle on the PRNG object

*/
//============================================================================
GSvoid __stdcall StopNumberGenerator(GShandle hPRNG);

//============================================================================
// Function GenerateBit
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-06-03 16:35:36
/*!
 \brief	 Generate random bit
 \par       Description:
 This function is used to generate a random bit

 \return    Generated bit

 \param	hPRNG		Handle on the PRNG object
*/
//============================================================================
GSubyte __stdcall GenerateBit(GShandle hPRNG);

//============================================================================
// Function GenerateBit
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-06-03 16:35:36
/*!
 \brief	 Generate random byte
 \par       Description:
 This function is used to generate a random byte

 \return    Generated byte

 \param	hPRNG		Handle on the PRNG object
*/
//============================================================================
GSubyte __stdcall GenerateByte(GShandle hPRNG);

//============================================================================
// Function GenerateNumber
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-06-03 16:35:36
/*!
 \brief	 Generate random number
 \par       Description:
 This function is used to generate a random number

 \return    Generated number

 \param	hPRNG		Handle on the PRNG object
 \param	ulMax		Highest possible number that can be generated
 \param	ulMin		Lowest possible number that can be generated
*/
//============================================================================
GSulong __stdcall GenerateNumber(GShandle hPRNG, GSulong ulMax = (GSulong)-1,GSulong ulMin = 0);

//============================================================================
// Function GenerateBlock
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-06-03 16:35:37
/*!
 \brief	 Generate random array of bytes
 \par       Description:
 This function is used to generate a random array of bytes

 \return    void  

 \param	hPRNG		Handle on the PRNG object
 \param	pucBlock	Pointer to a block of byte to fill
 \param	uiBlockSize	Block size

*/
//============================================================================
GSvoid __stdcall GenerateBlock(GShandle hPRNG, GSubyte *pucBlock,GSuint uiBlockSize);

/*! @} end of groupcrypto2 */

/*! @defgroup groupcrypto3 Data encryption functions
\brief Data encryption functions

These functions are used to the encrypt data using symmetric encryption algorithms.
    @{
*/


//============================================================================
// Function	InitializeCipher
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-06-06 09:25:38
/*!
 \brief	 Initialize the encryption module
 \par       Description:
 Initialize the encryption module

 \return    Status of the function call

 \retval	Identification of the cipher algorithm created
 \retval	0 if the operation failed

 \param	eAlgo	Encryption algorithm to be used
 \param	ucKey	The key to be use for encryption and decryption
 \param	uiKeyLength	Lentgh of the key

*/
//============================================================================
GShandle __stdcall InitializeCipher(GSCRYPTO_CIPHER_ALGO eAlgo,
									const GSubyte* ucKey, GSuint uiKeyLength);


//============================================================================
// Function   UnitializeCipher
// Author:	  Guillaume Plante gplante@ubisoft.com
// Date:	  2002-06-06 09:30:12
/*!
 \brief	 Uninitialize the encryption module
 \par       Description:
 Uninitialize the encryption module

 \return    void

 \param	hCipher	handle on the cipher algorithm

*/
//============================================================================
GSvoid   __stdcall UninitializeCipher(GShandle hCipher);


//============================================================================
// Function   ResetKey
// Author:	  Guillaume Plante gplante@ubisoft.com
// Date:	  2002-06-06 09:31:48
/*!
 \brief	 Reset the encryption key
 \par       Description:
 Reset the encryption key without having to reinitialize the module

 \return    Status of the function call

 \retval	GS_TRUE	the operation suceeded.
 \retval	GS_FALSE the operation failed.

 \param	hCipher	Handle on the cipher algorithm
 \param	ucKey	The key to be use for encryption and decryption
 \param	uiKeyLength	Lentgh of the key

*/
//============================================================================
GSbool   __stdcall ResetKey(GShandle hCipher,const GSubyte* ucKey, GSuint uiKeyLength);


//============================================================================
// Function   Encrypt
// Author:	  Guillaume Plante gplante@ubisoft.com
// Date:	  2002-06-06 09:34:15
/*!
 \brief	 Encrypt a data buffer
 \par       Description:
 This function encrypt a data buffer, and gives back the result in 
 the output buffer, if you pass NULL as the output buffer, the function
 will return GS_FALSE and will give the predicted output buffer length.

 \return    Status of the function call

 \retval	GS_TRUE	the operation suceeded.
 \retval	GS_FALSE the operation failed.

 \param	hCipher	Handle on the cipher algorithm
 \param	pInputBuffer	Input buffer to be encrypted
 \param	uiInBufferLength	Input buffer length in bytes
 \param	pOutputBuffer	Output buffer (encrypted data)
 \param	puiOutBufferLength	Output buffer length in bytes

*/
//============================================================================
GSbool   __stdcall Encrypt(GShandle hCipher,const GSvoid* pInputBuffer, GSuint uiInBufferLength,
						   GSvoid* pOutputBuffer, GSuint* puiOutBufferLength);


//============================================================================
// Function   Decrypt
// Author:	  Guillaume Plante gplante@ubisoft.com
// Date:		2002-06-06 09:51:10
/*!
 \brief	 Decrypt a data buffer
 \par       Description:
 This function decrypt a data buffer, and gives back the result in 
 the output buffer, if you pass NULL as the output buffer, the function
 will return GS_FALSE and will give the predicted output buffer length.

 \return    Status of the function call

 \retval	GS_TRUE	the operation suceeded.
 \retval	GS_FALSE the operation failed.

 \param	hCipher	Handle on the cipher algorithm
 \param	pInputBuffer	Input buffer to be decrypted
 \param	uiInBufferLength	Input buffer length in bytes
 \param	pOutputBuffer	Output buffer (decrypted data)
 \param	puiOutBufferLength	Output buffer length in bytes

*/
//============================================================================
GSbool   __stdcall Decrypt(GShandle hCipher,const GSvoid* pInputBuffer, GSuint uiInBufferLength,
						   GSvoid* pOutputBuffer, GSuint* puiOutBufferLength);


/*! @} end of groupcrypto3 */


/*! @defgroup groupcrypto4 Public key cryptography
\brief Asymetric cryptosystem interface

These functions are used to the encrypt data using public key encryption algorithms.
    @{
*/

//============================================================================
// Function InitializePKC
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:	2002-05-07 16:27:15
/*!
 \brief	 Initialize the hash context 
 \par       Description:
 This function will initialize the hash context. This must be called before
 any call to the UpdateHash() function. This function will return GS_FALSE
 if the hash context is not yet terminated.

 \return    Status of the function call

 \retval	Identification of the hash algorithm created
 \retval	NULL if the operation failed

 \param	IN eAlgo	The type of hash algorithm to use.

*/
//============================================================================
GShandle __stdcall InitializePKC(GSCRYPTO_PKC_ALGO eAlgo);

//============================================================================
// Function UninitializeHash
// Author:	Guillaume Plante gplante@ubisoft.com
// Date:		2002-06-03 17:05:49
/*!
 \brief	  Uninitiaze the hash module
 \par       Description:
 Uninitiaze the hash module

 \return    void

 \param	hHash	Handle on the hash module

*/
//============================================================================
GSvoid   __stdcall UninitializePKC(GShandle hPKC);


GSbool	 __stdcall RandomInit(GShandle hPKC,RANDOM_STRUCT *pRandomData);
GSbool	 __stdcall RandomUpdate (GShandle hPKC,RANDOM_STRUCT *pRandomData, GSubyte *block, GSuint blockLen);
GSvoid	 __stdcall RandomFinal(GShandle hPKC,RANDOM_STRUCT *pRandomData);
GSint	 __stdcall GetRandomBytesNeeded(GShandle hPKC,GSuint *bytesNeeded,RANDOM_STRUCT * pRandomData);

GSbool	 __stdcall GenerateKeyPair(GShandle hPKC,RSA_PUBLIC_KEY *pPublicKey, RSA_PRIVATE_KEY *pPrivateKey,
								   RSA_PROTO_KEY *pProtoKey,RANDOM_STRUCT *pRandomData);

GSbool	 __stdcall PublicEncrypt(GShandle hPKC,GSubyte *pInputBuffer, GSuint uiInputBufferLength, 
								 GSubyte *pOutputBuffer, GSuint *pOutputBufferLength,RSA_PUBLIC_KEY * pPublicKey, RANDOM_STRUCT * pRandomData);

GSbool	 __stdcall PrivateEncrypt(GShandle hPKC,GSubyte *pInputBuffer, GSuint uiInputBufferLength, 
								  GSubyte *pOutputBuffer, GSuint *pOutputBufferLength,RSA_PRIVATE_KEY *pPrivateKey);

GSbool	 __stdcall PublicDecrypt(GShandle hPKC,GSubyte *pInputBuffer, GSuint uiInputBufferLength, 
								 GSubyte *pOutputBuffer, GSuint *pOutputBufferLength,RSA_PUBLIC_KEY *pPublicKey);

GSbool	 __stdcall PrivateDecrypt(GShandle hPKC,GSubyte *pInputBuffer, GSuint uiInputBufferLength, 
								  GSubyte *pOutputBuffer, GSuint *pOutputBufferLength,RSA_PRIVATE_KEY *pPrivateKey);

/*! @} end of groupcrypto4 */

} // extern "C"

#endif // __GSCRYPTOINTERFACE_H__








