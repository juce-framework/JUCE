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

#ifndef JUCER_MISCUTILITIES_H_INCLUDED
#define JUCER_MISCUTILITIES_H_INCLUDED


String hexString8Digits (int value);

String createAlphaNumericUID();
String createGUID (const String& seed); // Turns a seed into a windows GUID

String escapeSpaces (const String& text); // replaces spaces with blackslash-space
String addQuotesIfContainsSpaces (const String& text);

StringPairArray parsePreprocessorDefs (const String& defs);
StringPairArray mergePreprocessorDefs (StringPairArray inheritedDefs, const StringPairArray& overridingDefs);
String createGCCPreprocessorFlags (const StringPairArray& defs);
String replacePreprocessorDefs (const StringPairArray& definitions, String sourceString);

StringArray getCleanedStringArray (StringArray);
StringArray getSearchPathsFromString (const String& searchPath);
StringArray getCommaOrWhitespaceSeparatedItems (const String&);

void setValueIfVoid (Value value, const var& defaultValue);

void addPlistDictionaryKey (XmlElement* xml, const String& key, const String& value);
void addPlistDictionaryKeyBool (XmlElement* xml, const String& key, bool value);
void addPlistDictionaryKeyInt (XmlElement* xml, const String& key, int value);

bool fileNeedsCppSyntaxHighlighting (const File& file);

//==============================================================================
int indexOfLineStartingWith (const StringArray& lines, const String& text, int startIndex);

void autoScrollForMouseEvent (const MouseEvent& e, bool scrollX = true, bool scrollY = true);

//==============================================================================
struct PropertyListBuilder
{
    void add (PropertyComponent* propertyComp)
    {
        components.add (propertyComp);
    }

    void add (PropertyComponent* propertyComp, const String& tooltip)
    {
        propertyComp->setTooltip (tooltip);
        add (propertyComp);
    }

    void addSearchPathProperty (const Value& value, const String& name, const String& mainHelpText)
    {
        add (new TextPropertyComponent (value, name, 16384, true),
             mainHelpText + " Use semi-colons or new-lines to separate multiple paths.");
    }

    void setPreferredHeight (int height)
    {
        for (int j = components.size(); --j >= 0;)
            components.getUnchecked(j)->setPreferredHeight (height);
    }

    Array<PropertyComponent*> components;
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

    void valueChanged (Value&) override      { sendChangeMessage (true); }

protected:
    Value sourceValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueSourceFilter)
};


#endif // JUCER_MISCUTILITIES_H_INCLUDED
