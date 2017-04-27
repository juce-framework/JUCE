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

#ifndef JUCER_TRANSLATIONTOOLCOMPONENT_H_INCLUDED
#define JUCER_TRANSLATIONTOOLCOMPONENT_H_INCLUDED

#include "jucer_TranslationHelpers.h"

//==============================================================================
class TranslationToolComponent  : public Component,
                                  public ButtonListener
{
public:
    TranslationToolComponent()
        : editorOriginal (documentOriginal, nullptr),
          editorPre (documentPre, nullptr),
          editorPost (documentPost, nullptr),
          editorResult (documentResult, nullptr)
    {
        instructionsLabel.setText (
            "This utility converts translation files to/from a format that can be passed to automatic translation tools."
            "\n\n"
            "First, choose whether to scan the current project for all TRANS() macros, or "
            "pick an existing translation file to load:", dontSendNotification);
        addAndMakeVisible (instructionsLabel);

        label1.setText ("..then copy-and-paste this annotated text into Google Translate or some other translator:", dontSendNotification);
        addAndMakeVisible (label1);

        label2.setText ("...then, take the translated result and paste it into the box below:", dontSendNotification);
        addAndMakeVisible (label2);

        label3.setText ("Finally, click the 'Generate' button, and a translation file will be created below. "
                        "Remember to update its language code at the top!", dontSendNotification);
        addAndMakeVisible (label3);

        label4.setText ("If you load an existing file the already translated strings will be removed. Ensure this box is empty to create a fresh translation", dontSendNotification);
        addAndMakeVisible (label4);

        addAndMakeVisible (editorOriginal);
        addAndMakeVisible (editorPre);
        addAndMakeVisible (editorPost);
        addAndMakeVisible (editorResult);

        generateButton.setButtonText (TRANS("Generate"));
        addAndMakeVisible (generateButton);
        scanProjectButton.setButtonText ("Scan Project for TRANS macros");
        addAndMakeVisible (scanProjectButton);
        scanFolderButton.setButtonText ("Scan Folder for TRANS macros");
        addAndMakeVisible (scanFolderButton);
        loadTranslationButton.setButtonText ("Load existing translation File...");
        addAndMakeVisible (loadTranslationButton);
        generateButton.addListener (this);

        scanProjectButton.addListener (this);
        scanFolderButton.addListener (this);
        loadTranslationButton.addListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (backgroundColourId));
    }

    void resized() override
    {
        const int m          = 6;
        const int textH      = 44;
        const int extraH     = (7 * textH);
        const int editorH    = (getHeight() - extraH) / 4;
        const int numButtons = 3;

        Rectangle<int> r (getLocalBounds().withTrimmedBottom (m));
        const int buttonWidth = r.getWidth() / numButtons;

        instructionsLabel.setBounds (r.removeFromTop (textH * 2).reduced (m));
        r.removeFromTop (m);
        Rectangle<int> r2 (r.removeFromTop (textH - (2 * m)));
        scanProjectButton    .setBounds (r2.removeFromLeft (buttonWidth).reduced (m, 0));
        scanFolderButton     .setBounds (r2.removeFromLeft (buttonWidth).reduced (m, 0));
        loadTranslationButton.setBounds (r2.reduced (m, 0));

        label1   .setBounds (r.removeFromTop (textH)  .reduced (m));
        editorPre.setBounds (r.removeFromTop (editorH).reduced (m, 0));

        label2    .setBounds (r.removeFromTop (textH)  .reduced (m));
        editorPost.setBounds (r.removeFromTop (editorH).reduced (m, 0));

        r2 = r.removeFromTop (textH);
        generateButton.setBounds (r2.removeFromRight (152).reduced (m));
        label3        .setBounds (r2.reduced (m));
        editorResult  .setBounds (r.removeFromTop (editorH).reduced (m, 0));

        label4        .setBounds (r.removeFromTop (textH).reduced (m));
        editorOriginal.setBounds (r.reduced (m, 0));
    }

private:
    CodeDocument documentOriginal, documentPre, documentPost, documentResult;
    CodeEditorComponent editorOriginal, editorPre, editorPost, editorResult;
    Label label1, label2, label3, label4;
    TextButton generateButton;
    Label instructionsLabel;
    TextButton scanProjectButton;
    TextButton scanFolderButton;
    TextButton loadTranslationButton;

    void buttonClicked (Button* b) override
    {
        if      (b == &generateButton)        generate();
        else if (b == &scanProjectButton)     scanProject();
        else if (b == &scanFolderButton)      scanFolder();
        else if (b == &loadTranslationButton) loadFile();
        else
            jassertfalse;
    }

    void generate()
    {
        StringArray preStrings  (TranslationHelpers::breakApart (documentPre.getAllContent()));
        StringArray postStrings (TranslationHelpers::breakApart (documentPost.getAllContent()));

        if (postStrings.size() != preStrings.size())
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              TRANS("Error"),
                                              TRANS("The pre- and post-translation text doesn't match!\n\n"
                                                    "Perhaps it got mangled by the translator?"));
            return;
        }

        const LocalisedStrings originalTranslation (documentOriginal.getAllContent(), false);
        documentResult.replaceAllContent (TranslationHelpers::createFinishedTranslationFile (preStrings, postStrings, originalTranslation));
    }

    void scanProject()
    {
        if (Project* project = ProjucerApplication::getApp().mainWindowList.getFrontmostProject())
            setPreTranslationText (TranslationHelpers::getPreTranslationText (*project));
        else
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Translation Tool",
                                              "This will only work when you have a project open!");
    }

    void scanFolder()
    {
        FileChooser fc ("Choose the root folder to search for the TRANS macros",
                        File(), "*");

        if (fc.browseForDirectory())
        {
            StringArray strings;
            TranslationHelpers::scanFolderForTranslations (strings, fc.getResult());
            setPreTranslationText (TranslationHelpers::mungeStrings(strings));
        }
    }

    void loadFile()
    {
        FileChooser fc ("Choose a translation file to load",
                        File(), "*");

        if (fc.browseForFileToOpen())
        {
            const LocalisedStrings loadedStrings (fc.getResult(), false);
            documentOriginal.replaceAllContent (fc.getResult().loadFileAsString().trim());
            setPreTranslationText (TranslationHelpers::getPreTranslationText (loadedStrings));
        }
    }

    void setPreTranslationText (const String& text)
    {
        documentPre.replaceAllContent (text);
        editorPre.grabKeyboardFocus();
        editorPre.selectAll();
    }
};


#endif   // JUCER_TRANSLATIONTOOLCOMPONENT_H_INCLUDED
