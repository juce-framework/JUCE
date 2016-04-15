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

#ifndef JUCER_FILEPREVIEWCOMPONENT_H_INCLUDED
#define JUCER_FILEPREVIEWCOMPONENT_H_INCLUDED

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
        ProjucerLookAndFeel::fillWithBackgroundTexture (*this, g);

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
        g.setColour (findColour (mainBackgroundColourId).contrasting());
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


#endif   // JUCER_FILEPREVIEWCOMPONENT_H_INCLUDED
