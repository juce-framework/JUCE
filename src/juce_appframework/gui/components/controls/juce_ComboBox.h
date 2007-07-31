/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_COMBOBOX_JUCEHEADER__
#define __JUCE_COMBOBOX_JUCEHEADER__

#include "juce_Label.h"
#include "../../../../juce_core/text/juce_StringArray.h"
class ComboBox;


//==============================================================================
/**
    A class for receiving events from a ComboBox.

    You can register a ComboBoxListener with a ComboBox using the ComboBox::addListener()
    method, and it will be called when the selected item in the box changes.

    @see ComboBox::addListener, ComboBox::removeListener
*/
class JUCE_API  ComboBoxListener
{
public:
    /** Destructor. */
    virtual ~ComboBoxListener() {}

    /** Called when a Label's text has changed.
    */
    virtual void comboBoxChanged (ComboBox* comboBoxThatHasChanged) = 0;
};


//==============================================================================
/**
    A component that lets the user choose from a drop-down list of choices.

    The combo-box has a list of text strings, each with an associated id number,
    that will be shown in the drop-down list when the user clicks on the component.

    The currently selected choice is displayed in the combo-box, and this can
    either be read-only text, or editable.

    To find out when the user selects a different item or edits the text, you
    can register a ComboBoxListener to receive callbacks.

    @see ComboBoxListener
*/
class JUCE_API  ComboBox  : public Component,
                            public SettableTooltipClient,
                            private LabelListener,
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
    ComboBox (const String& componentName);

    /** Destructor. */
    ~ComboBox();

    //==============================================================================
    /** Sets whether the test in the combo-box is editable.

        The default state for a new ComboBox is non-editable, and can only be changed
        by choosing from the drop-down list.
    */
    void setEditableText (const bool isEditable);

    /** Returns true if the text is directly editable.
        @see setEditableText
    */
    bool isTextEditable() const throw();

    /** Sets the style of justification to be used for positioning the text.

        The default is Justification::centredLeft. The text is displayed using a
        Label component inside the ComboBox.
    */
    void setJustificationType (const Justification& justification) throw();

    /** Returns the current justification for the text box.
        @see setJustificationType
    */
    const Justification getJustificationType() const throw();

    //==============================================================================
    /** Adds an item to be shown in the drop-down list.

        @param newItemText      the text of the item to show in the list
        @param newItemId        an associated ID number that can be set or retrieved - see
                                getSelectedId() and setSelectedId()
        @see setItemEnabled, addSeparator, addSectionHeading, removeItem, getNumItems, getItemText, getItemId
    */
    void addItem (const String& newItemText,
                  const int newItemId) throw();

    /** Adds a separator line to the drop-down list.

        This is like adding a separator to a popup menu. See PopupMenu::addSeparator().
    */
    void addSeparator() throw();

    /** Adds a heading to the drop-down list, so that you can group the items into
        different sections.

        The headings are indented slightly differently to set them apart from the
        items on the list, and obviously can't be selected. You might want to add
        separators between your sections too.

        @see addItem, addSeparator
    */
    void addSectionHeading (const String& headingName) throw();

    /** This allows items in the drop-down list to be selectively disabled.

        When you add an item, it's enabled by default, but you can call this
        method to change its status.

        If you disable an item which is already selected, this won't change the
        current selection - it just stops the user choosing that item from the list.
    */
    void setItemEnabled (const int itemId,
                         const bool isEnabled) throw();

    /** Changes the text for an existing item.
    */
    void changeItemText (const int itemId,
                         const String& newText) throw();

    /** Removes all the items from the drop-down list.

        @see addItem, removeItem, getNumItems
    */
    void clear();

    /** Returns the number of items that have been added to the list.

        Note that this doesn't include headers or separators.
    */
    int getNumItems() const throw();

    /** Returns the text for one of the items in the list.

        Note that this doesn't include headers or separators.

        @param index    the item's index from 0 to (getNumItems() - 1)
    */
    const String getItemText (const int index) const throw();

    /** Returns the ID for one of the items in the list.

        Note that this doesn't include headers or separators.

        @param index    the item's index from 0 to (getNumItems() - 1)
    */
    int getItemId (const int index) const throw();

    //==============================================================================
    /** Returns the ID of the item that's currently shown in the box.

        If no item is selected, or if the text is editable and the user
        has entered something which isn't one of the items in the list, then
        this will return 0.

        @see setSelectedId, getSelectedItemIndex, getText
    */
    int getSelectedId() const throw();

    /** Sets one of the items to be the current selection.

        This will set the ComboBox's text to that of the item that matches
        this ID.

        @param newItemId                the new item to select
        @param dontSendChangeMessage    if set to true, this method won't trigger a
                                        change notification
        @see getSelectedId, setSelectedItemIndex, setText
    */
    void setSelectedId (const int newItemId,
                        const bool dontSendChangeMessage = false) throw();

    //==============================================================================
    /** Returns the index of the item that's currently shown in the box.

        If no item is selected, or if the text is editable and the user
        has entered something which isn't one of the items in the list, then
        this will return -1.

        @see setSelectedItemIndex, getSelectedId, getText
    */
    int getSelectedItemIndex() const throw();

    /** Sets one of the items to be the current selection.

        This will set the ComboBox's text to that of the item at the given
        index in the list.

        @param newItemIndex             the new item to select
        @param dontSendChangeMessage    if set to true, this method won't trigger a
                                        change notification
        @see getSelectedItemIndex, setSelectedId, setText
    */
    void setSelectedItemIndex (const int newItemIndex,
                               const bool dontSendChangeMessage = false) throw();

    //==============================================================================
    /** Returns the text that is currently shown in the combo-box's text field.

        If the ComboBox has editable text, then this text may have been edited
        by the user; otherwise it will be one of the items from the list, or
        possibly an empty string if nothing was selected.

        @see setText, getSelectedId, getSelectedItemIndex
    */
    const String getText() const throw();

    /** Sets the contents of the combo-box's text field.

        The text passed-in will be set as the current text regardless of whether
        it is one of the items in the list. If the current text isn't one of the
        items, then getSelectedId() will return -1, otherwise it wil return
        the approriate ID.

        @param newText                  the text to select
        @param dontSendChangeMessage    if set to true, this method won't trigger a
                                        change notification
        @see getText
    */
    void setText (const String& newText,
                  const bool dontSendChangeMessage = false) throw();

    //==============================================================================
    /** Registers a listener that will be called when the box's content changes. */
    void addListener (ComboBoxListener* const listener) throw();

    /** Deregisters a previously-registered listener. */
    void removeListener (ComboBoxListener* const listener) throw();

    //==============================================================================
    /** Sets a message to display when there is no item currently selected.

        @see getTextWhenNothingSelected
    */
    void setTextWhenNothingSelected (const String& newMessage) throw();

    /** Returns the text that is shown when no item is selected.

        @see setTextWhenNothingSelected
    */
    const String getTextWhenNothingSelected() const throw();


    /** Sets the message to show when there are no items in the list, and the user clicks
        on the drop-down box.

        By default it just says "no choices", but this lets you change it to something more
        meaningful.
    */
    void setTextWhenNoChoicesAvailable (const String& newMessage) throw();

    /** Returns the text shown when no items have been added to the list.
        @see setTextWhenNoChoicesAvailable
    */
    const String getTextWhenNoChoicesAvailable() const throw();

    //==============================================================================
    /** Gives the ComboBox a tooltip. */
    void setTooltip (const String& newTooltip);


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
    };

    //==============================================================================
    /** @internal */
    void labelTextChanged (Label*);
    /** @internal */
    void enablementChanged();
    /** @internal */
    void colourChanged();
    /** @internal */
    void focusGained (Component::FocusChangeType cause);
    /** @internal */
    void focusLost (Component::FocusChangeType cause);
    /** @internal */
    void handleAsyncUpdate();
    /** @internal */
    const String getTooltip()                                       { return label->getTooltip(); }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    struct ItemInfo
    {
        String name;
        int id;
        bool isEnabled : 1, isHeading : 1;

        bool isSeparator() const throw();
        bool isRealItem() const throw();
    };

    OwnedArray <ItemInfo> items;
    int currentIndex;
    bool isButtonDown;
    bool separatorPending;
    bool menuActive;
    SortedSet <void*> listeners;
    Label* label;
    String textWhenNothingSelected, noChoicesMessage;

    void mouseDown (const MouseEvent&);
    void mouseDrag (const MouseEvent&);
    void mouseUp (const MouseEvent&);
    void lookAndFeelChanged();
    void paint (Graphics&);
    void resized();
    bool keyStateChanged();
    bool keyPressed (const KeyPress&);

    void showPopup();

    ItemInfo* getItemForId (const int id) const throw();
    ItemInfo* getItemForIndex (const int index) const throw();

    ComboBox (const ComboBox&);
    const ComboBox& operator= (const ComboBox&);
};

#endif   // __JUCE_COMBOBOX_JUCEHEADER__
