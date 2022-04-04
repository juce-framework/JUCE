/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

ImagePreviewComponent::ImagePreviewComponent()
{
}

ImagePreviewComponent::~ImagePreviewComponent()
{
}

//==============================================================================
void ImagePreviewComponent::getThumbSize (int& w, int& h) const
{
    auto availableW = proportionOfWidth (0.97f);
    auto availableH = getHeight() - 13 * 4;

    auto scale = jmin (1.0,
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

    currentThumbnail = Image();
    currentDetails.clear();
    repaint();

    FileInputStream in (fileToLoad);

    if (in.openedOk() && fileToLoad.existsAsFile())
    {
        if (auto format = ImageFileFormat::findImageFormatForStream (in))
        {
            currentThumbnail = format->decodeImage (in);

            if (currentThumbnail.isValid())
            {
                auto w = currentThumbnail.getWidth();
                auto h = currentThumbnail.getHeight();

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

        auto w = currentThumbnail.getWidth();
        auto h = currentThumbnail.getHeight();
        getThumbSize (w, h);

        const int numLines = 4;
        auto totalH = 13 * numLines + h + 4;
        auto y = (getHeight() - totalH) / 2;

        g.drawImageWithin (currentThumbnail,
                           (getWidth() - w) / 2, y, w, h,
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                           false);

        g.drawFittedText (currentDetails,
                          0, y + h + 4, getWidth(), 100,
                          Justification::centredTop, numLines);
    }
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> ImagePreviewComponent::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::image);
}

} // namespace juce
