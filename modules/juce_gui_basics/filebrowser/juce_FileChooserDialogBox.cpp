/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class FileChooserDialogBox::ContentComponent  : public Component
{
public:
    ContentComponent (const String& name, const String& desc, FileBrowserComponent& chooser)
        : Component (name),
          chooserComponent (chooser),
          okButton (chooser.getActionVerb()),
          cancelButton (TRANS ("Cancel")),
          newFolderButton (TRANS ("New Folder")),
          instructions (desc)
    {
        addAndMakeVisible (chooserComponent);

        addAndMakeVisible (okButton);
        okButton.addShortcut (KeyPress (KeyPress::returnKey));

        addAndMakeVisible (cancelButton);
        cancelButton.addShortcut (KeyPress (KeyPress::escapeKey));

        addChildComponent (newFolderButton);

        setInterceptsMouseClicks (false, true);
    }

    void paint (Graphics& g) override
    {
        text.draw (g, getLocalBounds().reduced (6)
                        .removeFromTop ((int) text.getHeight()).toFloat());
    }

    void resized() override
    {
        const int buttonHeight = 26;

        auto area = getLocalBounds();

        text.createLayout (getLookAndFeel().createFileChooserHeaderText (getName(), instructions),
                           (float) getWidth() - 12.0f);

        area.removeFromTop (roundToInt (text.getHeight()) + 10);

        chooserComponent.setBounds (area.removeFromTop (area.getHeight() - buttonHeight - 20));
        auto buttonArea = area.reduced (16, 10);

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
    String instructions;
    TextLayout text;
};

//==============================================================================
FileChooserDialogBox::FileChooserDialogBox (const String& name,
                                            const String& instructions,
                                            FileBrowserComponent& chooserComponent,
                                            bool shouldWarn,
                                            Colour backgroundColour,
                                            Component* parentComp)
    : ResizableWindow (name, backgroundColour, parentComp == nullptr),
      warnAboutOverwritingExistingFiles (shouldWarn)
{
    content = new ContentComponent (name, instructions, chooserComponent);
    setContentOwned (content, false);

    setResizable (true, true);
    setResizeLimits (300, 300, 1200, 1000);

    content->okButton.onClick        = [this] { okButtonPressed(); };
    content->cancelButton.onClick    = [this] { closeButtonPressed(); };
    content->newFolderButton.onClick = [this] { createNewFolder(); };

    content->chooserComponent.addListener (this);

    FileChooserDialogBox::selectionChanged();

    if (parentComp != nullptr)
        parentComp->addAndMakeVisible (this);
    else
        setAlwaysOnTop (WindowUtils::areThereAnyAlwaysOnTopWindows());
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
    if (w <= 0)  w = getDefaultWidth();
    if (h <= 0)  h = 500;

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
    centreAroundComponent (componentToCentreAround, getDefaultWidth(), 500);
}

int FileChooserDialogBox::getDefaultWidth() const
{
    if (auto* previewComp = content->chooserComponent.getPreviewComponent())
        return 400 + previewComp->getWidth();

    return 600;
}

//==============================================================================
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

void FileChooserDialogBox::okButtonPressed()
{
    if (warnAboutOverwritingExistingFiles
         && content->chooserComponent.isSaveMode()
         && content->chooserComponent.getSelectedFile(0).exists())
    {
        auto options = MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType::WarningIcon,
                                                               TRANS ("File already exists"),
                                                               TRANS ("There's already a file called: FLNM")
                                                                  .replace ("FLNM", content->chooserComponent.getSelectedFile(0).getFullPathName())
                                                                 + "\n\n"
                                                                 + TRANS ("Are you sure you want to overwrite it?"),
                                                               TRANS ("Overwrite"),
                                                               TRANS ("Cancel"),
                                                               this);
        messageBox = AlertWindow::showScopedAsync (options, [this] (int result)
        {
            if (result != 0)
                exitModalState (1);
        });
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
    auto parent = content->chooserComponent.getRoot();

    if (parent.isDirectory())
    {
        auto* aw = new AlertWindow (TRANS("New Folder"),
                                    TRANS("Please enter the name for the folder"),
                                    MessageBoxIconType::NoIcon, this);

        aw->addTextEditor ("Folder Name", String(), String(), false);
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
    auto name = File::createLegalFileName (nameFromDialog);

    if (! name.isEmpty())
    {
        auto parent = content->chooserComponent.getRoot();

        if (! parent.getChildFile (name).createDirectory())
        {
            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                             TRANS ("New Folder"),
                                                             TRANS ("Couldn't create the folder!"));
            messageBox = AlertWindow::showScopedAsync (options, nullptr);
        }

        content->chooserComponent.refresh();
    }
}

} // namespace juce
