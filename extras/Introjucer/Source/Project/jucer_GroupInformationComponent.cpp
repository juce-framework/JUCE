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

#include "jucer_GroupInformationComponent.h"


//==============================================================================
GroupInformationComponent::GroupInformationComponent (const Project::Item& item_)
    : item (item_)
{
    list.setModel (this);
    addAndMakeVisible (&list);
    list.updateContent();
    list.setRowHeight (20);
    item.state.addListener (this);
}

GroupInformationComponent::~GroupInformationComponent()
{
    item.state.removeListener (this);
}

void GroupInformationComponent::resized()
{
    list.setSize (getWidth(), getHeight());
}

int GroupInformationComponent::getNumRows()
{
    return item.getNumChildren();
}

void GroupInformationComponent::paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
}

//==============================================================================
void GroupInformationComponent::valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property)
{
    list.updateContent();
}

void GroupInformationComponent::valueTreeChildAdded (ValueTree&, ValueTree&)
{
    list.updateContent();
}

void GroupInformationComponent::valueTreeChildRemoved (ValueTree&, ValueTree&)
{
    list.updateContent();
}

void GroupInformationComponent::valueTreeChildOrderChanged (ValueTree&)
{
    list.updateContent();
}

void GroupInformationComponent::valueTreeParentChanged (ValueTree&)
{
    list.updateContent();
}

//==============================================================================
class FileOptionComponent  : public Component
{
public:
    FileOptionComponent (const Project::Item& item_)
        : item (item_),
          compileButton ("Compile"),
          resourceButton ("Add to Binary Resources")
    {
        if (item.isFile())
        {
            addAndMakeVisible (&compileButton);
            compileButton.getToggleStateValue().referTo (item.getShouldCompileValue());

            addAndMakeVisible (&resourceButton);
            resourceButton.getToggleStateValue().referTo (item.getShouldAddToResourceValue());
        }
    }

    void paint (Graphics& g)
    {
        int x = getHeight() + 6;

        item.getIcon()->drawWithin (g, Rectangle<float> (2.0f, 2.0f, x - 4.0f, getHeight() - 4.0f),
                                    RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);

        g.setColour (Colours::black);
        g.setFont (getHeight() * 0.6f);

        const int x2 = compileButton.isVisible() ? compileButton.getX() - 4
                                                 : getWidth() - 4;

        g.drawText (item.getName(), x, 0, x2 - x, getHeight(), Justification::centredLeft, true);

        g.setColour (Colours::lightgrey);
        g.fillRect (0, getHeight() - 1, getWidth(), 1);
    }

    void resized()
    {
        int w = 180;
        resourceButton.setBounds (getWidth() - w, 1, w, getHeight() - 2);
        w = 100;
        compileButton.setBounds (resourceButton.getX() - w, 1, w, getHeight() - 2);
    }

    Project::Item item;

private:
    ToggleButton compileButton, resourceButton;
};

Component* GroupInformationComponent::refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate)
{
    if (rowNumber < getNumRows())
    {
        Project::Item child (item.getChild (rowNumber));

        if (existingComponentToUpdate == nullptr
             || dynamic_cast <FileOptionComponent*> (existingComponentToUpdate)->item != child)
        {
            delete existingComponentToUpdate;
            existingComponentToUpdate = new FileOptionComponent (child);
        }
    }
    else
    {
        deleteAndZero (existingComponentToUpdate);
    }

    return existingComponentToUpdate;
}
