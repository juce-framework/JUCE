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

        if (paintRoutine.getSelectedElements().getNumSelected() == 1) // xxx need to cope with multiple
        {
            if (PaintElement* const pe = paintRoutine.getSelectedElements().getSelectedItem (0))
            {
                if (paintRoutine.containsElement (pe))
                {
                    Array <PropertyComponent*> props;
                    pe->getEditableProperties (props);

                    propsPanel->addSection (pe->getTypeName(), props);
                }
            }
        }

        if (paintRoutine.getSelectedPoints().getNumSelected() == 1) // xxx need to cope with multiple
        {
            if (PathPoint* const point = paintRoutine.getSelectedPoints().getSelectedItem (0))
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
