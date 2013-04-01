/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

class FileChooserDialogBox::ContentComponent  : public Component
{
public:
    //==============================================================================
    ContentComponent (const String& name, const String& desc, FileBrowserComponent& chooser)
        : Component (name),
          chooserComponent (chooser),
          okButton (chooser.getActionVerb()),
          cancelButton (TRANS ("Cancel")),
          newFolderButton (TRANS ("New Folder")),
          instructions (desc)
    {
        addAndMakeVisible (&chooserComponent);

        addAndMakeVisible (&okButton);
        okButton.addShortcut (KeyPress (KeyPress::returnKey));

        addAndMakeVisible (&cancelButton);
        cancelButton.addShortcut (KeyPress (KeyPress::escapeKey));

        addChildComponent (&newFolderButton);

        setInterceptsMouseClicks (false, true);
    }

    void paint (Graphics& g)
    {
        g.setColour (getLookAndFeel().findColour (FileChooserDialogBox::titleTextColourId));
        text.draw (g);
    }

    void resized()
    {
        const int buttonHeight = 26;

        Rectangle<int> area (getLocalBounds());

        getLookAndFeel().createFileChooserHeaderText (getName(), instructions, text, getWidth());
        const Rectangle<float> bb (text.getBoundingBox (0, text.getNumGlyphs(), false));
        area.removeFromTop (roundToInt (bb.getBottom()) + 10);

        chooserComponent.setBounds (area.removeFromTop (area.getHeight() - buttonHeight - 20));
        Rectangle<int> buttonArea (area.reduced (16, 10));

        okButton.changeWidthToFitText (buttonHeight);
        okButton.setBounds (buttonArea.removeFromRight (okButton.getWidth() + 16));

        buttonArea.removeFromRight (16);

        cancelButton.changeWidthToFitText (buttonHeight);
        cancelButton.setBounds (buttonArea.removeFromRight (cancelButton.getWidth()));

        newFolderButton.changeWidthToFitText (buttonHeight);
        newFolderButton.setBounds (buttonArea.removeFromLeft (newFolderButton.getWidth()));
    }

    FileBrowserComponent& chooserComponent;
    TextButton okButton, cancelButton, newFolderButton;

private:
    String instructions;
    GlyphArrangement text;
};

//==============================================================================
FileChooserDialogBox::FileChooserDialogBox (const String& name,
                                            const String& instructions,
                                            FileBrowserComponent& chooserComponent,
                                            const bool warnAboutOverwritingExistingFiles_,
                                            const Colour& backgroundColour)
    : ResizableWindow (name, backgroundColour, true),
      warnAboutOverwritingExistingFiles (warnAboutOverwritingExistingFiles_)
{
    content = new ContentComponent (name, instructions, chooserComponent);
    setContentOwned (content, false);

    setResizable (true, true);
    setResizeLimits (300, 300, 1200, 1000);

    content->okButton.addListener (this);
    content->cancelButton.addListener (this);
    content->newFolderButton.addListener (this);
    content->chooserComponent.addListener (this);

    FileChooserDialogBox::selectionChanged();
}

FileChooserDialogBox::~FileChooserDialogBox()
{
    content->chooserComponent.removeListener (this);
}

//==============================================================================
#if JUCE_MODAL_LOOPS_PERMITTED
bool FileChooserDialogBox::show (int w, int h)
{
    return showAt (-1, -1, w, h);
}

bool FileChooserDialogBox::showAt (int x, int y, int w, int h)
{
    if (w <= 0)
    {
        Component* const previewComp = content->chooserComponent.getPreviewComponent();

        if (previewComp != nullptr)
            w = 400 + previewComp->getWidth();
        else
            w = 600;
    }

    if (h <= 0)
        h = 500;

    if (x < 0 || y < 0)
        centreWithSize (w, h);
    else
        setBounds (x, y, w, h);

    const bool ok = (runModalLoop() != 0);
    setVisible (false);
    return ok;
}
#endif

void FileChooserDialogBox::centreWithDefaultSize (Component* componentToCentreAround)
{
    Component* const previewComp = content->chooserComponent.getPreviewComponent();

    centreAroundComponent (componentToCentreAround,
                           previewComp != nullptr ? 400 + previewComp->getWidth() : 600,
                           500);
}

//==============================================================================
void FileChooserDialogBox::buttonClicked (Button* button)
{
    if (button == &(content->okButton))
    {
        okButtonPressed();
    }
    else if (button == &(content->cancelButton))
    {
        closeButtonPressed();
    }
    else if (button == &(content->newFolderButton))
    {
        createNewFolder();
    }
}

void FileChooserDialogBox::closeButtonPressed()
{
    setVisible (false);
}

void FileChooserDialogBox::selectionChanged()
{
    content->okButton.setEnabled (content->chooserComponent.currentFileIsValid());

    content->newFolderButton.setVisible (content->chooserComponent.isSaveMode()
                                          && content->chooserComponent.getRoot().isDirectory());
}

void FileChooserDialogBox::fileDoubleClicked (const File&)
{
    selectionChanged();
    content->okButton.triggerClick();
}

void FileChooserDialogBox::fileClicked (const File&, const MouseEvent&) {}
void FileChooserDialogBox::browserRootChanged (const File&) {}

void FileChooserDialogBox::okToOverwriteFileCallback (int result, FileChooserDialogBox* box)
{
    if (result != 0 && box != nullptr)
        box->exitModalState (1);
}

void FileChooserDialogBox::okButtonPressed()
{
    if (warnAboutOverwritingExistingFiles
         && content->chooserComponent.isSaveMode()
         && content->chooserComponent.getSelectedFile(0).exists())
    {
        AlertWindow::showOkCancelBox (AlertWindow::WarningIcon,
                                         TRANS("File already exists"),
                                         TRANS("There's already a file called:")
                                           + "\n\n" + content->chooserComponent.getSelectedFile(0).getFullPathName()
                                           + "\n\n" + TRANS("Are you sure you want to overwrite it?"),
                                         TRANS("overwrite"),
                                         TRANS("cancel"),
                                      this,
                                      ModalCallbackFunction::forComponent (okToOverwriteFileCallback, this));
    }
    else
    {
        exitModalState (1);
    }
}

void FileChooserDialogBox::createNewFolderCallback (int result, FileChooserDialogBox* box,
                                                    Component::SafePointer<AlertWindow> alert)
{
    if (result != 0 && alert != nullptr && box != nullptr)
    {
        alert->setVisible (false);
        box->createNewFolderConfirmed (alert->getTextEditorContents ("Folder Name"));
    }
}

void FileChooserDialogBox::createNewFolder()
{
    File parent (content->chooserComponent.getRoot());

    if (parent.isDirectory())
    {
        AlertWindow* aw = new AlertWindow (TRANS("New Folder"),
                                           TRANS("Please enter the name for the folder"),
                                           AlertWindow::NoIcon, this);

        aw->addTextEditor ("Folder Name", String::empty, String::empty, false);
        aw->addButton (TRANS("Create Folder"), 1, KeyPress (KeyPress::returnKey));
        aw->addButton (TRANS("Cancel"),        0, KeyPress (KeyPress::escapeKey));

        aw->enterModalState (true,
                             ModalCallbackFunction::forComponent (createNewFolderCallback, this,
                                                                  Component::SafePointer<AlertWindow> (aw)),
                             true);
    }
}

void FileChooserDialogBox::createNewFolderConfirmed (const String& nameFromDialog)
{
    const String name (File::createLegalFileName (nameFromDialog));

    if (! name.isEmpty())
    {
        const File parent (content->chooserComponent.getRoot());

        if (! parent.getChildFile (name).createDirectory())
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              TRANS ("New Folder"),
                                              TRANS ("Couldn't create the folder!"));
        }

        content->chooserComponent.refresh();
    }
}
