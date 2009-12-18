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

#ifndef __JUCE_TREEVIEW_JUCEHEADER__
#define __JUCE_TREEVIEW_JUCEHEADER__

#include "../layout/juce_Viewport.h"
#include "../../../text/juce_XmlElement.h"
#include "../../../events/juce_AsyncUpdater.h"
#include "../mouse/juce_FileDragAndDropTarget.h"
#include "../mouse/juce_DragAndDropTarget.h"
class TreeView;


//==============================================================================
/**
    An item in a treeview.

    A TreeViewItem can either be a leaf-node in the tree, or it can contain its
    own sub-items.

    To implement an item that contains sub-items, override the itemOpennessChanged()
    method so that when it is opened, it adds the new sub-items to itself using the
    addSubItem method. Depending on the nature of the item it might choose to only
    do this the first time it's opened, or it might want to refresh itself each time.
    It also has the option of deleting its sub-items when it is closed, or leaving them
    in place.
*/
class JUCE_API  TreeViewItem
{
public:
    //==============================================================================
    /** Constructor. */
    TreeViewItem();

    /** Destructor. */
    virtual ~TreeViewItem();

    //==============================================================================
    /** Returns the number of sub-items that have been added to this item.

        Note that this doesn't mean much if the node isn't open.

        @see getSubItem, mightContainSubItems, addSubItem
    */
    int getNumSubItems() const throw();

    /** Returns one of the item's sub-items.

        Remember that the object returned might get deleted at any time when its parent
        item is closed or refreshed, depending on the nature of the items you're using.

        @see getNumSubItems
    */
    TreeViewItem* getSubItem (const int index) const throw();

    /** Removes any sub-items. */
    void clearSubItems();

    /** Adds a sub-item.

        @param newItem  the object to add to the item's sub-item list. Once added, these can be
                        found using getSubItem(). When the items are later removed with
                        removeSubItem() (or when this item is deleted), they will be deleted.
        @param insertPosition   the index which the new item should have when it's added. If this
                                value is less than 0, the item will be added to the end of the list.
    */
    void addSubItem (TreeViewItem* const newItem,
                     const int insertPosition = -1);

    /** Removes one of the sub-items.

        @param index        the item to remove
        @param deleteItem   if true, the item that is removed will also be deleted.
    */
    void removeSubItem (const int index,
                        const bool deleteItem = true);

    //==============================================================================
    /** Returns the TreeView to which this item belongs. */
    TreeView* getOwnerView() const throw()              { return ownerView; }

    /** Returns the item within which this item is contained. */
    TreeViewItem* getParentItem() const throw()         { return parentItem; }

    //==============================================================================
    /** True if this item is currently open in the treeview. */
    bool isOpen() const throw();

    /** Opens or closes the item.

        When opened or closed, the item's itemOpennessChanged() method will be called,
        and a subclass should use this callback to create and add any sub-items that
        it needs to.

        @see itemOpennessChanged, mightContainSubItems
    */
    void setOpen (const bool shouldBeOpen);

    /** True if this item is currently selected.

        Use this when painting the node, to decide whether to draw it as selected or not.
    */
    bool isSelected() const throw();

    /** Selects or deselects the item.

        This will cause a callback to itemSelectionChanged()
    */
    void setSelected (const bool shouldBeSelected,
                      const bool deselectOtherItemsFirst);

    /** Returns the rectangle that this item occupies.

        If relativeToTreeViewTopLeft is true, the co-ordinates are relative to the
        top-left of the TreeView comp, so this will depend on the scroll-position of
        the tree. If false, it is relative to the top-left of the topmost item in the
        tree (so this would be unaffected by scrolling the view).
    */
    const Rectangle getItemPosition (const bool relativeToTreeViewTopLeft) const throw();

    /** Sends a signal to the treeview to make it refresh itself.

        Call this if your items have changed and you want the tree to update to reflect
        this.
    */
    void treeHasChanged() const throw();

    /** Sends a repaint message to redraw just this item.

        Note that you should only call this if you want to repaint a superficial change. If
        you're altering the tree's nodes, you should instead call treeHasChanged().
    */
    void repaintItem() const;

    /** Returns the row number of this item in the tree.

        The row number of an item will change according to which items are open.

        @see TreeView::getNumRowsInTree(), TreeView::getItemOnRow()
    */
    int getRowNumberInTree() const throw();

    /** Returns true if all the item's parent nodes are open.

        This is useful to check whether the item might actually be visible or not.
    */
    bool areAllParentsOpen() const throw();

    /** Changes whether lines are drawn to connect any sub-items to this item.

        By default, line-drawing is turned on.
    */
    void setLinesDrawnForSubItems (const bool shouldDrawLines) throw();

    //==============================================================================
    /** Tells the tree whether this item can potentially be opened.

        If your item could contain sub-items, this should return true; if it returns
        false then the tree will not try to open the item. This determines whether or
        not the item will be drawn with a 'plus' button next to it.
    */
    virtual bool mightContainSubItems() = 0;

    /** Returns a string to uniquely identify this item.

        If you're planning on using the TreeView::getOpennessState() method, then
        these strings will be used to identify which nodes are open. The string
        should be unique amongst the item's sibling items, but it's ok for there
        to be duplicates at other levels of the tree.

        If you're not going to store the state, then it's ok not to bother implementing
        this method.
    */
    virtual const String getUniqueName() const;

    /** Called when an item is opened or closed.

        When setOpen() is called and the item has specified that it might
        have sub-items with the mightContainSubItems() method, this method
        is called to let the item create or manage its sub-items.

        So when this is called with isNowOpen set to true (i.e. when the item is being
        opened), a subclass might choose to use clearSubItems() and addSubItem() to
        refresh its sub-item list.

        When this is called with isNowOpen set to false, the subclass might want
        to use clearSubItems() to save on space, or it might choose to leave them,
        depending on the nature of the tree.

        You could also use this callback as a trigger to start a background process
        which asynchronously creates sub-items and adds them, if that's more
        appropriate for the task in hand.

        @see mightContainSubItems
    */
    virtual void itemOpennessChanged (bool isNowOpen);

    /** Must return the width required by this item.

        If your item needs to have a particular width in pixels, return that value; if
        you'd rather have it just fill whatever space is available in the treeview,
        return -1.

        If all your items return -1, no horizontal scrollbar will be shown, but if any
        items have fixed widths and extend beyond the width of the treeview, a
        scrollbar will appear.

        Each item can be a different width, but if they change width, you should call
        treeHasChanged() to update the tree.
    */
    virtual int getItemWidth() const                                { return -1; }

    /** Must return the height required by this item.

        This is the height in pixels that the item will take up. Items in the tree
        can be different heights, but if they change height, you should call
        treeHasChanged() to update the tree.
    */
    virtual int getItemHeight() const                               { return 20; }

    /** You can override this method to return false if you don't want to allow the
        user to select this item.
    */
    virtual bool canBeSelected() const                              { return true; }

    /** Creates a component that will be used to represent this item.

        You don't have to implement this method - if it returns 0 then no component
        will be used for the item, and you can just draw it using the paintItem()
        callback. But if you do return a component, it will be positioned in the
        treeview so that it can be used to represent this item.

        The component returned will be managed by the treeview, so always return
        a new component, and don't keep a reference to it, as the treeview will
        delete it later when it goes off the screen or is no longer needed. Also
        bear in mind that if the component keeps a reference to the item that
        created it, that item could be deleted before the component. Its position
        and size will be completely managed by the tree, so don't attempt to move it
        around.

        Something you may want to do with your component is to give it a pointer to
        the TreeView that created it. This is perfectly safe, and there's no danger
        of it becoming a dangling pointer because the TreeView will always delete
        the component before it is itself deleted.

        As long as you stick to these rules you can return whatever kind of
        component you like. It's most useful if you're doing things like drag-and-drop
        of items, or want to use a Label component to edit item names, etc.
    */
    virtual Component* createItemComponent()                        { return 0; }

    //==============================================================================
    /** Draws the item's contents.

        You can choose to either implement this method and draw each item, or you
        can use createItemComponent() to create a component that will represent the
        item.

        If all you need in your tree is to be able to draw the items and detect when
        the user selects or double-clicks one of them, it's probably enough to
        use paintItem(), itemClicked() and itemDoubleClicked(). If you need more
        complicated interactions, you may need to use createItemComponent() instead.

        @param g        the graphics context to draw into
        @param width    the width of the area available for drawing
        @param height   the height of the area available for drawing
    */
    virtual void paintItem (Graphics& g, int width, int height);

    /** Draws the item's open/close button.

        If you don't implement this method, the default behaviour is to
        call LookAndFeel::drawTreeviewPlusMinusBox(), but you can override
        it for custom effects.
    */
    virtual void paintOpenCloseButton (Graphics& g, int width, int height, bool isMouseOver);

    /** Called when the user clicks on this item.

        If you're using createItemComponent() to create a custom component for the
        item, the mouse-clicks might not make it through to the treeview, but this
        is how you find out about clicks when just drawing each item individually.

        The associated mouse-event details are passed in, so you can find out about
        which button, where it was, etc.

        @see itemDoubleClicked
    */
    virtual void itemClicked (const MouseEvent& e);

    /** Called when the user double-clicks on this item.

        If you're using createItemComponent() to create a custom component for the
        item, the mouse-clicks might not make it through to the treeview, but this
        is how you find out about clicks when just drawing each item individually.

        The associated mouse-event details are passed in, so you can find out about
        which button, where it was, etc.

        If not overridden, the base class method here will open or close the item as
        if the 'plus' button had been clicked.

        @see itemClicked
    */
    virtual void itemDoubleClicked (const MouseEvent& e);

    /** Called when the item is selected or deselected.

        Use this if you want to do something special when the item's selectedness
        changes. By default it'll get repainted when this happens.
    */
    virtual void itemSelectionChanged (bool isNowSelected);

    /** The item can return a tool tip string here if it wants to.
        @see TooltipClient
    */
    virtual const String getTooltip();

    //==============================================================================
    /** To allow items from your treeview to be dragged-and-dropped, implement this method.

        If this returns a non-empty name then when the user drags an item, the treeview will
        try to find a DragAndDropContainer in its parent hierarchy, and will use it to trigger
        a drag-and-drop operation, using this string as the source description, with the treeview
        itself as the source component.

        If you need more complex drag-and-drop behaviour, you can use custom components for
        the items, and use those to trigger the drag.

        To accept drag-and-drop in your tree, see isInterestedInDragSource(),
        isInterestedInFileDrag(), etc.

        @see DragAndDropContainer::startDragging
    */
    virtual const String getDragSourceDescription();

    /** If you want your item to be able to have files drag-and-dropped onto it, implement this
        method and return true.

        If you return true and allow some files to be dropped, you'll also need to implement the
        filesDropped() method to do something with them.

        Note that this will be called often, so make your implementation very quick! There's
        certainly no time to try opening the files and having a think about what's inside them!

        For responding to internal drag-and-drop of other types of object, see isInterestedInDragSource().
        @see FileDragAndDropTarget::isInterestedInFileDrag, isInterestedInDragSource
    */
    virtual bool isInterestedInFileDrag (const StringArray& files);

    /** When files are dropped into this item, this callback is invoked.

        For this to work, you'll need to have also implemented isInterestedInFileDrag().
        The insertIndex value indicates where in the list of sub-items the files were dropped.
        @see FileDragAndDropTarget::filesDropped, isInterestedInFileDrag
    */
    virtual void filesDropped (const StringArray& files, int insertIndex);

    /** If you want your item to act as a DragAndDropTarget, implement this method and return true.

        If you implement this method, you'll also need to implement itemDropped() in order to handle
        the items when they are dropped.
        To respond to drag-and-drop of files from external applications, see isInterestedInFileDrag().
        @see DragAndDropTarget::isInterestedInDragSource, itemDropped
    */
    virtual bool isInterestedInDragSource (const String& sourceDescription, Component* sourceComponent);

    /** When a things are dropped into this item, this callback is invoked.

        For this to work, you need to have also implemented isInterestedInDragSource().
        The insertIndex value indicates where in the list of sub-items the new items should be placed.
        @see isInterestedInDragSource, DragAndDropTarget::itemDropped
    */
    virtual void itemDropped (const String& sourceDescription, Component* sourceComponent, int insertIndex);

    //==============================================================================
    /** Sets a flag to indicate that the item wants to be allowed
        to draw all the way across to the left edge of the treeview.

        By default this is false, which means that when the paintItem()
        method is called, its graphics context is clipped to only allow
        drawing within the item's rectangle. If this flag is set to true,
        then the graphics context isn't clipped on its left side, so it
        can draw all the way across to the left margin. Note that the
        context will still have its origin in the same place though, so
        the coordinates of anything to its left will be negative. It's
        mostly useful if you want to draw a wider bar behind the
        highlighted item.
    */
    void setDrawsInLeftMargin (bool canDrawInLeftMargin) throw();

    //==============================================================================
    /** Saves the current state of open/closed nodes so it can be restored later.

        This takes a snapshot of which sub-nodes have been explicitly opened or closed,
        and records it as XML. To identify node objects it uses the
        TreeViewItem::getUniqueName() method to create named paths. This
        means that the same state of open/closed nodes can be restored to a
        completely different instance of the tree, as long as it contains nodes
        whose unique names are the same.

        You'd normally want to use TreeView::getOpennessState() rather than call it
        for a specific item, but this can be handy if you need to briefly save the state
        for a section of the tree.

        The caller is responsible for deleting the object that is returned.
        @see TreeView::getOpennessState, restoreOpennessState
    */
    XmlElement* getOpennessState() const throw();

    /** Restores the openness of this item and all its sub-items from a saved state.

        See TreeView::restoreOpennessState for more details.

        You'd normally want to use TreeView::restoreOpennessState() rather than call it
        for a specific item, but this can be handy if you need to briefly save the state
        for a section of the tree.

        @see TreeView::restoreOpennessState, getOpennessState
    */
    void restoreOpennessState (const XmlElement& xml) throw();

    //==============================================================================
    /** Returns the index of this item in its parent's sub-items. */
    int getIndexInParent() const throw();

    /** Returns true if this item is the last of its parent's sub-itens. */
    bool isLastOfSiblings() const throw();

    /** Creates a string that can be used to uniquely retrieve this item in the tree.

        The string that is returned can be passed to TreeView::findItemFromIdentifierString().
        The string takes the form of a path, constructed from the getUniqueName() of this
        item and all its parents, so these must all be correctly implemented for it to work.
        @see TreeView::findItemFromIdentifierString, getUniqueName
    */
    const String getItemIdentifierString() const;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    TreeView* ownerView;
    TreeViewItem* parentItem;
    OwnedArray <TreeViewItem> subItems;
    int y, itemHeight, totalHeight, itemWidth, totalWidth;
    int uid;
    bool selected           : 1;
    bool redrawNeeded       : 1;
    bool drawLinesInside    : 1;
    bool drawsInLeftMargin  : 1;
    unsigned int openness   : 2;

    friend class TreeView;
    friend class TreeViewContentComponent;

    void updatePositions (int newY);
    int getIndentX() const throw();
    void setOwnerView (TreeView* const newOwner) throw();
    void paintRecursively (Graphics& g, int width);
    TreeViewItem* getTopLevelItem() throw();
    TreeViewItem* findItemRecursively (int y) throw();
    TreeViewItem* getDeepestOpenParentItem() throw();
    int getNumRows() const throw();
    TreeViewItem* getItemOnRow (int index) throw();
    void deselectAllRecursively();
    int countSelectedItemsRecursively() const throw();
    TreeViewItem* getSelectedItemWithIndex (int index) throw();
    TreeViewItem* getNextVisibleItem (const bool recurse) const throw();
    TreeViewItem* findItemFromIdentifierString (const String& identifierString);

    TreeViewItem (const TreeViewItem&);
    const TreeViewItem& operator= (const TreeViewItem&);
};


//==============================================================================
/**
    A tree-view component.

    Use one of these to hold and display a structure of TreeViewItem objects.

*/
class JUCE_API  TreeView  : public Component,
                            public SettableTooltipClient,
                            public FileDragAndDropTarget,
                            public DragAndDropTarget,
                            private AsyncUpdater
{
public:
    //==============================================================================
    /** Creates an empty treeview.

        Once you've got a treeview component, you'll need to give it something to
        display, using the setRootItem() method.
    */
    TreeView (const String& componentName = String::empty);

    /** Destructor. */
    ~TreeView();

    //==============================================================================
    /** Sets the item that is displayed in the treeview.

        A tree has a single root item which contains as many sub-items as it needs. If
        you want the tree to contain a number of root items, you should still use a single
        root item above these, but hide it using setRootItemVisible().

        You can pass in 0 to this method to clear the tree and remove its current root item.

        The object passed in will not be deleted by the treeview, it's up to the caller
        to delete it when no longer needed. BUT make absolutely sure that you don't delete
        this item until you've removed it from the tree, either by calling setRootItem (0),
        or by deleting the tree first. You can also use deleteRootItem() as a quick way
        to delete it.
    */
    void setRootItem (TreeViewItem* const newRootItem);

    /** Returns the tree's root item.

        This will be the last object passed to setRootItem(), or 0 if none has been set.
    */
    TreeViewItem* getRootItem() const throw()                       { return rootItem; }

    /** This will remove and delete the current root item.

        It's a convenient way of deleting the item and calling setRootItem (0).
    */
    void deleteRootItem();

    /** Changes whether the tree's root item is shown or not.

        If the root item is hidden, only its sub-items will be shown in the treeview - this
        lets you make the tree look as if it's got many root items. If it's hidden, this call
        will also make sure the root item is open (otherwise the treeview would look empty).
    */
    void setRootItemVisible (const bool shouldBeVisible);

    /** Returns true if the root item is visible.

        @see setRootItemVisible
    */
    bool isRootItemVisible() const throw()                          { return rootItemVisible; }

    /** Sets whether items are open or closed by default.

        Normally, items are closed until the user opens them, but you can use this
        to make them default to being open until explicitly closed.

        @see areItemsOpenByDefault
    */
    void setDefaultOpenness (const bool isOpenByDefault);

    /** Returns true if the tree's items default to being open.

        @see setDefaultOpenness
    */
    bool areItemsOpenByDefault() const throw()                      { return defaultOpenness; }

    /** This sets a flag to indicate that the tree can be used for multi-selection.

        You can always select multiple items internally by calling the
        TreeViewItem::setSelected() method, but this flag indicates whether the user
        is allowed to multi-select by clicking on the tree.

        By default it is disabled.

        @see isMultiSelectEnabled
    */
    void setMultiSelectEnabled (const bool canMultiSelect);

    /** Returns whether multi-select has been enabled for the tree.

        @see setMultiSelectEnabled
    */
    bool isMultiSelectEnabled() const throw()                       { return multiSelectEnabled; }

    /** Sets a flag to indicate whether to hide the open/close buttons.

        @see areOpenCloseButtonsVisible
    */
    void setOpenCloseButtonsVisible (const bool shouldBeVisible);

    /** Returns whether open/close buttons are shown.

        @see setOpenCloseButtonsVisible
    */
    bool areOpenCloseButtonsVisible() const throw()                 { return openCloseButtonsVisible; }

    //==============================================================================
    /** Deselects any items that are currently selected. */
    void clearSelectedItems();

    /** Returns the number of items that are currently selected.

        @see getSelectedItem, clearSelectedItems
    */
    int getNumSelectedItems() const throw();

    /** Returns one of the selected items in the tree.

        @param index    the index, 0 to (getNumSelectedItems() - 1)
    */
    TreeViewItem* getSelectedItem (const int index) const throw();

    //==============================================================================
    /** Returns the number of rows the tree is using.

        This will depend on which items are open.

        @see TreeViewItem::getRowNumberInTree()
    */
    int getNumRowsInTree() const;

    /** Returns the item on a particular row of the tree.

        If the index is out of range, this will return 0.

        @see getNumRowsInTree, TreeViewItem::getRowNumberInTree()
    */
    TreeViewItem* getItemOnRow (int index) const;

    /** Returns the item that contains a given y position.
        The y is relative to the top of the TreeView component.
    */
    TreeViewItem* getItemAt (int yPosition) const throw();

    /** Tries to scroll the tree so that this item is on-screen somewhere. */
    void scrollToKeepItemVisible (TreeViewItem* item);

    /** Returns the treeview's Viewport object. */
    Viewport* getViewport() const throw()                           { return viewport; }

    /** Returns the number of pixels by which each nested level of the tree is indented.
        @see setIndentSize
    */
    int getIndentSize() const throw()                               { return indentSize; }

    /** Changes the distance by which each nested level of the tree is indented.
        @see getIndentSize
    */
    void setIndentSize (const int newIndentSize);

    /** Searches the tree for an item with the specified identifier.
        The identifer string must have been created by calling TreeViewItem::getItemIdentifierString().
        If no such item exists, this will return false. If the item is found, all of its items
        will be automatically opened.
    */
    TreeViewItem* findItemFromIdentifierString (const String& identifierString) const;

    //==============================================================================
    /** Saves the current state of open/closed nodes so it can be restored later.

        This takes a snapshot of which nodes have been explicitly opened or closed,
        and records it as XML. To identify node objects it uses the
        TreeViewItem::getUniqueName() method to create named paths. This
        means that the same state of open/closed nodes can be restored to a
        completely different instance of the tree, as long as it contains nodes
        whose unique names are the same.

        The caller is responsible for deleting the object that is returned.

        @param alsoIncludeScrollPosition    if this is true, the state will also
                                            include information about where the
                                            tree has been scrolled to vertically,
                                            so this can also be restored
        @see restoreOpennessState
    */
    XmlElement* getOpennessState (const bool alsoIncludeScrollPosition) const;

    /** Restores a previously saved arrangement of open/closed nodes.

        This will try to restore a snapshot of the tree's state that was created by
        the getOpennessState() method. If any of the nodes named in the original
        XML aren't present in this tree, they will be ignored.

        @see getOpennessState
    */
    void restoreOpennessState (const XmlElement& newState);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the treeview.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId            = 0x1000500, /**< A background colour to fill the component with. */
        linesColourId                 = 0x1000501, /**< The colour to draw the lines with.*/
        dragAndDropIndicatorColourId  = 0x1000502  /**< The colour to use for the drag-and-drop target position indicator. */
    };

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void resized();
    /** @internal */
    bool keyPressed (const KeyPress& key);
    /** @internal */
    void colourChanged();
    /** @internal */
    void enablementChanged();
    /** @internal */
    bool isInterestedInFileDrag (const StringArray& files);
    /** @internal */
    void fileDragEnter (const StringArray& files, int x, int y);
    /** @internal */
    void fileDragMove (const StringArray& files, int x, int y);
    /** @internal */
    void fileDragExit (const StringArray& files);
    /** @internal */
    void filesDropped (const StringArray& files, int x, int y);
    /** @internal */
    bool isInterestedInDragSource (const String& sourceDescription, Component* sourceComponent);
    /** @internal */
    void itemDragEnter (const String& sourceDescription, Component* sourceComponent, int x, int y);
    /** @internal */
    void itemDragMove (const String& sourceDescription, Component* sourceComponent, int x, int y);
    /** @internal */
    void itemDragExit (const String& sourceDescription, Component* sourceComponent);
    /** @internal */
    void itemDropped (const String& sourceDescription, Component* sourceComponent, int x, int y);

    juce_UseDebuggingNewOperator

private:
    friend class TreeViewItem;
    friend class TreeViewContentComponent;
    Viewport* viewport;
    CriticalSection nodeAlterationLock;
    TreeViewItem* rootItem;
    Component* dragInsertPointHighlight;
    Component* dragTargetGroupHighlight;
    int indentSize;
    bool defaultOpenness : 1;
    bool needsRecalculating : 1;
    bool rootItemVisible : 1;
    bool multiSelectEnabled : 1;
    bool openCloseButtonsVisible : 1;

    void itemsChanged() throw();
    void handleAsyncUpdate();
    void moveSelectedRow (int delta);
    void updateButtonUnderMouse (const MouseEvent& e);
    void showDragHighlight (TreeViewItem* item, int insertIndex, int x, int y) throw();
    void hideDragHighlight() throw();
    void handleDrag (const StringArray& files, const String& sourceDescription, Component* sourceComponent, int x, int y);
    void handleDrop (const StringArray& files, const String& sourceDescription, Component* sourceComponent, int x, int y);
    TreeViewItem* getInsertPosition (int& x, int& y, int& insertIndex,
                                     const StringArray& files, const String& sourceDescription,
                                     Component* sourceComponent) const throw();

    TreeView (const TreeView&);
    const TreeView& operator= (const TreeView&);
};

#endif   // __JUCE_TREEVIEW_JUCEHEADER__
