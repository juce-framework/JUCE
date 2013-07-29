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

#ifndef JUCE_PREFERENCESPANEL_H_INCLUDED
#define JUCE_PREFERENCESPANEL_H_INCLUDED


//==============================================================================
/**
    A component with a set of buttons at the top for changing between pages of
    preferences.

    This is just a handy way of writing a Mac-style preferences panel where you
    have a row of buttons along the top for the different preference categories,
    each button having an icon above its name. Clicking these will show an
    appropriate prefs page below it.

    You can either put one of these inside your own component, or just use the
    showInDialogBox() method to show it in a window and run it modally.

    To use it, just add a set of named pages with the addSettingsPage() method,
    and implement the createComponentForPage() method to create suitable components
    for each of these pages.
*/
class JUCE_API  PreferencesPanel  : public Component,
                                    private ButtonListener // (can't use Button::Listener due to idiotic VC2005 bug)
{
public:
    //==============================================================================
    /** Creates an empty panel.

        Use addSettingsPage() to add some pages to it in your constructor.
    */
    PreferencesPanel();

    /** Destructor. */
    ~PreferencesPanel();

    //==============================================================================
    /** Creates a page using a set of drawables to define the page's icon.

        Note that the other version of this method is much easier if you're using
        an image instead of a custom drawable.

        @param pageTitle    the name of this preferences page - you'll need to
                            make sure your createComponentForPage() method creates
                            a suitable component when it is passed this name
        @param normalIcon   the drawable to display in the page's button normally
        @param overIcon     the drawable to display in the page's button when the mouse is over
        @param downIcon     the drawable to display in the page's button when the button is down
        @see DrawableButton
    */
    void addSettingsPage (const String& pageTitle,
                          const Drawable* normalIcon,
                          const Drawable* overIcon,
                          const Drawable* downIcon);

    /** Creates a page using a set of drawables to define the page's icon.

        The other version of this method gives you more control over the icon, but this
        one is much easier if you're just loading it from a file.

        @param pageTitle        the name of this preferences page - you'll need to
                                make sure your createComponentForPage() method creates
                                a suitable component when it is passed this name
        @param imageData        a block of data containing an image file, e.g. a jpeg, png or gif.
                                For this to look good, you'll probably want to use a nice
                                transparent png file.
        @param imageDataSize    the size of the image data, in bytes
    */
    void addSettingsPage (const String& pageTitle,
                          const void* imageData,
                          int imageDataSize);

    /** Utility method to display this panel in a DialogWindow.

        Calling this will create a DialogWindow containing this panel with the
        given size and title, and will run it modally, returning when the user
        closes the dialog box.
    */
    void showInDialogBox (const String& dialogTitle,
                          int dialogWidth,
                          int dialogHeight,
                          Colour backgroundColour = Colours::white);

    //==============================================================================
    /** Subclasses must override this to return a component for each preferences page.

        The subclass should return a pointer to a new component representing the named
        page, which the panel will then display.

        The panel will delete the component later when the user goes to another page
        or deletes the panel.
    */
    virtual Component* createComponentForPage (const String& pageName) = 0;

    //==============================================================================
    /** Changes the current page being displayed. */
    void setCurrentPage (const String& pageName);

    /** Returns the size of the buttons shown along the top. */
    int getButtonSize() const noexcept;

    /** Changes the size of the buttons shown along the top. */
    void setButtonSize (int newSize);

    //==============================================================================
    /** @internal */
    void resized() override;
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void buttonClicked (Button*) override;

private:
    //==============================================================================
    String currentPageName;
    ScopedPointer <Component> currentPage;
    OwnedArray<DrawableButton> buttons;
    int buttonSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PreferencesPanel)
};



#endif   // JUCE_PREFERENCESPANEL_H_INCLUDED
