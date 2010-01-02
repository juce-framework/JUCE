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

#ifndef __JUCE_PREFERENCESPANEL_JUCEHEADER__
#define __JUCE_PREFERENCESPANEL_JUCEHEADER__

#include "../juce_Component.h"
#include "../buttons/juce_Button.h"
#include "../../graphics/drawables/juce_Drawable.h"


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
                                    private ButtonListener
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
                          const char* imageData,
                          const int imageDataSize);

    /** Utility method to display this panel in a DialogWindow.

        Calling this will create a DialogWindow containing this panel with the
        given size and title, and will run it modally, returning when the user
        closes the dialog box.
    */
    void showInDialogBox (const String& dialogtitle,
                          int dialogWidth,
                          int dialogHeight,
                          const Colour& backgroundColour = Colours::white);

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

    //==============================================================================
    /** @internal */
    void resized();
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void buttonClicked (Button* button);

    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    String currentPageName;
    ScopedPointer <Component> currentPage;
    int buttonSize;

    PreferencesPanel (const PreferencesPanel&);
    const PreferencesPanel& operator= (const PreferencesPanel&);
};



#endif   // __JUCE_PREFERENCESPANEL_JUCEHEADER__
