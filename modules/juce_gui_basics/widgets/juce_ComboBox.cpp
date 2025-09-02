/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

ComboBox::ComboBox (const String& name)
    : Component (name),
      noChoicesMessage (TRANS ("(no choices)"))
{
    setRepaintsOnMouseActivity (true);
    lookAndFeelChanged();
    currentId.addListener (this);
}

ComboBox::~ComboBox()
{
    currentId.removeListener (this);
    hidePopup();
    label.reset();
}

//==============================================================================
void ComboBox::setEditableText (const bool isEditable)
{
    if (label->isEditableOnSingleClick() != isEditable || label->isEditableOnDoubleClick() != isEditable)
    {
        label->setEditable (isEditable, isEditable, false);
        labelEditableState = (isEditable ? labelIsEditable : labelIsNotEditable);

        const auto isLabelEditable = (labelEditableState == labelIsEditable);

        setWantsKeyboardFocus (! isLabelEditable);
        label->setAccessible (isLabelEditable);

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
void ComboBox::addItem (const String& newItemText, int newItemId)
{
    // you can't add empty strings to the list..
    jassert (newItemText.isNotEmpty());

    // IDs must be non-zero, as zero is used to indicate a lack of selection.
    jassert (newItemId != 0);

    // you shouldn't use duplicate item IDs!
    jassert (getItemForId (newItemId) == nullptr);

    if (newItemText.isNotEmpty() && newItemId != 0)
        currentMenu.addItem (newItemId, newItemText, true, false);
}

void ComboBox::addItemList (const StringArray& itemsToAdd, int firstItemID)
{
    for (auto& i : itemsToAdd)
        currentMenu.addItem (firstItemID++, i);
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
        currentMenu.addSectionHeader (headingName);
}

void ComboBox::setItemEnabled (int itemId, bool shouldBeEnabled)
{
    if (auto* item = getItemForId (itemId))
        item->isEnabled = shouldBeEnabled;
}

bool ComboBox::isItemEnabled (int itemId) const noexcept
{
    if (auto* item = getItemForId (itemId))
        return item->isEnabled;

    return false;
}

void ComboBox::changeItemText (int itemId, const String& newText)
{
    if (auto* item = getItemForId (itemId))
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
PopupMenu::Item* ComboBox::getItemForId (int itemId) const noexcept
{
    if (itemId != 0)
    {
        for (PopupMenu::MenuItemIterator iterator (currentMenu, true); iterator.next();)
        {
            auto& item = iterator.getItem();

            if (item.itemID == itemId)
                return &item;
        }
    }

    return nullptr;
}

PopupMenu::Item* ComboBox::getItemForIndex (const int index) const noexcept
{
    int n = 0;

    for (PopupMenu::MenuItemIterator iterator (currentMenu, true); iterator.next();)
    {
        auto& item = iterator.getItem();

        if (item.itemID != 0)
            if (n++ == index)
                return &item;
    }

    return nullptr;
}

int ComboBox::getNumItems() const noexcept
{
    int n = 0;

    for (PopupMenu::MenuItemIterator iterator (currentMenu, true); iterator.next();)
    {
        auto& item = iterator.getItem();

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

        for (PopupMenu::MenuItemIterator iterator (currentMenu, true); iterator.next();)
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
    auto index = indexOfItemId (currentId.getValue());

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
    if (auto* item = getItemForId (currentId.getValue()))
        if (getText() == item->text)
            return item->itemID;

    return 0;
}

void ComboBox::setSelectedId (const int newItemId, const NotificationType notification)
{
    auto* item = getItemForId (newItemId);
    auto newItemText = item != nullptr ? item->text : String();

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
    if (auto* item = getItemForIndex (index))
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
    for (PopupMenu::MenuItemIterator iterator (currentMenu, true); iterator.next();)
    {
        auto& item = iterator.getItem();

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

    if (textWhenNothingSelected.isNotEmpty() && label->getText().isEmpty() && ! label->isBeingEdited())
        getLookAndFeel().drawComboBoxTextWhenNothingSelected (g, *this, *label);
}

void ComboBox::resized()
{
    if (getHeight() > 0 && getWidth() > 0)
        getLookAndFeel().positionComboBoxText (*this, *label);
}

void ComboBox::enablementChanged()
{
    if (! isEnabled())
        hidePopup();

    repaint();
}

void ComboBox::colourChanged()
{
    label->setColour (Label::backgroundColourId, Colours::transparentBlack);
    label->setColour (Label::textColourId, findColour (ComboBox::textColourId));

    label->setColour (TextEditor::textColourId, findColour (ComboBox::textColourId));
    label->setColour (TextEditor::backgroundColourId, Colours::transparentBlack);
    label->setColour (TextEditor::highlightColourId, findColour (TextEditor::highlightColourId));
    label->setColour (TextEditor::outlineColourId, Colours::transparentBlack);
    repaint();
}

void ComboBox::parentHierarchyChanged()
{
    lookAndFeelChanged();
}

void ComboBox::lookAndFeelChanged()
{
    {
        std::unique_ptr<Label> newLabel (getLookAndFeel().createComboBoxTextBox (*this));
        jassert (newLabel != nullptr);

        if (label != nullptr)
        {
            newLabel->setEditable (label->isEditable());
            newLabel->setJustificationType (label->getJustificationType());
            newLabel->setTooltip (label->getTooltip());
            newLabel->setText (label->getText(), dontSendNotification);
        }

        std::swap (label, newLabel);
    }

    addAndMakeVisible (label.get());

    EditableState newEditableState = (label->isEditable() ? labelIsEditable : labelIsNotEditable);

    if (newEditableState != labelEditableState)
    {
        labelEditableState = newEditableState;
        setWantsKeyboardFocus (labelEditableState == labelIsNotEditable);
    }

    label->onTextChange = [this] { triggerAsyncUpdate(); };
    label->addMouseListener (this, false);
    label->setAccessible (labelEditableState == labelIsEditable);

    colourChanged();
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

//==============================================================================
void ComboBox::showPopupIfNotActive()
{
    if (! menuActive)
    {
        menuActive = true;

        // as this method was triggered by a mouse event, the same mouse event may have
        // exited the modal state of other popups currently on the screen. By calling
        // showPopup asynchronously, we are giving the other popups a chance to properly
        // close themselves
        MessageManager::callAsync ([safePointer = SafePointer<ComboBox> { this }]() mutable { if (safePointer != nullptr) safePointer->showPopup(); });
        repaint();
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

        if (auto* handler = combo->getAccessibilityHandler())
            handler->grabFocus();
    }
}

void ComboBox::showPopup()
{
    if (! menuActive)
        menuActive = true;

    auto menu = currentMenu;

    if (menu.getNumItems() > 0)
    {
        auto selectedId = getSelectedId();

        for (PopupMenu::MenuItemIterator iterator (menu, true); iterator.next();)
        {
            auto& item = iterator.getItem();

            if (item.itemID != 0)
                item.isTicked = (item.itemID == selectedId);
        }
    }
    else
    {
        menu.addItem (1, noChoicesMessage, false, false);
    }

    auto& lf = getLookAndFeel();

    menu.setLookAndFeel (&lf);
    menu.showMenuAsync (lf.getOptionsForComboBoxPopupMenu (*this, *label),
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

        auto e = e2.getEventRelativeTo (this);

        if (reallyContains (e.getPosition(), true)
             && (e2.eventComponent == this || ! label->isEditable()))
        {
            showPopupIfNotActive();
        }
    }
}

void ComboBox::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    if (! menuActive && scrollWheelEnabled && e.eventComponent == this && ! approximatelyEqual (wheel.deltaY, 0.0f))
    {
        mouseWheelAccumulator += wheel.deltaY * 5.0f;

        while (mouseWheelAccumulator > 1.0f)
        {
            mouseWheelAccumulator -= 1.0f;
            nudgeSelectedItem (-1);
        }

        while (mouseWheelAccumulator < -1.0f)
        {
            mouseWheelAccumulator += 1.0f;
            nudgeSelectedItem (1);
        }
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
void ComboBox::addListener    (ComboBox::Listener* l)    { listeners.add (l); }
void ComboBox::removeListener (ComboBox::Listener* l)    { listeners.remove (l); }

void ComboBox::handleAsyncUpdate()
{
    Component::BailOutChecker checker (this);
    listeners.callChecked (checker, [this] (Listener& l) { l.comboBoxChanged (this); });

    if (checker.shouldBailOut())
        return;

    NullCheckedInvocation::invoke (onChange);

    if (checker.shouldBailOut())
        return;

    if (auto* handler = getAccessibilityHandler())
        handler->notifyAccessibilityEvent (AccessibilityEvent::valueChanged);
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

//==============================================================================
class ComboBoxAccessibilityHandler final : public AccessibilityHandler
{
public:
    explicit ComboBoxAccessibilityHandler (ComboBox& comboBoxToWrap)
        : AccessibilityHandler (comboBoxToWrap,
                                AccessibilityRole::comboBox,
                                getAccessibilityActions (comboBoxToWrap),
                                { std::make_unique<ComboBoxValueInterface> (comboBoxToWrap) }),
          comboBox (comboBoxToWrap)
    {
    }

    AccessibleState getCurrentState() const override
    {
        auto state = AccessibilityHandler::getCurrentState().withExpandable();

        return comboBox.isPopupActive() ? state.withExpanded() : state.withCollapsed();
    }

    String getTitle() const override  { return comboBox.getTitle(); }
    String getHelp() const override   { return comboBox.getTooltip(); }

private:
    class ComboBoxValueInterface final : public AccessibilityTextValueInterface
    {
    public:
        explicit ComboBoxValueInterface (ComboBox& comboBoxToWrap)
            : comboBox (comboBoxToWrap)
        {
        }

        bool isReadOnly() const override                 { return true; }
        String getCurrentValueAsString() const override  { return comboBox.getText(); }
        void setValueAsString (const String&) override   {}

    private:
        ComboBox& comboBox;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComboBoxValueInterface)
    };

    static AccessibilityActions getAccessibilityActions (ComboBox& comboBox)
    {
        return AccessibilityActions().addAction (AccessibilityActionType::press,    [&comboBox] { comboBox.showPopup(); })
                                     .addAction (AccessibilityActionType::showMenu, [&comboBox] { comboBox.showPopup(); });
    }

    ComboBox& comboBox;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComboBoxAccessibilityHandler)
};

std::unique_ptr<AccessibilityHandler> ComboBox::createAccessibilityHandler()
{
    return std::make_unique<ComboBoxAccessibilityHandler> (*this);
}

} // namespace juce
