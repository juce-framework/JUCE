/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ImagePreviewComponent.h"
#include "../../graphics/imaging/juce_ImageFileFormat.h"
#include "../../../io/files/juce_FileInputStream.h"


//==============================================================================
ImagePreviewComponent::ImagePreviewComponent()
{
}

ImagePreviewComponent::~ImagePreviewComponent()
{
}

//==============================================================================
void ImagePreviewComponent::getThumbSize (int& w, int& h) const
{
    const int availableW = proportionOfWidth (0.97f);
    const int availableH = getHeight() - 13 * 4;

    const double scale = jmin (1.0,
                               availableW / (double) w,
                               availableH / (double) h);

    w = roundToInt (scale * w);
    h = roundToInt (scale * h);
}

void ImagePreviewComponent::selectedFileChanged (const File& file)
{
    if (fileToLoad != file)
    {
        fileToLoad = file;
        startTimer (100);
    }
}

void ImagePreviewComponent::timerCallback()
{
    stopTimer();

    currentThumbnail = 0;
    currentDetails = String::empty;
    repaint();

    ScopedPointer <FileInputStream> in (fileToLoad.createInputStream());

    if (in != 0)
    {
        ImageFileFormat* const format = ImageFileFormat::findImageFormatForStream (*in);

        if (format != 0)
        {
            currentThumbnail = format->decodeImage (*in);

            if (currentThumbnail != 0)
            {
                int w = currentThumbnail->getWidth();
                int h = currentThumbnail->getHeight();

                currentDetails
                    << fileToLoad.getFileName() << "\n"
                    << format->getFormatName() << "\n"
                    << w << " x " << h << " pixels\n"
                    << File::descriptionOfSizeInBytes (fileToLoad.getSize());

                getThumbSize (w, h);

                currentThumbnail = currentThumbnail->createCopy (w, h);
            }
        }
    }
}

void ImagePreviewComponent::paint (Graphics& g)
{
    if (currentThumbnail != 0)
    {
        g.setFont (13.0f);

        int w = currentThumbnail->getWidth();
        int h = currentThumbnail->getHeight();
        getThumbSize (w, h);

        const int numLines = 4;
        const int totalH = 13 * numLines + h + 4;
        const int y = (getHeight() - totalH) / 2;

        g.drawImageWithin (currentThumbnail,
                           (getWidth() - w) / 2, y, w, h,
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                           false);

        g.drawFittedText (currentDetails,
                          0, y + h + 4, getWidth(), 100,
                          Justification::centredTop, numLines);
    }
}


END_JUCE_NAMESPACE
