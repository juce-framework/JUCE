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
        : ColourPropertyComponent ("background", false),
          document (document_),
          routine (routine_)
    {
        document.addChangeListener (this);
    }

    ~ComponentBackgroundColourProperty()
    {
        document.removeChangeListener (this);
    }

    void changeListenerCallback (ChangeBroadcaster*)
    {
        refresh();
    }

    void setColour (const Colour& newColour)    { routine.setBackgroundColour (newColour); }
    Colour getColour() const                    { return routine.getBackgroundColour(); }

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
        XmlElement* const state = propsPanel->getOpennessState();

        clear();

        if (document != 0)
        {
            Array <PropertyComponent*> props;
            props.add (new ComponentBackgroundColourProperty (*document, paintRoutine));

            propsPanel->addSection ("Class Properties", props);
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

const Rectangle<int> PaintRoutinePanel::getComponentArea() const
{
    return ((PaintRoutineEditor*) editor)->getComponentArea();
}
