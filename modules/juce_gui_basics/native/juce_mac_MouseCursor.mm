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

#if JUCE_MAC

//==============================================================================
namespace MouseCursorHelpers
{
    NSImage* createNSImage (const Image&);
    NSImage* createNSImage (const Image& image)
    {
        JUCE_AUTORELEASEPOOL
        {
            NSImage* im = [[NSImage alloc] init];
            [im setSize: NSMakeSize (image.getWidth(), image.getHeight())];
            [im lockFocus];

            CGColorSpaceRef colourSpace = CGColorSpaceCreateDeviceRGB();
            CGImageRef imageRef = juce_createCoreGraphicsImage (image, colourSpace, false);
            CGColorSpaceRelease (colourSpace);

            CGContextRef cg = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];
            CGContextDrawImage (cg, convertToCGRect (image.getBounds()), imageRef);

            CGImageRelease (imageRef);
            [im unlockFocus];

            return im;
        }
    }

    static void* fromWebKitFile (const char* filename, float hx, float hy)
    {
        FileInputStream fileStream (File ("/System/Library/Frameworks/WebKit.framework/Frameworks/WebCore.framework/Resources").getChildFile (filename));
        BufferedInputStream buf (fileStream, 4096);

        PNGImageFormat pngFormat;
        Image im (pngFormat.decodeImage (buf));

        if (im.isValid())
            return CustomMouseCursorInfo (im, (int) (hx * im.getWidth()),
                                              (int) (hy * im.getHeight())).create();

        return nullptr;
    }
}

void* CustomMouseCursorInfo::create() const
{
    NSImage* im = MouseCursorHelpers::createNSImage (image);
    NSCursor* c = [[NSCursor alloc] initWithImage: im
                                          hotSpot: NSMakePoint (hotspot.x, hotspot.y)];
    [im release];
    return c;
}

void* MouseCursor::createStandardMouseCursor (MouseCursor::StandardCursorType type)
{
    JUCE_AUTORELEASEPOOL
    {
        NSCursor* c = nil;

        switch (type)
        {
            case NormalCursor:
            case ParentCursor:          c = [NSCursor arrowCursor]; break;
            case NoCursor:              return CustomMouseCursorInfo (Image (Image::ARGB, 8, 8, true), 0, 0).create();
            case DraggingHandCursor:    c = [NSCursor openHandCursor]; break;
            case WaitCursor:            c = [NSCursor arrowCursor]; break; // avoid this on the mac, let the OS provide the beachball
            case IBeamCursor:           c = [NSCursor IBeamCursor]; break;
            case PointingHandCursor:    c = [NSCursor pointingHandCursor]; break;
            case LeftRightResizeCursor: c = [NSCursor resizeLeftRightCursor]; break;
            case LeftEdgeResizeCursor:  c = [NSCursor resizeLeftCursor]; break;
            case RightEdgeResizeCursor: c = [NSCursor resizeRightCursor]; break;
            case CrosshairCursor:       c = [NSCursor crosshairCursor]; break;

            case CopyingCursor:
            {
               #if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_6
                if (void* m = MouseCursorHelpers::fromWebKitFile ("copyCursor.png", 0, 0))
                    return m;
               #endif
                c = [NSCursor dragCopyCursor]; // added in 10.6
                break;
            }

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

void* CustomMouseCursorInfo::create() const                                              { return nullptr; }
void* MouseCursor::createStandardMouseCursor (MouseCursor::StandardCursorType)           { return nullptr; }
void MouseCursor::deleteMouseCursor (void*, bool)                                        {}
void MouseCursor::showInAllWindows() const                                               {}
void MouseCursor::showInWindow (ComponentPeer*) const                                    {}

#endif
