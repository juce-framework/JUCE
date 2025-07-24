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

namespace juce::build_tools
{

    Icons Icons::fromFilesSmallAndBig (const File& small, const File& big)
    {
        Icons result;
        result.small = Drawable::createFromImageFile (small);
        result.big   = Drawable::createFromImageFile (big);
        return result;
    }

    Array<const Drawable*> asArray (const Icons& icons)
    {
        Array<const Drawable*> result;

        for (auto getter : { &Icons::getSmall, &Icons::getBig })
            if (auto* got = (icons.*getter)())
                result.add (got);

        return result;
    }

    namespace mac
    {
        static Image fixIconImageSize (const Drawable& image)
        {
            const int validSizes[] = { 16, 32, 64, 128, 256, 512, 1024 };

            auto w = image.getWidth();
            auto h = image.getHeight();

            int bestSize = 16;

            for (int size : validSizes)
            {
                if (w == h && w == size)
                {
                    bestSize = w;
                    break;
                }

                if (jmax (w, h) > size)
                    bestSize = size;
            }

            return rescaleImageForIcon (image, bestSize);
        }

        static void writeIconData (MemoryOutputStream& out, const Image& image, const char* type)
        {
            MemoryOutputStream pngData;
            PNGImageFormat pngFormat;
            pngFormat.writeImageToStream (image, pngData);

            out.write (type, 4);
            out.writeIntBigEndian (8 + (int) pngData.getDataSize());
            out << pngData;
        }
    } // namespace mac

    static void writeMacIcon (const Icons& icons, OutputStream& out)
    {
        MemoryOutputStream data;
        auto smallest = std::numeric_limits<int>::max();
        const Drawable* smallestImage = nullptr;

        const auto images = asArray (icons);

        for (int i = 0; i < images.size(); ++i)
        {
            auto image = mac::fixIconImageSize (*images[i]);
            jassert (image.getWidth() == image.getHeight());

            if (image.getWidth() < smallest)
            {
                smallest = image.getWidth();
                smallestImage = images[i];
            }

            switch (image.getWidth())
            {
                case 16:   mac::writeIconData (data, image, "icp4"); break;
                case 32:   mac::writeIconData (data, image, "icp5"); break;
                case 64:   mac::writeIconData (data, image, "icp6"); break;
                case 128:  mac::writeIconData (data, image, "ic07"); break;
                case 256:  mac::writeIconData (data, image, "ic08"); break;
                case 512:  mac::writeIconData (data, image, "ic09"); break;
                case 1024: mac::writeIconData (data, image, "ic10"); break;
                default:   break;
            }
        }

        jassert (data.getDataSize() > 0); // no suitable sized images?

        // If you only supply a 1024 image, the file doesn't work on 10.8, so we need
        // to force a smaller one in there too..
        if (smallest > 512 && smallestImage != nullptr)
            mac::writeIconData (data, rescaleImageForIcon (*smallestImage, 512), "ic09");

        out.write ("icns", 4);
        out.writeIntBigEndian ((int) data.getDataSize() + 8);
        out << data;
    }

    Image getBestIconForSize (const Icons& icons,
                              int size,
                              bool returnNullIfNothingBigEnough)
    {
        auto* const im = std::invoke ([&]() -> const Drawable*
        {
            if ((icons.getSmall() != nullptr) != (icons.getBig() != nullptr))
                return icons.getSmall() != nullptr ? icons.getSmall() : icons.getBig();

            if (icons.getSmall() != nullptr && icons.getBig() != nullptr)
            {
                if (icons.getSmall()->getWidth() >= size && icons.getBig()->getWidth() >= size)
                    return icons.getSmall()->getWidth() < icons.getBig()->getWidth() ? icons.getSmall() : icons.getBig();

                if (icons.getSmall()->getWidth() >= size)
                    return icons.getSmall();

                if (icons.getBig()->getWidth() >= size)
                    return icons.getBig();
            }

            return nullptr;
        });

        if (im == nullptr)
            return {};

        if (returnNullIfNothingBigEnough && im->getWidth() < size && im->getHeight() < size)
            return {};

        return rescaleImageForIcon (*im, size);
    }

    namespace win
    {
        static void writeBMPImage (const Image& image, const int w, const int h, MemoryOutputStream& out)
        {
            int maskStride = (w / 8 + 3) & ~3;

            out.writeInt (40); // bitmapinfoheader size
            out.writeInt (w);
            out.writeInt (h * 2);
            out.writeShort (1); // planes
            out.writeShort (32); // bits
            out.writeInt (0); // compression
            out.writeInt ((h * w * 4) + (h * maskStride)); // size image
            out.writeInt (0); // x pixels per meter
            out.writeInt (0); // y pixels per meter
            out.writeInt (0); // clr used
            out.writeInt (0); // clr important

            Image::BitmapData bitmap (image, Image::BitmapData::readOnly);
            int alphaThreshold = 5;

            int y;
            for (y = h; --y >= 0;)
            {
                for (int x = 0; x < w; ++x)
                {
                    auto pixel = bitmap.getPixelColour (x, y);

                    if (pixel.getAlpha() <= alphaThreshold)
                    {
                        out.writeInt (0);
                    }
                    else
                    {
                        out.writeByte ((char) pixel.getBlue());
                        out.writeByte ((char) pixel.getGreen());
                        out.writeByte ((char) pixel.getRed());
                        out.writeByte ((char) pixel.getAlpha());
                    }
                }
            }

            for (y = h; --y >= 0;)
            {
                int mask = 0, count = 0;

                for (int x = 0; x < w; ++x)
                {
                    auto pixel = bitmap.getPixelColour (x, y);

                    mask <<= 1;
                    if (pixel.getAlpha() <= alphaThreshold)
                        mask |= 1;

                    if (++count == 8)
                    {
                        out.writeByte ((char) mask);
                        count = 0;
                        mask = 0;
                    }
                }

                if (mask != 0)
                    out.writeByte ((char) mask);

                for (int i = maskStride - w / 8; --i >= 0;)
                    out.writeByte (0);
            }
        }

        static void writeIcon (const Array<Image>& images, OutputStream& out)
        {
            out.writeShort (0); // reserved
            out.writeShort (1); // .ico tag
            out.writeShort ((short) images.size());

            MemoryOutputStream dataBlock;

            int imageDirEntrySize = 16;
            int dataBlockStart = 6 + images.size() * imageDirEntrySize;

            for (int i = 0; i < images.size(); ++i)
            {
                auto oldDataSize = dataBlock.getDataSize();

                auto& image = images.getReference (i);
                auto w = image.getWidth();
                auto h = image.getHeight();

                if (w >= 256 || h >= 256)
                {
                    PNGImageFormat pngFormat;
                    pngFormat.writeImageToStream (image, dataBlock);
                }
                else
                {
                    writeBMPImage (image, w, h, dataBlock);
                }

                out.writeByte ((char) w);
                out.writeByte ((char) h);
                out.writeByte (0);
                out.writeByte (0);
                out.writeShort (1); // colour planes
                out.writeShort (32); // bits per pixel
                out.writeInt ((int) (dataBlock.getDataSize() - oldDataSize));
                out.writeInt (dataBlockStart + (int) oldDataSize);
            }

            jassert (out.getPosition() == dataBlockStart);
            out << dataBlock;
        }
    } // namespace win

    static void writeWinIcon (const Icons& icons, OutputStream& os)
    {
        Array<Image> images;
        int sizes[] = { 16, 32, 48, 256 };

        for (int size : sizes)
        {
            auto im = getBestIconForSize (icons, size, true);

            if (im.isValid())
                images.add (im);
        }

        if (images.size() > 0)
            win::writeIcon (images, os);
    }

    void writeMacIcon (const Icons& icons, const File& file)
    {
        writeStreamToFile (file, [&] (MemoryOutputStream& mo) { writeMacIcon (icons, mo); });
    }

    void writeWinIcon (const Icons& icons, const File& file)
    {
        writeStreamToFile (file, [&] (MemoryOutputStream& mo) { writeWinIcon (icons, mo); });
    }

    Image rescaleImageForIcon (const Drawable& d, const int size)
    {
        if (auto* drawableImage = dynamic_cast<const DrawableImage*> (&d))
        {
            auto im = SoftwareImageType().convert (drawableImage->getImage());

            if (im.getWidth() == size && im.getHeight() == size)
                return im;

            // (scale it down in stages for better resampling)
            while (im.getWidth() > 2 * size && im.getHeight() > 2 * size)
                im = im.rescaled (im.getWidth() / 2,
                                  im.getHeight() / 2);

            Image newIm (Image::ARGB, size, size, true, SoftwareImageType());
            Graphics g (newIm);
            g.drawImageWithin (im, 0, 0, size, size,
                               RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, false);
            return newIm;
        }

        Image im (Image::ARGB, size, size, true, SoftwareImageType());
        Graphics g (im);
        d.drawWithin (g, im.getBounds().toFloat(), RectanglePlacement::centred, 1.0f);
        return im;
    }

    struct AppIconType
    {
        const char* idiom;
        const char* sizeString;
        const char* filename;
        const char* scale;
        int size;
    };

    static const AppIconType iOSAppIconTypes[]
    {
        { "iphone",          "20x20",     "Icon-Notification-20@2x.png",       "2x", 40   },
        { "iphone",          "20x20",     "Icon-Notification-20@3x.png",       "3x", 60   },
        { "iphone",          "29x29",     "Icon-29.png",                       "1x", 29   },
        { "iphone",          "29x29",     "Icon-29@2x.png",                    "2x", 58   },
        { "iphone",          "29x29",     "Icon-29@3x.png",                    "3x", 87   },
        { "iphone",          "40x40",     "Icon-Spotlight-40@2x.png",          "2x", 80   },
        { "iphone",          "40x40",     "Icon-Spotlight-40@3x.png",          "3x", 120  },
        { "iphone",          "60x60",     "Icon-60@2x.png",                    "2x", 120  },
        { "iphone",          "60x60",     "Icon-@3x.png",                      "3x", 180  },
        { "ipad",            "20x20",     "Icon-Notifications-20.png",         "1x", 20   },
        { "ipad",            "20x20",     "Icon-Notifications-20@2x.png",      "2x", 40   },
        { "ipad",            "29x29",     "Icon-Small-1.png",                  "1x", 29   },
        { "ipad",            "29x29",     "Icon-Small@2x-1.png",               "2x", 58   },
        { "ipad",            "40x40",     "Icon-Spotlight-40.png",             "1x", 40   },
        { "ipad",            "40x40",     "Icon-Spotlight-40@2x-1.png",        "2x", 80   },
        { "ipad",            "76x76",     "Icon-76.png",                       "1x", 76   },
        { "ipad",            "76x76",     "Icon-76@2x.png",                    "2x", 152  },
        { "ipad",            "83.5x83.5", "Icon-83.5@2x.png",                  "2x", 167  },
        { "ios-marketing",   "1024x1024", "Icon-AppStore-1024.png",            "1x", 1024 }
    };

    static void createiOSIconFiles (const Icons& icons, File appIconSet)
    {
        auto* imageToUse = icons.getBig() != nullptr ? icons.getBig()
                                                     : icons.getSmall();

        if (imageToUse != nullptr)
        {
            for (auto& type : iOSAppIconTypes)
            {
                auto image = rescaleImageForIcon (*imageToUse, type.size);

                if (image.hasAlphaChannel())
                {
                    Image background (Image::RGB, image.getWidth(), image.getHeight(), false);
                    Graphics g (background);
                    g.fillAll (Colours::white);

                    g.drawImageWithin (image, 0, 0, image.getWidth(), image.getHeight(),
                                       RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize);

                    image = background;
                }

                MemoryOutputStream pngData;
                PNGImageFormat pngFormat;
                pngFormat.writeImageToStream (image, pngData);

                overwriteFileIfDifferentOrThrow (appIconSet.getChildFile (type.filename), pngData);
            }
        }
    }

    static String getiOSAssetContents (var images)
    {
        DynamicObject::Ptr v (new DynamicObject());

        var info (new DynamicObject());
        info.getDynamicObject()->setProperty ("version", 1);
        info.getDynamicObject()->setProperty ("author", "xcode");

        v->setProperty ("images", images);
        v->setProperty ("info", info);

        return JSON::toString (var (v.get()));
    }

    //==============================================================================
    static String getiOSAppIconContents()
    {
        var images;

        for (auto& type : iOSAppIconTypes)
        {
            DynamicObject::Ptr d (new DynamicObject());
            d->setProperty ("idiom",    type.idiom);
            d->setProperty ("size",     type.sizeString);
            d->setProperty ("filename", type.filename);
            d->setProperty ("scale",    type.scale);
            images.append (var (d.get()));
        }

        return getiOSAssetContents (images);
    }

    struct ImageType
    {
        const char* orientation;
        const char* idiom;
        const char* subtype;
        const char* extent;
        const char* scale;
        const char* filename;
        int width;
        int height;
    };

    static const ImageType iOSLaunchImageTypes[]
    {
        { "portrait", "iphone", nullptr,      "full-screen", "2x", "LaunchImage-iphone-2x.png",         640, 960 },
        { "portrait", "iphone", "retina4",    "full-screen", "2x", "LaunchImage-iphone-retina4.png",    640, 1136 },
        { "portrait", "ipad",   nullptr,      "full-screen", "1x", "LaunchImage-ipad-portrait-1x.png",  768, 1024 },
        { "landscape","ipad",   nullptr,      "full-screen", "1x", "LaunchImage-ipad-landscape-1x.png", 1024, 768 },
        { "portrait", "ipad",   nullptr,      "full-screen", "2x", "LaunchImage-ipad-portrait-2x.png",  1536, 2048 },
        { "landscape","ipad",   nullptr,      "full-screen", "2x", "LaunchImage-ipad-landscape-2x.png", 2048, 1536 }
    };

    static void createiOSLaunchImageFiles (const File& launchImageSet)
    {
        for (auto& type : iOSLaunchImageTypes)
        {
            Image image (Image::ARGB, type.width, type.height, true); // (empty black image)
            image.clear (image.getBounds(), Colours::black);

            MemoryOutputStream pngData;
            PNGImageFormat pngFormat;
            pngFormat.writeImageToStream (image, pngData);
            build_tools::overwriteFileIfDifferentOrThrow (launchImageSet.getChildFile (type.filename), pngData);
        }
    }

    static String getiOSLaunchImageContents()
    {
        var images;

        for (auto& type : iOSLaunchImageTypes)
        {
            DynamicObject::Ptr d (new DynamicObject());
            d->setProperty ("orientation", type.orientation);
            d->setProperty ("idiom", type.idiom);
            d->setProperty ("extent",  type.extent);
            d->setProperty ("minimum-system-version", "7.0");
            d->setProperty ("scale", type.scale);
            d->setProperty ("filename", type.filename);

            if (type.subtype != nullptr)
                d->setProperty ("subtype", type.subtype);

            images.append (var (d.get()));
        }

        return getiOSAssetContents (images);
    }

    RelativePath createXcassetsFolderFromIcons (const Icons& icons,
                                                const File& targetFolder,
                                                String projectFilenameRootString)
    {
        const auto assets      = targetFolder.getChildFile (projectFilenameRootString)
                                             .getChildFile ("Images.xcassets");
        const auto iconSet     = assets.getChildFile ("AppIcon.appiconset");
        const auto launchImage = assets.getChildFile ("LaunchImage.launchimage");

        overwriteFileIfDifferentOrThrow (iconSet.getChildFile ("Contents.json"), getiOSAppIconContents());
        createiOSIconFiles (icons, iconSet);

        overwriteFileIfDifferentOrThrow (launchImage.getChildFile ("Contents.json"), getiOSLaunchImageContents());
        createiOSLaunchImageFiles (launchImage);

        return { assets, targetFolder, RelativePath::buildTargetFolder };
    }

    //==============================================================================
    //==============================================================================
   #if JUCE_UNIT_TESTS

    class IconsUnitTests : public UnitTest
    {
    public:
        IconsUnitTests() : UnitTest ("Generate icon files", UnitTestCategories::graphics) {}

        void runTest() override
        {
            const ScopedJuceInitialiser_GUI scope;

            beginTest ("Load icons from vector file");
            {
                TemporaryFile tempFile ("vector");

                {
                    auto stream = tempFile.getFile().createOutputStream();
                    expect (stream != nullptr);
                    stream->write (svg, std::size (svg));
                }

                const auto icons = Icons::fromFilesSmallAndBig (tempFile.getFile(), {});

                expect (icons.getSmall() != nullptr);
                expect (icons.getBig() == nullptr);

                expect (dynamic_cast<const DrawableImage*> (icons.getSmall()) == nullptr,
                        "Vector data should not be rasterised on load");
            }

            beginTest ("Load icons from raster file");
            {
                TemporaryFile tempFile ("raster");

                {
                    auto stream = tempFile.getFile().createOutputStream();
                    expect (stream != nullptr);
                    stream->write (png, std::size (png));
                }

                const auto icons = Icons::fromFilesSmallAndBig ({}, tempFile.getFile());

                expect (icons.getSmall() == nullptr);
                expect (icons.getBig() != nullptr);

                expect (dynamic_cast<const DrawableImage*> (icons.getBig()) != nullptr,
                        "Raster data is loaded as a DrawableImage");
            }
        }

    private:
        static constexpr uint8_t svg[] = R"svg(
        <svg xmlns="http://www.w3.org/2000/svg" width="284.4" height="284.4" viewBox="0 0 284.4 284.4">
            <g>
                <ellipse cx="142.2" cy="142.2" rx="132.82" ry="132.74" fill="#fff"/>
            </g>
        </svg>)svg";

        static constexpr uint8_t png[]
        {
            0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
            0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x07,
            0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0x52, 0x57, 0xd3, 0x00, 0x00, 0x00,
            0x5e, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0x55, 0x8d, 0x49, 0x0e, 0x00,
            0x21, 0x08, 0x04, 0xfd, 0x89, 0xe2, 0x12, 0x13, 0xf5, 0xea, 0xff, 0x7f,
            0x46, 0x4b, 0x9b, 0xe8, 0x38, 0x87, 0x0a, 0x84, 0x5e, 0x70, 0x21, 0x04,
            0x25, 0xde, 0x7b, 0xcd, 0x39, 0x6b, 0x4a, 0x69, 0xef, 0xc4, 0x89, 0x08,
            0x48, 0xef, 0x1d, 0x63, 0x0c, 0xcc, 0x39, 0xd1, 0x5a, 0xe3, 0xed, 0x13,
            0x2d, 0x71, 0xa1, 0xd1, 0x0c, 0xea, 0xac, 0x12, 0x31, 0x46, 0x58, 0xe5,
            0x86, 0x22, 0x67, 0xad, 0xf5, 0x9f, 0x3c, 0x86, 0x52, 0x0a, 0xee, 0x4f,
            0xa6, 0xdf, 0x6a, 0xee, 0x5b, 0x64, 0xe5, 0x49, 0xbf, 0x50, 0x5c, 0x2f,
            0xb3, 0x44, 0xdf, 0x94, 0x9e, 0x62, 0xe2, 0x00, 0x00, 0x00, 0x00, 0x49,
            0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
        };
    };

    static IconsUnitTests iconsUnitTests;

   #endif

} // namespace juce::build_tools
