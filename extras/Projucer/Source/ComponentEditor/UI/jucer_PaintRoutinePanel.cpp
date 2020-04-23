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

#include "../../Application/jucer_Headers.h"
#include "jucer_PaintRoutinePanel.h"
#include "../Properties/jucer_ColourPropertyComponent.h"
#include "../PaintElements/jucer_PaintElementPath.h"

//==============================================================================
class ComponentBackgroundColourProperty    : public JucerColourPropertyComponent,
                                             private ChangeListener
{
public:
    ComponentBackgroundColourProperty (JucerDocument& doc,
                                       PaintRoutine& routine_)
        : JucerColourPropertyComponent ("background", false),
          document (doc),
          routine (routine_)
    {
        document.addChangeListener (this);
    }

    ~ComponentBackgroundColourProperty() override
    {
        document.removeChangeListener (this);
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        refresh();
    }

    void setColour (Colour newColour) override    { routine.setBackgroundColour (newColour); }
    Colour getColour() const override             { return routine.getBackgroundColour(); }

    void resetToDefault() override
    {
        jassertfalse; // option shouldn't be visible
    }

protected:
    JucerDocument& document;
    PaintRoutine& routine;
};


//==============================================================================
class GraphicsPropsPanel  : public Component,
                            public ChangeListener
{
public:
    GraphicsPropsPanel (PaintRoutine& paintRoutine_,
                        JucerDocument* doc)
        : paintRoutine (paintRoutine_),
          document (doc)
    {
        paintRoutine.getSelectedElements().addChangeListener (this);
        paintRoutine.getSelectedPoints().addChangeListener (this);

        addAndMakeVisible (propsPanel = new PropertyPanel());
    }

    ~GraphicsPropsPanel()
    {
        paintRoutine.getSelectedPoints().removeChangeListener (this);
        paintRoutine.getSelectedElements().removeChangeListener (this);

        clear();
        deleteAllChildren();
    }

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
        auto state = propsPanel->getOpennessState();

        clear();

        if (document != nullptr)
        {
            Array <PropertyComponent*> props;
            props.add (new ComponentBackgroundColourProperty (*document, paintRoutine));

            propsPanel->addSection ("Class Properties", props);
        }

        if (state != nullptr)
            propsPanel->restoreOpennessState (*state);

        auto numSelected = paintRoutine.getSelectedElements().getNumSelected();

        if (numSelected > 0) // xxx need to cope with multiple
        {
            if (auto* pe = paintRoutine.getSelectedElements().getSelectedItem (0))
            {
                if (paintRoutine.containsElement (pe))
                {
                    Array <PropertyComponent*> props;
                    pe->getEditableProperties (props, numSelected > 1);

                    propsPanel->addSection (pe->getTypeName(), props);
                }
            }
        }

        if (paintRoutine.getSelectedPoints().getNumSelected() == 1) // xxx need to cope with multiple
        {
            if (auto* point = paintRoutine.getSelectedPoints().getSelectedItem (0))
            {
                Array <PropertyComponent*> props;
                point->getEditableProperties (props, false);

                propsPanel->addSection ("Path segment", props);
            }
        }
    }

private:
    PaintRoutine& paintRoutine;
    JucerDocument* document;

    PropertyPanel* propsPanel;
};


//==============================================================================
PaintRoutinePanel::PaintRoutinePanel (JucerDocument& doc, PaintRoutine& pr,
                                      JucerDocumentEditor* documentHolder)
    : EditingPanelBase (doc,
                        new GraphicsPropsPanel (pr, &doc),
                        new PaintRoutineEditor (pr, doc, documentHolder)),
      routine (pr)
{
}

PaintRoutinePanel::~PaintRoutinePanel()
{
    deleteAllChildren();
}

void PaintRoutinePanel::updatePropertiesList()
{
    ((GraphicsPropsPanel*) propsPanel)->updateList();
}

Rectangle<int> PaintRoutinePanel::getComponentArea() const
{
    return ((PaintRoutineEditor*) editor)->getComponentArea();
}
