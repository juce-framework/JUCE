/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "juce_Button.h"
#include "../keyboard/juce_KeyPressMappingSet.h"
#include "../../../text/juce_LocalisedStrings.h"
#include "../../../events/juce_Timer.h"


//==============================================================================
class Button::RepeatTimer  : public Timer
{
public:
    RepeatTimer (Button& owner_) : owner (owner_)   {}
    void timerCallback()    { owner.repeatTimerCallback(); }

    juce_UseDebuggingNewOperator

private:
    Button& owner;

    RepeatTimer (const RepeatTimer&);
    RepeatTimer& operator= (const RepeatTimer&);
};

//==============================================================================
Button::Button (const String& name)
  : Component (name),
    text (name),
    buttonPressTime (0),
    lastTimeCallbackTime (0),
    commandManagerToUse (0),
    autoRepeatDelay (-1),
    autoRepeatSpeed (0),
    autoRepeatMinimumDelay (-1),
    radioGroupId (0),
    commandID (0),
    connectedEdgeFlags (0),
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

    if (commandManagerToUse != 0)
        commandManagerToUse->removeListener (this);

    repeatTimer = 0;
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

const String Button::getTooltip()
{
    if (generateTooltip && commandManagerToUse != 0 && commandID != 0)
    {
        String tt (commandManagerToUse->getDescriptionOfCommand (commandID));

        Array <KeyPress> keyPresses (commandManagerToUse->getKeyMappings()->getKeyPressesAssignedToCommand (commandID));

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

void Button::setConnectedEdges (const int connectedEdgeFlags_)
{
    if (connectedEdgeFlags != connectedEdgeFlags_)
    {
        connectedEdgeFlags = connectedEdgeFlags_;
        repaint();
    }
}

//==============================================================================
void Button::setToggleState (const bool shouldBeOn,
                             const bool sendChangeNotification)
{
    if (shouldBeOn != lastToggleState)
    {
        if (isOn != shouldBeOn)  // this test means that if the value is void rather than explicitly set to
            isOn = shouldBeOn;   // false, it won't be changed unless the required value is true.

        lastToggleState = shouldBeOn;
        repaint();

        if (sendChangeNotification)
        {
            Component::SafePointer<Component> deletionWatcher (this);
            sendClickMessage (ModifierKeys());

            if (deletionWatcher == 0)
                return;
        }

        if (lastToggleState)
            turnOffOtherButtonsInGroup (sendChangeNotification);
    }
}

void Button::setClickingTogglesState (const bool shouldToggle) throw()
{
    clickTogglesState = shouldToggle;

    // if you've got clickTogglesState turned on, you shouldn't also connect the button
    // up to be a command invoker. Instead, your command handler must flip the state of whatever
    // it is that this button represents, and the button will update its state to reflect this
    // in the applicationCommandListChanged() method.
    jassert (commandManagerToUse == 0 || ! clickTogglesState);
}

bool Button::getClickingTogglesState() const throw()
{
    return clickTogglesState;
}

void Button::valueChanged (Value& value)
{
    if (value.refersToSameSourceAs (isOn))
        setToggleState (isOn.getValue(), true);
}

void Button::setRadioGroupId (const int newGroupId)
{
    if (radioGroupId != newGroupId)
    {
        radioGroupId = newGroupId;

        if (lastToggleState)
            turnOffOtherButtonsInGroup (true);
    }
}

void Button::turnOffOtherButtonsInGroup (const bool sendChangeNotification)
{
    Component* const p = getParentComponent();

    if (p != 0 && radioGroupId != 0)
    {
        Component::SafePointer<Component> deletionWatcher (this);

        for (int i = p->getNumChildComponents(); --i >= 0;)
        {
            Component* const c = p->getChildComponent (i);

            if (c != this)
            {
                Button* const b = dynamic_cast <Button*> (c);

                if (b != 0 && b->getRadioGroupId() == radioGroupId)
                {
                    b->setToggleState (false, sendChangeNotification);

                    if (deletionWatcher == 0)
                        return;
                }
            }
        }
    }
}

//==============================================================================
void Button::enablementChanged()
{
    updateState (0);
    repaint();
}

Button::ButtonState Button::updateState (const MouseEvent* const e)
{
    ButtonState state = buttonNormal;

    if (isEnabled() && isVisible() && ! isCurrentlyBlockedByAnotherModalComponent())
    {
        Point<int> mousePos;

        if (e == 0)
            mousePos = getMouseXYRelative();
        else
            mousePos = e->getEventRelativeTo (this).getPosition();

        const bool over = reallyContains (mousePos.getX(), mousePos.getY(), true);
        const bool down = isMouseButtonDown();

        if ((down && (over || (triggerOnMouseDown && buttonState == buttonDown))) || isKeyDown)
            state = buttonDown;
        else if (over)
            state = buttonOver;
    }

    setState (state);
    return state;
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
            lastTimeCallbackTime = buttonPressTime;
        }

        sendStateMessage();
    }
}

bool Button::isDown() const throw()
{
    return buttonState == buttonDown;
}

bool Button::isOver() const throw()
{
    return buttonState != buttonNormal;
}

void Button::buttonStateChanged()
{
}

uint32 Button::getMillisecondsSinceButtonDown() const throw()
{
    const uint32 now = Time::getApproximateMillisecondCounter();
    return now > buttonPressTime ? now - buttonPressTime : 0;
}

void Button::setTriggeredOnMouseDown (const bool isTriggeredOnMouseDown) throw()
{
    triggerOnMouseDown = isTriggeredOnMouseDown;
}

//==============================================================================
void Button::clicked()
{
}

void Button::clicked (const ModifierKeys& /*modifiers*/)
{
    clicked();
}

static const int clickMessageId = 0x2f3f4f99;

void Button::triggerClick()
{
    postCommandMessage (clickMessageId);
}

void Button::internalClickCallback (const ModifierKeys& modifiers)
{
    if (clickTogglesState)
        setToggleState ((radioGroupId != 0) || ! lastToggleState, false);

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
void Button::addButtonListener (Listener* const newListener)
{
    buttonListeners.add (newListener);
}

void Button::removeButtonListener (Listener* const listener)
{
    buttonListeners.remove (listener);
}

void Button::sendClickMessage (const ModifierKeys& modifiers)
{
    Component::BailOutChecker checker (this);

    if (commandManagerToUse != 0 && commandID != 0)
    {
        ApplicationCommandTarget::InvocationInfo info (commandID);
        info.invocationMethod = ApplicationCommandTarget::InvocationInfo::fromButton;
        info.originatingComponent = this;

        commandManagerToUse->invoke (info, true);
    }

    clicked (modifiers);

    if (! checker.shouldBailOut())
        buttonListeners.callChecked (checker, &Listener::buttonClicked, this);
}

void Button::sendStateMessage()
{
    Component::BailOutChecker checker (this);

    buttonStateChanged();

    if (! checker.shouldBailOut())
        buttonListeners.callChecked (checker, &Listener::buttonStateChanged, this);
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
void Button::mouseEnter (const MouseEvent& e)
{
    updateState (&e);
}

void Button::mouseExit (const MouseEvent& e)
{
    updateState (&e);
}

void Button::mouseDown (const MouseEvent& e)
{
    updateState (&e);

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
    updateState (&e);

    if (wasDown && isOver() && ! triggerOnMouseDown)
        internalClickCallback (e.mods);
}

void Button::mouseDrag (const MouseEvent& e)
{
    const ButtonState oldState = buttonState;
    updateState (&e);

    if (autoRepeatDelay >= 0 && buttonState != oldState && isDown())
        getRepeatTimer().startTimer (autoRepeatSpeed);
}

void Button::focusGained (FocusChangeType)
{
    updateState (0);
    repaint();
}

void Button::focusLost (FocusChangeType)
{
    updateState (0);
    repaint();
}

//==============================================================================
void Button::setVisible (bool shouldBeVisible)
{
    if (shouldBeVisible != isVisible())
    {
        Component::setVisible (shouldBeVisible);

        if (! shouldBeVisible)
            needsToRelease = false;

        updateState (0);
    }
    else
    {
        Component::setVisible (shouldBeVisible);
    }
}

void Button::parentHierarchyChanged()
{
    Component* const newKeySource = (shortcuts.size() == 0) ? 0 : getTopLevelComponent();

    if (newKeySource != keySource.getComponent())
    {
        if (keySource != 0)
            keySource->removeKeyListener (this);

        keySource = newKeySource;

        if (keySource != 0)
            keySource->addKeyListener (this);
    }
}

//==============================================================================
void Button::setCommandToTrigger (ApplicationCommandManager* const commandManagerToUse_,
                                  const int commandID_,
                                  const bool generateTooltip_)
{
    commandID = commandID_;
    generateTooltip = generateTooltip_;

    if (commandManagerToUse != commandManagerToUse_)
    {
        if (commandManagerToUse != 0)
            commandManagerToUse->removeListener (this);

        commandManagerToUse = commandManagerToUse_;

        if (commandManagerToUse != 0)
            commandManagerToUse->addListener (this);

        // if you've got clickTogglesState turned on, you shouldn't also connect the button
        // up to be a command invoker. Instead, your command handler must flip the state of whatever
        // it is that this button represents, and the button will update its state to reflect this
        // in the applicationCommandListChanged() method.
        jassert (commandManagerToUse == 0 || ! clickTogglesState);
    }

    if (commandManagerToUse != 0)
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
    if (commandManagerToUse != 0)
    {
        ApplicationCommandInfo info (0);

        ApplicationCommandTarget* const target = commandManagerToUse->getTargetForCommand (commandID, info);

        setEnabled (target != 0 && (info.flags & ApplicationCommandInfo::isDisabled) == 0);

        if (target != 0)
            setToggleState ((info.flags & ApplicationCommandInfo::isTicked) != 0, false);
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
    {
        for (int i = shortcuts.size(); --i >= 0;)
            if (shortcuts.getReference(i).isCurrentlyDown())
                return true;
    }

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

    updateState (0);

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
                             const int minimumDelayInMillisecs) throw()
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
        updateState (0);
        needsRepainting = false;
    }
    else if (autoRepeatSpeed > 0 && (isKeyDown || (updateState (0) == buttonDown)))
    {
        int repeatSpeed = autoRepeatSpeed;

        if (autoRepeatMinimumDelay >= 0)
        {
            double timeHeldDown = jmin (1.0, getMillisecondsSinceButtonDown() / 4000.0);
            timeHeldDown *= timeHeldDown;

            repeatSpeed = repeatSpeed + (int) (timeHeldDown * (autoRepeatMinimumDelay - repeatSpeed));
        }

        repeatSpeed = jmax (1, repeatSpeed);

        getRepeatTimer().startTimer (repeatSpeed);

        const uint32 now = Time::getApproximateMillisecondCounter();
        const int numTimesToCallback = (now > lastTimeCallbackTime) ? jmax (1, (int) (now - lastTimeCallbackTime) / repeatSpeed) : 1;

        lastTimeCallbackTime = now;

        Component::SafePointer<Component> deletionWatcher (this);

        for (int i = numTimesToCallback; --i >= 0;)
        {
            internalClickCallback (ModifierKeys::getCurrentModifiers());

            if (deletionWatcher == 0 || ! isDown())
                return;
        }
    }
    else if (! needsToRelease)
    {
        getRepeatTimer().stopTimer();
    }
}

Button::RepeatTimer& Button::getRepeatTimer()
{
    if (repeatTimer == 0)
        repeatTimer = new RepeatTimer (*this);

    return *repeatTimer;
}

END_JUCE_NAMESPACE
