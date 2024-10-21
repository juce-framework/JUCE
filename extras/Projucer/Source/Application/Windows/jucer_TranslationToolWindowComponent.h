/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../../Utility/Helpers/jucer_TranslationHelpers.h"

//==============================================================================
class TranslationToolComponent final : public Component
{
public:
    TranslationToolComponent()
        : editorOriginal (documentOriginal, nullptr),
          editorPre      (documentPre,      nullptr),
          editorPost     (documentPost,     nullptr),
          editorResult   (documentResult,   nullptr)
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

        addAndMakeVisible (generateButton);
        generateButton.onClick = [this] { generate(); };

        addAndMakeVisible (scanProjectButton);
        scanProjectButton.onClick = [this] { scanProject(); };

        addAndMakeVisible (scanFolderButton);
        scanFolderButton.onClick = [this] { scanFolder(); };

        addAndMakeVisible (loadTranslationButton);
        loadTranslationButton.onClick = [this] { loadFile(); };
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
    //==============================================================================
    void generate()
    {
        StringArray preStrings  (TranslationHelpers::breakApart (documentPre.getAllContent()));
        StringArray postStrings (TranslationHelpers::breakApart (documentPost.getAllContent()));

        if (postStrings.size() != preStrings.size())
        {
            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                             TRANS ("Error"),
                                                             TRANS ("The pre- and post-translation text doesn't match!\n\n"
                                                                    "Perhaps it got mangled by the translator?"));
            messageBox = AlertWindow::showScopedAsync (options, nullptr);
            return;
        }

        const LocalisedStrings originalTranslation (documentOriginal.getAllContent(), false);
        documentResult.replaceAllContent (TranslationHelpers::createFinishedTranslationFile (preStrings, postStrings, originalTranslation));
    }

    void scanProject()
    {
        if (Project* project = ProjucerApplication::getApp().mainWindowList.getFrontmostProject())
        {
            setPreTranslationText (TranslationHelpers::getPreTranslationText (*project));
        }
        else
        {
            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                             "Translation Tool",
                                                             "This will only work when you have a project open!");
            messageBox = AlertWindow::showScopedAsync (options, nullptr);
        }
    }

    void scanFolder()
    {
        chooser = std::make_unique<FileChooser> ("Choose the root folder to search for the TRANS macros",
                                                 File(), "*");
        auto chooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;

        chooser->launchAsync (chooserFlags, [this] (const FileChooser& fc)
        {
            if (fc.getResult() == File{})
                return;

            StringArray strings;
            TranslationHelpers::scanFolderForTranslations (strings, fc.getResult());
            setPreTranslationText (TranslationHelpers::mungeStrings (strings));
        });
    }

    void loadFile()
    {
        chooser = std::make_unique<FileChooser> ("Choose a translation file to load", File(), "*");
        auto chooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;

        chooser->launchAsync (chooserFlags, [this] (const FileChooser& fc)
        {
            if (fc.getResult() == File{})
                return;

            const LocalisedStrings loadedStrings (fc.getResult(), false);
            documentOriginal.replaceAllContent (fc.getResult().loadFileAsString().trim());
            setPreTranslationText (TranslationHelpers::getPreTranslationText (loadedStrings));
        });
    }

    void setPreTranslationText (const String& text)
    {
        documentPre.replaceAllContent (text);
        editorPre.grabKeyboardFocus();
        editorPre.selectAll();
    }

    //==============================================================================
    CodeDocument documentOriginal, documentPre, documentPost, documentResult;
    CodeEditorComponent editorOriginal, editorPre, editorPost, editorResult;

    Label label1, label2, label3, label4;
    Label instructionsLabel;

    TextButton generateButton        { TRANS ("Generate") },
               scanProjectButton     { "Scan project for TRANS macros" },
               scanFolderButton      { "Scan folder for TRANS macros" },
               loadTranslationButton { "Load existing translation file..."};

    std::unique_ptr<FileChooser> chooser;
    ScopedMessageBox messageBox;
};
