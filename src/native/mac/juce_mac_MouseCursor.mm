/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

#if JUCE_MAC


//==============================================================================
void* juce_createMouseCursorFromImage (const Image& image, int hotspotX, int hotspotY) throw()
{
    NSImage* im = CoreGraphicsImage::createNSImage (image);
    NSCursor* c = [[NSCursor alloc] initWithImage: im
                                          hotSpot: NSMakePoint (hotspotX, hotspotY)];
    [im release];

    return (void*) c;
}

static void* juce_cursorFromData (const unsigned char* data, const size_t size, float hx, float hy) throw()
{
    ScopedPointer <Image> im (ImageFileFormat::loadFrom ((const char*) data, (int) size));
    jassert (im != 0);

    if (im == 0)
        return 0;

    return juce_createMouseCursorFromImage (*im,
                                            (int) (hx * im->getWidth()),
                                            (int) (hy * im->getHeight()));
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

#else

void* juce_createMouseCursorFromImage (const Image& image, int hotspotX, int hotspotY) throw()  { return 0; }
void* juce_createStandardMouseCursor (MouseCursor::StandardCursorType type) throw()             { return 0; }
void juce_deleteMouseCursor (void* const cursorHandle, const bool isStandard) throw()           {}
void MouseCursor::showInAllWindows() const throw()                                              {}
void MouseCursor::showInWindow (ComponentPeer*) const throw()                                   {}

#endif

#endif
