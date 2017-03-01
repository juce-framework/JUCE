/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

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

Image getIconFromIcnsFile (const File& icnsFile, const int size)
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
    Image hostIcon;

    if (CFStringRef pathCFString = CFStringCreateWithCString (kCFAllocatorDefault, applicationPath.toRawUTF8(), kCFStringEncodingUTF8))
    {
        if (CFURLRef url = CFURLCreateWithFileSystemPath (kCFAllocatorDefault, pathCFString, kCFURLPOSIXPathStyle, 1))
        {
            if (CFBundleRef appBundle = CFBundleCreate (kCFAllocatorDefault, url))
            {
                if (CFTypeRef infoValue = CFBundleGetValueForInfoDictionaryKey (appBundle, CFSTR("CFBundleIconFile")))
                {
                    if (CFGetTypeID (infoValue) == CFStringGetTypeID())
                    {
                        CFStringRef iconFilename = reinterpret_cast<CFStringRef> (infoValue);
                        CFStringRef resourceURLSuffix = CFStringHasSuffix (iconFilename, CFSTR(".icns")) ? nullptr : CFSTR("icns");
                        if (CFURLRef iconURL = CFBundleCopyResourceURL (appBundle, iconFilename, resourceURLSuffix, nullptr))
                        {
                            if (CFStringRef iconPath = CFURLCopyFileSystemPath (iconURL, kCFURLPOSIXPathStyle))
                            {
                                File icnsFile (CFStringGetCStringPtr (iconPath, CFStringGetSystemEncoding()));
                                hostIcon = getIconFromIcnsFile (icnsFile, size);
                                CFRelease (iconPath);
                            }
                            CFRelease (iconURL);
                        }
                    }
                }
                CFRelease (appBundle);
            }
            CFRelease (url);
        }
        CFRelease (pathCFString);
    }

    return hostIcon;
}
