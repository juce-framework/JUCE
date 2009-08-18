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
#include "jucer_PaintRoutinePanel.h"
#include "../properties/jucer_ColourPropertyComponent.h"
#include "../model/paintelements/jucer_PaintElementPath.h"


//==============================================================================
class ComponentBackgroundColourProperty    : public ColourPropertyComponent,
                                             private ChangeListener
{
public:
    ComponentBackgroundColourProperty (JucerDocument& document_,
                                       PaintRoutine& routine_)
        : ColourPropertyComponent (T("background"), false),
          document (document_),
          routine (routine_)
    {
        document.addChangeListener (this);
    }

    ~ComponentBackgroundColourProperty()
    {
        document.removeChangeListener (this);
    }

    void changeListenerCallback (void*)
    {
        refresh();
    }

    void setColour (const Colour& newColour)    { routine.setBackgroundColour (newColour); }
    const Colour getColour() const              { return routine.getBackgroundColour(); }

    void resetToDefault()
    {
        jassertfalse // option shouldn't be visible
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
    //==============================================================================
    GraphicsPropsPanel (PaintRoutine& paintRoutine_,
                        JucerDocument* document_)
        : paintRoutine (paintRoutine_),
          document (document_)
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
        XmlElement* const state = propsPanel->getOpennessState();

        clear();

        if (document != 0)
        {
            Array <PropertyComponent*> props;
            props.add (new ComponentBackgroundColourProperty (*document, paintRoutine));

            propsPanel->addSection (T("Class Properties"), props);
        }

        if (state != 0)
        {
            propsPanel->restoreOpennessState (*state);
            delete state;
        }

        if (paintRoutine.getSelectedElements().getNumSelected() == 1) // xxx need to cope with multiple
        {
            PaintElement* const pe = paintRoutine.getSelectedElements().getSelectedItem (0);

            if (pe != 0)
            {
                Array <PropertyComponent*> props;
                pe->getEditableProperties (props);

                propsPanel->addSection (pe->getTypeName(), props);
            }
        }

        if (paintRoutine.getSelectedPoints().getNumSelected() == 1) // xxx need to cope with multiple
        {
            PathPoint* const point = paintRoutine.getSelectedPoints().getSelectedItem (0);

            if (point != 0)
            {
                Array <PropertyComponent*> props;
                point->getEditableProperties (props);

                propsPanel->addSection (T("Path segment"), props);
            }
        }
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    PaintRoutine& paintRoutine;
    JucerDocument* document;

    PropertyPanel* propsPanel;
};


//==============================================================================
PaintRoutinePanel::PaintRoutinePanel (JucerDocument& document_,
                                      PaintRoutine& routine_,
                                      JucerDocumentHolder* documentHolder)
    : EditingPanelBase (document_,
                        new GraphicsPropsPanel (routine_, &document_),
                        new PaintRoutineEditor (routine_, document_, documentHolder)),
      routine (routine_)
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

const Rectangle PaintRoutinePanel::getComponentArea() const
{
    return ((PaintRoutineEditor*) editor)->getComponentArea();
}
