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

StringPairArray parsePreprocessorDefs (const String& defs);
StringPairArray mergePreprocessorDefs (StringPairArray inheritedDefs, const StringPairArray& overridingDefs);
String createGCCPreprocessorFlags (const StringPairArray& defs);
String replacePreprocessorDefs (const StringPairArray& definitions, String sourceString);

//==============================================================================
int indexOfLineStartingWith (const StringArray& lines, const String& text, int startIndex);

void autoScrollForMouseEvent (const MouseEvent& e, bool scrollX = true, bool scrollY = true);

void drawComponentPlaceholder (Graphics& g, int w, int h, const String& text);
void drawRecessedShadows (Graphics& g, int w, int h, int shadowSize);

void showUTF8ToolWindow();

// Start a callout modally, which will delete the content comp when it's dismissed.
void launchAsyncCallOutBox (Component& attachTo, Component* content);

//==============================================================================
class PropertyPanelWithTooltips  : public Component,
                                   public Timer
{
public:
    PropertyPanelWithTooltips();
    ~PropertyPanelWithTooltips();

    void paint (Graphics& g);
    void resized();
    void timerCallback();

    PropertyPanel panel;

private:
    Component* lastComp;
    String lastTip;

    Rectangle<int> getTipArea() const;
    String findTip (Component* c);
};

//==============================================================================
class FloatingLabelComponent    : public Component
{
public:
    FloatingLabelComponent();

    void remove();
    void update (Component* parent, const String& text, const Colour& textColour, int x, int y, bool toRight, bool below);
    void paint (Graphics& g);

private:
    Font font;
    Colour colour;
    GlyphArrangement glyphs;
};
