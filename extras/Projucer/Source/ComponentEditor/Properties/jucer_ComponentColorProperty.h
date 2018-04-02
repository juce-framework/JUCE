/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-license
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "jucer_ColorPropertyComponent.h"

//==============================================================================
template <class ComponentType>
class ComponentColorProperty  : public JucerColorPropertyComponent,
                                 private ChangeListener
{
public:
    ComponentColorProperty (const String& name,
                             ComponentType* comp,
                             JucerDocument& doc,
                             const bool canResetToDefault)
        : JucerColorPropertyComponent (name, canResetToDefault),
          component (comp),
          document (doc)
    {
        document.addChangeListener (this);
    }

    ~ComponentColorProperty()
    {
        document.removeChangeListener (this);
    }

    void changeListenerCallback (ChangeBroadcaster*)
    {
        refresh();
    }

protected:
    ComponentType* component;
    JucerDocument& document;
};


//==============================================================================
class ComponentColorIdProperty : public ComponentColorProperty <Component>
{
public:
    //==============================================================================
    ComponentColorIdProperty (Component* const comp,
                               JucerDocument& doc,
                               const int colorId_,
                               const String& name,
                               const bool canResetToDefault)
        : ComponentColorProperty <Component> (name, comp, doc, canResetToDefault),
          colorId (colorId_)
    {
    }

    //==============================================================================
    Color getColor() const
    {
        return component->findColor (colorId);
    }

    void setColor (Color newColor)
    {
        if (component->findColor (colorId) != newColor)
        {
            document.getUndoManager().undoCurrentTransactionOnly();

            document.perform (new ColorChangeAction (component,
                                                      *document.getComponentLayout(),
                                                      colorId,
                                                      newColor,
                                                      false),
                              "Change color");
        }
    }

    void resetToDefault()
    {
        document.getUndoManager().undoCurrentTransactionOnly();

        document.perform (new ColorChangeAction (component,
                                                  *document.getComponentLayout(),
                                                  colorId,
                                                  Colors::black,
                                                  true),
                          "Reset color");
    }

private:
    const int colorId;

    class ColorChangeAction  : public ComponentUndoableAction <Component>
    {
    public:
        ColorChangeAction (Component* const comp,
                            ComponentLayout& l,
                            const int colorId_,
                            Color newColor_,
                            const bool newColorIsDefault)
            : ComponentUndoableAction<Component> (comp, l),
              colorId (colorId_),
              newColor (newColor_),
              isDefault (newColorIsDefault)
        {
        }

        bool perform()
        {
            showCorrectTab();

            wasSpecified = getComponent()->isColorSpecified (colorId);
            oldColor = getComponent()->findColor (colorId);

            if (isDefault)
                getComponent()->removeColor (colorId);
            else
                getComponent()->setColor (colorId, newColor);

            changed();
            return true;
        }

        bool undo()
        {
            showCorrectTab();

            if (wasSpecified)
                getComponent()->setColor (colorId, oldColor);
            else
                getComponent()->removeColor (colorId);

            if (TextEditor* const te = dynamic_cast<TextEditor*> (getComponent()))
                te->applyFontToAllText (te->getFont());

            changed();
            return true;
        }

        int colorId;
        Color newColor, oldColor;
        bool isDefault, wasSpecified;
    };
};
