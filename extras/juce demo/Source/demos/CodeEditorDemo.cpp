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
    {
        setName ("Code Editor");
        setOpaque (true);

        // Create the editor..
        addAndMakeVisible (editor = new CodeEditorComponent (codeDocument, &cppTokeniser));

        // Create a file chooser control to load files into it..
        addAndMakeVisible (fileChooser = new FilenameComponent ("File", File::nonexistent, true, false, false,
                                                                "*.cpp;*.h;*.hpp;*.c;*.mm;*.m", String::empty,
                                                                "Choose a C++ file to open it in the editor"));
        fileChooser->addListener (this);


        editor->loadContent ("\n\n/* Code editor demo! Please be gentle, this component is still an alpha version! */\n\n");
    }

    ~CodeEditorDemo()
    {
        deleteAllChildren();
    }

    void filenameComponentChanged (FilenameComponent* fileComponentThatHasChanged)
    {
        File f (fileChooser->getCurrentFile());
        editor->loadContent (f.loadFileAsString());
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::lightgrey);
    }

    void resized()
    {
        editor->setBounds (10, 45, getWidth() - 20, getHeight() - 55);
        fileChooser->setBounds (10, 10, getWidth() - 20, 25);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    // this is the document that the editor component is showing
    CodeDocument codeDocument;

    // this is a tokeniser to do the c++ syntax highlighting
    CPlusPlusCodeTokeniser cppTokeniser;

    // the editor component
    CodeEditorComponent* editor;

    FilenameComponent* fileChooser;
};


//==============================================================================
Component* createCodeEditorDemo()
{
    return new CodeEditorDemo();
}
