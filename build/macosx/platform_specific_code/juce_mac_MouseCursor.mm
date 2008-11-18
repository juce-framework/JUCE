/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#ifdef JUCE_INCLUDED_FILE

//==============================================================================
static NSImage* juceImageToNSImage (const Image& image)
{
    const ScopedAutoReleasePool pool;
    int lineStride, pixelStride;
    const uint8* pixels = image.lockPixelDataReadOnly (0, 0, image.getWidth(), image.getHeight(),
                                                       lineStride, pixelStride);

    NSBitmapImageRep* rep = [[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes: NULL
                      pixelsWide: image.getWidth()
                      pixelsHigh: image.getHeight()
                   bitsPerSample: 8
                 samplesPerPixel: image.hasAlphaChannel() ? 4 : 3
                        hasAlpha: image.hasAlphaChannel()
                        isPlanar: NO
                  colorSpaceName: NSCalibratedRGBColorSpace
                    bitmapFormat: (NSBitmapFormat) 0
                     bytesPerRow: lineStride
                    bitsPerPixel: pixelStride * 8];

    unsigned char* newData = [rep bitmapData];
    memcpy (newData, pixels, lineStride * image.getHeight());
    image.releasePixelDataReadOnly (pixels);

    NSImage* im = [[NSImage alloc] init];
    [im addRepresentation: rep];
    [rep release];

    return im;
}

//==============================================================================
void* juce_createMouseCursorFromImage (const Image& image, int hotspotX, int hotspotY) throw()
{
    NSImage* im = juceImageToNSImage (image);
    NSCursor* c = [[NSCursor alloc] initWithImage: im
                                          hotSpot: NSMakePoint (hotspotX, hotspotY)];
    [im release];

    return (void*) c;
}

static void* juce_cursorFromData (const unsigned char* data, const int size, float hx, float hy) throw()
{
    Image* const im = ImageFileFormat::loadFrom ((const char*) data, size);
    jassert (im != 0);

    if (im == 0)
        return 0;

    void* const curs = juce_createMouseCursorFromImage (*im,
                                                        (int) (hx * im->getWidth()),
                                                        (int) (hy * im->getHeight()));
    delete im;
    return curs;
}

static void* juce_cursorFromWebKitFile (const char* filename, float hx, float hy)
{
    File f ("/System/Library/Frameworks/WebKit.framework/Frameworks/WebCore.framework/Resources");

    MemoryBlock mb;
    if (f.getChildFile (filename).loadFileAsData (mb))
        return juce_cursorFromData ((const unsigned char*) mb.getData(), mb.getSize(), hx, hy);

    return 0;
}

void* juce_createStandardMouseCursor (MouseCursor::StandardCursorType type) throw()
{
    const ScopedAutoReleasePool pool;
    NSCursor* c = 0;

    switch (type)
    {
    case MouseCursor::NormalCursor:
        c = [NSCursor arrowCursor];
        break;

    case MouseCursor::NoCursor:
        {
            Image blank (Image::ARGB, 8, 8, true);
            return juce_createMouseCursorFromImage (blank, 0, 0);
        }

    case MouseCursor::DraggingHandCursor:
        c = [NSCursor openHandCursor];
        break;

    case MouseCursor::CopyingCursor:
        return juce_cursorFromWebKitFile ("copyCursor.png", 0, 0);

    case MouseCursor::WaitCursor:
        c = [NSCursor arrowCursor]; // avoid this on the mac, let the OS provide the beachball
        break;
        //return juce_cursorFromWebKitFile ("waitCursor.png", 0.5f, 0.5f);

    case MouseCursor::IBeamCursor:
        c = [NSCursor IBeamCursor];
        break;

    case MouseCursor::PointingHandCursor:
        c = [NSCursor pointingHandCursor];
        break;

    case MouseCursor::LeftRightResizeCursor:
        c = [NSCursor resizeLeftRightCursor];
        break;

    case MouseCursor::LeftEdgeResizeCursor:
        c = [NSCursor resizeLeftCursor];
        break;

    case MouseCursor::RightEdgeResizeCursor:
        c = [NSCursor resizeRightCursor];
        break;

    case MouseCursor::UpDownResizeCursor:
    case MouseCursor::TopEdgeResizeCursor:
    case MouseCursor::BottomEdgeResizeCursor:
        return juce_cursorFromWebKitFile ("northSouthResizeCursor.png", 0.5f, 0.5f);

    case MouseCursor::TopLeftCornerResizeCursor:
    case MouseCursor::BottomRightCornerResizeCursor:
        return juce_cursorFromWebKitFile ("northWestSouthEastResizeCursor.png", 0.5f, 0.5f);

    case MouseCursor::TopRightCornerResizeCursor:
    case MouseCursor::BottomLeftCornerResizeCursor:
        return juce_cursorFromWebKitFile ("northEastSouthWestResizeCursor.png", 0.5f, 0.5f);

    case MouseCursor::UpDownLeftRightResizeCursor:
        return juce_cursorFromWebKitFile ("moveCursor.png", 0.5f, 0.5f);

    case MouseCursor::CrosshairCursor:
        c = [NSCursor crosshairCursor];
        break;
    }

    [c retain];
    return (void*) c;
}

void juce_deleteMouseCursor (void* const cursorHandle, const bool isStandard) throw()
{
    NSCursor* c = (NSCursor*) cursorHandle;
    [c release];
}

void MouseCursor::showInAllWindows() const throw()
{
    showInWindow (0);
}

void MouseCursor::showInWindow (ComponentPeer*) const throw()
{
    NSCursor* const c = (NSCursor*) getHandle();
    [c set];
}

#endif
