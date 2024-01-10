/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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

class ToolbarItemComponent;
class ToolbarItemFactory;


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

    @tags{GUI}
*/
class JUCE_API  Toolbar   : public Component,
                            public DragAndDropContainer,
                            public DragAndDropTarget
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
    ~Toolbar() override;

    //==============================================================================
    /** Changes the bar's orientation.
        @see isVertical
    */
    void setVertical (bool shouldBeVertical);

    /** Returns true if the bar is set to be vertical, or false if it's horizontal.

        You can change the bar's orientation with setVertical().
    */
    bool isVertical() const noexcept                 { return vertical; }

    /** Returns the depth of the bar.

        If the bar is horizontal, this will return its height; if it's vertical, it
        will return its width.

        @see getLength
    */
    int getThickness() const noexcept;

    /** Returns the length of the bar.

        If the bar is horizontal, this will return its width; if it's vertical, it
        will return its height.

        @see getThickness
    */
    int getLength() const noexcept;

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
                  int itemId,
                  int insertIndex = -1);

    /** Deletes one of the items from the bar. */
    void removeToolbarItem (int itemIndex);

    /** Removes an item from the bar and returns it. */
    ToolbarItemComponent* removeAndReturnItem (int itemIndex);

    /** Returns the number of items currently on the toolbar.

        @see getItemId, getItemComponent
    */
    int getNumItems() const noexcept;

    /** Returns the ID of the item with the given index.

        If the index is less than zero or greater than the number of items,
        this will return nullptr.

        @see getNumItems
    */
    int getItemId (int itemIndex) const noexcept;

    /** Returns the component being used for the item with the given index.

        If the index is less than zero or greater than the number of items,
        this will return nullptr.

        @see getNumItems
    */
    ToolbarItemComponent* getItemComponent (int itemIndex) const noexcept;

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
    ToolbarItemStyle getStyle() const noexcept               { return toolbarStyle; }

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
        aspects of the toolbar. The dialog box will be opened modally, but the method will
        return immediately.

        The factory is used to determine the set of items that will be shown on the
        palette.

        The optionFlags parameter is a bitwise-or of values from the CustomisationFlags
        enum.

        @see ToolbarItemPalette
    */
    void showCustomisationDialog (ToolbarItemFactory& factory,
                                  int optionFlags = allCustomisationOptionsEnabled);

    /** Turns on or off the toolbar's editing mode, in which its items can be
        rearranged by the user.

        (In most cases it's easier just to use showCustomisationDialog() instead of
        trying to enable editing directly).

        @see ToolbarItemPalette
    */
    void setEditingActive (bool editingEnabled);

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

        editingModeOutlineColourId  = 0x1003250,  /**< A colour to use for an outline around buttons when
                                                       the customisation dialog is active and the mouse moves over them. */

        customisationDialogBackgroundColourId = 0x1003260 /**< A colour used to paint the background of the CustomisationDialog. */
    };

    //==============================================================================
    /** Returns a string that represents the toolbar's current set of items.

        This lets you later restore the same item layout using restoreFromString().

        @see restoreFromString
    */
    String toString() const;

    /** Restores a set of items that was previously stored in a string by the toString()
        method.

        The factory object is used to create any item components that are needed.

        @see toString
    */
    bool restoreFromString (ToolbarItemFactory& factoryToUse,
                            const String& savedVersion);

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual void paintToolbarBackground (Graphics&, int width, int height, Toolbar&) = 0;

        virtual Button* createToolbarMissingItemsButton (Toolbar&) = 0;

        virtual void paintToolbarButtonBackground (Graphics&, int width, int height,
                                                   bool isMouseOver, bool isMouseDown,
                                                   ToolbarItemComponent&) = 0;

        virtual void paintToolbarButtonLabel (Graphics&, int x, int y, int width, int height,
                                              const String& text, ToolbarItemComponent&) = 0;
    };

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void mouseDown (const MouseEvent&) override;
    /** @internal */
    bool isInterestedInDragSource (const SourceDetails&) override;
    /** @internal */
    void itemDragMove (const SourceDetails&) override;
    /** @internal */
    void itemDragExit (const SourceDetails&) override;
    /** @internal */
    void itemDropped (const SourceDetails&) override;
    /** @internal */
    void lookAndFeelChanged() override;
    /** @internal */
    void updateAllItemPositions (bool animate);
    /** @internal */
    static ToolbarItemComponent* createItem (ToolbarItemFactory&, int itemId);
    /** @internal */
    static const char* const toolbarDragDescriptor;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    std::unique_ptr<Button> missingItemsButton;
    bool vertical = false, isEditingActive = false;
    ToolbarItemStyle toolbarStyle = iconsOnly;
    class MissingItemsComponent;
    friend class MissingItemsComponent;
    OwnedArray<ToolbarItemComponent> items;
    class Spacer;
    class CustomisationDialog;

    void initMissingItemButton();
    void showMissingItems();
    void addItemInternal (ToolbarItemFactory& factory, int itemId, int insertIndex);

    ToolbarItemComponent* getNextActiveComponent (int index, int delta) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Toolbar)
};

} // namespace juce
