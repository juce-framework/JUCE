/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             CodeEditorDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Displays a code editor.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        CodeEditorDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class CodeEditorDemo  : public Component,
                        private FilenameComponentListener
{
public:
    CodeEditorDemo()
    {
        setOpaque (true);

        // Create the editor..
        editor.reset (new CodeEditorComponent (codeDocument, &cppTokeniser));
        addAndMakeVisible (editor.get());

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

        setSize (500, 500);
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
        auto r = getLocalBounds().reduced (8);

        fileChooser.setBounds (r.removeFromTop (25));
        editor->setBounds     (r.withTrimmedTop (8));
    }

private:
    // this is the document that the editor component is showing
    CodeDocument codeDocument;

    // this is a tokeniser to apply the C++ syntax highlighting
    CPlusPlusCodeTokeniser cppTokeniser;

    // the editor component
    std::unique_ptr<CodeEditorComponent> editor;

    FilenameComponent fileChooser { "File", {}, true, false, false, "*.cpp;*.h;*.hpp;*.c;*.mm;*.m", {},
                                    "Choose a C++ file to open it in the editor" };

    //==============================================================================
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
