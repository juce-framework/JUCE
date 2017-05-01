/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../Utility/jucer_ProjucerLookAndFeel.h"


//==============================================================================
/**
*/
class ItemPreviewComponent  : public Component
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
            Rectangle<float> contentBounds (drawable->getDrawableBounds());

            if (DrawableComposite* dc = dynamic_cast<DrawableComposite*> (drawable.get()))
            {
                Rectangle<float> r (dc->getContentArea().resolve (nullptr));

                if (! r.isEmpty())
                    contentBounds = r;
            }

            Rectangle<float> area = RectanglePlacement (RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize)
                                        .appliedTo (contentBounds, Rectangle<float> (4.0f, 22.0f, getWidth() - 8.0f, getHeight() - 26.0f));

            Path p;
            p.addRectangle (area);
            DropShadow (Colours::black.withAlpha (0.5f), 6, Point<int> (0, 1)).drawForPath (g, p);

            g.fillCheckerBoard (area.getSmallestIntegerContainer(), 24, 24,
                                Colour (0xffffffff), Colour (0xffeeeeee));

            drawable->draw (g, 1.0f, RectanglePlacement (RectanglePlacement::stretchToFit)
                                        .getTransformToFit (contentBounds, area.toFloat()));
        }

        g.setFont (Font (14.0f, Font::bold));
        g.setColour (findColour (defaultTextColourId));
        g.drawMultiLineText (facts.joinIntoString ("\n"), 10, 15, getWidth() - 16);
    }

private:
    StringArray facts;
    File file;
    ScopedPointer<Drawable> drawable;

    void tryToLoadImage()
    {
        facts.clear();
        facts.add (file.getFullPathName());
        drawable = nullptr;

        {
            ScopedPointer<InputStream> input (file.createInputStream());

            if (input != nullptr)
            {
                const int64 totalSize = input->getTotalLength();

                String formatName;
                if (ImageFileFormat* format = ImageFileFormat::findImageFormatForStream (*input))
                    formatName = " " + format->getFormatName();

                input = nullptr;

                Image image (ImageCache::getFromFile (file));

                if (image.isValid())
                {
                    DrawableImage* d = new DrawableImage();
                    d->setImage (image);
                    drawable = d;

                    facts.add (String (image.getWidth()) + " x " + String (image.getHeight()) + formatName);
                }

                if (totalSize > 0)
                    facts.add (File::descriptionOfSizeInBytes (totalSize));
            }
        }

        if (drawable == nullptr)
        {
            ScopedPointer<XmlElement> svg (XmlDocument::parse (file));

            if (svg != nullptr)
                drawable = Drawable::createFromSVG (*svg);
        }

        facts.removeEmptyStrings (true);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemPreviewComponent)
};
