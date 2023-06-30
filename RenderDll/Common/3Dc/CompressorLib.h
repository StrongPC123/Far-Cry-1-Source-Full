//
// Header file for compression library
//

#ifndef _COMPRESSORLIB_H
#define _COMPRESSORLIB_H

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(LINUX)
	#define _stdcall
#else
	#include "Windows.h"
#endif

typedef enum
{
    FORMAT_COMP_ATI2N,
    FORMAT_COMP_ATI2N_DXT5,
    FORMAT_COMP_TOOBIG
} COMPRESSED_FORMAT;


typedef enum
{
    FORMAT_ARGB_8888,
    FORMAT_ARGB_TOOBIG
} UNCOMPRESSED_FORMAT;


typedef enum
{
    COMPRESSOR_ERROR_NONE,
    COMPRESSOR_ERROR_NO_INPUT_DATA,
    COMPRESSOR_ERROR_NO_OUTPUT_POINTER,
    COMPRESSOR_ERROR_UNSUPPORTED_SOURCE_FORMAT,
    COMPRESSOR_ERROR_UNSUPPORTED_DESTINATION_FORMAT,
    COMPRESSOR_ERROR_UNABLE_TO_INIT_CODEC,
    COMPRESSOR_ERROR_GENERIC
} COMPRESSOR_ERROR;



//
// Compressor entry point
//
// Parameters -
// 
// width - width of input image in pixels
// height - height of input image in pixels
// sourceFormat - format of source texture (must be ARGB_8888 at the moment)
// destinationFormat - format of output texture (can be ATI2N or ATI2N_DXT5)
// inputData - pointer to uncompressed texture data
// dataOut - address of pointer that will point to the compressed data
// outDataSize - filled with the size of the compressed data buffer (can be NULL)
//

COMPRESSOR_ERROR _stdcall CompressTexture(DWORD width,
                                          DWORD height,
                                          UNCOMPRESSED_FORMAT sourceFormat,
                                          COMPRESSED_FORMAT destinationFormat,
                                          void    *inputData,
                                          void    **dataOut,
                                          DWORD   *outDataSize);


#ifdef __cplusplus
}
#endif //__cplusplus

typedef void (*FnDeleteDataATI)(void *pData);
typedef COMPRESSOR_ERROR (*FnCompressTextureATI)(DWORD width,
                                                           DWORD height,
                                                           UNCOMPRESSED_FORMAT sourceFormat,
                                                           COMPRESSED_FORMAT destinationFormat,
                                                           void    *inputData,
                                                           void    **dataOut,
                                                           DWORD   *outDataSize);

extern void (*DeleteDataATI)(void *pData);
extern COMPRESSOR_ERROR (*CompressTextureATI)(DWORD width,
                                                        DWORD height,
                                                        UNCOMPRESSED_FORMAT sourceFormat,
                                                        COMPRESSED_FORMAT destinationFormat,
                                                        void    *inputData,
                                                        void    **dataOut,
                                                        DWORD   *outDataSize);

#endif // _COMPRESSORLIB_H