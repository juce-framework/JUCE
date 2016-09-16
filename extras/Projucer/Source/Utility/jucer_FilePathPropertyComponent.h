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

#ifndef JUCER_FILEPATHPROPERTYCOMPONENT_H_INCLUDED
#define JUCER_FILEPATHPROPERTYCOMPONENT_H_INCLUDED


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
                               const File& rootToUseForRelativePaths = File())
        : PropertyComponent (propertyDescription),
          innerComp (valueToControl, isDirectory, wildcards, rootToUseForRelativePaths)
    {
        addAndMakeVisible (innerComp);
    }

    void refresh() override {} // N/A

private:
    struct InnerComponent   : public Component,
                              public FileDragAndDropTarget,
                              private Button::Listener
    {
        InnerComponent (Value v, bool isDir, const String& wc, const File& rt)
            : value (v),
              isDirectory (isDir),
              highlightForDragAndDrop (false),
              wildcards (wc),
              root (rt),
              button ("...")
        {
            addAndMakeVisible (textbox);
            textbox.getTextValue().referTo (value);

            addAndMakeVisible (button);
            button.addListener (this);
        }

        void paintOverChildren (Graphics& g) override
        {
            if (highlightForDragAndDrop)
            {
                g.setColour (Colours::green.withAlpha (0.1f));
                g.fillRect (getLocalBounds());
            }
        }

        void resized() override
        {
            juce::Rectangle<int> r (getLocalBounds());

            button.setBounds (r.removeFromRight (24));
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

        void setTo (const File& f)
        {
            value = (root == File()) ? f.getFullPathName()
                                     : f.getRelativePathFrom (root);
        }

        Value value;
        bool isDirectory, highlightForDragAndDrop;
        String wildcards;
        File root;
        TextEditor textbox;
        TextButton button;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InnerComponent)
    };

    InnerComponent innerComp;  // Used so that the PropertyComponent auto first-child positioning works

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilePathPropertyComponent)
};


#endif  // JUCER_FILEPATHPROPERTYCOMPONENT_H_INCLUDED
