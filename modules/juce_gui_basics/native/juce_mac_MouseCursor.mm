/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_MAC

//==============================================================================
namespace MouseCursorHelpers
{
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
            auto cursorPath = String ("/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/"
                                      "HIServices.framework/Versions/A/Resources/cursors/")
                                  + filename;

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

            auto hotspotX = (float) [[info valueForKey: nsStringLiteral ("hotx")] doubleValue];
            auto hotspotY = (float) [[info valueForKey: nsStringLiteral ("hoty")] doubleValue];

            return fromNSImage (resultImage, NSMakePoint (hotspotX, hotspotY));
        }
    }
}

void* CustomMouseCursorInfo::create() const
{
    return MouseCursorHelpers::fromNSImage (imageToNSImage (image, scaleFactor),
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
            case NoCursor:              return CustomMouseCursorInfo (Image (Image::ARGB, 8, 8, true), {}).create();
            case DraggingHandCursor:    c = [NSCursor openHandCursor]; break;
            case WaitCursor:            c = [NSCursor arrowCursor]; break; // avoid this on the mac, let the OS provide the beachball
            case IBeamCursor:           c = [NSCursor IBeamCursor]; break;
            case PointingHandCursor:    c = [NSCursor pointingHandCursor]; break;
            case LeftEdgeResizeCursor:  c = [NSCursor resizeLeftCursor]; break;
            case RightEdgeResizeCursor: c = [NSCursor resizeRightCursor]; break;
            case CrosshairCursor:       c = [NSCursor crosshairCursor]; break;

            case CopyingCursor:
            {
                c = [NSCursor dragCopyCursor];
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

            case NumStandardCursorTypes:
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

void MouseCursor::showInWindow (ComponentPeer*) const
{
    auto c = (NSCursor*) getHandle();

    if (c == nil)
        c = [NSCursor arrowCursor];

    [c set];
}

#else

void* CustomMouseCursorInfo::create() const                                              { return nullptr; }
void* MouseCursor::createStandardMouseCursor (MouseCursor::StandardCursorType)           { return nullptr; }
void MouseCursor::deleteMouseCursor (void*, bool)                                        {}
void MouseCursor::showInWindow (ComponentPeer*) const                                    {}

#endif

} // namespace juce
