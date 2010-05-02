/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "../../jucer_Headers.h"
#include "jucer_ItemPreviewComponent.h"


//==============================================================================
ItemPreviewComponent::ItemPreviewComponent (const File& file_)
    : file (file_)
{
    facts.add (file.getFullPathName());
    tryToLoadImage (file.createInputStream());
    facts.removeEmptyStrings (true);
}

ItemPreviewComponent::ItemPreviewComponent (InputStream* input, const String& name)
{
    facts.add (name);
    tryToLoadImage (input);
    facts.removeEmptyStrings (true);
}

ItemPreviewComponent::~ItemPreviewComponent()
{
}

void ItemPreviewComponent::tryToLoadImage (InputStream* in)
{
    if (in != 0)
    {
        ScopedPointer <InputStream> input (in);

        ImageFileFormat* format = ImageFileFormat::findImageFormatForStream (*input);

        String formatName;
        if (format != 0)
            formatName = " " + format->getFormatName();

        image = ImageFileFormat::loadFrom (*input);

        if (image != 0)
            facts.add (String (image->getWidth()) + " x " + String (image->getHeight()) + formatName);

        const int64 totalSize = input->getTotalLength();

        if (totalSize > 0)
            facts.add (File::descriptionOfSizeInBytes (totalSize));
    }
}

void ItemPreviewComponent::paint (Graphics& g)
{
    if (image != 0)
    {
        g.drawImageWithin (image,
                           2, 22, getWidth() - 4, getHeight() - 24,
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                           false);
    }

    g.setFont (15.0f, Font::bold);
    g.setColour (Colours::white);
    g.drawMultiLineText (facts.joinIntoString ("\n"),
                         10, 15, getWidth() - 16);

}

void ItemPreviewComponent::resized()
{
}
