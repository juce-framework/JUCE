/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
 #include <juce_audio_plugin_client/AAX/juce_AAX_Modifier_Injector.h>
#endif

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

ScopedThreadDPIAwarenessSetter::ScopedThreadDPIAwarenessSetter (ScopedThreadDPIAwarenessSetter&&) noexcept = default;
ScopedThreadDPIAwarenessSetter& ScopedThreadDPIAwarenessSetter::operator= (ScopedThreadDPIAwarenessSetter&&) noexcept = default;

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
RTL_OSVERSIONINFOW getWindowsVersionInfo();

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
    using Ptr = ReferenceCountedObjectPtr<WindowsBitmapImage>;

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

    std::unique_ptr<ImageType> createType() const override
    {
        // This type only exists to return a type ID that's different to the SoftwareImageType's ID,
        // so that `SoftwareImageType{}.convert (windowsBitmapImage)` works.
        // If we return SoftwareImageType here, then SoftwareImageType{}.convert() will compare the
        // type IDs and assume the source image is already of the correct type.
        struct Type : public ImageType
        {
            int getTypeID() const override { return ByteOrder::makeInt ('w', 'b', 'i', 't'); }
            ImagePixelData::Ptr create (Image::PixelFormat, int, int, bool) const override { return {}; }
            Image convert (const Image&) const override { return {}; }
        };

        return std::make_unique<Type>();
    }

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
        Image newImage { SoftwareImageType{}.create (pixelFormat, width, height, pixelFormat != Image::RGB) };

        {
            Graphics g (newImage);
            g.drawImageAt (Image { *this }, 0, 0);
        }

        return newImage.getPixelData();
    }

    static void updateLayeredWindow (HDC sourceHdc, HWND hwnd, Point<int> pt, float constantAlpha)
    {
        const auto windowBounds = getWindowScreenRect (hwnd);

        auto p = D2DUtilities::toPOINT (pt);
        POINT pos = { windowBounds.left, windowBounds.top };
        SIZE size = { windowBounds.right - windowBounds.left,
                      windowBounds.bottom - windowBounds.top };

        BLENDFUNCTION bf { AC_SRC_OVER, 0, (BYTE) (255.0f * constantAlpha), AC_SRC_ALPHA };

        UpdateLayeredWindow (hwnd, nullptr, &pos, &size, sourceHdc, &p, 0, &bf, ULW_ALPHA);
    }

    void updateLayeredWindow (HWND hwnd, Point<int> pt, float constantAlpha) const noexcept
    {
        updateLayeredWindow (hdc, hwnd, pt, constantAlpha);
    }

    void blitToDC (HDC dc, int x, int y) const noexcept
    {
        SetMapMode (dc, MM_TEXT);

        StretchDIBits (dc,
                       x, y, width, height,
                       0, 0, width, height,
                       bitmapData, (const BITMAPINFO*) &bitmapInfo,
                       DIB_RGB_COLORS, SRCCOPY);
    }

    HBITMAP getHBITMAP() const { return hBitmap; }
    HDC getHDC() const { return hdc; }

private:
    HBITMAP hBitmap;
    HGDIOBJ previousBitmap;
    BITMAPV4HEADER bitmapInfo;
    HDC hdc;
    uint8* bitmapData;
    int pixelStride, lineStride;
    uint8* imageData;

    static bool isGraphicsCard32Bit()
    {
        ScopedDeviceContext deviceContext { nullptr };
        return GetDeviceCaps (deviceContext.dc, BITSPIXEL) > 24;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsBitmapImage)
};

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

                Image result = Image (Image::ARGB, bm.bmWidth, bm.bmHeight, true, SoftwareImageType{});
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
        info.hbmColor = nativeBitmap->getHBITMAP();

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
/*  This is an interface to functionality that is implemented differently depending on the rendering
    backend, currently either GDI or Direct2D on Windows.

    This isn't public, so feel free to add or remove functions if necessary.
    In general, it's best to keep things consistent between renderers, so try to make changes
    in the HWNDComponentPeer rather than in implementations of RenderContext wherever possible.
    However, any behaviour that is only required in specific renderers should be added to the
    RenderContext implementations of those renderers.
*/
struct RenderContext
{
    virtual ~RenderContext() = default;

    /*  The name of the renderer backend.
        This must be unique - no two backends may share the same name.
        The name may be displayed to the user, so it should be descriptive.
    */
    virtual const char* getName() const = 0;

    /*  The following functions will all be called by the peer to update the state of the renderer. */
    virtual void updateConstantAlpha() = 0;
    virtual void handlePaintMessage() = 0;
    virtual void repaint (const Rectangle<int>& area) = 0;
    virtual void dispatchDeferredRepaints() = 0;
    virtual void performAnyPendingRepaintsNow() = 0;
    virtual void onVBlank() = 0;
    virtual void handleShowWindow() = 0;

    /*  Gets a snapshot of whatever the render context is currently showing. */
    virtual Image createSnapshot() = 0;
};

//==============================================================================
class HWNDComponentPeer final : public ComponentPeer
                              , private ComponentPeer::VBlankListener
                              , private Timer
                              #if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
                              , public ModifierKeyReceiver
                              #endif
{
public:
    //==============================================================================
    HWNDComponentPeer (Component& comp,
                       int windowStyleFlags,
                       HWND parent,
                       bool nonRepainting,
                       int engine)
        : ComponentPeer (comp, windowStyleFlags),
          dontRepaint (nonRepainting),
          parentToAddTo (parent)
    {
        getNativeRealtimeModifiers = getMouseModifiers;

        // CreateWindowEx needs to be called from the message thread
        callFunctionIfNotLocked (&createWindowCallback, this);

        // Complete the window initialisation on the calling thread
        setTitle (component.getName());
        updateShadower();

        updateCurrentMonitorAndRefreshVBlankDispatcher (ForceRefreshDispatcher::yes);

        if (parentToAddTo != nullptr)
        {
            monitorUpdateTimer.emplace ([this]
                                        {
                                            updateCurrentMonitorAndRefreshVBlankDispatcher (ForceRefreshDispatcher::yes);
                                            monitorUpdateTimer->startTimer (1000);
                                        });
        }

        suspendResumeRegistration = ScopedSuspendResumeNotificationRegistration { hwnd };

        setCurrentRenderingEngine (engine);
    }

    ~HWNDComponentPeer() override
    {
        // Clean up that needs to happen on the calling thread
        suspendResumeRegistration = {};

        VBlankDispatcher::getInstance()->removeListener (*this);

        // do this first to avoid messages arriving for this window before it's destroyed
        JuceWindowIdentifier::setAsJUCEWindow (hwnd, false);

        if (isAccessibilityActive)
            WindowsAccessibility::revokeUIAMapEntriesForWindow (hwnd);

        shadower = nullptr;
        currentTouches.deleteAllTouchesForPeer (this);

        // Destroy the window from the message thread
        callFunctionIfNotLocked (&destroyWindowCallback, this);

        // And one last little bit of cleanup
        if (dropTarget != nullptr)
        {
            dropTarget->peerIsDeleted = true;
            dropTarget->Release();
            dropTarget = nullptr;
        }
    }

    //==============================================================================
    auto getHWND() const { return hwnd; }
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
        if (getTransparencyKind() == TransparencyKind::perPixel
            && lastPaintTime > 0
            && Time::getMillisecondCounter() > lastPaintTime + 30)
        {
            handlePaintMessage();
        }
    }

    std::optional<BorderSize<int>> getCustomBorderSize() const
    {
        if (hasTitleBar() || (styleFlags & windowIsTemporary) != 0 || isFullScreen())
            return {};

        return BorderSize<int> { 0, 0, 0, 0 };
    }

    std::optional<BorderSize<int>> findPhysicalBorderSize() const
    {
        if (const auto custom = getCustomBorderSize())
            return *custom;

        ScopedThreadDPIAwarenessSetter setter { hwnd };

        WINDOWINFO info{};
        info.cbSize = sizeof (info);

        if (! GetWindowInfo (hwnd, &info))
            return {};

        // Sometimes GetWindowInfo returns bogus information when called in the middle of restoring
        // the window
        if (info.rcWindow.left <= -32000 && info.rcWindow.top <= -32000)
            return {};

        return BorderSize<int> { info.rcClient.top - info.rcWindow.top,
                                 info.rcClient.left - info.rcWindow.left,
                                 info.rcWindow.bottom - info.rcClient.bottom,
                                 info.rcWindow.right - info.rcClient.right };
    }

    void setBounds (const Rectangle<int>& bounds, bool isNowFullScreen) override
    {
        // If we try to set new bounds while handling an existing position change,
        // Windows may get confused about our current scale and size.
        // This can happen when moving a window between displays, because the mouse-move
        // generator in handlePositionChanged can cause the window to move again.
        if (inHandlePositionChanged)
            return;

        if (isNowFullScreen != isFullScreen())
            setFullScreen (isNowFullScreen);

        const ScopedValueSetter<bool> scope (shouldIgnoreModalDismiss, true);

        const auto borderSize = findPhysicalBorderSize().value_or (BorderSize<int>{});
        auto newBounds = borderSize.addedTo ([&]
        {
            ScopedThreadDPIAwarenessSetter setter { hwnd };

            if (! isPerMonitorDPIAwareWindow (hwnd))
                return bounds;

            if (inDpiChange)
                return convertLogicalScreenRectangleToPhysical (bounds, hwnd);

            return convertLogicalScreenRectangleToPhysical (bounds, hwnd)
                    .withPosition (Desktop::getInstance().getDisplays().logicalToPhysical (bounds.getTopLeft()));
        }());

        if (getTransparencyKind() == TransparencyKind::perPixel)
        {
            if (auto parentHwnd = GetParent (hwnd))
            {
                auto parentRect = convertPhysicalScreenRectangleToLogical (D2DUtilities::toRectangle (getWindowScreenRect (parentHwnd)), hwnd);
                newBounds.translate (parentRect.getX(), parentRect.getY());
            }
        }

        const auto oldBounds = [this]
        {
            ScopedThreadDPIAwarenessSetter setter { hwnd };
            RECT result;
            GetWindowRect (hwnd, &result);
            return D2DUtilities::toRectangle (result);
        }();

        const bool hasMoved = (oldBounds.getPosition() != bounds.getPosition());
        const bool hasResized = (oldBounds.getWidth() != bounds.getWidth()
                                  || oldBounds.getHeight() != bounds.getHeight());

        DWORD flags = SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
        if (! hasMoved)    flags |= SWP_NOMOVE;
        if (! hasResized)  flags |= SWP_NOSIZE;

        SetWindowPos (hwnd,
                      nullptr,
                      newBounds.getX(),
                      newBounds.getY(),
                      newBounds.getWidth(),
                      newBounds.getHeight(),
                      flags);

        if (hasResized && isValidPeer (this))
        {
            repaintNowIfTransparent();
        }
    }

    Rectangle<int> getBounds() const override
    {
        if (parentToAddTo == nullptr)
        {
            if (hasTitleBar())
            {
                // Depending on the desktop scale factor, the physical size of the window may not map to
                // an integral client-area size.
                // In this case, we always round the width and height of the client area up to the next
                // integer.
                // This means that we may end up clipping off up to one logical pixel under the physical
                // window border, but this is preferable to displaying an uninitialised/unpainted
                // region of the client area.
                const auto physicalBorder = findPhysicalBorderSize().value_or (BorderSize<int>{});

                const auto physicalBounds = D2DUtilities::toRectangle (getWindowScreenRect (hwnd));
                const auto physicalClient = physicalBorder.subtractedFrom (physicalBounds);
                const auto logicalClient = convertPhysicalScreenRectangleToLogical (physicalClient.toFloat(), hwnd);
                const auto snapped = logicalClient.withPosition (logicalClient.getPosition().roundToInt().toFloat()).getSmallestIntegerContainer();
                return snapped;
            }

            const auto logicalClient = convertPhysicalScreenRectangleToLogical (getClientRectInScreen(), hwnd);
            return logicalClient;
        }

        auto localBounds = D2DUtilities::toRectangle (getWindowClientRect (hwnd));

        if (isPerMonitorDPIAwareWindow (hwnd))
            return (localBounds.toDouble() / getPlatformScaleFactor()).toNearestInt();

        return localBounds;
    }

    Point<int> getScreenPosition() const
    {
        return convertPhysicalScreenPointToLogical (getClientRectInScreen().getPosition(), hwnd);
    }

    Point<float> localToGlobal (Point<float> relativePosition) override  { return relativePosition + getScreenPosition().toFloat(); }
    Point<float> globalToLocal (Point<float> screenPosition) override    { return screenPosition   - getScreenPosition().toFloat(); }

    using ComponentPeer::localToGlobal;
    using ComponentPeer::globalToLocal;

    enum class TransparencyKind
    {
        perPixel,
        constant,
        opaque,
    };

    TransparencyKind getTransparencyKind() const
    {
        return transparencyKind;
    }

    void setAlpha (float) override
    {
        setLayeredWindow();

        if (renderContext != nullptr)
            renderContext->updateConstantAlpha();
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

    bool isShowing() const override
    {
        return IsWindowVisible (hwnd) && ! isMinimised();
    }

    void setFullScreen (bool shouldBeFullScreen) override
    {
        const ScopedValueSetter<bool> scope (shouldIgnoreModalDismiss, true);

        setMinimised (false);

        if (isFullScreen() != shouldBeFullScreen)
        {
            if (constrainer != nullptr)
                constrainer->resizeStart();

            const WeakReference<Component> deletionChecker (&component);

            if (shouldBeFullScreen)
            {
                ShowWindow (hwnd, SW_SHOWMAXIMIZED);
            }
            else
            {
                auto boundsCopy = lastNonFullscreenBounds;

                ShowWindow (hwnd, SW_SHOWNORMAL);

                if (! boundsCopy.isEmpty())
                    setBounds (detail::ScalingHelpers::scaledScreenPosToUnscaled (component, boundsCopy), false);
            }

            if (deletionChecker != nullptr)
                handleMovedOrResized();

            if (constrainer != nullptr)
                constrainer->resizeEnd();
        }
    }

    bool isFullScreen() const override
    {
        WINDOWPLACEMENT wp;
        wp.length = sizeof (wp);
        GetWindowPlacement (hwnd, &wp);

        return wp.showCmd == SW_SHOWMAXIMIZED;
    }

    Rectangle<int> getClientRectInScreen() const
    {
        ScopedThreadDPIAwarenessSetter setter { hwnd };

        RECT rect{};
        GetClientRect (hwnd, &rect);
        auto points = readUnaligned<std::array<POINT, 2>> (&rect);
        MapWindowPoints (hwnd, nullptr, points.data(), (UINT) points.size());
        const auto result = readUnaligned<RECT> (&points);

        return D2DUtilities::toRectangle (result);
    }

    bool contains (Point<int> localPos, bool trueIfInAChildWindow) const override
    {
        auto r = convertPhysicalScreenRectangleToLogical (D2DUtilities::toRectangle (getWindowScreenRect (hwnd)), hwnd);

        if (! r.withZeroOrigin().contains (localPos))
            return false;

        const auto screenPos = convertLogicalScreenPointToPhysical (localPos + getScreenPosition(), hwnd);

        auto w = WindowFromPoint (D2DUtilities::toPOINT (screenPos));
        return w == hwnd || (trueIfInAChildWindow && (IsChild (hwnd, w) != 0));
    }

    OptionalBorderSize getFrameSizeIfPresent() const override
    {
        return ComponentPeer::OptionalBorderSize { getFrameSize() };
    }

    BorderSize<int> getFrameSize() const override
    {
        return findPhysicalBorderSize().value_or (BorderSize<int>{}).multipliedBy (1.0 / scaleFactor);
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
        const ScopedValueSetter ignoreDismissScope (shouldIgnoreModalDismiss, true);
        const ScopedValueSetter titlebarScope (shouldDeactivateTitleBar, (styleFlags & windowIsTemporary) == 0);

        callFunctionIfNotLocked (&setFocusCallback, hwnd);
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
        if (renderContext != nullptr)
            renderContext->repaint ((area.toDouble() * getPlatformScaleFactor()).getSmallestIntegerContainer());
    }

    void dispatchDeferredRepaints()
    {
        if (renderContext != nullptr)
            renderContext->dispatchDeferredRepaints();
    }

    void performAnyPendingRepaintsNow() override
    {
        if (renderContext != nullptr)
            renderContext->performAnyPendingRepaintsNow();
    }

    Image createSnapshot()
    {
        if (renderContext != nullptr)
            return renderContext->createSnapshot();

        return {};
    }

    //==============================================================================
    void onVBlank (double timestampSec) override
    {
        callVBlankListeners (timestampSec);
        dispatchDeferredRepaints();

        if (renderContext != nullptr)
            renderContext->onVBlank();
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

        ModifierKeys::currentModifiers = ModifierKeys::getCurrentModifiers().withOnlyMouseButtons().withFlags (keyMods);
    }

    static void updateModifiersFromWParam (const WPARAM wParam)
    {
        int mouseMods = 0;
        if (wParam & MK_LBUTTON)   mouseMods |= ModifierKeys::leftButtonModifier;
        if (wParam & MK_RBUTTON)   mouseMods |= ModifierKeys::rightButtonModifier;
        if (wParam & MK_MBUTTON)   mouseMods |= ModifierKeys::middleButtonModifier;
        if (wParam & MK_XBUTTON1)  mouseMods |= ModifierKeys::backButtonModifier;
        if (wParam & MK_XBUTTON2)  mouseMods |= ModifierKeys::forwardButtonModifier;

        ModifierKeys::currentModifiers = ModifierKeys::getCurrentModifiers().withoutMouseButtons().withFlags (mouseMods);
        updateKeyModifiers();
    }

    //==============================================================================
    bool dontRepaint;
    static inline ModifierKeys modifiersAtLastCallback;

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
        DragInfo dragInfo;
        bool peerIsDeleted = false;

    private:
        Point<float> getMousePos (POINTL mousePos) const
        {
            const auto originalPos = D2DUtilities::toPoint ({ mousePos.x, mousePos.y });
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

    static void getLastError()
    {
        TCHAR messageBuffer[256] = {};

        FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr,
                       GetLastError(),
                       MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                       messageBuffer,
                       (DWORD) numElementsInArray (messageBuffer) - 1,
                       nullptr);

        DBG (messageBuffer);
        jassertfalse;
    }

    DWORD computeNativeStyleFlags() const
    {
        const auto titled = ! isKioskMode() && (styleFlags & windowHasTitleBar) != 0;
        const auto usesDropShadow = windowUsesNativeShadow();
        const auto hasClose = (styleFlags & windowHasCloseButton) != 0;
        const auto hasMin = (styleFlags & windowHasMinimiseButton) != 0;
        const auto hasMax = (styleFlags & windowHasMaximiseButton) != 0;
        const auto resizable = (styleFlags & windowIsResizable) != 0;

        DWORD result = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

        if (parentToAddTo != nullptr)
        {
            result |= WS_CHILD;
        }
        else if (titled || usesDropShadow)
        {
            result |= usesDropShadow ? WS_CAPTION : 0;
            result |= titled ? (WS_OVERLAPPED | WS_CAPTION) : WS_POPUP;
            result |= hasClose ? (WS_SYSMENU | WS_CAPTION) : 0;
            result |= hasMin ? (WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU) : 0;
            result |= hasMax ? (WS_MAXIMIZEBOX | WS_CAPTION | WS_SYSMENU) : 0;
            result |= resizable ? WS_THICKFRAME : 0;
        }
        else
        {
            // Transparent windows need WS_POPUP and not WS_OVERLAPPED | WS_CAPTION, otherwise
            // the top corners of the window will get rounded unconditionally.
            // Unfortunately, this disables nice mouse handling for the caption area.
            result |= WS_POPUP;
        }

        return result;
    }

    bool hasTitleBar() const                 { return (styleFlags & windowHasTitleBar) != 0; }
    double getScaleFactor() const            { return scaleFactor; }

private:
    HWND hwnd, parentToAddTo;
    std::unique_ptr<DropShadower> shadower;
    uint32 lastPaintTime = 0;
    ULONGLONG lastMagnifySize = 0;
    bool isDragging = false, isMouseOver = false,
         hasCreatedCaret = false, constrainerIsResizing = false, sizing = false;
    IconConverters::IconPtr currentWindowIcon;
    FileDropTarget* dropTarget = nullptr;
    UWPUIViewSettings uwpViewSettings;
    TransparencyKind transparencyKind = TransparencyKind::opaque;
   #if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
    ModifierKeyProvider* modProvider = nullptr;
   #endif

    double scaleFactor = 1.0;
    bool inDpiChange = 0, inHandlePositionChanged = 0;
    HMONITOR currentMonitor = nullptr;

    bool isAccessibilityActive = false;

    //==============================================================================
    static inline MultiTouchMapper<DWORD> currentTouches;

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

        JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL_INLINE (WindowClassHolder)

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
        static_cast<HWNDComponentPeer*> (userData)->createWindowOnMessageThread();
        return nullptr;
    }

    void createWindowOnMessageThread()
    {
        const auto type = computeNativeStyleFlags();

        const auto exstyle = std::invoke ([&]() -> DWORD
        {
            if (parentToAddTo != nullptr)
                return 0;

            const auto appearsOnTaskbar = (styleFlags & windowAppearsOnTaskbar) != 0;
            return appearsOnTaskbar ? WS_EX_APPWINDOW : WS_EX_TOOLWINDOW;
        });

        hwnd = CreateWindowEx (exstyle, WindowClassHolder::getInstance()->getWindowClassName(),
                               L"", type, 0, 0, 0, 0, parentToAddTo, nullptr,
                               (HINSTANCE) Process::getCurrentModuleInstanceHandle(), nullptr);

        const auto titled = (styleFlags & windowHasTitleBar) != 0;
        const auto usesDropShadow = windowUsesNativeShadow();

        if (! titled && usesDropShadow)
        {
            // The choice of margins is very particular.
            // - Using 0 for all values disables the system decoration (shadow etc.) completely.
            // - Using -1 for all values breaks the software renderer, because the client content
            //   gets blended with the system-drawn controls.
            //   It looks OK most of the time with the D2D renderer, but can look very ugly during
            //   resize because the native window controls still get drawn under the client area.
            // - Using 1 for all values looks the way we want for both renderers, but seems to
            //   prevent the Windows 11 maximize-button flyout from appearing (?).
            // - Using 1 for left and right, and 0 for top and bottom shows the system shadow and
            //   maximize-button flyout.
            static constexpr MARGINS margins { 1, 1, 0, 0 };
            ::DwmExtendFrameIntoClientArea (hwnd, &margins);
            ::SetWindowPos (hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
        }

       #if JUCE_DEBUG
        // The DPI-awareness context of this window and JUCE's hidden message window are different.
        // You normally want these to match otherwise timer events and async messages will happen
        // in a different context to normal HWND messages which can cause issues with UI scaling.
        jassert (isPerMonitorDPIAwareWindow (hwnd) == isPerMonitorDPIAwareWindow (juce_messageWindowHandle)
                   || numActiveScopedDpiAwarenessDisablers > 0);
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
            checkForPointerAPI();

            // This is needed so that our plugin window gets notified of WM_SETTINGCHANGE messages
            // and can respond to display scale changes
            if (! JUCEApplication::isStandaloneApp())
                settingChangeCallback = ComponentPeer::forceDisplayUpdate;

            // Calling this function here is (for some reason) necessary to make Windows
            // correctly enable the menu items that we specify in the wm_initmenu message.
            GetSystemMenu (hwnd, false);

            setAlpha (component.getAlpha());
        }
        else
        {
            getLastError();
        }
    }

    static BOOL CALLBACK revokeChildDragDropCallback (HWND hwnd, LPARAM)    { RevokeDragDrop (hwnd); return TRUE; }

    static void* destroyWindowCallback (void* userData)
    {
        static_cast<HWNDComponentPeer*> (userData)->destroyWindowOnMessageThread();
        return nullptr;
    }

    void destroyWindowOnMessageThread() noexcept
    {
        if (IsWindow (hwnd))
        {
            RevokeDragDrop (hwnd);

            // NB: we need to do this before DestroyWindow() as child HWNDs will be invalid after
            EnumChildWindows (hwnd, revokeChildDragDropCallback, 0);

            DestroyWindow (hwnd);
        }
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

    bool isOpaque() const
    {
        return component.isOpaque();
    }

    bool windowUsesNativeShadow() const
    {
        return ! isKioskMode()
               && (hasTitleBar()
                   || (   (0 != (styleFlags & windowHasDropShadow))
                       && (0 == (styleFlags & windowIsSemiTransparent))
                       && (0 == (styleFlags & windowIsTemporary))));
    }

    void updateShadower()
    {
        if (! component.isCurrentlyModal()
            && (styleFlags & windowHasDropShadow) != 0
            && ! windowUsesNativeShadow())
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

    TransparencyKind computeTransparencyKind() const
    {
        if (! hasTitleBar() && ! component.isOpaque())
            return TransparencyKind::perPixel;

        // If you hit this assertion, you're trying to create a window with a native titlebar
        // and per-pixel transparency. If you want a semi-transparent window, then remove the
        // native title bar. Otherwise, ensure that the window's component is opaque.
        jassert (! hasTitleBar() || component.isOpaque());

        if (component.getAlpha() < 1.0f)
            return TransparencyKind::constant;

        return TransparencyKind::opaque;
    }

    void setLayeredWindow()
    {
        const auto old = std::exchange (transparencyKind, computeTransparencyKind());

        if (old == getTransparencyKind())
            return;

        const auto prev = GetWindowLongPtr (hwnd, GWL_EXSTYLE);

        // UpdateLayeredWindow will fail if SetLayeredWindowAttributes has previously been called
        // without unsetting and resetting the layering style bit.
        // UpdateLayeredWindow is used for perPixel windows; SetLayeredWindowAttributes is used for
        // windows with a constant alpha but otherwise "opaque" contents (i.e. component.isOpaque()
        // returns true but component.getAlpha() is less than 1.0f).
        if (getTransparencyKind() == TransparencyKind::perPixel)
            SetWindowLongPtr (hwnd, GWL_EXSTYLE, prev & ~WS_EX_LAYERED);

        const auto newStyle = getTransparencyKind() == TransparencyKind::opaque
                              ? (prev & ~WS_EX_LAYERED)
                              : (prev | WS_EX_LAYERED);

        SetWindowLongPtr (hwnd, GWL_EXSTYLE, newStyle);
        RedrawWindow (hwnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
    }

    //==============================================================================
    void handlePaintMessage()
    {
        if (renderContext != nullptr)
            renderContext->handlePaintMessage();

        lastPaintTime = Time::getMillisecondCounter();
    }

    //==============================================================================
    void doMouseEvent (Point<float> position, float pressure, float orientation = 0.0f, ModifierKeys mods = ModifierKeys::currentModifiers)
    {
        handleMouseEvent (MouseInputSource::InputSourceType::mouse, position, mods, pressure, orientation, getMouseEventTime());
    }

    StringArray getAvailableRenderingEngines() override;
    int getCurrentRenderingEngine() const override;
    void setCurrentRenderingEngine (int e) override;

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

    enum class WindowArea
    {
        nonclient,
        client,
    };

    std::optional<LRESULT> doMouseMoveAtPoint (bool isMouseDownEvent, WindowArea area, Point<float> position)
    {
        auto modsToSend = ModifierKeys::getCurrentModifiers();

        // this will be handled by WM_TOUCH
        if (isTouchEvent() || areOtherTouchSourcesActive())
            return {};

        if (! isMouseOver)
        {
            isMouseOver = true;

            // This avoids a rare stuck-button problem when focus is lost unexpectedly, but must
            // not be called as part of a move, in case it's actually a mouse-drag from another
            // app which ends up here when we get focus before the mouse is released..
            if (isMouseDownEvent)
                NullCheckedInvocation::invoke (getNativeRealtimeModifiers);

            updateKeyModifiers();
            updateModifiersFromModProvider();

            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof (tme);
            tme.dwFlags = TME_LEAVE | (area == WindowArea::nonclient ? TME_NONCLIENT : 0);
            tme.hwndTrack = hwnd;
            tme.dwHoverTime = 0;

            if (! TrackMouseEvent (&tme))
                jassertfalse;

            if (area == WindowArea::client)
                Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
        }
        else if (! isDragging && ! contains (position.roundToInt(), false))
        {
            return {};
        }

        static uint32 lastMouseTime = 0;
        auto now = Time::getMillisecondCounter();

        if (! Desktop::getInstance().getMainMouseSource().isDragging())
            modsToSend = modsToSend.withoutMouseButtons();

        if (now >= lastMouseTime)
        {
            lastMouseTime = now;
            doMouseEvent (position, MouseInputSource::defaultPressure,
                          MouseInputSource::defaultOrientation, modsToSend);
        }

        return {};
    }

    std::optional<LRESULT> doMouseMove (const LPARAM lParam, bool isMouseDownEvent, WindowArea area)
    {
        // Check if the mouse has moved since being pressed in the caption area.
        // If it has, then we defer to DefWindowProc to handle the mouse movement.
        // Allowing DefWindowProc to handle WM_NCLBUTTONDOWN directly will pause message
        // processing (and therefore painting) when the mouse is clicked in the caption area,
        // which is why we wait until the mouse is *moved* before asking the system to take over.
        // Letting the system handle the move is important for things like Aero Snap to work.
        if (area == WindowArea::nonclient && captionMouseDown.has_value() && *captionMouseDown != lParam)
        {
            captionMouseDown.reset();
            return handleNcMouseEventThenFixModifiers (WM_NCLBUTTONDOWN, HTCAPTION, lParam);
        }

        const auto position = area == WindowArea::client ? getPointFromLocalLParam (lParam)
                                                         : getLocalPointFromScreenLParam (lParam);

        return doMouseMoveAtPoint (isMouseDownEvent, area, position);
    }

    LRESULT handleNcMouseEventThenFixModifiers (UINT msg, WPARAM wParam, LPARAM lParam)
    {
        // When clicking and dragging on the caption area (including in the edge resize areas), a
        // new modal loop is started inside DefWindowProc. This modal loop appears to consume some
        // mouse events, without forwarding them back to our own window proc. In particular, we
        // never get to see the WM_NCLBUTTONUP event with the HTCAPTION argument, or any other
        // kind of mouse-up event to signal that the loop exited, so
        // ModifierKeys::currentModifiers gets left in the wrong state. As a workaround, we
        // manually update the modifier keys after DefWindowProc exits, and update the
        // capture state if necessary.
        const auto result = DefWindowProc (hwnd, msg, wParam, lParam);
        getMouseModifiers();
        releaseCaptureIfNecessary();
        return result;
    }

    void updateModifiersFromModProvider() const
    {
       #if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
        if (modProvider != nullptr)
            ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withFlags (modProvider->getWin32Modifiers());
       #endif
    }

    void updateModifiersWithMouseWParam (const WPARAM wParam) const
    {
        updateModifiersFromWParam (wParam);
        updateModifiersFromModProvider();
    }

    void releaseCaptureIfNecessary() const
    {
        if (! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown() && hwnd == GetCapture())
            ReleaseCapture();
    }

    void doMouseDown (LPARAM lParam, const WPARAM wParam)
    {
        // this will be handled by WM_TOUCH
        if (isTouchEvent() || areOtherTouchSourcesActive())
            return;

        if (GetCapture() != hwnd)
            SetCapture (hwnd);

        doMouseMove (lParam, true, WindowArea::client);

        if (isValidPeer (this))
        {
            updateModifiersWithMouseWParam (wParam);

            isDragging = true;

            doMouseEvent (getPointFromLocalLParam (lParam), MouseInputSource::defaultPressure);
        }

        // If this is the first event after receiving both a MOUSEACTIVATE and a SETFOCUS, then
        // process the postponed focus update.
        if (std::exchange (mouseActivateFlags, (uint8_t) 0) == (gotMouseActivate | gotSetFocus))
            handleSetFocus();
    }

    void doMouseUp (Point<float> position, const WPARAM wParam, bool adjustCapture = true)
    {
        // this will be handled by WM_TOUCH
        if (isTouchEvent() || areOtherTouchSourcesActive())
            return;

        updateModifiersWithMouseWParam (wParam);

        const bool wasDragging = std::exchange (isDragging, false);

        // release the mouse capture if the user has released all buttons
        if (adjustCapture)
            releaseCaptureIfNecessary();

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
            doMouseUp (getCurrentMousePos(), (WPARAM) 0, false);
    }

    /*  The parameter specifies the area the cursor just left. */
    void doMouseExit (WindowArea area)
    {
        isMouseOver = false;

        const auto messagePos = GetMessagePos();

        // If the system tells us that the mouse left an area, but the cursor is still over that
        // area, respect the system's decision and treat this as a mouse-leave event.
        // Starting a native drag-n-drop or dragging the window caption may cause the system to send
        // a mouse-leave event while the mouse is still within the window's bounds.
        const auto shouldRestartTracking = std::invoke ([&]
        {
            auto* peer = getOwnerOfWindow (WindowFromPoint (getPOINTFromLParam (messagePos)));

            if (peer != this)
                return false;

            const auto newAreaNative = peerWindowProc (hwnd, WM_NCHITTEST, 0, messagePos);

            if (newAreaNative == HTNOWHERE || newAreaNative == HTTRANSPARENT)
                return false;

            if (newAreaNative == HTCLIENT)
                return area == WindowArea::nonclient;

            return area == WindowArea::client;
        });

        if (shouldRestartTracking)
        {
            doMouseMoveAtPoint (false,
                                area == WindowArea::client ? WindowArea::nonclient : WindowArea::client,
                                getLocalPointFromScreenLParam (messagePos));
        }
        else if (! areOtherTouchSourcesActive())
        {
            doMouseEvent (getCurrentMousePos(), MouseInputSource::defaultPressure);
        }
    }

    std::tuple<ComponentPeer*, Point<float>> findPeerUnderMouse()
    {
        auto currentMousePos = getPOINTFromLParam (GetMessagePos());

        auto* peer = getOwnerOfWindow (WindowFromPoint (currentMousePos));

        if (peer == nullptr)
            peer = this;

        return std::tuple (peer, peer->globalToLocal (convertPhysicalScreenPointToLogical (D2DUtilities::toPoint (currentMousePos), hwnd).toFloat()));
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

    bool doMouseWheel (const WPARAM wParam, const bool isVertical)
    {
        updateKeyModifiers();
        const float amount = jlimit (-1000.0f, 1000.0f, 0.5f * (short) HIWORD (wParam));

        MouseWheelDetails wheel;
        wheel.deltaX = isVertical ? 0.0f : amount / -256.0f;
        wheel.deltaY = isVertical ? amount / 256.0f : 0.0f;
        wheel.isReversed = false;
        wheel.isSmooth = false;
        wheel.isInertial = false;

        // From Windows 10 onwards, mouse events are sent first to the window under the mouse, not
        // the window with focus, despite what the MSDN docs might say.
        // This is the behaviour we want; if we're receiving a scroll event, we can assume it
        // should be processed by the current peer.
        const auto localPos = getLocalPointFromScreenLParam ((LPARAM) GetMessagePos());
        handleMouseWheel (getPointerType (wParam), localPos, getMouseEventTime(), wheel);
        return true;
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
        const auto pos = globalToLocal (convertPhysicalScreenPointToLogical (D2DUtilities::toPoint ({ roundToInt (touch.x / 100.0f),
                                                                                                      roundToInt (touch.y / 100.0f) }), hwnd).toFloat());
        const auto pressure = touchPressure;
        auto modsToSend = ModifierKeys::getCurrentModifiers();

        if (isDown)
        {
            ModifierKeys::currentModifiers = ModifierKeys::getCurrentModifiers().withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
            modsToSend = ModifierKeys::getCurrentModifiers();

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
            modsToSend = ModifierKeys::getCurrentModifiers().withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
        }

        handleMouseEvent (MouseInputSource::InputSourceType::touch, pos, modsToSend,
                          pressure, orientation, time, {}, touchIndex);

        if (! isValidPeer (this))
            return false;

        if (isUp)
        {
            handleMouseEvent (MouseInputSource::InputSourceType::touch, MouseInputSource::offscreenMousePos, ModifierKeys::getCurrentModifiers().withoutMouseButtons(),
                              pressure, orientation, time, {}, touchIndex);

            if (! isValidPeer (this))
                return false;

            if (isCancel)
            {
                currentTouches.clear();
                ModifierKeys::currentModifiers = ModifierKeys::getCurrentModifiers().withoutMouseButtons();
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

            if (! handlePenInput (penInfo, globalToLocal (convertPhysicalScreenPointToLogical (D2DUtilities::toPoint (getPOINTFromLParam (lParam)), hwnd).toFloat()),
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
            ModifierKeys::currentModifiers = ModifierKeys::getCurrentModifiers().withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
        else if ((pInfoFlags & POINTER_FLAG_SECONDBUTTON) != 0)
            ModifierKeys::currentModifiers = ModifierKeys::getCurrentModifiers().withoutMouseButtons().withFlags (ModifierKeys::rightButtonModifier);

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
            ModifierKeys::currentModifiers = ModifierKeys::getCurrentModifiers().withoutMouseButtons();
        }

        handleMouseEvent (MouseInputSource::InputSourceType::pen, pos, modsToSend, pressure,
                          MouseInputSource::defaultOrientation, time, penDetails);

        if (! isValidPeer (this)) // (in case this component was deleted by the event)
            return false;

        if (isUp)
        {
            handleMouseEvent (MouseInputSource::InputSourceType::pen, MouseInputSource::offscreenMousePos, ModifierKeys::getCurrentModifiers(),
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
            if (textChar < ' ' && ModifierKeys::getCurrentModifiers().testFlags (ModifierKeys::ctrlModifier | ModifierKeys::altModifier))
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
        return constrainer != nullptr && ! isKioskMode();
    }

    LRESULT handleSizeConstraining (RECT& r, const WPARAM wParam)
    {
        if (isConstrainedNativeWindow())
        {
            const auto movingTop    = wParam == WMSZ_TOP    || wParam == WMSZ_TOPLEFT    || wParam == WMSZ_TOPRIGHT;
            const auto movingLeft   = wParam == WMSZ_LEFT   || wParam == WMSZ_TOPLEFT    || wParam == WMSZ_BOTTOMLEFT;
            const auto movingBottom = wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT;
            const auto movingRight  = wParam == WMSZ_RIGHT  || wParam == WMSZ_TOPRIGHT   || wParam == WMSZ_BOTTOMRIGHT;

            const auto requestedPhysicalBounds = D2DUtilities::toRectangle (r);
            const auto modifiedPhysicalBounds = getConstrainedBounds (requestedPhysicalBounds,
                                                                      movingTop,
                                                                      movingLeft,
                                                                      movingBottom,
                                                                      movingRight);

            if (! modifiedPhysicalBounds.has_value())
                return TRUE;

            r = D2DUtilities::toRECT (*modifiedPhysicalBounds);
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
                const auto requestedPhysicalBounds = D2DUtilities::toRectangle ({ wp.x, wp.y, wp.x + wp.cx, wp.y + wp.cy });

                if (const auto modifiedPhysicalBounds = getConstrainedBounds (requestedPhysicalBounds, false, false, false, false))
                {
                    wp.x  = modifiedPhysicalBounds->getX();
                    wp.y  = modifiedPhysicalBounds->getY();
                    wp.cx = modifiedPhysicalBounds->getWidth();
                    wp.cy = modifiedPhysicalBounds->getHeight();
                }
            }
        }

        if (((wp.flags & SWP_SHOWWINDOW) != 0 && ! component.isVisible()))
            component.setVisible (true);
        else if (((wp.flags & SWP_HIDEWINDOW) != 0 && component.isVisible()))
            component.setVisible (false);

        return 0;
    }

    std::optional<Rectangle<int>> getConstrainedBounds (Rectangle<int> proposed,
                                                        bool top,
                                                        bool left,
                                                        bool bottom,
                                                        bool right) const
    {
        const auto physicalBorder = findPhysicalBorderSize();

        if (! physicalBorder.has_value())
            return {};

        const auto logicalBorder = getFrameSize();

        // The constrainer expects to operate in logical coordinate space.
        // Additionally, the ComponentPeer can only report the current frame size as an integral
        // number of logical pixels, but at fractional scale factors it may not be possible to
        // express the logical frame size accurately as an integer.
        // To work around this, we replace the physical borders with the currently-reported logical
        // border size before invoking the constrainer.
        // After the constrainer returns, we substitute in the other direction, replacing logical
        // borders with physical.
        const auto requestedPhysicalBounds = proposed;
        const auto requestedPhysicalClient = physicalBorder->subtractedFrom (requestedPhysicalBounds);
        const auto requestedLogicalClient = detail::ScalingHelpers::unscaledScreenPosToScaled (
                component,
                convertPhysicalScreenRectangleToLogical (requestedPhysicalClient, hwnd));
        const auto requestedLogicalBounds = logicalBorder.addedTo (requestedLogicalClient);

        const auto originalLogicalBounds = logicalBorder.addedTo (component.getBounds());

        auto modifiedLogicalBounds = requestedLogicalBounds;

        constrainer->checkBounds (modifiedLogicalBounds,
                                  originalLogicalBounds,
                                  Desktop::getInstance().getDisplays().getTotalBounds (true),
                                  top,
                                  left,
                                  bottom,
                                  right);

        const auto modifiedLogicalClient = logicalBorder.subtractedFrom (modifiedLogicalBounds);
        const auto modifiedPhysicalClient = convertLogicalScreenRectangleToPhysical (
                detail::ScalingHelpers::scaledScreenPosToUnscaled (component, modifiedLogicalClient).toFloat(),
                hwnd);

        const auto closestIntegralSize = modifiedPhysicalClient
                .withPosition (requestedPhysicalClient.getPosition().toFloat())
                .getLargestIntegerWithin();

        const auto withSnappedPosition = [&]
        {
            auto modified = closestIntegralSize;

            if (left || right)
            {
                modified = left ? modified.withRightX (requestedPhysicalClient.getRight())
                                : modified.withX (requestedPhysicalClient.getX());
            }

            if (top || bottom)
            {
                modified = top ? modified.withBottomY (requestedPhysicalClient.getBottom())
                               : modified.withY (requestedPhysicalClient.getY());
            }

            return modified;
        }();

        return physicalBorder->addedTo (withSnappedPosition);
    }

    enum class ForceRefreshDispatcher
    {
        no,
        yes
    };

    static void updateVBlankDispatcherForAllPeers (ForceRefreshDispatcher force = ForceRefreshDispatcher::no)
    {
        // There's an edge case where only top-level windows seem to get WM_SETTINGCHANGE
        // messages, which means that if we have a plugin that opens its own top-level/desktop
        // window, then the extra window might get a SETTINGCHANGE but the plugin window may not.
        // If we only update the vblank dispatcher for windows that get a SETTINGCHANGE, we might
        // miss child windows, and those windows won't be able to repaint.

        for (auto i = getNumPeers(); --i >= 0;)
            if (auto* peer = static_cast<HWNDComponentPeer*> (getPeer (i)))
                peer->updateCurrentMonitorAndRefreshVBlankDispatcher (force);
    }

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
            {
                auto modsToSend = ModifierKeys::getCurrentModifiers();

                if (! Desktop::getInstance().getMainMouseSource().isDragging())
                    modsToSend = modsToSend.withoutMouseButtons();

                doMouseEvent (pos, MouseInputSource::defaultPressure, 0.0f, modsToSend);
            }

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
        if (sendInputAttemptWhenModalMessage())
            return;

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
            return;

        default:
            break;
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

        auto* dispatcher = VBlankDispatcher::getInstance();
        dispatcher->reconfigureDisplays();
        updateVBlankDispatcherForAllPeers (ForceRefreshDispatcher::yes);
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

    Point<float> getLocalPointFromScreenLParam (LPARAM lParam)
    {
        const auto globalPos = D2DUtilities::toPoint (getPOINTFromLParam (lParam));
        return globalToLocal (convertPhysicalScreenPointToLogical (globalPos, hwnd).toFloat());
    }

    Point<float> getPointFromLocalLParam (LPARAM lParam) noexcept
    {
        const auto p = D2DUtilities::toPoint (getPOINTFromLParam (lParam));

        if (! isPerMonitorDPIAwareWindow (hwnd))
            return p.toFloat();

        // LPARAM is relative to this window's top-left but may be on a different monitor so we need to calculate the
        // physical screen position and then convert this to local logical coordinates
        auto r = getWindowScreenRect (hwnd);
        const auto windowBorder = findPhysicalBorderSize().value_or (BorderSize<int>{});
        const auto offset = p
                          + Point { (int) r.left, (int) r.top }
                          + Point { windowBorder.getLeft(), windowBorder.getTop() };
        return globalToLocal (Desktop::getInstance().getDisplays().physicalToLogical (offset).toFloat());
    }

    Point<float> getCurrentMousePos() noexcept
    {
        return globalToLocal (convertPhysicalScreenPointToLogical (D2DUtilities::toPoint (getPOINTFromLParam ((LPARAM) GetMessagePos())), hwnd).toFloat());
    }

    static ModifierKeys getMouseModifiers()
    {
        updateKeyModifiers();

        int mouseMods = 0;
        if (isKeyDown (VK_LBUTTON))  mouseMods |= ModifierKeys::leftButtonModifier;
        if (isKeyDown (VK_RBUTTON))  mouseMods |= ModifierKeys::rightButtonModifier;
        if (isKeyDown (VK_MBUTTON))  mouseMods |= ModifierKeys::middleButtonModifier;

        ModifierKeys::currentModifiers = ModifierKeys::getCurrentModifiers().withoutMouseButtons().withFlags (mouseMods);

        return ModifierKeys::getCurrentModifiers();
    }

    std::optional<LRESULT> onNcLButtonDown (WPARAM wParam, LPARAM lParam)
    {
        handleLeftClickInNCArea (wParam);

        switch (wParam)
        {
            case HTCLOSE:
            case HTMAXBUTTON:
            case HTMINBUTTON:
                // The default implementation in DefWindowProc for these functions has some
                // unwanted behaviour. Specifically, it seems to draw some ugly grey buttons over
                // our custom nonclient area, just for one frame.
                // To avoid this, we handle the message ourselves. The actual handling happens
                // in WM_NCLBUTTONUP.
                return 0;

            case HTCAPTION:
                // The default click-in-caption handler appears to block the message loop until a
                // mouse move is detected, which prevents the view from repainting. We want to keep
                // painting, so log the click ourselves and only defer to DefWindowProc once the
                // mouse moves with the button held.
                captionMouseDown = lParam;
                return 0;
        }

        return {};
    }

    void handleSetFocus()
    {
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
    }

    LRESULT peerWindowProc (HWND h, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
            //==============================================================================
            case WM_NCHITTEST:
            {
                if ((styleFlags & windowIgnoresMouseClicks) != 0)
                    return HTTRANSPARENT;

                if (! hasTitleBar() && (styleFlags & windowIsTemporary) == 0 && parentToAddTo == nullptr)
                {
                    if ((styleFlags & windowIsResizable) != 0)
                        if (const auto result = DefWindowProc (h, message, wParam, lParam); HTSIZEFIRST <= result && result <= HTSIZELAST)
                            return result;

                    const auto physicalPoint = D2DUtilities::toPoint (getPOINTFromLParam (lParam));
                    const auto logicalPoint = convertPhysicalScreenPointToLogical (physicalPoint, hwnd);
                    const auto localPoint = globalToLocal (logicalPoint.toFloat());
                    const auto componentPoint = detail::ScalingHelpers::unscaledScreenPosToScaled (component, localPoint);

                    const auto kind = component.findControlAtPoint (componentPoint);

                    using Kind = Component::WindowControlKind;
                    switch (kind)
                    {
                        case Kind::caption:         return HTCAPTION;
                        case Kind::minimise:        return HTMINBUTTON;
                        case Kind::maximise:        return HTMAXBUTTON;
                        case Kind::close:           return HTCLOSE;
                        case Kind::sizeTop:         return HTTOP;
                        case Kind::sizeLeft:        return HTLEFT;
                        case Kind::sizeRight:       return HTRIGHT;
                        case Kind::sizeBottom:      return HTBOTTOM;
                        case Kind::sizeTopLeft:     return HTTOPLEFT;
                        case Kind::sizeTopRight:    return HTTOPRIGHT;
                        case Kind::sizeBottomLeft:  return HTBOTTOMLEFT;
                        case Kind::sizeBottomRight: return HTBOTTOMRIGHT;

                        case Kind::client:
                            break;
                    }

                    // For a bordered window, Windows would normally let you resize by hovering just
                    // outside the client area (over the drop shadow).
                    // When we disable the border by doing nothing in WM_NCCALCSIZE, the client
                    // size will match the total window size.
                    // It seems that, when there's no nonclient area, Windows won't send us
                    // WM_NCHITTEST when hovering the window shadow.
                    // We only start getting NCHITTEST messages once the cursor is inside the client
                    // area.
                    // The upshot of all this is that we need to emulate the resizable border
                    // ourselves, but inside the window.
                    // Other borderless apps (1Password, Spotify, VS Code) seem to do the same thing,
                    // and if Microsoft's own VS Code doesn't have perfect mouse handling I don't
                    // think we can be expected to either!

                    if ((styleFlags & windowIsResizable) != 0 && ! isKioskMode())
                    {
                        const ScopedThreadDPIAwarenessSetter scope { hwnd };

                        const auto cursor = getPOINTFromLParam (lParam);
                        RECT client{};
                        ::GetWindowRect (h, &client);

                        const auto dpi = GetDpiForWindow (hwnd);
                        const auto padding = GetSystemMetricsForDpi (SM_CXPADDEDBORDER, dpi);
                        const auto borderX = GetSystemMetricsForDpi (SM_CXFRAME, dpi) + padding;
                        const auto borderY = GetSystemMetricsForDpi (SM_CYFRAME, dpi) + padding;

                        const auto left   = cursor.x < client.left + borderX;
                        const auto right  = client.right - borderX < cursor.x;
                        const auto top    = cursor.y < client.top + borderY;
                        const auto bottom = client.bottom - borderY < cursor.y;

                        enum Bits
                        {
                            bitL = 1 << 0,
                            bitR = 1 << 1,
                            bitT = 1 << 2,
                            bitB = 1 << 3,
                        };

                        const auto positionMask = (left   ? bitL : 0)
                                                | (right  ? bitR : 0)
                                                | (top    ? bitT : 0)
                                                | (bottom ? bitB : 0);

                        switch (positionMask)
                        {
                            case bitL: return HTLEFT;
                            case bitR: return HTRIGHT;
                            case bitT: return HTTOP;
                            case bitB: return HTBOTTOM;

                            case bitT | bitL: return HTTOPLEFT;
                            case bitT | bitR: return HTTOPRIGHT;
                            case bitB | bitL: return HTBOTTOMLEFT;
                            case bitB | bitR: return HTBOTTOMRIGHT;
                        }
                    }

                    return HTCLIENT;
                }

                break;
            }

            //==============================================================================
            case WM_PAINT:
                handlePaintMessage();
                return 0;

            case WM_NCPAINT:
                // this must be done, even with native titlebars, or there are rendering artifacts.
                handlePaintMessage();
                // Even if we're *not* using a native titlebar (i.e. extending into the nonclient area)
                // the system needs to handle the NCPAINT to draw rounded corners and shadows.
                break;

            case WM_ERASEBKGND:
                if (hasTitleBar())
                    break;

                return 1;

            case WM_NCCALCSIZE:
            {
                // If we're using the native titlebar, then the default window proc behaviour will
                // do the right thing.
                if (hasTitleBar())
                    break;

                auto* param = (RECT*) lParam;

                // If we're not using a native titlebar, and the window is maximised, then the
                // proposed window may be a bit bigger than the available space. Remove the padding
                // so that the client area exactly fills the available space.
                if (isFullScreen())
                {
                    const auto monitor = MonitorFromWindow (hwnd, MONITOR_DEFAULTTONULL);

                    if (monitor == nullptr)
                        return 0;

                    MONITORINFOEX info{};
                    info.cbSize = sizeof (info);
                    GetMonitorInfo (monitor, &info);

                    const auto padX = info.rcMonitor.left - param->left;
                    const auto padY = info.rcMonitor.top - param->top;

                    param->left  += padX;
                    param->right -= padX;

                    param->top    += padY;
                    param->bottom -= padY;
                }

                return 0;
            }

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
            case WM_NCMOUSEMOVE:
            case WM_MOUSEMOVE:
                return doMouseMove (lParam, false, message == WM_MOUSEMOVE ? WindowArea::client : WindowArea::nonclient).value_or (0);

            case WM_POINTERLEAVE:
            case WM_NCMOUSELEAVE:
            case WM_MOUSELEAVE:
                doMouseExit (message == WM_NCMOUSELEAVE ? WindowArea::nonclient : WindowArea::client);
                return 0;

            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_XBUTTONDOWN:
                doMouseDown (lParam, wParam);
                return 0;

            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
            case WM_XBUTTONUP:
                doMouseUp (getPointFromLocalLParam (lParam), wParam);
                return 0;

            case WM_POINTERWHEEL:
            case WM_MOUSEWHEEL:
                if (doMouseWheel (wParam, true))
                    return 0;
                break;

            case WM_POINTERHWHEEL:
            case WM_MOUSEHWHEEL:
                if (doMouseWheel (wParam, false))
                    return 0;
                break;

            case WM_CAPTURECHANGED:
                doCaptureChanged();
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
            case WM_ENTERSIZEMOVE:
                sizing = true;
                break;

            case WM_EXITSIZEMOVE:
                sizing = false;
                break;

            case WM_SIZING:
                sizing = true;
                return handleSizeConstraining (*(RECT*) lParam, wParam);

            case WM_MOVING:
                return handleSizeConstraining (*(RECT*) lParam, 0);

            case WM_WINDOWPOSCHANGING:
                if (hasTitleBar() && sizing)
                    break;

                return handlePositionChanging (*(WINDOWPOS*) lParam);

            case 0x2e0: /* WM_DPICHANGED */  return handleDPIChanging ((int) HIWORD (wParam), *(RECT*) lParam);

            case WM_WINDOWPOSCHANGED:
            {
                const WINDOWPOS& wPos = *reinterpret_cast<WINDOWPOS*> (lParam);

                if ((wPos.flags & SWP_NOMOVE) != 0 && (wPos.flags & SWP_NOSIZE) != 0)
                    startTimer (100);
                else if (handlePositionChanged())
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
                mouseActivateFlags |= gotSetFocus;

                // If we've received a MOUSEACTIVATE, wait until we've seen the relevant mouse event
                // before updating the focus.
                if ((mouseActivateFlags & gotMouseActivate) != 0)
                    break;

                handleSetFocus();
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
                        if ((peer->getStyleFlags() & windowIsTemporary) != 0)
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
            {
                mouseActivateFlags = 0;

                if (! component.getMouseClickGrabsKeyboardFocus())
                    return MA_NOACTIVATE;

                mouseActivateFlags |= gotMouseActivate;
                break;
            }

            case WM_SHOWWINDOW:
                if (wParam != 0)
                {
                    component.setVisible (true);
                    handleBroughtToFront();

                    if (renderContext != nullptr)
                        renderContext->handleShowWindow();
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

                    PostMessage (h, WM_CLOSE, 0, 0);
                    return 0;

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
                    if (h == GetCapture())
                        ReleaseCapture();

                    break;

                case SC_MAXIMIZE:
                    if (sendInputAttemptWhenModalMessage())
                        return 0;

                    setFullScreen (true);
                    return 0;

                case SC_MINIMIZE:
                    if (sendInputAttemptWhenModalMessage())
                        return 0;

                    setMinimised (true);
                    return 0;

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
                handleLeftClickInNCArea (HIWORD (wParam));
                break;

            case WM_NCLBUTTONDOWN:
            {
                if (auto result = onNcLButtonDown (wParam, lParam))
                    return *result;

                return handleNcMouseEventThenFixModifiers (WM_NCLBUTTONDOWN, wParam, lParam);
            }

            case WM_NCLBUTTONUP:
                switch (wParam)
                {
                    case HTCLOSE:
                        if ((styleFlags & windowHasCloseButton) != 0 && ! sendInputAttemptWhenModalMessage())
                        {
                            if (hasTitleBar())
                                PostMessage (h, WM_CLOSE, 0, 0);
                            else
                                component.windowControlClickedClose();
                        }
                        return 0;

                    case HTMAXBUTTON:
                        if ((styleFlags & windowHasMaximiseButton) != 0 && ! sendInputAttemptWhenModalMessage())
                        {
                            if (hasTitleBar())
                                setFullScreen (! isFullScreen());
                            else
                                component.windowControlClickedMaximise();
                        }
                        return 0;

                    case HTMINBUTTON:
                        if ((styleFlags & windowHasMinimiseButton) != 0 && ! sendInputAttemptWhenModalMessage())
                        {
                            if (hasTitleBar())
                                setMinimised (true);
                            else
                                component.windowControlClickedMinimise();
                        }
                        return 0;
                }
                break;

            case WM_NCRBUTTONDOWN:
            case WM_NCMBUTTONDOWN:
                sendInputAttemptWhenModalMessage();
                return 0;

            case WM_IME_SETCONTEXT:
                imeHandler.handleSetContext (h, wParam == TRUE);
                lParam &= ~(LPARAM) ISC_SHOWUICOMPOSITIONWINDOW;
                break;

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

        return DefWindowProc (h, message, wParam, lParam);
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
                buffer.calloc ((size_t) stringSizeBytes / sizeof (TCHAR) + 1);
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
                const auto screenPos = targetComp->localPointToGlobal (target->getCaretRectangle().getBottomLeft());
                const auto relativePos = peer.globalToLocal (screenPos) * peer.getPlatformScaleFactor();

                CANDIDATEFORM pos { 0, CFS_CANDIDATEPOS, D2DUtilities::toPOINT (relativePos), { 0, 0, 0, 0 } };
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

                // This undocumented bit seems to get set when minimising/maximising windows with Win+D.
                // If we attempt to dismiss modals while this bit is set, we might end up bringing
                // modals to the front, which in turn may attempt to un-minimise them.
                constexpr auto SWP_STATECHANGED = 0x8000;

                if ((windowPosFlags & maskToCheck) == maskToCheck || (windowPosFlags & SWP_STATECHANGED) != 0)
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

    /*  When the user clicks on a window, the window gets sent WM_MOUSEACTIVATE, WM_ACTIVATE,
        and WM_SETFOCUS, before sending a WM_LBUTTONDOWN or other pointer event.
        However, if the WM_SETFOCUS message immediately calls SetFocus to move the focus to a
        different window (e.g. a foreground modal window), then no mouse event will be sent to the
        initially-activated window. This differs from the behaviour on other platforms, where the
        mouse event always reaches the activated window. Failing to emit a mouse event breaks user
        interaction: in the Toolbars pane of the WidgetsDemo, we create a foreground modal
        customisation dialog. The window containing the toolbar is not modal, but still expects to
        receive mouse events so that the toolbar buttons can be dragged around.

        To avoid the system eating the mouse event sent to the initially-activated window, we
        postpone processing the WM_SETFOCUS until *after* the activation mouse event.
    */
    enum MouseActivateFlags : uint8_t
    {
        gotMouseActivate = 1 << 0,
        gotSetFocus      = 1 << 1,
    };
    uint8_t mouseActivateFlags = 0;

    ScopedSuspendResumeNotificationRegistration suspendResumeRegistration;
    std::optional<TimedCallback> monitorUpdateTimer;

    std::unique_ptr<RenderContext> renderContext;
    std::optional<LPARAM> captionMouseDown;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HWNDComponentPeer)
};

ComponentPeer* Component::createNewPeer (int styleFlags, void* parentHWND)
{
    return new HWNDComponentPeer { *this, styleFlags, (HWND) parentHWND, false, 1 };
}

Image createSnapshotOfNativeWindow (void* nativeWindowHandle)
{
    int numDesktopComponents = Desktop::getInstance().getNumComponents();

    for (int index = 0; index < numDesktopComponents; ++index)
    {
        auto component = Desktop::getInstance().getComponent (index);

        if (auto peer = dynamic_cast<HWNDComponentPeer*> (component->getPeer()))
            if (peer->getNativeHandle() == nativeWindowHandle)
                return peer->createSnapshot();
    }

    return {};
}

class GDIRenderContext : public RenderContext
{
public:
    static constexpr auto name = "Software Renderer";

    explicit GDIRenderContext (HWNDComponentPeer& peerIn)
        : peer (peerIn)
    {
        RedrawWindow (peer.getHWND(),
                      nullptr,
                      nullptr,
                      RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
    }

    const char* getName() const override { return name; }

    void updateConstantAlpha() override
    {
        InvalidateRect (peer.getHWND(), nullptr, false);
    }

    void handlePaintMessage() override
    {
        HRGN rgn = CreateRectRgn (0, 0, 0, 0);
        const int regionType = GetUpdateRgn (peer.getHWND(), rgn, false);

        PAINTSTRUCT paintStruct;
        HDC dc = BeginPaint (peer.getHWND(), &paintStruct); // Note this can immediately generate a WM_NCPAINT
                                                            // message and become re-entrant, but that's OK

        // If something in a paint handler calls, e.g. a message box, this can become reentrant and
        // corrupt the image it's using to paint into, so do a check here.
        static bool reentrant = false;

        if (! reentrant)
        {
            const ScopedValueSetter<bool> setter (reentrant, true, false);

            if (peer.dontRepaint)
                peer.getComponent().handleCommandMessage (0); // (this triggers a repaint in the openGL context)
            else
                performPaint (dc, rgn, regionType, paintStruct);
        }

        DeleteObject (rgn);
        EndPaint (peer.getHWND(), &paintStruct);

       #if JUCE_MSVC
        _fpreset(); // because some graphics cards can unmask FP exceptions
       #endif
    }

    void repaint (const Rectangle<int>& area) override
    {
        JUCE_TRACE_EVENT_INT_RECT (etw::repaint, etw::paintKeyword, area);
        deferredRepaints.add (area);
    }

    void dispatchDeferredRepaints() override
    {
        for (auto deferredRect : deferredRepaints)
        {
            auto r = D2DUtilities::toRECT (deferredRect);
            InvalidateRect (peer.getHWND(), &r, FALSE);
        }

        deferredRepaints.clear();
    }

    void performAnyPendingRepaintsNow() override
    {
        if (! peer.getComponent().isVisible())
            return;

        dispatchDeferredRepaints();

        WeakReference localRef (&peer.getComponent());
        MSG m;

        if (peer.getTransparencyKind() == HWNDComponentPeer::TransparencyKind::perPixel
            || PeekMessage (&m, peer.getHWND(), WM_PAINT, WM_PAINT, PM_REMOVE))
        {
            if (localRef != nullptr) // (the PeekMessage call can dispatch messages, which may delete this comp)
                handlePaintMessage();
        }
    }

    Image createSnapshot() override
    {
        return peer.getTransparencyKind() == HWNDComponentPeer::TransparencyKind::perPixel
             ? createSnapshotOfLayeredWindow()
             : createSnapshotOfNormalWindow();
    }

    void onVBlank() override {}

    void handleShowWindow() override {}

private:
    // If we've called UpdateLayeredWindow to display the window contents, retrieving the
    // contents of the window DC will fail.
    // Instead, we produce a fresh render of the window into a temporary image.
    // Child windows will not be included.
    Image createSnapshotOfLayeredWindow() const
    {
        const auto rect = peer.getClientRectInScreen();
        Image result { Image::ARGB, rect.getWidth(), rect.getHeight(), true, SoftwareImageType{} };

        {
            auto context = peer.getComponent()
                               .getLookAndFeel()
                               .createGraphicsContext (result, {}, rect.withZeroOrigin());

            context->addTransform (AffineTransform::scale ((float) peer.getPlatformScaleFactor()));
            peer.handlePaint (*context);
        }

        return result;
    }

    // If UpdateLayeredWindow hasn't been called, then we can blit the window contents directly
    // from the window's DC.
    Image createSnapshotOfNormalWindow() const
    {
        auto hwnd = peer.getHWND();

        auto r = convertPhysicalScreenRectangleToLogical (D2DUtilities::toRectangle (getWindowScreenRect (hwnd)), hwnd);
        const auto w = r.getWidth();
        const auto h = r.getHeight();

        WindowsBitmapImage::Ptr nativeBitmap = new WindowsBitmapImage (Image::RGB, w, h, true);
        Image bitmap (nativeBitmap);

        ScopedDeviceContext deviceContext { hwnd };

        const auto hdc = nativeBitmap->getHDC();

        if (isPerMonitorDPIAwareProcess())
        {
            auto scale = getScaleFactorForWindow (hwnd);
            auto prevStretchMode = SetStretchBltMode (hdc, HALFTONE);
            SetBrushOrgEx (hdc, 0, 0, nullptr);

            StretchBlt (hdc, 0, 0, w, h,
                        deviceContext.dc, 0, 0, roundToInt (w * scale), roundToInt (h * scale),
                        SRCCOPY);

            SetStretchBltMode (hdc, prevStretchMode);
        }
        else
        {
            BitBlt (hdc, 0, 0, w, h, deviceContext.dc, 0, 0, SRCCOPY);
        }

        return SoftwareImageType().convert (bitmap);
    }

    void performPaint (HDC dc, HRGN rgn, int regionType, PAINTSTRUCT& paintStruct)
    {
        int x = paintStruct.rcPaint.left;
        int y = paintStruct.rcPaint.top;
        int w = paintStruct.rcPaint.right - x;
        int h = paintStruct.rcPaint.bottom - y;

        const auto perPixelTransparent = peer.getTransparencyKind() == HWNDComponentPeer::TransparencyKind::perPixel;

        if (perPixelTransparent)
        {
            // it's not possible to have a transparent window with a title bar at the moment!
            jassert (! peer.hasTitleBar());

            auto r = getWindowScreenRect (peer.getHWND());
            x = y = 0;
            w = r.right - r.left;
            h = r.bottom - r.top;
        }

        if (w > 0 && h > 0)
        {
            Image& offscreenImage = offscreenImageGenerator.getImage (perPixelTransparent, w, h);

            RectangleList<int> contextClip;
            const Rectangle<int> clipBounds (w, h);

            bool needToPaintAll = true;

            if (regionType == COMPLEXREGION && ! perPixelTransparent)
            {
                HRGN clipRgn = CreateRectRgnIndirect (&paintStruct.rcPaint);
                CombineRgn (rgn, rgn, clipRgn, RGN_AND);
                DeleteObject (clipRgn);

                alignas (RGNDATA) std::byte rgnData[8192];
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

            ChildWindowClippingInfo childClipInfo = { dc, &peer, &contextClip, Point<int> (x, y), 0 };
            EnumChildWindows (peer.getHWND(), clipChildWindowCallback, (LPARAM) &childClipInfo);

            if (! contextClip.isEmpty())
            {
                if (perPixelTransparent)
                    for (auto& i : contextClip)
                        offscreenImage.clear (i);

                {
                    auto context = peer.getComponent()
                                       .getLookAndFeel()
                                       .createGraphicsContext (offscreenImage, { -x, -y }, contextClip);

                    context->addTransform (AffineTransform::scale ((float) peer.getPlatformScaleFactor()));
                    peer.handlePaint (*context);
                }

                auto* image = static_cast<WindowsBitmapImage*> (offscreenImage.getPixelData().get());

                if (perPixelTransparent)
                {
                    image->updateLayeredWindow (peer.getHWND(), { x, y }, peer.getComponent().getAlpha());
                }
                else
                {
                    image->blitToDC (dc, x, y);

                    if (peer.getTransparencyKind() == HWNDComponentPeer::TransparencyKind::constant)
                        SetLayeredWindowAttributes (peer.getHWND(), {}, (BYTE) (255.0f * peer.getComponent().getAlpha()), LWA_ALPHA);
                }
            }

            if (childClipInfo.savedDC != 0)
                RestoreDC (dc, childClipInfo.savedDC);
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

            if (GetParent (hwnd) == info.peer->getHWND())
            {
                auto clip = D2DUtilities::toRectangle (getWindowClientRect (hwnd));

                info.clip->subtract (clip - info.origin);

                if (info.savedDC == 0)
                    info.savedDC = SaveDC (info.dc);

                ExcludeClipRect (info.dc, clip.getX(), clip.getY(), clip.getRight(), clip.getBottom());
            }
        }

        return TRUE;
    }

    //==============================================================================
    struct TemporaryImage final : private Timer
    {
        TemporaryImage() = default;

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

    HWNDComponentPeer& peer;
    TemporaryImage offscreenImageGenerator;
    RectangleList<int> deferredRepaints;
};

class D2DRenderContext : public RenderContext,
                         private Direct2DHwndContext::SwapchainDelegate
{
public:
    static constexpr auto name = "Direct2D";

    explicit D2DRenderContext (HWNDComponentPeer& peerIn)
        : peer (peerIn)
    {
    }

    const char* getName() const override { return name; }

    void updateConstantAlpha() override
    {
        const auto transparent = peer.getTransparencyKind() != HWNDComponentPeer::TransparencyKind::opaque;

        if (transparent != direct2DContext->supportsTransparency())
        {
            direct2DContext.reset();
            direct2DContext = getContextForPeer (peer, *this);
        }

        if (direct2DContext->supportsTransparency())
            direct2DContext->updateAlpha();
    }

    void handlePaintMessage() override
    {
       #if JUCE_DIRECT2D_METRICS
        auto paintStartTicks = Time::getHighResolutionTicks();
       #endif

        updateRegion.findRECTAndValidate (peer.getHWND());

        for (const auto& rect : updateRegion.getRects())
            repaint (D2DUtilities::toRectangle (rect));

       #if JUCE_DIRECT2D_METRICS
        lastPaintStartTicks = paintStartTicks;
       #endif
    }

    void repaint (const Rectangle<int>& area) override
    {
        direct2DContext->addDeferredRepaint (area);
    }

    void dispatchDeferredRepaints() override {}

    void performAnyPendingRepaintsNow() override {}

    Image createSnapshot() override
    {
        return direct2DContext->createSnapshot();
    }

    void onVBlank() override
    {
        handleDirect2DPaint();
    }

    void handleShowWindow() override
    {
        direct2DContext->handleShowWindow();
        handleDirect2DPaint();
    }

private:
    void onSwapchainEvent() override
    {
        handleDirect2DPaint();
    }

    struct WrappedD2DHwndContextBase
    {
        virtual ~WrappedD2DHwndContextBase() = default;
        virtual void addDeferredRepaint (Rectangle<int> area) = 0;
        virtual Image createSnapshot() const = 0;
        virtual void handleShowWindow() = 0;
        virtual LowLevelGraphicsContext* startFrame (float dpiScale) = 0;
        virtual void endFrame() = 0;
        virtual bool supportsTransparency() const = 0;
        virtual void updateAlpha() = 0;
        virtual Direct2DMetrics::Ptr getMetrics() const = 0;
    };

    /** This is a D2D context that uses a swap chain for presentation.
        D2D contexts that use a swapchain can be made transparent using DirectComposition, but this
        ends up causing other problems in JUCE, such as:
        - The window redirection bitmap also needs to be disabled, which is a permanent window
          setting, so it can't be enabled on the same window - instead a new window needs to be created.
          This means that dynamically changing a component's alpha level at runtime might force the
          window to be recreated, which is not ideal.
        - We can't just disable the redirection bitmap by default, because it's needed to display
          child windows, notably plugin editors
        - The mouse gets captured inside the entire window bounds, rather than just the non-transparent parts

        To avoid these problems, we only use the swapchain to present opaque windows.
        For transparent windows, we use a different technique - see below.
    */
    class WrappedD2DHwndContext : public WrappedD2DHwndContextBase
    {
    public:
        WrappedD2DHwndContext (HWND hwnd, SwapchainDelegate& swapDelegate)
            : ctx (hwnd, swapDelegate) {}

        void addDeferredRepaint (Rectangle<int> area) override
        {
            ctx.addDeferredRepaint (area);
        }

        Image createSnapshot() const override
        {
            return ctx.createSnapshot();
        }

        void handleShowWindow() override
        {
            ctx.handleShowWindow();
        }

        LowLevelGraphicsContext* startFrame (float scale) override
        {
            if (ctx.startFrame (scale))
                return &ctx;

            return nullptr;
        }

        void endFrame() override
        {
            ctx.endFrame();
        }

        bool supportsTransparency() const override
        {
            return false;
        }

        void updateAlpha() override
        {
            // This doesn't support transparency, so updating the alpha won't do anything
            jassertfalse;
        }

        Direct2DMetrics::Ptr getMetrics() const override
        {
            return ctx.metrics;
        }

    private:
        Direct2DHwndContext ctx;
    };

    class DxgiBitmapRenderer
    {
    public:
        LowLevelGraphicsContext* startFrame (HWND hwnd, float scale, const RectangleList<int>& dirty)
        {
            RECT r{};
            GetClientRect (hwnd, &r);

            const auto w = r.right - r.left;
            const auto h = r.bottom - r.top;
            const auto size = D2D1::SizeU ((UINT32) w, (UINT32) h);

            const auto lastAdapter = std::exchange (adapter, directX->adapters.getAdapterForHwnd (hwnd));

            const auto needsNewDC = lastAdapter != adapter || deviceContext == nullptr;

            if (needsNewDC)
            {
                deviceContext = Direct2DDeviceContext::create (adapter);
                bitmap = nullptr;
                context = nullptr;
            }

            if (deviceContext == nullptr)
                return nullptr;

            const auto needsNewBitmap = bitmap == nullptr || ! equal (bitmap->GetPixelSize(), size);

            if (needsNewBitmap)
            {
                bitmap = Direct2DBitmap::createBitmap (deviceContext,
                                                       Image::ARGB,
                                                       size,
                                                       D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE);
                context = nullptr;
            }

            if (bitmap == nullptr)
                return nullptr;

            const auto paintAreas = needsNewBitmap ? Rectangle { (int) w, (int) h } : dirty;

            if (paintAreas.isEmpty())
                return nullptr;

            if (context == nullptr)
                context = std::make_unique<Direct2DImageContext> (deviceContext, bitmap, paintAreas);

            if (! context->startFrame (scale))
                context = nullptr;

            if (context == nullptr)
                return nullptr;

            context->setFill (Colours::transparentBlack);
            context->fillRect ({ (int) size.width, (int) size.height }, true);

            return context.get();
        }

        void endFrame()
        {
            if (context != nullptr)
                context->endFrame();
        }

        Image getImage() const
        {
            return Image { new Direct2DPixelData { adapter->direct2DDevice, bitmap } };
        }

        ComSmartPtr<ID2D1Bitmap1> getBitmap() const
        {
            return bitmap;
        }

        Direct2DMetrics::Ptr getMetrics() const
        {
            if (context != nullptr)
                return context->metrics;

            return {};
        }

    private:
        static constexpr bool equal (D2D1_SIZE_U a, D2D1_SIZE_U b)
        {
            const auto tie = [] (auto& x) { return std::tie (x.width, x.height); };
            return tie (a) == tie (b);
        }

        SharedResourcePointer<DirectX> directX;
        DxgiAdapter::Ptr adapter;
        ComSmartPtr<ID2D1DeviceContext1> deviceContext;
        ComSmartPtr<ID2D1Bitmap1> bitmap;
        std::unique_ptr<Direct2DImageContext> context;
    };

    /*  This wrapper facilitates drawing Direct2D content into a transparent/layered window.

        As an alternative to using DirectComposition, we instead use the older technique of using
        a layered window, and calling UpdateLayeredWindow to set per-pixel alpha on the window.
        This will be slower than going through the swap chain, but means that we can still set
        the alpha level dynamically at runtime, support child windows as before, and support
        per-pixel mouse hit-testing.

        UpdateLayeredWindow is an older API that expects a HDC input containing the image that is
        blitted to the screen. To get an HDC out of Direct2D, we cast a D2D bitmap to IDXGISurface1,
        which exposes a suitable DC. This only works if the target bitmap is constructed with the
        D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE option.
    */
    class WrappedD2DHwndContextTransparent : public WrappedD2DHwndContextBase
    {
    public:
        explicit WrappedD2DHwndContextTransparent (HWNDComponentPeer& p) : peer (p) {}

        void addDeferredRepaint (Rectangle<int> area) override
        {
            deferredRepaints.add (area);
        }

        Image createSnapshot() const override
        {
            DxgiBitmapRenderer renderer;

            if (auto* ctx = renderer.startFrame (peer.getHWND(), (float) peer.getPlatformScaleFactor(), {}))
            {
                peer.handlePaint (*ctx);
                renderer.endFrame();
            }

            return renderer.getImage();
        }

        void handleShowWindow() override {}

        LowLevelGraphicsContext* startFrame (float scale) override
        {
            auto* result = bitmapRenderer.startFrame (peer.getHWND(), scale, deferredRepaints);

            if (result != nullptr)
                deferredRepaints.clear();

            return result;
        }

        void endFrame() override
        {
            bitmapRenderer.endFrame();
            updateLayeredWindow();
        }

        bool supportsTransparency() const override
        {
            return true;
        }

        void updateAlpha() override
        {
            updateLayeredWindow();
        }

        Direct2DMetrics::Ptr getMetrics() const override
        {
            return bitmapRenderer.getMetrics();
        }

    private:
        void updateLayeredWindow()
        {
            const auto bitmap = bitmapRenderer.getBitmap();

            if (bitmap == nullptr)
                return;

            ComSmartPtr<IDXGISurface> surface;
            if (const auto hr = bitmap->GetSurface (surface.resetAndGetPointerAddress());
                FAILED (hr) || surface == nullptr)
            {
                jassertfalse;
                return;
            }

            ComSmartPtr<IDXGISurface1> surface1;
            surface.QueryInterface (surface1);

            if (surface1 == nullptr)
            {
                jassertfalse;
                return;
            }

            HDC hdc{};
            if (const auto hr = surface1->GetDC (false, &hdc); FAILED (hr))
            {
                jassertfalse;
                return;
            }

            const ScopeGuard releaseDC { [&]
                                         {
                                             RECT emptyRect { 0, 0, 0, 0 };
                                             const auto hr = surface1->ReleaseDC (&emptyRect);
                                             jassertquiet (SUCCEEDED (hr));
                                         } };

            if (peer.getTransparencyKind() == HWNDComponentPeer::TransparencyKind::perPixel)
            {
                WindowsBitmapImage::updateLayeredWindow (hdc, peer.getHWND(), {}, peer.getComponent().getAlpha());
            }
            else
            {
                const ScopedDeviceContext scope { peer.getHWND() };
                const auto size = bitmap->GetPixelSize();
                BitBlt (scope.dc, 0, 0, (int) size.width, (int) size.height, hdc, 0, 0, SRCCOPY);

                if (peer.getTransparencyKind() == HWNDComponentPeer::TransparencyKind::constant)
                    SetLayeredWindowAttributes (peer.getHWND(), {}, (BYTE) (255.0f * peer.getComponent().getAlpha()), LWA_ALPHA);
            }
        }

        HWNDComponentPeer& peer;

        DxgiBitmapRenderer bitmapRenderer;
        RectangleList<int> deferredRepaints;
    };

    void handleDirect2DPaint()
    {
       #if JUCE_DIRECT2D_METRICS
        auto paintStartTicks = Time::getHighResolutionTicks();
       #endif

        // Use the ID2D1DeviceContext to paint a swap chain buffer, then tell the swap chain to present
        // the next buffer.
        //
        // Direct2DLowLevelGraphicsContext::startFrame checks if there are any areas to be painted and if the
        // renderer is ready to go; if so, startFrame allocates any needed Direct2D resources,
        // and calls ID2D1DeviceContext::BeginDraw
        //
        // handlePaint() makes various calls into the Direct2DLowLevelGraphicsContext which in turn calls
        // the appropriate ID2D1DeviceContext functions to draw rectangles, clip, set the fill color, etc.
        //
        // Direct2DLowLevelGraphicsContext::endFrame calls ID2D1DeviceContext::EndDraw to finish painting
        // and then tells the swap chain to present the next swap chain back buffer.
        if (auto* ctx = direct2DContext->startFrame ((float) peer.getPlatformScaleFactor()))
        {
            peer.handlePaint (*ctx);
            direct2DContext->endFrame();
        }

       #if JUCE_DIRECT2D_METRICS
        if (lastPaintStartTicks > 0)
        {
            if (auto metrics = direct2DContext->getMetrics())
            {
                metrics->addValueTicks (Direct2DMetrics::messageThreadPaintDuration,
                                        Time::getHighResolutionTicks() - paintStartTicks);
                metrics->addValueTicks (Direct2DMetrics::frameInterval,
                                        paintStartTicks - lastPaintStartTicks);
            }
        }
        lastPaintStartTicks = paintStartTicks;
       #endif
    }

    static std::unique_ptr<WrappedD2DHwndContextBase> getContextForPeer (HWNDComponentPeer& peer,
                                                                         SwapchainDelegate& delegate)
    {
        if (peer.getTransparencyKind() != HWNDComponentPeer::TransparencyKind::opaque)
            return std::make_unique<WrappedD2DHwndContextTransparent> (peer);

        return std::make_unique<WrappedD2DHwndContext> (peer.getHWND(), delegate);
    }

    HWNDComponentPeer& peer;

    std::unique_ptr<WrappedD2DHwndContextBase> direct2DContext = getContextForPeer (peer, *this);
    UpdateRegion updateRegion;

   #if JUCE_ETW_TRACELOGGING
    struct ETWEventProvider
    {
        ETWEventProvider()
        {
            const auto hr = TraceLoggingRegister (::juce::etw::JUCETraceLogProvider);
            jassertquiet (SUCCEEDED (hr));
        }

        ~ETWEventProvider()
        {
            TraceLoggingUnregister (::juce::etw::JUCETraceLogProvider);
        }
    };

    SharedResourcePointer<ETWEventProvider> etwEventProvider;
   #endif

   #if JUCE_DIRECT2D_METRICS
    int64 lastPaintStartTicks = 0;
   #endif
};

using Constructor = std::unique_ptr<RenderContext> (*) (HWNDComponentPeer&);
struct ContextDescriptor
{
    const char* name = nullptr;
    Constructor construct = nullptr;
};

template <typename... T>
inline constexpr ContextDescriptor contextDescriptorList[]
{
    {
        T::name,
        [] (HWNDComponentPeer& p) -> std::unique_ptr<RenderContext> { return std::make_unique<T> (p); }
    }...
};

// To add a new rendering backend, implement RenderContext for that backend, and then append the backend to this typelist
inline constexpr auto& contextDescriptors = contextDescriptorList<GDIRenderContext, D2DRenderContext>;

void HWNDComponentPeer::setCurrentRenderingEngine (int e)
{
    if (isPositiveAndBelow (e, std::size (contextDescriptors)) && (renderContext == nullptr || getCurrentRenderingEngine() != e))
    {
        // Reset the old context before creating the new context, because some context resources
        // can only be created once per window.
        renderContext.reset();
        renderContext = contextDescriptors[e].construct (*this);
    }
}

StringArray HWNDComponentPeer::getAvailableRenderingEngines()
{
    StringArray results;

    for (const auto& d : contextDescriptors)
        results.add (d.name);

    return results;
}

int HWNDComponentPeer::getCurrentRenderingEngine() const
{
    jassert (renderContext != nullptr);

    for (const auto [index, d] : enumerate (contextDescriptors, int{}))
        if (d.name == renderContext->getName())
            return index;

    return -1;
}

JUCE_API ComponentPeer* createNonRepaintingEmbeddedWindowsPeer (Component& component, Component* parentComponent);
JUCE_API ComponentPeer* createNonRepaintingEmbeddedWindowsPeer (Component& component, Component* parentComponent)
{
    if (auto parentPeer = parentComponent->getPeer())
    {
        // Explicitly set the top-level window to software renderer mode in case
        // this is switching from Direct2D to OpenGL
        //
        // HWNDComponentPeer and Direct2DComponentPeer rely on virtual methods for initialization; hence the call to
        // embeddedWindowPeer->initialise() after creating the peer
        int styleFlags = ComponentPeer::windowIgnoresMouseClicks;
        return new HWNDComponentPeer (component,
                                      styleFlags,
                                      (HWND) parentPeer->getNativeHandle(),
                                      true, /* nonRepainting*/
                                      0);
    }

    return nullptr;
}

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

    auto p = D2DUtilities::toPoint (mousePos);

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

    auto point = D2DUtilities::toPOINT (newPositionInt);
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
    if (auto* peer = dynamic_cast<HWNDComponentPeer*> (kioskModeComp->getPeer()))
    {
        const auto prevFlags = (DWORD) GetWindowLong (peer->getHWND(), GWL_STYLE);
        const auto nextVisibility = prevFlags & WS_VISIBLE;
        const auto nextFlags = peer->computeNativeStyleFlags() | nextVisibility;

        if (nextFlags != prevFlags)
        {
            SetWindowLong (peer->getHWND(), GWL_STYLE, nextFlags);

            // After changing the window style flags, the window border visibility may have changed.
            // Call SetWindowPos with no changes other than SWP_FRAMECHANGED to ensure that
            // GetWindowInfo returns up-to-date border-size values.
            static constexpr auto frameChangeOnly = SWP_NOSIZE
                                                  | SWP_NOMOVE
                                                  | SWP_NOZORDER
                                                  | SWP_NOREDRAW
                                                  | SWP_NOACTIVATE
                                                  | SWP_FRAMECHANGED
                                                  | SWP_NOOWNERZORDER
                                                  | SWP_NOSENDCHANGING;
            SetWindowPos (peer->getHWND(), nullptr, 0, 0, 0, 0, frameChangeOnly);
        }
    }
    else
    {
        jassertfalse;
    }

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

            return Desktop::getInstance().getDisplays().physicalToLogical (D2DUtilities::toRectangle (getWindowScreenRect (hwnd)));
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
    MonitorInfo (bool main, RECT totalArea, RECT workArea, double d, std::optional<double> frequency) noexcept
        : isMain (main),
          totalAreaRect (totalArea),
          workAreaRect (workArea),
          dpi (d),
          verticalFrequencyHz (frequency)
    {
    }

    bool isMain;
    RECT totalAreaRect, workAreaRect;
    double dpi;
    std::optional<double> verticalFrequencyHz;
};

static BOOL CALLBACK enumMonitorsProc (HMONITOR hm, HDC, LPRECT, LPARAM userInfo)
{
    MONITORINFOEX info = {};
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

    // Call EnumDisplayDevices and EnumDisplaySettings to get the refresh rate of the monitor
    BOOL ok = TRUE;
    std::optional<double> frequency;
    for (uint32 deviceNumber = 0; ok; ++deviceNumber)
    {
        DISPLAY_DEVICEW displayDevice{};
        displayDevice.cb = sizeof (displayDevice);
        ok = EnumDisplayDevicesW (nullptr, deviceNumber, &displayDevice, 0);
        if (ok && (displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP))
        {
            DEVMODE displaySettings{};
            ok = EnumDisplaySettingsW (displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &displaySettings);
            if (ok)
            {
                if (String { displayDevice.DeviceName } == String { info.szDevice })
                {
                    frequency = (double) displaySettings.dmDisplayFrequency;
                    break;
                }
            }
        }
    }

    ((Array<MonitorInfo>*) userInfo)->add ({ isMain, info.rcMonitor, info.rcWork, dpi, frequency });
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
        monitors.add ({ true, windowRect, windowRect, globalDPI, std::optional<double>{} });
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

        d.totalArea = D2DUtilities::toRectangle (monitor.totalAreaRect);
        d.userArea  = D2DUtilities::toRectangle (monitor.workAreaRect);

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

    ComSmartPtr<JuceIVirtualDesktopManager> manager;

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
    manager.CoCreateInstance (__uuidof (JuceVirtualDesktopManager), CLSCTX_ALL);
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

    if (manager == nullptr)
        return true;

    BOOL current = false;

    if (FAILED (manager->IsWindowOnCurrentVirtualDesktop (static_cast<HWND> (x), &current)))
        return true;

    return current != false;
}
