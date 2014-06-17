/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_COMBOBOX_H_INCLUDED
#define JUCE_COMBOBOX_H_INCLUDED


//==============================================================================
/**
    A component that lets the user choose from a drop-down list of choices.

    The combo-box has a list of text strings, each with an associated id number,
    that will be shown in the drop-down list when the user clicks on the component.

    The currently selected choice is displayed in the combo-box, and this can
    either be read-only text, or editable.

    To find out when the user selects a different item or edits the text, you
    can register a ComboBox::Listener to receive callbacks.

    @see ComboBox::Listener
*/
class JUCE_API  ComboBox  : public Component,
                            public SettableTooltipClient,
                            public LabelListener,  // (can't use Label::Listener due to idiotic VC2005 bug)
                            public ValueListener,
                            private AsyncUpdater
{
public:
    //==============================================================================
    /** Creates a combo-box.

        On construction, the text field will be empty, so you should call the
        setSelectedId() or setText() method to choose the initial value before
        displaying it.

        @param componentName    the name to set for the component (see Component::setName())
    */
    explicit ComboBox (const String& componentName = String::empty);

    /** Destructor. */
    ~ComboBox();

    //==============================================================================
    /** Sets whether the test in the combo-box is editable.

        The default state for a new ComboBox is non-editable, and can only be changed
        by choosing from the drop-down list.
    */
    void setEditableText (bool isEditable);

    /** Returns true if the text is directly editable.
        @see setEditableText
    */
    bool isTextEditable() const noexcept;

    /** Sets the style of justification to be used for positioning the text.

        The default is Justification::centredLeft. The text is displayed using a
        Label component inside the ComboBox.
    */
    void setJustificationType (Justification justification);

    /** Returns the current justification for the text box.
        @see setJustificationType
    */
    Justification getJustificationType() const noexcept;

    //==============================================================================
    /** Adds an item to be shown in the drop-down list.

        @param newItemText      the text of the item to show in the list
        @param newItemId        an associated ID number that can be set or retrieved - see
                                getSelectedId() and setSelectedId(). Note that this value can not
                                be 0!
        @see setItemEnabled, addSeparator, addSectionHeading, getNumItems, getItemText, getItemId
    */
    void addItem (const String& newItemText, int newItemId);

    /** Adds an array of items to the drop-down list.
        The item ID of each item will be its index in the StringArray + firstItemIdOffset.
    */
    void addItemList (const StringArray& items, int firstItemIdOffset);

    /** Adds a separator line to the drop-down list.

        This is like adding a separator to a popup menu. See PopupMenu::addSeparator().
    */
    void addSeparator();

    /** Adds a heading to the drop-down list, so that you can group the items into
        different sections.

        The headings are indented slightly differently to set them apart from the
        items on the list, and obviously can't be selected. You might want to add
        separators between your sections too.

        @see addItem, addSeparator
    */
    void addSectionHeading (const String& headingName);

    /** This allows items in the drop-down list to be selectively disabled.

        When you add an item, it's enabled by default, but you can call this
        method to change its status.

        If you disable an item which is already selected, this won't change the
        current selection - it just stops the user choosing that item from the list.
    */
    void setItemEnabled (int itemId, bool shouldBeEnabled);

    /** Returns true if the given item is enabled. */
    bool isItemEnabled (int itemId) const noexcept;

    /** Changes the text for an existing item.
    */
    void changeItemText (int itemId, const String& newText);

    /** Removes all the items from the drop-down list.

        If this call causes the content to be cleared, and a change-message
        will be broadcast according to the notification parameter.

        @see addItem, getNumItems
    */
    void clear (NotificationType notification = sendNotificationAsync);

    /** Returns the number of items that have been added to the list.

        Note that this doesn't include headers or separators.
    */
    int getNumItems() const noexcept;

    /** Returns the text for one of the items in the list.
        Note that this doesn't include headers or separators.
        @param index    the item's index from 0 to (getNumItems() - 1)
    */
    String getItemText (int index) const;

    /** Returns the ID for one of the items in the list.
        Note that this doesn't include headers or separators.
        @param index    the item's index from 0 to (getNumItems() - 1)
    */
    int getItemId (int index) const noexcept;

    /** Returns the index in the list of a particular item ID.
        If no such ID is found, this will return -1.
    */
    int indexOfItemId (int itemId) const noexcept;

    //==============================================================================
    /** Returns the ID of the item that's currently shown in the box.

        If no item is selected, or if the text is editable and the user
        has entered something which isn't one of the items in the list, then
        this will return 0.

        @see setSelectedId, getSelectedItemIndex, getText
    */
    int getSelectedId() const noexcept;

    /** Returns a Value object that can be used to get or set the selected item's ID.

        You can call Value::referTo() on this object to make the combo box control
        another Value object.
    */
    Value& getSelectedIdAsValue()                       { return currentId; }

    /** Sets one of the items to be the current selection.

        This will set the ComboBox's text to that of the item that matches
        this ID.

        @param newItemId        the new item to select
        @param notification     determines the type of change notification that will
                                be sent to listeners if the value changes
        @see getSelectedId, setSelectedItemIndex, setText
    */
    void setSelectedId (int newItemId,
                        NotificationType notification = sendNotificationAsync);

    //==============================================================================
    /** Returns the index of the item that's currently shown in the box.

        If no item is selected, or if the text is editable and the user
        has entered something which isn't one of the items in the list, then
        this will return -1.

        @see setSelectedItemIndex, getSelectedId, getText
    */
    int getSelectedItemIndex() const;

    /** Sets one of the items to be the current selection.

        This will set the ComboBox's text to that of the item at the given
        index in the list.

        @param newItemIndex     the new item to select
        @param notification     determines the type of change notification that will
                                be sent to listeners if the value changes
        @see getSelectedItemIndex, setSelectedId, setText
    */
    void setSelectedItemIndex (int newItemIndex,
                               NotificationType notification = sendNotificationAsync);

    //==============================================================================
    /** Returns the text that is currently shown in the combo-box's text field.

        If the ComboBox has editable text, then this text may have been edited
        by the user; otherwise it will be one of the items from the list, or
        possibly an empty string if nothing was selected.

        @see setText, getSelectedId, getSelectedItemIndex
    */
    String getText() const;

    /** Sets the contents of the combo-box's text field.

        The text passed-in will be set as the current text regardless of whether
        it is one of the items in the list. If the current text isn't one of the
        items, then getSelectedId() will return -1, otherwise it wil return
        the approriate ID.

        @param newText          the text to select
        @param notification     determines the type of change notification that will
                                be sent to listeners if the text changes
        @see getText
    */
    void setText (const String& newText,
                  NotificationType notification = sendNotificationAsync);

    /** Programmatically opens the text editor to allow the user to edit the current item.

        This is the same effect as when the box is clicked-on.
        @see Label::showEditor();
    */
    void showEditor();

    /** Pops up the combo box's list.
        This is virtual so that you can override it with your own custom popup
        mechanism if you need some really unusual behaviour.
    */
    virtual void showPopup();

    //==============================================================================
    /**
        A class for receiving events from a ComboBox.

        You can register a ComboBox::Listener with a ComboBox using the ComboBox::addListener()
        method, and it will be called when the selected item in the box changes.

        @see ComboBox::addListener, ComboBox::removeListener
    */
    class JUCE_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() {}

        /** Called when a ComboBox has its selected item changed. */
        virtual void comboBoxChanged (ComboBox* comboBoxThatHasChanged) = 0;
    };

    /** Registers a listener that will be called when the box's content changes. */
    void addListener (Listener* listener);

    /** Deregisters a previously-registered listener. */
    void removeListener (Listener* listener);

    //==============================================================================
    /** Sets a message to display when there is no item currently selected.
        @see getTextWhenNothingSelected
    */
    void setTextWhenNothingSelected (const String& newMessage);

    /** Returns the text that is shown when no item is selected.
        @see setTextWhenNothingSelected
    */
    String getTextWhenNothingSelected() const;

    /** Sets the message to show when there are no items in the list, and the user clicks
        on the drop-down box.

        By default it just says "no choices", but this lets you change it to something more
        meaningful.
    */
    void setTextWhenNoChoicesAvailable (const String& newMessage);

    /** Returns the text shown when no items have been added to the list.
        @see setTextWhenNoChoicesAvailable
    */
    String getTextWhenNoChoicesAvailable() const;

    //==============================================================================
    /** Gives the ComboBox a tooltip. */
    void setTooltip (const String& newTooltip) override;

    /** This can be used to allow the scroll-wheel to nudge the chosen item.
        By default it's disabled, and I'd recommend leaving it disabled if there's any
        chance that the control might be inside a scrollable list or viewport.
    */
    void setScrollWheelEnabled (bool enabled) noexcept;


    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the combo box.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        To change the colours of the menu that pops up

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId  = 0x1000b00,    /**< The background colour to fill the box with. */
        textColourId        = 0x1000a00,    /**< The colour for the text in the box. */
        outlineColourId     = 0x1000c00,    /**< The colour for an outline around the box. */
        buttonColourId      = 0x1000d00,    /**< The base colour for the button (a LookAndFeel class will probably use variations on this). */
        arrowColourId       = 0x1000e00,    /**< The colour for the arrow shape that pops up the menu */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        ComboBox functionality.
    */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() {}

        virtual void drawComboBox (Graphics&, int width, int height, bool isButtonDown,
                                   int buttonX, int buttonY, int buttonW, int buttonH,
                                   ComboBox&) = 0;

        virtual Font getComboBoxFont (ComboBox&) = 0;

        virtual Label* createComboBoxTextBox (ComboBox&) = 0;

        virtual void positionComboBoxText (ComboBox&, Label& labelToPosition) = 0;
    };

    //==============================================================================
    /** @internal */
    void labelTextChanged (Label*) override;
    /** @internal */
    void enablementChanged() override;
    /** @internal */
    void colourChanged() override;
    /** @internal */
    void focusGained (Component::FocusChangeType) override;
    /** @internal */
    void focusLost (Component::FocusChangeType) override;
    /** @internal */
    void handleAsyncUpdate() override;
    /** @internal */
    String getTooltip() override                        { return label->getTooltip(); }
    /** @internal */
    void mouseDown (const MouseEvent&) override;
    /** @internal */
    void mouseDrag (const MouseEvent&) override;
    /** @internal */
    void mouseUp (const MouseEvent&) override;
    /** @internal */
    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;
    /** @internal */
    void lookAndFeelChanged() override;
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void resized() override;
    /** @internal */
    bool keyStateChanged (bool) override;
    /** @internal */
    bool keyPressed (const KeyPress&) override;
    /** @internal */
    void valueChanged (Value&) override;
    /** @internal */
    void parentHierarchyChanged() override;

    // These methods' bool parameters have changed: see their new method signatures.
    JUCE_DEPRECATED (void clear (bool));
    JUCE_DEPRECATED (void setSelectedId (int, bool));
    JUCE_DEPRECATED (void setSelectedItemIndex (int, bool));
    JUCE_DEPRECATED (void setText (const String&, bool));

private:
    //==============================================================================
    struct ItemInfo
    {
        ItemInfo (const String&, int itemId, bool isEnabled, bool isHeading);
        bool isSeparator() const noexcept;
        bool isRealItem() const noexcept;

        String name;
        int itemId;
        bool isEnabled : 1, isHeading : 1;
    };

    OwnedArray <ItemInfo> items;
    Value currentId;
    int lastCurrentId;
    bool isButtonDown, separatorPending, menuActive, scrollWheelEnabled;
    float mouseWheelAccumulator;
    ListenerList <Listener> listeners;
    ScopedPointer<Label> label;
    String textWhenNothingSelected, noChoicesMessage;

    ItemInfo* getItemForId (int) const noexcept;
    ItemInfo* getItemForIndex (int) const noexcept;
    bool selectIfEnabled (int index);
    bool nudgeSelectedItem (int delta);
    void sendChange (NotificationType);
    static void popupMenuFinishedCallback (int, ComboBox*);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComboBox)
};

/** This typedef is just for compatibility with old code - newer code should use the ComboBox::Listener class directly. */
typedef ComboBox::Listener ComboBoxListener;

#endif   // JUCE_COMBOBOX_H_INCLUDED
