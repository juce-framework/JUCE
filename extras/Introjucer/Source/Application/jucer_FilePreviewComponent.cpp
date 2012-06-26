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

#include "../jucer_Headers.h"
#include "jucer_FilePreviewComponent.h"


//==============================================================================
ItemPreviewComponent::ItemPreviewComponent (const File& file_)
    : file (file_)
{
    tryToLoadImage();
}

void ItemPreviewComponent::tryToLoadImage()
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

void ItemPreviewComponent::paint (Graphics& g)
{
    g.drawImageWithin (image, 2, 22, getWidth() - 4, getHeight() - 24,
                       RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                       false);

    g.setFont (Font (15.0f, Font::bold));
    g.setColour (Colours::white);
    g.drawMultiLineText (facts.joinIntoString ("\n"),
                         10, 15, getWidth() - 16);
}
