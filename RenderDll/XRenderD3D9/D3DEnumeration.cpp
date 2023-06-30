#include "RenderPCH.h"
#include "DriverD3D9.h"


//-----------------------------------------------------------------------------
// Name: ColorChannelBits
// Desc: Returns the number of color channel bits in the specified D3DFORMAT
//-----------------------------------------------------------------------------
UINT ColorChannelBits( D3DFORMAT fmt )
{
    switch( fmt )
    {
        case D3DFMT_R8G8B8:
            return 8;
        case D3DFMT_A8R8G8B8:
            return 8;
        case D3DFMT_X8R8G8B8:
            return 8;
        case D3DFMT_R5G6B5:
            return 5;
        case D3DFMT_X1R5G5B5:
            return 5;
        case D3DFMT_A1R5G5B5:
            return 5;
        case D3DFMT_A4R4G4B4:
            return 4;
        case D3DFMT_R3G3B2:
            return 2;
        case D3DFMT_A8R3G3B2:
            return 2;
        case D3DFMT_X4R4G4B4:
            return 4;
        case D3DFMT_A2B10G10R10:
            return 10;
        case D3DFMT_A2R10G10B10:
            return 10;
        default:
            return 0;
    }
}


//-----------------------------------------------------------------------------
// Name: ColorBits
// Desc: Returns the number of color bits in the specified D3DFORMAT
//-----------------------------------------------------------------------------
UINT ColorBits( D3DFORMAT fmt )
{
  switch( fmt )
  {
    case D3DFMT_R8G8B8:
      return 24;
    case D3DFMT_A8R8G8B8:
      return 32;
    case D3DFMT_X8R8G8B8:
      return 24;
    case D3DFMT_R5G6B5:
      return 16;
    case D3DFMT_X1R5G5B5:
      return 16;
    case D3DFMT_A1R5G5B5:
      return 16;
    case D3DFMT_A4R4G4B4:
      return 16;
    case D3DFMT_R3G3B2:
      return 8;
    case D3DFMT_A8R3G3B2:
      return 16;
    case D3DFMT_X4R4G4B4:
      return 16;
    case D3DFMT_A2B10G10R10:
      return 32;
    case D3DFMT_A2R10G10B10:
      return 32;
    default:
      return 0;
  }
}


//-----------------------------------------------------------------------------
// Name: AlphaChannelBits
// Desc: Returns the number of alpha channel bits in the specified D3DFORMAT
//-----------------------------------------------------------------------------
static UINT AlphaChannelBits( D3DFORMAT fmt )
{
    switch( fmt )
    {
        case D3DFMT_R8G8B8:
            return 0;
        case D3DFMT_A8R8G8B8:
            return 8;
        case D3DFMT_X8R8G8B8:
            return 0;
        case D3DFMT_R5G6B5:
            return 0;
        case D3DFMT_X1R5G5B5:
            return 0;
        case D3DFMT_A1R5G5B5:
            return 1;
        case D3DFMT_A4R4G4B4:
            return 4;
        case D3DFMT_R3G3B2:
            return 0;
        case D3DFMT_A8R3G3B2:
            return 8;
        case D3DFMT_X4R4G4B4:
            return 0;
        case D3DFMT_A2B10G10R10:
            return 2;
        case D3DFMT_A2R10G10B10:
            return 2;
        default:
            return 0;
    }
}




//-----------------------------------------------------------------------------
// Name: DepthBits
// Desc: Returns the number of depth bits in the specified D3DFORMAT
//-----------------------------------------------------------------------------
static UINT DepthBits( D3DFORMAT fmt )
{
  switch( fmt )
  {
    case D3DFMT_D16:
      return 16;
    case D3DFMT_D15S1:
      return 15;
    case D3DFMT_D24X8:
      return 24;
    case D3DFMT_D24S8:
      return 24;
    case D3DFMT_D24X4S4:
      return 24;
    case D3DFMT_D32:
      return 32;
    default:
      return 0;
  }
}




//-----------------------------------------------------------------------------
// Name: StencilBits
// Desc: Returns the number of stencil bits in the specified D3DFORMAT
//-----------------------------------------------------------------------------
static UINT StencilBits( D3DFORMAT fmt )
{
  switch( fmt )
  {
    case D3DFMT_D16:
      return 0;
    case D3DFMT_D15S1:
      return 1;
    case D3DFMT_D24X8:
      return 0;
    case D3DFMT_D24S8:
      return 8;
    case D3DFMT_D24X4S4:
      return 4;
    case D3DFMT_D32:
      return 0;
    default:
      return 0;
  }
}




//-----------------------------------------------------------------------------
// Name: D3DAdapterInfo destructor
// Desc: 
//-----------------------------------------------------------------------------
D3DAdapterInfo::~D3DAdapterInfo( void )
{
  if( pDisplayModeList != NULL )
    delete pDisplayModeList;
  if( pDeviceInfoList != NULL )
  {
    for(int idi=0; idi<pDeviceInfoList->Num(); idi++)
    {
      delete pDeviceInfoList->Get(idi);
    }
    delete pDeviceInfoList;
  }
}




//-----------------------------------------------------------------------------
// Name: D3DDeviceInfo destructor
// Desc: 
//-----------------------------------------------------------------------------
D3DDeviceInfo::~D3DDeviceInfo( void )
{
  if( pDeviceComboList != NULL )
  {
    for(int idc=0; idc<pDeviceComboList->Num(); idc++)
    {
      delete pDeviceComboList->Get(idc);
    }
    delete pDeviceComboList;
  }
}




//-----------------------------------------------------------------------------
// Name: D3DDeviceCombo destructor
// Desc: 
//-----------------------------------------------------------------------------
D3DDeviceCombo::~D3DDeviceCombo( void )
{
  if( pDepthStencilFormatList != NULL )
    delete pDepthStencilFormatList;
  if( pMultiSampleTypeList != NULL )
    delete pMultiSampleTypeList;
  if( pMultiSampleQualityList != NULL )
    delete pMultiSampleQualityList;
  if( pDSMSConflictList != NULL )
    delete pDSMSConflictList;
  if( pVertexProcessingTypeList != NULL )
    delete pVertexProcessingTypeList;
  if( pPresentIntervalList != NULL )
    delete pPresentIntervalList;
}



//-----------------------------------------------------------------------------
// Name: CD3DEnumeration constructor
// Desc: 
//-----------------------------------------------------------------------------
CD3DEnumeration::CD3DEnumeration()
{
  m_pAdapterInfoList = NULL;
  m_pAllowedAdapterFormatList = NULL;
  AppMinFullscreenWidth = 640;
  AppMinFullscreenHeight = 480;
  AppMinColorChannelBits = 5;
  AppMinAlphaChannelBits = 0;
  AppMinDepthBits = 15;
  AppMinStencilBits = 0;
  AppUsesDepthBuffer = true;
  AppUsesMixedVP = false;
  AppRequiresWindowed = false;
  AppRequiresFullscreen = false;
}




//-----------------------------------------------------------------------------
// Name: CD3DEnumeration destructor
// Desc: 
//-----------------------------------------------------------------------------
CD3DEnumeration::~CD3DEnumeration()
{
  if( m_pAdapterInfoList != NULL )
  {
    for(int iai=0; iai < m_pAdapterInfoList->Num(); iai++)
    {
      delete m_pAdapterInfoList->Get(iai);
    }
    delete m_pAdapterInfoList;
  }
  SAFE_DELETE( m_pAllowedAdapterFormatList );
}




//-----------------------------------------------------------------------------
// Name: SortModesCallback
// Desc: Used to sort D3DDISPLAYMODEs
//-----------------------------------------------------------------------------
static int __cdecl SortModesCallback( const void* arg1, const void* arg2 )
{
  D3DDISPLAYMODE* pdm1 = (D3DDISPLAYMODE*)arg1;
  D3DDISPLAYMODE* pdm2 = (D3DDISPLAYMODE*)arg2;

  if (pdm1->Width > pdm2->Width)
    return 1;
  if (pdm1->Width < pdm2->Width)
    return -1;
  if (pdm1->Height > pdm2->Height)
    return 1;
  if (pdm1->Height < pdm2->Height)
    return -1;
  if (pdm1->Format > pdm2->Format)
    return 1;
  if (pdm1->Format < pdm2->Format)
    return -1;
  if (pdm1->RefreshRate > pdm2->RefreshRate)
    return 1;
  if (pdm1->RefreshRate < pdm2->RefreshRate)
    return -1;
  return 0;
}




//-----------------------------------------------------------------------------
// Name: Enumerate
// Desc: Enumerates available D3D adapters, devices, modes, etc.
//-----------------------------------------------------------------------------
HRESULT CD3DEnumeration::Enumerate()
{
  HRESULT hr;
  TArray<D3DFORMAT> adapterFormatList;

  if( m_pD3D == NULL )
    return E_FAIL;

  m_pAdapterInfoList = new TArray<D3DAdapterInfo *>;
  if( m_pAdapterInfoList == NULL )
    return E_OUTOFMEMORY;

  m_pAllowedAdapterFormatList = new TArray<D3DFORMAT>;
  if( m_pAllowedAdapterFormatList == NULL )
    return E_OUTOFMEMORY;
  m_pAllowedAdapterFormatList->AddElem(D3DFMT_X8R8G8B8);
  m_pAllowedAdapterFormatList->AddElem(D3DFMT_X1R5G5B5);
  m_pAllowedAdapterFormatList->AddElem(D3DFMT_R5G6B5);
  m_pAllowedAdapterFormatList->AddElem(D3DFMT_A2R10G10B10);

  D3DAdapterInfo* pAdapterInfo = NULL;
  UINT numAdapters = m_pD3D->GetAdapterCount();

  for (UINT adapterOrdinal=0; adapterOrdinal<numAdapters; adapterOrdinal++)
  {
    pAdapterInfo = new D3DAdapterInfo;
    if( pAdapterInfo == NULL )
      return E_OUTOFMEMORY;
    pAdapterInfo->m_MaxWidth = 0;
    pAdapterInfo->m_MaxHeight = 0;
    pAdapterInfo->pDisplayModeList = new TArray<D3DDISPLAYMODE>; 
    pAdapterInfo->pDeviceInfoList = new TArray<D3DDeviceInfo *>;
    if(pAdapterInfo->pDisplayModeList == NULL || pAdapterInfo->pDeviceInfoList == NULL)
    {
      delete pAdapterInfo;
      return E_OUTOFMEMORY;
    }
    pAdapterInfo->AdapterOrdinal = adapterOrdinal;
    m_pD3D->GetAdapterIdentifier(adapterOrdinal, 0, &pAdapterInfo->AdapterIdentifier);

    // Get list of all display modes on this adapter.  
    // Also build a temporary list of all display adapter formats.
    adapterFormatList.Free();
    for(int iaaf=0; iaaf<m_pAllowedAdapterFormatList->Num(); iaaf++)
    {
      D3DFORMAT allowedAdapterFormat = m_pAllowedAdapterFormatList->Get(iaaf);
      UINT numAdapterModes = m_pD3D->GetAdapterModeCount( adapterOrdinal, allowedAdapterFormat );
      for (UINT mode=0; mode<numAdapterModes; mode++)
      {
        D3DDISPLAYMODE displayMode;
        m_pD3D->EnumAdapterModes( adapterOrdinal, allowedAdapterFormat, mode, &displayMode );
        if( displayMode.Width < AppMinFullscreenWidth || displayMode.Height < AppMinFullscreenHeight || ColorChannelBits(displayMode.Format) < AppMinColorChannelBits )
          continue;
        pAdapterInfo->m_MaxWidth = max(pAdapterInfo->m_MaxWidth, displayMode.Width);
        pAdapterInfo->m_MaxHeight = max(pAdapterInfo->m_MaxHeight, displayMode.Height);
        pAdapterInfo->pDisplayModeList->AddElem(displayMode);
        if(adapterFormatList.Find(displayMode.Format) < 0)
          adapterFormatList.AddElem(displayMode.Format);
      }
    }

    // Sort displaymode list
    qsort((void *)&pAdapterInfo->pDisplayModeList->Get(0), pAdapterInfo->pDisplayModeList->Num(), sizeof(D3DDISPLAYMODE), SortModesCallback );

    // Get info for each device on this adapter
    if(FAILED(hr = EnumerateDevices( pAdapterInfo, &adapterFormatList)))
    {
      delete pAdapterInfo;
      return hr;
    }

    // If at least one device on this adapter is available and compatible
    // with the app, add the adapterInfo to the list
    if (pAdapterInfo->pDeviceInfoList->Num() == 0)
      delete pAdapterInfo;
    else
      m_pAdapterInfoList->AddElem(pAdapterInfo);
  }
  return S_OK;
}




//-----------------------------------------------------------------------------
// Name: EnumerateDevices
// Desc: Enumerates D3D devices for a particular adapter.
//-----------------------------------------------------------------------------
HRESULT CD3DEnumeration::EnumerateDevices(D3DAdapterInfo* pAdapterInfo, TArray<D3DFORMAT> *pAdapterFormatList)
{
  const D3DDEVTYPE devTypeArray[] = { D3DDEVTYPE_HAL, D3DDEVTYPE_SW, D3DDEVTYPE_REF };
  const UINT devTypeArrayCount = sizeof(devTypeArray) / sizeof(devTypeArray[0]);
  HRESULT hr;

  D3DDeviceInfo* pDeviceInfo = NULL;
  for(UINT idt=0; idt<devTypeArrayCount; idt++)
  {
    pDeviceInfo = new D3DDeviceInfo;
    if( pDeviceInfo == NULL )
      return E_OUTOFMEMORY;
    pDeviceInfo->pDeviceComboList = new TArray<D3DDeviceCombo *>; 
    if( pDeviceInfo->pDeviceComboList == NULL )
    {
      delete pDeviceInfo;
      return E_OUTOFMEMORY;
    }
    pDeviceInfo->AdapterOrdinal = pAdapterInfo->AdapterOrdinal;
    pDeviceInfo->DevType = devTypeArray[idt];
    if( FAILED( m_pD3D->GetDeviceCaps(pAdapterInfo->AdapterOrdinal, pDeviceInfo->DevType, &pDeviceInfo->Caps)))
    {
      delete pDeviceInfo;
      continue;
    }

    // Get info for each devicecombo on this device
    if(FAILED(hr = EnumerateDeviceCombos(pDeviceInfo, pAdapterFormatList)))
    {
      delete pDeviceInfo;
      return hr;
    }

    // If at least one devicecombo for this device is found, 
    // add the deviceInfo to the list
    if (pDeviceInfo->pDeviceComboList->Num() == 0)
    {
      delete pDeviceInfo;
      continue;
    }
    pAdapterInfo->pDeviceInfoList->AddElem(pDeviceInfo);
  }
  return S_OK;
}




//-----------------------------------------------------------------------------
// Name: EnumerateDeviceCombos
// Desc: Enumerates DeviceCombos for a particular device.
//-----------------------------------------------------------------------------
HRESULT CD3DEnumeration::EnumerateDeviceCombos(D3DDeviceInfo* pDeviceInfo, TArray<D3DFORMAT> *pAdapterFormatList)
{
  const D3DFORMAT backBufferFormatArray[] = 
      {   D3DFMT_A8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_A2R10G10B10, 
          D3DFMT_R5G6B5, D3DFMT_A1R5G5B5, D3DFMT_X1R5G5B5 };
  const UINT backBufferFormatArrayCount = sizeof(backBufferFormatArray) / sizeof(backBufferFormatArray[0]);
  bool isWindowedArray[] = { false, true };

  // See which adapter formats are supported by this device
  D3DFORMAT adapterFormat;
  for(int iaf=0; iaf<pAdapterFormatList->Num(); iaf++)
  {
    adapterFormat = pAdapterFormatList->Get(iaf);
    D3DFORMAT backBufferFormat;
    for(UINT ibbf=0; ibbf<backBufferFormatArrayCount; ibbf++)
    {
      backBufferFormat = backBufferFormatArray[ibbf];
      if (AlphaChannelBits(backBufferFormat) < AppMinAlphaChannelBits)
        continue;
      bool isWindowed;
      for( UINT iiw=0; iiw<2; iiw++)
      {
        isWindowed = isWindowedArray[iiw];
        if (!isWindowed && AppRequiresWindowed)
          continue;
        if (isWindowed && AppRequiresFullscreen)
          continue;
        if (FAILED(m_pD3D->CheckDeviceType(pDeviceInfo->AdapterOrdinal, pDeviceInfo->DevType, adapterFormat, backBufferFormat, isWindowed)))
          continue;
        // At this point, we have an adapter/device/adapterformat/backbufferformat/iswindowed
        // DeviceCombo that is supported by the system.  We still need to confirm that it's 
        // compatible with the app, and find one or more suitable depth/stencil buffer format,
        // multisample type, vertex processing type, and present interval.
        D3DDeviceCombo* pDeviceCombo = NULL;
        pDeviceCombo = new D3DDeviceCombo;
        if( pDeviceCombo == NULL )
          return E_OUTOFMEMORY;
        pDeviceCombo->pDepthStencilFormatList = new TArray<D3DFORMAT>;
        pDeviceCombo->pMultiSampleTypeList = new TArray<D3DMULTISAMPLE_TYPE>;
        pDeviceCombo->pMultiSampleQualityList = new TArray<DWORD>;
        pDeviceCombo->pDSMSConflictList = new TArray<D3DDSMSConflict>;
        pDeviceCombo->pVertexProcessingTypeList = new TArray<VertexProcessingType>;
        pDeviceCombo->pPresentIntervalList = new TArray<UINT>;
        if( pDeviceCombo->pDepthStencilFormatList == NULL ||
            pDeviceCombo->pMultiSampleTypeList == NULL || 
            pDeviceCombo->pMultiSampleQualityList == NULL || 
            pDeviceCombo->pDSMSConflictList == NULL || 
            pDeviceCombo->pVertexProcessingTypeList == NULL ||
            pDeviceCombo->pPresentIntervalList == NULL )
        {
          delete pDeviceCombo;
          return E_OUTOFMEMORY;
        }
        pDeviceCombo->AdapterOrdinal = pDeviceInfo->AdapterOrdinal;
        pDeviceCombo->DevType = pDeviceInfo->DevType;
        pDeviceCombo->AdapterFormat = adapterFormat;
        pDeviceCombo->BackBufferFormat = backBufferFormat;
        pDeviceCombo->IsWindowed = isWindowed;
        if (AppUsesDepthBuffer)
        {
          BuildDepthStencilFormatList(pDeviceCombo);
          if (pDeviceCombo->pDepthStencilFormatList->Num() == 0)
          {
            delete pDeviceCombo;
            continue;
          }
        }
        BuildMultiSampleTypeList(pDeviceCombo);
        if (pDeviceCombo->pMultiSampleTypeList->Num() == 0)
        {
          delete pDeviceCombo;
          continue;
        }
        BuildDSMSConflictList(pDeviceCombo);
        BuildVertexProcessingTypeList(pDeviceInfo, pDeviceCombo);
        if (pDeviceCombo->pVertexProcessingTypeList->Num() == 0)
        {
          delete pDeviceCombo;
          continue;
        }
        BuildPresentIntervalList(pDeviceInfo, pDeviceCombo);

        pDeviceInfo->pDeviceComboList->AddElem(pDeviceCombo);
      }
    }
  }

  return S_OK;
}




//-----------------------------------------------------------------------------
// Name: BuildDepthStencilFormatList
// Desc: Adds all depth/stencil formats that are compatible with the device 
//       and app to the given D3DDeviceCombo.
//-----------------------------------------------------------------------------
void CD3DEnumeration::BuildDepthStencilFormatList( D3DDeviceCombo* pDeviceCombo )
{
  const D3DFORMAT depthStencilFormatArray[] = 
  {
      D3DFMT_D16,
      D3DFMT_D15S1,
      D3DFMT_D24X8,
      D3DFMT_D24S8,
      D3DFMT_D24X4S4,
      D3DFMT_D32,
  };
  const UINT depthStencilFormatArrayCount = sizeof(depthStencilFormatArray) / 
                                            sizeof(depthStencilFormatArray[0]);

  D3DFORMAT depthStencilFmt;
  for(UINT idsf=0; idsf<depthStencilFormatArrayCount; idsf++)
  {
    depthStencilFmt = depthStencilFormatArray[idsf];
    if (DepthBits(depthStencilFmt) < AppMinDepthBits)
        continue;
    if (StencilBits(depthStencilFmt) < AppMinStencilBits)
        continue;
    if (SUCCEEDED(m_pD3D->CheckDeviceFormat(pDeviceCombo->AdapterOrdinal, pDeviceCombo->DevType, pDeviceCombo->AdapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, depthStencilFmt)))
    {
      if (SUCCEEDED(m_pD3D->CheckDepthStencilMatch(pDeviceCombo->AdapterOrdinal, pDeviceCombo->DevType, pDeviceCombo->AdapterFormat, pDeviceCombo->BackBufferFormat, depthStencilFmt)))
      {
        pDeviceCombo->pDepthStencilFormatList->AddElem(depthStencilFmt);
      }
    }
  }
}




//-----------------------------------------------------------------------------
// Name: BuildMultiSampleTypeList
// Desc: Adds all multisample types that are compatible with the device and app to
//       the given D3DDeviceCombo.
//-----------------------------------------------------------------------------
void CD3DEnumeration::BuildMultiSampleTypeList( D3DDeviceCombo* pDeviceCombo )
{
  const D3DMULTISAMPLE_TYPE msTypeArray[] = { 
      D3DMULTISAMPLE_NONE,
      D3DMULTISAMPLE_NONMASKABLE,
      D3DMULTISAMPLE_2_SAMPLES,
      D3DMULTISAMPLE_3_SAMPLES,
      D3DMULTISAMPLE_4_SAMPLES,
      D3DMULTISAMPLE_5_SAMPLES,
      D3DMULTISAMPLE_6_SAMPLES,
      D3DMULTISAMPLE_7_SAMPLES,
      D3DMULTISAMPLE_8_SAMPLES,
      D3DMULTISAMPLE_9_SAMPLES,
      D3DMULTISAMPLE_10_SAMPLES,
      D3DMULTISAMPLE_11_SAMPLES,
      D3DMULTISAMPLE_12_SAMPLES,
      D3DMULTISAMPLE_13_SAMPLES,
      D3DMULTISAMPLE_14_SAMPLES,
      D3DMULTISAMPLE_15_SAMPLES,
      D3DMULTISAMPLE_16_SAMPLES,
  };
  const UINT msTypeArrayCount = sizeof(msTypeArray) / sizeof(msTypeArray[0]);

  D3DMULTISAMPLE_TYPE msType;
  DWORD msQuality;
  for(UINT imst=0; imst<msTypeArrayCount; imst++)
  {
    msType = msTypeArray[imst];
    if (SUCCEEDED(m_pD3D->CheckDeviceMultiSampleType(pDeviceCombo->AdapterOrdinal, pDeviceCombo->DevType, pDeviceCombo->BackBufferFormat, pDeviceCombo->IsWindowed, msType, &msQuality)))
    {
      pDeviceCombo->pMultiSampleTypeList->AddElem(msType);
      pDeviceCombo->pMultiSampleQualityList->AddElem(msQuality);
    }
  }
}




//-----------------------------------------------------------------------------
// Name: BuildDSMSConflictList
// Desc: Find any conflicts between the available depth/stencil formats and
//       multisample types.
//-----------------------------------------------------------------------------
void CD3DEnumeration::BuildDSMSConflictList(D3DDeviceCombo* pDeviceCombo)
{
  D3DDSMSConflict DSMSConflict;

  for(int ids=0; ids<pDeviceCombo->pDepthStencilFormatList->Num(); ids++)
  {
    D3DFORMAT dsFmt = pDeviceCombo->pDepthStencilFormatList->Get(ids);
    for(int ims=0; ims< pDeviceCombo->pMultiSampleTypeList->Num(); ims++)
    {
      D3DMULTISAMPLE_TYPE msType = pDeviceCombo->pMultiSampleTypeList->Get(ims);
      if( FAILED( m_pD3D->CheckDeviceMultiSampleType( pDeviceCombo->AdapterOrdinal, pDeviceCombo->DevType, dsFmt, pDeviceCombo->IsWindowed, msType, NULL)))
      {
        DSMSConflict.DSFormat = dsFmt;
        DSMSConflict.MSType = msType;
        pDeviceCombo->pDSMSConflictList->AddElem(DSMSConflict);
      }
    }
  }
}




//-----------------------------------------------------------------------------
// Name: BuildVertexProcessingTypeList
// Desc: Adds all vertex processing types that are compatible with the device 
//       and app to the given D3DDeviceCombo.
//-----------------------------------------------------------------------------
void CD3DEnumeration::BuildVertexProcessingTypeList(D3DDeviceInfo* pDeviceInfo, D3DDeviceCombo* pDeviceCombo)
{
  if ((pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
  {
    if ((pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_PUREDEVICE) != 0)
    {
      if (ConfirmDeviceCallback == NULL || ConfirmDeviceCallback(&pDeviceInfo->Caps, PURE_HARDWARE_VP, pDeviceCombo->AdapterFormat, pDeviceCombo->BackBufferFormat))
      {
        pDeviceCombo->pVertexProcessingTypeList->AddElem(PURE_HARDWARE_VP);
      }
    }
    if (ConfirmDeviceCallback == NULL || ConfirmDeviceCallback(&pDeviceInfo->Caps, HARDWARE_VP, pDeviceCombo->AdapterFormat, pDeviceCombo->BackBufferFormat))
    {
      pDeviceCombo->pVertexProcessingTypeList->AddElem(HARDWARE_VP);
    }
    if (AppUsesMixedVP && (ConfirmDeviceCallback == NULL || ConfirmDeviceCallback(&pDeviceInfo->Caps, MIXED_VP, pDeviceCombo->AdapterFormat, pDeviceCombo->BackBufferFormat)))
    {
      pDeviceCombo->pVertexProcessingTypeList->AddElem(MIXED_VP);
    }
  }
  if (ConfirmDeviceCallback == NULL || ConfirmDeviceCallback(&pDeviceInfo->Caps, SOFTWARE_VP, pDeviceCombo->AdapterFormat, pDeviceCombo->BackBufferFormat))
  {
    pDeviceCombo->pVertexProcessingTypeList->AddElem(SOFTWARE_VP);
  }
}




//-----------------------------------------------------------------------------
// Name: BuildPresentIntervalList
// Desc: Adds all present intervals that are compatible with the device and app 
//       to the given D3DDeviceCombo.
//-----------------------------------------------------------------------------
void CD3DEnumeration::BuildPresentIntervalList(D3DDeviceInfo* pDeviceInfo, D3DDeviceCombo* pDeviceCombo)
{
  const UINT piArray[] = { 
      D3DPRESENT_INTERVAL_IMMEDIATE,
      D3DPRESENT_INTERVAL_DEFAULT,
      D3DPRESENT_INTERVAL_ONE,
      D3DPRESENT_INTERVAL_TWO,
      D3DPRESENT_INTERVAL_THREE,
      D3DPRESENT_INTERVAL_FOUR,
  };
  const UINT piArrayCount = sizeof(piArray) / sizeof(piArray[0]);

  UINT pi;
  for(UINT ipi=0; ipi<piArrayCount; ipi++)
  {
    pi = piArray[ipi];
    if( pDeviceCombo->IsWindowed )
    {
      if( pi == D3DPRESENT_INTERVAL_TWO || pi == D3DPRESENT_INTERVAL_THREE || pi == D3DPRESENT_INTERVAL_FOUR)
      {
        // These intervals are not supported in windowed mode.
        continue;
      }
    }
    // Note that D3DPRESENT_INTERVAL_DEFAULT is zero, so you
    // can't do a caps check for it -- it is always available.
    if( pi == D3DPRESENT_INTERVAL_DEFAULT || (pDeviceInfo->Caps.PresentationIntervals & pi))
    {
      pDeviceCombo->pPresentIntervalList->AddElem(pi);
    }
  }
}

void CD3D9Renderer::SetRendParms(D3DDISPLAYMODE *pModeInfo, D3DDeviceInfo *pDeviceInfo)
{
  m_bFullScreen = !m_D3DSettings.IsWindowed;
  m_abpp = 0;
  if (m_bFullScreen)
  {
    CRenderer::m_width = pModeInfo->Width;
    CRenderer::m_height = pModeInfo->Height;
  }
  else
  {
    pModeInfo->Width = CRenderer::m_width;
    pModeInfo->Height = CRenderer::m_height;
    if (m_cbpp == 32)
      pModeInfo->Format = D3DFMT_A8R8G8B8;
    else
    if (m_cbpp == 24)
      pModeInfo->Format = D3DFMT_X8R8G8B8;
    else
      pModeInfo->Format = D3DFMT_R5G6B5;
  }
  CRenderer::m_abpp = AlphaChannelBits(m_D3DSettings.BackBufferFormat());
  CRenderer::m_cbpp = ColorBits(m_D3DSettings.BackBufferFormat());
  CRenderer::m_sbpp = StencilBits(m_D3DSettings.DepthStencilBufferFormat());
  CRenderer::m_zbpp = DepthBits(m_D3DSettings.DepthStencilBufferFormat());
  if (CV_d3d9_texturebits)
    m_TextureBits = CV_d3d9_texturebits;
  else
    m_TextureBits = CRenderer::m_cbpp;
}

D3DFORMAT CD3D9Renderer::FindBestDepthFormat(D3DDeviceCombo* pBestDeviceCombo)
{
  int i;
  int mindPP = 10000;
  int minsPP = 10000;
  bool bFound = true;
  D3DFORMAT fmt = D3DFMT_UNKNOWN;
  if (CRenderer::m_zbpp < 24)
    CRenderer::m_sbpp = 0;
  for (i=0; i<pBestDeviceCombo->pDepthStencilFormatList->Num(); i++)
  {
    int sPP = StencilBits(pBestDeviceCombo->pDepthStencilFormatList->Get(i));
    int dPP = DepthBits(pBestDeviceCombo->pDepthStencilFormatList->Get(i));
    if (sPP == CRenderer::m_sbpp && dPP == CRenderer::m_zbpp)
    {
      fmt = pBestDeviceCombo->pDepthStencilFormatList->Get(i);
      break;
    }
    else
    if (CRenderer::m_sbpp)
    {
      if (minsPP > abs(sPP-CRenderer::m_sbpp))
      {
        minsPP = abs(sPP-CRenderer::m_sbpp);
        fmt = pBestDeviceCombo->pDepthStencilFormatList->Get(i);
      }
    }
    else
    {
      if (mindPP > abs(dPP-CRenderer::m_zbpp))
      {
        mindPP = abs(dPP-CRenderer::m_zbpp);
        fmt = pBestDeviceCombo->pDepthStencilFormatList->Get(i);
      }
    }
  }
  return fmt;
}

int CD3D9Renderer::FindSuitableDevice(int a, bool bAllowSoft)
{
  int idc;
  if (a > m_D3DEnum.m_pAdapterInfoList->Num())
    a = m_D3DEnum.m_pAdapterInfoList->Num()-1;
  if (a < 0)
    a = 0;
  D3DAdapterInfo* pBestAdapterInfo = NULL;
  D3DDeviceInfo* pBestDeviceInfo = NULL;
  D3DDeviceCombo* pBestDeviceCombo = NULL;
  D3DAdapterInfo *pAdapter = m_D3DEnum.m_pAdapterInfoList->Get(a);

  D3DDISPLAYMODE requiredDisplayMode;
  requiredDisplayMode.Width = CRenderer::m_width;
  requiredDisplayMode.Height = CRenderer::m_height;
  D3DFORMAT BackBufferFormat;
  D3DFORMAT AdapterFormat;
  if (CRenderer::m_cbpp == 32)
  {
    BackBufferFormat = D3DFMT_A8R8G8B8;
    AdapterFormat = D3DFMT_X8R8G8B8;
  }
  else
  if (CRenderer::m_cbpp == 24)
  {
    BackBufferFormat = D3DFMT_X8R8G8B8;
    AdapterFormat = D3DFMT_X8R8G8B8;
  }
  else
  {
    BackBufferFormat = D3DFMT_X8R8G8B8;
    AdapterFormat = D3DFMT_R5G6B5;
  }

  D3DDISPLAYMODE primaryDesktopDisplayMode;
  m_pD3D->GetAdapterDisplayMode(0, &primaryDesktopDisplayMode);

  // Need to find a display mode on the best adapter that uses pBestDeviceCombo->AdapterFormat
  // and is as close to bestAdapterDesktopDisplayMode's res as possible
  D3DDISPLAYMODE bestDisplayMode;
  bestDisplayMode.Width = 0;
  bestDisplayMode.Height = 0;
  bestDisplayMode.Format = D3DFMT_UNKNOWN;
  bestDisplayMode.RefreshRate = 0;
  int d;
  for(d=0; d<pAdapter->pDeviceInfoList->Num(); d++)
  {
    D3DAdapterInfo *pAdapter = m_D3DEnum.m_pAdapterInfoList->Get(a);
    D3DDeviceInfo  *pDevice = pAdapter->pDeviceInfoList->Get(d);
    if ((CV_d3d9_forcesoftware && pDevice->DevType == D3DDEVTYPE_HAL) || (pDevice->DevType != D3DDEVTYPE_HAL && !bAllowSoft))
      continue;
    for(idc=0; idc<pDevice->pDeviceComboList->Num(); idc++)
    {
      D3DDeviceCombo* pDeviceCombo = pDevice->pDeviceComboList->Get(idc);
      if (m_bFullScreen)
      {
        if (pDeviceCombo->IsWindowed)
          continue;
        bool bAdapterMatches = (pDeviceCombo->AdapterFormat == AdapterFormat);
        // If we haven't found a compatible set yet, or if this set
        // is better (because it's a HAL, and/or because formats match better),
        // save it
        if (pBestDeviceCombo == NULL ||
          (pBestDeviceCombo->DevType != D3DDEVTYPE_HAL && pDevice->DevType == D3DDEVTYPE_HAL) ||
          (pDeviceCombo->DevType == D3DDEVTYPE_HAL && pBestDeviceCombo->BackBufferFormat != BackBufferFormat && bAdapterMatches))
        {
          pBestAdapterInfo = pAdapter;
          pBestDeviceInfo = pDevice;
          pBestDeviceCombo = pDeviceCombo;
          if (pDevice->DevType == D3DDEVTYPE_HAL && bAdapterMatches && pBestDeviceCombo->BackBufferFormat == BackBufferFormat)
          {
            // This fullscreen device combo looks great -- take it
            break;
          }
          // Otherwise keep looking for a better fullscreen device combo
        }
      }
      else
      {
        if (!pDeviceCombo->IsWindowed)
          continue;
        if (pDeviceCombo->AdapterFormat != primaryDesktopDisplayMode.Format)
          continue;
        bool bAdapterBBMatchesBB = (pDeviceCombo->BackBufferFormat == BackBufferFormat);
        // If we haven't found a compatible DeviceCombo yet, or if this set
        // is better (because it's a HAL, and/or because formats match better),
        // save it
        if( pBestDeviceCombo == NULL || (bAdapterBBMatchesBB && pDeviceCombo->DevType == D3DDEVTYPE_HAL))
        {
          pBestAdapterInfo = pAdapter;
          pBestDeviceInfo = pDevice;
          pBestDeviceCombo = pDeviceCombo;
          // This windowed device combo looks great -- take it
          if( bAdapterBBMatchesBB )
            break;
        }
      }
    }
    if (idc != pDevice->pDeviceComboList->Num())
      break;
  }
  if (!pBestDeviceCombo)
    return -1;

  if (!m_bFullScreen)
    CRenderer::m_cbpp = ColorBits(pBestDeviceCombo->BackBufferFormat);
  if (CV_d3d9_texturebits)
    m_TextureBits = CV_d3d9_texturebits;
  else
    m_TextureBits = CRenderer::m_cbpp;

  if (m_bFullScreen)
  {
    int BestError = 999999;
    int Best;
    int BestCD;
    for(int idm=0; idm<pAdapter->pDisplayModeList->Num(); idm++)
    {
      D3DDISPLAYMODE* pdm = &pAdapter->pDisplayModeList->Get(idm);
      if(pdm->Format != pBestDeviceCombo->AdapterFormat)
        continue;
      int thisCD = ColorBits(pBestDeviceCombo->AdapterFormat);
      if (thisCD <= 0)
        continue;
      int ThisError = abs((int)pdm->Width-(int)requiredDisplayMode.Width) + abs((int)pdm->Height-(int)requiredDisplayMode.Height) + abs((int)thisCD-(int)CRenderer::m_cbpp);
      if (ThisError < BestError)
      {
        Best = idm;
        BestCD = thisCD;
        BestError = ThisError;
        bestDisplayMode = *pdm;
      }
    }
    if( BestError == 999999 )
      return -1;

    //CRenderer::m_cbpp    = BestCD;
    CRenderer::m_width  = bestDisplayMode.Width;
    CRenderer::m_height = bestDisplayMode.Height;
    iLog->Log ("Best-match display mode: %ix%ix%i (Error=%i)\n",CRenderer::m_width,CRenderer::m_height,CRenderer::m_cbpp,BestError);

    m_D3DSettings.pFullscreen_AdapterInfo = pBestAdapterInfo;
    m_D3DSettings.pFullscreen_DeviceInfo = pBestDeviceInfo;
    m_D3DSettings.pFullscreen_DeviceCombo = pBestDeviceCombo;
    m_D3DSettings.IsWindowed = false;
    m_D3DSettings.Fullscreen_DisplayMode = bestDisplayMode;
    m_D3DSettings.Fullscreen_DisplayMode.RefreshRate = D3DPRESENT_RATE_DEFAULT;
    if (m_D3DEnum.AppUsesDepthBuffer)
    {
      m_D3DSettings.Fullscreen_DepthStencilBufferFormat = FindBestDepthFormat(pBestDeviceCombo);
    }
    m_D3DSettings.Fullscreen_MultisampleType = pBestDeviceCombo->pMultiSampleTypeList->Get(0);
    m_D3DSettings.Fullscreen_MultisampleQuality = 0;
    m_D3DSettings.Fullscreen_VertexProcessingType = pBestDeviceCombo->pVertexProcessingTypeList->Get(0);
    m_D3DSettings.Fullscreen_PresentInterval = D3DPRESENT_INTERVAL_DEFAULT;
  }
  else
  {
    m_D3DSettings.pWindowed_AdapterInfo = pBestAdapterInfo;
    m_D3DSettings.pWindowed_DeviceInfo = pBestDeviceInfo;
    m_D3DSettings.pWindowed_DeviceCombo = pBestDeviceCombo;
    m_D3DSettings.IsWindowed = true;
    m_D3DSettings.Windowed_DisplayMode = primaryDesktopDisplayMode;
    m_D3DSettings.Windowed_Width = CRenderer::m_width;
    m_D3DSettings.Windowed_Height = CRenderer::m_height;
    if (m_D3DEnum.AppUsesDepthBuffer)
    {
      if (CV_d3d9_forcesoftware)
        m_D3DSettings.Windowed_DepthStencilBufferFormat = D3DFMT_D16_LOCKABLE;
      else
        m_D3DSettings.Windowed_DepthStencilBufferFormat = FindBestDepthFormat(pBestDeviceCombo);
    }
    m_D3DSettings.Windowed_MultisampleType = pBestDeviceCombo->pMultiSampleTypeList->Get(0);
    m_D3DSettings.Windowed_MultisampleQuality = 0;
    m_D3DSettings.Windowed_VertexProcessingType = pBestDeviceCombo->pVertexProcessingTypeList->Get(0);
    m_D3DSettings.Windowed_PresentInterval = pBestDeviceCombo->pPresentIntervalList->Get(0);
  }
  return d;
}

