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

Image* juce_loadPNGImageFromStream (InputStream& inputStream);

//==============================================================================
void* juce_createMouseCursorFromImage (const Image& image, int hotspotX, int hotspotY)
{
    NSImage* im = CoreGraphicsImage::createNSImage (image);
    NSCursor* c = [[NSCursor alloc] initWithImage: im
                                          hotSpot: NSMakePoint (hotspotX, hotspotY)];
    [im release];

    return c;
}

static void* juce_cursorFromData (const MemoryBlock& data, const float hx, const float hy)
{
    MemoryInputStream stream (data, false);
    ScopedPointer <Image> im (juce_loadPNGImageFromStream (stream));
    jassert (im != 0);

    if (im == 0)
        return 0;

    return juce_createMouseCursorFromImage (*im,
                                            (int) (hx * im->getWidth()),
                                            (int) (hy * im->getHeight()));
}

static void* juce_cursorFromWebKitFile (const char* filename, float hx, float hy)
{
    const File f ("/System/Library/Frameworks/WebKit.framework/Frameworks/WebCore.framework/Resources");

    MemoryBlock mb;
    if (f.getChildFile (filename).loadFileAsData (mb))
        return juce_cursorFromData (mb, hx, hy);

    return 0;
}

void* juce_createStandardMouseCursor (MouseCursor::StandardCursorType type)
{
    const ScopedAutoReleasePool pool;
    NSCursor* c = 0;

    switch (type)
    {
    case MouseCursor::NormalCursor:
        c = [NSCursor arrowCursor];
        break;

    case MouseCursor::NoCursor:
        return juce_createMouseCursorFromImage (Image (Image::ARGB, 8, 8, true), 0, 0);

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
    return c;
}

void juce_deleteMouseCursor (void* const cursorHandle, const bool isStandard)
{
    NSCursor* c = (NSCursor*) cursorHandle;
    [c release];
}

void MouseCursor::showInAllWindows() const
{
    showInWindow (0);
}

void MouseCursor::showInWindow (ComponentPeer*) const
{
    NSCursor* const c = (NSCursor*) getHandle();
    [c set];
}

#else

void* juce_createMouseCursorFromImage (const Image& image, int hotspotX, int hotspotY)          { return 0; }
void* juce_createStandardMouseCursor (MouseCursor::StandardCursorType type)                     { return 0; }
void juce_deleteMouseCursor (void* const cursorHandle, const bool isStandard)                   {}
void MouseCursor::showInAllWindows() const                                                      {}
void MouseCursor::showInWindow (ComponentPeer*) const                                           {}

#endif

#endif
