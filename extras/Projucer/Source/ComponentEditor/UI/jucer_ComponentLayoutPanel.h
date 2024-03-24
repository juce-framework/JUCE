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

    ~ComponentLayoutPanel() override
    {
        deleteAllChildren();
    }

    void updatePropertiesList() override
    {
        ((LayoutPropsPanel*) propsPanel)->updateList();
    }

    Rectangle<int> getComponentArea() const override
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
                              private ChangeListener
    {
    public:
        LayoutPropsPanel (JucerDocument& doc, ComponentLayout& l)
            : document (doc), layout (l)
        {
            layout.getSelectedSet().addChangeListener (this);
            addAndMakeVisible (propsPanel);
        }

        ~LayoutPropsPanel() override
        {
            layout.getSelectedSet().removeChangeListener (this);
            clear();
        }

        void resized() override
        {
            propsPanel.setBounds (4, 4, getWidth() - 8, getHeight() - 8);
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
        void changeListenerCallback (ChangeBroadcaster*) override
        {
            updateList();
        }

        JucerDocument& document;
        ComponentLayout& layout;
        PropertyPanel propsPanel;
    };
};
