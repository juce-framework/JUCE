/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
// String::hashCode64 actually hit some dupes, so this is a more powerful version.
const int64 hashCode64 (const String& s);
const String randomHexString (Random& random, int numChars);
const String hexString8Digits (int value);

const String createAlphaNumericUID();
const String createGUID (const String& seed); // Turns a seed into a windows GUID

const String escapeSpaces (const String& text); // replaces spaces with blackslash-space

const StringPairArray parsePreprocessorDefs (const String& defs);
const StringPairArray mergePreprocessorDefs (StringPairArray inheritedDefs, const StringPairArray& overridingDefs);
const String createGCCPreprocessorFlags (const StringPairArray& defs);
const String replacePreprocessorDefs (const StringPairArray& definitions, String sourceString);

//==============================================================================
int indexOfLineStartingWith (const StringArray& lines, const String& text, int startIndex);

void autoScrollForMouseEvent (const MouseEvent& e, bool scrollX = true, bool scrollY = true);

void drawComponentPlaceholder (Graphics& g, int w, int h, const String& text);
void drawRecessedShadows (Graphics& g, int w, int h, int shadowSize);


//==============================================================================
class PropertyPanelWithTooltips  : public Component,
                                   public Timer
{
public:
    PropertyPanelWithTooltips();
    ~PropertyPanelWithTooltips();

    PropertyPanel& getPanel() noexcept        { return panel; }

    void paint (Graphics& g);
    void resized();
    void timerCallback();

private:
    PropertyPanel panel;
    TextLayout layout;
    Component* lastComp;
    String lastTip;

    const String findTip (Component* c);
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
