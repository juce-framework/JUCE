/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../jucer_Headers.h"
#include "jucer_ComponentLayoutPanel.h"


//==============================================================================
class LayoutPropsPanel  : public Component,
                          public ChangeListener
{
public:
    //==============================================================================
    LayoutPropsPanel (JucerDocument& document_, ComponentLayout& layout_)
        : document (document_),
          layout (layout_)
    {
        layout.getSelectedSet().addChangeListener (this);

        addAndMakeVisible (propsPanel = new PropertyPanel());
    }

    ~LayoutPropsPanel()
    {
        layout.getSelectedSet().removeChangeListener (this);

        clear();
        deleteAllChildren();
    }

    //==============================================================================
    void resized()
    {
        propsPanel->setBounds (4, 4, getWidth() - 8, getHeight() - 8);
    }

    void changeListenerCallback (void*)
    {
        updateList();
    }

    void clear()
    {
        propsPanel->clear();
    }

    void updateList()
    {
        clear();

        if (layout.getSelectedSet().getNumSelected() == 1) // xxx need to cope with multiple
        {
            Component* comp = layout.getSelectedSet().getSelectedItem (0);

            if (comp != 0)
            {
                ComponentTypeHandler* const type = ComponentTypeHandler::getHandlerFor (*comp);

                if (type != 0)
                    type->addPropertiesToPropertyPanel (comp, document, *propsPanel);
            }
        }
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    JucerDocument& document;
    ComponentLayout& layout;

    PropertyPanel* propsPanel;
    bool editingGraphics;
};


//==============================================================================
ComponentLayoutPanel::ComponentLayoutPanel (JucerDocument& document_, ComponentLayout& layout_)
    : EditingPanelBase (document_,
                        new LayoutPropsPanel (document_, layout_),
                        new ComponentLayoutEditor (document_, layout_)),
      layout (layout_)
{
}

ComponentLayoutPanel::~ComponentLayoutPanel()
{
    deleteAllChildren();
}

void ComponentLayoutPanel::updatePropertiesList()
{
    ((LayoutPropsPanel*) propsPanel)->updateList();
}

const Rectangle ComponentLayoutPanel::getComponentArea() const
{
    return ((ComponentLayoutEditor*) editor)->getComponentArea();
}

Image* ComponentLayoutPanel::createComponentSnapshot() const
{
    return ((ComponentLayoutEditor*) editor)->createComponentLayerSnapshot();
}
