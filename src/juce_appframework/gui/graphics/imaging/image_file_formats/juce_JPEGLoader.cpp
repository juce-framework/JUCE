/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../../juce_core/basics/juce_StandardHeader.h"

#if JUCE_MSVC
  #pragma warning (push)
#endif

extern "C"
{
#include "jpglib/jpeglib.h"
}

#if JUCE_MSVC
  #pragma warning (pop)
#endif

BEGIN_JUCE_NAMESPACE


#include "../juce_Image.h"
#include "../../../../../juce_core/io/juce_InputStream.h"
#include "../../../../../juce_core/io/juce_OutputStream.h"
#include "../../colour/juce_PixelFormats.h"


//==============================================================================
static void silentErrorCallback1 (j_common_ptr)
{
}

static void silentErrorCallback2 (j_common_ptr, int)
{
}

static void silentErrorCallback3 (j_common_ptr, char*)
{
}

static void setupSilentErrorHandler (struct jpeg_error_mgr& err)
{
    zerostruct (err);

    err.error_exit = silentErrorCallback1;
    err.emit_message = silentErrorCallback2;
    err.output_message = silentErrorCallback1;
    err.format_message = silentErrorCallback3;
    err.reset_error_mgr = silentErrorCallback1;
}


//==============================================================================
static void dummyCallback1 (j_decompress_ptr) throw()
{
}

static void jpegSkip (j_decompress_ptr decompStruct, long num) throw()
{
    decompStruct->src->next_input_byte += num;
    decompStruct->src->bytes_in_buffer -= num;
}

static boolean jpegFill (j_decompress_ptr) throw()
{
    return 0;
}

//==============================================================================
Image* juce_loadJPEGImageFromStream (InputStream& in) throw()
{
    MemoryBlock mb;
    in.readIntoMemoryBlock (mb);

    Image* image = 0;

    if (mb.getSize() > 16)
    {
        struct jpeg_decompress_struct jpegDecompStruct;

        struct jpeg_error_mgr jerr;
        setupSilentErrorHandler (jerr);
        jpegDecompStruct.err = &jerr;

        jpeg_create_decompress (&jpegDecompStruct);

        jpegDecompStruct.src = (jpeg_source_mgr*)(jpegDecompStruct.mem->alloc_small)
            ((j_common_ptr)(&jpegDecompStruct), JPOOL_PERMANENT, sizeof (jpeg_source_mgr));

        jpegDecompStruct.src->init_source       = dummyCallback1;
        jpegDecompStruct.src->fill_input_buffer = jpegFill;
        jpegDecompStruct.src->skip_input_data   = jpegSkip;
        jpegDecompStruct.src->resync_to_restart = jpeg_resync_to_restart;
        jpegDecompStruct.src->term_source       = dummyCallback1;

        jpegDecompStruct.src->next_input_byte   = (const unsigned char*) mb.getData();
        jpegDecompStruct.src->bytes_in_buffer   = mb.getSize();

        jpeg_read_header (&jpegDecompStruct, TRUE);

        jpeg_calc_output_dimensions (&jpegDecompStruct);

        const int width = jpegDecompStruct.output_width;
        const int height = jpegDecompStruct.output_height;

        jpegDecompStruct.out_color_space = JCS_RGB;

        JSAMPARRAY buffer
            = (*jpegDecompStruct.mem->alloc_sarray) ((j_common_ptr) &jpegDecompStruct,
                                                     JPOOL_IMAGE,
                                                     width * 3, 1);

        if (jpeg_start_decompress (&jpegDecompStruct))
        {
            image = new Image (Image::RGB, width, height, false);

            for (int y = 0; y < height; ++y)
            {
                jpeg_read_scanlines (&jpegDecompStruct, buffer, 1);

                int stride, pixelStride;
                uint8* pixels = image->lockPixelDataReadWrite (0, y, width, 1, stride, pixelStride);
                const uint8* src = *buffer;
                uint8* dest = pixels;

                for (int i = width; --i >= 0;)
                {
                    ((PixelRGB*) dest)->setARGB (0, src[0], src[1], src[2]);
                    dest += pixelStride;
                    src += 3;
                }

                image->releasePixelDataReadWrite (pixels);
            }

            jpeg_finish_decompress (&jpegDecompStruct);
        }

        jpeg_destroy_decompress (&jpegDecompStruct);
    }

    return image;
}


//==============================================================================
static const int bufferSize = 512;

struct JuceJpegDest  : public jpeg_destination_mgr
{
    OutputStream* output;
    char* buffer;
};

static void jpegWriteInit (j_compress_ptr) throw()
{
}

static void jpegWriteTerminate (j_compress_ptr cinfo) throw()
{
    JuceJpegDest* const dest = (JuceJpegDest*) cinfo->dest;

    const int numToWrite = bufferSize - dest->free_in_buffer;
    dest->output->write (dest->buffer, numToWrite);
}

static boolean jpegWriteFlush (j_compress_ptr cinfo) throw()
{
    JuceJpegDest* const dest = (JuceJpegDest*) cinfo->dest;

    const int numToWrite = bufferSize;

    dest->next_output_byte = (JOCTET*) dest->buffer;
    dest->free_in_buffer = bufferSize;

    return dest->output->write (dest->buffer, numToWrite);
}

//==============================================================================
bool juce_writeJPEGImageToStream (const Image& image,
                                  OutputStream& out,
                                  float quality) throw()
{
    if (image.hasAlphaChannel())
    {
        // this method could fill the background in white and still save the image..
        jassertfalse
        return true;
    }

    struct jpeg_compress_struct jpegCompStruct;

    struct jpeg_error_mgr jerr;
    setupSilentErrorHandler (jerr);
    jpegCompStruct.err = &jerr;

    jpeg_create_compress (&jpegCompStruct);

    JuceJpegDest dest;
    jpegCompStruct.dest = &dest;

    dest.output = &out;
    dest.buffer = (char*) juce_malloc (bufferSize);
    dest.next_output_byte = (JOCTET*) dest.buffer;
    dest.free_in_buffer = bufferSize;
    dest.init_destination = jpegWriteInit;
    dest.empty_output_buffer = jpegWriteFlush;
    dest.term_destination = jpegWriteTerminate;

    jpegCompStruct.image_width = image.getWidth();
    jpegCompStruct.image_height = image.getHeight();
    jpegCompStruct.input_components = 3;
    jpegCompStruct.in_color_space = JCS_RGB;
    jpegCompStruct.write_JFIF_header = 1;

    jpegCompStruct.X_density = 72;
    jpegCompStruct.Y_density = 72;

    jpeg_set_defaults (&jpegCompStruct);

    jpegCompStruct.dct_method = JDCT_FLOAT;
    jpegCompStruct.optimize_coding = 1;
//    jpegCompStruct.smoothing_factor = 10;

    if (quality < 0.0f)
        quality = 6.0f;

    jpeg_set_quality (&jpegCompStruct, jlimit (0, 100, roundFloatToInt (quality * 100.0f)), TRUE);

    jpeg_start_compress (&jpegCompStruct, TRUE);

    const int strideBytes = jpegCompStruct.image_width * jpegCompStruct.input_components;

    JSAMPARRAY buffer = (*jpegCompStruct.mem->alloc_sarray) ((j_common_ptr) &jpegCompStruct,
                                                    JPOOL_IMAGE,
                                                    strideBytes, 1);

    while (jpegCompStruct.next_scanline < jpegCompStruct.image_height)
    {
        int stride, pixelStride;
        const uint8* pixels = image.lockPixelDataReadOnly (0, jpegCompStruct.next_scanline, jpegCompStruct.image_width, 1, stride, pixelStride);
        const uint8* src = pixels;
        uint8* dst = *buffer;

        for (int i = jpegCompStruct.image_width; --i >= 0;)
        {
            *dst++ = ((const PixelRGB*) src)->getRed();
            *dst++ = ((const PixelRGB*) src)->getGreen();
            *dst++ = ((const PixelRGB*) src)->getBlue();
            src += pixelStride;
        }

        jpeg_write_scanlines (&jpegCompStruct, buffer, 1);
        image.releasePixelDataReadOnly (pixels);
    }

    jpeg_finish_compress (&jpegCompStruct);
    jpeg_destroy_compress (&jpegCompStruct);

    juce_free (dest.buffer);

    out.flush();

    return true;
}


END_JUCE_NAMESPACE
