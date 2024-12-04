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

namespace juce
{

namespace X11SymbolHelpers
{

template <typename FuncPtr>
struct SymbolBinding
{
    FuncPtr& func;
    const char* name;
};

template <typename FuncPtr>
SymbolBinding<FuncPtr> makeSymbolBinding (FuncPtr& func, const char* name)
{
    return { func, name };
}

template <typename FuncPtr>
bool loadSymbols (DynamicLibrary& lib, SymbolBinding<FuncPtr> binding)
{
    if (auto* func = lib.getFunction (binding.name))
    {
        binding.func = reinterpret_cast<FuncPtr> (func);
        return true;
    }

    return false;
}

template <typename FuncPtr, typename... Args>
bool loadSymbols (DynamicLibrary& lib1, DynamicLibrary& lib2, SymbolBinding<FuncPtr> binding)
{
    return loadSymbols (lib1, binding) || loadSymbols (lib2, binding);
}

template <typename FuncPtr, typename... Args>
bool loadSymbols (DynamicLibrary& lib, SymbolBinding<FuncPtr> binding, Args... args)
{
    return loadSymbols (lib, binding) && loadSymbols (lib, args...);
}

template <typename FuncPtr, typename... Args>
bool loadSymbols (DynamicLibrary& lib1, DynamicLibrary& lib2, SymbolBinding<FuncPtr> binding, Args... args)
{
    return loadSymbols (lib1, lib2, binding) && loadSymbols (lib1, lib2, args...);
}

}

//==============================================================================
bool X11Symbols::loadAllSymbols()
{
    using namespace X11SymbolHelpers;

    if (! loadSymbols (xLib, xextLib,
                       makeSymbolBinding (xAllocClassHint,             "XAllocClassHint"),
                       makeSymbolBinding (xAllocSizeHints,             "XAllocSizeHints"),
                       makeSymbolBinding (xAllocWMHints,               "XAllocWMHints"),
                       makeSymbolBinding (xBitmapBitOrder,             "XBitmapBitOrder"),
                       makeSymbolBinding (xBitmapUnit,                 "XBitmapUnit"),
                       makeSymbolBinding (xChangeActivePointerGrab,    "XChangeActivePointerGrab"),
                       makeSymbolBinding (xChangeProperty,             "XChangeProperty"),
                       makeSymbolBinding (xCheckTypedWindowEvent,      "XCheckTypedWindowEvent"),
                       makeSymbolBinding (xCheckWindowEvent,           "XCheckWindowEvent"),
                       makeSymbolBinding (xClearArea,                  "XClearArea"),
                       makeSymbolBinding (xCloseDisplay,               "XCloseDisplay"),
                       makeSymbolBinding (xConnectionNumber,           "XConnectionNumber"),
                       makeSymbolBinding (xConvertSelection,           "XConvertSelection"),
                       makeSymbolBinding (xCreateColormap,             "XCreateColormap"),
                       makeSymbolBinding (xCreateFontCursor,           "XCreateFontCursor"),
                       makeSymbolBinding (xCreateGC,                   "XCreateGC"),
                       makeSymbolBinding (xCreateImage,                "XCreateImage"),
                       makeSymbolBinding (xCreatePixmap,               "XCreatePixmap"),
                       makeSymbolBinding (xCreatePixmapCursor,         "XCreatePixmapCursor"),
                       makeSymbolBinding (xCreatePixmapFromBitmapData, "XCreatePixmapFromBitmapData"),
                       makeSymbolBinding (xCreateWindow,               "XCreateWindow"),
                       makeSymbolBinding (xDefaultRootWindow,          "XDefaultRootWindow"),
                       makeSymbolBinding (xDefaultScreen,              "XDefaultScreen"),
                       makeSymbolBinding (xDefaultScreenOfDisplay,     "XDefaultScreenOfDisplay"),
                       makeSymbolBinding (xDefaultVisual,              "XDefaultVisual"),
                       makeSymbolBinding (xDefineCursor,               "XDefineCursor"),
                       makeSymbolBinding (xDeleteContext,              "XDeleteContext"),
                       makeSymbolBinding (xDeleteProperty,             "XDeleteProperty"),
                       makeSymbolBinding (xDestroyImage,               "XDestroyImage"),
                       makeSymbolBinding (xDestroyWindow,              "XDestroyWindow"),
                       makeSymbolBinding (xDisplayHeight,              "XDisplayHeight"),
                       makeSymbolBinding (xDisplayHeightMM,            "XDisplayHeightMM"),
                       makeSymbolBinding (xDisplayWidth,               "XDisplayWidth"),
                       makeSymbolBinding (xDisplayWidthMM,             "XDisplayWidthMM"),
                       makeSymbolBinding (xEventsQueued,               "XEventsQueued"),
                       makeSymbolBinding (xFindContext,                "XFindContext"),
                       makeSymbolBinding (xFlush,                      "XFlush"),
                       makeSymbolBinding (xFree,                       "XFree"),
                       makeSymbolBinding (xFreeCursor,                 "XFreeCursor"),
                       makeSymbolBinding (xFreeColormap,               "XFreeColormap"),
                       makeSymbolBinding (xFreeGC,                     "XFreeGC"),
                       makeSymbolBinding (xFreeModifiermap,            "XFreeModifiermap"),
                       makeSymbolBinding (xFreePixmap,                 "XFreePixmap"),
                       makeSymbolBinding (xGetAtomName,                "XGetAtomName"),
                       makeSymbolBinding (xGetErrorDatabaseText,       "XGetErrorDatabaseText"),
                       makeSymbolBinding (xGetErrorText,               "XGetErrorText"),
                       makeSymbolBinding (xGetGeometry,                "XGetGeometry"),
                       makeSymbolBinding (xGetImage,                   "XGetImage"),
                       makeSymbolBinding (xGetInputFocus,              "XGetInputFocus"),
                       makeSymbolBinding (xGetModifierMapping,         "XGetModifierMapping"),
                       makeSymbolBinding (xGetPointerMapping,          "XGetPointerMapping"),
                       makeSymbolBinding (xGetSelectionOwner,          "XGetSelectionOwner"),
                       makeSymbolBinding (xGetVisualInfo,              "XGetVisualInfo"),
                       makeSymbolBinding (xGetWMHints,                 "XGetWMHints"),
                       makeSymbolBinding (xGetWindowAttributes,        "XGetWindowAttributes"),
                       makeSymbolBinding (xGetWindowProperty,          "XGetWindowProperty"),
                       makeSymbolBinding (xGrabPointer,                "XGrabPointer"),
                       makeSymbolBinding (xGrabServer,                 "XGrabServer"),
                       makeSymbolBinding (xImageByteOrder,             "XImageByteOrder"),
                       makeSymbolBinding (xInitImage,                  "XInitImage"),
                       makeSymbolBinding (xInitThreads,                "XInitThreads"),
                       makeSymbolBinding (xInstallColormap,            "XInstallColormap"),
                       makeSymbolBinding (xInternAtom,                 "XInternAtom"),
                       makeSymbolBinding (xkbKeycodeToKeysym,          "XkbKeycodeToKeysym"),
                       makeSymbolBinding (xKeysymToKeycode,            "XKeysymToKeycode"),
                       makeSymbolBinding (xListProperties,             "XListProperties"),
                       makeSymbolBinding (xLockDisplay,                "XLockDisplay"),
                       makeSymbolBinding (xLookupString,               "XLookupString"),
                       makeSymbolBinding (xMapRaised,                  "XMapRaised"),
                       makeSymbolBinding (xMapWindow,                  "XMapWindow"),
                       makeSymbolBinding (xMoveResizeWindow,           "XMoveResizeWindow"),
                       makeSymbolBinding (xNextEvent,                  "XNextEvent"),
                       makeSymbolBinding (xOpenDisplay,                "XOpenDisplay"),
                       makeSymbolBinding (xPeekEvent,                  "XPeekEvent"),
                       makeSymbolBinding (xPending,                    "XPending"),
                       makeSymbolBinding (xPutImage,                   "XPutImage"),
                       makeSymbolBinding (xPutPixel,                   "XPutPixel"),
                       makeSymbolBinding (xQueryBestCursor,            "XQueryBestCursor"),
                       makeSymbolBinding (xQueryExtension,             "XQueryExtension"),
                       makeSymbolBinding (xQueryPointer,               "XQueryPointer"),
                       makeSymbolBinding (xQueryTree,                  "XQueryTree"),
                       makeSymbolBinding (xRefreshKeyboardMapping,     "XRefreshKeyboardMapping"),
                       makeSymbolBinding (xReparentWindow,             "XReparentWindow"),
                       makeSymbolBinding (xResizeWindow,               "XResizeWindow"),
                       makeSymbolBinding (xRestackWindows,             "XRestackWindows"),
                       makeSymbolBinding (xRootWindow,                 "XRootWindow"),
                       makeSymbolBinding (xSaveContext,                "XSaveContext"),
                       makeSymbolBinding (xScreenCount,                "XScreenCount"),
                       makeSymbolBinding (xScreenNumberOfScreen,       "XScreenNumberOfScreen"),
                       makeSymbolBinding (xSelectInput,                "XSelectInput"),
                       makeSymbolBinding (xSendEvent,                  "XSendEvent"),
                       makeSymbolBinding (xSetClassHint,               "XSetClassHint"),
                       makeSymbolBinding (xSetErrorHandler,            "XSetErrorHandler"),
                       makeSymbolBinding (xSetIOErrorHandler,          "XSetIOErrorHandler"),
                       makeSymbolBinding (xSetInputFocus,              "XSetInputFocus"),
                       makeSymbolBinding (xSetSelectionOwner,          "XSetSelectionOwner"),
                       makeSymbolBinding (xSetWMHints,                 "XSetWMHints"),
                       makeSymbolBinding (xSetWMIconName,              "XSetWMIconName"),
                       makeSymbolBinding (xSetWMName,                  "XSetWMName"),
                       makeSymbolBinding (xSetWMNormalHints,           "XSetWMNormalHints"),
                       makeSymbolBinding (xStringListToTextProperty,   "XStringListToTextProperty"),
                       makeSymbolBinding (xSync,                       "XSync"),
                       makeSymbolBinding (xSynchronize,                "XSynchronize"),
                       makeSymbolBinding (xTranslateCoordinates,       "XTranslateCoordinates"),
                       makeSymbolBinding (xrmUniqueQuark,              "XrmUniqueQuark"),
                       makeSymbolBinding (xUngrabPointer,              "XUngrabPointer"),
                       makeSymbolBinding (xUngrabServer,               "XUngrabServer"),
                       makeSymbolBinding (xUnlockDisplay,              "XUnlockDisplay"),
                       makeSymbolBinding (xUnmapWindow,                "XUnmapWindow"),
                       makeSymbolBinding (xutf8TextListToTextProperty, "Xutf8TextListToTextProperty"),
                       makeSymbolBinding (xWarpPointer,                "XWarpPointer")))
        return false;

   #if JUCE_USE_XCURSOR
    loadSymbols (xcursorLib,
                 makeSymbolBinding (xcursorImageCreate,          "XcursorImageCreate"),
                 makeSymbolBinding (xcursorImageLoadCursor,      "XcursorImageLoadCursor"),
                 makeSymbolBinding (xcursorImageDestroy,         "XcursorImageDestroy"));
   #endif
   #if JUCE_USE_XINERAMA
    loadSymbols (xineramaLib,
                 makeSymbolBinding (xineramaIsActive,            "XineramaIsActive"),
                 makeSymbolBinding (xineramaQueryScreens,        "XineramaQueryScreens"));
   #endif
   #if JUCE_USE_XRENDER
    loadSymbols (xrenderLib,
                 makeSymbolBinding (xRenderQueryVersion,         "XRenderQueryVersion"),
                 makeSymbolBinding (xRenderFindStandardFormat,   "XRenderFindStandardFormat"),
                 makeSymbolBinding (xRenderFindFormat,           "XRenderFindFormat"),
                 makeSymbolBinding (xRenderFindVisualFormat,     "XRenderFindVisualFormat"));
   #endif
   #if JUCE_USE_XRANDR
    loadSymbols (xrandrLib,
                 makeSymbolBinding (xRRGetScreenResources,       "XRRGetScreenResources"),
                 makeSymbolBinding (xRRFreeScreenResources,      "XRRFreeScreenResources"),
                 makeSymbolBinding (xRRGetOutputInfo,            "XRRGetOutputInfo"),
                 makeSymbolBinding (xRRFreeOutputInfo,           "XRRFreeOutputInfo"),
                 makeSymbolBinding (xRRGetCrtcInfo,              "XRRGetCrtcInfo"),
                 makeSymbolBinding (xRRFreeCrtcInfo,             "XRRFreeCrtcInfo"),
                 makeSymbolBinding (xRRGetOutputPrimary,         "XRRGetOutputPrimary"));
   #endif
   #if JUCE_USE_XSHM
    loadSymbols (xLib, xextLib,
                 makeSymbolBinding (xShmAttach,                  "XShmAttach"),
                 makeSymbolBinding (xShmCreateImage,             "XShmCreateImage"),
                 makeSymbolBinding (xShmDetach,                  "XShmDetach"),
                 makeSymbolBinding (xShmGetEventBase,            "XShmGetEventBase"),
                 makeSymbolBinding (xShmPutImage,                "XShmPutImage"),
                 makeSymbolBinding (xShmQueryVersion,            "XShmQueryVersion"));
   #endif

    return true;
}

} // namespace juce
