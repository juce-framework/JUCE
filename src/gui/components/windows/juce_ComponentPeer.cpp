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

#include "../../../application/juce_Application.h"
#include "../juce_Component.h"
#include "../juce_ComponentDeletionWatcher.h"
#include "../juce_Desktop.h"
#include "../../../events/juce_MessageManager.h"
#include "../../../core/juce_Time.h"
#include "../../../core/juce_Random.h"
#include "../layout/juce_ComponentBoundsConstrainer.h"
#include "../mouse/juce_FileDragAndDropTarget.h"

//#define JUCE_ENABLE_REPAINT_DEBUGGING 1


//==============================================================================
// these are over in juce_component.cpp
extern int64 juce_recentMouseDownTimes[4];
extern int juce_recentMouseDownX [4];
extern int juce_recentMouseDownY [4];
extern Component* juce_recentMouseDownComponent [4];
extern int juce_LastMousePosX;
extern int juce_LastMousePosY;
extern int juce_MouseClickCounter;
extern bool juce_MouseHasMovedSignificantlySincePressed;

static const int fakeMouseMoveMessage = 0x7fff00ff;

static VoidArray heavyweightPeers;


//==============================================================================
ComponentPeer::ComponentPeer (Component* const component_,
                              const int styleFlags_) throw()
    : component (component_),
      styleFlags (styleFlags_),
      lastPaintTime (0),
      constrainer (0),
      lastFocusedComponent (0),
      lastDragAndDropCompUnderMouse (0),
      fakeMouseMessageSent (false),
      isWindowMinimised (false)
{
    heavyweightPeers.add (this);
}

ComponentPeer::~ComponentPeer()
{
    heavyweightPeers.removeValue (this);

    Desktop::getInstance().triggerFocusCallback();
}

//==============================================================================
int ComponentPeer::getNumPeers() throw()
{
    return heavyweightPeers.size();
}

ComponentPeer* ComponentPeer::getPeer (const int index) throw()
{
    return (ComponentPeer*) heavyweightPeers [index];
}

ComponentPeer* ComponentPeer::getPeerFor (const Component* const component) throw()
{
    for (int i = heavyweightPeers.size(); --i >= 0;)
    {
        ComponentPeer* const peer = (ComponentPeer*) heavyweightPeers.getUnchecked(i);

        if (peer->getComponent() == component)
            return peer;
    }

    return 0;
}

bool ComponentPeer::isValidPeer (const ComponentPeer* const peer) throw()
{
    return heavyweightPeers.contains (const_cast <ComponentPeer*> (peer));
}

void ComponentPeer::updateCurrentModifiers() throw()
{
    ModifierKeys::updateCurrentModifiers();
}

//==============================================================================
void ComponentPeer::handleMouseEnter (int x, int y, const int64 time)
{
    jassert (component->isValidComponent());
    updateCurrentModifiers();

    Component* c = component->getComponentAt (x, y);
    const ComponentDeletionWatcher deletionChecker (component);

    if (c != Component::componentUnderMouse && Component::componentUnderMouse != 0)
    {
        jassert (Component::componentUnderMouse->isValidComponent());

        const int oldX = x;
        const int oldY = y;
        component->relativePositionToOtherComponent (Component::componentUnderMouse, x, y);
        Component::componentUnderMouse->internalMouseExit (x, y, time);
        Component::componentUnderMouse = 0;

        if (deletionChecker.hasBeenDeleted())
            return;

        c = component->getComponentAt (oldX, oldY);
    }

    Component::componentUnderMouse = c;

    if (Component::componentUnderMouse != 0)
    {
        component->relativePositionToOtherComponent (Component::componentUnderMouse, x, y);
        Component::componentUnderMouse->internalMouseEnter (x, y, time);
    }
}

void ComponentPeer::handleMouseMove (int x, int y, const int64 time)
{
    jassert (component->isValidComponent());
    updateCurrentModifiers();

    fakeMouseMessageSent = false;

    const ComponentDeletionWatcher deletionChecker (component);
    Component* c = component->getComponentAt (x, y);

    if (c != Component::componentUnderMouse)
    {
        const int oldX = x;
        const int oldY = y;

        if (Component::componentUnderMouse != 0)
        {
            component->relativePositionToOtherComponent (Component::componentUnderMouse, x, y);
            Component::componentUnderMouse->internalMouseExit (x, y, time);
            x = oldX;
            y = oldY;

            Component::componentUnderMouse = 0;

            if (deletionChecker.hasBeenDeleted())
                return; // if this window has just been deleted..

            c = component->getComponentAt (x, y);
        }

        Component::componentUnderMouse = c;

        if (c != 0)
        {
            component->relativePositionToOtherComponent (c, x, y);
            c->internalMouseEnter (x, y, time);
            x = oldX;
            y = oldY;

            if (deletionChecker.hasBeenDeleted())
                return; // if this window has just been deleted..
        }
    }

    if (Component::componentUnderMouse != 0)
    {
        component->relativePositionToOtherComponent (Component::componentUnderMouse, x, y);
        Component::componentUnderMouse->internalMouseMove (x, y, time);
    }
}

void ComponentPeer::handleMouseDown (int x, int y, const int64 time)
{
    ++juce_MouseClickCounter;

    updateCurrentModifiers();

    int numMouseButtonsDown = 0;

    if (ModifierKeys::getCurrentModifiers().isLeftButtonDown())
        ++numMouseButtonsDown;

    if (ModifierKeys::getCurrentModifiers().isRightButtonDown())
        ++numMouseButtonsDown;

    if (ModifierKeys::getCurrentModifiers().isMiddleButtonDown())
        ++numMouseButtonsDown;

    if (numMouseButtonsDown == 1)
    {
        Component::componentUnderMouse = component->getComponentAt (x, y);

        if (Component::componentUnderMouse != 0)
        {
            // can't set these in the mouseDownInt() method, because it's re-entrant, so do it here..

            for (int i = numElementsInArray (juce_recentMouseDownTimes); --i > 0;)
            {
                juce_recentMouseDownTimes [i] = juce_recentMouseDownTimes [i - 1];
                juce_recentMouseDownX [i] = juce_recentMouseDownX [i - 1];
                juce_recentMouseDownY [i] = juce_recentMouseDownY [i - 1];
                juce_recentMouseDownComponent [i] = juce_recentMouseDownComponent [i - 1];
            }

            juce_recentMouseDownTimes[0] = time;
            juce_recentMouseDownX[0] = x;
            juce_recentMouseDownY[0] = y;
            juce_recentMouseDownComponent[0] = Component::componentUnderMouse;
            relativePositionToGlobal (juce_recentMouseDownX[0], juce_recentMouseDownY[0]);
            juce_MouseHasMovedSignificantlySincePressed = false;

            component->relativePositionToOtherComponent (Component::componentUnderMouse, x, y);
            Component::componentUnderMouse->internalMouseDown (x, y);
        }
    }
}

void ComponentPeer::handleMouseDrag (int x, int y, const int64 time)
{
    updateCurrentModifiers();

    if (Component::componentUnderMouse != 0)
    {
        component->relativePositionToOtherComponent (Component::componentUnderMouse, x, y);

        Component::componentUnderMouse->internalMouseDrag (x, y, time);
    }
}

void ComponentPeer::handleMouseUp (const int oldModifiers, int x, int y, const int64 time)
{
    updateCurrentModifiers();

    int numMouseButtonsDown = 0;

    if ((oldModifiers & ModifierKeys::leftButtonModifier) != 0)
        ++numMouseButtonsDown;

    if ((oldModifiers & ModifierKeys::rightButtonModifier) != 0)
        ++numMouseButtonsDown;

    if ((oldModifiers & ModifierKeys::middleButtonModifier) != 0)
        ++numMouseButtonsDown;

    if (numMouseButtonsDown == 1)
    {
        const ComponentDeletionWatcher deletionChecker (component);
        Component* c = component->getComponentAt (x, y);

        if (c != Component::componentUnderMouse)
        {
            const int oldX = x;
            const int oldY = y;

            if (Component::componentUnderMouse != 0)
            {
                component->relativePositionToOtherComponent (Component::componentUnderMouse, x, y);
                Component::componentUnderMouse->internalMouseUp (oldModifiers, x, y, time);
                x = oldX;
                y = oldY;

                if (Component::componentUnderMouse != 0)
                    Component::componentUnderMouse->internalMouseExit (x, y, time);

                if (deletionChecker.hasBeenDeleted())
                    return;

                c = component->getComponentAt (oldX, oldY);
            }

            Component::componentUnderMouse = c;

            if (Component::componentUnderMouse != 0)
            {
                component->relativePositionToOtherComponent (Component::componentUnderMouse, x, y);
                Component::componentUnderMouse->internalMouseEnter (x, y, time);
            }
        }
        else
        {
            if (Component::componentUnderMouse != 0)
            {
                component->relativePositionToOtherComponent (Component::componentUnderMouse, x, y);
                Component::componentUnderMouse->internalMouseUp (oldModifiers, x, y, time);
            }
        }
    }
}

void ComponentPeer::handleMouseExit (int x, int y, const int64 time)
{
    jassert (component->isValidComponent());
    updateCurrentModifiers();

    if (Component::componentUnderMouse != 0)
    {
        component->relativePositionToOtherComponent (Component::componentUnderMouse, x, y);

        Component::componentUnderMouse->internalMouseExit (x, y, time);
        Component::componentUnderMouse = 0;
    }
}

void ComponentPeer::handleMouseWheel (const int amountX, const int amountY, const int64 time)
{
    updateCurrentModifiers();

    if (Component::componentUnderMouse != 0)
        Component::componentUnderMouse->internalMouseWheel (amountX, amountY, time);
}

void ComponentPeer::sendFakeMouseMove() throw()
{
    if ((! fakeMouseMessageSent)
         && component->flags.hasHeavyweightPeerFlag
         && ! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown())
    {
        if (! isMinimised())
        {
            int realX, realY, realW, realH;
            getBounds (realX, realY, realW, realH);

            component->bounds_.setBounds (realX, realY, realW, realH);
        }

        int x, y;
        component->getMouseXYRelative (x, y);

        if (((unsigned int) x) < (unsigned int) component->getWidth()
             && ((unsigned int) y) < (unsigned int) component->getHeight()
             && contains (x, y, false))
        {
            postMessage (new Message (fakeMouseMoveMessage, x, y, 0));
        }

        fakeMouseMessageSent = true;
    }
}

void ComponentPeer::handleMessage (const Message& message)
{
    if (message.intParameter1 == fakeMouseMoveMessage)
    {
        if (! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown())
            handleMouseMove (message.intParameter2,
                             message.intParameter3,
                             Time::currentTimeMillis());
    }
}

//==============================================================================
void ComponentPeer::handlePaint (LowLevelGraphicsContext& contextToPaintTo)
{
    Graphics g (&contextToPaintTo);

#if JUCE_ENABLE_REPAINT_DEBUGGING
    g.saveState();
#endif

    JUCE_TRY
    {
        component->paintEntireComponent (g);
    }
    JUCE_CATCH_EXCEPTION

#if JUCE_ENABLE_REPAINT_DEBUGGING
    // enabling this code will fill all areas that get repainted with a colour overlay, to show
    // clearly when things are being repainted.
    {
        g.restoreState();

        g.fillAll (Colour ((uint8) Random::getSystemRandom().nextInt (255),
                           (uint8) Random::getSystemRandom().nextInt (255),
                           (uint8) Random::getSystemRandom().nextInt (255),
                           (uint8) 0x50));
    }
#endif
}

bool ComponentPeer::handleKeyPress (const int keyCode,
                                    const juce_wchar textCharacter)
{
    updateCurrentModifiers();

    Component* target = Component::currentlyFocusedComponent->isValidComponent()
                            ? Component::currentlyFocusedComponent
                            : component;

    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        Component* const currentModalComp = Component::getCurrentlyModalComponent();

        if (currentModalComp != 0)
            target = currentModalComp;
    }

    const KeyPress keyInfo (keyCode,
                            ModifierKeys::getCurrentModifiers().getRawFlags()
                               & ModifierKeys::allKeyboardModifiers,
                            textCharacter);

    bool keyWasUsed = false;

    while (target != 0)
    {
        const ComponentDeletionWatcher deletionChecker (target);

        if (target->keyListeners_ != 0)
        {
            for (int i = target->keyListeners_->size(); --i >= 0;)
            {
                keyWasUsed = ((KeyListener*) target->keyListeners_->getUnchecked(i))->keyPressed (keyInfo, target);

                if (keyWasUsed || deletionChecker.hasBeenDeleted())
                    return keyWasUsed;

                i = jmin (i, target->keyListeners_->size());
            }
        }

        keyWasUsed = target->keyPressed (keyInfo);

        if (keyWasUsed || deletionChecker.hasBeenDeleted())
            break;

        if (keyInfo.isKeyCode (KeyPress::tabKey) && Component::getCurrentlyFocusedComponent() != 0)
        {
            Component* const currentlyFocused = Component::getCurrentlyFocusedComponent();
            currentlyFocused->moveKeyboardFocusToSibling (! keyInfo.getModifiers().isShiftDown());
            keyWasUsed = (currentlyFocused != Component::getCurrentlyFocusedComponent());
            break;
        }

        target = target->parentComponent_;
    }

    return keyWasUsed;
}

bool ComponentPeer::handleKeyUpOrDown (const bool isKeyDown)
{
    updateCurrentModifiers();

    Component* target = Component::currentlyFocusedComponent->isValidComponent()
                            ? Component::currentlyFocusedComponent
                            : component;

    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        Component* const currentModalComp = Component::getCurrentlyModalComponent();

        if (currentModalComp != 0)
            target = currentModalComp;
    }

    bool keyWasUsed = false;

    while (target != 0)
    {
        const ComponentDeletionWatcher deletionChecker (target);

        keyWasUsed = target->keyStateChanged (isKeyDown);

        if (keyWasUsed || deletionChecker.hasBeenDeleted())
            break;

        if (target->keyListeners_ != 0)
        {
            for (int i = target->keyListeners_->size(); --i >= 0;)
            {
                keyWasUsed = ((KeyListener*) target->keyListeners_->getUnchecked(i))->keyStateChanged (isKeyDown, target);

                if (keyWasUsed || deletionChecker.hasBeenDeleted())
                    return keyWasUsed;

                i = jmin (i, target->keyListeners_->size());
            }
        }

        target = target->parentComponent_;
    }

    return keyWasUsed;
}

void ComponentPeer::handleModifierKeysChange()
{
    updateCurrentModifiers();

    Component* target = Component::getComponentUnderMouse();

    if (target == 0)
        target = Component::getCurrentlyFocusedComponent();

    if (target == 0)
        target = component;

    if (target->isValidComponent())
        target->internalModifierKeysChanged();
}

//==============================================================================
void ComponentPeer::handleBroughtToFront()
{
    updateCurrentModifiers();

    if (component != 0)
        component->internalBroughtToFront();
}

void ComponentPeer::setConstrainer (ComponentBoundsConstrainer* const newConstrainer) throw()
{
    constrainer = newConstrainer;
}

void ComponentPeer::handleMovedOrResized()
{
    jassert (component->isValidComponent());
    updateCurrentModifiers();

    const bool nowMinimised = isMinimised();

    if (component->flags.hasHeavyweightPeerFlag && ! nowMinimised)
    {
        const ComponentDeletionWatcher deletionChecker (component);

        int realX, realY, realW, realH;
        getBounds (realX, realY, realW, realH);

        const bool wasMoved   = (component->getX() != realX || component->getY() != realY);
        const bool wasResized = (component->getWidth() != realW || component->getHeight() != realH);

        if (wasMoved || wasResized)
        {
            component->bounds_.setBounds (realX, realY, realW, realH);

            if (wasResized)
                component->repaint();

            component->sendMovedResizedMessages (wasMoved, wasResized);

            if (deletionChecker.hasBeenDeleted())
                return;
        }
    }

    if (isWindowMinimised != nowMinimised)
    {
        isWindowMinimised = nowMinimised;
        component->minimisationStateChanged (nowMinimised);
        component->sendVisibilityChangeMessage();
    }

    if (! isFullScreen())
        lastNonFullscreenBounds = component->getBounds();
}

void ComponentPeer::handleFocusGain()
{
    updateCurrentModifiers();

    if (component->isParentOf (lastFocusedComponent))
    {
        Component::currentlyFocusedComponent = lastFocusedComponent;
        Desktop::getInstance().triggerFocusCallback();
        lastFocusedComponent->internalFocusGain (Component::focusChangedDirectly);
    }
    else
    {
        if (! component->isCurrentlyBlockedByAnotherModalComponent())
            component->grabKeyboardFocus();
        else
            Component::bringModalComponentToFront();
    }
}

void ComponentPeer::handleFocusLoss()
{
    updateCurrentModifiers();

    if (component->hasKeyboardFocus (true))
    {
        lastFocusedComponent = Component::currentlyFocusedComponent;

        if (lastFocusedComponent != 0)
        {
            Component::currentlyFocusedComponent = 0;
            Desktop::getInstance().triggerFocusCallback();
            lastFocusedComponent->internalFocusLoss (Component::focusChangedByMouseClick);
        }
    }
}

Component* ComponentPeer::getLastFocusedSubcomponent() const throw()
{
    return (component->isParentOf (lastFocusedComponent) && lastFocusedComponent->isShowing())
                ? lastFocusedComponent
                : component;
}

void ComponentPeer::handleScreenSizeChange()
{
    updateCurrentModifiers();

    component->parentSizeChanged();
    handleMovedOrResized();
}

void ComponentPeer::setNonFullScreenBounds (const Rectangle& newBounds) throw()
{
    lastNonFullscreenBounds = newBounds;
}

const Rectangle& ComponentPeer::getNonFullScreenBounds() const throw()
{
    return lastNonFullscreenBounds;
}

//==============================================================================
static FileDragAndDropTarget* findDragAndDropTarget (Component* c,
                                                     const StringArray& files,
                                                     FileDragAndDropTarget* const lastOne)
{
    while (c != 0)
    {
        FileDragAndDropTarget* const t = dynamic_cast <FileDragAndDropTarget*> (c);

        if (t != 0 && (t == lastOne || t->isInterestedInFileDrag (files)))
            return t;

        c = c->getParentComponent();
    }

    return 0;
}

void ComponentPeer::handleFileDragMove (const StringArray& files, int x, int y)
{
    updateCurrentModifiers();

    FileDragAndDropTarget* lastTarget = 0;

    if (dragAndDropTargetComponent != 0 && ! dragAndDropTargetComponent->hasBeenDeleted())
        lastTarget = const_cast <FileDragAndDropTarget*> (dynamic_cast <const FileDragAndDropTarget*> (dragAndDropTargetComponent->getComponent()));

    FileDragAndDropTarget* newTarget = 0;

    Component* const compUnderMouse = component->getComponentAt (x, y);

    if (compUnderMouse != lastDragAndDropCompUnderMouse)
    {
        lastDragAndDropCompUnderMouse = compUnderMouse;
        newTarget = findDragAndDropTarget (compUnderMouse, files, lastTarget);

        if (newTarget != lastTarget)
        {
            if (lastTarget != 0)
                lastTarget->fileDragExit (files);

            dragAndDropTargetComponent = 0;

            if (newTarget != 0)
            {
                Component* const targetComp = dynamic_cast <Component*> (newTarget);
                int mx = x, my = y;
                component->relativePositionToOtherComponent (targetComp, mx, my);

                dragAndDropTargetComponent = new ComponentDeletionWatcher (dynamic_cast <Component*> (newTarget));
                newTarget->fileDragEnter (files, mx, my);
            }
        }
    }
    else
    {
        newTarget = lastTarget;
    }

    if (newTarget != 0)
    {
        Component* const targetComp = dynamic_cast <Component*> (newTarget);
        component->relativePositionToOtherComponent (targetComp, x, y);

        newTarget->fileDragMove (files, x, y);
    }
}

void ComponentPeer::handleFileDragExit (const StringArray& files)
{
    handleFileDragMove (files, -1, -1);

    jassert (dragAndDropTargetComponent == 0);
    lastDragAndDropCompUnderMouse = 0;
}

void ComponentPeer::handleFileDragDrop (const StringArray& files, int x, int y)
{
    handleFileDragMove (files, x, y);

    if (dragAndDropTargetComponent != 0 && ! dragAndDropTargetComponent->hasBeenDeleted())
    {
        FileDragAndDropTarget* const target = const_cast <FileDragAndDropTarget*> (dynamic_cast <const FileDragAndDropTarget*> (dragAndDropTargetComponent->getComponent()));

        dragAndDropTargetComponent = 0;
        lastDragAndDropCompUnderMouse = 0;

        if (target != 0)
        {
            Component* const targetComp = dynamic_cast <Component*> (target);

            if (targetComp->isCurrentlyBlockedByAnotherModalComponent())
            {
                targetComp->internalModalInputAttempt();

                if (targetComp->isCurrentlyBlockedByAnotherModalComponent())
                    return;
            }

            component->relativePositionToOtherComponent (targetComp, x, y);
            target->filesDropped (files, x, y);
        }
    }
}

//==============================================================================
void ComponentPeer::handleUserClosingWindow()
{
    updateCurrentModifiers();

    component->userTriedToCloseWindow();
}

//==============================================================================
void ComponentPeer::bringModalComponentToFront()
{
    Component::bringModalComponentToFront();
}

//==============================================================================
void ComponentPeer::clearMaskedRegion() throw()
{
    maskedRegion.clear();
}

void ComponentPeer::addMaskedRegion (int x, int y, int w, int h) throw()
{
    maskedRegion.add (x, y, w, h);
}

//==============================================================================
const StringArray ComponentPeer::getAvailableRenderingEngines() throw()
{
    StringArray s;
    s.add ("Software Renderer");
    return s;
}

int ComponentPeer::getCurrentRenderingEngine() throw()
{
    return 0;
}

void ComponentPeer::setCurrentRenderingEngine (int /*index*/) throw()
{
}

END_JUCE_NAMESPACE
