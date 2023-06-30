#ifndef D3DSETTINGS_H
#define D3DSETTINGS_H


//-----------------------------------------------------------------------------
// Name: class CD3DSettings
// Desc: Current D3D settings: adapter, device, mode, formats, etc.
//-----------------------------------------------------------------------------
class CD3DSettings 
{
public:
    bool IsWindowed;

    D3DAdapterInfo* pWindowed_AdapterInfo;
    D3DDeviceInfo* pWindowed_DeviceInfo;
    D3DDeviceCombo* pWindowed_DeviceCombo;

    D3DDISPLAYMODE Windowed_DisplayMode; // not changable by the user
    D3DFORMAT Windowed_DepthStencilBufferFormat;
    D3DMULTISAMPLE_TYPE Windowed_MultisampleType;
    DWORD Windowed_MultisampleQuality;
    VertexProcessingType Windowed_VertexProcessingType;
    UINT Windowed_PresentInterval;
    int Windowed_Width;
    int Windowed_Height;

    D3DAdapterInfo* pFullscreen_AdapterInfo;
    D3DDeviceInfo* pFullscreen_DeviceInfo;
    D3DDeviceCombo* pFullscreen_DeviceCombo;

    D3DDISPLAYMODE Fullscreen_DisplayMode; // changable by the user
    D3DFORMAT Fullscreen_DepthStencilBufferFormat;
    D3DMULTISAMPLE_TYPE Fullscreen_MultisampleType;
    DWORD Fullscreen_MultisampleQuality;
    VertexProcessingType Fullscreen_VertexProcessingType;
    UINT Fullscreen_PresentInterval;

    D3DAdapterInfo* PAdapterInfo() { return IsWindowed ? pWindowed_AdapterInfo : pFullscreen_AdapterInfo; }
    D3DDeviceInfo* PDeviceInfo() { return IsWindowed ? pWindowed_DeviceInfo : pFullscreen_DeviceInfo; }
    D3DDeviceCombo* PDeviceCombo() { return IsWindowed ? pWindowed_DeviceCombo : pFullscreen_DeviceCombo; }

    int AdapterOrdinal() { return PDeviceCombo()->AdapterOrdinal; }
    D3DDEVTYPE DevType() { return PDeviceCombo()->DevType; }
    D3DFORMAT BackBufferFormat() { return PDeviceCombo()->BackBufferFormat; }
    D3DFORMAT AdapterFormat() { return PDeviceCombo()->AdapterFormat; }

    D3DDISPLAYMODE DisplayMode() { return IsWindowed ? Windowed_DisplayMode : Fullscreen_DisplayMode; }
    void SetDisplayMode(D3DDISPLAYMODE value) { if (IsWindowed) Windowed_DisplayMode = value; else Fullscreen_DisplayMode = value; }

    D3DFORMAT DepthStencilBufferFormat() { return IsWindowed ? Windowed_DepthStencilBufferFormat : Fullscreen_DepthStencilBufferFormat; }
    void SetDepthStencilBufferFormat(D3DFORMAT value) { if (IsWindowed) Windowed_DepthStencilBufferFormat = value; else Fullscreen_DepthStencilBufferFormat = value; }

    D3DMULTISAMPLE_TYPE MultisampleType() { return IsWindowed ? Windowed_MultisampleType : Fullscreen_MultisampleType; }
    void SetMultisampleType(D3DMULTISAMPLE_TYPE value) { if (IsWindowed) Windowed_MultisampleType = value; else Fullscreen_MultisampleType = value; }

    DWORD MultisampleQuality() { return IsWindowed ? Windowed_MultisampleQuality : Fullscreen_MultisampleQuality; }
    void SetMultisampleQuality(DWORD value) { if (IsWindowed) Windowed_MultisampleQuality = value; else Fullscreen_MultisampleQuality = value; }

    VertexProcessingType GetVertexProcessingType() { return IsWindowed ? Windowed_VertexProcessingType : Fullscreen_VertexProcessingType; }
    void SetVertexProcessingType(VertexProcessingType value) { if (IsWindowed) Windowed_VertexProcessingType = value; else Fullscreen_VertexProcessingType = value; }

    UINT PresentInterval() { return IsWindowed ? Windowed_PresentInterval : Fullscreen_PresentInterval; }
    void SetPresentInterval(UINT value) { if (IsWindowed) Windowed_PresentInterval = value; else Fullscreen_PresentInterval = value; }
};





#endif



