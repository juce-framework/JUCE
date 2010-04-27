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

#include "jucer_JucerTreeViewBase.h"


//==============================================================================
JucerTreeViewBase::JucerTreeViewBase()
{
    setLinesDrawnForSubItems (false);
}

JucerTreeViewBase::~JucerTreeViewBase()
{
}

const Font JucerTreeViewBase::getFont() const
{
    return Font (getItemHeight() * 0.6f);
}

int JucerTreeViewBase::getTextX() const
{
    return getItemHeight() + 6;
}

void JucerTreeViewBase::paintItem (Graphics& g, int width, int height)
{
    if (isSelected())
        g.fillAll (Colour (0x401111ee));

    const int x = getTextX();

    g.setColour (isMissing() ? Colours::red : Colours::black);
    Image* icon = getIcon();

    if (icon != 0)
    {
        g.drawImageWithin (icon, 2, 2, x - 4, height - 4,
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                           false);

        ImageCache::release (icon);
    }

    g.setFont (getFont());
    g.drawFittedText (getDisplayName(), x, 0, width - x, height, Justification::centredLeft, 1, 0.8f);
}

void JucerTreeViewBase::paintOpenCloseButton (Graphics& g, int width, int height, bool isMouseOver)
{
    Path p;

    if (isOpen())
        p.addTriangle (width * 0.2f, height * 0.25f, width * 0.8f, height * 0.25f, width * 0.5f, height * 0.75f);
    else
        p.addTriangle (width * 0.25f, height * 0.25f, width * 0.8f, height * 0.5f, width * 0.25f, height * 0.75f);

    g.setColour (Colours::lightgrey);
    g.fillPath (p);
}

//==============================================================================
void JucerTreeViewBase::showRenameBox()
{
    TextEditor ed (String::empty);
    ed.setMultiLine (false, false);
    ed.setPopupMenuEnabled (false);
    ed.setSelectAllWhenFocused (true);
    ed.setFont (getFont());
    ed.addListener (this);
    ed.setText (getRenamingName());

    Rectangle<int> r (getItemPosition (true));
    r.setLeft (r.getX() + getTextX());
    r.setHeight (getItemHeight());
    ed.setBounds (r);
    getOwnerView()->addAndMakeVisible (&ed);

    if (ed.runModalLoop() != 0)
        setName (ed.getText());
}
