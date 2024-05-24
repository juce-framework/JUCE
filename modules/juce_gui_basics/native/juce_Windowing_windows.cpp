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

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-function-type")

#undef GetSystemMetrics // multimon overrides this for some reason and causes a mess..

// these are in the windows SDK, but need to be repeated here for GCC..
#ifndef GET_APPCOMMAND_LPARAM
 #define GET_APPCOMMAND_LPARAM(lParam)     ((short) (HIWORD (lParam) & ~FAPPCOMMAND_MASK))

 #define FAPPCOMMAND_MASK                  0xF000
 #define APPCOMMAND_MEDIA_NEXTTRACK        11
 #define APPCOMMAND_MEDIA_PREVIOUSTRACK    12
 #define APPCOMMAND_MEDIA_STOP             13
 #define APPCOMMAND_MEDIA_PLAY_PAUSE       14
#endif

#ifndef WM_APPCOMMAND
 #define WM_APPCOMMAND                     0x0319
#endif

void juce_repeatLastProcessPriority();

using CheckEventBlockedByModalComps = bool (*) (const MSG&);
extern CheckEventBlockedByModalComps isEventBlockedByModalComps;

static bool shouldDeactivateTitleBar = true;

void* getUser32Function (const char*);

#if JUCE_DEBUG
 int numActiveScopedDpiAwarenessDisablers = 0;
 static bool isInScopedDPIAwarenessDisabler() { return numActiveScopedDpiAwarenessDisablers > 0; }
 extern HWND juce_messageWindowHandle;
#endif

struct ScopedDeviceContext
{
    explicit ScopedDeviceContext (HWND h)
        : hwnd (h), dc (GetDC (hwnd))
    {
    }

    ~ScopedDeviceContext()
    {
        ReleaseDC (hwnd, dc);
    }

    HWND hwnd;
    HDC dc;

    JUCE_DECLARE_NON_COPYABLE (ScopedDeviceContext)
    JUCE_DECLARE_NON_MOVEABLE (ScopedDeviceContext)
};

//==============================================================================
#ifndef WM_TOUCH
 enum
 {
     WM_TOUCH         = 0x0240,
     TOUCHEVENTF_MOVE = 0x0001,
     TOUCHEVENTF_DOWN = 0x0002,
     TOUCHEVENTF_UP   = 0x0004
 };

 typedef HANDLE HTOUCHINPUT;
 typedef HANDLE HGESTUREINFO;

 struct TOUCHINPUT
 {
     LONG         x;
     LONG         y;
     HANDLE       hSource;
     DWORD        dwID;
     DWORD        dwFlags;
     DWORD        dwMask;
     DWORD        dwTime;
     ULONG_PTR    dwExtraInfo;
     DWORD        cxContact;
     DWORD        cyContact;
 };

 struct GESTUREINFO
 {
     UINT         cbSize;
     DWORD        dwFlags;
     DWORD        dwID;
     HWND         hwndTarget;
     POINTS       ptsLocation;
     DWORD        dwInstanceID;
     DWORD        dwSequenceID;
     ULONGLONG    ullArguments;
     UINT         cbExtraArgs;
 };
#endif

#ifndef WM_NCPOINTERUPDATE
 enum
 {
     WM_NCPOINTERUPDATE       = 0x241,
     WM_NCPOINTERDOWN         = 0x242,
     WM_NCPOINTERUP           = 0x243,
     WM_POINTERUPDATE         = 0x245,
     WM_POINTERDOWN           = 0x246,
     WM_POINTERUP             = 0x247,
     WM_POINTERENTER          = 0x249,
     WM_POINTERLEAVE          = 0x24A,
     WM_POINTERACTIVATE       = 0x24B,
     WM_POINTERCAPTURECHANGED = 0x24C,
     WM_TOUCHHITTESTING       = 0x24D,
     WM_POINTERWHEEL          = 0x24E,
     WM_POINTERHWHEEL         = 0x24F,
     WM_POINTERHITTEST        = 0x250
 };

 enum
 {
     PT_TOUCH = 0x00000002,
     PT_PEN   = 0x00000003
 };

 enum POINTER_BUTTON_CHANGE_TYPE
 {
     POINTER_CHANGE_NONE,
     POINTER_CHANGE_FIRSTBUTTON_DOWN,
     POINTER_CHANGE_FIRSTBUTTON_UP,
     POINTER_CHANGE_SECONDBUTTON_DOWN,
     POINTER_CHANGE_SECONDBUTTON_UP,
     POINTER_CHANGE_THIRDBUTTON_DOWN,
     POINTER_CHANGE_THIRDBUTTON_UP,
     POINTER_CHANGE_FOURTHBUTTON_DOWN,
     POINTER_CHANGE_FOURTHBUTTON_UP,
     POINTER_CHANGE_FIFTHBUTTON_DOWN,
     POINTER_CHANGE_FIFTHBUTTON_UP
 };

 enum
 {
     PEN_MASK_NONE      = 0x00000000,
     PEN_MASK_PRESSURE  = 0x00000001,
     PEN_MASK_ROTATION  = 0x00000002,
     PEN_MASK_TILT_X    = 0x00000004,
     PEN_MASK_TILT_Y    = 0x00000008
 };

 enum
 {
     TOUCH_MASK_NONE        = 0x00000000,
     TOUCH_MASK_CONTACTAREA = 0x00000001,
     TOUCH_MASK_ORIENTATION = 0x00000002,
     TOUCH_MASK_PRESSURE    = 0x00000004
 };

 enum
 {
     POINTER_FLAG_NONE           = 0x00000000,
     POINTER_FLAG_NEW            = 0x00000001,
     POINTER_FLAG_INRANGE        = 0x00000002,
     POINTER_FLAG_INCONTACT      = 0x00000004,
     POINTER_FLAG_FIRSTBUTTON    = 0x00000010,
     POINTER_FLAG_SECONDBUTTON   = 0x00000020,
     POINTER_FLAG_THIRDBUTTON    = 0x00000040,
     POINTER_FLAG_FOURTHBUTTON   = 0x00000080,
     POINTER_FLAG_FIFTHBUTTON    = 0x00000100,
     POINTER_FLAG_PRIMARY        = 0x00002000,
     POINTER_FLAG_CONFIDENCE     = 0x00004000,
     POINTER_FLAG_CANCELED       = 0x00008000,
     POINTER_FLAG_DOWN           = 0x00010000,
     POINTER_FLAG_UPDATE         = 0x00020000,
     POINTER_FLAG_UP             = 0x00040000,
     POINTER_FLAG_WHEEL          = 0x00080000,
     POINTER_FLAG_HWHEEL         = 0x00100000,
     POINTER_FLAG_CAPTURECHANGED = 0x00200000,
     POINTER_FLAG_HASTRANSFORM   = 0x00400000
 };

 typedef DWORD  POINTER_INPUT_TYPE;
 typedef UINT32 POINTER_FLAGS;
 typedef UINT32 PEN_FLAGS;
 typedef UINT32 PEN_MASK;
 typedef UINT32 TOUCH_FLAGS;
 typedef UINT32 TOUCH_MASK;

 struct POINTER_INFO
 {
     POINTER_INPUT_TYPE         pointerType;
     UINT32                     pointerId;
     UINT32                     frameId;
     POINTER_FLAGS              pointerFlags;
     HANDLE                     sourceDevice;
     HWND                       hwndTarget;
     POINT                      ptPixelLocation;
     POINT                      ptHimetricLocation;
     POINT                      ptPixelLocationRaw;
     POINT                      ptHimetricLocationRaw;
     DWORD                      dwTime;
     UINT32                     historyCount;
     INT32                      InputData;
     DWORD                      dwKeyStates;
     UINT64                     PerformanceCount;
     POINTER_BUTTON_CHANGE_TYPE ButtonChangeType;
 };

 struct POINTER_TOUCH_INFO
 {
     POINTER_INFO    pointerInfo;
     TOUCH_FLAGS     touchFlags;
     TOUCH_MASK      touchMask;
     RECT            rcContact;
     RECT            rcContactRaw;
     UINT32          orientation;
     UINT32          pressure;
 };

 struct POINTER_PEN_INFO
 {
     POINTER_INFO    pointerInfo;
     PEN_FLAGS       penFlags;
     PEN_MASK        penMask;
     UINT32          pressure;
     UINT32          rotation;
     INT32           tiltX;
     INT32           tiltY;
 };

 #define GET_POINTERID_WPARAM(wParam)    (LOWORD(wParam))
#endif

#ifndef MONITOR_DPI_TYPE
 enum Monitor_DPI_Type
 {
     MDT_Effective_DPI = 0,
     MDT_Angular_DPI   = 1,
     MDT_Raw_DPI       = 2,
     MDT_Default       = MDT_Effective_DPI
 };
#endif

#ifndef DPI_AWARENESS
 enum DPI_Awareness
 {
     DPI_Awareness_Invalid           = -1,
     DPI_Awareness_Unaware           = 0,
     DPI_Awareness_System_Aware      = 1,
     DPI_Awareness_Per_Monitor_Aware = 2
 };
#endif

#ifndef USER_DEFAULT_SCREEN_DPI
 #define USER_DEFAULT_SCREEN_DPI 96
#endif

#ifndef _DPI_AWARENESS_CONTEXTS_
 typedef HANDLE DPI_AWARENESS_CONTEXT;

 #define DPI_AWARENESS_CONTEXT_UNAWARE              ((DPI_AWARENESS_CONTEXT) - 1)
 #define DPI_AWARENESS_CONTEXT_SYSTEM_AWARE         ((DPI_AWARENESS_CONTEXT) - 2)
 #define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE    ((DPI_AWARENESS_CONTEXT) - 3)
 #define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT) - 4)
#endif

// Some versions of the Windows 10 SDK define _DPI_AWARENESS_CONTEXTS_ but not
// DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
 #define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT) - 4)
#endif

//==============================================================================
using RegisterTouchWindowFunc    = BOOL (WINAPI*) (HWND, ULONG);
using GetTouchInputInfoFunc      = BOOL (WINAPI*) (HTOUCHINPUT, UINT, TOUCHINPUT*, int);
using CloseTouchInputHandleFunc  = BOOL (WINAPI*) (HTOUCHINPUT);
using GetGestureInfoFunc         = BOOL (WINAPI*) (HGESTUREINFO, GESTUREINFO*);

static RegisterTouchWindowFunc   registerTouchWindow   = nullptr;
static GetTouchInputInfoFunc     getTouchInputInfo     = nullptr;
static CloseTouchInputHandleFunc closeTouchInputHandle = nullptr;
static GetGestureInfoFunc        getGestureInfo        = nullptr;

static bool hasCheckedForMultiTouch = false;

static bool canUseMultiTouch()
{
    if (registerTouchWindow == nullptr && ! hasCheckedForMultiTouch)
    {
        hasCheckedForMultiTouch = true;

        registerTouchWindow   = (RegisterTouchWindowFunc)   getUser32Function ("RegisterTouchWindow");
        getTouchInputInfo     = (GetTouchInputInfoFunc)     getUser32Function ("GetTouchInputInfo");
        closeTouchInputHandle = (CloseTouchInputHandleFunc) getUser32Function ("CloseTouchInputHandle");
        getGestureInfo        = (GetGestureInfoFunc)        getUser32Function ("GetGestureInfo");
    }

    return registerTouchWindow != nullptr;
}

//==============================================================================
using GetPointerTypeFunc       =  BOOL (WINAPI*) (UINT32, POINTER_INPUT_TYPE*);
using GetPointerTouchInfoFunc  =  BOOL (WINAPI*) (UINT32, POINTER_TOUCH_INFO*);
using GetPointerPenInfoFunc    =  BOOL (WINAPI*) (UINT32, POINTER_PEN_INFO*);

static GetPointerTypeFunc      getPointerTypeFunction = nullptr;
static GetPointerTouchInfoFunc getPointerTouchInfo    = nullptr;
static GetPointerPenInfoFunc   getPointerPenInfo      = nullptr;

static bool canUsePointerAPI = false;

static void checkForPointerAPI()
{
    getPointerTypeFunction = (GetPointerTypeFunc)      getUser32Function ("GetPointerType");
    getPointerTouchInfo    = (GetPointerTouchInfoFunc) getUser32Function ("GetPointerTouchInfo");
    getPointerPenInfo      = (GetPointerPenInfoFunc)   getUser32Function ("GetPointerPenInfo");

    canUsePointerAPI = (getPointerTypeFunction != nullptr
                     && getPointerTouchInfo    != nullptr
                     && getPointerPenInfo      != nullptr);
}

//==============================================================================
using SetProcessDPIAwareFunc                   = BOOL                  (WINAPI*) ();
using SetProcessDPIAwarenessContextFunc        = BOOL                  (WINAPI*) (DPI_AWARENESS_CONTEXT);
using SetProcessDPIAwarenessFunc               = HRESULT               (WINAPI*) (DPI_Awareness);
using SetThreadDPIAwarenessContextFunc         = DPI_AWARENESS_CONTEXT (WINAPI*) (DPI_AWARENESS_CONTEXT);
using GetDPIForWindowFunc                      = UINT                  (WINAPI*) (HWND);
using GetDPIForMonitorFunc                     = HRESULT               (WINAPI*) (HMONITOR, Monitor_DPI_Type, UINT*, UINT*);
using GetSystemMetricsForDpiFunc               = int                   (WINAPI*) (int, UINT);
using GetProcessDPIAwarenessFunc               = HRESULT               (WINAPI*) (HANDLE, DPI_Awareness*);
using GetWindowDPIAwarenessContextFunc         = DPI_AWARENESS_CONTEXT (WINAPI*) (HWND);
using GetThreadDPIAwarenessContextFunc         = DPI_AWARENESS_CONTEXT (WINAPI*) ();
using GetAwarenessFromDpiAwarenessContextFunc  = DPI_Awareness         (WINAPI*) (DPI_AWARENESS_CONTEXT);
using EnableNonClientDPIScalingFunc            = BOOL                  (WINAPI*) (HWND);

static SetProcessDPIAwareFunc                  setProcessDPIAware                  = nullptr;
static SetProcessDPIAwarenessContextFunc       setProcessDPIAwarenessContext       = nullptr;
static SetProcessDPIAwarenessFunc              setProcessDPIAwareness              = nullptr;
static SetThreadDPIAwarenessContextFunc        setThreadDPIAwarenessContext        = nullptr;
static GetDPIForMonitorFunc                    getDPIForMonitor                    = nullptr;
static GetDPIForWindowFunc                     getDPIForWindow                     = nullptr;
static GetProcessDPIAwarenessFunc              getProcessDPIAwareness              = nullptr;
static GetWindowDPIAwarenessContextFunc        getWindowDPIAwarenessContext        = nullptr;
static GetThreadDPIAwarenessContextFunc        getThreadDPIAwarenessContext        = nullptr;
static GetAwarenessFromDpiAwarenessContextFunc getAwarenessFromDPIAwarenessContext = nullptr;
static EnableNonClientDPIScalingFunc           enableNonClientDPIScaling           = nullptr;

static bool hasCheckedForDPIAwareness = false;

static void loadDPIAwarenessFunctions()
{
    setProcessDPIAware = (SetProcessDPIAwareFunc) getUser32Function ("SetProcessDPIAware");

    constexpr auto shcore = "SHCore.dll";
    LoadLibraryA (shcore);
    const auto shcoreModule = GetModuleHandleA (shcore);

    if (shcoreModule == nullptr)
        return;

    getDPIForMonitor                    = (GetDPIForMonitorFunc) GetProcAddress (shcoreModule, "GetDpiForMonitor");
    setProcessDPIAwareness              = (SetProcessDPIAwarenessFunc) GetProcAddress (shcoreModule, "SetProcessDpiAwareness");

   #if JUCE_WIN_PER_MONITOR_DPI_AWARE
    getDPIForWindow                     = (GetDPIForWindowFunc) getUser32Function ("GetDpiForWindow");
    getProcessDPIAwareness              = (GetProcessDPIAwarenessFunc) GetProcAddress (shcoreModule, "GetProcessDpiAwareness");
    getWindowDPIAwarenessContext        = (GetWindowDPIAwarenessContextFunc) getUser32Function ("GetWindowDpiAwarenessContext");
    setThreadDPIAwarenessContext        = (SetThreadDPIAwarenessContextFunc) getUser32Function ("SetThreadDpiAwarenessContext");
    getThreadDPIAwarenessContext        = (GetThreadDPIAwarenessContextFunc) getUser32Function ("GetThreadDpiAwarenessContext");
    getAwarenessFromDPIAwarenessContext = (GetAwarenessFromDpiAwarenessContextFunc) getUser32Function ("GetAwarenessFromDpiAwarenessContext");
    setProcessDPIAwarenessContext       = (SetProcessDPIAwarenessContextFunc) getUser32Function ("SetProcessDpiAwarenessContext");
    enableNonClientDPIScaling           = (EnableNonClientDPIScalingFunc) getUser32Function ("EnableNonClientDpiScaling");
   #endif
}

static void setDPIAwareness()
{
    if (hasCheckedForDPIAwareness)
        return;

    hasCheckedForDPIAwareness = true;

    if (! JUCEApplicationBase::isStandaloneApp())
        return;

    loadDPIAwarenessFunctions();

    if (setProcessDPIAwarenessContext != nullptr
        && setProcessDPIAwarenessContext (DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
        return;

    if (setProcessDPIAwareness != nullptr && enableNonClientDPIScaling != nullptr
        && SUCCEEDED (setProcessDPIAwareness (DPI_Awareness::DPI_Awareness_Per_Monitor_Aware)))
        return;

    if (setProcessDPIAwareness != nullptr && getDPIForMonitor != nullptr
        && SUCCEEDED (setProcessDPIAwareness (DPI_Awareness::DPI_Awareness_System_Aware)))
        return;

    NullCheckedInvocation::invoke (setProcessDPIAware);
}

static bool isPerMonitorDPIAwareProcess()
{
   #if ! JUCE_WIN_PER_MONITOR_DPI_AWARE
    return false;
   #else
    static bool dpiAware = []() -> bool
    {
        setDPIAwareness();

        if (! JUCEApplication::isStandaloneApp())
            return false;

        if (getProcessDPIAwareness == nullptr)
            return false;

        DPI_Awareness context;
        getProcessDPIAwareness (nullptr, &context);

        return context == DPI_Awareness::DPI_Awareness_Per_Monitor_Aware;
    }();

    return dpiAware;
   #endif
}

static bool isPerMonitorDPIAwareWindow ([[maybe_unused]] HWND nativeWindow)
{
   #if ! JUCE_WIN_PER_MONITOR_DPI_AWARE
    return false;
   #else
    setDPIAwareness();

    if (getWindowDPIAwarenessContext != nullptr
        && getAwarenessFromDPIAwarenessContext != nullptr)
    {
        return (getAwarenessFromDPIAwarenessContext (getWindowDPIAwarenessContext (nativeWindow))
                  == DPI_Awareness::DPI_Awareness_Per_Monitor_Aware);
    }

    return isPerMonitorDPIAwareProcess();
   #endif
}

static bool isPerMonitorDPIAwareThread (GetThreadDPIAwarenessContextFunc getThreadDPIAwarenessContextIn = getThreadDPIAwarenessContext,
                                        GetAwarenessFromDpiAwarenessContextFunc getAwarenessFromDPIAwarenessContextIn = getAwarenessFromDPIAwarenessContext)
{
   #if ! JUCE_WIN_PER_MONITOR_DPI_AWARE
    return false;
   #else
    setDPIAwareness();

    if (getThreadDPIAwarenessContextIn != nullptr
        && getAwarenessFromDPIAwarenessContextIn != nullptr)
    {
        return (getAwarenessFromDPIAwarenessContextIn (getThreadDPIAwarenessContextIn())
                  == DPI_Awareness::DPI_Awareness_Per_Monitor_Aware);
    }

    return isPerMonitorDPIAwareProcess();
   #endif
}

static double getGlobalDPI()
{
    setDPIAwareness();

    ScopedDeviceContext deviceContext { nullptr };
    return (GetDeviceCaps (deviceContext.dc, LOGPIXELSX) + GetDeviceCaps (deviceContext.dc, LOGPIXELSY)) / 2.0;
}

//==============================================================================
class ScopedSuspendResumeNotificationRegistration
{
    static auto& getFunctions()
    {
        struct Functions
        {
            using Register = HPOWERNOTIFY (WINAPI*) (HANDLE, DWORD);
            using Unregister = BOOL (WINAPI*) (HPOWERNOTIFY);

            Register registerNotification = (Register) getUser32Function ("RegisterSuspendResumeNotification");
            Unregister unregisterNotification = (Unregister) getUser32Function ("UnregisterSuspendResumeNotification");

            bool isValid() const { return registerNotification != nullptr && unregisterNotification != nullptr; }

            Functions() = default;
            JUCE_DECLARE_NON_COPYABLE (Functions)
            JUCE_DECLARE_NON_MOVEABLE (Functions)
        };

        static const Functions functions;
        return functions;
    }

public:
    ScopedSuspendResumeNotificationRegistration() = default;

    explicit ScopedSuspendResumeNotificationRegistration (HWND window)
        : handle (getFunctions().isValid()
                      ? getFunctions().registerNotification (window, DEVICE_NOTIFY_WINDOW_HANDLE)
                      : nullptr)
    {}

private:
    struct Destructor
    {
        void operator() (HPOWERNOTIFY ptr) const
        {
            if (ptr != nullptr)
                getFunctions().unregisterNotification (ptr);
        }
    };

    std::unique_ptr<std::remove_pointer_t<HPOWERNOTIFY>, Destructor> handle;
};

//==============================================================================
class ScopedThreadDPIAwarenessSetter::NativeImpl
{
public:
    static auto& getFunctions()
    {
        struct Functions
        {
            SetThreadDPIAwarenessContextFunc setThreadAwareness             = (SetThreadDPIAwarenessContextFunc) getUser32Function ("SetThreadDpiAwarenessContext");
            GetWindowDPIAwarenessContextFunc getWindowAwareness             = (GetWindowDPIAwarenessContextFunc) getUser32Function ("GetWindowDpiAwarenessContext");
            GetThreadDPIAwarenessContextFunc getThreadAwareness             = (GetThreadDPIAwarenessContextFunc) getUser32Function ("GetThreadDpiAwarenessContext");
            GetAwarenessFromDpiAwarenessContextFunc getAwarenessFromContext = (GetAwarenessFromDpiAwarenessContextFunc) getUser32Function ("GetAwarenessFromDpiAwarenessContext");

            bool isLoaded() const noexcept
            {
                return setThreadAwareness != nullptr
                    && getWindowAwareness != nullptr
                    && getThreadAwareness != nullptr
                    && getAwarenessFromContext != nullptr;
            }

            Functions() = default;
            JUCE_DECLARE_NON_COPYABLE (Functions)
            JUCE_DECLARE_NON_MOVEABLE (Functions)
        };

        static const Functions functions;
        return functions;
    }

    explicit NativeImpl (HWND nativeWindow [[maybe_unused]])
    {
       #if JUCE_WIN_PER_MONITOR_DPI_AWARE
        if (const auto& functions = getFunctions(); functions.isLoaded())
        {
            auto dpiAwareWindow = (functions.getAwarenessFromContext (functions.getWindowAwareness (nativeWindow))
                                   == DPI_Awareness::DPI_Awareness_Per_Monitor_Aware);

            auto dpiAwareThread = (functions.getAwarenessFromContext (functions.getThreadAwareness())
                                   == DPI_Awareness::DPI_Awareness_Per_Monitor_Aware);

            if (dpiAwareWindow && ! dpiAwareThread)
                oldContext = functions.setThreadAwareness (DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
            else if (! dpiAwareWindow && dpiAwareThread)
                oldContext = functions.setThreadAwareness (DPI_AWARENESS_CONTEXT_UNAWARE);
        }
       #endif
    }

    ~NativeImpl()
    {
        if (oldContext != nullptr)
            getFunctions().setThreadAwareness (oldContext);
    }

private:
    DPI_AWARENESS_CONTEXT oldContext = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeImpl)
    JUCE_DECLARE_NON_MOVEABLE (NativeImpl)
};

ScopedThreadDPIAwarenessSetter::ScopedThreadDPIAwarenessSetter (void* nativeWindow)
{
    pimpl = std::make_unique<NativeImpl> ((HWND) nativeWindow);
}

ScopedThreadDPIAwarenessSetter::~ScopedThreadDPIAwarenessSetter() = default;

static auto& getScopedDPIAwarenessDisablerFunctions()
{
    struct Functions
    {
        GetThreadDPIAwarenessContextFunc localGetThreadDpiAwarenessContext = (GetThreadDPIAwarenessContextFunc) getUser32Function ("GetThreadDpiAwarenessContext");
        GetAwarenessFromDpiAwarenessContextFunc localGetAwarenessFromDpiAwarenessContextFunc = (GetAwarenessFromDpiAwarenessContextFunc) getUser32Function ("GetAwarenessFromDpiAwarenessContext");
        SetThreadDPIAwarenessContextFunc localSetThreadDPIAwarenessContext = (SetThreadDPIAwarenessContextFunc) getUser32Function ("SetThreadDpiAwarenessContext");

        Functions() = default;
        JUCE_DECLARE_NON_COPYABLE (Functions)
        JUCE_DECLARE_NON_MOVEABLE (Functions)
    };

    static const Functions functions;
    return functions;
}

ScopedDPIAwarenessDisabler::ScopedDPIAwarenessDisabler()
{
    const auto& functions = getScopedDPIAwarenessDisablerFunctions();

    if (! isPerMonitorDPIAwareThread (functions.localGetThreadDpiAwarenessContext, functions.localGetAwarenessFromDpiAwarenessContextFunc))
        return;

    if (auto* localSetThreadDPIAwarenessContext = functions.localSetThreadDPIAwarenessContext)
    {
        previousContext = localSetThreadDPIAwarenessContext (DPI_AWARENESS_CONTEXT_UNAWARE);

       #if JUCE_DEBUG
        ++numActiveScopedDpiAwarenessDisablers;
       #endif
    }
}

ScopedDPIAwarenessDisabler::~ScopedDPIAwarenessDisabler()
{
    if (previousContext != nullptr)
    {
        if (auto* localSetThreadDPIAwarenessContext = getScopedDPIAwarenessDisablerFunctions().localSetThreadDPIAwarenessContext)
            localSetThreadDPIAwarenessContext ((DPI_AWARENESS_CONTEXT) previousContext);

       #if JUCE_DEBUG
        --numActiveScopedDpiAwarenessDisablers;
       #endif
    }
}

//==============================================================================
using SettingChangeCallbackFunc = void (*)(void);
extern SettingChangeCallbackFunc settingChangeCallback;

//==============================================================================
static Rectangle<int> rectangleFromRECT (RECT r) noexcept    { return { r.left, r.top, r.right - r.left, r.bottom - r.top }; }
static RECT RECTFromRectangle (Rectangle<int> r) noexcept    { return { r.getX(), r.getY(), r.getRight(), r.getBottom() }; }

static Point<int> pointFromPOINT (POINT p) noexcept          { return { p.x, p.y }; }
static POINT POINTFromPoint (Point<int> p) noexcept          { return { p.x, p.y }; }

//==============================================================================
static const Displays::Display* getCurrentDisplayFromScaleFactor (HWND hwnd);

template <typename ValueType>
static Rectangle<ValueType> convertPhysicalScreenRectangleToLogical (Rectangle<ValueType> r, HWND h) noexcept
{
    if (isPerMonitorDPIAwareWindow (h))
        return Desktop::getInstance().getDisplays().physicalToLogical (r, getCurrentDisplayFromScaleFactor (h));

    return r;
}

template <typename ValueType>
static Rectangle<ValueType> convertLogicalScreenRectangleToPhysical (Rectangle<ValueType> r, HWND h) noexcept
{
    if (isPerMonitorDPIAwareWindow (h))
        return Desktop::getInstance().getDisplays().logicalToPhysical (r, getCurrentDisplayFromScaleFactor (h));

    return r;
}

static Point<int> convertPhysicalScreenPointToLogical (Point<int> p, HWND h) noexcept
{
    if (isPerMonitorDPIAwareWindow (h))
        return Desktop::getInstance().getDisplays().physicalToLogical (p, getCurrentDisplayFromScaleFactor (h));

    return p;
}

static Point<int> convertLogicalScreenPointToPhysical (Point<int> p, HWND h) noexcept
{
    if (isPerMonitorDPIAwareWindow (h))
        return Desktop::getInstance().getDisplays().logicalToPhysical (p, getCurrentDisplayFromScaleFactor (h));

    return p;
}

JUCE_API double getScaleFactorForWindow (HWND h);
JUCE_API double getScaleFactorForWindow (HWND h)
{
    // NB. Using a local function here because we need to call this method from the plug-in wrappers
    // which don't load the DPI-awareness functions on startup
    static auto localGetDPIForWindow = (GetDPIForWindowFunc) getUser32Function ("GetDpiForWindow");

    if (localGetDPIForWindow != nullptr)
        return (double) localGetDPIForWindow (h) / USER_DEFAULT_SCREEN_DPI;

    return 1.0;
 }

//==============================================================================
static void setWindowPos (HWND hwnd, Rectangle<int> bounds, UINT flags, bool adjustTopLeft = false)
{
    ScopedThreadDPIAwarenessSetter setter { hwnd };

    if (isPerMonitorDPIAwareWindow (hwnd))
    {
        if (adjustTopLeft)
            bounds = convertLogicalScreenRectangleToPhysical (bounds, hwnd)
                      .withPosition (Desktop::getInstance().getDisplays().logicalToPhysical (bounds.getTopLeft()));
        else
            bounds = convertLogicalScreenRectangleToPhysical (bounds, hwnd);
    }

    SetWindowPos (hwnd, nullptr, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), flags);
}

static RECT getWindowScreenRect (HWND hwnd)
{
    ScopedThreadDPIAwarenessSetter setter { hwnd };

    RECT rect;
    GetWindowRect (hwnd, &rect);
    return rect;
}

static RECT getWindowClientRect (HWND hwnd)
{
    auto rect = getWindowScreenRect (hwnd);

    if (auto parentH = GetParent (hwnd))
    {
        ScopedThreadDPIAwarenessSetter setter { hwnd };
        MapWindowPoints (HWND_DESKTOP, parentH, (LPPOINT) &rect, 2);
    }

    return rect;
}

static void setWindowZOrder (HWND hwnd, HWND insertAfter)
{
    SetWindowPos (hwnd, insertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
}

//==============================================================================
#if ! JUCE_MINGW
extern RTL_OSVERSIONINFOW getWindowsVersionInfo();
#endif

double Desktop::getDefaultMasterScale()
{
    if (! JUCEApplicationBase::isStandaloneApp() || isPerMonitorDPIAwareProcess())
        return 1.0;

    return getGlobalDPI() / USER_DEFAULT_SCREEN_DPI;
}

bool Desktop::canUseSemiTransparentWindows() noexcept
{
    return true;
}

class Desktop::NativeDarkModeChangeDetectorImpl
{
public:
    NativeDarkModeChangeDetectorImpl()
    {
       #if ! JUCE_MINGW
        const auto winVer = getWindowsVersionInfo();

        if (winVer.dwMajorVersion >= 10 && winVer.dwBuildNumber >= 17763)
        {
            const auto uxtheme = "uxtheme.dll";
            LoadLibraryA (uxtheme);
            const auto uxthemeModule = GetModuleHandleA (uxtheme);

            if (uxthemeModule != nullptr)
            {
                shouldAppsUseDarkMode = (ShouldAppsUseDarkModeFunc) GetProcAddress (uxthemeModule, MAKEINTRESOURCEA (132));

                if (shouldAppsUseDarkMode != nullptr)
                    darkModeEnabled = shouldAppsUseDarkMode() && ! isHighContrast();
            }
        }
       #endif
    }

    ~NativeDarkModeChangeDetectorImpl()
    {
        UnhookWindowsHookEx (hook);
    }

    bool isDarkModeEnabled() const noexcept  { return darkModeEnabled; }

private:
    static bool isHighContrast()
    {
        HIGHCONTRASTW highContrast {};

        if (SystemParametersInfoW (SPI_GETHIGHCONTRAST, sizeof (highContrast), &highContrast, false))
            return highContrast.dwFlags & HCF_HIGHCONTRASTON;

        return false;
    }

    static LRESULT CALLBACK callWndProc (int nCode, WPARAM wParam, LPARAM lParam)
    {
        auto* params = reinterpret_cast<CWPSTRUCT*> (lParam);

        if (nCode >= 0
            && params != nullptr
            && params->message == WM_SETTINGCHANGE
            && params->lParam != 0
            && CompareStringOrdinal (reinterpret_cast<LPWCH> (params->lParam), -1, L"ImmersiveColorSet", -1, true) == CSTR_EQUAL)
        {
            Desktop::getInstance().nativeDarkModeChangeDetectorImpl->colourSetChanged();
        }

        return CallNextHookEx ({}, nCode, wParam, lParam);
    }

    void colourSetChanged()
    {
        if (shouldAppsUseDarkMode != nullptr)
        {
            const auto wasDarkModeEnabled = std::exchange (darkModeEnabled, shouldAppsUseDarkMode() && ! isHighContrast());

            if (darkModeEnabled != wasDarkModeEnabled)
                Desktop::getInstance().darkModeChanged();
        }
    }

    using ShouldAppsUseDarkModeFunc = bool (WINAPI*)();
    ShouldAppsUseDarkModeFunc shouldAppsUseDarkMode = nullptr;

    bool darkModeEnabled = false;
    HHOOK hook { SetWindowsHookEx (WH_CALLWNDPROC,
                                   callWndProc,
                                   (HINSTANCE) juce::Process::getCurrentModuleInstanceHandle(),
                                   GetCurrentThreadId()) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeDarkModeChangeDetectorImpl)
};

std::unique_ptr<Desktop::NativeDarkModeChangeDetectorImpl> Desktop::createNativeDarkModeChangeDetectorImpl()
{
    return std::make_unique<NativeDarkModeChangeDetectorImpl>();
}

bool Desktop::isDarkModeActive() const
{
    return nativeDarkModeChangeDetectorImpl->isDarkModeEnabled();
}

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const
{
    return upright;
}

int64 getMouseEventTime();
int64 getMouseEventTime()
{
    static int64 eventTimeOffset = 0;
    static LONG lastMessageTime = 0;
    const LONG thisMessageTime = GetMessageTime();

    if (thisMessageTime < lastMessageTime || lastMessageTime == 0)
    {
        lastMessageTime = thisMessageTime;
        eventTimeOffset = Time::currentTimeMillis() - thisMessageTime;
    }

    return eventTimeOffset + thisMessageTime;
}

//==============================================================================
const int extendedKeyModifier               = 0x10000;

const int KeyPress::spaceKey                = VK_SPACE;
const int KeyPress::returnKey               = VK_RETURN;
const int KeyPress::escapeKey               = VK_ESCAPE;
const int KeyPress::backspaceKey            = VK_BACK;
const int KeyPress::deleteKey               = VK_DELETE         | extendedKeyModifier;
const int KeyPress::insertKey               = VK_INSERT         | extendedKeyModifier;
const int KeyPress::tabKey                  = VK_TAB;
const int KeyPress::leftKey                 = VK_LEFT           | extendedKeyModifier;
const int KeyPress::rightKey                = VK_RIGHT          | extendedKeyModifier;
const int KeyPress::upKey                   = VK_UP             | extendedKeyModifier;
const int KeyPress::downKey                 = VK_DOWN           | extendedKeyModifier;
const int KeyPress::homeKey                 = VK_HOME           | extendedKeyModifier;
const int KeyPress::endKey                  = VK_END            | extendedKeyModifier;
const int KeyPress::pageUpKey               = VK_PRIOR          | extendedKeyModifier;
const int KeyPress::pageDownKey             = VK_NEXT           | extendedKeyModifier;
const int KeyPress::F1Key                   = VK_F1             | extendedKeyModifier;
const int KeyPress::F2Key                   = VK_F2             | extendedKeyModifier;
const int KeyPress::F3Key                   = VK_F3             | extendedKeyModifier;
const int KeyPress::F4Key                   = VK_F4             | extendedKeyModifier;
const int KeyPress::F5Key                   = VK_F5             | extendedKeyModifier;
const int KeyPress::F6Key                   = VK_F6             | extendedKeyModifier;
const int KeyPress::F7Key                   = VK_F7             | extendedKeyModifier;
const int KeyPress::F8Key                   = VK_F8             | extendedKeyModifier;
const int KeyPress::F9Key                   = VK_F9             | extendedKeyModifier;
const int KeyPress::F10Key                  = VK_F10            | extendedKeyModifier;
const int KeyPress::F11Key                  = VK_F11            | extendedKeyModifier;
const int KeyPress::F12Key                  = VK_F12            | extendedKeyModifier;
const int KeyPress::F13Key                  = VK_F13            | extendedKeyModifier;
const int KeyPress::F14Key                  = VK_F14            | extendedKeyModifier;
const int KeyPress::F15Key                  = VK_F15            | extendedKeyModifier;
const int KeyPress::F16Key                  = VK_F16            | extendedKeyModifier;
const int KeyPress::F17Key                  = VK_F17            | extendedKeyModifier;
const int KeyPress::F18Key                  = VK_F18            | extendedKeyModifier;
const int KeyPress::F19Key                  = VK_F19            | extendedKeyModifier;
const int KeyPress::F20Key                  = VK_F20            | extendedKeyModifier;
const int KeyPress::F21Key                  = VK_F21            | extendedKeyModifier;
const int KeyPress::F22Key                  = VK_F22            | extendedKeyModifier;
const int KeyPress::F23Key                  = VK_F23            | extendedKeyModifier;
const int KeyPress::F24Key                  = VK_F24            | extendedKeyModifier;
const int KeyPress::F25Key                  = 0x31000;          // Windows doesn't support F-keys 25 or higher
const int KeyPress::F26Key                  = 0x31001;
const int KeyPress::F27Key                  = 0x31002;
const int KeyPress::F28Key                  = 0x31003;
const int KeyPress::F29Key                  = 0x31004;
const int KeyPress::F30Key                  = 0x31005;
const int KeyPress::F31Key                  = 0x31006;
const int KeyPress::F32Key                  = 0x31007;
const int KeyPress::F33Key                  = 0x31008;
const int KeyPress::F34Key                  = 0x31009;
const int KeyPress::F35Key                  = 0x3100a;

const int KeyPress::numberPad0              = VK_NUMPAD0        | extendedKeyModifier;
const int KeyPress::numberPad1              = VK_NUMPAD1        | extendedKeyModifier;
const int KeyPress::numberPad2              = VK_NUMPAD2        | extendedKeyModifier;
const int KeyPress::numberPad3              = VK_NUMPAD3        | extendedKeyModifier;
const int KeyPress::numberPad4              = VK_NUMPAD4        | extendedKeyModifier;
const int KeyPress::numberPad5              = VK_NUMPAD5        | extendedKeyModifier;
const int KeyPress::numberPad6              = VK_NUMPAD6        | extendedKeyModifier;
const int KeyPress::numberPad7              = VK_NUMPAD7        | extendedKeyModifier;
const int KeyPress::numberPad8              = VK_NUMPAD8        | extendedKeyModifier;
const int KeyPress::numberPad9              = VK_NUMPAD9        | extendedKeyModifier;
const int KeyPress::numberPadAdd            = VK_ADD            | extendedKeyModifier;
const int KeyPress::numberPadSubtract       = VK_SUBTRACT       | extendedKeyModifier;
const int KeyPress::numberPadMultiply       = VK_MULTIPLY       | extendedKeyModifier;
const int KeyPress::numberPadDivide         = VK_DIVIDE         | extendedKeyModifier;
const int KeyPress::numberPadSeparator      = VK_SEPARATOR      | extendedKeyModifier;
const int KeyPress::numberPadDecimalPoint   = VK_DECIMAL        | extendedKeyModifier;
const int KeyPress::numberPadEquals         = 0x92 /*VK_OEM_NEC_EQUAL*/  | extendedKeyModifier;
const int KeyPress::numberPadDelete         = VK_DELETE         | extendedKeyModifier;
const int KeyPress::playKey                 = 0x30000;
const int KeyPress::stopKey                 = 0x30001;
const int KeyPress::fastForwardKey          = 0x30002;
const int KeyPress::rewindKey               = 0x30003;


//==============================================================================
class WindowsBitmapImage final : public ImagePixelData
{
public:
    WindowsBitmapImage (const Image::PixelFormat format,
                        const int w, const int h, const bool clearImage)
        : ImagePixelData (format, w, h)
    {
        jassert (format == Image::RGB || format == Image::ARGB);

        static bool alwaysUse32Bits = isGraphicsCard32Bit(); // NB: for 32-bit cards, it's faster to use a 32-bit image.

        pixelStride = (alwaysUse32Bits || format == Image::ARGB) ? 4 : 3;
        lineStride = -((w * pixelStride + 3) & ~3);

        zerostruct (bitmapInfo);
        bitmapInfo.bV4Size     = sizeof (BITMAPV4HEADER);
        bitmapInfo.bV4Width    = w;
        bitmapInfo.bV4Height   = h;
        bitmapInfo.bV4Planes   = 1;
        bitmapInfo.bV4CSType   = 1;
        bitmapInfo.bV4BitCount = (unsigned short) (pixelStride * 8);

        if (format == Image::ARGB)
        {
            bitmapInfo.bV4AlphaMask      = 0xff000000;
            bitmapInfo.bV4RedMask        = 0xff0000;
            bitmapInfo.bV4GreenMask      = 0xff00;
            bitmapInfo.bV4BlueMask       = 0xff;
            bitmapInfo.bV4V4Compression  = BI_BITFIELDS;
        }
        else
        {
            bitmapInfo.bV4V4Compression  = BI_RGB;
        }

        {
            ScopedDeviceContext deviceContext { nullptr };
            hdc = CreateCompatibleDC (deviceContext.dc);
        }

        SetMapMode (hdc, MM_TEXT);

        hBitmap = CreateDIBSection (hdc, (BITMAPINFO*) &(bitmapInfo), DIB_RGB_COLORS,
                                    (void**) &bitmapData, nullptr, 0);

        if (hBitmap != nullptr)
            previousBitmap = SelectObject (hdc, hBitmap);

        if (format == Image::ARGB && clearImage)
            zeromem (bitmapData, (size_t) std::abs (h * lineStride));

        imageData = bitmapData - (lineStride * (h - 1));
    }

    ~WindowsBitmapImage() override
    {
        SelectObject (hdc, previousBitmap); // Selecting the previous bitmap before deleting the DC avoids a warning in BoundsChecker
        DeleteDC (hdc);
        DeleteObject (hBitmap);
    }

    std::unique_ptr<ImageType> createType() const override    { return std::make_unique<NativeImageType>(); }

    std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override
    {
        sendDataChangeMessage();
        return std::make_unique<LowLevelGraphicsSoftwareRenderer> (Image (this));
    }

    void initialiseBitmapData (Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode mode) override
    {
        const auto offset = (size_t) (x * pixelStride + y * lineStride);
        bitmap.data = imageData + offset;
        bitmap.size = (size_t) (lineStride * height) - offset;
        bitmap.pixelFormat = pixelFormat;
        bitmap.lineStride = lineStride;
        bitmap.pixelStride = pixelStride;

        if (mode != Image::BitmapData::readOnly)
            sendDataChangeMessage();
    }

    ImagePixelData::Ptr clone() override
    {
        auto im = new WindowsBitmapImage (pixelFormat, width, height, false);

        for (int i = 0; i < height; ++i)
            memcpy (im->imageData + i * lineStride, imageData + i * lineStride, (size_t) lineStride);

        return im;
    }

    void blitToWindow (HWND hwnd, HDC dc, bool transparent, int x, int y, uint8 updateLayeredWindowAlpha) noexcept
    {
        SetMapMode (dc, MM_TEXT);

        if (transparent)
        {
            auto windowBounds = getWindowScreenRect (hwnd);

            POINT p = { -x, -y };
            POINT pos = { windowBounds.left, windowBounds.top };
            SIZE size = { windowBounds.right - windowBounds.left,
                          windowBounds.bottom - windowBounds.top };

            BLENDFUNCTION bf;
            bf.AlphaFormat = 1 /*AC_SRC_ALPHA*/;
            bf.BlendFlags = 0;
            bf.BlendOp = AC_SRC_OVER;
            bf.SourceConstantAlpha = updateLayeredWindowAlpha;

            UpdateLayeredWindow (hwnd, nullptr, &pos, &size, hdc, &p, 0, &bf, 2 /*ULW_ALPHA*/);
        }
        else
        {
            StretchDIBits (dc,
                           x, y, width, height,
                           0, 0, width, height,
                           bitmapData, (const BITMAPINFO*) &bitmapInfo,
                           DIB_RGB_COLORS, SRCCOPY);
        }
    }

    HBITMAP hBitmap;
    HGDIOBJ previousBitmap;
    BITMAPV4HEADER bitmapInfo;
    HDC hdc;
    uint8* bitmapData;
    int pixelStride, lineStride;
    uint8* imageData;

private:
    static bool isGraphicsCard32Bit()
    {
        ScopedDeviceContext deviceContext { nullptr };
        return GetDeviceCaps (deviceContext.dc, BITSPIXEL) > 24;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsBitmapImage)
};

//==============================================================================
Image createSnapshotOfNativeWindow (void* nativeWindowHandle)
{
    auto hwnd = (HWND) nativeWindowHandle;

    auto r = convertPhysicalScreenRectangleToLogical (rectangleFromRECT (getWindowScreenRect (hwnd)), hwnd);
    const auto w = r.getWidth();
    const auto h = r.getHeight();

    auto nativeBitmap = new WindowsBitmapImage (Image::RGB, w, h, true);
    Image bitmap (nativeBitmap);

    ScopedDeviceContext deviceContext { hwnd };

    if (isPerMonitorDPIAwareProcess())
    {
        auto scale = getScaleFactorForWindow (hwnd);
        auto prevStretchMode = SetStretchBltMode (nativeBitmap->hdc, HALFTONE);
        SetBrushOrgEx (nativeBitmap->hdc, 0, 0, nullptr);

        StretchBlt (nativeBitmap->hdc, 0, 0, w, h,
                    deviceContext.dc, 0, 0, roundToInt (w * scale), roundToInt (h * scale),
                    SRCCOPY);

        SetStretchBltMode (nativeBitmap->hdc, prevStretchMode);
    }
    else
    {
        BitBlt (nativeBitmap->hdc, 0, 0, w, h, deviceContext.dc, 0, 0, SRCCOPY);
    }

    return SoftwareImageType().convert (bitmap);
}

//==============================================================================
namespace IconConverters
{
    struct IconDestructor
    {
        void operator() (HICON ptr) const { if (ptr != nullptr) DestroyIcon (ptr); }
    };

    using IconPtr = std::unique_ptr<std::remove_pointer_t<HICON>, IconDestructor>;

    static Image createImageFromHICON (HICON icon)
    {
        if (icon == nullptr)
            return {};

        struct ScopedICONINFO final : public ICONINFO
        {
            ScopedICONINFO()
            {
                hbmColor = nullptr;
                hbmMask = nullptr;
            }

            ~ScopedICONINFO()
            {
                if (hbmColor != nullptr)
                    ::DeleteObject (hbmColor);

                if (hbmMask != nullptr)
                    ::DeleteObject (hbmMask);
            }
        };

        ScopedICONINFO info;

        if (! ::GetIconInfo (icon, &info))
            return {};

        BITMAP bm;

        if (! (::GetObject (info.hbmColor, sizeof (BITMAP), &bm)
               && bm.bmWidth > 0 && bm.bmHeight > 0))
            return {};

        ScopedDeviceContext deviceContext { nullptr };

        if (auto* dc = ::CreateCompatibleDC (deviceContext.dc))
        {
            BITMAPV5HEADER header = {};
            header.bV5Size = sizeof (BITMAPV5HEADER);
            header.bV5Width = bm.bmWidth;
            header.bV5Height = -bm.bmHeight;
            header.bV5Planes = 1;
            header.bV5Compression = BI_RGB;
            header.bV5BitCount = 32;
            header.bV5RedMask = 0x00FF0000;
            header.bV5GreenMask = 0x0000FF00;
            header.bV5BlueMask = 0x000000FF;
            header.bV5AlphaMask = 0xFF000000;
            header.bV5CSType = 0x57696E20; // 'Win '
            header.bV5Intent = LCS_GM_IMAGES;

            uint32* bitmapImageData = nullptr;

            if (auto* dib = ::CreateDIBSection (deviceContext.dc, (BITMAPINFO*) &header, DIB_RGB_COLORS,
                                                (void**) &bitmapImageData, nullptr, 0))
            {
                auto oldObject = ::SelectObject (dc, dib);

                auto numPixels = bm.bmWidth * bm.bmHeight;
                auto numColourComponents = (size_t) numPixels * 4;

                // Windows icon data comes as two layers, an XOR mask which contains the bulk
                // of the image data and an AND mask which provides the transparency. Annoyingly
                // the XOR mask can also contain an alpha channel, in which case the transparency
                // mask should not be applied, but there's no way to find out a priori if the XOR
                // mask contains an alpha channel.

                HeapBlock<bool> opacityMask (numPixels);
                memset (bitmapImageData, 0, numColourComponents);
                ::DrawIconEx (dc, 0, 0, icon, bm.bmWidth, bm.bmHeight, 0, nullptr, DI_MASK);

                for (int i = 0; i < numPixels; ++i)
                    opacityMask[i] = (bitmapImageData[i] == 0);

                Image result = Image (Image::ARGB, bm.bmWidth, bm.bmHeight, true);
                Image::BitmapData imageData (result, Image::BitmapData::readWrite);

                memset (bitmapImageData, 0, numColourComponents);
                ::DrawIconEx (dc, 0, 0, icon, bm.bmWidth, bm.bmHeight, 0, nullptr, DI_NORMAL);
                memcpy (imageData.data, bitmapImageData, numColourComponents);

                auto imageHasAlphaChannel = [&imageData, numPixels]()
                {
                    for (int i = 0; i < numPixels; ++i)
                        if (imageData.data[i * 4] != 0)
                            return true;

                    return false;
                };

                if (! imageHasAlphaChannel())
                    for (int i = 0; i < numPixels; ++i)
                        imageData.data[i * 4] = opacityMask[i] ? 0xff : 0x00;

                ::SelectObject (dc, oldObject);
                ::DeleteObject (dib);
                ::DeleteDC (dc);

                return result;
            }

            ::DeleteDC (dc);
        }

        return {};
    }

    HICON createHICONFromImage (const Image& image, const BOOL isIcon, int hotspotX, int hotspotY);
    HICON createHICONFromImage (const Image& image, const BOOL isIcon, int hotspotX, int hotspotY)
    {
        auto nativeBitmap = new WindowsBitmapImage (Image::ARGB, image.getWidth(), image.getHeight(), true);
        Image bitmap (nativeBitmap);

        {
            Graphics g (bitmap);
            g.drawImageAt (image, 0, 0);
        }

        auto mask = CreateBitmap (image.getWidth(), image.getHeight(), 1, 1, nullptr);

        ICONINFO info;
        info.fIcon = isIcon;
        info.xHotspot = (DWORD) hotspotX;
        info.yHotspot = (DWORD) hotspotY;
        info.hbmMask = mask;
        info.hbmColor = nativeBitmap->hBitmap;

        auto hi = CreateIconIndirect (&info);
        DeleteObject (mask);
        return hi;
    }
} // namespace IconConverters

//==============================================================================
JUCE_IUNKNOWNCLASS (ITipInvocation, "37c994e7-432b-4834-a2f7-dce1f13b834b")
{
    static CLSID getCLSID() noexcept   { return { 0x4ce576fa, 0x83dc, 0x4f88, { 0x95, 0x1c, 0x9d, 0x07, 0x82, 0xb4, 0xe3, 0x76 } }; }

    JUCE_COMCALL Toggle (HWND) = 0;
};

} // namespace juce

#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL (juce::ITipInvocation, 0x37c994e7, 0x432b, 0x4834, 0xa2, 0xf7, 0xdc, 0xe1, 0xf1, 0x3b, 0x83, 0x4b)
#endif

namespace juce
{

//==============================================================================
struct HSTRING_PRIVATE;
typedef HSTRING_PRIVATE* HSTRING;

struct IInspectable : public IUnknown
{
    JUCE_COMCALL GetIids (ULONG* ,IID**) = 0;
    JUCE_COMCALL GetRuntimeClassName (HSTRING*) = 0;
    JUCE_COMCALL GetTrustLevel (void*) = 0;
};

JUCE_COMCLASS (IUIViewSettingsInterop, "3694dbf9-8f68-44be-8ff5-195c98ede8a6")  : public IInspectable
{
    JUCE_COMCALL GetForWindow (HWND, REFIID, void**) = 0;
};

JUCE_COMCLASS (IUIViewSettings, "c63657f6-8850-470d-88f8-455e16ea2c26")  : public IInspectable
{
    enum UserInteractionMode
    {
        Mouse = 0,
        Touch = 1
    };

    JUCE_COMCALL GetUserInteractionMode (UserInteractionMode*) = 0;
};

} // namespace juce

#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL (juce::IUIViewSettingsInterop, 0x3694dbf9, 0x8f68, 0x44be, 0x8f, 0xf5, 0x19, 0x5c, 0x98, 0xed, 0xe8, 0xa6)
__CRT_UUID_DECL (juce::IUIViewSettings,        0xc63657f6, 0x8850, 0x470d, 0x88, 0xf8, 0x45, 0x5e, 0x16, 0xea, 0x2c, 0x26)
#endif

namespace juce
{
struct UWPUIViewSettings
{
    UWPUIViewSettings()
    {
        ComBaseModule dll (L"api-ms-win-core-winrt-l1-1-0");

        if (dll.h != nullptr)
        {
            roInitialize           = (RoInitializeFuncPtr)           ::GetProcAddress (dll.h, "RoInitialize");
            roGetActivationFactory = (RoGetActivationFactoryFuncPtr) ::GetProcAddress (dll.h, "RoGetActivationFactory");
            createHString          = (WindowsCreateStringFuncPtr)    ::GetProcAddress (dll.h, "WindowsCreateString");
            deleteHString          = (WindowsDeleteStringFuncPtr)    ::GetProcAddress (dll.h, "WindowsDeleteString");

            if (roInitialize == nullptr || roGetActivationFactory == nullptr
                 || createHString == nullptr || deleteHString == nullptr)
                return;

            auto status = roInitialize (1);

            if (status != S_OK && status != S_FALSE && (unsigned) status != 0x80010106L)
                return;

            LPCWSTR uwpClassName = L"Windows.UI.ViewManagement.UIViewSettings";
            HSTRING uwpClassId = nullptr;

            if (createHString (uwpClassName, (::UINT32) wcslen (uwpClassName), &uwpClassId) != S_OK
                 || uwpClassId == nullptr)
                return;

            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
            status = roGetActivationFactory (uwpClassId, __uuidof (IUIViewSettingsInterop),
                                             (void**) viewSettingsInterop.resetAndGetPointerAddress());
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
            deleteHString (uwpClassId);

            if (status != S_OK || viewSettingsInterop == nullptr)
                return;

            // move dll into member var
            comBaseDLL = std::move (dll);
        }
    }

private:
    //==============================================================================
    struct ComBaseModule
    {
        ComBaseModule() = default;
        ComBaseModule (LPCWSTR libraryName) : h (::LoadLibrary (libraryName)) {}
        ComBaseModule (ComBaseModule&& o) : h (o.h) { o.h = nullptr; }
        ~ComBaseModule() { release(); }

        void release() { if (h != nullptr) ::FreeLibrary (h); h = nullptr; }
        ComBaseModule& operator= (ComBaseModule&& o) { release(); h = o.h; o.h = nullptr; return *this; }

        HMODULE h = {};
    };

    using RoInitializeFuncPtr           = HRESULT (WINAPI*) (int);
    using RoGetActivationFactoryFuncPtr = HRESULT (WINAPI*) (HSTRING, REFIID, void**);
    using WindowsCreateStringFuncPtr    = HRESULT (WINAPI*) (LPCWSTR,UINT32, HSTRING*);
    using WindowsDeleteStringFuncPtr    = HRESULT (WINAPI*) (HSTRING);

    ComBaseModule comBaseDLL;
    ComSmartPtr<IUIViewSettingsInterop> viewSettingsInterop;

    RoInitializeFuncPtr roInitialize;
    RoGetActivationFactoryFuncPtr roGetActivationFactory;
    WindowsCreateStringFuncPtr createHString;
    WindowsDeleteStringFuncPtr deleteHString;
};

//==============================================================================
static HMONITOR getMonitorFromOutput (ComSmartPtr<IDXGIOutput> output)
{
    DXGI_OUTPUT_DESC desc = {};
    return (FAILED (output->GetDesc (&desc)) || ! desc.AttachedToDesktop)
        ? nullptr
        : desc.Monitor;
}

using VBlankListener = ComponentPeer::VBlankListener;

//==============================================================================
class VSyncThread final : private Thread,
                          private AsyncUpdater
{
public:
    VSyncThread (ComSmartPtr<IDXGIOutput> out,
                 HMONITOR mon,
                 VBlankListener& listener)
        : Thread ("VSyncThread"),
          output (out),
          monitor (mon)
    {
        listeners.push_back (listener);
        startThread (Priority::highest);
    }

    ~VSyncThread() override
    {
        cancelPendingUpdate();

        {
            const std::scoped_lock lock { mutex };
            threadState = ThreadState::exit;
        }

        condvar.notify_one();

        stopThread (-1);
    }

    void updateMonitor()
    {
        monitor = getMonitorFromOutput (output);
    }

    HMONITOR getMonitor() const noexcept { return monitor; }

    void addListener (VBlankListener& listener)
    {
        listeners.push_back (listener);
    }

    bool removeListener (const VBlankListener& listener)
    {
        auto it = std::find_if (listeners.cbegin(),
                                listeners.cend(),
                                [&listener] (const auto& l) { return &(l.get()) == &listener; });

        if (it != listeners.cend())
        {
            listeners.erase (it);
            return true;
        }

        return false;
    }

    bool hasNoListeners() const noexcept
    {
        return listeners.empty();
    }

    bool hasListener (const VBlankListener& listener) const noexcept
    {
        return std::any_of (listeners.cbegin(),
                            listeners.cend(),
                            [&listener] (const auto& l) { return &(l.get()) == &listener; });
    }

private:
    //==============================================================================
    void run() override
    {
        for (;;)
        {
            if (output->WaitForVBlank() == S_OK)
            {
                std::unique_lock lock { mutex };
                condvar.wait (lock, [this] { return threadState != ThreadState::sleep; });

                if (threadState == ThreadState::exit)
                    return;

                threadState = ThreadState::sleep;
                triggerAsyncUpdate();
            }
            else
            {
                Thread::sleep (1);
            }
        }
    }

    void handleAsyncUpdate() override
    {
        for (auto& listener : listeners)
            listener.get().onVBlank();

        {
            const std::scoped_lock lock { mutex };

            if (threadState == ThreadState::sleep)
                threadState = ThreadState::paint;
        }

        condvar.notify_one();
    }

    //==============================================================================
    ComSmartPtr<IDXGIOutput> output;
    HMONITOR monitor = nullptr;
    std::vector<std::reference_wrapper<VBlankListener>> listeners;

    enum class ThreadState
    {
        sleep,
        paint,
        exit,
    };

    ThreadState threadState = ThreadState::paint;
    std::condition_variable condvar;
    std::mutex mutex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VSyncThread)
    JUCE_DECLARE_NON_MOVEABLE (VSyncThread)
};

//==============================================================================
class VBlankDispatcher final : public DeletedAtShutdown
{
public:
    void updateDisplay (VBlankListener& listener, HMONITOR monitor)
    {
        if (monitor == nullptr)
        {
            removeListener (listener);
            return;
        }

        auto threadWithListener = threads.end();
        auto threadWithMonitor  = threads.end();

        for (auto it = threads.begin(); it != threads.end(); ++it)
        {
            if ((*it)->hasListener (listener))
                threadWithListener = it;

            if ((*it)->getMonitor() == monitor)
                threadWithMonitor = it;

            if (threadWithListener != threads.end()
                && threadWithMonitor != threads.end())
            {
                if (threadWithListener == threadWithMonitor)
                    return;

                (*threadWithMonitor)->addListener (listener);

                // This may invalidate iterators, so be careful!
                removeListener (threadWithListener, listener);
                return;
            }
        }

        if (threadWithMonitor != threads.end())
        {
            (*threadWithMonitor)->addListener (listener);
            return;
        }

        if (threadWithListener != threads.end())
            removeListener (threadWithListener, listener);

        for (auto adapter : adapters)
        {
            UINT i = 0;
            ComSmartPtr<IDXGIOutput> output;

            while (adapter->EnumOutputs (i, output.resetAndGetPointerAddress()) != DXGI_ERROR_NOT_FOUND)
            {
                if (getMonitorFromOutput (output) == monitor)
                {
                    threads.emplace_back (std::make_unique<VSyncThread> (output, monitor, listener));
                    return;
                }

                ++i;
            }
        }
    }

    void removeListener (const VBlankListener& listener)
    {
        for (auto it = threads.begin(); it != threads.end(); ++it)
            if (removeListener (it, listener))
                return;
    }

    void reconfigureDisplays()
    {
        adapters.clear();

        ComSmartPtr<IDXGIFactory> factory;
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
        CreateDXGIFactory (__uuidof (IDXGIFactory), (void**) factory.resetAndGetPointerAddress());
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        if (factory == nullptr)
        {
            jassertfalse;
            return;
        }

        UINT i = 0;
        ComSmartPtr<IDXGIAdapter> adapter;

        while (factory->EnumAdapters (i, adapter.resetAndGetPointerAddress()) != DXGI_ERROR_NOT_FOUND)
        {
            adapters.push_back (adapter);
            ++i;
        }

        for (auto& thread : threads)
            thread->updateMonitor();

        threads.erase (std::remove_if (threads.begin(),
                                       threads.end(),
                                       [] (const auto& thread) { return thread->getMonitor() == nullptr; }),
                       threads.end());
    }

    JUCE_DECLARE_SINGLETON_SINGLETHREADED (VBlankDispatcher, false)

private:
    //==============================================================================
    using Threads = std::vector<std::unique_ptr<VSyncThread>>;

    VBlankDispatcher()
    {
        reconfigureDisplays();
    }

    ~VBlankDispatcher() override
    {
        threads.clear();
        clearSingletonInstance();
    }

    // This may delete the corresponding thread and invalidate iterators,
    // so be careful!
    bool removeListener (Threads::iterator it, const VBlankListener& listener)
    {
        if ((*it)->removeListener (listener))
        {
            if ((*it)->hasNoListeners())
                threads.erase (it);

            return true;
        }

        return false;
    }

    //==============================================================================
    std::vector<ComSmartPtr<IDXGIAdapter>> adapters;
    Threads threads;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VBlankDispatcher)
    JUCE_DECLARE_NON_MOVEABLE (VBlankDispatcher)
};

JUCE_IMPLEMENT_SINGLETON (VBlankDispatcher)

//==============================================================================
class HWNDComponentPeer final : public ComponentPeer,
                                private VBlankListener,
                                private Timer
                               #if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
                                , public ModifierKeyReceiver
                               #endif
{
public:
    enum RenderingEngineType
    {
        softwareRenderingEngine = 0,
        direct2DRenderingEngine
    };

    //==============================================================================
    HWNDComponentPeer (Component& comp, int windowStyleFlags, HWND parent, bool nonRepainting)
        : ComponentPeer (comp, windowStyleFlags),
          dontRepaint (nonRepainting),
          parentToAddTo (parent),
          currentRenderingEngine (softwareRenderingEngine)
    {
        callFunctionIfNotLocked (&createWindowCallback, this);

        setTitle (component.getName());
        updateShadower();

        getNativeRealtimeModifiers = []
        {
            HWNDComponentPeer::updateKeyModifiers();

            int mouseMods = 0;
            if (HWNDComponentPeer::isKeyDown (VK_LBUTTON))  mouseMods |= ModifierKeys::leftButtonModifier;
            if (HWNDComponentPeer::isKeyDown (VK_RBUTTON))  mouseMods |= ModifierKeys::rightButtonModifier;
            if (HWNDComponentPeer::isKeyDown (VK_MBUTTON))  mouseMods |= ModifierKeys::middleButtonModifier;

            ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (mouseMods);

            return ModifierKeys::currentModifiers;
        };

        updateCurrentMonitorAndRefreshVBlankDispatcher();

        if (parentToAddTo != nullptr)
        {
            monitorUpdateTimer.emplace ([this] { updateCurrentMonitorAndRefreshVBlankDispatcher(); });
            monitorUpdateTimer->startTimer (1000);
        }

        suspendResumeRegistration = ScopedSuspendResumeNotificationRegistration { hwnd };
    }

    ~HWNDComponentPeer() override
    {
        suspendResumeRegistration = {};

        VBlankDispatcher::getInstance()->removeListener (*this);

        // do this first to avoid messages arriving for this window before it's destroyed
        JuceWindowIdentifier::setAsJUCEWindow (hwnd, false);

        if (isAccessibilityActive)
            WindowsAccessibility::revokeUIAMapEntriesForWindow (hwnd);

        shadower = nullptr;
        currentTouches.deleteAllTouchesForPeer (this);

        callFunctionIfNotLocked (&destroyWindowCallback, (void*) hwnd);

        if (dropTarget != nullptr)
        {
            dropTarget->peerIsDeleted = true;
            dropTarget->Release();
            dropTarget = nullptr;
        }

       #if JUCE_DIRECT2D
        direct2DContext = nullptr;
       #endif
    }

    //==============================================================================
    void* getNativeHandle() const override    { return hwnd; }

    void setVisible (bool shouldBeVisible) override
    {
        const ScopedValueSetter<bool> scope (shouldIgnoreModalDismiss, true);

        ShowWindow (hwnd, shouldBeVisible ? SW_SHOWNA : SW_HIDE);

        if (shouldBeVisible)
            InvalidateRect (hwnd, nullptr, 0);
        else
            lastPaintTime = 0;
    }

    void setTitle (const String& title) override
    {
        // Unfortunately some ancient bits of win32 mean you can only perform this operation from the message thread.
        JUCE_ASSERT_MESSAGE_THREAD

        SetWindowText (hwnd, title.toWideCharPointer());
    }

    void repaintNowIfTransparent()
    {
        if (isUsingUpdateLayeredWindow() && lastPaintTime > 0 && Time::getMillisecondCounter() > lastPaintTime + 30)
            handlePaintMessage();
    }

    void updateBorderSize()
    {
        WINDOWINFO info;
        info.cbSize = sizeof (info);

        if (GetWindowInfo (hwnd, &info))
            windowBorder = BorderSize<int> (roundToInt ((info.rcClient.top    - info.rcWindow.top)    / scaleFactor),
                                            roundToInt ((info.rcClient.left   - info.rcWindow.left)   / scaleFactor),
                                            roundToInt ((info.rcWindow.bottom - info.rcClient.bottom) / scaleFactor),
                                            roundToInt ((info.rcWindow.right  - info.rcClient.right)  / scaleFactor));

       #if JUCE_DIRECT2D
        if (direct2DContext != nullptr)
            direct2DContext->resized();
       #endif
    }

    void setBounds (const Rectangle<int>& bounds, bool isNowFullScreen) override
    {
        // If we try to set new bounds while handling an existing position change,
        // Windows may get confused about our current scale and size.
        // This can happen when moving a window between displays, because the mouse-move
        // generator in handlePositionChanged can cause the window to move again.
        if (inHandlePositionChanged)
            return;

        const ScopedValueSetter<bool> scope (shouldIgnoreModalDismiss, true);

        fullScreen = isNowFullScreen;

        auto newBounds = windowBorder.addedTo (bounds);

        if (isUsingUpdateLayeredWindow())
        {
            if (auto parentHwnd = GetParent (hwnd))
            {
                auto parentRect = convertPhysicalScreenRectangleToLogical (rectangleFromRECT (getWindowScreenRect (parentHwnd)), hwnd);
                newBounds.translate (parentRect.getX(), parentRect.getY());
            }
        }

        auto oldBounds = getBounds();

        const bool hasMoved = (oldBounds.getPosition() != bounds.getPosition());
        const bool hasResized = (oldBounds.getWidth() != bounds.getWidth()
                                  || oldBounds.getHeight() != bounds.getHeight());

        DWORD flags = SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER;
        if (! hasMoved)    flags |= SWP_NOMOVE;
        if (! hasResized)  flags |= SWP_NOSIZE;

        setWindowPos (hwnd, newBounds, flags, ! inDpiChange);

        if (hasResized && isValidPeer (this))
        {
            updateBorderSize();
            repaintNowIfTransparent();
        }
    }

    Rectangle<int> getBounds() const override
    {
        auto bounds = [this]
        {
            if (parentToAddTo == nullptr)
                return convertPhysicalScreenRectangleToLogical (rectangleFromRECT (getWindowScreenRect (hwnd)), hwnd);

            auto localBounds = rectangleFromRECT (getWindowClientRect (hwnd));

            if (isPerMonitorDPIAwareWindow (hwnd))
                return (localBounds.toDouble() / getPlatformScaleFactor()).toNearestInt();

            return localBounds;
        }();

        return windowBorder.subtractedFrom (bounds);
    }

    Point<int> getScreenPosition() const
    {
        auto r = convertPhysicalScreenRectangleToLogical (rectangleFromRECT (getWindowScreenRect (hwnd)), hwnd);

        return { r.getX() + windowBorder.getLeft(),
                 r.getY() + windowBorder.getTop() };
    }

    Point<float> localToGlobal (Point<float> relativePosition) override  { return relativePosition + getScreenPosition().toFloat(); }
    Point<float> globalToLocal (Point<float> screenPosition) override    { return screenPosition   - getScreenPosition().toFloat(); }

    using ComponentPeer::localToGlobal;
    using ComponentPeer::globalToLocal;

    void setAlpha (float newAlpha) override
    {
        const ScopedValueSetter<bool> scope (shouldIgnoreModalDismiss, true);

        auto intAlpha = (uint8) jlimit (0, 255, (int) (newAlpha * 255.0f));

        if (component.isOpaque())
        {
            if (newAlpha < 1.0f)
            {
                SetWindowLong (hwnd, GWL_EXSTYLE, GetWindowLong (hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
                SetLayeredWindowAttributes (hwnd, RGB (0, 0, 0), intAlpha, LWA_ALPHA);
            }
            else
            {
                SetWindowLong (hwnd, GWL_EXSTYLE, GetWindowLong (hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
                RedrawWindow (hwnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
            }
        }
        else
        {
            updateLayeredWindowAlpha = intAlpha;
            component.repaint();
        }
    }

    void setMinimised (bool shouldBeMinimised) override
    {
        const ScopedValueSetter<bool> scope (shouldIgnoreModalDismiss, true);

        if (shouldBeMinimised != isMinimised())
            ShowWindow (hwnd, shouldBeMinimised ? SW_MINIMIZE : SW_RESTORE);
    }

    bool isMinimised() const override
    {
        WINDOWPLACEMENT wp;
        wp.length = sizeof (WINDOWPLACEMENT);
        GetWindowPlacement (hwnd, &wp);

        return wp.showCmd == SW_SHOWMINIMIZED;
    }

    void setFullScreen (bool shouldBeFullScreen) override
    {
        const ScopedValueSetter<bool> scope (shouldIgnoreModalDismiss, true);

        setMinimised (false);

        if (isFullScreen() != shouldBeFullScreen)
        {
            if (constrainer != nullptr)
                constrainer->resizeStart();

            fullScreen = shouldBeFullScreen;
            const WeakReference<Component> deletionChecker (&component);

            if (! fullScreen)
            {
                auto boundsCopy = lastNonFullscreenBounds;

                if (hasTitleBar())
                    ShowWindow (hwnd, SW_SHOWNORMAL);

                if (! boundsCopy.isEmpty())
                    setBounds (detail::ScalingHelpers::scaledScreenPosToUnscaled (component, boundsCopy), false);
            }
            else
            {
                if (hasTitleBar())
                    ShowWindow (hwnd, SW_SHOWMAXIMIZED);
                else
                    SendMessageW (hwnd, WM_SETTINGCHANGE, 0, 0);
            }

            if (deletionChecker != nullptr)
                handleMovedOrResized();

            if (constrainer != nullptr)
                constrainer->resizeEnd();
        }
    }

    bool isFullScreen() const override
    {
        if (! hasTitleBar())
            return fullScreen;

        WINDOWPLACEMENT wp;
        wp.length = sizeof (wp);
        GetWindowPlacement (hwnd, &wp);

        return wp.showCmd == SW_SHOWMAXIMIZED;
    }

    bool contains (Point<int> localPos, bool trueIfInAChildWindow) const override
    {
        auto r = convertPhysicalScreenRectangleToLogical (rectangleFromRECT (getWindowScreenRect (hwnd)), hwnd);

        if (! r.withZeroOrigin().contains (localPos))
            return false;

        auto w = WindowFromPoint (POINTFromPoint (convertLogicalScreenPointToPhysical (localPos + getScreenPosition(),
                                                                                       hwnd)));

        return w == hwnd || (trueIfInAChildWindow && (IsChild (hwnd, w) != 0));
    }

    OptionalBorderSize getFrameSizeIfPresent() const override
    {
        return ComponentPeer::OptionalBorderSize { windowBorder };
    }

    BorderSize<int> getFrameSize() const override
    {
        return windowBorder;
    }

    bool setAlwaysOnTop (bool alwaysOnTop) override
    {
        const bool oldDeactivate = shouldDeactivateTitleBar;
        shouldDeactivateTitleBar = ((styleFlags & windowIsTemporary) == 0);

        setWindowZOrder (hwnd, alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST);

        shouldDeactivateTitleBar = oldDeactivate;

        if (shadower != nullptr)
            handleBroughtToFront();

        return true;
    }

    void toFront (bool makeActive) override
    {
        const ScopedValueSetter<bool> scope (shouldIgnoreModalDismiss, true);

        setMinimised (false);

        const bool oldDeactivate = shouldDeactivateTitleBar;
        shouldDeactivateTitleBar = ((styleFlags & windowIsTemporary) == 0);

        callFunctionIfNotLocked (makeActive ? &toFrontCallback1 : &toFrontCallback2, hwnd);

        shouldDeactivateTitleBar = oldDeactivate;

        if (! makeActive)
        {
            // in this case a broughttofront call won't have occurred, so do it now..
            handleBroughtToFront();
        }
    }

    void toBehind (ComponentPeer* other) override
    {
        const ScopedValueSetter<bool> scope (shouldIgnoreModalDismiss, true);

        if (auto* otherPeer = dynamic_cast<HWNDComponentPeer*> (other))
        {
            setMinimised (false);

            // Must be careful not to try to put a topmost window behind a normal one, or Windows
            // promotes the normal one to be topmost!
            if (component.isAlwaysOnTop() == otherPeer->getComponent().isAlwaysOnTop())
                setWindowZOrder (hwnd, otherPeer->hwnd);
            else if (otherPeer->getComponent().isAlwaysOnTop())
                setWindowZOrder (hwnd, HWND_TOP);
        }
        else
        {
            jassertfalse; // wrong type of window?
        }
    }

    bool isFocused() const override
    {
        return callFunctionIfNotLocked (&getFocusCallback, nullptr) == (void*) hwnd;
    }

    void grabFocus() override
    {
        const ScopedValueSetter<bool> scope (shouldIgnoreModalDismiss, true);

        const bool oldDeactivate = shouldDeactivateTitleBar;
        shouldDeactivateTitleBar = ((styleFlags & windowIsTemporary) == 0);

        callFunctionIfNotLocked (&setFocusCallback, hwnd);

        shouldDeactivateTitleBar = oldDeactivate;
    }

    void textInputRequired (Point<int>, TextInputTarget&) override
    {
        if (! hasCreatedCaret)
            hasCreatedCaret = CreateCaret (hwnd, (HBITMAP) 1, 0, 0);

        if (hasCreatedCaret)
        {
            SetCaretPos (0, 0);
            ShowCaret (hwnd);
        }

        ImmAssociateContext (hwnd, nullptr);

        // MSVC complains about the nullptr argument, but the docs for this
        // function say that the second argument is ignored when the third
        // argument is IACE_DEFAULT.
        JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6387)
        ImmAssociateContextEx (hwnd, nullptr, IACE_DEFAULT);
        JUCE_END_IGNORE_WARNINGS_MSVC
    }

    void closeInputMethodContext() override
    {
        imeHandler.handleSetContext (hwnd, false);
    }

    void dismissPendingTextInput() override
    {
        closeInputMethodContext();

        ImmAssociateContext (hwnd, nullptr);

        if (std::exchange (hasCreatedCaret, false))
            DestroyCaret();
    }

    void repaint (const Rectangle<int>& area) override
    {
        deferredRepaints.add ((area.toDouble() * getPlatformScaleFactor()).getSmallestIntegerContainer());
    }

    void dispatchDeferredRepaints()
    {
        for (auto deferredRect : deferredRepaints)
        {
            auto r = RECTFromRectangle (deferredRect);
            InvalidateRect (hwnd, &r, FALSE);
        }

        deferredRepaints.clear();
    }

    void performAnyPendingRepaintsNow() override
    {
        if (component.isVisible())
        {
            dispatchDeferredRepaints();

            WeakReference<Component> localRef (&component);
            MSG m;

            if (isUsingUpdateLayeredWindow() || PeekMessage (&m, hwnd, WM_PAINT, WM_PAINT, PM_REMOVE))
                if (localRef != nullptr) // (the PeekMessage call can dispatch messages, which may delete this comp)
                    handlePaintMessage();
        }
    }

    //==============================================================================
    void onVBlank() override
    {
        vBlankListeners.call ([] (auto& l) { l.onVBlank(); });
        dispatchDeferredRepaints();
    }

    //==============================================================================
    static HWNDComponentPeer* getOwnerOfWindow (HWND h) noexcept
    {
        if (h != nullptr && JuceWindowIdentifier::isJUCEWindow (h))
            return (HWNDComponentPeer*) GetWindowLongPtr (h, 8);

        return nullptr;
    }

    //==============================================================================
    bool isInside (HWND h) const noexcept
    {
        return GetAncestor (hwnd, GA_ROOT) == h;
    }

    //==============================================================================
    static bool isKeyDown (const int key) noexcept  { return (GetAsyncKeyState (key) & 0x8000) != 0; }

    static void updateKeyModifiers() noexcept
    {
        int keyMods = 0;
        if (isKeyDown (VK_SHIFT))   keyMods |= ModifierKeys::shiftModifier;
        if (isKeyDown (VK_CONTROL)) keyMods |= ModifierKeys::ctrlModifier;
        if (isKeyDown (VK_MENU))    keyMods |= ModifierKeys::altModifier;

        // workaround: Windows maps AltGr to left-Ctrl + right-Alt.
        if (isKeyDown (VK_RMENU) && !isKeyDown (VK_RCONTROL))
        {
            keyMods = (keyMods & ~ModifierKeys::ctrlModifier) | ModifierKeys::altModifier;
        }

        ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withOnlyMouseButtons().withFlags (keyMods);
    }

    static void updateModifiersFromWParam (const WPARAM wParam)
    {
        int mouseMods = 0;
        if (wParam & MK_LBUTTON)   mouseMods |= ModifierKeys::leftButtonModifier;
        if (wParam & MK_RBUTTON)   mouseMods |= ModifierKeys::rightButtonModifier;
        if (wParam & MK_MBUTTON)   mouseMods |= ModifierKeys::middleButtonModifier;

        ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (mouseMods);
        updateKeyModifiers();
    }

    //==============================================================================
    bool dontRepaint;
    static ModifierKeys modifiersAtLastCallback;

    //==============================================================================
    struct FileDropTarget final : public ComBaseClassHelper<IDropTarget>
    {
        FileDropTarget (HWNDComponentPeer& p)   : peer (p) {}

        JUCE_COMRESULT DragEnter (IDataObject* pDataObject, DWORD grfKeyState, POINTL mousePos, DWORD* pdwEffect) override
        {
            auto hr = updateFileList (pDataObject);

            if (FAILED (hr))
                return hr;

            return DragOver (grfKeyState, mousePos, pdwEffect);
        }

        JUCE_COMRESULT DragLeave() override
        {
            if (peerIsDeleted)
                return S_FALSE;

            peer.handleDragExit (dragInfo);
            return S_OK;
        }

        JUCE_COMRESULT DragOver (DWORD /*grfKeyState*/, POINTL mousePos, DWORD* pdwEffect) override
        {
            if (peerIsDeleted)
                return S_FALSE;

            dragInfo.position = getMousePos (mousePos).roundToInt();
            *pdwEffect = peer.handleDragMove (dragInfo) ? (DWORD) DROPEFFECT_COPY
                                                        : (DWORD) DROPEFFECT_NONE;
            return S_OK;
        }

        JUCE_COMRESULT Drop (IDataObject* pDataObject, DWORD /*grfKeyState*/, POINTL mousePos, DWORD* pdwEffect) override
        {
            auto hr = updateFileList (pDataObject);

            if (FAILED (hr))
                return hr;

            dragInfo.position = getMousePos (mousePos).roundToInt();
            *pdwEffect = peer.handleDragDrop (dragInfo) ? (DWORD) DROPEFFECT_COPY
                                                        : (DWORD) DROPEFFECT_NONE;
            return S_OK;
        }

        HWNDComponentPeer& peer;
        ComponentPeer::DragInfo dragInfo;
        bool peerIsDeleted = false;

    private:
        Point<float> getMousePos (POINTL mousePos) const
        {
            const auto originalPos = pointFromPOINT ({ mousePos.x, mousePos.y });
            const auto logicalPos = convertPhysicalScreenPointToLogical (originalPos, peer.hwnd);
            return detail::ScalingHelpers::screenPosToLocalPos (peer.component, logicalPos.toFloat());
        }

        struct DroppedData
        {
            DroppedData (IDataObject* dataObject, CLIPFORMAT type)
            {
                FORMATETC format = { type, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

                if (SUCCEEDED (error = dataObject->GetData (&format, &medium)) && medium.hGlobal != nullptr)
                {
                    dataSize = GlobalSize (medium.hGlobal);
                    data = GlobalLock (medium.hGlobal);
                }
            }

            ~DroppedData()
            {
                if (data != nullptr && medium.hGlobal != nullptr)
                    GlobalUnlock (medium.hGlobal);
            }

            HRESULT error;
            STGMEDIUM medium { TYMED_HGLOBAL, { nullptr }, nullptr };
            void* data = {};
            SIZE_T dataSize;
        };

        void parseFileList (HDROP dropFiles)
        {
            dragInfo.files.clearQuick();

            std::vector<TCHAR> nameBuffer;

            const auto numFiles = DragQueryFile (dropFiles, ~(UINT) 0, nullptr, 0);

            for (UINT i = 0; i < numFiles; ++i)
            {
                const auto bufferSize = DragQueryFile (dropFiles, i, nullptr, 0);
                nameBuffer.clear();
                nameBuffer.resize (bufferSize + 1, 0); // + 1 for the null terminator

                [[maybe_unused]] const auto readCharacters = DragQueryFile (dropFiles, i, nameBuffer.data(), (UINT) nameBuffer.size());
                jassert (readCharacters == bufferSize);

                dragInfo.files.add (String (nameBuffer.data()));
            }
        }

        HRESULT updateFileList (IDataObject* const dataObject)
        {
            if (peerIsDeleted)
                return S_FALSE;

            dragInfo.clear();

            {
                DroppedData fileData (dataObject, CF_HDROP);

                if (SUCCEEDED (fileData.error))
                {
                    parseFileList (static_cast<HDROP> (fileData.data));
                    return S_OK;
                }
            }

            DroppedData textData (dataObject, CF_UNICODETEXT);

            if (SUCCEEDED (textData.error))
            {
                dragInfo.text = String (CharPointer_UTF16 ((const WCHAR*) textData.data),
                                        CharPointer_UTF16 ((const WCHAR*) addBytesToPointer (textData.data, textData.dataSize)));
                return S_OK;
            }

            return textData.error;
        }

        JUCE_DECLARE_NON_COPYABLE (FileDropTarget)
    };

    static bool offerKeyMessageToJUCEWindow (const MSG& msg)
    {
        // If this isn't a keyboard message, let the host deal with it.

        constexpr UINT messages[] { WM_KEYDOWN, WM_SYSKEYDOWN, WM_KEYUP, WM_SYSKEYUP, WM_CHAR, WM_SYSCHAR };

        if (std::find (std::begin (messages), std::end (messages), msg.message) == std::end (messages))
            return false;

        auto* peer = getOwnerOfWindow (msg.hwnd);
        auto* focused = Component::getCurrentlyFocusedComponent();

        if (focused == nullptr || peer == nullptr || focused->getPeer() != peer)
            return false;

        auto* hwnd = static_cast<HWND> (peer->getNativeHandle());

        if (hwnd == nullptr)
            return false;

        ScopedThreadDPIAwarenessSetter threadDpiAwarenessSetter { hwnd };

        // If we've been sent a text character, process it as text.

        if (msg.message == WM_CHAR || msg.message == WM_SYSCHAR)
            return peer->doKeyChar ((int) msg.wParam, msg.lParam);

        // The event was a keypress, rather than a text character

        if (peer->findCurrentTextInputTarget() != nullptr)
        {
            // If there's a focused text input target, we want to attempt "real" text input with an
            // IME, and we want to prevent the host from eating keystrokes (spaces etc.).

            TranslateMessage (&msg);

            // TranslateMessage may post WM_CHAR back to the window, so we remove those messages
            // from the queue before the host gets to see them.
            // This will dispatch pending WM_CHAR messages, so we may end up reentering
            // offerKeyMessageToJUCEWindow and hitting the WM_CHAR case above.
            // We always return true if WM_CHAR is posted so that the keypress is not forwarded
            // to the host. Otherwise, the host may call TranslateMessage again on this message,
            // resulting in duplicate WM_CHAR messages being posted.

            MSG peeked{};
            if (PeekMessage (&peeked, hwnd, WM_CHAR, WM_DEADCHAR, PM_REMOVE)
                || PeekMessage (&peeked, hwnd, WM_SYSCHAR, WM_SYSDEADCHAR, PM_REMOVE))
            {
                return true;
            }

            // If TranslateMessage didn't add a WM_CHAR to the queue, fall back to processing the
            // event as a plain keypress
        }

        // There's no text input target, or the key event wasn't translated, so we'll just see if we
        // can use the plain keystroke event

        if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN)
            return peer->doKeyDown (msg.wParam);

        return peer->doKeyUp (msg.wParam);
    }

    double getPlatformScaleFactor() const noexcept override
    {
       #if ! JUCE_WIN_PER_MONITOR_DPI_AWARE
        return 1.0;
       #else
        if (! isPerMonitorDPIAwareWindow (hwnd))
            return 1.0;

        if (auto* parentHWND = GetParent (hwnd))
        {
            if (auto* parentPeer = getOwnerOfWindow (parentHWND))
                return parentPeer->getPlatformScaleFactor();

            if (getDPIForWindow != nullptr)
                return getScaleFactorForWindow (parentHWND);
        }

        return scaleFactor;
       #endif
    }

private:
    HWND hwnd, parentToAddTo;
    std::unique_ptr<DropShadower> shadower;
    RenderingEngineType currentRenderingEngine;
   #if JUCE_DIRECT2D
    std::unique_ptr<Direct2DLowLevelGraphicsContext> direct2DContext;
   #endif
    uint32 lastPaintTime = 0;
    ULONGLONG lastMagnifySize = 0;
    bool fullScreen = false, isDragging = false, isMouseOver = false,
         hasCreatedCaret = false, constrainerIsResizing = false;
    BorderSize<int> windowBorder;
    IconConverters::IconPtr currentWindowIcon;
    FileDropTarget* dropTarget = nullptr;
    uint8 updateLayeredWindowAlpha = 255;
    UWPUIViewSettings uwpViewSettings;
   #if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
    ModifierKeyProvider* modProvider = nullptr;
   #endif

    double scaleFactor = 1.0;
    bool inDpiChange = 0, inHandlePositionChanged = 0;
    HMONITOR currentMonitor = nullptr;

    bool isAccessibilityActive = false;

    //==============================================================================
    static MultiTouchMapper<DWORD> currentTouches;

    //==============================================================================
    struct TemporaryImage final : private Timer
    {
        TemporaryImage() {}

        Image& getImage (bool transparent, int w, int h)
        {
            auto format = transparent ? Image::ARGB : Image::RGB;

            if ((! image.isValid()) || image.getWidth() < w || image.getHeight() < h || image.getFormat() != format)
                image = Image (new WindowsBitmapImage (format, (w + 31) & ~31, (h + 31) & ~31, false));

            startTimer (3000);
            return image;
        }

        void timerCallback() override
        {
            stopTimer();
            image = {};
        }

    private:
        Image image;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemporaryImage)
    };

    TemporaryImage offscreenImageGenerator;

    //==============================================================================
    class WindowClassHolder final : private DeletedAtShutdown
    {
    public:
        WindowClassHolder()
        {
            // this name has to be different for each app/dll instance because otherwise poor old Windows can
            // get a bit confused (even despite it not being a process-global window class).
            String windowClassName ("JUCE_");
            windowClassName << String::toHexString (Time::currentTimeMillis());

            auto moduleHandle = (HINSTANCE) Process::getCurrentModuleInstanceHandle();

            TCHAR moduleFile[1024] = {};
            GetModuleFileName (moduleHandle, moduleFile, 1024);

            WNDCLASSEX wcex = {};
            wcex.cbSize         = sizeof (wcex);
            wcex.style          = CS_OWNDC;
            wcex.lpfnWndProc    = (WNDPROC) windowProc;
            wcex.lpszClassName  = windowClassName.toWideCharPointer();
            wcex.cbWndExtra     = 32;
            wcex.hInstance      = moduleHandle;

            for (const auto& [index, field, ptr] : { std::tuple { 0, &wcex.hIcon,   &iconBig },
                                                     std::tuple { 1, &wcex.hIconSm, &iconSmall } })
            {
                auto iconNum = static_cast<WORD> (index);
                ptr->reset (*field = ExtractAssociatedIcon (moduleHandle, moduleFile, &iconNum));
            }

            atom = RegisterClassEx (&wcex);
            jassert (atom != 0);

            isEventBlockedByModalComps = checkEventBlockedByModalComps;
        }

        ~WindowClassHolder()
        {
            if (ComponentPeer::getNumPeers() == 0)
                UnregisterClass (getWindowClassName(), (HINSTANCE) Process::getCurrentModuleInstanceHandle());

            clearSingletonInstance();
        }

        LPCTSTR getWindowClassName() const noexcept     { return (LPCTSTR) (pointer_sized_uint) atom; }

        JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL (WindowClassHolder)

    private:
        ATOM atom;

        static bool isHWNDBlockedByModalComponents (HWND h)
        {
            for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
                if (auto* c = Desktop::getInstance().getComponent (i))
                    if ((! c->isCurrentlyBlockedByAnotherModalComponent())
                          && IsChild ((HWND) c->getWindowHandle(), h))
                        return false;

            return true;
        }

        static bool checkEventBlockedByModalComps (const MSG& m)
        {
            if (Component::getNumCurrentlyModalComponents() == 0 || JuceWindowIdentifier::isJUCEWindow (m.hwnd))
                return false;

            switch (m.message)
            {
                case WM_MOUSEMOVE:
                case WM_NCMOUSEMOVE:
                case 0x020A: /* WM_MOUSEWHEEL */
                case 0x020E: /* WM_MOUSEHWHEEL */
                case WM_KEYUP:
                case WM_SYSKEYUP:
                case WM_CHAR:
                case WM_APPCOMMAND:
                case WM_LBUTTONUP:
                case WM_MBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MOUSEACTIVATE:
                case WM_NCMOUSEHOVER:
                case WM_MOUSEHOVER:
                case WM_TOUCH:
                case WM_POINTERUPDATE:
                case WM_NCPOINTERUPDATE:
                case WM_POINTERWHEEL:
                case WM_POINTERHWHEEL:
                case WM_POINTERUP:
                case WM_POINTERACTIVATE:
                    return isHWNDBlockedByModalComponents (m.hwnd);
                case WM_NCLBUTTONDOWN:
                case WM_NCLBUTTONDBLCLK:
                case WM_NCRBUTTONDOWN:
                case WM_NCRBUTTONDBLCLK:
                case WM_NCMBUTTONDOWN:
                case WM_NCMBUTTONDBLCLK:
                case WM_LBUTTONDOWN:
                case WM_LBUTTONDBLCLK:
                case WM_MBUTTONDOWN:
                case WM_MBUTTONDBLCLK:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONDBLCLK:
                case WM_KEYDOWN:
                case WM_SYSKEYDOWN:
                case WM_NCPOINTERDOWN:
                case WM_POINTERDOWN:
                    if (isHWNDBlockedByModalComponents (m.hwnd))
                    {
                        if (auto* modal = Component::getCurrentlyModalComponent (0))
                            modal->inputAttemptWhenModal();

                        return true;
                    }
                    break;

                default:
                    break;
            }

            return false;
        }

        IconConverters::IconPtr iconBig, iconSmall;

        JUCE_DECLARE_NON_COPYABLE (WindowClassHolder)
    };

    //==============================================================================
    static void* createWindowCallback (void* userData)
    {
        static_cast<HWNDComponentPeer*> (userData)->createWindow();
        return nullptr;
    }

    void createWindow()
    {
        DWORD exstyle = 0;
        DWORD type = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

        if (hasTitleBar())
        {
            type |= WS_OVERLAPPED;

            if ((styleFlags & windowHasCloseButton) != 0)
            {
                type |= WS_SYSMENU;
            }
            else
            {
                // annoyingly, windows won't let you have a min/max button without a close button
                jassert ((styleFlags & (windowHasMinimiseButton | windowHasMaximiseButton)) == 0);
            }

            if ((styleFlags & windowIsResizable) != 0)
                type |= WS_THICKFRAME;
        }
        else if (parentToAddTo != nullptr)
        {
            type |= WS_CHILD;
        }
        else
        {
            type |= WS_POPUP | WS_SYSMENU;
        }

        if ((styleFlags & windowAppearsOnTaskbar) == 0)
            exstyle |= WS_EX_TOOLWINDOW;
        else
            exstyle |= WS_EX_APPWINDOW;

        if ((styleFlags & windowHasMinimiseButton) != 0)    type |= WS_MINIMIZEBOX;
        if ((styleFlags & windowHasMaximiseButton) != 0)    type |= WS_MAXIMIZEBOX;
        if ((styleFlags & windowIgnoresMouseClicks) != 0)   exstyle |= WS_EX_TRANSPARENT;
        if ((styleFlags & windowIsSemiTransparent) != 0)    exstyle |= WS_EX_LAYERED;

        hwnd = CreateWindowEx (exstyle, WindowClassHolder::getInstance()->getWindowClassName(),
                               L"", type, 0, 0, 0, 0, parentToAddTo, nullptr,
                               (HINSTANCE) Process::getCurrentModuleInstanceHandle(), nullptr);

       #if JUCE_DEBUG
        // The DPI-awareness context of this window and JUCE's hidden message window are different.
        // You normally want these to match otherwise timer events and async messages will happen
        // in a different context to normal HWND messages which can cause issues with UI scaling.
        jassert (isPerMonitorDPIAwareWindow (hwnd) == isPerMonitorDPIAwareWindow (juce_messageWindowHandle)
                   || isInScopedDPIAwarenessDisabler());
       #endif

        if (hwnd != nullptr)
        {
            SetWindowLongPtr (hwnd, 0, 0);
            SetWindowLongPtr (hwnd, 8, (LONG_PTR) this);
            JuceWindowIdentifier::setAsJUCEWindow (hwnd, true);

            if (dropTarget == nullptr)
            {
                HWNDComponentPeer* peer = nullptr;

                if (dontRepaint)
                    peer = getOwnerOfWindow (parentToAddTo);

                if (peer == nullptr)
                    peer = this;

                dropTarget = new FileDropTarget (*peer);
            }

            RegisterDragDrop (hwnd, dropTarget);

            if (canUseMultiTouch())
                registerTouchWindow (hwnd, 0);

            setDPIAwareness();

            if (isPerMonitorDPIAwareThread())
                scaleFactor = getScaleFactorForWindow (hwnd);

            setMessageFilter();
            updateBorderSize();
            checkForPointerAPI();

            // This is needed so that our plugin window gets notified of WM_SETTINGCHANGE messages
            // and can respond to display scale changes
            if (! JUCEApplication::isStandaloneApp())
                settingChangeCallback = ComponentPeer::forceDisplayUpdate;

            // Calling this function here is (for some reason) necessary to make Windows
            // correctly enable the menu items that we specify in the wm_initmenu message.
            GetSystemMenu (hwnd, false);

            auto alpha = component.getAlpha();
            if (alpha < 1.0f)
                setAlpha (alpha);
        }
        else
        {
            TCHAR messageBuffer[256] = {};

            FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           nullptr, GetLastError(), MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                           messageBuffer, (DWORD) numElementsInArray (messageBuffer) - 1, nullptr);

            DBG (messageBuffer);
            jassertfalse;
        }
    }

    static BOOL CALLBACK revokeChildDragDropCallback (HWND hwnd, LPARAM)    { RevokeDragDrop (hwnd); return TRUE; }

    static void* destroyWindowCallback (void* handle)
    {
        auto hwnd = reinterpret_cast<HWND> (handle);

        if (IsWindow (hwnd))
        {
            RevokeDragDrop (hwnd);

            // NB: we need to do this before DestroyWindow() as child HWNDs will be invalid after
            EnumChildWindows (hwnd, revokeChildDragDropCallback, 0);

            DestroyWindow (hwnd);
        }

        return nullptr;
    }

    static void* toFrontCallback1 (void* h)
    {
        BringWindowToTop ((HWND) h);
        return nullptr;
    }

    static void* toFrontCallback2 (void* h)
    {
        setWindowZOrder ((HWND) h, HWND_TOP);
        return nullptr;
    }

    static void* setFocusCallback (void* h)
    {
        SetFocus ((HWND) h);
        return nullptr;
    }

    static void* getFocusCallback (void*)
    {
        return GetFocus();
    }

    bool isUsingUpdateLayeredWindow() const
    {
        return ! component.isOpaque();
    }

    bool hasTitleBar() const noexcept        { return (styleFlags & windowHasTitleBar) != 0; }

    void updateShadower()
    {
        if (! component.isCurrentlyModal() && (styleFlags & windowHasDropShadow) != 0
            && ((! hasTitleBar()) || SystemStats::getOperatingSystemType() < SystemStats::WinVista))
        {
            shadower = component.getLookAndFeel().createDropShadowerForComponent (component);

            if (shadower != nullptr)
                shadower->setOwner (&component);
        }
    }

    void setIcon (const Image& newIcon) override
    {
        if (IconConverters::IconPtr hicon { IconConverters::createHICONFromImage (newIcon, TRUE, 0, 0) })
        {
            SendMessage (hwnd, WM_SETICON, ICON_BIG,   reinterpret_cast<LPARAM> (hicon.get()));
            SendMessage (hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM> (hicon.get()));
            currentWindowIcon = std::move (hicon);
        }
    }

    void setMessageFilter()
    {
        using ChangeWindowMessageFilterExFunc = BOOL (WINAPI*) (HWND, UINT, DWORD, PVOID);

        static auto changeMessageFilter = (ChangeWindowMessageFilterExFunc) getUser32Function ("ChangeWindowMessageFilterEx");

        if (changeMessageFilter != nullptr)
        {
            changeMessageFilter (hwnd, WM_DROPFILES, 1 /*MSGFLT_ALLOW*/, nullptr);
            changeMessageFilter (hwnd, WM_COPYDATA, 1 /*MSGFLT_ALLOW*/, nullptr);
            changeMessageFilter (hwnd, 0x49, 1 /*MSGFLT_ALLOW*/, nullptr);
        }
    }

    struct ChildWindowClippingInfo
    {
        HDC dc;
        HWNDComponentPeer* peer;
        RectangleList<int>* clip;
        Point<int> origin;
        int savedDC;
    };

    static BOOL CALLBACK clipChildWindowCallback (HWND hwnd, LPARAM context)
    {
        if (IsWindowVisible (hwnd))
        {
            auto& info = *(ChildWindowClippingInfo*) context;

            if (GetParent (hwnd) == info.peer->hwnd)
            {
                auto clip = rectangleFromRECT (getWindowClientRect (hwnd));

                info.clip->subtract (clip - info.origin);

                if (info.savedDC == 0)
                    info.savedDC = SaveDC (info.dc);

                ExcludeClipRect (info.dc, clip.getX(), clip.getY(), clip.getRight(), clip.getBottom());
            }
        }

        return TRUE;
    }

    //==============================================================================
    void handlePaintMessage()
    {
       #if JUCE_DIRECT2D
        if (direct2DContext != nullptr)
        {
            RECT r;

            if (GetUpdateRect (hwnd, &r, false))
            {
                direct2DContext->start();
                direct2DContext->clipToRectangle (convertPhysicalScreenRectangleToLogical (rectangleFromRECT (r), hwnd));
                handlePaint (*direct2DContext);
                direct2DContext->end();
                ValidateRect (hwnd, &r);
            }
        }
        else
       #endif
        {
            HRGN rgn = CreateRectRgn (0, 0, 0, 0);
            const int regionType = GetUpdateRgn (hwnd, rgn, false);

            PAINTSTRUCT paintStruct;
            HDC dc = BeginPaint (hwnd, &paintStruct); // Note this can immediately generate a WM_NCPAINT
                                                      // message and become re-entrant, but that's OK

            // if something in a paint handler calls, e.g. a message box, this can become reentrant and
            // corrupt the image it's using to paint into, so do a check here.
            static bool reentrant = false;

            if (! reentrant)
            {
                const ScopedValueSetter<bool> setter (reentrant, true, false);

                if (dontRepaint)
                    component.handleCommandMessage (0); // (this triggers a repaint in the openGL context)
                else
                    performPaint (dc, rgn, regionType, paintStruct);
            }

            DeleteObject (rgn);
            EndPaint (hwnd, &paintStruct);

           #if JUCE_MSVC
            _fpreset(); // because some graphics cards can unmask FP exceptions
           #endif

        }

        lastPaintTime = Time::getMillisecondCounter();
    }

    void performPaint (HDC dc, HRGN rgn, int regionType, PAINTSTRUCT& paintStruct)
    {
        int x = paintStruct.rcPaint.left;
        int y = paintStruct.rcPaint.top;
        int w = paintStruct.rcPaint.right - x;
        int h = paintStruct.rcPaint.bottom - y;

        const bool transparent = isUsingUpdateLayeredWindow();

        if (transparent)
        {
            // it's not possible to have a transparent window with a title bar at the moment!
            jassert (! hasTitleBar());

            auto r = getWindowScreenRect (hwnd);
            x = y = 0;
            w = r.right - r.left;
            h = r.bottom - r.top;
        }

        if (w > 0 && h > 0)
        {
            Image& offscreenImage = offscreenImageGenerator.getImage (transparent, w, h);

            RectangleList<int> contextClip;
            const Rectangle<int> clipBounds (w, h);

            bool needToPaintAll = true;

            if (regionType == COMPLEXREGION && ! transparent)
            {
                HRGN clipRgn = CreateRectRgnIndirect (&paintStruct.rcPaint);
                CombineRgn (rgn, rgn, clipRgn, RGN_AND);
                DeleteObject (clipRgn);

                std::aligned_storage_t<8192, alignof (RGNDATA)> rgnData;
                const DWORD res = GetRegionData (rgn, sizeof (rgnData), (RGNDATA*) &rgnData);

                if (res > 0 && res <= sizeof (rgnData))
                {
                    const RGNDATAHEADER* const hdr = &(((const RGNDATA*) &rgnData)->rdh);

                    if (hdr->iType == RDH_RECTANGLES
                         && hdr->rcBound.right - hdr->rcBound.left >= w
                         && hdr->rcBound.bottom - hdr->rcBound.top >= h)
                    {
                        needToPaintAll = false;

                        auto rects = unalignedPointerCast<const RECT*> ((char*) &rgnData + sizeof (RGNDATAHEADER));

                        for (int i = (int) ((RGNDATA*) &rgnData)->rdh.nCount; --i >= 0;)
                        {
                            if (rects->right <= x + w && rects->bottom <= y + h)
                            {
                                const int cx = jmax (x, (int) rects->left);
                                contextClip.addWithoutMerging (Rectangle<int> (cx - x, rects->top - y,
                                                                               rects->right - cx, rects->bottom - rects->top)
                                                                   .getIntersection (clipBounds));
                            }
                            else
                            {
                                needToPaintAll = true;
                                break;
                            }

                            ++rects;
                        }
                    }
                }
            }

            if (needToPaintAll)
            {
                contextClip.clear();
                contextClip.addWithoutMerging (Rectangle<int> (w, h));
            }

            ChildWindowClippingInfo childClipInfo = { dc, this, &contextClip, Point<int> (x, y), 0 };
            EnumChildWindows (hwnd, clipChildWindowCallback, (LPARAM) &childClipInfo);

            if (! contextClip.isEmpty())
            {
                if (transparent)
                    for (auto& i : contextClip)
                        offscreenImage.clear (i);

                {
                    auto context = component.getLookAndFeel()
                                    .createGraphicsContext (offscreenImage, { -x, -y }, contextClip);

                    context->addTransform (AffineTransform::scale ((float) getPlatformScaleFactor()));
                    handlePaint (*context);
                }

                static_cast<WindowsBitmapImage*> (offscreenImage.getPixelData())
                    ->blitToWindow (hwnd, dc, transparent, x, y, updateLayeredWindowAlpha);
            }

            if (childClipInfo.savedDC != 0)
                RestoreDC (dc, childClipInfo.savedDC);
        }
    }

    //==============================================================================
    void doMouseEvent (Point<float> position, float pressure, float orientation = 0.0f, ModifierKeys mods = ModifierKeys::currentModifiers)
    {
        handleMouseEvent (MouseInputSource::InputSourceType::mouse, position, mods, pressure, orientation, getMouseEventTime());
    }

    StringArray getAvailableRenderingEngines() override
    {
        StringArray s ("Software Renderer");

       #if JUCE_DIRECT2D
        if (SystemStats::getOperatingSystemType() >= SystemStats::Windows7)
            s.add ("Direct2D");
       #endif

        return s;
    }

    int getCurrentRenderingEngine() const override    { return currentRenderingEngine; }

   #if JUCE_DIRECT2D
    void updateDirect2DContext()
    {
        if (currentRenderingEngine != direct2DRenderingEngine)
            direct2DContext = nullptr;
        else if (direct2DContext == nullptr)
            direct2DContext.reset (new Direct2DLowLevelGraphicsContext (hwnd));
    }
   #endif

    void setCurrentRenderingEngine ([[maybe_unused]] int index) override
    {
       #if JUCE_DIRECT2D
        if (getAvailableRenderingEngines().size() > 1)
        {
            currentRenderingEngine = index == 1 ? direct2DRenderingEngine : softwareRenderingEngine;
            updateDirect2DContext();
            repaint (component.getLocalBounds());
        }
       #endif
    }

    static uint32 getMinTimeBetweenMouseMoves()
    {
        if (SystemStats::getOperatingSystemType() >= SystemStats::WinVista)
            return 0;

        return 1000 / 60;  // Throttling the incoming mouse-events seems to still be needed in XP..
    }

    bool isTouchEvent() noexcept
    {
        if (registerTouchWindow == nullptr)
            return false;

        // Relevant info about touch/pen detection flags:
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms703320(v=vs.85).aspx
        // http://www.petertissen.de/?p=4

        return ((uint32_t) GetMessageExtraInfo() & 0xFFFFFF80 /*SIGNATURE_MASK*/) == 0xFF515780 /*MI_WP_SIGNATURE*/;
    }

    static bool areOtherTouchSourcesActive()
    {
        for (auto& ms : Desktop::getInstance().getMouseSources())
            if (ms.isDragging() && (ms.getType() == MouseInputSource::InputSourceType::touch
                                     || ms.getType() == MouseInputSource::InputSourceType::pen))
                return true;

        return false;
    }

    void doMouseMove (Point<float> position, bool isMouseDownEvent)
    {
        ModifierKeys modsToSend (ModifierKeys::currentModifiers);

        // this will be handled by WM_TOUCH
        if (isTouchEvent() || areOtherTouchSourcesActive())
            return;

        if (! isMouseOver)
        {
            isMouseOver = true;

            // This avoids a rare stuck-button problem when focus is lost unexpectedly, but must
            // not be called as part of a move, in case it's actually a mouse-drag from another
            // app which ends up here when we get focus before the mouse is released..
            if (isMouseDownEvent)
                NullCheckedInvocation::invoke (getNativeRealtimeModifiers);

            updateKeyModifiers();

           #if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
            if (modProvider != nullptr)
                ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withFlags (modProvider->getWin32Modifiers());
           #endif

            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof (tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            tme.dwHoverTime = 0;

            if (! TrackMouseEvent (&tme))
                jassertfalse;

            Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
        }
        else if (! isDragging)
        {
            if (! contains (position.roundToInt(), false))
                return;
        }

        static uint32 lastMouseTime = 0;
        static auto minTimeBetweenMouses = getMinTimeBetweenMouseMoves();
        auto now = Time::getMillisecondCounter();

        if (! Desktop::getInstance().getMainMouseSource().isDragging())
            modsToSend = modsToSend.withoutMouseButtons();

        if (now >= lastMouseTime + minTimeBetweenMouses)
        {
            lastMouseTime = now;
            doMouseEvent (position, MouseInputSource::defaultPressure,
                          MouseInputSource::defaultOrientation, modsToSend);
        }
    }

    void doMouseDown (Point<float> position, const WPARAM wParam)
    {
        // this will be handled by WM_TOUCH
        if (isTouchEvent() || areOtherTouchSourcesActive())
            return;

        if (GetCapture() != hwnd)
            SetCapture (hwnd);

        doMouseMove (position, true);

        if (isValidPeer (this))
        {
            updateModifiersFromWParam (wParam);

           #if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
            if (modProvider != nullptr)
                ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withFlags (modProvider->getWin32Modifiers());
           #endif

            isDragging = true;

            doMouseEvent (position, MouseInputSource::defaultPressure);
        }
    }

    void doMouseUp (Point<float> position, const WPARAM wParam)
    {
        // this will be handled by WM_TOUCH
        if (isTouchEvent() || areOtherTouchSourcesActive())
            return;

        updateModifiersFromWParam (wParam);

       #if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
        if (modProvider != nullptr)
            ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withFlags (modProvider->getWin32Modifiers());
       #endif

        const bool wasDragging = isDragging;
        isDragging = false;

        // release the mouse capture if the user has released all buttons
        if ((wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON)) == 0 && hwnd == GetCapture())
            ReleaseCapture();

        // NB: under some circumstances (e.g. double-clicking a native title bar), a mouse-up can
        // arrive without a mouse-down, so in that case we need to avoid sending a message.
        if (wasDragging)
            doMouseEvent (position, MouseInputSource::defaultPressure);
    }

    void doCaptureChanged()
    {
        if (constrainerIsResizing)
        {
            if (constrainer != nullptr)
                constrainer->resizeEnd();

            constrainerIsResizing = false;
        }

        if (isDragging)
            doMouseUp (getCurrentMousePos(), (WPARAM) 0);
    }

    void doMouseExit()
    {
        isMouseOver = false;

        if (! areOtherTouchSourcesActive())
            doMouseEvent (getCurrentMousePos(), MouseInputSource::defaultPressure);
    }

    std::tuple<ComponentPeer*, Point<float>> findPeerUnderMouse()
    {
        auto currentMousePos = getPOINTFromLParam ((LPARAM) GetMessagePos());

        // Because Windows stupidly sends all wheel events to the window with the keyboard
        // focus, we have to redirect them here according to the mouse pos..
        auto* peer = getOwnerOfWindow (WindowFromPoint (currentMousePos));

        if (peer == nullptr)
            peer = this;

        return std::tuple (peer, peer->globalToLocal (convertPhysicalScreenPointToLogical (pointFromPOINT (currentMousePos), hwnd).toFloat()));
    }

    static MouseInputSource::InputSourceType getPointerType (WPARAM wParam)
    {
        if (getPointerTypeFunction != nullptr)
        {
            POINTER_INPUT_TYPE pointerType;

            if (getPointerTypeFunction (GET_POINTERID_WPARAM (wParam), &pointerType))
            {
                if (pointerType == 2)
                    return MouseInputSource::InputSourceType::touch;

                if (pointerType == 3)
                    return MouseInputSource::InputSourceType::pen;
            }
        }

        return MouseInputSource::InputSourceType::mouse;
    }

    void doMouseWheel (const WPARAM wParam, const bool isVertical)
    {
        updateKeyModifiers();
        const float amount = jlimit (-1000.0f, 1000.0f, 0.5f * (short) HIWORD (wParam));

        MouseWheelDetails wheel;
        wheel.deltaX = isVertical ? 0.0f : amount / -256.0f;
        wheel.deltaY = isVertical ? amount / 256.0f : 0.0f;
        wheel.isReversed = false;
        wheel.isSmooth = false;
        wheel.isInertial = false;

        if (const auto [peer, localPos] = findPeerUnderMouse(); peer != nullptr)
            peer->handleMouseWheel (getPointerType (wParam), localPos, getMouseEventTime(), wheel);
    }

    bool doGestureEvent (LPARAM lParam)
    {
        GESTUREINFO gi;
        zerostruct (gi);
        gi.cbSize = sizeof (gi);

        if (getGestureInfo != nullptr && getGestureInfo ((HGESTUREINFO) lParam, &gi))
        {
            updateKeyModifiers();

            if (const auto [peer, localPos] = findPeerUnderMouse(); peer != nullptr)
            {
                switch (gi.dwID)
                {
                    case 3: /*GID_ZOOM*/
                        if (gi.dwFlags != 1 /*GF_BEGIN*/ && lastMagnifySize > 0)
                            peer->handleMagnifyGesture (MouseInputSource::InputSourceType::touch, localPos, getMouseEventTime(),
                                                        (float) ((double) gi.ullArguments / (double) lastMagnifySize));

                        lastMagnifySize = gi.ullArguments;
                        return true;

                    case 4: /*GID_PAN*/
                    case 5: /*GID_ROTATE*/
                    case 6: /*GID_TWOFINGERTAP*/
                    case 7: /*GID_PRESSANDTAP*/
                    default:
                        break;
                }
            }
        }

        return false;
    }

    LRESULT doTouchEvent (const int numInputs, HTOUCHINPUT eventHandle)
    {
        if ((styleFlags & windowIgnoresMouseClicks) != 0)
            if (auto* parent = getOwnerOfWindow (GetParent (hwnd)))
                if (parent != this)
                    return parent->doTouchEvent (numInputs, eventHandle);

        HeapBlock<TOUCHINPUT> inputInfo (numInputs);

        if (getTouchInputInfo (eventHandle, (UINT) numInputs, inputInfo, sizeof (TOUCHINPUT)))
        {
            for (int i = 0; i < numInputs; ++i)
            {
                auto flags = inputInfo[i].dwFlags;

                if ((flags & (TOUCHEVENTF_DOWN | TOUCHEVENTF_MOVE | TOUCHEVENTF_UP)) != 0)
                    if (! handleTouchInput (inputInfo[i], (flags & TOUCHEVENTF_DOWN) != 0, (flags & TOUCHEVENTF_UP) != 0))
                        return 0;  // abandon method if this window was deleted by the callback
            }
        }

        closeTouchInputHandle (eventHandle);
        return 0;
    }

    bool handleTouchInput (const TOUCHINPUT& touch, const bool isDown, const bool isUp,
                           const float touchPressure = MouseInputSource::defaultPressure,
                           const float orientation = 0.0f)
    {
        auto isCancel = false;

        const auto touchIndex = currentTouches.getIndexOfTouch (this, touch.dwID);
        const auto time = getMouseEventTime();
        const auto pos = globalToLocal (convertPhysicalScreenPointToLogical (pointFromPOINT ({ roundToInt (touch.x / 100.0f),
                                                                                               roundToInt (touch.y / 100.0f) }), hwnd).toFloat());
        const auto pressure = touchPressure;
        auto modsToSend = ModifierKeys::currentModifiers;

        if (isDown)
        {
            ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
            modsToSend = ModifierKeys::currentModifiers;

            // this forces a mouse-enter/up event, in case for some reason we didn't get a mouse-up before.
            handleMouseEvent (MouseInputSource::InputSourceType::touch, pos, modsToSend.withoutMouseButtons(),
                              pressure, orientation, time, {}, touchIndex);

            if (! isValidPeer (this)) // (in case this component was deleted by the event)
                return false;
        }
        else if (isUp)
        {
            modsToSend = modsToSend.withoutMouseButtons();
            ModifierKeys::currentModifiers = modsToSend;
            currentTouches.clearTouch (touchIndex);

            if (! currentTouches.areAnyTouchesActive())
                isCancel = true;
        }
        else
        {
            modsToSend = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
        }

        handleMouseEvent (MouseInputSource::InputSourceType::touch, pos, modsToSend,
                          pressure, orientation, time, {}, touchIndex);

        if (! isValidPeer (this))
            return false;

        if (isUp)
        {
            handleMouseEvent (MouseInputSource::InputSourceType::touch, MouseInputSource::offscreenMousePos, ModifierKeys::currentModifiers.withoutMouseButtons(),
                              pressure, orientation, time, {}, touchIndex);

            if (! isValidPeer (this))
                return false;

            if (isCancel)
            {
                currentTouches.clear();
                ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons();
            }
        }

        return true;
    }

    bool handlePointerInput (WPARAM wParam, LPARAM lParam, const bool isDown, const bool isUp)
    {
        if (! canUsePointerAPI)
            return false;

        auto pointerType = getPointerType (wParam);

        if (pointerType == MouseInputSource::InputSourceType::touch)
        {
            POINTER_TOUCH_INFO touchInfo;

            if (! getPointerTouchInfo (GET_POINTERID_WPARAM (wParam), &touchInfo))
                return false;

            const auto pressure = touchInfo.touchMask & TOUCH_MASK_PRESSURE ? static_cast<float> (touchInfo.pressure)
                                                                            : MouseInputSource::defaultPressure;
            const auto orientation = touchInfo.touchMask & TOUCH_MASK_ORIENTATION ? degreesToRadians (static_cast<float> (touchInfo.orientation))
                                                                                  : MouseInputSource::defaultOrientation;

            if (! handleTouchInput (emulateTouchEventFromPointer (touchInfo.pointerInfo.ptPixelLocationRaw, wParam),
                                    isDown, isUp, pressure, orientation))
                return false;
        }
        else if (pointerType == MouseInputSource::InputSourceType::pen)
        {
            POINTER_PEN_INFO penInfo;

            if (! getPointerPenInfo (GET_POINTERID_WPARAM (wParam), &penInfo))
                return false;

            const auto pressure = (penInfo.penMask & PEN_MASK_PRESSURE) ? (float) penInfo.pressure / 1024.0f : MouseInputSource::defaultPressure;

            if (! handlePenInput (penInfo, globalToLocal (convertPhysicalScreenPointToLogical (pointFromPOINT (getPOINTFromLParam (lParam)), hwnd).toFloat()),
                                  pressure, isDown, isUp))
                return false;
        }
        else
        {
            return false;
        }

        return true;
    }

    TOUCHINPUT emulateTouchEventFromPointer (POINT p, WPARAM wParam)
    {
        TOUCHINPUT touchInput;

        touchInput.dwID = GET_POINTERID_WPARAM (wParam);
        touchInput.x = p.x * 100;
        touchInput.y = p.y * 100;

        return touchInput;
    }

    bool handlePenInput (POINTER_PEN_INFO penInfo, Point<float> pos, const float pressure, bool isDown, bool isUp)
    {
        const auto time = getMouseEventTime();
        ModifierKeys modsToSend (ModifierKeys::currentModifiers);
        PenDetails penDetails;

        penDetails.rotation = (penInfo.penMask & PEN_MASK_ROTATION) ? degreesToRadians (static_cast<float> (penInfo.rotation)) : MouseInputSource::defaultRotation;
        penDetails.tiltX = (penInfo.penMask & PEN_MASK_TILT_X) ? (float) penInfo.tiltX / 90.0f : MouseInputSource::defaultTiltX;
        penDetails.tiltY = (penInfo.penMask & PEN_MASK_TILT_Y) ? (float) penInfo.tiltY / 90.0f : MouseInputSource::defaultTiltY;

        auto pInfoFlags = penInfo.pointerInfo.pointerFlags;

        if ((pInfoFlags & POINTER_FLAG_FIRSTBUTTON) != 0)
            ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
        else if ((pInfoFlags & POINTER_FLAG_SECONDBUTTON) != 0)
            ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::rightButtonModifier);

        if (isDown)
        {
            modsToSend = ModifierKeys::currentModifiers;

            // this forces a mouse-enter/up event, in case for some reason we didn't get a mouse-up before.
            handleMouseEvent (MouseInputSource::InputSourceType::pen, pos, modsToSend.withoutMouseButtons(),
                              pressure, MouseInputSource::defaultOrientation, time, penDetails);

            if (! isValidPeer (this)) // (in case this component was deleted by the event)
                return false;
        }
        else if (isUp || ! (pInfoFlags & POINTER_FLAG_INCONTACT))
        {
            modsToSend = modsToSend.withoutMouseButtons();
            ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons();
        }

        handleMouseEvent (MouseInputSource::InputSourceType::pen, pos, modsToSend, pressure,
                          MouseInputSource::defaultOrientation, time, penDetails);

        if (! isValidPeer (this)) // (in case this component was deleted by the event)
            return false;

        if (isUp)
        {
            handleMouseEvent (MouseInputSource::InputSourceType::pen, MouseInputSource::offscreenMousePos, ModifierKeys::currentModifiers,
                              pressure, MouseInputSource::defaultOrientation, time, penDetails);

            if (! isValidPeer (this))
                return false;
        }

        return true;
    }

    //==============================================================================
    void sendModifierKeyChangeIfNeeded()
    {
        if (modifiersAtLastCallback != ModifierKeys::currentModifiers)
        {
            modifiersAtLastCallback = ModifierKeys::currentModifiers;
            handleModifierKeysChange();
        }
    }

    bool doKeyUp (const WPARAM key)
    {
        updateKeyModifiers();

        switch (key)
        {
            case VK_SHIFT:
            case VK_CONTROL:
            case VK_MENU:
            case VK_CAPITAL:
            case VK_LWIN:
            case VK_RWIN:
            case VK_APPS:
            case VK_NUMLOCK:
            case VK_SCROLL:
            case VK_LSHIFT:
            case VK_RSHIFT:
            case VK_LCONTROL:
            case VK_LMENU:
            case VK_RCONTROL:
            case VK_RMENU:
                sendModifierKeyChangeIfNeeded();
        }

        return handleKeyUpOrDown (false)
                || Component::getCurrentlyModalComponent() != nullptr;
    }

    bool doKeyDown (const WPARAM key)
    {
        updateKeyModifiers();
        bool used = false;

        switch (key)
        {
            case VK_SHIFT:
            case VK_LSHIFT:
            case VK_RSHIFT:
            case VK_CONTROL:
            case VK_LCONTROL:
            case VK_RCONTROL:
            case VK_MENU:
            case VK_LMENU:
            case VK_RMENU:
            case VK_LWIN:
            case VK_RWIN:
            case VK_CAPITAL:
            case VK_NUMLOCK:
            case VK_SCROLL:
            case VK_APPS:
                used = handleKeyUpOrDown (true);
                sendModifierKeyChangeIfNeeded();
                break;

            case VK_LEFT:
            case VK_RIGHT:
            case VK_UP:
            case VK_DOWN:
            case VK_PRIOR:
            case VK_NEXT:
            case VK_HOME:
            case VK_END:
            case VK_DELETE:
            case VK_INSERT:
            case VK_F1:
            case VK_F2:
            case VK_F3:
            case VK_F4:
            case VK_F5:
            case VK_F6:
            case VK_F7:
            case VK_F8:
            case VK_F9:
            case VK_F10:
            case VK_F11:
            case VK_F12:
            case VK_F13:
            case VK_F14:
            case VK_F15:
            case VK_F16:
            case VK_F17:
            case VK_F18:
            case VK_F19:
            case VK_F20:
            case VK_F21:
            case VK_F22:
            case VK_F23:
            case VK_F24:
                used = handleKeyUpOrDown (true);
                used = handleKeyPress (extendedKeyModifier | (int) key, 0) || used;
                break;

            default:
                used = handleKeyUpOrDown (true);

                {
                    MSG msg;
                    if (! PeekMessage (&msg, hwnd, WM_CHAR, WM_DEADCHAR, PM_NOREMOVE))
                    {
                        // if there isn't a WM_CHAR or WM_DEADCHAR message pending, we need to
                        // manually generate the key-press event that matches this key-down.
                        const UINT keyChar  = MapVirtualKey ((UINT) key, 2);
                        const UINT scanCode = MapVirtualKey ((UINT) key, 0);
                        BYTE keyState[256];
                        [[maybe_unused]] const auto state = GetKeyboardState (keyState);

                        WCHAR text[16] = { 0 };
                        if (ToUnicode ((UINT) key, scanCode, keyState, text, 8, 0) != 1)
                            text[0] = 0;

                        used = handleKeyPress ((int) LOWORD (keyChar), (juce_wchar) text[0]) || used;
                    }
                }

                break;
        }

        return used || (Component::getCurrentlyModalComponent() != nullptr);
    }

    bool doKeyChar (int key, const LPARAM flags)
    {
        updateKeyModifiers();

        auto textChar = (juce_wchar) key;
        const int virtualScanCode = (flags >> 16) & 0xff;

        if (key >= '0' && key <= '9')
        {
            switch (virtualScanCode)  // check for a numeric keypad scan-code
            {
                case 0x52:
                case 0x4f:
                case 0x50:
                case 0x51:
                case 0x4b:
                case 0x4c:
                case 0x4d:
                case 0x47:
                case 0x48:
                case 0x49:
                    key = (key - '0') + KeyPress::numberPad0;
                    break;
                default:
                    break;
            }
        }
        else
        {
            // convert the scan code to an unmodified character code..
            const UINT virtualKey = MapVirtualKey ((UINT) virtualScanCode, 1);
            UINT keyChar = MapVirtualKey (virtualKey, 2);

            keyChar = LOWORD (keyChar);

            if (keyChar != 0)
                key = (int) keyChar;

            // avoid sending junk text characters for some control-key combinations
            if (textChar < ' ' && ModifierKeys::currentModifiers.testFlags (ModifierKeys::ctrlModifier | ModifierKeys::altModifier))
                textChar = 0;
        }

        return handleKeyPress (key, textChar);
    }

    void forwardMessageToParent (UINT message, WPARAM wParam, LPARAM lParam) const
    {
        if (HWND parentH = GetParent (hwnd))
            PostMessage (parentH, message, wParam, lParam);
    }

    bool doAppCommand (const LPARAM lParam)
    {
        int key = 0;

        switch (GET_APPCOMMAND_LPARAM (lParam))
        {
            case APPCOMMAND_MEDIA_PLAY_PAUSE:       key = KeyPress::playKey; break;
            case APPCOMMAND_MEDIA_STOP:             key = KeyPress::stopKey; break;
            case APPCOMMAND_MEDIA_NEXTTRACK:        key = KeyPress::fastForwardKey; break;
            case APPCOMMAND_MEDIA_PREVIOUSTRACK:    key = KeyPress::rewindKey; break;
            default: break;
        }

        if (key != 0)
        {
            updateKeyModifiers();

            if (hwnd == GetActiveWindow())
                return handleKeyPress (key, 0);
        }

        return false;
    }

    bool isConstrainedNativeWindow() const
    {
        return constrainer != nullptr
                && (styleFlags & (windowHasTitleBar | windowIsResizable)) == (windowHasTitleBar | windowIsResizable)
                && ! isKioskMode();
    }

    Rectangle<int> getCurrentScaledBounds() const
    {
        return detail::ScalingHelpers::unscaledScreenPosToScaled (component, windowBorder.addedTo (detail::ScalingHelpers::scaledScreenPosToUnscaled (component, component.getBounds())));
    }

    LRESULT handleSizeConstraining (RECT& r, const WPARAM wParam)
    {
        if (isConstrainedNativeWindow())
        {
            const auto logicalBounds = convertPhysicalScreenRectangleToLogical (rectangleFromRECT (r).toFloat(), hwnd);
            auto pos = detail::ScalingHelpers::unscaledScreenPosToScaled (component, logicalBounds).toNearestInt();

            const auto original = getCurrentScaledBounds();

            constrainer->checkBounds (pos, original,
                                      Desktop::getInstance().getDisplays().getTotalBounds (true),
                                      wParam == WMSZ_TOP    || wParam == WMSZ_TOPLEFT    || wParam == WMSZ_TOPRIGHT,
                                      wParam == WMSZ_LEFT   || wParam == WMSZ_TOPLEFT    || wParam == WMSZ_BOTTOMLEFT,
                                      wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT,
                                      wParam == WMSZ_RIGHT  || wParam == WMSZ_TOPRIGHT   || wParam == WMSZ_BOTTOMRIGHT);

            r = RECTFromRectangle (convertLogicalScreenRectangleToPhysical (detail::ScalingHelpers::scaledScreenPosToUnscaled (component, pos.toFloat()).toNearestInt(), hwnd));
        }

        return TRUE;
    }

    LRESULT handlePositionChanging (WINDOWPOS& wp)
    {
        if (isConstrainedNativeWindow() && ! isFullScreen())
        {
            if ((wp.flags & (SWP_NOMOVE | SWP_NOSIZE)) != (SWP_NOMOVE | SWP_NOSIZE)
                 && (wp.x > -32000 && wp.y > -32000)
                 && ! Component::isMouseButtonDownAnywhere())
            {
                const auto logicalBounds = convertPhysicalScreenRectangleToLogical (rectangleFromRECT ({ wp.x, wp.y, wp.x + wp.cx, wp.y + wp.cy }).toFloat(), hwnd);
                auto pos = detail::ScalingHelpers::unscaledScreenPosToScaled (component, logicalBounds).toNearestInt();

                const auto original = getCurrentScaledBounds();

                constrainer->checkBounds (pos, original,
                                          Desktop::getInstance().getDisplays().getTotalBounds (true),
                                          pos.getY() != original.getY() && pos.getBottom() == original.getBottom(),
                                          pos.getX() != original.getX() && pos.getRight()  == original.getRight(),
                                          pos.getY() == original.getY() && pos.getBottom() != original.getBottom(),
                                          pos.getX() == original.getX() && pos.getRight()  != original.getRight());

                auto physicalBounds = convertLogicalScreenRectangleToPhysical (detail::ScalingHelpers::scaledScreenPosToUnscaled (component, pos.toFloat()), hwnd);

                auto getNewPositionIfNotRoundingError = [] (int posIn, float newPos)
                {
                    return (std::abs ((float) posIn - newPos) >= 1.0f) ? roundToInt (newPos) : posIn;
                };

                wp.x  = getNewPositionIfNotRoundingError (wp.x,  physicalBounds.getX());
                wp.y  = getNewPositionIfNotRoundingError (wp.y,  physicalBounds.getY());
                wp.cx = getNewPositionIfNotRoundingError (wp.cx, physicalBounds.getWidth());
                wp.cy = getNewPositionIfNotRoundingError (wp.cy, physicalBounds.getHeight());
            }
        }

        if (((wp.flags & SWP_SHOWWINDOW) != 0 && ! component.isVisible()))
            component.setVisible (true);
        else if (((wp.flags & SWP_HIDEWINDOW) != 0 && component.isVisible()))
            component.setVisible (false);

        return 0;
    }

    enum class ForceRefreshDispatcher
    {
        no,
        yes
    };

    void updateCurrentMonitorAndRefreshVBlankDispatcher (ForceRefreshDispatcher force = ForceRefreshDispatcher::no)
    {
        auto monitor = MonitorFromWindow (hwnd, MONITOR_DEFAULTTONULL);

        if (std::exchange (currentMonitor, monitor) != monitor || force == ForceRefreshDispatcher::yes)
            VBlankDispatcher::getInstance()->updateDisplay (*this, currentMonitor);
    }

    bool handlePositionChanged()
    {
        auto pos = getCurrentMousePos();

        if (contains (pos.roundToInt(), false))
        {
            const ScopedValueSetter<bool> scope (inHandlePositionChanged, true);

            if (! areOtherTouchSourcesActive())
                doMouseEvent (pos, MouseInputSource::defaultPressure);

            if (! isValidPeer (this))
                return true;
        }

        handleMovedOrResized();
        updateCurrentMonitorAndRefreshVBlankDispatcher();

        return ! dontRepaint; // to allow non-accelerated openGL windows to draw themselves correctly.
    }

    //==============================================================================
    LRESULT handleDPIChanging (int newDPI, RECT newRect)
    {
        // Sometimes, windows that should not be automatically scaled (secondary windows in plugins)
        // are sent WM_DPICHANGED. The size suggested by the OS is incorrect for our unscaled
        // window, so we should ignore it.
        if (! isPerMonitorDPIAwareWindow (hwnd))
            return 0;

        const auto newScale = (double) newDPI / USER_DEFAULT_SCREEN_DPI;

        if (approximatelyEqual (scaleFactor, newScale))
            return 0;

        scaleFactor = newScale;

        {
            const ScopedValueSetter<bool> setter (inDpiChange, true);
            SetWindowPos (hwnd,
                          nullptr,
                          newRect.left,
                          newRect.top,
                          newRect.right  - newRect.left,
                          newRect.bottom - newRect.top,
                          SWP_NOZORDER | SWP_NOACTIVATE);
        }

        // This is to handle reentrancy. If responding to a DPI change triggers further DPI changes,
        // we should only notify listeners and resize windows once all of the DPI changes have
        // resolved.
        if (inDpiChange)
        {
            // Danger! Re-entrant call to handleDPIChanging.
            // Please report this issue on the JUCE forum, along with instructions
            // so that a JUCE developer can reproduce the issue.
            jassertfalse;
            return 0;
        }

        updateShadower();
        InvalidateRect (hwnd, nullptr, FALSE);

        scaleFactorListeners.call ([this] (ScaleFactorListener& l) { l.nativeScaleFactorChanged (scaleFactor); });

        return 0;
    }

    //==============================================================================
    void handleAppActivation (const WPARAM wParam)
    {
        modifiersAtLastCallback = -1;
        updateKeyModifiers();

        if (isMinimised())
        {
            component.repaint();
            handleMovedOrResized();

            if (! isValidPeer (this))
                return;
        }

        auto* underMouse = component.getComponentAt (component.getMouseXYRelative());

        if (underMouse == nullptr)
            underMouse = &component;

        if (underMouse->isCurrentlyBlockedByAnotherModalComponent())
        {
            if (LOWORD (wParam) == WA_CLICKACTIVE)
                Component::getCurrentlyModalComponent()->inputAttemptWhenModal();
            else
                ModalComponentManager::getInstance()->bringModalComponentsToFront();
        }
        else
        {
            handleBroughtToFront();
        }
    }

    void handlePowerBroadcast (WPARAM wParam)
    {
        if (auto* app = JUCEApplicationBase::getInstance())
        {
            switch (wParam)
            {
                case PBT_APMSUSPEND:                app->suspended(); break;

                case PBT_APMQUERYSUSPENDFAILED:
                case PBT_APMRESUMECRITICAL:
                case PBT_APMRESUMESUSPEND:
                case PBT_APMRESUMEAUTOMATIC:        app->resumed(); break;

                default: break;
            }
        }
    }

    void handleLeftClickInNCArea (WPARAM wParam)
    {
        if (! sendInputAttemptWhenModalMessage())
        {
            switch (wParam)
            {
            case HTBOTTOM:
            case HTBOTTOMLEFT:
            case HTBOTTOMRIGHT:
            case HTGROWBOX:
            case HTLEFT:
            case HTRIGHT:
            case HTTOP:
            case HTTOPLEFT:
            case HTTOPRIGHT:
                if (isConstrainedNativeWindow())
                {
                    constrainerIsResizing = true;
                    constrainer->resizeStart();
                }
                break;

            default:
                break;
            }
        }
    }

    void initialiseSysMenu (HMENU menu) const
    {
        if (! hasTitleBar())
        {
            if (isFullScreen())
            {
                EnableMenuItem (menu, SC_RESTORE,  MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem (menu, SC_MOVE,     MF_BYCOMMAND | MF_GRAYED);
            }
            else if (! isMinimised())
            {
                EnableMenuItem (menu, SC_MAXIMIZE, MF_BYCOMMAND | MF_GRAYED);
            }
        }
    }

    void doSettingChange()
    {
        forceDisplayUpdate();

        if (fullScreen && ! isMinimised())
            setWindowPos (hwnd, detail::ScalingHelpers::scaledScreenPosToUnscaled (component, Desktop::getInstance().getDisplays()
                                                                                              .getDisplayForRect (component.getScreenBounds())->userArea),
                          SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOSENDCHANGING);

        auto* dispatcher = VBlankDispatcher::getInstance();
        dispatcher->reconfigureDisplays();
        updateCurrentMonitorAndRefreshVBlankDispatcher (ForceRefreshDispatcher::yes);
    }

    //==============================================================================
   #if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
    void setModifierKeyProvider (ModifierKeyProvider* provider) override
    {
        modProvider = provider;
    }

    void removeModifierKeyProvider() override
    {
        modProvider = nullptr;
    }
   #endif

public:
    static LRESULT CALLBACK windowProc (HWND h, UINT message, WPARAM wParam, LPARAM lParam)
    {
        // Ensure that non-client areas are scaled for per-monitor DPI awareness v1 - can't
        // do this in peerWindowProc as we have no window at this point
        if (message == WM_NCCREATE)
            NullCheckedInvocation::invoke (enableNonClientDPIScaling, h);

        if (auto* peer = getOwnerOfWindow (h))
        {
            jassert (isValidPeer (peer));
            return peer->peerWindowProc (h, message, wParam, lParam);
        }

        return DefWindowProcW (h, message, wParam, lParam);
    }

private:
    static void* callFunctionIfNotLocked (MessageCallbackFunction* callback, void* userData)
    {
        auto& mm = *MessageManager::getInstance();

        if (mm.currentThreadHasLockedMessageManager())
            return callback (userData);

        return mm.callFunctionOnMessageThread (callback, userData);
    }

    static POINT getPOINTFromLParam (LPARAM lParam) noexcept
    {
        return { GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam) };
    }

    Point<float> getPointFromLocalLParam (LPARAM lParam) noexcept
    {
        auto p = pointFromPOINT (getPOINTFromLParam (lParam));

        if (isPerMonitorDPIAwareWindow (hwnd))
        {
            // LPARAM is relative to this window's top-left but may be on a different monitor so we need to calculate the
            // physical screen position and then convert this to local logical coordinates
            auto r = getWindowScreenRect (hwnd);
            return globalToLocal (Desktop::getInstance().getDisplays().physicalToLogical (pointFromPOINT ({ r.left + p.x + roundToInt (windowBorder.getLeft() * scaleFactor),
                                                                                                            r.top  + p.y + roundToInt (windowBorder.getTop()  * scaleFactor) })).toFloat());
        }

        return p.toFloat();
    }

    Point<float> getCurrentMousePos() noexcept
    {
        return globalToLocal (convertPhysicalScreenPointToLogical (pointFromPOINT (getPOINTFromLParam ((LPARAM) GetMessagePos())), hwnd).toFloat());
    }

    LRESULT peerWindowProc (HWND h, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
            //==============================================================================
            case WM_NCHITTEST:
                if ((styleFlags & windowIgnoresMouseClicks) != 0)
                    return HTTRANSPARENT;

                if (! hasTitleBar())
                    return HTCLIENT;

                break;

            //==============================================================================
            case WM_PAINT:
                handlePaintMessage();
                return 0;

            case WM_NCPAINT:
                handlePaintMessage(); // this must be done, even with native titlebars, or there are rendering artifacts.

                if (hasTitleBar())
                    break; // let the DefWindowProc handle drawing the frame.

                return 0;

            case WM_ERASEBKGND:
            case WM_NCCALCSIZE:
                if (hasTitleBar())
                    break;

                return 1;

            //==============================================================================
            case WM_POINTERUPDATE:
                if (handlePointerInput (wParam, lParam, false, false))
                    return 0;
                break;

            case WM_POINTERDOWN:
                if (handlePointerInput (wParam, lParam, true, false))
                    return 0;
                break;

            case WM_POINTERUP:
                if (handlePointerInput (wParam, lParam, false, true))
                    return 0;
                break;

            //==============================================================================
            case WM_MOUSEMOVE:          doMouseMove (getPointFromLocalLParam (lParam), false); return 0;

            case WM_POINTERLEAVE:
            case WM_MOUSELEAVE:         doMouseExit(); return 0;

            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:        doMouseDown (getPointFromLocalLParam (lParam), wParam); return 0;

            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:          doMouseUp (getPointFromLocalLParam (lParam), wParam); return 0;

            case WM_POINTERWHEEL:
            case 0x020A: /* WM_MOUSEWHEEL */   doMouseWheel (wParam, true);  return 0;

            case WM_POINTERHWHEEL:
            case 0x020E: /* WM_MOUSEHWHEEL */  doMouseWheel (wParam, false); return 0;

            case WM_CAPTURECHANGED:     doCaptureChanged(); return 0;

            case WM_NCPOINTERUPDATE:
            case WM_NCMOUSEMOVE:
                if (hasTitleBar())
                    break;

                return 0;

            case WM_TOUCH:
                if (getTouchInputInfo != nullptr)
                    return doTouchEvent ((int) wParam, (HTOUCHINPUT) lParam);

                break;

            case 0x119: /* WM_GESTURE */
                if (doGestureEvent (lParam))
                    return 0;

                break;

            //==============================================================================
            case WM_SIZING:                  return handleSizeConstraining (*(RECT*) lParam, wParam);
            case WM_WINDOWPOSCHANGING:       return handlePositionChanging (*(WINDOWPOS*) lParam);
            case 0x2e0: /* WM_DPICHANGED */  return handleDPIChanging ((int) HIWORD (wParam), *(RECT*) lParam);

            case WM_WINDOWPOSCHANGED:
            {
                const WINDOWPOS& wPos = *reinterpret_cast<WINDOWPOS*> (lParam);

                if ((wPos.flags & SWP_NOMOVE) != 0 && (wPos.flags & SWP_NOSIZE) != 0)
                    startTimer (100);
                else
                    if (handlePositionChanged())
                        return 0;
            }
            break;

            //==============================================================================
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                if (doKeyDown (wParam))
                    return 0;

                forwardMessageToParent (message, wParam, lParam);
                break;

            case WM_KEYUP:
            case WM_SYSKEYUP:
                if (doKeyUp (wParam))
                    return 0;

                forwardMessageToParent (message, wParam, lParam);
                break;

            case WM_CHAR:
                if (doKeyChar ((int) wParam, lParam))
                    return 0;

                forwardMessageToParent (message, wParam, lParam);
                break;

            case WM_APPCOMMAND:
                if (doAppCommand (lParam))
                    return TRUE;

                break;

            case WM_MENUCHAR: // triggered when alt+something is pressed
                return MNC_CLOSE << 16; // (avoids making the default system beep)

            //==============================================================================
            case WM_SETFOCUS:
                /*  When the HWND receives Focus from the system it sends a
                    UIA_AutomationFocusChangedEventId notification redirecting the focus to the HWND
                    itself. This is a built-in behaviour of the HWND.

                    This means that whichever JUCE managed provider was active before the entire
                    window lost and then regained the focus, loses its focused state, and the
                    window's root element will become focused under which all JUCE managed providers
                    can be found.

                    This needs to be reflected on currentlyFocusedHandler so that the JUCE
                    accessibility mechanisms can detect that the root window got the focus, and send
                    another FocusChanged event to the system to redirect focus to a JUCE managed
                    provider if necessary.
                */
                AccessibilityHandler::clearCurrentlyFocusedHandler();
                updateKeyModifiers();
                handleFocusGain();
                break;

            case WM_KILLFOCUS:
                if (hasCreatedCaret)
                {
                    hasCreatedCaret = false;
                    DestroyCaret();
                }

                handleFocusLoss();

                if (auto* modal = Component::getCurrentlyModalComponent())
                    if (auto* peer = modal->getPeer())
                        if ((peer->getStyleFlags() & ComponentPeer::windowIsTemporary) != 0)
                            sendInputAttemptWhenModalMessage();

                break;

            case WM_ACTIVATEAPP:
                // Windows does weird things to process priority when you swap apps,
                // so this forces an update when the app is brought to the front
                if (wParam != FALSE)
                    juce_repeatLastProcessPriority();
                else
                    Desktop::getInstance().setKioskModeComponent (nullptr); // turn kiosk mode off if we lose focus

                detail::TopLevelWindowManager::checkCurrentlyFocusedTopLevelWindow();
                modifiersAtLastCallback = -1;
                return 0;

            case WM_ACTIVATE:
                if (LOWORD (wParam) == WA_ACTIVE || LOWORD (wParam) == WA_CLICKACTIVE)
                {
                    handleAppActivation (wParam);
                    return 0;
                }

                break;

            case WM_NCACTIVATE:
                // while a temporary window is being shown, prevent Windows from deactivating the
                // title bars of our main windows.
                if (wParam == 0 && ! shouldDeactivateTitleBar)
                    wParam = TRUE; // change this and let it get passed to the DefWindowProc.

                break;

            case WM_POINTERACTIVATE:
            case WM_MOUSEACTIVATE:
                if (! component.getMouseClickGrabsKeyboardFocus())
                    return MA_NOACTIVATE;

                break;

            case WM_SHOWWINDOW:
                if (wParam != 0)
                {
                    component.setVisible (true);
                    handleBroughtToFront();
                }

                break;

            case WM_CLOSE:
                if (! component.isCurrentlyBlockedByAnotherModalComponent())
                    handleUserClosingWindow();

                return 0;

           #if JUCE_REMOVE_COMPONENT_FROM_DESKTOP_ON_WM_DESTROY
            case WM_DESTROY:
                getComponent().removeFromDesktop();
                return 0;
           #endif

            case WM_QUERYENDSESSION:
                if (auto* app = JUCEApplicationBase::getInstance())
                {
                    app->systemRequestedQuit();
                    return MessageManager::getInstance()->hasStopMessageBeenSent();
                }
                return TRUE;

            case WM_POWERBROADCAST:
                handlePowerBroadcast (wParam);
                break;

            case WM_SYNCPAINT:
                return 0;

            case WM_DISPLAYCHANGE:
                InvalidateRect (h, nullptr, 0);
                // intentional fall-through...
                JUCE_FALLTHROUGH
            case WM_SETTINGCHANGE:  // note the fall-through in the previous case!
                doSettingChange();
                break;

            case WM_INITMENU:
                initialiseSysMenu ((HMENU) wParam);
                break;

            case WM_SYSCOMMAND:
                switch (wParam & 0xfff0)
                {
                case SC_CLOSE:
                    if (sendInputAttemptWhenModalMessage())
                        return 0;

                    if (hasTitleBar())
                    {
                        PostMessage (h, WM_CLOSE, 0, 0);
                        return 0;
                    }
                    break;

                case SC_KEYMENU:
                   #if ! JUCE_WINDOWS_ALT_KEY_TRIGGERS_MENU
                    // This test prevents a press of the ALT key from triggering the ancient top-left window menu.
                    // By default we suppress this behaviour because it's unlikely that more than a tiny subset of
                    // our users will actually want it, and it causes problems if you're trying to use the ALT key
                    // as a modifier for mouse actions. If you really need the old behaviour, then just define
                    // JUCE_WINDOWS_ALT_KEY_TRIGGERS_MENU=1 in your app.
                    if ((lParam >> 16) <= 0) // Values above zero indicate that a mouse-click triggered the menu
                        return 0;
                   #endif

                    // (NB mustn't call sendInputAttemptWhenModalMessage() here because of very obscure
                    // situations that can arise if a modal loop is started from an alt-key keypress).
                    if (hasTitleBar() && h == GetCapture())
                        ReleaseCapture();

                    break;

                case SC_MAXIMIZE:
                    if (! sendInputAttemptWhenModalMessage())
                        setFullScreen (true);

                    return 0;

                case SC_MINIMIZE:
                    if (sendInputAttemptWhenModalMessage())
                        return 0;

                    if (! hasTitleBar())
                    {
                        setMinimised (true);
                        return 0;
                    }
                    break;

                case SC_RESTORE:
                    if (sendInputAttemptWhenModalMessage())
                        return 0;

                    if (hasTitleBar())
                    {
                        if (isFullScreen())
                        {
                            setFullScreen (false);
                            return 0;
                        }
                    }
                    else
                    {
                        if (isMinimised())
                            setMinimised (false);
                        else if (isFullScreen())
                            setFullScreen (false);

                        return 0;
                    }
                    break;
                }

                break;

            case WM_NCPOINTERDOWN:
            case WM_NCLBUTTONDOWN:
                handleLeftClickInNCArea (wParam);
                break;

            case WM_NCRBUTTONDOWN:
            case WM_NCMBUTTONDOWN:
                sendInputAttemptWhenModalMessage();
                break;

            case WM_IME_SETCONTEXT:
                imeHandler.handleSetContext (h, wParam == TRUE);
                lParam &= ~(LPARAM) ISC_SHOWUICOMPOSITIONWINDOW;
                return ImmIsUIMessage (h, message, wParam, lParam);

            case WM_IME_STARTCOMPOSITION:  imeHandler.handleStartComposition (*this); return 0;
            case WM_IME_ENDCOMPOSITION:    imeHandler.handleEndComposition (*this, h); return 0;
            case WM_IME_COMPOSITION:       imeHandler.handleComposition (*this, h, lParam); return 0;

            case WM_GETDLGCODE:
                return DLGC_WANTALLKEYS;

            case WM_GETOBJECT:
            {
                if (static_cast<long> (lParam) == WindowsAccessibility::getUiaRootObjectId())
                {
                    if (auto* handler = component.getAccessibilityHandler())
                    {
                        LRESULT res = 0;

                        if (WindowsAccessibility::handleWmGetObject (handler, wParam, lParam, &res))
                        {
                            isAccessibilityActive = true;
                            return res;
                        }
                    }
                }

                break;
            }
            default:
                break;
        }

        return DefWindowProcW (h, message, wParam, lParam);
    }

    bool sendInputAttemptWhenModalMessage()
    {
        if (! component.isCurrentlyBlockedByAnotherModalComponent())
            return false;

        if (auto* current = Component::getCurrentlyModalComponent())
            if (auto* owner = getOwnerOfWindow ((HWND) current->getWindowHandle()))
                if (! owner->shouldIgnoreModalDismiss)
                    current->inputAttemptWhenModal();

        return true;
    }

    //==============================================================================
    struct IMEHandler
    {
        IMEHandler()
        {
            reset();
        }

        void handleSetContext (HWND hWnd, const bool windowIsActive)
        {
            if (compositionInProgress && ! windowIsActive)
            {
                if (HIMC hImc = ImmGetContext (hWnd))
                {
                    ImmNotifyIME (hImc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
                    ImmReleaseContext (hWnd, hImc);
                }

                // If the composition is still in progress, calling ImmNotifyIME may call back
                // into handleComposition to let us know that the composition has finished.
                // We need to set compositionInProgress *after* calling handleComposition, so that
                // the text replaces the current selection, rather than being inserted after the
                // caret.
                compositionInProgress = false;
            }
        }

        void handleStartComposition (ComponentPeer& owner)
        {
            reset();

            if (auto* target = owner.findCurrentTextInputTarget())
                target->insertTextAtCaret (String());
        }

        void handleEndComposition (ComponentPeer& owner, HWND hWnd)
        {
            if (compositionInProgress)
            {
                // If this occurs, the user has cancelled the composition, so clear their changes..
                if (auto* target = owner.findCurrentTextInputTarget())
                {
                    target->setHighlightedRegion (compositionRange);
                    target->insertTextAtCaret (String());
                    compositionRange.setLength (0);

                    target->setHighlightedRegion (Range<int>::emptyRange (compositionRange.getEnd()));
                    target->setTemporaryUnderlining ({});
                }

                if (auto hImc = ImmGetContext (hWnd))
                {
                    ImmNotifyIME (hImc, NI_CLOSECANDIDATE, 0, 0);
                    ImmReleaseContext (hWnd, hImc);
                }
            }

            reset();
        }

        void handleComposition (ComponentPeer& owner, HWND hWnd, const LPARAM lParam)
        {
            if (auto* target = owner.findCurrentTextInputTarget())
            {
                if (auto hImc = ImmGetContext (hWnd))
                {
                    if (compositionRange.getStart() < 0)
                        compositionRange = Range<int>::emptyRange (target->getHighlightedRegion().getStart());

                    if ((lParam & GCS_RESULTSTR) != 0) // (composition has finished)
                    {
                        replaceCurrentSelection (target, getCompositionString (hImc, GCS_RESULTSTR),
                                                 Range<int>::emptyRange (-1));

                        reset();
                        target->setTemporaryUnderlining ({});
                    }
                    else if ((lParam & GCS_COMPSTR) != 0) // (composition is still in-progress)
                    {
                        replaceCurrentSelection (target, getCompositionString (hImc, GCS_COMPSTR),
                                                 getCompositionSelection (hImc, lParam));

                        target->setTemporaryUnderlining (getCompositionUnderlines (hImc, lParam));
                        compositionInProgress = true;
                    }

                    moveCandidateWindowToLeftAlignWithSelection (hImc, owner, target);
                    ImmReleaseContext (hWnd, hImc);
                }
            }
        }

    private:
        //==============================================================================
        Range<int> compositionRange; // The range being modified in the TextInputTarget
        bool compositionInProgress;

        //==============================================================================
        void reset()
        {
            compositionRange = Range<int>::emptyRange (-1);
            compositionInProgress = false;
        }

        String getCompositionString (HIMC hImc, const DWORD type) const
        {
            jassert (hImc != HIMC{});

            const auto stringSizeBytes = ImmGetCompositionString (hImc, type, nullptr, 0);

            if (stringSizeBytes > 0)
            {
                HeapBlock<TCHAR> buffer;
                buffer.jcalloc ((size_t) stringSizeBytes / sizeof (TCHAR) + 1);
                ImmGetCompositionString (hImc, type, buffer, (DWORD) stringSizeBytes);
                return String (buffer.get());
            }

            return {};
        }

        int getCompositionCaretPos (HIMC hImc, LPARAM lParam, const String& currentIMEString) const
        {
            jassert (hImc != HIMC{});

            if ((lParam & CS_NOMOVECARET) != 0)
                return compositionRange.getStart();

            if ((lParam & GCS_CURSORPOS) != 0)
            {
                const int localCaretPos = ImmGetCompositionString (hImc, GCS_CURSORPOS, nullptr, 0);
                return compositionRange.getStart() + jmax (0, localCaretPos);
            }

            return compositionRange.getStart() + currentIMEString.length();
        }

        // Get selected/highlighted range while doing composition:
        // returned range is relative to beginning of TextInputTarget, not composition string
        Range<int> getCompositionSelection (HIMC hImc, LPARAM lParam) const
        {
            jassert (hImc != HIMC{});
            int selectionStart = 0;
            int selectionEnd = 0;

            if ((lParam & GCS_COMPATTR) != 0)
            {
                // Get size of attributes array:
                const int attributeSizeBytes = ImmGetCompositionString (hImc, GCS_COMPATTR, nullptr, 0);

                if (attributeSizeBytes > 0)
                {
                    // Get attributes (8 bit flag per character):
                    HeapBlock<char> attributes (attributeSizeBytes);
                    ImmGetCompositionString (hImc, GCS_COMPATTR, attributes, (DWORD) attributeSizeBytes);

                    selectionStart = 0;

                    for (selectionStart = 0; selectionStart < attributeSizeBytes; ++selectionStart)
                        if (attributes[selectionStart] == ATTR_TARGET_CONVERTED || attributes[selectionStart] == ATTR_TARGET_NOTCONVERTED)
                            break;

                    for (selectionEnd = selectionStart; selectionEnd < attributeSizeBytes; ++selectionEnd)
                        if (attributes[selectionEnd] != ATTR_TARGET_CONVERTED && attributes[selectionEnd] != ATTR_TARGET_NOTCONVERTED)
                            break;
                }
            }

            return Range<int> (selectionStart, selectionEnd) + compositionRange.getStart();
        }

        void replaceCurrentSelection (TextInputTarget* const target, const String& newContent, Range<int> newSelection)
        {
            if (compositionInProgress)
                target->setHighlightedRegion (compositionRange);

            target->insertTextAtCaret (newContent);
            compositionRange.setLength (newContent.length());

            if (newSelection.getStart() < 0)
                newSelection = Range<int>::emptyRange (compositionRange.getEnd());

            target->setHighlightedRegion (newSelection);
        }

        Array<Range<int>> getCompositionUnderlines (HIMC hImc, LPARAM lParam) const
        {
            Array<Range<int>> result;

            if (hImc != HIMC{} && (lParam & GCS_COMPCLAUSE) != 0)
            {
                auto clauseDataSizeBytes = ImmGetCompositionString (hImc, GCS_COMPCLAUSE, nullptr, 0);

                if (clauseDataSizeBytes > 0)
                {
                    const auto numItems = (size_t) clauseDataSizeBytes / sizeof (uint32);
                    HeapBlock<uint32> clauseData (numItems);

                    if (ImmGetCompositionString (hImc, GCS_COMPCLAUSE, clauseData, (DWORD) clauseDataSizeBytes) > 0)
                        for (size_t i = 0; i + 1 < numItems; ++i)
                            result.add (Range<int> ((int) clauseData[i], (int) clauseData[i + 1]) + compositionRange.getStart());
                }
            }

            return result;
        }

        void moveCandidateWindowToLeftAlignWithSelection (HIMC hImc, ComponentPeer& peer, TextInputTarget* target) const
        {
            if (auto* targetComp = dynamic_cast<Component*> (target))
            {
                auto area = peer.getComponent().getLocalArea (targetComp, target->getCaretRectangle());

                CANDIDATEFORM pos = { 0, CFS_CANDIDATEPOS, { area.getX(), area.getBottom() }, { 0, 0, 0, 0 } };
                ImmSetCandidateWindow (hImc, &pos);
            }
        }

        JUCE_DECLARE_NON_COPYABLE (IMEHandler)
    };

    void timerCallback() override
    {
        handlePositionChanged();
        stopTimer();
    }

    static bool isAncestor (HWND outer, HWND inner)
    {
        if (outer == nullptr || inner == nullptr)
            return false;

        if (outer == inner)
            return true;

        return isAncestor (outer, GetAncestor (inner, GA_PARENT));
    }

    void windowShouldDismissModals (HWND originator)
    {
        if (shouldIgnoreModalDismiss)
            return;

        if (isAncestor (originator, hwnd))
            sendInputAttemptWhenModalMessage();
    }

    // Unfortunately SetWindowsHookEx only allows us to register a static function as a hook.
    // To get around this, we keep a static list of listeners which are interested in
    // top-level window events, and notify all of these listeners from the callback.
    class TopLevelModalDismissBroadcaster
    {
    public:
        TopLevelModalDismissBroadcaster()
            : hook (SetWindowsHookEx (WH_CALLWNDPROC,
                                      callWndProc,
                                      (HINSTANCE) juce::Process::getCurrentModuleInstanceHandle(),
                                      GetCurrentThreadId()))
        {}

        ~TopLevelModalDismissBroadcaster() noexcept
        {
            UnhookWindowsHookEx (hook);
        }

    private:
        static void processMessage (int nCode, const CWPSTRUCT* info)
        {
            if (nCode < 0 || info == nullptr)
                return;

            constexpr UINT events[] { WM_MOVE,
                                      WM_SIZE,
                                      WM_WINDOWPOSCHANGING,
                                      WM_NCPOINTERDOWN,
                                      WM_NCLBUTTONDOWN,
                                      WM_NCRBUTTONDOWN,
                                      WM_NCMBUTTONDOWN };

            if (std::find (std::begin (events), std::end (events), info->message) == std::end (events))
                return;

            if (info->message == WM_WINDOWPOSCHANGING)
            {
                const auto* windowPos = reinterpret_cast<const WINDOWPOS*> (info->lParam);
                const auto windowPosFlags = windowPos->flags;

                constexpr auto maskToCheck = SWP_NOMOVE | SWP_NOSIZE;

                if ((windowPosFlags & maskToCheck) == maskToCheck)
                    return;
            }

            // windowMayDismissModals could affect the number of active ComponentPeer instances
            for (auto i = ComponentPeer::getNumPeers(); --i >= 0;)
                if (i < ComponentPeer::getNumPeers())
                    if (auto* hwndPeer = dynamic_cast<HWNDComponentPeer*> (ComponentPeer::getPeer (i)))
                        hwndPeer->windowShouldDismissModals (info->hwnd);
        }

        static LRESULT CALLBACK callWndProc (int nCode, WPARAM wParam, LPARAM lParam)
        {
            processMessage (nCode, reinterpret_cast<CWPSTRUCT*> (lParam));
            return CallNextHookEx ({}, nCode, wParam, lParam);
        }

        HHOOK hook;
    };

    SharedResourcePointer<TopLevelModalDismissBroadcaster> modalDismissBroadcaster;
    IMEHandler imeHandler;
    bool shouldIgnoreModalDismiss = false;

    RectangleList<int> deferredRepaints;
    ScopedSuspendResumeNotificationRegistration suspendResumeRegistration;
    std::optional<TimedCallback> monitorUpdateTimer;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HWNDComponentPeer)
};

MultiTouchMapper<DWORD> HWNDComponentPeer::currentTouches;
ModifierKeys HWNDComponentPeer::modifiersAtLastCallback;

ComponentPeer* Component::createNewPeer (int styleFlags, void* parentHWND)
{
    return new HWNDComponentPeer (*this, styleFlags, (HWND) parentHWND, false);
}

JUCE_API ComponentPeer* createNonRepaintingEmbeddedWindowsPeer (Component& component, void* parentHWND);
JUCE_API ComponentPeer* createNonRepaintingEmbeddedWindowsPeer (Component& component, void* parentHWND)
{
    return new HWNDComponentPeer (component, ComponentPeer::windowIgnoresMouseClicks,
                                  (HWND) parentHWND, true);
}

JUCE_IMPLEMENT_SINGLETON (HWNDComponentPeer::WindowClassHolder)

//==============================================================================
bool KeyPress::isKeyCurrentlyDown (const int keyCode)
{
    const auto k = [&]
    {
        if ((keyCode & extendedKeyModifier) != 0)
            return keyCode & (extendedKeyModifier - 1);

        const auto vk = BYTE (VkKeyScan ((WCHAR) keyCode) & 0xff);
        return vk != (BYTE) -1 ? vk : keyCode;
    }();

    return HWNDComponentPeer::isKeyDown (k);
}

//==============================================================================
static DWORD getProcess (HWND hwnd)
{
    DWORD result = 0;
    GetWindowThreadProcessId (hwnd, &result);
    return result;
}

/*  Returns true if the viewComponent is embedded into a window
    owned by the foreground process.
*/
bool detail::WindowingHelpers::isEmbeddedInForegroundProcess (Component* c)
{
    if (c == nullptr)
        return false;

    auto* peer = c->getPeer();
    auto* hwnd = peer != nullptr ? static_cast<HWND> (peer->getNativeHandle()) : nullptr;

    if (hwnd == nullptr)
        return true;

    const auto fgProcess    = getProcess (GetForegroundWindow());
    const auto ownerProcess = getProcess (GetAncestor (hwnd, GA_ROOTOWNER));
    return fgProcess == ownerProcess;
}

bool JUCE_CALLTYPE Process::isForegroundProcess()
{
    if (auto fg = GetForegroundWindow())
        return getProcess (fg) == GetCurrentProcessId();

    return true;
}

// N/A on Windows as far as I know.
void JUCE_CALLTYPE Process::makeForegroundProcess() {}
void JUCE_CALLTYPE Process::hide() {}

//==============================================================================
bool detail::MouseInputSourceList::addSource()
{
    auto numSources = sources.size();

    if (numSources == 0 || canUseMultiTouch())
    {
        addSource (numSources, numSources == 0 ? MouseInputSource::InputSourceType::mouse
                                               : MouseInputSource::InputSourceType::touch);
        return true;
    }

    return false;
}

bool detail::MouseInputSourceList::canUseTouch() const
{
    return canUseMultiTouch();
}

Point<float> MouseInputSource::getCurrentRawMousePosition()
{
    POINT mousePos;
    GetCursorPos (&mousePos);

    auto p = pointFromPOINT (mousePos);

    if (isPerMonitorDPIAwareThread())
        p = Desktop::getInstance().getDisplays().physicalToLogical (p);

    return p.toFloat();
}

void MouseInputSource::setRawMousePosition (Point<float> newPosition)
{
    auto newPositionInt = newPosition.roundToInt();

   #if JUCE_WIN_PER_MONITOR_DPI_AWARE
    if (isPerMonitorDPIAwareThread())
        newPositionInt = Desktop::getInstance().getDisplays().logicalToPhysical (newPositionInt);
   #endif

    auto point = POINTFromPoint (newPositionInt);
    SetCursorPos (point.x, point.y);
}

//==============================================================================
class ScreenSaverDefeater final : public Timer
{
public:
    ScreenSaverDefeater()
    {
        startTimer (10000);
        timerCallback();
    }

    void timerCallback() override
    {
        if (Process::isForegroundProcess())
        {
            INPUT input = {};
            input.type = INPUT_MOUSE;
            input.mi.mouseData = MOUSEEVENTF_MOVE;

            SendInput (1, &input, sizeof (INPUT));
        }
    }
};

static std::unique_ptr<ScreenSaverDefeater> screenSaverDefeater;

void Desktop::setScreenSaverEnabled (const bool isEnabled)
{
    if (isEnabled)
        screenSaverDefeater = nullptr;
    else if (screenSaverDefeater == nullptr)
        screenSaverDefeater.reset (new ScreenSaverDefeater());
}

bool Desktop::isScreenSaverEnabled()
{
    return screenSaverDefeater == nullptr;
}

//==============================================================================
void LookAndFeel::playAlertSound()
{
    MessageBeep (MB_OK);
}

//==============================================================================
void SystemClipboard::copyTextToClipboard (const String& text)
{
    if (OpenClipboard (nullptr) != 0)
    {
        if (EmptyClipboard() != 0)
        {
            auto bytesNeeded = CharPointer_UTF16::getBytesRequiredFor (text.getCharPointer()) + 4;

            if (bytesNeeded > 0)
            {
                if (auto bufH = GlobalAlloc (GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, bytesNeeded + sizeof (WCHAR)))
                {
                    if (auto* data = static_cast<WCHAR*> (GlobalLock (bufH)))
                    {
                        text.copyToUTF16 (data, bytesNeeded);
                        GlobalUnlock (bufH);

                        SetClipboardData (CF_UNICODETEXT, bufH);
                    }
                }
            }
        }

        CloseClipboard();
    }
}

String SystemClipboard::getTextFromClipboard()
{
    String result;

    if (OpenClipboard (nullptr) != 0)
    {
        if (auto bufH = GetClipboardData (CF_UNICODETEXT))
        {
            if (auto* data = (const WCHAR*) GlobalLock (bufH))
            {
                result = String (data, (size_t) (GlobalSize (bufH) / sizeof (WCHAR)));
                GlobalUnlock (bufH);
            }
        }

        CloseClipboard();
    }

    return result;
}

//==============================================================================
void Desktop::setKioskComponent (Component* kioskModeComp, bool enableOrDisable, bool /*allowMenusAndBars*/)
{
    if (auto* tlw = dynamic_cast<TopLevelWindow*> (kioskModeComp))
        tlw->setUsingNativeTitleBar (! enableOrDisable);

    if (kioskModeComp != nullptr && enableOrDisable)
        kioskModeComp->setBounds (getDisplays().getDisplayForRect (kioskModeComp->getScreenBounds())->totalArea);
}

void Desktop::allowedOrientationsChanged() {}

//==============================================================================
static const Displays::Display* getCurrentDisplayFromScaleFactor (HWND hwnd)
{
    Array<const Displays::Display*> candidateDisplays;

    const auto scaleToLookFor = [&]
    {
        if (auto* peer = HWNDComponentPeer::getOwnerOfWindow (hwnd))
            return peer->getPlatformScaleFactor();

        return getScaleFactorForWindow (hwnd);
    }();

    auto globalScale = Desktop::getInstance().getGlobalScaleFactor();

    for (auto& d : Desktop::getInstance().getDisplays().displays)
        if (approximatelyEqual (d.scale / globalScale, scaleToLookFor))
            candidateDisplays.add (&d);

    if (candidateDisplays.size() > 0)
    {
        if (candidateDisplays.size() == 1)
            return candidateDisplays[0];

        const auto bounds = [&]
        {
            if (auto* peer = HWNDComponentPeer::getOwnerOfWindow (hwnd))
                return peer->getComponent().getTopLevelComponent()->getBounds();

            return Desktop::getInstance().getDisplays().physicalToLogical (rectangleFromRECT (getWindowScreenRect (hwnd)));
        }();

        const Displays::Display* retVal = nullptr;
        int maxArea = -1;

        for (auto* d : candidateDisplays)
        {
            auto intersection = d->totalArea.getIntersection (bounds);
            auto area = intersection.getWidth() * intersection.getHeight();

            if (area > maxArea)
            {
                maxArea = area;
                retVal = d;
            }
        }

        if (retVal != nullptr)
            return retVal;
    }

    return Desktop::getInstance().getDisplays().getPrimaryDisplay();
}

//==============================================================================
struct MonitorInfo
{
    MonitorInfo (bool main, RECT totalArea, RECT workArea, double d) noexcept
        : isMain (main),
          totalAreaRect (totalArea),
          workAreaRect (workArea),
          dpi (d)
    {
    }

    bool isMain;
    RECT totalAreaRect, workAreaRect;
    double dpi;
};

static BOOL CALLBACK enumMonitorsProc (HMONITOR hm, HDC, LPRECT, LPARAM userInfo)
{
    MONITORINFO info = {};
    info.cbSize = sizeof (info);
    GetMonitorInfo (hm, &info);

    auto isMain = (info.dwFlags & 1 /* MONITORINFOF_PRIMARY */) != 0;
    auto dpi = 0.0;

    if (getDPIForMonitor != nullptr)
    {
        UINT dpiX = 0, dpiY = 0;

        if (SUCCEEDED (getDPIForMonitor (hm, MDT_Default, &dpiX, &dpiY)))
            dpi = (dpiX + dpiY) / 2.0;
    }

    ((Array<MonitorInfo>*) userInfo)->add ({ isMain, info.rcMonitor, info.rcWork, dpi });
    return TRUE;
}

void Displays::findDisplays (float masterScale)
{
    setDPIAwareness();

    Array<MonitorInfo> monitors;
    EnumDisplayMonitors (nullptr, nullptr, &enumMonitorsProc, (LPARAM) &monitors);

    auto globalDPI = getGlobalDPI();

    if (monitors.size() == 0)
    {
        auto windowRect = getWindowScreenRect (GetDesktopWindow());
        monitors.add ({ true, windowRect, windowRect, globalDPI });
    }

    // make sure the first in the list is the main monitor
    for (int i = 1; i < monitors.size(); ++i)
        if (monitors.getReference (i).isMain)
            monitors.swap (i, 0);

    for (auto& monitor : monitors)
    {
        Display d;

        d.isMain = monitor.isMain;
        d.dpi = monitor.dpi;

        if (approximatelyEqual (d.dpi, 0.0))
        {
            d.dpi = globalDPI;
            d.scale = masterScale;
        }
        else
        {
            d.scale = (d.dpi / USER_DEFAULT_SCREEN_DPI) * (masterScale / Desktop::getDefaultMasterScale());
        }

        d.totalArea = rectangleFromRECT (monitor.totalAreaRect);
        d.userArea  = rectangleFromRECT (monitor.workAreaRect);

        displays.add (d);
    }

   #if JUCE_WIN_PER_MONITOR_DPI_AWARE
    if (isPerMonitorDPIAwareThread())
        updateToLogical();
    else
   #endif
    {
        for (auto& d : displays)
        {
            d.totalArea /= masterScale;
            d.userArea  /= masterScale;
        }
    }
}

//==============================================================================
static auto extractFileHICON (const File& file)
{
    WORD iconNum = 0;
    WCHAR name[MAX_PATH * 2];
    file.getFullPathName().copyToUTF16 (name, sizeof (name));

    return IconConverters::IconPtr { ExtractAssociatedIcon ((HINSTANCE) Process::getCurrentModuleInstanceHandle(),
                                                            name,
                                                            &iconNum) };
}

Image detail::WindowingHelpers::createIconForFile (const File& file)
{
    if (const auto icon = extractFileHICON (file))
        return IconConverters::createImageFromHICON (icon.get());

    return {};
}

//==============================================================================
class MouseCursor::PlatformSpecificHandle
{
public:
    explicit PlatformSpecificHandle (const MouseCursor::StandardCursorType type)
        : impl (makeHandle (type)) {}

    explicit PlatformSpecificHandle (const detail::CustomMouseCursorInfo& info)
        : impl (makeHandle (info)) {}

    static void showInWindow (PlatformSpecificHandle* handle, ComponentPeer* peer)
    {
        SetCursor ([&]
        {
            if (handle != nullptr && handle->impl != nullptr && peer != nullptr)
                return handle->impl->getCursor (*peer);

            return LoadCursor (nullptr, IDC_ARROW);
        }());
    }

private:
    struct Impl
    {
        virtual ~Impl() = default;
        virtual HCURSOR getCursor (ComponentPeer&) = 0;
    };

    class BuiltinImpl : public Impl
    {
    public:
        explicit BuiltinImpl (HCURSOR cursorIn)
            : cursor (cursorIn) {}

        HCURSOR getCursor (ComponentPeer&) override { return cursor; }

    private:
        HCURSOR cursor;
    };

    class ImageImpl : public Impl
    {
    public:
        explicit ImageImpl (const detail::CustomMouseCursorInfo& infoIn) : info (infoIn) {}

        HCURSOR getCursor (ComponentPeer& peer) override
        {
            JUCE_ASSERT_MESSAGE_THREAD;

            static auto getCursorSize = getCursorSizeForPeerFunction();

            const auto size = getCursorSize (peer);
            const auto iter = cursorsBySize.find (size);

            if (iter != cursorsBySize.end())
                return iter->second.get();

            const auto logicalSize = info.image.getScaledBounds();
            const auto scale = (float) size / (float) unityCursorSize;
            const auto physicalSize = logicalSize * scale;

            const auto& image = info.image.getImage();
            const auto rescaled = image.rescaled (roundToInt ((float) physicalSize.getWidth()),
                                                  roundToInt ((float) physicalSize.getHeight()));

            const auto effectiveScale = rescaled.getWidth() / logicalSize.getWidth();

            const auto hx = jlimit (0, rescaled.getWidth(),  roundToInt ((float) info.hotspot.x * effectiveScale));
            const auto hy = jlimit (0, rescaled.getHeight(), roundToInt ((float) info.hotspot.y * effectiveScale));

            return cursorsBySize.emplace (size, CursorPtr { IconConverters::createHICONFromImage (rescaled, false, hx, hy) }).first->second.get();
        }

    private:
        struct CursorDestructor
        {
            void operator() (HCURSOR ptr) const { if (ptr != nullptr) DestroyCursor (ptr); }
        };

        using CursorPtr = std::unique_ptr<std::remove_pointer_t<HCURSOR>, CursorDestructor>;

        const detail::CustomMouseCursorInfo info;
        std::map<int, CursorPtr> cursorsBySize;
    };

    static auto getCursorSizeForPeerFunction() -> int (*) (ComponentPeer&)
    {
        static const auto getDpiForMonitor = []() -> GetDPIForMonitorFunc
        {
            constexpr auto library = "SHCore.dll";
            LoadLibraryA (library);

            if (auto* handle = GetModuleHandleA (library))
                return (GetDPIForMonitorFunc) GetProcAddress (handle, "GetDpiForMonitor");

            return nullptr;
        }();

        static const auto getSystemMetricsForDpi = []() -> GetSystemMetricsForDpiFunc
        {
            constexpr auto library = "User32.dll";
            LoadLibraryA (library);

            if (auto* handle = GetModuleHandleA (library))
                return (GetSystemMetricsForDpiFunc) GetProcAddress (handle, "GetSystemMetricsForDpi");

            return nullptr;
        }();

        if (getDpiForMonitor == nullptr || getSystemMetricsForDpi == nullptr)
            return [] (ComponentPeer&) { return unityCursorSize; };

        return [] (ComponentPeer& p)
        {
            const ScopedThreadDPIAwarenessSetter threadDpiAwarenessSetter { p.getNativeHandle() };

            UINT dpiX = 0, dpiY = 0;

            if (auto* monitor = MonitorFromWindow ((HWND) p.getNativeHandle(), MONITOR_DEFAULTTONULL))
                if (SUCCEEDED (getDpiForMonitor (monitor, MDT_Default, &dpiX, &dpiY)))
                    return getSystemMetricsForDpi (SM_CXCURSOR, dpiX);

            return unityCursorSize;
        };
    }

    static constexpr auto unityCursorSize = 32;

    static std::unique_ptr<Impl> makeHandle (const detail::CustomMouseCursorInfo& info)
    {
        return std::make_unique<ImageImpl> (info);
    }

    static std::unique_ptr<Impl> makeHandle (const MouseCursor::StandardCursorType type)
    {
        LPCTSTR cursorName = IDC_ARROW;

        switch (type)
        {
            case NormalCursor:
            case ParentCursor:                  break;
            case NoCursor:                      return std::make_unique<BuiltinImpl> (nullptr);
            case WaitCursor:                    cursorName = IDC_WAIT; break;
            case IBeamCursor:                   cursorName = IDC_IBEAM; break;
            case PointingHandCursor:            cursorName = MAKEINTRESOURCE (32649); break;
            case CrosshairCursor:               cursorName = IDC_CROSS; break;

            case LeftRightResizeCursor:
            case LeftEdgeResizeCursor:
            case RightEdgeResizeCursor:         cursorName = IDC_SIZEWE; break;

            case UpDownResizeCursor:
            case TopEdgeResizeCursor:
            case BottomEdgeResizeCursor:        cursorName = IDC_SIZENS; break;

            case TopLeftCornerResizeCursor:
            case BottomRightCornerResizeCursor: cursorName = IDC_SIZENWSE; break;

            case TopRightCornerResizeCursor:
            case BottomLeftCornerResizeCursor:  cursorName = IDC_SIZENESW; break;

            case UpDownLeftRightResizeCursor:   cursorName = IDC_SIZEALL; break;

            case DraggingHandCursor:
            {
                static const unsigned char dragHandData[]
                    { 71,73,70,56,57,97,16,0,16,0,145,2,0,0,0,0,255,255,255,0,0,0,0,0,0,33,249,4,1,0,0,2,0,44,0,0,0,0,16,0,
                      16,0,0,2,52,148,47,0,200,185,16,130,90,12,74,139,107,84,123,39,132,117,151,116,132,146,248,60,209,138,
                      98,22,203,114,34,236,37,52,77,217,247,154,191,119,110,240,193,128,193,95,163,56,60,234,98,135,2,0,59 };

                return makeHandle ({ ScaledImage (ImageFileFormat::loadFrom (dragHandData, sizeof (dragHandData))), { 8, 7 } });
            }

            case CopyingCursor:
            {
                static const unsigned char copyCursorData[]
                    { 71,73,70,56,57,97,21,0,21,0,145,0,0,0,0,0,255,255,255,0,128,128,255,255,255,33,249,4,1,0,0,3,0,44,0,0,0,0,21,0,
                      21,0,0,2,72,4,134,169,171,16,199,98,11,79,90,71,161,93,56,111,78,133,218,215,137,31,82,154,100,200,86,91,202,142,
                      12,108,212,87,235,174, 15,54,214,126,237,226,37,96,59,141,16,37,18,201,142,157,230,204,51,112,252,114,147,74,83,
                      5,50,68,147,208,217,16,71,149,252,124,5,0,59,0,0 };

                return makeHandle ({ ScaledImage (ImageFileFormat::loadFrom (copyCursorData, sizeof (copyCursorData))), { 1, 3 } });
            }

            case NumStandardCursorTypes: JUCE_FALLTHROUGH
            default:
                jassertfalse; break;
        }

        return std::make_unique<BuiltinImpl> ([&]
        {
            if (auto* c = LoadCursor (nullptr, cursorName))
                return c;

            return LoadCursor (nullptr, IDC_ARROW);
        }());
    }

    std::unique_ptr<Impl> impl;
};

//==============================================================================
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

//==============================================================================
JUCE_COMCLASS (JuceIVirtualDesktopManager, "a5cd92ff-29be-454c-8d04-d82879fb3f1b") : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE IsWindowOnCurrentVirtualDesktop(
         __RPC__in HWND topLevelWindow,
         __RPC__out BOOL * onCurrentDesktop) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetWindowDesktopId(
         __RPC__in HWND topLevelWindow,
         __RPC__out GUID * desktopId) = 0;

    virtual HRESULT STDMETHODCALLTYPE MoveWindowToDesktop(
         __RPC__in HWND topLevelWindow,
         __RPC__in REFGUID desktopId) = 0;
};

JUCE_COMCLASS (JuceVirtualDesktopManager, "aa509086-5ca9-4c25-8f95-589d3c07b48a");

} // namespace juce

#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL (juce::JuceIVirtualDesktopManager, 0xa5cd92ff, 0x29be, 0x454c, 0x8d, 0x04, 0xd8, 0x28, 0x79, 0xfb, 0x3f, 0x1b)
__CRT_UUID_DECL (juce::JuceVirtualDesktopManager,  0xaa509086, 0x5ca9, 0x4c25, 0x8f, 0x95, 0x58, 0x9d, 0x3c, 0x07, 0xb4, 0x8a)
#endif

bool juce::detail::WindowingHelpers::isWindowOnCurrentVirtualDesktop (void* x)
{
    if (x == nullptr)
        return false;

    static auto* desktopManager = []
    {
        JuceIVirtualDesktopManager* result = nullptr;

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

        if (SUCCEEDED (CoCreateInstance (__uuidof (JuceVirtualDesktopManager), nullptr, CLSCTX_ALL, IID_PPV_ARGS (&result))))
            return result;

        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        return static_cast<JuceIVirtualDesktopManager*> (nullptr);
    }();

    BOOL current = false;

    if (auto* dm = desktopManager)
        if (SUCCEEDED (dm->IsWindowOnCurrentVirtualDesktop (static_cast<HWND> (x), &current)))
            return current != false;

    return true;
}
