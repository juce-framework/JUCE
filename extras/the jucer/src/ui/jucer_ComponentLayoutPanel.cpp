/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

    void changeListenerCallback (ChangeBroadcaster*)
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

const Rectangle<int> ComponentLayoutPanel::getComponentArea() const
{
    return ((ComponentLayoutEditor*) editor)->getComponentArea();
}

const Image ComponentLayoutPanel::createComponentSnapshot() const
{
    return ((ComponentLayoutEditor*) editor)->createComponentLayerSnapshot();
}
