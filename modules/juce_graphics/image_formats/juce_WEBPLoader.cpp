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
#if JUCE_INCLUDE_WEBPLIB_CODE

JUCE_BEGIN_IGNORE_WARNINGS_MSVC(4310 4127 4244 4005)

#undef MULTIPLIER

#include "webplib/webp/decode.h"
#include "webplib/dec/webp_dec.c"
#include "webplib/dec/buffer_dec.c"
#include "webplib/dec/vp8_dec.c"
#include "webplib/dec/vp8l_dec.c" 
#include "webplib/dec/io_dec.c"
#include "webplib/utils/rescaler_utils.c"
#include "webplib/utils/bit_reader_utils.c"
#include "webplib/dsp/yuv.c"
#include "webplib/dsp/upsampling.c"
#include "webplib/dsp/dec.c"
#include "webplib/dsp/dec_clip_tables.c"
#include "webplib/dsp/rescaler.c"
#include "webplib/dsp/alpha_processing.c"
#include "webplib/utils/color_cache_utils.c"
#include "webplib/utils/huffman_utils.c"
#include "webplib/utils/thread_utils.c"
#include "webplib/dec/tree_dec.c"
#include "webplib/dec/quant_dec.c"
#include "webplib/dec/frame_dec.c"
#include "webplib/utils/random_utils.c"
#include "webplib/dec/alpha_dec.c"
#include "webplib/dsp/filters.c"
#include "webplib/utils/utils.c"
#include "webplib/dsp/lossless.c"
#include "webplib/dsp/yuv_sse2.c"
#include "webplib/dsp/yuv_sse41.c"
#include "webplib/dsp/yuv_neon.c"
#include "webplib/dsp/upsampling_sse2.c"
#include "webplib/dsp/upsampling_sse41.c"
#include "webplib/dsp/upsampling_neon.c"
#include "webplib/dsp/dec_sse2.c"
#include "webplib/dsp/dec_sse41.c"
#include "webplib/dsp/dec_neon.c"
#include "webplib/dsp/rescaler_sse2.c"
#include "webplib/dsp/rescaler_neon.c"
#include "webplib/dsp/alpha_processing_sse2.c"
#include "webplib/dsp/alpha_processing_sse41.c"
#include "webplib/dsp/alpha_processing_neon.c"
#include "webplib/utils/quant_levels_dec_utils.c"
#include "webplib/dsp/filters_sse2.c"
#include "webplib/dsp/filters_neon.c"
#include "webplib/utils/palette.c"
#include "webplib/dsp/lossless_sse2.c"
#include "webplib/dsp/lossless_sse41.c"
#include "webplib/dsp/lossless_neon.c"
#include "webplib/dsp/cpu.c"

JUCE_END_IGNORE_WARNINGS_MSVC

#endif

namespace juce
{

//==============================================================================
class WEBPLoader
{
public:
    WEBPLoader (InputStream& in)
    {
        WEBPImageFormat imageFmt;
        image = imageFmt.decodeImage(in);
    }

    Image image;

private:
 

    JUCE_DECLARE_NON_COPYABLE (WEBPLoader)
};





//==============================================================================
WEBPImageFormat::WEBPImageFormat() {}
WEBPImageFormat::~WEBPImageFormat() {}

String WEBPImageFormat::getFormatName()                  { return "WEBP"; }
bool WEBPImageFormat::usesFileExtension (const File& f)  { return f.hasFileExtension ("webp"); }

bool WEBPImageFormat::canUnderstand (InputStream& in)
{
#if JUCE_INCLUDE_WEBPLIB_CODE
    int bytes = 1024;
    juce::MemoryBlock data(bytes);
    bytes = in.read(data.getData(), bytes);
    int width, height = 0;
    

    bool canUnderstand = WebPGetInfo((uint8_t*)data.getData(), bytes, &width, &height);
    return canUnderstand;
#else
    jassertfalse;
    return false;
#endif
}

Image WEBPImageFormat::decodeImage (InputStream& in)
{
#if JUCE_INCLUDE_WEBPLIB_CODE
    Image image;

    WebPBitstreamFeatures features;

    // read whole file from input stream
    size_t streamSize = in.getTotalLength();
    size_t bytesRead = 0;
    juce::MemoryBlock data(streamSize);
    bytesRead = in.read(data.getData(), streamSize);
    if (bytesRead != streamSize)
    {
        // did not ready expected amount of data
        jassertfalse;
        return image;
    }

    auto statusCode = WebPGetFeatures((uint8_t*)data.getData(), bytesRead, &features);
    if (statusCode != VP8_STATUS_OK)
    {
        jassertfalse;
        return image;
    }

    image = Image(features.has_alpha ? Image::ARGB : Image::RGB, features.width, features.height, true);
    Image::BitmapData destData(image, Image::BitmapData::writeOnly);
    uint8* p = destData.getPixelPointer(0, 0);
    
    if (features.has_alpha)
        jassert(NULL!=WebPDecodeBGRAInto((uint8_t*)data.getData(), bytesRead, p, destData.size, destData.lineStride));
    else 
        jassert(NULL!=WebPDecodeBGRInto((uint8_t*)data.getData(), bytesRead, p, destData.size, destData.lineStride));
        
    return image;
#else
    jassertfalse;
    return Image();
#endif
}

bool WEBPImageFormat::writeImageToStream (const Image& /*sourceImage*/, OutputStream& /*destStream*/)
{
    // Not yet implemented
    jassertfalse;
    return false;
}







} // namespace juce