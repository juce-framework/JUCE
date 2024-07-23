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

//==============================================================================
/**
    Base class for components that live inside a file chooser dialog box and
    show previews of the files that get selected.

    One of these allows special extra information to be displayed for files
    in a dialog box as the user selects them. Each time the current file or
    directory is changed, the selectedFileChanged() method will be called
    to allow it to update itself appropriately.

    @see FileChooser, ImagePreviewComponent

    @tags{GUI}
*/
class JUCE_API  FilePreviewComponent  : public Component
{
public:
    //==============================================================================
    /** Creates a FilePreviewComponent. */
    FilePreviewComponent();

    /** Destructor. */
    ~FilePreviewComponent() override;

    /** Called to indicate that the user's currently selected file has changed.

        @param newSelectedFile  the newly selected file or directory, which may be
                                a default File() object if none is selected.
    */
    virtual void selectedFileChanged (const File& newSelectedFile) = 0;


private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilePreviewComponent)
};

} // namespace juce
