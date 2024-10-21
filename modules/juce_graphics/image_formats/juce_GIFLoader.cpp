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

#if (JUCE_MAC || JUCE_IOS) && USE_COREGRAPHICS_RENDERING && JUCE_USE_COREIMAGE_LOADER
 Image juce_loadWithCoreImage (InputStream& input);
#else

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6385)

//==============================================================================
class GIFLoader
{
public:
    GIFLoader (InputStream& in)
        : input (in),
          dataBlockIsZero (false), fresh (false), finished (false),
          currentBit (0), lastBit (0), lastByteIndex (0),
          codeSize (0), setCodeSize (0), maxCode (0), maxCodeSize (0),
          firstcode (0), oldcode (0), clearCode (0), endCode (0)
    {
        int imageWidth, imageHeight;
        if (! getSizeFromHeader (imageWidth, imageHeight))
            return;

        uint8 buf [16];
        if (in.read (buf, 3) != 3)
            return;

        int numColours = 2 << (buf[0] & 7);
        int transparent = -1;

        if ((buf[0] & 0x80) != 0)
            readPalette (numColours);

        for (;;)
        {
            if (input.read (buf, 1) != 1 || buf[0] == ';')
                break;

            if (buf[0] == '!')
            {
                if (readExtension (transparent))
                    continue;

                break;
            }

            if (buf[0] != ',')
                continue;

            if (input.read (buf, 9) == 9)
            {
                imageWidth  = (int) ByteOrder::littleEndianShort (buf + 4);
                imageHeight = (int) ByteOrder::littleEndianShort (buf + 6);

                numColours = 2 << (buf[8] & 7);

                if ((buf[8] & 0x80) != 0)
                    if (! readPalette (numColours))
                        break;

                image = Image (transparent >= 0 ? Image::ARGB : Image::RGB,
                               imageWidth, imageHeight, transparent >= 0);

                image.getProperties()->set ("originalImageHadAlpha", transparent >= 0);

                readImage ((buf[8] & 0x40) != 0, transparent);
            }

            break;
        }
    }

    Image image;

private:
    InputStream& input;
    uint8 buffer [260];
    PixelARGB palette [256];
    bool dataBlockIsZero, fresh, finished;
    int currentBit, lastBit, lastByteIndex;
    int codeSize, setCodeSize;
    int maxCode, maxCodeSize;
    int firstcode, oldcode;
    int clearCode, endCode;
    enum { maxGifCode = 1 << 12 };
    int table [2] [maxGifCode];
    int stack [2 * maxGifCode];
    int* sp;

    bool getSizeFromHeader (int& w, int& h)
    {
        // Add an extra byte for the zero terminator
        char b[7]{};

        if (input.read (b, 6) == 6
             && (strncmp ("GIF87a", b, 6) == 0
                  || strncmp ("GIF89a", b, 6) == 0))
        {
            if (input.read (b, 4) == 4)
            {
                w = (int) ByteOrder::littleEndianShort (b);
                h = (int) ByteOrder::littleEndianShort (b + 2);
                return w > 0 && h > 0;
            }
        }

        return false;
    }

    bool readPalette (const int numCols)
    {
        for (int i = 0; i < numCols; ++i)
        {
            uint8 rgb[4];
            input.read (rgb, 3);

            palette[i].setARGB (0xff, rgb[0], rgb[1], rgb[2]);
            palette[i].premultiply();
        }

        return true;
    }

    int readDataBlock (uint8* const dest)
    {
        uint8 n;
        if (input.read (&n, 1) == 1)
        {
            dataBlockIsZero = (n == 0);

            if (dataBlockIsZero || (input.read (dest, n) == n))
                return n;
        }

        return -1;
    }

    int readExtension (int& transparent)
    {
        uint8 type;
        if (input.read (&type, 1) != 1)
            return false;

        uint8 b [260];
        int n = 0;

        if (type == 0xf9)
        {
            n = readDataBlock (b);
            if (n < 0)
                return 1;

            if ((b[0] & 1) != 0)
                transparent = b[3];
        }

        do
        {
            n = readDataBlock (b);
        }
        while (n > 0);

        return n >= 0;
    }

    void clearTable()
    {
        int i;
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
    }

    void initialise (const int inputCodeSize)
    {
        setCodeSize = inputCodeSize;
        codeSize = setCodeSize + 1;
        clearCode = 1 << setCodeSize;
        endCode = clearCode + 1;
        maxCodeSize = 2 * clearCode;
        maxCode = clearCode + 2;

        getCode (0, true);

        fresh = true;
        clearTable();
        sp = stack;
    }

    int readLZWByte()
    {
        if (fresh)
        {
            fresh = false;

            for (;;)
            {
                firstcode = oldcode = getCode (codeSize, false);

                if (firstcode != clearCode)
                    return firstcode;
            }
        }

        if (sp > stack)
            return *--sp;

        int code;

        while ((code = getCode (codeSize, false)) >= 0)
        {
            if (code == clearCode)
            {
                clearTable();
                codeSize = setCodeSize + 1;
                maxCodeSize = 2 * clearCode;
                maxCode = clearCode + 2;
                sp = stack;
                firstcode = oldcode = getCode (codeSize, false);
                return firstcode;
            }
            else if (code == endCode)
            {
                if (dataBlockIsZero)
                    return -2;

                uint8 buf [260];
                int n;

                while ((n = readDataBlock (buf)) > 0)
                {}

                if (n != 0)
                    return -2;
            }

            const int incode = code;

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

                if (maxCode >= maxCodeSize && maxCodeSize < maxGifCode)
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

    int getCode (const int codeSize_, const bool shouldInitialise)
    {
        if (shouldInitialise)
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

            buffer[0] = buffer [jmax (0, lastByteIndex - 2)];
            buffer[1] = buffer [jmax (0, lastByteIndex - 1)];

            const int n = readDataBlock (buffer + 2);

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

    bool readImage (const int interlace, const int transparent)
    {
        uint8 c;
        if (input.read (&c, 1) != 1)
            return false;

        initialise (c);

        if (transparent >= 0)
            palette [transparent].setARGB (0, 0, 0, 0);

        int xpos = 0, ypos = 0, yStep = 8, pass = 0;

        const Image::BitmapData destData (image, Image::BitmapData::writeOnly);
        uint8* p = destData.getPixelPointer (0, 0);
        const bool hasAlpha = image.hasAlphaChannel();

        for (;;)
        {
            const int index = readLZWByte();
            if (index < 0)
                break;

            if (hasAlpha)
                ((PixelARGB*) p)->set (palette [index]);
            else
                ((PixelRGB*)  p)->set (palette [index]);

            p += destData.pixelStride;

            if (++xpos == destData.width)
            {
                xpos = 0;

                if (interlace)
                {
                    ypos += yStep;

                    while (ypos >= destData.height)
                    {
                        switch (++pass)
                        {
                            case 1:     ypos = 4; yStep = 8; break;
                            case 2:     ypos = 2; yStep = 4; break;
                            case 3:     ypos = 1; yStep = 2; break;
                            default:    return true;
                        }
                    }
                }
                else
                {
                    if (++ypos >= destData.height)
                        break;
                }

                p = destData.getPixelPointer (xpos, ypos);
            }
        }

        return true;
    }

    JUCE_DECLARE_NON_COPYABLE (GIFLoader)
};

JUCE_END_IGNORE_WARNINGS_MSVC

#endif

//==============================================================================
GIFImageFormat::GIFImageFormat() {}
GIFImageFormat::~GIFImageFormat() {}

String GIFImageFormat::getFormatName()                  { return "GIF"; }
bool GIFImageFormat::usesFileExtension (const File& f)  { return f.hasFileExtension ("gif"); }

bool GIFImageFormat::canUnderstand (InputStream& in)
{
    char header [4];

    return (in.read (header, sizeof (header)) == (int) sizeof (header))
             && header[0] == 'G'
             && header[1] == 'I'
             && header[2] == 'F';
}

Image GIFImageFormat::decodeImage (InputStream& in)
{
   #if (JUCE_MAC || JUCE_IOS) && USE_COREGRAPHICS_RENDERING && JUCE_USE_COREIMAGE_LOADER
    return juce_loadWithCoreImage (in);
   #else
    const std::unique_ptr<GIFLoader> loader (new GIFLoader (in));
    return loader->image;
   #endif
}

bool GIFImageFormat::writeImageToStream (const Image& /*sourceImage*/, OutputStream& /*destStream*/)
{
    jassertfalse; // writing isn't implemented for GIFs!
    return false;
}

} // namespace juce
