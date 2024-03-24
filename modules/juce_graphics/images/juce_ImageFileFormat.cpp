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

struct DefaultImageFormats
{
    static ImageFileFormat** get()
    {
        static DefaultImageFormats formats;
        return formats.formats;
    }

private:
    DefaultImageFormats() noexcept
    {
        formats[0] = &png;
        formats[1] = &jpg;
        formats[2] = &gif;
        formats[3] = nullptr;
    }

    PNGImageFormat  png;
    JPEGImageFormat jpg;
    GIFImageFormat  gif;

    ImageFileFormat* formats[4];
};

ImageFileFormat* ImageFileFormat::findImageFormatForStream (InputStream& input)
{
    const int64 streamPos = input.getPosition();

    for (ImageFileFormat** i = DefaultImageFormats::get(); *i != nullptr; ++i)
    {
        const bool found = (*i)->canUnderstand (input);
        input.setPosition (streamPos);

        if (found)
            return *i;
    }

    return nullptr;
}

ImageFileFormat* ImageFileFormat::findImageFormatForFileExtension (const File& file)
{
    for (ImageFileFormat** i = DefaultImageFormats::get(); *i != nullptr; ++i)
        if ((*i)->usesFileExtension (file))
            return *i;

    return nullptr;
}

//==============================================================================
Image ImageFileFormat::loadFrom (InputStream& input)
{
    if (ImageFileFormat* format = findImageFormatForStream (input))
        return format->decodeImage (input);

    return Image();
}

Image ImageFileFormat::loadFrom (const File& file)
{
    FileInputStream stream (file);

    if (stream.openedOk())
    {
        BufferedInputStream b (stream, 8192);
        return loadFrom (b);
    }

    return Image();
}

Image ImageFileFormat::loadFrom (const void* rawData, const size_t numBytes)
{
    if (rawData != nullptr && numBytes > 4)
    {
        MemoryInputStream stream (rawData, numBytes, false);
        return loadFrom (stream);
    }

    return Image();
}

} // namespace juce
