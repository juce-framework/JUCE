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

namespace juce::detail
{

struct StandardCachedComponentImage : public CachedComponentImage
{
    explicit StandardCachedComponentImage (Component& c) noexcept
        : owner (c)
    {
    }

    void paint (Graphics& g) override
    {
        scale = g.getInternalContext().getPhysicalPixelScaleFactor();
        auto compBounds = owner.getLocalBounds();
        auto imageBounds = compBounds * scale;

        if (image.isNull() || image.getBounds() != imageBounds)
        {
            image = Image (owner.isOpaque() ? Image::RGB
                                            : Image::ARGB,
                           jmax (1, imageBounds.getWidth()),
                           jmax (1, imageBounds.getHeight()),
                           ! owner.isOpaque());

            validArea.clear();
        }

        if (! validArea.containsRectangle (compBounds))
        {
            Graphics imG (image);
            auto& lg = imG.getInternalContext();

            lg.addTransform (AffineTransform::scale (scale));

            for (auto& i : validArea)
                lg.excludeClipRectangle (i);

            if (! owner.isOpaque())
            {
                lg.setFill (Colours::transparentBlack);
                lg.fillRect (compBounds, true);
                lg.setFill (Colours::black);
            }

            owner.paintEntireComponent (imG, true);
        }

        validArea = compBounds;

        g.setColour (Colours::black.withAlpha (owner.getAlpha()));
        g.drawImageTransformed (image, AffineTransform::scale ((float) compBounds.getWidth()  / (float) imageBounds.getWidth(),
                                                               (float) compBounds.getHeight() / (float) imageBounds.getHeight()), false);
    }

    bool invalidateAll() override                            { validArea.clear(); return true; }
    bool invalidate (const Rectangle<int>& area) override    { validArea.subtract (area); return true; }
    void releaseResources() override                         { image = Image(); }

private:
    Image image;
    RectangleList<int> validArea;
    Component& owner;
    float scale = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandardCachedComponentImage)
};

} // namespace juce::detail
