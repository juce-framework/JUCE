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

#if JUCE_MAC

//==============================================================================
namespace MouseCursorHelpers
{
    static NSImage* createNSImage (const Image& image)
    {
        JUCE_AUTORELEASEPOOL

        NSImage* im = [[NSImage alloc] init];
        [im setSize: NSMakeSize (image.getWidth(), image.getHeight())];
        [im lockFocus];

        CGColorSpaceRef colourSpace = CGColorSpaceCreateDeviceRGB();
        CGImageRef imageRef = juce_createCoreGraphicsImage (image, false, colourSpace, false);
        CGColorSpaceRelease (colourSpace);

        CGContextRef cg = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];
        CGContextDrawImage (cg, CGRectMake (0, 0, image.getWidth(), image.getHeight()), imageRef);

        CGImageRelease (imageRef);
        [im unlockFocus];

        return im;
    }

    static void* createFromImage (const Image& image, float hotspotX, float hotspotY)
    {
        NSImage* im = createNSImage (image);
        NSCursor* c = [[NSCursor alloc] initWithImage: im
                                              hotSpot: NSMakePoint (hotspotX, hotspotY)];
        [im release];
        return c;
    }

    static void* fromWebKitFile (const char* filename, float hx, float hy)
    {
        FileInputStream fileStream (String ("/System/Library/Frameworks/WebKit.framework/Frameworks/WebCore.framework/Resources/") + filename);
        BufferedInputStream buf (fileStream, 4096);

        PNGImageFormat pngFormat;
        Image im (pngFormat.decodeImage (buf));

        if (im.isValid())
            return createFromImage (im, hx * im.getWidth(), hy * im.getHeight());

        jassertfalse;
        return nullptr;
    }
}

void* MouseCursor::createMouseCursorFromImage (const Image& image, int hotspotX, int hotspotY)
{
    return MouseCursorHelpers::createFromImage (image, (float) hotspotX, (float) hotspotY);
}

void* MouseCursor::createStandardMouseCursor (MouseCursor::StandardCursorType type)
{
    JUCE_AUTORELEASEPOOL
    NSCursor* c = nil;

    switch (type)
    {
        case NormalCursor:          c = [NSCursor arrowCursor]; break;
        case NoCursor:              return createMouseCursorFromImage (Image (Image::ARGB, 8, 8, true), 0, 0);
        case DraggingHandCursor:    c = [NSCursor openHandCursor]; break;
        case WaitCursor:            c = [NSCursor arrowCursor]; break; // avoid this on the mac, let the OS provide the beachball
        case IBeamCursor:           c = [NSCursor IBeamCursor]; break;
        case PointingHandCursor:    c = [NSCursor pointingHandCursor]; break;
        case LeftRightResizeCursor: c = [NSCursor resizeLeftRightCursor]; break;
        case LeftEdgeResizeCursor:  c = [NSCursor resizeLeftCursor]; break;
        case RightEdgeResizeCursor: c = [NSCursor resizeRightCursor]; break;
        case CrosshairCursor:       c = [NSCursor crosshairCursor]; break;
        case CopyingCursor:         return MouseCursorHelpers::fromWebKitFile ("copyCursor.png", 0, 0);

        case UpDownResizeCursor:
        case TopEdgeResizeCursor:
        case BottomEdgeResizeCursor:
            return MouseCursorHelpers::fromWebKitFile ("northSouthResizeCursor.png", 0.5f, 0.5f);

        case TopLeftCornerResizeCursor:
        case BottomRightCornerResizeCursor:
            return MouseCursorHelpers::fromWebKitFile ("northWestSouthEastResizeCursor.png", 0.5f, 0.5f);

        case TopRightCornerResizeCursor:
        case BottomLeftCornerResizeCursor:
            return MouseCursorHelpers::fromWebKitFile ("northEastSouthWestResizeCursor.png", 0.5f, 0.5f);

        case UpDownLeftRightResizeCursor:
            return MouseCursorHelpers::fromWebKitFile ("moveCursor.png", 0.5f, 0.5f);

        default:
            jassertfalse;
            break;
    }

    [c retain];
    return c;
}

void MouseCursor::deleteMouseCursor (void* const cursorHandle, const bool /*isStandard*/)
{
    [((NSCursor*) cursorHandle) release];
}

void MouseCursor::showInAllWindows() const
{
    showInWindow (nullptr);
}

void MouseCursor::showInWindow (ComponentPeer*) const
{
    NSCursor* c = (NSCursor*) getHandle();

    if (c == nil)
        c = [NSCursor arrowCursor];

    [c set];
}

#else

void* MouseCursor::createMouseCursorFromImage (const Image& image, int hotspotX, int hotspotY)  { return nullptr; }
void* MouseCursor::createStandardMouseCursor (MouseCursor::StandardCursorType type)             { return nullptr; }
void MouseCursor::deleteMouseCursor (void* const cursorHandle, const bool isStandard)           {}
void MouseCursor::showInAllWindows() const                                                      {}
void MouseCursor::showInWindow (ComponentPeer*) const                                           {}

#endif
