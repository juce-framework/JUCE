/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             MDIDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Displays and edits MDI files.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        MDIDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
/** The Note class contains text editor used to display and edit the note's contents and will
    also listen to changes in the text and mark the FileBasedDocument as 'dirty'. This 'dirty'
    flag is used to prompt the user to save the note when it is closed.
 */
class Note final : public Component,
                   public FileBasedDocument
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
        editor.onTextChange = [this] { changed(); };
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
        if (file.replaceWithText (editor.getText()))
            return Result::ok();

        return Result::fail ("Can't write to file");
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

    File getSuggestedSaveAsFile (const File&) override
    {
        return File::getSpecialLocation (File::userDesktopDirectory)
                    .getChildFile (getName())
                    .withFileExtension ("jnote");
    }

private:
    Value textValueObject;
    TextEditor editor;

    void lookAndFeelChanged() override
    {
        editor.applyFontToAllText (editor.getFont());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Note)
};


//==============================================================================
/** Simple MultiDocumentPanel that just tries to save our notes when they are closed.
 */
class DemoMultiDocumentPanel final : public MultiDocumentPanel
{
public:
    DemoMultiDocumentPanel() = default;

    void tryToCloseDocumentAsync (Component* component, std::function<void (bool)> callback) override
    {
        if (auto* note = dynamic_cast<Note*> (component))
        {
            SafePointer<DemoMultiDocumentPanel> parent { this };
            note->saveIfNeededAndUserAgreesAsync ([parent, callback] (FileBasedDocument::SaveResult result)
            {
                if (parent != nullptr)
                    callback (result == FileBasedDocument::savedOk);
            });
        }
    }

    void activeDocumentChanged() override
    {
        if (auto* activeDoc = getActiveDocument())
            Logger::outputDebugString ("activeDocumentChanged() to " + activeDoc->getName());
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoMultiDocumentPanel)
};

//==============================================================================
/** Simple multi-document panel that manages a number of notes that you can store to files.
    By default this will look for notes saved to the desktop and load them up.
 */
class MDIDemo final : public Component,
                      public FileDragAndDropTarget
{
public:
    MDIDemo()
    {
        setOpaque (true);

        showInTabsButton.setToggleState (false, dontSendNotification);
        showInTabsButton.onClick = [this] { updateLayoutMode(); };
        addAndMakeVisible (showInTabsButton);

        oneDocShouldBeFullscreenButton.onClick = [this]
        {
            multiDocumentPanel.useFullscreenWhenOneDocument (oneDocShouldBeFullscreenButton.getToggleState());
        };
        addAndMakeVisible (oneDocShouldBeFullscreenButton);
        oneDocShouldBeFullscreenButton.setToggleState (false, juce::sendNotification);

        addNoteButton.onClick = [this]
        {
            addNote ("Note " + String (noteCounter), "Hello World! " + String (noteCounter));
            ++noteCounter;
        };
        addAndMakeVisible (addNoteButton);

        closeActiveDocumentButton.onClick = [this]
        {
            multiDocumentPanel.closeDocumentAsync (multiDocumentPanel.getActiveDocument(), false, [] (auto) {});
        };
        addAndMakeVisible (closeActiveDocumentButton);

        closeApplicationButton.onClick = [this]
        {
            multiDocumentPanel.closeAllDocumentsAsync (true, [] (bool allSaved)
            {
                if (allSaved)
                    JUCEApplicationBase::quit();
            });
        };
        addAndMakeVisible (closeApplicationButton);

        addAndMakeVisible (multiDocumentPanel);
        multiDocumentPanel.setBackgroundColour (Colours::transparentBlack);

        updateLayoutMode();
        addNote ("Notes Demo", "You can drag-and-drop text files onto this page to open them as notes..");
        addExistingNotes();

        setSize (650, 500);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto area = getLocalBounds();

        auto topButtonRow = area.removeFromTop (28).reduced (2);

        showInTabsButton              .setBounds (topButtonRow.removeFromLeft (150));

        closeApplicationButton        .setBounds (topButtonRow.removeFromRight (150));
        addNoteButton                 .setBounds (topButtonRow.removeFromRight (150));
        closeActiveDocumentButton     .setBounds (topButtonRow.removeFromRight (150));

        oneDocShouldBeFullscreenButton.setBounds (area.removeFromTop (28).reduced (2).removeFromLeft (240));

        multiDocumentPanel.setBounds (area);
    }

    bool isInterestedInFileDrag (const StringArray&) override
    {
        return true;
    }

    void filesDropped (const StringArray& filenames, int /*x*/, int /*y*/) override
    {
        Array<File> files;

        for (auto& f : filenames)
            files.add ({ f });

        createNotesForFiles (files);
    }

    void createNotesForFiles (const Array<File>& files)
    {
        for (auto& file : files)
        {
            auto content = file.loadFileAsString();

            if (content.length() > 20000)
                content = "Too long!";

            addNote (file.getFileName(), content);
        }
    }

private:
    void updateLayoutMode()
    {
        multiDocumentPanel.setLayoutMode (showInTabsButton.getToggleState() ? MultiDocumentPanel::MaximisedWindowsWithTabs
                                                                            : MultiDocumentPanel::FloatingWindows);
    }

    void addNote (const String& name, const String& content)
    {
        auto* newNote = new Note (name, content);
        newNote->setSize (200, 200);

        multiDocumentPanel.addDocument (newNote, Colours::lightblue.withAlpha (0.6f), true);
    }

    void addExistingNotes()
    {
        Array<File> files;
        File::getSpecialLocation (File::userDesktopDirectory).findChildFiles (files, File::findFiles, false, "*.jnote");
        createNotesForFiles (files);
    }

    ToggleButton showInTabsButton               { "Show with tabs" };
    ToggleButton oneDocShouldBeFullscreenButton { "Fill screen when only one note is open" };
    TextButton   addNoteButton                  { "Create a new note" },
                 closeApplicationButton         { "Close app" },
                 closeActiveDocumentButton      { "Close active document" };

    DemoMultiDocumentPanel multiDocumentPanel;
    int noteCounter = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MDIDemo)
};
