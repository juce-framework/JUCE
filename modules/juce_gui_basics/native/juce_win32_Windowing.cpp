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

extern void juce_repeatLastProcessPriority();
extern void juce_checkCurrentlyFocusedTopLevelWindow();  // in juce_TopLevelWindow.cpp
extern bool juce_isRunningInWine();

typedef bool (*CheckEventBlockedByModalComps) (const MSG&);
extern CheckEventBlockedByModalComps isEventBlockedByModalComps;

static bool shouldDeactivateTitleBar = true;

extern void* getUser32Function (const char*);

//==============================================================================
typedef BOOL (WINAPI* UpdateLayeredWinFunc) (HWND, HDC, POINT*, SIZE*, HDC, POINT*, COLORREF, BLENDFUNCTION*, DWORD);
static UpdateLayeredWinFunc updateLayeredWindow = nullptr;

bool Desktop::canUseSemiTransparentWindows() noexcept
{
    if (updateLayeredWindow == nullptr && ! juce_isRunningInWine())
        updateLayeredWindow = (UpdateLayeredWinFunc) getUser32Function ("UpdateLayeredWindow");

    return updateLayeredWindow != nullptr;
}

//==============================================================================
#ifndef WM_TOUCH
 #define WM_TOUCH 0x0240
 #define TOUCH_COORD_TO_PIXEL(l)  ((l) / 100)
 #define TOUCHEVENTF_MOVE    0x0001
 #define TOUCHEVENTF_DOWN    0x0002
 #define TOUCHEVENTF_UP      0x0004
 #define TOUCHEVENTF_PRIMARY 0x0010
 DECLARE_HANDLE (HTOUCHINPUT);
 DECLARE_HANDLE (HGESTUREINFO);

 struct TOUCHINPUT
 {
    LONG x;
    LONG y;
    HANDLE hSource;
    DWORD dwID;
    DWORD dwFlags;
    DWORD dwMask;
    DWORD dwTime;
    ULONG_PTR dwExtraInfo;
    DWORD cxContact;
    DWORD cyContact;
 };

 struct GESTUREINFO
 {
    UINT cbSize;
    DWORD dwFlags;
    DWORD dwID;
    HWND hwndTarget;
    POINTS ptsLocation;
    DWORD dwInstanceID;
    DWORD dwSequenceID;
    ULONGLONG ullArguments;
    UINT cbExtraArgs;
 };

#endif

#ifndef MONITOR_DPI_TYPE
  enum Monitor_DPI_Type
  {
    MDT_Effective_DPI  = 0,
    MDT_Angular_DPI    = 1,
    MDT_Raw_DPI        = 2,
    MDT_Default        = MDT_Effective_DPI
  };

  enum Process_DPI_Awareness
  {
    Process_DPI_Unaware            = 0,
    Process_System_DPI_Aware       = 1,
    Process_Per_Monitor_DPI_Aware  = 2
  };
#endif

typedef BOOL (WINAPI* RegisterTouchWindowFunc) (HWND, ULONG);
typedef BOOL (WINAPI* GetTouchInputInfoFunc) (HTOUCHINPUT, UINT, TOUCHINPUT*, int);
typedef BOOL (WINAPI* CloseTouchInputHandleFunc) (HTOUCHINPUT);
typedef BOOL (WINAPI* GetGestureInfoFunc) (HGESTUREINFO, GESTUREINFO*);
typedef BOOL (WINAPI* SetProcessDPIAwareFunc)();
typedef BOOL (WINAPI* SetProcessDPIAwarenessFunc) (Process_DPI_Awareness);
typedef HRESULT (WINAPI* GetDPIForMonitorFunc) (HMONITOR, Monitor_DPI_Type, UINT*, UINT*);

static RegisterTouchWindowFunc    registerTouchWindow = nullptr;
static GetTouchInputInfoFunc      getTouchInputInfo = nullptr;
static CloseTouchInputHandleFunc  closeTouchInputHandle = nullptr;
static GetGestureInfoFunc         getGestureInfo = nullptr;
static SetProcessDPIAwareFunc     setProcessDPIAware = nullptr;
static SetProcessDPIAwarenessFunc setProcessDPIAwareness = nullptr;
static GetDPIForMonitorFunc       getDPIForMonitor = nullptr;

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

static Rectangle<int> rectangleFromRECT (const RECT& r) noexcept
{
    return Rectangle<int>::leftTopRightBottom ((int) r.left, (int) r.top, (int) r.right, (int) r.bottom);
}

static void setWindowPos (HWND hwnd, Rectangle<int> bounds, UINT flags)
{
    SetWindowPos (hwnd, 0, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), flags);
}

static RECT getWindowRect (HWND hwnd)
{
    RECT r;
    GetWindowRect (hwnd, &r);
    return r;
}

static void setWindowZOrder (HWND hwnd, HWND insertAfter)
{
    SetWindowPos (hwnd, insertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
}

//==============================================================================
static void setDPIAwareness()
{
    if (JUCEApplicationBase::isStandaloneApp())
    {
        if (setProcessDPIAwareness == nullptr)
        {
            HMODULE shcoreModule = GetModuleHandleA ("SHCore.dll");

            if (shcoreModule != 0)
            {
                setProcessDPIAwareness = (SetProcessDPIAwarenessFunc) GetProcAddress (shcoreModule, "SetProcessDpiAwareness");
                getDPIForMonitor = (GetDPIForMonitorFunc) GetProcAddress (shcoreModule, "GetDpiForMonitor");

                if (setProcessDPIAwareness != nullptr && getDPIForMonitor != nullptr
//                     && SUCCEEDED (setProcessDPIAwareness (Process_Per_Monitor_DPI_Aware)))
                     && SUCCEEDED (setProcessDPIAwareness (Process_System_DPI_Aware))) // (keep using this mode temporarily..)
                    return;
            }

            if (setProcessDPIAware == nullptr)
            {
                setProcessDPIAware = (SetProcessDPIAwareFunc) getUser32Function ("SetProcessDPIAware");

                if (setProcessDPIAware != nullptr)
                    setProcessDPIAware();
            }
        }
    }
}

static double getGlobalDPI()
{
    setDPIAwareness();

    HDC dc = GetDC (0);
    const double dpi = (GetDeviceCaps (dc, LOGPIXELSX)
                      + GetDeviceCaps (dc, LOGPIXELSY)) / 2.0;
    ReleaseDC (0, dc);
    return dpi;
}

double Desktop::getDefaultMasterScale()
{
    return JUCEApplicationBase::isStandaloneApp() ? getGlobalDPI() / 96.0
                                                  : 1.0;
}

//==============================================================================
Desktop::DisplayOrientation Desktop::getCurrentOrientation() const
{
    return upright;
}

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
class WindowsBitmapImage  : public ImagePixelData
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

        HDC dc = GetDC (0);
        hdc = CreateCompatibleDC (dc);
        ReleaseDC (0, dc);

        SetMapMode (hdc, MM_TEXT);

        hBitmap = CreateDIBSection (hdc, (BITMAPINFO*) &(bitmapInfo), DIB_RGB_COLORS,
                                    (void**) &bitmapData, 0, 0);

        previousBitmap = SelectObject (hdc, hBitmap);

        if (format == Image::ARGB && clearImage)
            zeromem (bitmapData, (size_t) std::abs (h * lineStride));

        imageData = bitmapData - (lineStride * (h - 1));
    }

    ~WindowsBitmapImage()
    {
        SelectObject (hdc, previousBitmap); // Selecting the previous bitmap before deleting the DC avoids a warning in BoundsChecker
        DeleteDC (hdc);
        DeleteObject (hBitmap);
    }

    ImageType* createType() const override                       { return new NativeImageType(); }

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
        WindowsBitmapImage* im = new WindowsBitmapImage (pixelFormat, width, height, false);

        for (int i = 0; i < height; ++i)
            memcpy (im->imageData + i * lineStride, imageData + i * lineStride, (size_t) lineStride);

        return im;
    }

    void blitToWindow (HWND hwnd, HDC dc, const bool transparent,
                       const int x, const int y,
                       const uint8 updateLayeredWindowAlpha) noexcept
    {
        SetMapMode (dc, MM_TEXT);

        if (transparent)
        {
            RECT windowBounds = getWindowRect (hwnd);

            POINT p = { -x, -y };
            POINT pos = { windowBounds.left, windowBounds.top };
            SIZE size = { windowBounds.right - windowBounds.left,
                          windowBounds.bottom - windowBounds.top };

            BLENDFUNCTION bf;
            bf.AlphaFormat = 1 /*AC_SRC_ALPHA*/;
            bf.BlendFlags = 0;
            bf.BlendOp = AC_SRC_OVER;
            bf.SourceConstantAlpha = updateLayeredWindowAlpha;

            updateLayeredWindow (hwnd, 0, &pos, &size, hdc, &p, 0, &bf, 2 /*ULW_ALPHA*/);
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
        HDC dc = GetDC (0);
        const int bitsPerPixel = GetDeviceCaps (dc, BITSPIXEL);
        ReleaseDC (0, dc);
        return bitsPerPixel > 24;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsBitmapImage)
};

//==============================================================================
namespace IconConverters
{
    Image createImageFromHBITMAP (HBITMAP bitmap)
    {
        Image im;

        if (bitmap != 0)
        {
            BITMAP bm;

            if (GetObject (bitmap, sizeof (BITMAP), &bm)
                 && bm.bmWidth > 0 && bm.bmHeight > 0)
            {
                HDC tempDC = GetDC (0);
                HDC dc = CreateCompatibleDC (tempDC);
                ReleaseDC (0, tempDC);

                SelectObject (dc, bitmap);

                im = Image (Image::ARGB, bm.bmWidth, bm.bmHeight, true);
                Image::BitmapData imageData (im, Image::BitmapData::writeOnly);

                for (int y = bm.bmHeight; --y >= 0;)
                {
                    for (int x = bm.bmWidth; --x >= 0;)
                    {
                        COLORREF col = GetPixel (dc, x, y);

                        imageData.setPixelColour (x, y, Colour ((uint8) GetRValue (col),
                                                                (uint8) GetGValue (col),
                                                                (uint8) GetBValue (col)));
                    }
                }

                DeleteDC (dc);
            }
        }

        return im;
    }

    Image createImageFromHICON (HICON icon)
    {
        ICONINFO info;

        if (GetIconInfo (icon, &info))
        {
            Image mask  (createImageFromHBITMAP (info.hbmMask));
            Image image (createImageFromHBITMAP (info.hbmColor));

            if (mask.isValid() && image.isValid())
            {
                for (int y = image.getHeight(); --y >= 0;)
                {
                    for (int x = image.getWidth(); --x >= 0;)
                    {
                        const float brightness = mask.getPixelAt (x, y).getBrightness();

                        if (brightness > 0.0f)
                            image.multiplyAlphaAt (x, y, 1.0f - brightness);
                    }
                }

                return image;
            }
        }

        return Image::null;
    }

    HICON createHICONFromImage (const Image& image, const BOOL isIcon, int hotspotX, int hotspotY)
    {
        WindowsBitmapImage* nativeBitmap = new WindowsBitmapImage (Image::ARGB, image.getWidth(), image.getHeight(), true);
        Image bitmap (nativeBitmap);

        {
            Graphics g (bitmap);
            g.drawImageAt (image, 0, 0);
        }

        HBITMAP mask = CreateBitmap (image.getWidth(), image.getHeight(), 1, 1, 0);

        ICONINFO info;
        info.fIcon = isIcon;
        info.xHotspot = (DWORD) hotspotX;
        info.yHotspot = (DWORD) hotspotY;
        info.hbmMask = mask;
        info.hbmColor = nativeBitmap->hBitmap;

        HICON hi = CreateIconIndirect (&info);
        DeleteObject (mask);
        return hi;
    }
}

//==============================================================================
class HWNDComponentPeer  : public ComponentPeer
{
public:
    enum RenderingEngineType
    {
        softwareRenderingEngine = 0,
        direct2DRenderingEngine
    };

    //==============================================================================
    HWNDComponentPeer (Component& comp, const int windowStyleFlags, HWND parent, bool nonRepainting)
        : ComponentPeer (comp, windowStyleFlags),
          dontRepaint (nonRepainting),
          parentToAddTo (parent),
          currentRenderingEngine (softwareRenderingEngine),
          lastPaintTime (0),
          lastMagnifySize (0),
          fullScreen (false),
          isDragging (false),
          isMouseOver (false),
          hasCreatedCaret (false),
          constrainerIsResizing (false),
          currentWindowIcon (0),
          dropTarget (nullptr),
          updateLayeredWindowAlpha (255)
    {
        callFunctionIfNotLocked (&createWindowCallback, this);

        setTitle (component.getName());

        if ((windowStyleFlags & windowHasDropShadow) != 0
             && Desktop::canUseSemiTransparentWindows()
             && ((! hasTitleBar()) || SystemStats::getOperatingSystemType() < SystemStats::WinVista))
        {
            shadower = component.getLookAndFeel().createDropShadowerForComponent (&component);

            if (shadower != nullptr)
                shadower->setOwner (&component);
        }
    }

    ~HWNDComponentPeer()
    {
        shadower = nullptr;

        // do this before the next bit to avoid messages arriving for this window
        // before it's destroyed
        JuceWindowIdentifier::setAsJUCEWindow (hwnd, false);

        callFunctionIfNotLocked (&destroyWindowCallback, (void*) hwnd);

        if (currentWindowIcon != 0)
            DestroyIcon (currentWindowIcon);

        if (dropTarget != nullptr)
        {
            dropTarget->clear();
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
        ShowWindow (hwnd, shouldBeVisible ? SW_SHOWNA : SW_HIDE);

        if (shouldBeVisible)
            InvalidateRect (hwnd, 0, 0);
        else
            lastPaintTime = 0;
    }

    void setTitle (const String& title) override
    {
        // Unfortunately some ancient bits of win32 mean you can only perform this operation from the message thread.
        jassert (MessageManager::getInstance()->isThisTheMessageThread());

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
            windowBorder = BorderSize<int> (info.rcClient.top - info.rcWindow.top,
                                            info.rcClient.left - info.rcWindow.left,
                                            info.rcWindow.bottom - info.rcClient.bottom,
                                            info.rcWindow.right - info.rcClient.right);

       #if JUCE_DIRECT2D
        if (direct2DContext != nullptr)
            direct2DContext->resized();
       #endif
    }

    void setBounds (const Rectangle<int>& bounds, bool isNowFullScreen) override
    {
        fullScreen = isNowFullScreen;

        Rectangle<int> newBounds (windowBorder.addedTo (bounds));

        if (isUsingUpdateLayeredWindow())
        {
            if (HWND parentHwnd = GetParent (hwnd))
            {
                RECT parentRect = getWindowRect (parentHwnd);
                newBounds.translate (parentRect.left, parentRect.top);
            }
        }

        const Rectangle<int> oldBounds (getBounds());
        const bool hasMoved = (oldBounds.getPosition() != bounds.getPosition());
        const bool hasResized = (oldBounds.getWidth() != bounds.getWidth()
                                  || oldBounds.getHeight() != bounds.getHeight());

        DWORD flags = SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER;
        if (! hasMoved)    flags |= SWP_NOMOVE;
        if (! hasResized)  flags |= SWP_NOSIZE;

        setWindowPos (hwnd, newBounds, flags);

        if (hasResized && isValidPeer (this))
        {
            updateBorderSize();
            repaintNowIfTransparent();
        }
    }

    Rectangle<int> getBounds() const override
    {
        Rectangle<int> bounds (rectangleFromRECT (getWindowRect (hwnd)));

        if (HWND parentH = GetParent (hwnd))
        {
            RECT r = getWindowRect (parentH);
            bounds.translate (-r.left, -r.top);
        }

        return windowBorder.subtractedFrom (bounds);
    }

    Point<int> getScreenPosition() const
    {
        RECT r = getWindowRect (hwnd);

        return Point<int> (r.left + windowBorder.getLeft(),
                           r.top  + windowBorder.getTop());
    }

    Point<int> localToGlobal (Point<int> relativePosition) override  { return relativePosition + getScreenPosition(); }
    Point<int> globalToLocal (Point<int> screenPosition) override    { return screenPosition   - getScreenPosition(); }

    void setAlpha (float newAlpha) override
    {
        const uint8 intAlpha = (uint8) jlimit (0, 255, (int) (newAlpha * 255.0f));

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
                RedrawWindow (hwnd, 0, 0, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
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
        if (shouldBeMinimised != isMinimised())
            ShowWindow (hwnd, shouldBeMinimised ? SW_MINIMIZE : SW_SHOWNORMAL);
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
        setMinimised (false);

        if (isFullScreen() != shouldBeFullScreen)
        {
            fullScreen = shouldBeFullScreen;
            const WeakReference<Component> deletionChecker (&component);

            if (! fullScreen)
            {
                const Rectangle<int> boundsCopy (lastNonFullscreenBounds);

                if (hasTitleBar())
                    ShowWindow (hwnd, SW_SHOWNORMAL);

                if (! boundsCopy.isEmpty())
                    setBounds (boundsCopy, false);
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
        RECT r = getWindowRect (hwnd);

        if (! (isPositiveAndBelow (localPos.x, (int) (r.right - r.left))
                && isPositiveAndBelow (localPos.y, (int) (r.bottom - r.top))))
            return false;

        POINT p = { localPos.x + r.left + windowBorder.getLeft(),
                    localPos.y + r.top  + windowBorder.getTop() };

        HWND w = WindowFromPoint (p);
        return w == hwnd || (trueIfInAChildWindow && (IsChild (hwnd, w) != 0));
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
        setMinimised (false);

        const bool oldDeactivate = shouldDeactivateTitleBar;
        shouldDeactivateTitleBar = ((styleFlags & windowIsTemporary) == 0);

        callFunctionIfNotLocked (makeActive ? &toFrontCallback1 : &toFrontCallback2, hwnd);

        shouldDeactivateTitleBar = oldDeactivate;

        if (! makeActive)
        {
            // in this case a broughttofront call won't have occured, so do it now..
            handleBroughtToFront();
        }
    }

    void toBehind (ComponentPeer* other) override
    {
        if (HWNDComponentPeer* const otherPeer = dynamic_cast <HWNDComponentPeer*> (other))
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
        return callFunctionIfNotLocked (&getFocusCallback, 0) == (void*) hwnd;
    }

    void grabFocus() override
    {
        const bool oldDeactivate = shouldDeactivateTitleBar;
        shouldDeactivateTitleBar = ((styleFlags & windowIsTemporary) == 0);

        callFunctionIfNotLocked (&setFocusCallback, hwnd);

        shouldDeactivateTitleBar = oldDeactivate;
    }

    void textInputRequired (Point<int>, TextInputTarget&) override
    {
        if (! hasCreatedCaret)
        {
            hasCreatedCaret = true;
            CreateCaret (hwnd, (HBITMAP) 1, 0, 0);
        }

        ShowCaret (hwnd);
        SetCaretPos (0, 0);
    }

    void dismissPendingTextInput() override
    {
        imeHandler.handleSetContext (hwnd, false);
    }

    void repaint (const Rectangle<int>& area) override
    {
        const RECT r = { area.getX(), area.getY(), area.getRight(), area.getBottom() };
        InvalidateRect (hwnd, &r, FALSE);
    }

    void performAnyPendingRepaintsNow() override
    {
        if (component.isVisible())
        {
            WeakReference<Component> localRef (&component);
            MSG m;

            if (isUsingUpdateLayeredWindow() || PeekMessage (&m, hwnd, WM_PAINT, WM_PAINT, PM_REMOVE))
                if (localRef != nullptr) // (the PeekMessage call can dispatch messages, which may delete this comp)
                    handlePaintMessage();
        }
    }

    //==============================================================================
    static HWNDComponentPeer* getOwnerOfWindow (HWND h) noexcept
    {
        if (h != 0 && JuceWindowIdentifier::isJUCEWindow (h))
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
        if (isKeyDown (VK_RMENU))   keyMods &= ~(ModifierKeys::ctrlModifier | ModifierKeys::altModifier);

        currentModifiers = currentModifiers.withOnlyMouseButtons().withFlags (keyMods);
    }

    static void updateModifiersFromWParam (const WPARAM wParam)
    {
        int mouseMods = 0;
        if (wParam & MK_LBUTTON)   mouseMods |= ModifierKeys::leftButtonModifier;
        if (wParam & MK_RBUTTON)   mouseMods |= ModifierKeys::rightButtonModifier;
        if (wParam & MK_MBUTTON)   mouseMods |= ModifierKeys::middleButtonModifier;

        currentModifiers = currentModifiers.withoutMouseButtons().withFlags (mouseMods);
        updateKeyModifiers();
    }

    //==============================================================================
    bool dontRepaint;

    static ModifierKeys currentModifiers;
    static ModifierKeys modifiersAtLastCallback;

    //==============================================================================
    class JuceDropTarget    : public ComBaseClassHelper <IDropTarget>
    {
    public:
        JuceDropTarget (HWNDComponentPeer& p)   : ownerInfo (new OwnerInfo (p)) {}

        void clear()
        {
            ownerInfo = nullptr;
        }

        JUCE_COMRESULT DragEnter (IDataObject* pDataObject, DWORD grfKeyState, POINTL mousePos, DWORD* pdwEffect)
        {
            HRESULT hr = updateFileList (pDataObject);
            if (FAILED (hr))
                return hr;

            return DragOver (grfKeyState, mousePos, pdwEffect);
        }

        JUCE_COMRESULT DragLeave()
        {
            if (ownerInfo == nullptr)
                return S_FALSE;

            ownerInfo->owner.handleDragExit (ownerInfo->dragInfo);
            return S_OK;
        }

        JUCE_COMRESULT DragOver (DWORD /*grfKeyState*/, POINTL mousePos, DWORD* pdwEffect)
        {
            if (ownerInfo == nullptr)
                return S_FALSE;

            ownerInfo->dragInfo.position = ownerInfo->getMousePos (mousePos);
            const bool wasWanted = ownerInfo->owner.handleDragMove (ownerInfo->dragInfo);
            *pdwEffect = wasWanted ? (DWORD) DROPEFFECT_COPY : (DWORD) DROPEFFECT_NONE;
            return S_OK;
        }

        JUCE_COMRESULT Drop (IDataObject* pDataObject, DWORD /*grfKeyState*/, POINTL mousePos, DWORD* pdwEffect)
        {
            HRESULT hr = updateFileList (pDataObject);
            if (SUCCEEDED (hr))
            {
                ownerInfo->dragInfo.position = ownerInfo->getMousePos (mousePos);
                const bool wasWanted = ownerInfo->owner.handleDragDrop (ownerInfo->dragInfo);
                *pdwEffect = wasWanted ? (DWORD) DROPEFFECT_COPY : (DWORD) DROPEFFECT_NONE;
                hr = S_OK;
            }

            return hr;
        }

    private:
        struct OwnerInfo
        {
            OwnerInfo (HWNDComponentPeer& p) : owner (p) {}

            Point<int> getMousePos (const POINTL& mousePos) const
            {
                return owner.globalToLocal (Point<int> (mousePos.x, mousePos.y));
            }

            template <typename CharType>
            void parseFileList (const CharType* names, const SIZE_T totalLen)
            {
                unsigned int i = 0;

                for (;;)
                {
                    unsigned int len = 0;
                    while (i + len < totalLen && names [i + len] != 0)
                        ++len;

                    if (len == 0)
                        break;

                    dragInfo.files.add (String (names + i, len));
                    i += len + 1;
                }
            }

            HWNDComponentPeer& owner;
            ComponentPeer::DragInfo dragInfo;

            JUCE_DECLARE_NON_COPYABLE (OwnerInfo)
        };

        ScopedPointer<OwnerInfo> ownerInfo;

        struct DroppedData
        {
            DroppedData (IDataObject* const dataObject, const CLIPFORMAT type)
                : data (nullptr)
            {
                FORMATETC format = { type, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
                STGMEDIUM resetMedium = { TYMED_HGLOBAL, { 0 }, 0 };
                medium = resetMedium;

                if (SUCCEEDED (error = dataObject->GetData (&format, &medium)))
                {
                    dataSize = GlobalSize (medium.hGlobal);
                    data = GlobalLock (medium.hGlobal);
                }
            }

            ~DroppedData()
            {
                if (data != nullptr)
                    GlobalUnlock (medium.hGlobal);
            }

            HRESULT error;
            STGMEDIUM medium;
            void* data;
            SIZE_T dataSize;
        };

        HRESULT updateFileList (IDataObject* const dataObject)
        {
            if (ownerInfo == nullptr)
                return S_FALSE;

            ownerInfo->dragInfo.clear();

            DroppedData textData (dataObject, CF_UNICODETEXT);

            if (SUCCEEDED (textData.error))
            {
                ownerInfo->dragInfo.text = String (CharPointer_UTF16 ((const WCHAR*) textData.data),
                                                   CharPointer_UTF16 ((const WCHAR*) addBytesToPointer (textData.data, textData.dataSize)));
            }
            else
            {
                DroppedData fileData (dataObject, CF_HDROP);

                if (SUCCEEDED (fileData.error))
                {
                    const LPDROPFILES dropFiles = static_cast <const LPDROPFILES> (fileData.data);
                    const void* const names = addBytesToPointer (dropFiles, sizeof (DROPFILES));

                    if (dropFiles->fWide)
                        ownerInfo->parseFileList (static_cast <const WCHAR*> (names), fileData.dataSize);
                    else
                        ownerInfo->parseFileList (static_cast <const char*>  (names), fileData.dataSize);
                }
                else
                {
                    return fileData.error;
                }
            }

            return S_OK;
        }

        JUCE_DECLARE_NON_COPYABLE (JuceDropTarget)
    };

   #if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
    static bool offerKeyMessageToJUCEWindow (MSG& m)
    {
        if (m.message == WM_KEYDOWN || m.message == WM_KEYUP)
            if (Component::getCurrentlyFocusedComponent() != nullptr)
                if (HWNDComponentPeer* h = getOwnerOfWindow (m.hwnd))
                    if (m.message == WM_KEYDOWN ? h->doKeyDown (m.wParam)
                                                : h->doKeyUp (m.wParam))
                        return true;

        return false;
    }
   #endif

private:
    HWND hwnd, parentToAddTo;
    ScopedPointer<DropShadower> shadower;
    RenderingEngineType currentRenderingEngine;
   #if JUCE_DIRECT2D
    ScopedPointer<Direct2DLowLevelGraphicsContext> direct2DContext;
   #endif
    uint32 lastPaintTime;
    ULONGLONG lastMagnifySize;
    bool fullScreen, isDragging, isMouseOver, hasCreatedCaret, constrainerIsResizing;
    BorderSize<int> windowBorder;
    HICON currentWindowIcon;
    JuceDropTarget* dropTarget;
    uint8 updateLayeredWindowAlpha;
    MultiTouchMapper<DWORD> currentTouches;

    //==============================================================================
    class TemporaryImage    : public Timer
    {
    public:
        TemporaryImage() {}

        Image& getImage (const bool transparent, const int w, const int h)
        {
            const Image::PixelFormat format = transparent ? Image::ARGB : Image::RGB;

            if ((! image.isValid()) || image.getWidth() < w || image.getHeight() < h || image.getFormat() != format)
                image = Image (new WindowsBitmapImage (format, (w + 31) & ~31, (h + 31) & ~31, false));

            startTimer (3000);
            return image;
        }

        void timerCallback() override
        {
            stopTimer();
            image = Image::null;
        }

    private:
        Image image;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemporaryImage)
    };

    TemporaryImage offscreenImageGenerator;

    //==============================================================================
    class WindowClassHolder    : private DeletedAtShutdown
    {
    public:
        WindowClassHolder()
        {
            // this name has to be different for each app/dll instance because otherwise poor old Windows can
            // get a bit confused (even despite it not being a process-global window class).
            String windowClassName ("JUCE_");
            windowClassName << String::toHexString (Time::currentTimeMillis());

            HINSTANCE moduleHandle = (HINSTANCE) Process::getCurrentModuleInstanceHandle();

            TCHAR moduleFile [1024] = { 0 };
            GetModuleFileName (moduleHandle, moduleFile, 1024);
            WORD iconNum = 0;

            WNDCLASSEX wcex = { 0 };
            wcex.cbSize         = sizeof (wcex);
            wcex.style          = CS_OWNDC;
            wcex.lpfnWndProc    = (WNDPROC) windowProc;
            wcex.lpszClassName  = windowClassName.toWideCharPointer();
            wcex.cbWndExtra     = 32;
            wcex.hInstance      = moduleHandle;
            wcex.hIcon          = ExtractAssociatedIcon (moduleHandle, moduleFile, &iconNum);
            iconNum = 1;
            wcex.hIconSm        = ExtractAssociatedIcon (moduleHandle, moduleFile, &iconNum);

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

        LPCTSTR getWindowClassName() const noexcept     { return (LPCTSTR) MAKELONG (atom, 0); }

        juce_DeclareSingleton_SingleThreaded_Minimal (WindowClassHolder);

    private:
        ATOM atom;

        static bool isHWNDBlockedByModalComponents (HWND h)
        {
            for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
                if (Component* const c = Desktop::getInstance().getComponent (i))
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
                    if (isHWNDBlockedByModalComponents (m.hwnd))
                    {
                        if (Component* const modal = Component::getCurrentlyModalComponent (0))
                            modal->inputAttemptWhenModal();

                        return true;
                    }
                    break;

                default:
                    break;
            }

            return false;
        }

        JUCE_DECLARE_NON_COPYABLE (WindowClassHolder)
    };

    //==============================================================================
    static void* createWindowCallback (void* userData)
    {
        static_cast <HWNDComponentPeer*> (userData)->createWindow();
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
        else if (parentToAddTo != 0)
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

        if ((styleFlags & windowIsSemiTransparent) != 0 && Desktop::canUseSemiTransparentWindows())
            exstyle |= WS_EX_LAYERED;

        hwnd = CreateWindowEx (exstyle, WindowClassHolder::getInstance()->getWindowClassName(),
                               L"", type, 0, 0, 0, 0, parentToAddTo, 0,
                               (HINSTANCE) Process::getCurrentModuleInstanceHandle(), 0);

        if (hwnd != 0)
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

                dropTarget = new JuceDropTarget (*peer);
            }

            RegisterDragDrop (hwnd, dropTarget);

            if (canUseMultiTouch())
                registerTouchWindow (hwnd, 0);

            setDPIAwareness();
            setMessageFilter();
            updateBorderSize();

            // Calling this function here is (for some reason) necessary to make Windows
            // correctly enable the menu items that we specify in the wm_initmenu message.
            GetSystemMenu (hwnd, false);

            const float alpha = component.getAlpha();
            if (alpha < 1.0f)
                setAlpha (alpha);
        }
        else
        {
            jassertfalse;
        }
    }

    static void* destroyWindowCallback (void* handle)
    {
        RevokeDragDrop ((HWND) handle);
        DestroyWindow ((HWND) handle);
        return nullptr;
    }

    static void* toFrontCallback1 (void* h)
    {
        SetForegroundWindow ((HWND) h);
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


    void setIcon (const Image& newIcon)
    {
        HICON hicon = IconConverters::createHICONFromImage (newIcon, TRUE, 0, 0);

        if (hicon != 0)
        {
            SendMessage (hwnd, WM_SETICON, ICON_BIG, (LPARAM) hicon);
            SendMessage (hwnd, WM_SETICON, ICON_SMALL, (LPARAM) hicon);

            if (currentWindowIcon != 0)
                DestroyIcon (currentWindowIcon);

            currentWindowIcon = hicon;
        }
    }

    void setMessageFilter()
    {
        typedef BOOL (WINAPI* ChangeWindowMessageFilterExFunc) (HWND, UINT, DWORD, PVOID);

        if (ChangeWindowMessageFilterExFunc changeMessageFilter
                = (ChangeWindowMessageFilterExFunc) getUser32Function ("ChangeWindowMessageFilterEx"))
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
            ChildWindowClippingInfo& info = *(ChildWindowClippingInfo*) context;

            HWND parent = GetParent (hwnd);

            if (parent == info.peer->hwnd)
            {
                RECT r = getWindowRect (hwnd);
                POINT pos = { r.left, r.top };
                ScreenToClient (GetParent (hwnd), &pos);

                Rectangle<int> clip (Rectangle<int> (pos.x, pos.y,
                                                     r.right  - r.left,
                                                     r.bottom - r.top));

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
                direct2DContext->clipToRectangle (rectangleFromRECT (r));
                handlePaint (*direct2DContext);
                direct2DContext->end();
            }
        }
        else
       #endif

        HRGN rgn = CreateRectRgn (0, 0, 0, 0);
        const int regionType = GetUpdateRgn (hwnd, rgn, false);

        PAINTSTRUCT paintStruct;
        HDC dc = BeginPaint (hwnd, &paintStruct); // Note this can immediately generate a WM_NCPAINT
                                                  // message and become re-entrant, but that's OK

        // if something in a paint handler calls, e.g. a message box, this can become reentrant and
        // corrupt the image it's using to paint into, so do a check here.
        static bool reentrant = false;
        if (! (reentrant || dontRepaint))
        {
            const ScopedValueSetter<bool> setter (reentrant, true, false);
            performPaint (dc, rgn, regionType, paintStruct);
        }

        DeleteObject (rgn);
        EndPaint (hwnd, &paintStruct);

       #ifndef JUCE_GCC
        _fpreset(); // because some graphics cards can unmask FP exceptions
       #endif

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

            RECT r = getWindowRect (hwnd);
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

                char rgnData [8192];
                const DWORD res = GetRegionData (rgn, sizeof (rgnData), (RGNDATA*) rgnData);

                if (res > 0 && res <= sizeof (rgnData))
                {
                    const RGNDATAHEADER* const hdr = &(((const RGNDATA*) rgnData)->rdh);

                    if (hdr->iType == RDH_RECTANGLES
                         && hdr->rcBound.right - hdr->rcBound.left >= w
                         && hdr->rcBound.bottom - hdr->rcBound.top >= h)
                    {
                        needToPaintAll = false;

                        const RECT* rects = (const RECT*) (rgnData + sizeof (RGNDATAHEADER));

                        for (int i = (int) ((RGNDATA*) rgnData)->rdh.nCount; --i >= 0;)
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
                    for (const Rectangle<int>* i = contextClip.begin(), * const e = contextClip.end(); i != e; ++i)
                        offscreenImage.clear (*i);

                // if the component's not opaque, this won't draw properly unless the platform can support this
                jassert (Desktop::canUseSemiTransparentWindows() || component.isOpaque());

                {
                    ScopedPointer<LowLevelGraphicsContext> context (component.getLookAndFeel()
                                                                        .createGraphicsContext (offscreenImage, Point<int> (-x, -y), contextClip));
                    handlePaint (*context);
                }

                static_cast <WindowsBitmapImage*> (offscreenImage.getPixelData())
                    ->blitToWindow (hwnd, dc, transparent, x, y, updateLayeredWindowAlpha);
            }

            if (childClipInfo.savedDC != 0)
                RestoreDC (dc, childClipInfo.savedDC);
        }
    }

    //==============================================================================
    void doMouseEvent (Point<int> position)
    {
        handleMouseEvent (0, position, currentModifiers, getMouseEventTime());
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
            direct2DContext = 0;
        else if (direct2DContext == 0)
            direct2DContext = new Direct2DLowLevelGraphicsContext (hwnd);
    }
   #endif

    void setCurrentRenderingEngine (int index) override
    {
        (void) index;

       #if JUCE_DIRECT2D
        if (getAvailableRenderingEngines().size() > 1)
        {
            currentRenderingEngine = index == 1 ? direct2DRenderingEngine : softwareRenderingEngine;
            updateDirect2DContext();
            repaint (component.getLocalBounds());
        }
       #endif
    }

    static int getMinTimeBetweenMouseMoves()
    {
        if (SystemStats::getOperatingSystemType() >= SystemStats::WinVista)
            return 0;

        return 1000 / 60;  // Throttling the incoming mouse-events seems to still be needed in XP..
    }

    void doMouseMove (Point<int> position)
    {
        if (! isMouseOver)
        {
            isMouseOver = true;
            ModifierKeys::getCurrentModifiersRealtime(); // (This avoids a rare stuck-button problem when focus is lost unexpectedly)
            updateKeyModifiers();

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
            if (! contains (position, false))
                return;
        }

        static uint32 lastMouseTime = 0;
        static int minTimeBetweenMouses = getMinTimeBetweenMouseMoves();
        const uint32 now = Time::getMillisecondCounter();

        if (now >= lastMouseTime + minTimeBetweenMouses)
        {
            lastMouseTime = now;
            doMouseEvent (position);
        }
    }

    void doMouseDown (Point<int> position, const WPARAM wParam)
    {
        if (GetCapture() != hwnd)
            SetCapture (hwnd);

        doMouseMove (position);

        updateModifiersFromWParam (wParam);
        isDragging = true;

        doMouseEvent (position);
    }

    void doMouseUp (Point<int> position, const WPARAM wParam)
    {
        updateModifiersFromWParam (wParam);
        const bool wasDragging = isDragging;
        isDragging = false;

        // release the mouse capture if the user has released all buttons
        if ((wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON)) == 0 && hwnd == GetCapture())
            ReleaseCapture();

        // NB: under some circumstances (e.g. double-clicking a native title bar), a mouse-up can
        // arrive without a mouse-down, so in that case we need to avoid sending a message.
        if (wasDragging)
            doMouseEvent (position);
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
        doMouseEvent (getCurrentMousePos());
    }

    ComponentPeer* findPeerUnderMouse (Point<int>& localPos)
    {
        const Point<int> globalPos (getCurrentMousePosGlobal());

        // Because Windows stupidly sends all wheel events to the window with the keyboard
        // focus, we have to redirect them here according to the mouse pos..
        POINT p = { globalPos.x, globalPos.y };
        HWNDComponentPeer* peer = getOwnerOfWindow (WindowFromPoint (p));

        if (peer == nullptr)
            peer = this;

        localPos = peer->globalToLocal (globalPos);
        return peer;
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

        Point<int> localPos;
        if (ComponentPeer* const peer = findPeerUnderMouse (localPos))
            peer->handleMouseWheel (0, localPos, getMouseEventTime(), wheel);
    }

    bool doGestureEvent (LPARAM lParam)
    {
        GESTUREINFO gi;
        zerostruct (gi);
        gi.cbSize = sizeof (gi);

        if (getGestureInfo != nullptr && getGestureInfo ((HGESTUREINFO) lParam, &gi))
        {
            updateKeyModifiers();
            Point<int> localPos;

            if (ComponentPeer* const peer = findPeerUnderMouse (localPos))
            {
                switch (gi.dwID)
                {
                    case 3: /*GID_ZOOM*/
                        if (gi.dwFlags != 1 /*GF_BEGIN*/ && lastMagnifySize > 0)
                            peer->handleMagnifyGesture (0, localPos, getMouseEventTime(),
                                                        (float) (gi.ullArguments / (double) lastMagnifySize));

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
            if (HWNDComponentPeer* const parent = getOwnerOfWindow (GetParent (hwnd)))
                if (parent != this)
                    return parent->doTouchEvent (numInputs, eventHandle);

        HeapBlock<TOUCHINPUT> inputInfo (numInputs);

        if (getTouchInputInfo (eventHandle, numInputs, inputInfo, sizeof (TOUCHINPUT)))
        {
            for (int i = 0; i < numInputs; ++i)
            {
                const DWORD flags = inputInfo[i].dwFlags;

                if ((flags & (TOUCHEVENTF_DOWN | TOUCHEVENTF_MOVE | TOUCHEVENTF_UP)) != 0)
                    if (! handleTouchInput (inputInfo[i], (flags & TOUCHEVENTF_PRIMARY) != 0,
                                            (flags & TOUCHEVENTF_DOWN) != 0, (flags & TOUCHEVENTF_UP) != 0))
                        return 0;  // abandon method if this window was deleted by the callback
            }
        }

        closeTouchInputHandle (eventHandle);
        return 0;
    }

    bool handleTouchInput (const TOUCHINPUT& touch, const bool isPrimary, const bool isDown, const bool isUp)
    {
        bool isCancel = false;
        const int touchIndex = currentTouches.getIndexOfTouch (touch.dwID);
        const int64 time = getMouseEventTime();
        const Point<int> pos (globalToLocal (Point<int> ((int) TOUCH_COORD_TO_PIXEL (touch.x),
                                                         (int) TOUCH_COORD_TO_PIXEL (touch.y))));
        ModifierKeys modsToSend (currentModifiers);

        if (isDown)
        {
            currentModifiers = currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
            modsToSend = currentModifiers;

            if (! isPrimary)
            {
                // this forces a mouse-enter/up event, in case for some reason we didn't get a mouse-up before.
                handleMouseEvent (touchIndex, pos, modsToSend.withoutMouseButtons(), time);
                if (! isValidPeer (this)) // (in case this component was deleted by the event)
                    return false;
            }
        }
        else if (isUp)
        {
            modsToSend = modsToSend.withoutMouseButtons();
            currentTouches.clearTouch (touchIndex);

            if (! currentTouches.areAnyTouchesActive())
                isCancel = true;
        }
        else
        {
            modsToSend = currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
        }

        if (isCancel)
        {
            currentTouches.clear();
            currentModifiers = currentModifiers.withoutMouseButtons();
        }

        if (! isPrimary)
        {
            handleMouseEvent (touchIndex, pos, modsToSend, time);
            if (! isValidPeer (this)) // (in case this component was deleted by the event)
                return false;
        }

        if ((isUp || isCancel) && ! isPrimary)
        {
            handleMouseEvent (touchIndex, Point<int> (-10, -10), currentModifiers, time);
            if (! isValidPeer (this))
                return false;
        }

        return true;
    }

    //==============================================================================
    void sendModifierKeyChangeIfNeeded()
    {
        if (modifiersAtLastCallback != currentModifiers)
        {
            modifiersAtLastCallback = currentModifiers;
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
                        GetKeyboardState (keyState);

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

        juce_wchar textChar = (juce_wchar) key;

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
            if (textChar < ' ' && currentModifiers.testFlags (ModifierKeys::ctrlModifier | ModifierKeys::altModifier))
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
            {
                handleKeyPress (key, 0);
                return true;
            }
        }

        return false;
    }

    bool isConstrainedNativeWindow() const
    {
        return constrainer != nullptr
                && (styleFlags & (windowHasTitleBar | windowIsResizable)) == (windowHasTitleBar | windowIsResizable)
                && ! isKioskMode();
    }

    Rectangle<int> getCurrentScaledBounds (float scale) const
    {
        return ScalingHelpers::unscaledScreenPosToScaled (scale, windowBorder.addedTo (ScalingHelpers::scaledScreenPosToUnscaled (scale, component.getBounds())));
    }

    LRESULT handleSizeConstraining (RECT& r, const WPARAM wParam)
    {
        if (isConstrainedNativeWindow())
        {
            const float scale = getComponent().getDesktopScaleFactor();
            Rectangle<int> pos (ScalingHelpers::unscaledScreenPosToScaled (scale, rectangleFromRECT (r)));
            const Rectangle<int> current (getCurrentScaledBounds (scale));

            constrainer->checkBounds (pos, current,
                                      Desktop::getInstance().getDisplays().getTotalBounds (true),
                                      wParam == WMSZ_TOP    || wParam == WMSZ_TOPLEFT    || wParam == WMSZ_TOPRIGHT,
                                      wParam == WMSZ_LEFT   || wParam == WMSZ_TOPLEFT    || wParam == WMSZ_BOTTOMLEFT,
                                      wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT,
                                      wParam == WMSZ_RIGHT  || wParam == WMSZ_TOPRIGHT   || wParam == WMSZ_BOTTOMRIGHT);

            pos = ScalingHelpers::scaledScreenPosToUnscaled (scale, pos);
            r.left   = pos.getX();
            r.top    = pos.getY();
            r.right  = pos.getRight();
            r.bottom = pos.getBottom();
        }

        return TRUE;
    }

    LRESULT handlePositionChanging (WINDOWPOS& wp)
    {
        if (isConstrainedNativeWindow())
        {
            if ((wp.flags & (SWP_NOMOVE | SWP_NOSIZE)) != (SWP_NOMOVE | SWP_NOSIZE)
                 && ! Component::isMouseButtonDownAnywhere())
            {
                const float scale = getComponent().getDesktopScaleFactor();
                Rectangle<int> pos (ScalingHelpers::unscaledScreenPosToScaled (scale, Rectangle<int> (wp.x, wp.y, wp.cx, wp.cy)));
                const Rectangle<int> current (getCurrentScaledBounds (scale));

                constrainer->checkBounds (pos, current,
                                          Desktop::getInstance().getDisplays().getTotalBounds (true),
                                          pos.getY() != current.getY() && pos.getBottom() == current.getBottom(),
                                          pos.getX() != current.getX() && pos.getRight()  == current.getRight(),
                                          pos.getY() == current.getY() && pos.getBottom() != current.getBottom(),
                                          pos.getX() == current.getX() && pos.getRight()  != current.getRight());

                pos = ScalingHelpers::scaledScreenPosToUnscaled (scale, pos);
                wp.x = pos.getX();
                wp.y = pos.getY();
                wp.cx = pos.getWidth();
                wp.cy = pos.getHeight();
            }
        }

        if (((wp.flags & SWP_SHOWWINDOW) != 0 && ! component.isVisible()))
            component.setVisible (true);
        else if (((wp.flags & SWP_HIDEWINDOW) != 0 && component.isVisible()))
            component.setVisible (false);

        return 0;
    }

    void handleAppActivation (const WPARAM wParam)
    {
        modifiersAtLastCallback = -1;
        updateKeyModifiers();

        if (isMinimised())
        {
            component.repaint();
            handleMovedOrResized();

            if (! ComponentPeer::isValidPeer (this))
                return;
        }

        Component* underMouse = component.getComponentAt (component.getMouseXYRelative());

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
        Desktop& desktop = Desktop::getInstance();

        const_cast <Desktop::Displays&> (desktop.getDisplays()).refresh();

        if (fullScreen && ! isMinimised())
        {
            const Desktop::Displays::Display& display
                    = desktop.getDisplays().getDisplayContaining (component.getScreenBounds().getCentre());

            setWindowPos (hwnd, display.userArea * display.scale,
                          SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOSENDCHANGING);
        }
    }

    void handleDPIChange() // happens when a window moves to a screen with a different DPI.
    {
    }

    //==============================================================================
public:
    static LRESULT CALLBACK windowProc (HWND h, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (HWNDComponentPeer* const peer = getOwnerOfWindow (h))
        {
            jassert (isValidPeer (peer));
            return peer->peerWindowProc (h, message, wParam, lParam);
        }

        return DefWindowProcW (h, message, wParam, lParam);
    }

private:
    static void* callFunctionIfNotLocked (MessageCallbackFunction* callback, void* userData)
    {
        if (MessageManager::getInstance()->currentThreadHasLockedMessageManager())
            return callback (userData);

        return MessageManager::getInstance()->callFunctionOnMessageThread (callback, userData);
    }

    static Point<int> getPointFromLParam (LPARAM lParam) noexcept
    {
        return Point<int> (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
    }

    static Point<int> getCurrentMousePosGlobal() noexcept
    {
        return getPointFromLParam (GetMessagePos());
    }

    Point<int> getCurrentMousePos() noexcept
    {
        return globalToLocal (getCurrentMousePosGlobal());
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
                if (wParam != 1) // (1 = a repaint of the entire NC region)
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
            case WM_MOUSEMOVE:          doMouseMove (getPointFromLParam (lParam)); return 0;
            case WM_MOUSELEAVE:         doMouseExit(); return 0;

            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:        doMouseDown (getPointFromLParam (lParam), wParam); return 0;

            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:          doMouseUp (getPointFromLParam (lParam), wParam); return 0;

            case 0x020A: /* WM_MOUSEWHEEL */   doMouseWheel (wParam, true);  return 0;
            case 0x020E: /* WM_MOUSEHWHEEL */  doMouseWheel (wParam, false); return 0;

            case WM_CAPTURECHANGED:     doCaptureChanged(); return 0;

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
            case WM_SIZING:                return handleSizeConstraining (*(RECT*) lParam, wParam);
            case WM_WINDOWPOSCHANGING:     return handlePositionChanging (*(WINDOWPOS*) lParam);

            case WM_WINDOWPOSCHANGED:
                {
                    const Point<int> pos (getCurrentMousePos());
                    if (contains (pos, false))
                        doMouseEvent (pos);
                }

                handleMovedOrResized();

                if (dontRepaint)
                    break;  // needed for non-accelerated openGL windows to draw themselves correctly..

                return 0;

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
                break;

            case WM_ACTIVATEAPP:
                // Windows does weird things to process priority when you swap apps,
                // so this forces an update when the app is brought to the front
                if (wParam != FALSE)
                    juce_repeatLastProcessPriority();
                else
                    Desktop::getInstance().setKioskModeComponent (nullptr); // turn kiosk mode off if we lose focus

                juce_checkCurrentlyFocusedTopLevelWindow();
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

            case WM_QUERYENDSESSION:
                if (JUCEApplicationBase* const app = JUCEApplicationBase::getInstance())
                {
                    app->systemRequestedQuit();
                    return MessageManager::getInstance()->hasStopMessageBeenSent();
                }
                return TRUE;

            case WM_SYNCPAINT:
                return 0;

            case WM_DISPLAYCHANGE:
                InvalidateRect (h, 0, 0);
                // intentional fall-through...
            case WM_SETTINGCHANGE:  // note the fall-through in the previous case!
                doSettingChange();
                break;

            case 0x2e0: // WM_DPICHANGED
                handleDPIChange();
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

            case WM_NCLBUTTONDOWN:
                handleLeftClickInNCArea (wParam);
                break;

            case WM_NCRBUTTONDOWN:
            case WM_NCMBUTTONDOWN:
                sendInputAttemptWhenModalMessage();
                break;

            case WM_IME_SETCONTEXT:
                imeHandler.handleSetContext (h, wParam == TRUE);
                lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
                break;

            case WM_IME_STARTCOMPOSITION:  imeHandler.handleStartComposition (*this); return 0;
            case WM_IME_ENDCOMPOSITION:    imeHandler.handleEndComposition (*this, h); break;
            case WM_IME_COMPOSITION:       imeHandler.handleComposition (*this, h, lParam); return 0;

            case WM_GETDLGCODE:
                return DLGC_WANTALLKEYS;

            default:
                break;
        }

        return DefWindowProcW (h, message, wParam, lParam);
    }

    bool sendInputAttemptWhenModalMessage()
    {
        if (component.isCurrentlyBlockedByAnotherModalComponent())
        {
            if (Component* const current = Component::getCurrentlyModalComponent())
                current->inputAttemptWhenModal();

            return true;
        }

        return false;
    }

    //==============================================================================
    class IMEHandler
    {
    public:
        IMEHandler()
        {
            reset();
        }

        void handleSetContext (HWND hWnd, const bool windowIsActive)
        {
            if (compositionInProgress && ! windowIsActive)
            {
                compositionInProgress = false;

                if (HIMC hImc = ImmGetContext (hWnd))
                {
                    ImmNotifyIME (hImc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
                    ImmReleaseContext (hWnd, hImc);
                }
            }
        }

        void handleStartComposition (ComponentPeer& owner)
        {
            reset();

            if (TextInputTarget* const target = owner.findCurrentTextInputTarget())
                target->insertTextAtCaret (String::empty);
        }

        void handleEndComposition (ComponentPeer& owner, HWND hWnd)
        {
            if (compositionInProgress)
            {
                // If this occurs, the user has cancelled the composition, so clear their changes..
                if (TextInputTarget* const target = owner.findCurrentTextInputTarget())
                {
                    target->setHighlightedRegion (compositionRange);
                    target->insertTextAtCaret (String::empty);
                    compositionRange.setLength (0);

                    target->setHighlightedRegion (Range<int>::emptyRange (compositionRange.getEnd()));
                    target->setTemporaryUnderlining (Array<Range<int> >());
                }

                if (HIMC hImc = ImmGetContext (hWnd))
                {
                    ImmNotifyIME (hImc, NI_CLOSECANDIDATE, 0, 0);
                    ImmReleaseContext (hWnd, hImc);
                }
            }

            reset();
        }

        void handleComposition (ComponentPeer& owner, HWND hWnd, const LPARAM lParam)
        {
            TextInputTarget* const target = owner.findCurrentTextInputTarget();
            HIMC hImc = ImmGetContext (hWnd);

            if (target == nullptr || hImc == 0)
                return;

            if (compositionRange.getStart() < 0)
                compositionRange = Range<int>::emptyRange (target->getHighlightedRegion().getStart());

            if ((lParam & GCS_RESULTSTR) != 0) // (composition has finished)
            {
                replaceCurrentSelection (target, getCompositionString (hImc, GCS_RESULTSTR),
                                         Range<int>::emptyRange (-1));

                reset();
                target->setTemporaryUnderlining (Array<Range<int> >());
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
            jassert (hImc != 0);

            const int stringSizeBytes = ImmGetCompositionString (hImc, type, 0, 0);

            if (stringSizeBytes > 0)
            {
                HeapBlock<TCHAR> buffer;
                buffer.calloc (stringSizeBytes / sizeof (TCHAR) + 1);
                ImmGetCompositionString (hImc, type, buffer, (DWORD) stringSizeBytes);
                return String (buffer);
            }

            return String::empty;
        }

        int getCompositionCaretPos (HIMC hImc, LPARAM lParam, const String& currentIMEString) const
        {
            jassert (hImc != 0);

            if ((lParam & CS_NOMOVECARET) != 0)
                return compositionRange.getStart();

            if ((lParam & GCS_CURSORPOS) != 0)
            {
                const int localCaretPos = ImmGetCompositionString (hImc, GCS_CURSORPOS, 0, 0);
                return compositionRange.getStart() + jmax (0, localCaretPos);
            }

            return compositionRange.getStart() + currentIMEString.length();
        }

        // Get selected/highlighted range while doing composition:
        // returned range is relative to beginning of TextInputTarget, not composition string
        Range<int> getCompositionSelection (HIMC hImc, LPARAM lParam) const
        {
            jassert (hImc != 0);
            int selectionStart = 0;
            int selectionEnd = 0;

            if ((lParam & GCS_COMPATTR) != 0)
            {
                // Get size of attributes array:
                const int attributeSizeBytes = ImmGetCompositionString (hImc, GCS_COMPATTR, 0, 0);

                if (attributeSizeBytes > 0)
                {
                    // Get attributes (8 bit flag per character):
                    HeapBlock<char> attributes ((size_t) attributeSizeBytes);
                    ImmGetCompositionString (hImc, GCS_COMPATTR, attributes, (DWORD) attributeSizeBytes);

                    selectionStart = 0;

                    for (selectionStart = 0; selectionStart < attributeSizeBytes; ++selectionStart)
                        if (attributes[selectionStart] == ATTR_TARGET_CONVERTED || attributes[selectionStart] == ATTR_TARGET_NOTCONVERTED)
                            break;

                    for (selectionEnd = selectionStart; selectionEnd < attributeSizeBytes; ++selectionEnd)
                        if (attributes [selectionEnd] != ATTR_TARGET_CONVERTED && attributes[selectionEnd] != ATTR_TARGET_NOTCONVERTED)
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

        Array<Range<int> > getCompositionUnderlines (HIMC hImc, LPARAM lParam) const
        {
            Array<Range<int> > result;

            if (hImc != 0 && (lParam & GCS_COMPCLAUSE) != 0)
            {
                const int clauseDataSizeBytes = ImmGetCompositionString (hImc, GCS_COMPCLAUSE, 0, 0);

                if (clauseDataSizeBytes > 0)
                {
                    const size_t numItems = clauseDataSizeBytes / sizeof (uint32);
                    HeapBlock<uint32> clauseData (numItems);

                    if (ImmGetCompositionString (hImc, GCS_COMPCLAUSE, clauseData, (DWORD) clauseDataSizeBytes) > 0)
                        for (size_t i = 0; i + 1 < numItems; ++i)
                            result.add (Range<int> ((int) clauseData [i], (int) clauseData [i + 1]) + compositionRange.getStart());
                }
            }

            return result;
        }

        void moveCandidateWindowToLeftAlignWithSelection (HIMC hImc, ComponentPeer& peer, TextInputTarget* target) const
        {
            if (Component* const targetComp = dynamic_cast <Component*> (target))
            {
                const Rectangle<int> area (peer.getComponent().getLocalArea (targetComp, target->getCaretRectangle()));

                CANDIDATEFORM pos = { 0, CFS_CANDIDATEPOS, { area.getX(), area.getBottom() }, { 0, 0, 0, 0 } };
                ImmSetCandidateWindow (hImc, &pos);
            }
        }

        JUCE_DECLARE_NON_COPYABLE (IMEHandler)
    };

    IMEHandler imeHandler;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HWNDComponentPeer)
};

ModifierKeys HWNDComponentPeer::currentModifiers;
ModifierKeys HWNDComponentPeer::modifiersAtLastCallback;

ComponentPeer* Component::createNewPeer (int styleFlags, void* nativeWindowToAttachTo)
{
    return new HWNDComponentPeer (*this, styleFlags,
                                  (HWND) nativeWindowToAttachTo, false);
}

ComponentPeer* createNonRepaintingEmbeddedWindowsPeer (Component* component, void* parent)
{
    jassert (component != nullptr);

    return new HWNDComponentPeer (*component, ComponentPeer::windowIgnoresMouseClicks,
                                  (HWND) parent, true);
}


juce_ImplementSingleton_SingleThreaded (HWNDComponentPeer::WindowClassHolder);


//==============================================================================
void ModifierKeys::updateCurrentModifiers() noexcept
{
    currentModifiers = HWNDComponentPeer::currentModifiers;
}

ModifierKeys ModifierKeys::getCurrentModifiersRealtime() noexcept
{
    HWNDComponentPeer::updateKeyModifiers();

    int mouseMods = 0;
    if (HWNDComponentPeer::isKeyDown (VK_LBUTTON))  mouseMods |= ModifierKeys::leftButtonModifier;
    if (HWNDComponentPeer::isKeyDown (VK_RBUTTON))  mouseMods |= ModifierKeys::rightButtonModifier;
    if (HWNDComponentPeer::isKeyDown (VK_MBUTTON))  mouseMods |= ModifierKeys::middleButtonModifier;

    HWNDComponentPeer::currentModifiers
        = HWNDComponentPeer::currentModifiers.withoutMouseButtons().withFlags (mouseMods);

    return HWNDComponentPeer::currentModifiers;
}

//==============================================================================
bool KeyPress::isKeyCurrentlyDown (const int keyCode)
{
    SHORT k = (SHORT) keyCode;

    if ((keyCode & extendedKeyModifier) == 0
         && (k >= (SHORT) 'a' && k <= (SHORT) 'z'))
        k += (SHORT) 'A' - (SHORT) 'a';

    const SHORT translatedValues[] = { (SHORT) ',', VK_OEM_COMMA,
                                       (SHORT) '+', VK_OEM_PLUS,
                                       (SHORT) '-', VK_OEM_MINUS,
                                       (SHORT) '.', VK_OEM_PERIOD,
                                       (SHORT) ';', VK_OEM_1,
                                       (SHORT) ':', VK_OEM_1,
                                       (SHORT) '/', VK_OEM_2,
                                       (SHORT) '?', VK_OEM_2,
                                       (SHORT) '[', VK_OEM_4,
                                       (SHORT) ']', VK_OEM_6 };

    for (int i = 0; i < numElementsInArray (translatedValues); i += 2)
        if (k == translatedValues [i])
            k = translatedValues [i + 1];

    return HWNDComponentPeer::isKeyDown (k);
}

#if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
bool offerKeyMessageToJUCEWindow (MSG& m)   { return HWNDComponentPeer::offerKeyMessageToJUCEWindow (m); }
#endif

//==============================================================================
bool JUCE_CALLTYPE Process::isForegroundProcess()
{
    HWND fg = GetForegroundWindow();

    if (fg == 0)
        return true;

    // when running as a plugin in IE8, the browser UI runs in a different process to the plugin, so
    // process ID isn't a reliable way to check if the foreground window belongs to us - instead, we
    // have to see if any of our windows are children of the foreground window
    fg = GetAncestor (fg, GA_ROOT);

    for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
        if (HWNDComponentPeer* const wp = dynamic_cast <HWNDComponentPeer*> (ComponentPeer::getPeer (i)))
            if (wp->isInside (fg))
                return true;

    return false;
}

// N/A on Windows as far as I know.
void JUCE_CALLTYPE Process::makeForegroundProcess() {}
void JUCE_CALLTYPE Process::hide() {}

//==============================================================================
static BOOL CALLBACK enumAlwaysOnTopWindows (HWND hwnd, LPARAM lParam)
{
    if (IsWindowVisible (hwnd))
    {
        DWORD processID = 0;
        GetWindowThreadProcessId (hwnd, &processID);

        if (processID == GetCurrentProcessId())
        {
            WINDOWINFO info;

            if (GetWindowInfo (hwnd, &info)
                 && (info.dwExStyle & WS_EX_TOPMOST) != 0)
            {
                *reinterpret_cast <bool*> (lParam) = true;
                return FALSE;
            }
        }
    }

    return TRUE;
}

bool juce_areThereAnyAlwaysOnTopWindows()
{
    bool anyAlwaysOnTopFound = false;
    EnumWindows (&enumAlwaysOnTopWindows, (LPARAM) &anyAlwaysOnTopFound);
    return anyAlwaysOnTopFound;
}

//==============================================================================
class WindowsMessageBox  : public AsyncUpdater
{
public:
    WindowsMessageBox (AlertWindow::AlertIconType iconType,
                       const String& boxTitle, const String& m,
                       Component* associatedComponent, UINT extraFlags,
                       ModalComponentManager::Callback* cb, const bool runAsync)
        : flags (extraFlags | getMessageBoxFlags (iconType)),
          owner (getWindowForMessageBox (associatedComponent)),
          title (boxTitle), message (m), callback (cb)
    {
        if (runAsync)
            triggerAsyncUpdate();
    }

    int getResult() const
    {
        const int r = MessageBox (owner, message.toWideCharPointer(), title.toWideCharPointer(), flags);
        return (r == IDYES || r == IDOK) ? 1 : (r == IDNO ? 2 : 0);
    }

    void handleAsyncUpdate() override
    {
        const int result = getResult();

        if (callback != nullptr)
            callback->modalStateFinished (result);

        delete this;
    }

private:
    UINT flags;
    HWND owner;
    String title, message;
    ScopedPointer<ModalComponentManager::Callback> callback;

    static UINT getMessageBoxFlags (AlertWindow::AlertIconType iconType) noexcept
    {
        UINT flags = MB_TASKMODAL | MB_SETFOREGROUND;

        switch (iconType)
        {
            case AlertWindow::QuestionIcon:  flags |= MB_ICONQUESTION; break;
            case AlertWindow::WarningIcon:   flags |= MB_ICONWARNING; break;
            case AlertWindow::InfoIcon:      flags |= MB_ICONINFORMATION; break;
            default: break;
        }

        return flags;
    }

    static HWND getWindowForMessageBox (Component* associatedComponent)
    {
        return associatedComponent != nullptr ? (HWND) associatedComponent->getWindowHandle() : 0;
    }
};

#if JUCE_MODAL_LOOPS_PERMITTED
void JUCE_CALLTYPE NativeMessageBox::showMessageBox (AlertWindow::AlertIconType iconType,
                                                     const String& title, const String& message,
                                                     Component* associatedComponent)
{
    WindowsMessageBox box (iconType, title, message, associatedComponent, MB_OK, 0, false);
    (void) box.getResult();
}
#endif

void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (AlertWindow::AlertIconType iconType,
                                                          const String& title, const String& message,
                                                          Component* associatedComponent,
                                                          ModalComponentManager::Callback* callback)
{
    new WindowsMessageBox (iconType, title, message, associatedComponent, MB_OK, callback, true);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (AlertWindow::AlertIconType iconType,
                                                      const String& title, const String& message,
                                                      Component* associatedComponent,
                                                      ModalComponentManager::Callback* callback)
{
    ScopedPointer<WindowsMessageBox> mb (new WindowsMessageBox (iconType, title, message, associatedComponent,
                                                                MB_OKCANCEL, callback, callback != nullptr));
    if (callback == nullptr)
        return mb->getResult() != 0;

    mb.release();
    return false;
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoCancelBox (AlertWindow::AlertIconType iconType,
                                                        const String& title, const String& message,
                                                        Component* associatedComponent,
                                                        ModalComponentManager::Callback* callback)
{
    ScopedPointer<WindowsMessageBox> mb (new WindowsMessageBox (iconType, title, message, associatedComponent,
                                                                MB_YESNOCANCEL, callback, callback != nullptr));
    if (callback == nullptr)
        return mb->getResult();

    mb.release();
    return 0;
}

//==============================================================================
bool MouseInputSource::SourceList::addSource()
{
    const int numSources = sources.size();

    if (numSources == 0 || canUseMultiTouch())
    {
        addSource (numSources, numSources == 0);
        return true;
    }

    return false;
}

Point<int> MouseInputSource::getCurrentRawMousePosition()
{
    POINT mousePos;
    GetCursorPos (&mousePos);
    return Point<int> (mousePos.x, mousePos.y);
}

void MouseInputSource::setRawMousePosition (Point<int> newPosition)
{
    SetCursorPos (newPosition.x, newPosition.y);
}

//==============================================================================
class ScreenSaverDefeater   : public Timer
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
            INPUT input = { 0 };
            input.type = INPUT_MOUSE;
            input.mi.mouseData = MOUSEEVENTF_MOVE;

            SendInput (1, &input, sizeof (INPUT));
        }
    }
};

static ScopedPointer<ScreenSaverDefeater> screenSaverDefeater;

void Desktop::setScreenSaverEnabled (const bool isEnabled)
{
    if (isEnabled)
        screenSaverDefeater = nullptr;
    else if (screenSaverDefeater == nullptr)
        screenSaverDefeater = new ScreenSaverDefeater();
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
    if (OpenClipboard (0) != 0)
    {
        if (EmptyClipboard() != 0)
        {
            const size_t bytesNeeded = CharPointer_UTF16::getBytesRequiredFor (text.getCharPointer()) + 4;

            if (bytesNeeded > 0)
            {
                if (HGLOBAL bufH = GlobalAlloc (GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, bytesNeeded + sizeof (WCHAR)))
                {
                    if (WCHAR* const data = static_cast <WCHAR*> (GlobalLock (bufH)))
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

    if (OpenClipboard (0) != 0)
    {
        if (HANDLE bufH = GetClipboardData (CF_UNICODETEXT))
        {
            if (const WCHAR* const data = (const WCHAR*) GlobalLock (bufH))
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
void Desktop::setKioskComponent (Component* kioskModeComponent, bool enableOrDisable, bool /*allowMenusAndBars*/)
{
    if (TopLevelWindow* tlw = dynamic_cast<TopLevelWindow*> (kioskModeComponent))
        tlw->setUsingNativeTitleBar (! enableOrDisable);

    if (enableOrDisable)
        kioskModeComponent->setBounds (getDisplays().getMainDisplay().totalArea);
}

//==============================================================================
struct MonitorInfo
{
    MonitorInfo (Rectangle<int> rect, bool main, double d) noexcept
        : bounds (rect), dpi (d), isMain (main) {}

    Rectangle<int> bounds;
    double dpi;
    bool isMain;
};

static BOOL CALLBACK enumMonitorsProc (HMONITOR hm, HDC, LPRECT r, LPARAM userInfo)
{
    MONITORINFO info = { 0 };
    info.cbSize = sizeof (info);
    GetMonitorInfo (hm, &info);
    const bool isMain = (info.dwFlags & 1 /* MONITORINFOF_PRIMARY */) != 0;
    double dpi = 0;

    if (getDPIForMonitor != nullptr)
    {
        UINT dpiX = 0, dpiY = 0;

        if (SUCCEEDED (getDPIForMonitor (hm, MDT_Default, &dpiX, &dpiY)))
            dpi = (dpiX + dpiY) / 2.0;
    }

    ((Array<MonitorInfo>*) userInfo)->add (MonitorInfo (rectangleFromRECT (*r), isMain, dpi));

    return TRUE;
}

void Desktop::Displays::findDisplays (float masterScale)
{
    setDPIAwareness();

    Array<MonitorInfo> monitors;
    EnumDisplayMonitors (0, 0, &enumMonitorsProc, (LPARAM) &monitors);

    const double globalDPI = getGlobalDPI();

    if (monitors.size() == 0)
        monitors.add (MonitorInfo (rectangleFromRECT (getWindowRect (GetDesktopWindow())), true, globalDPI));

    // make sure the first in the list is the main monitor
    for (int i = 1; i < monitors.size(); ++i)
        if (monitors.getReference(i).isMain)
            monitors.swap (i, 0);

    RECT workArea;
    SystemParametersInfo (SPI_GETWORKAREA, 0, &workArea, 0);

    for (int i = 0; i < monitors.size(); ++i)
    {
        Display d;
        d.userArea  = d.totalArea = monitors.getReference(i).bounds / masterScale;
        d.isMain    = monitors.getReference(i).isMain;
        d.dpi       = monitors.getReference(i).dpi;

        if (d.dpi == 0)
        {
            d.scale = masterScale;
            d.dpi = globalDPI;
        }
        else
        {
            d.scale = d.dpi / 96.0;
        }

        if (d.isMain)
            d.userArea = d.userArea.getIntersection (rectangleFromRECT (workArea) / masterScale);

        displays.add (d);
    }
}

//==============================================================================
static HICON extractFileHICON (const File& file)
{
    WORD iconNum = 0;
    WCHAR name [MAX_PATH * 2];
    file.getFullPathName().copyToUTF16 (name, sizeof (name));

    return ExtractAssociatedIcon ((HINSTANCE) Process::getCurrentModuleInstanceHandle(),
                                  name, &iconNum);
}

Image juce_createIconForFile (const File& file)
{
    Image image;
    if (HICON icon = extractFileHICON (file))
    {
        image = IconConverters::createImageFromHICON (icon);
        DestroyIcon (icon);
    }

    return image;
}

//==============================================================================
void* CustomMouseCursorInfo::create() const
{
    const int maxW = GetSystemMetrics (SM_CXCURSOR);
    const int maxH = GetSystemMetrics (SM_CYCURSOR);

    Image im (image);
    int hotspotX = hotspot.x;
    int hotspotY = hotspot.y;

    if (im.getWidth() > maxW || im.getHeight() > maxH)
    {
        im = im.rescaled (maxW, maxH);

        hotspotX = (hotspotX * maxW) / image.getWidth();
        hotspotY = (hotspotY * maxH) / image.getHeight();
    }

    return IconConverters::createHICONFromImage (im, FALSE, hotspotX, hotspotY);
}

void MouseCursor::deleteMouseCursor (void* const cursorHandle, const bool isStandard)
{
    if (cursorHandle != nullptr && ! isStandard)
        DestroyCursor ((HCURSOR) cursorHandle);
}

enum
{
    hiddenMouseCursorHandle = 32500 // (arbitrary non-zero value to mark this type of cursor)
};

void* MouseCursor::createStandardMouseCursor (const MouseCursor::StandardCursorType type)
{
    LPCTSTR cursorName = IDC_ARROW;

    switch (type)
    {
        case NormalCursor:
        case ParentCursor:                  break;
        case NoCursor:                      return (void*) hiddenMouseCursorHandle;
        case WaitCursor:                    cursorName = IDC_WAIT; break;
        case IBeamCursor:                   cursorName = IDC_IBEAM; break;
        case PointingHandCursor:            cursorName = MAKEINTRESOURCE(32649); break;
        case CrosshairCursor:               cursorName = IDC_CROSS; break;
        case CopyingCursor:                 break; // can't seem to find one of these in the system list..

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
            static void* dragHandCursor = nullptr;

            if (dragHandCursor == nullptr)
            {
                static const unsigned char dragHandData[] =
                    { 71,73,70,56,57,97,16,0,16,0,145,2,0,0,0,0,255,255,255,0,0,0,0,0,0,33,249,4,1,0,0,2,0,44,0,0,0,0,16,0,
                      16,0,0,2,52,148,47,0,200,185,16,130,90,12,74,139,107,84,123,39,132,117,151,116,132,146,248,60,209,138,
                      98,22,203,114,34,236,37,52,77,217,247,154,191,119,110,240,193,128,193,95,163,56,60,234,98,135,2,0,59 };

                dragHandCursor = CustomMouseCursorInfo (ImageFileFormat::loadFrom (dragHandData, sizeof (dragHandData)), 8, 7).create();
            }

            return dragHandCursor;
        }

        default:
            jassertfalse; break;
    }

    HCURSOR cursorH = LoadCursor (0, cursorName);

    if (cursorH == 0)
        cursorH = LoadCursor (0, IDC_ARROW);

    return cursorH;
}

//==============================================================================
void MouseCursor::showInWindow (ComponentPeer*) const
{
    HCURSOR c = (HCURSOR) getHandle();

    if (c == 0)
        c = LoadCursor (0, IDC_ARROW);
    else if (c == (HCURSOR) hiddenMouseCursorHandle)
        c = 0;

    SetCursor (c);
}

void MouseCursor::showInAllWindows() const
{
    showInWindow (nullptr);
}
