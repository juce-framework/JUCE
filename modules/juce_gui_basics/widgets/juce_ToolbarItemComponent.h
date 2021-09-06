/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

//==============================================================================
/**
    A component that can be used as one of the items in a Toolbar.

    Each of the items on a toolbar must be a component derived from ToolbarItemComponent,
    and these objects are always created by a ToolbarItemFactory - see the ToolbarItemFactory
    class for further info about creating them.

    The ToolbarItemComponent class is actually a button, but can be used to hold non-button
    components too. To do this, set the value of isBeingUsedAsAButton to false when
    calling the constructor, and override contentAreaChanged(), in which you can position
    any sub-components you need to add.

    To add basic buttons without writing a special subclass, have a look at the
    ToolbarButton class.

    @see ToolbarButton, Toolbar, ToolbarItemFactory

    @tags{GUI}
*/
class JUCE_API  ToolbarItemComponent  : public Button
{
public:
    //==============================================================================
    /** Constructor.

        @param itemId       the ID of the type of toolbar item which this represents
        @param labelText    the text to display if the toolbar's style is set to
                            Toolbar::iconsWithText or Toolbar::textOnly
        @param isBeingUsedAsAButton     set this to false if you don't want the button
                            to draw itself with button over/down states when the mouse
                            moves over it or clicks
    */
    ToolbarItemComponent (int itemId,
                          const String& labelText,
                          bool isBeingUsedAsAButton);

    /** Destructor. */
    ~ToolbarItemComponent() override;

    //==============================================================================
    /** Returns the item type ID that this component represents.
        This value is in the constructor.
    */
    int getItemId() const noexcept                                  { return itemId; }

    /** Returns the toolbar that contains this component, or nullptr if it's not currently
        inside one.
    */
    Toolbar* getToolbar() const;

    /** Returns true if this component is currently inside a toolbar which is vertical.
        @see Toolbar::isVertical
    */
    bool isToolbarVertical() const;

    /** Returns the current style setting of this item.

        Styles are listed in the Toolbar::ToolbarItemStyle enum.
        @see setStyle, Toolbar::getStyle
    */
    Toolbar::ToolbarItemStyle getStyle() const noexcept             { return toolbarStyle; }

    /** Changes the current style setting of this item.

        Styles are listed in the Toolbar::ToolbarItemStyle enum, and are automatically updated
        by the toolbar that holds this item.

        @see setStyle, Toolbar::setStyle
    */
    virtual void setStyle (const Toolbar::ToolbarItemStyle& newStyle);

    /** Returns the area of the component that should be used to display the button image or
        other contents of the item.

        This content area may change when the item's style changes, and may leave a space around the
        edge of the component where the text label can be shown.

        @see contentAreaChanged
    */
    Rectangle<int> getContentArea() const noexcept                  { return contentArea; }

    //==============================================================================
    /** This method must return the size criteria for this item, based on a given toolbar
        size and orientation.

        The preferredSize, minSize and maxSize values must all be set by your implementation
        method. If the toolbar is horizontal, these will be the width of the item; for a vertical
        toolbar, they refer to the item's height.

        The preferredSize is the size that the component would like to be, and this must be
        between the min and max sizes. For a fixed-size item, simply set all three variables to
        the same value.

        The toolbarThickness parameter tells you the depth of the toolbar - the same as calling
        Toolbar::getThickness().

        The isToolbarVertical parameter tells you whether the bar is oriented horizontally or
        vertically.
    */
    virtual bool getToolbarItemSizes (int toolbarThickness,
                                      bool isToolbarVertical,
                                      int& preferredSize,
                                      int& minSize,
                                      int& maxSize) = 0;

    /** Your subclass should use this method to draw its content area.

        The graphics object that is passed-in will have been clipped and had its origin
        moved to fit the content area as specified get getContentArea(). The width and height
        parameters are the width and height of the content area.

        If the component you're writing isn't a button, you can just do nothing in this method.
    */
    virtual void paintButtonArea (Graphics& g,
                                  int width, int height,
                                  bool isMouseOver, bool isMouseDown) = 0;

    /** Callback to indicate that the content area of this item has changed.

        This might be because the component was resized, or because the style changed and
        the space needed for the text label is different.

        See getContentArea() for a description of what the area is.
    */
    virtual void contentAreaChanged (const Rectangle<int>& newBounds) = 0;


    //==============================================================================
    /** Editing modes.
        These are used by setEditingMode(), but will be rarely needed in user code.
    */
    enum ToolbarEditingMode
    {
        normalMode = 0,     /**< Means that the component is active, inside a toolbar. */
        editableOnToolbar,  /**< Means that the component is on a toolbar, but the toolbar is in
                                 customisation mode, and the items can be dragged around. */
        editableOnPalette   /**< Means that the component is on an new-item palette, so it can be
                                 dragged onto a toolbar to add it to that bar.*/
    };

    /** Changes the editing mode of this component.

        This is used by the ToolbarItemPalette and related classes for making the items draggable,
        and is unlikely to be of much use in end-user-code.
    */
    void setEditingMode (const ToolbarEditingMode newMode);

    /** Returns the current editing mode of this component.

        This is used by the ToolbarItemPalette and related classes for making the items draggable,
        and is unlikely to be of much use in end-user-code.
    */
    ToolbarEditingMode getEditingMode() const noexcept                  { return mode; }


    //==============================================================================
    /** @internal */
    void paintButton (Graphics&, bool isMouseOver, bool isMouseDown) override;
    /** @internal */
    void resized() override;

private:
    friend class Toolbar;
    class ItemDragAndDropOverlayComponent;
    friend class ItemDragAndDropOverlayComponent;

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

    const int itemId;
    ToolbarEditingMode mode;
    Toolbar::ToolbarItemStyle toolbarStyle;
    std::unique_ptr<Component> overlayComp;
    int dragOffsetX, dragOffsetY;
    bool isActive, isBeingDragged, isBeingUsedAsAButton;
    Rectangle<int> contentArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolbarItemComponent)
};

} // namespace juce
