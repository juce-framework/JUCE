/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_MAC

//==============================================================================
class MouseCursor::PlatformSpecificHandle
{
public:
    PlatformSpecificHandle (const MouseCursor::StandardCursorType type)
        : cursorHandle (createCursor (type)) {}

    PlatformSpecificHandle (const detail::CustomMouseCursorInfo& info)
        : cursorHandle (createCursor (info)) {}

    ~PlatformSpecificHandle()
    {
        [cursorHandle release];
    }

    static void showInWindow (PlatformSpecificHandle* handle, ComponentPeer*)
    {
        auto c = [&]
        {
            if (handle == nullptr || handle->cursorHandle == nullptr)
                return [NSCursor arrowCursor];

            return handle->cursorHandle;
        }();

        [c set];
    }

private:
    static NSCursor* fromNSImage (NSImage* im, NSPoint hotspot)
    {
        NSCursor* c = [[NSCursor alloc] initWithImage: im
                                              hotSpot: hotspot];
        [im release];
        return c;
    }

    static NSCursor* fromHIServices (const char* filename)
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
                                                                               hints: [NSDictionary dictionaryWithObjectsAndKeys: scaleTransform, NSImageHintCTM, nil]])
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
    static NSCursor* createCursor (const detail::CustomMouseCursorInfo& info)
    {
        return fromNSImage (imageToNSImage (info.image),
                            NSMakePoint (info.hotspot.x, info.hotspot.y));
    }

    static NSCursor* createCursor (const MouseCursor::StandardCursorType type)
    {
        JUCE_AUTORELEASEPOOL
        {
            NSCursor* c = nil;

            switch (type)
            {
                case NormalCursor:
                case ParentCursor:          c = [NSCursor arrowCursor]; break;
                case NoCursor:              return createCursor ({ ScaledImage (Image (Image::ARGB, 8, 8, true)), {} });
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
                    if (NSCursor* m = fromHIServices ("resizenorthsouth"))
                        return m;

                    c = [NSCursor resizeUpDownCursor];
                    break;

                case LeftRightResizeCursor:
                    if (NSCursor* m = fromHIServices ("resizeeastwest"))
                        return m;

                    c = [NSCursor resizeLeftRightCursor];
                    break;

                case TopLeftCornerResizeCursor:
                case BottomRightCornerResizeCursor:
                    return fromHIServices ("resizenorthwestsoutheast");

                case TopRightCornerResizeCursor:
                case BottomLeftCornerResizeCursor:
                    return fromHIServices ("resizenortheastsouthwest");

                case UpDownLeftRightResizeCursor:
                    return fromHIServices ("move");

                case NumStandardCursorTypes:
                default:
                    jassertfalse;
                    break;
            }

            [c retain];
            return c;
        }
    }

    NSCursor* cursorHandle;
};

#else

class MouseCursor::PlatformSpecificHandle
{
public:
    PlatformSpecificHandle (const MouseCursor::StandardCursorType)      {}
    PlatformSpecificHandle (const detail::CustomMouseCursorInfo&)       {}

    static void showInWindow (PlatformSpecificHandle*, ComponentPeer*)  {}
};

#endif

} // namespace juce
