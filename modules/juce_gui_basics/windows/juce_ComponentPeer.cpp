/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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


//#define JUCE_ENABLE_REPAINT_DEBUGGING 1

//==============================================================================
static Array <ComponentPeer*> heavyweightPeers;
static uint32 lastUniqueID = 1;

//==============================================================================
ComponentPeer::ComponentPeer (Component* const component_, const int styleFlags_)
    : component (component_),
      styleFlags (styleFlags_),
      lastPaintTime (0),
      constrainer (nullptr),
      lastDragAndDropCompUnderMouse (nullptr),
      uniqueID (lastUniqueID += 2), // increment by 2 so that this can never hit 0
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
int ComponentPeer::getNumPeers() noexcept
{
    return heavyweightPeers.size();
}

ComponentPeer* ComponentPeer::getPeer (const int index) noexcept
{
    return heavyweightPeers [index];
}

ComponentPeer* ComponentPeer::getPeerFor (const Component* const component) noexcept
{
    for (int i = heavyweightPeers.size(); --i >= 0;)
    {
        ComponentPeer* const peer = heavyweightPeers.getUnchecked(i);

        if (peer->getComponent() == component)
            return peer;
    }

    return nullptr;
}

bool ComponentPeer::isValidPeer (const ComponentPeer* const peer) noexcept
{
    return heavyweightPeers.contains (const_cast <ComponentPeer*> (peer));
}

void ComponentPeer::updateCurrentModifiers() noexcept
{
    ModifierKeys::updateCurrentModifiers();
}

//==============================================================================
void ComponentPeer::handleMouseEvent (const int touchIndex, const Point<int>& positionWithinPeer, const ModifierKeys& newMods, const int64 time)
{
    MouseInputSource* const mouse = Desktop::getInstance().getMouseSource (touchIndex);
    jassert (mouse != nullptr); // not enough sources!

    mouse->handleEvent (this, positionWithinPeer, time, newMods);
}

void ComponentPeer::handleMouseWheel (const int touchIndex, const Point<int>& positionWithinPeer, const int64 time, const float x, const float y)
{
    MouseInputSource* const mouse = Desktop::getInstance().getMouseSource (touchIndex);
    jassert (mouse != nullptr); // not enough sources!

    mouse->handleWheel (this, positionWithinPeer, time, x, y);
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
        component->paintEntireComponent (g, true);
    }
    JUCE_CATCH_EXCEPTION

   #if JUCE_ENABLE_REPAINT_DEBUGGING
    // enabling this code will fill all areas that get repainted with a colour overlay, to show
    // clearly when things are being repainted.
    g.restoreState();

    static Random rng;

    g.fillAll (Colour ((uint8) rng.nextInt (255),
                       (uint8) rng.nextInt (255),
                       (uint8) rng.nextInt (255),
                       (uint8) 0x50));
   #endif

    /** If this fails, it's probably be because your CPU floating-point precision mode has
        been set to low.. This setting is sometimes changed by things like Direct3D, and can
        mess up a lot of the calculations that the library needs to do.
    */
    jassert (roundToInt (10.1f) == 10);
}

bool ComponentPeer::handleKeyPress (const int keyCode,
                                    const juce_wchar textCharacter)
{
    updateCurrentModifiers();

    Component* target = Component::getCurrentlyFocusedComponent() != nullptr
                            ? Component::getCurrentlyFocusedComponent()
                            : component;

    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        Component* const currentModalComp = Component::getCurrentlyModalComponent();

        if (currentModalComp != nullptr)
            target = currentModalComp;
    }

    const KeyPress keyInfo (keyCode,
                            ModifierKeys::getCurrentModifiers().getRawFlags()
                               & ModifierKeys::allKeyboardModifiers,
                            textCharacter);

    bool keyWasUsed = false;

    while (target != nullptr)
    {
        const WeakReference<Component> deletionChecker (target);
        const Array <KeyListener*>* const keyListeners = target->keyListeners;

        if (keyListeners != nullptr)
        {
            for (int i = keyListeners->size(); --i >= 0;)
            {
                keyWasUsed = keyListeners->getUnchecked(i)->keyPressed (keyInfo, target);

                if (keyWasUsed || deletionChecker == nullptr)
                    return keyWasUsed;

                i = jmin (i, keyListeners->size());
            }
        }

        keyWasUsed = target->keyPressed (keyInfo);

        if (keyWasUsed || deletionChecker == nullptr)
            break;

        Component* const currentlyFocused = Component::getCurrentlyFocusedComponent();

        if (currentlyFocused != nullptr)
        {
            const bool isTab      = (keyInfo == KeyPress (KeyPress::tabKey, ModifierKeys::noModifiers, 0));
            const bool isShiftTab = (keyInfo == KeyPress (KeyPress::tabKey, ModifierKeys::shiftModifier, 0));

            if (isTab || isShiftTab)
            {
                currentlyFocused->moveKeyboardFocusToSibling (isTab);
                keyWasUsed = (currentlyFocused != Component::getCurrentlyFocusedComponent());
                break;
            }
        }

        target = target->getParentComponent();
    }

    return keyWasUsed;
}

bool ComponentPeer::handleKeyUpOrDown (const bool isKeyDown)
{
    updateCurrentModifiers();

    Component* target = Component::getCurrentlyFocusedComponent() != nullptr
                            ? Component::getCurrentlyFocusedComponent()
                            : component;

    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        Component* const currentModalComp = Component::getCurrentlyModalComponent();

        if (currentModalComp != nullptr)
            target = currentModalComp;
    }

    bool keyWasUsed = false;

    while (target != nullptr)
    {
        const WeakReference<Component> deletionChecker (target);

        keyWasUsed = target->keyStateChanged (isKeyDown);

        if (keyWasUsed || deletionChecker == nullptr)
            break;

        const Array <KeyListener*>* const keyListeners = target->keyListeners;

        if (keyListeners != nullptr)
        {
            for (int i = keyListeners->size(); --i >= 0;)
            {
                keyWasUsed = keyListeners->getUnchecked(i)->keyStateChanged (isKeyDown, target);

                if (keyWasUsed || deletionChecker == nullptr)
                    return keyWasUsed;

                i = jmin (i, keyListeners->size());
            }
        }

        target = target->getParentComponent();
    }

    return keyWasUsed;
}

void ComponentPeer::handleModifierKeysChange()
{
    updateCurrentModifiers();

    Component* target = Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();

    if (target == nullptr)
        target = Component::getCurrentlyFocusedComponent();

    if (target == nullptr)
        target = component;

    if (target != nullptr)
        target->internalModifierKeysChanged();
}

TextInputTarget* ComponentPeer::findCurrentTextInputTarget()
{
    Component* const c = Component::getCurrentlyFocusedComponent();
    if (component->isParentOf (c))
    {
        TextInputTarget* const ti = dynamic_cast <TextInputTarget*> (c);
        if (ti != nullptr && ti->isTextInputActive())
            return ti;
    }

    return nullptr;
}

void ComponentPeer::dismissPendingTextInput()
{
}

//==============================================================================
void ComponentPeer::handleBroughtToFront()
{
    updateCurrentModifiers();

    if (component != nullptr)
        component->internalBroughtToFront();
}

void ComponentPeer::setConstrainer (ComponentBoundsConstrainer* const newConstrainer) noexcept
{
    constrainer = newConstrainer;
}

void ComponentPeer::handleMovedOrResized()
{
    updateCurrentModifiers();

    const bool nowMinimised = isMinimised();

    if (component->flags.hasHeavyweightPeerFlag && ! nowMinimised)
    {
        const WeakReference<Component> deletionChecker (component);

        const Rectangle<int> newBounds (getBounds());
        const bool wasMoved   = (component->getPosition() != newBounds.getPosition());
        const bool wasResized = (component->getWidth() != newBounds.getWidth() || component->getHeight() != newBounds.getHeight());

        if (wasMoved || wasResized)
        {
            component->bounds = newBounds;

            if (wasResized)
                component->repaint();

            component->sendMovedResizedMessages (wasMoved, wasResized);

            if (deletionChecker == nullptr)
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
            ModalComponentManager::getInstance()->bringModalComponentsToFront();
    }
}

void ComponentPeer::handleFocusLoss()
{
    updateCurrentModifiers();

    if (component->hasKeyboardFocus (true))
    {
        lastFocusedComponent = Component::currentlyFocusedComponent;

        if (lastFocusedComponent != nullptr)
        {
            Component::currentlyFocusedComponent = nullptr;
            Desktop::getInstance().triggerFocusCallback();
            lastFocusedComponent->internalFocusLoss (Component::focusChangedByMouseClick);
        }
    }
}

Component* ComponentPeer::getLastFocusedSubcomponent() const noexcept
{
    return (component->isParentOf (lastFocusedComponent) && lastFocusedComponent->isShowing())
                ? static_cast <Component*> (lastFocusedComponent)
                : component;
}

void ComponentPeer::handleScreenSizeChange()
{
    updateCurrentModifiers();

    component->parentSizeChanged();
    handleMovedOrResized();
}

void ComponentPeer::setNonFullScreenBounds (const Rectangle<int>& newBounds) noexcept
{
    lastNonFullscreenBounds = newBounds;
}

const Rectangle<int>& ComponentPeer::getNonFullScreenBounds() const noexcept
{
    return lastNonFullscreenBounds;
}

Rectangle<int> ComponentPeer::localToGlobal (const Rectangle<int>& relativePosition)
{
    return relativePosition.withPosition (localToGlobal (relativePosition.getPosition()));
}

Rectangle<int> ComponentPeer::globalToLocal (const Rectangle<int>& screenPosition)
{
    return screenPosition.withPosition (globalToLocal (screenPosition.getPosition()));
}

//==============================================================================
namespace ComponentPeerHelpers
{
    FileDragAndDropTarget* findDragAndDropTarget (Component* c,
                                                  const StringArray& files,
                                                  FileDragAndDropTarget* const lastOne)
    {
        while (c != nullptr)
        {
            FileDragAndDropTarget* const t = dynamic_cast <FileDragAndDropTarget*> (c);

            if (t != nullptr && (t == lastOne || t->isInterestedInFileDrag (files)))
                return t;

            c = c->getParentComponent();
        }

        return nullptr;
    }
}

bool ComponentPeer::handleFileDragMove (const StringArray& files, const Point<int>& position)
{
    updateCurrentModifiers();

    FileDragAndDropTarget* lastTarget
        = dynamic_cast<FileDragAndDropTarget*> (static_cast<Component*> (dragAndDropTargetComponent));

    FileDragAndDropTarget* newTarget = nullptr;

    Component* const compUnderMouse = component->getComponentAt (position);

    if (compUnderMouse != lastDragAndDropCompUnderMouse)
    {
        lastDragAndDropCompUnderMouse = compUnderMouse;
        newTarget = ComponentPeerHelpers::findDragAndDropTarget (compUnderMouse, files, lastTarget);

        if (newTarget != lastTarget)
        {
            if (lastTarget != nullptr)
                lastTarget->fileDragExit (files);

            dragAndDropTargetComponent = nullptr;

            if (newTarget != nullptr)
            {
                dragAndDropTargetComponent = dynamic_cast <Component*> (newTarget);
                const Point<int> pos (dragAndDropTargetComponent->getLocalPoint (component, position));
                newTarget->fileDragEnter (files, pos.getX(), pos.getY());
            }
        }
    }
    else
    {
        newTarget = lastTarget;
    }

    if (newTarget == nullptr)
        return false;

    Component* const targetComp = dynamic_cast <Component*> (newTarget);
    const Point<int> pos (targetComp->getLocalPoint (component, position));
    newTarget->fileDragMove (files, pos.getX(), pos.getY());
    return true;
}

bool ComponentPeer::handleFileDragExit (const StringArray& files)
{
    const bool used = handleFileDragMove (files, Point<int> (-1, -1));

    jassert (dragAndDropTargetComponent == nullptr);
    lastDragAndDropCompUnderMouse = nullptr;
    return used;
}

// We'll use an async message to deliver the drop, because if the target decides
// to run a modal loop, it can gum-up the operating system..
class AsyncFileDropMessage  : public CallbackMessage
{
public:
    AsyncFileDropMessage (Component* target_, FileDragAndDropTarget* dropTarget_,
                          const Point<int>& position_, const StringArray& files_)
        : target (target_), dropTarget (dropTarget_), position (position_), files (files_)
    {
    }

    void messageCallback()
    {
        if (target.get() != nullptr)
            dropTarget->filesDropped (files, position.getX(), position.getY());
    }

private:
    WeakReference<Component> target;
    FileDragAndDropTarget* const dropTarget;
    const Point<int> position;
    const StringArray files;

    JUCE_DECLARE_NON_COPYABLE (AsyncFileDropMessage);
};

bool ComponentPeer::handleFileDragDrop (const StringArray& files, const Point<int>& position)
{
    handleFileDragMove (files, position);

    if (dragAndDropTargetComponent != nullptr)
    {
        FileDragAndDropTarget* const target
            = dynamic_cast<FileDragAndDropTarget*> (static_cast<Component*> (dragAndDropTargetComponent));

        dragAndDropTargetComponent = nullptr;
        lastDragAndDropCompUnderMouse = nullptr;

        if (target != nullptr)
        {
            Component* const targetComp = dynamic_cast <Component*> (target);

            if (targetComp->isCurrentlyBlockedByAnotherModalComponent())
            {
                targetComp->internalModalInputAttempt();

                if (targetComp->isCurrentlyBlockedByAnotherModalComponent())
                    return true;
            }

            (new AsyncFileDropMessage (targetComp, target, targetComp->getLocalPoint (component, position), files))->post();
            return true;
        }
    }

    return false;
}

//==============================================================================
void ComponentPeer::handleUserClosingWindow()
{
    updateCurrentModifiers();
    component->userTriedToCloseWindow();
}

//==============================================================================
void ComponentPeer::clearMaskedRegion()
{
    maskedRegion.clear();
}

void ComponentPeer::addMaskedRegion (const Rectangle<int>& area)
{
    maskedRegion.add (area);
}

//==============================================================================
StringArray ComponentPeer::getAvailableRenderingEngines()
{
    return StringArray ("Software Renderer");
}

int ComponentPeer::getCurrentRenderingEngine() const
{
    return 0;
}

void ComponentPeer::setCurrentRenderingEngine (int /*index*/)
{
}
