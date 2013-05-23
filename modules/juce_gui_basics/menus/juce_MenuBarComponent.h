/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_MENUBARCOMPONENT_JUCEHEADER__
#define __JUCE_MENUBARCOMPONENT_JUCEHEADER__

#include "juce_MenuBarModel.h"


//==============================================================================
/**
    A menu bar component.

    @see MenuBarModel
*/
class JUCE_API  MenuBarComponent  : public Component,
                                    private MenuBarModel::Listener,
                                    private Timer
{
public:
    //==============================================================================
    /** Creates a menu bar.

        @param model        the model object to use to control this bar. You can
                            pass 0 into this if you like, and set the model later
                            using the setModel() method
    */
    MenuBarComponent (MenuBarModel* model);

    /** Destructor. */
    ~MenuBarComponent();

    //==============================================================================
    /** Changes the model object to use to control the bar.

        This can be a null pointer, in which case the bar will be empty. Don't delete the object
        that is passed-in while it's still being used by this MenuBar.
    */
    void setModel (MenuBarModel* newModel);

    /** Returns the current menu bar model being used.
    */
    MenuBarModel* getModel() const noexcept;

    //==============================================================================
    /** Pops up one of the menu items.

        This lets you manually open one of the menus - it could be triggered by a
        key shortcut, for example.
    */
    void showMenu (int menuIndex);

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void resized();
    /** @internal */
    void mouseEnter (const MouseEvent& e);
    /** @internal */
    void mouseExit (const MouseEvent& e);
    /** @internal */
    void mouseDown (const MouseEvent& e);
    /** @internal */
    void mouseDrag (const MouseEvent& e);
    /** @internal */
    void mouseUp (const MouseEvent& e);
    /** @internal */
    void mouseMove (const MouseEvent& e);
    /** @internal */
    void handleCommandMessage (int commandId);
    /** @internal */
    bool keyPressed (const KeyPress& key);
    /** @internal */
    void menuBarItemsChanged (MenuBarModel* menuBarModel);
    /** @internal */
    void menuCommandInvoked (MenuBarModel* menuBarModel,
                             const ApplicationCommandTarget::InvocationInfo& info);

private:
    //==============================================================================
    MenuBarModel* model;

    StringArray menuNames;
    Array<int> xPositions;
    Point<int> lastMousePos;
    int itemUnderMouse, currentPopupIndex, topLevelIndexClicked;

    int getItemAt (Point<int>);
    void setItemUnderMouse (int index);
    void setOpenItem (int index);
    void updateItemUnderMouse (Point<int>);
    void timerCallback();
    void repaintMenuItem (int index);
    void menuDismissed (int topLevelIndex, int itemId);
    static void menuBarMenuDismissedCallback (int, MenuBarComponent*, int);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MenuBarComponent)
};

#endif   // __JUCE_MENUBARCOMPONENT_JUCEHEADER__
