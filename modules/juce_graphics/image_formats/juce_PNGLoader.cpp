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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

#if ! defined (JUCE_INCLUDE_PNGLIB_CODE) || JUCE_INCLUDE_PNGLIB_CODE
#include "juce_graphics/image_formats/pnglib/png.h"
#else
extern "C"
{
#include JUCE_PNGLIB_INCLUDE_PATH
}
#endif

namespace juce
{

//==============================================================================
namespace PNGHelpers
{
    static void JUCE_CDECL writeDataCallback (png_structp png, png_bytep data, png_size_t length)
    {
        static_cast<OutputStream*> (png_get_io_ptr (png))->write (data, length);
    }

    static void JUCE_CDECL errorCallback (png_structp p, png_const_charp)
    {
        JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4611)
       #ifdef PNG_SETJMP_SUPPORTED
        setjmp (png_jmpbuf (p));
       #else
        longjmp (*(jmp_buf*) p->error_ptr, 1);
       #endif
        JUCE_END_IGNORE_WARNINGS_MSVC
    }

    static void JUCE_CDECL warningCallback (png_structp, png_const_charp) {}

    #if ! JUCE_USING_COREIMAGE_LOADER
    static void JUCE_CDECL readCallback (png_structp png, png_bytep data, png_size_t length)
    {
        static_cast<InputStream*> (png_get_io_ptr (png))->read (data, (int) length);
    }

    struct PNGErrorStruct {};

    JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4611)

    static bool readHeader (InputStream& in, png_structp pngReadStruct, png_infop pngInfoStruct, jmp_buf& errorJumpBuf,
                            png_uint_32& width, png_uint_32& height, int& bitDepth, int& colorType, int& interlaceType) noexcept
    {
        if (setjmp (errorJumpBuf) == 0)
        {
            // read the header
            png_set_read_fn (pngReadStruct, &in, readCallback);

            png_read_info (pngReadStruct, pngInfoStruct);

            png_get_IHDR (pngReadStruct, pngInfoStruct,
                          &width, &height,
                          &bitDepth, &colorType,
                          &interlaceType, nullptr, nullptr);

            if (bitDepth == 16)
                png_set_strip_16 (pngReadStruct);

            if (colorType == PNG_COLOR_TYPE_PALETTE)
                png_set_expand (pngReadStruct);

            if (bitDepth < 8)
                png_set_expand (pngReadStruct);

            if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
                png_set_gray_to_rgb (pngReadStruct);

            return true;
        }

        return false;
    }

    static bool readImageData (png_structp pngReadStruct, png_infop pngInfoStruct, jmp_buf& errorJumpBuf, png_bytepp rows) noexcept
    {
        if (setjmp (errorJumpBuf) == 0)
        {
            if (png_get_valid (pngReadStruct, pngInfoStruct, PNG_INFO_tRNS))
                png_set_expand (pngReadStruct);

            png_set_add_alpha (pngReadStruct, 0xff, PNG_FILLER_AFTER);

            png_read_image (pngReadStruct, rows);
            png_read_end (pngReadStruct, pngInfoStruct);
            return true;
        }

        return false;
    }

    JUCE_END_IGNORE_WARNINGS_MSVC

    static Image createImageFromData (bool hasAlphaChan, int width, int height, png_bytepp rows)
    {
        // now convert the data to a juce image format
        Image image (hasAlphaChan ? Image::ARGB : Image::RGB, width, height, hasAlphaChan);

        image.getProperties()->set ("originalImageHadAlpha", image.hasAlphaChannel());
        hasAlphaChan = image.hasAlphaChannel(); // (the native image creator may not give back what we expect)

        const Image::BitmapData destData (image, Image::BitmapData::writeOnly);

        for (int y = 0; y < (int) height; ++y)
        {
            const uint8* src = rows[y];
            uint8* dest = destData.getLinePointer (y);

            if (hasAlphaChan)
            {
                for (int i = (int) width; --i >= 0;)
                {
                    ((PixelARGB*) dest)->setARGB (src[3], src[0], src[1], src[2]);
                    ((PixelARGB*) dest)->premultiply();
                    dest += destData.pixelStride;
                    src += 4;
                }
            }
            else
            {
                for (int i = (int) width; --i >= 0;)
                {
                    ((PixelRGB*) dest)->setARGB (0, src[0], src[1], src[2]);
                    dest += destData.pixelStride;
                    src += 4;
                }
            }
        }

        return image;
    }

    static Image readImage (InputStream& in, png_structp pngReadStruct, png_infop pngInfoStruct)
    {
        jmp_buf errorJumpBuf;
        png_set_error_fn (pngReadStruct, &errorJumpBuf, errorCallback, warningCallback);

        png_uint_32 width = 0, height = 0;
        int bitDepth = 0, colorType = 0, interlaceType = 0;

        if (readHeader (in, pngReadStruct, pngInfoStruct, errorJumpBuf,
                        width, height, bitDepth, colorType, interlaceType))
        {
            // load the image into a temp buffer
            const size_t lineStride = width * 4;
            HeapBlock<uint8> tempBuffer (height * lineStride);
            HeapBlock<png_bytep> rows (height);

            for (size_t y = 0; y < height; ++y)
                rows[y] = (png_bytep) (tempBuffer + lineStride * y);

            png_bytep trans_alpha = nullptr;
            png_color_16p trans_color = nullptr;
            int num_trans = 0;
            png_get_tRNS (pngReadStruct, pngInfoStruct, &trans_alpha, &num_trans, &trans_color);

            if (readImageData (pngReadStruct, pngInfoStruct, errorJumpBuf, rows))
                return createImageFromData ((colorType & PNG_COLOR_MASK_ALPHA) != 0 || num_trans != 0,
                                            (int) width, (int) height, rows);
        }

        return Image();
    }

    static Image readImage (InputStream& in)
    {
        if (png_structp pngReadStruct = png_create_read_struct (PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr))
        {
            if (png_infop pngInfoStruct = png_create_info_struct (pngReadStruct))
            {
                Image image (readImage (in, pngReadStruct, pngInfoStruct));
                png_destroy_read_struct (&pngReadStruct, &pngInfoStruct, nullptr);
                return image;
            }

            png_destroy_read_struct (&pngReadStruct, nullptr, nullptr);
        }

        return Image();
    }
   #endif
}

//==============================================================================
PNGImageFormat::PNGImageFormat()    {}
PNGImageFormat::~PNGImageFormat()   {}

String PNGImageFormat::getFormatName()                   { return "PNG"; }
bool PNGImageFormat::usesFileExtension (const File& f)   { return f.hasFileExtension ("png"); }

bool PNGImageFormat::canUnderstand (InputStream& in)
{
    const int bytesNeeded = 4;
    char header [bytesNeeded];

    return in.read (header, bytesNeeded) == bytesNeeded
            && header[1] == 'P'
            && header[2] == 'N'
            && header[3] == 'G';
}

#if JUCE_USING_COREIMAGE_LOADER
 Image juce_loadWithCoreImage (InputStream&);
#endif

Image PNGImageFormat::decodeImage (InputStream& in)
{
   #if JUCE_USING_COREIMAGE_LOADER
    return juce_loadWithCoreImage (in);
   #else
    return PNGHelpers::readImage (in);
   #endif
}

bool PNGImageFormat::writeImageToStream (const Image& image, OutputStream& out)
{
    if (! image.isValid())
        return false;

    auto width = image.getWidth();
    auto height = image.getHeight();

    HeapBlock<uint8> rowData (width * 4);
    const Image::BitmapData srcData (image, Image::BitmapData::readOnly);

    auto pngWriteStruct = png_create_write_struct (PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (pngWriteStruct == nullptr)
        return false;

    jmp_buf errorJumpBuf;
    png_set_error_fn (pngWriteStruct, &errorJumpBuf, PNGHelpers::errorCallback, PNGHelpers::warningCallback);

    JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4611)

    if (setjmp (errorJumpBuf) != 0)
        return false;

    JUCE_END_IGNORE_WARNINGS_MSVC

    auto pngInfoStruct = png_create_info_struct (pngWriteStruct);

    if (pngInfoStruct == nullptr)
    {
        png_destroy_write_struct (&pngWriteStruct, nullptr);
        return false;
    }

    png_set_write_fn (pngWriteStruct, &out, PNGHelpers::writeDataCallback, nullptr);

    png_set_IHDR (pngWriteStruct, pngInfoStruct, (png_uint_32) width, (png_uint_32) height, 8,
                  image.hasAlphaChannel() ? PNG_COLOR_TYPE_RGB_ALPHA
                                          : PNG_COLOR_TYPE_RGB,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_BASE,
                  PNG_FILTER_TYPE_BASE);

    png_color_8 sig_bit;
    sig_bit.red   = 8;
    sig_bit.green = 8;
    sig_bit.blue  = 8;
    sig_bit.gray  = 0;
    sig_bit.alpha = 8;
    png_set_sBIT (pngWriteStruct, pngInfoStruct, &sig_bit);

    png_write_info (pngWriteStruct, pngInfoStruct);

    png_set_shift (pngWriteStruct, &sig_bit);
    png_set_packing (pngWriteStruct);

    for (int y = 0; y < height; ++y)
    {
        uint8* dst = rowData;
        const uint8* src = srcData.getLinePointer (y);

        if (image.hasAlphaChannel())
        {
            for (int i = width; --i >= 0;)
            {
                PixelARGB p (*(const PixelARGB*) src);
                p.unpremultiply();

                *dst++ = p.getRed();
                *dst++ = p.getGreen();
                *dst++ = p.getBlue();
                *dst++ = p.getAlpha();
                src += srcData.pixelStride;
            }
        }
        else
        {
            for (int i = width; --i >= 0;)
            {
                *dst++ = ((const PixelRGB*) src)->getRed();
                *dst++ = ((const PixelRGB*) src)->getGreen();
                *dst++ = ((const PixelRGB*) src)->getBlue();
                src += srcData.pixelStride;
            }
        }

        png_bytep rowPtr = rowData;
        png_write_rows (pngWriteStruct, &rowPtr, 1);
    }

    png_write_end (pngWriteStruct, pngInfoStruct);
    png_destroy_write_struct (&pngWriteStruct, &pngInfoStruct);

    return true;
}

} // namespace juce
