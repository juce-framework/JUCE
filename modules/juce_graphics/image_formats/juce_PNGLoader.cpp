/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#if ! defined (jmp_buf) || ! defined (longjmp)
 #include <setjmp.h>
#endif

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

  #undef check
  using std::abs;
  #define NO_DUMMY_DECL
  #define PNGLCONF_H 1

 #if JUCE_ANDROID
  #define PNG_ARM_NEON_SUPPORTED
 #endif

  #define PNG_16BIT_SUPPORTED
  #define PNG_ALIGNED_MEMORY_SUPPORTED
  #define PNG_BENIGN_ERRORS_SUPPORTED
  #define PNG_BENIGN_READ_ERRORS_SUPPORTED
  #define PNG_BUILD_GRAYSCALE_PALETTE_SUPPORTED
  #define PNG_CHECK_FOR_INVALID_INDEX_SUPPORTED
  #define PNG_COLORSPACE_SUPPORTED
  #define PNG_CONSOLE_IO_SUPPORTED
  #define PNG_EASY_ACCESS_SUPPORTED
  #define PNG_FIXED_POINT_SUPPORTED
  #define PNG_FLOATING_ARITHMETIC_SUPPORTED
  #define PNG_FLOATING_POINT_SUPPORTED
  #define PNG_FORMAT_AFIRST_SUPPORTED
  #define PNG_FORMAT_BGR_SUPPORTED
  #define PNG_GAMMA_SUPPORTED
  #define PNG_GET_PALETTE_MAX_SUPPORTED
  #define PNG_HANDLE_AS_UNKNOWN_SUPPORTED
  #define PNG_INCH_CONVERSIONS_SUPPORTED
  #define PNG_INFO_IMAGE_SUPPORTED
  #define PNG_IO_STATE_SUPPORTED
  #define PNG_MNG_FEATURES_SUPPORTED
  #define PNG_POINTER_INDEXING_SUPPORTED
  #define PNG_PROGRESSIVE_READ_SUPPORTED
  #define PNG_READ_16BIT_SUPPORTED
  #define PNG_READ_ALPHA_MODE_SUPPORTED
  #define PNG_READ_ANCILLARY_CHUNKS_SUPPORTED
  #define PNG_READ_BACKGROUND_SUPPORTED
  #define PNG_READ_BGR_SUPPORTED
  #define PNG_READ_CHECK_FOR_INVALID_INDEX_SUPPORTED
  #define PNG_READ_COMPOSITE_NODIV_SUPPORTED
  #define PNG_READ_COMPRESSED_TEXT_SUPPORTED
  #define PNG_READ_EXPAND_16_SUPPORTED
  #define PNG_READ_EXPAND_SUPPORTED
  #define PNG_READ_FILLER_SUPPORTED
  #define PNG_READ_GAMMA_SUPPORTED
  #define PNG_READ_GET_PALETTE_MAX_SUPPORTED
  #define PNG_READ_GRAY_TO_RGB_SUPPORTED
  #define PNG_READ_INTERLACING_SUPPORTED
  #define PNG_READ_INT_FUNCTIONS_SUPPORTED
  #define PNG_READ_INVERT_ALPHA_SUPPORTED
  #define PNG_READ_INVERT_SUPPORTED
  #define PNG_READ_OPT_PLTE_SUPPORTED
  #define PNG_READ_PACKSWAP_SUPPORTED
  #define PNG_READ_PACK_SUPPORTED
  #define PNG_READ_QUANTIZE_SUPPORTED
  #define PNG_READ_RGB_TO_GRAY_SUPPORTED
  #define PNG_READ_SCALE_16_TO_8_SUPPORTED
  #define PNG_READ_SHIFT_SUPPORTED
  #define PNG_READ_STRIP_16_TO_8_SUPPORTED
  #define PNG_READ_STRIP_ALPHA_SUPPORTED
  #define PNG_READ_SUPPORTED
  #define PNG_READ_SWAP_ALPHA_SUPPORTED
  #define PNG_READ_SWAP_SUPPORTED
  #define PNG_READ_TEXT_SUPPORTED
  #define PNG_READ_TRANSFORMS_SUPPORTED
  #define PNG_READ_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_READ_USER_CHUNKS_SUPPORTED
  #define PNG_READ_USER_TRANSFORM_SUPPORTED
  #define PNG_READ_bKGD_SUPPORTED
  #define PNG_READ_cHRM_SUPPORTED
  #define PNG_READ_gAMA_SUPPORTED
  #define PNG_READ_hIST_SUPPORTED
  #define PNG_READ_iCCP_SUPPORTED
  #define PNG_READ_iTXt_SUPPORTED
  #define PNG_READ_oFFs_SUPPORTED
  #define PNG_READ_pCAL_SUPPORTED
  #define PNG_READ_pHYs_SUPPORTED
  #define PNG_READ_sBIT_SUPPORTED
  #define PNG_READ_sCAL_SUPPORTED
  #define PNG_READ_sPLT_SUPPORTED
  #define PNG_READ_sRGB_SUPPORTED
  #define PNG_READ_tEXt_SUPPORTED
  #define PNG_READ_tIME_SUPPORTED
  #define PNG_READ_tRNS_SUPPORTED
  #define PNG_READ_zTXt_SUPPORTED
  #define PNG_SAVE_INT_32_SUPPORTED
  #define PNG_SAVE_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_SEQUENTIAL_READ_SUPPORTED
  #define PNG_SET_CHUNK_CACHE_LIMIT_SUPPORTED
  #define PNG_SET_CHUNK_MALLOC_LIMIT_SUPPORTED
  #define PNG_SET_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_SET_USER_LIMITS_SUPPORTED
  #define PNG_SIMPLIFIED_READ_AFIRST_SUPPORTED
  #define PNG_SIMPLIFIED_READ_BGR_SUPPORTED
  #define PNG_SIMPLIFIED_WRITE_AFIRST_SUPPORTED
  #define PNG_SIMPLIFIED_WRITE_BGR_SUPPORTED
  #define PNG_STDIO_SUPPORTED
  #define PNG_STORE_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_TEXT_SUPPORTED
  #define PNG_TIME_RFC1123_SUPPORTED
  #define PNG_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_USER_CHUNKS_SUPPORTED
  #define PNG_USER_LIMITS_SUPPORTED
  #define PNG_USER_TRANSFORM_INFO_SUPPORTED
  #define PNG_USER_TRANSFORM_PTR_SUPPORTED
  #define PNG_WARNINGS_SUPPORTED
  #define PNG_WRITE_16BIT_SUPPORTED
  #define PNG_WRITE_ANCILLARY_CHUNKS_SUPPORTED
  #define PNG_WRITE_BGR_SUPPORTED
  #define PNG_WRITE_CHECK_FOR_INVALID_INDEX_SUPPORTED
  #define PNG_WRITE_COMPRESSED_TEXT_SUPPORTED
  #define PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED
  #define PNG_WRITE_FILLER_SUPPORTED
  #define PNG_WRITE_FILTER_SUPPORTED
  #define PNG_WRITE_FLUSH_SUPPORTED
  #define PNG_WRITE_GET_PALETTE_MAX_SUPPORTED
  #define PNG_WRITE_INTERLACING_SUPPORTED
  #define PNG_WRITE_INT_FUNCTIONS_SUPPORTED
  #define PNG_WRITE_INVERT_ALPHA_SUPPORTED
  #define PNG_WRITE_INVERT_SUPPORTED
  #define PNG_WRITE_OPTIMIZE_CMF_SUPPORTED
  #define PNG_WRITE_PACKSWAP_SUPPORTED
  #define PNG_WRITE_PACK_SUPPORTED
  #define PNG_WRITE_SHIFT_SUPPORTED
  #define PNG_WRITE_SUPPORTED
  #define PNG_WRITE_SWAP_ALPHA_SUPPORTED
  #define PNG_WRITE_SWAP_SUPPORTED
  #define PNG_WRITE_TEXT_SUPPORTED
  #define PNG_WRITE_TRANSFORMS_SUPPORTED
  #define PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_WRITE_USER_TRANSFORM_SUPPORTED
  #define PNG_WRITE_WEIGHTED_FILTER_SUPPORTED
  #define PNG_WRITE_bKGD_SUPPORTED
  #define PNG_WRITE_cHRM_SUPPORTED
  #define PNG_WRITE_gAMA_SUPPORTED
  #define PNG_WRITE_hIST_SUPPORTED
  #define PNG_WRITE_iCCP_SUPPORTED
  #define PNG_WRITE_iTXt_SUPPORTED
  #define PNG_WRITE_oFFs_SUPPORTED
  #define PNG_WRITE_pCAL_SUPPORTED
  #define PNG_WRITE_pHYs_SUPPORTED
  #define PNG_WRITE_sBIT_SUPPORTED
  #define PNG_WRITE_sCAL_SUPPORTED
  #define PNG_WRITE_sPLT_SUPPORTED
  #define PNG_WRITE_sRGB_SUPPORTED
  #define PNG_WRITE_tEXt_SUPPORTED
  #define PNG_WRITE_tIME_SUPPORTED
  #define PNG_WRITE_tRNS_SUPPORTED
  #define PNG_WRITE_zTXt_SUPPORTED
  #define PNG_bKGD_SUPPORTED
  #define PNG_cHRM_SUPPORTED
  #define PNG_gAMA_SUPPORTED
  #define PNG_hIST_SUPPORTED
  #define PNG_iCCP_SUPPORTED
  #define PNG_iTXt_SUPPORTED
  #define PNG_oFFs_SUPPORTED
  #define PNG_pCAL_SUPPORTED
  #define PNG_pHYs_SUPPORTED
  #define PNG_sBIT_SUPPORTED
  #define PNG_sCAL_SUPPORTED
  #define PNG_sPLT_SUPPORTED
  #define PNG_sRGB_SUPPORTED
  #define PNG_tEXt_SUPPORTED
  #define PNG_tIME_SUPPORTED
  #define PNG_tRNS_SUPPORTED
  #define PNG_zTXt_SUPPORTED

  #define PNG_STRING_COPYRIGHT "";
  #define PNG_STRING_NEWLINE "\n"
  #define PNG_LITERAL_SHARP 0x23
  #define PNG_LITERAL_LEFT_SQUARE_BRACKET 0x5b
  #define PNG_LITERAL_RIGHT_SQUARE_BRACKET 0x5d

  #define PNG_API_RULE 0
  #define PNG_CALLOC_SUPPORTED
  #define PNG_COST_SHIFT 3
  #define PNG_DEFAULT_READ_MACROS 1
  #define PNG_GAMMA_THRESHOLD_FIXED 5000
  #define PNG_IDAT_READ_SIZE PNG_ZBUF_SIZE
  #define PNG_INFLATE_BUF_SIZE 1024
  #define PNG_MAX_GAMMA_8 11
  #define PNG_QUANTIZE_BLUE_BITS 5
  #define PNG_QUANTIZE_GREEN_BITS 5
  #define PNG_QUANTIZE_RED_BITS 5
  #define PNG_TEXT_Z_DEFAULT_COMPRESSION (-1)
  #define PNG_TEXT_Z_DEFAULT_STRATEGY 0
  #define PNG_WEIGHT_SHIFT 8
  #define PNG_ZBUF_SIZE 8192
  #define PNG_Z_DEFAULT_COMPRESSION (-1)
  #define PNG_Z_DEFAULT_NOFILTER_STRATEGY 0
  #define PNG_Z_DEFAULT_STRATEGY 1
  #define PNG_sCAL_PRECISION 5
  #define PNG_sRGB_PROFILE_CHECKS 2

  #define png_debug(a, b)
  #define png_debug1(a, b, c)
  #define png_debug2(a, b, c, d)

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
        static_cast<OutputStream*> (png_get_io_ptr (png))->write (data, length);
    }

   #if ! JUCE_USING_COREIMAGE_LOADER
    static void JUCE_CDECL readCallback (png_structp png, png_bytep data, png_size_t length)
    {
        static_cast<InputStream*> (png_get_io_ptr (png))->read (data, (int) length);
    }

    struct PNGErrorStruct {};

    static void JUCE_CDECL errorCallback (png_structp p, png_const_charp)
    {
        longjmp (*(jmp_buf*) p->error_ptr, 1);
    }

    static void JUCE_CDECL warningCallback (png_structp, png_const_charp) {}

   #if JUCE_MSVC
    #pragma warning (push)
    #pragma warning (disable: 4611) // (warning about setjmp)
   #endif

    static bool readHeader (InputStream& in, png_structp pngReadStruct, png_infop pngInfoStruct, jmp_buf& errorJumpBuf,
                            png_uint_32& width, png_uint_32& height, int& bitDepth, int& colorType, int& interlaceType) noexcept
    {
        if (setjmp (errorJumpBuf) == 0)
        {
            // read the header..
            png_set_read_fn (pngReadStruct, &in, readCallback);

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

   #if JUCE_MSVC
    #pragma warning (pop)
   #endif

    static Image createImageFromData (bool hasAlphaChan, int width, int height, png_bytepp rows)
    {
        // now convert the data to a juce image format..
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
            // Load the image into a temp buffer..
            const size_t lineStride = width * 4;
            HeapBlock<uint8> tempBuffer (height * lineStride);
            HeapBlock<png_bytep> rows (height);

            for (size_t y = 0; y < height; ++y)
                rows[y] = (png_bytep) (tempBuffer + lineStride * y);

            if (readImageData (pngReadStruct, pngInfoStruct, errorJumpBuf, rows))
                return createImageFromData ((colorType & PNG_COLOR_MASK_ALPHA) != 0 || pngInfoStruct->num_trans > 0,
                                            (int) width, (int) height, rows);
        }

        return Image();
    }

    static Image readImage (InputStream& in)
    {
        if (png_structp pngReadStruct = png_create_read_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0))
        {
            if (png_infop pngInfoStruct = png_create_info_struct (pngReadStruct))
            {
                Image image (readImage (in, pngReadStruct, pngInfoStruct));
                png_destroy_read_struct (&pngReadStruct, &pngInfoStruct, 0);
                return image;
            }

            png_destroy_read_struct (&pngReadStruct, 0, 0);
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
    using namespace pnglibNamespace;
    const int width = image.getWidth();
    const int height = image.getHeight();

    png_structp pngWriteStruct = png_create_write_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if (pngWriteStruct == nullptr)
        return false;

    png_infop pngInfoStruct = png_create_info_struct (pngWriteStruct);

    if (pngInfoStruct == nullptr)
    {
        png_destroy_write_struct (&pngWriteStruct, (png_infopp) nullptr);
        return false;
    }

    png_set_write_fn (pngWriteStruct, &out, PNGHelpers::writeDataCallback, 0);

    png_set_IHDR (pngWriteStruct, pngInfoStruct, (png_uint_32) width, (png_uint_32) height, 8,
                  image.hasAlphaChannel() ? PNG_COLOR_TYPE_RGB_ALPHA
                                          : PNG_COLOR_TYPE_RGB,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_BASE,
                  PNG_FILTER_TYPE_BASE);

    HeapBlock<uint8> rowData ((size_t) width * 4);

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
