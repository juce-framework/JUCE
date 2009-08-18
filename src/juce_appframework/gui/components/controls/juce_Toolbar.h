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

#ifndef __JUCE_TOOLBAR_JUCEHEADER__
#define __JUCE_TOOLBAR_JUCEHEADER__

#include "../mouse/juce_DragAndDropContainer.h"
#include "../layout/juce_ComponentAnimator.h"
#include "../buttons/juce_Button.h"
class ToolbarItemComponent;
class ToolbarItemFactory;
class MissingItemsComponent;


//==============================================================================
/**
    A toolbar component.

    A toolbar contains a horizontal or vertical strip of ToolbarItemComponents,
    and looks after their order and layout.

    Items (icon buttons or other custom components) are added to a toolbar using a
    ToolbarItemFactory - each type of item is given a unique ID number, and a
    toolbar might contain more than one instance of a particular item type.

    Toolbars can be interactively customised, allowing the user to drag the items
    around, and to drag items onto or off the toolbar, using the ToolbarItemPalette
    component as a source of new items.

    @see ToolbarItemFactory, ToolbarItemComponent, ToolbarItemPalette
*/
class JUCE_API  Toolbar   : public Component,
                            public DragAndDropContainer,
                            public DragAndDropTarget,
                            private ButtonListener
{
public:
    //==============================================================================
    /** Creates an empty toolbar component.

        To add some icons or other components to your toolbar, you'll need to
        create a ToolbarItemFactory class that can create a suitable set of
        ToolbarItemComponents.

        @see ToolbarItemFactory, ToolbarItemComponents
    */
    Toolbar();

    /** Destructor.

        Any items on the bar will be deleted when the toolbar is deleted.
    */
    ~Toolbar();

    //==============================================================================
    /** Changes the bar's orientation.
        @see isVertical
    */
    void setVertical (const bool shouldBeVertical);

    /** Returns true if the bar is set to be vertical, or false if it's horizontal.

        You can change the bar's orientation with setVertical().
    */
    bool isVertical() const throw()                  { return vertical; }

    /** Returns the depth of the bar.

        If the bar is horizontal, this will return its height; if it's vertical, it
        will return its width.

        @see getLength
    */
    int getThickness() const throw();

    /** Returns the length of the bar.

        If the bar is horizontal, this will return its width; if it's vertical, it
        will return its height.

        @see getThickness
    */
    int getLength() const throw();

    //==============================================================================
    /** Deletes all items from the bar.
    */
    void clear();

    /** Adds an item to the toolbar.

        The factory's ToolbarItemFactory::createItem() will be called by this method
        to create the component that will actually be added to the bar.

        The new item will be inserted at the specified index (if the index is -1, it
        will be added to the right-hand or bottom end of the bar).

        Once added, the component will be automatically deleted by this object when it
        is no longer needed.

        @see ToolbarItemFactory
    */
    void addItem (ToolbarItemFactory& factory,
                  const int itemId,
                  const int insertIndex = -1);

    /** Deletes one of the items from the bar.
    */
    void removeToolbarItem (const int itemIndex);

    /** Returns the number of items currently on the toolbar.

        @see getItemId, getItemComponent
    */
    int getNumItems() const throw();

    /** Returns the ID of the item with the given index.

        If the index is less than zero or greater than the number of items,
        this will return 0.

        @see getNumItems
    */
    int getItemId (const int itemIndex) const throw();

    /** Returns the component being used for the item with the given index.

        If the index is less than zero or greater than the number of items,
        this will return 0.

        @see getNumItems
    */
    ToolbarItemComponent* getItemComponent (const int itemIndex) const throw();

    /** Clears this toolbar and adds to it the default set of items that the specified
        factory creates.

        @see ToolbarItemFactory::getDefaultItemSet
    */
    void addDefaultItems (ToolbarItemFactory& factoryToUse);

    //==============================================================================
    /** Options for the way items should be displayed.
        @see setStyle, getStyle
    */
    enum ToolbarItemStyle
    {
        iconsOnly,       /**< Means that the toolbar should just contain icons. */
        iconsWithText,   /**< Means that the toolbar should have text labels under each icon. */
        textOnly         /**< Means that the toolbar only display text labels for each item. */
    };

    /** Returns the toolbar's current style.
        @see ToolbarItemStyle, setStyle
    */
    ToolbarItemStyle getStyle() const throw()                { return toolbarStyle; }

    /** Changes the toolbar's current style.
        @see ToolbarItemStyle, getStyle, ToolbarItemComponent::setStyle
    */
    void setStyle (const ToolbarItemStyle& newStyle);

    //==============================================================================
    /** Flags used by the showCustomisationDialog() method. */
    enum CustomisationFlags
    {
        allowIconsOnlyChoice            = 1,    /**< If this flag is specified, the customisation dialog can
                                                     show the "icons only" option on its choice of toolbar styles. */
        allowIconsWithTextChoice        = 2,    /**< If this flag is specified, the customisation dialog can
                                                     show the "icons with text" option on its choice of toolbar styles. */
        allowTextOnlyChoice             = 4,    /**< If this flag is specified, the customisation dialog can
                                                     show the "text only" option on its choice of toolbar styles. */
        showResetToDefaultsButton       = 8,    /**< If this flag is specified, the customisation dialog can
                                                     show a button to reset the toolbar to its default set of items. */

        allCustomisationOptionsEnabled = (allowIconsOnlyChoice | allowIconsWithTextChoice | allowTextOnlyChoice | showResetToDefaultsButton)
    };

    /** Pops up a modal dialog box that allows this toolbar to be customised by the user.

        The dialog contains a ToolbarItemPalette and various controls for editing other
        aspects of the toolbar. This method will block and run the dialog box modally,
        returning when the user closes it.

        The factory is used to determine the set of items that will be shown on the
        palette.

        The optionFlags parameter is a bitwise-or of values from the CustomisationFlags
        enum.

        @see ToolbarItemPalette
    */
    void showCustomisationDialog (ToolbarItemFactory& factory,
                                  const int optionFlags = allCustomisationOptionsEnabled);

    /** Turns on or off the toolbar's editing mode, in which its items can be
        rearranged by the user.

        (In most cases it's easier just to use showCustomisationDialog() instead of
        trying to enable editing directly).

        @see ToolbarItemPalette
    */
    void setEditingActive (const bool editingEnabled);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the toolbar.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId          = 0x1003200,  /**< A colour to use to fill the toolbar's background. For
                                                       more control over this, override LookAndFeel::paintToolbarBackground(). */
        separatorColourId           = 0x1003210,  /**< A colour to use to draw the separator lines. */

        buttonMouseOverBackgroundColourId = 0x1003220,  /**< A colour used to paint the background of buttons when the mouse is
                                                             over them. */
        buttonMouseDownBackgroundColourId = 0x1003230,  /**< A colour used to paint the background of buttons when the mouse is
                                                             held down on them. */

        labelTextColourId           = 0x1003240,        /**< A colour to use for drawing the text under buttons
                                                             when the style is set to iconsWithText or textOnly. */

        editingModeOutlineColourId  = 0x1003250   /**< A colour to use for an outline around buttons when
                                                       the customisation dialog is active and the mouse moves over them. */
    };

    //==============================================================================
    /** Returns a string that represents the toolbar's current set of items.

        This lets you later restore the same item layout using restoreFromString().

        @see restoreFromString
    */
    const String toString() const;

    /** Restores a set of items that was previously stored in a string by the toString()
        method.

        The factory object is used to create any item components that are needed.

        @see toString
    */
    bool restoreFromString (ToolbarItemFactory& factoryToUse,
                            const String& savedVersion);

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void resized();
    /** @internal */
    void buttonClicked (Button*);
    /** @internal */
    void mouseDown (const MouseEvent&);
    /** @internal */
    bool isInterestedInDragSource (const String&, Component*);
    /** @internal */
    void itemDragMove (const String&, Component*, int, int);
    /** @internal */
    void itemDragExit (const String&, Component*);
    /** @internal */
    void itemDropped (const String&, Component*, int, int);
    /** @internal */
    void updateAllItemPositions (const bool animate);
    /** @internal */
    static ToolbarItemComponent* createItem (ToolbarItemFactory&, const int itemId);

    juce_UseDebuggingNewOperator

private:
    Button* missingItemsButton;
    bool vertical, isEditingActive;
    ToolbarItemStyle toolbarStyle;
    ComponentAnimator animator;
    friend class MissingItemsComponent;
    Array <ToolbarItemComponent*> items;

    friend class ItemDragAndDropOverlayComponent;
    static const tchar* const toolbarDragDescriptor;

    void addItemInternal (ToolbarItemFactory& factory, const int itemId, const int insertIndex);

    ToolbarItemComponent* getNextActiveComponent (int index, const int delta) const;

    Toolbar (const Toolbar&);
    const Toolbar& operator= (const Toolbar&);
};


#endif   // __JUCE_TOOLBAR_JUCEHEADER__
