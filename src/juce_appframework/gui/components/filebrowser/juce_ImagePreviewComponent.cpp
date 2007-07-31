/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ImagePreviewComponent.h"
#include "../../graphics/imaging/juce_ImageFileFormat.h"
#include "../../../../juce_core/io/files/juce_FileInputStream.h"


//==============================================================================
ImagePreviewComponent::ImagePreviewComponent()
    : currentThumbnail (0)
{
}

ImagePreviewComponent::~ImagePreviewComponent()
{
    delete currentThumbnail;
}

//==============================================================================
void ImagePreviewComponent::getThumbSize (int& w, int& h) const
{
    const int availableW = proportionOfWidth (0.97f);
    const int availableH = getHeight() - 13 * 4;

    const double scale = jmin (1.0,
                               availableW / (double) w,
                               availableH / (double) h);

    w = roundDoubleToInt (scale * w);
    h = roundDoubleToInt (scale * h);
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

    deleteAndZero (currentThumbnail);
    currentDetails = String::empty;
    repaint();

    FileInputStream* const in = fileToLoad.createInputStream();

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

                Image* const reduced = currentThumbnail->createCopy (w, h);

                delete currentThumbnail;
                currentThumbnail = reduced;
            }
        }

        delete in;
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
