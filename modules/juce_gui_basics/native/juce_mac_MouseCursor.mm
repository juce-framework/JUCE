/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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
    NSImage* createNSImage (const Image&, float scaleFactor = 1.f);
    NSImage* createNSImage (const Image& image, float scaleFactor)
    {
        JUCE_AUTORELEASEPOOL
        {
            NSImage* im = [[NSImage alloc] init];
            const NSSize requiredSize = NSMakeSize (image.getWidth() / scaleFactor, image.getHeight() / scaleFactor);

            [im setSize: requiredSize];
            CGColorSpaceRef colourSpace = CGColorSpaceCreateDeviceRGB();
            CGImageRef imageRef = juce_createCoreGraphicsImage (image, colourSpace, true);
            CGColorSpaceRelease (colourSpace);

            NSBitmapImageRep* imageRep = [[NSBitmapImageRep alloc] initWithCGImage: imageRef];
            [imageRep setSize: requiredSize];
            [im addRepresentation: imageRep];
            [imageRep release];
            CGImageRelease (imageRef);
            return im;
        }
    }

    static NSCursor* fromNSImage (NSImage* im, NSPoint hotspot)
    {
        NSCursor* c = [[NSCursor alloc] initWithImage: im
                                              hotSpot: hotspot];
        [im release];
        return c;
    }

    static void* fromHIServices (const char* filename)
    {
        JUCE_AUTORELEASEPOOL
        {
            const String cursorPath (String ("/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/"
                                             "HIServices.framework/Versions/A/Resources/cursors/")
                                       + filename);

            NSImage* originalImage = [[NSImage alloc] initByReferencingFile: juceStringToNS (cursorPath + "/cursor.pdf")];
            NSSize originalSize = [originalImage size];
            NSImage* resultImage   = [[NSImage alloc] initWithSize: originalSize];

            for (int scale = 1; scale <= 4; ++scale)
            {
                NSAffineTransform* scaleTransform = [NSAffineTransform transform];
                [scaleTransform scaleBy: (float) scale];

                if (CGImageRef rasterCGImage = [originalImage CGImageForProposedRect: nil
                                                                             context: nil
                                                                               hints: [NSDictionary dictionaryWithObjectsAndKeys:
                                                                                         NSImageHintCTM, scaleTransform, nil]])
                {
                    NSBitmapImageRep* imageRep = [[NSBitmapImageRep alloc] initWithCGImage: rasterCGImage];
                    [imageRep setSize: originalSize];

                    [resultImage addRepresentation: imageRep];
                    [imageRep release];
                }
                else
                {
                    return nil;
                }
            }

            [originalImage release];

            NSDictionary* info = [NSDictionary dictionaryWithContentsOfFile: juceStringToNS (cursorPath + "/info.plist")];

            const float hotspotX = (float) [[info valueForKey: nsStringLiteral ("hotx")] doubleValue];
            const float hotspotY = (float) [[info valueForKey: nsStringLiteral ("hoty")] doubleValue];

            return fromNSImage (resultImage, NSMakePoint (hotspotX, hotspotY));
        }
    }
}

void* CustomMouseCursorInfo::create() const
{
    return MouseCursorHelpers::fromNSImage (MouseCursorHelpers::createNSImage (image, scaleFactor),
                                            NSMakePoint (hotspot.x, hotspot.y));
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
            case LeftEdgeResizeCursor:  c = [NSCursor resizeLeftCursor]; break;
            case RightEdgeResizeCursor: c = [NSCursor resizeRightCursor]; break;
            case CrosshairCursor:       c = [NSCursor crosshairCursor]; break;

            case CopyingCursor:
            {
               #if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_6
                if (void* m = MouseCursorHelpers::fromHIServices ("copy"))
                    return m;
               #endif

                c = [NSCursor dragCopyCursor]; // added in 10.6
                break;
            }

            case UpDownResizeCursor:
            case TopEdgeResizeCursor:
            case BottomEdgeResizeCursor:
                if (void* m = MouseCursorHelpers::fromHIServices ("resizenorthsouth"))
                    return m;

                c = [NSCursor resizeUpDownCursor];
                break;

            case LeftRightResizeCursor:
                if (void* m = MouseCursorHelpers::fromHIServices ("resizeeastwest"))
                    return m;

                c = [NSCursor resizeLeftRightCursor];
                break;

            case TopLeftCornerResizeCursor:
            case BottomRightCornerResizeCursor:
                return MouseCursorHelpers::fromHIServices ("resizenorthwestsoutheast");

            case TopRightCornerResizeCursor:
            case BottomLeftCornerResizeCursor:
                return MouseCursorHelpers::fromHIServices ("resizenortheastsouthwest");

            case UpDownLeftRightResizeCursor:
                return MouseCursorHelpers::fromHIServices ("move");

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
