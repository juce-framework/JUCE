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

#include "juce_ToolbarButton.h"
#include "../controls/juce_ToolbarItemFactory.h"
#include "../../graphics/imaging/juce_Image.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
ToolbarButton::ToolbarButton (const int itemId,
                              const String& buttonText,
                              Drawable* const normalImage_,
                              Drawable* const toggledOnImage_)
   : ToolbarItemComponent (itemId, buttonText, true),
     normalImage (normalImage_),
     toggledOnImage (toggledOnImage_)
{
}

ToolbarButton::~ToolbarButton()
{
    delete normalImage;
    delete toggledOnImage;
}

//==============================================================================
bool ToolbarButton::getToolbarItemSizes (int toolbarDepth,
                                         bool /*isToolbarVertical*/,
                                         int& preferredSize,
                                         int& minSize, int& maxSize)
{
    preferredSize = minSize = maxSize = toolbarDepth;
    return true;
}

void ToolbarButton::paintButtonArea (Graphics& g,
                                     int width, int height,
                                     bool /*isMouseOver*/,
                                     bool /*isMouseDown*/)
{
    Drawable* d = normalImage;

    if (getToggleState() && toggledOnImage != 0)
        d = toggledOnImage;

    if (! isEnabled())
    {
        Image im (Image::ARGB, width, height, true);
        Graphics g2 (im);
        d->drawWithin (g2, 0, 0, width, height, RectanglePlacement::centred);
        im.desaturate();

        g.drawImageAt (&im, 0, 0);
    }
    else
    {
        d->drawWithin (g, 0, 0, width, height, RectanglePlacement::centred);
    }
}

void ToolbarButton::contentAreaChanged (const Rectangle&)
{
}


END_JUCE_NAMESPACE
