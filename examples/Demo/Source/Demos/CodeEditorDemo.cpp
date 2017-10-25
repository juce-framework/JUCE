/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../JuceDemoHeader.h"


//==============================================================================
class CodeEditorDemo  : public Component,
                        private FilenameComponentListener
{
public:
    CodeEditorDemo()
        : fileChooser ("File", File(), true, false, false,
                       "*.cpp;*.h;*.hpp;*.c;*.mm;*.m", String(),
                       "Choose a C++ file to open it in the editor")
    {
        setOpaque (true);

        // Create the editor..
        addAndMakeVisible (editor = new CodeEditorComponent (codeDocument, &cppTokeniser));

        editor->loadContent ("\n"
                             "/* Code editor demo!\n"
                             "\n"
                             "   To see a real-world example of the code editor\n"
                             "   in action, have a look at the Projucer!\n"
                             "\n"
                             "*/\n"
                             "\n");

        // Create a file chooser control to load files into it..
        addAndMakeVisible (fileChooser);
        fileChooser.addListener (this);

        lookAndFeelChanged();
    }

    ~CodeEditorDemo()
    {
        fileChooser.removeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                           Colours::lightgrey));
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (8));

        fileChooser.setBounds (r.removeFromTop (25));
        editor->setBounds (r.withTrimmedTop (8));
    }

private:
    // this is the document that the editor component is showing
    CodeDocument codeDocument;

    // this is a tokeniser to apply the C++ syntax highlighting
    CPlusPlusCodeTokeniser cppTokeniser;

    // the editor component
    ScopedPointer<CodeEditorComponent> editor;

    FilenameComponent fileChooser;

    void filenameComponentChanged (FilenameComponent*) override
    {
        editor->loadContent (fileChooser.getCurrentFile().loadFileAsString());
    }

    void lookAndFeelChanged() override
    {
        if (auto* v4 = dynamic_cast<LookAndFeel_V4*> (&LookAndFeel::getDefaultLookAndFeel()))
        {
            auto useLight = v4->getCurrentColourScheme() == LookAndFeel_V4::getLightColourScheme();
            editor->setColourScheme (useLight ? getLightCodeEditorColourScheme()
                                              : getDarkCodeEditorColourScheme());
        }
        else
        {
            editor->setColourScheme (cppTokeniser.getDefaultColourScheme());
        }
    }

    CodeEditorComponent::ColourScheme getDarkCodeEditorColourScheme()
    {
        struct Type
        {
            const char* name;
            juce::uint32 colour;
        };

        const Type types[] =
        {
            { "Error",              0xffe60000 },
            { "Comment",            0xff72d20c },
            { "Keyword",            0xffee6f6f },
            { "Operator",           0xffc4eb19 },
            { "Identifier",         0xffcfcfcf },
            { "Integer",            0xff42c8c4 },
            { "Float",              0xff885500 },
            { "String",             0xffbc45dd },
            { "Bracket",            0xff058202 },
            { "Punctuation",        0xffcfbeff },
            { "Preprocessor Text",  0xfff8f631 }
        };

        CodeEditorComponent::ColourScheme cs;

        for (auto& t : types)
            cs.set (t.name, Colour (t.colour));

        return cs;
    }

    CodeEditorComponent::ColourScheme getLightCodeEditorColourScheme()
    {
        struct Type
        {
            const char* name;
            juce::uint32 colour;
        };

        const Type types[] =
        {
            { "Error",              0xffcc0000 },
            { "Comment",            0xff00aa00 },
            { "Keyword",            0xff0000cc },
            { "Operator",           0xff225500 },
            { "Identifier",         0xff000000 },
            { "Integer",            0xff880000 },
            { "Float",              0xff885500 },
            { "String",             0xff990099 },
            { "Bracket",            0xff000055 },
            { "Punctuation",        0xff004400 },
            { "Preprocessor Text",  0xff660000 }
        };

        CodeEditorComponent::ColourScheme cs;

        for (auto& t : types)
            cs.set (t.name, Colour (t.colour));

        return cs;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CodeEditorDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<CodeEditorDemo> demo ("10 Components: Code Editor");
