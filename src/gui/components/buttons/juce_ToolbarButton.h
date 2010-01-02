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

#ifndef __JUCE_TOOLBARBUTTON_JUCEHEADER__
#define __JUCE_TOOLBARBUTTON_JUCEHEADER__

#include "../controls/juce_ToolbarItemComponent.h"


//==============================================================================
/**
    A type of button designed to go on a toolbar.

    This simple button can have two Drawable objects specified - one for normal
    use and another one (optionally) for the button's "on" state if it's a
    toggle button.

    @see Toolbar, ToolbarItemFactory, ToolbarItemComponent, Drawable, Button
*/
class JUCE_API  ToolbarButton   : public ToolbarItemComponent
{
public:
    //==============================================================================
    /** Creates a ToolbarButton.

        @param itemId       the ID for this toolbar item type. This is passed through to the
                            ToolbarItemComponent constructor
        @param labelText    the text to display on the button (if the toolbar is using a style
                            that shows text labels). This is passed through to the
                            ToolbarItemComponent constructor
        @param normalImage  a drawable object that the button should use as its icon. The object
                            that is passed-in here will be kept by this object and will be
                            deleted when no longer needed or when this button is deleted.
        @param toggledOnImage  a drawable object that the button can use as its icon if the button
                            is in a toggled-on state (see the Button::getToggleState() method). If
                            0 is passed-in here, then the normal image will be used instead, regardless
                            of the toggle state. The object that is passed-in here will be kept by
                            this object and will be deleted when no longer needed or when this button
                            is deleted.
    */
    ToolbarButton (const int itemId,
                   const String& labelText,
                   Drawable* const normalImage,
                   Drawable* const toggledOnImage);

    /** Destructor. */
    ~ToolbarButton();


    //==============================================================================
    /** @internal */
    bool getToolbarItemSizes (int toolbarDepth, bool isToolbarVertical, int& preferredSize,
                              int& minSize, int& maxSize);
    /** @internal */
    void paintButtonArea (Graphics& g, int width, int height, bool isMouseOver, bool isMouseDown);
    /** @internal */
    void contentAreaChanged (const Rectangle& newBounds);

    juce_UseDebuggingNewOperator

private:
    ScopedPointer <Drawable> normalImage, toggledOnImage;

    ToolbarButton (const ToolbarButton&);
    const ToolbarButton& operator= (const ToolbarButton&);
};


#endif   // __JUCE_TOOLBARBUTTON_JUCEHEADER__
