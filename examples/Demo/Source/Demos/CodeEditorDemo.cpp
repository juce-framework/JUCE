/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-12 by Raw Material Software Ltd.

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

#include "../JuceDemoHeader.h"


//==============================================================================
class CodeEditorDemo  : public Component,
                        private FilenameComponentListener
{
public:
    CodeEditorDemo()
        : fileChooser ("File", File::nonexistent, true, false, false,
                       "*.cpp;*.h;*.hpp;*.c;*.mm;*.m", String::empty,
                       "Choose a C++ file to open it in the editor")
    {
        setOpaque (true);

        // Create the editor..
        addAndMakeVisible (editor = new CodeEditorComponent (codeDocument, &cppTokeniser));

        editor->loadContent ("\n"
                             "/* Code editor demo!\n"
                             "\n"
                             "   To see a real-world example of the code editor\n"
                             "   in action, try the Introjucer!\n"
                             "\n"
                             "*/\n"
                             "\n");

        // Create a file chooser control to load files into it..
        addAndMakeVisible (fileChooser);
        fileChooser.addListener (this);
    }

    ~CodeEditorDemo()
    {
        fileChooser.removeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::lightgrey);
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CodeEditorDemo);
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<CodeEditorDemo> demo ("10 Components: Code Editor");
