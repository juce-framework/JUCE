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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_FileChooserDialogBox.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../../../text/juce_LocalisedStrings.h"
#include "../windows/juce_AlertWindow.h"


//==============================================================================
FileChooserDialogBox::FileChooserDialogBox (const String& name,
                                            const String& instructions,
                                            FileBrowserComponent& chooserComponent,
                                            const bool warnAboutOverwritingExistingFiles_,
                                            const Colour& backgroundColour)
    : ResizableWindow (name, backgroundColour, true),
      warnAboutOverwritingExistingFiles (warnAboutOverwritingExistingFiles_)
{
    content = new ContentComponent();
    content->setName (name);
    content->instructions = instructions;
    content->chooserComponent = &chooserComponent;

    content->addAndMakeVisible (&chooserComponent);

    content->okButton = new TextButton (chooserComponent.getActionVerb());
    content->addAndMakeVisible (content->okButton);
    content->okButton->addButtonListener (this);
    content->okButton->setEnabled (chooserComponent.currentFileIsValid());
    content->okButton->addShortcut (KeyPress (KeyPress::returnKey, 0, 0));

    content->cancelButton = new TextButton (TRANS("Cancel"));
    content->addAndMakeVisible (content->cancelButton);
    content->cancelButton->addButtonListener (this);
    content->cancelButton->addShortcut (KeyPress (KeyPress::escapeKey, 0, 0));

    setContentComponent (content);

    setResizable (true, true);
    setResizeLimits (300, 300, 1200, 1000);

    content->chooserComponent->addListener (this);
}

FileChooserDialogBox::~FileChooserDialogBox()
{
    content->chooserComponent->removeListener (this);
}

//==============================================================================
bool FileChooserDialogBox::show (int w, int h)
{
    if (w <= 0)
    {
        Component* const previewComp = content->chooserComponent->getPreviewComponent();
        if (previewComp != 0)
            w = 400 + previewComp->getWidth();
        else
            w = 600;
    }

    if (h <= 0)
        h = 500;

    centreWithSize (w, h);

    const bool ok = (runModalLoop() != 0);
    setVisible (false);
    return ok;
}

//==============================================================================
void FileChooserDialogBox::buttonClicked (Button* button)
{
    if (button == content->okButton)
    {
        if (warnAboutOverwritingExistingFiles
             && content->chooserComponent->isSaveMode()
             && content->chooserComponent->getSelectedFile(0).exists())
        {
            if (! AlertWindow::showOkCancelBox (AlertWindow::WarningIcon,
                                                TRANS("File already exists"),
                                                TRANS("There's already a file called:\n\n")
                                                + content->chooserComponent->getSelectedFile(0).getFullPathName()
                                                + T("\n\nAre you sure you want to overwrite it?"),
                                                TRANS("overwrite"),
                                                TRANS("cancel")))
            {
                return;
            }
        }

        exitModalState (1);
    }
    else if (button == content->cancelButton)
        closeButtonPressed();
}

void FileChooserDialogBox::closeButtonPressed()
{
    setVisible (false);
}

void FileChooserDialogBox::selectionChanged()
{
    content->okButton->setEnabled (content->chooserComponent->currentFileIsValid());
}

void FileChooserDialogBox::fileClicked (const File&, const MouseEvent&)
{
}

void FileChooserDialogBox::fileDoubleClicked (const File&)
{
    selectionChanged();
    content->okButton->triggerClick();
}

//==============================================================================
FileChooserDialogBox::ContentComponent::ContentComponent()
{
    setInterceptsMouseClicks (false, true);
}

FileChooserDialogBox::ContentComponent::~ContentComponent()
{
    delete okButton;
    delete cancelButton;
}

void FileChooserDialogBox::ContentComponent::paint (Graphics& g)
{
    g.setColour (Colours::black);
    text.draw (g);
}

void FileChooserDialogBox::ContentComponent::resized()
{
    getLookAndFeel().createFileChooserHeaderText (getName(), instructions, text, getWidth());

    float left, top, right, bottom;
    text.getBoundingBox (0, text.getNumGlyphs(), left, top, right, bottom, false);

    const int y = roundToInt (bottom) + 10;
    const int buttonHeight = 26;
    const int buttonY = getHeight() - buttonHeight - 8;

    chooserComponent->setBounds (0, y, getWidth(), buttonY - y - 20);

    okButton->setBounds (proportionOfWidth (0.25f), buttonY,
                         proportionOfWidth (0.2f), buttonHeight);

    cancelButton->setBounds (proportionOfWidth (0.55f), buttonY,
                             proportionOfWidth (0.2f), buttonHeight);
}



END_JUCE_NAMESPACE
