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

#ifndef __JUCE_TABBEDBUTTONBAR_JUCEHEADER__
#define __JUCE_TABBEDBUTTONBAR_JUCEHEADER__

#include "../buttons/juce_Button.h"
#include "../../../events/juce_ChangeBroadcaster.h"
#include "../../graphics/effects/juce_DropShadowEffect.h"
class TabbedButtonBar;


//==============================================================================
/** In a TabbedButtonBar, this component is used for each of the buttons.

    If you want to create a TabbedButtonBar with custom tab components, derive
    your component from this class, and override the TabbedButtonBar::createTabButton()
    method to create it instead of the default one.

    @see TabbedButtonBar
*/
class JUCE_API  TabBarButton  : public Button
{
public:
    //==============================================================================
    /** Creates the tab button. */
    TabBarButton (const String& name,
                  TabbedButtonBar* const ownerBar,
                  const int tabIndex);

    /** Destructor. */
    ~TabBarButton();

    //==============================================================================
    /** Chooses the best length for the tab, given the specified depth.

        If the tab is horizontal, this should return its width, and the depth
        specifies its height. If it's vertical, it should return the height, and
        the depth is actually its width.
    */
    virtual int getBestTabLength (const int depth);

    //==============================================================================
    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown);
    void clicked (const ModifierKeys& mods);
    bool hitTest (int x, int y);

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    friend class TabbedButtonBar;
    TabbedButtonBar* const owner;
    int tabIndex, overlapPixels;
    DropShadowEffect shadow;

    /** Returns an area of the component that's safe to draw in.

        This deals with the orientation of the tabs, which affects which side is
        touching the tabbed box's content component.
    */
    void getActiveArea (int& x, int& y, int& w, int& h);

private:
    TabBarButton (const TabBarButton&);
    const TabBarButton& operator= (const TabBarButton&);
};


//==============================================================================
/**
    A vertical or horizontal bar containing tabs that you can select.

    You can use one of these to generate things like a dialog box that has
    tabbed pages you can flip between. Attach a ChangeListener to the
    button bar to be told when the user changes the page.

    An easier method than doing this is to use a TabbedComponent, which
    contains its own TabbedButtonBar and which takes care of the layout
    and other housekeeping.

    @see TabbedComponent
*/
class JUCE_API  TabbedButtonBar  : public Component,
                                   public ChangeBroadcaster,
                                   public ButtonListener
{
public:
    //==============================================================================
    /** The placement of the tab-bar

        @see setOrientation, getOrientation
    */
    enum Orientation
    {
        TabsAtTop,
        TabsAtBottom,
        TabsAtLeft,
        TabsAtRight
    };

    //==============================================================================
    /** Creates a TabbedButtonBar with a given placement.

        You can change the orientation later if you need to.
    */
    TabbedButtonBar (const Orientation orientation);

    /** Destructor. */
    ~TabbedButtonBar();

    //==============================================================================
    /** Changes the bar's orientation.

        This won't change the bar's actual size - you'll need to do that yourself,
        but this determines which direction the tabs go in, and which side they're
        stuck to.
    */
    void setOrientation (const Orientation orientation);

    /** Returns the current orientation.

        @see setOrientation
    */
    Orientation getOrientation() const throw()                      { return orientation; }

    //==============================================================================
    /** Deletes all the tabs from the bar.

        @see addTab
    */
    void clearTabs();

    /** Adds a tab to the bar.

        Tabs are added in left-to-right reading order.

        If this is the first tab added, it'll also be automatically selected.
    */
    void addTab (const String& tabName,
                 const Colour& tabBackgroundColour,
                 int insertIndex = -1);

    /** Changes the name of one of the tabs. */
    void setTabName (const int tabIndex,
                     const String& newName);

    /** Gets rid of one of the tabs. */
    void removeTab (const int tabIndex);

    /** Moves a tab to a new index in the list.

        Pass -1 as the index to move it to the end of the list.
    */
    void moveTab (const int currentIndex,
                  const int newIndex);

    /** Returns the number of tabs in the bar. */
    int getNumTabs() const;

    /** Returns a list of all the tab names in the bar. */
    const StringArray getTabNames() const;

    /** Changes the currently selected tab.

        This will send a change message and cause a synchronous callback to
        the currentTabChanged() method. (But if the given tab is already selected,
        nothing will be done).

        To deselect all the tabs, use an index of -1.
    */
    void setCurrentTabIndex (int newTabIndex, const bool sendChangeMessage = true);

    /** Returns the name of the currently selected tab.

        This could be an empty string if none are selected.
    */
    const String& getCurrentTabName() const throw()                     { return tabs [currentTabIndex]; }

    /** Returns the index of the currently selected tab.

        This could return -1 if none are selected.
    */
    int getCurrentTabIndex() const throw()                              { return currentTabIndex; }

    /** Returns the button for a specific tab.

        The button that is returned may be deleted later by this component, so don't hang
        on to the pointer that is returned. A null pointer may be returned if the index is
        out of range.
    */
    TabBarButton* getTabButton (const int index) const;

    //==============================================================================
    /** Callback method to indicate the selected tab has been changed.

        @see setCurrentTabIndex
    */
    virtual void currentTabChanged (const int newCurrentTabIndex,
                                    const String& newCurrentTabName);

    /** Callback method to indicate that the user has right-clicked on a tab.

        (Or ctrl-clicked on the Mac)
    */
    virtual void popupMenuClickOnTab (const int tabIndex,
                                      const String& tabName);

    /** Returns the colour of a tab.

        This is the colour that was specified in addTab().
    */
    const Colour getTabBackgroundColour (const int tabIndex);

    /** Changes the background colour of a tab.

        @see addTab, getTabBackgroundColour
    */
    void setTabBackgroundColour (const int tabIndex, const Colour& newColour);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the component.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        tabOutlineColourId              = 0x1005812,    /**< The colour to use to draw an outline around the tabs.  */
        tabTextColourId                 = 0x1005813,    /**< The colour to use to draw the tab names. If this isn't specified,
                                                             the look and feel will choose an appropriate colour. */
        frontOutlineColourId            = 0x1005814,    /**< The colour to use to draw an outline around the currently-selected tab.  */
        frontTextColourId               = 0x1005815,    /**< The colour to use to draw the currently-selected tab name. If
                                                             this isn't specified, the look and feel will choose an appropriate
                                                             colour. */
    };

    //==============================================================================
    /** @internal */
    void resized();
    /** @internal */
    void buttonClicked (Button* button);
    /** @internal */
    void lookAndFeelChanged();

    juce_UseDebuggingNewOperator

protected:
    //==============================================================================
    /** This creates one of the tabs.

        If you need to use custom tab components, you can override this method and
        return your own class instead of the default.
    */
    virtual TabBarButton* createTabButton (const String& tabName,
                                           const int tabIndex);

private:
    Orientation orientation;

    StringArray tabs;
    Array <Colour> tabColours;
    int currentTabIndex;
    Component* behindFrontTab;
    Button* extraTabsButton;

    TabbedButtonBar (const TabbedButtonBar&);
    const TabbedButtonBar& operator= (const TabbedButtonBar&);
};


#endif   // __JUCE_TABBEDBUTTONBAR_JUCEHEADER__
