/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCER_FILEPREVIEWCOMPONENT_JUCEHEADER__
#define __JUCER_FILEPREVIEWCOMPONENT_JUCEHEADER__


//==============================================================================
/**
*/
class ItemPreviewComponent  : public Component
{
public:
    ItemPreviewComponent (const File& file_)
        : file (file_)
    {
        setOpaque (true);
        tryToLoadImage();
    }

    void paint (Graphics& g)
    {
        dynamic_cast<IntrojucerLookAndFeel&> (getLookAndFeel()).fillWithBackgroundTexture (g);

        Rectangle<int> area = RectanglePlacement (RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize)
                                .appliedTo (image.getBounds(), Rectangle<int> (4, 22, getWidth() - 8, getHeight() - 26));

        Path p;
        p.addRectangle (area);
        DropShadow (Colours::black.withAlpha (0.5f), 6, Point<int> (0, 1)).drawForPath (g, p);

        g.fillCheckerBoard (area, 24, 24, Colour (0xffffffff), Colour (0xffeeeeee));

        g.setOpacity (1.0f);
        g.drawImageWithin (image, area.getX(), area.getY(), area.getWidth(), area.getHeight(),
                           RectanglePlacement::stretchToFit, false);

        g.setFont (Font (14.0f, Font::bold));
        g.setColour (findColour (mainBackgroundColourId).contrasting());
        g.drawMultiLineText (facts.joinIntoString ("\n"), 10, 15, getWidth() - 16);
    }

private:
    StringArray facts;
    File file;
    Image image;

    void tryToLoadImage()
    {
        facts.clear();
        facts.add (file.getFullPathName());
        image = Image();

        ScopedPointer <InputStream> input (file.createInputStream());

        if (input != nullptr)
        {
            const int64 totalSize = input->getTotalLength();
            ImageFileFormat* format = ImageFileFormat::findImageFormatForStream (*input);
            input = nullptr;

            String formatName;
            if (format != nullptr)
                formatName = " " + format->getFormatName();

            image = ImageCache::getFromFile (file);

            if (image.isValid())
                facts.add (String (image.getWidth()) + " x " + String (image.getHeight()) + formatName);

            if (totalSize > 0)
                facts.add (File::descriptionOfSizeInBytes (totalSize));
        }

        facts.removeEmptyStrings (true);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemPreviewComponent);
};


#endif   // __JUCER_FILEPREVIEWCOMPONENT_JUCEHEADER__
