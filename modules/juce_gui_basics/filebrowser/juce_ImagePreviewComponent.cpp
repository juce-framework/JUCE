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

    currentThumbnail = Image::null;
    currentDetails = String::empty;
    repaint();

    ScopedPointer<FileInputStream> in (fileToLoad.createInputStream());

    if (in != nullptr)
    {
        if (ImageFileFormat* const format = ImageFileFormat::findImageFormatForStream (*in))
        {
            currentThumbnail = format->decodeImage (*in);

            if (currentThumbnail.isValid())
            {
                int w = currentThumbnail.getWidth();
                int h = currentThumbnail.getHeight();

                currentDetails
                    << fileToLoad.getFileName() << "\n"
                    << format->getFormatName() << "\n"
                    << w << " x " << h << " pixels\n"
                    << File::descriptionOfSizeInBytes (fileToLoad.getSize());

                getThumbSize (w, h);

                currentThumbnail = currentThumbnail.rescaled (w, h);
            }
        }
    }
}

void ImagePreviewComponent::paint (Graphics& g)
{
    if (currentThumbnail.isValid())
    {
        g.setFont (13.0f);

        int w = currentThumbnail.getWidth();
        int h = currentThumbnail.getHeight();
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
