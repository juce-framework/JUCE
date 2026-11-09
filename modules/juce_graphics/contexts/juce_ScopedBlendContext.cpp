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

namespace juce
{

ScopedBlendContext::ScopedBlendContext (Graphics& g, const Options& optionsIn)
    : parent (g),
      options (optionsIn)
{
    const auto clip = std::invoke ([&]
    {
        const auto parentClip = parent.getClipBounds();

        if (! options.getScopeClip().has_value() || options.getScopeClip()->isEmpty())
            return parentClip;

        return options.getScopeClip()->getIntersection (parentClip);
    });

    auto imageType = std::invoke ([&]() -> std::unique_ptr<ImageType>
    {
        auto t = parent.getInternalContext().getPreferredImageTypeForTemporaryImages();

        if (t->getTypeID() == 3)
            return std::make_unique<SoftwareImageType>();

        return t;
    });

    const auto sourceOnlyBlendMode =    options.getBlendMode() == BlendMode::sourceOver
                                     || options.getBlendMode() == BlendMode::source;

    // If the blending mode only affects the source area, we can restrict the final image blend to
    // the current the clip bounds. If, however, we're doing something more sophisticated, all
    // pixels outside the currently drawn bounds need to be treated as transparent black, and
    // blended with all pixels of the parent image.
    const auto clipToUse = sourceOnlyBlendMode ? clip : parent.getClipBounds();
    clipIsEmpty = clipToUse.isEmpty();

    const auto blendContextScale = parent.getInternalContext().getPhysicalPixelScaleFactor();
    const auto scaledClip = clipToUse * blendContextScale;

    buffer = { Image::ARGB,
               std::max (1, scaledClip.getWidth()),
               std::max (1, scaledClip.getHeight()),
               true,
               *imageType };

    blendContext.emplace (buffer);
    blendContextTransform = AffineTransform::scale (blendContextScale).translated ((float) -scaledClip.getX(),
                                                                                   (float) -scaledClip.getY());
    blendContext->addTransform (blendContextTransform);
}

ScopedBlendContext::~ScopedBlendContext()
{
    blendContext.reset();

    if (clipIsEmpty)
        return;

    if (options.isLuminanceMask())
        convertLuminanceToAlpha (buffer);

    const Graphics::ScopedSaveState s { parent };
    parent.setImageBlendMode (options.getBlendMode());
    parent.setOpacity (options.getOpacity());
    parent.addTransform (blendContextTransform.inverted());
    parent.drawImage (buffer,
                      bufferPosition.getX(),
                      bufferPosition.getY(),
                      buffer.getWidth(),
                      buffer.getHeight(),
                      0,
                      0,
                      buffer.getWidth(),
                      buffer.getHeight());
}

void ScopedBlendContext::convertLuminanceToAlpha (Image& image)
{
    if (! image.isARGB())
    {
        jassertfalse;
        return;
    }

    Image::BitmapData data (image, Image::BitmapData::readWrite);

    for (int y = 0; y < data.height; ++y)
    {
        auto* line = data.getLinePointer (y);

        for (int x = 0; x < data.width; ++x)
        {
            auto* p = reinterpret_cast<PixelARGB*> (line);

            const auto a = (int) p->getAlpha();
            auto r = (int) p->getRed();
            auto g = (int) p->getGreen();
            auto b = (int) p->getBlue();

            if (a > 0)
            {
                r = (r * 255) / a;
                g = (g * 255) / a;
                b = (b * 255) / a;
            }

            auto l = (r * 0.2125 + g * 0.7154 + b * 0.0721) * (a / 255.0);
            *p = PixelARGB { (uint8) l, (uint8) 0, (uint8) 0, (uint8) 0 };

            line += data.pixelStride;
        }
    }
}

} // namespace juce
