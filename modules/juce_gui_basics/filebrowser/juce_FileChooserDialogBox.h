/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_FILECHOOSERDIALOGBOX_H_INCLUDED
#define JUCE_FILECHOOSERDIALOGBOX_H_INCLUDED


//==============================================================================
/**
    A file open/save dialog box.

    This is a Juce-based file dialog box; to use a native file chooser, see the
    FileChooser class.

    To use one of these, create it and call its show() method. e.g.

    @code
    {
        WildcardFileFilter wildcardFilter ("*.foo", String::empty, "Foo files");

        FileBrowserComponent browser (FileBrowserComponent::canSelectFiles,
                                      File::nonexistent,
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
*/
class JUCE_API  FileChooserDialogBox : public ResizableWindow,
                                       private ButtonListener,  // (can't use Button::Listener due to idiotic VC2005 bug)
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

        @see FileBrowserComponent, FilePreviewComponent
    */
    FileChooserDialogBox (const String& title,
                          const String& instructions,
                          FileBrowserComponent& browserComponent,
                          bool warnAboutOverwritingExistingFiles,
                          Colour backgroundColour);

    /** Destructor. */
    ~FileChooserDialogBox();

    //==============================================================================
   #if JUCE_MODAL_LOOPS_PERMITTED
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

    void buttonClicked (Button*) override;
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


#endif   // JUCE_FILECHOOSERDIALOGBOX_H_INCLUDED
