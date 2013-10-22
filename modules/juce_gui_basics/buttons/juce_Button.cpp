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

class Button::RepeatTimer  : public Timer
{
public:
    RepeatTimer (Button& b) : owner (b)   {}
    void timerCallback() override    { owner.repeatTimerCallback(); }

private:
    Button& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RepeatTimer)
};

//==============================================================================
Button::Button (const String& name)
  : Component (name),
    text (name),
    buttonPressTime (0),
    lastRepeatTime (0),
    commandManagerToUse (nullptr),
    autoRepeatDelay (-1),
    autoRepeatSpeed (0),
    autoRepeatMinimumDelay (-1),
    radioGroupId (0),
    connectedEdgeFlags (0),
    commandID(),
    buttonState (buttonNormal),
    lastToggleState (false),
    clickTogglesState (false),
    needsToRelease (false),
    needsRepainting (false),
    isKeyDown (false),
    triggerOnMouseDown (false),
    generateTooltip (false)
{
    setWantsKeyboardFocus (true);
    isOn.addListener (this);
}

Button::~Button()
{
    isOn.removeListener (this);

    if (commandManagerToUse != nullptr)
        commandManagerToUse->removeListener (this);

    repeatTimer = nullptr;
    clearShortcuts();
}

//==============================================================================
void Button::setButtonText (const String& newText)
{
    if (text != newText)
    {
        text = newText;
        repaint();
    }
}

void Button::setTooltip (const String& newTooltip)
{
    SettableTooltipClient::setTooltip (newTooltip);
    generateTooltip = false;
}

String Button::getTooltip()
{
    if (generateTooltip && commandManagerToUse != nullptr && commandID != 0)
    {
        String tt (commandManagerToUse->getDescriptionOfCommand (commandID));

        Array<KeyPress> keyPresses (commandManagerToUse->getKeyMappings()->getKeyPressesAssignedToCommand (commandID));

        for (int i = 0; i < keyPresses.size(); ++i)
        {
            const String key (keyPresses.getReference(i).getTextDescription());

            tt << " [";

            if (key.length() == 1)
                tt << TRANS("shortcut") << ": '" << key << "']";
            else
                tt << key << ']';
        }

        return tt;
    }

    return SettableTooltipClient::getTooltip();
}

void Button::setConnectedEdges (const int newFlags)
{
    if (connectedEdgeFlags != newFlags)
    {
        connectedEdgeFlags = newFlags;
        repaint();
    }
}

//==============================================================================
void Button::setToggleState (const bool shouldBeOn, const NotificationType notification)
{
    if (shouldBeOn != lastToggleState)
    {
        if (getToggleState() != shouldBeOn)  // this test means that if the value is void rather than explicitly set to
            isOn = shouldBeOn;               // false, it won't be changed unless the required value is true.

        lastToggleState = shouldBeOn;
        repaint();

        WeakReference<Component> deletionWatcher (this);

        if (notification != dontSendNotification)
        {
            // async callbacks aren't possible here
            jassert (notification != sendNotificationAsync);

            sendClickMessage (ModifierKeys());

            if (deletionWatcher == nullptr)
                return;
        }

        if (lastToggleState)
        {
            turnOffOtherButtonsInGroup (notification);

            if (deletionWatcher == nullptr)
                return;
        }

        if (notification != dontSendNotification)
            sendStateMessage();
    }
}

void Button::setToggleState (const bool shouldBeOn, bool sendChange)
{
    setToggleState (shouldBeOn, sendChange ? sendNotification : dontSendNotification);
}

void Button::setClickingTogglesState (const bool shouldToggle) noexcept
{
    clickTogglesState = shouldToggle;

    // if you've got clickTogglesState turned on, you shouldn't also connect the button
    // up to be a command invoker. Instead, your command handler must flip the state of whatever
    // it is that this button represents, and the button will update its state to reflect this
    // in the applicationCommandListChanged() method.
    jassert (commandManagerToUse == nullptr || ! clickTogglesState);
}

bool Button::getClickingTogglesState() const noexcept
{
    return clickTogglesState;
}

void Button::valueChanged (Value& value)
{
    if (value.refersToSameSourceAs (isOn))
        setToggleState (isOn.getValue(), sendNotification);
}

void Button::setRadioGroupId (const int newGroupId, NotificationType notification)
{
    if (radioGroupId != newGroupId)
    {
        radioGroupId = newGroupId;

        if (lastToggleState)
            turnOffOtherButtonsInGroup (notification);
    }
}

void Button::turnOffOtherButtonsInGroup (const NotificationType notification)
{
    if (Component* const p = getParentComponent())
    {
        if (radioGroupId != 0)
        {
            WeakReference<Component> deletionWatcher (this);

            for (int i = p->getNumChildComponents(); --i >= 0;)
            {
                Component* const c = p->getChildComponent (i);

                if (c != this)
                {
                    if (Button* const b = dynamic_cast <Button*> (c))
                    {
                        if (b->getRadioGroupId() == radioGroupId)
                        {
                            b->setToggleState (false, notification);

                            if (deletionWatcher == nullptr)
                                return;
                        }
                    }
                }
            }
        }
    }
}

//==============================================================================
void Button::enablementChanged()
{
    updateState();
    repaint();
}

Button::ButtonState Button::updateState()
{
    return updateState (isMouseOver (true), isMouseButtonDown());
}

Button::ButtonState Button::updateState (const bool over, const bool down)
{
    ButtonState newState = buttonNormal;

    if (isEnabled() && isVisible() && ! isCurrentlyBlockedByAnotherModalComponent())
    {
        if ((down && (over || (triggerOnMouseDown && buttonState == buttonDown))) || isKeyDown)
            newState = buttonDown;
        else if (over)
            newState = buttonOver;
    }

    setState (newState);
    return newState;
}

void Button::setState (const ButtonState newState)
{
    if (buttonState != newState)
    {
        buttonState = newState;
        repaint();

        if (buttonState == buttonDown)
        {
            buttonPressTime = Time::getApproximateMillisecondCounter();
            lastRepeatTime = 0;
        }

        sendStateMessage();
    }
}

bool Button::isDown() const noexcept    { return buttonState == buttonDown; }
bool Button::isOver() const noexcept    { return buttonState != buttonNormal; }

void Button::buttonStateChanged() {}

uint32 Button::getMillisecondsSinceButtonDown() const noexcept
{
    const uint32 now = Time::getApproximateMillisecondCounter();
    return now > buttonPressTime ? now - buttonPressTime : 0;
}

void Button::setTriggeredOnMouseDown (const bool isTriggeredOnMouseDown) noexcept
{
    triggerOnMouseDown = isTriggeredOnMouseDown;
}

//==============================================================================
void Button::clicked()
{
}

void Button::clicked (const ModifierKeys&)
{
    clicked();
}

enum { clickMessageId = 0x2f3f4f99 };

void Button::triggerClick()
{
    postCommandMessage (clickMessageId);
}

void Button::internalClickCallback (const ModifierKeys& modifiers)
{
    if (clickTogglesState)
        setToggleState (radioGroupId != 0 || ! lastToggleState, sendNotification);

    if (radioGroupId != 0 || ! clickTogglesState)
        sendClickMessage (modifiers);
}

void Button::flashButtonState()
{
    if (isEnabled())
    {
        needsToRelease = true;
        setState (buttonDown);
        getRepeatTimer().startTimer (100);
    }
}

void Button::handleCommandMessage (int commandId)
{
    if (commandId == clickMessageId)
    {
        if (isEnabled())
        {
            flashButtonState();
            internalClickCallback (ModifierKeys::getCurrentModifiers());
        }
    }
    else
    {
        Component::handleCommandMessage (commandId);
    }
}

//==============================================================================
void Button::addListener (ButtonListener* const newListener)
{
    buttonListeners.add (newListener);
}

void Button::removeListener (ButtonListener* const listener)
{
    buttonListeners.remove (listener);
}

void Button::sendClickMessage (const ModifierKeys& modifiers)
{
    Component::BailOutChecker checker (this);

    if (commandManagerToUse != nullptr && commandID != 0)
    {
        ApplicationCommandTarget::InvocationInfo info (commandID);
        info.invocationMethod = ApplicationCommandTarget::InvocationInfo::fromButton;
        info.originatingComponent = this;

        commandManagerToUse->invoke (info, true);
    }

    clicked (modifiers);

    if (! checker.shouldBailOut())
        buttonListeners.callChecked (checker, &ButtonListener::buttonClicked, this);  // (can't use Button::Listener due to idiotic VC2005 bug)
}

void Button::sendStateMessage()
{
    Component::BailOutChecker checker (this);

    buttonStateChanged();

    if (! checker.shouldBailOut())
        buttonListeners.callChecked (checker, &ButtonListener::buttonStateChanged, this);
}

//==============================================================================
void Button::paint (Graphics& g)
{
    if (needsToRelease && isEnabled())
    {
        needsToRelease = false;
        needsRepainting = true;
    }

    paintButton (g, isOver(), isDown());
}

//==============================================================================
void Button::mouseEnter (const MouseEvent&)     { updateState (true,  false); }
void Button::mouseExit (const MouseEvent&)      { updateState (false, false); }

void Button::mouseDown (const MouseEvent& e)
{
    updateState (true, true);

    if (isDown())
    {
        if (autoRepeatDelay >= 0)
            getRepeatTimer().startTimer (autoRepeatDelay);

        if (triggerOnMouseDown)
            internalClickCallback (e.mods);
    }
}

void Button::mouseUp (const MouseEvent& e)
{
    const bool wasDown = isDown();
    const bool wasOver = isOver();
    updateState (isMouseOver(), false);

    if (wasDown && wasOver && ! triggerOnMouseDown)
        internalClickCallback (e.mods);
}

void Button::mouseDrag (const MouseEvent&)
{
    const ButtonState oldState = buttonState;
    updateState (isMouseOver(), true);

    if (autoRepeatDelay >= 0 && buttonState != oldState && isDown())
        getRepeatTimer().startTimer (autoRepeatSpeed);
}

void Button::focusGained (FocusChangeType)
{
    updateState();
    repaint();
}

void Button::focusLost (FocusChangeType)
{
    updateState();
    repaint();
}

void Button::visibilityChanged()
{
    needsToRelease = false;
    updateState();
}

void Button::parentHierarchyChanged()
{
    Component* const newKeySource = (shortcuts.size() == 0) ? nullptr : getTopLevelComponent();

    if (newKeySource != keySource.get())
    {
        if (keySource != nullptr)
            keySource->removeKeyListener (this);

        keySource = newKeySource;

        if (keySource != nullptr)
            keySource->addKeyListener (this);
    }
}

//==============================================================================
void Button::setCommandToTrigger (ApplicationCommandManager* const newCommandManager,
                                  const CommandID newCommandID, const bool generateTip)
{
    commandID = newCommandID;
    generateTooltip = generateTip;

    if (commandManagerToUse != newCommandManager)
    {
        if (commandManagerToUse != nullptr)
            commandManagerToUse->removeListener (this);

        commandManagerToUse = newCommandManager;

        if (commandManagerToUse != nullptr)
            commandManagerToUse->addListener (this);

        // if you've got clickTogglesState turned on, you shouldn't also connect the button
        // up to be a command invoker. Instead, your command handler must flip the state of whatever
        // it is that this button represents, and the button will update its state to reflect this
        // in the applicationCommandListChanged() method.
        jassert (commandManagerToUse == nullptr || ! clickTogglesState);
    }

    if (commandManagerToUse != nullptr)
        applicationCommandListChanged();
    else
        setEnabled (true);
}

void Button::applicationCommandInvoked (const ApplicationCommandTarget::InvocationInfo& info)
{
    if (info.commandID == commandID
         && (info.commandFlags & ApplicationCommandInfo::dontTriggerVisualFeedback) == 0)
    {
        flashButtonState();
    }
}

void Button::applicationCommandListChanged()
{
    if (commandManagerToUse != nullptr)
    {
        ApplicationCommandInfo info (0);

        if (commandManagerToUse->getTargetForCommand (commandID, info) != nullptr)
        {
            setEnabled ((info.flags & ApplicationCommandInfo::isDisabled) == 0);
            setToggleState ((info.flags & ApplicationCommandInfo::isTicked) != 0, dontSendNotification);
        }
        else
        {
            setEnabled (false);
        }
    }
}

//==============================================================================
void Button::addShortcut (const KeyPress& key)
{
    if (key.isValid())
    {
        jassert (! isRegisteredForShortcut (key));  // already registered!

        shortcuts.add (key);
        parentHierarchyChanged();
    }
}

void Button::clearShortcuts()
{
    shortcuts.clear();
    parentHierarchyChanged();
}

bool Button::isShortcutPressed() const
{
    if (! isCurrentlyBlockedByAnotherModalComponent())
        for (int i = shortcuts.size(); --i >= 0;)
            if (shortcuts.getReference(i).isCurrentlyDown())
                return true;

    return false;
}

bool Button::isRegisteredForShortcut (const KeyPress& key) const
{
    for (int i = shortcuts.size(); --i >= 0;)
        if (key == shortcuts.getReference(i))
            return true;

    return false;
}

bool Button::keyStateChanged (const bool, Component*)
{
    if (! isEnabled())
        return false;

    const bool wasDown = isKeyDown;
    isKeyDown = isShortcutPressed();

    if (autoRepeatDelay >= 0 && (isKeyDown && ! wasDown))
        getRepeatTimer().startTimer (autoRepeatDelay);

    updateState();

    if (isEnabled() && wasDown && ! isKeyDown)
    {
        internalClickCallback (ModifierKeys::getCurrentModifiers());

        // (return immediately - this button may now have been deleted)
        return true;
    }

    return wasDown || isKeyDown;
}

bool Button::keyPressed (const KeyPress&, Component*)
{
    // returning true will avoid forwarding events for keys that we're using as shortcuts
    return isShortcutPressed();
}

bool Button::keyPressed (const KeyPress& key)
{
    if (isEnabled() && key.isKeyCode (KeyPress::returnKey))
    {
        triggerClick();
        return true;
    }

    return false;
}

//==============================================================================
void Button::setRepeatSpeed (const int initialDelayMillisecs,
                             const int repeatMillisecs,
                             const int minimumDelayInMillisecs) noexcept
{
    autoRepeatDelay = initialDelayMillisecs;
    autoRepeatSpeed = repeatMillisecs;
    autoRepeatMinimumDelay = jmin (autoRepeatSpeed, minimumDelayInMillisecs);
}

void Button::repeatTimerCallback()
{
    if (needsRepainting)
    {
        getRepeatTimer().stopTimer();
        updateState();
        needsRepainting = false;
    }
    else if (autoRepeatSpeed > 0 && (isKeyDown || (updateState() == buttonDown)))
    {
        int repeatSpeed = autoRepeatSpeed;

        if (autoRepeatMinimumDelay >= 0)
        {
            double timeHeldDown = jmin (1.0, getMillisecondsSinceButtonDown() / 4000.0);
            timeHeldDown *= timeHeldDown;

            repeatSpeed = repeatSpeed + (int) (timeHeldDown * (autoRepeatMinimumDelay - repeatSpeed));
        }

        repeatSpeed = jmax (1, repeatSpeed);

        const uint32 now = Time::getMillisecondCounter();

        // if we've been blocked from repeating often enough, speed up the repeat timer to compensate..
        if (lastRepeatTime != 0 && (int) (now - lastRepeatTime) > repeatSpeed * 2)
            repeatSpeed = jmax (1, repeatSpeed / 2);

        lastRepeatTime = now;
        getRepeatTimer().startTimer (repeatSpeed);

        internalClickCallback (ModifierKeys::getCurrentModifiers());
    }
    else if (! needsToRelease)
    {
        getRepeatTimer().stopTimer();
    }
}

Button::RepeatTimer& Button::getRepeatTimer()
{
    if (repeatTimer == nullptr)
        repeatTimer = new RepeatTimer (*this);

    return *repeatTimer;
}
