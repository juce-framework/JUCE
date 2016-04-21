/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCER_GROUPINFORMATIONCOMPONENT_H_INCLUDED
#define JUCER_GROUPINFORMATIONCOMPONENT_H_INCLUDED

#include "../Project/jucer_Project.h"


//==============================================================================
class GroupInformationComponent  : public Component,
                                   private ListBoxModel,
                                   private ValueTree::Listener
{
public:
    GroupInformationComponent (const Project::Item& group)
        : item (group)
    {
        list.setModel (this);
        list.setColour (ListBox::backgroundColourId, Colours::transparentBlack);
        addAndMakeVisible (list);
        list.updateContent();
        list.setRowHeight (20);
        item.state.addListener (this);
        lookAndFeelChanged();
    }

    ~GroupInformationComponent()
    {
        item.state.removeListener (this);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        ProjucerLookAndFeel::fillWithBackgroundTexture (*this, g);
    }

    void resized() override
    {
        list.setBounds (getLocalBounds().reduced (5, 4));
    }

    int getNumRows() override
    {
        return item.getNumChildren();
    }

    void paintListBoxItem (int /*rowNumber*/, Graphics& g, int width, int height, bool /*rowIsSelected*/) override
    {
        g.setColour (Colours::white.withAlpha (0.4f));
        g.fillRect (0, 0, width, height - 1);
    }

    Component* refreshComponentForRow (int rowNumber, bool /*isRowSelected*/, Component* existingComponentToUpdate) override
    {
        ScopedPointer<Component> existing (existingComponentToUpdate);

        if (rowNumber < getNumRows())
        {
            Project::Item child (item.getChild (rowNumber));

            if (existingComponentToUpdate == nullptr
                 || dynamic_cast<FileOptionComponent*> (existing.get())->item != child)
            {
                existing = nullptr;
                existing = new FileOptionComponent (child);
            }
        }

        return existing.release();
    }

    //==============================================================================
    void valueTreePropertyChanged (ValueTree&, const Identifier&) override    { itemChanged(); }
    void valueTreeChildAdded (ValueTree&, ValueTree&) override                { itemChanged(); }
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override         { itemChanged(); }
    void valueTreeChildOrderChanged (ValueTree&, int, int) override           { itemChanged(); }
    void valueTreeParentChanged (ValueTree&) override                         { itemChanged(); }

private:
    Project::Item item;
    ListBox list;

    void itemChanged()
    {
        list.updateContent();
        repaint();
    }

    //==============================================================================
    class FileOptionComponent  : public Component
    {
    public:
        FileOptionComponent (const Project::Item& fileItem)
            : item (fileItem),
              compileButton ("Compile"),
              binaryResourceButton ("Binary Resource"),
              xcodeResourceButton ("Xcode Resource")
        {
            if (item.isFile())
            {
                addAndMakeVisible (compileButton);
                compileButton.getToggleStateValue().referTo (item.getShouldCompileValue());

                addAndMakeVisible (binaryResourceButton);
                binaryResourceButton.getToggleStateValue().referTo (item.getShouldAddToBinaryResourcesValue());

                addAndMakeVisible (xcodeResourceButton);
                xcodeResourceButton.getToggleStateValue().referTo (item.getShouldAddToXcodeResourcesValue());
            }
        }

        void paint (Graphics& g) override
        {
            int x = getHeight() + 6;

            item.getIcon().withContrastingColourTo (Colours::grey)
                .draw (g, Rectangle<float> (3.0f, 2.0f, x - 6.0f, getHeight() - 4.0f),
                       item.isIconCrossedOut());

            g.setColour (Colours::black);
            g.setFont (getHeight() * 0.6f);

            const int x2 = compileButton.isVisible() ? compileButton.getX() - 4
                                                     : getWidth() - 4;

            g.drawText (item.getName(), x, 0, x2 - x, getHeight(), Justification::centredLeft, true);
        }

        void resized() override
        {
            binaryResourceButton.setBounds (getWidth() - 110, 1, 110, getHeight() - 2);
            xcodeResourceButton.setBounds (binaryResourceButton.getX() - 110, 1, 110, getHeight() - 2);
            compileButton.setBounds (xcodeResourceButton.getX() - 70, 1, 70, getHeight() - 2);
        }

        Project::Item item;

    private:
        ToggleButton compileButton, binaryResourceButton, xcodeResourceButton;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroupInformationComponent)
};


#endif   // JUCER_GROUPINFORMATIONCOMPONENT_H_INCLUDED
