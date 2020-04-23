/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "jucer_ComponentLayoutEditor.h"
#include "jucer_EditingPanelBase.h"

//==============================================================================
class ComponentLayoutPanel  : public EditingPanelBase
{
public:
    //==============================================================================
    ComponentLayoutPanel (JucerDocument& doc, ComponentLayout& l)
        : EditingPanelBase (doc,
                            new LayoutPropsPanel (doc, l),
                            new ComponentLayoutEditor (doc, l)),
          layout (l)
    {
    }

    ~ComponentLayoutPanel()
    {
        deleteAllChildren();
    }

    void updatePropertiesList()
    {
        ((LayoutPropsPanel*) propsPanel)->updateList();
    }

    Rectangle<int> getComponentArea() const
    {
        return ((ComponentLayoutEditor*) editor)->getComponentArea();
    }

    Image createComponentSnapshot() const
    {
        return ((ComponentLayoutEditor*) editor)->createComponentLayerSnapshot();
    }

    ComponentLayout& layout;

private:
    class LayoutPropsPanel  : public Component,
                              public ChangeListener
    {
    public:
        LayoutPropsPanel (JucerDocument& doc, ComponentLayout& l)
            : document (doc), layout (l)
        {
            layout.getSelectedSet().addChangeListener (this);
            addAndMakeVisible (propsPanel);
        }

        ~LayoutPropsPanel()
        {
            layout.getSelectedSet().removeChangeListener (this);
            clear();
        }

        void resized()
        {
            propsPanel.setBounds (4, 4, getWidth() - 8, getHeight() - 8);
        }

        void changeListenerCallback (ChangeBroadcaster*)
        {
            updateList();
        }

        void clear()
        {
            propsPanel.clear();
        }

        void updateList()
        {
            clear();

            auto numSelected = layout.getSelectedSet().getNumSelected();

            if (numSelected > 0) // xxx need to cope with multiple
            {
                if (auto* comp = layout.getSelectedSet().getSelectedItem (0))
                    if (auto* type = ComponentTypeHandler::getHandlerFor (*comp))
                        type->addPropertiesToPropertyPanel (comp, document, propsPanel, numSelected > 1);
            }
        }

    private:
        JucerDocument& document;
        ComponentLayout& layout;
        PropertyPanel propsPanel;
    };
};
