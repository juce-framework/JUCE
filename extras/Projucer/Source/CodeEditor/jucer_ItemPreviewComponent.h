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

#pragma once


//==============================================================================
class ItemPreviewComponent final : public Component
{
public:
    ItemPreviewComponent (const File& f)  : file (f)
    {
        setOpaque (true);
        tryToLoadImage();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (backgroundColourId));

        if (drawable != nullptr)
        {
            auto contentBounds = drawable->getDrawableBounds();

            if (auto* dc = dynamic_cast<DrawableComposite*> (drawable.get()))
            {
                auto r = dc->getContentArea();

                if (! r.isEmpty())
                    contentBounds = r;
            }

            auto area = RectanglePlacement (RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize)
                            .appliedTo (contentBounds, Rectangle<float> (4.0f, 22.0f, (float) getWidth() - 8.0f, (float) getHeight() - 26.0f));

            Path p;
            p.addRectangle (area);
            DropShadow (Colours::black.withAlpha (0.5f), 6, Point<int> (0, 1)).drawForPath (g, p);

            g.fillCheckerBoard (area, 24.0f, 24.0f, Colour (0xffffffff), Colour (0xffeeeeee));

            drawable->draw (g, 1.0f, RectanglePlacement (RectanglePlacement::stretchToFit)
                                        .getTransformToFit (contentBounds, area.toFloat()));
        }

        g.setFont (FontOptions (14.0f, Font::bold));
        g.setColour (findColour (defaultTextColourId));
        g.drawMultiLineText (facts.joinIntoString ("\n"), 10, 15, getWidth() - 16);
    }

private:
    StringArray facts;
    File file;
    std::unique_ptr<Drawable> drawable;

    void tryToLoadImage()
    {
        facts.clear();
        facts.add (file.getFullPathName());
        drawable.reset();

        if (auto input = std::unique_ptr<FileInputStream> (file.createInputStream()))
        {
            auto totalSize = input->getTotalLength();
            String formatName;

            if (auto* format = ImageFileFormat::findImageFormatForStream (*input))
                formatName = " " + format->getFormatName();

            input.reset();

            auto image = ImageCache::getFromFile (file);

            if (image.isValid())
            {
                auto* d = new DrawableImage();
                d->setImage (image);
                drawable.reset (d);

                facts.add (String (image.getWidth()) + " x " + String (image.getHeight()) + formatName);
            }

            if (totalSize > 0)
                facts.add (File::descriptionOfSizeInBytes (totalSize));
        }

        if (drawable == nullptr)
            if (auto svg = parseXML (file))
                drawable = Drawable::createFromSVG (*svg);

        facts.removeEmptyStrings (true);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemPreviewComponent)
};
