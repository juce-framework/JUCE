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

#include "../jucedemo_headers.h"


//==============================================================================
class CodeEditorDemo   : public Component,
                         public FilenameComponentListener
{
public:
    //==============================================================================
    CodeEditorDemo()
        : fileChooser ("File", File::nonexistent, true, false, false,
                       "*.cpp;*.h;*.hpp;*.c;*.mm;*.m", String::empty,
                       "Choose a C++ file to open it in the editor")
    {
        setName ("Code Editor");
        setOpaque (true);

        // Create the editor..
        addAndMakeVisible (editor = new CodeEditorComponent (codeDocument, &cppTokeniser));
        editor->loadContent ("\n\n/* Code editor demo! To see a real-world example of the "
                             "code editor in action, try the Introjucer! */\n\n");

        // Create a file chooser control to load files into it..
        addAndMakeVisible (&fileChooser);
        fileChooser.addListener (this);
    }

    ~CodeEditorDemo()
    {
    }

    void filenameComponentChanged (FilenameComponent*)
    {
        editor->loadContent (fileChooser.getCurrentFile().loadFileAsString());
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::lightgrey);
    }

    void resized()
    {
        editor->setBounds (10, 45, getWidth() - 20, getHeight() - 55);
        fileChooser.setBounds (10, 10, getWidth() - 20, 25);
    }

private:
    // this is the document that the editor component is showing
    CodeDocument codeDocument;

    // this is a tokeniser to do the c++ syntax highlighting
    CPlusPlusCodeTokeniser cppTokeniser;

    // the editor component
    ScopedPointer<CodeEditorComponent> editor;

    FilenameComponent fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CodeEditorDemo)
};


//==============================================================================
Component* createCodeEditorDemo()
{
    return new CodeEditorDemo();
}
