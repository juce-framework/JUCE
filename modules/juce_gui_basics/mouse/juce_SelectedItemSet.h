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
/** Manages a list of selectable items.

    Use one of these to keep a track of things that the user has highlighted, like
    icons or things in a list.

    The class is templated so that you can use it to hold either a set of pointers
    to objects, or a set of ID numbers or handles, for cases where each item may
    not always have a corresponding object.

    To be informed when items are selected/deselected, register a ChangeListener with
    this object.

    @tags{GUI}
*/
template <class SelectableItemType>
class SelectedItemSet   : public ChangeBroadcaster
{
public:
    //==============================================================================
    using ItemType = SelectableItemType;
    using ItemArray = Array<SelectableItemType>;
    using ParameterType = typename TypeHelpers::ParameterType<SelectableItemType>::type;

    //==============================================================================
    /** Creates an empty set. */
    SelectedItemSet() = default;

    /** Creates a set based on an array of items. */
    explicit SelectedItemSet (const ItemArray& items)
        : selectedItems (items)
    {
    }

    /** Creates a copy of another set. */
    SelectedItemSet (const SelectedItemSet& other)
        : ChangeBroadcaster(), selectedItems (other.selectedItems)
    {
    }

    /** Creates a copy of another set. */
    SelectedItemSet& operator= (const SelectedItemSet& other)
    {
        if (selectedItems != other.selectedItems)
        {
            changed();

            for (int i = selectedItems.size(); --i >= 0;)
                if (! other.isSelected (selectedItems.getReference (i)))
                    itemDeselected (selectedItems.removeAndReturn (i));

            for (auto& i : other.selectedItems)
            {
                if (! isSelected (i))
                {
                    selectedItems.add (i);
                    itemSelected (i);
                }
            }
        }

        return *this;
    }

    //==============================================================================
    /** Clears any other currently selected items, and selects this item.

        If this item is already the only thing selected, no change notification
        will be sent out.

        @see addToSelection, addToSelectionBasedOnModifiers
    */
    void selectOnly (ParameterType item)
    {
        if (isSelected (item))
        {
            for (int i = selectedItems.size(); --i >= 0;)
            {
                if (selectedItems.getUnchecked(i) != item)
                {
                    deselect (selectedItems.getUnchecked(i));
                    i = jmin (i, selectedItems.size());
                }
            }
        }
        else
        {
            changed();
            deselectAll();

            selectedItems.add (item);
            itemSelected (item);
        }
    }

    /** Selects an item.
        If the item is already selected, no change notification will be sent out.
        @see selectOnly, addToSelectionBasedOnModifiers
    */
    void addToSelection (ParameterType item)
    {
        if (! isSelected (item))
        {
            changed();

            selectedItems.add (item);
            itemSelected (item);
        }
    }

    /** Selects or deselects an item.

        This will use the modifier keys to decide whether to deselect other items
        first.

        So if the shift key is held down, the item will be added without deselecting
        anything (same as calling addToSelection() )

        If no modifiers are down, the current selection will be cleared first (same
        as calling selectOnly() )

        If the ctrl (or command on the Mac) key is held down, the item will be toggled -
        so it'll be added to the set unless it's already there, in which case it'll be
        deselected.

        If the items that you're selecting can also be dragged, you may need to use the
        addToSelectionOnMouseDown() and addToSelectionOnMouseUp() calls to handle the
        subtleties of this kind of usage.

        @see selectOnly, addToSelection, addToSelectionOnMouseDown, addToSelectionOnMouseUp
    */
    void addToSelectionBasedOnModifiers (ParameterType item,
                                         ModifierKeys modifiers)
    {
        if (modifiers.isShiftDown())
        {
            addToSelection (item);
        }
        else if (modifiers.isCommandDown())
        {
            if (isSelected (item))
                deselect (item);
            else
                addToSelection (item);
        }
        else
        {
            selectOnly (item);
        }
    }

    /** Selects or deselects items that can also be dragged, based on a mouse-down event.

        If you call addToSelectionOnMouseDown() at the start of your mouseDown event,
        and then call addToSelectionOnMouseUp() at the end of your mouseUp event, this
        makes it easy to handle multiple-selection of sets of objects that can also
        be dragged.

        For example, if you have several items already selected, and you click on
        one of them (without dragging), then you'd expect this to deselect the other, and
        just select the item you clicked on. But if you had clicked on this item and
        dragged it, you'd have expected them all to stay selected.

        When you call this method, you'll need to store the boolean result, because the
        addToSelectionOnMouseUp() method will need to be know this value.

        @see addToSelectionOnMouseUp, addToSelectionBasedOnModifiers
    */
    bool addToSelectionOnMouseDown (ParameterType item,
                                    ModifierKeys modifiers)
    {
        if (isSelected (item))
            return ! modifiers.isPopupMenu();

        addToSelectionBasedOnModifiers (item, modifiers);
        return false;
    }

    /** Selects or deselects items that can also be dragged, based on a mouse-up event.

        Call this during a mouseUp callback, when you have previously called the
        addToSelectionOnMouseDown() method during your mouseDown event.

        See addToSelectionOnMouseDown() for more info

        @param item             the item to select (or deselect)
        @param modifiers        the modifiers from the mouse-up event
        @param wasItemDragged   true if your item was dragged during the mouse click
        @param resultOfMouseDownSelectMethod    this is the boolean return value that came
                                back from the addToSelectionOnMouseDown() call that you
                                should have made during the matching mouseDown event
    */
    void addToSelectionOnMouseUp (ParameterType item,
                                  ModifierKeys modifiers,
                                  const bool wasItemDragged,
                                  const bool resultOfMouseDownSelectMethod)
    {
        if (resultOfMouseDownSelectMethod && ! wasItemDragged)
            addToSelectionBasedOnModifiers (item, modifiers);
    }

    /** Deselects an item. */
    void deselect (ParameterType item)
    {
        const int i = selectedItems.indexOf (item);

        if (i >= 0)
        {
            changed();
            itemDeselected (selectedItems.removeAndReturn (i));
        }
    }

    /** Deselects all items. */
    void deselectAll()
    {
        if (selectedItems.size() > 0)
        {
            changed();

            for (int i = selectedItems.size(); --i >= 0;)
            {
                itemDeselected (selectedItems.removeAndReturn (i));
                i = jmin (i, selectedItems.size());
            }
        }
    }

    //==============================================================================
    /** Returns the number of currently selected items.
        @see getSelectedItem
    */
    int getNumSelected() const noexcept                         { return selectedItems.size(); }

    /** Returns one of the currently selected items.
        If the index is out-of-range, this returns a default-constructed SelectableItemType.
        @see getNumSelected
    */
    SelectableItemType getSelectedItem (const int index) const  { return selectedItems [index]; }

    /** True if this item is currently selected. */
    bool isSelected (ParameterType item) const noexcept         { return selectedItems.contains (item); }

    /** Provides access to the array of items. */
    const ItemArray& getItemArray() const noexcept              { return selectedItems; }

    /** Provides iterator access to the array of items. */
    SelectableItemType* begin() noexcept                        { return selectedItems.begin(); }

    const SelectableItemType* begin() const noexcept            { return selectedItems.begin(); }

    /** Provides iterator access to the array of items. */
    SelectableItemType* end() noexcept                          { return selectedItems.end(); }

    /** Provides iterator access to the array of items. */
    const SelectableItemType* end() const noexcept              { return selectedItems.end(); }

    //==============================================================================
    /** Can be overridden to do special handling when an item is selected.

        For example, if the item is an object, you might want to call it and tell
        it that it's being selected.
    */
    virtual void itemSelected (SelectableItemType)              {}

    /** Can be overridden to do special handling when an item is deselected.

        For example, if the item is an object, you might want to call it and tell
        it that it's being deselected.
    */
    virtual void itemDeselected (SelectableItemType)            {}

    /** Used internally, but can be called to force a change message to be sent
        to the ChangeListeners.
    */
    void changed()
    {
        sendChangeMessage();
    }

    /** Used internally, but can be called to force a change message to be sent
        to the ChangeListeners.
    */
    void changed (const bool synchronous)
    {
        if (synchronous)
            sendSynchronousChangeMessage();
        else
            sendChangeMessage();
    }

private:
    //==============================================================================
    ItemArray selectedItems;

    JUCE_LEAK_DETECTOR (SelectedItemSet<SelectableItemType>)
};

} // namespace juce
