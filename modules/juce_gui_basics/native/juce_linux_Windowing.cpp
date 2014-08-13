/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

extern Display* display;
extern XContext windowHandleXContext;
typedef void (*WindowMessageReceiveCallback) (XEvent&);
extern WindowMessageReceiveCallback dispatchWindowMessage;


//==============================================================================
struct Atoms
{
    Atoms()
    {
        protocols                       = getIfExists ("WM_PROTOCOLS");
        protocolList [TAKE_FOCUS]       = getIfExists ("WM_TAKE_FOCUS");
        protocolList [DELETE_WINDOW]    = getIfExists ("WM_DELETE_WINDOW");
        protocolList [PING]             = getIfExists ("_NET_WM_PING");
        changeState                     = getIfExists ("WM_CHANGE_STATE");
        state                           = getIfExists ("WM_STATE");
        userTime                        = getCreating ("_NET_WM_USER_TIME");
        activeWin                       = getCreating ("_NET_ACTIVE_WINDOW");
        pid                             = getCreating ("_NET_WM_PID");
        windowType                      = getIfExists ("_NET_WM_WINDOW_TYPE");
        windowState                     = getIfExists ("_NET_WM_STATE");
        compositingManager              = getCreating ("_NET_WM_CM_S0");

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
        XdndActionPrivate               = getCreating ("XdndActionPrivate");
        XdndActionDescription           = getCreating ("XdndActionDescription");

        allowedMimeTypes[0]             = getCreating ("UTF8_STRING");
        allowedMimeTypes[1]             = getCreating ("text/plain;charset=utf-8");
        allowedMimeTypes[2]             = getCreating ("text/plain");
        allowedMimeTypes[3]             = getCreating ("text/uri-list");

        externalAllowedFileMimeTypes[0] = getCreating ("text/uri-list");
        externalAllowedTextMimeTypes[0] = getCreating ("text/plain");

        allowedActions[0]               = getCreating ("XdndActionMove");
        allowedActions[1]               = XdndActionCopy;
        allowedActions[2]               = getCreating ("XdndActionLink");
        allowedActions[3]               = getCreating ("XdndActionAsk");
        allowedActions[4]               = XdndActionPrivate;
    }

    static const Atoms& get()
    {
        static Atoms atoms;
        return atoms;
    }

    enum ProtocolItems
    {
        TAKE_FOCUS = 0,
        DELETE_WINDOW = 1,
        PING = 2
    };

    Atom protocols, protocolList[3], changeState, state, userTime,
         activeWin, pid, windowType, windowState, compositingManager,
         XdndAware, XdndEnter, XdndLeave, XdndPosition, XdndStatus,
         XdndDrop, XdndFinished, XdndSelection, XdndTypeList, XdndActionList,
         XdndActionDescription, XdndActionCopy, XdndActionPrivate,
         allowedActions[5],
         allowedMimeTypes[4],
         externalAllowedFileMimeTypes[1],
         externalAllowedTextMimeTypes[1];

    static const unsigned long DndVersion;

    static Atom getIfExists (const char* name)    { return XInternAtom (display, name, True); }
    static Atom getCreating (const char* name)    { return XInternAtom (display, name, False); }

    static String getName (const Atom atom)
    {
        if (atom == None)
            return "None";

        return String (XGetAtomName (display, atom));
    }

    static bool isMimeTypeFile (const Atom atom)  { return getName (atom).equalsIgnoreCase ("text/uri-list"); }
};

const unsigned long Atoms::DndVersion = 3;

//==============================================================================
struct GetXProperty
{
    GetXProperty (Window window, Atom atom, long offset, long length, bool shouldDelete, Atom requestedType)
        : data (nullptr)
    {
        success = (XGetWindowProperty (display, window, atom, offset, length,
                                       (Bool) shouldDelete, requestedType, &actualType,
                                       &actualFormat, &numItems, &bytesLeft, &data) == Success)
                    && data != nullptr;
    }

    ~GetXProperty()
    {
        if (data != nullptr)
            XFree (data);
    }

    bool success;
    unsigned char* data;
    unsigned long numItems, bytesLeft;
    Atom actualType;
    int actualFormat;
};

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

                XShmSegmentInfo segmentInfo;
                zerostruct (segmentInfo);

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
    typedef XRenderPictFormat* (*tXRenderFindStandardFormat) (Display*, int);
    typedef XRenderPictFormat* (*tXRenderFindFormat) (Display*, unsigned long, XRenderPictFormat*, int);
    typedef XRenderPictFormat* (*tXRenderFindVisualFormat) (Display*, Visual*);

    static tXRenderQueryVersion xRenderQueryVersion = nullptr;
    static tXRenderFindStandardFormat xRenderFindStandardFormat = nullptr;
    static tXRenderFindFormat xRenderFindFormat = nullptr;
    static tXRenderFindVisualFormat xRenderFindVisualFormat = nullptr;

    static bool isAvailable()
    {
        static bool hasLoaded = false;

        if (! hasLoaded)
        {
            ScopedXLock xlock;
            hasLoaded = true;

            if (void* h = dlopen ("libXrender.so", RTLD_GLOBAL | RTLD_NOW))
            {
                xRenderQueryVersion         = (tXRenderQueryVersion)        dlsym (h, "XRenderQueryVersion");
                xRenderFindStandardFormat   = (tXRenderFindStandardFormat)  dlsym (h, "XRenderFindStandardFormat");
                xRenderFindFormat           = (tXRenderFindFormat)          dlsym (h, "XRenderFindFormat");
                xRenderFindVisualFormat     = (tXRenderFindVisualFormat)    dlsym (h, "XRenderFindVisualFormat");
            }

            if (xRenderQueryVersion != nullptr
                 && xRenderFindStandardFormat != nullptr
                 && xRenderFindFormat != nullptr
                 && xRenderFindVisualFormat != nullptr)
            {
                int major, minor;
                if (xRenderQueryVersion (display, &major, &minor))
                    return true;
            }

            xRenderQueryVersion = nullptr;
        }

        return xRenderQueryVersion != nullptr;
    }

    static bool hasCompositingWindowManager()
    {
        return XGetSelectionOwner (display, Atoms::get().compositingManager) != 0;
    }

    static XRenderPictFormat* findPictureFormat()
    {
        ScopedXLock xlock;
        XRenderPictFormat* pictFormat = nullptr;

        if (isAvailable())
        {
            pictFormat = xRenderFindStandardFormat (display, PictStandardARGB32);

            if (pictFormat == nullptr)
            {
                XRenderPictFormat desiredFormat;
                desiredFormat.type = PictTypeDirect;
                desiredFormat.depth = 32;

                desiredFormat.direct.alphaMask = 0xff;
                desiredFormat.direct.redMask   = 0xff;
                desiredFormat.direct.greenMask = 0xff;
                desiredFormat.direct.blueMask  = 0xff;

                desiredFormat.direct.alpha = 24;
                desiredFormat.direct.red   = 16;
                desiredFormat.direct.green = 8;
                desiredFormat.direct.blue  = 0;

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

        if (xvinfos != nullptr)
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
                        if (xvinfos != nullptr)
                        {
                            for (int i = 0; i < numVisuals; ++i)
                            {
                                XRenderPictFormat* pictVisualFormat = XRender::xRenderFindVisualFormat (display, xvinfos[i].visual);

                                if (pictVisualFormat != nullptr
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
                if (visual == nullptr)
                {
                    visual = findVisualWithDepth (32);
                    if (visual != nullptr)
                        matchedDepth = 32;
                }
            }
           #endif
        }

        if (visual == nullptr && desiredDepth >= 24)
        {
            visual = findVisualWithDepth (24);
            if (visual != nullptr)
                matchedDepth = 24;
        }

        if (visual == nullptr && desiredDepth >= 16)
        {
            visual = findVisualWithDepth (16);
            if (visual != nullptr)
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

            if (xImage != nullptr)
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

    LowLevelGraphicsContext* createLowLevelContext() override
    {
        sendDataChangeMessage();
        return new LowLevelGraphicsSoftwareRenderer (Image (this));
    }

    void initialiseBitmapData (Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode mode) override
    {
        bitmap.data = imageData + x * pixelStride + y * lineStride;
        bitmap.pixelFormat = pixelFormat;
        bitmap.lineStride = lineStride;
        bitmap.pixelStride = pixelStride;

        if (mode != Image::BitmapData::readOnly)
            sendDataChangeMessage();
    }

    ImagePixelData* clone() override
    {
        jassertfalse;
        return nullptr;
    }

    ImageType* createType() const override     { return new NativeImageType(); }

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
            const uint32 rMask   = xImage->red_mask;
            const uint32 gMask   = xImage->green_mask;
            const uint32 bMask   = xImage->blue_mask;
            const uint32 rShiftL = jmax (0,  getShiftNeeded (rMask));
            const uint32 rShiftR = jmax (0, -getShiftNeeded (rMask));
            const uint32 gShiftL = jmax (0,  getShiftNeeded (gMask));
            const uint32 gShiftR = jmax (0, -getShiftNeeded (gMask));
            const uint32 bShiftL = jmax (0,  getShiftNeeded (bMask));
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XBitmapImage)
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

static void* createDraggingHandCursor()
{
    static unsigned char dragHandData[] = { 71,73,70,56,57,97,16,0,16,0,145,2,0,0,0,0,255,255,255,0,
      0,0,0,0,0,33,249,4,1,0,0,2,0,44,0,0,0,0,16,0, 16,0,0,2,52,148,47,0,200,185,16,130,90,12,74,139,107,84,123,39,
      132,117,151,116,132,146,248,60,209,138,98,22,203,114,34,236,37,52,77,217, 247,154,191,119,110,240,193,128,193,95,163,56,60,234,98,135,2,0,59 };
    const int dragHandDataSize = 99;

    return CustomMouseCursorInfo (ImageFileFormat::loadFrom (dragHandData, dragHandDataSize), 8, 7).create();
}

//==============================================================================
static int numAlwaysOnTopPeers = 0;

bool juce_areThereAnyAlwaysOnTopWindows()
{
    return numAlwaysOnTopPeers > 0;
}

//==============================================================================
class LinuxComponentPeer  : public ComponentPeer
{
public:
    LinuxComponentPeer (Component& comp, const int windowStyleFlags, Window parentToAddTo)
        : ComponentPeer (comp, windowStyleFlags),
          windowH (0), parentWindow (0),
          fullScreen (false), mapped (false),
          visual (nullptr), depth (0),
          isAlwaysOnTop (comp.isAlwaysOnTop())
    {
        // it's dangerous to create a window on a thread other than the message thread..
        jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

        dispatchWindowMessage = windowMessageReceive;
        repainter = new LinuxRepaintManager (*this);

        if (isAlwaysOnTop)
            ++numAlwaysOnTopPeers;

        createWindow (parentToAddTo);

        setTitle (component.getName());
    }

    ~LinuxComponentPeer()
    {
        // it's dangerous to delete a window on a thread other than the message thread..
        jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

        deleteIconPixmaps();
        destroyWindow();
        windowH = 0;

        if (isAlwaysOnTop)
            --numAlwaysOnTopPeers;
    }

    // (this callback is hooked up in the messaging code)
    static void windowMessageReceive (XEvent& event)
    {
        if (event.xany.window != None)
        {
            if (LinuxComponentPeer* const peer = getPeerFor (event.xany.window))
                peer->handleWindowMessage (event);
        }
        else if (event.xany.type == KeymapNotify)
        {
            const XKeymapEvent& keymapEvent = (const XKeymapEvent&) event.xkeymap;
            memcpy (Keys::keyStates, keymapEvent.key_vector, 32);
        }
    }

    //==============================================================================
    void* getNativeHandle() const override
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

    void setVisible (bool shouldBeVisible) override
    {
        ScopedXLock xlock;
        if (shouldBeVisible)
            XMapWindow (display, windowH);
        else
            XUnmapWindow (display, windowH);
    }

    void setTitle (const String& title) override
    {
        XTextProperty nameProperty;
        char* strings[] = { const_cast <char*> (title.toRawUTF8()) };
        ScopedXLock xlock;

        if (XStringListToTextProperty (strings, 1, &nameProperty))
        {
            XSetWMName (display, windowH, &nameProperty);
            XSetWMIconName (display, windowH, &nameProperty);

            XFree (nameProperty.value);
        }
    }

    void setBounds (const Rectangle<int>& newBounds, bool isNowFullScreen) override
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
                clientMsg.message_type = Atoms::get().windowState;
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
            bounds = newBounds.withSize (jmax (1, newBounds.getWidth()),
                                         jmax (1, newBounds.getHeight()));

            WeakReference<Component> deletionChecker (&component);
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

    Rectangle<int> getBounds() const override          { return bounds; }

    Point<float> localToGlobal (Point<float> relativePosition) override
    {
        return relativePosition + bounds.getPosition().toFloat();
    }

    Point<float> globalToLocal (Point<float> screenPosition) override
    {
        return screenPosition - bounds.getPosition().toFloat();
    }

    void setAlpha (float /* newAlpha */) override
    {
        //xxx todo!
    }

    StringArray getAvailableRenderingEngines() override
    {
        return StringArray ("Software Renderer");
    }

    void setMinimised (bool shouldBeMinimised) override
    {
        if (shouldBeMinimised)
        {
            Window root = RootWindow (display, DefaultScreen (display));

            XClientMessageEvent clientMsg;
            clientMsg.display = display;
            clientMsg.window = windowH;
            clientMsg.type = ClientMessage;
            clientMsg.format = 32;
            clientMsg.message_type = Atoms::get().changeState;
            clientMsg.data.l[0] = IconicState;

            ScopedXLock xlock;
            XSendEvent (display, root, false, SubstructureRedirectMask | SubstructureNotifyMask, (XEvent*) &clientMsg);
        }
        else
        {
            setVisible (true);
        }
    }

    bool isMinimised() const override
    {
        ScopedXLock xlock;
        const Atoms& atoms = Atoms::get();
        GetXProperty prop (windowH, atoms.state, 0, 64, false, atoms.state);

        return prop.success
                && prop.actualType == atoms.state
                && prop.actualFormat == 32
                && prop.numItems > 0
                && ((unsigned long*) prop.data)[0] == IconicState;
    }

    void setFullScreen (const bool shouldBeFullScreen) override
    {
        Rectangle<int> r (lastNonFullscreenBounds); // (get a copy of this before de-minimising)

        setMinimised (false);

        if (fullScreen != shouldBeFullScreen)
        {
            if (shouldBeFullScreen)
                r = Desktop::getInstance().getDisplays().getMainDisplay().userArea;

            if (! r.isEmpty())
                setBounds (ScalingHelpers::scaledScreenPosToUnscaled (component, r), shouldBeFullScreen);

            component.repaint();
        }
    }

    bool isFullScreen() const override
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
            if (windowList != nullptr)
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

        if (windowList != nullptr)
            XFree (windowList);

        return result;
    }

    bool contains (Point<int> localPos, bool trueIfInAChildWindow) const override
    {
        if (! bounds.withZeroOrigin().contains (localPos))
            return false;

        for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
        {
            Component* const c = Desktop::getInstance().getComponent (i);

            if (c == &component)
                break;

            if (ComponentPeer* peer = c->getPeer())
                if (peer->contains (localPos + bounds.getPosition() - peer->getBounds().getPosition(), true))
                    return false;
        }

        if (trueIfInAChildWindow)
            return true;

        ::Window root, child;
        int wx, wy;
        unsigned int ww, wh, bw, depth;

        ScopedXLock xlock;

        return XGetGeometry (display, (::Drawable) windowH, &root, &wx, &wy, &ww, &wh, &bw, &depth)
                && XTranslateCoordinates (display, windowH, windowH, localPos.getX(), localPos.getY(), &wx, &wy, &child)
                && child == None;
    }

    BorderSize<int> getFrameSize() const override
    {
        return BorderSize<int>();
    }

    bool setAlwaysOnTop (bool /* alwaysOnTop */) override
    {
        return false;
    }

    void toFront (bool makeActive) override
    {
        if (makeActive)
        {
            setVisible (true);
            grabFocus();
        }

        {
            ScopedXLock xlock;
            XEvent ev;
            ev.xclient.type = ClientMessage;
            ev.xclient.serial = 0;
            ev.xclient.send_event = True;
            ev.xclient.message_type = Atoms::get().activeWin;
            ev.xclient.window = windowH;
            ev.xclient.format = 32;
            ev.xclient.data.l[0] = 2;
            ev.xclient.data.l[1] = getUserTime();
            ev.xclient.data.l[2] = 0;
            ev.xclient.data.l[3] = 0;
            ev.xclient.data.l[4] = 0;

            XSendEvent (display, RootWindow (display, DefaultScreen (display)),
                        False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);

            XWindowAttributes attr;
            XGetWindowAttributes (display, windowH, &attr);

            if (component.isAlwaysOnTop())
                XRaiseWindow (display, windowH);

            XSync (display, False);
        }

        handleBroughtToFront();
    }

    void toBehind (ComponentPeer* other) override
    {
        if (LinuxComponentPeer* const otherPeer = dynamic_cast<LinuxComponentPeer*> (other))
        {
            setMinimised (false);

            Window newStack[] = { otherPeer->windowH, windowH };

            ScopedXLock xlock;
            XRestackWindows (display, newStack, 2);
        }
        else
            jassertfalse; // wrong type of window?
    }

    bool isFocused() const override
    {
        int revert = 0;
        Window focusedWindow = 0;
        ScopedXLock xlock;
        XGetInputFocus (display, &focusedWindow, &revert);

        return focusedWindow == windowH;
    }

    void grabFocus() override
    {
        XWindowAttributes atts;
        ScopedXLock xlock;

        if (windowH != 0
            && XGetWindowAttributes (display, windowH, &atts)
            && atts.map_state == IsViewable
            && ! isFocused())
        {
            XSetInputFocus (display, windowH, RevertToParent, getUserTime());
            isActiveApplication = true;
        }
    }

    void textInputRequired (Point<int>, TextInputTarget&) override {}

    void repaint (const Rectangle<int>& area) override
    {
        repainter->repaint (area.getIntersection (bounds.withZeroOrigin()));
    }

    void performAnyPendingRepaintsNow() override
    {
        repainter->performAnyPendingRepaintsNow();
    }

    void setIcon (const Image& newIcon) override
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
    void handleWindowMessage (XEvent& event)
    {
        switch (event.xany.type)
        {
            case KeyPressEventType:     handleKeyPressEvent (event.xkey); break;
            case KeyRelease:            handleKeyReleaseEvent (event.xkey); break;
            case ButtonPress:           handleButtonPressEvent (event.xbutton); break;
            case ButtonRelease:         handleButtonReleaseEvent (event.xbutton); break;
            case MotionNotify:          handleMotionNotifyEvent (event.xmotion); break;
            case EnterNotify:           handleEnterNotifyEvent (event.xcrossing); break;
            case LeaveNotify:           handleLeaveNotifyEvent (event.xcrossing); break;
            case FocusIn:               handleFocusInEvent(); break;
            case FocusOut:              handleFocusOutEvent(); break;
            case Expose:                handleExposeEvent (event.xexpose); break;
            case MappingNotify:         handleMappingNotify (event.xmapping); break;
            case ClientMessage:         handleClientMessageEvent (event.xclient, event); break;
            case SelectionNotify:       handleDragAndDropSelection (event); break;
            case ConfigureNotify:       handleConfigureNotifyEvent (event.xconfigure); break;
            case ReparentNotify:        handleReparentNotifyEvent(); break;
            case GravityNotify:         handleGravityNotify(); break;
            case SelectionClear:        handleExternalSelectionClear(); break;
            case SelectionRequest:      handleExternalSelectionRequest (event); break;

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

            default:
               #if JUCE_USE_XSHM
                if (XSHMHelpers::isShmAvailable())
                {
                    ScopedXLock xlock;
                    if (event.xany.type == XShmGetEventBase (display))
                        repainter->notifyPaintCompleted();
                }
               #endif
                break;
        }
    }

    void handleKeyPressEvent (XKeyEvent& keyEvent)
    {
        char utf8 [64] = { 0 };
        juce_wchar unicodeChar = 0;
        int keyCode = 0;
        bool keyDownChange = false;
        KeySym sym;

        {
            ScopedXLock xlock;
            updateKeyStates (keyEvent.keycode, true);

            String oldLocale (::setlocale (LC_ALL, 0));
            ::setlocale (LC_ALL, "");
            XLookupString (&keyEvent, utf8, sizeof (utf8), &sym, 0);

            if (oldLocale.isNotEmpty())
                ::setlocale (LC_ALL, oldLocale.toRawUTF8());

            unicodeChar = *CharPointer_UTF8 (utf8);
            keyCode = (int) unicodeChar;

            if (keyCode < 0x20)
                keyCode = XkbKeycodeToKeysym (display, keyEvent.keycode, 0, currentModifiers.isShiftDown() ? 1 : 0);

            keyDownChange = (sym != NoSymbol) && ! updateKeyModifiersFromSym (sym, true);
        }

        const ModifierKeys oldMods (currentModifiers);
        bool keyPressed = false;

        if ((sym & 0xff00) == 0xff00 || sym == XK_ISO_Left_Tab)
        {
            switch (sym)  // Translate keypad
            {
                case XK_KP_Add:         keyCode = XK_plus; break;
                case XK_KP_Subtract:    keyCode = XK_hyphen; break;
                case XK_KP_Divide:      keyCode = XK_slash; break;
                case XK_KP_Multiply:    keyCode = XK_asterisk; break;
                case XK_KP_Enter:       keyCode = XK_Return; break;
                case XK_KP_Insert:      keyCode = XK_Insert; break;
                case XK_Delete:
                case XK_KP_Delete:      keyCode = XK_Delete; break;
                case XK_KP_Left:        keyCode = XK_Left; break;
                case XK_KP_Right:       keyCode = XK_Right; break;
                case XK_KP_Up:          keyCode = XK_Up; break;
                case XK_KP_Down:        keyCode = XK_Down; break;
                case XK_KP_Home:        keyCode = XK_Home; break;
                case XK_KP_End:         keyCode = XK_End; break;
                case XK_KP_Page_Down:   keyCode = XK_Page_Down; break;
                case XK_KP_Page_Up:     keyCode = XK_Page_Up; break;

                case XK_KP_0:           keyCode = XK_0; break;
                case XK_KP_1:           keyCode = XK_1; break;
                case XK_KP_2:           keyCode = XK_2; break;
                case XK_KP_3:           keyCode = XK_3; break;
                case XK_KP_4:           keyCode = XK_4; break;
                case XK_KP_5:           keyCode = XK_5; break;
                case XK_KP_6:           keyCode = XK_6; break;
                case XK_KP_7:           keyCode = XK_7; break;
                case XK_KP_8:           keyCode = XK_8; break;
                case XK_KP_9:           keyCode = XK_9; break;

                default:                break;
            }

            switch (keyCode)
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
                    keyCode = (keyCode & 0xff) | Keys::extendedKeyModifier;
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

    static bool isKeyReleasePartOfAutoRepeat (const XKeyEvent& keyReleaseEvent)
    {
        if (XPending (display))
        {
            XEvent e;
            XPeekEvent (display, &e);

            // Look for a subsequent key-down event with the same timestamp and keycode
            return e.type == KeyPressEventType
                    && e.xkey.keycode == keyReleaseEvent.keycode
                    && e.xkey.time == keyReleaseEvent.time;
        }

        return false;
    }

    void handleKeyReleaseEvent (const XKeyEvent& keyEvent)
    {
        if (! isKeyReleasePartOfAutoRepeat (keyEvent))
        {
            updateKeyStates (keyEvent.keycode, false);
            KeySym sym;

            {
                ScopedXLock xlock;
                sym = XkbKeycodeToKeysym (display, keyEvent.keycode, 0, 0);
            }

            const ModifierKeys oldMods (currentModifiers);
            const bool keyDownChange = (sym != NoSymbol) && ! updateKeyModifiersFromSym (sym, false);

            if (oldMods != currentModifiers)
                handleModifierKeysChange();

            if (keyDownChange)
                handleKeyUpOrDown (false);
        }
    }

    template <typename EventType>
    static Point<float> getMousePos (const EventType& e) noexcept
    {
        return Point<float> ((float) e.x, (float) e.y);
    }

    void handleWheelEvent (const XButtonPressedEvent& buttonPressEvent, const float amount)
    {
        MouseWheelDetails wheel;
        wheel.deltaX = 0.0f;
        wheel.deltaY = amount;
        wheel.isReversed = false;
        wheel.isSmooth = false;

        handleMouseWheel (0, getMousePos (buttonPressEvent), getEventTime (buttonPressEvent), wheel);
    }

    void handleButtonPressEvent (const XButtonPressedEvent& buttonPressEvent, int buttonModifierFlag)
    {
        currentModifiers = currentModifiers.withFlags (buttonModifierFlag);
        toFront (true);
        handleMouseEvent (0, getMousePos (buttonPressEvent), currentModifiers, getEventTime (buttonPressEvent));
    }

    void handleButtonPressEvent (const XButtonPressedEvent& buttonPressEvent)
    {
        updateKeyModifiers (buttonPressEvent.state);

        switch (pointerMap [buttonPressEvent.button - Button1])
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

    void handleButtonReleaseEvent (const XButtonReleasedEvent& buttonRelEvent)
    {
        updateKeyModifiers (buttonRelEvent.state);

        if (parentWindow != 0)
            updateWindowBounds();

        switch (pointerMap [buttonRelEvent.button - Button1])
        {
            case Keys::LeftButton:      currentModifiers = currentModifiers.withoutFlags (ModifierKeys::leftButtonModifier); break;
            case Keys::RightButton:     currentModifiers = currentModifiers.withoutFlags (ModifierKeys::rightButtonModifier); break;
            case Keys::MiddleButton:    currentModifiers = currentModifiers.withoutFlags (ModifierKeys::middleButtonModifier); break;
            default: break;
        }

        if (dragState.dragging)
            handleExternalDragButtonReleaseEvent();

        handleMouseEvent (0, getMousePos (buttonRelEvent), currentModifiers, getEventTime (buttonRelEvent));

        clearLastMousePos();
    }

    void handleMotionNotifyEvent (const XPointerMovedEvent& movedEvent)
    {
        updateKeyModifiers (movedEvent.state);

        lastMousePos = Point<int> (movedEvent.x_root, movedEvent.y_root);

        if (dragState.dragging)
            handleExternalDragMotionNotify();

        handleMouseEvent (0, getMousePos (movedEvent), currentModifiers, getEventTime (movedEvent));
    }

    void handleEnterNotifyEvent (const XEnterWindowEvent& enterEvent)
    {
        if (parentWindow != 0)
            updateWindowBounds();

        clearLastMousePos();

        if (! currentModifiers.isAnyMouseButtonDown())
        {
            updateKeyModifiers (enterEvent.state);
            handleMouseEvent (0, getMousePos (enterEvent), currentModifiers, getEventTime (enterEvent));
        }
    }

    void handleLeaveNotifyEvent (const XLeaveWindowEvent& leaveEvent)
    {
        // Suppress the normal leave if we've got a pointer grab, or if
        // it's a bogus one caused by clicking a mouse button when running
        // in a Window manager
        if (((! currentModifiers.isAnyMouseButtonDown()) && leaveEvent.mode == NotifyNormal)
             || leaveEvent.mode == NotifyUngrab)
        {
            updateKeyModifiers (leaveEvent.state);
            handleMouseEvent (0, getMousePos (leaveEvent), currentModifiers, getEventTime (leaveEvent));
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

    void handleExposeEvent (XExposeEvent& exposeEvent)
    {
        // Batch together all pending expose events
        XEvent nextEvent;
        ScopedXLock xlock;

        if (exposeEvent.window != windowH)
        {
            Window child;
            XTranslateCoordinates (display, exposeEvent.window, windowH,
                                   exposeEvent.x, exposeEvent.y, &exposeEvent.x, &exposeEvent.y,
                                   &child);
        }

        repaint (Rectangle<int> (exposeEvent.x, exposeEvent.y,
                                 exposeEvent.width, exposeEvent.height));

        while (XEventsQueued (display, QueuedAfterFlush) > 0)
        {
            XPeekEvent (display, &nextEvent);
            if (nextEvent.type != Expose || nextEvent.xany.window != exposeEvent.window)
                break;

            XNextEvent (display, &nextEvent);
            const XExposeEvent& nextExposeEvent = (const XExposeEvent&) nextEvent.xexpose;
            repaint (Rectangle<int> (nextExposeEvent.x, nextExposeEvent.y,
                                     nextExposeEvent.width, nextExposeEvent.height));
        }
    }

    void handleConfigureNotifyEvent (XConfigureEvent& confEvent)
    {
        updateWindowBounds();
        updateBorderSize();
        handleMovedOrResized();

        // if the native title bar is dragged, need to tell any active menus, etc.
        if ((styleFlags & windowHasTitleBar) != 0
              && component.isCurrentlyBlockedByAnotherModalComponent())
        {
            if (Component* const currentModalComp = Component::getCurrentlyModalComponent())
                currentModalComp->inputAttemptWhenModal();
        }

        if (confEvent.window == windowH
             && confEvent.above != 0
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
        updateWindowBounds();
        updateBorderSize();
        handleMovedOrResized();
    }

    void handleMappingNotify (XMappingEvent& mappingEvent)
    {
        if (mappingEvent.request != MappingPointer)
        {
            // Deal with modifier/keyboard mapping
            ScopedXLock xlock;
            XRefreshKeyboardMapping (&mappingEvent);
            updateModifierMappings();
        }
    }

    void handleClientMessageEvent (XClientMessageEvent& clientMsg, XEvent& event)
    {
        const Atoms& atoms = Atoms::get();

        if (clientMsg.message_type == atoms.protocols && clientMsg.format == 32)
        {
            const Atom atom = (Atom) clientMsg.data.l[0];

            if (atom == atoms.protocolList [Atoms::PING])
            {
                Window root = RootWindow (display, DefaultScreen (display));

                clientMsg.window = root;

                XSendEvent (display, root, False, NoEventMask, &event);
                XFlush (display);
            }
            else if (atom == atoms.protocolList [Atoms::TAKE_FOCUS])
            {
                if ((getStyleFlags() & juce::ComponentPeer::windowIgnoresKeyPresses) == 0)
                {
                    XWindowAttributes atts;

                    ScopedXLock xlock;
                    if (clientMsg.window != 0
                         && XGetWindowAttributes (display, clientMsg.window, &atts))
                    {
                        if (atts.map_state == IsViewable)
                            XSetInputFocus (display, clientMsg.window, RevertToParent, clientMsg.data.l[1]);
                    }
                }
            }
            else if (atom == atoms.protocolList [Atoms::DELETE_WINDOW])
            {
                handleUserClosingWindow();
            }
        }
        else if (clientMsg.message_type == atoms.XdndEnter)
        {
            handleDragAndDropEnter (clientMsg);
        }
        else if (clientMsg.message_type == atoms.XdndLeave)
        {
            handleDragExit (dragInfo);
            resetDragAndDrop();
        }
        else if (clientMsg.message_type == atoms.XdndPosition)
        {
            handleDragAndDropPosition (clientMsg);
        }
        else if (clientMsg.message_type == atoms.XdndDrop)
        {
            handleDragAndDropDrop (clientMsg);
        }
        else if (clientMsg.message_type == atoms.XdndStatus)
        {
            handleExternalDragAndDropStatus (clientMsg);
        }
        else if (clientMsg.message_type == atoms.XdndFinished)
        {
            externalResetDragAndDrop();
        }
    }

    bool externalDragTextInit (const String& text)
    {
        if (dragState.dragging)
            return false;

        return externalDragInit (true, text);
    }

    bool externalDragFileInit (const StringArray& files, bool /*canMoveFiles*/)
    {
        if (dragState.dragging)
            return false;

        StringArray uriList;

        for (int i = 0; i < files.size(); ++i)
        {
            const String& f = files[i];

            if (f.matchesWildcard ("?*://*", false))
                uriList.add (f);
            else
                uriList.add ("file://" + f);
        }

        return externalDragInit (false, uriList.joinIntoString ("\r\n"));
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
    class LinuxRepaintManager   : public Timer
    {
    public:
        LinuxRepaintManager (LinuxComponentPeer& p)
            : peer (p), lastTimeImageUsed (0)
        {
           #if JUCE_USE_XSHM
            shmPaintsPending = 0;

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

        void timerCallback() override
        {
           #if JUCE_USE_XSHM
            if (shmPaintsPending != 0)
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
            if (shmPaintsPending != 0)
            {
                startTimer (repaintTimerPeriod);
                return;
            }
           #endif

            RectangleList<int>  originalRepaintRegion (regionsNeedingRepaint);
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
                                                     (totalArea.getWidth()  + 31) & ~31,
                                                     (totalArea.getHeight() + 31) & ~31,
                                                     false, peer.depth, peer.visual));
                }

                startTimer (repaintTimerPeriod);

                RectangleList<int> adjustedList (originalRepaintRegion);
                adjustedList.offsetAll (-totalArea.getX(), -totalArea.getY());

                if (peer.depth == 32)
                    for (const Rectangle<int>* i = originalRepaintRegion.begin(), * const e = originalRepaintRegion.end(); i != e; ++i)
                        image.clear (*i - totalArea.getPosition());

                {
                    ScopedPointer<LowLevelGraphicsContext> context (peer.getComponent().getLookAndFeel()
                                                                      .createGraphicsContext (image, -totalArea.getPosition(), adjustedList));
                    peer.handlePaint (*context);
                }

                for (const Rectangle<int>* i = originalRepaintRegion.begin(), * const e = originalRepaintRegion.end(); i != e; ++i)
                {
                   #if JUCE_USE_XSHM
                    if (XSHMHelpers::isShmAvailable())
                        ++shmPaintsPending;
                   #endif

                    static_cast<XBitmapImage*> (image.getPixelData())
                        ->blitToWindow (peer.windowH,
                                        i->getX(), i->getY(), i->getWidth(), i->getHeight(),
                                        i->getX() - totalArea.getX(), i->getY() - totalArea.getY());
                }
            }

            lastTimeImageUsed = Time::getApproximateMillisecondCounter();
            startTimer (repaintTimerPeriod);
        }

       #if JUCE_USE_XSHM
        void notifyPaintCompleted() noexcept        { --shmPaintsPending; }
       #endif

    private:
        enum { repaintTimerPeriod = 1000 / 100 };

        LinuxComponentPeer& peer;
        Image image;
        uint32 lastTimeImageUsed;
        RectangleList<int> regionsNeedingRepaint;

       #if JUCE_USE_XSHM
        bool useARGBImagesForRendering;
        int shmPaintsPending;
       #endif
        JUCE_DECLARE_NON_COPYABLE (LinuxRepaintManager)
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
    bool isAlwaysOnTop;
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
            case XK_Shift_R:        modifier = ModifierKeys::shiftModifier; break;

            case XK_Control_L:
            case XK_Control_R:      modifier = ModifierKeys::ctrlModifier; break;

            case XK_Alt_L:
            case XK_Alt_R:          modifier = ModifierKeys::altModifier; break;

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

        if (XModifierKeymap* const mapping = XGetModifierMapping (display))
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
            MotifWmHints motifHints;
            zerostruct (motifHints);

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
            MotifWmHints motifHints;
            zerostruct (motifHints);

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

        xchangeProperty (windowH, Atoms::get().windowType, XA_ATOM, 32, &netHints, 2);

        int numHints = 0;

        if ((styleFlags & windowAppearsOnTaskbar) == 0)
            netHints [numHints++] = Atoms::getIfExists ("_NET_WM_STATE_SKIP_TASKBAR");

        if (component.isAlwaysOnTop())
            netHints [numHints++] = Atoms::getIfExists ("_NET_WM_STATE_ABOVE");

        if (numHints > 0)
            xchangeProperty (windowH, Atoms::get().windowState, XA_ATOM, 32, &netHints, numHints);
    }

    void createWindow (Window parentToAddTo)
    {
        ScopedXLock xlock;
        resetDragAndDrop();

        // Get defaults for various properties
        const int screen = DefaultScreen (display);
        Window root = RootWindow (display, screen);

        // Try to obtain a 32-bit visual or fallback to 24 or 16
        visual = Visuals::findVisualFormat ((styleFlags & windowIsSemiTransparent) ? 32 : 24, depth);

        if (visual == nullptr)
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
        swa.override_redirect = (component.isAlwaysOnTop() && (styleFlags & windowIsTemporary) != 0) ? True : False;
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

        setTitle (component.getName());

        const Atoms& atoms = Atoms::get();

        // Associate the PID, allowing to be shut down when something goes wrong
        unsigned long pid = getpid();
        xchangeProperty (windowH, atoms.pid, XA_CARDINAL, 32, &pid, 1);

        // Set window manager protocols
        xchangeProperty (windowH, atoms.protocols, XA_ATOM, 32, atoms.protocolList, 2);

        // Set drag and drop flags
        xchangeProperty (windowH, atoms.XdndTypeList, XA_ATOM, 32, atoms.allowedMimeTypes, numElementsInArray (atoms.allowedMimeTypes));
        xchangeProperty (windowH, atoms.XdndActionList, XA_ATOM, 32, atoms.allowedActions, numElementsInArray (atoms.allowedActions));
        xchangeProperty (windowH, atoms.XdndActionDescription, XA_STRING, 8, "", 0);
        xchangeProperty (windowH, atoms.XdndAware, XA_ATOM, 32, &Atoms::DndVersion, 1);

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

    template <typename EventType>
    static int64 getEventTime (const EventType& t)
    {
        return getEventTime (t.time);
    }

    static int64 getEventTime (::Time t)
    {
        static int64 eventTimeOffset = 0x12345678;
        const int64 thisMessageTime = t;

        if (eventTimeOffset == 0x12345678)
            eventTimeOffset = Time::currentTimeMillis() - thisMessageTime;

        return eventTimeOffset + thisMessageTime;
    }

    long getUserTime() const
    {
        GetXProperty prop (windowH, Atoms::get().userTime, 0, 65536, false, XA_CARDINAL);
        return prop.success ? *(long*) prop.data : 0;
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
                GetXProperty prop (windowH, hints, 0, 4, false, XA_CARDINAL);

                if (prop.success && prop.actualFormat == 32)
                {
                    const unsigned long* const sizes = (const unsigned long*) prop.data;

                    windowBorder = BorderSize<int> ((int) sizes[2], (int) sizes[0],
                                                    (int) sizes[3], (int) sizes[1]);
                }
            }
        }
    }

    void updateWindowBounds()
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
    struct DragState
    {
        DragState() noexcept
           : isText (false), dragging (false), expectingStatus (false),
             canDrop (false), targetWindow (None), xdndVersion (-1)
        {
        }

        bool isText;
        bool dragging;         // currently performing outgoing external dnd as Xdnd source, have grabbed mouse
        bool expectingStatus;  // XdndPosition sent, waiting for XdndStatus
        bool canDrop;          // target window signals it will accept the drop
        Window targetWindow;   // potential drop target
        int xdndVersion;       // negotiated version with target
        Rectangle<int> silentRect;
        String textOrFiles;

        const Atom* getMimeTypes() const noexcept   { return isText ? Atoms::get().externalAllowedTextMimeTypes
                                                                    : Atoms::get().externalAllowedFileMimeTypes; }

        int getNumMimeTypes() const noexcept { return isText ? numElementsInArray (Atoms::get().externalAllowedTextMimeTypes)
                                                             : numElementsInArray (Atoms::get().externalAllowedFileMimeTypes); }

        bool matchesTarget (Atom targetType) const
        {
            for (int i = getNumMimeTypes(); --i >= 0;)
                if (getMimeTypes()[i] == targetType)
                    return true;

            return false;
        }
    };

    //==============================================================================
    void resetDragAndDrop()
    {
        dragInfo.clear();
        dragInfo.position = Point<int> (-1, -1);
        dragAndDropCurrentMimeType = 0;
        dragAndDropSourceWindow = 0;
        srcMimeTypeAtomList.clear();
        finishAfterDropDataReceived = false;
    }

    void resetExternalDragState()
    {
        dragState = DragState();
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

    bool sendExternalDragAndDropMessage (XClientMessageEvent& msg, const Window targetWindow)
    {
        msg.type      = ClientMessage;
        msg.display   = display;
        msg.window    = targetWindow;
        msg.format    = 32;
        msg.data.l[0] = windowH;

        ScopedXLock xlock;
        return XSendEvent (display, targetWindow, False, 0, (XEvent*) &msg) != 0;
    }

    void sendExternalDragAndDropDrop (const Window targetWindow)
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = Atoms::get().XdndDrop;
        msg.data.l[2] = CurrentTime;

        sendExternalDragAndDropMessage (msg, targetWindow);
    }

    void sendExternalDragAndDropEnter (const Window targetWindow)
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = Atoms::get().XdndEnter;

        const Atom* mimeTypes  = dragState.getMimeTypes();
        const int numMimeTypes = dragState.getNumMimeTypes();

        msg.data.l[1] = (dragState.xdndVersion << 24) | (numMimeTypes > 3);
        msg.data.l[2] = numMimeTypes > 0 ? mimeTypes[0] : 0;
        msg.data.l[3] = numMimeTypes > 1 ? mimeTypes[1] : 0;
        msg.data.l[4] = numMimeTypes > 2 ? mimeTypes[2] : 0;

        sendExternalDragAndDropMessage (msg, targetWindow);
    }

    void sendExternalDragAndDropPosition (const Window targetWindow)
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = Atoms::get().XdndPosition;

        const Point<int> mousePos (Desktop::getInstance().getMousePosition());

        if (dragState.silentRect.contains (mousePos)) // we've been asked to keep silent
            return;

        msg.data.l[1] = 0;
        msg.data.l[2] = (mousePos.x << 16) | mousePos.y;
        msg.data.l[3] = CurrentTime;
        msg.data.l[4] = Atoms::get().XdndActionCopy; // this is all JUCE currently supports

        dragState.expectingStatus = sendExternalDragAndDropMessage (msg, targetWindow);
    }

    void sendDragAndDropStatus (const bool acceptDrop, Atom dropAction)
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = Atoms::get().XdndStatus;
        msg.data.l[1] = (acceptDrop ? 1 : 0) | 2; // 2 indicates that we want to receive position messages
        msg.data.l[4] = dropAction;

        sendDragAndDropMessage (msg);
    }

    void sendExternalDragAndDropLeave (const Window targetWindow)
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = Atoms::get().XdndLeave;
        sendExternalDragAndDropMessage (msg, targetWindow);
    }

    void sendDragAndDropFinish()
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = Atoms::get().XdndFinished;
        sendDragAndDropMessage (msg);
    }

    void handleExternalSelectionClear()
    {
        if (dragState.dragging)
            externalResetDragAndDrop();
    }

    void handleExternalSelectionRequest (const XEvent& evt)
    {
        Atom targetType = evt.xselectionrequest.target;

        XEvent s;
        s.xselection.type = SelectionNotify;
        s.xselection.requestor = evt.xselectionrequest.requestor;
        s.xselection.selection = evt.xselectionrequest.selection;
        s.xselection.target = targetType;
        s.xselection.property = None;
        s.xselection.time = evt.xselectionrequest.time;

        if (dragState.matchesTarget (targetType))
        {
            s.xselection.property = evt.xselectionrequest.property;

            xchangeProperty (evt.xselectionrequest.requestor,
                             evt.xselectionrequest.property,
                             targetType, 8,
                             dragState.textOrFiles.toRawUTF8(),
                             dragState.textOrFiles.getNumBytesAsUTF8());
        }

        XSendEvent (display, evt.xselectionrequest.requestor, True, 0, &s);
    }

    void handleExternalDragAndDropStatus (const XClientMessageEvent& clientMsg)
    {
        if (dragState.expectingStatus)
        {
            dragState.expectingStatus = false;
            dragState.canDrop = false;
            dragState.silentRect = Rectangle<int>();

            if ((clientMsg.data.l[1] & 1) != 0
                 && ((Atom) clientMsg.data.l[4] == Atoms::get().XdndActionCopy
                      || (Atom) clientMsg.data.l[4] == Atoms::get().XdndActionPrivate))
            {
                if ((clientMsg.data.l[1] & 2) == 0) // target requests silent rectangle
                    dragState.silentRect.setBounds (clientMsg.data.l[2] >> 16,
                                                    clientMsg.data.l[2] & 0xffff,
                                                    clientMsg.data.l[3] >> 16,
                                                    clientMsg.data.l[3] & 0xffff);

                dragState.canDrop = true;
            }
        }
    }

    void handleExternalDragButtonReleaseEvent()
    {
        if (dragState.dragging)
            XUngrabPointer (display, CurrentTime);

        if (dragState.canDrop)
        {
            sendExternalDragAndDropDrop (dragState.targetWindow);
        }
        else
        {
            sendExternalDragAndDropLeave (dragState.targetWindow);
            externalResetDragAndDrop();
        }
    }

    void handleExternalDragMotionNotify()
    {
        Window targetWindow = externalFindDragTargetWindow (RootWindow (display, DefaultScreen (display)));

        if (dragState.targetWindow != targetWindow)
        {
            if (dragState.targetWindow != None)
                sendExternalDragAndDropLeave (dragState.targetWindow);

            dragState.canDrop = false;
            dragState.silentRect = Rectangle<int>();

            if (targetWindow == None)
                return;

            GetXProperty prop (targetWindow, Atoms::get().XdndAware,
                               0, 2, false, AnyPropertyType);

            if (prop.success
                 && prop.data != None
                 && prop.actualFormat == 32
                 && prop.numItems == 1)
            {
                dragState.xdndVersion = jmin ((int) prop.data[0], (int) Atoms::DndVersion);
            }
            else
            {
                dragState.xdndVersion = -1;
                return;
            }

            sendExternalDragAndDropEnter (targetWindow);
            dragState.targetWindow = targetWindow;
        }

        if (! dragState.expectingStatus)
            sendExternalDragAndDropPosition (targetWindow);
    }

    void handleDragAndDropPosition (const XClientMessageEvent& clientMsg)
    {
        if (dragAndDropSourceWindow == 0)
            return;

        dragAndDropSourceWindow = clientMsg.data.l[0];

        Point<int> dropPos ((int) clientMsg.data.l[2] >> 16,
                            (int) clientMsg.data.l[2] & 0xffff);
        dropPos -= bounds.getPosition();

        const Atoms& atoms = Atoms::get();
        Atom targetAction = atoms.XdndActionCopy;

        for (int i = numElementsInArray (atoms.allowedActions); --i >= 0;)
        {
            if ((Atom) clientMsg.data.l[4] == atoms.allowedActions[i])
            {
                targetAction = atoms.allowedActions[i];
                break;
            }
        }

        sendDragAndDropStatus (true, targetAction);

        if (dragInfo.position != dropPos)
        {
            dragInfo.position = dropPos;

            if (dragInfo.isEmpty())
                updateDraggedFileList (clientMsg);

            if (! dragInfo.isEmpty())
                handleDragMove (dragInfo);
        }
    }

    void handleDragAndDropDrop (const XClientMessageEvent& clientMsg)
    {
        if (dragInfo.isEmpty())
        {
            // no data, transaction finished in handleDragAndDropSelection()
            finishAfterDropDataReceived = true;
            updateDraggedFileList (clientMsg);
        }
        else
        {
            handleDragAndDropDataReceived();  // data was already received
        }
    }

    void handleDragAndDropDataReceived()
    {
        DragInfo dragInfoCopy (dragInfo);

        sendDragAndDropFinish();
        resetDragAndDrop();

        if (! dragInfoCopy.isEmpty())
            handleDragDrop (dragInfoCopy);
    }

    void handleDragAndDropEnter (const XClientMessageEvent& clientMsg)
    {
        dragInfo.clear();
        srcMimeTypeAtomList.clear();

        dragAndDropCurrentMimeType = 0;
        const unsigned long dndCurrentVersion = static_cast <unsigned long> (clientMsg.data.l[1] & 0xff000000) >> 24;

        if (dndCurrentVersion < 3 || dndCurrentVersion > Atoms::DndVersion)
        {
            dragAndDropSourceWindow = 0;
            return;
        }

        dragAndDropSourceWindow = clientMsg.data.l[0];

        if ((clientMsg.data.l[1] & 1) != 0)
        {
            ScopedXLock xlock;
            GetXProperty prop (dragAndDropSourceWindow, Atoms::get().XdndTypeList, 0, 0x8000000L, false, XA_ATOM);

            if (prop.success
                 && prop.actualType == XA_ATOM
                 && prop.actualFormat == 32
                 && prop.numItems != 0)
            {
                const unsigned long* const types = (const unsigned long*) prop.data;

                for (unsigned long i = 0; i < prop.numItems; ++i)
                    if (types[i] != None)
                        srcMimeTypeAtomList.add (types[i]);
            }
        }

        if (srcMimeTypeAtomList.size() == 0)
        {
            for (int i = 2; i < 5; ++i)
                if (clientMsg.data.l[i] != None)
                    srcMimeTypeAtomList.add (clientMsg.data.l[i]);

            if (srcMimeTypeAtomList.size() == 0)
            {
                dragAndDropSourceWindow = 0;
                return;
            }
        }

        const Atoms& atoms = Atoms::get();
        for (int i = 0; i < srcMimeTypeAtomList.size() && dragAndDropCurrentMimeType == 0; ++i)
            for (int j = 0; j < numElementsInArray (atoms.allowedMimeTypes); ++j)
                if (srcMimeTypeAtomList[i] == atoms.allowedMimeTypes[j])
                    dragAndDropCurrentMimeType = atoms.allowedMimeTypes[j];

        handleDragAndDropPosition (clientMsg);
    }

    void handleDragAndDropSelection (const XEvent& evt)
    {
        dragInfo.clear();

        if (evt.xselection.property != None)
        {
            StringArray lines;

            {
                MemoryBlock dropData;

                for (;;)
                {
                    GetXProperty prop (evt.xany.window, evt.xselection.property,
                                       dropData.getSize() / 4, 65536, false, AnyPropertyType);

                    if (! prop.success)
                        break;

                    dropData.append (prop.data, prop.numItems * prop.actualFormat / 8);

                    if (prop.bytesLeft <= 0)
                        break;
                }

                lines.addLines (dropData.toString());
            }

            if (Atoms::isMimeTypeFile (dragAndDropCurrentMimeType))
            {
                for (int i = 0; i < lines.size(); ++i)
                    dragInfo.files.add (URL::removeEscapeChars (lines[i].replace ("file://", String::empty, true)));

                dragInfo.files.trim();
                dragInfo.files.removeEmptyStrings();
            }
            else
            {
                dragInfo.text = lines.joinIntoString ("\n");
            }

            if (finishAfterDropDataReceived)
                handleDragAndDropDataReceived();
        }
    }

    void updateDraggedFileList (const XClientMessageEvent& clientMsg)
    {
        jassert (dragInfo.isEmpty());

        if (dragAndDropSourceWindow != None
             && dragAndDropCurrentMimeType != None)
        {
            ScopedXLock xlock;
            XConvertSelection (display,
                               Atoms::get().XdndSelection,
                               dragAndDropCurrentMimeType,
                               Atoms::getCreating ("JXSelectionWindowProperty"),
                               windowH,
                               clientMsg.data.l[2]);
        }
    }

    static bool isWindowDnDAware (Window w)
    {
        int numProperties = 0;
        Atom* const atoms = XListProperties (display, w, &numProperties);

        bool dndAwarePropFound = false;
        for (int i = 0; i < numProperties; ++i)
            if (atoms[i] == Atoms::get().XdndAware)
                dndAwarePropFound = true;

        if (atoms != nullptr)
            XFree (atoms);

        return dndAwarePropFound;
    }

    Window externalFindDragTargetWindow (Window targetWindow)
    {
        if (targetWindow == None)
            return None;

        if (isWindowDnDAware (targetWindow))
            return targetWindow;

        Window child, phonyWin;
        int phony;
        unsigned int uphony;

        XQueryPointer (display, targetWindow, &phonyWin, &child,
                       &phony, &phony, &phony, &phony, &uphony);

        return externalFindDragTargetWindow (child);
    }

    bool externalDragInit (bool isText, const String& textOrFiles)
    {
        ScopedXLock xlock;

        resetExternalDragState();
        dragState.isText = isText;
        dragState.textOrFiles = textOrFiles;
        dragState.targetWindow = windowH;

        const int pointerGrabMask = Button1MotionMask | ButtonReleaseMask;

        if (XGrabPointer (display, windowH, True, pointerGrabMask,
                          GrabModeAsync, GrabModeAsync, None, None, CurrentTime) == GrabSuccess)
        {
            // No other method of changing the pointer seems to work, this call is needed from this very context
            XChangeActivePointerGrab (display, pointerGrabMask, (Cursor) createDraggingHandCursor(), CurrentTime);

            const Atoms& atoms = Atoms::get();
            XSetSelectionOwner (display, atoms.XdndSelection, windowH, CurrentTime);

            // save the available types to XdndTypeList
            xchangeProperty (windowH, atoms.XdndTypeList, XA_ATOM, 32,
                             dragState.getMimeTypes(),
                             dragState.getNumMimeTypes());

            dragState.dragging = true;
            handleExternalDragMotionNotify();
            return true;
        }

        return false;
    }

    void externalResetDragAndDrop()
    {
        if (dragState.dragging)
        {
            ScopedXLock xlock;
            XUngrabPointer (display, CurrentTime);
        }

        resetExternalDragState();
    }

    DragState dragState;
    DragInfo dragInfo;
    Atom dragAndDropCurrentMimeType;
    Window dragAndDropSourceWindow;
    bool finishAfterDropDataReceived;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinuxComponentPeer)
};

ModifierKeys LinuxComponentPeer::currentModifiers;
bool LinuxComponentPeer::isActiveApplication = false;
Point<int> LinuxComponentPeer::lastMousePos;

//==============================================================================
JUCE_API bool JUCE_CALLTYPE Process::isForegroundProcess()
{
    return LinuxComponentPeer::isActiveApplication;
}

// N/A on Linux as far as I know.
JUCE_API void JUCE_CALLTYPE Process::makeForegroundProcess() {}
JUCE_API void JUCE_CALLTYPE Process::hide() {}

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
void Desktop::setKioskComponent (Component* kioskModeComponent, bool enableOrDisable, bool /* allowMenusAndBars */)
{
    if (enableOrDisable)
        kioskModeComponent->setBounds (getDisplays().getMainDisplay().totalArea);
}

//==============================================================================
ComponentPeer* Component::createNewPeer (int styleFlags, void* nativeWindowToAttachTo)
{
    return new LinuxComponentPeer (*this, styleFlags, (Window) nativeWindowToAttachTo);
}

//==============================================================================
static double getDisplayDPI (int index)
{
    double dpiX = (DisplayWidth  (display, index) * 25.4) / DisplayWidthMM  (display, index);
    double dpiY = (DisplayHeight (display, index) * 25.4) / DisplayHeightMM (display, index);
    return (dpiX + dpiY) / 2.0;
}

void Desktop::Displays::findDisplays (float masterScale)
{
    if (display == 0)
        return;

    ScopedXLock xlock;

  #if JUCE_USE_XINERAMA
    int major_opcode, first_event, first_error;

    if (XQueryExtension (display, "XINERAMA", &major_opcode, &first_event, &first_error))
    {
        typedef Bool (*tXineramaIsActive) (::Display*);
        typedef XineramaScreenInfo* (*tXineramaQueryScreens) (::Display*, int*);

        static tXineramaIsActive xineramaIsActive = nullptr;
        static tXineramaQueryScreens xineramaQueryScreens = nullptr;

        if (xineramaIsActive == nullptr || xineramaQueryScreens == nullptr)
        {
            void* h = dlopen ("libXinerama.so", RTLD_GLOBAL | RTLD_NOW);

            if (h == nullptr)
                h = dlopen ("libXinerama.so.1", RTLD_GLOBAL | RTLD_NOW);

            if (h != nullptr)
            {
                xineramaIsActive = (tXineramaIsActive) dlsym (h, "XineramaIsActive");
                xineramaQueryScreens = (tXineramaQueryScreens) dlsym (h, "XineramaQueryScreens");
            }
        }

        if (xineramaIsActive != nullptr
            && xineramaQueryScreens != nullptr
            && xineramaIsActive (display))
        {
            int numMonitors = 0;

            if (XineramaScreenInfo* const screens = xineramaQueryScreens (display, &numMonitors))
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
                                                                       screens[j].height) / masterScale;
                            d.isMain = (index == 0);
                            d.scale = masterScale;
                            d.dpi = getDisplayDPI (0); // (all screens share the same DPI)

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
                GetXProperty prop (RootWindow (display, i), hints, 0, 4, false, XA_CARDINAL);

                if (prop.success && prop.actualType == XA_CARDINAL && prop.actualFormat == 32 && prop.numItems == 4)
                {
                    const long* const position = (const long*) prop.data;

                    Display d;
                    d.userArea = d.totalArea = Rectangle<int> (position[0], position[1],
                                                               position[2], position[3]) / masterScale;
                    d.isMain = (displays.size() == 0);
                    d.scale = masterScale;
                    d.dpi = getDisplayDPI (i);

                    displays.add (d);
                }
            }
        }

        if (displays.size() == 0)
        {
            Display d;
            d.userArea = d.totalArea = Rectangle<int> (DisplayWidth  (display, DefaultScreen (display)),
                                                       DisplayHeight (display, DefaultScreen (display))) * masterScale;
            d.isMain = true;
            d.scale = masterScale;
            d.dpi = getDisplayDPI (0);

            displays.add (d);
        }
    }
}

//==============================================================================
bool MouseInputSource::SourceList::addSource()
{
    if (sources.size() == 0)
    {
        addSource (0, true);
        return true;
    }

    return false;
}

bool Desktop::canUseSemiTransparentWindows() noexcept
{
   #if JUCE_USE_XRENDER
    if (XRender::hasCompositingWindowManager())
    {
        int matchedDepth = 0, desiredDepth = 32;

        return Visuals::findVisualFormat (desiredDepth, matchedDepth) != 0
                 && matchedDepth == desiredDepth;
    }
   #endif

    return false;
}

Point<float> MouseInputSource::getCurrentRawMousePosition()
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

    return Point<float> ((float) x, (float) y);
}

void MouseInputSource::setRawMousePosition (Point<float> newPosition)
{
    ScopedXLock xlock;
    Window root = RootWindow (display, DefaultScreen (display));
    XWarpPointer (display, None, root, 0, 0, 0, 0, roundToInt (newPosition.getX()), roundToInt (newPosition.getY()));
}

double Desktop::getDefaultMasterScale()
{
    return 1.0;
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
        static tXScreenSaverSuspend xScreenSaverSuspend = nullptr;

        if (xScreenSaverSuspend == nullptr)
            if (void* h = dlopen ("libXss.so", RTLD_GLOBAL | RTLD_NOW))
                xScreenSaverSuspend = (tXScreenSaverSuspend) dlsym (h, "XScreenSaverSuspend");

        ScopedXLock xlock;
        if (xScreenSaverSuspend != nullptr)
            xScreenSaverSuspend (display, ! isEnabled);
    }
}

bool Desktop::isScreenSaverEnabled()
{
    return screenSaverAllowed;
}

//==============================================================================
void* CustomMouseCursorInfo::create() const
{
    ScopedXLock xlock;
    const unsigned int imageW = image.getWidth();
    const unsigned int imageH = image.getHeight();
    int hotspotX = hotspot.x;
    int hotspotY = hotspot.y;

  #if JUCE_USE_XCURSOR
    {
        typedef XcursorBool (*tXcursorSupportsARGB) (Display*);
        typedef XcursorImage* (*tXcursorImageCreate) (int, int);
        typedef void (*tXcursorImageDestroy) (XcursorImage*);
        typedef Cursor (*tXcursorImageLoadCursor) (Display*, const XcursorImage*);

        static tXcursorSupportsARGB    xcursorSupportsARGB    = nullptr;
        static tXcursorImageCreate     xcursorImageCreate     = nullptr;
        static tXcursorImageDestroy    xcursorImageDestroy    = nullptr;
        static tXcursorImageLoadCursor xcursorImageLoadCursor = nullptr;
        static bool hasBeenLoaded = false;

        if (! hasBeenLoaded)
        {
            hasBeenLoaded = true;

            if (void* h = dlopen ("libXcursor.so", RTLD_GLOBAL | RTLD_NOW))
            {
                xcursorSupportsARGB    = (tXcursorSupportsARGB)    dlsym (h, "XcursorSupportsARGB");
                xcursorImageCreate     = (tXcursorImageCreate)     dlsym (h, "XcursorImageCreate");
                xcursorImageLoadCursor = (tXcursorImageLoadCursor) dlsym (h, "XcursorImageLoadCursor");
                xcursorImageDestroy    = (tXcursorImageDestroy)    dlsym (h, "XcursorImageDestroy");

                if (xcursorSupportsARGB == nullptr || xcursorImageCreate == nullptr
                      || xcursorImageLoadCursor == nullptr || xcursorImageDestroy == nullptr
                      || ! xcursorSupportsARGB (display))
                    xcursorSupportsARGB = nullptr;
            }
        }

        if (xcursorSupportsARGB != nullptr)
        {
            if (XcursorImage* xcImage = xcursorImageCreate (imageW, imageH))
            {
                xcImage->xhot = hotspotX;
                xcImage->yhot = hotspotY;
                XcursorPixel* dest = xcImage->pixels;

                for (int y = 0; y < (int) imageH; ++y)
                    for (int x = 0; x < (int) imageW; ++x)
                        *dest++ = image.getPixelAt (x, y).getARGB();

                void* result = (void*) xcursorImageLoadCursor (display, xcImage);
                xcursorImageDestroy (xcImage);

                if (result != nullptr)
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

            if (c.getAlpha() >= 128)        maskPlane[offset]   |= mask;
            if (c.getBrightness() >= 0.5f)  sourcePlane[offset] |= mask;
        }
    }

    Pixmap sourcePixmap = XCreatePixmapFromBitmapData (display, root, sourcePlane.getData(), cursorW, cursorH, 0xffff, 0, 1);
    Pixmap maskPixmap   = XCreatePixmapFromBitmapData (display, root, maskPlane.getData(),   cursorW, cursorH, 0xffff, 0, 1);

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
    if (cursorHandle != nullptr)
        XFreeCursor (display, (Cursor) cursorHandle);
}

void* MouseCursor::createStandardMouseCursor (MouseCursor::StandardCursorType type)
{
    unsigned int shape;

    switch (type)
    {
        case NormalCursor:
        case ParentCursor:                  return None; // Use parent cursor
        case NoCursor:                      return CustomMouseCursorInfo (Image (Image::ARGB, 16, 16, true), 0, 0).create();

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
        case DraggingHandCursor:            return createDraggingHandCursor();

        case CopyingCursor:
        {
            static unsigned char copyCursorData[] = { 71,73,70,56,57,97,21,0,21,0,145,0,0,0,0,0,255,255,255,0,
              128,128,255,255,255,33,249,4,1,0,0,3,0,44,0,0,0,0,21,0, 21,0,0,2,72,4,134,169,171,16,199,98,11,79,90,71,161,93,56,111,
              78,133,218,215,137,31,82,154,100,200,86,91,202,142,12,108,212,87,235,174, 15,54,214,126,237,226,37,96,59,141,16,37,18,201,142,157,230,204,51,112,
              252,114,147,74,83,5,50,68,147,208,217,16,71,149,252,124,5,0,59,0,0 };
            const int copyCursorSize = 119;

            return CustomMouseCursorInfo (ImageFileFormat::loadFrom (copyCursorData, copyCursorSize), 1, 3).create();
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
    if (LinuxComponentPeer* const lp = dynamic_cast<LinuxComponentPeer*> (peer))
        lp->showMouseCursor ((Cursor) getHandle());
}

void MouseCursor::showInAllWindows() const
{
    for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
        showInWindow (ComponentPeer::getPeer (i));
}

//==============================================================================
Image juce_createIconForFile (const File& /* file */)
{
    return Image::null;
}

//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const bool canMoveFiles)
{
    if (files.size() == 0)
        return false;

    if (MouseInputSource* draggingSource = Desktop::getInstance().getDraggingMouseSource(0))
        if (Component* sourceComp = draggingSource->getComponentUnderMouse())
            if (LinuxComponentPeer* const lp = dynamic_cast<LinuxComponentPeer*> (sourceComp->getPeer()))
                return lp->externalDragFileInit (files, canMoveFiles);

    // This method must be called in response to a component's mouseDown or mouseDrag event!
    jassertfalse;
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text)
{
    if (text.isEmpty())
        return false;

    if (MouseInputSource* draggingSource = Desktop::getInstance().getDraggingMouseSource(0))
        if (Component* sourceComp = draggingSource->getComponentUnderMouse())
            if (LinuxComponentPeer* const lp = dynamic_cast<LinuxComponentPeer*> (sourceComp->getPeer()))
                return lp->externalDragTextInit (text);

    // This method must be called in response to a component's mouseDown or mouseDrag event!
    jassertfalse;
    return false;
}

//==============================================================================
void LookAndFeel::playAlertSound()
{
    std::cout << "\a" << std::flush;
}


//==============================================================================
#if JUCE_MODAL_LOOPS_PERMITTED
void JUCE_CALLTYPE NativeMessageBox::showMessageBox (AlertWindow::AlertIconType iconType,
                                                     const String& title, const String& message,
                                                     Component* /* associatedComponent */)
{
    AlertWindow::showMessageBox (iconType, title, message);
}
#endif

void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (AlertWindow::AlertIconType iconType,
                                                          const String& title, const String& message,
                                                          Component* associatedComponent,
                                                          ModalComponentManager::Callback* callback)
{
    AlertWindow::showMessageBoxAsync (iconType, title, message, String::empty, associatedComponent, callback);
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
