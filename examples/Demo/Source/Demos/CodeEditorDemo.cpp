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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CodeEditorDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<CodeEditorDemo> demo ("10 Components: Code Editor");
