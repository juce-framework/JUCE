/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_FILECHOOSERDIALOGBOX_JUCEHEADER__
#define __JUCE_FILECHOOSERDIALOGBOX_JUCEHEADER__

#include "juce_FileBrowserComponent.h"
#include "../windows/juce_ResizableWindow.h"
#include "../buttons/juce_TextButton.h"
#include "../../graphics/fonts/juce_GlyphArrangement.h"


//==============================================================================
/**
    A file open/save dialog box.

    This is a Juce-based file dialog box; to use a native file chooser, see the
    FileChooser class.

    To use one of these, create it and call its show() method. e.g.

    @code
    {
        WildcardFileFilter wildcardFilter (T("*.foo"), T("Foo files"));

        FileBrowserComponent browser (FileBrowserComponent::loadFileMode,
                                      File::nonexistent,
                                      &wildcardFilter,
                                      0);

        FileChooserDialogBox dialogBox (T("Open some kind of file"),
                                        T("Please choose some kind of file that you want to open..."),
                                        browser,
                                        getLookAndFeel().alertWindowBackground);

        if (dialogBox.show())
        {
            File selectedFile = browser.getCurrentFile();
            ...
        }
    }
    @endcode

    @see FileChooser
*/
class JUCE_API  FileChooserDialogBox : public ResizableWindow,
                                       public ButtonListener,
                                       public FileBrowserListener
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
                          const bool warnAboutOverwritingExistingFiles,
                          const Colour& backgroundColour);

    /** Destructor. */
    ~FileChooserDialogBox();

    //==============================================================================
    /** Displays and runs the dialog box modally.

        This will show the box with the specified size, returning true if the user
        pressed 'ok', or false if they cancelled.

        Leave the width or height as 0 to use the default size
    */
    bool show (int width = 0,int height = 0);


    //==============================================================================
    /** @internal */
    void buttonClicked (Button* button);
    /** @internal */
    void closeButtonPressed();
    /** @internal */
    void selectionChanged();
    /** @internal */
    void fileClicked (const File& file, const MouseEvent& e);
    /** @internal */
    void fileDoubleClicked (const File& file);

    juce_UseDebuggingNewOperator

private:
    class ContentComponent  : public Component
    {
    public:
        ContentComponent();
        ~ContentComponent();

        void paint (Graphics& g);
        void resized();

        String instructions;
        GlyphArrangement text;

        FileBrowserComponent* chooserComponent;
        FilePreviewComponent* previewComponent;
        TextButton* okButton;
        TextButton* cancelButton;
    };

    ContentComponent* content;
    const bool warnAboutOverwritingExistingFiles;

    FileChooserDialogBox (const FileChooserDialogBox&);
    const FileChooserDialogBox& operator= (const FileChooserDialogBox&);
};


#endif   // __JUCE_FILECHOOSERDIALOGBOX_JUCEHEADER__
