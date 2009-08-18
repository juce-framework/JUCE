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

#include "../../jucer_Headers.h"
#include "jucer_ButtonDocument.h"

//==============================================================================
static const int normalOff = 0;
static const int overOff = 1;
static const int downOff = 2;
static const int normalOn = 3;
static const int overOn = 4;
static const int downOn = 5;
static const int background = 6;


//==============================================================================
ButtonDocument::ButtonDocument()
{
    paintStatesEnabled [normalOff] = true;
    paintStatesEnabled [overOff] = true;
    paintStatesEnabled [downOff] = true;
    paintStatesEnabled [normalOn] = false;
    paintStatesEnabled [overOn] = false;
    paintStatesEnabled [downOn] = false;
    paintStatesEnabled [background] = false;

    parentClasses = T("public Button");

    for (int i = 7; --i >= 0;)
    {
        paintRoutines [i] = new PaintRoutine();
        paintRoutines [i]->setDocument (this);
        paintRoutines [i]->setBackgroundColour (Colours::transparentBlack);
    }
}

ButtonDocument::~ButtonDocument()
{
    for (int i = 7; --i >= 0;)
        delete paintRoutines [i];
}

static const tchar* const stateNames[] =
{
    T("normal"), T("over"), T("down"),
    T("normal on"), T("over on"), T("down on"),
    T("common background")
};

int stateNameToIndex (const String& name)
{
    for (int i = 7; --i >= 0;)
        if (name.equalsIgnoreCase (stateNames[i]))
            return i;

    jassertfalse
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

const StringArray ButtonDocument::getPaintRoutineNames() const
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
                return paintRoutines [i];
            else
                ++n;
        }
    }

    jassertfalse
    return 0;
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
    case normalOff:
        return normalOff;

    case overOff:
        return paintStatesEnabled [overOff] ? overOff : normalOff;

    case downOff:
        return paintStatesEnabled [downOff] ? downOff : chooseBestEnabledPaintRoutine (overOff);

    case normalOn:
        return paintStatesEnabled [normalOn] ? normalOn : normalOff;

    case overOn:
        return paintStatesEnabled [overOn] ? overOn : (paintStatesEnabled [normalOn] ? normalOn : chooseBestEnabledPaintRoutine (overOff));

    case downOn:
        return paintStatesEnabled [downOn] ? downOn
                                               : ((paintStatesEnabled [overOn] || paintStatesEnabled [normalOn])
                                                    ? chooseBestEnabledPaintRoutine (overOn)
                                                    : chooseBestEnabledPaintRoutine (downOff));

    default:
        jassertfalse
        break;
    }

    return normalOff;
}

//==============================================================================
const String ButtonDocument::getTypeName() const
{
    return T("Button");
}

JucerDocument* ButtonDocument::createCopy()
{
    ButtonDocument* newOne = new ButtonDocument();

    newOne->resources = resources;
    newOne->setFile (getFile());

    XmlElement* const xml = createXml();
    newOne->loadFromXml (*xml);
    delete xml;

    return newOne;
}

XmlElement* ButtonDocument::createXml() const
{
    XmlElement* const doc = JucerDocument::createXml();

    for (int i = 0; i < 7; ++i)
    {
        XmlElement* e = paintRoutines [i]->createXml();
        e->setAttribute (T("buttonState"), stateNames [i]);
        e->setAttribute (T("enabled"), paintStatesEnabled [i]);

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

        forEachXmlChildElementWithTagName (xml, e, PaintRoutine::xmlTagName)
        {
            const int stateIndex = stateNameToIndex (e->getStringAttribute (T("buttonState")));

            paintRoutines [stateIndex]->loadFromXml (*e);
            paintStatesEnabled [stateIndex] = e->getBoolAttribute (T("enabled"), stateIndex < normalOn);
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

    addMethod ("Button", "void", "clicked()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Button", "void", "buttonStateChanged()", "", baseClasses, returnValues, methods, initialContents);
}

//==============================================================================
class ButtonStatePaintEnabledProperty   : public BooleanPropertyComponent,
                                          private ChangeListener
{
public:
    ButtonStatePaintEnabledProperty (const String& name, ButtonDocument& document_, const int stateMethod_)
        : BooleanPropertyComponent (name, T("enabled"), T("disabled")),
          document (document_),
          stateMethod (stateMethod_)
    {
        document.addChangeListener (this);
    }

    ~ButtonStatePaintEnabledProperty()
    {
        document.removeChangeListener (this);
    }

    void setState (const bool newState)
    {
        document.setStatePaintRoutineEnabled (stateMethod, newState);
    }

    bool getState() const
    {
        return document.isStatePaintRoutineEnabled (stateMethod);
    }

private:
    void changeListenerCallback (void*)
    {
        refresh();
    }

    ButtonDocument& document;
    const int stateMethod;
};

void ButtonDocument::addExtraClassProperties (PropertyPanel* panel)
{
    Array <PropertyComponent*> props;

    for (int i = 1; i < 7; ++i)
        props.add (new ButtonStatePaintEnabledProperty (stateNames[i], *this, i));

    panel->addSection (T("Button paint routines"), props);
}

//==============================================================================
class ButtonTestComponent   : public Button
{
public:
    ButtonTestComponent (ButtonDocument* const document_, const bool alwaysFillBackground_)
        : Button (String::empty),
          document (document_),
          alwaysFillBackground (alwaysFillBackground_)
    {
        setClickingTogglesState (true);
    }

    ~ButtonTestComponent()
    {
    }

    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown)
    {
        if (document->paintStatesEnabled [background])
        {
            document->paintRoutines [background]->fillWithBackground (g, alwaysFillBackground);
            document->paintRoutines [background]->drawElements (g, Rectangle (0, 0, getWidth(), getHeight()));
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
        document->paintRoutines [stateIndex]->drawElements (g, Rectangle (0, 0, getWidth(), getHeight()));
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

    code.parentClassInitialiser = T("Button (") + quotedString (code.componentName) + T(")");
    code.removeCallback (T("void"), T("paint (Graphics& g)"));
}

void ButtonDocument::fillInPaintCode (GeneratedCode& code) const
{
    jassert (paintStatesEnabled [normalOff]);
    String paintCode [7];

    for (int i = 0; i < 7; ++i)
        if (paintStatesEnabled [i])
            paintRoutines[i]->fillInGeneratedCode (code, paintCode [i]);

    String& s = code.getCallbackCode (T("public Button"),
                                      T("void"),
                                      T("paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown)"),
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
              << indentCode (paintCode [normalOn], 4).trimEnd();
        }
        else if (paintStatesEnabled [overOff])
        {
            s << "if (isButtonDown || isMouseOverButton)\n{\n    "
              << indentCode (paintCode [overOff], 4).trimEnd();
        }
        else
        {
            s << "if (isButtonDown)\n{\n    "
              << indentCode (paintCode [downOff], 4).trimEnd();
        }

        s << "\n}\nelse\n{\n    "
          <<  indentCode (paintCode [normalOff], 4).trimEnd()
          << "\n}\n";
    }
    else if (numPaintRoutines == normalOn && paintStatesEnabled [overOff] && paintStatesEnabled [downOff])
    {
        s << "if (isButtonDown)\n{\n    "
          << indentCode (paintCode [downOff], 4).trimEnd()
          << "\n}\nelse if (isMouseOverButton)\n{\n    "
          << indentCode (paintCode [overOff], 4).trimEnd()
          << "\n}\nelse\n{\n    "
          << indentCode (paintCode [normalOff], 4).trimEnd()
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
                  << indentCode (paintCode [i], 8).trimEnd()
                  << "\n        break;\n    }\n\n";
            }
        }

        s << "default:\n    break;\n}\n";
    }
}
