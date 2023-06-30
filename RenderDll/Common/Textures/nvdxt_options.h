/****************************************************************************************
	
    Copyright (C) NVIDIA Corporation 2003

    TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
    *AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
    OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
    BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
    WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
    BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
    ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
    BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*****************************************************************************************/
#pragma once

#if defined(LINUX)
	#include "WinBase.h"
#else
	#include <windows.h>
#endif

typedef HRESULT (*MIPcallback)(void * data, int miplevel, DWORD size, int width, int height, void * user_data);


inline char * GetDXTCVersion() { return "Version 6.54";}


enum
{

    dBackgroundNameStatic = 3,
    dProfileDirNameStatic = 4,
    dSaveButton = 5,
    dProfileNameStatic = 6,





    dbSwapRGB = 29,
    dGenerateMipMaps = 30,
    dMIPMapSourceFirst = dGenerateMipMaps,
    dUseExistingMipMaps = 31,
    dNoMipMaps = 32,
    dMIPMapSourceLast = dNoMipMaps,

    dSpecifiedMipMapsCombo = 39,



    dbShowDifferences = 40,
    dbShowFiltering = 41,
    dbShowMipMapping = 42,
    dbShowAnisotropic = 43,

    dChangeClearColorButton = 50,
    dDitherColor = 53,

    dLoadBackgroundImageButton = 54,
    dUseBackgroundImage = 55,

    dbBinaryAlpha = 56,
    dAlphaBlending = 57,

    dFadeColor = 58,  //.
    dFadeAlpha = 59,

    dFadeToColorButton = 60,
    dAlphaBorder = 61,
    dBorder = 62,
    dBorderColorButton = 63,

    dNormalMapCombo = 64,

    dDitherMIP0 = 66,
    dGreyScale = 67,
    dQuickCompress = 68,
    dLightingEnable = 69,

    dbPreviewDisableMIPmaps = 71,

    // WarpSharp is currently disabled
    /*dSharpenEdgeRadius = 71,
    dSharpenLambda = 72,
    dSharpenMu = 73,
    dSharpenTheta = 74,
    dbSharpenUseTwoComponents = 75,
    dbSharpenNonMaximalSuppression = 76,
    dbSharpenFlavor2 = 77,
    dbSharpenSharpBlur = 78,*/

    dZoom = 79,


    dTextureTypeCombo = 80,

    dFadeAmount = 90,
    dFadeToAlpha = 91,
    dFadeToDelay = 92,

    dBinaryAlphaThreshold = 94,

    dFilterGamma = 100,
    dFilterBlur = 101,
    dFilterWidth = 102,
    dbOverrideFilterWidth = 103,
    dLoadProfile = 104,
    dSaveProfile = 105,
    dSharpenMethodCombo = 106,
    dProfileCombo = 107,
    dSelectProfileDirectory = 108,





    dViewDXT1 = 200,
    dViewDXT2 = 201,
    dViewDXT3 = 202,
    dViewDXT5 = 203,
    dViewA4R4G4B4 = 204,
    dViewA1R5G5B5 = 205,
    dViewR5G6B5 = 206,
    dViewA8R8G8B8 = 207,


    // 3d viewing options
    d3DPreviewButton = 300, 
    d2DPreviewButton = 301, 
    dPreviewRefresh = 302, 




    dAskToLoadMIPMaps = 400,
    dShowAlphaWarning = 401,
    dShowPower2Warning = 402,
    dTextureFormatBasedOnAlpha = 403,
    dSystemMessage = 404,
    dHidePreviewButtons = 405,

    dSpecifyAlphaBlendingButton = 500,
    dUserSpecifiedFadingAmounts = 501,
    dSharpenSettingsButton = 502,
    dFilterSettingsButton = 503,
    dNormalMapGenerationSettingsButton = 504,
    dConfigSettingsButton = 505,
    dFadingDialogButton = 506,
    dPreviewDialogButton = 507,
    dResetDefaultsButton = 508,
    dImageDialogButton = 510,



    dSaveTextureFormatCombo = 600,
    dMIPFilterCombo = 601,


    ///////////  Normal Map


    dScaleEditText = 1003,
    dProxyItem = 1005,
    dMinZEditText = 1008,



    dALPHA = 1009,
    dFirstCnvRadio = dALPHA,
    dAVERAGE_RGB = 1010,
    dBIASED_RGB = 1011,
    dRED = 1012,
    dGREEN = 1013,
    dBLUE = 1014,
    dMAX = 1015,
    dCOLORSPACE = 1016,
    dNORMALIZE = 1017,
    dConvertToHeightMap = 1018,
    dLastCnvRadio = dConvertToHeightMap,

    d3DPreview = 1021,      
    dDecalTexture = 1022,
    dbUseDecalTexture = 1023,
    dbBrighten = 1024,
    dbAnimateLight = 1025,
    dStaticDecalName = 1026,
    dbSignedOutput = 1027,
    dbNormalMapSwapRGB = 1028,


    dbWrap = 1030,
    dbMultipleLayers = 1031,
    db_16_16 = 1032,

    dAlphaNone = 1033,
    dFirstAlphaRadio = dAlphaNone,
    dAlphaHeight = 1034,
    dAlphaClear = 1035,
    dAlphaWhite = 1036,
    dLastAlphaRadio = dAlphaWhite,

    dbInvertY = 1037,
    db_12_12_8 = 1038,
    dbInvertX = 1039,

    dFilter4x = 1040,
    dFirstFilterRadio = dFilter4x,
    dFilter3x3 = 1041,
    dFilter5x5 = 1042,
    dFilterDuDv = 1043,
    dFilter7x7 = 1044,
    dFilter9x9 = 1045,
    dLastFilterRadio = dFilter9x9,


    dbNormalMapConversion = 1050,
    dbErrorDiffusion = 1051,


    dCustomFilterFirst = 2000,
    // 5x5  Filter 0- 24

    dCustomDiv = 2025,
    dCustomBias = 2026,

    dUnSharpRadius = 2027,
    dUnSharpAmount = 2028,
    dUnSharpThreshold = 2029,

    dXSharpenStrength = 2030,
    dXSharpenThreshold = 2031,

    /*dWarpSharpenDisplaceAmount = 2032,
    dWarpSharpenEdgeStrength = 2033,
    dWarpSharpenRadius = 2034,
    dWarpSharpenDepth = 2035,
    dWarpSharpenElevation = 2036,*/

    dCustomFilterLast = dXSharpenThreshold,  


    dSharpenTimesMIP0 = 2100,
    dSharpenTimesFirst = dSharpenTimesMIP0,

    dSharpenTimesMIP1 = 2101,
    dSharpenTimesMIP2 = 2102,
    dSharpenTimesMIP3 = 2103,
    dSharpenTimesMIP4 = 2104,
    dSharpenTimesMIP5 = 2105,
    dSharpenTimesMIP6 = 2106,
    dSharpenTimesMIP7 = 2107,
    dSharpenTimesMIP8 = 2108,
    dSharpenTimesMIP9 = 2109,
    dSharpenTimesMIP10 = 2110,
    dSharpenTimesMIP11 = 2111,
    dSharpenTimesMIP12 = 2112,
    dSharpenTimesMIP13 = 2113,
    dSharpenTimesLast = dSharpenTimesMIP13,

    dDeriveDiv = 2200,   // balance
    dDeriveBias = 2201,



};



#ifndef TRGBA
#define TRGBA
typedef	struct	
{
	unsigned char	rgba[4];
} rgba_t;
#endif

#ifndef TPIXEL
#define TPIXEL
union tPixel
{
  unsigned long u;
  rgba_t c;
};
#endif


// Windows handle for our plug-in (seen as a dynamically linked library):
extern HANDLE hDllInstance;
class CMyD3DApplication;

typedef enum RescaleTypes
{
    RESCALE_NONE,               // no rescale
    RESCALE_NEAREST_POWER2,     // rescale to nearest power of two
    RESCALE_BIGGEST_POWER2,   // rescale to next bigger power of 2
    RESCALE_SMALLEST_POWER2,  // rescale to smaller power of 2 
    RESCALE_NEXT_SMALLEST_POWER2,  // rescale to next smaller power of 2
    RESCALE_PRESCALE,           // rescale to this size
    RESCALE_RELSCALE,           // relative rescale
    RESCALE_CLAMP,              //
    RESCALE_LAST,              //


} RescaleTypes;


enum SharpenFilterTypes
{
    kSharpenFilterNone,
    kSharpenFilterNegative,
    kSharpenFilterLighter,
    kSharpenFilterDarker,
    kSharpenFilterContrastMore,
    kSharpenFilterContrastLess,
    kSharpenFilterSmoothen,
    kSharpenFilterSharpenSoft,
    kSharpenFilterSharpenMedium,
    kSharpenFilterSharpenStrong,
    kSharpenFilterFindEdges,
    kSharpenFilterContour,
    kSharpenFilterEdgeDetect,
    kSharpenFilterEdgeDetectSoft,
    kSharpenFilterEmboss,
    kSharpenFilterMeanRemoval,
    kSharpenFilterUnSharp,
    kSharpenFilterXSharpen,
    kSharpenFilterWarpSharp,
    kSharpenFilterCustom,
    kSharpenFilterLast,
};



enum MIPFilterTypes
{
    kMIPFilterPoint ,    
    kMIPFilterBox ,      
    kMIPFilterTriangle , 
    kMIPFilterQuadratic ,
    kMIPFilterCubic ,    

    kMIPFilterCatrom ,   
    kMIPFilterMitchell , 

    kMIPFilterGaussian , 
    kMIPFilterSinc ,     
    kMIPFilterBessel ,   

    kMIPFilterHanning ,  
    kMIPFilterHamming ,  
    kMIPFilterBlackman , 
    kMIPFilterKaiser,
    kMIPFilterLast,
};


enum TextureFormats
{
    kDXT1 ,
    kDXT1a ,  // DXT1 with one bit alpha
    kDXT3 ,   // explicit alpha
    kDXT5 ,   // interpolated alpha
    k4444 ,   // a4 r4 g4 b4
    k1555 ,   // a1 r5 g5 b5
    k565 ,    // a0 r5 g6 b5
    k8888 ,   // a8 r8 g8 b8
    k888 ,    // a0 r8 g8 b8
    k555 ,    // a0 r5 g5 b5
    k8   ,   // paletted
    kV8U8 ,   // DuDv 
    kCxV8U8 ,   // normal map
    kA8 ,            // alpha only
    k4  ,            // 16 bit color      
    kQ8W8V8U8,
    kA8L8,
    kTextureFormatLast
};


enum TextureTypes
{
    kTextureType2D,
    kTextureTypeCube,
    kTextureTypeVolume,  
    kTextureTypeLast,  
};

enum NormalMapTypes
{
    kColorTextureMap,
    kTangentSpaceNormalMap,
    kObjectSpaceNormalMap,
};


#define CUSTOM_FILTER_ENTRIES 27
#define UNSHARP_ENTRIES 3
#define XSHARP_ENTRIES 3
//#define WARPSHARP_ENTRIES 5

#define CUSTOM_DATA_ENTRIES (CUSTOM_FILTER_ENTRIES+UNSHARP_ENTRIES+XSHARP_ENTRIES)
#define SHARP_TIMES_ENTRIES 14

typedef struct CompressionOptions
{
    CompressionOptions()
    {

        rescaleImageType = RESCALE_NONE; 
        scaleX = 1;
        scaleY = 1;

        //bMipMapsInImage = false;    // mip have been loaded in during read

        MipMapType = dGenerateMipMaps;         // dNoMipMaps, dUseExistingMipMaps, dGenerateMipMaps
        SpecifiedMipMaps = 0;   // if dSpecifyMipMaps or dUseExistingMipMaps is set (number of mipmaps to generate)

        MIPFilterType = kMIPFilterTriangle;      // for MIP maps
        /* 
            for MIPFilterType, specify one betwee dMIPFilterFirst and dMIPFilterLast
        */


        bBinaryAlpha = false;       // zero or one alpha channel

        normalMapType = kColorTextureMap;         

        bAlphaBorder= false;       // make an alpha border
        bBorder= false;            // make a color border
        BorderColor.u = 0;        // color of border


        bFadeColor = false;         // fade color over MIP maps
        bFadeAlpha = false;         // fade alpha over MIP maps

        FadeToColor.u = 0;        // color to fade to
        FadeToAlpha = 0;        // alpha value to fade to (0-255)

        FadeToDelay = 0;        // start fading after 'n' MIP maps

        FadeAmount = 0;         // percentage of color to fade in

        BinaryAlphaThreshold = 128;  // When Binary Alpha is selected, below this value, alpha is zero


        bDitherColor = false;       // enable dithering during 16 bit conversion
        bDitherMIP0 = false;// enable dithering during 16 bit conversion for each MIP level (after filtering)
        bGreyScale = false;         // treat image as a grey scale
        bQuickCompress = false;         // Fast compression scheme
        bForceDXT1FourColors = false;  // do not let DXT1 use 3 color representation


        SharpenFilterType = kSharpenFilterNone;
        bErrorDiffusion = false;

        // sharpening after creating each MIP map level
        // warp sharp filter parameters
        // look here for details:
        //          
        // "Enhancement by Image-Dependent Warping", 
        // IEEE Transactions on Image Processing, 1999, Vol. 8, No. 8, S. 1063
        // Nur Arad and Craig Gotsman
        // http://www.cs.technion.ac.il/~gotsman/AmendedPubl/EnhancementByImage/EnhancementByI-D.pdf




        /*SharpenEdgeRadius = 2;
        SharpenLambda = 10.0f;
        SharpenMu = 0.01f;
        SharpenTheta =  0.75;
        bSharpenUseTwoComponents = false;
        bSharpenNonMaximalSuppression = false;
        bSharpenSharpBlur = false;
        bSharpenFlavor2 = false;*/

        // gamma value for Kaiser, Light Linear
        FilterGamma = 2.2f;
        // alpha value for 
        FilterBlur = 1.0f;
        // width of filter
        FilterWidth = 10.0f;
        bOverrideFilterWidth = false;

        TextureType = kTextureType2D;        // regular decal, cube or volume  
        /*
        for TextureType, specify one of:
        dTextureType2D 
        dTextureTypeCube 
        dTextureTypeImage 
        dTextureTypeVolume
        */

        TextureFormat = kDXT1;	    
        /* 
        for TextureFormat, specify any from dTextureFormatFirst to 
        dTextureFormatLast

  
        */

        bSwapRGB = false;           // swap color positions R and G
        user_data = 0;

    };

    RescaleTypes   rescaleImageType;     // changes to just rescale
    float           scaleX;
    float       scaleY;

    //bool            bMipMapsInImage;    // mip have been loaded in during read
    short           MipMapType;         // dNoMipMaps, dSpecifyMipMaps, dUseExistingMipMaps, dGenerateMipMaps

    short           SpecifiedMipMaps;   // if dSpecifyMipMaps or dUseExistingMipMaps is set (number of mipmaps to generate)

    MIPFilterTypes   MIPFilterType;      // for MIP map, select from MIPFilterTypes

    bool        bBinaryAlpha;       // zero or one alpha channel

    NormalMapTypes  normalMapType;  // 

    bool        bAlphaBorder;       // make an alpha border
    bool        bBorder;            // make a color border
    tPixel      BorderColor;        // color of border


    bool        bFadeColor;         // fade color over MIP maps
    bool        bFadeAlpha;         // fade alpha over MIP maps

    tPixel      FadeToColor;        // color to fade to
    int         FadeToAlpha;        // alpha value to fade to (0-255)

    int         FadeToDelay;        // start fading after 'n' MIP maps

    int         FadeAmount;         // percentage of color to fade in

    int         BinaryAlphaThreshold;  // When Binary Alpha is selected, below this value, alpha is zero


    bool        bDitherColor;       // enable dithering during 16 bit conversion
    bool        bDitherMIP0;// enable dithering during 16 bit conversion for each MIP level (after filtering)
    bool        bGreyScale;         // treat image as a grey scale
    bool        bQuickCompress;         // Fast compression scheme
    bool        bForceDXT1FourColors;  // do not let DXT1 use 3 color representation


    // sharpening after creating each MIP map level



    float custom_data[CUSTOM_DATA_ENTRIES]; 
    int sharpening_passes_per_mip_level[SHARP_TIMES_ENTRIES]; 

    SharpenFilterTypes  SharpenFilterType; 

    bool bErrorDiffusion; // diffuse error



    // warp sharp filter parameters (disabled)
    // look here for details:
    //          
    // "Enhancement by Image-Dependent Warping", 
    // IEEE Transactions on Image Processing, 1999, Vol. 8, No. 8, S. 1063
    // Nur Arad and Craig Gotsman
    // http://www.cs.technion.ac.il/~gotsman/AmendedPubl/EnhancementByImage/EnhancementByI-D.pdf

    /*int SharpenEdgeRadius;
    float SharpenLambda;
    float SharpenMu;
    float SharpenTheta;
    bool bSharpenUseTwoComponents;
    bool bSharpenNonMaximalSuppression;
    bool bSharpenSharpBlur;
    bool bSharpenFlavor2;*/

    // gamma value for Kaiser, Light Linear
    float FilterGamma;  // not implemented yet
    // alpha value for kaiser filter
    float FilterBlur;
    // width of filter
    float FilterWidth;

    bool bOverrideFilterWidth; // use the specified width,instead of the default

	TextureTypes 		TextureType;        
	TextureFormats 		TextureFormat;	    

    bool   bSwapRGB;           // swap color positions R and G
    void * user_data;

} CompressionOptions;

