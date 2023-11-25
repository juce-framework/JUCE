/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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

extern Image JUCE_API getIconFromApplication (const String&, int);

static Image getIconFromIcnsFile (const File& icnsFile, const int size)
{
    FileInputStream stream (icnsFile);

    if (! stream.openedOk())
        return {};

    const int numHeaderSectionBytes = 4;
    char headerSection [numHeaderSectionBytes];

    if (stream.read (headerSection, numHeaderSectionBytes) != numHeaderSectionBytes
          || headerSection[0] != 'i'
          || headerSection[1] != 'c'
          || headerSection[2] != 'n'
          || headerSection[3] != 's')
        return {};

    if (stream.read (headerSection, numHeaderSectionBytes) != numHeaderSectionBytes)
        return {};

    const auto dataSize = juce::ByteOrder::bigEndianInt (headerSection);

    if (dataSize <= 0)
        return {};

    OwnedArray<juce::ImageFileFormat> internalFormats;
    internalFormats.add (new  PNGImageFormat());
    internalFormats.add (new JPEGImageFormat());

    Array<Image> images;
    auto maxWidth = 0;
    auto maxWidthIndex = -1;

    while (stream.getPosition() < dataSize)
    {
        const auto sectionStart = stream.getPosition();

        if (! stream.setPosition (sectionStart + 4))
            break;

        if (stream.read (headerSection, numHeaderSectionBytes) != numHeaderSectionBytes)
            break;

        const auto sectionSize = ByteOrder::bigEndianInt (headerSection);

        if (sectionSize <= 0)
            break;

        const auto sectionDataStart = stream.getPosition();

        for (auto* fmt : internalFormats)
        {
            if (fmt->canUnderstand (stream))
            {
                stream.setPosition (sectionDataStart);

                images.add (fmt->decodeImage (stream));

                const auto lastImageIndex = images.size() - 1;
                const auto lastWidth = images.getReference (lastImageIndex).getWidth();

                if (lastWidth > maxWidth)
                {
                    maxWidthIndex = lastImageIndex;
                    maxWidth = lastWidth;
                }
            }

            stream.setPosition (sectionDataStart);
        }

        stream.setPosition (sectionStart + sectionSize);
    }

    return maxWidthIndex == -1 ? juce::Image()
                               : images.getReference (maxWidthIndex).rescaled (size, size, Graphics::ResamplingQuality::highResamplingQuality);
}

Image JUCE_API getIconFromApplication (const String& applicationPath, const int size)
{
    if (auto pathCFString = CFUniquePtr<CFStringRef> (CFStringCreateWithCString (kCFAllocatorDefault, applicationPath.toRawUTF8(), kCFStringEncodingUTF8)))
    {
        if (auto url = CFUniquePtr<CFURLRef> (CFURLCreateWithFileSystemPath (kCFAllocatorDefault, pathCFString.get(), kCFURLPOSIXPathStyle, 1)))
        {
            if (auto appBundle = CFUniquePtr<CFBundleRef> (CFBundleCreate (kCFAllocatorDefault, url.get())))
            {
                if (CFTypeRef infoValue = CFBundleGetValueForInfoDictionaryKey (appBundle.get(), CFSTR ("CFBundleIconFile")))
                {
                    if (CFGetTypeID (infoValue) == CFStringGetTypeID())
                    {
                        CFStringRef iconFilename = reinterpret_cast<CFStringRef> (infoValue);
                        CFStringRef resourceURLSuffix = CFStringHasSuffix (iconFilename, CFSTR (".icns")) ? nullptr : CFSTR ("icns");

                        if (auto iconURL = CFUniquePtr<CFURLRef> (CFBundleCopyResourceURL (appBundle.get(), iconFilename, resourceURLSuffix, nullptr)))
                        {
                            if (auto iconPath = CFUniquePtr<CFStringRef> (CFURLCopyFileSystemPath (iconURL.get(), kCFURLPOSIXPathStyle)))
                            {
                                File icnsFile (CFStringGetCStringPtr (iconPath.get(), CFStringGetSystemEncoding()));
                                return getIconFromIcnsFile (icnsFile, size);
                            }
                        }
                    }
                }
            }
        }
    }

    return {};
}

} // namespace juce
