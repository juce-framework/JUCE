/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_GIFLoader.h"
#include "../../colour/juce_PixelFormats.h"


//==============================================================================
GIFLoader::GIFLoader (InputStream& in)
    : image (0),
      input (in),
      dataBlockIsZero (false),
      fresh (false),
      finished (false)
{
    currentBit = lastBit = lastByteIndex = 0;
    maxCode = maxCodeSize = codeSize = setCodeSize = 0;
    firstcode = oldcode = 0;
    clearCode = end_code = 0;

    int imageWidth, imageHeight;
    int transparent = -1;

    if (! getSizeFromHeader (imageWidth, imageHeight))
        return;

    if ((imageWidth <= 0) || (imageHeight <= 0))
        return;

    unsigned char buf [16];
    if (in.read (buf, 3) != 3)
        return;

    int numColours = 2 << (buf[0] & 7);

    if ((buf[0] & 0x80) != 0)
        readPalette (numColours);

    for (;;)
    {
        if (input.read (buf, 1) != 1)
            break;

        if (buf[0] == ';')
            break;

        if (buf[0] == '!')
        {
            if (input.read (buf, 1) != 1)
                break;

            if (processExtension (buf[0], transparent) < 0)
                break;

            continue;
        }

        if (buf[0] != ',')
            continue;

        if (input.read (buf, 9) != 9)
            break;

        imageWidth  = makeWord (buf[4], buf[5]);
        imageHeight = makeWord (buf[6], buf[7]);

        numColours = 2 << (buf[8] & 7);

        if ((buf[8] & 0x80) != 0)
            if (! readPalette (numColours))
                break;

        image = Image::createNativeImage ((transparent >= 0) ? Image::ARGB : Image::RGB,
                                          imageWidth, imageHeight, (transparent >= 0));

        readImage (imageWidth, imageHeight,
                   (buf[8] & 0x40) != 0,
                   transparent);

        break;
    }
}

GIFLoader::~GIFLoader()
{
}

bool GIFLoader::getSizeFromHeader (int& w, int& h)
{
    unsigned char b [8];

    if (input.read (b, 6) == 6)
    {
        if ((strncmp ("GIF87a", (char*) b, 6) == 0)
             || (strncmp ("GIF89a", (char*) b, 6) == 0))
        {
            if (input.read (b, 4) == 4)
            {
                w = makeWord (b[0], b[1]);
                h = makeWord (b[2], b[3]);
                return true;
            }
        }
    }

    return false;
}

bool GIFLoader::readPalette (const int numCols)
{
    unsigned char rgb[4];

    for (int i = 0; i < numCols; ++i)
    {
        input.read (rgb, 3);

        palette [i][0] = rgb[0];
        palette [i][1] = rgb[1];
        palette [i][2] = rgb[2];
        palette [i][3] = 0xff;
    }

    return true;
}

int GIFLoader::readDataBlock (unsigned char* const dest)
{
    unsigned char n;

    if (input.read (&n, 1) == 1)
    {
        dataBlockIsZero = (n == 0);

        if (dataBlockIsZero || (input.read (dest, n) == n))
            return n;
    }

    return -1;
}

int GIFLoader::processExtension (const int type, int& transparent)
{
    unsigned char b [300];
    int n = 0;

    if (type == 0xf9)
    {
        n = readDataBlock (b);
        if (n < 0)
            return 1;

        if ((b[0] & 0x1) != 0)
            transparent = b[3];
    }

    do
    {
        n = readDataBlock (b);
    }
    while (n > 0);

    return n;
}

int GIFLoader::getCode (const int codeSize_, const bool initialise)
{
    if (initialise)
    {
        currentBit = 0;
        lastBit = 0;
        finished = false;
        return 0;
    }

    if ((currentBit + codeSize_) >= lastBit)
    {
        if (finished)
            return -1;

        buffer[0] = buffer [lastByteIndex - 2];
        buffer[1] = buffer [lastByteIndex - 1];

        const int n = readDataBlock (&buffer[2]);

        if (n == 0)
            finished = true;

        lastByteIndex = 2 + n;
        currentBit = (currentBit - lastBit) + 16;
        lastBit = (2 + n) * 8 ;
    }

    int result = 0;
    int i = currentBit;

    for (int j = 0; j < codeSize_; ++j)
    {
        result |= ((buffer[i >> 3] & (1 << (i & 7))) != 0) << j;
        ++i;
    }

    currentBit += codeSize_;

    return result;
}

int GIFLoader::readLZWByte (const bool initialise, const int inputCodeSize)
{
    int code, incode, i;

    if (initialise)
    {
        setCodeSize = inputCodeSize;
        codeSize = setCodeSize + 1;
        clearCode = 1 << setCodeSize;
        end_code = clearCode + 1;
        maxCodeSize = 2 * clearCode;
        maxCode = clearCode + 2;

        getCode (0, true);

        fresh = true;

        for (i = 0; i < clearCode; ++i)
        {
            table[0][i] = 0;
            table[1][i] = i;
        }

        for (; i < maxGifCode; ++i)
        {
            table[0][i] = 0;
            table[1][i] = 0;
        }

        sp = stack;

        return 0;
    }
    else if (fresh)
    {
        fresh = false;

        do
        {
            firstcode = oldcode
                = getCode (codeSize, false);
        }
        while (firstcode == clearCode);

        return firstcode;
    }

    if (sp > stack)
        return *--sp;

    while ((code = getCode (codeSize, false)) >= 0)
    {
        if (code == clearCode)
        {
            for (i = 0; i < clearCode; ++i)
            {
                table[0][i] = 0;
                table[1][i] = i;
            }

            for (; i < maxGifCode; ++i)
            {
                table[0][i] = 0;
                table[1][i] = 0;
            }

            codeSize = setCodeSize + 1;
            maxCodeSize = 2 * clearCode;
            maxCode = clearCode + 2;
            sp = stack;
            firstcode = oldcode = getCode (codeSize, false);
            return firstcode;

        }
        else if (code == end_code)
        {
            if (dataBlockIsZero)
                return -2;

            unsigned char buf [260];

            int n;
            while ((n = readDataBlock (buf)) > 0)
            {}

            if (n != 0)
                return -2;
        }

        incode = code;

        if (code >= maxCode)
        {
            *sp++ = firstcode;
            code = oldcode;
        }

        while (code >= clearCode)
        {
            *sp++ = table[1][code];
            if (code == table[0][code])
                return -2;

            code = table[0][code];
        }

        *sp++ = firstcode = table[1][code];

        if ((code = maxCode) < maxGifCode)
        {
            table[0][code] = oldcode;
            table[1][code] = firstcode;
            ++maxCode;

            if ((maxCode >= maxCodeSize)
                && (maxCodeSize < maxGifCode))
            {
                maxCodeSize <<= 1;
                ++codeSize;
            }
        }

        oldcode = incode;

        if (sp > stack)
            return *--sp;
    }

    return code;
}

bool GIFLoader::readImage (const int width, const int height,
                           const int interlace, const int transparent)
{
    unsigned char c;

    if (input.read (&c, 1) != 1
         || readLZWByte (true, c) < 0)
        return false;

    if (transparent >= 0)
    {
        palette [transparent][0] = 0;
        palette [transparent][1] = 0;
        palette [transparent][2] = 0;
        palette [transparent][3] = 0;
    }

    int index;
    int xpos = 0, ypos = 0, pass = 0;

    const Image::BitmapData destData (*image, 0, 0, width, height, true);
    uint8* p = destData.data;
    const bool hasAlpha = image->hasAlphaChannel();

    while ((index = readLZWByte (false, c)) >= 0)
    {
        const uint8* const paletteEntry = palette [index];

        if (hasAlpha)
        {
            ((PixelARGB*) p)->setARGB (paletteEntry[3],
                                       paletteEntry[0],
                                       paletteEntry[1],
                                       paletteEntry[2]);

            ((PixelARGB*) p)->premultiply();
        }
        else
        {
            ((PixelRGB*) p)->setARGB (0,
                                      paletteEntry[0],
                                      paletteEntry[1],
                                      paletteEntry[2]);
        }

        p += destData.pixelStride;
        ++xpos;

        if (xpos == width)
        {
            xpos = 0;

            if (interlace)
            {
                switch (pass)
                {
                case 0:
                case 1:
                    ypos += 8;
                    break;
                case 2:
                    ypos += 4;
                    break;
                case 3:
                    ypos += 2;
                    break;
                }

                while (ypos >= height)
                {
                    ++pass;

                    switch (pass)
                    {
                    case 1:
                        ypos = 4;
                        break;
                    case 2:
                        ypos = 2;
                        break;
                    case 3:
                        ypos = 1;
                        break;
                    default:
                        return true;
                    }
                }
            }
            else
            {
                ++ypos;
            }

            p = destData.getPixelPointer (xpos, ypos);
        }

        if (ypos >= height)
            break;
    }

    return true;
}


END_JUCE_NAMESPACE
