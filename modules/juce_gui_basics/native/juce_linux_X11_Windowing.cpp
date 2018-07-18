/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_DEBUG && ! defined (JUCE_DEBUG_XERRORS)
 #define JUCE_DEBUG_XERRORS 1
#endif

#if JUCE_MODULE_AVAILABLE_juce_gui_extra
 #define JUCE_X11_SUPPORTS_XEMBED 1
#else
 #define JUCE_X11_SUPPORTS_XEMBED 0
#endif

#if JUCE_X11_SUPPORTS_XEMBED
bool juce_handleXEmbedEvent (ComponentPeer*, void*);
unsigned long juce_getCurrentFocusWindow (ComponentPeer*);
#endif

extern WindowMessageReceiveCallback dispatchWindowMessage;

extern XContext windowHandleXContext;

//=============================== X11 - Keys ===================================

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

bool KeyPress::isKeyCurrentlyDown (int keyCode)
{
    ScopedXDisplay xDisplay;

    if (auto display = xDisplay.display)
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

        ScopedXLock xlock (display);

        const int keycode = XKeysymToKeycode (display, (KeySym) keysym);
        const int keybyte = keycode >> 3;
        const int keybit = (1 << (keycode & 7));

        return (Keys::keyStates [keybyte] & keybit) != 0;
    }

    return false;
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
const int KeyPress::F17Key                  = (XK_F17 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F18Key                  = (XK_F18 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F19Key                  = (XK_F19 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F20Key                  = (XK_F20 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F21Key                  = (XK_F21 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F22Key                  = (XK_F22 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F23Key                  = (XK_F23 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F24Key                  = (XK_F24 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F25Key                  = (XK_F25 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F26Key                  = (XK_F26 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F27Key                  = (XK_F27 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F28Key                  = (XK_F28 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F29Key                  = (XK_F29 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F30Key                  = (XK_F30 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F31Key                  = (XK_F31 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F32Key                  = (XK_F32 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F33Key                  = (XK_F33 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F34Key                  = (XK_F34 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F35Key                  = (XK_F35 & 0xff) | Keys::extendedKeyModifier;

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
const int KeyPress::playKey                 = ((int) 0xffeeff00) | Keys::extendedKeyModifier;
const int KeyPress::stopKey                 = ((int) 0xffeeff01) | Keys::extendedKeyModifier;
const int KeyPress::fastForwardKey          = ((int) 0xffeeff02) | Keys::extendedKeyModifier;
const int KeyPress::rewindKey               = ((int) 0xffeeff03) | Keys::extendedKeyModifier;

//================================== X11 - Shm =================================

#if JUCE_USE_XSHM
namespace XSHMHelpers
{
    static int trappedErrorCode = 0;

    extern "C" int errorTrapHandler (Display*, XErrorEvent* err)
    {
        trappedErrorCode = err->error_code;
        return 0;
    }

    static bool isShmAvailable (::Display* display) noexcept
    {
        static bool isChecked = false;
        static bool isAvailable = false;

        if (! isChecked)
        {
            isChecked = true;

            if (display != nullptr)
            {
                int major, minor;
                Bool pixmaps;

                ScopedXLock xlock (display);

                if (XShmQueryVersion (display, &major, &minor, &pixmaps))
                {
                    trappedErrorCode = 0;
                    XErrorHandler oldHandler = XSetErrorHandler (errorTrapHandler);

                    XShmSegmentInfo segmentInfo;
                    zerostruct (segmentInfo);

                    if (auto* xImage = XShmCreateImage (display, DefaultVisual (display, DefaultScreen (display)),
                                                        24, ZPixmap, 0, &segmentInfo, 50, 50))
                    {
                        if ((segmentInfo.shmid = shmget (IPC_PRIVATE,
                                                         (size_t) (xImage->bytes_per_line * xImage->height),
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
            }
        }

        return isAvailable;
    }
}
#endif

//=============================== X11 - Render =================================

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

    static bool isAvailable (::Display* display)
    {
        static bool hasLoaded = false;

        if (! hasLoaded)
        {
            if (display != nullptr)
            {
                hasLoaded = true;

                ScopedXLock xlock (display);

                if (void* h = dlopen ("libXrender.so.1", RTLD_GLOBAL | RTLD_NOW))
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
            }

            xRenderQueryVersion = nullptr;
        }

        return xRenderQueryVersion != nullptr;
    }

    static bool hasCompositingWindowManager (::Display* display) noexcept
    {
        return display != nullptr
                && XGetSelectionOwner (display, Atoms::getCreating ("_NET_WM_CM_S0")) != 0;
    }

    static XRenderPictFormat* findPictureFormat (::Display* display)
    {
        ScopedXLock xlock (display);
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

//================================ X11 - Visuals ===============================

namespace Visuals
{
    static Visual* findVisualWithDepth (::Display* display, int desiredDepth) noexcept
    {
        ScopedXLock xlock (display);

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

        if (auto* xvinfos = XGetVisualInfo (display, desiredMask, &desiredVisual, &numVisuals))
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

    static Visual* findVisualFormat (::Display* display, int desiredDepth, int& matchedDepth) noexcept
    {
        Visual* visual = nullptr;

        if (desiredDepth == 32)
        {
           #if JUCE_USE_XSHM
            if (XSHMHelpers::isShmAvailable (display))
            {
               #if JUCE_USE_XRENDER
                if (XRender::isAvailable (display))
                {
                    if (auto pictFormat = XRender::findPictureFormat (display))
                    {
                        int numVisuals = 0;
                        XVisualInfo desiredVisual;
                        desiredVisual.screen = DefaultScreen (display);
                        desiredVisual.depth = 32;
                        desiredVisual.bits_per_rgb = 8;

                        if (auto xvinfos = XGetVisualInfo (display,
                                                           VisualScreenMask | VisualDepthMask | VisualBitsPerRGBMask,
                                                           &desiredVisual, &numVisuals))
                        {
                            for (int i = 0; i < numVisuals; ++i)
                            {
                                auto pictVisualFormat = XRender::xRenderFindVisualFormat (display, xvinfos[i].visual);

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
                    visual = findVisualWithDepth (display, 32);

                    if (visual != nullptr)
                        matchedDepth = 32;
                }
            }
           #endif
        }

        if (visual == nullptr && desiredDepth >= 24)
        {
            visual = findVisualWithDepth (display, 24);

            if (visual != nullptr)
                matchedDepth = 24;
        }

        if (visual == nullptr && desiredDepth >= 16)
        {
            visual = findVisualWithDepth (display, 16);

            if (visual != nullptr)
                matchedDepth = 16;
        }

        return visual;
    }
}

//================================= X11 - Bitmap ===============================

class XBitmapImage  : public ImagePixelData
{
public:
    XBitmapImage (::Display* d, Image::PixelFormat format, int w, int h,
                  bool clearImage, unsigned int imageDepth_, Visual* visual)
        : ImagePixelData (format, w, h),
          imageDepth (imageDepth_),
          display (d)
    {
        jassert (format == Image::RGB || format == Image::ARGB);

        pixelStride = (format == Image::RGB) ? 3 : 4;
        lineStride = ((w * pixelStride + 3) & ~3);

        ScopedXLock xlock (display);

       #if JUCE_USE_XSHM
        usingXShm = false;

        if ((imageDepth > 16) && XSHMHelpers::isShmAvailable (display))
        {
            zerostruct (segmentInfo);

            segmentInfo.shmid = -1;
            segmentInfo.shmaddr = (char *) -1;
            segmentInfo.readOnly = False;

            xImage = XShmCreateImage (display, visual, imageDepth, ZPixmap, 0,
                                      &segmentInfo, (unsigned int) w, (unsigned int) h);

            if (xImage != nullptr)
            {
                if ((segmentInfo.shmid = shmget (IPC_PRIVATE,
                                                 (size_t) (xImage->bytes_per_line * xImage->height),
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

        if (! isUsingXShm())
       #endif
        {
            imageDataAllocated.allocate ((size_t) (lineStride * h), format == Image::ARGB && clearImage);
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
                const int pixStride = 2;
                const int stride = ((w * pixStride + 3) & ~3);

                imageData16Bit.malloc (stride * h);
                xImage->data = imageData16Bit;
                xImage->bitmap_pad = 16;
                xImage->depth = pixStride * 8;
                xImage->bytes_per_line = stride;
                xImage->bits_per_pixel = pixStride * 8;
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
        ScopedXLock xlock (display);

        if (gc != None)
            XFreeGC (display, gc);

       #if JUCE_USE_XSHM
        if (isUsingXShm())
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

    void initialiseBitmapData (Image::BitmapData& bitmap, int x, int y,
                               Image::BitmapData::ReadWriteMode mode) override
    {
        bitmap.data = imageData + x * pixelStride + y * lineStride;
        bitmap.pixelFormat = pixelFormat;
        bitmap.lineStride = lineStride;
        bitmap.pixelStride = pixelStride;

        if (mode != Image::BitmapData::readOnly)
            sendDataChangeMessage();
    }

    ImagePixelData::Ptr clone() override
    {
        jassertfalse;
        return nullptr;
    }

    ImageType* createType() const override     { return new NativeImageType(); }

    void blitToWindow (Window window, int dx, int dy,
                       unsigned int dw, unsigned int dh, int sx, int sy)
    {
        ScopedXLock xlock (display);

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
            auto rMask   = (uint32) xImage->red_mask;
            auto gMask   = (uint32) xImage->green_mask;
            auto bMask   = (uint32) xImage->blue_mask;
            auto rShiftL = (uint32) jmax (0,  getShiftNeeded (rMask));
            auto rShiftR = (uint32) jmax (0, -getShiftNeeded (rMask));
            auto gShiftL = (uint32) jmax (0,  getShiftNeeded (gMask));
            auto gShiftR = (uint32) jmax (0, -getShiftNeeded (gMask));
            auto bShiftL = (uint32) jmax (0,  getShiftNeeded (bMask));
            auto bShiftR = (uint32) jmax (0, -getShiftNeeded (bMask));

            const Image::BitmapData srcData (Image (this), Image::BitmapData::readOnly);

            for (int y = sy; y < sy + (int)dh; ++y)
            {
                const uint8* p = srcData.getPixelPointer (sx, y);

                for (int x = sx; x < sx + (int)dw; ++x)
                {
                    auto* pixel = (const PixelRGB*) p;
                    p += srcData.pixelStride;

                    XPutPixel (xImage, x, y,
                                   (((((uint32) pixel->getRed())   << rShiftL) >> rShiftR) & rMask)
                                 | (((((uint32) pixel->getGreen()) << gShiftL) >> gShiftR) & gMask)
                                 | (((((uint32) pixel->getBlue())  << bShiftL) >> bShiftR) & bMask));
                }
            }
        }

        // blit results to screen.
       #if JUCE_USE_XSHM
        if (isUsingXShm())
            XShmPutImage (display, (::Drawable) window, gc, xImage, sx, sy, dx, dy, dw, dh, True);
        else
       #endif
            XPutImage (display, (::Drawable) window, gc, xImage, sx, sy, dx, dy, dw, dh);
    }

    #if JUCE_USE_XSHM
    bool isUsingXShm() const noexcept       { return usingXShm; }
    #endif

private:
    //==============================================================================
    XImage* xImage = {};
    const unsigned int imageDepth;
    HeapBlock<uint8> imageDataAllocated;
    HeapBlock<char> imageData16Bit;
    int pixelStride, lineStride;
    uint8* imageData = {};
    GC gc = None;
    ::Display* display = {};

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

//================================ X11 - DisplayGeometry =======================

class DisplayGeometry
{
private:
    //==============================================================================
    DisplayGeometry (::Display* display, double masterScale)
    {
        jassert (instance == nullptr);
        instance = this;

        queryDisplayInfos (display, masterScale);
        updatePositions();
    }

public:
    //==============================================================================
    struct ExtendedInfo
    {
        // Unlike Desktop::Displays::Display, the following is in
        // physical pixels, i.e. the area is not scaled
        Rectangle<int> totalBounds;
        // Usable bounds is the usable area in local coordinates
        // with respect to the above totalBounds
        Rectangle<int> usableBounds;
        // top-left point of display in scaled coordinates. This
        // is different from totalBounds.getTopLeft() / scale,
        // because the neighbouring display may have a different
        // scale factor
        Point<int> topLeftScaled;
        double dpi, scale;
        bool isMain;
    };

    Array<ExtendedInfo> infos;

    //==============================================================================
    ExtendedInfo& findDisplayForRect (Rectangle<int> bounds, bool isScaledBounds)
    {
        int maxArea = -1;
        ExtendedInfo* retval = nullptr;

        for (int i = 0; i < infos.size(); ++i)
        {
            auto& dpy = infos.getReference (i);

            auto displayBounds = dpy.totalBounds;

            if (isScaledBounds)
                displayBounds = (displayBounds.withZeroOrigin() / dpy.scale) + dpy.topLeftScaled;

            displayBounds = displayBounds.getIntersection (bounds);
            int area = displayBounds.getWidth() * displayBounds.getHeight();

            if (area >= maxArea)
            {
                maxArea = area;
                retval = &dpy;
            }
        }

        return *retval;
    }

    ExtendedInfo& findDisplayForPoint (Point<int> pt, bool isScaledPoint)
    {
        int minDistance = (int) ((((unsigned int)(-1)) >> 1) - 1);
        ExtendedInfo* retval = nullptr;

        for (int i = 0; i < infos.size(); ++i)
        {
            auto& dpy = infos.getReference (i);

            auto displayBounds = dpy.totalBounds;

            if (isScaledPoint)
                displayBounds = (displayBounds.withZeroOrigin() / dpy.scale) + dpy.topLeftScaled;

            if (displayBounds.contains (pt))
                return dpy;

            int distance = displayBounds.getCentre().getDistanceFrom (pt);
            if (distance <= minDistance)
            {
                minDistance = distance;
                retval = &dpy;
            }
        }

        return *retval;
    }

    //==============================================================================
    static Rectangle<int> physicalToScaled (Rectangle<int> physicalBounds)
    {
        // first find with which display physicalBounds has the most overlap
        auto& dpy = getInstance().findDisplayForRect (physicalBounds, false);

        // convert to local screen bounds
        physicalBounds -= dpy.totalBounds.getTopLeft();

        // now we can safely scale the coordinates and convert to global again
        return (physicalBounds / dpy.scale) + dpy.topLeftScaled;
    }

    static Rectangle<int> scaledToPhysical (Rectangle<int> scaledBounds)
    {
        // first find with which display physicalBounds has the most overlap
        auto& dpy = getInstance().findDisplayForRect (scaledBounds, true);

        // convert to local screen bounds
        scaledBounds -= dpy.topLeftScaled;

        // now we can safely scale the coordinates and convert to global again
        return (scaledBounds * dpy.scale) + dpy.totalBounds.getTopLeft();
    }

    //==============================================================================
    template <typename ValueType>
    static Point<ValueType> physicalToScaled (Point<ValueType> physicalPoint)
    {
        auto& dpy = getInstance().findDisplayForPoint (physicalPoint.roundToInt(), false);

        Point<ValueType> scaledTopLeft   (dpy.topLeftScaled.getX(), dpy.topLeftScaled.getY());
        Point<ValueType> physicalTopLeft (dpy.totalBounds.getX(), dpy.totalBounds.getY());

        return ((physicalPoint - physicalTopLeft) / dpy.scale) + scaledTopLeft;
    }

    template <typename ValueType>
    static Point<ValueType> scaledToPhysical (const Point<ValueType>& scaledPoint)
    {
        auto& dpy = getInstance().findDisplayForPoint (scaledPoint.roundToInt(), true);

        Point<ValueType> scaledTopLeft   (dpy.topLeftScaled.getX(), dpy.topLeftScaled.getY());
        Point<ValueType> physicalTopLeft (dpy.totalBounds.getX(), dpy.totalBounds.getY());

        return ((scaledPoint - scaledTopLeft) * dpy.scale) + physicalTopLeft;
    }

    //==============================================================================
    static DisplayGeometry& getInstance()
    {
        jassert (instance != nullptr);
        return *instance;
    }

    static DisplayGeometry& getOrCreateInstance (::Display* display, double masterScale)
    {
        if (instance == nullptr)
            new DisplayGeometry (display, masterScale);

        return getInstance();
    }

private:
    //==============================================================================
    static DisplayGeometry* instance;

    //==============================================================================
   #if JUCE_USE_XINERAMA
    static Array<XineramaScreenInfo> XineramaQueryDisplays (::Display* display)
    {
        typedef Bool (*tXineramaIsActive) (::Display*);
        typedef XineramaScreenInfo* (*tXineramaQueryScreens) (::Display*, int*);

        int major_opcode, first_event, first_error;

        if (XQueryExtension (display, "XINERAMA", &major_opcode, &first_event, &first_error))
        {
            static void* libXinerama = nullptr;
            static tXineramaIsActive isActiveFuncPtr = nullptr;
            static tXineramaQueryScreens xineramaQueryScreens = nullptr;

            if (libXinerama == nullptr)
            {
                libXinerama = dlopen ("libXinerama.so", RTLD_GLOBAL | RTLD_NOW);

                if (libXinerama == nullptr)
                    libXinerama = dlopen ("libXinerama.so.1", RTLD_GLOBAL | RTLD_NOW);

                if (libXinerama != nullptr)
                {
                    isActiveFuncPtr = (tXineramaIsActive) dlsym (libXinerama, "XineramaIsActive");
                    xineramaQueryScreens = (tXineramaQueryScreens) dlsym (libXinerama, "XineramaQueryScreens");
                }
            }

            if (isActiveFuncPtr != nullptr && xineramaQueryScreens != nullptr && isActiveFuncPtr (display) != 0)
            {
                int numScreens;

                if (auto* xinfo = xineramaQueryScreens (display, &numScreens))
                {
                    Array<XineramaScreenInfo> infos (xinfo, numScreens);
                    XFree (xinfo);

                    return infos;
                }
            }
        }

        return {};
    }
   #endif

    //==============================================================================
   #if JUCE_USE_XRANDR
    class XRandrWrapper
    {
    private:
        XRandrWrapper()
        {
            if (libXrandr == nullptr)
            {
                libXrandr = dlopen ("libXrandr.so", RTLD_GLOBAL | RTLD_NOW);

                if (libXrandr == nullptr)
                    libXrandr = dlopen ("libXrandr.so.2", RTLD_GLOBAL | RTLD_NOW);

                if (libXrandr != nullptr)
                {
                    getScreenResourcesPtr  = (tXRRGetScreenResources)  dlsym (libXrandr, "XRRGetScreenResources");
                    freeScreenResourcesPtr = (tXRRFreeScreenResources) dlsym (libXrandr, "XRRFreeScreenResources");
                    getOutputInfoPtr       = (tXRRGetOutputInfo)       dlsym (libXrandr, "XRRGetOutputInfo");
                    freeOutputInfoPtr      = (tXRRFreeOutputInfo)      dlsym (libXrandr, "XRRFreeOutputInfo");
                    getCrtcInfoPtr         = (tXRRGetCrtcInfo)         dlsym (libXrandr, "XRRGetCrtcInfo");
                    freeCrtcInfoPtr        = (tXRRFreeCrtcInfo)        dlsym (libXrandr, "XRRFreeCrtcInfo");
                    getOutputPrimaryPtr    = (tXRRGetOutputPrimary)    dlsym (libXrandr, "XRRGetOutputPrimary");
                }
            }

            instance = this;
        }

    public:
        //==============================================================================
        static XRandrWrapper& getInstance()
        {
            if (instance == nullptr)
                instance = new XRandrWrapper();

            return *instance;
        }

        //==============================================================================
        XRRScreenResources* getScreenResources (::Display* display, ::Window window)
        {
            if (getScreenResourcesPtr != nullptr)
                return getScreenResourcesPtr (display, window);

            return nullptr;
        }

        XRROutputInfo* getOutputInfo (::Display* display, XRRScreenResources* resources, RROutput output)
        {
            if (getOutputInfoPtr != nullptr)
                return getOutputInfoPtr (display, resources, output);

            return nullptr;
        }

        XRRCrtcInfo* getCrtcInfo (::Display* display, XRRScreenResources* resources, RRCrtc crtc)
        {
            if (getCrtcInfoPtr != nullptr)
                return getCrtcInfoPtr (display, resources, crtc);

            return nullptr;
        }

        RROutput getOutputPrimary (::Display* display, ::Window window)
        {
            if (getOutputPrimaryPtr != nullptr)
                return getOutputPrimaryPtr (display, window);

            return 0;
        }

        //==============================================================================
        void freeScreenResources (XRRScreenResources* ptr)
        {
            if (freeScreenResourcesPtr != nullptr)
                freeScreenResourcesPtr (ptr);
        }

        void freeOutputInfo (XRROutputInfo* ptr)
        {
            if (freeOutputInfoPtr != nullptr)
                freeOutputInfoPtr (ptr);
        }

        void freeCrtcInfo (XRRCrtcInfo* ptr)
        {
            if (freeCrtcInfoPtr != nullptr)
                freeCrtcInfoPtr (ptr);
        }

    private:
        static XRandrWrapper* instance;

        using tXRRGetScreenResources   = XRRScreenResources* (*) (::Display*, ::Window);
        using tXRRFreeScreenResources  = void (*) (XRRScreenResources*);
        using tXRRGetOutputInfo        = XRROutputInfo* (*) (::Display*, XRRScreenResources*, RROutput);
        using tXRRFreeOutputInfo       = void (*) (XRROutputInfo*);
        using tXRRGetCrtcInfo          = XRRCrtcInfo* (*) (::Display*, XRRScreenResources*, RRCrtc);
        using tXRRFreeCrtcInfo         = void (*) (XRRCrtcInfo*);
        using tXRRGetOutputPrimary     = RROutput (*) (::Display*, ::Window);

        void* libXrandr = nullptr;
        tXRRGetScreenResources getScreenResourcesPtr = nullptr;
        tXRRFreeScreenResources freeScreenResourcesPtr = nullptr;
        tXRRGetOutputInfo getOutputInfoPtr = nullptr;
        tXRRFreeOutputInfo freeOutputInfoPtr = nullptr;
        tXRRGetCrtcInfo getCrtcInfoPtr = nullptr;
        tXRRFreeCrtcInfo freeCrtcInfoPtr = nullptr;
        tXRRGetOutputPrimary getOutputPrimaryPtr = nullptr;
    };
   #endif


    static double getDisplayDPI (::Display* display, int index)
    {
        double dpiX = (DisplayWidth  (display, index) * 25.4) / DisplayWidthMM  (display, index);
        double dpiY = (DisplayHeight (display, index) * 25.4) / DisplayHeightMM (display, index);
        return (dpiX + dpiY) / 2.0;
    }

    static double getScaleForDisplay (const String& name, const ExtendedInfo& info)
    {
        if (name.isNotEmpty())
        {
            // Ubuntu and derived distributions now save a per-display scale factor as a configuration
            // variable. This can be changed in the Monitor system settings panel.
            ChildProcess dconf;

            if (File ("/usr/bin/dconf").existsAsFile()
                 && dconf.start ("/usr/bin/dconf read /com/ubuntu/user-interface/scale-factor", ChildProcess::wantStdOut))
            {
                if (dconf.waitForProcessToFinish (200))
                {
                    auto jsonOutput = dconf.readAllProcessOutput().replaceCharacter ('\'', '"');

                    if (dconf.getExitCode() == 0 && jsonOutput.isNotEmpty())
                    {
                        auto jsonVar = JSON::parse (jsonOutput);

                        if (auto* object = jsonVar.getDynamicObject())
                        {
                            auto scaleFactorVar = object->getProperty (name);

                            if (! scaleFactorVar.isVoid())
                            {
                                auto scaleFactor = ((double) scaleFactorVar) / 8.0;

                                if (scaleFactor > 0.0)
                                    return scaleFactor;
                            }
                        }
                    }
                }
            }
        }

        {
            // Other gnome based distros now use gsettings for a global scale factor
            ChildProcess gsettings;

            if (File ("/usr/bin/gsettings").existsAsFile()
                 && gsettings.start ("/usr/bin/gsettings get org.gnome.desktop.interface scaling-factor", ChildProcess::wantStdOut))
            {
                if (gsettings.waitForProcessToFinish (200))
                {
                    auto gsettingsOutput = StringArray::fromTokens (gsettings.readAllProcessOutput(), true);

                    if (gsettingsOutput.size() >= 2 && gsettingsOutput[1].length() > 0)
                    {
                        auto scaleFactor = gsettingsOutput[1].getDoubleValue();

                        if (scaleFactor > 0.0)
                            return scaleFactor;
                    }
                }
            }
        }

        // If no scale factor is set by GNOME or Ubuntu then calculate from monitor dpi
        // We use the same approach as chromium which simply divides the dpi by 96
        // and then rounds the result
        return round (info.dpi / 150.0);
    }

    //==============================================================================
    void queryDisplayInfos (::Display* display, double masterScale) noexcept
    {
        ScopedXLock xlock (display);

       #if JUCE_USE_XRANDR
        {
            int major_opcode, first_event, first_error;

            if (XQueryExtension (display, "RANDR", &major_opcode, &first_event, &first_error))
            {
                auto& xrandr = XRandrWrapper::getInstance();

                auto numMonitors = ScreenCount (display);
                auto mainDisplay = xrandr.getOutputPrimary (display, RootWindow (display, 0));

                for (int i = 0; i < numMonitors; ++i)
                {
                    if (auto* screens = xrandr.getScreenResources (display, RootWindow (display, i)))
                    {
                        for (int j = 0; j < screens->noutput; ++j)
                        {
                            if (screens->outputs[j])
                            {
                                // Xrandr on the raspberry pi fails to determine the main display (mainDisplay == 0)!
                                // Detect this edge case and make the first found display the main display
                                if (! mainDisplay)
                                    mainDisplay = screens->outputs[j];

                                if (auto* output = xrandr.getOutputInfo (display, screens, screens->outputs[j]))
                                {
                                    if (output->crtc)
                                    {
                                        if (auto* crtc = xrandr.getCrtcInfo (display, screens, output->crtc))
                                        {
                                            ExtendedInfo e;
                                            e.totalBounds = Rectangle<int> (crtc->x, crtc->y,
                                                                            (int) crtc->width, (int) crtc->height);
                                            e.usableBounds = e.totalBounds.withZeroOrigin(); // Support for usable area is not implemented in JUCE yet
                                            e.topLeftScaled = e.totalBounds.getTopLeft();
                                            e.isMain = (mainDisplay == screens->outputs[j]) && (i == 0);
                                            e.dpi = getDisplayDPI (display, 0);

                                            // The raspberry pi returns a zero sized display, so we need to guard for divide-by-zero
                                            if (output->mm_width > 0 && output->mm_height > 0)
                                                e.dpi = ((static_cast<double> (crtc->width) * 25.4 * 0.5) / static_cast<double> (output->mm_width))
                                                    + ((static_cast<double> (crtc->height) * 25.4 * 0.5) / static_cast<double> (output->mm_height));

                                            double scale = getScaleForDisplay (output->name, e);
                                            scale = (scale <= 0.1 ? 1.0 : scale);

                                            e.scale = masterScale * scale;

                                            infos.add (e);

                                            xrandr.freeCrtcInfo (crtc);
                                        }
                                    }

                                    xrandr.freeOutputInfo (output);
                                }
                            }
                        }

                        xrandr.freeScreenResources (screens);
                    }
                }
            }
        }

        if (infos.isEmpty())
       #endif
       #if JUCE_USE_XINERAMA
        {
            auto screens = XineramaQueryDisplays (display);
            int numMonitors = screens.size();

            for (int index = 0; index < numMonitors; ++index)
            {
                for (int j = numMonitors; --j >= 0;)
                {
                    if (screens[j].screen_number == index)
                    {
                        ExtendedInfo e;
                        e.totalBounds = Rectangle<int> (screens[j].x_org,
                                                        screens[j].y_org,
                                                        screens[j].width,
                                                        screens[j].height);
                        e.usableBounds = e.totalBounds.withZeroOrigin(); // Support for usable area is not implemented in JUCE yet
                        e.topLeftScaled = e.totalBounds.getTopLeft(); // this will be overwritten by updatePositions later
                        e.isMain = (index == 0);
                        e.scale = masterScale;
                        e.dpi = getDisplayDPI (display, 0); // (all screens share the same DPI)

                        infos.add (e);
                    }
                }
            }
        }

        if (infos.isEmpty())
       #endif
        {
            Atom hints = Atoms::getIfExists (display, "_NET_WORKAREA");

            if (hints != None)
            {
                auto numMonitors = ScreenCount (display);

                for (int i = 0; i < numMonitors; ++i)
                {
                    GetXProperty prop (display, RootWindow (display, i), hints, 0, 4, false, XA_CARDINAL);

                    if (prop.success && prop.actualType == XA_CARDINAL && prop.actualFormat == 32 && prop.numItems == 4)
                    {
                        auto position = (const long*) prop.data;

                        ExtendedInfo e;
                        e.totalBounds = Rectangle<int> ((int) position[0], (int) position[1],
                                                        (int) position[2], (int) position[3]);
                        e.usableBounds = e.totalBounds.withZeroOrigin(); // Support for usable area is not implemented in JUCE yet
                        e.topLeftScaled = e.totalBounds.getTopLeft(); // this will be overwritten by updatePositions later
                        e.isMain = infos.isEmpty();
                        e.scale = masterScale;
                        e.dpi = getDisplayDPI (display, i);

                        infos.add (e);
                    }
                }
            }

            if (infos.isEmpty())
            {
                ExtendedInfo e;
                e.totalBounds = Rectangle<int> (DisplayWidth  (display, DefaultScreen (display)),
                                                DisplayHeight (display, DefaultScreen (display)));
                e.usableBounds = e.totalBounds; // Support for usable area is not implemented in JUCE yet
                e.topLeftScaled = e.totalBounds.getTopLeft(); // this will be overwritten by updatePositions later
                e.isMain = true;
                e.scale = masterScale;
                e.dpi = getDisplayDPI (display, 0);

                infos.add (e);
            }
        }
    }

    //==============================================================================
    void updateScaledDisplayCoordinate (bool updateYCoordinates)
    {
        if (infos.size() < 2)
            return;

        Array<ExtendedInfo*> copy;

        for (auto& i : infos)
            copy.add (&i);

        std::sort (copy.begin(), copy.end(), [updateYCoordinates] (const ExtendedInfo* a, const ExtendedInfo* b)
        {
            if (updateYCoordinates)
                return a->totalBounds.getY() < b->totalBounds.getY();

            return a->totalBounds.getX() < b->totalBounds.getX();
        });

        for (int i = 1; i < copy.size(); ++i)
        {
            auto& current = *copy[i];

            // Is this screen's position aligned to any other previous display?
            for (int j = i - 1; j >= 0; --j)
            {
                auto& other = *copy[j];
                auto prevCoordinate = updateYCoordinates ? other.totalBounds.getBottom() : other.totalBounds.getRight();
                auto curCoordinate  = updateYCoordinates ? current.totalBounds.getY() : current.totalBounds.getX();

                if (prevCoordinate == curCoordinate)
                {
                    // both displays are aligned! As "other" comes before "current" in the array, it must already
                    // have a valid topLeftScaled which we can use
                    auto topLeftScaled = other.topLeftScaled;
                    topLeftScaled += Point<int> (other.totalBounds.getWidth(), other.totalBounds.getHeight()) / other.scale;

                    if (updateYCoordinates)
                        current.topLeftScaled.setY (topLeftScaled.getY());
                    else
                        current.topLeftScaled.setX (topLeftScaled.getX());

                    break;
                }
            }
        }
    }

    void updatePositions()
    {
        updateScaledDisplayCoordinate (false);
        updateScaledDisplayCoordinate (true);
    }
};

DisplayGeometry* DisplayGeometry::instance = nullptr;

#if JUCE_USE_XRANDR
DisplayGeometry::XRandrWrapper* DisplayGeometry::XRandrWrapper::instance = nullptr;
#endif

//=============================== X11 - Pixmap =================================

namespace PixmapHelpers
{
    Pixmap createColourPixmapFromImage (::Display* display, const Image& image)
    {
        ScopedXLock xlock (display);

        auto width = (unsigned int) image.getWidth();
        auto height = (unsigned int) image.getHeight();
        HeapBlock<uint32> colour (width * height);
        int index = 0;

        for (int y = 0; y < (int)  height; ++y)
            for (int x = 0; x < (int) width; ++x)
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

    Pixmap createMaskPixmapFromImage (::Display* display, const Image& image)
    {
        ScopedXLock xlock (display);

        auto width = (unsigned int) image.getWidth();
        auto height = (unsigned int) image.getHeight();
        auto stride = (width + 7) >> 3;
        HeapBlock<char> mask;
        mask.calloc (stride * height);
        const bool msbfirst = (BitmapBitOrder (display) == MSBFirst);

        for (unsigned int y = 0; y < height; ++y)
        {
            for (unsigned int x = 0; x < width; ++x)
            {
                auto bit = (char) (1 << (msbfirst ? (7 - (x & 7)) : (x & 7)));
                const unsigned int offset = y * stride + (x >> 3);

                if (image.getPixelAt ((int) x, (int) y).getAlpha() >= 128)
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

    return CustomMouseCursorInfo (ImageFileFormat::loadFrom (dragHandData, dragHandDataSize), { 8, 7 }).create();
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
    LinuxComponentPeer (Component& comp, int windowStyleFlags, Window parentToAddTo)
        : ComponentPeer (comp, windowStyleFlags),
          isAlwaysOnTop (comp.isAlwaysOnTop())
    {
        // it's dangerous to create a window on a thread other than the message thread..
        jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

        display = XWindowSystem::getInstance()->displayRef();

        atoms.reset (new Atoms (display));
        dragState.reset (new DragState (display));
        repainter.reset (new LinuxRepaintManager (*this, display));

        if (isAlwaysOnTop)
            ++numAlwaysOnTopPeers;

        createWindow (parentToAddTo);

        setTitle (component.getName());

        getNativeRealtimeModifiers = []
        {
            ScopedXDisplay xDisplay;

            if (auto display = xDisplay.display)
            {
                Window root, child;
                int x, y, winx, winy;
                unsigned int mask;
                int mouseMods = 0;

                ScopedXLock xlock (display);

                if (XQueryPointer (display, RootWindow (display, DefaultScreen (display)),
                                   &root, &child, &x, &y, &winx, &winy, &mask) != False)
                {
                    if ((mask & Button1Mask) != 0)  mouseMods |= ModifierKeys::leftButtonModifier;
                    if ((mask & Button2Mask) != 0)  mouseMods |= ModifierKeys::middleButtonModifier;
                    if ((mask & Button3Mask) != 0)  mouseMods |= ModifierKeys::rightButtonModifier;
                }

                ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (mouseMods);
            }

            return ModifierKeys::currentModifiers;
        };
    }

    ~LinuxComponentPeer()
    {
        // it's dangerous to delete a window on a thread other than the message thread..
        jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

       #if JUCE_X11_SUPPORTS_XEMBED
        juce_handleXEmbedEvent (this, nullptr);
       #endif

        deleteIconPixmaps();
        destroyWindow();
        windowH = 0;

        if (isAlwaysOnTop)
            --numAlwaysOnTopPeers;

        // delete before display
        repainter = nullptr;

        display = XWindowSystem::getInstance()->displayUnref();
    }

    //==============================================================================
    void* getNativeHandle() const override
    {
        return (void*) windowH;
    }

    static LinuxComponentPeer* getPeerFor (Window windowHandle) noexcept
    {
        XPointer peer = nullptr;

        if (display != nullptr)
        {
            ScopedXLock xlock (display);

            if (! XFindContext (display, (XID) windowHandle, windowHandleXContext, &peer))
                if (peer != nullptr && ! ComponentPeer::isValidPeer (reinterpret_cast<LinuxComponentPeer*> (peer)))
                    peer = nullptr;
        }

        return reinterpret_cast<LinuxComponentPeer*> (peer);
    }

    void setVisible (bool shouldBeVisible) override
    {
        ScopedXLock xlock (display);

        if (shouldBeVisible)
            XMapWindow (display, windowH);
        else
            XUnmapWindow (display, windowH);
    }

    void setTitle (const String& title) override
    {
        XTextProperty nameProperty;
        char* strings[] = { const_cast<char*> (title.toRawUTF8()) };
        ScopedXLock xlock (display);

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
            Atom fs = Atoms::getIfExists (display, "_NET_WM_STATE_FULLSCREEN");

            if (fs != None)
            {
                Window root = RootWindow (display, DefaultScreen (display));

                XClientMessageEvent clientMsg;
                clientMsg.display = display;
                clientMsg.window = windowH;
                clientMsg.type = ClientMessage;
                clientMsg.format = 32;
                clientMsg.message_type = atoms->windowState;
                clientMsg.data.l[0] = 0;  // Remove
                clientMsg.data.l[1] = (long) fs;
                clientMsg.data.l[2] = 0;
                clientMsg.data.l[3] = 1;  // Normal Source

                ScopedXLock xlock (display);
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

            currentScaleFactor = DisplayGeometry::getInstance().findDisplayForRect (bounds, true).scale;

            auto physicalBounds = DisplayGeometry::scaledToPhysical (bounds);

            WeakReference<Component> deletionChecker (&component);
            ScopedXLock xlock (display);

            auto* hints = XAllocSizeHints();
            hints->flags  = USSize | USPosition;
            hints->x      = physicalBounds.getX();
            hints->y      = physicalBounds.getY();
            hints->width  = physicalBounds.getWidth();
            hints->height = physicalBounds.getHeight();

            if ((getStyleFlags() & windowIsResizable) == 0)
            {
                hints->min_width  = hints->max_width  = hints->width;
                hints->min_height = hints->max_height = hints->height;
                hints->flags |= PMinSize | PMaxSize;
            }

            XSetWMNormalHints (display, windowH, hints);
            XFree (hints);

            XMoveResizeWindow (display, windowH,
                               physicalBounds.getX() - windowBorder.getLeft(),
                               physicalBounds.getY() - windowBorder.getTop(),
                               (unsigned int) physicalBounds.getWidth(),
                               (unsigned int) physicalBounds.getHeight());

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
            clientMsg.message_type = atoms->changeState;
            clientMsg.data.l[0] = IconicState;

            ScopedXLock xlock (display);
            XSendEvent (display, root, false, SubstructureRedirectMask | SubstructureNotifyMask, (XEvent*) &clientMsg);
        }
        else
        {
            setVisible (true);
        }
    }

    bool isMinimised() const override
    {
        ScopedXLock xlock (display);
        GetXProperty prop (display, windowH, atoms->state, 0, 64, false, atoms->state);

        return prop.success
                && prop.actualType == atoms->state
                && prop.actualFormat == 32
                && prop.numItems > 0
                && ((unsigned long*) prop.data)[0] == IconicState;
    }

    void setFullScreen (bool shouldBeFullScreen) override
    {
        auto r = lastNonFullscreenBounds; // (get a copy of this before de-minimising)

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

        ScopedXLock xlock (display);
        if (XQueryTree (display, windowH, &root, &parent, &windowList, &windowListSize) != 0)
        {
            if (windowList != nullptr)
                XFree (windowList);

            return parent == possibleParent;
        }

        return false;
    }

    bool isParentWindowOf (Window possibleChild) const
    {
        if (windowH != 0 && possibleChild != 0)
        {
            if (possibleChild == windowH)
                return true;

            Window* windowList = nullptr;
            uint32 windowListSize = 0;
            Window parent, root;

            ScopedXLock xlock (display);
            if (XQueryTree (display, possibleChild, &root, &parent, &windowList, &windowListSize) != 0)
            {
                if (windowList != nullptr)
                    XFree (windowList);

                if (parent == root)
                    return false;

                return isParentWindowOf (parent);
            }
        }

        return false;
    }

    bool isFrontWindow() const
    {
        Window* windowList = nullptr;
        uint32 windowListSize = 0;
        bool result = false;

        ScopedXLock xlock (display);
        Window parent, root = RootWindow (display, DefaultScreen (display));

        if (XQueryTree (display, root, &root, &parent, &windowList, &windowListSize) != 0)
        {
            for (int i = (int) windowListSize; --i >= 0;)
            {
                if (auto* peer = LinuxComponentPeer::getPeerFor (windowList[i]))
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
            auto* c = Desktop::getInstance().getComponent (i);

            if (c == &component)
                break;

            if (! c->isVisible())
                continue;

            if (auto* peer = c->getPeer())
                if (peer->contains (localPos + bounds.getPosition() - peer->getBounds().getPosition(), true))
                    return false;
        }

        if (trueIfInAChildWindow)
            return true;

        ::Window root, child;
        int wx, wy;
        unsigned int ww, wh, bw, bitDepth;

        ScopedXLock xlock (display);

        localPos *= currentScaleFactor;

        return XGetGeometry (display, (::Drawable) windowH, &root, &wx, &wy, &ww, &wh, &bw, &bitDepth)
                && XTranslateCoordinates (display, windowH, windowH, localPos.getX(), localPos.getY(), &wx, &wy, &child)
                && child == None;
    }

    BorderSize<int> getFrameSize() const override
    {
        return {};
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
            ScopedXLock xlock (display);
            XEvent ev;
            ev.xclient.type = ClientMessage;
            ev.xclient.serial = 0;
            ev.xclient.send_event = True;
            ev.xclient.message_type = atoms->activeWin;
            ev.xclient.window = windowH;
            ev.xclient.format = 32;
            ev.xclient.data.l[0] = 2;
            ev.xclient.data.l[1] = getUserTime();
            ev.xclient.data.l[2] = 0;
            ev.xclient.data.l[3] = 0;
            ev.xclient.data.l[4] = 0;

            XSendEvent (display, RootWindow (display, DefaultScreen (display)),
                        False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);

            XSync (display, False);
        }

        handleBroughtToFront();
    }

    void toBehind (ComponentPeer* other) override
    {
        if (auto* otherPeer = dynamic_cast<LinuxComponentPeer*> (other))
        {
            if (otherPeer->styleFlags & windowIsTemporary)
                return;

            setMinimised (false);

            Window newStack[] = { otherPeer->windowH, windowH };

            ScopedXLock xlock (display);
            XRestackWindows (display, newStack, 2);
        }
        else
            jassertfalse; // wrong type of window?
    }

    bool isFocused() const override
    {
        int revert = 0;
        Window focusedWindow = 0;
        ScopedXLock xlock (display);
        XGetInputFocus (display, &focusedWindow, &revert);

        return isParentWindowOf (focusedWindow);
    }

    Window getFocusWindow()
    {
       #if JUCE_X11_SUPPORTS_XEMBED
        if (Window w = (Window) juce_getCurrentFocusWindow (this))
            return w;
       #endif

        return windowH;
    }

    void grabFocus() override
    {
        XWindowAttributes atts;
        ScopedXLock xlock (display);

        if (windowH != 0
            && XGetWindowAttributes (display, windowH, &atts)
            && atts.map_state == IsViewable
            && ! isFocused())
        {
            XSetInputFocus (display, getFocusWindow(), RevertToParent, (::Time) getUserTime());
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
        HeapBlock<unsigned long> data (dataSize);

        int index = 0;
        data[index++] = (unsigned long) newIcon.getWidth();
        data[index++] = (unsigned long) newIcon.getHeight();

        for (int y = 0; y < newIcon.getHeight(); ++y)
            for (int x = 0; x < newIcon.getWidth(); ++x)
                data[index++] = (unsigned long) newIcon.getPixelAt (x, y).getARGB();

        ScopedXLock xlock (display);
        xchangeProperty (windowH, Atoms::getCreating (display, "_NET_WM_ICON"), XA_CARDINAL, 32, data.getData(), dataSize);

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
        ScopedXLock xlock (display);

        if (auto* wmHints = XGetWMHints (display, windowH))
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
                if (XSHMHelpers::isShmAvailable (display))
                {
                    ScopedXLock xlock (display);
                    if (event.xany.type == XShmGetEventBase (display))
                        repainter->notifyPaintCompleted();
                }
               #endif
                break;
        }
    }

    void handleKeyPressEvent (XKeyEvent& keyEvent)
    {
        auto oldMods = ModifierKeys::currentModifiers;

        char utf8 [64] = { 0 };
        juce_wchar unicodeChar = 0;
        int keyCode = 0;
        bool keyDownChange = false;
        KeySym sym;

        {
            ScopedXLock xlock (display);
            updateKeyStates ((int) keyEvent.keycode, true);

            String oldLocale (::setlocale (LC_ALL, 0));
            ::setlocale (LC_ALL, "");
            XLookupString (&keyEvent, utf8, sizeof (utf8), &sym, 0);

            if (oldLocale.isNotEmpty())
                ::setlocale (LC_ALL, oldLocale.toRawUTF8());

            unicodeChar = *CharPointer_UTF8 (utf8);
            keyCode = (int) unicodeChar;

            if (keyCode < 0x20)
                keyCode = (int) XkbKeycodeToKeysym (display, (::KeyCode) keyEvent.keycode, 0, ModifierKeys::currentModifiers.isShiftDown() ? 1 : 0);

            keyDownChange = (sym != NoSymbol) && ! updateKeyModifiersFromSym (sym, true);
        }

        bool keyPressed = false;

        if ((sym & 0xff00) == 0xff00 || keyCode == XK_ISO_Left_Tab)
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

                case XK_ISO_Left_Tab:
                    keyPressed = true;
                    keyCode = XK_Tab & 0xff;
                    break;

                default:
                    if (sym >= XK_F1 && sym <= XK_F35)
                    {
                        keyPressed = true;
                        keyCode = (sym & 0xff) | Keys::extendedKeyModifier;
                    }
                    break;
            }
        }

        if (utf8[0] != 0 || ((sym & 0xff00) == 0 && sym >= 8))
            keyPressed = true;

        if (oldMods != ModifierKeys::currentModifiers)
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
            updateKeyStates ((int) keyEvent.keycode, false);
            KeySym sym;

            {
                ScopedXLock xlock (display);
                sym = XkbKeycodeToKeysym (display, (::KeyCode) keyEvent.keycode, 0, 0);
            }

            auto oldMods = ModifierKeys::currentModifiers;
            const bool keyDownChange = (sym != NoSymbol) && ! updateKeyModifiersFromSym (sym, false);

            if (oldMods != ModifierKeys::currentModifiers)
                handleModifierKeysChange();

            if (keyDownChange)
                handleKeyUpOrDown (false);
        }
    }

    template <typename EventType>
    Point<float> getMousePos (const EventType& e) noexcept
    {
        return Point<float> ((float) e.x, (float) e.y) / currentScaleFactor;
    }

    void handleWheelEvent (const XButtonPressedEvent& buttonPressEvent, float amount)
    {
        MouseWheelDetails wheel;
        wheel.deltaX = 0.0f;
        wheel.deltaY = amount;
        wheel.isReversed = false;
        wheel.isSmooth = false;
        wheel.isInertial = false;

        handleMouseWheel (MouseInputSource::InputSourceType::mouse, getMousePos (buttonPressEvent),
                          getEventTime (buttonPressEvent), wheel);
    }

    void handleButtonPressEvent (const XButtonPressedEvent& buttonPressEvent, int buttonModifierFlag)
    {
        ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withFlags (buttonModifierFlag);
        toFront (true);
        handleMouseEvent (MouseInputSource::InputSourceType::mouse, getMousePos (buttonPressEvent), ModifierKeys::currentModifiers,
                          MouseInputSource::invalidPressure, MouseInputSource::invalidOrientation, getEventTime (buttonPressEvent), {});
    }

    void handleButtonPressEvent (const XButtonPressedEvent& buttonPressEvent)
    {
        updateKeyModifiers ((int) buttonPressEvent.state);

        auto mapIndex = (uint32) (buttonPressEvent.button - Button1);

        if (mapIndex < (uint32) numElementsInArray (pointerMap))
        {
            switch (pointerMap[mapIndex])
            {
                case Keys::WheelUp:         handleWheelEvent (buttonPressEvent,  50.0f / 256.0f); break;
                case Keys::WheelDown:       handleWheelEvent (buttonPressEvent, -50.0f / 256.0f); break;
                case Keys::LeftButton:      handleButtonPressEvent (buttonPressEvent, ModifierKeys::leftButtonModifier); break;
                case Keys::RightButton:     handleButtonPressEvent (buttonPressEvent, ModifierKeys::rightButtonModifier); break;
                case Keys::MiddleButton:    handleButtonPressEvent (buttonPressEvent, ModifierKeys::middleButtonModifier); break;
                default: break;
            }
        }

        clearLastMousePos();
    }

    void handleButtonReleaseEvent (const XButtonReleasedEvent& buttonRelEvent)
    {
        updateKeyModifiers ((int) buttonRelEvent.state);

        if (parentWindow != 0)
            updateWindowBounds();

        auto mapIndex = (uint32) (buttonRelEvent.button - Button1);

        if (mapIndex < (uint32) numElementsInArray (pointerMap))
        {
            switch (pointerMap[mapIndex])
            {
                case Keys::LeftButton:      ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutFlags (ModifierKeys::leftButtonModifier); break;
                case Keys::RightButton:     ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutFlags (ModifierKeys::rightButtonModifier); break;
                case Keys::MiddleButton:    ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutFlags (ModifierKeys::middleButtonModifier); break;
                default: break;
            }
        }

        if (dragState->dragging)
            handleExternalDragButtonReleaseEvent();

        handleMouseEvent (MouseInputSource::InputSourceType::mouse, getMousePos (buttonRelEvent), ModifierKeys::currentModifiers,
                          MouseInputSource::invalidPressure, MouseInputSource::invalidOrientation, getEventTime (buttonRelEvent));

        clearLastMousePos();
    }

    void handleMotionNotifyEvent (const XPointerMovedEvent& movedEvent)
    {
        updateKeyModifiers ((int) movedEvent.state);

        lastMousePos = Point<int> (movedEvent.x_root, movedEvent.y_root);

        if (dragState->dragging)
            handleExternalDragMotionNotify();

        handleMouseEvent (MouseInputSource::InputSourceType::mouse, getMousePos (movedEvent), ModifierKeys::currentModifiers,
                          MouseInputSource::invalidPressure, MouseInputSource::invalidOrientation, getEventTime (movedEvent));
    }

    void handleEnterNotifyEvent (const XEnterWindowEvent& enterEvent)
    {
        if (parentWindow != 0)
            updateWindowBounds();

        clearLastMousePos();

        if (! ModifierKeys::currentModifiers.isAnyMouseButtonDown())
        {
            updateKeyModifiers ((int) enterEvent.state);
            handleMouseEvent (MouseInputSource::InputSourceType::mouse, getMousePos (enterEvent), ModifierKeys::currentModifiers,
                              MouseInputSource::invalidPressure, MouseInputSource::invalidOrientation, getEventTime (enterEvent));
        }
    }

    void handleLeaveNotifyEvent (const XLeaveWindowEvent& leaveEvent)
    {
        // Suppress the normal leave if we've got a pointer grab, or if
        // it's a bogus one caused by clicking a mouse button when running
        // in a Window manager
        if (((! ModifierKeys::currentModifiers.isAnyMouseButtonDown()) && leaveEvent.mode == NotifyNormal)
             || leaveEvent.mode == NotifyUngrab)
        {
            updateKeyModifiers ((int) leaveEvent.state);
            handleMouseEvent (MouseInputSource::InputSourceType::mouse, getMousePos (leaveEvent), ModifierKeys::currentModifiers,
                              MouseInputSource::invalidPressure, MouseInputSource::invalidOrientation, getEventTime (leaveEvent));
        }
    }

    void handleFocusInEvent()
    {
        isActiveApplication = true;

        if (isFocused() && ! focused)
        {
            focused = true;
            handleFocusGain();
        }
    }

    void handleFocusOutEvent()
    {
        if (! isFocused() && focused)
        {
            focused = false;
            isActiveApplication = false;

            handleFocusLoss();
        }
    }

    void handleExposeEvent (XExposeEvent& exposeEvent)
    {
        // Batch together all pending expose events
        XEvent nextEvent;
        ScopedXLock xlock (display);

        // if we have opengl contexts then just repaint them all
        // regardless if this is really necessary
        repaintOpenGLContexts();

        if (exposeEvent.window != windowH)
        {
            Window child;
            XTranslateCoordinates (display, exposeEvent.window, windowH,
                                   exposeEvent.x, exposeEvent.y, &exposeEvent.x, &exposeEvent.y,
                                   &child);
        }

        // exposeEvent is in local window local coordinates so do not convert with
        // physicalToScaled, but rather use currentScaleFactor
        repaint (Rectangle<int> (exposeEvent.x, exposeEvent.y,
                                 exposeEvent.width, exposeEvent.height) / currentScaleFactor);

        while (XEventsQueued (display, QueuedAfterFlush) > 0)
        {
            XPeekEvent (display, &nextEvent);

            if (nextEvent.type != Expose || nextEvent.xany.window != exposeEvent.window)
                break;

            XNextEvent (display, &nextEvent);
            auto& nextExposeEvent = (const XExposeEvent&) nextEvent.xexpose;
            repaint (Rectangle<int> (nextExposeEvent.x, nextExposeEvent.y,
                                     nextExposeEvent.width, nextExposeEvent.height) / currentScaleFactor);
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
            if (auto* currentModalComp = Component::getCurrentlyModalComponent())
                currentModalComp->inputAttemptWhenModal();
        }

        if (confEvent.window == windowH && confEvent.above != 0 && isFrontWindow())
            handleBroughtToFront();
    }

    void handleReparentNotifyEvent()
    {
        parentWindow = 0;
        Window wRoot = 0;
        Window* wChild = nullptr;
        unsigned int numChildren;

        {
            ScopedXLock xlock (display);
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
            ScopedXLock xlock (display);
            XRefreshKeyboardMapping (&mappingEvent);
            updateModifierMappings();
        }
    }

    void handleClientMessageEvent (XClientMessageEvent& clientMsg, XEvent& event)
    {
        if (clientMsg.message_type == atoms->protocols && clientMsg.format == 32)
        {
            auto atom = (Atom) clientMsg.data.l[0];

            if (atom == atoms->protocolList [Atoms::PING])
            {
                Window root = RootWindow (display, DefaultScreen (display));

                clientMsg.window = root;

                XSendEvent (display, root, False, NoEventMask, &event);
                XFlush (display);
            }
            else if (atom == atoms->protocolList [Atoms::TAKE_FOCUS])
            {
                if ((getStyleFlags() & juce::ComponentPeer::windowIgnoresKeyPresses) == 0)
                {
                    XWindowAttributes atts;

                    ScopedXLock xlock (display);
                    if (clientMsg.window != 0
                         && XGetWindowAttributes (display, clientMsg.window, &atts))
                    {
                        if (atts.map_state == IsViewable)
                            XSetInputFocus (display,
                                            (clientMsg.window == windowH ? getFocusWindow()
                                                                         : clientMsg.window),
                                            RevertToParent,
                                            (::Time) clientMsg.data.l[1]);
                    }
                }
            }
            else if (atom == atoms->protocolList [Atoms::DELETE_WINDOW])
            {
                handleUserClosingWindow();
            }
        }
        else if (clientMsg.message_type == atoms->XdndEnter)
        {
            handleDragAndDropEnter (clientMsg);
        }
        else if (clientMsg.message_type == atoms->XdndLeave)
        {
            handleDragExit (dragInfo);
            resetDragAndDrop();
        }
        else if (clientMsg.message_type == atoms->XdndPosition)
        {
            handleDragAndDropPosition (clientMsg);
        }
        else if (clientMsg.message_type == atoms->XdndDrop)
        {
            handleDragAndDropDrop (clientMsg);
        }
        else if (clientMsg.message_type == atoms->XdndStatus)
        {
            handleExternalDragAndDropStatus (clientMsg);
        }
        else if (clientMsg.message_type == atoms->XdndFinished)
        {
            externalResetDragAndDrop();
        }
    }

    bool externalDragTextInit (const String& text, std::function<void()> cb)
    {
        if (dragState->dragging)
            return false;

        return externalDragInit (true, text, cb);
    }

    bool externalDragFileInit (const StringArray& files, bool /*canMoveFiles*/, std::function<void()> cb)
    {
        if (dragState->dragging)
            return false;

        StringArray uriList;

        for (auto& f : files)
        {
            if (f.matchesWildcard ("?*://*", false))
                uriList.add (f);
            else
                uriList.add ("file://" + f);
        }

        return externalDragInit (false, uriList.joinIntoString ("\r\n"), cb);
    }

    //==============================================================================
    void showMouseCursor (Cursor cursor) noexcept
    {
        ScopedXLock xlock (display);
        XDefineCursor (display, windowH, cursor);
    }

    //==============================================================================
    double getCurrentScale() noexcept
    {
        return currentScaleFactor;
    }

    //==============================================================================
    void addOpenGLRepaintListener (Component* dummy)
    {
        if (dummy != nullptr)
            glRepaintListeners.addIfNotAlreadyThere (dummy);
    }

    void removeOpenGLRepaintListener (Component* dummy)
    {
        if (dummy != nullptr)
            glRepaintListeners.removeAllInstancesOf (dummy);
    }

    void repaintOpenGLContexts()
    {
        for (int i = 0; i < glRepaintListeners.size(); ++i)
            if (auto* c = glRepaintListeners [i])
                c->handleCommandMessage (0);
    }

    //==============================================================================
    unsigned long createKeyProxy()
    {
        jassert (keyProxy == 0 && windowH != 0);

        if (keyProxy == 0 && windowH != 0)
        {
            XSetWindowAttributes swa;
            swa.event_mask = KeyPressMask | KeyReleaseMask | FocusChangeMask;

            keyProxy = XCreateWindow (display, windowH,
                                      -1, -1, 1, 1, 0, 0,
                                      InputOnly, CopyFromParent,
                                      CWEventMask,
                                      &swa);

            XMapWindow (display, keyProxy);
            XSaveContext (display, (XID) keyProxy, windowHandleXContext, (XPointer) this);
        }

        return keyProxy;
    }

    void deleteKeyProxy()
    {
        jassert (keyProxy != 0);

        if (keyProxy != 0)
        {
            XPointer handlePointer;

            if (! XFindContext (display, (XID) keyProxy, windowHandleXContext, &handlePointer))
                XDeleteContext (display, (XID) keyProxy, windowHandleXContext);

            XDestroyWindow (display, keyProxy);
            XSync (display, false);

            XEvent event;
            while (XCheckWindowEvent (display, keyProxy, getAllEventsMask(), &event) == True)
            {}

            keyProxy = 0;
        }
    }

    //==============================================================================
    bool dontRepaint;
    static bool isActiveApplication;

private:
    //==============================================================================
    class LinuxRepaintManager   : public Timer
    {
    public:
        LinuxRepaintManager (LinuxComponentPeer& p, ::Display* d)
            : peer (p), display (d)
        {
           #if JUCE_USE_XSHM
            useARGBImagesForRendering = XSHMHelpers::isShmAvailable (display);

            if (useARGBImagesForRendering)
            {
                ScopedXLock xlock (display);
                XShmSegmentInfo segmentinfo;

                auto testImage = XShmCreateImage (display, DefaultVisual (display, DefaultScreen (display)),
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
                image = Image();
            }
        }

        void repaint (Rectangle<int> area)
        {
            if (! isTimerRunning())
                startTimer (repaintTimerPeriod);

            regionsNeedingRepaint.add (area * peer.currentScaleFactor);
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

            auto originalRepaintRegion = regionsNeedingRepaint;
            regionsNeedingRepaint.clear();
            auto totalArea = originalRepaintRegion.getBounds();

            if (! totalArea.isEmpty())
            {
                if (image.isNull() || image.getWidth() < totalArea.getWidth()
                     || image.getHeight() < totalArea.getHeight())
                {
                   #if JUCE_USE_XSHM
                    image = Image (new XBitmapImage (display, useARGBImagesForRendering ? Image::ARGB
                                                                                        : Image::RGB,
                   #else
                    image = Image (new XBitmapImage (display, Image::RGB,
                   #endif
                                                     (totalArea.getWidth()  + 31) & ~31,
                                                     (totalArea.getHeight() + 31) & ~31,
                                                     false, (unsigned int) peer.depth, peer.visual));
                }

                startTimer (repaintTimerPeriod);

                RectangleList<int> adjustedList (originalRepaintRegion);
                adjustedList.offsetAll (-totalArea.getX(), -totalArea.getY());

                if (peer.depth == 32)
                    for (auto& i : originalRepaintRegion)
                        image.clear (i - totalArea.getPosition());

                {
                    std::unique_ptr<LowLevelGraphicsContext> context (peer.getComponent().getLookAndFeel()
                                                                          .createGraphicsContext (image, -totalArea.getPosition(), adjustedList));
                    context->addTransform (AffineTransform::scale ((float) peer.currentScaleFactor));
                    peer.handlePaint (*context);
                }

                for (auto& i : originalRepaintRegion)
                {
                    auto* xbitmap = static_cast<XBitmapImage*> (image.getPixelData());

                   #if JUCE_USE_XSHM
                    if (xbitmap->isUsingXShm())
                        ++shmPaintsPending;
                   #endif


                   xbitmap->blitToWindow (peer.windowH,
                                          i.getX(), i.getY(),
                                          (unsigned int) i.getWidth(),
                                          (unsigned int) i.getHeight(),
                                          i.getX() - totalArea.getX(), i.getY() - totalArea.getY());
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
        uint32 lastTimeImageUsed = 0;
        RectangleList<int> regionsNeedingRepaint;
        ::Display* display;

       #if JUCE_USE_XSHM
        bool useARGBImagesForRendering;
        int shmPaintsPending = 0;
       #endif
        JUCE_DECLARE_NON_COPYABLE (LinuxRepaintManager)
    };

    std::unique_ptr<Atoms> atoms;
    std::unique_ptr<LinuxRepaintManager> repainter;

    friend class LinuxRepaintManager;
    Window windowH = {}, parentWindow = {}, keyProxy = {};
    Rectangle<int> bounds;
    Image taskbarImage;
    bool fullScreen = false, mapped = false, focused = false;
    Visual* visual = {};
    int depth = 0;
    BorderSize<int> windowBorder;
    bool isAlwaysOnTop;
    double currentScaleFactor = 1.0;
    Array<Component*> glRepaintListeners;
    enum { KeyPressEventType = 2 };
    static ::Display* display;

    struct MotifWmHints
    {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long input_mode;
        unsigned long status;
    };

    static void updateKeyStates (int keycode, bool press) noexcept
    {
        const int keybyte = keycode >> 3;
        const int keybit = (1 << (keycode & 7));

        if (press)
            Keys::keyStates [keybyte] |= keybit;
        else
            Keys::keyStates [keybyte] &= ~keybit;
    }

    static void updateKeyModifiers (int status) noexcept
    {
        int keyMods = 0;

        if ((status & ShiftMask) != 0)     keyMods |= ModifierKeys::shiftModifier;
        if ((status & ControlMask) != 0)   keyMods |= ModifierKeys::ctrlModifier;
        if ((status & Keys::AltMask) != 0) keyMods |= ModifierKeys::altModifier;

        ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withOnlyMouseButtons().withFlags (keyMods);

        Keys::numLock  = ((status & Keys::NumLockMask) != 0);
        Keys::capsLock = ((status & LockMask) != 0);
    }

    static bool updateKeyModifiersFromSym (KeySym sym, bool press) noexcept
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

        ModifierKeys::currentModifiers = press ? ModifierKeys::currentModifiers.withFlags (modifier)
                                               : ModifierKeys::currentModifiers.withoutFlags (modifier);

        return isModifier;
    }

    // Alt and Num lock are not defined by standard X
    // modifier constants: check what they're mapped to
    static void updateModifierMappings() noexcept
    {
        ScopedXLock xlock (display);
        int altLeftCode = XKeysymToKeycode (display, XK_Alt_L);
        int numLockCode = XKeysymToKeycode (display, XK_Num_Lock);

        Keys::AltMask = 0;
        Keys::NumLockMask = 0;

        if (auto* mapping = XGetModifierMapping (display))
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
        Atom hints = Atoms::getIfExists (display, "_MOTIF_WM_HINTS");

        if (hints != None)
        {
            MotifWmHints motifHints;
            zerostruct (motifHints);

            motifHints.flags = 2; /* MWM_HINTS_DECORATIONS */
            motifHints.decorations = 0;

            ScopedXLock xlock (display);
            xchangeProperty (wndH, hints, hints, 32, &motifHints, 4);
        }

        hints = Atoms::getIfExists (display, "_WIN_HINTS");

        if (hints != None)
        {
            long gnomeHints = 0;

            ScopedXLock xlock (display);
            xchangeProperty (wndH, hints, hints, 32, &gnomeHints, 1);
        }

        hints = Atoms::getIfExists (display, "KWM_WIN_DECORATION");

        if (hints != None)
        {
            long kwmHints = 2; /*KDE_tinyDecoration*/

            ScopedXLock xlock (display);
            xchangeProperty (wndH, hints, hints, 32, &kwmHints, 1);
        }

        hints = Atoms::getIfExists (display, "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE");

        if (hints != None)
        {
            ScopedXLock xlock (display);
            xchangeProperty (wndH, atoms->windowType, XA_ATOM, 32, &hints, 1);
        }
    }

    void addWindowButtons (Window wndH)
    {
        ScopedXLock xlock (display);
        Atom hints = Atoms::getIfExists (display, "_MOTIF_WM_HINTS");

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

        hints = Atoms::getIfExists (display, "_NET_WM_ALLOWED_ACTIONS");

        if (hints != None)
        {
            Atom netHints [6];
            int num = 0;

            if ((styleFlags & windowIsResizable) != 0)
                netHints [num++] = Atoms::getIfExists (display, "_NET_WM_ACTION_RESIZE");

            if ((styleFlags & windowHasMaximiseButton) != 0)
                netHints [num++] = Atoms::getIfExists (display, "_NET_WM_ACTION_FULLSCREEN");

            if ((styleFlags & windowHasMinimiseButton) != 0)
                netHints [num++] = Atoms::getIfExists (display, "_NET_WM_ACTION_MINIMIZE");

            if ((styleFlags & windowHasCloseButton) != 0)
                netHints [num++] = Atoms::getIfExists (display, "_NET_WM_ACTION_CLOSE");

            xchangeProperty (wndH, hints, XA_ATOM, 32, &netHints, num);
        }
    }

    void setWindowType()
    {
        Atom netHints [2];

        if ((styleFlags & windowIsTemporary) != 0
             || ((styleFlags & windowHasDropShadow) == 0 && Desktop::canUseSemiTransparentWindows()))
            netHints [0] = Atoms::getIfExists (display, "_NET_WM_WINDOW_TYPE_COMBO");
        else
            netHints [0] = Atoms::getIfExists (display, "_NET_WM_WINDOW_TYPE_NORMAL");

        xchangeProperty (windowH, atoms->windowType, XA_ATOM, 32, &netHints, 1);

        int numHints = 0;

        if ((styleFlags & windowAppearsOnTaskbar) == 0)
            netHints [numHints++] = Atoms::getIfExists (display, "_NET_WM_STATE_SKIP_TASKBAR");

        if (component.isAlwaysOnTop())
            netHints [numHints++] = Atoms::getIfExists (display, "_NET_WM_STATE_ABOVE");

        if (numHints > 0)
            xchangeProperty (windowH, atoms->windowState, XA_ATOM, 32, &netHints, numHints);
    }

    void createWindow (Window parentToAddTo)
    {
        ScopedXLock xlock (display);
        resetDragAndDrop();

        // Get defaults for various properties
        const int screen = DefaultScreen (display);
        Window root = RootWindow (display, screen);

        parentWindow = parentToAddTo;

        // Try to obtain a 32-bit visual or fallback to 24 or 16
        visual = Visuals::findVisualFormat (display, (styleFlags & windowIsSemiTransparent) ? 32 : 24, depth);

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
        swa.override_redirect = ((styleFlags & windowIsTemporary) != 0) ? True : False;
        swa.event_mask = getAllEventsMask();

        windowH = XCreateWindow (display, parentToAddTo != 0 ? parentToAddTo : root,
                                 0, 0, 1, 1,
                                 0, depth, InputOutput, visual,
                                 CWBorderPixel | CWColormap | CWBackPixmap | CWEventMask | CWOverrideRedirect,
                                 &swa);

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

        // Associate the PID, allowing to be shut down when something goes wrong
        unsigned long pid = (unsigned long) getpid();
        xchangeProperty (windowH, atoms->pid, XA_CARDINAL, 32, &pid, 1);

        // Set window manager protocols
        xchangeProperty (windowH, atoms->protocols, XA_ATOM, 32, atoms->protocolList, 2);

        // Set drag and drop flags
        xchangeProperty (windowH, atoms->XdndTypeList, XA_ATOM, 32, atoms->allowedMimeTypes, numElementsInArray (atoms->allowedMimeTypes));
        xchangeProperty (windowH, atoms->XdndActionList, XA_ATOM, 32, atoms->allowedActions, numElementsInArray (atoms->allowedActions));
        xchangeProperty (windowH, atoms->XdndActionDescription, XA_STRING, 8, "", 0);
        xchangeProperty (windowH, atoms->XdndAware, XA_ATOM, 32, &atoms->DndVersion, 1);

        initialisePointerMap();
        updateModifierMappings();
    }

    void destroyWindow()
    {
        ScopedXLock xlock (display);

        XPointer handlePointer;

        if (keyProxy != 0)
            deleteKeyProxy();

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

    int getAllEventsMask() const noexcept
    {
        return NoEventMask | KeyPressMask | KeyReleaseMask
                 | EnterWindowMask | LeaveWindowMask | PointerMotionMask | KeymapStateMask
                 | ExposureMask | StructureNotifyMask | FocusChangeMask
                 | ((styleFlags & windowIgnoresMouseClicks) != 0 ?  0 : (ButtonPressMask | ButtonReleaseMask));
    }

    template <typename EventType>
    static int64 getEventTime (const EventType& t)
    {
        return getEventTime (t.time);
    }

    static int64 getEventTime (::Time t)
    {
        static int64 eventTimeOffset = 0x12345678;
        auto thisMessageTime = (int64) t;

        if (eventTimeOffset == 0x12345678)
            eventTimeOffset = Time::currentTimeMillis() - thisMessageTime;

        return eventTimeOffset + thisMessageTime;
    }

    long getUserTime() const
    {
        GetXProperty prop (display, windowH, atoms->userTime, 0, 65536, false, XA_CARDINAL);
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
            ScopedXLock xlock (display);
            Atom hints = Atoms::getIfExists (display, "_NET_FRAME_EXTENTS");

            if (hints != None)
            {
                GetXProperty prop (display, windowH, hints, 0, 4, false, XA_CARDINAL);

                if (prop.success && prop.actualFormat == 32)
                {
                    auto* sizes = (const unsigned long*) prop.data;

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
            unsigned int ww = 0, wh = 0, bw, bitDepth;

            ScopedXLock xlock (display);

            if (XGetGeometry (display, (::Drawable) windowH, &root, &wx, &wy, &ww, &wh, &bw, &bitDepth))
                if (! XTranslateCoordinates (display, windowH, root, 0, 0, &wx, &wy, &child))
                    wx = wy = 0;

            Rectangle<int> physicalBounds (wx, wy, (int) ww, (int) wh);

            currentScaleFactor =
                DisplayGeometry::getInstance().findDisplayForRect (physicalBounds, false).scale;

            bounds = DisplayGeometry::physicalToScaled (physicalBounds);
        }
    }

    //==============================================================================
    struct DragState
    {
        DragState (::Display* d)
        {
            if (isText)
                allowedTypes.add (Atoms::getCreating (d, "text/plain"));
            else
                allowedTypes.add (Atoms::getCreating (d, "text/uri-list"));
        }

        bool isText = false;
        bool dragging = false;         // currently performing outgoing external dnd as Xdnd source, have grabbed mouse
        bool expectingStatus = false;  // XdndPosition sent, waiting for XdndStatus
        bool canDrop = false;          // target window signals it will accept the drop
        Window targetWindow = None;    // potential drop target
        int xdndVersion = -1;          // negotiated version with target
        Rectangle<int> silentRect;
        String textOrFiles;
        Array<Atom> allowedTypes;
        std::function<void()> completionCallback;
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
        dragState.reset (new DragState (display));
    }

    void sendDragAndDropMessage (XClientMessageEvent& msg)
    {
        msg.type = ClientMessage;
        msg.display = display;
        msg.window = dragAndDropSourceWindow;
        msg.format = 32;
        msg.data.l[0] = (long) windowH;

        ScopedXLock xlock (display);
        XSendEvent (display, dragAndDropSourceWindow, False, 0, (XEvent*) &msg);
    }

    bool sendExternalDragAndDropMessage (XClientMessageEvent& msg, Window targetWindow)
    {
        msg.type      = ClientMessage;
        msg.display   = display;
        msg.window    = targetWindow;
        msg.format    = 32;
        msg.data.l[0] = (long) windowH;

        ScopedXLock xlock (display);
        return XSendEvent (display, targetWindow, False, 0, (XEvent*) &msg) != 0;
    }

    void sendExternalDragAndDropDrop (Window targetWindow)
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = atoms->XdndDrop;
        msg.data.l[2] = CurrentTime;

        sendExternalDragAndDropMessage (msg, targetWindow);
    }

    void sendExternalDragAndDropEnter (Window targetWindow)
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = atoms->XdndEnter;
        msg.data.l[1] = (dragState->xdndVersion << 24);

        for (int i = 0; i < 3; ++i)
            msg.data.l[i + 2] = (long) dragState->allowedTypes[i];

        sendExternalDragAndDropMessage (msg, targetWindow);
    }

    void sendExternalDragAndDropPosition (Window targetWindow)
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = atoms->XdndPosition;

        Point<int> mousePos (Desktop::getInstance().getMousePosition());

        if (dragState->silentRect.contains (mousePos)) // we've been asked to keep silent
            return;

        mousePos = DisplayGeometry::scaledToPhysical (mousePos);
        msg.data.l[1] = 0;
        msg.data.l[2] = (mousePos.x << 16) | mousePos.y;
        msg.data.l[3] = CurrentTime;
        msg.data.l[4] = (long) atoms->XdndActionCopy; // this is all JUCE currently supports

        dragState->expectingStatus = sendExternalDragAndDropMessage (msg, targetWindow);
    }

    void sendDragAndDropStatus (bool acceptDrop, Atom dropAction)
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = atoms->XdndStatus;
        msg.data.l[1] = (acceptDrop ? 1 : 0) | 2; // 2 indicates that we want to receive position messages
        msg.data.l[4] = (long) dropAction;

        sendDragAndDropMessage (msg);
    }

    void sendExternalDragAndDropLeave (Window targetWindow)
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = atoms->XdndLeave;
        sendExternalDragAndDropMessage (msg, targetWindow);
    }

    void sendDragAndDropFinish()
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = atoms->XdndFinished;
        sendDragAndDropMessage (msg);
    }

    void handleExternalSelectionClear()
    {
        if (dragState->dragging)
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

        if (dragState->allowedTypes.contains (targetType))
        {
            s.xselection.property = evt.xselectionrequest.property;

            xchangeProperty (evt.xselectionrequest.requestor,
                             evt.xselectionrequest.property,
                             targetType, 8,
                             dragState->textOrFiles.toRawUTF8(),
                             (int) dragState->textOrFiles.getNumBytesAsUTF8());
        }

        XSendEvent (display, evt.xselectionrequest.requestor, True, 0, &s);
    }

    void handleExternalDragAndDropStatus (const XClientMessageEvent& clientMsg)
    {
        if (dragState->expectingStatus)
        {
            dragState->expectingStatus = false;
            dragState->canDrop = false;
            dragState->silentRect = Rectangle<int>();

            if ((clientMsg.data.l[1] & 1) != 0
                 && ((Atom) clientMsg.data.l[4] == atoms->XdndActionCopy
                      || (Atom) clientMsg.data.l[4] == atoms->XdndActionPrivate))
            {
                if ((clientMsg.data.l[1] & 2) == 0) // target requests silent rectangle
                    dragState->silentRect.setBounds ((int) clientMsg.data.l[2] >> 16,
                                                    (int) clientMsg.data.l[2] & 0xffff,
                                                    (int) clientMsg.data.l[3] >> 16,
                                                    (int) clientMsg.data.l[3] & 0xffff);

                dragState->canDrop = true;
            }
        }
    }

    void handleExternalDragButtonReleaseEvent()
    {
        if (dragState->dragging)
            XUngrabPointer (display, CurrentTime);

        if (dragState->canDrop)
        {
            sendExternalDragAndDropDrop (dragState->targetWindow);
        }
        else
        {
            sendExternalDragAndDropLeave (dragState->targetWindow);
            externalResetDragAndDrop();
        }
    }

    void handleExternalDragMotionNotify()
    {
        Window targetWindow = externalFindDragTargetWindow (RootWindow (display, DefaultScreen (display)));

        if (dragState->targetWindow != targetWindow)
        {
            if (dragState->targetWindow != None)
                sendExternalDragAndDropLeave (dragState->targetWindow);

            dragState->canDrop = false;
            dragState->silentRect = Rectangle<int>();

            if (targetWindow == None)
                return;

            GetXProperty prop (display, targetWindow, atoms->XdndAware,
                               0, 2, false, AnyPropertyType);

            if (prop.success
                 && prop.data != None
                 && prop.actualFormat == 32
                 && prop.numItems == 1)
            {
                dragState->xdndVersion = jmin ((int) prop.data[0], (int) atoms->DndVersion);
            }
            else
            {
                dragState->xdndVersion = -1;
                return;
            }

            sendExternalDragAndDropEnter (targetWindow);
            dragState->targetWindow = targetWindow;
        }

        if (! dragState->expectingStatus)
            sendExternalDragAndDropPosition (targetWindow);
    }

    void handleDragAndDropPosition (const XClientMessageEvent& clientMsg)
    {
        if (dragAndDropSourceWindow == 0)
            return;

        dragAndDropSourceWindow = (::Window) clientMsg.data.l[0];

        Point<int> dropPos ((int) clientMsg.data.l[2] >> 16,
                            (int) clientMsg.data.l[2] & 0xffff);
        dropPos -= bounds.getPosition();

        Atom targetAction = atoms->XdndActionCopy;

        for (int i = numElementsInArray (atoms->allowedActions); --i >= 0;)
        {
            if ((Atom) clientMsg.data.l[4] == atoms->allowedActions[i])
            {
                targetAction = atoms->allowedActions[i];
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
        auto dndCurrentVersion = static_cast<unsigned long> (clientMsg.data.l[1] & 0xff000000) >> 24;

        if (dndCurrentVersion < 3 || dndCurrentVersion > Atoms::DndVersion)
        {
            dragAndDropSourceWindow = 0;
            return;
        }

        dragAndDropSourceWindow = (::Window) clientMsg.data.l[0];

        if ((clientMsg.data.l[1] & 1) != 0)
        {
            ScopedXLock xlock (display);
            GetXProperty prop (display, dragAndDropSourceWindow, atoms->XdndTypeList, 0, 0x8000000L, false, XA_ATOM);

            if (prop.success
                 && prop.actualType == XA_ATOM
                 && prop.actualFormat == 32
                 && prop.numItems != 0)
            {
                auto* types = (const unsigned long*) prop.data;

                for (unsigned long i = 0; i < prop.numItems; ++i)
                    if (types[i] != None)
                        srcMimeTypeAtomList.add (types[i]);
            }
        }

        if (srcMimeTypeAtomList.isEmpty())
        {
            for (int i = 2; i < 5; ++i)
                if (clientMsg.data.l[i] != None)
                    srcMimeTypeAtomList.add ((unsigned long) clientMsg.data.l[i]);

            if (srcMimeTypeAtomList.isEmpty())
            {
                dragAndDropSourceWindow = 0;
                return;
            }
        }

        for (int i = 0; i < srcMimeTypeAtomList.size() && dragAndDropCurrentMimeType == 0; ++i)
            for (int j = 0; j < numElementsInArray (atoms->allowedMimeTypes); ++j)
                if (srcMimeTypeAtomList[i] == atoms->allowedMimeTypes[j])
                    dragAndDropCurrentMimeType = atoms->allowedMimeTypes[j];

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
                    GetXProperty prop (display, evt.xany.window, evt.xselection.property,
                                       dropData.getSize() / 4, 65536, false, AnyPropertyType);

                    if (! prop.success)
                        break;

                    dropData.append (prop.data, prop.numItems * (size_t) prop.actualFormat / 8);

                    if (prop.bytesLeft <= 0)
                        break;
                }

                lines.addLines (dropData.toString());
            }

            if (Atoms::isMimeTypeFile (display, dragAndDropCurrentMimeType))
            {
                for (int i = 0; i < lines.size(); ++i)
                    dragInfo.files.add (URL::removeEscapeChars (lines[i].replace ("file://", String(), true)));

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
            ScopedXLock xlock (display);
            XConvertSelection (display,
                               atoms->XdndSelection,
                               dragAndDropCurrentMimeType,
                               Atoms::getCreating (display, "JXSelectionWindowProperty"),
                               windowH,
                               (::Time) clientMsg.data.l[2]);
        }
    }

    bool isWindowDnDAware (Window w) const
    {
        int numProperties = 0;
        auto* properties = XListProperties (display, w, &numProperties);
        bool dndAwarePropFound = false;

        for (int i = 0; i < numProperties; ++i)
            if (properties[i] == atoms->XdndAware)
                dndAwarePropFound = true;

        if (properties != nullptr)
            XFree (properties);

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

    bool externalDragInit (bool isText, const String& textOrFiles, std::function<void()> cb)
    {
        ScopedXLock xlock (display);

        resetExternalDragState();
        dragState->isText = isText;
        dragState->textOrFiles = textOrFiles;
        dragState->targetWindow = windowH;
        dragState->completionCallback = cb;

        const int pointerGrabMask = Button1MotionMask | ButtonReleaseMask;

        if (XGrabPointer (display, windowH, True, pointerGrabMask,
                          GrabModeAsync, GrabModeAsync, None, None, CurrentTime) == GrabSuccess)
        {
            // No other method of changing the pointer seems to work, this call is needed from this very context
            XChangeActivePointerGrab (display, pointerGrabMask, (Cursor) createDraggingHandCursor(), CurrentTime);

            XSetSelectionOwner (display, atoms->XdndSelection, windowH, CurrentTime);

            // save the available types to XdndTypeList
            xchangeProperty (windowH, atoms->XdndTypeList, XA_ATOM, 32,
                             dragState->allowedTypes.getRawDataPointer(),
                             dragState->allowedTypes.size());

            dragState->dragging = true;
            handleExternalDragMotionNotify();
            return true;
        }

        return false;
    }

    void externalResetDragAndDrop()
    {
        if (dragState->dragging)
        {
            ScopedXLock xlock (display);
            XUngrabPointer (display, CurrentTime);
        }

        if (dragState->completionCallback != nullptr)
            dragState->completionCallback();

        resetExternalDragState();
    }

    std::unique_ptr<DragState> dragState;
    DragInfo dragInfo;
    Atom dragAndDropCurrentMimeType;
    Window dragAndDropSourceWindow;
    bool finishAfterDropDataReceived;

    Array<Atom> srcMimeTypeAtomList;

    int pointerMap[5] = {};

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

bool LinuxComponentPeer::isActiveApplication = false;
Point<int> LinuxComponentPeer::lastMousePos;
::Display* LinuxComponentPeer::display = nullptr;

//==============================================================================
namespace WindowingHelpers
{
    static void windowMessageReceive (XEvent& event)
    {
        if (event.xany.window != None)
        {
           #if JUCE_X11_SUPPORTS_XEMBED
            if (! juce_handleXEmbedEvent (nullptr, &event))
           #endif
            {
                if (auto* peer = LinuxComponentPeer::getPeerFor (event.xany.window))
                    peer->handleWindowMessage (event);
            }
        }
        else if (event.xany.type == KeymapNotify)
        {
            auto& keymapEvent = (const XKeymapEvent&) event.xkeymap;
            memcpy (Keys::keyStates, keymapEvent.key_vector, 32);
        }
    }
}

struct WindowingCallbackInitialiser
{
    WindowingCallbackInitialiser()
    {
        dispatchWindowMessage = WindowingHelpers::windowMessageReceive;
    }
};

static WindowingCallbackInitialiser windowingInitialiser;

//==============================================================================
JUCE_API bool JUCE_CALLTYPE Process::isForegroundProcess()
{
    return LinuxComponentPeer::isActiveApplication;
}

// N/A on Linux as far as I know.
JUCE_API void JUCE_CALLTYPE Process::makeForegroundProcess() {}
JUCE_API void JUCE_CALLTYPE Process::hide() {}

//==============================================================================
void Desktop::setKioskComponent (Component* comp, bool enableOrDisable, bool /* allowMenusAndBars */)
{
    if (enableOrDisable)
        comp->setBounds (getDisplays().getMainDisplay().totalArea);
}

void Desktop::allowedOrientationsChanged() {}

//==============================================================================
ComponentPeer* Component::createNewPeer (int styleFlags, void* nativeWindowToAttachTo)
{
    return new LinuxComponentPeer (*this, styleFlags, (Window) nativeWindowToAttachTo);
}

//==============================================================================
void Desktop::Displays::findDisplays (float masterScale)
{
    ScopedXDisplay xDisplay;

    if (auto display = xDisplay.display)
    {
        auto& geometry = DisplayGeometry::getOrCreateInstance (display, masterScale);

        // add the main display first
        int mainDisplayIdx;

        for (mainDisplayIdx = 0; mainDisplayIdx < geometry.infos.size(); ++mainDisplayIdx)
        {
            auto& info = geometry.infos.getReference (mainDisplayIdx);

            if (info.isMain)
                break;
        }

        // no main display found then use the first
        if (mainDisplayIdx >= geometry.infos.size())
            mainDisplayIdx = 0;

        // add the main display
        {
            auto& info = geometry.infos.getReference (mainDisplayIdx);

            Desktop::Displays::Display d;
            d.isMain = true;
            d.scale = masterScale * info.scale;
            d.dpi = info.dpi;
            d.totalArea = DisplayGeometry::physicalToScaled (info.totalBounds);
            d.userArea = (info.usableBounds / d.scale) + info.topLeftScaled;

            displays.add (d);
        }

        for (int i = 0; i < geometry.infos.size(); ++i)
        {
            // don't add the main display a second time
            if (i == mainDisplayIdx)
                continue;

            auto& info = geometry.infos.getReference (i);

            Desktop::Displays::Display d;
            d.isMain = false;
            d.scale = masterScale * info.scale;
            d.dpi = info.dpi;
            d.totalArea = DisplayGeometry::physicalToScaled (info.totalBounds);
            d.userArea = (info.usableBounds / d.scale) + info.topLeftScaled;

            displays.add (d);
        }
    }
}

//==============================================================================
bool MouseInputSource::SourceList::addSource()
{
    if (sources.isEmpty())
    {
        addSource (0, MouseInputSource::InputSourceType::mouse);
        return true;
    }

    return false;
}

bool MouseInputSource::SourceList::canUseTouch()
{
    return false;
}

bool Desktop::canUseSemiTransparentWindows() noexcept
{
   #if JUCE_USE_XRENDER
    if (XRender::hasCompositingWindowManager())
    {
        int matchedDepth = 0, desiredDepth = 32;

        return Visuals::findVisualFormat (display, desiredDepth, matchedDepth) != 0
                 && matchedDepth == desiredDepth;
    }
   #endif

    return false;
}

Point<float> MouseInputSource::getCurrentRawMousePosition()
{
    ScopedXDisplay xDisplay;
    auto display = xDisplay.display;

    if (display == nullptr)
        return {};

    Window root, child;
    int x, y, winx, winy;
    unsigned int mask;

    ScopedXLock xlock (display);

    if (XQueryPointer (display,
                       RootWindow (display, DefaultScreen (display)),
                       &root, &child,
                       &x, &y, &winx, &winy, &mask) == False)
    {
        // Pointer not on the default screen
        x = y = -1;
    }

    return DisplayGeometry::physicalToScaled (Point<float> ((float) x, (float) y));
}

void MouseInputSource::setRawMousePosition (Point<float> newPosition)
{
    ScopedXDisplay xDisplay;

    if (auto display = xDisplay.display)
    {
        ScopedXLock xlock (display);
        Window root = RootWindow (display, DefaultScreen (display));
        newPosition = DisplayGeometry::scaledToPhysical (newPosition);
        XWarpPointer (display, None, root, 0, 0, 0, 0, roundToInt (newPosition.getX()), roundToInt (newPosition.getY()));
    }
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

void Desktop::setScreenSaverEnabled (bool isEnabled)
{
    if (screenSaverAllowed != isEnabled)
    {
        screenSaverAllowed = isEnabled;

        ScopedXDisplay xDisplay;

        if (auto display = xDisplay.display)
        {
            typedef void (*tXScreenSaverSuspend) (Display*, Bool);
            static tXScreenSaverSuspend xScreenSaverSuspend = nullptr;

            if (xScreenSaverSuspend == nullptr)
                if (void* h = dlopen ("libXss.so.1", RTLD_GLOBAL | RTLD_NOW))
                    xScreenSaverSuspend = (tXScreenSaverSuspend) dlsym (h, "XScreenSaverSuspend");

            ScopedXLock xlock (display);
            if (xScreenSaverSuspend != nullptr)
                xScreenSaverSuspend (display, ! isEnabled);
        }
    }
}

bool Desktop::isScreenSaverEnabled()
{
    return screenSaverAllowed;
}

//==============================================================================
Image juce_createIconForFile (const File& /* file */)
{
    return {};
}

//==============================================================================
void LookAndFeel::playAlertSound()
{
    std::cout << "\a" << std::flush;
}

//==============================================================================
Rectangle<int> juce_LinuxScaledToPhysicalBounds (ComponentPeer* peer, Rectangle<int> bounds)
{
    if (auto* linuxPeer = dynamic_cast<LinuxComponentPeer*> (peer))
        bounds *= linuxPeer->getCurrentScale();

    return bounds;
}

void juce_LinuxAddRepaintListener (ComponentPeer* peer, Component* dummy)
{
    if (auto* linuxPeer = dynamic_cast<LinuxComponentPeer*> (peer))
        linuxPeer->addOpenGLRepaintListener (dummy);
}

void juce_LinuxRemoveRepaintListener (ComponentPeer* peer, Component* dummy)
{
    if (auto* linuxPeer = dynamic_cast<LinuxComponentPeer*> (peer))
        linuxPeer->removeOpenGLRepaintListener (dummy);
}

unsigned long juce_createKeyProxyWindow (ComponentPeer* peer)
{
    if (auto* linuxPeer = dynamic_cast<LinuxComponentPeer*> (peer))
        return linuxPeer->createKeyProxy();

    return 0;
}

void juce_deleteKeyProxyWindow (ComponentPeer* peer)
{
    if (auto* linuxPeer = dynamic_cast<LinuxComponentPeer*> (peer))
        linuxPeer->deleteKeyProxy();
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
    AlertWindow::showMessageBoxAsync (iconType, title, message, String(), associatedComponent, callback);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (AlertWindow::AlertIconType iconType,
                                                      const String& title, const String& message,
                                                      Component* associatedComponent,
                                                      ModalComponentManager::Callback* callback)
{
    return AlertWindow::showOkCancelBox (iconType, title, message, String(), String(),
                                         associatedComponent, callback);
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoCancelBox (AlertWindow::AlertIconType iconType,
                                                        const String& title, const String& message,
                                                        Component* associatedComponent,
                                                        ModalComponentManager::Callback* callback)
{
    return AlertWindow::showYesNoCancelBox (iconType, title, message,
                                            String(), String(), String(),
                                            associatedComponent, callback);
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoBox (AlertWindow::AlertIconType iconType,
                                                  const String& title, const String& message,
                                                  Component* associatedComponent,
                                                  ModalComponentManager::Callback* callback)
{
    return AlertWindow::showOkCancelBox (iconType, title, message, TRANS ("Yes"), TRANS ("No"),
                                         associatedComponent, callback);
}

//============================== X11 - MouseCursor =============================

void* CustomMouseCursorInfo::create() const
{
    ScopedXDisplay xDisplay;
    auto display = xDisplay.display;

    if (display == nullptr)
        return nullptr;

    ScopedXLock xlock (display);
    auto imageW = (unsigned int) image.getWidth();
    auto imageH = (unsigned int) image.getHeight();
    int hotspotX = hotspot.x;
    int hotspotY = hotspot.y;

  #if JUCE_USE_XCURSOR
    {
        using tXcursorSupportsARGB    = XcursorBool (*) (Display*);
        using tXcursorImageCreate     = XcursorImage* (*) (int, int);
        using tXcursorImageDestroy    = void (*) (XcursorImage*);
        using tXcursorImageLoadCursor = Cursor (*) (Display*, const XcursorImage*);

        static tXcursorSupportsARGB    xcursorSupportsARGB    = nullptr;
        static tXcursorImageCreate     xcursorImageCreate     = nullptr;
        static tXcursorImageDestroy    xcursorImageDestroy    = nullptr;
        static tXcursorImageLoadCursor xcursorImageLoadCursor = nullptr;
        static bool hasBeenLoaded = false;

        if (! hasBeenLoaded)
        {
            hasBeenLoaded = true;

            if (void* h = dlopen ("libXcursor.so.1", RTLD_GLOBAL | RTLD_NOW))
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
            if (XcursorImage* xcImage = xcursorImageCreate ((int) imageW, (int) imageH))
            {
                xcImage->xhot = (XcursorDim) hotspotX;
                xcImage->yhot = (XcursorDim) hotspotY;
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

    Image im (Image::ARGB, (int) cursorW, (int) cursorH, true);

    {
        Graphics g (im);

        if (imageW > cursorW || imageH > cursorH)
        {
            hotspotX = (hotspotX * (int) cursorW) / (int) imageW;
            hotspotY = (hotspotY * (int) cursorH) / (int) imageH;

            g.drawImage (image, Rectangle<float> ((float) imageW, (float) imageH),
                         RectanglePlacement::xLeft | RectanglePlacement::yTop | RectanglePlacement::onlyReduceInSize);
        }
        else
        {
            g.drawImageAt (image, 0, 0);
        }
    }

    const unsigned int stride = (cursorW + 7) >> 3;
    HeapBlock<char> maskPlane, sourcePlane;
    maskPlane.calloc (stride * cursorH);
    sourcePlane.calloc (stride * cursorH);

    const bool msbfirst = (BitmapBitOrder (display) == MSBFirst);

    for (int y = (int) cursorH; --y >= 0;)
    {
        for (int x = (int) cursorW; --x >= 0;)
        {
            auto mask = (char) (1 << (msbfirst ? (7 - (x & 7)) : (x & 7)));
            auto offset = (unsigned int) y * stride + ((unsigned int) x >> 3);

            auto c = im.getPixelAt (x, y);

            if (c.getAlpha() >= 128)        maskPlane[offset]   |= mask;
            if (c.getBrightness() >= 0.5f)  sourcePlane[offset] |= mask;
        }
    }

    Pixmap sourcePixmap = XCreatePixmapFromBitmapData (display, root, sourcePlane.getData(), cursorW, cursorH, 0xffff, 0, 1);
    Pixmap maskPixmap   = XCreatePixmapFromBitmapData (display, root, maskPlane.getData(),   cursorW, cursorH, 0xffff, 0, 1);

    XColor white, black;
    black.red = black.green = black.blue = 0;
    white.red = white.green = white.blue = 0xffff;

    void* result = (void*) XCreatePixmapCursor (display, sourcePixmap, maskPixmap, &white, &black,
                                                (unsigned int) hotspotX, (unsigned int) hotspotY);

    XFreePixmap (display, sourcePixmap);
    XFreePixmap (display, maskPixmap);

    return result;
}

void MouseCursor::deleteMouseCursor (void* cursorHandle, bool)
{
    if (cursorHandle != nullptr)
    {
        ScopedXDisplay xDisplay;

        if (auto display = xDisplay.display)
        {
            ScopedXLock xlock (display);
            XFreeCursor (display, (Cursor) cursorHandle);
        }
    }
}

void* MouseCursor::createStandardMouseCursor (MouseCursor::StandardCursorType type)
{
    ScopedXDisplay xDisplay;
    auto display = xDisplay.display;

    if (display == nullptr)
        return None;

    unsigned int shape;

    switch (type)
    {
        case NormalCursor:
        case ParentCursor:                  return None; // Use parent cursor
        case NoCursor:                      return CustomMouseCursorInfo (Image (Image::ARGB, 16, 16, true), {}).create();

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

            return CustomMouseCursorInfo (ImageFileFormat::loadFrom (copyCursorData, copyCursorSize), { 1, 3 }).create();
        }

        default:
            jassertfalse;
            return None;
    }

    ScopedXLock xlock (display);
    return (void*) XCreateFontCursor (display, shape);
}

void MouseCursor::showInWindow (ComponentPeer* peer) const
{
    if (auto* lp = dynamic_cast<LinuxComponentPeer*> (peer))
        lp->showMouseCursor ((Cursor) getHandle());
}

void MouseCursor::showInAllWindows() const
{
    for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
        showInWindow (ComponentPeer::getPeer (i));
}

//=================================== X11 - DND ================================
static LinuxComponentPeer* getPeerForDragEvent (Component* sourceComp)
{
    if (sourceComp == nullptr)
        if (auto* draggingSource = Desktop::getInstance().getDraggingMouseSource(0))
            sourceComp = draggingSource->getComponentUnderMouse();

    if (sourceComp != nullptr)
        if (auto* lp = dynamic_cast<LinuxComponentPeer*> (sourceComp->getPeer()))
            return lp;

    jassertfalse;  // This method must be called in response to a component's mouseDown or mouseDrag event!
    return nullptr;
}

bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, bool canMoveFiles,
                                                           Component* sourceComp, std::function<void()> callback)
{
    if (files.isEmpty())
        return false;

    if (auto* lp = getPeerForDragEvent (sourceComp))
        return lp->externalDragFileInit (files, canMoveFiles, callback);

    // This method must be called in response to a component's mouseDown or mouseDrag event!
    jassertfalse;
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text, Component* sourceComp,
                                                          std::function<void()> callback)
{
    if (text.isEmpty())
        return false;

    if (auto* lp = getPeerForDragEvent (sourceComp))
        return lp->externalDragTextInit (text, callback);

    // This method must be called in response to a component's mouseDown or mouseDrag event!
    jassertfalse;
    return false;
}

} // namespace juce
