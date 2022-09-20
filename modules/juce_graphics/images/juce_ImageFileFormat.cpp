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
