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

#ifndef __JUCE_TABBEDCOMPONENT_JUCEHEADER__
#define __JUCE_TABBEDCOMPONENT_JUCEHEADER__

#include "juce_TabbedButtonBar.h"


//==============================================================================
/**
    A component with a TabbedButtonBar along one of its sides.

    This makes it easy to create a set of tabbed pages, just add a bunch of tabs
    with addTab(), and this will take care of showing the pages for you when the
    user clicks on a different tab.

    @see TabbedButtonBar
*/
class JUCE_API  TabbedComponent  : public Component
{
public:
    //==============================================================================
    /** Creates a TabbedComponent, specifying where the tabs should be placed.

        Once created, add some tabs with the addTab() method.
    */
    TabbedComponent (const TabbedButtonBar::Orientation orientation);

    /** Destructor. */
    ~TabbedComponent();

    //==============================================================================
    /** Changes the placement of the tabs.

        This will rearrange the layout to place the tabs along the appropriate
        side of this component, and will shift the content component accordingly.

        @see TabbedButtonBar::setOrientation
    */
    void setOrientation (const TabbedButtonBar::Orientation orientation);

    /** Returns the current tab placement.

        @see setOrientation, TabbedButtonBar::getOrientation
    */
    TabbedButtonBar::Orientation getOrientation() const throw();

    /** Specifies how many pixels wide or high the tab-bar should be.

        If the tabs are placed along the top or bottom, this specified the height
        of the bar; if they're along the left or right edges, it'll be the width
        of the bar.
    */
    void setTabBarDepth (const int newDepth);

    /** Returns the current thickness of the tab bar.

        @see setTabBarDepth
    */
    int getTabBarDepth() const throw()                          { return tabDepth; }

    /** Specifies the thickness of an outline that should be drawn around the content component.

        If this thickness is > 0, a line will be drawn around the three sides of the content
        component which don't touch the tab-bar, and the content component will be inset by this amount.

        To set the colour of the line, use setColour (outlineColourId, ...).
    */
    void setOutline (const int newThickness);

    /** Specifies a gap to leave around the edge of the content component.

        Each edge of the content component will be indented by the given number of pixels.
    */
    void setIndent (const int indentThickness);

    //==============================================================================
    /** Removes all the tabs from the bar.

        @see TabbedButtonBar::clearTabs
    */
    void clearTabs();

    /** Adds a tab to the tab-bar.

        The component passed in will be shown for the tab, and if deleteComponentWhenNotNeeded
        is true, it will be deleted when the tab is removed or when this object is
        deleted.

        @see TabbedButtonBar::addTab
    */
    void addTab (const String& tabName,
                 const Colour& tabBackgroundColour,
                 Component* const contentComponent,
                 const bool deleteComponentWhenNotNeeded,
                 const int insertIndex = -1);

    /** Changes the name of one of the tabs. */
    void setTabName (const int tabIndex,
                     const String& newName);

    /** Gets rid of one of the tabs. */
    void removeTab (const int tabIndex);

    /** Returns the number of tabs in the bar. */
    int getNumTabs() const;

    /** Returns a list of all the tab names in the bar. */
    const StringArray getTabNames() const;

    /** Returns the content component that was added for the given index.

        Be sure not to use or delete the components that are returned, as this may interfere
        with the TabbedComponent's use of them.
    */
    Component* getTabContentComponent (const int tabIndex) const throw();

    /** Returns the colour of one of the tabs. */
    const Colour getTabBackgroundColour (const int tabIndex) const throw();

    /** Changes the background colour of one of the tabs. */
    void setTabBackgroundColour (const int tabIndex, const Colour& newColour);

    //==============================================================================
    /** Changes the currently-selected tab.

        To deselect all the tabs, pass -1 as the index.

        @see TabbedButtonBar::setCurrentTabIndex
    */
    void setCurrentTabIndex (const int newTabIndex, const bool sendChangeMessage = true);

    /** Returns the index of the currently selected tab.

        @see addTab, TabbedButtonBar::getCurrentTabIndex()
    */
    int getCurrentTabIndex() const;

    /** Returns the name of the currently selected tab.

        @see addTab, TabbedButtonBar::getCurrentTabName()
    */
    const String& getCurrentTabName() const;

    /** Returns the current component that's filling the panel.

        This will return 0 if there isn't one.
    */
    Component* getCurrentContentComponent() const throw()           { return panelComponent; }

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

    /** Returns the tab button bar component that is being used.
    */
    TabbedButtonBar& getTabbedButtonBar() const throw()             { return *tabs; }

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the component.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId          = 0x1005800,    /**< The colour to fill the background behind the tabs. */
        outlineColourId             = 0x1005801,    /**< The colour to use to draw an outline around the content.
                                                         (See setOutline)  */
    };

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void resized();
    /** @internal */
    void lookAndFeelChanged();

    juce_UseDebuggingNewOperator

protected:
    //==============================================================================
    TabbedButtonBar* tabs;

    //==============================================================================
    /** This creates one of the tab buttons.

        If you need to use custom tab components, you can override this method and
        return your own class instead of the default.
    */
    virtual TabBarButton* createTabButton (const String& tabName,
                                           const int tabIndex);

private:
    //==============================================================================
    Array <Component*> contentComponents;
    Component* panelComponent;
    int tabDepth;
    int outlineThickness, edgeIndent;

    friend class TabCompButtonBar;
    void changeCallback (const int newCurrentTabIndex, const String& newTabName);

    TabbedComponent (const TabbedComponent&);
    const TabbedComponent& operator= (const TabbedComponent&);
};


#endif   // __JUCE_TABBEDCOMPONENT_JUCEHEADER__
