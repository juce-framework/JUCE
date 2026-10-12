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

#if JUCE_USE_WEBP

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant")

#if ! defined (JUCE_INCLUDE_WEBPLIB_CODE) || JUCE_INCLUDE_WEBPLIB_CODE
 #include "juce_graphics/image_formats/libwebp/src/webp/decode.h"
 #include "juce_graphics/image_formats/libwebp/src/webp/encode.h"
#else
 extern "C"
 {
  #include JUCE_WEBPLIB_DECODE_INCLUDE_PATH
  #include JUCE_WEBPLIB_ENCODE_INCLUDE_PATH
 }
#endif

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

namespace juce
{

void WebPImageFormat::setQuality (float newQuality)     { quality = newQuality; }
void WebPImageFormat::setLossless (bool shouldBeLossless) { lossless = shouldBeLossless; }

String WebPImageFormat::getFormatName()                  { return "WebP"; }
bool WebPImageFormat::usesFileExtension (const File& f)  { return f.hasFileExtension ("webp"); }

bool WebPImageFormat::canUnderstand (InputStream& in)
{
    static constexpr int headerSize = 12;
    char header [headerSize];

    return in.read (header, sizeof (header)) == headerSize
            && memcmp (header, "RIFF", 4) == 0
            && memcmp (header + 8, "WEBP", 4) == 0;
}

Image WebPImageFormat::decodeImage (InputStream& in)
{
    MemoryBlock data;
    in.readIntoMemoryBlock (data);

    WebPDecoderConfig config;

    if (! WebPInitDecoderConfig (&config))
    {
        jassertfalse;
        return {};
    }

    if (WebPGetFeatures ((uint8_t*) data.getData(), data.getSize(), &config.input) != VP8_STATUS_OK)
        return {};

    const auto sourceHasAlpha = (bool) config.input.has_alpha;

    config.output.colorspace = sourceHasAlpha ? MODE_rgbA : MODE_RGB;
    VP8StatusCode status = WebPDecode ((const uint8_t*) data.getData(), data.getSize(), &config);
    ScopeGuard scope { [&] { WebPFreeDecBuffer (&config.output); } };

    if (status != VP8_STATUS_OK)
        return {};

    const auto srcPixelStride = sourceHasAlpha ? 4 : 3;
    const auto srcLineStride = config.output.u.RGBA.stride;

    Image image (sourceHasAlpha ? Image::ARGB : Image::RGB, config.input.width, config.input.height, false);
    image.getProperties()->set ("originalImageHadAlpha", sourceHasAlpha);
    const auto destHasAlpha = image.hasAlphaChannel();

    const Image::BitmapData destData { image, Image::BitmapData::writeOnly };

    for (int y = 0; y < image.getHeight(); ++y)
    {
        auto* src = config.output.u.RGBA.rgba + srcLineStride * y;
        auto* dest = destData.getLinePointer (y);

        for (int x = 0; x < image.getWidth(); ++x)
        {
            const uint8_t sourcePixel[4] = { sourceHasAlpha ? src[3] : (uint8_t) 255, src[0], src[1], src[2] };

            if (destHasAlpha)
                ((PixelARGB*) dest)->setARGB (sourcePixel[0], sourcePixel[1], sourcePixel[2], sourcePixel[3]);
            else
                ((PixelRGB*) dest)->setARGB (sourcePixel[0], sourcePixel[1], sourcePixel[2], sourcePixel[3]);

            src += srcPixelStride;
            dest += destData.pixelStride;
        }
    }

    return image;
}

struct UnpremultipliedRGBA
{
    CopyableHeapBlock<uint8_t> buffer;
    bool hasAlpha{};
};

static UnpremultipliedRGBA getBuffer (const Image& image)
{
    const auto hasAlpha = image.hasAlphaChannel();

    UnpremultipliedRGBA buffer
    {
        CopyableHeapBlock<uint8_t> ((size_t) (image.getWidth() * image.getHeight() * (hasAlpha ? 4 : 3))),
        hasAlpha
    };

    const Image::BitmapData srcData (image, Image::BitmapData::readOnly);
    auto* dest = buffer.buffer.data();

    if (image.isARGB())
    {
        for (int y = 0; y < image.getHeight(); ++y)
        {
            auto* src = srcData.getLinePointer (y);

            for (int i = image.getWidth(); --i >= 0;)
            {
                PixelARGB p (*(const PixelARGB*) src);
                p.unpremultiply();

                *dest++ = p.getRed();
                *dest++ = p.getGreen();
                *dest++ = p.getBlue();
                *dest++ = p.getAlpha();
                src += srcData.pixelStride;
            }
        }
    }
    else if (image.isRGB())
    {
        for (int y = 0; y < image.getHeight(); ++y)
        {
            auto* src = srcData.getLinePointer (y);

            for (int i = image.getWidth(); --i >= 0;)
            {
                const auto* p = (const PixelRGB*) src;
                *dest++ = p->getRed();
                *dest++ = p->getGreen();
                *dest++ = p->getBlue();
                src += srcData.pixelStride;
            }
        }
    }
    else if (image.isSingleChannel())
    {
        for (int y = 0; y < image.getHeight(); ++y)
        {
            auto* src = srcData.getLinePointer (y);

            for (int i = image.getWidth(); --i >= 0;)
            {
                *dest++ = 0;
                *dest++ = 0;
                *dest++ = 0;
                *dest++ = ((const PixelAlpha*) src)->getAlpha();;
                src += srcData.pixelStride;
            }
        }
    }

    return buffer;
}

bool WebPImageFormat::writeImageToStream (const Image& image, OutputStream& out)
{
    if (! image.isValid())
        return false;

    const auto width = image.getWidth();
    const auto height = image.getHeight();

    if (width > WEBP_MAX_DIMENSION || height > WEBP_MAX_DIMENSION)
    {
        jassertfalse;
        return false;
    }

    const auto buffer = getBuffer (image);

    const int stride = width * (buffer.hasAlpha ? 4 : 3);
    uint8_t* encoded = nullptr;

    const auto encodedSize = std::invoke ([&]
    {
        if (lossless)
        {
            if (buffer.hasAlpha)
                return WebPEncodeLosslessRGBA (buffer.buffer.data(), width, height, stride, &encoded);

            return WebPEncodeLosslessRGB (buffer.buffer.data(), width, height, stride, &encoded);
        }

        const auto q = std::clamp (quality * 100.0f, 0.0f, 100.0f);

        if (buffer.hasAlpha)
            return WebPEncodeRGBA (buffer.buffer.data(), width, height, stride, q, &encoded);

        return WebPEncodeRGB (buffer.buffer.data(), width, height, stride, q, &encoded);
    });

    ScopeGuard scope { [&] { WebPFree (encoded); } };

    if (encoded == nullptr || encodedSize == 0)
        return false;

    return out.write (encoded, encodedSize);
}

//==============================================================================
#if JUCE_UNIT_TESTS

class WebPImageFormatTests final : public UnitTest
{
public:
    WebPImageFormatTests()
        : UnitTest ("WebPImageFormat", UnitTestCategories::graphics)
    {
    }

    /*  Returns a copy of the image, with the loss that results from converting each pixel
        to straight alpha and back to premultiplied alpha at 8-bit precision.
    */
    static Image withPremultiplicationLoss (const Image& image)
    {
        auto result = image.createCopy();

        if (! result.hasAlphaChannel())
            return result;

        const Image::BitmapData data (result, Image::BitmapData::readWrite);

        for (int y = 0; y < result.getHeight(); ++y)
        {
            auto* dest = data.getLinePointer (y);

            for (int i = result.getWidth(); --i >= 0;)
            {
                auto* pixel = (PixelARGB*) dest;
                pixel->unpremultiply();
                pixel->premultiply();

                dest += data.pixelStride;
            }
        }

        return result;
    }

    void runTest() override
    {
        beginTest ("Lossless round-trip preserves every pixel");
        {
            const auto source = createTestImage (true);

            WebPImageFormat format;
            format.setLossless (true);

            MemoryOutputStream out;
            expect (format.writeImageToStream (source, out));

            const auto decoded = decode (out);
            expect (decoded.isValid());
            expectEquals (decoded.getWidth(), source.getWidth());
            expectEquals (decoded.getHeight(), source.getHeight());
            expect (decoded.hasAlphaChannel());

            // Fully opaque should match exactly
            expect (imagesMatch (opaqueRegion (source), opaqueRegion (decoded), 0));

            /*  Partially transparent pixels cannot round-trip exactly, and this is
                a property of Image::ARGB rather than of the codec.

                Image::ARGB stores premultiplied colour, whereas WebP stores
                straight alpha, so writing has to unpremultiply and reading has to
                premultiply again. Neither step is reversible at 8-bit precision,
                so up to 2 levels are lost for partially transparent pixels.

                To show that the codec itself adds nothing, we compare the decoded
                image to one where we apply the premultiplication loss.

                A tolerance of 1 is needed because decoding lets libwebp perform
                the final premultiplication, and its rounding may differ from
                PixelARGB::premultiply() by one level. It also differs between
                libwebp's own C and SIMD code paths.
            */
            const auto expected = withPremultiplicationLoss (source);
            expect (imagesMatch (expected, decoded, 1));
        }

        beginTest ("Lossless round-trip works without an alpha channel");
        {
            const auto source = createTestImage (false);

            WebPImageFormat format;
            format.setLossless (true);

            MemoryOutputStream out;
            expect (format.writeImageToStream (source, out));

            const auto decoded = decode (out);
            expect (decoded.isValid());
            expect (imagesMatch (source, decoded, 0));
        }

        beginTest ("Lossy encoding stays close to the original");
        {
            const auto source = createTestImage (false);

            WebPImageFormat format;
            format.setQuality (1.0f);

            MemoryOutputStream out;
            expect (format.writeImageToStream (source, out));

            const auto decoded = decode (out);
            expect (decoded.isValid());
            expect (imagesMatch (source, decoded, 6));
        }

        beginTest ("canUnderstand only accepts a RIFF/WEBP header");
        {
            WebPImageFormat format;

            MemoryOutputStream valid;
            expect (format.writeImageToStream (createTestImage (false), valid));

            expect (canUnderstand (format, valid.getMemoryBlock()));

            // A RIFF container that is not WebP must be rejected.
            const char wav[] = "RIFF\x00\x00\x00\x00WAVEfmt ";
            expect (! canUnderstand (format, MemoryBlock (wav, sizeof (wav) - 1)));

            const char truncated[] = "RIFF";
            expect (! canUnderstand (format, MemoryBlock (truncated, sizeof (truncated) - 1)));
        }

        beginTest ("The format is discoverable through ImageFileFormat");
        {
            WebPImageFormat format;
            format.setLossless (true);

            MemoryOutputStream out;
            expect (format.writeImageToStream (createTestImage (true), out));

            const auto& block = out.getMemoryBlock();
            MemoryInputStream in (block.getData(), block.getSize(), false);

            auto* found = ImageFileFormat::findImageFormatForStream (in);
            expect (found != nullptr);

            if (found != nullptr)
                expectEquals (found->getFormatName(), String ("WebP"));

            expect (ImageFileFormat::loadFrom (block.getData(), block.getSize()).isValid());
        }

        beginTest ("Garbage is rejected rather than crashing");
        {
            WebPImageFormat format;

            // A valid header followed by nonsense.
            MemoryBlock bogus ("RIFF\x20\x00\x00\x00WEBPVP8 \x01\x02\x03\x04\x05\x06\x07\x08", 24);
            MemoryInputStream in (bogus.getData(), bogus.getSize(), false);
            expect (! format.decodeImage (in).isValid());

            MemoryInputStream empty (nullptr, 0, false);
            expect (! format.decodeImage (empty).isValid());
        }

        beginTest ("An invalid image is not written");
        {
            WebPImageFormat format;
            MemoryOutputStream out;
            expect (! format.writeImageToStream (Image(), out));
        }
    }

private:
    static constexpr int opaqueXBound = 41;

    static Image createTestImage (bool withAlpha)
    {
        Image image (withAlpha ? Image::ARGB : Image::RGB, 61, 37, true);
        const Image::BitmapData data (image, Image::BitmapData::writeOnly);

        for (int y = 0; y < image.getHeight(); ++y)
        {
            for (int x = 0; x < image.getWidth(); ++x)
            {
                // Smooth gradients, so that the lossy comparison is meaningful.
                const auto r = (uint8) (x * 4);
                const auto g = (uint8) (y * 6);
                const auto b = (uint8) ((x + y) * 2);
                const auto a = (uint8) (withAlpha ? (x >= opaqueXBound ? (x % 128) : 255) : 255);

                data.setPixelColour (x, y, Colour::fromRGBA (r, g, b, a));
            }
        }

        return image;
    }

    static Image decode (const MemoryOutputStream& out)
    {
        const auto& block = out.getMemoryBlock();
        MemoryInputStream in (block.getData(), block.getSize(), false);

        WebPImageFormat format;
        return format.decodeImage (in);
    }

    static bool canUnderstand (WebPImageFormat& format, const MemoryBlock& block)
    {
        MemoryInputStream in (block.getData(), block.getSize(), false);
        return format.canUnderstand (in);
    }

    /** The left-hand part of the test image, where alpha is fully opaque. */
    static Image opaqueRegion (const Image& image)
    {
        return image.getClippedImage ({ 0, 0, opaqueXBound, image.getHeight() });
    }

    /*  Compares the raw pixel data, i.e. premultiplied values for ARGB images. This avoids
        the amplification of small premultiplied differences that unpremultiplying during the
        comparison would cause at low alpha values.
    */
    static bool imagesMatch (const Image& a, const Image& b, int tolerance)
    {
        if (a.getWidth() != b.getWidth() || a.getHeight() != b.getHeight())
            return false;

        const Image::BitmapData dataA (a, Image::BitmapData::readOnly);
        const Image::BitmapData dataB (b, Image::BitmapData::readOnly);

        const auto getPixel = [] (const Image::BitmapData& data, int x, int y)
        {
            const auto* p = data.getPixelPointer (x, y);

            if (data.pixelFormat == Image::ARGB)
                return *(const PixelARGB*) p;

            PixelARGB result;
            result.set (*(const PixelRGB*) p);
            return result;
        };

        for (int y = 0; y < a.getHeight(); ++y)
        {
            for (int x = 0; x < a.getWidth(); ++x)
            {
                const auto pa = getPixel (dataA, x, y);
                const auto pb = getPixel (dataB, x, y);

                const auto dr = std::abs ((int) pa.getRed()   - (int) pb.getRed());
                const auto dg = std::abs ((int) pa.getGreen() - (int) pb.getGreen());
                const auto db = std::abs ((int) pa.getBlue()  - (int) pb.getBlue());
                const auto da = std::abs ((int) pa.getAlpha() - (int) pb.getAlpha());

                if (jmax (dr, dg, db, da) > tolerance)
                    return false;
            }
        }

        return true;
    }
};

static WebPImageFormatTests webpImageFormatTests;

#endif

} // namespace juce

#endif
