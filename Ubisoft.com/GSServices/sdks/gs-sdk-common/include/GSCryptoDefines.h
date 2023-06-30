//****************************************************************************
//*   Author:  Guillaume Plante  gsdevelopers@ubisoft.com
//*   Date:    2002-06-10 10:11:16
 /*!  \file   GSCryptoDefines.h
  *   \brief  Cryptographic sdk variable definitions
  *
  *   This file defines all the global values and structures used by the
  *   gs-sdk-crypto.
  */
//****************************************************************************

#ifndef __GSCRYPTODEFINES_H__
#define __GSCRYPTODEFINES_H__

/*! 
	\brief Cryptographic hash algorithms enumeration

    This structure contains the valid hash algorithms that can be used with this sdk.
	E_MD5 represent the MD5 algorithm and E_SHA1 represent the Secure Hash Algorithm .
*/
enum GSCRYPTO_HASH_ALGO {
    E_MD5,
    E_SHA1
};

#define MD5_DIGESTSIZE	16	//!< One-way hash digest size (MD5)
#define SHA1_DIGESTSIZE	20	//!< One-way hash digest size (SHA1)

#define MD5_HEXASIZE	(2 * MD5_DIGESTSIZE)	//!< One-way hash hexadecimal output size (MD5)
#define SHA1_HEXASIZE	(2 * SHA1_DIGESTSIZE)	//!< One-way hash hexadecimal output size (SHA1)

/*! 
	\brief Symmetric cryptographic algorithms enumeration

    This structure contains the valid cipher algorithms that can be used with this sdk.
	E_BLOWFISH represent the blowfish algorithm and E_GSXOR represent the 
	bitshift algorithm use by the the ubi.com gs-client.
*/
enum GSCRYPTO_CIPHER_ALGO {
    E_BLOWFISH,
    E_GSXOR
};

/*! 
	\brief Asymmetric cryptographic algorithms enumeration

    This structure contains the valid public/private key algorithms that can be used with this sdk.
	E_RSA represent the RSA algorithm.
*/
enum GSCRYPTO_PKC_ALGO {
    E_RSA
};

/*! 
	\brief Pseudo-Random Number Generator algorithms enumeration

    This structure contains the valid PRNG algorithms that can be used with this sdk.
	E_MGF1 represent the Mask Generation Function algorithm.
*/
enum GSCRYPTO_PRNG_ALGO {
    E_MGF1
};

#define	MGF1_HASHMULTIPLES		500    //!< Multiples of <HASH>_DIGESTSIZE for byte string size


///// BEGIN RSA CHANGES /////


/* RSA key lengths.
 */
#define MIN_RSA_MODULUS_BITS 508	//!< Minimum length in bits of the modulus used in the RSA algorithm
#define MAX_RSA_MODULUS_BITS 1024	//!< Maximum length in bits of the modulus used in the RSA algorithm
#define MAX_RSA_MODULUS_LEN ((MAX_RSA_MODULUS_BITS + 7) / 8) //!< Maximum length in bytes of the modulus used in the RSA algorithm
#define MAX_RSA_PRIME_BITS ((MAX_RSA_MODULUS_BITS + 1) / 2) //!< Maximum length in bits of a prime
#define MAX_RSA_PRIME_LEN ((MAX_RSA_PRIME_BITS + 7) / 8) 

/* Maximum lengths of encoded and encrypted content, as a function of
   content length len. Also, inverse functions.
 */
#define ENCODED_CONTENT_LEN(len) (4*(len)/3 + 3)
#define ENCRYPTED_CONTENT_LEN(len) ENCODED_CONTENT_LEN ((len)+8)
#define DECODED_CONTENT_LEN(len) (3*(len)/4 + 1)
#define DECRYPTED_CONTENT_LEN(len) DECODED_CONTENT_LEN ((len)-1)



/* Random structure.
 */
typedef struct {
  GSuint  bytesNeeded;
  GSubyte state[16];
  GSuint  outputAvailable;
  GSubyte output[16];
} RANDOM_STRUCT;

/* RSA public and private key.
 */
typedef struct {
  GSuint bits;                           /* length in bits of modulus */
  GSubyte modulus[MAX_RSA_MODULUS_LEN];                    /* modulus */
  GSubyte exponent[MAX_RSA_MODULUS_LEN];           /* public exponent */
} RSA_PUBLIC_KEY;

typedef struct {
  GSuint bits;                           /* length in bits of modulus */
  GSubyte modulus[MAX_RSA_MODULUS_LEN];                    /* modulus */
  GSubyte publicExponent[MAX_RSA_MODULUS_LEN];     /* public exponent */
  GSubyte exponent[MAX_RSA_MODULUS_LEN];          /* private exponent */
  GSubyte prime[2][MAX_RSA_PRIME_LEN];               /* prime factors */
  GSubyte primeExponent[2][MAX_RSA_PRIME_LEN];   /* exponents for CRT */
  GSubyte coefficient[MAX_RSA_PRIME_LEN];          /* CRT coefficient */
} RSA_PRIVATE_KEY;

/* RSA prototype key.
 */
typedef struct {
  GSuint bits;                           /* length in bits of modulus */
  GSint useFormat4;                        /* public exponent (1 = F4, 0 = 3) */
} RSA_PROTO_KEY;

///// END RSA CHANGES /////

#endif // __GSCRYPTODEFINES_H__
