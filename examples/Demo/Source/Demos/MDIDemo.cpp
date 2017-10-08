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
/** The Note class contains text editor used to display and edit the note's contents and will
    also listen to changes in the text and mark the FileBasedDocument as 'dirty'. This 'dirty'
    flag is used to promt the user to save the note when it is closed.
 */
class Note    : public Component,
                public FileBasedDocument,
                private TextEditor::Listener
{
public:
    Note (const String& name, const String& contents)
        : FileBasedDocument (".jnote", "*.jnote",
                             "Browse for note to load",
                             "Choose file to save note to"),
          textValueObject (contents)
    {
        // we need to use an separate Value object as our text source so it doesn't get marked
        // as changed immediately
        setName (name);

        editor.setMultiLine (true);
        editor.setReturnKeyStartsNewLine (true);
        editor.getTextValue().referTo (textValueObject);
        addAndMakeVisible (editor);
        editor.addListener (this);
    }

    ~Note()
    {
        editor.removeListener (this);
    }

    void resized() override
    {
        editor.setBounds (getLocalBounds());
    }

    String getDocumentTitle() override
    {
        return getName();
    }

    Result loadDocument (const File& file) override
    {
        editor.setText (file.loadFileAsString());
        return Result::ok();
    }

    Result saveDocument (const File& file) override
    {
        // attempt to save the contents into the given file
        FileOutputStream os (file);

        if (os.openedOk())
            os.writeText (editor.getText(), false, false);

        return Result::ok();
    }

    File getLastDocumentOpened() override
    {
        // not interested in this for now
        return {};
    }

    void setLastDocumentOpened (const File& /*file*/) override
    {
        // not interested in this for now
    }

   #if JUCE_MODAL_LOOPS_PERMITTED
    File getSuggestedSaveAsFile (const File&) override
    {
        return File::getSpecialLocation (File::userDesktopDirectory).getChildFile (getName()).withFileExtension ("jnote");
    }
   #endif

private:
    Value textValueObject;
    TextEditor editor;

    void textEditorTextChanged (TextEditor& ed) override
    {
        // let our FileBasedDocument know we've changed
        if (&ed == &editor)
            changed();
    }

    void lookAndFeelChanged() override
    {
        editor.applyFontToAllText (editor.getFont());
    }

    void textEditorReturnKeyPressed (TextEditor&) override {}
    void textEditorEscapeKeyPressed (TextEditor&) override {}
    void textEditorFocusLost (TextEditor&) override {}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Note)
};


//==============================================================================
/** Simple MultiDocumentPanel that just tries to save our notes when they are closed.
 */
class DemoMultiDocumentPanel    : public MultiDocumentPanel
{
public:
    DemoMultiDocumentPanel()
    {
    }

    ~DemoMultiDocumentPanel()
    {
        closeAllDocuments (true);
    }

    bool tryToCloseDocument (Component* component) override
    {
       #if JUCE_MODAL_LOOPS_PERMITTED
        if (Note* note = dynamic_cast<Note*> (component))
            return note->saveIfNeededAndUserAgrees() != FileBasedDocument::failedToWriteToFile;
       #else
        ignoreUnused (component);
       #endif

        return true;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoMultiDocumentPanel)
};

//==============================================================================
/** Simple multi-document panel that manages a number of notes that you can store to files.
    By default this will look for notes saved to the desktop and load them up.
 */
class MDIDemo   : public Component,
                  public FileDragAndDropTarget,
                  private Button::Listener
{
public:
    MDIDemo()
    {
        setOpaque (true);

        showInTabsButton.setButtonText ("Show with tabs");
        showInTabsButton.setToggleState (false, dontSendNotification);
        showInTabsButton.addListener (this);
        addAndMakeVisible (showInTabsButton);

        addNoteButton.setButtonText ("Create a new note");
        addNoteButton.addListener (this);
        addAndMakeVisible (addNoteButton);

        addAndMakeVisible (multiDocumentPanel);
        multiDocumentPanel.setBackgroundColour (Colours::transparentBlack);

        updateLayoutMode();
        addNote ("Notes Demo", "You can drag-and-drop text files onto this page to open them as notes..");
        addExistingNotes();
    }

    ~MDIDemo()
    {
        addNoteButton.removeListener (this);
        showInTabsButton.removeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds());
        Rectangle<int> buttonArea (area.removeFromTop (28).reduced (2));
        addNoteButton.setBounds (buttonArea.removeFromRight (150));
        showInTabsButton.setBounds (buttonArea);

        multiDocumentPanel.setBounds (area);
    }

    bool isInterestedInFileDrag (const StringArray&) override
    {
        return true;
    }

    void filesDropped (const StringArray& filenames, int /* x */, int /* y */) override
    {
        Array<File> files;

        for (int i = 0; i < filenames.size(); ++i)
            files.add (File (filenames[i]));

        createNotesForFiles (files);
    }

    void createNotesForFiles (const Array<File>& files)
    {
        for (int i = 0; i < files.size(); ++i)
        {
            const File file (files[i]);

            String content = file.loadFileAsString();

            if (content.length() > 20000)
                content = "Too long!";

            addNote (file.getFileName(), content);
        }
    }

private:
    ToggleButton showInTabsButton;
    TextButton addNoteButton;
    DemoMultiDocumentPanel multiDocumentPanel;

    void updateLayoutMode()
    {
        multiDocumentPanel.setLayoutMode (showInTabsButton.getToggleState() ? MultiDocumentPanel::MaximisedWindowsWithTabs
                                                                            : MultiDocumentPanel::FloatingWindows);
    }

    void addNote (const String& name, const String& content)
    {
        Note* newNote = new Note (name, content);
        newNote->setSize (200, 200);

        multiDocumentPanel.addDocument (newNote, Colours::lightblue.withAlpha (0.6f), true);
    }

    void addExistingNotes()
    {
        Array<File> files;
        File::getSpecialLocation (File::userDesktopDirectory).findChildFiles (files, File::findFiles, false, "*.jnote");
        createNotesForFiles (files);
    }

    void buttonClicked (Button* b) override
    {
        if (b == &showInTabsButton)
            updateLayoutMode();
        else if (b == &addNoteButton)
            addNote (String ("Note ") + String (multiDocumentPanel.getNumDocuments() + 1), "Hello World!");
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MDIDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<MDIDemo> demo ("10 Components: MDI");
