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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ComboBox.h"
#include "../menus/juce_PopupMenu.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../../../text/juce_LocalisedStrings.h"


//==============================================================================
ComboBox::ComboBox (const String& name)
    : Component (name),
      lastCurrentId (0),
      isButtonDown (false),
      separatorPending (false),
      menuActive (false),
      label (0)
{
    noChoicesMessage = TRANS("(no choices)");
    setRepaintsOnMouseActivity (true);

    lookAndFeelChanged();

    currentId.addListener (this);
}

ComboBox::~ComboBox()
{
    currentId.removeListener (this);

    if (menuActive)
        PopupMenu::dismissAllActiveMenus();

    deleteAllChildren();
}

//==============================================================================
void ComboBox::setEditableText (const bool isEditable)
{
    label->setEditable (isEditable, isEditable, false);

    setWantsKeyboardFocus (! isEditable);
    resized();
}

bool ComboBox::isTextEditable() const throw()
{
    return label->isEditable();
}

void ComboBox::setJustificationType (const Justification& justification) throw()
{
    label->setJustificationType (justification);
}

const Justification ComboBox::getJustificationType() const throw()
{
    return label->getJustificationType();
}

void ComboBox::setTooltip (const String& newTooltip)
{
    SettableTooltipClient::setTooltip (newTooltip);
    label->setTooltip (newTooltip);
}

//==============================================================================
void ComboBox::addItem (const String& newItemText,
                        const int newItemId) throw()
{
    // you can't add empty strings to the list..
    jassert (newItemText.isNotEmpty());

    // IDs must be non-zero, as zero is used to indicate a lack of selecion.
    jassert (newItemId != 0);

    // you shouldn't use duplicate item IDs!
    jassert (getItemForId (newItemId) == 0);

    if (newItemText.isNotEmpty() && newItemId != 0)
    {
        if (separatorPending)
        {
            separatorPending = false;

            ItemInfo* const item = new ItemInfo();
            item->itemId = 0;
            item->isEnabled = false;
            item->isHeading = false;
            items.add (item);
        }

        ItemInfo* const item = new ItemInfo();
        item->name = newItemText;
        item->itemId = newItemId;
        item->isEnabled = true;
        item->isHeading = false;
        items.add (item);
    }
}

void ComboBox::addSeparator() throw()
{
    separatorPending = (items.size() > 0);
}

void ComboBox::addSectionHeading (const String& headingName) throw()
{
    // you can't add empty strings to the list..
    jassert (headingName.isNotEmpty());

    if (headingName.isNotEmpty())
    {
        if (separatorPending)
        {
            separatorPending = false;

            ItemInfo* const item = new ItemInfo();
            item->itemId = 0;
            item->isEnabled = false;
            item->isHeading = false;
            items.add (item);
        }

        ItemInfo* const item = new ItemInfo();
        item->name = headingName;
        item->itemId = 0;
        item->isEnabled = true;
        item->isHeading = true;
        items.add (item);
    }
}

void ComboBox::setItemEnabled (const int itemId,
                               const bool shouldBeEnabled) throw()
{
    ItemInfo* const item = getItemForId (itemId);

    if (item != 0)
        item->isEnabled = shouldBeEnabled;
}

void ComboBox::changeItemText (const int itemId,
                               const String& newText) throw()
{
    ItemInfo* const item = getItemForId (itemId);

    jassert (item != 0);

    if (item != 0)
        item->name = newText;
}

void ComboBox::clear (const bool dontSendChangeMessage)
{
    items.clear();
    separatorPending = false;

    if (! label->isEditable())
        setSelectedItemIndex (-1, dontSendChangeMessage);
}

//==============================================================================
bool ComboBox::ItemInfo::isSeparator() const throw()
{
    return name.isEmpty();
}

bool ComboBox::ItemInfo::isRealItem() const throw()
{
    return ! (isHeading || name.isEmpty());
}

//==============================================================================
ComboBox::ItemInfo* ComboBox::getItemForId (const int itemId) const throw()
{
    if (itemId != 0)
    {
        for (int i = items.size(); --i >= 0;)
            if (items.getUnchecked(i)->itemId == itemId)
                return items.getUnchecked(i);
    }

    return 0;
}

ComboBox::ItemInfo* ComboBox::getItemForIndex (const int index) const throw()
{
    int n = 0;

    for (int i = 0; i < items.size(); ++i)
    {
        ItemInfo* const item = items.getUnchecked(i);

        if (item->isRealItem())
            if (n++ == index)
                return item;
    }

    return 0;
}

int ComboBox::getNumItems() const throw()
{
    int n = 0;

    for (int i = items.size(); --i >= 0;)
        if (items.getUnchecked(i)->isRealItem())
            ++n;

    return n;
}

const String ComboBox::getItemText (const int index) const throw()
{
    const ItemInfo* const item = getItemForIndex (index);

    if (item != 0)
        return item->name;

    return String::empty;
}

int ComboBox::getItemId (const int index) const throw()
{
    const ItemInfo* const item = getItemForIndex (index);

    return (item != 0) ? item->itemId : 0;
}

int ComboBox::indexOfItemId (const int itemId) const throw()
{
    int n = 0;

    for (int i = 0; i < items.size(); ++i)
    {
        const ItemInfo* const item = items.getUnchecked(i);

        if (item->isRealItem())
        {
            if (item->itemId == itemId)
                return n;

            ++n;
        }
    }

    return -1;
}

//==============================================================================
int ComboBox::getSelectedItemIndex() const throw()
{
    int index = indexOfItemId (currentId.getValue());

    if (getText() != getItemText (index))
        index = -1;

    return index;
}

void ComboBox::setSelectedItemIndex (const int index,
                                     const bool dontSendChangeMessage) throw()
{
    setSelectedId (getItemId (index), dontSendChangeMessage);
}

int ComboBox::getSelectedId() const throw()
{
    const ItemInfo* const item = getItemForId (currentId.getValue());

    return (item != 0 && getText() == item->name)
                ? item->itemId
                : 0;
}

void ComboBox::setSelectedId (const int newItemId,
                              const bool dontSendChangeMessage) throw()
{
    const ItemInfo* const item = getItemForId (newItemId);
    const String newItemText (item != 0 ? item->name : String::empty);

    if (lastCurrentId != newItemId || label->getText() != newItemText)
    {
        if (! dontSendChangeMessage)
            triggerAsyncUpdate();

        label->setText (newItemText, false);
        lastCurrentId = newItemId;
        currentId = newItemId;

        repaint();  // for the benefit of the 'none selected' text
    }
}

void ComboBox::valueChanged (Value&)
{
    if (lastCurrentId != (int) currentId.getValue())
        setSelectedId (currentId.getValue(), false);
}

//==============================================================================
const String ComboBox::getText() const throw()
{
    return label->getText();
}

void ComboBox::setText (const String& newText,
                        const bool dontSendChangeMessage) throw()
{
    for (int i = items.size(); --i >= 0;)
    {
        const ItemInfo* const item = items.getUnchecked(i);

        if (item->isRealItem()
             && item->name == newText)
        {
            setSelectedId (item->itemId, dontSendChangeMessage);
            return;
        }
    }

    lastCurrentId = 0;
    currentId = 0;

    if (label->getText() != newText)
    {
        label->setText (newText, false);

        if (! dontSendChangeMessage)
            triggerAsyncUpdate();
    }

    repaint();
}

void ComboBox::showEditor()
{
    jassert (isTextEditable()); // you probably shouldn't do this to a non-editable combo box?

    label->showEditor();
}

//==============================================================================
void ComboBox::setTextWhenNothingSelected (const String& newMessage) throw()
{
    textWhenNothingSelected = newMessage;
    repaint();
}

const String ComboBox::getTextWhenNothingSelected() const throw()
{
    return textWhenNothingSelected;
}

void ComboBox::setTextWhenNoChoicesAvailable (const String& newMessage) throw()
{
    noChoicesMessage = newMessage;
}

const String ComboBox::getTextWhenNoChoicesAvailable() const throw()
{
    return noChoicesMessage;
}

//==============================================================================
void ComboBox::paint (Graphics& g)
{
    getLookAndFeel().drawComboBox (g,
                                   getWidth(),
                                   getHeight(),
                                   isButtonDown,
                                   label->getRight(),
                                   0,
                                   getWidth() - label->getRight(),
                                   getHeight(),
                                   *this);

    if (textWhenNothingSelected.isNotEmpty()
        && label->getText().isEmpty()
        && ! label->isBeingEdited())
    {
        g.setColour (findColour (textColourId).withMultipliedAlpha (0.5f));
        g.setFont (label->getFont());
        g.drawFittedText (textWhenNothingSelected,
                          label->getX() + 2, label->getY() + 1,
                          label->getWidth() - 4, label->getHeight() - 2,
                          label->getJustificationType(),
                          jmax (1, (int) (label->getHeight() / label->getFont().getHeight())));
    }
}

void ComboBox::resized()
{
    if (getHeight() > 0 && getWidth() > 0)
        getLookAndFeel().positionComboBoxText (*this, *label);
}

void ComboBox::enablementChanged()
{
    repaint();
}

void ComboBox::lookAndFeelChanged()
{
    repaint();

    Label* const newLabel = getLookAndFeel().createComboBoxTextBox (*this);

    if (label != 0)
    {
        newLabel->setEditable (label->isEditable());
        newLabel->setJustificationType (label->getJustificationType());
        newLabel->setTooltip (label->getTooltip());
        newLabel->setText (label->getText(), false);
    }

    delete label;
    label = newLabel;

    addAndMakeVisible (newLabel);

    newLabel->addListener (this);
    newLabel->addMouseListener (this, false);

    newLabel->setColour (Label::backgroundColourId, Colours::transparentBlack);
    newLabel->setColour (Label::textColourId, findColour (ComboBox::textColourId));

    newLabel->setColour (TextEditor::textColourId, findColour (ComboBox::textColourId));
    newLabel->setColour (TextEditor::backgroundColourId, Colours::transparentBlack);
    newLabel->setColour (TextEditor::highlightColourId, findColour (TextEditor::highlightColourId));
    newLabel->setColour (TextEditor::outlineColourId, Colours::transparentBlack);

    resized();
}

void ComboBox::colourChanged()
{
    lookAndFeelChanged();
}

//==============================================================================
bool ComboBox::keyPressed (const KeyPress& key)
{
    bool used = false;

    if (key.isKeyCode (KeyPress::upKey)
        || key.isKeyCode (KeyPress::leftKey))
    {
        setSelectedItemIndex (jmax (0, getSelectedItemIndex() - 1));
        used = true;
    }
    else if (key.isKeyCode (KeyPress::downKey)
              || key.isKeyCode (KeyPress::rightKey))
    {
        setSelectedItemIndex (jmin (getSelectedItemIndex() + 1, getNumItems() - 1));
        used = true;
    }
    else if (key.isKeyCode (KeyPress::returnKey))
    {
        showPopup();
        used = true;
    }

    return used;
}

bool ComboBox::keyStateChanged (const bool isKeyDown)
{
    // only forward key events that aren't used by this component
    return isKeyDown
            && (KeyPress::isKeyCurrentlyDown (KeyPress::upKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::leftKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::downKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::rightKey));
}

//==============================================================================
void ComboBox::focusGained (FocusChangeType)
{
    repaint();
}

void ComboBox::focusLost (FocusChangeType)
{
    repaint();
}

//==============================================================================
void ComboBox::labelTextChanged (Label*)
{
    triggerAsyncUpdate();
}


//==============================================================================
void ComboBox::showPopup()
{
    if (! menuActive)
    {
        const int selectedId = getSelectedId();
        ComponentDeletionWatcher deletionWatcher (this);

        PopupMenu menu;

        menu.setLookAndFeel (&getLookAndFeel());

        for (int i = 0; i < items.size(); ++i)
        {
            const ItemInfo* const item = items.getUnchecked(i);

            if (item->isSeparator())
                menu.addSeparator();
            else if (item->isHeading)
                menu.addSectionHeader (item->name);
            else
                menu.addItem (item->itemId, item->name,
                              item->isEnabled, item->itemId == selectedId);
        }

        if (items.size() == 0)
            menu.addItem (1, noChoicesMessage, false);

        const int itemHeight = jlimit (12, 24, getHeight());

        menuActive = true;
        const int resultId = menu.showAt (this, selectedId,
                                          getWidth(), 1, itemHeight);

        if (deletionWatcher.hasBeenDeleted())
            return;

        menuActive = false;

        if (resultId != 0)
            setSelectedId (resultId);
    }
}

//==============================================================================
void ComboBox::mouseDown (const MouseEvent& e)
{
    beginDragAutoRepeat (300);

    isButtonDown = isEnabled();

    if (isButtonDown
         && (e.eventComponent == this || ! label->isEditable()))
    {
        showPopup();
    }
}

void ComboBox::mouseDrag (const MouseEvent& e)
{
    beginDragAutoRepeat (50);

    if (isButtonDown && ! e.mouseWasClicked())
        showPopup();
}

void ComboBox::mouseUp (const MouseEvent& e2)
{
    if (isButtonDown)
    {
        isButtonDown = false;
        repaint();

        const MouseEvent e (e2.getEventRelativeTo (this));

        if (reallyContains (e.x, e.y, true)
             && (e2.eventComponent == this || ! label->isEditable()))
        {
            showPopup();
        }
    }
}

//==============================================================================
void ComboBox::addListener (ComboBoxListener* const listener) throw()
{
    jassert (listener != 0);
    if (listener != 0)
        listeners.add (listener);
}

void ComboBox::removeListener (ComboBoxListener* const listener) throw()
{
    listeners.removeValue (listener);
}

void ComboBox::handleAsyncUpdate()
{
    for (int i = listeners.size(); --i >= 0;)
    {
        ((ComboBoxListener*) listeners.getUnchecked (i))->comboBoxChanged (this);
        i = jmin (i, listeners.size());
    }
}


END_JUCE_NAMESPACE
