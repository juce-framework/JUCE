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

ImageFileFormat* ImageFileFormat::findImageFormatForStream (InputStream& input)
{
    struct DefaultImageFormats
    {
        PNGImageFormat  png;
        JPEGImageFormat jpg;
        GIFImageFormat  gif;
    };

    static DefaultImageFormats defaultImageFormats;

    ImageFileFormat* formats[] = { &defaultImageFormats.png,
                                   &defaultImageFormats.jpg,
                                   &defaultImageFormats.gif };

    const int64 streamPos = input.getPosition();

    for (int i = 0; i < numElementsInArray (formats); ++i)
    {
        const bool found = formats[i]->canUnderstand (input);
        input.setPosition (streamPos);

        if (found)
            return formats[i];
    }

    return nullptr;
}

//==============================================================================
Image ImageFileFormat::loadFrom (InputStream& input)
{
    ImageFileFormat* const format = findImageFormatForStream (input);

    if (format != nullptr)
        return format->decodeImage (input);

    return Image::null;
}

Image ImageFileFormat::loadFrom (const File& file)
{
    FileInputStream stream (file);

    if (stream.openedOk())
    {
        BufferedInputStream b (stream, 8192);
        return loadFrom (b);
    }

    return Image::null;
}

Image ImageFileFormat::loadFrom (const void* rawData, const size_t numBytes)
{
    if (rawData != nullptr && numBytes > 4)
    {
        MemoryInputStream stream (rawData, numBytes, false);
        return loadFrom (stream);
    }

    return Image::null;
}
