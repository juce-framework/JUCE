/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../../jucer_Headers.h"
#include "jucer_PaintRoutinePanel.h"
#include "../properties/jucer_ColourPropertyComponent.h"
#include "../paintelements/jucer_PaintElementPath.h"


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

    ~ComponentBackgroundColourProperty()
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
        ScopedPointer<XmlElement> state (propsPanel->getOpennessState());

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
