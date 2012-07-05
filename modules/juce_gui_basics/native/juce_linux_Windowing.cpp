/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

extern Display* display;
extern XContext windowHandleXContext;

//==============================================================================
namespace Atoms
{
    enum ProtocolItems
    {
        TAKE_FOCUS = 0,
        DELETE_WINDOW = 1,
        PING = 2
    };

    static Atom Protocols, ProtocolList[3], ChangeState, State,
                ActiveWin, Pid, WindowType, WindowState,
                XdndAware, XdndEnter, XdndLeave, XdndPosition, XdndStatus,
                XdndDrop, XdndFinished, XdndSelection, XdndTypeList, XdndActionList,
                XdndActionDescription, XdndActionCopy,
                allowedActions[5],
                allowedMimeTypes[2];

    const unsigned long DndVersion = 3;

    Atom getIfExists (const char* name)    { return XInternAtom (display, name, True); }
    Atom getCreating (const char* name)    { return XInternAtom (display, name, False); }

    //==============================================================================
    void initialiseAtoms()
    {
        static bool atomsInitialised = false;

        if (! atomsInitialised)
        {
            atomsInitialised = true;

            Protocols                       = getIfExists ("WM_PROTOCOLS");
            ProtocolList [TAKE_FOCUS]       = getIfExists ("WM_TAKE_FOCUS");
            ProtocolList [DELETE_WINDOW]    = getIfExists ("WM_DELETE_WINDOW");
            ProtocolList [PING]             = getIfExists ("_NET_WM_PING");
            ChangeState                     = getIfExists ("WM_CHANGE_STATE");
            State                           = getIfExists ("WM_STATE");
            ActiveWin                       = getCreating ("_NET_ACTIVE_WINDOW");
            Pid                             = getCreating ("_NET_WM_PID");
            WindowType                      = getIfExists ("_NET_WM_WINDOW_TYPE");
            WindowState                     = getIfExists ("_NET_WM_STATE");

            XdndAware                       = getCreating ("XdndAware");
            XdndEnter                       = getCreating ("XdndEnter");
            XdndLeave                       = getCreating ("XdndLeave");
            XdndPosition                    = getCreating ("XdndPosition");
            XdndStatus                      = getCreating ("XdndStatus");
            XdndDrop                        = getCreating ("XdndDrop");
            XdndFinished                    = getCreating ("XdndFinished");
            XdndSelection                   = getCreating ("XdndSelection");

            XdndTypeList                    = getCreating ("XdndTypeList");
            XdndActionList                  = getCreating ("XdndActionList");
            XdndActionCopy                  = getCreating ("XdndActionCopy");
            XdndActionDescription           = getCreating ("XdndActionDescription");

            allowedMimeTypes[0]             = getCreating ("text/plain");
            allowedMimeTypes[1]             = getCreating ("text/uri-list");

            allowedActions[0]               = getCreating ("XdndActionMove");
            allowedActions[1]               = XdndActionCopy;
            allowedActions[2]               = getCreating ("XdndActionLink");
            allowedActions[3]               = getCreating ("XdndActionAsk");
            allowedActions[4]               = getCreating ("XdndActionPrivate");
        }
    }
}

//==============================================================================
namespace Keys
{
    enum MouseButtons
    {
        NoButton = 0,
        LeftButton = 1,
        MiddleButton = 2,
        RightButton = 3,
        WheelUp = 4,
        WheelDown = 5
    };

    static int AltMask = 0;
    static int NumLockMask = 0;
    static bool numLock = false;
    static bool capsLock = false;
    static char keyStates [32];
    static const int extendedKeyModifier = 0x10000000;
}

bool KeyPress::isKeyCurrentlyDown (const int keyCode)
{
    int keysym;

    if (keyCode & Keys::extendedKeyModifier)
    {
        keysym = 0xff00 | (keyCode & 0xff);
    }
    else
    {
        keysym = keyCode;

        if (keysym == (XK_Tab & 0xff)
            || keysym == (XK_Return & 0xff)
            || keysym == (XK_Escape & 0xff)
            || keysym == (XK_BackSpace & 0xff))
        {
            keysym |= 0xff00;
        }
    }

    ScopedXLock xlock;

    const int keycode = XKeysymToKeycode (display, keysym);

    const int keybyte = keycode >> 3;
    const int keybit = (1 << (keycode & 7));
    return (Keys::keyStates [keybyte] & keybit) != 0;
}

//==============================================================================
#if JUCE_USE_XSHM
namespace XSHMHelpers
{
    static int trappedErrorCode = 0;
    extern "C" int errorTrapHandler (Display*, XErrorEvent* err)
    {
        trappedErrorCode = err->error_code;
        return 0;
    }

    static bool isShmAvailable() noexcept
    {
        static bool isChecked = false;
        static bool isAvailable = false;

        if (! isChecked)
        {
            isChecked = true;
            int major, minor;
            Bool pixmaps;

            ScopedXLock xlock;

            if (XShmQueryVersion (display, &major, &minor, &pixmaps))
            {
                trappedErrorCode = 0;
                XErrorHandler oldHandler = XSetErrorHandler (errorTrapHandler);

                XShmSegmentInfo segmentInfo = { 0 };
                XImage* xImage = XShmCreateImage (display, DefaultVisual (display, DefaultScreen (display)),
                                                  24, ZPixmap, 0, &segmentInfo, 50, 50);

                if ((segmentInfo.shmid = shmget (IPC_PRIVATE,
                                                 xImage->bytes_per_line * xImage->height,
                                                 IPC_CREAT | 0777)) >= 0)
                {
                    segmentInfo.shmaddr = (char*) shmat (segmentInfo.shmid, 0, 0);

                    if (segmentInfo.shmaddr != (void*) -1)
                    {
                        segmentInfo.readOnly = False;
                        xImage->data = segmentInfo.shmaddr;
                        XSync (display, False);

                        if (XShmAttach (display, &segmentInfo) != 0)
                        {
                            XSync (display, False);
                            XShmDetach (display, &segmentInfo);

                            isAvailable = true;
                        }
                    }

                    XFlush (display);
                    XDestroyImage (xImage);

                    shmdt (segmentInfo.shmaddr);
                }

                shmctl (segmentInfo.shmid, IPC_RMID, 0);

                XSetErrorHandler (oldHandler);
                if (trappedErrorCode != 0)
                    isAvailable = false;
            }
        }

        return isAvailable;
    }
}
#endif

//==============================================================================
#if JUCE_USE_XRENDER
namespace XRender
{
    typedef Status (*tXRenderQueryVersion) (Display*, int*, int*);
    typedef XRenderPictFormat* (*tXrenderFindStandardFormat) (Display*, int);
    typedef XRenderPictFormat* (*tXRenderFindFormat) (Display*, unsigned long, XRenderPictFormat*, int);
    typedef XRenderPictFormat* (*tXRenderFindVisualFormat) (Display*, Visual*);

    static tXRenderQueryVersion xRenderQueryVersion = 0;
    static tXrenderFindStandardFormat xRenderFindStandardFormat = 0;
    static tXRenderFindFormat xRenderFindFormat = 0;
    static tXRenderFindVisualFormat xRenderFindVisualFormat = 0;

    static bool isAvailable()
    {
        static bool hasLoaded = false;

        if (! hasLoaded)
        {
            ScopedXLock xlock;
            hasLoaded = true;

            void* h = dlopen ("libXrender.so", RTLD_GLOBAL | RTLD_NOW);

            if (h != 0)
            {
                xRenderQueryVersion         = (tXRenderQueryVersion)        dlsym (h, "XRenderQueryVersion");
                xRenderFindStandardFormat   = (tXrenderFindStandardFormat)  dlsym (h, "XrenderFindStandardFormat");
                xRenderFindFormat           = (tXRenderFindFormat)          dlsym (h, "XRenderFindFormat");
                xRenderFindVisualFormat     = (tXRenderFindVisualFormat)    dlsym (h, "XRenderFindVisualFormat");
            }

            if (xRenderQueryVersion != 0
                 && xRenderFindStandardFormat != 0
                 && xRenderFindFormat != 0
                 && xRenderFindVisualFormat != 0)
            {
                int major, minor;
                if (xRenderQueryVersion (display, &major, &minor))
                    return true;
            }

            xRenderQueryVersion = 0;
        }

        return xRenderQueryVersion != 0;
    }

    static XRenderPictFormat* findPictureFormat()
    {
        ScopedXLock xlock;

        XRenderPictFormat* pictFormat = nullptr;

        if (isAvailable())
        {
            pictFormat = xRenderFindStandardFormat (display, PictStandardARGB32);

            if (pictFormat == 0)
            {
                XRenderPictFormat desiredFormat;
                desiredFormat.type = PictTypeDirect;
                desiredFormat.depth = 32;

                desiredFormat.direct.alphaMask = 0xff;
                desiredFormat.direct.redMask = 0xff;
                desiredFormat.direct.greenMask = 0xff;
                desiredFormat.direct.blueMask = 0xff;

                desiredFormat.direct.alpha = 24;
                desiredFormat.direct.red = 16;
                desiredFormat.direct.green = 8;
                desiredFormat.direct.blue = 0;

                pictFormat = xRenderFindFormat (display,
                                                PictFormatType | PictFormatDepth
                                                 | PictFormatRedMask | PictFormatRed
                                                 | PictFormatGreenMask | PictFormatGreen
                                                 | PictFormatBlueMask | PictFormatBlue
                                                 | PictFormatAlphaMask | PictFormatAlpha,
                                                &desiredFormat,
                                                0);
            }
        }

        return pictFormat;
    }
}
#endif

//==============================================================================
namespace Visuals
{
    static Visual* findVisualWithDepth (const int desiredDepth) noexcept
    {
        ScopedXLock xlock;

        Visual* visual = nullptr;
        int numVisuals = 0;
        long desiredMask = VisualNoMask;
        XVisualInfo desiredVisual;

        desiredVisual.screen = DefaultScreen (display);
        desiredVisual.depth = desiredDepth;

        desiredMask = VisualScreenMask | VisualDepthMask;

        if (desiredDepth == 32)
        {
            desiredVisual.c_class    = TrueColor;
            desiredVisual.red_mask   = 0x00FF0000;
            desiredVisual.green_mask = 0x0000FF00;
            desiredVisual.blue_mask  = 0x000000FF;
            desiredVisual.bits_per_rgb = 8;

            desiredMask |= VisualClassMask;
            desiredMask |= VisualRedMaskMask;
            desiredMask |= VisualGreenMaskMask;
            desiredMask |= VisualBlueMaskMask;
            desiredMask |= VisualBitsPerRGBMask;
        }

        XVisualInfo* xvinfos = XGetVisualInfo (display,
                                               desiredMask,
                                               &desiredVisual,
                                               &numVisuals);

        if (xvinfos != 0)
        {
            for (int i = 0; i < numVisuals; i++)
            {
                if (xvinfos[i].depth == desiredDepth)
                {
                    visual = xvinfos[i].visual;
                    break;
                }
            }

            XFree (xvinfos);
        }

        return visual;
    }

    static Visual* findVisualFormat (const int desiredDepth, int& matchedDepth) noexcept
    {
        Visual* visual = nullptr;

        if (desiredDepth == 32)
        {
           #if JUCE_USE_XSHM
            if (XSHMHelpers::isShmAvailable())
            {
               #if JUCE_USE_XRENDER
                if (XRender::isAvailable())
                {
                    XRenderPictFormat* pictFormat = XRender::findPictureFormat();

                    if (pictFormat != 0)
                    {
                        int numVisuals = 0;
                        XVisualInfo desiredVisual;
                        desiredVisual.screen = DefaultScreen (display);
                        desiredVisual.depth = 32;
                        desiredVisual.bits_per_rgb = 8;

                        XVisualInfo* xvinfos = XGetVisualInfo (display,
                                                               VisualScreenMask | VisualDepthMask | VisualBitsPerRGBMask,
                                                               &desiredVisual, &numVisuals);
                        if (xvinfos != 0)
                        {
                            for (int i = 0; i < numVisuals; ++i)
                            {
                                XRenderPictFormat* pictVisualFormat = XRender::xRenderFindVisualFormat (display, xvinfos[i].visual);

                                if (pictVisualFormat != 0
                                     && pictVisualFormat->type == PictTypeDirect
                                     && pictVisualFormat->direct.alphaMask)
                                {
                                    visual = xvinfos[i].visual;
                                    matchedDepth = 32;
                                    break;
                                }
                            }

                            XFree (xvinfos);
                        }
                    }
                }
               #endif
                if (visual == 0)
                {
                    visual = findVisualWithDepth (32);
                    if (visual != 0)
                        matchedDepth = 32;
                }
            }
           #endif
        }

        if (visual == 0 && desiredDepth >= 24)
        {
            visual = findVisualWithDepth (24);
            if (visual != 0)
                matchedDepth = 24;
        }

        if (visual == 0 && desiredDepth >= 16)
        {
            visual = findVisualWithDepth (16);
            if (visual != 0)
                matchedDepth = 16;
        }

        return visual;
    }
}

//==============================================================================
class XBitmapImage  : public ImagePixelData
{
public:
    XBitmapImage (const Image::PixelFormat format, const int w, const int h,
                  const bool clearImage, const int imageDepth_, Visual* visual)
        : ImagePixelData (format, w, h),
          imageDepth (imageDepth_),
          gc (None)
    {
        jassert (format == Image::RGB || format == Image::ARGB);

        pixelStride = (format == Image::RGB) ? 3 : 4;
        lineStride = ((w * pixelStride + 3) & ~3);

        ScopedXLock xlock;

       #if JUCE_USE_XSHM
        usingXShm = false;

        if ((imageDepth > 16) && XSHMHelpers::isShmAvailable())
        {
            zerostruct (segmentInfo);

            segmentInfo.shmid = -1;
            segmentInfo.shmaddr = (char *) -1;
            segmentInfo.readOnly = False;

            xImage = XShmCreateImage (display, visual, imageDepth, ZPixmap, 0, &segmentInfo, w, h);

            if (xImage != 0)
            {
                if ((segmentInfo.shmid = shmget (IPC_PRIVATE,
                                                 xImage->bytes_per_line * xImage->height,
                                                 IPC_CREAT | 0777)) >= 0)
                {
                    if (segmentInfo.shmid != -1)
                    {
                        segmentInfo.shmaddr = (char*) shmat (segmentInfo.shmid, 0, 0);

                        if (segmentInfo.shmaddr != (void*) -1)
                        {
                            segmentInfo.readOnly = False;

                            xImage->data = segmentInfo.shmaddr;
                            imageData = (uint8*) segmentInfo.shmaddr;

                            if (XShmAttach (display, &segmentInfo) != 0)
                                usingXShm = true;
                            else
                                jassertfalse;
                        }
                        else
                        {
                            shmctl (segmentInfo.shmid, IPC_RMID, 0);
                        }
                    }
                }
            }
        }

        if (! usingXShm)
       #endif
        {
            imageDataAllocated.allocate (lineStride * h, format == Image::ARGB && clearImage);
            imageData = imageDataAllocated;

            xImage = (XImage*) ::calloc (1, sizeof (XImage));

            xImage->width = w;
            xImage->height = h;
            xImage->xoffset = 0;
            xImage->format = ZPixmap;
            xImage->data = (char*) imageData;
            xImage->byte_order = ImageByteOrder (display);
            xImage->bitmap_unit = BitmapUnit (display);
            xImage->bitmap_bit_order = BitmapBitOrder (display);
            xImage->bitmap_pad = 32;
            xImage->depth = pixelStride * 8;
            xImage->bytes_per_line = lineStride;
            xImage->bits_per_pixel = pixelStride * 8;
            xImage->red_mask   = 0x00FF0000;
            xImage->green_mask = 0x0000FF00;
            xImage->blue_mask  = 0x000000FF;

            if (imageDepth == 16)
            {
                const int pixelStride = 2;
                const int lineStride = ((w * pixelStride + 3) & ~3);

                imageData16Bit.malloc (lineStride * h);
                xImage->data = imageData16Bit;
                xImage->bitmap_pad = 16;
                xImage->depth = pixelStride * 8;
                xImage->bytes_per_line = lineStride;
                xImage->bits_per_pixel = pixelStride * 8;
                xImage->red_mask   = visual->red_mask;
                xImage->green_mask = visual->green_mask;
                xImage->blue_mask  = visual->blue_mask;
            }

            if (! XInitImage (xImage))
                jassertfalse;
        }
    }

    ~XBitmapImage()
    {
        ScopedXLock xlock;

        if (gc != None)
            XFreeGC (display, gc);

       #if JUCE_USE_XSHM
        if (usingXShm)
        {
            XShmDetach (display, &segmentInfo);

            XFlush (display);
            XDestroyImage (xImage);

            shmdt (segmentInfo.shmaddr);
            shmctl (segmentInfo.shmid, IPC_RMID, 0);
        }
        else
       #endif
        {
            xImage->data = nullptr;
            XDestroyImage (xImage);
        }
    }

    LowLevelGraphicsContext* createLowLevelContext()
    {
        return new LowLevelGraphicsSoftwareRenderer (Image (this));
    }

    void initialiseBitmapData (Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode)
    {
        bitmap.data = imageData + x * pixelStride + y * lineStride;
        bitmap.pixelFormat = pixelFormat;
        bitmap.lineStride = lineStride;
        bitmap.pixelStride = pixelStride;
    }

    ImagePixelData* clone()
    {
        jassertfalse;
        return nullptr;
    }

    ImageType* createType() const                       { return new NativeImageType(); }

    void blitToWindow (Window window, int dx, int dy, int dw, int dh, int sx, int sy)
    {
        ScopedXLock xlock;

        if (gc == None)
        {
            XGCValues gcvalues;
            gcvalues.foreground = None;
            gcvalues.background = None;
            gcvalues.function = GXcopy;
            gcvalues.plane_mask = AllPlanes;
            gcvalues.clip_mask = None;
            gcvalues.graphics_exposures = False;

            gc = XCreateGC (display, window,
                            GCBackground | GCForeground | GCFunction | GCPlaneMask | GCClipMask | GCGraphicsExposures,
                            &gcvalues);
        }

        if (imageDepth == 16)
        {
            const uint32 rMask = xImage->red_mask;
            const uint32 rShiftL = jmax (0, getShiftNeeded (rMask));
            const uint32 rShiftR = jmax (0, -getShiftNeeded (rMask));
            const uint32 gMask = xImage->green_mask;
            const uint32 gShiftL = jmax (0, getShiftNeeded (gMask));
            const uint32 gShiftR = jmax (0, -getShiftNeeded (gMask));
            const uint32 bMask = xImage->blue_mask;
            const uint32 bShiftL = jmax (0, getShiftNeeded (bMask));
            const uint32 bShiftR = jmax (0, -getShiftNeeded (bMask));

            const Image::BitmapData srcData (Image (this), Image::BitmapData::readOnly);

            for (int y = sy; y < sy + dh; ++y)
            {
                const uint8* p = srcData.getPixelPointer (sx, y);

                for (int x = sx; x < sx + dw; ++x)
                {
                    const PixelRGB* const pixel = (const PixelRGB*) p;
                    p += srcData.pixelStride;

                    XPutPixel (xImage, x, y,
                               (((((uint32) pixel->getRed()) << rShiftL) >> rShiftR) & rMask)
                                 | (((((uint32) pixel->getGreen()) << gShiftL) >> gShiftR) & gMask)
                                 | (((((uint32) pixel->getBlue()) << bShiftL) >> bShiftR) & bMask));
                }
            }
        }

        // blit results to screen.
       #if JUCE_USE_XSHM
        if (usingXShm)
            XShmPutImage (display, (::Drawable) window, gc, xImage, sx, sy, dx, dy, dw, dh, True);
        else
       #endif
            XPutImage (display, (::Drawable) window, gc, xImage, sx, sy, dx, dy, dw, dh);
    }

private:
    //==============================================================================
    XImage* xImage;
    const int imageDepth;
    HeapBlock <uint8> imageDataAllocated;
    HeapBlock <char> imageData16Bit;
    int pixelStride, lineStride;
    uint8* imageData;
    GC gc;

   #if JUCE_USE_XSHM
    XShmSegmentInfo segmentInfo;
    bool usingXShm;
   #endif

    static int getShiftNeeded (const uint32 mask) noexcept
    {
        for (int i = 32; --i >= 0;)
            if (((mask >> i) & 1) != 0)
                return i - 7;

        jassertfalse;
        return 0;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XBitmapImage);
};

//==============================================================================
namespace PixmapHelpers
{
    Pixmap createColourPixmapFromImage (Display* display, const Image& image)
    {
        ScopedXLock xlock;

        const int width = image.getWidth();
        const int height = image.getHeight();
        HeapBlock <uint32> colour (width * height);
        int index = 0;

        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
                colour[index++] = image.getPixelAt (x, y).getARGB();

        XImage* ximage = XCreateImage (display, CopyFromParent, 24, ZPixmap,
                                       0, reinterpret_cast<char*> (colour.getData()),
                                       width, height, 32, 0);

        Pixmap pixmap = XCreatePixmap (display, DefaultRootWindow (display),
                                       width, height, 24);

        GC gc = XCreateGC (display, pixmap, 0, 0);
        XPutImage (display, pixmap, gc, ximage, 0, 0, 0, 0, width, height);
        XFreeGC (display, gc);

        return pixmap;
    }

    Pixmap createMaskPixmapFromImage (Display* display, const Image& image)
    {
        ScopedXLock xlock;

        const int width = image.getWidth();
        const int height = image.getHeight();
        const int stride = (width + 7) >> 3;
        HeapBlock <char> mask;
        mask.calloc (stride * height);
        const bool msbfirst = (BitmapBitOrder (display) == MSBFirst);

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const char bit = (char) (1 << (msbfirst ? (7 - (x & 7)) : (x & 7)));
                const int offset = y * stride + (x >> 3);

                if (image.getPixelAt (x, y).getAlpha() >= 128)
                    mask[offset] |= bit;
            }
        }

        return XCreatePixmapFromBitmapData (display, DefaultRootWindow (display),
                                            mask.getData(), width, height, 1, 0, 1);
    }
}


//==============================================================================
class LinuxComponentPeer  : public ComponentPeer
{
public:
    LinuxComponentPeer (Component* const component, const int windowStyleFlags, Window parentToAddTo)
        : ComponentPeer (component, windowStyleFlags),
          windowH (0), parentWindow (0),
          fullScreen (false), mapped (false),
          visual (0), depth (0)
    {
        // it's dangerous to create a window on a thread other than the message thread..
        jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

        repainter = new LinuxRepaintManager (this);

        createWindow (parentToAddTo);

        setTitle (component->getName());
    }

    ~LinuxComponentPeer()
    {
        // it's dangerous to delete a window on a thread other than the message thread..
        jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

        deleteIconPixmaps();

        destroyWindow();

        windowH = 0;
    }

    //==============================================================================
    void* getNativeHandle() const
    {
        return (void*) windowH;
    }

    static LinuxComponentPeer* getPeerFor (Window windowHandle) noexcept
    {
        XPointer peer = nullptr;

        ScopedXLock xlock;
        if (! XFindContext (display, (XID) windowHandle, windowHandleXContext, &peer))
            if (peer != nullptr && ! ComponentPeer::isValidPeer (reinterpret_cast <LinuxComponentPeer*> (peer)))
                peer = nullptr;

        return reinterpret_cast <LinuxComponentPeer*> (peer);
    }

    void setVisible (bool shouldBeVisible)
    {
        ScopedXLock xlock;
        if (shouldBeVisible)
            XMapWindow (display, windowH);
        else
            XUnmapWindow (display, windowH);
    }

    void setTitle (const String& title)
    {
        XTextProperty nameProperty;
        char* strings[] = { const_cast <char*> (title.toUTF8().getAddress()) };
        ScopedXLock xlock;

        if (XStringListToTextProperty (strings, 1, &nameProperty))
        {
            XSetWMName (display, windowH, &nameProperty);
            XSetWMIconName (display, windowH, &nameProperty);

            XFree (nameProperty.value);
        }
    }

    void setBounds (int x, int y, int w, int h, bool isNowFullScreen)
    {
        if (fullScreen && ! isNowFullScreen)
        {
            // When transitioning back from fullscreen, we might need to remove
            // the FULLSCREEN window property
            Atom fs = Atoms::getIfExists ("_NET_WM_STATE_FULLSCREEN");

            if (fs != None)
            {
                Window root = RootWindow (display, DefaultScreen (display));

                XClientMessageEvent clientMsg;
                clientMsg.display = display;
                clientMsg.window = windowH;
                clientMsg.type = ClientMessage;
                clientMsg.format = 32;
                clientMsg.message_type = Atoms::WindowState;
                clientMsg.data.l[0] = 0;  // Remove
                clientMsg.data.l[1] = fs;
                clientMsg.data.l[2] = 0;
                clientMsg.data.l[3] = 1;  // Normal Source

                ScopedXLock xlock;
                XSendEvent (display, root, false,
                            SubstructureRedirectMask | SubstructureNotifyMask,
                            (XEvent*) &clientMsg);
            }
        }

        fullScreen = isNowFullScreen;

        if (windowH != 0)
        {
            bounds.setBounds (x, y, jmax (1, w), jmax (1, h));

            WeakReference<Component> deletionChecker (component);
            ScopedXLock xlock;

            XSizeHints* const hints = XAllocSizeHints();
            hints->flags  = USSize | USPosition;
            hints->x      = bounds.getX();
            hints->y      = bounds.getY();
            hints->width  = bounds.getWidth();
            hints->height = bounds.getHeight();

            if ((getStyleFlags() & (windowHasTitleBar | windowIsResizable)) == windowHasTitleBar)
            {
                hints->min_width  = hints->max_width  = hints->width;
                hints->min_height = hints->max_height = hints->height;
                hints->flags |= PMinSize | PMaxSize;
            }

            XSetWMNormalHints (display, windowH, hints);
            XFree (hints);

            XMoveResizeWindow (display, windowH,
                               bounds.getX() - windowBorder.getLeft(),
                               bounds.getY() - windowBorder.getTop(),
                               bounds.getWidth(),
                               bounds.getHeight());

            if (deletionChecker != nullptr)
            {
                updateBorderSize();
                handleMovedOrResized();
            }
        }
    }

    void setPosition (int x, int y)           { setBounds (x, y, bounds.getWidth(), bounds.getHeight(), false); }
    void setSize (int w, int h)               { setBounds (bounds.getX(), bounds.getY(), w, h, false); }
    Rectangle<int> getBounds() const          { return bounds; }
    Point<int> getScreenPosition() const      { return bounds.getPosition(); }

    Point<int> localToGlobal (const Point<int>& relativePosition)
    {
        return relativePosition + getScreenPosition();
    }

    Point<int> globalToLocal (const Point<int>& screenPosition)
    {
        return screenPosition - getScreenPosition();
    }

    void setAlpha (float newAlpha)
    {
        //xxx todo!
    }

    void setMinimised (bool shouldBeMinimised)
    {
        if (shouldBeMinimised)
        {
            Window root = RootWindow (display, DefaultScreen (display));

            XClientMessageEvent clientMsg;
            clientMsg.display = display;
            clientMsg.window = windowH;
            clientMsg.type = ClientMessage;
            clientMsg.format = 32;
            clientMsg.message_type = Atoms::ChangeState;
            clientMsg.data.l[0] = IconicState;

            ScopedXLock xlock;
            XSendEvent (display, root, false, SubstructureRedirectMask | SubstructureNotifyMask, (XEvent*) &clientMsg);
        }
        else
        {
            setVisible (true);
        }
    }

    bool isMinimised() const
    {
        bool minimised = false;

        unsigned char* stateProp;
        unsigned long nitems, bytesLeft;
        Atom actualType;
        int actualFormat;

        ScopedXLock xlock;
        if (XGetWindowProperty (display, windowH, Atoms::State, 0, 64, False,
                                Atoms::State, &actualType, &actualFormat, &nitems, &bytesLeft,
                                &stateProp) == Success
            && actualType == Atoms::State
            && actualFormat == 32
            && nitems > 0)
        {
            if (((unsigned long*) stateProp)[0] == IconicState)
                minimised = true;

            XFree (stateProp);
        }

        return minimised;
    }

    void setFullScreen (const bool shouldBeFullScreen)
    {
        Rectangle<int> r (lastNonFullscreenBounds); // (get a copy of this before de-minimising)

        setMinimised (false);

        if (fullScreen != shouldBeFullScreen)
        {
            if (shouldBeFullScreen)
                r = Desktop::getInstance().getDisplays().getMainDisplay().userArea;

            if (! r.isEmpty())
                setBounds (r.getX(), r.getY(), r.getWidth(), r.getHeight(), shouldBeFullScreen);

            getComponent()->repaint();
        }
    }

    bool isFullScreen() const
    {
        return fullScreen;
    }

    bool isChildWindowOf (Window possibleParent) const
    {
        Window* windowList = nullptr;
        uint32 windowListSize = 0;
        Window parent, root;

        ScopedXLock xlock;
        if (XQueryTree (display, windowH, &root, &parent, &windowList, &windowListSize) != 0)
        {
            if (windowList != 0)
                XFree (windowList);

            return parent == possibleParent;
        }

        return false;
    }

    bool isFrontWindow() const
    {
        Window* windowList = nullptr;
        uint32 windowListSize = 0;
        bool result = false;

        ScopedXLock xlock;
        Window parent, root = RootWindow (display, DefaultScreen (display));

        if (XQueryTree (display, root, &root, &parent, &windowList, &windowListSize) != 0)
        {
            for (int i = windowListSize; --i >= 0;)
            {
                LinuxComponentPeer* const peer = LinuxComponentPeer::getPeerFor (windowList[i]);

                if (peer != 0)
                {
                    result = (peer == this);
                    break;
                }
            }
        }

        if (windowList != 0)
            XFree (windowList);

        return result;
    }

    bool contains (const Point<int>& position, bool trueIfInAChildWindow) const
    {
        if (! bounds.withZeroOrigin().contains (position))
            return false;

        for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
        {
            Component* const c = Desktop::getInstance().getComponent (i);

            if (c == getComponent())
                break;

            if (c->contains (position + bounds.getPosition() - c->getScreenPosition()))
                return false;
        }

        if (trueIfInAChildWindow)
            return true;

        ::Window root, child;
        int wx, wy;
        unsigned int ww, wh, bw, depth;

        ScopedXLock xlock;

        return XGetGeometry (display, (::Drawable) windowH, &root, &wx, &wy, &ww, &wh, &bw, &depth)
                && XTranslateCoordinates (display, windowH, windowH, position.getX(), position.getY(), &wx, &wy, &child)
                && child == None;
    }

    BorderSize<int> getFrameSize() const
    {
        return BorderSize<int>();
    }

    bool setAlwaysOnTop (bool alwaysOnTop)
    {
        return false;
    }

    void toFront (bool makeActive)
    {
        if (makeActive)
        {
            setVisible (true);
            grabFocus();
        }

        XEvent ev;
        ev.xclient.type = ClientMessage;
        ev.xclient.serial = 0;
        ev.xclient.send_event = True;
        ev.xclient.message_type = Atoms::ActiveWin;
        ev.xclient.window = windowH;
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = 2;
        ev.xclient.data.l[1] = CurrentTime;
        ev.xclient.data.l[2] = 0;
        ev.xclient.data.l[3] = 0;
        ev.xclient.data.l[4] = 0;

        {
            ScopedXLock xlock;
            XSendEvent (display, RootWindow (display, DefaultScreen (display)),
                        False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);

            XWindowAttributes attr;
            XGetWindowAttributes (display, windowH, &attr);

            if (component->isAlwaysOnTop())
                XRaiseWindow (display, windowH);

            XSync (display, False);
        }

        handleBroughtToFront();
    }

    void toBehind (ComponentPeer* other)
    {
        LinuxComponentPeer* const otherPeer = dynamic_cast <LinuxComponentPeer*> (other);
        jassert (otherPeer != nullptr); // wrong type of window?

        if (otherPeer != nullptr)
        {
            setMinimised (false);

            Window newStack[] = { otherPeer->windowH, windowH };

            ScopedXLock xlock;
            XRestackWindows (display, newStack, 2);
        }
    }

    bool isFocused() const
    {
        int revert = 0;
        Window focusedWindow = 0;
        ScopedXLock xlock;
        XGetInputFocus (display, &focusedWindow, &revert);

        return focusedWindow == windowH;
    }

    void grabFocus()
    {
        XWindowAttributes atts;
        ScopedXLock xlock;

        if (windowH != 0
            && XGetWindowAttributes (display, windowH, &atts)
            && atts.map_state == IsViewable
            && ! isFocused())
        {
            XSetInputFocus (display, windowH, RevertToParent, CurrentTime);
            isActiveApplication = true;
        }
    }

    void textInputRequired (const Point<int>&)
    {
    }

    void repaint (const Rectangle<int>& area)
    {
        repainter->repaint (area.getIntersection (getComponent()->getLocalBounds()));
    }

    void performAnyPendingRepaintsNow()
    {
        repainter->performAnyPendingRepaintsNow();
    }

    void setIcon (const Image& newIcon)
    {
        const int dataSize = newIcon.getWidth() * newIcon.getHeight() + 2;
        HeapBlock <unsigned long> data (dataSize);

        int index = 0;
        data[index++] = (unsigned long) newIcon.getWidth();
        data[index++] = (unsigned long) newIcon.getHeight();

        for (int y = 0; y < newIcon.getHeight(); ++y)
            for (int x = 0; x < newIcon.getWidth(); ++x)
                data[index++] = (unsigned long) newIcon.getPixelAt (x, y).getARGB();

        ScopedXLock xlock;
        xchangeProperty (windowH, Atoms::getCreating ("_NET_WM_ICON"), XA_CARDINAL, 32, data.getData(), dataSize);

        deleteIconPixmaps();

        XWMHints* wmHints = XGetWMHints (display, windowH);

        if (wmHints == nullptr)
            wmHints = XAllocWMHints();

        wmHints->flags |= IconPixmapHint | IconMaskHint;
        wmHints->icon_pixmap = PixmapHelpers::createColourPixmapFromImage (display, newIcon);
        wmHints->icon_mask = PixmapHelpers::createMaskPixmapFromImage (display, newIcon);

        XSetWMHints (display, windowH, wmHints);
        XFree (wmHints);

        XSync (display, False);
    }

    void deleteIconPixmaps()
    {
        ScopedXLock xlock;
        XWMHints* wmHints = XGetWMHints (display, windowH);

        if (wmHints != nullptr)
        {
            if ((wmHints->flags & IconPixmapHint) != 0)
            {
                wmHints->flags &= ~IconPixmapHint;
                XFreePixmap (display, wmHints->icon_pixmap);
            }

            if ((wmHints->flags & IconMaskHint) != 0)
            {
                wmHints->flags &= ~IconMaskHint;
                XFreePixmap (display, wmHints->icon_mask);
            }

            XSetWMHints (display, windowH, wmHints);
            XFree (wmHints);
        }
    }

    //==============================================================================
    void handleWindowMessage (XEvent* event)
    {
        switch (event->xany.type)
        {
            case KeyPressEventType:     handleKeyPressEvent ((XKeyEvent*) &event->xkey); break;
            case KeyRelease:            handleKeyReleaseEvent ((const XKeyEvent*) &event->xkey); break;
            case ButtonPress:           handleButtonPressEvent ((const XButtonPressedEvent*) &event->xbutton); break;
            case ButtonRelease:         handleButtonReleaseEvent ((const XButtonReleasedEvent*) &event->xbutton); break;
            case MotionNotify:          handleMotionNotifyEvent ((const XPointerMovedEvent*) &event->xmotion); break;
            case EnterNotify:           handleEnterNotifyEvent ((const XEnterWindowEvent*) &event->xcrossing); break;
            case LeaveNotify:           handleLeaveNotifyEvent ((const XLeaveWindowEvent*) &event->xcrossing); break;
            case FocusIn:               handleFocusInEvent(); break;
            case FocusOut:              handleFocusOutEvent(); break;
            case Expose:                handleExposeEvent ((XExposeEvent*) &event->xexpose); break;
            case MappingNotify:         handleMappingNotify ((XMappingEvent*) &event->xmapping); break;
            case ClientMessage:         handleClientMessageEvent ((XClientMessageEvent*) &event->xclient, event); break;
            case SelectionNotify:       handleDragAndDropSelection (event); break;
            case ConfigureNotify:       handleConfigureNotifyEvent ((XConfigureEvent*) &event->xconfigure); break;
            case ReparentNotify:        handleReparentNotifyEvent(); break;
            case GravityNotify:         handleGravityNotify(); break;

            case CirculateNotify:
            case CreateNotify:
            case DestroyNotify:
                // Think we can ignore these
                break;

            case MapNotify:
                mapped = true;
                handleBroughtToFront();
                break;

            case UnmapNotify:
                mapped = false;
                break;

            case SelectionClear:
            case SelectionRequest:
                break;

            default:
               #if JUCE_USE_XSHM
                {
                    ScopedXLock xlock;
                    if (event->xany.type == XShmGetEventBase (display))
                        repainter->notifyPaintCompleted();
                }
               #endif
                break;
        }
    }

    void handleKeyPressEvent (XKeyEvent* const keyEvent)
    {
        char utf8 [64] = { 0 };
        juce_wchar unicodeChar = 0;
        int keyCode = 0;
        bool keyDownChange = false;
        KeySym sym;

        {
            ScopedXLock xlock;
            updateKeyStates (keyEvent->keycode, true);

            const char* oldLocale = ::setlocale (LC_ALL, 0);
            ::setlocale (LC_ALL, "");
            XLookupString (keyEvent, utf8, sizeof (utf8), &sym, 0);
            ::setlocale (LC_ALL, oldLocale);

            unicodeChar = *CharPointer_UTF8 (utf8);
            keyCode = (int) unicodeChar;

            if (keyCode < 0x20)
                keyCode = XkbKeycodeToKeysym (display, keyEvent->keycode, 0, currentModifiers.isShiftDown() ? 1 : 0);

            keyDownChange = (sym != NoSymbol) && ! updateKeyModifiersFromSym (sym, true);
        }

        const ModifierKeys oldMods (currentModifiers);
        bool keyPressed = false;

        if ((sym & 0xff00) == 0xff00)
        {
            switch (sym)  // Translate keypad
            {
                case XK_KP_Divide:      keyCode = XK_slash; break;
                case XK_KP_Multiply:    keyCode = XK_asterisk; break;
                case XK_KP_Subtract:    keyCode = XK_hyphen; break;
                case XK_KP_Add:         keyCode = XK_plus; break;
                case XK_KP_Enter:       keyCode = XK_Return; break;
                case XK_KP_Decimal:     keyCode = Keys::numLock ? XK_period : XK_Delete; break;
                case XK_KP_0:           keyCode = Keys::numLock ? XK_0 : XK_Insert; break;
                case XK_KP_1:           keyCode = Keys::numLock ? XK_1 : XK_End; break;
                case XK_KP_2:           keyCode = Keys::numLock ? XK_2 : XK_Down; break;
                case XK_KP_3:           keyCode = Keys::numLock ? XK_3 : XK_Page_Down; break;
                case XK_KP_4:           keyCode = Keys::numLock ? XK_4 : XK_Left; break;
                case XK_KP_5:           keyCode = XK_5; break;
                case XK_KP_6:           keyCode = Keys::numLock ? XK_6 : XK_Right; break;
                case XK_KP_7:           keyCode = Keys::numLock ? XK_7 : XK_Home; break;
                case XK_KP_8:           keyCode = Keys::numLock ? XK_8 : XK_Up; break;
                case XK_KP_9:           keyCode = Keys::numLock ? XK_9 : XK_Page_Up; break;
                default:                break;
            }

            switch (sym)
            {
                case XK_Left:
                case XK_Right:
                case XK_Up:
                case XK_Down:
                case XK_Page_Up:
                case XK_Page_Down:
                case XK_End:
                case XK_Home:
                case XK_Delete:
                case XK_Insert:
                    keyPressed = true;
                    keyCode = (sym & 0xff) | Keys::extendedKeyModifier;
                    break;

                case XK_Tab:
                case XK_Return:
                case XK_Escape:
                case XK_BackSpace:
                    keyPressed = true;
                    keyCode &= 0xff;
                    break;

                default:
                    if (sym >= XK_F1 && sym <= XK_F16)
                    {
                        keyPressed = true;
                        keyCode = (sym & 0xff) | Keys::extendedKeyModifier;
                    }
                    break;
            }
        }

        if (utf8[0] != 0 || ((sym & 0xff00) == 0 && sym >= 8))
            keyPressed = true;

        if (oldMods != currentModifiers)
            handleModifierKeysChange();

        if (keyDownChange)
            handleKeyUpOrDown (true);

        if (keyPressed)
            handleKeyPress (keyCode, unicodeChar);
    }

    static bool isKeyReleasePartOfAutoRepeat (const XKeyEvent* const keyReleaseEvent)
    {
        if (XPending (display))
        {
            XEvent e;
            XPeekEvent (display, &e);

            // Look for a subsequent key-down event with the same timestamp and keycode
            return e.type == KeyPressEventType
                    && e.xkey.keycode == keyReleaseEvent->keycode
                    && e.xkey.time == keyReleaseEvent->time;
        }

        return false;
    }

    void handleKeyReleaseEvent (const XKeyEvent* const keyEvent)
    {
        if (! isKeyReleasePartOfAutoRepeat (keyEvent))
        {
            updateKeyStates (keyEvent->keycode, false);
            KeySym sym;

            {
                ScopedXLock xlock;
                sym = XkbKeycodeToKeysym (display, keyEvent->keycode, 0, 0);
            }

            const ModifierKeys oldMods (currentModifiers);
            const bool keyDownChange = (sym != NoSymbol) && ! updateKeyModifiersFromSym (sym, false);

            if (oldMods != currentModifiers)
                handleModifierKeysChange();

            if (keyDownChange)
                handleKeyUpOrDown (false);
        }
    }

    void handleWheelEvent (const XButtonPressedEvent* const buttonPressEvent, const float amount)
    {
        MouseWheelDetails wheel;
        wheel.deltaX = 0.0f;
        wheel.deltaY = amount;
        wheel.isReversed = false;
        wheel.isSmooth = false;

        handleMouseWheel (0, Point<int> (buttonPressEvent->x, buttonPressEvent->y),
                          getEventTime (buttonPressEvent->time), wheel);
    }

    void handleButtonPressEvent (const XButtonPressedEvent* const buttonPressEvent, int buttonModifierFlag)
    {
        currentModifiers = currentModifiers.withFlags (buttonModifierFlag);
        toFront (true);
        handleMouseEvent (0, Point<int> (buttonPressEvent->x, buttonPressEvent->y), currentModifiers,
                          getEventTime (buttonPressEvent->time));
    }

    void handleButtonPressEvent (const XButtonPressedEvent* const buttonPressEvent)
    {
        updateKeyModifiers (buttonPressEvent->state);

        switch (pointerMap [buttonPressEvent->button - Button1])
        {
            case Keys::WheelUp:         handleWheelEvent (buttonPressEvent,  50.0f / 256.0f); break;
            case Keys::WheelDown:       handleWheelEvent (buttonPressEvent, -50.0f / 256.0f); break;
            case Keys::LeftButton:      handleButtonPressEvent (buttonPressEvent, ModifierKeys::leftButtonModifier); break;
            case Keys::RightButton:     handleButtonPressEvent (buttonPressEvent, ModifierKeys::rightButtonModifier); break;
            case Keys::MiddleButton:    handleButtonPressEvent (buttonPressEvent, ModifierKeys::middleButtonModifier); break;
            default: break;
        }

        clearLastMousePos();
    }

    void handleButtonReleaseEvent (const XButtonReleasedEvent* const buttonRelEvent)
    {
        updateKeyModifiers (buttonRelEvent->state);

        switch (pointerMap [buttonRelEvent->button - Button1])
        {
            case Keys::LeftButton:      currentModifiers = currentModifiers.withoutFlags (ModifierKeys::leftButtonModifier); break;
            case Keys::RightButton:     currentModifiers = currentModifiers.withoutFlags (ModifierKeys::rightButtonModifier); break;
            case Keys::MiddleButton:    currentModifiers = currentModifiers.withoutFlags (ModifierKeys::middleButtonModifier); break;
            default: break;
        }

        handleMouseEvent (0, Point<int> (buttonRelEvent->x, buttonRelEvent->y), currentModifiers,
                          getEventTime (buttonRelEvent->time));

        clearLastMousePos();
    }

    void handleMotionNotifyEvent (const XPointerMovedEvent* const movedEvent)
    {
        updateKeyModifiers (movedEvent->state);
        const Point<int> mousePos (movedEvent->x_root, movedEvent->y_root);

        if (lastMousePos != mousePos)
        {
            lastMousePos = mousePos;

            if (parentWindow != 0 && (styleFlags & windowHasTitleBar) == 0)
            {
                Window wRoot = 0, wParent = 0;

                {
                    ScopedXLock xlock;
                    unsigned int numChildren;
                    Window* wChild = nullptr;
                    XQueryTree (display, windowH, &wRoot, &wParent, &wChild, &numChildren);
                }

                if (wParent != 0
                     && wParent != windowH
                     && wParent != wRoot)
                {
                    parentWindow = wParent;
                    updateBounds();
                }
                else
                {
                    parentWindow = 0;
                }
            }

            handleMouseEvent (0, mousePos - getScreenPosition(), currentModifiers, getEventTime (movedEvent->time));
        }
    }

    void handleEnterNotifyEvent (const XEnterWindowEvent* const enterEvent)
    {
        clearLastMousePos();

        if (! currentModifiers.isAnyMouseButtonDown())
        {
            updateKeyModifiers (enterEvent->state);
            handleMouseEvent (0, Point<int> (enterEvent->x, enterEvent->y), currentModifiers, getEventTime (enterEvent->time));
        }
    }

    void handleLeaveNotifyEvent (const XLeaveWindowEvent* const leaveEvent)
    {
        // Suppress the normal leave if we've got a pointer grab, or if
        // it's a bogus one caused by clicking a mouse button when running
        // in a Window manager
        if (((! currentModifiers.isAnyMouseButtonDown()) && leaveEvent->mode == NotifyNormal)
             || leaveEvent->mode == NotifyUngrab)
        {
            updateKeyModifiers (leaveEvent->state);
            handleMouseEvent (0, Point<int> (leaveEvent->x, leaveEvent->y), currentModifiers, getEventTime (leaveEvent->time));
        }
    }

    void handleFocusInEvent()
    {
        isActiveApplication = true;
        if (isFocused())
            handleFocusGain();
    }

    void handleFocusOutEvent()
    {
        isActiveApplication = false;
        if (! isFocused())
            handleFocusLoss();
    }

    void handleExposeEvent (XExposeEvent* exposeEvent)
    {
        // Batch together all pending expose events
        XEvent nextEvent;
        ScopedXLock xlock;

        if (exposeEvent->window != windowH)
        {
            Window child;
            XTranslateCoordinates (display, exposeEvent->window, windowH,
                                   exposeEvent->x, exposeEvent->y, &exposeEvent->x, &exposeEvent->y,
                                   &child);
        }

        repaint (Rectangle<int> (exposeEvent->x, exposeEvent->y,
                                 exposeEvent->width, exposeEvent->height));

        while (XEventsQueued (display, QueuedAfterFlush) > 0)
        {
            XPeekEvent (display, &nextEvent);
            if (nextEvent.type != Expose || nextEvent.xany.window != exposeEvent->window)
                break;

            XNextEvent (display, &nextEvent);
            XExposeEvent* nextExposeEvent = (XExposeEvent*) &nextEvent.xexpose;
            repaint (Rectangle<int> (nextExposeEvent->x, nextExposeEvent->y,
                                     nextExposeEvent->width, nextExposeEvent->height));
        }
    }

    void handleConfigureNotifyEvent (XConfigureEvent* const confEvent)
    {
        updateBounds();
        updateBorderSize();
        handleMovedOrResized();

        // if the native title bar is dragged, need to tell any active menus, etc.
        if ((styleFlags & windowHasTitleBar) != 0
              && component->isCurrentlyBlockedByAnotherModalComponent())
        {
            Component* const currentModalComp = Component::getCurrentlyModalComponent();

            if (currentModalComp != 0)
                currentModalComp->inputAttemptWhenModal();
        }

        if (confEvent->window == windowH
             && confEvent->above != 0
             && isFrontWindow())
        {
            handleBroughtToFront();
        }
    }

    void handleReparentNotifyEvent()
    {
        parentWindow = 0;
        Window wRoot = 0;
        Window* wChild = nullptr;
        unsigned int numChildren;

        {
            ScopedXLock xlock;
            XQueryTree (display, windowH, &wRoot, &parentWindow, &wChild, &numChildren);
        }

        if (parentWindow == windowH || parentWindow == wRoot)
            parentWindow = 0;

        handleGravityNotify();
    }

    void handleGravityNotify()
    {
        updateBounds();
        updateBorderSize();
        handleMovedOrResized();
    }

    void handleMappingNotify (XMappingEvent* const mappingEvent)
    {
        if (mappingEvent->request != MappingPointer)
        {
            // Deal with modifier/keyboard mapping
            ScopedXLock xlock;
            XRefreshKeyboardMapping (mappingEvent);
            updateModifierMappings();
        }
    }

    void handleClientMessageEvent (XClientMessageEvent* const clientMsg, XEvent* event)
    {
        if (clientMsg->message_type == Atoms::Protocols && clientMsg->format == 32)
        {
            const Atom atom = (Atom) clientMsg->data.l[0];

            if (atom == Atoms::ProtocolList [Atoms::PING])
            {
                Window root = RootWindow (display, DefaultScreen (display));

                clientMsg->window = root;

                XSendEvent (display, root, False, NoEventMask, event);
                XFlush (display);
            }
            else if (atom == Atoms::ProtocolList [Atoms::TAKE_FOCUS])
            {
                if ((getStyleFlags() & juce::ComponentPeer::windowIgnoresKeyPresses) == 0)
                {
                    XWindowAttributes atts;

                    ScopedXLock xlock;
                    if (clientMsg->window != 0
                         && XGetWindowAttributes (display, clientMsg->window, &atts))
                    {
                        if (atts.map_state == IsViewable)
                            XSetInputFocus (display, clientMsg->window, RevertToParent, clientMsg->data.l[1]);
                    }
                }
            }
            else if (atom == Atoms::ProtocolList [Atoms::DELETE_WINDOW])
            {
                handleUserClosingWindow();
            }
        }
        else if (clientMsg->message_type == Atoms::XdndEnter)
        {
            handleDragAndDropEnter (clientMsg);
        }
        else if (clientMsg->message_type == Atoms::XdndLeave)
        {
            resetDragAndDrop();
        }
        else if (clientMsg->message_type == Atoms::XdndPosition)
        {
            handleDragAndDropPosition (clientMsg);
        }
        else if (clientMsg->message_type == Atoms::XdndDrop)
        {
            handleDragAndDropDrop (clientMsg);
        }
        else if (clientMsg->message_type == Atoms::XdndStatus)
        {
            handleDragAndDropStatus (clientMsg);
        }
        else if (clientMsg->message_type == Atoms::XdndFinished)
        {
            resetDragAndDrop();
        }
    }

    //==============================================================================
    void showMouseCursor (Cursor cursor) noexcept
    {
        ScopedXLock xlock;
        XDefineCursor (display, windowH, cursor);
    }

    //==============================================================================
    bool dontRepaint;

    static ModifierKeys currentModifiers;
    static bool isActiveApplication;

private:
    //==============================================================================
    class LinuxRepaintManager : public Timer
    {
    public:
        LinuxRepaintManager (LinuxComponentPeer* const peer_)
            : peer (peer_),
              lastTimeImageUsed (0)
        {
           #if JUCE_USE_XSHM
            shmCompletedDrawing = true;

            useARGBImagesForRendering = XSHMHelpers::isShmAvailable();

            if (useARGBImagesForRendering)
            {
                ScopedXLock xlock;
                XShmSegmentInfo segmentinfo;

                XImage* const testImage
                    = XShmCreateImage (display, DefaultVisual (display, DefaultScreen (display)),
                                       24, ZPixmap, 0, &segmentinfo, 64, 64);

                useARGBImagesForRendering = (testImage->bits_per_pixel == 32);
                XDestroyImage (testImage);
            }
           #endif
        }

        void timerCallback()
        {
           #if JUCE_USE_XSHM
            if (! shmCompletedDrawing)
                return;
           #endif
            if (! regionsNeedingRepaint.isEmpty())
            {
                stopTimer();
                performAnyPendingRepaintsNow();
            }
            else if (Time::getApproximateMillisecondCounter() > lastTimeImageUsed + 3000)
            {
                stopTimer();
                image = Image::null;
            }
        }

        void repaint (const Rectangle<int>& area)
        {
            if (! isTimerRunning())
                startTimer (repaintTimerPeriod);

            regionsNeedingRepaint.add (area);
        }

        void performAnyPendingRepaintsNow()
        {
           #if JUCE_USE_XSHM
            if (! shmCompletedDrawing)
            {
                startTimer (repaintTimerPeriod);
                return;
            }
           #endif

            peer->clearMaskedRegion();

            RectangleList originalRepaintRegion (regionsNeedingRepaint);
            regionsNeedingRepaint.clear();
            const Rectangle<int> totalArea (originalRepaintRegion.getBounds());

            if (! totalArea.isEmpty())
            {
                if (image.isNull() || image.getWidth() < totalArea.getWidth()
                     || image.getHeight() < totalArea.getHeight())
                {
                   #if JUCE_USE_XSHM
                    image = Image (new XBitmapImage (useARGBImagesForRendering ? Image::ARGB
                                                                               : Image::RGB,
                   #else
                    image = Image (new XBitmapImage (Image::RGB,
                   #endif
                                                     (totalArea.getWidth() + 31) & ~31,
                                                     (totalArea.getHeight() + 31) & ~31,
                                                     false, peer->depth, peer->visual));
                }

                startTimer (repaintTimerPeriod);

                RectangleList adjustedList (originalRepaintRegion);
                adjustedList.offsetAll (-totalArea.getX(), -totalArea.getY());

                if (peer->depth == 32)
                {
                    RectangleList::Iterator i (originalRepaintRegion);

                    while (i.next())
                        image.clear (*i.getRectangle() - totalArea.getPosition());
                }

                {
                    ScopedPointer<LowLevelGraphicsContext> context (peer->getComponent()->getLookAndFeel()
                                                                      .createGraphicsContext (image, -totalArea.getPosition(), adjustedList));
                    peer->handlePaint (*context);
                }

                if (! peer->maskedRegion.isEmpty())
                    originalRepaintRegion.subtract (peer->maskedRegion);

                for (RectangleList::Iterator i (originalRepaintRegion); i.next();)
                {
                   #if JUCE_USE_XSHM
                    shmCompletedDrawing = false;
                   #endif
                    const Rectangle<int>& r = *i.getRectangle();

                    static_cast<XBitmapImage*> (image.getPixelData())
                        ->blitToWindow (peer->windowH,
                                        r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                        r.getX() - totalArea.getX(), r.getY() - totalArea.getY());
                }
            }

            lastTimeImageUsed = Time::getApproximateMillisecondCounter();
            startTimer (repaintTimerPeriod);
        }

       #if JUCE_USE_XSHM
        void notifyPaintCompleted()                 { shmCompletedDrawing = true; }
       #endif

    private:
        enum { repaintTimerPeriod = 1000 / 100 };

        LinuxComponentPeer* const peer;
        Image image;
        uint32 lastTimeImageUsed;
        RectangleList regionsNeedingRepaint;

       #if JUCE_USE_XSHM
        bool useARGBImagesForRendering, shmCompletedDrawing;
       #endif
        JUCE_DECLARE_NON_COPYABLE (LinuxRepaintManager);
    };

    ScopedPointer <LinuxRepaintManager> repainter;

    friend class LinuxRepaintManager;
    Window windowH, parentWindow;
    Rectangle<int> bounds;
    Image taskbarImage;
    bool fullScreen, mapped;
    Visual* visual;
    int depth;
    BorderSize<int> windowBorder;
    enum { KeyPressEventType = 2 };

    struct MotifWmHints
    {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long input_mode;
        unsigned long status;
    };

    static void updateKeyStates (const int keycode, const bool press) noexcept
    {
        const int keybyte = keycode >> 3;
        const int keybit = (1 << (keycode & 7));

        if (press)
            Keys::keyStates [keybyte] |= keybit;
        else
            Keys::keyStates [keybyte] &= ~keybit;
    }

    static void updateKeyModifiers (const int status) noexcept
    {
        int keyMods = 0;

        if ((status & ShiftMask) != 0)     keyMods |= ModifierKeys::shiftModifier;
        if ((status & ControlMask) != 0)   keyMods |= ModifierKeys::ctrlModifier;
        if ((status & Keys::AltMask) != 0) keyMods |= ModifierKeys::altModifier;

        currentModifiers = currentModifiers.withOnlyMouseButtons().withFlags (keyMods);

        Keys::numLock  = ((status & Keys::NumLockMask) != 0);
        Keys::capsLock = ((status & LockMask) != 0);
    }

    static bool updateKeyModifiersFromSym (KeySym sym, const bool press) noexcept
    {
        int modifier = 0;
        bool isModifier = true;

        switch (sym)
        {
            case XK_Shift_L:
            case XK_Shift_R:
                modifier = ModifierKeys::shiftModifier;
                break;

            case XK_Control_L:
            case XK_Control_R:
                modifier = ModifierKeys::ctrlModifier;
                break;

            case XK_Alt_L:
            case XK_Alt_R:
                modifier = ModifierKeys::altModifier;
                break;

            case XK_Num_Lock:
                if (press)
                    Keys::numLock = ! Keys::numLock;

                break;

            case XK_Caps_Lock:
                if (press)
                    Keys::capsLock = ! Keys::capsLock;

                break;

            case XK_Scroll_Lock:
                break;

            default:
                isModifier = false;
                break;
        }

        currentModifiers = press ? currentModifiers.withFlags (modifier)
                                 : currentModifiers.withoutFlags (modifier);

        return isModifier;
    }

    // Alt and Num lock are not defined by standard X
    // modifier constants: check what they're mapped to
    static void updateModifierMappings() noexcept
    {
        ScopedXLock xlock;
        const int altLeftCode = XKeysymToKeycode (display, XK_Alt_L);
        const int numLockCode = XKeysymToKeycode (display, XK_Num_Lock);

        Keys::AltMask = 0;
        Keys::NumLockMask = 0;

        XModifierKeymap* mapping = XGetModifierMapping (display);

        if (mapping)
        {
            for (int i = 0; i < 8; i++)
            {
                if (mapping->modifiermap [i << 1] == altLeftCode)
                    Keys::AltMask = 1 << i;
                else if (mapping->modifiermap [i << 1] == numLockCode)
                    Keys::NumLockMask = 1 << i;
            }

            XFreeModifiermap (mapping);
        }
    }

    //==============================================================================
    static void xchangeProperty (Window wndH, Atom property, Atom type, int format, const void* data, int numElements)
    {
        XChangeProperty (display, wndH, property, type, format, PropModeReplace, (const unsigned char*) data, numElements);
    }

    void removeWindowDecorations (Window wndH)
    {
        Atom hints = Atoms::getIfExists ("_MOTIF_WM_HINTS");

        if (hints != None)
        {
            MotifWmHints motifHints = { 0 };
            motifHints.flags = 2; /* MWM_HINTS_DECORATIONS */
            motifHints.decorations = 0;

            ScopedXLock xlock;
            xchangeProperty (wndH, hints, hints, 32, &motifHints, 4);
        }

        hints = Atoms::getIfExists ("_WIN_HINTS");

        if (hints != None)
        {
            long gnomeHints = 0;

            ScopedXLock xlock;
            xchangeProperty (wndH, hints, hints, 32, &gnomeHints, 1);
        }

        hints = Atoms::getIfExists ("KWM_WIN_DECORATION");

        if (hints != None)
        {
            long kwmHints = 2; /*KDE_tinyDecoration*/

            ScopedXLock xlock;
            xchangeProperty (wndH, hints, hints, 32, &kwmHints, 1);
        }
    }

    void addWindowButtons (Window wndH)
    {
        ScopedXLock xlock;
        Atom hints = Atoms::getIfExists ("_MOTIF_WM_HINTS");

        if (hints != None)
        {
            MotifWmHints motifHints = { 0 };
            motifHints.flags = 1 | 2; /* MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS */
            motifHints.decorations = 2 /* MWM_DECOR_BORDER */ | 8 /* MWM_DECOR_TITLE */ | 16; /* MWM_DECOR_MENU */

            motifHints.functions = 4 /* MWM_FUNC_MOVE */;

            if ((styleFlags & windowHasCloseButton) != 0)
                motifHints.functions |= 32; /* MWM_FUNC_CLOSE */

            if ((styleFlags & windowHasMinimiseButton) != 0)
            {
                motifHints.functions |= 8; /* MWM_FUNC_MINIMIZE */
                motifHints.decorations |= 0x20; /* MWM_DECOR_MINIMIZE */
            }

            if ((styleFlags & windowHasMaximiseButton) != 0)
            {
                motifHints.functions |= 0x10; /* MWM_FUNC_MAXIMIZE */
                motifHints.decorations |= 0x40; /* MWM_DECOR_MAXIMIZE */
            }

            if ((styleFlags & windowIsResizable) != 0)
            {
                motifHints.functions |= 2; /* MWM_FUNC_RESIZE */
                motifHints.decorations |= 0x4; /* MWM_DECOR_RESIZEH */
            }

            xchangeProperty (wndH, hints, hints, 32, &motifHints, 5);
        }

        hints = Atoms::getIfExists ("_NET_WM_ALLOWED_ACTIONS");

        if (hints != None)
        {
            Atom netHints [6];
            int num = 0;

            if ((styleFlags & windowIsResizable) != 0)
                netHints [num++] = Atoms::getIfExists ("_NET_WM_ACTION_RESIZE");

            if ((styleFlags & windowHasMaximiseButton) != 0)
                netHints [num++] = Atoms::getIfExists ("_NET_WM_ACTION_FULLSCREEN");

            if ((styleFlags & windowHasMinimiseButton) != 0)
                netHints [num++] = Atoms::getIfExists ("_NET_WM_ACTION_MINIMIZE");

            if ((styleFlags & windowHasCloseButton) != 0)
                netHints [num++] = Atoms::getIfExists ("_NET_WM_ACTION_CLOSE");

            xchangeProperty (wndH, hints, XA_ATOM, 32, &netHints, num);
        }
    }

    void setWindowType()
    {
        Atom netHints [2];

        if ((styleFlags & windowIsTemporary) != 0
             || ((styleFlags & windowHasDropShadow) == 0 && Desktop::canUseSemiTransparentWindows()))
            netHints [0] = Atoms::getIfExists ("_NET_WM_WINDOW_TYPE_COMBO");
        else
            netHints [0] = Atoms::getIfExists ("_NET_WM_WINDOW_TYPE_NORMAL");

        netHints[1] = Atoms::getIfExists ("_KDE_NET_WM_WINDOW_TYPE_OVERRIDE");

        xchangeProperty (windowH, Atoms::WindowType, XA_ATOM, 32, &netHints, 2);

        int numHints = 0;

        if ((styleFlags & windowAppearsOnTaskbar) == 0)
            netHints [numHints++] = Atoms::getIfExists ("_NET_WM_STATE_SKIP_TASKBAR");

        if (component->isAlwaysOnTop())
            netHints [numHints++] = Atoms::getIfExists ("_NET_WM_STATE_ABOVE");

        if (numHints > 0)
            xchangeProperty (windowH, Atoms::WindowState, XA_ATOM, 32, &netHints, numHints);
    }

    void createWindow (Window parentToAddTo)
    {
        ScopedXLock xlock;
        Atoms::initialiseAtoms();
        resetDragAndDrop();

        // Get defaults for various properties
        const int screen = DefaultScreen (display);
        Window root = RootWindow (display, screen);

        // Try to obtain a 32-bit visual or fallback to 24 or 16
        visual = Visuals::findVisualFormat ((styleFlags & windowIsSemiTransparent) ? 32 : 24, depth);

        if (visual == 0)
        {
            Logger::outputDebugString ("ERROR: System doesn't support 32, 24 or 16 bit RGB display.\n");
            Process::terminate();
        }

        // Create and install a colormap suitable fr our visual
        Colormap colormap = XCreateColormap (display, root, visual, AllocNone);
        XInstallColormap (display, colormap);

        // Set up the window attributes
        XSetWindowAttributes swa;
        swa.border_pixel = 0;
        swa.background_pixmap = None;
        swa.colormap = colormap;
        swa.override_redirect = (getComponent()->isAlwaysOnTop() && (styleFlags & windowIsTemporary) != 0) ? True : False;
        swa.event_mask = getAllEventsMask();

        windowH = XCreateWindow (display, parentToAddTo != 0 ? parentToAddTo : root,
                                 0, 0, 1, 1,
                                 0, depth, InputOutput, visual,
                                 CWBorderPixel | CWColormap | CWBackPixmap | CWEventMask | CWOverrideRedirect,
                                 &swa);

        XGrabButton (display, AnyButton, AnyModifier, windowH, False,
                     ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask,
                     GrabModeAsync, GrabModeAsync, None, None);

        // Set the window context to identify the window handle object
        if (XSaveContext (display, (XID) windowH, windowHandleXContext, (XPointer) this))
        {
            // Failed
            jassertfalse;
            Logger::outputDebugString ("Failed to create context information for window.\n");
            XDestroyWindow (display, windowH);
            windowH = 0;
            return;
        }

        // Set window manager hints
        XWMHints* wmHints = XAllocWMHints();
        wmHints->flags = InputHint | StateHint;
        wmHints->input = True;      // Locally active input model
        wmHints->initial_state = NormalState;
        XSetWMHints (display, windowH, wmHints);
        XFree (wmHints);

        // Set the window type
        setWindowType();

        // Define decoration
        if ((styleFlags & windowHasTitleBar) == 0)
            removeWindowDecorations (windowH);
        else
            addWindowButtons (windowH);

        setTitle (getComponent()->getName());

        // Associate the PID, allowing to be shut down when something goes wrong
        unsigned long pid = getpid();
        xchangeProperty (windowH, Atoms::Pid, XA_CARDINAL, 32, &pid, 1);

        // Set window manager protocols
        xchangeProperty (windowH, Atoms::Protocols, XA_ATOM, 32, Atoms::ProtocolList, 2);

        // Set drag and drop flags
        xchangeProperty (windowH, Atoms::XdndTypeList, XA_ATOM, 32, Atoms::allowedMimeTypes, numElementsInArray (Atoms::allowedMimeTypes));
        xchangeProperty (windowH, Atoms::XdndActionList, XA_ATOM, 32, Atoms::allowedActions, numElementsInArray (Atoms::allowedActions));
        xchangeProperty (windowH, Atoms::XdndActionDescription, XA_STRING, 8, "", 0);
        xchangeProperty (windowH, Atoms::XdndAware, XA_ATOM, 32, &Atoms::DndVersion, 1);

        initialisePointerMap();
        updateModifierMappings();
    }

    void destroyWindow()
    {
        ScopedXLock xlock;

        XPointer handlePointer;
        if (! XFindContext (display, (XID) windowH, windowHandleXContext, &handlePointer))
            XDeleteContext (display, (XID) windowH, windowHandleXContext);

        XDestroyWindow (display, windowH);

        // Wait for it to complete and then remove any events for this
        // window from the event queue.
        XSync (display, false);

        XEvent event;
        while (XCheckWindowEvent (display, windowH, getAllEventsMask(), &event) == True)
        {}
    }

    static int getAllEventsMask() noexcept
    {
        return NoEventMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask
                 | EnterWindowMask | LeaveWindowMask | PointerMotionMask | KeymapStateMask
                 | ExposureMask | StructureNotifyMask | FocusChangeMask;
    }

    static int64 getEventTime (::Time t)
    {
        static int64 eventTimeOffset = 0x12345678;
        const int64 thisMessageTime = t;

        if (eventTimeOffset == 0x12345678)
            eventTimeOffset = Time::currentTimeMillis() - thisMessageTime;

        return eventTimeOffset + thisMessageTime;
    }

    void updateBorderSize()
    {
        if ((styleFlags & windowHasTitleBar) == 0)
        {
            windowBorder = BorderSize<int> (0);
        }
        else if (windowBorder.getTopAndBottom() == 0 && windowBorder.getLeftAndRight() == 0)
        {
            ScopedXLock xlock;
            Atom hints = Atoms::getIfExists ("_NET_FRAME_EXTENTS");

            if (hints != None)
            {
                unsigned char* data = nullptr;
                unsigned long nitems, bytesLeft;
                Atom actualType;
                int actualFormat;

                if (XGetWindowProperty (display, windowH, hints, 0, 4, False,
                                        XA_CARDINAL, &actualType, &actualFormat, &nitems, &bytesLeft,
                                        &data) == Success)
                {
                    const unsigned long* const sizes = (const unsigned long*) data;

                    if (actualFormat == 32)
                        windowBorder = BorderSize<int> ((int) sizes[2], (int) sizes[0],
                                                        (int) sizes[3], (int) sizes[1]);

                    XFree (data);
                }
            }
        }
    }

    void updateBounds()
    {
        jassert (windowH != 0);
        if (windowH != 0)
        {
            Window root, child;
            int wx = 0, wy = 0;
            unsigned int ww = 0, wh = 0, bw, depth;

            ScopedXLock xlock;

            if (XGetGeometry (display, (::Drawable) windowH, &root, &wx, &wy, &ww, &wh, &bw, &depth))
                if (! XTranslateCoordinates (display, windowH, root, 0, 0, &wx, &wy, &child))
                    wx = wy = 0;

            bounds.setBounds (wx, wy, ww, wh);
        }
    }

    //==============================================================================
    void resetDragAndDrop()
    {
        dragInfo.files.clear();
        dragInfo.text = String::empty;
        dragInfo.position = Point<int> (-1, -1);
        dragAndDropCurrentMimeType = 0;
        dragAndDropSourceWindow = 0;
        srcMimeTypeAtomList.clear();
    }

    void sendDragAndDropMessage (XClientMessageEvent& msg)
    {
        msg.type = ClientMessage;
        msg.display = display;
        msg.window = dragAndDropSourceWindow;
        msg.format = 32;
        msg.data.l[0] = windowH;

        ScopedXLock xlock;
        XSendEvent (display, dragAndDropSourceWindow, False, 0, (XEvent*) &msg);
    }

    void sendDragAndDropStatus (const bool acceptDrop, Atom dropAction)
    {
        XClientMessageEvent msg = { 0 };
        msg.message_type = Atoms::XdndStatus;
        msg.data.l[1] = (acceptDrop ? 1 : 0) | 2; // 2 indicates that we want to receive position messages
        msg.data.l[4] = dropAction;

        sendDragAndDropMessage (msg);
    }

    void sendDragAndDropLeave()
    {
        XClientMessageEvent msg = { 0 };
        msg.message_type = Atoms::XdndLeave;
        sendDragAndDropMessage (msg);
    }

    void sendDragAndDropFinish()
    {
        XClientMessageEvent msg = { 0 };
        msg.message_type = Atoms::XdndFinished;
        sendDragAndDropMessage (msg);
    }

    void handleDragAndDropStatus (const XClientMessageEvent* const clientMsg)
    {
        if ((clientMsg->data.l[1] & 1) == 0)
        {
            sendDragAndDropLeave();

            if (dragInfo.files.size() > 0)
                handleDragExit (dragInfo);

            dragInfo.files.clear();
        }
    }

    void handleDragAndDropPosition (const XClientMessageEvent* const clientMsg)
    {
        if (dragAndDropSourceWindow == 0)
            return;

        dragAndDropSourceWindow = clientMsg->data.l[0];

        Point<int> dropPos ((int) clientMsg->data.l[2] >> 16,
                            (int) clientMsg->data.l[2] & 0xffff);
        dropPos -= getScreenPosition();

        if (dragInfo.position != dropPos)
        {
            dragInfo.position = dropPos;

            Atom targetAction = Atoms::XdndActionCopy;

            for (int i = numElementsInArray (Atoms::allowedActions); --i >= 0;)
            {
                if ((Atom) clientMsg->data.l[4] == Atoms::allowedActions[i])
                {
                    targetAction = Atoms::allowedActions[i];
                    break;
                }
            }

            sendDragAndDropStatus (true, targetAction);

            if (dragInfo.files.size() == 0)
                updateDraggedFileList (clientMsg);

            if (dragInfo.files.size() > 0)
                handleDragMove (dragInfo);
        }
    }

    void handleDragAndDropDrop (const XClientMessageEvent* const clientMsg)
    {
        if (dragInfo.files.size() == 0)
            updateDraggedFileList (clientMsg);

        DragInfo dragInfoCopy (dragInfo);

        sendDragAndDropFinish();
        resetDragAndDrop();

        if (dragInfoCopy.files.size() > 0)
            handleDragDrop (dragInfoCopy);
    }

    void handleDragAndDropEnter (const XClientMessageEvent* const clientMsg)
    {
        dragInfo.files.clear();
        srcMimeTypeAtomList.clear();

        dragAndDropCurrentMimeType = 0;
        const unsigned long dndCurrentVersion = static_cast <unsigned long> (clientMsg->data.l[1] & 0xff000000) >> 24;

        if (dndCurrentVersion < 3 || dndCurrentVersion > Atoms::DndVersion)
        {
            dragAndDropSourceWindow = 0;
            return;
        }

        dragAndDropSourceWindow = clientMsg->data.l[0];

        if ((clientMsg->data.l[1] & 1) != 0)
        {
            Atom actual;
            int format;
            unsigned long count = 0, remaining = 0;
            unsigned char* data = 0;

            ScopedXLock xlock;
            XGetWindowProperty (display, dragAndDropSourceWindow, Atoms::XdndTypeList,
                                0, 0x8000000L, False, XA_ATOM, &actual, &format,
                                &count, &remaining, &data);

            if (data != 0)
            {
                if (actual == XA_ATOM && format == 32 && count != 0)
                {
                    const unsigned long* const types = (const unsigned long*) data;

                    for (unsigned int i = 0; i < count; ++i)
                        if (types[i] != None)
                            srcMimeTypeAtomList.add (types[i]);
                }

                XFree (data);
            }
        }

        if (srcMimeTypeAtomList.size() == 0)
        {
            for (int i = 2; i < 5; ++i)
                if (clientMsg->data.l[i] != None)
                    srcMimeTypeAtomList.add (clientMsg->data.l[i]);

            if (srcMimeTypeAtomList.size() == 0)
            {
                dragAndDropSourceWindow = 0;
                return;
            }
        }

        for (int i = 0; i < srcMimeTypeAtomList.size() && dragAndDropCurrentMimeType == 0; ++i)
            for (int j = 0; j < numElementsInArray (Atoms::allowedMimeTypes); ++j)
                if (srcMimeTypeAtomList[i] == Atoms::allowedMimeTypes[j])
                    dragAndDropCurrentMimeType = Atoms::allowedMimeTypes[j];

        handleDragAndDropPosition (clientMsg);
    }

    void handleDragAndDropSelection (const XEvent* const evt)
    {
        dragInfo.files.clear();

        if (evt->xselection.property != 0)
        {
            StringArray lines;

            {
                MemoryBlock dropData;

                for (;;)
                {
                    Atom actual;
                    uint8* data = 0;
                    unsigned long count = 0, remaining = 0;
                    int format = 0;
                    ScopedXLock xlock;

                    if (XGetWindowProperty (display, evt->xany.window, evt->xselection.property,
                                            dropData.getSize() / 4, 65536, 1, AnyPropertyType, &actual,
                                            &format, &count, &remaining, &data) == Success)
                    {
                        dropData.append (data, count * format / 8);
                        XFree (data);

                        if (remaining == 0)
                            break;
                    }
                    else
                    {
                        XFree (data);
                        break;
                    }
                }

                lines.addLines (dropData.toString());
            }

            for (int i = 0; i < lines.size(); ++i)
                dragInfo.files.add (URL::removeEscapeChars (lines[i].fromFirstOccurrenceOf ("file://", false, true)));

            dragInfo.files.trim();
            dragInfo.files.removeEmptyStrings();
        }
    }

    void updateDraggedFileList (const XClientMessageEvent* const clientMsg)
    {
        dragInfo.files.clear();

        if (dragAndDropSourceWindow != None
             && dragAndDropCurrentMimeType != 0)
        {
            ScopedXLock xlock;
            XConvertSelection (display,
                               Atoms::XdndSelection,
                               dragAndDropCurrentMimeType,
                               Atoms::getCreating ("JXSelectionWindowProperty"),
                               windowH,
                               clientMsg->data.l[2]);
        }
    }

    DragInfo dragInfo;
    Atom dragAndDropCurrentMimeType;
    Window dragAndDropSourceWindow;

    Array <Atom> srcMimeTypeAtomList;

    int pointerMap[5];

    void initialisePointerMap()
    {
        const int numButtons = XGetPointerMapping (display, 0, 0);
        pointerMap[2] = pointerMap[3] = pointerMap[4] = Keys::NoButton;

        if (numButtons == 2)
        {
            pointerMap[0] = Keys::LeftButton;
            pointerMap[1] = Keys::RightButton;
        }
        else if (numButtons >= 3)
        {
            pointerMap[0] = Keys::LeftButton;
            pointerMap[1] = Keys::MiddleButton;
            pointerMap[2] = Keys::RightButton;

            if (numButtons >= 5)
            {
                pointerMap[3] = Keys::WheelUp;
                pointerMap[4] = Keys::WheelDown;
            }
        }
    }

    static Point<int> lastMousePos;

    static void clearLastMousePos() noexcept
    {
        lastMousePos = Point<int> (0x100000, 0x100000);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinuxComponentPeer);
};

ModifierKeys LinuxComponentPeer::currentModifiers;
bool LinuxComponentPeer::isActiveApplication = false;
Point<int> LinuxComponentPeer::lastMousePos;

//==============================================================================
bool Process::isForegroundProcess()
{
    return LinuxComponentPeer::isActiveApplication;
}

//==============================================================================
void ModifierKeys::updateCurrentModifiers() noexcept
{
    currentModifiers = LinuxComponentPeer::currentModifiers;
}

ModifierKeys ModifierKeys::getCurrentModifiersRealtime() noexcept
{
    Window root, child;
    int x, y, winx, winy;
    unsigned int mask;
    int mouseMods = 0;

    ScopedXLock xlock;

    if (XQueryPointer (display, RootWindow (display, DefaultScreen (display)),
                       &root, &child, &x, &y, &winx, &winy, &mask) != False)
    {
        if ((mask & Button1Mask) != 0)  mouseMods |= ModifierKeys::leftButtonModifier;
        if ((mask & Button2Mask) != 0)  mouseMods |= ModifierKeys::middleButtonModifier;
        if ((mask & Button3Mask) != 0)  mouseMods |= ModifierKeys::rightButtonModifier;
    }

    LinuxComponentPeer::currentModifiers = LinuxComponentPeer::currentModifiers.withoutMouseButtons().withFlags (mouseMods);
    return LinuxComponentPeer::currentModifiers;
}


//==============================================================================
void Desktop::setKioskComponent (Component* kioskModeComponent, bool enableOrDisable, bool allowMenusAndBars)
{
    if (enableOrDisable)
        kioskModeComponent->setBounds (Desktop::getInstance().getDisplays().getMainDisplay().totalArea);
}

//==============================================================================
ComponentPeer* Component::createNewPeer (int styleFlags, void* nativeWindowToAttachTo)
{
    return new LinuxComponentPeer (this, styleFlags, (Window) nativeWindowToAttachTo);
}


//==============================================================================
// (this callback is hooked up in the messaging code)
void juce_windowMessageReceive (XEvent* event)
{
    if (event->xany.window != None)
    {
        LinuxComponentPeer* const peer = LinuxComponentPeer::getPeerFor (event->xany.window);

        if (ComponentPeer::isValidPeer (peer))
            peer->handleWindowMessage (event);
    }
    else
    {
        switch (event->xany.type)
        {
            case KeymapNotify:
            {
                const XKeymapEvent* const keymapEvent = (const XKeymapEvent*) &event->xkeymap;
                memcpy (Keys::keyStates, keymapEvent->key_vector, 32);
                break;
            }

            default:
                break;
        }
    }
}

//==============================================================================
void Desktop::Displays::findDisplays()
{
    if (display == 0)
        return;

  #if JUCE_USE_XINERAMA
    int major_opcode, first_event, first_error;

    ScopedXLock xlock;
    if (XQueryExtension (display, "XINERAMA", &major_opcode, &first_event, &first_error))
    {
        typedef Bool (*tXineramaIsActive) (::Display*);
        typedef XineramaScreenInfo* (*tXineramaQueryScreens) (::Display*, int*);

        static tXineramaIsActive xXineramaIsActive = 0;
        static tXineramaQueryScreens xXineramaQueryScreens = 0;

        if (xXineramaIsActive == 0 || xXineramaQueryScreens == 0)
        {
            void* h = dlopen ("libXinerama.so", RTLD_GLOBAL | RTLD_NOW);

            if (h == 0)
                h = dlopen ("libXinerama.so.1", RTLD_GLOBAL | RTLD_NOW);

            if (h != 0)
            {
                xXineramaIsActive = (tXineramaIsActive) dlsym (h, "XineramaIsActive");
                xXineramaQueryScreens = (tXineramaQueryScreens) dlsym (h, "XineramaQueryScreens");
            }
        }

        if (xXineramaIsActive != 0
            && xXineramaQueryScreens != 0
            && xXineramaIsActive (display))
        {
            int numMonitors = 0;
            XineramaScreenInfo* const screens = xXineramaQueryScreens (display, &numMonitors);

            if (screens != nullptr)
            {
                for (int index = 0; index < numMonitors; ++index)
                {
                    for (int j = numMonitors; --j >= 0;)
                    {
                        if (screens[j].screen_number == index)
                        {
                            Display d;
                            d.userArea = d.totalArea = Rectangle<int> (screens[j].x_org,
                                                                       screens[j].y_org,
                                                                       screens[j].width,
                                                                       screens[j].height);
                            d.isMain = (index == 0);
                            d.scale = 1.0;

                            displays.add (d);
                        }
                    }
                }

                XFree (screens);
            }
        }
    }

    if (displays.size() == 0)
  #endif
    {
        Atom hints = Atoms::getIfExists ("_NET_WORKAREA");

        if (hints != None)
        {
            const int numMonitors = ScreenCount (display);

            for (int i = 0; i < numMonitors; ++i)
            {
                Window root = RootWindow (display, i);

                unsigned long nitems, bytesLeft;
                Atom actualType;
                int actualFormat;
                unsigned char* data = nullptr;

                if (XGetWindowProperty (display, root, hints, 0, 4, False,
                                        XA_CARDINAL, &actualType, &actualFormat, &nitems, &bytesLeft,
                                        &data) == Success)
                {
                    const long* const position = (const long*) data;

                    if (actualType == XA_CARDINAL && actualFormat == 32 && nitems == 4)
                    {
                        Display d;
                        d.userArea = d.totalArea = Rectangle<int> (position[0], position[1],
                                                                   position[2], position[3]);
                        d.isMain = (displays.size() == 0);
                        d.scale = 1.0;

                        displays.add (d);
                    }

                    XFree (data);
                }
            }
        }

        if (displays.size() == 0)
        {
            Display d;
            d.userArea = d.totalArea = Rectangle<int> (DisplayWidth (display, DefaultScreen (display)),
                                                       DisplayHeight (display, DefaultScreen (display)));
            d.isMain = true;
            d.scale = 1.0;

            displays.add (d);
        }
    }
}

//==============================================================================
void Desktop::createMouseInputSources()
{
    mouseSources.add (new MouseInputSource (0, true));
}

bool Desktop::canUseSemiTransparentWindows() noexcept
{
    int matchedDepth = 0;
    const int desiredDepth = 32;

    return Visuals::findVisualFormat (desiredDepth, matchedDepth) != 0
             && (matchedDepth == desiredDepth);
}

Point<int> MouseInputSource::getCurrentMousePosition()
{
    Window root, child;
    int x, y, winx, winy;
    unsigned int mask;

    ScopedXLock xlock;

    if (XQueryPointer (display,
                       RootWindow (display, DefaultScreen (display)),
                       &root, &child,
                       &x, &y, &winx, &winy, &mask) == False)
    {
        // Pointer not on the default screen
        x = y = -1;
    }

    return Point<int> (x, y);
}

void Desktop::setMousePosition (const Point<int>& newPosition)
{
    ScopedXLock xlock;
    Window root = RootWindow (display, DefaultScreen (display));
    XWarpPointer (display, None, root, 0, 0, 0, 0, newPosition.getX(), newPosition.getY());
}

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const
{
    return upright;
}

//==============================================================================
static bool screenSaverAllowed = true;

void Desktop::setScreenSaverEnabled (const bool isEnabled)
{
    if (screenSaverAllowed != isEnabled)
    {
        screenSaverAllowed = isEnabled;

        typedef void (*tXScreenSaverSuspend) (Display*, Bool);
        static tXScreenSaverSuspend xScreenSaverSuspend = 0;

        if (xScreenSaverSuspend == 0)
        {
            void* h = dlopen ("libXss.so", RTLD_GLOBAL | RTLD_NOW);

            if (h != 0)
                xScreenSaverSuspend = (tXScreenSaverSuspend) dlsym (h, "XScreenSaverSuspend");
        }

        ScopedXLock xlock;
        if (xScreenSaverSuspend != 0)
            xScreenSaverSuspend (display, ! isEnabled);
    }
}

bool Desktop::isScreenSaverEnabled()
{
    return screenSaverAllowed;
}

//==============================================================================
void* MouseCursor::createMouseCursorFromImage (const Image& image, int hotspotX, int hotspotY)
{
    ScopedXLock xlock;
    const unsigned int imageW = image.getWidth();
    const unsigned int imageH = image.getHeight();

  #if JUCE_USE_XCURSOR
    {
        typedef XcursorBool (*tXcursorSupportsARGB) (Display*);
        typedef XcursorImage* (*tXcursorImageCreate) (int, int);
        typedef void (*tXcursorImageDestroy) (XcursorImage*);
        typedef Cursor (*tXcursorImageLoadCursor) (Display*, const XcursorImage*);

        static tXcursorSupportsARGB xXcursorSupportsARGB = 0;
        static tXcursorImageCreate xXcursorImageCreate = 0;
        static tXcursorImageDestroy xXcursorImageDestroy = 0;
        static tXcursorImageLoadCursor xXcursorImageLoadCursor = 0;
        static bool hasBeenLoaded = false;

        if (! hasBeenLoaded)
        {
            hasBeenLoaded = true;
            void* h = dlopen ("libXcursor.so", RTLD_GLOBAL | RTLD_NOW);

            if (h != 0)
            {
                xXcursorSupportsARGB    = (tXcursorSupportsARGB)    dlsym (h, "XcursorSupportsARGB");
                xXcursorImageCreate     = (tXcursorImageCreate)     dlsym (h, "XcursorImageCreate");
                xXcursorImageLoadCursor = (tXcursorImageLoadCursor) dlsym (h, "XcursorImageLoadCursor");
                xXcursorImageDestroy    = (tXcursorImageDestroy)    dlsym (h, "XcursorImageDestroy");

                if (xXcursorSupportsARGB == 0 || xXcursorImageCreate == 0
                      || xXcursorImageLoadCursor == 0 || xXcursorImageDestroy == 0
                      || ! xXcursorSupportsARGB (display))
                    xXcursorSupportsARGB = 0;
            }
        }

        if (xXcursorSupportsARGB != 0)
        {
            XcursorImage* xcImage = xXcursorImageCreate (imageW, imageH);

            if (xcImage != 0)
            {
                xcImage->xhot = hotspotX;
                xcImage->yhot = hotspotY;
                XcursorPixel* dest = xcImage->pixels;

                for (int y = 0; y < (int) imageH; ++y)
                    for (int x = 0; x < (int) imageW; ++x)
                        *dest++ = image.getPixelAt (x, y).getARGB();

                void* result = (void*) xXcursorImageLoadCursor (display, xcImage);
                xXcursorImageDestroy (xcImage);

                if (result != 0)
                    return result;
            }
        }
    }
  #endif

    Window root = RootWindow (display, DefaultScreen (display));
    unsigned int cursorW, cursorH;
    if (! XQueryBestCursor (display, root, imageW, imageH, &cursorW, &cursorH))
        return nullptr;

    Image im (Image::ARGB, cursorW, cursorH, true);

    {
        Graphics g (im);

        if (imageW > cursorW || imageH > cursorH)
        {
            hotspotX = (hotspotX * cursorW) / imageW;
            hotspotY = (hotspotY * cursorH) / imageH;

            g.drawImageWithin (image, 0, 0, imageW, imageH,
                               RectanglePlacement::xLeft | RectanglePlacement::yTop | RectanglePlacement::onlyReduceInSize,
                               false);
        }
        else
        {
            g.drawImageAt (image, 0, 0);
        }
    }

    const int stride = (cursorW + 7) >> 3;
    HeapBlock <char> maskPlane, sourcePlane;
    maskPlane.calloc (stride * cursorH);
    sourcePlane.calloc (stride * cursorH);

    const bool msbfirst = (BitmapBitOrder (display) == MSBFirst);

    for (int y = cursorH; --y >= 0;)
    {
        for (int x = cursorW; --x >= 0;)
        {
            const char mask = (char) (1 << (msbfirst ? (7 - (x & 7)) : (x & 7)));
            const int offset = y * stride + (x >> 3);

            const Colour c (im.getPixelAt (x, y));

            if (c.getAlpha() >= 128)
                maskPlane[offset] |= mask;

            if (c.getBrightness() >= 0.5f)
                sourcePlane[offset] |= mask;
        }
    }

    Pixmap sourcePixmap = XCreatePixmapFromBitmapData (display, root, sourcePlane.getData(), cursorW, cursorH, 0xffff, 0, 1);
    Pixmap maskPixmap = XCreatePixmapFromBitmapData (display, root, maskPlane.getData(), cursorW, cursorH, 0xffff, 0, 1);

    XColor white, black;
    black.red = black.green = black.blue = 0;
    white.red = white.green = white.blue = 0xffff;

    void* result = (void*) XCreatePixmapCursor (display, sourcePixmap, maskPixmap, &white, &black, hotspotX, hotspotY);

    XFreePixmap (display, sourcePixmap);
    XFreePixmap (display, maskPixmap);

    return result;
}

void MouseCursor::deleteMouseCursor (void* const cursorHandle, const bool)
{
    ScopedXLock xlock;
    if (cursorHandle != 0)
        XFreeCursor (display, (Cursor) cursorHandle);
}

void* MouseCursor::createStandardMouseCursor (MouseCursor::StandardCursorType type)
{
    unsigned int shape;

    switch (type)
    {
        case NormalCursor:                  return None; // Use parent cursor
        case NoCursor:                      return createMouseCursorFromImage (Image (Image::ARGB, 16, 16, true), 0, 0);

        case WaitCursor:                    shape = XC_watch; break;
        case IBeamCursor:                   shape = XC_xterm; break;
        case PointingHandCursor:            shape = XC_hand2; break;
        case LeftRightResizeCursor:         shape = XC_sb_h_double_arrow; break;
        case UpDownResizeCursor:            shape = XC_sb_v_double_arrow; break;
        case UpDownLeftRightResizeCursor:   shape = XC_fleur; break;
        case TopEdgeResizeCursor:           shape = XC_top_side; break;
        case BottomEdgeResizeCursor:        shape = XC_bottom_side; break;
        case LeftEdgeResizeCursor:          shape = XC_left_side; break;
        case RightEdgeResizeCursor:         shape = XC_right_side; break;
        case TopLeftCornerResizeCursor:     shape = XC_top_left_corner; break;
        case TopRightCornerResizeCursor:    shape = XC_top_right_corner; break;
        case BottomLeftCornerResizeCursor:  shape = XC_bottom_left_corner; break;
        case BottomRightCornerResizeCursor: shape = XC_bottom_right_corner; break;
        case CrosshairCursor:               shape = XC_crosshair; break;

        case DraggingHandCursor:
        {
            static unsigned char dragHandData[] = { 71,73,70,56,57,97,16,0,16,0,145,2,0,0,0,0,255,255,255,0,
              0,0,0,0,0,33,249,4,1,0,0,2,0,44,0,0,0,0,16,0, 16,0,0,2,52,148,47,0,200,185,16,130,90,12,74,139,107,84,123,39,
              132,117,151,116,132,146,248,60,209,138,98,22,203,114,34,236,37,52,77,217, 247,154,191,119,110,240,193,128,193,95,163,56,60,234,98,135,2,0,59 };
            const int dragHandDataSize = 99;

            return createMouseCursorFromImage (ImageFileFormat::loadFrom (dragHandData, dragHandDataSize), 8, 7);
        }

        case CopyingCursor:
        {
            static unsigned char copyCursorData[] = { 71,73,70,56,57,97,21,0,21,0,145,0,0,0,0,0,255,255,255,0,
              128,128,255,255,255,33,249,4,1,0,0,3,0,44,0,0,0,0,21,0, 21,0,0,2,72,4,134,169,171,16,199,98,11,79,90,71,161,93,56,111,
              78,133,218,215,137,31,82,154,100,200,86,91,202,142,12,108,212,87,235,174, 15,54,214,126,237,226,37,96,59,141,16,37,18,201,142,157,230,204,51,112,
              252,114,147,74,83,5,50,68,147,208,217,16,71,149,252,124,5,0,59,0,0 };
            const int copyCursorSize = 119;

            return createMouseCursorFromImage (ImageFileFormat::loadFrom (copyCursorData, copyCursorSize), 1, 3);
        }

        default:
            jassertfalse;
            return None;
    }

    ScopedXLock xlock;
    return (void*) XCreateFontCursor (display, shape);
}

void MouseCursor::showInWindow (ComponentPeer* peer) const
{
    LinuxComponentPeer* const lp = dynamic_cast <LinuxComponentPeer*> (peer);

    if (lp != 0)
        lp->showMouseCursor ((Cursor) getHandle());
}

void MouseCursor::showInAllWindows() const
{
    for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
        showInWindow (ComponentPeer::getPeer (i));
}

//==============================================================================
Image juce_createIconForFile (const File& file)
{
    return Image::null;
}

ImagePixelData* NativeImageType::create (Image::PixelFormat format, int width, int height, bool clearImage) const
{
    return SoftwareImageType().create (format, width, height, clearImage);
}

//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const bool canMoveFiles)
{
    jassertfalse;    // not implemented!
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text)
{
    jassertfalse;    // not implemented!
    return false;
}

//==============================================================================
void LookAndFeel::playAlertSound()
{
    std::cout << "\a" << std::flush;
}


//==============================================================================
void JUCE_CALLTYPE NativeMessageBox::showMessageBox (AlertWindow::AlertIconType iconType,
                                                     const String& title, const String& message,
                                                     Component* associatedComponent)
{
    AlertWindow::showMessageBox (AlertWindow::NoIcon, title, message);
}

void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (AlertWindow::AlertIconType iconType,
                                                          const String& title, const String& message,
                                                          Component* associatedComponent)
{
    AlertWindow::showMessageBoxAsync (AlertWindow::NoIcon, title, message);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (AlertWindow::AlertIconType iconType,
                                                      const String& title, const String& message,
                                                      Component* associatedComponent,
                                                      ModalComponentManager::Callback* callback)
{
    return AlertWindow::showOkCancelBox (iconType, title, message, String::empty, String::empty,
                                         associatedComponent, callback);
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoCancelBox (AlertWindow::AlertIconType iconType,
                                                        const String& title, const String& message,
                                                        Component* associatedComponent,
                                                        ModalComponentManager::Callback* callback)
{
    return AlertWindow::showYesNoCancelBox (iconType, title, message,
                                            String::empty, String::empty, String::empty,
                                            associatedComponent, callback);
}


//==============================================================================
const int KeyPress::spaceKey                = XK_space & 0xff;
const int KeyPress::returnKey               = XK_Return & 0xff;
const int KeyPress::escapeKey               = XK_Escape & 0xff;
const int KeyPress::backspaceKey            = XK_BackSpace & 0xff;
const int KeyPress::leftKey                 = (XK_Left & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::rightKey                = (XK_Right & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::upKey                   = (XK_Up & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::downKey                 = (XK_Down & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::pageUpKey               = (XK_Page_Up & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::pageDownKey             = (XK_Page_Down & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::endKey                  = (XK_End & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::homeKey                 = (XK_Home & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::insertKey               = (XK_Insert & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::deleteKey               = (XK_Delete & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::tabKey                  = XK_Tab & 0xff;
const int KeyPress::F1Key                   = (XK_F1 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F2Key                   = (XK_F2 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F3Key                   = (XK_F3 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F4Key                   = (XK_F4 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F5Key                   = (XK_F5 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F6Key                   = (XK_F6 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F7Key                   = (XK_F7 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F8Key                   = (XK_F8 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F9Key                   = (XK_F9 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F10Key                  = (XK_F10 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F11Key                  = (XK_F11 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F12Key                  = (XK_F12 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F13Key                  = (XK_F13 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F14Key                  = (XK_F14 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F15Key                  = (XK_F15 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F16Key                  = (XK_F16 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::numberPad0              = (XK_KP_0 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::numberPad1              = (XK_KP_1 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::numberPad2              = (XK_KP_2 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::numberPad3              = (XK_KP_3 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::numberPad4              = (XK_KP_4 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::numberPad5              = (XK_KP_5 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::numberPad6              = (XK_KP_6 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::numberPad7              = (XK_KP_7 & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPad8              = (XK_KP_8 & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPad9              = (XK_KP_9 & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadAdd            = (XK_KP_Add & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadSubtract       = (XK_KP_Subtract & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadMultiply       = (XK_KP_Multiply & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadDivide         = (XK_KP_Divide & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadSeparator      = (XK_KP_Separator & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadDecimalPoint   = (XK_KP_Decimal & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadEquals         = (XK_KP_Equal & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadDelete         = (XK_KP_Delete & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::playKey                 = (0xffeeff00) | Keys::extendedKeyModifier;
const int KeyPress::stopKey                 = (0xffeeff01) | Keys::extendedKeyModifier;
const int KeyPress::fastForwardKey          = (0xffeeff02) | Keys::extendedKeyModifier;
const int KeyPress::rewindKey               = (0xffeeff03) | Keys::extendedKeyModifier;
