/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
const char* getPreferredLineFeed();
String joinLinesIntoSourceFile (StringArray& lines);
String replaceLineFeeds (const String& content, const String& lineFeed);
String getLineFeedForFile (const String& fileContent);

var parseJUCEHeaderMetadata (const File&);

String trimCommentCharsFromStartOfLine (const String& line);

String createAlphaNumericUID();
String createGUID (const String& seed); // Turns a seed into a windows GUID

String escapeSpaces (const String& text); // replaces spaces with blackslash-space
String escapeQuotesAndSpaces (const String& text);
String addQuotesIfContainsSpaces (const String& text);

StringPairArray parsePreprocessorDefs (const String& defs);
StringPairArray mergePreprocessorDefs (StringPairArray inheritedDefs, const StringPairArray& overridingDefs);
String createGCCPreprocessorFlags (const StringPairArray& defs);

StringArray getCleanedStringArray (StringArray);
StringArray getSearchPathsFromString (const String& searchPath);
StringArray getCommaOrWhitespaceSeparatedItems (const String&);

void setValueIfVoid (Value value, const var& defaultValue);

bool fileNeedsCppSyntaxHighlighting (const File& file);

void writeAutoGenWarningComment (OutputStream& outStream);

StringArray getJUCEModules() noexcept;
bool isJUCEModule (const String& moduleID) noexcept;

StringArray getModulesRequiredForConsole() noexcept;
StringArray getModulesRequiredForComponent() noexcept;
StringArray getModulesRequiredForAudioProcessor() noexcept;

bool isPIPFile (const File&) noexcept;
int findBestLineToScrollToForClass (StringArray, const String&, bool isPlugin = false);

bool isValidJUCEExamplesDirectory (const File&) noexcept;

bool isJUCEModulesFolder (const File&);
bool isJUCEFolder (const File&);

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

    void addSearchPathProperty (const Value& value,
                                const String& name,
                                const String& mainHelpText)
    {
        add (new TextPropertyComponent (value, name, 16384, true),
             mainHelpText + " Use semi-colons or new-lines to separate multiple paths.");
    }

    void addSearchPathProperty (const ValueTreePropertyWithDefault& value,
                                const String& name,
                                const String& mainHelpText)
    {
        add (new TextPropertyComponent (value, name, 16384, true),
             mainHelpText + " Use semi-colons or new-lines to separate multiple paths.");
    }

    void setPreferredHeight (int height)
    {
        for (int j = components.size(); --j >= 0;)
            components.getUnchecked (j)->setPreferredHeight (height);
    }

    Array<PropertyComponent*> components;
};

//==============================================================================
// A ValueSource which takes an input source, and forwards any changes in it.
// This class is a handy way to create sources which re-map a value.
class ValueSourceFilter : public Value::ValueSource,
                          private Value::Listener
{
public:
    ValueSourceFilter (const Value& source)  : sourceValue (source)
    {
        sourceValue.addListener (this);
    }

protected:
    Value sourceValue;

private:
    void valueChanged (Value&) override      { sendChangeMessage (true); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueSourceFilter)
};
