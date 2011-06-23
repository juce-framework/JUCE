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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
#undef GetSystemMetrics // multimon overrides this for some reason and causes a mess..

// these are in the windows SDK, but need to be repeated here for GCC..
#ifndef GET_APPCOMMAND_LPARAM
  #define FAPPCOMMAND_MASK                  0xF000
  #define GET_APPCOMMAND_LPARAM(lParam)     ((short) (HIWORD (lParam) & ~FAPPCOMMAND_MASK))
  #define APPCOMMAND_MEDIA_NEXTTRACK        11
  #define APPCOMMAND_MEDIA_PREVIOUSTRACK    12
  #define APPCOMMAND_MEDIA_STOP             13
  #define APPCOMMAND_MEDIA_PLAY_PAUSE       14
#endif

extern void juce_repeatLastProcessPriority(); // in juce_win32_Threads.cpp
extern void juce_CheckCurrentlyFocusedTopLevelWindow();  // in juce_TopLevelWindow.cpp
extern bool juce_IsRunningInWine();

#ifndef ULW_ALPHA
  #define ULW_ALPHA     0x00000002
#endif

#ifndef AC_SRC_ALPHA
  #define AC_SRC_ALPHA  0x01
#endif

static bool shouldDeactivateTitleBar = true;

#define WM_TRAYNOTIFY WM_USER + 100

//==============================================================================
typedef BOOL (WINAPI* UpdateLayeredWinFunc) (HWND, HDC, POINT*, SIZE*, HDC, POINT*, COLORREF, BLENDFUNCTION*, DWORD);
static UpdateLayeredWinFunc updateLayeredWindow = 0;

bool Desktop::canUseSemiTransparentWindows() noexcept
{
    if (updateLayeredWindow == 0)
    {
        if (! juce_IsRunningInWine())
        {
            HMODULE user32Mod = GetModuleHandle (_T("user32.dll"));
            updateLayeredWindow = (UpdateLayeredWinFunc) GetProcAddress (user32Mod, "UpdateLayeredWindow");
        }
    }

    return updateLayeredWindow != 0;
}

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const
{
    return upright;
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
class WindowsBitmapImage  : public Image::SharedImage
{
public:
    //==============================================================================
    WindowsBitmapImage (const Image::PixelFormat format_,
                        const int w, const int h, const bool clearImage)
        : Image::SharedImage (format_, w, h)
    {
        jassert (format_ == Image::RGB || format_ == Image::ARGB);

        pixelStride = (format_ == Image::RGB) ? 3 : 4;
        lineStride = -((w * pixelStride + 3) & ~3);

        zerostruct (bitmapInfo);
        bitmapInfo.bV4Size = sizeof (BITMAPV4HEADER);
        bitmapInfo.bV4Width = w;
        bitmapInfo.bV4Height = h;
        bitmapInfo.bV4Planes = 1;
        bitmapInfo.bV4CSType = 1;
        bitmapInfo.bV4BitCount = (unsigned short) (pixelStride * 8);

        if (format_ == Image::ARGB)
        {
            bitmapInfo.bV4AlphaMask        = 0xff000000;
            bitmapInfo.bV4RedMask          = 0xff0000;
            bitmapInfo.bV4GreenMask        = 0xff00;
            bitmapInfo.bV4BlueMask         = 0xff;
            bitmapInfo.bV4V4Compression    = BI_BITFIELDS;
        }
        else
        {
            bitmapInfo.bV4V4Compression    = BI_RGB;
        }

        HDC dc = GetDC (0);
        hdc = CreateCompatibleDC (dc);
        ReleaseDC (0, dc);

        SetMapMode (hdc, MM_TEXT);

        hBitmap = CreateDIBSection (hdc, (BITMAPINFO*) &(bitmapInfo), DIB_RGB_COLORS,
                                    (void**) &bitmapData, 0, 0);

        previousBitmap = SelectObject (hdc, hBitmap);

        if (format_ == Image::ARGB && clearImage)
            zeromem (bitmapData, std::abs (h * lineStride));

        imageData = bitmapData - (lineStride * (h - 1));
    }

    ~WindowsBitmapImage()
    {
        SelectObject (hdc, previousBitmap); // Selecting the previous bitmap before deleting the DC avoids a warning in BoundsChecker
        DeleteDC (hdc);
        DeleteObject (hBitmap);
    }

    Image::ImageType getType() const    { return Image::NativeImage; }

    LowLevelGraphicsContext* createLowLevelContext()
    {
        return new LowLevelGraphicsSoftwareRenderer (Image (this));
    }

    void initialiseBitmapData (Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode /*mode*/)
    {
        bitmap.data = imageData + x * pixelStride + y * lineStride;
        bitmap.pixelFormat = format;
        bitmap.lineStride = lineStride;
        bitmap.pixelStride = pixelStride;
    }

    Image::SharedImage* clone()
    {
        WindowsBitmapImage* im = new WindowsBitmapImage (format, width, height, false);

        for (int i = 0; i < height; ++i)
            memcpy (im->imageData + i * lineStride, imageData + i * lineStride, lineStride);

        return im;
    }

    void blitToWindow (HWND hwnd, HDC dc, const bool transparent,
                       const int x, const int y,
                       const RectangleList& maskedRegion,
                       const uint8 updateLayeredWindowAlpha) noexcept
    {
        static HDRAWDIB hdd = 0;
        static bool needToCreateDrawDib = true;

        if (needToCreateDrawDib)
        {
            needToCreateDrawDib = false;

            HDC dc = GetDC (0);
            const int n = GetDeviceCaps (dc, BITSPIXEL);
            ReleaseDC (0, dc);

            // only open if we're not palettised
            if (n > 8)
                hdd = DrawDibOpen();
        }

        SetMapMode (dc, MM_TEXT);

        if (transparent)
        {
            if (! maskedRegion.isEmpty())
            {
                for (RectangleList::Iterator i (maskedRegion); i.next();)
                {
                    const Rectangle<int>& r = *i.getRectangle();
                    ExcludeClipRect (hdc, r.getX(), r.getY(), r.getRight(), r.getBottom());
                }
            }

            RECT windowBounds;
            GetWindowRect (hwnd, &windowBounds);

            POINT p = { -x, -y };
            POINT pos = { windowBounds.left, windowBounds.top };
            SIZE size = { windowBounds.right - windowBounds.left,
                          windowBounds.bottom - windowBounds.top };

            BLENDFUNCTION bf;
            bf.AlphaFormat = AC_SRC_ALPHA;
            bf.BlendFlags = 0;
            bf.BlendOp = AC_SRC_OVER;
            bf.SourceConstantAlpha = updateLayeredWindowAlpha;

            updateLayeredWindow (hwnd, 0, &pos, &size, hdc, &p, 0, &bf, ULW_ALPHA);
        }
        else
        {
            int savedDC = 0;

            if (! maskedRegion.isEmpty())
            {
                savedDC = SaveDC (dc);

                for (RectangleList::Iterator i (maskedRegion); i.next();)
                {
                    const Rectangle<int>& r = *i.getRectangle();
                    ExcludeClipRect (dc, r.getX(), r.getY(), r.getRight(), r.getBottom());
                }
            }

            if (hdd == 0)
            {
                StretchDIBits (dc,
                               x, y, width, height,
                               0, 0, width, height,
                               bitmapData, (const BITMAPINFO*) &bitmapInfo,
                               DIB_RGB_COLORS, SRCCOPY);
            }
            else
            {
                DrawDibDraw (hdd, dc, x, y, -1, -1,
                             (BITMAPINFOHEADER*) &bitmapInfo, bitmapData,
                             0, 0, width, height, 0);
            }

            if (! maskedRegion.isEmpty())
                RestoreDC (dc, savedDC);
        }
    }

    //==============================================================================
    HBITMAP hBitmap;
    HGDIOBJ previousBitmap;
    BITMAPV4HEADER bitmapInfo;
    HDC hdc;
    uint8* bitmapData;
    int pixelStride, lineStride;
    uint8* imageData;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsBitmapImage);
};

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
            Image mask (createImageFromHBITMAP (info.hbmMask));
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
        info.xHotspot = hotspotX;
        info.yHotspot = hotspotY;
        info.hbmMask = mask;
        info.hbmColor = nativeBitmap->hBitmap;

        HICON hi = CreateIconIndirect (&info);
        DeleteObject (mask);
        return hi;
    }
}

//==============================================================================
class Win32ComponentPeer  : public ComponentPeer
{
public:
    enum RenderingEngineType
    {
        softwareRenderingEngine = 0,
        direct2DRenderingEngine
    };

    //==============================================================================
    Win32ComponentPeer (Component* const component,
                        const int windowStyleFlags,
                        HWND parentToAddTo_)
        : ComponentPeer (component, windowStyleFlags),
          dontRepaint (false),
          currentRenderingEngine (softwareRenderingEngine),
          fullScreen (false),
          isDragging (false),
          isMouseOver (false),
          hasCreatedCaret (false),
          constrainerIsResizing (false),
          currentWindowIcon (0),
          dropTarget (nullptr),
          parentToAddTo (parentToAddTo_),
          updateLayeredWindowAlpha (255)
    {
        callFunctionIfNotLocked (&createWindowCallback, this);

        setTitle (component->getName());

        if ((windowStyleFlags & windowHasDropShadow) != 0
             && Desktop::canUseSemiTransparentWindows())
        {
            shadower = component->getLookAndFeel().createDropShadowerForComponent (component);

            if (shadower != nullptr)
                shadower->setOwner (component);
        }
    }

    ~Win32ComponentPeer()
    {
        setTaskBarIcon (Image());
        shadower = nullptr;

        // do this before the next bit to avoid messages arriving for this window
        // before it's destroyed
        JuceWindowIdentifier::setAsJUCEWindow (hwnd, false);

        callFunctionIfNotLocked (&destroyWindowCallback, (void*) hwnd);

        if (currentWindowIcon != 0)
            DestroyIcon (currentWindowIcon);

        if (dropTarget != nullptr)
        {
            dropTarget->Release();
            dropTarget = nullptr;
        }

      #if JUCE_DIRECT2D
        direct2DContext = nullptr;
      #endif
    }

    //==============================================================================
    void* getNativeHandle() const
    {
        return hwnd;
    }

    void setVisible (bool shouldBeVisible)
    {
        ShowWindow (hwnd, shouldBeVisible ? SW_SHOWNA : SW_HIDE);

        if (shouldBeVisible)
            InvalidateRect (hwnd, 0, 0);
        else
            lastPaintTime = 0;
    }

    void setTitle (const String& title)
    {
        SetWindowText (hwnd, title.toWideCharPointer());
    }

    void setPosition (int x, int y)
    {
        offsetWithinParent (x, y);
        SetWindowPos (hwnd, 0,
                      x - windowBorder.getLeft(),
                      y - windowBorder.getTop(),
                      0, 0,
                      SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
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
        {
            windowBorder = BorderSize<int> (info.rcClient.top - info.rcWindow.top,
                                            info.rcClient.left - info.rcWindow.left,
                                            info.rcWindow.bottom - info.rcClient.bottom,
                                            info.rcWindow.right - info.rcClient.right);
        }

      #if JUCE_DIRECT2D
        if (direct2DContext != nullptr)
            direct2DContext->resized();
      #endif
    }

    void setSize (int w, int h)
    {
        SetWindowPos (hwnd, 0, 0, 0,
                      w + windowBorder.getLeftAndRight(),
                      h + windowBorder.getTopAndBottom(),
                      SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);

        updateBorderSize();

        repaintNowIfTransparent();
    }

    void setBounds (int x, int y, int w, int h, bool isNowFullScreen)
    {
        fullScreen = isNowFullScreen;
        offsetWithinParent (x, y);

        SetWindowPos (hwnd, 0,
                      x - windowBorder.getLeft(),
                      y - windowBorder.getTop(),
                      w + windowBorder.getLeftAndRight(),
                      h + windowBorder.getTopAndBottom(),
                      SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);

        updateBorderSize();

        repaintNowIfTransparent();
    }

    const Rectangle<int> getBounds() const
    {
        RECT r;
        GetWindowRect (hwnd, &r);

        Rectangle<int> bounds (r.left, r.top, r.right - r.left, r.bottom - r.top);

        HWND parentH = GetParent (hwnd);
        if (parentH != 0)
        {
            GetWindowRect (parentH, &r);
            bounds.translate (-r.left, -r.top);
        }

        return windowBorder.subtractedFrom (bounds);
    }

    const Point<int> getScreenPosition() const
    {
        RECT r;
        GetWindowRect (hwnd, &r);
        return Point<int> (r.left + windowBorder.getLeft(),
                           r.top + windowBorder.getTop());
    }

    const Point<int> localToGlobal (const Point<int>& relativePosition)
    {
        return relativePosition + getScreenPosition();
    }

    const Point<int> globalToLocal (const Point<int>& screenPosition)
    {
        return screenPosition - getScreenPosition();
    }

    void setAlpha (float newAlpha)
    {
        const uint8 intAlpha = (uint8) jlimit (0, 255, (int) (newAlpha * 255.0f));

        if (component->isOpaque())
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
            component->repaint();
        }
    }

    void setMinimised (bool shouldBeMinimised)
    {
        if (shouldBeMinimised != isMinimised())
            ShowWindow (hwnd, shouldBeMinimised ? SW_MINIMIZE : SW_SHOWNORMAL);
    }

    bool isMinimised() const
    {
        WINDOWPLACEMENT wp;
        wp.length = sizeof (WINDOWPLACEMENT);
        GetWindowPlacement (hwnd, &wp);

        return wp.showCmd == SW_SHOWMINIMIZED;
    }

    void setFullScreen (bool shouldBeFullScreen)
    {
        setMinimised (false);

        if (isFullScreen() != shouldBeFullScreen)
        {
            fullScreen = shouldBeFullScreen;
            const WeakReference<Component> deletionChecker (component);

            if (! fullScreen)
            {
                const Rectangle<int> boundsCopy (lastNonFullscreenBounds);

                if (hasTitleBar())
                    ShowWindow (hwnd, SW_SHOWNORMAL);

                if (! boundsCopy.isEmpty())
                {
                    setBounds (boundsCopy.getX(),
                               boundsCopy.getY(),
                               boundsCopy.getWidth(),
                               boundsCopy.getHeight(),
                               false);
                }
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

    bool isFullScreen() const
    {
        if (! hasTitleBar())
            return fullScreen;

        WINDOWPLACEMENT wp;
        wp.length = sizeof (wp);
        GetWindowPlacement (hwnd, &wp);

        return wp.showCmd == SW_SHOWMAXIMIZED;
    }

    bool contains (const Point<int>& position, bool trueIfInAChildWindow) const
    {
        if (! (isPositiveAndBelow (position.getX(), component->getWidth())
                && isPositiveAndBelow (position.getY(), component->getHeight())))
            return false;

        RECT r;
        GetWindowRect (hwnd, &r);

        POINT p = { position.getX() + r.left + windowBorder.getLeft(),
                    position.getY() + r.top  + windowBorder.getTop() };

        HWND w = WindowFromPoint (p);
        return w == hwnd || (trueIfInAChildWindow && (IsChild (hwnd, w) != 0));
    }

    const BorderSize<int> getFrameSize() const
    {
        return windowBorder;
    }

    bool setAlwaysOnTop (bool alwaysOnTop)
    {
        const bool oldDeactivate = shouldDeactivateTitleBar;
        shouldDeactivateTitleBar = ((styleFlags & windowIsTemporary) == 0);

        SetWindowPos (hwnd, alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                      0, 0, 0, 0,
                      SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);

        shouldDeactivateTitleBar = oldDeactivate;

        if (shadower != nullptr)
            shadower->componentBroughtToFront (*component);

        return true;
    }

    void toFront (bool makeActive)
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

    void toBehind (ComponentPeer* other)
    {
        Win32ComponentPeer* const otherPeer = dynamic_cast <Win32ComponentPeer*> (other);

        jassert (otherPeer != nullptr); // wrong type of window?

        if (otherPeer != nullptr)
        {
            setMinimised (false);

            // must be careful not to try to put a topmost window behind a normal one, or win32
            // promotes the normal one to be topmost!
            if (getComponent()->isAlwaysOnTop() == otherPeer->getComponent()->isAlwaysOnTop())
                SetWindowPos (hwnd, otherPeer->hwnd, 0, 0, 0, 0,
                              SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
            else if (otherPeer->getComponent()->isAlwaysOnTop())
                SetWindowPos (hwnd, HWND_TOP, 0, 0, 0, 0,
                              SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
        }
    }

    bool isFocused() const
    {
        return callFunctionIfNotLocked (&getFocusCallback, 0) == (void*) hwnd;
    }

    void grabFocus()
    {
        const bool oldDeactivate = shouldDeactivateTitleBar;
        shouldDeactivateTitleBar = ((styleFlags & windowIsTemporary) == 0);

        callFunctionIfNotLocked (&setFocusCallback, hwnd);

        shouldDeactivateTitleBar = oldDeactivate;
    }

    void textInputRequired (const Point<int>&)
    {
        if (! hasCreatedCaret)
        {
            hasCreatedCaret = true;
            CreateCaret (hwnd, (HBITMAP) 1, 0, 0);
        }

        ShowCaret (hwnd);
        SetCaretPos (0, 0);
    }

    void dismissPendingTextInput()
    {
        imeHandler.handleSetContext (hwnd, false);
    }

    void repaint (const Rectangle<int>& area)
    {
        const RECT r = { area.getX(), area.getY(), area.getRight(), area.getBottom() };
        InvalidateRect (hwnd, &r, FALSE);
    }

    void performAnyPendingRepaintsNow()
    {
        MSG m;
        if (component->isVisible()
             && (PeekMessage (&m, hwnd, WM_PAINT, WM_PAINT, PM_REMOVE) || isUsingUpdateLayeredWindow()))
            handlePaintMessage();
    }

    //==============================================================================
    static Win32ComponentPeer* getOwnerOfWindow (HWND h) noexcept
    {
        if (h != 0 && JuceWindowIdentifier::isJUCEWindow (h))
            return (Win32ComponentPeer*) (pointer_sized_int) GetWindowLongPtr (h, 8);

        return nullptr;
    }

    //==============================================================================
    void setTaskBarIcon (const Image& image)
    {
        if (image.isValid())
        {
            HICON hicon = IconConverters::createHICONFromImage (image, TRUE, 0, 0);

            if (taskBarIcon == 0)
            {
                taskBarIcon = new NOTIFYICONDATA();
                zeromem (taskBarIcon, sizeof (NOTIFYICONDATA));
                taskBarIcon->cbSize = sizeof (NOTIFYICONDATA);
                taskBarIcon->hWnd = (HWND) hwnd;
                taskBarIcon->uID = (int) (pointer_sized_int) hwnd;
                taskBarIcon->uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
                taskBarIcon->uCallbackMessage = WM_TRAYNOTIFY;
                taskBarIcon->hIcon = hicon;
                taskBarIcon->szTip[0] = 0;

                Shell_NotifyIcon (NIM_ADD, taskBarIcon);
            }
            else
            {
                HICON oldIcon = taskBarIcon->hIcon;

                taskBarIcon->hIcon = hicon;
                taskBarIcon->uFlags = NIF_ICON;
                Shell_NotifyIcon (NIM_MODIFY, taskBarIcon);

                DestroyIcon (oldIcon);
            }
        }
        else if (taskBarIcon != 0)
        {
            taskBarIcon->uFlags = 0;
            Shell_NotifyIcon (NIM_DELETE, taskBarIcon);
            DestroyIcon (taskBarIcon->hIcon);
            taskBarIcon = nullptr;
        }
    }

    void setTaskBarIconToolTip (const String& toolTip) const
    {
        if (taskBarIcon != 0)
        {
            taskBarIcon->uFlags = NIF_TIP;
            toolTip.copyToUTF16 (taskBarIcon->szTip, sizeof (taskBarIcon->szTip) - 1);
            Shell_NotifyIcon (NIM_MODIFY, taskBarIcon);
        }
    }

    void handleTaskBarEvent (const LPARAM lParam)
    {
        if (component->isCurrentlyBlockedByAnotherModalComponent())
        {
            if (lParam == WM_LBUTTONDOWN || lParam == WM_RBUTTONDOWN
                 || lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONDBLCLK)
            {
                Component* const current = Component::getCurrentlyModalComponent();

                if (current != nullptr)
                    current->inputAttemptWhenModal();
            }
        }
        else
        {
            ModifierKeys eventMods (ModifierKeys::getCurrentModifiersRealtime());

            if (lParam == WM_LBUTTONDOWN || lParam == WM_LBUTTONDBLCLK)
                eventMods = eventMods.withFlags (ModifierKeys::leftButtonModifier);
            else if (lParam == WM_RBUTTONDOWN || lParam == WM_RBUTTONDBLCLK)
                eventMods = eventMods.withFlags (ModifierKeys::rightButtonModifier);
            else if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
                eventMods = eventMods.withoutMouseButtons();

            const MouseEvent e (Desktop::getInstance().getMainMouseSource(),
                                Point<int>(), eventMods, component, component, Time (getMouseEventTime()),
                                Point<int>(), Time (getMouseEventTime()), 1, false);

            if (lParam == WM_LBUTTONDOWN || lParam == WM_RBUTTONDOWN)
            {
                SetFocus (hwnd);
                SetForegroundWindow (hwnd);
                component->mouseDown (e);
            }
            else if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
            {
                component->mouseUp (e);
            }
            else if (lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONDBLCLK)
            {
                component->mouseDoubleClick (e);
            }
            else if (lParam == WM_MOUSEMOVE)
            {
                component->mouseMove (e);
            }
        }
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

    static int64 getMouseEventTime()
    {
        static int64 eventTimeOffset = 0;
        static DWORD lastMessageTime = 0;
        const DWORD thisMessageTime = GetMessageTime();

        if (thisMessageTime < lastMessageTime || lastMessageTime == 0)
        {
            lastMessageTime = thisMessageTime;
            eventTimeOffset = Time::currentTimeMillis() - thisMessageTime;
        }

        return eventTimeOffset + thisMessageTime;
    }

    //==============================================================================
    bool dontRepaint;

    static ModifierKeys currentModifiers;
    static ModifierKeys modifiersAtLastCallback;

private:
    HWND hwnd, parentToAddTo;
    ScopedPointer<DropShadower> shadower;
    RenderingEngineType currentRenderingEngine;
   #if JUCE_DIRECT2D
    ScopedPointer<Direct2DLowLevelGraphicsContext> direct2DContext;
   #endif
    bool fullScreen, isDragging, isMouseOver, hasCreatedCaret, constrainerIsResizing;
    BorderSize<int> windowBorder;
    HICON currentWindowIcon;
    ScopedPointer<NOTIFYICONDATA> taskBarIcon;
    IDropTarget* dropTarget;
    uint8 updateLayeredWindowAlpha;

    //==============================================================================
    class TemporaryImage    : public Timer
    {
    public:
        TemporaryImage() {}

        const Image& getImage (const bool transparent, const int w, const int h)
        {
            const Image::PixelFormat format = transparent ? Image::ARGB : Image::RGB;

            if ((! image.isValid()) || image.getWidth() < w || image.getHeight() < h || image.getFormat() != format)
                image = Image (new WindowsBitmapImage (format, (w + 31) & ~31, (h + 31) & ~31, false));

            startTimer (3000);
            return image;
        }

        void timerCallback()
        {
            stopTimer();
            image = Image::null;
        }

    private:
        Image image;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemporaryImage);
    };

    TemporaryImage offscreenImageGenerator;

    //==============================================================================
    class WindowClassHolder    : public DeletedAtShutdown
    {
    public:
        WindowClassHolder()
        {
            // this name has to be different for each app/dll instance because otherwise poor old Win32 can
            // get a bit confused (even despite it not being a process-global window class).
            String windowClassName ("JUCE_");
            windowClassName << (int) (Time::currentTimeMillis() & 0x7fffffff);

            HINSTANCE moduleHandle = (HINSTANCE) PlatformUtilities::getCurrentModuleInstanceHandle();

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
        }

        ~WindowClassHolder()
        {
            if (ComponentPeer::getNumPeers() == 0)
                UnregisterClass (getWindowClassName(), (HINSTANCE) PlatformUtilities::getCurrentModuleInstanceHandle());

            clearSingletonInstance();
        }

        LPCTSTR getWindowClassName() const noexcept     { return (LPCTSTR) MAKELONG (atom, 0); }

        juce_DeclareSingleton_SingleThreaded_Minimal (WindowClassHolder);

    private:
        ATOM atom;

        JUCE_DECLARE_NON_COPYABLE (WindowClassHolder);
    };

    //==============================================================================
    static void* createWindowCallback (void* userData)
    {
        static_cast <Win32ComponentPeer*> (userData)->createWindow();
        return nullptr;
    }

    void createWindow()
    {
        DWORD exstyle = WS_EX_ACCEPTFILES;
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

        if ((styleFlags & windowHasMinimiseButton) != 0)
            type |= WS_MINIMIZEBOX;

        if ((styleFlags & windowHasMaximiseButton) != 0)
            type |= WS_MAXIMIZEBOX;

        if ((styleFlags & windowIgnoresMouseClicks) != 0)
            exstyle |= WS_EX_TRANSPARENT;

        if ((styleFlags & windowIsSemiTransparent) != 0 && Desktop::canUseSemiTransparentWindows())
            exstyle |= WS_EX_LAYERED;

        hwnd = CreateWindowEx (exstyle, WindowClassHolder::getInstance()->getWindowClassName(),
                               L"", type, 0, 0, 0, 0, parentToAddTo, 0,
                               (HINSTANCE) PlatformUtilities::getCurrentModuleInstanceHandle(), 0);

       #if JUCE_DIRECT2D
        setCurrentRenderingEngine (1);
       #endif

        if (hwnd != 0)
        {
            SetWindowLongPtr (hwnd, 0, 0);
            SetWindowLongPtr (hwnd, 8, (LONG_PTR) this);
            JuceWindowIdentifier::setAsJUCEWindow (hwnd, true);

            if (dropTarget == nullptr)
                dropTarget = new JuceDropTarget (this);

            RegisterDragDrop (hwnd, dropTarget);

            updateBorderSize();

            // Calling this function here is (for some reason) necessary to make Windows
            // correctly enable the menu items that we specify in the wm_initmenu message.
            GetSystemMenu (hwnd, false);

            const float alpha = component->getAlpha();
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
        SetWindowPos ((HWND) h, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
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

    void offsetWithinParent (int& x, int& y) const
    {
        if (isUsingUpdateLayeredWindow())
        {
            HWND parentHwnd = GetParent (hwnd);

            if (parentHwnd != 0)
            {
                RECT parentRect;
                GetWindowRect (parentHwnd, &parentRect);
                x += parentRect.left;
                y += parentRect.top;
            }
        }
    }

    bool isUsingUpdateLayeredWindow() const
    {
        return ! component->isOpaque();
    }

    inline bool hasTitleBar() const noexcept        { return (styleFlags & windowHasTitleBar) != 0; }


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
                direct2DContext->clipToRectangle (Rectangle<int> (r.left, r.top, r.right - r.left, r.bottom - r.top));
                handlePaint (*direct2DContext);
                direct2DContext->end();
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
            if (reentrant)
            {
                DeleteObject (rgn);
                EndPaint (hwnd, &paintStruct);
                return;
            }

            const ScopedValueSetter<bool> setter (reentrant, true, false);

            // this is the rectangle to update..
            int x = paintStruct.rcPaint.left;
            int y = paintStruct.rcPaint.top;
            int w = paintStruct.rcPaint.right - x;
            int h = paintStruct.rcPaint.bottom - y;

            const bool transparent = isUsingUpdateLayeredWindow();

            if (transparent)
            {
                // it's not possible to have a transparent window with a title bar at the moment!
                jassert (! hasTitleBar());

                RECT r;
                GetWindowRect (hwnd, &r);
                x = y = 0;
                w = r.right - r.left;
                h = r.bottom - r.top;
            }

            if (w > 0 && h > 0)
            {
                clearMaskedRegion();

                Image offscreenImage (offscreenImageGenerator.getImage (transparent, w, h));

                RectangleList contextClip;
                const Rectangle<int> clipBounds (0, 0, w, h);

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
                            int num = ((RGNDATA*) rgnData)->rdh.nCount;

                            while (--num >= 0)
                            {
                                if (rects->right <= x + w && rects->bottom <= y + h)
                                {
                                    const int cx = jmax (x, (int) rects->left);
                                    contextClip.addWithoutMerging (Rectangle<int> (cx - x, rects->top - y, rects->right - cx, rects->bottom - rects->top)
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

                if (transparent)
                {
                    RectangleList::Iterator i (contextClip);

                    while (i.next())
                        offscreenImage.clear (*i.getRectangle());
                }

                // if the component's not opaque, this won't draw properly unless the platform can support this
                jassert (Desktop::canUseSemiTransparentWindows() || component->isOpaque());

                updateCurrentModifiers();

                LowLevelGraphicsSoftwareRenderer context (offscreenImage, -x, -y, contextClip);
                handlePaint (context);

                if (! dontRepaint)
                    static_cast <WindowsBitmapImage*> (offscreenImage.getSharedImage())
                        ->blitToWindow (hwnd, dc, transparent, x, y, maskedRegion, updateLayeredWindowAlpha);
            }

            DeleteObject (rgn);
            EndPaint (hwnd, &paintStruct);
        }

       #ifndef JUCE_GCC
        _fpreset(); // because some graphics cards can unmask FP exceptions
       #endif

        lastPaintTime = Time::getMillisecondCounter();
    }

    //==============================================================================
    void doMouseEvent (const Point<int>& position)
    {
        handleMouseEvent (0, position, currentModifiers, getMouseEventTime());
    }

    StringArray getAvailableRenderingEngines()
    {
        StringArray s (ComponentPeer::getAvailableRenderingEngines());

       #if JUCE_DIRECT2D
        if (SystemStats::getOperatingSystemType() >= SystemStats::Windows7)
            s.add ("Direct2D");
       #endif

        return s;
    }

    int getCurrentRenderingEngine() const    { return currentRenderingEngine; }

   #if JUCE_DIRECT2D
    void updateDirect2DContext()
    {
        if (currentRenderingEngine != direct2DRenderingEngine)
            direct2DContext = 0;
        else if (direct2DContext == 0)
            direct2DContext = new Direct2DLowLevelGraphicsContext (hwnd);
    }
   #endif

    void setCurrentRenderingEngine (int index)
    {
        (void) index;

       #if JUCE_DIRECT2D
        if (getAvailableRenderingEngines().size() > 1)
        {
            currentRenderingEngine = index == 1 ? direct2DRenderingEngine : softwareRenderingEngine;
            updateDirect2DContext();
            repaint (component->getLocalBounds());
        }
       #endif
    }

    static int getMinTimeBetweenMouseMoves()
    {
        if (SystemStats::getOperatingSystemType() >= SystemStats::WinVista)
            return 0;

        return 1000 / 60;  // Throttling the incoming mouse-events seems to still be needed in XP..
    }

    void doMouseMove (const Point<int>& position)
    {
        if (! isMouseOver)
        {
            isMouseOver = true;
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

    void doMouseDown (const Point<int>& position, const WPARAM wParam)
    {
        if (GetCapture() != hwnd)
            SetCapture (hwnd);

        doMouseMove (position);

        updateModifiersFromWParam (wParam);
        isDragging = true;

        doMouseEvent (position);
    }

    void doMouseUp (const Point<int>& position, const WPARAM wParam)
    {
        updateModifiersFromWParam (wParam);
        isDragging = false;

        // release the mouse capture if the user has released all buttons
        if ((wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON)) == 0 && hwnd == GetCapture())
            ReleaseCapture();

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

    void doMouseWheel (const Point<int>& globalPos, const WPARAM wParam, const bool isVertical)
    {
        updateKeyModifiers();
        const float amount = jlimit (-1000.0f, 1000.0f, 0.75f * (short) HIWORD (wParam));

        // Because win32 stupidly sends all wheel events to the window with the keyboard
        // focus, we have to redirect them here according to the mouse pos..
        POINT p = { globalPos.getX(), globalPos.getY() };
        Win32ComponentPeer* peer = getOwnerOfWindow (WindowFromPoint (p));

        if (peer == nullptr)
            peer = this;

        peer->handleMouseWheel (0, peer->globalToLocal (globalPos), getMouseEventTime(),
                                isVertical ? 0.0f : -amount,
                                isVertical ? amount : 0.0f);
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

            case VK_ADD:
            case VK_SUBTRACT:
            case VK_MULTIPLY:
            case VK_DIVIDE:
            case VK_SEPARATOR:
            case VK_DECIMAL:
                used = handleKeyUpOrDown (true);
                break;

            default:
                used = handleKeyUpOrDown (true);

                {
                    MSG msg;

                    if (! PeekMessage (&msg, hwnd, WM_CHAR, WM_DEADCHAR, PM_NOREMOVE))
                    {
                        // if there isn't a WM_CHAR or WM_DEADCHAR message pending, we need to
                        // manually generate the key-press event that matches this key-down.

                        const UINT keyChar = MapVirtualKey (key, 2);
                        used = handleKeyPress ((int) LOWORD (keyChar), 0) || used;
                    }
                }

                break;
        }

        if (Component::getCurrentlyModalComponent() != nullptr)
            used = true;

        return used;
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
            const UINT virtualKey = MapVirtualKey (virtualScanCode, 1);
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
        HWND parentH = GetParent (hwnd);
        if (parentH != 0)
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
                && (styleFlags & (windowHasTitleBar | windowIsResizable)) == (windowHasTitleBar | windowIsResizable);
    }

    LRESULT handleSizeConstraining (RECT* const r, const WPARAM wParam)
    {
        if (isConstrainedNativeWindow())
        {
            Rectangle<int> pos (r->left, r->top, r->right - r->left, r->bottom - r->top);

            constrainer->checkBounds (pos, windowBorder.addedTo (component->getBounds()),
                                      Desktop::getInstance().getAllMonitorDisplayAreas().getBounds(),
                                      wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT,
                                      wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT,
                                      wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT,
                                      wParam == WMSZ_RIGHT || wParam == WMSZ_TOPRIGHT || wParam == WMSZ_BOTTOMRIGHT);
            r->left = pos.getX();
            r->top = pos.getY();
            r->right = pos.getRight();
            r->bottom = pos.getBottom();
        }

        return TRUE;
    }

    LRESULT handlePositionChanging (WINDOWPOS* const wp)
    {
        if (isConstrainedNativeWindow())
        {
            if ((wp->flags & (SWP_NOMOVE | SWP_NOSIZE)) != (SWP_NOMOVE | SWP_NOSIZE)
                 && ! Component::isMouseButtonDownAnywhere())
            {
                Rectangle<int> pos (wp->x, wp->y, wp->cx, wp->cy);
                const Rectangle<int> current (windowBorder.addedTo (component->getBounds()));

                constrainer->checkBounds (pos, current,
                                          Desktop::getInstance().getAllMonitorDisplayAreas().getBounds(),
                                          pos.getY() != current.getY() && pos.getBottom() == current.getBottom(),
                                          pos.getX() != current.getX() && pos.getRight() == current.getRight(),
                                          pos.getY() == current.getY() && pos.getBottom() != current.getBottom(),
                                          pos.getX() == current.getX() && pos.getRight() != current.getRight());
                wp->x = pos.getX();
                wp->y = pos.getY();
                wp->cx = pos.getWidth();
                wp->cy = pos.getHeight();
            }
        }

        return 0;
    }

    void handleAppActivation (const WPARAM wParam)
    {
        modifiersAtLastCallback = -1;
        updateKeyModifiers();

        if (isMinimised())
        {
            component->repaint();
            handleMovedOrResized();

            if (! ComponentPeer::isValidPeer (this))
                return;
        }

        Component* underMouse = component->getComponentAt (component->getMouseXYRelative());

        if (underMouse == nullptr)
            underMouse = component;

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

    //==============================================================================
    class JuceDropTarget    : public ComBaseClassHelper <IDropTarget>
    {
    public:
        JuceDropTarget (Win32ComponentPeer* const owner_)
            : owner (owner_)
        {
        }

        HRESULT __stdcall DragEnter (IDataObject* pDataObject, DWORD /*grfKeyState*/, POINTL mousePos, DWORD* pdwEffect)
        {
            updateFileList (pDataObject);
            owner->handleFileDragMove (files, owner->globalToLocal (Point<int> (mousePos.x, mousePos.y)));
            *pdwEffect = DROPEFFECT_COPY;
            return S_OK;
        }

        HRESULT __stdcall DragLeave()
        {
            owner->handleFileDragExit (files);
            return S_OK;
        }

        HRESULT __stdcall DragOver (DWORD /*grfKeyState*/, POINTL mousePos, DWORD* pdwEffect)
        {
            owner->handleFileDragMove (files, owner->globalToLocal (Point<int> (mousePos.x, mousePos.y)));
            *pdwEffect = DROPEFFECT_COPY;
            return S_OK;
        }

        HRESULT __stdcall Drop (IDataObject* pDataObject, DWORD /*grfKeyState*/, POINTL mousePos, DWORD* pdwEffect)
        {
            updateFileList (pDataObject);
            owner->handleFileDragDrop (files, owner->globalToLocal (Point<int> (mousePos.x, mousePos.y)));
            *pdwEffect = DROPEFFECT_COPY;
            return S_OK;
        }

    private:
        Win32ComponentPeer* const owner;
        StringArray files;

        void updateFileList (IDataObject* const pDataObject)
        {
            files.clear();

            FORMATETC format = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
            STGMEDIUM medium = { TYMED_HGLOBAL, { 0 }, 0 };

            if (pDataObject->GetData (&format, &medium) == S_OK)
            {
                const SIZE_T totalLen = GlobalSize (medium.hGlobal);
                const LPDROPFILES pDropFiles = (const LPDROPFILES) GlobalLock (medium.hGlobal);
                unsigned int i = 0;

                if (pDropFiles->fWide)
                {
                    const WCHAR* const fname = (WCHAR*) addBytesToPointer (pDropFiles, sizeof (DROPFILES));

                    for (;;)
                    {
                        unsigned int len = 0;
                        while (i + len < totalLen && fname [i + len] != 0)
                            ++len;

                        if (len == 0)
                            break;

                        files.add (String (fname + i, len));
                        i += len + 1;
                    }
                }
                else
                {
                    const char* const fname = (const char*) addBytesToPointer (pDropFiles, sizeof (DROPFILES));

                    for (;;)
                    {
                        unsigned int len = 0;
                        while (i + len < totalLen && fname [i + len] != 0)
                            ++len;

                        if (len == 0)
                            break;

                        files.add (String (fname + i, len));
                        i += len + 1;
                    }
                }

                GlobalUnlock (medium.hGlobal);
            }
        }

        JUCE_DECLARE_NON_COPYABLE (JuceDropTarget);
    };

    void doSettingChange()
    {
        Desktop::getInstance().refreshMonitorSizes();

        if (fullScreen && ! isMinimised())
        {
            const Rectangle<int> r (component->getParentMonitorArea());

            SetWindowPos (hwnd, 0, r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                          SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOSENDCHANGING);
        }
    }

    //==============================================================================
public:
    static LRESULT CALLBACK windowProc (HWND h, UINT message, WPARAM wParam, LPARAM lParam)
    {
        Win32ComponentPeer* const peer = getOwnerOfWindow (h);

        if (peer != nullptr)
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
        else
            return MessageManager::getInstance()->callFunctionOnMessageThread (callback, userData);
    }

    static const Point<int> getPointFromLParam (LPARAM lParam) noexcept
    {
        return Point<int> (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
    }

    static const Point<int> getCurrentMousePosGlobal() noexcept
    {
        const DWORD mp = GetMessagePos();
        return Point<int> (GET_X_LPARAM (mp),
                           GET_Y_LPARAM (mp));
    }

    const Point<int> getCurrentMousePos() noexcept
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
                else if (! hasTitleBar())
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
            case WM_MOUSEMOVE:
                doMouseMove (getPointFromLParam (lParam));
                return 0;

            case WM_MOUSELEAVE:
                doMouseExit();
                return 0;

            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
                doMouseDown (getPointFromLParam (lParam), wParam);
                return 0;

            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
                doMouseUp (getPointFromLParam (lParam), wParam);
                return 0;

            case WM_CAPTURECHANGED:
                doCaptureChanged();
                return 0;

            case WM_NCMOUSEMOVE:
                if (hasTitleBar())
                    break;

                return 0;

            case 0x020A: /* WM_MOUSEWHEEL */
            case 0x020E: /* WM_MOUSEHWHEEL */
                doMouseWheel (getCurrentMousePosGlobal(), wParam, message == 0x020A);
                return 0;

            //==============================================================================
            case WM_SIZING:
                return handleSizeConstraining ((RECT*) lParam, wParam);

            case WM_WINDOWPOSCHANGING:
                return handlePositionChanging ((WINDOWPOS*) lParam);

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
                else
                    forwardMessageToParent (message, wParam, lParam);

                break;

            case WM_KEYUP:
            case WM_SYSKEYUP:
                if (doKeyUp (wParam))
                    return 0;
                else
                    forwardMessageToParent (message, wParam, lParam);

                break;

            case WM_CHAR:
                if (doKeyChar ((int) wParam, lParam))
                    return 0;
                else
                    forwardMessageToParent (message, wParam, lParam);

                break;

            case WM_APPCOMMAND:
                if (doAppCommand (lParam))
                    return TRUE;

                break;

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

                juce_CheckCurrentlyFocusedTopLevelWindow();
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
                if (! component->getMouseClickGrabsKeyboardFocus())
                    return MA_NOACTIVATE;

                break;

            case WM_SHOWWINDOW:
                if (wParam != 0)
                    handleBroughtToFront();

                break;

            case WM_CLOSE:
                if (! component->isCurrentlyBlockedByAnotherModalComponent())
                    handleUserClosingWindow();

                return 0;

            case WM_QUERYENDSESSION:
                if (JUCEApplication::getInstance() != nullptr)
                {
                    JUCEApplication::getInstance()->systemRequestedQuit();
                    return MessageManager::getInstance()->hasStopMessageBeenSent();
                }
                return TRUE;

            case WM_TRAYNOTIFY:
                handleTaskBarEvent (lParam);
                break;

            case WM_SYNCPAINT:
                return 0;

            case WM_DISPLAYCHANGE:
                InvalidateRect (h, 0, 0);
                // intentional fall-through...
            case WM_SETTINGCHANGE:  // note the fall-through in the previous case!
                doSettingChange();
                break;

            case WM_INITMENU:
                if (! hasTitleBar())
                {
                    if (isFullScreen())
                    {
                        EnableMenuItem ((HMENU) wParam, SC_RESTORE, MF_BYCOMMAND | MF_ENABLED);
                        EnableMenuItem ((HMENU) wParam, SC_MOVE, MF_BYCOMMAND | MF_GRAYED);
                    }
                    else if (! isMinimised())
                    {
                        EnableMenuItem ((HMENU) wParam, SC_MAXIMIZE, MF_BYCOMMAND | MF_GRAYED);
                    }
                }
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
                if (taskBarIcon != nullptr)
                {
                    static const DWORD taskbarCreatedMessage = RegisterWindowMessage (TEXT("TaskbarCreated"));

                    if (message == taskbarCreatedMessage)
                    {
                        taskBarIcon->uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
                        Shell_NotifyIcon (NIM_ADD, taskBarIcon);
                    }
                }

                break;
        }

        return DefWindowProcW (h, message, wParam, lParam);
    }

    bool sendInputAttemptWhenModalMessage()
    {
        if (component->isCurrentlyBlockedByAnotherModalComponent())
        {
            Component* const current = Component::getCurrentlyModalComponent();

            if (current != nullptr)
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

                HIMC hImc = ImmGetContext (hWnd);
                if (hImc != 0)
                {
                    ImmNotifyIME (hImc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
                    ImmReleaseContext (hWnd, hImc);
                }
            }
        }

        void handleStartComposition (ComponentPeer& owner)
        {
            reset();
            TextInputTarget* const target = owner.findCurrentTextInputTarget();

            if (target != nullptr)
                target->insertTextAtCaret (String::empty);
        }

        void handleEndComposition (ComponentPeer& owner, HWND hWnd)
        {
            if (compositionInProgress)
            {
                // If this occurs, the user has cancelled the composition, so clear their changes..
                TextInputTarget* const target = owner.findCurrentTextInputTarget();

                if (target != nullptr)
                {
                    target->setHighlightedRegion (compositionRange);
                    target->insertTextAtCaret (String::empty);
                    compositionRange.setLength (0);

                    target->setHighlightedRegion (Range<int>::emptyRange (compositionRange.getEnd()));
                    target->setTemporaryUnderlining (Array<Range<int> >());
                }

                HIMC hImc = ImmGetContext (hWnd);

                if (hImc != 0)
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
                                         Range<int>::emptyRange (compositionRange.getEnd()));

                target->setTemporaryUnderlining (Array<Range<int> >());

                compositionInProgress = false;
            }
            else if ((lParam & GCS_COMPSTR) != 0) // (composition is still in-progress)
            {
                const String newContent (getCompositionString (hImc, GCS_COMPSTR));
                const Range<int> selection (getCompositionSelection (hImc, lParam));

                replaceCurrentSelection (target, newContent, selection);

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
                ImmGetCompositionString (hImc, type, buffer, stringSizeBytes);
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
                    HeapBlock<char> attributes (attributeSizeBytes);
                    ImmGetCompositionString (hImc, GCS_COMPATTR, attributes, attributeSizeBytes);

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

        void replaceCurrentSelection (TextInputTarget* const target, const String& newContent, const Range<int>& newSelection)
        {
            target->setHighlightedRegion (compositionRange);
            target->insertTextAtCaret (newContent);
            compositionRange.setLength (newContent.length());

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
                    const int numItems = clauseDataSizeBytes / sizeof (uint32);
                    HeapBlock<uint32> clauseData (numItems);

                    if (ImmGetCompositionString (hImc, GCS_COMPCLAUSE, clauseData, clauseDataSizeBytes) > 0)
                        for (int i = 0; i < numItems - 1; ++i)
                            result.add (Range<int> (clauseData [i], clauseData [i + 1]) + compositionRange.getStart());
                }
            }

            return result;
        }

        void moveCandidateWindowToLeftAlignWithSelection (HIMC hImc, ComponentPeer& peer, TextInputTarget* target) const
        {
            Component* const targetComp = dynamic_cast <Component*> (target);

            if (targetComp != nullptr)
            {
                const Rectangle<int> area (peer.getComponent()
                                              ->getLocalArea (targetComp, target->getCaretRectangle()));

                CANDIDATEFORM pos = { 0, CFS_CANDIDATEPOS, { area.getX(), area.getBottom() }, { 0, 0, 0, 0 } };
                ImmSetCandidateWindow (hImc, &pos);
            }
        }

        JUCE_DECLARE_NON_COPYABLE (IMEHandler);
    };

    IMEHandler imeHandler;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Win32ComponentPeer);
};

ModifierKeys Win32ComponentPeer::currentModifiers;
ModifierKeys Win32ComponentPeer::modifiersAtLastCallback;

ComponentPeer* Component::createNewPeer (int styleFlags, void* nativeWindowToAttachTo)
{
    return new Win32ComponentPeer (this, styleFlags, (HWND) nativeWindowToAttachTo);
}

juce_ImplementSingleton_SingleThreaded (Win32ComponentPeer::WindowClassHolder);


//==============================================================================
void ModifierKeys::updateCurrentModifiers() noexcept
{
    currentModifiers = Win32ComponentPeer::currentModifiers;
}

const ModifierKeys ModifierKeys::getCurrentModifiersRealtime() noexcept
{
    Win32ComponentPeer::updateKeyModifiers();

    int mouseMods = 0;
    if (Win32ComponentPeer::isKeyDown (VK_LBUTTON))  mouseMods |= ModifierKeys::leftButtonModifier;
    if (Win32ComponentPeer::isKeyDown (VK_RBUTTON))  mouseMods |= ModifierKeys::rightButtonModifier;
    if (Win32ComponentPeer::isKeyDown (VK_MBUTTON))  mouseMods |= ModifierKeys::middleButtonModifier;

    Win32ComponentPeer::currentModifiers
        = Win32ComponentPeer::currentModifiers.withoutMouseButtons().withFlags (mouseMods);

    return Win32ComponentPeer::currentModifiers;
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

    return Win32ComponentPeer::isKeyDown (k);
}

//==============================================================================
void SystemTrayIconComponent::setIconImage (const Image& newImage)
{
    Win32ComponentPeer* const wp = dynamic_cast <Win32ComponentPeer*> (getPeer());

    if (wp != nullptr)
        wp->setTaskBarIcon (newImage);
}

void SystemTrayIconComponent::setIconTooltip (const String& tooltip)
{
    Win32ComponentPeer* const wp = dynamic_cast <Win32ComponentPeer*> (getPeer());

    if (wp != nullptr)
        wp->setTaskBarIconToolTip (tooltip);
}

//==============================================================================
void juce_setWindowStyleBit (HWND h, const int styleType, const int feature, const bool bitIsSet) noexcept
{
    DWORD val = GetWindowLong (h, styleType);

    if (bitIsSet)
        val |= feature;
    else
        val &= ~feature;

    SetWindowLongPtr (h, styleType, val);
    SetWindowPos (h, 0, 0, 0, 0, 0,
                  SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER
                   | SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_NOSENDCHANGING);
}


//==============================================================================
bool Process::isForegroundProcess()
{
    HWND fg = GetForegroundWindow();

    if (fg == 0)
        return true;

    // when running as a plugin in IE8, the browser UI runs in a different process to the plugin, so
    // process ID isn't a reliable way to check if the foreground window belongs to us - instead, we
    // have to see if any of our windows are children of the foreground window
    fg = GetAncestor (fg, GA_ROOT);

    for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
    {
        Win32ComponentPeer* const wp = dynamic_cast <Win32ComponentPeer*> (ComponentPeer::getPeer (i));

        if (wp != nullptr && wp->isInside (fg))
            return true;
    }

    return false;
}

//==============================================================================
class Win32MessageBox  : public AsyncUpdater
{
public:
    Win32MessageBox (AlertWindow::AlertIconType iconType,
                     const String& title_, const String& message_,
                     Component* associatedComponent,
                     UINT extraFlags,
                     ModalComponentManager::Callback* callback_,
                     const bool runAsync)
        : flags (extraFlags | getMessageBoxFlags (iconType)),
          owner (getWindowForMessageBox (associatedComponent)),
          title (title_), message (message_), callback (callback_)
    {
        if (runAsync)
            triggerAsyncUpdate();
    }

    int getResult() const
    {
        const int r = MessageBox (owner, message.toWideCharPointer(), title.toWideCharPointer(), flags);
        return (r == IDYES || r == IDOK) ? 1 : (r == IDNO ? 2 : 0);
    }

    void handleAsyncUpdate()
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
    ModalComponentManager::Callback* callback;

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

void JUCE_CALLTYPE NativeMessageBox::showMessageBox (AlertWindow::AlertIconType iconType,
                                                     const String& title, const String& message,
                                                     Component* associatedComponent)
{
    Win32MessageBox box (iconType, title, message, associatedComponent, MB_OK, 0, false);
    (void) box.getResult();
}

void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (AlertWindow::AlertIconType iconType,
                                                          const String& title, const String& message,
                                                          Component* associatedComponent)
{
    new Win32MessageBox (iconType, title, message, associatedComponent, MB_OK, 0, true);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (AlertWindow::AlertIconType iconType,
                                                      const String& title, const String& message,
                                                      Component* associatedComponent,
                                                      ModalComponentManager::Callback* callback)
{
    ScopedPointer<Win32MessageBox> mb (new Win32MessageBox (iconType, title, message, associatedComponent,
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
    ScopedPointer<Win32MessageBox> mb (new Win32MessageBox (iconType, title, message, associatedComponent,
                                                            MB_YESNOCANCEL, callback, callback != nullptr));
    if (callback == nullptr)
        return mb->getResult();

    mb.release();
    return 0;
}

//==============================================================================
void Desktop::createMouseInputSources()
{
    mouseSources.add (new MouseInputSource (0, true));
}

const Point<int> MouseInputSource::getCurrentMousePosition()
{
    POINT mousePos;
    GetCursorPos (&mousePos);
    return Point<int> (mousePos.x, mousePos.y);
}

void Desktop::setMousePosition (const Point<int>& newPosition)
{
    SetCursorPos (newPosition.getX(), newPosition.getY());
}

//==============================================================================
Image::SharedImage* Image::SharedImage::createNativeImage (PixelFormat format, int width, int height, bool clearImage)
{
    return createSoftwareImage (format, width, height, clearImage);
}

//==============================================================================
class ScreenSaverDefeater   : public Timer,
                              public DeletedAtShutdown
{
public:
    ScreenSaverDefeater()
    {
        startTimer (10000);
        timerCallback();
    }

    void timerCallback()
    {
        if (Process::isForegroundProcess())
        {
            // simulate a shift key getting pressed..
            INPUT input[2];
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_SHIFT;
            input[0].ki.dwFlags = 0;
            input[0].ki.dwExtraInfo = 0;

            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = VK_SHIFT;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            input[1].ki.dwExtraInfo = 0;

            SendInput (2, input, sizeof (INPUT));
        }
    }
};

static ScreenSaverDefeater* screenSaverDefeater = nullptr;

void Desktop::setScreenSaverEnabled (const bool isEnabled)
{
    if (isEnabled)
        deleteAndZero (screenSaverDefeater);
    else if (screenSaverDefeater == nullptr)
        screenSaverDefeater = new ScreenSaverDefeater();
}

bool Desktop::isScreenSaverEnabled()
{
    return screenSaverDefeater == 0;
}

/* (The code below is the "correct" way to disable the screen saver, but it
    completely fails on winXP when the saver is password-protected...)

static bool juce_screenSaverEnabled = true;

void Desktop::setScreenSaverEnabled (const bool isEnabled) noexcept
{
    juce_screenSaverEnabled = isEnabled;
    SetThreadExecutionState (isEnabled ? ES_CONTINUOUS
                                       : (ES_DISPLAY_REQUIRED | ES_CONTINUOUS));
}

bool Desktop::isScreenSaverEnabled() noexcept
{
    return juce_screenSaverEnabled;
}
*/

//==============================================================================
void Desktop::setKioskComponent (Component* kioskModeComponent, bool enableOrDisable, bool /*allowMenusAndBars*/)
{
    if (enableOrDisable)
        kioskModeComponent->setBounds (Desktop::getInstance().getMainMonitorArea (false));
}

//==============================================================================
static BOOL CALLBACK enumMonitorsProc (HMONITOR, HDC, LPRECT r, LPARAM userInfo)
{
    Array <Rectangle<int> >* const monitorCoords = (Array <Rectangle<int> >*) userInfo;
    monitorCoords->add (Rectangle<int> (r->left, r->top, r->right - r->left, r->bottom - r->top));
    return TRUE;
}

void Desktop::getCurrentMonitorPositions (Array <Rectangle<int> >& monitorCoords, const bool clipToWorkArea)
{
    EnumDisplayMonitors (0, 0, &enumMonitorsProc, (LPARAM) &monitorCoords);

    // make sure the first in the list is the main monitor
    for (int i = 1; i < monitorCoords.size(); ++i)
        if (monitorCoords[i].getX() == 0 && monitorCoords[i].getY() == 0)
            monitorCoords.swap (i, 0);

    if (monitorCoords.size() == 0)
    {
        RECT r;
        GetWindowRect (GetDesktopWindow(), &r);

        monitorCoords.add (Rectangle<int> (r.left, r.top, r.right - r.left, r.bottom - r.top));
    }

    if (clipToWorkArea)
    {
        // clip the main monitor to the active non-taskbar area
        RECT r;
        SystemParametersInfo (SPI_GETWORKAREA, 0, &r, 0);

        Rectangle<int>& screen = monitorCoords.getReference (0);

        screen.setPosition (jmax (screen.getX(), (int) r.left),
                            jmax (screen.getY(), (int) r.top));

        screen.setSize (jmin (screen.getRight(), (int) r.right) - screen.getX(),
                        jmin (screen.getBottom(), (int) r.bottom) - screen.getY());
    }
}

//==============================================================================
Image juce_createIconForFile (const File& file)
{
    Image image;
    WORD iconNum = 0;

    HICON icon = ExtractAssociatedIcon ((HINSTANCE) PlatformUtilities::getCurrentModuleInstanceHandle(),
                                        const_cast <WCHAR*> (file.getFullPathName().toWideCharPointer()), &iconNum);

    if (icon != 0)
    {
        image = IconConverters::createImageFromHICON (icon);
        DestroyIcon (icon);
    }

    return image;
}

//==============================================================================
void* MouseCursor::createMouseCursorFromImage (const Image& image, int hotspotX, int hotspotY)
{
    const int maxW = GetSystemMetrics (SM_CXCURSOR);
    const int maxH = GetSystemMetrics (SM_CYCURSOR);

    Image im (image);

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
        case NormalCursor:                  break;
        case NoCursor:                      return (void*) hiddenMouseCursorHandle;
        case WaitCursor:                    cursorName = IDC_WAIT; break;
        case IBeamCursor:                   cursorName = IDC_IBEAM; break;
        case PointingHandCursor:            cursorName = MAKEINTRESOURCE(32649); break;
        case CrosshairCursor:               cursorName = IDC_CROSS; break;
        case CopyingCursor:                 break; // can't seem to find one of these in the win32 list..

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

                dragHandCursor = createMouseCursorFromImage (ImageFileFormat::loadFrom (dragHandData, sizeof (dragHandData)), 8, 7);
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

//==============================================================================
//==============================================================================
class JuceDropSource   : public ComBaseClassHelper <IDropSource>
{
public:
    JuceDropSource() {}

    HRESULT __stdcall QueryContinueDrag (BOOL escapePressed, DWORD keys)
    {
        if (escapePressed)
            return DRAGDROP_S_CANCEL;

        if ((keys & (MK_LBUTTON | MK_RBUTTON)) == 0)
            return DRAGDROP_S_DROP;

        return S_OK;
    }

    HRESULT __stdcall GiveFeedback (DWORD)
    {
        return DRAGDROP_S_USEDEFAULTCURSORS;
    }
};


class JuceEnumFormatEtc   : public ComBaseClassHelper <IEnumFORMATETC>
{
public:
    JuceEnumFormatEtc (const FORMATETC* const format_)
        : format (format_),
          index (0)
    {
    }

    HRESULT __stdcall Clone (IEnumFORMATETC** result)
    {
        if (result == 0)
            return E_POINTER;

        JuceEnumFormatEtc* const newOne = new JuceEnumFormatEtc (format);
        newOne->index = index;

        *result = newOne;
        return S_OK;
    }

    HRESULT __stdcall Next (ULONG celt, LPFORMATETC lpFormatEtc, ULONG* pceltFetched)
    {
        if (pceltFetched != nullptr)
            *pceltFetched = 0;
        else if (celt != 1)
            return S_FALSE;

        if (index == 0 && celt > 0 && lpFormatEtc != 0)
        {
            copyFormatEtc (lpFormatEtc [0], *format);
            ++index;

            if (pceltFetched != nullptr)
                *pceltFetched = 1;

            return S_OK;
        }

        return S_FALSE;
    }

    HRESULT __stdcall Skip (ULONG celt)
    {
        if (index + (int) celt >= 1)
            return S_FALSE;

        index += celt;
        return S_OK;
    }

    HRESULT __stdcall Reset()
    {
        index = 0;
        return S_OK;
    }

private:
    const FORMATETC* const format;
    int index;

    static void copyFormatEtc (FORMATETC& dest, const FORMATETC& source)
    {
        dest = source;

        if (source.ptd != 0)
        {
            dest.ptd = (DVTARGETDEVICE*) CoTaskMemAlloc (sizeof (DVTARGETDEVICE));
            *(dest.ptd) = *(source.ptd);
        }
    }

    JUCE_DECLARE_NON_COPYABLE (JuceEnumFormatEtc);
};

class JuceDataObject  : public ComBaseClassHelper <IDataObject>
{
public:
    JuceDataObject (JuceDropSource* const dropSource_,
                    const FORMATETC* const format_,
                    const STGMEDIUM* const medium_)
        : dropSource (dropSource_),
          format (format_),
          medium (medium_)
    {
    }

    ~JuceDataObject()
    {
        jassert (refCount == 0);
    }

    HRESULT __stdcall GetData (FORMATETC* pFormatEtc, STGMEDIUM* pMedium)
    {
        if ((pFormatEtc->tymed & format->tymed) != 0
             && pFormatEtc->cfFormat == format->cfFormat
             && pFormatEtc->dwAspect == format->dwAspect)
        {
            pMedium->tymed = format->tymed;
            pMedium->pUnkForRelease = 0;

            if (format->tymed == TYMED_HGLOBAL)
            {
                const SIZE_T len = GlobalSize (medium->hGlobal);
                void* const src = GlobalLock (medium->hGlobal);
                void* const dst = GlobalAlloc (GMEM_FIXED, len);

                memcpy (dst, src, len);

                GlobalUnlock (medium->hGlobal);

                pMedium->hGlobal = dst;
                return S_OK;
            }
        }

        return DV_E_FORMATETC;
    }

    HRESULT __stdcall QueryGetData (FORMATETC* f)
    {
        if (f == 0)
            return E_INVALIDARG;

        if (f->tymed == format->tymed
              && f->cfFormat == format->cfFormat
              && f->dwAspect == format->dwAspect)
            return S_OK;

        return DV_E_FORMATETC;
    }

    HRESULT __stdcall GetCanonicalFormatEtc (FORMATETC*, FORMATETC* pFormatEtcOut)
    {
        pFormatEtcOut->ptd = 0;
        return E_NOTIMPL;
    }

    HRESULT __stdcall EnumFormatEtc (DWORD direction, IEnumFORMATETC** result)
    {
        if (result == 0)
            return E_POINTER;

        if (direction == DATADIR_GET)
        {
            *result = new JuceEnumFormatEtc (format);
            return S_OK;
        }

        *result = 0;
        return E_NOTIMPL;
    }

    HRESULT __stdcall GetDataHere (FORMATETC*, STGMEDIUM*)                  { return DATA_E_FORMATETC; }
    HRESULT __stdcall SetData (FORMATETC*, STGMEDIUM*, BOOL)                { return E_NOTIMPL; }
    HRESULT __stdcall DAdvise (FORMATETC*, DWORD, IAdviseSink*, DWORD*)     { return OLE_E_ADVISENOTSUPPORTED; }
    HRESULT __stdcall DUnadvise (DWORD)                                     { return E_NOTIMPL; }
    HRESULT __stdcall EnumDAdvise (IEnumSTATDATA**)                         { return OLE_E_ADVISENOTSUPPORTED; }

private:
    JuceDropSource* const dropSource;
    const FORMATETC* const format;
    const STGMEDIUM* const medium;

    JUCE_DECLARE_NON_COPYABLE (JuceDataObject);
};

static HDROP createHDrop (const StringArray& fileNames)
{
    int totalBytes = 0;
    for (int i = fileNames.size(); --i >= 0;)
        totalBytes += CharPointer_UTF16::getBytesRequiredFor (fileNames[i].getCharPointer()) + sizeof (WCHAR);

    HDROP hDrop = (HDROP) GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof (DROPFILES) + totalBytes + 4);

    if (hDrop != 0)
    {
        LPDROPFILES pDropFiles = (LPDROPFILES) GlobalLock (hDrop);
        pDropFiles->pFiles = sizeof (DROPFILES);
        pDropFiles->fWide = true;

        WCHAR* fname = reinterpret_cast<WCHAR*> (addBytesToPointer (pDropFiles, sizeof (DROPFILES)));

        for (int i = 0; i < fileNames.size(); ++i)
        {
            const int bytesWritten = fileNames[i].copyToUTF16 (fname, 2048);
            fname = reinterpret_cast<WCHAR*> (addBytesToPointer (fname, bytesWritten));
        }

        *fname = 0;

        GlobalUnlock (hDrop);
    }

    return hDrop;
}

static bool performDragDrop (FORMATETC* const format, STGMEDIUM* const medium, const DWORD whatToDo)
{
    JuceDropSource* const source = new JuceDropSource();
    JuceDataObject* const data = new JuceDataObject (source, format, medium);

    DWORD effect;
    const HRESULT res = DoDragDrop (data, source, whatToDo, &effect);

    data->Release();
    source->Release();

    return res == DRAGDROP_S_DROP;
}

bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const bool canMove)
{
    FORMATETC format = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM medium = { TYMED_HGLOBAL, { 0 }, 0 };

    medium.hGlobal = createHDrop (files);

    return performDragDrop (&format, &medium, canMove ? (DROPEFFECT_COPY | DROPEFFECT_MOVE)
                                                      : DROPEFFECT_COPY);
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text)
{
    FORMATETC format = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM medium = { TYMED_HGLOBAL, { 0 }, 0 };

    const int numBytes = CharPointer_UTF16::getBytesRequiredFor (text.getCharPointer());

    medium.hGlobal = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, numBytes + 2);
    WCHAR* const data = static_cast <WCHAR*> (GlobalLock (medium.hGlobal));

    text.copyToUTF16 (data, numBytes);
    format.cfFormat = CF_UNICODETEXT;

    GlobalUnlock (medium.hGlobal);

    return performDragDrop (&format, &medium, DROPEFFECT_COPY | DROPEFFECT_MOVE);
}

#endif
