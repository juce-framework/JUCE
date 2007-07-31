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

#ifdef _MSC_VER
  #pragma warning (push)
#endif

#include "pnglib/png.h"

#ifdef _MSC_VER
  #pragma warning (pop)
#endif

#include "../../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "../juce_Image.h"
#include "../../../../../juce_core/io/juce_InputStream.h"
#include "../../../../../juce_core/io/juce_OutputStream.h"
#include "../../colour/juce_PixelFormats.h"


//==============================================================================
static void pngReadCallback (png_structp pngReadStruct, png_bytep data, png_size_t length) throw()
{
    InputStream* const in = (InputStream*) png_get_io_ptr (pngReadStruct);
    in->read (data, (int) length);
}

//==============================================================================
Image* juce_loadPNGImageFromStream (InputStream& in) throw()
{
    Image* image = 0;

    png_structp pngReadStruct;
    png_infop pngInfoStruct;

    pngReadStruct = png_create_read_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if (pngReadStruct != 0)
    {
        pngInfoStruct = png_create_info_struct (pngReadStruct);

        if (pngInfoStruct == 0)
        {
            png_destroy_read_struct (&pngReadStruct, 0, 0);
            return 0;
        }

        // read the header..
        png_set_read_fn (pngReadStruct, &in, pngReadCallback);
        png_read_info (pngReadStruct, pngInfoStruct);

        png_uint_32 width, height;
        int bitDepth, colorType, interlaceType;

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

        const bool hasAlphaChan = (colorType & PNG_COLOR_MASK_ALPHA) != 0
                                    || pngInfoStruct->num_trans > 0;

        // Load the image into a temp buffer in the pnglib format..
        uint8* const tempBuffer = (uint8*) juce_malloc (height * (width << 2));

        png_bytepp rows = (png_bytepp) juce_malloc (sizeof (png_bytep) * height);
        int y;
        for (y = (int) height; --y >= 0;)
            rows[y] = (png_bytep) (tempBuffer + (width << 2) * y);

        png_read_image (pngReadStruct, rows);

        juce_free (rows);

        png_read_end (pngReadStruct, pngInfoStruct);
        png_destroy_read_struct (&pngReadStruct, &pngInfoStruct, 0);

        // now convert the data to a juce image format..
        image = new Image (hasAlphaChan ? Image::ARGB : Image::RGB,
                           width, height, hasAlphaChan);

        int stride, pixelStride;
        uint8* const pixels = image->lockPixelDataReadWrite (0, 0, width, height, stride, pixelStride);
        uint8* srcRow = tempBuffer;
        uint8* destRow = pixels;

        for (y = 0; y < (int) height; ++y)
        {
            const uint8* src = srcRow;
            srcRow += (width << 2);
            uint8* dest = destRow;
            destRow += stride;

            if (hasAlphaChan)
            {
                for (int i = width; --i >= 0;)
                {
                    ((PixelARGB*) dest)->setARGB (src[3], src[0], src[1], src[2]);
                    ((PixelARGB*) dest)->premultiply();
                    dest += pixelStride;
                    src += 4;
                }
            }
            else
            {
                for (int i = width; --i >= 0;)
                {
                    ((PixelRGB*) dest)->setARGB (0, src[0], src[1], src[2]);
                    dest += pixelStride;
                    src += 4;
                }
            }
        }

        image->releasePixelDataReadWrite (pixels);
        juce_free (tempBuffer);
    }

    return image;
}

//==============================================================================
static void pngWriteDataCallback (png_structp png_ptr, png_bytep data, png_size_t length) throw()
{
    OutputStream* const out = (OutputStream*) png_ptr->io_ptr;

    const bool ok = out->write (data, length);

    (void) ok;
    jassert (ok);
}

bool juce_writePNGImageToStream (const Image& image, OutputStream& out) throw()
{
    const int width = image.getWidth();
    const int height = image.getHeight();

    png_structp pngWriteStruct = png_create_write_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if (pngWriteStruct == 0)
        return false;

    png_infop pngInfoStruct = png_create_info_struct (pngWriteStruct);

    if ((pngInfoStruct == 0)
        || setjmp (pngWriteStruct->jmpbuf))
    {
        png_destroy_write_struct (&pngWriteStruct, (png_infopp) 0);
        return false;
    }

    png_set_write_fn (pngWriteStruct, &out, pngWriteDataCallback, 0);

    png_set_IHDR (pngWriteStruct, pngInfoStruct, width, height, 8,
                  image.hasAlphaChannel() ? PNG_COLOR_TYPE_RGB_ALPHA
                                          : PNG_COLOR_TYPE_RGB,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_BASE,
                  PNG_FILTER_TYPE_BASE);

    png_bytep rowData = (png_bytep) juce_malloc (width * 4 * sizeof (png_byte));

    png_color_8 sig_bit;
    sig_bit.red = 8;
    sig_bit.green = 8;
    sig_bit.blue = 8;
    sig_bit.alpha = 8;
    png_set_sBIT (pngWriteStruct, pngInfoStruct, &sig_bit);

    png_write_info (pngWriteStruct, pngInfoStruct);

    png_set_shift (pngWriteStruct, &sig_bit);
    png_set_packing (pngWriteStruct);

    for (int y = 0; y < height; ++y)
    {
        uint8* dst = (uint8*) rowData;
        int stride, pixelStride;
        const uint8* pixels = image.lockPixelDataReadOnly (0, y, width, 1, stride, pixelStride);
        const uint8* src = pixels;

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
                src += pixelStride;
            }
        }
        else
        {
            for (int i = width; --i >= 0;)
            {
                *dst++ = ((const PixelRGB*) src)->getRed();
                *dst++ = ((const PixelRGB*) src)->getGreen();
                *dst++ = ((const PixelRGB*) src)->getBlue();
                src += pixelStride;
            }
        }

        png_write_rows (pngWriteStruct, &rowData, 1);
        image.releasePixelDataReadOnly (pixels);
    }

    juce_free (rowData);

    png_write_end (pngWriteStruct, pngInfoStruct);
    png_destroy_write_struct (&pngWriteStruct, &pngInfoStruct);

    out.flush();

    return true;
}


END_JUCE_NAMESPACE
