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
