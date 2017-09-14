/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

ComboBox::ComboBox (const String& name)
    : Component (name),
      lastCurrentId (0),
      isButtonDown (false),
      menuActive (false),
      scrollWheelEnabled (false),
      mouseWheelAccumulator (0),
      noChoicesMessage (TRANS("(no choices)")),
      labelEditableState (editableUnknown)
{
    setRepaintsOnMouseActivity (true);
    lookAndFeelChanged();
    currentId.addListener (this);
}

ComboBox::~ComboBox()
{
    currentId.removeListener (this);
    hidePopup();
    label = nullptr;
}

//==============================================================================
void ComboBox::setEditableText (const bool isEditable)
{
    if (label->isEditableOnSingleClick() != isEditable || label->isEditableOnDoubleClick() != isEditable)
    {
        label->setEditable (isEditable, isEditable, false);
        labelEditableState = (isEditable ? labelIsEditable : labelIsNotEditable);

        setWantsKeyboardFocus (labelEditableState == labelIsNotEditable);
        resized();
    }
}

bool ComboBox::isTextEditable() const noexcept
{
    return label->isEditable();
}

void ComboBox::setJustificationType (Justification justification)
{
    label->setJustificationType (justification);
}

Justification ComboBox::getJustificationType() const noexcept
{
    return label->getJustificationType();
}

void ComboBox::setTooltip (const String& newTooltip)
{
    SettableTooltipClient::setTooltip (newTooltip);
    label->setTooltip (newTooltip);
}

//==============================================================================
void ComboBox::addItem (const String& newItemText, const int newItemId)
{
    // you can't add empty strings to the list..
    jassert (newItemText.isNotEmpty());

    // IDs must be non-zero, as zero is used to indicate a lack of selecion.
    jassert (newItemId != 0);

    // you shouldn't use duplicate item IDs!
    jassert (getItemForId (newItemId) == nullptr);

    if (newItemText.isNotEmpty() && newItemId != 0)
    {
        currentMenu.addItem (newItemId, newItemText, true, false);
    }
}

void ComboBox::addItemList (const StringArray& itemsToAdd, const int firstItemIdOffset)
{
    for (int i = 0; i < itemsToAdd.size(); ++i)
        currentMenu.addItem (i + firstItemIdOffset, itemsToAdd[i]);
}

void ComboBox::addSeparator()
{
    currentMenu.addSeparator();
}

void ComboBox::addSectionHeading (const String& headingName)
{
    // you can't add empty strings to the list..
    jassert (headingName.isNotEmpty());

    if (headingName.isNotEmpty())
    {
        currentMenu.addSectionHeader (headingName);
    }
}

void ComboBox::setItemEnabled (const int itemId, const bool shouldBeEnabled)
{
    if (PopupMenu::Item* item = getItemForId (itemId))
        item->isEnabled = shouldBeEnabled;
}

bool ComboBox::isItemEnabled (int itemId) const noexcept
{
    const PopupMenu::Item* item = getItemForId (itemId);
    return item != nullptr && item->isEnabled;
}

void ComboBox::changeItemText (const int itemId, const String& newText)
{
    if (PopupMenu::Item* item = getItemForId (itemId))
        item->text = newText;
    else
        jassertfalse;
}

void ComboBox::clear (const NotificationType notification)
{
    currentMenu.clear();

    if (! label->isEditable())
        setSelectedItemIndex (-1, notification);
}

//==============================================================================
PopupMenu::Item* ComboBox::getItemForId (const int itemId) const noexcept
{
    if (itemId != 0)
    {
        PopupMenu::MenuItemIterator iterator (currentMenu, true);

        while (iterator.next())
        {
            PopupMenu::Item &item = iterator.getItem();

            if (item.itemID == itemId)
                return &item;
        }
    }

    return nullptr;
}

PopupMenu::Item* ComboBox::getItemForIndex (const int index) const noexcept
{
    int n = 0;
    PopupMenu::MenuItemIterator iterator (currentMenu, true);

    while (iterator.next())
    {
        PopupMenu::Item &item = iterator.getItem();

        if (item.itemID != 0)
            if (n++ == index)
                return &item;
    }

    return nullptr;
}

int ComboBox::getNumItems() const noexcept
{
    int n = 0;
    PopupMenu::MenuItemIterator iterator (currentMenu, true);

    while (iterator.next())
    {
        PopupMenu::Item &item = iterator.getItem();

        if (item.itemID != 0)
            n++;
    }

    return n;
}

String ComboBox::getItemText (const int index) const
{
    if (auto* item = getItemForIndex (index))
        return item->text;

    return {};
}

int ComboBox::getItemId (const int index) const noexcept
{
    if (auto* item = getItemForIndex (index))
        return item->itemID;

    return 0;
}

int ComboBox::indexOfItemId (const int itemId) const noexcept
{
    if (itemId != 0)
    {
        int n = 0;
        PopupMenu::MenuItemIterator iterator (currentMenu, true);

        while (iterator.next())
        {
            auto& item = iterator.getItem();

            if (item.itemID == itemId)
                return n;

            else if (item.itemID != 0)
                n++;
        }
    }

    return -1;
}

//==============================================================================
int ComboBox::getSelectedItemIndex() const
{
    int index = indexOfItemId (currentId.getValue());

    if (getText() != getItemText (index))
        index = -1;

    return index;
}

void ComboBox::setSelectedItemIndex (const int index, const NotificationType notification)
{
    setSelectedId (getItemId (index), notification);
}

int ComboBox::getSelectedId() const noexcept
{
    const PopupMenu::Item* const item = getItemForId (currentId.getValue());

    return (item != nullptr && getText() == item->text) ? item->itemID : 0;
}

void ComboBox::setSelectedId (const int newItemId, const NotificationType notification)
{
    const PopupMenu::Item* const item = getItemForId (newItemId);
    const String newItemText (item != nullptr ? item->text : String());

    if (lastCurrentId != newItemId || label->getText() != newItemText)
    {
        label->setText (newItemText, dontSendNotification);
        lastCurrentId = newItemId;
        currentId = newItemId;

        repaint();  // for the benefit of the 'none selected' text

        sendChange (notification);
    }
}

bool ComboBox::selectIfEnabled (const int index)
{
    if (const PopupMenu::Item* const item = getItemForIndex (index))
    {
        if (item->isEnabled)
        {
            setSelectedItemIndex (index);
            return true;
        }
    }

    return false;
}

bool ComboBox::nudgeSelectedItem (int delta)
{
    for (int i = getSelectedItemIndex() + delta; isPositiveAndBelow (i, getNumItems()); i += delta)
        if (selectIfEnabled (i))
            return true;

    return false;
}

void ComboBox::valueChanged (Value&)
{
    if (lastCurrentId != (int) currentId.getValue())
        setSelectedId (currentId.getValue());
}

//==============================================================================
String ComboBox::getText() const
{
    return label->getText();
}

void ComboBox::setText (const String& newText, const NotificationType notification)
{
    PopupMenu::MenuItemIterator iterator (currentMenu, true);

    while (iterator.next())
    {
        PopupMenu::Item &item = iterator.getItem();

        if (item.itemID != 0
            && item.text == newText)
        {
            setSelectedId (item.itemID, notification);
            return;
        }
    }

    lastCurrentId = 0;
    currentId = 0;
    repaint();

    if (label->getText() != newText)
    {
        label->setText (newText, dontSendNotification);
        sendChange (notification);
    }
}

void ComboBox::showEditor()
{
    jassert (isTextEditable()); // you probably shouldn't do this to a non-editable combo box?

    label->showEditor();
}

//==============================================================================
void ComboBox::setTextWhenNothingSelected (const String& newMessage)
{
    if (textWhenNothingSelected != newMessage)
    {
        textWhenNothingSelected = newMessage;
        repaint();
    }
}

String ComboBox::getTextWhenNothingSelected() const
{
    return textWhenNothingSelected;
}

void ComboBox::setTextWhenNoChoicesAvailable (const String& newMessage)
{
    noChoicesMessage = newMessage;
}

String ComboBox::getTextWhenNoChoicesAvailable() const
{
    return noChoicesMessage;
}

//==============================================================================
void ComboBox::paint (Graphics& g)
{
    getLookAndFeel().drawComboBox (g, getWidth(), getHeight(), isButtonDown,
                                   label->getRight(), 0, getWidth() - label->getRight(), getHeight(),
                                   *this);

    if (textWhenNothingSelected.isNotEmpty()
         && label->getText().isEmpty()
         && ! label->isBeingEdited())
    {
        g.setColour (findColour (textColourId).withMultipliedAlpha (0.5f));
        g.setFont (label->getLookAndFeel().getLabelFont (*label));
        g.drawFittedText (textWhenNothingSelected, label->getBounds().reduced (2, 1),
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

void ComboBox::colourChanged()
{
    lookAndFeelChanged();
}

void ComboBox::parentHierarchyChanged()
{
    lookAndFeelChanged();
}

void ComboBox::lookAndFeelChanged()
{
    repaint();

    {
        ScopedPointer<Label> newLabel (getLookAndFeel().createComboBoxTextBox (*this));
        jassert (newLabel != nullptr);

        if (label != nullptr)
        {
            newLabel->setEditable (label->isEditable());
            newLabel->setJustificationType (label->getJustificationType());
            newLabel->setTooltip (label->getTooltip());
            newLabel->setText (label->getText(), dontSendNotification);
        }

        label = newLabel;
    }

    addAndMakeVisible (label);

    EditableState newEditableState = (label->isEditable() ? labelIsEditable : labelIsNotEditable);

    if (newEditableState != labelEditableState)
    {
        labelEditableState = newEditableState;
        setWantsKeyboardFocus (labelEditableState == labelIsNotEditable);
    }

    label->addListener (this);
    label->addMouseListener (this, false);

    label->setColour (Label::backgroundColourId, Colours::transparentBlack);
    label->setColour (Label::textColourId, findColour (ComboBox::textColourId));

    label->setColour (TextEditor::textColourId, findColour (ComboBox::textColourId));
    label->setColour (TextEditor::backgroundColourId, Colours::transparentBlack);
    label->setColour (TextEditor::highlightColourId, findColour (TextEditor::highlightColourId));
    label->setColour (TextEditor::outlineColourId, Colours::transparentBlack);

    resized();
}

//==============================================================================
bool ComboBox::keyPressed (const KeyPress& key)
{
    if (key == KeyPress::upKey || key == KeyPress::leftKey)
    {
        nudgeSelectedItem (-1);
        return true;
    }

    if (key == KeyPress::downKey || key == KeyPress::rightKey)
    {
        nudgeSelectedItem (1);
        return true;
    }

    if (key == KeyPress::returnKey)
    {
        showPopupIfNotActive();
        return true;
    }

    return false;
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
void ComboBox::focusGained (FocusChangeType)    { repaint(); }
void ComboBox::focusLost (FocusChangeType)      { repaint(); }

void ComboBox::labelTextChanged (Label*)
{
    triggerAsyncUpdate();
}


//==============================================================================
void ComboBox::showPopupIfNotActive()
{
    if (! menuActive)
    {
        menuActive = true;

        SafePointer<ComboBox> safePointer (this);

        // as this method was triggered by a mouse event, the same mouse event may have
        // exited the modal state of other popups currently on the screen. By calling
        // showPopup asynchronously, we are giving the other popups a chance to properly
        // close themselves
        MessageManager::callAsync([safePointer] () mutable { if (safePointer != nullptr) safePointer->showPopup(); });
    }
}

void ComboBox::hidePopup()
{
    if (menuActive)
    {
        menuActive = false;
        PopupMenu::dismissAllActiveMenus();
        repaint();
    }
}

static void comboBoxPopupMenuFinishedCallback (int result, ComboBox* combo)
{
    if (combo != nullptr)
    {
        combo->hidePopup();

        if (result != 0)
            combo->setSelectedId (result);
    }
}

void ComboBox::showPopup()
{
    PopupMenu noChoicesMenu;
    const bool hasItems = (currentMenu.getNumItems() > 0);

    if (hasItems)
    {
        PopupMenu::MenuItemIterator iterator (currentMenu, true);
        const int selectedId = getSelectedId();

        while (iterator.next())
        {
            PopupMenu::Item &item = iterator.getItem();

            if (item.itemID != 0)
                item.isTicked = (item.itemID == selectedId);
        }
    }
    else
    {
        noChoicesMenu.addItem (1, noChoicesMessage, false, false);
    }

    PopupMenu& menuToShow = (hasItems ? currentMenu : noChoicesMenu);
    menuToShow.setLookAndFeel (&getLookAndFeel());
    menuToShow.showMenuAsync (PopupMenu::Options().withTargetComponent (this)
                                                  .withItemThatMustBeVisible (getSelectedId())
                                                  .withMinimumWidth (getWidth())
                                                  .withMaximumNumColumns (1)
                                                  .withStandardItemHeight (label->getHeight()),
                              ModalCallbackFunction::forComponent (comboBoxPopupMenuFinishedCallback, this));
}

//==============================================================================
void ComboBox::mouseDown (const MouseEvent& e)
{
    beginDragAutoRepeat (300);

    isButtonDown = isEnabled() && ! e.mods.isPopupMenu();

    if (isButtonDown && (e.eventComponent == this || ! label->isEditable()))
        showPopupIfNotActive();
}

void ComboBox::mouseDrag (const MouseEvent& e)
{
    beginDragAutoRepeat (50);

    if (isButtonDown && e.mouseWasDraggedSinceMouseDown())
        showPopupIfNotActive();
}

void ComboBox::mouseUp (const MouseEvent& e2)
{
    if (isButtonDown)
    {
        isButtonDown = false;
        repaint();

        const MouseEvent e (e2.getEventRelativeTo (this));

        if (reallyContains (e.getPosition(), true)
             && (e2.eventComponent == this || ! label->isEditable()))
        {
            showPopupIfNotActive();
        }
    }
}

void ComboBox::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    if (! menuActive && scrollWheelEnabled && e.eventComponent == this && wheel.deltaY != 0.0f)
    {
        auto oldPos = (int) mouseWheelAccumulator;
        mouseWheelAccumulator += wheel.deltaY * 5.0f;

        if (auto delta = oldPos - (int) mouseWheelAccumulator)
            nudgeSelectedItem (delta);
    }
    else
    {
        Component::mouseWheelMove (e, wheel);
    }
}

void ComboBox::setScrollWheelEnabled (bool enabled) noexcept
{
    scrollWheelEnabled = enabled;
}

//==============================================================================
void ComboBox::addListener (ComboBoxListener* listener)       { listeners.add (listener); }
void ComboBox::removeListener (ComboBoxListener* listener)    { listeners.remove (listener); }

void ComboBox::handleAsyncUpdate()
{
    Component::BailOutChecker checker (this);
    listeners.callChecked (checker, &ComboBox::Listener::comboBoxChanged, this);
}

void ComboBox::sendChange (const NotificationType notification)
{
    if (notification != dontSendNotification)
        triggerAsyncUpdate();

    if (notification == sendNotificationSync)
        handleUpdateNowIfNeeded();
}

// Old deprecated methods - remove eventually...
void ComboBox::clear (const bool dontSendChange)                                 { clear (dontSendChange ? dontSendNotification : sendNotification); }
void ComboBox::setSelectedItemIndex (const int index, const bool dontSendChange) { setSelectedItemIndex (index, dontSendChange ? dontSendNotification : sendNotification); }
void ComboBox::setSelectedId (const int newItemId, const bool dontSendChange)    { setSelectedId (newItemId, dontSendChange ? dontSendNotification : sendNotification); }
void ComboBox::setText (const String& newText, const bool dontSendChange)        { setText (newText, dontSendChange ? dontSendNotification : sendNotification); }

} // namespace juce
