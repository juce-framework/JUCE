/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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
    A file open/save dialog box.

    This is a Juce-based file dialog box; to use a native file chooser, see the
    FileChooser class.

    To use one of these, create it and call its show() method. e.g.

    @code
    {
        WildcardFileFilter wildcardFilter ("*.foo", String(), "Foo files");

        FileBrowserComponent browser (FileBrowserComponent::canSelectFiles,
                                      File(),
                                      &wildcardFilter,
                                      nullptr);

        FileChooserDialogBox dialogBox ("Open some kind of file",
                                        "Please choose some kind of file that you want to open...",
                                        browser,
                                        false,
                                        Colours::lightgrey);

        if (dialogBox.show())
        {
            File selectedFile = browser.getSelectedFile (0);

            ...etc..
        }
    }
    @endcode

    @see FileChooser

    @tags{GUI}
*/
class JUCE_API  FileChooserDialogBox : public ResizableWindow,
                                       private FileBrowserListener
{
public:
    //==============================================================================
    /** Creates a file chooser box.

        @param title            the main title to show at the top of the box
        @param instructions     an optional longer piece of text to show below the title in
                                a smaller font, describing in more detail what's required.
        @param browserComponent a FileBrowserComponent that will be shown inside this dialog
                                box. Make sure you delete this after (but not before!) the
                                dialog box has been deleted.
        @param warnAboutOverwritingExistingFiles     if true, then the user will be asked to confirm
                                if they try to select a file that already exists. (This
                                flag is only used when saving files)
        @param backgroundColour the background colour for the top level window
        @param parentComponent  an optional component which should be the parent
                                for the file chooser. If this is a nullptr then the
                                dialog box will be a top-level window. AUv3s on iOS
                                must specify this parameter as opening a top-level window
                                in an AUv3 is forbidden due to sandbox restrictions.

        @see FileBrowserComponent, FilePreviewComponent
    */
    FileChooserDialogBox (const String& title,
                          const String& instructions,
                          FileBrowserComponent& browserComponent,
                          bool warnAboutOverwritingExistingFiles,
                          Colour backgroundColour,
                          Component* parentComponent = nullptr);

    /** Destructor. */
    ~FileChooserDialogBox() override;

    //==============================================================================
   #if JUCE_MODAL_LOOPS_PERMITTED || DOXYGEN
    /** Displays and runs the dialog box modally.

        This will show the box with the specified size, returning true if the user
        pressed 'ok', or false if they cancelled.

        Leave the width or height as 0 to use the default size
    */
    bool show (int width = 0, int height = 0);

    /** Displays and runs the dialog box modally.

        This will show the box with the specified size at the specified location,
        returning true if the user pressed 'ok', or false if they cancelled.

        Leave the width or height as 0 to use the default size.
    */
    bool showAt (int x, int y, int width, int height);
   #endif

    /** Sets the size of this dialog box to its default and positions it either in the
        centre of the screen, or centred around a component that is provided.
    */
    void centreWithDefaultSize (Component* componentToCentreAround = nullptr);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the box.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        titleTextColourId      = 0x1000850, /**< The colour to use to draw the box's title. */
    };

private:
    class ContentComponent;
    ContentComponent* content;
    const bool warnAboutOverwritingExistingFiles;

    void closeButtonPressed();
    void selectionChanged() override;
    void fileClicked (const File&, const MouseEvent&) override;
    void fileDoubleClicked (const File&) override;
    void browserRootChanged (const File&) override;
    int getDefaultWidth() const;

    void okButtonPressed();
    void createNewFolder();
    void createNewFolderConfirmed (const String& name);

    static void okToOverwriteFileCallback (int result, FileChooserDialogBox*);
    static void createNewFolderCallback (int result, FileChooserDialogBox*, Component::SafePointer<AlertWindow>);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileChooserDialogBox)
};

} // namespace juce
