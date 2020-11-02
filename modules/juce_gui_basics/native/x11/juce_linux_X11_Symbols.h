/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

//==============================================================================
namespace ReturnHelpers
{
    template<typename Type>
    Type returnDefaultConstructedAnyType()               { return {}; }

    template<>
    inline void returnDefaultConstructedAnyType<void>()  {}
}

#define JUCE_GENERATE_FUNCTION_WITH_DEFAULT(functionName, objectName, args, returnType) \
    using functionName      = returnType (*) args; \
    functionName objectName = [] args -> returnType  { return ReturnHelpers::returnDefaultConstructedAnyType<returnType>(); };


//==============================================================================
class JUCE_API  X11Symbols
{
public:
    //==============================================================================
    bool loadAllSymbols();

    //==============================================================================
    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XAllocClassHint, xAllocClassHint,
                                         (),
                                         XClassHint*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XAllocSizeHints, xAllocSizeHints,
                                         (),
                                         XSizeHints*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XAllocWMHints, xAllocWMHints,
                                         (),
                                         XWMHints*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XBitmapBitOrder, xBitmapBitOrder,
                                         (::Display*),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XBitmapUnit, xBitmapUnit,
                                         (::Display*),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XChangeActivePointerGrab, xChangeActivePointerGrab,
                                         (::Display*, unsigned int, Cursor, ::Time),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XChangeProperty, xChangeProperty,
                                         (::Display*, ::Window, Atom, Atom, int, int, const unsigned char*, int),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XCheckTypedWindowEvent, xCheckTypedWindowEvent,
                                         (::Display*, ::Window, int, XEvent*),
                                         Bool)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XCheckWindowEvent, xCheckWindowEvent,
                                         (::Display*, ::Window, long, XEvent*),
                                         Bool)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XClearArea, xClearArea,
                                         (::Display*, ::Window, int, int, unsigned int, unsigned int, Bool),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XCloseDisplay, xCloseDisplay,
                                         (::Display*),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XConnectionNumber, xConnectionNumber,
                                         (::Display*),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XConvertSelection, xConvertSelection,
                                         (::Display*, Atom, Atom, Atom, ::Window, ::Time),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XCreateColormap, xCreateColormap,
                                         (::Display*, ::Window, Visual*, int),
                                         Colormap)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XCreateFontCursor, xCreateFontCursor,
                                         (::Display*, unsigned int),
                                         Cursor)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XCreateGC, xCreateGC,
                                         (::Display*, ::Drawable, unsigned long, XGCValues*),
                                         GC)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XCreateImage, xCreateImage,
                                         (::Display*, Visual*, unsigned int, int, int, const char*, unsigned int, unsigned int, int, int),
                                         XImage*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XCreatePixmap, xCreatePixmap,
                                         (::Display*, ::Drawable, unsigned int, unsigned int, unsigned int),
                                         Pixmap)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XCreatePixmapCursor, xCreatePixmapCursor,
                                         (::Display*, Pixmap, Pixmap, XColor*, XColor*, unsigned int, unsigned int),
                                         Cursor)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XCreatePixmapFromBitmapData, xCreatePixmapFromBitmapData,
                                         (::Display*, ::Drawable, const char*, unsigned int, unsigned int, unsigned long, unsigned long, unsigned int),
                                         Pixmap)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XCreateWindow, xCreateWindow,
                                         (::Display*, ::Window, int, int, unsigned int, unsigned int, unsigned int, int, unsigned int, Visual*, unsigned long, XSetWindowAttributes*),
                                         ::Window)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XDefaultRootWindow, xDefaultRootWindow,
                                         (::Display*),
                                         ::Window)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XDefaultScreen, xDefaultScreen,
                                         (::Display*),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XDefaultScreenOfDisplay, xDefaultScreenOfDisplay,
                                         (::Display*),
                                         Screen*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XDefaultVisual, xDefaultVisual,
                                         (::Display*, int),
                                         Visual*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XDefineCursor, xDefineCursor,
                                         (::Display*, ::Window, Cursor),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XDeleteContext, xDeleteContext,
                                         (::Display*, XID, XContext),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XDeleteProperty, xDeleteProperty,
                                         (::Display*, Window, Atom),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XDestroyImage, xDestroyImage,
                                         (XImage*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XDestroyWindow, xDestroyWindow,
                                         (::Display*, ::Window),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XDisplayHeight, xDisplayHeight,
                                         (::Display*, int),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XDisplayHeightMM, xDisplayHeightMM,
                                         (::Display*, int),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XDisplayWidth, xDisplayWidth,
                                         (::Display*, int),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XDisplayWidthMM, xDisplayWidthMM,
                                         (::Display*, int),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XEventsQueued, xEventsQueued,
                                         (::Display*, int),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XFindContext, xFindContext,
                                         (::Display*, XID, XContext, XPointer*),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XFlush, xFlush,
                                         (::Display*),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XFree, xFree,
                                         (void*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XFreeCursor, xFreeCursor,
                                         (::Display*, Cursor),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XFreeColormap ,xFreeColormap,
                                         (::Display*, Colormap),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XFreeGC, xFreeGC,
                                         (::Display*, GC),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XFreeModifiermap, xFreeModifiermap,
                                         (XModifierKeymap*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XFreePixmap, xFreePixmap,
                                         (::Display*, Pixmap),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XGetAtomName, xGetAtomName,
                                         (::Display*, Atom),
                                         char*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XGetErrorDatabaseText, xGetErrorDatabaseText,
                                         (::Display*, const char*, const char*, const char*, const char*, int),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XGetErrorText, xGetErrorText,
                                         (::Display*, int, const char*, int),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XGetGeometry, xGetGeometry,
                                         (::Display*, ::Drawable, ::Window*, int*, int*, unsigned int*, unsigned int*, unsigned int*, unsigned int*),
                                         Status)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XGetInputFocus, xGetInputFocus,
                                         (::Display*, ::Window*, int*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XGetModifierMapping, xGetModifierMapping,
                                         (::Display*),
                                         XModifierKeymap*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XGetPointerMapping, xGetPointerMapping,
                                         (::Display*, unsigned char[], int),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XGetSelectionOwner, xGetSelectionOwner,
                                         (::Display*, Atom),
                                         ::Window)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XGetVisualInfo, xGetVisualInfo,
                                         (::Display*, long, XVisualInfo*, int*),
                                         XVisualInfo*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XGetWMHints, xGetWMHints,
                                         (::Display*, ::Window),
                                         XWMHints*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XGetWindowAttributes, xGetWindowAttributes,
                                         (::Display*, ::Window, XWindowAttributes*),
                                         Status)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XGetWindowProperty, xGetWindowProperty,
                                         (::Display*, ::Window, Atom, long, long, Bool, Atom, Atom*, int*, unsigned long*, unsigned long*, unsigned char**),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XGrabPointer, xGrabPointer,
                                         (::Display*, ::Window, Bool, unsigned int, int, int, ::Window, Cursor, ::Time),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XGrabServer, xGrabServer,
                                         (::Display*),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XImageByteOrder, xImageByteOrder,
                                         (::Display*),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XInitImage, xInitImage,
                                         (XImage*),
                                         Status)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XInitThreads, xInitThreads,
                                         (),
                                         Status)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XInstallColormap, xInstallColormap,
                                         (::Display*, Colormap),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XInternAtom, xInternAtom,
                                         (::Display*, const char*, Bool),
                                         Atom)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XkbKeycodeToKeysym, xkbKeycodeToKeysym,
                                         (::Display*, KeyCode, unsigned int, unsigned int),
                                         KeySym)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XKeysymToKeycode, xKeysymToKeycode,
                                         (::Display*, KeySym),
                                         KeyCode)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XListProperties, xListProperties,
                                         (::Display*, Window, int*),
                                         Atom*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XLockDisplay, xLockDisplay,
                                         (::Display*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XLookupString, xLookupString,
                                         (XKeyEvent*, const char*, int, KeySym*, XComposeStatus*),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XMapRaised, xMapRaised,
                                         (::Display*, ::Window),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XMapWindow, xMapWindow,
                                         (::Display*, ::Window),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XMoveResizeWindow, xMoveResizeWindow,
                                         (::Display*, ::Window, int, int, unsigned int, unsigned int),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XNextEvent, xNextEvent,
                                         (::Display*, XEvent*),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XOpenDisplay, xOpenDisplay,
                                         (const char*),
                                         ::Display*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XPeekEvent, xPeekEvent,
                                         (::Display*, XEvent*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XPending, xPending,
                                         (::Display*),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XPutImage, xPutImage,
                                         (::Display*, ::Drawable, GC, XImage*, int, int, int, int, unsigned int, unsigned int),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XPutPixel, xPutPixel,
                                         (XImage*, int, int, unsigned long),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XQueryBestCursor, xQueryBestCursor,
                                         (::Display*, ::Drawable, unsigned int, unsigned int, unsigned int*, unsigned int*),
                                         Status)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XQueryExtension, xQueryExtension,
                                         (::Display*, const char*, int*, int*, int*),
                                         Bool)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XQueryPointer, xQueryPointer,
                                         (::Display*, ::Window, ::Window*, ::Window*, int*, int*, int*, int*, unsigned int*),
                                         Bool)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XQueryTree, xQueryTree,
                                         (::Display*, ::Window, ::Window*, ::Window*, ::Window**, unsigned int*),
                                         Status)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XRefreshKeyboardMapping, xRefreshKeyboardMapping,
                                         (XMappingEvent*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XReparentWindow, xReparentWindow,
                                         (::Display*, ::Window, ::Window, int, int),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XResizeWindow, xResizeWindow,
                                         (::Display*, Window, unsigned int, unsigned int),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XRestackWindows, xRestackWindows,
                                         (::Display*, ::Window[], int),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XRootWindow, xRootWindow,
                                         (::Display*, int),
                                         ::Window)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XSaveContext, xSaveContext,
                                         (::Display*, XID, XContext, XPointer),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XScreenCount, xScreenCount,
                                         (::Display*),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XScreenNumberOfScreen, xScreenNumberOfScreen,
                                         (Screen*),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XSelectInput, xSelectInput,
                                         (::Display*, ::Window, long),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XSendEvent, xSendEvent,
                                         (::Display*, ::Window, Bool, long, XEvent*),
                                         Status)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XSetClassHint, xSetClassHint,
                                         (::Display*, ::Window, XClassHint*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XSetErrorHandler, xSetErrorHandler,
                                         (XErrorHandler),
                                         XErrorHandler)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XSetIOErrorHandler, xSetIOErrorHandler,
                                         (XIOErrorHandler),
                                         XIOErrorHandler)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XSetInputFocus, xSetInputFocus,
                                         (::Display*, ::Window, int, ::Time),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XSetSelectionOwner, xSetSelectionOwner,
                                         (::Display*, Atom, ::Window, ::Time),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XSetWMHints, xSetWMHints,
                                         (::Display*, ::Window, XWMHints*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XSetWMIconName, xSetWMIconName,
                                         (::Display*, ::Window, XTextProperty*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XSetWMName, xSetWMName,
                                         (::Display*, ::Window, XTextProperty*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XSetWMNormalHints, xSetWMNormalHints,
                                         (::Display*, ::Window, XSizeHints*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XStringListToTextProperty, xStringListToTextProperty,
                                         (char**, int, XTextProperty*),
                                         Status)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XSync, xSync,
                                         (::Display*, Bool),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XSynchronize, xSynchronize,
                                         (::Display*, Bool),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XTranslateCoordinates, xTranslateCoordinates,
                                         (::Display*, ::Window, ::Window, int, int, int*, int*, ::Window*),
                                         Bool)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XrmUniqueQuark, xrmUniqueQuark,
                                         (),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XUngrabPointer, xUngrabPointer,
                                         (::Display*, ::Time),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XUngrabServer, xUngrabServer,
                                         (::Display*),
                                         int)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XUnlockDisplay, xUnlockDisplay,
                                         (::Display*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XUnmapWindow, xUnmapWindow,
                                         (::Display*, ::Window),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XWarpPointer, xWarpPointer,
                                         (::Display*, ::Window, ::Window, int, int, unsigned int, unsigned int, int, int),
                                         void)
   #if JUCE_USE_XCURSOR
    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XcursorImageCreate, xcursorImageCreate,
                                         (int, int),
                                         XcursorImage*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XcursorImageLoadCursor, xcursorImageLoadCursor,
                                         (::Display*, XcursorImage*),
                                         Cursor)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XcursorImageDestroy, xcursorImageDestroy,
                                         (XcursorImage*),
                                         void)
   #endif
   #if JUCE_USE_XINERAMA
    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XineramaIsActive, xineramaIsActive,
                                         (::Display*),
                                         Bool)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XineramaQueryScreens, xineramaQueryScreens,
                                         (::Display*, int*),
                                         XineramaScreenInfo*)
   #endif
   #if JUCE_USE_XRENDER
    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XRenderQueryVersion, xRenderQueryVersion,
                                         (::Display*, int*, int*),
                                         Status)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XRenderFindStandardFormat, xRenderFindStandardFormat,
                                         (Display*, int),
                                         XRenderPictFormat*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XRenderFindFormat, xRenderFindFormat,
                                         (Display*, unsigned long, XRenderPictFormat*, int),
                                         XRenderPictFormat*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XRenderFindVisualFormat, xRenderFindVisualFormat,
                                         (Display*, Visual*),
                                         XRenderPictFormat*)
   #endif
   #if JUCE_USE_XRANDR
    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XRRGetScreenResources, xRRGetScreenResources,
                                         (::Display*, Window),
                                         XRRScreenResources*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XRRFreeScreenResources, xRRFreeScreenResources,
                                         (XRRScreenResources*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XRRGetOutputInfo, xRRGetOutputInfo,
                                         (::Display*, XRRScreenResources*, RROutput),
                                         XRROutputInfo*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XRRFreeOutputInfo, xRRFreeOutputInfo,
                                         (XRROutputInfo*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XRRGetCrtcInfo, xRRGetCrtcInfo,
                                         (::Display*, XRRScreenResources*, RRCrtc),
                                         XRRCrtcInfo*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XRRFreeCrtcInfo, xRRFreeCrtcInfo,
                                         (XRRCrtcInfo*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XRRGetOutputPrimary, xRRGetOutputPrimary,
                                         (::Display*, ::Window),
                                         RROutput)
   #endif
   #if JUCE_USE_XSHM
    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XShmAttach, xShmAttach,
                                         (::Display*, XShmSegmentInfo*),
                                         Bool)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XShmCreateImage, xShmCreateImage,
                                         (::Display*, Visual*, unsigned int, int, const char*, XShmSegmentInfo*, unsigned int, unsigned int),
                                         XImage*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XShmDetach, xShmDetach,
                                         (::Display*, XShmSegmentInfo*),
                                         Bool)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XShmGetEventBase, xShmGetEventBase,
                                         (::Display*),
                                         Status)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XShmPutImage, xShmPutImage,
                                         (::Display*, ::Drawable, GC, XImage*, int, int, int, int, unsigned int, unsigned int, bool),
                                         Bool)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (XShmQueryVersion, xShmQueryVersion,
                                         (::Display*, int*, int*, Bool*),
                                         Bool)
   #endif

    //==============================================================================
    JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL (X11Symbols)

private:
    X11Symbols() = default;

    ~X11Symbols()
    {
        clearSingletonInstance();
    }

    //==============================================================================
    DynamicLibrary xLib { "libX11.so.6" }, xextLib { "libXext.so.6" };

   #if JUCE_USE_XCURSOR
    DynamicLibrary xcursorLib  { "libXcursor.so.1" };
   #endif
   #if JUCE_USE_XINERAMA
    DynamicLibrary xineramaLib { "libXinerama.so.1" };
   #endif
   #if JUCE_USE_XRENDER
    DynamicLibrary xrenderLib  { "libXrender.so.1" };
   #endif
   #if JUCE_USE_XRANDR
    DynamicLibrary xrandrLib   { "libXrandr.so.2" };
   #endif

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (X11Symbols)
};

}
