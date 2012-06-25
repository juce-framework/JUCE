/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

//==============================================================================
String hexString8Digits (int value);

String createAlphaNumericUID();
String createGUID (const String& seed); // Turns a seed into a windows GUID

String escapeSpaces (const String& text); // replaces spaces with blackslash-space
String addQuotesIfContainsSpaces (const String& text);

StringPairArray parsePreprocessorDefs (const String& defs);
StringPairArray mergePreprocessorDefs (StringPairArray inheritedDefs, const StringPairArray& overridingDefs);
String createGCCPreprocessorFlags (const StringPairArray& defs);
String replacePreprocessorDefs (const StringPairArray& definitions, String sourceString);

StringArray getSearchPathsFromString (const String& searchPath);

void setValueIfVoid (Value value, const var& defaultValue);

//==============================================================================
int indexOfLineStartingWith (const StringArray& lines, const String& text, int startIndex);

void autoScrollForMouseEvent (const MouseEvent& e, bool scrollX = true, bool scrollY = true);

void drawComponentPlaceholder (Graphics& g, int w, int h, const String& text);
void drawRecessedShadows (Graphics& g, int w, int h, int shadowSize);

void showUTF8ToolWindow();

// Start a callout modally, which will delete the content comp when it's dismissed.
void launchAsyncCallOutBox (Component& attachTo, Component* content);

bool cancelAnyModalComponents();
bool reinvokeCommandAfterCancellingModalComps (const ApplicationCommandTarget::InvocationInfo&);

//==============================================================================
class RolloverHelpComp   : public Component,
                           private Timer
{
public:
    RolloverHelpComp();

    void paint (Graphics& g);
    void timerCallback();

private:
    Component* lastComp;
    String lastTip;

    static String findTip (Component*);
};

//==============================================================================
class PropertyListBuilder
{
public:
    PropertyListBuilder() {}

    void add (PropertyComponent* propertyComp)
    {
        components.add (propertyComp);
    }

    void add (PropertyComponent* propertyComp, const String& tooltip)
    {
        propertyComp->setTooltip (tooltip);
        add (propertyComp);
    }

    void setPreferredHeight (int height)
    {
        for (int j = components.size(); --j >= 0;)
            components.getUnchecked(j)->setPreferredHeight (height);
    }

    Array <PropertyComponent*> components;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyListBuilder);
};

//==============================================================================
class FloatingLabelComponent    : public Component
{
public:
    FloatingLabelComponent();

    void remove();
    void update (Component* parent, const String& text, const Colour& textColour,
                 int x, int y, bool toRight, bool below);

    void paint (Graphics& g);

private:
    Font font;
    Colour colour;
    GlyphArrangement glyphs;
};

//==============================================================================
// A ValueSource which takes an input source, and forwards any changes in it.
// This class is a handy way to create sources which re-map a value.
class ValueSourceFilter   : public Value::ValueSource,
                            public Value::Listener
{
public:
    ValueSourceFilter (const Value& source)  : sourceValue (source)
    {
        sourceValue.addListener (this);
    }

    void valueChanged (Value&)      { sendChangeMessage (true); }

protected:
    Value sourceValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueSourceFilter);
};
