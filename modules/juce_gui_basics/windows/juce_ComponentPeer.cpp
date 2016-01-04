/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

static uint32 lastUniquePeerID = 1;

//==============================================================================
ComponentPeer::ComponentPeer (Component& comp, const int flags)
    : component (comp),
      styleFlags (flags),
      constrainer (nullptr),
      lastDragAndDropCompUnderMouse (nullptr),
      uniqueID (lastUniquePeerID += 2), // increment by 2 so that this can never hit 0
      isWindowMinimised (false)
{
    Desktop::getInstance().peers.add (this);
}

ComponentPeer::~ComponentPeer()
{
    Desktop& desktop = Desktop::getInstance();
    desktop.peers.removeFirstMatchingValue (this);
    desktop.triggerFocusCallback();
}

//==============================================================================
int ComponentPeer::getNumPeers() noexcept
{
    return Desktop::getInstance().peers.size();
}

ComponentPeer* ComponentPeer::getPeer (const int index) noexcept
{
    return Desktop::getInstance().peers [index];
}

ComponentPeer* ComponentPeer::getPeerFor (const Component* const component) noexcept
{
    const Array<ComponentPeer*>& peers = Desktop::getInstance().peers;

    for (int i = peers.size(); --i >= 0;)
    {
        ComponentPeer* const peer = peers.getUnchecked(i);

        if (&(peer->getComponent()) == component)
            return peer;
    }

    return nullptr;
}

bool ComponentPeer::isValidPeer (const ComponentPeer* const peer) noexcept
{
    return Desktop::getInstance().peers.contains (const_cast<ComponentPeer*> (peer));
}

void ComponentPeer::updateBounds()
{
    setBounds (ScalingHelpers::scaledScreenPosToUnscaled (component, component.getBoundsInParent()), false);
}

bool ComponentPeer::isKioskMode() const
{
    return Desktop::getInstance().getKioskModeComponent() == &component;
}

//==============================================================================
void ComponentPeer::handleMouseEvent (int touchIndex, Point<float> pos, ModifierKeys newMods, float newPressure, int64 time)
{
    if (MouseInputSource* mouse = Desktop::getInstance().mouseSources->getOrCreateMouseInputSource (touchIndex))
        MouseInputSource (*mouse).handleEvent (*this, pos, time, newMods, newPressure);
}

void ComponentPeer::handleMouseWheel (int touchIndex, Point<float> pos, int64 time, const MouseWheelDetails& wheel)
{
    if (MouseInputSource* mouse = Desktop::getInstance().mouseSources->getOrCreateMouseInputSource (touchIndex))
        MouseInputSource (*mouse).handleWheel (*this, pos, time, wheel);
}

void ComponentPeer::handleMagnifyGesture (int touchIndex, Point<float> pos, int64 time, float scaleFactor)
{
    if (MouseInputSource* mouse = Desktop::getInstance().mouseSources->getOrCreateMouseInputSource (touchIndex))
        MouseInputSource (*mouse).handleMagnifyGesture (*this, pos, time, scaleFactor);
}

//==============================================================================
void ComponentPeer::handlePaint (LowLevelGraphicsContext& contextToPaintTo)
{
    ModifierKeys::updateCurrentModifiers();

    Graphics g (contextToPaintTo);

    if (component.isTransformed())
        g.addTransform (component.getTransform());

    const Rectangle<int> peerBounds (getBounds());

    if (peerBounds.getWidth() != component.getWidth() || peerBounds.getHeight() != component.getHeight())
        // Tweak the scaling so that the component's integer size exactly aligns with the peer's scaled size
        g.addTransform (AffineTransform::scale (peerBounds.getWidth()  / (float) component.getWidth(),
                                                peerBounds.getHeight() / (float) component.getHeight()));

  #if JUCE_ENABLE_REPAINT_DEBUGGING
   #ifdef JUCE_IS_REPAINT_DEBUGGING_ACTIVE
    if (JUCE_IS_REPAINT_DEBUGGING_ACTIVE)
   #endif
    {
        g.saveState();
    }
  #endif

    JUCE_TRY
    {
        component.paintEntireComponent (g, true);
    }
    JUCE_CATCH_EXCEPTION

  #if JUCE_ENABLE_REPAINT_DEBUGGING
   #ifdef JUCE_IS_REPAINT_DEBUGGING_ACTIVE
    if (JUCE_IS_REPAINT_DEBUGGING_ACTIVE)
   #endif
    {
        // enabling this code will fill all areas that get repainted with a colour overlay, to show
        // clearly when things are being repainted.
        g.restoreState();

        static Random rng;

        g.fillAll (Colour ((uint8) rng.nextInt (255),
                           (uint8) rng.nextInt (255),
                           (uint8) rng.nextInt (255),
                           (uint8) 0x50));
    }
  #endif

    /** If this fails, it's probably be because your CPU floating-point precision mode has
        been set to low.. This setting is sometimes changed by things like Direct3D, and can
        mess up a lot of the calculations that the library needs to do.
    */
    jassert (roundToInt (10.1f) == 10);
}

Component* ComponentPeer::getTargetForKeyPress()
{
    Component* c = Component::getCurrentlyFocusedComponent();

    if (c == nullptr)
        c = &component;

    if (c->isCurrentlyBlockedByAnotherModalComponent())
        if (Component* const currentModalComp = Component::getCurrentlyModalComponent())
            c = currentModalComp;

    return c;
}

bool ComponentPeer::handleKeyPress (const int keyCode, const juce_wchar textCharacter)
{
    ModifierKeys::updateCurrentModifiers();

    return handleKeyPress (KeyPress (keyCode,
                                     ModifierKeys::getCurrentModifiers().withoutMouseButtons(),
                                     textCharacter));
}


bool ComponentPeer::handleKeyPress (const KeyPress& keyInfo)
{
    bool keyWasUsed = false;

    for (Component* target = getTargetForKeyPress(); target != nullptr; target = target->getParentComponent())
    {
        const WeakReference<Component> deletionChecker (target);

        if (const Array<KeyListener*>* const keyListeners = target->keyListeners)
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

        if (Component* const currentlyFocused = Component::getCurrentlyFocusedComponent())
        {
            const bool isTab      = (keyInfo == KeyPress::tabKey);
            const bool isShiftTab = (keyInfo == KeyPress (KeyPress::tabKey, ModifierKeys::shiftModifier, 0));

            if (isTab || isShiftTab)
            {
                currentlyFocused->moveKeyboardFocusToSibling (isTab);
                keyWasUsed = (currentlyFocused != Component::getCurrentlyFocusedComponent());

                if (keyWasUsed || deletionChecker == nullptr)
                    break;
            }
        }
    }

    return keyWasUsed;
}

bool ComponentPeer::handleKeyUpOrDown (const bool isKeyDown)
{
    ModifierKeys::updateCurrentModifiers();
    bool keyWasUsed = false;

    for (Component* target = getTargetForKeyPress(); target != nullptr; target = target->getParentComponent())
    {
        const WeakReference<Component> deletionChecker (target);

        keyWasUsed = target->keyStateChanged (isKeyDown);

        if (keyWasUsed || deletionChecker == nullptr)
            break;

        if (const Array<KeyListener*>* const keyListeners = target->keyListeners)
        {
            for (int i = keyListeners->size(); --i >= 0;)
            {
                keyWasUsed = keyListeners->getUnchecked(i)->keyStateChanged (isKeyDown, target);

                if (keyWasUsed || deletionChecker == nullptr)
                    return keyWasUsed;

                i = jmin (i, keyListeners->size());
            }
        }
    }

    return keyWasUsed;
}

void ComponentPeer::handleModifierKeysChange()
{
    ModifierKeys::updateCurrentModifiers();

    Component* target = Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();

    if (target == nullptr)
        target = Component::getCurrentlyFocusedComponent();

    if (target == nullptr)
        target = &component;

    if (target != nullptr)
        target->internalModifierKeysChanged();
}

TextInputTarget* ComponentPeer::findCurrentTextInputTarget()
{
    Component* const c = Component::getCurrentlyFocusedComponent();

    if (c == &component || component.isParentOf (c))
        if (TextInputTarget* const ti = dynamic_cast<TextInputTarget*> (c))
            if (ti->isTextInputActive())
                return ti;

    return nullptr;
}

void ComponentPeer::dismissPendingTextInput() {}

//==============================================================================
void ComponentPeer::handleBroughtToFront()
{
    ModifierKeys::updateCurrentModifiers();
    component.internalBroughtToFront();
}

void ComponentPeer::setConstrainer (ComponentBoundsConstrainer* const newConstrainer) noexcept
{
    constrainer = newConstrainer;
}

void ComponentPeer::handleMovedOrResized()
{
    ModifierKeys::updateCurrentModifiers();

    const bool nowMinimised = isMinimised();

    if (component.flags.hasHeavyweightPeerFlag && ! nowMinimised)
    {
        const WeakReference<Component> deletionChecker (&component);

        Rectangle<int> newBounds (Component::ComponentHelpers::rawPeerPositionToLocal (component, getBounds()));
        Rectangle<int> oldBounds (component.getBounds());

        const bool wasMoved   = (oldBounds.getPosition() != newBounds.getPosition());
        const bool wasResized = (oldBounds.getWidth() != newBounds.getWidth() || oldBounds.getHeight() != newBounds.getHeight());

        if (wasMoved || wasResized)
        {
            component.bounds = newBounds;

            if (wasResized)
                component.repaint();

            component.sendMovedResizedMessages (wasMoved, wasResized);

            if (deletionChecker == nullptr)
                return;
        }
    }

    if (isWindowMinimised != nowMinimised)
    {
        isWindowMinimised = nowMinimised;
        component.minimisationStateChanged (nowMinimised);
        component.sendVisibilityChangeMessage();
    }

    if (! isFullScreen())
        lastNonFullscreenBounds = component.getBounds();
}

void ComponentPeer::handleFocusGain()
{
    ModifierKeys::updateCurrentModifiers();

    if (component.isParentOf (lastFocusedComponent))
    {
        Component::currentlyFocusedComponent = lastFocusedComponent;
        Desktop::getInstance().triggerFocusCallback();
        lastFocusedComponent->internalFocusGain (Component::focusChangedDirectly);
    }
    else
    {
        if (! component.isCurrentlyBlockedByAnotherModalComponent())
            component.grabKeyboardFocus();
        else
            ModalComponentManager::getInstance()->bringModalComponentsToFront();
    }
}

void ComponentPeer::handleFocusLoss()
{
    ModifierKeys::updateCurrentModifiers();

    if (component.hasKeyboardFocus (true))
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
    return (component.isParentOf (lastFocusedComponent) && lastFocusedComponent->isShowing())
                ? static_cast<Component*> (lastFocusedComponent)
                : &component;
}

void ComponentPeer::handleScreenSizeChange()
{
    ModifierKeys::updateCurrentModifiers();

    component.parentSizeChanged();
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

Point<int> ComponentPeer::localToGlobal (Point<int> p)   { return localToGlobal (p.toFloat()).roundToInt(); }
Point<int> ComponentPeer::globalToLocal (Point<int> p)   { return globalToLocal (p.toFloat()).roundToInt(); }

Rectangle<int> ComponentPeer::localToGlobal (const Rectangle<int>& relativePosition)
{
    return relativePosition.withPosition (localToGlobal (relativePosition.getPosition()));
}

Rectangle<int> ComponentPeer::globalToLocal (const Rectangle<int>& screenPosition)
{
    return screenPosition.withPosition (globalToLocal (screenPosition.getPosition()));
}

Rectangle<int> ComponentPeer::getAreaCoveredBy (Component& subComponent) const
{
    return ScalingHelpers::scaledScreenPosToUnscaled
            (component, component.getLocalArea (&subComponent, subComponent.getLocalBounds()));
}

//==============================================================================
namespace DragHelpers
{
    static bool isFileDrag (const ComponentPeer::DragInfo& info)
    {
        return info.files.size() > 0;
    }

    static bool isSuitableTarget (const ComponentPeer::DragInfo& info, Component* target)
    {
        return isFileDrag (info) ? dynamic_cast<FileDragAndDropTarget*> (target) != nullptr
                                 : dynamic_cast<TextDragAndDropTarget*> (target) != nullptr;
    }

    static bool isInterested (const ComponentPeer::DragInfo& info, Component* target)
    {
        return isFileDrag (info) ? dynamic_cast<FileDragAndDropTarget*> (target)->isInterestedInFileDrag (info.files)
                                 : dynamic_cast<TextDragAndDropTarget*> (target)->isInterestedInTextDrag (info.text);
    }

    static Component* findDragAndDropTarget (Component* c, const ComponentPeer::DragInfo& info, Component* const lastOne)
    {
        for (; c != nullptr; c = c->getParentComponent())
            if (isSuitableTarget (info, c) && (c == lastOne || isInterested (info, c)))
                return c;

        return nullptr;
    }

    // We'll use an async message to deliver the drop, because if the target decides
    // to run a modal loop, it can gum-up the operating system..
    class AsyncDropMessage  : public CallbackMessage
    {
    public:
        AsyncDropMessage (Component* c, const ComponentPeer::DragInfo& d)  : target (c), info (d) {}

        void messageCallback() override
        {
            if (Component* const c = target.get())
            {
                if (isFileDrag (info))
                    dynamic_cast<FileDragAndDropTarget*> (c)->filesDropped (info.files, info.position.x, info.position.y);
                else
                    dynamic_cast<TextDragAndDropTarget*> (c)->textDropped (info.text, info.position.x, info.position.y);
            }
        }

    private:
        WeakReference<Component> target;
        const ComponentPeer::DragInfo info;

        JUCE_DECLARE_NON_COPYABLE (AsyncDropMessage)
    };
}

bool ComponentPeer::handleDragMove (const ComponentPeer::DragInfo& info)
{
    ModifierKeys::updateCurrentModifiers();

    Component* const compUnderMouse = component.getComponentAt (info.position);

    Component* const lastTarget = dragAndDropTargetComponent;
    Component* newTarget = nullptr;

    if (compUnderMouse != lastDragAndDropCompUnderMouse)
    {
        lastDragAndDropCompUnderMouse = compUnderMouse;
        newTarget = DragHelpers::findDragAndDropTarget (compUnderMouse, info, lastTarget);

        if (newTarget != lastTarget)
        {
            if (lastTarget != nullptr)
            {
                if (DragHelpers::isFileDrag (info))
                    dynamic_cast<FileDragAndDropTarget*> (lastTarget)->fileDragExit (info.files);
                else
                    dynamic_cast<TextDragAndDropTarget*> (lastTarget)->textDragExit (info.text);
            }

            dragAndDropTargetComponent = nullptr;

            if (DragHelpers::isSuitableTarget (info, newTarget))
            {
                dragAndDropTargetComponent = newTarget;
                const Point<int> pos (newTarget->getLocalPoint (&component, info.position));

                if (DragHelpers::isFileDrag (info))
                    dynamic_cast<FileDragAndDropTarget*> (newTarget)->fileDragEnter (info.files, pos.x, pos.y);
                else
                    dynamic_cast<TextDragAndDropTarget*> (newTarget)->textDragEnter (info.text, pos.x, pos.y);
            }
        }
    }
    else
    {
        newTarget = lastTarget;
    }

    if (! DragHelpers::isSuitableTarget (info, newTarget))
        return false;

    const Point<int> pos (newTarget->getLocalPoint (&component, info.position));

    if (DragHelpers::isFileDrag (info))
        dynamic_cast<FileDragAndDropTarget*> (newTarget)->fileDragMove (info.files, pos.x, pos.y);
    else
        dynamic_cast<TextDragAndDropTarget*> (newTarget)->textDragMove (info.text, pos.x, pos.y);

    return true;
}

bool ComponentPeer::handleDragExit (const ComponentPeer::DragInfo& info)
{
    DragInfo info2 (info);
    info2.position.setXY (-1, -1);
    const bool used = handleDragMove (info2);

    jassert (dragAndDropTargetComponent == nullptr);
    lastDragAndDropCompUnderMouse = nullptr;
    return used;
}

bool ComponentPeer::handleDragDrop (const ComponentPeer::DragInfo& info)
{
    handleDragMove (info);

    if (Component* const targetComp = dragAndDropTargetComponent)
    {
        dragAndDropTargetComponent = nullptr;
        lastDragAndDropCompUnderMouse = nullptr;

        if (DragHelpers::isSuitableTarget (info, targetComp))
        {
            if (targetComp->isCurrentlyBlockedByAnotherModalComponent())
            {
                targetComp->internalModalInputAttempt();

                if (targetComp->isCurrentlyBlockedByAnotherModalComponent())
                    return true;
            }

            ComponentPeer::DragInfo info2 (info);
            info2.position = targetComp->getLocalPoint (&component, info.position);

            (new DragHelpers::AsyncDropMessage (targetComp, info2))->post();
            return true;
        }
    }

    return false;
}

//==============================================================================
void ComponentPeer::handleUserClosingWindow()
{
    ModifierKeys::updateCurrentModifiers();
    component.userTriedToCloseWindow();
}

bool ComponentPeer::setDocumentEditedStatus (bool)
{
    return false;
}

void ComponentPeer::setRepresentedFile (const File&)
{
}

//==============================================================================
int ComponentPeer::getCurrentRenderingEngine() const            { return 0; }
void ComponentPeer::setCurrentRenderingEngine (int index)       { jassert (index == 0); ignoreUnused (index); }
