/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace ComTypes
{
/*
    These interfaces would normally be included in the system platform headers.
    However, those headers are likely to be incomplete when building with
    MinGW. In order to allow building video applications under MinGW,
    we reproduce all necessary definitions here.
*/

enum PIN_DIRECTION
{
    PINDIR_INPUT = 0,
    PINDIR_OUTPUT = PINDIR_INPUT + 1
};

enum VMRMode
{
    VMRMode_Windowed = 0x1,
    VMRMode_Windowless = 0x2,
    VMRMode_Renderless = 0x4,
    VMRMode_Mask = 0x7
};

enum VMR_ASPECT_RATIO_MODE
{
    VMR_ARMODE_NONE = 0,
    VMR_ARMODE_LETTER_BOX = VMR_ARMODE_NONE + 1
};

enum MFVideoAspectRatioMode
{
    MFVideoARMode_None = 0,
    MFVideoARMode_PreservePicture = 0x1,
    MFVideoARMode_PreservePixel = 0x2,
    MFVideoARMode_NonLinearStretch = 0x4,
    MFVideoARMode_Mask = 0x7
};

enum FILTER_STATE
{
    State_Stopped = 0,
    State_Paused = State_Stopped + 1,
    State_Running = State_Paused + 1
};

enum WMT_VERSION
{
    WMT_VER_4_0 = 0x40000,
    WMT_VER_7_0 = 0x70000,
    WMT_VER_8_0 = 0x80000,
    WMT_VER_9_0 = 0x90000
};

// We only ever refer to these through a pointer, so we don't need definitions for them.
struct IAMCopyCaptureFileProgress;
struct IBaseFilter;
struct IEnumFilters;
struct IEnumMediaTypes;
struct IReferenceClock;
struct IVMRImageCompositor;

struct FILTER_INFO;

struct AM_MEDIA_TYPE
{
    GUID majortype;
    GUID subtype;
    BOOL bFixedSizeSamples;
    BOOL bTemporalCompression;
    ULONG lSampleSize;
    GUID formattype;
    IUnknown* pUnk;
    ULONG cbFormat;
    BYTE* pbFormat;
};

typedef LONGLONG REFERENCE_TIME;
typedef LONG_PTR OAEVENT;
typedef LONG_PTR OAHWND;
typedef double REFTIME;
typedef long OAFilterState;

enum Constants
{
    EC_STATE_CHANGE = 0x32,
    EC_REPAINT = 0x05,
    EC_COMPLETE = 0x01,
    EC_ERRORABORT = 0x03,
    EC_ERRORABORTEX = 0x45,
    EC_USERABORT = 0x02,

    VFW_E_INVALID_FILE_FORMAT = (HRESULT) 0x8004022FL,
    VFW_E_NOT_FOUND = (HRESULT) 0x80040216L,
    VFW_E_UNKNOWN_FILE_TYPE = (HRESULT) 0x80040240L,
    VFW_E_UNSUPPORTED_STREAM = (HRESULT) 0x80040265L,
    VFW_E_CANNOT_CONNECT = (HRESULT) 0x80040217L,
    VFW_E_CANNOT_LOAD_SOURCE_FILTER = (HRESULT) 0x80040241L,
    VFW_E_NOT_CONNECTED = (HRESULT) 0x80040209L
};

struct MFVideoNormalizedRect
{
    float left;
    float top;
    float right;
    float bottom;
};

struct VIDEOINFOHEADER
{
    RECT rcSource;
    RECT rcTarget;
    DWORD dwBitRate;
    DWORD dwBitErrorRate;
    REFERENCE_TIME AvgTimePerFrame;
    BITMAPINFOHEADER bmiHeader;
};

struct VIDEO_STREAM_CONFIG_CAPS
{
    GUID guid;
    ULONG VideoStandard;
    SIZE InputSize;
    SIZE MinCroppingSize;
    SIZE MaxCroppingSize;
    int CropGranularityX;
    int CropGranularityY;
    int CropAlignX;
    int CropAlignY;
    SIZE MinOutputSize;
    SIZE MaxOutputSize;
    int OutputGranularityX;
    int OutputGranularityY;
    int StretchTapsX;
    int StretchTapsY;
    int ShrinkTapsX;
    int ShrinkTapsY;
    LONGLONG MinFrameInterval;
    LONGLONG MaxFrameInterval;
    LONG MinBitsPerSecond;
    LONG MaxBitsPerSecond;
};

struct PIN_INFO
{
    IBaseFilter* pFilter;
    PIN_DIRECTION dir;
    WCHAR achName[128];
};

JUCE_COMCLASS (ICreateDevEnum, "29840822-5B84-11D0-BD3B-00A0C911CE86") : public IUnknown
{
public:
    JUCE_COMCALL CreateClassEnumerator (REFCLSID clsidDeviceClass, _Out_ IEnumMoniker * *ppEnumMoniker, DWORD dwFlags) = 0;
};

JUCE_COMCLASS (IPin, "56a86891-0ad4-11ce-b03a-0020af0ba770") : public IUnknown
{
public:
    JUCE_COMCALL Connect (IPin * pReceivePin, _In_opt_ const AM_MEDIA_TYPE* pmt) = 0;
    JUCE_COMCALL ReceiveConnection (IPin * pConnector, const AM_MEDIA_TYPE* pmt) = 0;
    JUCE_COMCALL Disconnect() = 0;
    JUCE_COMCALL ConnectedTo (_Out_ IPin * *pPin) = 0;
    JUCE_COMCALL ConnectionMediaType (_Out_ AM_MEDIA_TYPE * pmt) = 0;
    JUCE_COMCALL QueryPinInfo (_Out_ PIN_INFO * pInfo) = 0;
    JUCE_COMCALL QueryDirection (_Out_ PIN_DIRECTION * pPinDir) = 0;
    JUCE_COMCALL QueryId (_Out_ LPWSTR * Id) = 0;
    JUCE_COMCALL QueryAccept (const AM_MEDIA_TYPE* pmt) = 0;
    JUCE_COMCALL EnumMediaTypes (_Out_ IEnumMediaTypes * *ppEnum) = 0;
    JUCE_COMCALL QueryInternalConnections (_Out_writes_to_opt_ (*nPin, *nPin) IPin * *apPin, ULONG * nPin) = 0;
    JUCE_COMCALL EndOfStream() = 0;
    JUCE_COMCALL BeginFlush() = 0;
    JUCE_COMCALL EndFlush() = 0;
    JUCE_COMCALL NewSegment (REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate) = 0;
};

JUCE_COMCLASS (IFilterGraph, "56a8689f-0ad4-11ce-b03a-0020af0ba770") : public IUnknown
{
public:
    JUCE_COMCALL AddFilter (IBaseFilter * pFilter, LPCWSTR pName) = 0;
    JUCE_COMCALL RemoveFilter (IBaseFilter * pFilter) = 0;
    JUCE_COMCALL EnumFilters (_Out_ IEnumFilters * *ppEnum) = 0;
    JUCE_COMCALL FindFilterByName (LPCWSTR pName, _Out_ IBaseFilter * *ppFilter) = 0;
    JUCE_COMCALL ConnectDirect (IPin * ppinOut, IPin * ppinIn, _In_opt_ const AM_MEDIA_TYPE* pmt) = 0;
    JUCE_COMCALL Reconnect (IPin * ppin) = 0;
    JUCE_COMCALL Disconnect (IPin * ppin) = 0;
    JUCE_COMCALL SetDefaultSyncSource() = 0;
};

JUCE_COMCLASS (IGraphBuilder, "56a868a9-0ad4-11ce-b03a-0020af0ba770") : public IFilterGraph
{
public:
    JUCE_COMCALL Connect (IPin * ppinOut, IPin * ppinIn) = 0;
    JUCE_COMCALL Render (IPin * ppinOut) = 0;
    JUCE_COMCALL RenderFile (LPCWSTR lpcwstrFile, _In_opt_ LPCWSTR lpcwstrPlayList) = 0;
    JUCE_COMCALL AddSourceFilter (LPCWSTR lpcwstrFileName, _In_opt_ LPCWSTR lpcwstrFilterName, _Out_ IBaseFilter * *ppFilter) = 0;
    JUCE_COMCALL SetLogFile (DWORD_PTR hFile) = 0;
    JUCE_COMCALL Abort() = 0;
    JUCE_COMCALL ShouldOperationContinue() = 0;
};

JUCE_COMCLASS (IMediaFilter, "56a86899-0ad4-11ce-b03a-0020af0ba770") : public IPersist
{
public:
    JUCE_COMCALL Stop() = 0;
    JUCE_COMCALL Pause() = 0;
    JUCE_COMCALL Run (REFERENCE_TIME tStart) = 0;
    JUCE_COMCALL GetState (DWORD dwMilliSecsTimeout, _Out_ FILTER_STATE * State) = 0;
    JUCE_COMCALL SetSyncSource (_In_opt_ IReferenceClock * pClock) = 0;
    JUCE_COMCALL GetSyncSource (_Outptr_result_maybenull_ IReferenceClock * *pClock) = 0;
};

JUCE_COMCLASS (IEnumPins, "56a86892-0ad4-11ce-b03a-0020af0ba770") : public IUnknown
{
public:
    JUCE_COMCALL Next (ULONG cPins, _Out_writes_to_ (cPins, *pcFetched) IPin * *ppPins, _Out_opt_ ULONG * pcFetched) = 0;
    JUCE_COMCALL Skip (ULONG cPins) = 0;
    JUCE_COMCALL Reset() = 0;
    JUCE_COMCALL Clone (_Out_ IEnumPins * *ppEnum) = 0;
};

JUCE_COMCLASS (IBaseFilter, "56a86895-0ad4-11ce-b03a-0020af0ba770") : public IMediaFilter
{
public:
    JUCE_COMCALL EnumPins (_Out_ IEnumPins * *ppEnum) = 0;
    JUCE_COMCALL FindPin (LPCWSTR Id, _Out_ IPin * *ppPin) = 0;
    JUCE_COMCALL QueryFilterInfo (_Out_ FILTER_INFO * pInfo) = 0;
    JUCE_COMCALL JoinFilterGraph (_In_opt_ IFilterGraph * pGraph, _In_opt_ LPCWSTR pName) = 0;
    JUCE_COMCALL QueryVendorInfo (_Out_ LPWSTR * pVendorInfo) = 0;
};

JUCE_COMCLASS (IVMRWindowlessControl, "0eb1088c-4dcd-46f0-878f-39dae86a51b7") : public IUnknown
{
public:
    JUCE_COMCALL GetNativeVideoSize (LONG * lpWidth, LONG * lpHeight, LONG * lpARWidth, LONG * lpARHeight) = 0;
    JUCE_COMCALL GetMinIdealVideoSize (LONG * lpWidth, LONG * lpHeight) = 0;
    JUCE_COMCALL GetMaxIdealVideoSize (LONG * lpWidth, LONG * lpHeight) = 0;
    JUCE_COMCALL SetVideoPosition (const LPRECT lpSRCRect, const LPRECT lpDSTRect) = 0;
    JUCE_COMCALL GetVideoPosition (LPRECT lpSRCRect, LPRECT lpDSTRect) = 0;
    JUCE_COMCALL GetAspectRatioMode (DWORD * lpAspectRatioMode) = 0;
    JUCE_COMCALL SetAspectRatioMode (DWORD AspectRatioMode) = 0;
    JUCE_COMCALL SetVideoClippingWindow (HWND hwnd) = 0;
    JUCE_COMCALL RepaintVideo (HWND hwnd, HDC hdc) = 0;
    JUCE_COMCALL DisplayModeChanged() = 0;
    JUCE_COMCALL GetCurrentImage (BYTE * *lpDib) = 0;
    JUCE_COMCALL SetBorderColor (COLORREF Clr) = 0;
    JUCE_COMCALL GetBorderColor (COLORREF * lpClr) = 0;
    JUCE_COMCALL SetColorKey (COLORREF Clr) = 0;
    JUCE_COMCALL GetColorKey (COLORREF * lpClr) = 0;
};

JUCE_COMCLASS (IVMRFilterConfig, "9e5530c5-7034-48b4-bb46-0b8a6efc8e36") : public IUnknown
{
public:
    JUCE_COMCALL SetImageCompositor (IVMRImageCompositor * lpVMRImgCompositor) = 0;
    JUCE_COMCALL SetNumberOfStreams (DWORD dwMaxStreams) = 0;
    JUCE_COMCALL GetNumberOfStreams (DWORD * pdwMaxStreams) = 0;
    JUCE_COMCALL SetRenderingPrefs (DWORD dwRenderFlags) = 0;
    JUCE_COMCALL GetRenderingPrefs (DWORD * pdwRenderFlags) = 0;
    JUCE_COMCALL SetRenderingMode (DWORD Mode) = 0;
    JUCE_COMCALL GetRenderingMode (DWORD * pMode) = 0;
};

JUCE_COMCLASS (IMFVideoDisplayControl, "a490b1e4-ab84-4d31-a1b2-181e03b1077a") : public IUnknown
{
public:
    JUCE_COMCALL GetNativeVideoSize (__RPC__inout_opt SIZE * pszVideo, __RPC__inout_opt SIZE * pszARVideo) = 0;
    JUCE_COMCALL GetIdealVideoSize (__RPC__inout_opt SIZE * pszMin, __RPC__inout_opt SIZE * pszMax) = 0;
    JUCE_COMCALL SetVideoPosition (__RPC__in_opt const MFVideoNormalizedRect* pnrcSource, __RPC__in_opt const LPRECT prcDest) = 0;
    JUCE_COMCALL GetVideoPosition (__RPC__out MFVideoNormalizedRect * pnrcSource, __RPC__out LPRECT prcDest) = 0;
    JUCE_COMCALL SetAspectRatioMode (DWORD dwAspectRatioMode) = 0;
    JUCE_COMCALL GetAspectRatioMode (__RPC__out DWORD * pdwAspectRatioMode) = 0;
    JUCE_COMCALL SetVideoWindow (__RPC__in HWND hwndVideo) = 0;
    JUCE_COMCALL GetVideoWindow (__RPC__deref_out_opt HWND * phwndVideo) = 0;
    JUCE_COMCALL RepaintVideo() = 0;
    JUCE_COMCALL GetCurrentImage (__RPC__inout BITMAPINFOHEADER * pBih, __RPC__deref_out_ecount_full_opt (*pcbDib) BYTE * *pDib, __RPC__out DWORD * pcbDib, __RPC__inout_opt LONGLONG * pTimeStamp) = 0;
    JUCE_COMCALL SetBorderColor (COLORREF Clr) = 0;
    JUCE_COMCALL GetBorderColor (__RPC__out COLORREF * pClr) = 0;
    JUCE_COMCALL SetRenderingPrefs (DWORD dwRenderFlags) = 0;
    JUCE_COMCALL GetRenderingPrefs (__RPC__out DWORD * pdwRenderFlags) = 0;
    JUCE_COMCALL SetFullscreen (BOOL fFullscreen) = 0;
    JUCE_COMCALL GetFullscreen (__RPC__out BOOL * pfFullscreen) = 0;
};

JUCE_COMCLASS (IMFGetService, "fa993888-4383-415a-a930-dd472a8cf6f7") : public IUnknown
{
public:
    JUCE_COMCALL GetService (__RPC__in REFGUID guidService, __RPC__in REFIID riid, __RPC__deref_out_opt LPVOID * ppvObject) = 0;
};

JUCE_COMCLASS (IMediaControl, "56a868b1-0ad4-11ce-b03a-0020af0ba770") : public IDispatch
{
public:
    JUCE_COMCALL Run() = 0;
    JUCE_COMCALL Pause() = 0;
    JUCE_COMCALL Stop() = 0;
    JUCE_COMCALL GetState (LONG msTimeout, __RPC__out OAFilterState * pfs) = 0;
    JUCE_COMCALL RenderFile (__RPC__in BSTR strFilename) = 0;
    JUCE_COMCALL AddSourceFilter (__RPC__in BSTR strFilename, __RPC__deref_out_opt IDispatch * *ppUnk) = 0;
    JUCE_COMCALL get_FilterCollection (__RPC__deref_out_opt IDispatch * *ppUnk) = 0;
    JUCE_COMCALL get_RegFilterCollection (__RPC__deref_out_opt IDispatch * *ppUnk) = 0;
    JUCE_COMCALL StopWhenReady() = 0;
};

JUCE_COMCLASS (IMediaPosition, "56a868b2-0ad4-11ce-b03a-0020af0ba770") : public IDispatch
{
public:
    JUCE_COMCALL get_Duration (__RPC__out REFTIME * plength) = 0;
    JUCE_COMCALL put_CurrentPosition (REFTIME llTime) = 0;
    JUCE_COMCALL get_CurrentPosition (__RPC__out REFTIME * pllTime) = 0;
    JUCE_COMCALL get_StopTime (__RPC__out REFTIME * pllTime) = 0;
    JUCE_COMCALL put_StopTime (REFTIME llTime) = 0;
    JUCE_COMCALL get_PrerollTime (__RPC__out REFTIME * pllTime) = 0;
    JUCE_COMCALL put_PrerollTime (REFTIME llTime) = 0;
    JUCE_COMCALL put_Rate (double dRate) = 0;
    JUCE_COMCALL get_Rate (__RPC__out double* pdRate) = 0;
    JUCE_COMCALL CanSeekForward (__RPC__out LONG * pCanSeekForward) = 0;
    JUCE_COMCALL CanSeekBackward (__RPC__out LONG * pCanSeekBackward) = 0;
};

JUCE_COMCLASS (IMediaEvent, "56a868b6-0ad4-11ce-b03a-0020af0ba770") : public IDispatch
{
public:
    JUCE_COMCALL GetEventHandle (__RPC__out OAEVENT * hEvent) = 0;
    JUCE_COMCALL GetEvent (__RPC__out long* lEventCode, __RPC__out LONG_PTR* lParam1, __RPC__out LONG_PTR* lParam2, long msTimeout) = 0;
    JUCE_COMCALL WaitForCompletion (long msTimeout, __RPC__out long* pEvCode) = 0;
    JUCE_COMCALL CancelDefaultHandling (long lEvCode) = 0;
    JUCE_COMCALL RestoreDefaultHandling (long lEvCode) = 0;
    JUCE_COMCALL FreeEventParams (long lEvCode, LONG_PTR lParam1, LONG_PTR lParam2) = 0;
};

JUCE_COMCLASS (IMediaEventEx, "56a868c0-0ad4-11ce-b03a-0020af0ba770") : public IMediaEvent
{
public:
    JUCE_COMCALL SetNotifyWindow (OAHWND hwnd, long lMsg, LONG_PTR lInstanceData) = 0;
    JUCE_COMCALL SetNotifyFlags (long lNoNotifyFlags) = 0;
    JUCE_COMCALL GetNotifyFlags (__RPC__out long* lplNoNotifyFlags) = 0;
};

JUCE_COMCLASS (IBasicAudio, "56a868b3-0ad4-11ce-b03a-0020af0ba770") : public IDispatch
{
public:
    JUCE_COMCALL put_Volume (long lVolume) = 0;
    JUCE_COMCALL get_Volume (__RPC__out long* plVolume) = 0;
    JUCE_COMCALL put_Balance (long lBalance) = 0;
    JUCE_COMCALL get_Balance (__RPC__out long* plBalance) = 0;
};

JUCE_COMCLASS (IMediaSample, "56a8689a-0ad4-11ce-b03a-0020af0ba770") : public IUnknown
{
public:
    JUCE_COMCALL GetPointer (BYTE * *ppBuffer) = 0;
    virtual long STDMETHODCALLTYPE GetSize() = 0;
    JUCE_COMCALL GetTime (_Out_ REFERENCE_TIME * pTimeStart, _Out_ REFERENCE_TIME * pTimeEnd) = 0;
    JUCE_COMCALL SetTime (_In_opt_ REFERENCE_TIME * pTimeStart, _In_opt_ REFERENCE_TIME * pTimeEnd) = 0;
    JUCE_COMCALL IsSyncPoint() = 0;
    JUCE_COMCALL SetSyncPoint (BOOL bIsSyncPoint) = 0;
    JUCE_COMCALL IsPreroll() = 0;
    JUCE_COMCALL SetPreroll (BOOL bIsPreroll) = 0;
    virtual long STDMETHODCALLTYPE GetActualDataLength() = 0;
    JUCE_COMCALL SetActualDataLength (long __MIDL__IMediaSample0000) = 0;
    JUCE_COMCALL GetMediaType (_Out_ AM_MEDIA_TYPE * *ppMediaType) = 0;
    JUCE_COMCALL SetMediaType (_In_ AM_MEDIA_TYPE * pMediaType) = 0;
    JUCE_COMCALL IsDiscontinuity() = 0;
    JUCE_COMCALL SetDiscontinuity (BOOL bDiscontinuity) = 0;
    JUCE_COMCALL GetMediaTime (_Out_ LONGLONG * pTimeStart, _Out_ LONGLONG * pTimeEnd) = 0;
    JUCE_COMCALL SetMediaTime (_In_opt_ LONGLONG * pTimeStart, _In_opt_ LONGLONG * pTimeEnd) = 0;
};

JUCE_COMCLASS (IFileSinkFilter, "a2104830-7c70-11cf-8bce-00aa00a3f1a6") : public IUnknown
{
public:
    JUCE_COMCALL SetFileName (LPCOLESTR pszFileName, _In_opt_ const AM_MEDIA_TYPE* pmt) = 0;
    JUCE_COMCALL GetCurFile (_Out_ LPOLESTR * ppszFileName, _Out_ AM_MEDIA_TYPE * pmt) = 0;
};

JUCE_COMCLASS (ICaptureGraphBuilder2, "93E5A4E0-2D50-11d2-ABFA-00A0C9C6E38D") : public IUnknown
{
public:
    JUCE_COMCALL SetFiltergraph (IGraphBuilder * pfg) = 0;
    JUCE_COMCALL GetFiltergraph (_Out_ IGraphBuilder * *ppfg) = 0;
    JUCE_COMCALL SetOutputFileName (const GUID* pType, LPCOLESTR lpstrFile, _Outptr_ IBaseFilter** ppf, _Outptr_opt_ IFileSinkFilter** ppSink) = 0;
    JUCE_COMCALL FindInterface (_In_opt_ const GUID* pCategory, _In_opt_ const GUID* pType, IBaseFilter* pf, REFIID riid, _Out_ void** ppint) = 0;
    JUCE_COMCALL RenderStream (_In_opt_ const GUID* pCategory, const GUID* pType, IUnknown* pSource, IBaseFilter* pfCompressor, IBaseFilter* pfRenderer) = 0;
    JUCE_COMCALL ControlStream (const GUID* pCategory, const GUID* pType, IBaseFilter* pFilter, _In_opt_ REFERENCE_TIME* pstart, _In_opt_ REFERENCE_TIME* pstop, WORD wStartCookie, WORD wStopCookie) = 0;
    JUCE_COMCALL AllocCapFile (LPCOLESTR lpstr, DWORDLONG dwlSize) = 0;
    JUCE_COMCALL CopyCaptureFile (_In_ LPOLESTR lpwstrOld, _In_ LPOLESTR lpwstrNew, int fAllowEscAbort, IAMCopyCaptureFileProgress* pCallback) = 0;
    JUCE_COMCALL FindPin (IUnknown * pSource, PIN_DIRECTION pindir, _In_opt_ const GUID* pCategory, _In_opt_ const GUID* pType, BOOL fUnconnected, int num, _Out_ IPin** ppPin) = 0;
};

JUCE_COMCLASS (IAMStreamConfig, "C6E13340-30AC-11d0-A18C-00A0C9118956") : public IUnknown
{
public:
    JUCE_COMCALL SetFormat (AM_MEDIA_TYPE * pmt) = 0;
    JUCE_COMCALL GetFormat (_Out_ AM_MEDIA_TYPE * *ppmt) = 0;
    JUCE_COMCALL GetNumberOfCapabilities (_Out_ int* piCount, _Out_ int* piSize) = 0;
    JUCE_COMCALL GetStreamCaps (int iIndex, _Out_ AM_MEDIA_TYPE** ppmt, _Out_ BYTE* pSCC) = 0;
};

JUCE_COMCLASS (ISampleGrabberCB, "0579154A-2B53-4994-B0D0-E773148EFF85") : public IUnknown
{
    JUCE_COMCALL SampleCB (double, ComTypes::IMediaSample*) = 0;
    JUCE_COMCALL BufferCB (double, BYTE*, long) = 0;
};

JUCE_COMCLASS (ISampleGrabber, "6B652FFF-11FE-4fce-92AD-0266B5D7C78F") : public IUnknown
{
    JUCE_COMCALL SetOneShot (BOOL) = 0;
    JUCE_COMCALL SetMediaType (const ComTypes::AM_MEDIA_TYPE*) = 0;
    JUCE_COMCALL GetConnectedMediaType (ComTypes::AM_MEDIA_TYPE*) = 0;
    JUCE_COMCALL SetBufferSamples (BOOL) = 0;
    JUCE_COMCALL GetCurrentBuffer (long*, long*) = 0;
    JUCE_COMCALL GetCurrentSample (ComTypes::IMediaSample**) = 0;
    JUCE_COMCALL SetCallback (ISampleGrabberCB*, long) = 0;
};

JUCE_COMCLASS (IAMLatency, "62EA93BA-EC62-11d2-B770-00C04FB6BD3D") : public IUnknown
{
public:
    JUCE_COMCALL GetLatency (_Out_ REFERENCE_TIME * prtLatency) = 0;
};

JUCE_COMCLASS (IAMPushSource, "F185FE76-E64E-11d2-B76E-00C04FB6BD3D") : public IAMLatency
{
public:
    JUCE_COMCALL GetPushSourceFlags (_Out_ ULONG * pFlags) = 0;
    JUCE_COMCALL SetPushSourceFlags (ULONG Flags) = 0;
    JUCE_COMCALL SetStreamOffset (REFERENCE_TIME rtOffset) = 0;
    JUCE_COMCALL GetStreamOffset (_Out_ REFERENCE_TIME * prtOffset) = 0;
    JUCE_COMCALL GetMaxStreamOffset (_Out_ REFERENCE_TIME * prtMaxOffset) = 0;
    JUCE_COMCALL SetMaxStreamOffset (REFERENCE_TIME rtMaxOffset) = 0;
};

JUCE_COMCLASS (IConfigAsfWriter, "45086030-F7E4-486a-B504-826BB5792A3B") : public IUnknown
{
public:
    JUCE_COMCALL ConfigureFilterUsingProfileId (DWORD dwProfileId) = 0;
    JUCE_COMCALL GetCurrentProfileId (__RPC__out DWORD * pdwProfileId) = 0;
    JUCE_COMCALL ConfigureFilterUsingProfileGuid (__RPC__in REFGUID guidProfile) = 0;
    JUCE_COMCALL GetCurrentProfileGuid (__RPC__out GUID * pProfileGuid) = 0;
    JUCE_COMCALL ConfigureFilterUsingProfile (__RPC__in_opt IWMProfile * pProfile) = 0;
    JUCE_COMCALL GetCurrentProfile (__RPC__deref_out_opt IWMProfile * *ppProfile) = 0;
    JUCE_COMCALL SetIndexMode (BOOL bIndexFile) = 0;
    JUCE_COMCALL GetIndexMode (__RPC__out BOOL * pbIndexFile) = 0;
};

constexpr CLSID CLSID_CaptureGraphBuilder2      = { 0xBF87B6E1, 0x8C27, 0x11d0, { 0xB3, 0xF0, 0x00, 0xAA, 0x00, 0x37, 0x61, 0xC5 } };
constexpr CLSID CLSID_EnhancedVideoRenderer     = { 0xfa10746c, 0x9b63, 0x4b6c, { 0xbc, 0x49, 0xfc, 0x30, 0x0e, 0xa5, 0xf2, 0x56 } };
constexpr CLSID CLSID_FilterGraph               = { 0xe436ebb3, 0x524f, 0x11ce, { 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
constexpr CLSID CLSID_NullRenderer              = { 0xC1F400A4, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
constexpr CLSID CLSID_SampleGrabber             = { 0xC1F400A0, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
constexpr CLSID CLSID_SmartTee                  = { 0xcc58e280, 0x8aa1, 0x11d1, { 0xb3, 0xf1, 0x00, 0xaa, 0x00, 0x37, 0x61, 0xc5 } };
constexpr CLSID CLSID_SystemDeviceEnum          = { 0x62BE5D10, 0x60EB, 0x11d0, { 0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86 } };
constexpr CLSID CLSID_VideoInputDeviceCategory  = { 0x860BB310, 0x5D01, 0x11d0, { 0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86 } };
constexpr CLSID CLSID_VideoMixingRenderer       = { 0xb87beb7b, 0x8d29, 0x423F, { 0xae, 0x4d, 0x65, 0x82, 0xc1, 0x01, 0x75, 0xac } };
constexpr CLSID CLSID_WMAsfWriter               = { 0x7c23220e, 0x55bb, 0x11d3, { 0x8b, 0x16, 0x00, 0xc0, 0x4f, 0xb6, 0xbd, 0x3d } };
constexpr CLSID FORMAT_VideoInfo                = { 0x05589f80, 0xc356, 0x11ce, { 0xbf, 0x01, 0x00, 0xaa, 0x00, 0x55, 0x59, 0x5a } };
constexpr CLSID MEDIASUBTYPE_RGB24              = { 0xe436eb7d, 0x524f, 0x11ce, { 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
constexpr CLSID MEDIATYPE_Video                 = { 0x73646976, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
constexpr CLSID MR_VIDEO_RENDER_SERVICE         = { 0x1092a86c, 0xab1a, 0x459a, { 0xa3, 0x36, 0x83, 0x1f, 0xbc, 0x4d, 0x11, 0xff } };
constexpr CLSID PIN_CATEGORY_CAPTURE            = { 0xfb6c4281, 0x0353, 0x11d1, { 0x90, 0x5f, 0x00, 0x00, 0xc0, 0xcc, 0x16, 0xba } };

} // namespace ComTypes
} // namespace juce

#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL (juce::ComTypes::IAMPushSource,               0xF185FE76, 0xE64E, 0x11d2, 0xB7, 0x6E, 0x00, 0xC0, 0x4F, 0xB6, 0xBD, 0x3D)
__CRT_UUID_DECL (juce::ComTypes::IAMStreamConfig,             0xC6E13340, 0x30AC, 0x11d0, 0xA1, 0x8C, 0x00, 0xA0, 0xC9, 0x11, 0x89, 0x56)
__CRT_UUID_DECL (juce::ComTypes::IBaseFilter,                 0x56a86895, 0x0ad4, 0x11ce, 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70)
__CRT_UUID_DECL (juce::ComTypes::IBasicAudio,                 0x56a868b3, 0x0ad4, 0x11ce, 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70)
__CRT_UUID_DECL (juce::ComTypes::ICaptureGraphBuilder2,       0x93E5A4E0, 0x2D50, 0x11d2, 0xAB, 0xFA, 0x00, 0xA0, 0xC9, 0xC6, 0xE3, 0x8D)
__CRT_UUID_DECL (juce::ComTypes::IConfigAsfWriter,            0x45086030, 0xF7E4, 0x486a, 0xB5, 0x04, 0x82, 0x6B, 0xB5, 0x79, 0x2A, 0x3B)
__CRT_UUID_DECL (juce::ComTypes::ICreateDevEnum,              0x29840822, 0x5B84, 0x11D0, 0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86)
__CRT_UUID_DECL (juce::ComTypes::IFileSinkFilter,             0xa2104830, 0x7c70, 0x11cf, 0x8b, 0xce, 0x00, 0xaa, 0x00, 0xa3, 0xf1, 0xa6)
__CRT_UUID_DECL (juce::ComTypes::IGraphBuilder,               0x56a868a9, 0x0ad4, 0x11ce, 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70)
__CRT_UUID_DECL (juce::ComTypes::IMFGetService,               0xfa993888, 0x4383, 0x415a, 0xa9, 0x30, 0xdd, 0x47, 0x2a, 0x8c, 0xf6, 0xf7)
__CRT_UUID_DECL (juce::ComTypes::IMFVideoDisplayControl,      0xa490b1e4, 0xab84, 0x4d31, 0xa1, 0xb2, 0x18, 0x1e, 0x03, 0xb1, 0x07, 0x7a)
__CRT_UUID_DECL (juce::ComTypes::IMediaControl,               0x56a868b1, 0x0ad4, 0x11ce, 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70)
__CRT_UUID_DECL (juce::ComTypes::IMediaEventEx,               0x56a868c0, 0x0ad4, 0x11ce, 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70)
__CRT_UUID_DECL (juce::ComTypes::IMediaPosition,              0x56a868b2, 0x0ad4, 0x11ce, 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70)
__CRT_UUID_DECL (juce::ComTypes::ISampleGrabber,              0x6B652FFF, 0x11FE, 0x4fce, 0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F)
__CRT_UUID_DECL (juce::ComTypes::ISampleGrabberCB,            0x0579154A, 0x2B53, 0x4994, 0xB0, 0xD0, 0xE7, 0x73, 0x14, 0x8E, 0xFF, 0x85)
__CRT_UUID_DECL (juce::ComTypes::IVMRFilterConfig,            0x9e5530c5, 0x7034, 0x48b4, 0xbb, 0x46, 0x0b, 0x8a, 0x6e, 0xfc, 0x8e, 0x36)
__CRT_UUID_DECL (juce::ComTypes::IVMRWindowlessControl,       0x0eb1088c, 0x4dcd, 0x46f0, 0x87, 0x8f, 0x39, 0xda, 0xe8, 0x6a, 0x51, 0xb7)
#endif
