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

#if JUCE_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4390 4611 4365 4267)
 #ifdef __INTEL_COMPILER
  #pragma warning (disable: 2544 2545)
 #endif
#endif

namespace zlibNamespace
{
#if JUCE_INCLUDE_ZLIB_CODE
  #undef OS_CODE
  #undef fdopen
  #include "../../juce_core/zip/zlib/zlib.h"
  #undef OS_CODE
#else
  #include JUCE_ZLIB_INCLUDE_PATH
#endif
}

namespace pnglibNamespace
{
  using namespace zlibNamespace;

#if JUCE_INCLUDE_PNGLIB_CODE || ! defined (JUCE_INCLUDE_PNGLIB_CODE)

  #if _MSC_VER != 1310
   using std::calloc; // (causes conflict in VS.NET 2003)
   using std::malloc;
   using std::free;
  #endif

  #if JUCE_CLANG
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wsign-conversion"
  #endif

  using std::abs;
  #define PNG_INTERNAL
  #define NO_DUMMY_DECL
  #define PNG_SETJMP_NOT_SUPPORTED

  #include "pnglib/png.h"
  #include "pnglib/pngconf.h"

  #define PNG_NO_EXTERN
  #include "pnglib/png.c"
  #include "pnglib/pngerror.c"
  #include "pnglib/pngget.c"
  #include "pnglib/pngmem.c"
  #include "pnglib/pngread.c"
  #include "pnglib/pngpread.c"
  #include "pnglib/pngrio.c"
  #include "pnglib/pngrtran.c"
  #include "pnglib/pngrutil.c"
  #include "pnglib/pngset.c"
  #include "pnglib/pngtrans.c"
  #include "pnglib/pngwio.c"
  #include "pnglib/pngwrite.c"
  #include "pnglib/pngwtran.c"
  #include "pnglib/pngwutil.c"

  #if JUCE_CLANG
   #pragma clang diagnostic pop
  #endif
#else
  extern "C"
  {
    #include <png.h>
    #include <pngconf.h>
  }
#endif
}

#undef max
#undef min
#undef fdopen

#if JUCE_MSVC
 #pragma warning (pop)
#endif

//==============================================================================
namespace PNGHelpers
{
    using namespace pnglibNamespace;

    static void JUCE_CDECL writeDataCallback (png_structp png, png_bytep data, png_size_t length)
    {
        static_cast<OutputStream*> (png_get_io_ptr (png))->write (data, (int) length);
    }

   #if ! JUCE_USING_COREIMAGE_LOADER
    static void JUCE_CDECL readCallback (png_structp png, png_bytep data, png_size_t length)
    {
        static_cast<InputStream*> (png_get_io_ptr (png))->read (data, (int) length);
    }

    struct PNGErrorStruct {};

    static void JUCE_CDECL errorCallback (png_structp, png_const_charp)
    {
        throw PNGErrorStruct();
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
 Image juce_loadWithCoreImage (InputStream& input);
#endif

Image PNGImageFormat::decodeImage (InputStream& in)
{
#if JUCE_USING_COREIMAGE_LOADER
    return juce_loadWithCoreImage (in);
#else
    using namespace pnglibNamespace;
    Image image;

    png_structp pngReadStruct;
    png_infop pngInfoStruct;

    pngReadStruct = png_create_read_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if (pngReadStruct != 0)
    {
        try
        {
            pngInfoStruct = png_create_info_struct (pngReadStruct);

            if (pngInfoStruct == 0)
            {
                png_destroy_read_struct (&pngReadStruct, 0, 0);
                return Image::null;
            }

            png_set_error_fn (pngReadStruct, 0, PNGHelpers::errorCallback, PNGHelpers::errorCallback );

            // read the header..
            png_set_read_fn (pngReadStruct, &in, PNGHelpers::readCallback);

            png_uint_32 width, height;
            int bitDepth, colorType, interlaceType;

            png_read_info (pngReadStruct, pngInfoStruct);

            png_get_IHDR (pngReadStruct, pngInfoStruct,
                          &width, &height,
                          &bitDepth, &colorType,
                          &interlaceType, 0, 0);

            if (bitDepth == 16)
                png_set_strip_16 (pngReadStruct);

            if (colorType == PNG_COLOR_TYPE_PALETTE)
                png_set_expand (pngReadStruct);

            if (bitDepth < 8)
                png_set_expand (pngReadStruct);

            if (png_get_valid (pngReadStruct, pngInfoStruct, PNG_INFO_tRNS))
                png_set_expand (pngReadStruct);

            if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
                png_set_gray_to_rgb (pngReadStruct);

            png_set_add_alpha (pngReadStruct, 0xff, PNG_FILLER_AFTER);

            bool hasAlphaChan = (colorType & PNG_COLOR_MASK_ALPHA) != 0
                                  || pngInfoStruct->num_trans > 0;

            // Load the image into a temp buffer in the pnglib format..
            HeapBlock <uint8> tempBuffer (height * (width << 2));

            {
                HeapBlock <png_bytep> rows (height);
                for (int y = (int) height; --y >= 0;)
                    rows[y] = (png_bytep) (tempBuffer + (width << 2) * y);

                try
                {
                    png_read_image (pngReadStruct, rows);
                    png_read_end (pngReadStruct, pngInfoStruct);
                }
                catch (PNGHelpers::PNGErrorStruct&)
                {}
            }

            png_destroy_read_struct (&pngReadStruct, &pngInfoStruct, 0);

            // now convert the data to a juce image format..
            image = Image (hasAlphaChan ? Image::ARGB : Image::RGB,
                           (int) width, (int) height, hasAlphaChan);

            image.getProperties()->set ("originalImageHadAlpha", image.hasAlphaChannel());
            hasAlphaChan = image.hasAlphaChannel(); // (the native image creator may not give back what we expect)

            const Image::BitmapData destData (image, Image::BitmapData::writeOnly);
            uint8* srcRow = tempBuffer;
            uint8* destRow = destData.data;

            for (int y = 0; y < (int) height; ++y)
            {
                const uint8* src = srcRow;
                srcRow += (width << 2);
                uint8* dest = destRow;
                destRow += destData.lineStride;

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
        }
        catch (PNGHelpers::PNGErrorStruct&)
        {}
    }

    return image;
#endif
}

bool PNGImageFormat::writeImageToStream (const Image& image, OutputStream& out)
{
    using namespace pnglibNamespace;
    const int width = image.getWidth();
    const int height = image.getHeight();

    png_structp pngWriteStruct = png_create_write_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if (pngWriteStruct == 0)
        return false;

    png_infop pngInfoStruct = png_create_info_struct (pngWriteStruct);

    if (pngInfoStruct == 0)
    {
        png_destroy_write_struct (&pngWriteStruct, (png_infopp) 0);
        return false;
    }

    png_set_write_fn (pngWriteStruct, &out, PNGHelpers::writeDataCallback, 0);

    png_set_IHDR (pngWriteStruct, pngInfoStruct, (png_uint_32) width, (png_uint_32) height, 8,
                  image.hasAlphaChannel() ? PNG_COLOR_TYPE_RGB_ALPHA
                                          : PNG_COLOR_TYPE_RGB,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_BASE,
                  PNG_FILTER_TYPE_BASE);

    HeapBlock <uint8> rowData ((size_t) width * 4);

    png_color_8 sig_bit;
    sig_bit.red = 8;
    sig_bit.green = 8;
    sig_bit.blue = 8;
    sig_bit.alpha = 8;
    png_set_sBIT (pngWriteStruct, pngInfoStruct, &sig_bit);

    png_write_info (pngWriteStruct, pngInfoStruct);

    png_set_shift (pngWriteStruct, &sig_bit);
    png_set_packing (pngWriteStruct);

    const Image::BitmapData srcData (image, Image::BitmapData::readOnly);

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
