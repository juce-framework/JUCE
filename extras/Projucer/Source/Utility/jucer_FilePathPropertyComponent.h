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

#pragma once


class FilePathPropertyComponent  : public PropertyComponent
{
public:
    /** A Property Component for selecting files or folders.

        The user may drag files over the property box, enter the path
        manually and/or click the '...' button to open a file selection
        dialog box
    */
    FilePathPropertyComponent (Value valueToControl,
                               const String& propertyDescription,
                               bool isDirectory,
                               const String& wildcards = "*",
                               const File& rootToUseForRelativePaths = File(),
                               const bool supportsMultiplePaths = false)
        : PropertyComponent (propertyDescription),
          innerComp (valueToControl, isDirectory, wildcards, rootToUseForRelativePaths, supportsMultiplePaths)
    {
        addAndMakeVisible (innerComp);
    }

    void refresh() override {} // N/A

private:
    struct InnerComponent   : public Component,
                              public FileDragAndDropTarget,
                              private Button::Listener,
                              private TextEditor::Listener
    {
        InnerComponent (Value v, bool isDir, const String& wc, const File& rt, const bool multiplePaths)
            : value (v),
              isDirectory (isDir),
              highlightForDragAndDrop (false),
              wildcards (wc),
              root (rt),
              button ("..."),
              supportsMultiplePaths (multiplePaths)
        {
            addAndMakeVisible (textbox);
            textbox.getTextValue().referTo (value);
            textbox.addListener (this);

            addAndMakeVisible (button);
            button.addListener (this);

            lookAndFeelChanged();
        }

        void paintOverChildren (Graphics& g) override
        {
            if (highlightForDragAndDrop)
            {
                g.setColour (findColour (defaultHighlightColourId).withAlpha (0.5f));
                g.fillRect (textbox.getBounds());
            }
        }

        void resized() override
        {
            juce::Rectangle<int> r (getLocalBounds());

            button.setBounds (r.removeFromRight (30));
            textbox.setBounds (r);
        }

        bool isInterestedInFileDrag (const StringArray&) override   { return true; }
        void fileDragEnter (const StringArray&, int, int) override  { highlightForDragAndDrop = true;  repaint(); }
        void fileDragExit (const StringArray&) override             { highlightForDragAndDrop = false; repaint(); }

        void filesDropped (const StringArray& files, int, int) override
        {
            const File firstFile (files[0]);

            if (isDirectory)
                setTo (firstFile.isDirectory() ? firstFile
                                               : firstFile.getParentDirectory());
            else
                setTo (firstFile);

            highlightForDragAndDrop = false;
            repaint();
        }

        void buttonClicked (Button*) override
        {
            const File currentFile (root.getChildFile (value.toString()));

            if (isDirectory)
            {
                FileChooser chooser ("Select directory", currentFile);

                if (chooser.browseForDirectory())
                    setTo (chooser.getResult());
            }
            else
            {
                FileChooser chooser ("Select file", currentFile, wildcards);

                if (chooser.browseForFileToOpen())
                    setTo (chooser.getResult());
            }
        }

        void textEditorReturnKeyPressed (TextEditor& editor) override   { updateEditorColour (editor); }
        void textEditorFocusLost (TextEditor& editor) override          { updateEditorColour (editor); }

        void updateEditorColour (TextEditor& editor)
        {
            if (supportsMultiplePaths)
            {
                auto paths = StringArray::fromTokens (editor.getTextValue().toString(), ";", {});

                editor.clear();

                AttributedString str;
                for (auto p : paths)
                {
                    if (root.getChildFile (p.trim()).exists())    editor.setColour (TextEditor::textColourId, findColour (widgetTextColourId));
                    else                                          editor.setColour (TextEditor::textColourId, Colours::red);

                    editor.insertTextAtCaret (p);

                    if (paths.indexOf (p) < paths.size() - 1)
                    {
                        editor.setColour (TextEditor::textColourId, findColour (widgetTextColourId));
                        editor.insertTextAtCaret (";");
                    }
                }

                editor.setColour (TextEditor::textColourId, findColour (widgetTextColourId));
            }
            else
            {
                auto pathToCheck = editor.getTextValue().toString();

                //android SDK/NDK paths
                if (pathToCheck.contains ("${user.home}"))
                    pathToCheck = pathToCheck.replace ("${user.home}", File::getSpecialLocation (File::userHomeDirectory).getFullPathName());

              #if JUCE_WINDOWS
                if (pathToCheck.startsWith ("~"))
                    pathToCheck = pathToCheck.replace ("~", File::getSpecialLocation (File::userHomeDirectory).getFullPathName());
              #endif

                const auto currentFile = root.getChildFile (pathToCheck);

                if (currentFile.exists())
                    editor.applyColourToAllText (findColour (widgetTextColourId));
                else
                    editor.applyColourToAllText (Colours::red);
            }
        }

        void setTo (const File& f)
        {
            auto pathName = (root == File()) ? f.getFullPathName()
                                             : f.getRelativePathFrom (root);

            if (supportsMultiplePaths && value.toString().isNotEmpty())
                value = value.toString().trimCharactersAtEnd (" ;") + "; " + pathName;
            else
                value = pathName;

            updateEditorColour (textbox);
        }

        void lookAndFeelChanged() override
        {
            textbox.setColour (TextEditor::backgroundColourId, findColour (widgetBackgroundColourId));
            textbox.setColour (TextEditor::outlineColourId, Colours::transparentBlack);
            updateEditorColour (textbox);

            button.setColour (TextButton::buttonColourId, findColour (secondaryButtonBackgroundColourId));
            button.setColour (TextButton::textColourOffId, Colours::white);
        }

        Value value;
        bool isDirectory, highlightForDragAndDrop;
        String wildcards;
        File root;
        TextEditor textbox;
        TextButton button;
        bool supportsMultiplePaths;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InnerComponent)
    };

    InnerComponent innerComp;  // Used so that the PropertyComponent auto first-child positioning works

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilePathPropertyComponent)
};
