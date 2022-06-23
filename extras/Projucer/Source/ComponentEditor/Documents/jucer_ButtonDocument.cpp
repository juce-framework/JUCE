/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../../Application/jucer_Headers.h"
#include "jucer_ButtonDocument.h"
#include "../jucer_UtilityFunctions.h"

//==============================================================================
static const int normalOff = 0;
static const int overOff = 1;
static const int downOff = 2;
static const int normalOn = 3;
static const int overOn = 4;
static const int downOn = 5;
static const int background = 6;


//==============================================================================
ButtonDocument::ButtonDocument (SourceCodeDocument* c)
    : JucerDocument (c)
{
    paintStatesEnabled [normalOff] = true;
    paintStatesEnabled [overOff] = true;
    paintStatesEnabled [downOff] = true;
    paintStatesEnabled [normalOn] = false;
    paintStatesEnabled [overOn] = false;
    paintStatesEnabled [downOn] = false;
    paintStatesEnabled [background] = false;

    parentClasses = "public juce::Button";

    for (int i = 7; --i >= 0;)
    {
        paintRoutines[i].reset (new PaintRoutine());
        paintRoutines[i]->setDocument (this);
        paintRoutines[i]->setBackgroundColour (Colours::transparentBlack);
    }
}

ButtonDocument::~ButtonDocument()
{
}

static const char* const stateNames[] =
{
    "normal", "over", "down",
    "normal on", "over on", "down on",
    "common background"
};

static int stateNameToIndex (const String& name)
{
    for (int i = 7; --i >= 0;)
        if (name.equalsIgnoreCase (stateNames[i]))
            return i;

    jassertfalse;
    return normalOff;
}


int ButtonDocument::getNumPaintRoutines() const
{
    int n = 0;

    for (int i = 7; --i >= 0;)
        if (paintStatesEnabled [i])
            ++n;

    return n;
}

StringArray ButtonDocument::getPaintRoutineNames() const
{
    StringArray s;

    for (int i = 0; i < 7; ++i)
        if (paintStatesEnabled [i])
            s.add (stateNames [i]);

    return s;
}

PaintRoutine* ButtonDocument::getPaintRoutine (const int index) const
{
    int n = 0;

    for (int i = 0; i < 7; ++i)
    {
        if (paintStatesEnabled [i])
        {
            if (index == n)
                return paintRoutines[i].get();

            ++n;
        }
    }

    jassertfalse;
    return {};
}

void ButtonDocument::setStatePaintRoutineEnabled (const int index, bool b)
{
    jassert (index > 0 && index < 7);

    if (paintStatesEnabled [index] != b)
    {
        paintStatesEnabled [index] = b;
        changed();
    }
}

bool ButtonDocument::isStatePaintRoutineEnabled (const int index) const
{
    return paintStatesEnabled [index];
}

int ButtonDocument::chooseBestEnabledPaintRoutine (int paintRoutineWanted) const
{
    switch (paintRoutineWanted)
    {
        case normalOff: return normalOff;
        case overOff:   return paintStatesEnabled [overOff] ? overOff : normalOff;
        case downOff:   return paintStatesEnabled [downOff] ? downOff : chooseBestEnabledPaintRoutine (overOff);
        case normalOn:  return paintStatesEnabled [normalOn] ? normalOn : normalOff;
        case overOn:    return paintStatesEnabled [overOn] ? overOn : (paintStatesEnabled [normalOn] ? normalOn : chooseBestEnabledPaintRoutine (overOff));
        case downOn:    return paintStatesEnabled [downOn] ? downOn : ((paintStatesEnabled [overOn] || paintStatesEnabled [normalOn])
                                                                        ? chooseBestEnabledPaintRoutine (overOn)
                                                                        : chooseBestEnabledPaintRoutine (downOff));
        default: jassertfalse; break;
    }

    return normalOff;
}

//==============================================================================
String ButtonDocument::getTypeName() const
{
    return "Button";
}

JucerDocument* ButtonDocument::createCopy()
{
    auto newOne = new ButtonDocument (cpp);
    newOne->resources = resources;
    newOne->loadFromXml (*createXml());
    return newOne;
}

std::unique_ptr<XmlElement> ButtonDocument::createXml() const
{
    auto doc = JucerDocument::createXml();

    for (int i = 0; i < 7; ++i)
    {
        auto e = paintRoutines[i]->createXml();
        e->setAttribute ("buttonState", stateNames [i]);
        e->setAttribute ("enabled", paintStatesEnabled [i]);

        doc->addChildElement (e);
    }

    return doc;
}

bool ButtonDocument::loadFromXml (const XmlElement& xml)
{
    if (JucerDocument::loadFromXml (xml))
    {
        for (int i = 7; --i >= 0;)
            paintStatesEnabled [i] = false;

        for (auto* e : xml.getChildWithTagNameIterator (PaintRoutine::xmlTagName))
        {
            const int stateIndex = stateNameToIndex (e->getStringAttribute ("buttonState"));

            paintRoutines [stateIndex]->loadFromXml (*e);
            paintStatesEnabled [stateIndex] = e->getBoolAttribute ("enabled", stateIndex < normalOn);
        }

        changed();
        getUndoManager().clearUndoHistory();
        return true;
    }

    return false;
}

void ButtonDocument::getOptionalMethods (StringArray& baseClasses,
                                         StringArray& returnValues,
                                         StringArray& methods,
                                         StringArray& initialContents) const
{
    JucerDocument::getOptionalMethods (baseClasses, returnValues, methods, initialContents);

    addMethod ("juce::Button", "void", "clicked()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Button", "void", "buttonStateChanged()", "", baseClasses, returnValues, methods, initialContents);
}

//==============================================================================
class ButtonStatePaintEnabledProperty   : public BooleanPropertyComponent,
                                          private ChangeListener
{
public:
    ButtonStatePaintEnabledProperty (const String& name, ButtonDocument& doc, const int stateMethod_)
        : BooleanPropertyComponent (name, "enabled", "disabled"),
          document (doc),
          stateMethod (stateMethod_)
    {
        document.addChangeListener (this);
    }

    ~ButtonStatePaintEnabledProperty()
    {
        document.removeChangeListener (this);
    }

    void setState (bool newState)
    {
        document.setStatePaintRoutineEnabled (stateMethod, newState);
    }

    bool getState() const
    {
        return document.isStatePaintRoutineEnabled (stateMethod);
    }

private:
    void changeListenerCallback (ChangeBroadcaster*)
    {
        refresh();
    }

    ButtonDocument& document;
    const int stateMethod;
};

void ButtonDocument::addExtraClassProperties (PropertyPanel& panel)
{
    Array <PropertyComponent*> props;

    for (int i = 1; i < 7; ++i)
        props.add (new ButtonStatePaintEnabledProperty (stateNames[i], *this, i));

    panel.addSection ("Button paint routines", props);
}

//==============================================================================
class ButtonTestComponent   : public Button
{
public:
    ButtonTestComponent (ButtonDocument* const doc, const bool fillBackground)
        : Button (String()),
          document (doc),
          alwaysFillBackground (fillBackground)
    {
        setClickingTogglesState (true);
    }

    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        if (document->paintStatesEnabled [background])
        {
            document->paintRoutines [background]->fillWithBackground (g, alwaysFillBackground);
            document->paintRoutines [background]->drawElements (g, getLocalBounds());
        }

        const int stateIndex
            = getToggleState()
                ? (isButtonDown ? document->chooseBestEnabledPaintRoutine (downOn)
                                : (isMouseOverButton ? document->chooseBestEnabledPaintRoutine (overOn)
                                                     : document->chooseBestEnabledPaintRoutine (normalOn)))
                : (isButtonDown ? document->chooseBestEnabledPaintRoutine (downOff)
                                : (isMouseOverButton ? document->chooseBestEnabledPaintRoutine (overOff)
                                                     : normalOff));

        document->paintRoutines [stateIndex]->fillWithBackground (g, ! document->paintStatesEnabled [background]);
        document->paintRoutines [stateIndex]->drawElements (g, getLocalBounds());
    }

private:
    ButtonDocument* const document;
    const bool alwaysFillBackground;
};

Component* ButtonDocument::createTestComponent (const bool alwaysFillBackground)
{
    return new ButtonTestComponent (this, alwaysFillBackground);
}

//==============================================================================
void ButtonDocument::fillInGeneratedCode (GeneratedCode& code) const
{
    JucerDocument::fillInGeneratedCode (code);

    code.parentClassInitialiser = "Button (" + quotedString (code.componentName, false) + ")";
    code.removeCallback ("void", "paint (Graphics& g)");
}

void ButtonDocument::fillInPaintCode (GeneratedCode& code) const
{
    jassert (paintStatesEnabled [normalOff]);
    String paintCode [7];

    for (int i = 0; i < 7; ++i)
        if (paintStatesEnabled [i])
            paintRoutines[i]->fillInGeneratedCode (code, paintCode [i]);

    String& s = code.getCallbackCode ("public juce::Button",
                                      "void",
                                      "paintButton (juce::Graphics& g, bool isMouseOverButton, bool isButtonDown)",
                                      false);

    int numPaintRoutines = getNumPaintRoutines();

    if (paintStatesEnabled [background])
    {
        s << paintCode [background] << "\n";
        --numPaintRoutines;
    }

    if (numPaintRoutines == 1)
    {
        s << paintCode [normalOff];
    }
    else if (numPaintRoutines == downOff && (paintStatesEnabled [overOff] || paintStatesEnabled [downOff] || paintStatesEnabled [normalOn]))
    {
        if (paintStatesEnabled [normalOn])
        {
            s << "if (getToggleState())\n{\n    "
              << CodeHelpers::indent (paintCode [normalOn], 4, false).trimEnd();
        }
        else if (paintStatesEnabled [overOff])
        {
            s << "if (isButtonDown || isMouseOverButton)\n{\n    "
              << CodeHelpers::indent (paintCode [overOff], 4, false).trimEnd();
        }
        else
        {
            s << "if (isButtonDown)\n{\n    "
              << CodeHelpers::indent (paintCode [downOff], 4, false).trimEnd();
        }

        s << "\n}\nelse\n{\n    "
          <<  CodeHelpers::indent (paintCode [normalOff], 4, false).trimEnd()
          << "\n}\n";
    }
    else if (numPaintRoutines == normalOn && paintStatesEnabled [overOff] && paintStatesEnabled [downOff])
    {
        s << "if (isButtonDown)\n{\n    "
          << CodeHelpers::indent (paintCode [downOff], 4, false).trimEnd()
          << "\n}\nelse if (isMouseOverButton)\n{\n    "
          << CodeHelpers::indent (paintCode [overOff], 4, false).trimEnd()
          << "\n}\nelse\n{\n    "
          << CodeHelpers::indent (paintCode [normalOff], 4, false).trimEnd()
          << "\n}\n";
    }
    else
    {
        if (paintStatesEnabled [normalOn] || paintStatesEnabled [overOn] || paintStatesEnabled [downOn])
        {
            s << "switch (getToggleState() ? (isButtonDown ? "
              << chooseBestEnabledPaintRoutine (downOn) << " : (isMouseOverButton ? "
              << chooseBestEnabledPaintRoutine (overOn) << " : "
              << chooseBestEnabledPaintRoutine (normalOn) << "))\n                         : (isButtonDown ? "
              << chooseBestEnabledPaintRoutine (downOff) << " : (isMouseOverButton ? "
              << chooseBestEnabledPaintRoutine (overOff) << " : 0)))\n{\n";
        }
        else
        {
            s << "switch (isButtonDown ? " << chooseBestEnabledPaintRoutine (downOff)
              << " : (isMouseOverButton ? " << chooseBestEnabledPaintRoutine (overOff)
              << " : 0))\n{\n";
        }

        for (int i = 0; i < 6; ++i)
        {
            if (paintStatesEnabled [i])
            {
                s << "case " << i << ":\n    {\n        "
                  << CodeHelpers::indent (paintCode [i], 8, false).trimEnd()
                  << "\n        break;\n    }\n\n";
            }
        }

        s << "default:\n    break;\n}\n";
    }
}
