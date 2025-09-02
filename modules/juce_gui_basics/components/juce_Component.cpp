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

#define JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN \
    jassert ((MessageManager::getInstanceWithoutCreating() != nullptr \
               && MessageManager::getInstanceWithoutCreating()->currentThreadHasLockedMessageManager()) \
              || getPeer() == nullptr);

namespace juce
{

static Component* findFirstEnabledAncestor (Component* in)
{
    if (in == nullptr)
        return nullptr;

    if (in->isEnabled())
        return in;

    return findFirstEnabledAncestor (in->getParentComponent());
}

Component* Component::currentlyFocusedComponent = nullptr;

//==============================================================================
class HierarchyChecker
{
public:
    /*  Creates a bail-out checker for comp and its ancestors, that will return true from
        shouldBailOut() if all of comp's ancestors are destroyed.
        @param comp     a safe pointer to a component. The pointer will be updated to point
                        to the nearest non-null ancestor on each call to shouldBailOut.
    */
    HierarchyChecker (Component::SafePointer<Component>* comp, const MouseEvent& originalEvent)
        : closestAncestor (*comp),
          me (originalEvent)
    {
        for (Component* c = *comp; c != nullptr; c = c->getParentComponent())
            hierarchy.emplace_back (c);
    }

    Component* nearestNonNullParent() const
    {
        return closestAncestor;
    }

    /*  Searches for the closest ancestor, and returns true if the closest ancestor is nullptr. */
    bool shouldBailOut() const
    {
        closestAncestor = findNearestNonNullParent();
        return closestAncestor == nullptr;
    }

    MouseEvent eventWithNearestParent() const
    {
        return { me.source,
                 me.position.toFloat(),
                 me.mods,
                 me.pressure, me.orientation, me.rotation,
                 me.tiltX, me.tiltY,
                 closestAncestor,
                 closestAncestor,
                 me.eventTime,
                 me.mouseDownPosition.toFloat(),
                 me.mouseDownTime,
                 me.getNumberOfClicks(),
                 me.mouseWasDraggedSinceMouseDown() };
    }

    template <typename Callback>
    void forEach (Callback&& callback)
    {
        for (auto& item : hierarchy)
            if (item != nullptr)
                callback (*item);
    }

private:
    Component* findNearestNonNullParent() const
    {
        for (auto& comp : hierarchy)
            if (comp != nullptr)
                return comp;

        return nullptr;
    }

    Component::SafePointer<Component>& closestAncestor;
    std::vector<Component::SafePointer<Component>> hierarchy;
    const MouseEvent me;
};

//==============================================================================
class Component::MouseListenerList
{
public:
    MouseListenerList() noexcept {}

    void addListener (MouseListener* newListener, bool wantsEventsForAllNestedChildComponents)
    {
        if (! listeners.contains (newListener))
        {
            if (wantsEventsForAllNestedChildComponents)
            {
                listeners.insert (0, newListener);
                ++numDeepMouseListeners;
            }
            else
            {
                listeners.add (newListener);
            }
        }
    }

    void removeListener (MouseListener* listenerToRemove)
    {
        auto index = listeners.indexOf (listenerToRemove);

        if (index >= 0)
        {
            if (index < numDeepMouseListeners)
                --numDeepMouseListeners;

            listeners.remove (index);
        }
    }

    template <typename EventMethod, typename... Params>
    static void sendMouseEvent (HierarchyChecker& checker, EventMethod&& eventMethod, Params&&... params)
    {
        const auto callListeners = [&] (auto& parentComp, const auto findNumListeners)
        {
            if (auto* list = parentComp.mouseListeners.get())
            {
                const WeakReference safePointer { &parentComp };

                for (int i = findNumListeners (*list); --i >= 0; i = jmin (i, findNumListeners (*list)))
                {
                    (list->listeners.getUnchecked (i)->*eventMethod) (checker.eventWithNearestParent(), params...);

                    if (checker.shouldBailOut() || safePointer == nullptr)
                        return false;
                }
            }

            return true;
        };

        if (auto* parent = checker.nearestNonNullParent())
            if (! callListeners (*parent, [] (auto& list) { return list.listeners.size(); }))
                return;

        if (auto* parent = checker.nearestNonNullParent())
            for (Component* p = parent->parentComponent; p != nullptr; p = p->parentComponent)
                if (! callListeners (*p, [] (auto& list) { return list.numDeepMouseListeners; }))
                    return;
    }

private:
    Array<MouseListener*> listeners;
    int numDeepMouseListeners = 0;

    JUCE_DECLARE_NON_COPYABLE (MouseListenerList)
};

class Component::EffectState
{
public:
    explicit EffectState (ImageEffectFilter& i) : effect (&i) {}

    ImageEffectFilter& getEffect() const
    {
        return *effect;
    }

    bool setEffect (ImageEffectFilter& i)
    {
        return std::exchange (effect, &i) != &i;
    }

    void paint (Graphics& g, Component& c, bool ignoreAlphaLevel)
    {
        auto scale = g.getInternalContext().getPhysicalPixelScaleFactor();
        auto scaledBounds = c.getLocalBounds() * scale;

        const auto preferredType = g.getInternalContext().getPreferredImageTypeForTemporaryImages();
        const auto pixelData = effectImage.getPixelData();
        const auto shouldCreateImage = pixelData == nullptr
                                       || pixelData->width != scaledBounds.getWidth()
                                       || pixelData->height != scaledBounds.getHeight()
                                       || pixelData->createType()->getTypeID() != preferredType->getTypeID();

        if (shouldCreateImage)
        {
            effectImage = Image { c.isOpaque() ? Image::RGB : Image::ARGB,
                                  scaledBounds.getWidth(),
                                  scaledBounds.getHeight(),
                                  false,
                                  *preferredType };
            effectImage.setBackupEnabled (false);
        }

        if (! c.isOpaque())
            effectImage.clear (effectImage.getBounds());

        {
            Graphics g2 (effectImage);
            g2.addTransform (AffineTransform::scale ((float) scaledBounds.getWidth()  / (float) c.getWidth(),
                                                     (float) scaledBounds.getHeight() / (float) c.getHeight()));
            c.paintComponentAndChildren (g2);
        }

        Graphics::ScopedSaveState ss (g);

        g.addTransform (AffineTransform::scale (1.0f / scale));
        effect->applyEffect (effectImage, g, scale, ignoreAlphaLevel ? 1.0f : c.getAlpha());
    }

    void releaseResources()
    {
        effectImage = {};
    }

private:
    Image effectImage;
    ImageEffectFilter* effect;
};

//==============================================================================
Component::Component() noexcept
  : componentFlags (0)
{
}

Component::Component (const String& name) noexcept
  : componentName (name), componentFlags (0)
{
}

Component::~Component()
{
    static_assert (sizeof (flags) <= sizeof (componentFlags), "componentFlags has too many bits!");

    componentListeners.call ([this] (ComponentListener& l) { l.componentBeingDeleted (*this); });

    while (childComponentList.size() > 0)
        removeChildComponent (childComponentList.size() - 1, false, true);

    masterReference.clear();

    if (parentComponent != nullptr)
        parentComponent->removeChildComponent (parentComponent->childComponentList.indexOf (this), true, false);
    else
        giveAwayKeyboardFocusInternal (isParentOf (currentlyFocusedComponent));

    if (flags.hasHeavyweightPeerFlag)
        removeFromDesktop();

    // Something has added some children to this component during its destructor! Not a smart idea!
    jassert (childComponentList.size() == 0);
}

//==============================================================================
void Component::setName (const String& name)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    if (componentName != name)
    {
        componentName = name;

        if (flags.hasHeavyweightPeerFlag)
            if (auto* peer = getPeer())
                peer->setTitle (name);

        BailOutChecker checker (this);
        componentListeners.callChecked (checker, [this] (ComponentListener& l) { l.componentNameChanged (*this); });
    }
}

void Component::setComponentID (const String& newID)
{
    componentID = newID;
}

void Component::setVisible (bool shouldBeVisible)
{
    if (flags.visibleFlag != shouldBeVisible)
    {
        // if component methods are being called from threads other than the message
        // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

        const WeakReference<Component> safePointer (this);
        flags.visibleFlag = shouldBeVisible;

        if (shouldBeVisible)
            repaint();
        else
            repaintParent();

        sendFakeMouseMove();

        if (! shouldBeVisible)
        {
            detail::ComponentHelpers::releaseAllCachedImageResources (*this);

            if (hasKeyboardFocus (true))
            {
                if (parentComponent != nullptr)
                    parentComponent->grabKeyboardFocus();

                // ensure that keyboard focus is given away if it wasn't taken by parent
                giveAwayKeyboardFocus();
            }
        }

        if (safePointer != nullptr)
        {
            sendVisibilityChangeMessage();

            if (safePointer != nullptr && flags.hasHeavyweightPeerFlag)
            {
                if (auto* peer = getPeer())
                {
                    peer->setVisible (shouldBeVisible);
                    internalHierarchyChanged();
                }
            }
        }
    }
}

void Component::visibilityChanged() {}

void Component::sendVisibilityChangeMessage()
{
    BailOutChecker checker (this);
    visibilityChanged();

    if (! checker.shouldBailOut())
        componentListeners.callChecked (checker, [this] (ComponentListener& l) { l.componentVisibilityChanged (*this); });
}

bool Component::isShowing() const
{
    if (! flags.visibleFlag)
        return false;

    if (parentComponent != nullptr)
        return parentComponent->isShowing();

    if (auto* peer = getPeer())
        return ! peer->isMinimised();

    return false;
}

//==============================================================================
void* Component::getWindowHandle() const
{
    if (auto* peer = getPeer())
        return peer->getNativeHandle();

    return nullptr;
}

//==============================================================================
void Component::addToDesktop (int styleWanted, void* nativeWindowToAttachTo)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (isOpaque())
        styleWanted &= ~ComponentPeer::windowIsSemiTransparent;
    else
        styleWanted |= ComponentPeer::windowIsSemiTransparent;

    // don't use getPeer(), so that we only get the peer that's specifically
    // for this comp, and not for one of its parents.
    auto* peer = ComponentPeer::getPeerFor (this);

    if (peer == nullptr || styleWanted != peer->getStyleFlags())
    {
        const WeakReference<Component> safePointer (this);

       #if JUCE_LINUX || JUCE_BSD
        // it's wise to give the component a non-zero size before
        // putting it on the desktop, as X windows get confused by this, and
        // a (1, 1) minimum size is enforced here.
        setSize (jmax (1, getWidth()),
                 jmax (1, getHeight()));
       #endif

        const auto unscaledPosition = detail::ScalingHelpers::scaledScreenPosToUnscaled (getScreenPosition());
        const auto topLeft = detail::ScalingHelpers::unscaledScreenPosToScaled (*this, unscaledPosition);

        bool wasFullscreen = false;
        bool wasMinimised = false;
        ComponentBoundsConstrainer* currentConstrainer = nullptr;
        Rectangle<int> oldNonFullScreenBounds;
        int oldRenderingEngine = -1;

        if (peer != nullptr)
        {
            std::unique_ptr<ComponentPeer> oldPeerToDelete (peer);

            wasFullscreen = peer->isFullScreen();
            wasMinimised = peer->isMinimised();
            currentConstrainer = peer->getConstrainer();
            oldNonFullScreenBounds = peer->getNonFullScreenBounds();
            oldRenderingEngine = peer->getCurrentRenderingEngine();

            flags.hasHeavyweightPeerFlag = false;
            Desktop::getInstance().removeDesktopComponent (this);
            internalHierarchyChanged(); // give comps a chance to react to the peer change before the old peer is deleted.

            if (safePointer == nullptr)
                return;

            setTopLeftPosition (topLeft);
        }

        if (parentComponent != nullptr)
            parentComponent->removeChildComponent (this);

        if (safePointer != nullptr)
        {
            flags.hasHeavyweightPeerFlag = true;

            peer = createNewPeer (styleWanted, nativeWindowToAttachTo);

            Desktop::getInstance().addDesktopComponent (this);

            boundsRelativeToParent.setPosition (topLeft);
            peer->updateBounds();

            if (oldRenderingEngine >= 0)
                peer->setCurrentRenderingEngine (oldRenderingEngine);

            peer->setVisible (isVisible());

            peer = ComponentPeer::getPeerFor (this);

            if (peer == nullptr)
                return;

            if (wasFullscreen)
            {
                peer->setFullScreen (true);
                peer->setNonFullScreenBounds (oldNonFullScreenBounds);
            }

            if (wasMinimised)
                peer->setMinimised (true);

           #if JUCE_WINDOWS
            if (isAlwaysOnTop())
                peer->setAlwaysOnTop (true);
           #endif

            peer->setConstrainer (currentConstrainer);

            repaint();

           #if JUCE_LINUX
            // Creating the peer Image on Linux will change the reported position of the window. If
            // the Image creation is interleaved with the coming configureNotifyEvents the window
            // will appear in the wrong position. To avoid this, we force the Image creation here,
            // before handling any of the configureNotifyEvents. The Linux implementation of
            // performAnyPendingRepaintsNow() will force update the peer position if necessary.
            peer->performAnyPendingRepaintsNow();
           #endif

            internalHierarchyChanged();

            if (auto* handler = getAccessibilityHandler())
                detail::AccessibilityHelpers::notifyAccessibilityEvent (*handler, detail::AccessibilityHelpers::Event::windowOpened);
        }
    }
}

void Component::removeFromDesktop()
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    if (flags.hasHeavyweightPeerFlag)
    {
        if (auto* handler = getAccessibilityHandler())
            detail::AccessibilityHelpers::notifyAccessibilityEvent (*handler, detail::AccessibilityHelpers::Event::windowClosed);

        detail::ComponentHelpers::releaseAllCachedImageResources (*this);

        auto* peer = ComponentPeer::getPeerFor (this);
        jassert (peer != nullptr);

        flags.hasHeavyweightPeerFlag = false;
        delete peer;

        Desktop::getInstance().removeDesktopComponent (this);
    }
}

bool Component::isOnDesktop() const noexcept
{
    return flags.hasHeavyweightPeerFlag;
}

ComponentPeer* Component::getPeer() const
{
    if (flags.hasHeavyweightPeerFlag)
        return ComponentPeer::getPeerFor (this);

    if (parentComponent == nullptr)
        return nullptr;

    return parentComponent->getPeer();
}

void Component::userTriedToCloseWindow()
{
    /* This means that the user's trying to get rid of your window with the 'close window' system
       menu option (on windows) or possibly the task manager - you should really handle this
       and delete or hide your component in an appropriate way.

       If you want to ignore the event and don't want to trigger this assertion, just override
       this method and do nothing.
    */
    jassertfalse;
}

void Component::minimisationStateChanged (bool) {}

float Component::getDesktopScaleFactor() const  { return Desktop::getInstance().getGlobalScaleFactor(); }

//==============================================================================
void Component::setOpaque (bool shouldBeOpaque)
{
    if (shouldBeOpaque != flags.opaqueFlag)
    {
        flags.opaqueFlag = shouldBeOpaque;

        if (flags.hasHeavyweightPeerFlag)
            if (auto* peer = ComponentPeer::getPeerFor (this))
                addToDesktop (peer->getStyleFlags());  // recreates the heavyweight window

        repaint();
    }
}

bool Component::isOpaque() const noexcept
{
    return flags.opaqueFlag;
}

//==============================================================================
void Component::setCachedComponentImage (CachedComponentImage* newCachedImage)
{
    if (cachedImage.get() != newCachedImage)
    {
        cachedImage.reset (newCachedImage);
        repaint();
    }
}

void Component::setBufferedToImage (bool shouldBeBuffered)
{
    // This assertion means that this component is already using a custom CachedComponentImage,
    // so by calling setBufferedToImage, you'll be deleting the custom one - this is almost certainly
    // not what you wanted to happen... If you really do know what you're doing here, and want to
    // avoid this assertion, just call setCachedComponentImage (nullptr) before setBufferedToImage().
    jassert (cachedImage == nullptr || dynamic_cast<detail::StandardCachedComponentImage*> (cachedImage.get()) != nullptr);

    if (shouldBeBuffered)
    {
        if (cachedImage == nullptr)
            cachedImage = std::make_unique<detail::StandardCachedComponentImage> (*this);
    }
    else
    {
        cachedImage.reset();
    }
}

void Component::invalidateCachedImageResources()
{
    if (cachedImage != nullptr)
        cachedImage->releaseResources();

    if (effectState != nullptr)
        effectState->releaseResources();
}

//==============================================================================
void Component::reorderChildInternal (int sourceIndex, int destIndex)
{
    if (sourceIndex != destIndex)
    {
        auto* c = childComponentList.getUnchecked (sourceIndex);
        jassert (c != nullptr);
        c->repaintParent();

        childComponentList.move (sourceIndex, destIndex);

        sendFakeMouseMove();
        internalChildrenChanged();
    }
}

void Component::toFront (bool shouldGrabKeyboardFocus)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    if (flags.hasHeavyweightPeerFlag)
    {
        if (auto* peer = getPeer())
        {
            peer->toFront (shouldGrabKeyboardFocus);

            if (shouldGrabKeyboardFocus && ! hasKeyboardFocus (true))
                grabKeyboardFocus();
        }
    }
    else if (parentComponent != nullptr)
    {
        auto& childList = parentComponent->childComponentList;

        if (childList.getLast() != this)
        {
            auto index = childList.indexOf (this);

            if (index >= 0)
            {
                int insertIndex = -1;

                if (! flags.alwaysOnTopFlag)
                {
                    insertIndex = childList.size() - 1;

                    while (insertIndex > 0 && childList.getUnchecked (insertIndex)->isAlwaysOnTop())
                        --insertIndex;
                }

                parentComponent->reorderChildInternal (index, insertIndex);
            }
        }

        if (shouldGrabKeyboardFocus)
        {
            internalBroughtToFront();

            if (isShowing())
                grabKeyboardFocus();
        }
    }
}

void Component::toBehind (Component* other)
{
    if (other != nullptr && other != this)
    {
        // the two components must belong to the same parent..
        jassert (parentComponent == other->parentComponent);

        if (parentComponent != nullptr)
        {
            auto& childList = parentComponent->childComponentList;
            auto index = childList.indexOf (this);

            if (index >= 0 && childList [index + 1] != other)
            {
                auto otherIndex = childList.indexOf (other);

                if (otherIndex >= 0)
                {
                    if (index < otherIndex)
                        --otherIndex;

                    parentComponent->reorderChildInternal (index, otherIndex);
                }
            }
        }
        else if (isOnDesktop())
        {
            jassert (other->isOnDesktop());

            if (other->isOnDesktop())
            {
                auto* us = getPeer();
                auto* them = other->getPeer();
                jassert (us != nullptr && them != nullptr);

                if (us != nullptr && them != nullptr)
                    us->toBehind (them);
            }
        }
    }
}

void Component::toBack()
{
    if (isOnDesktop())
    {
        jassertfalse; //xxx need to add this to native window
    }
    else if (parentComponent != nullptr)
    {
        auto& childList = parentComponent->childComponentList;

        if (childList.getFirst() != this)
        {
            auto index = childList.indexOf (this);

            if (index > 0)
            {
                int insertIndex = 0;

                if (flags.alwaysOnTopFlag)
                    while (insertIndex < childList.size() && ! childList.getUnchecked (insertIndex)->isAlwaysOnTop())
                        ++insertIndex;

                parentComponent->reorderChildInternal (index, insertIndex);
            }
        }
    }
}

void Component::setAlwaysOnTop (bool shouldStayOnTop)
{
    if (shouldStayOnTop != flags.alwaysOnTopFlag)
    {
        BailOutChecker checker (this);

        flags.alwaysOnTopFlag = shouldStayOnTop;

        if (isOnDesktop())
        {
            if (auto* peer = getPeer())
            {
                if (! peer->setAlwaysOnTop (shouldStayOnTop))
                {
                    // some kinds of peer can't change their always-on-top status, so
                    // for these, we'll need to create a new window
                    auto oldFlags = peer->getStyleFlags();
                    removeFromDesktop();
                    addToDesktop (oldFlags);
                }
            }
        }

        if (shouldStayOnTop && ! checker.shouldBailOut())
            toFront (false);

        if (! checker.shouldBailOut())
            internalHierarchyChanged();
    }
}

bool Component::isAlwaysOnTop() const noexcept
{
    return flags.alwaysOnTopFlag;
}

//==============================================================================
int Component::proportionOfWidth  (float proportion) const noexcept   { return roundToInt (proportion * (float) boundsRelativeToParent.getWidth()); }
int Component::proportionOfHeight (float proportion) const noexcept   { return roundToInt (proportion * (float) boundsRelativeToParent.getHeight()); }

int Component::getParentWidth() const noexcept
{
    return parentComponent != nullptr ? parentComponent->getWidth()
                                      : getParentMonitorArea().getWidth();
}

int Component::getParentHeight() const noexcept
{
    return parentComponent != nullptr ? parentComponent->getHeight()
                                      : getParentMonitorArea().getHeight();
}

Rectangle<int> Component::getParentMonitorArea() const
{
    return Desktop::getInstance().getDisplays().getDisplayForRect (getScreenBounds())->userArea;
}

int Component::getScreenX() const                       { return getScreenPosition().x; }
int Component::getScreenY() const                       { return getScreenPosition().y; }
Point<int>     Component::getScreenPosition() const     { return localPointToGlobal (Point<int>()); }
Rectangle<int> Component::getScreenBounds() const       { return localAreaToGlobal (getLocalBounds()); }

Point<int>       Component::getLocalPoint (const Component* source, Point<int> point) const       { return detail::ComponentHelpers::convertCoordinate (this, source, point); }
Point<float>     Component::getLocalPoint (const Component* source, Point<float> point) const     { return detail::ComponentHelpers::convertCoordinate (this, source, point); }
Rectangle<int>   Component::getLocalArea  (const Component* source, Rectangle<int> area) const    { return detail::ComponentHelpers::convertCoordinate (this, source, area); }
Rectangle<float> Component::getLocalArea  (const Component* source, Rectangle<float> area) const  { return detail::ComponentHelpers::convertCoordinate (this, source, area); }

Point<int>       Component::localPointToGlobal (Point<int> point) const       { return detail::ComponentHelpers::convertCoordinate (nullptr, this, point); }
Point<float>     Component::localPointToGlobal (Point<float> point) const     { return detail::ComponentHelpers::convertCoordinate (nullptr, this, point); }
Rectangle<int>   Component::localAreaToGlobal  (Rectangle<int> area) const    { return detail::ComponentHelpers::convertCoordinate (nullptr, this, area); }
Rectangle<float> Component::localAreaToGlobal  (Rectangle<float> area) const  { return detail::ComponentHelpers::convertCoordinate (nullptr, this, area); }

//==============================================================================
void Component::setBounds (int x, int y, int w, int h)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    if (w < 0) w = 0;
    if (h < 0) h = 0;

    const bool wasResized  = (getWidth() != w || getHeight() != h);
    const bool wasMoved    = (getX() != x || getY() != y);

   #if JUCE_DEBUG
    // It's a very bad idea to try to resize a window during its paint() method!
    jassert (! (flags.isInsidePaintCall && wasResized && isOnDesktop()));
   #endif

    if (wasMoved || wasResized)
    {
        const bool showing = isShowing();

        if (showing)
        {
            // send a fake mouse move to trigger enter/exit messages if needed..
            sendFakeMouseMove();

            if (! flags.hasHeavyweightPeerFlag)
                repaintParent();
        }

        boundsRelativeToParent.setBounds (x, y, w, h);

        if (showing)
        {
            if (wasResized)
                repaint();
            else if (! flags.hasHeavyweightPeerFlag)
                repaintParent();
        }
        else if (cachedImage != nullptr)
        {
            cachedImage->invalidateAll();
        }

        flags.isMoveCallbackPending = wasMoved;
        flags.isResizeCallbackPending = wasResized;

        if (flags.hasHeavyweightPeerFlag)
            if (auto* peer = getPeer())
                peer->updateBounds();

        sendMovedResizedMessagesIfPending();
    }
}

void Component::sendMovedResizedMessagesIfPending()
{
    const bool wasMoved   = flags.isMoveCallbackPending;
    const bool wasResized = flags.isResizeCallbackPending;

    if (wasMoved || wasResized)
    {
        flags.isMoveCallbackPending = false;
        flags.isResizeCallbackPending = false;

        sendMovedResizedMessages (wasMoved, wasResized);
    }
}

void Component::sendMovedResizedMessages (bool wasMoved, bool wasResized)
{
    BailOutChecker checker (this);

    if (wasMoved)
    {
        moved();

        if (checker.shouldBailOut())
            return;
    }

    if (wasResized)
    {
        resized();

        if (checker.shouldBailOut())
            return;

        for (int i = childComponentList.size(); --i >= 0;)
        {
            childComponentList.getUnchecked (i)->parentSizeChanged();

            if (checker.shouldBailOut())
                return;

            i = jmin (i, childComponentList.size());
        }
    }

    if (parentComponent != nullptr)
        parentComponent->childBoundsChanged (this);

    if (! checker.shouldBailOut())
    {
        componentListeners.callChecked (checker, [this, wasMoved, wasResized] (ComponentListener& l)
        {
            l.componentMovedOrResized (*this, wasMoved, wasResized);
        });
    }

    if ((wasMoved || wasResized) && ! checker.shouldBailOut())
        if (auto* handler = getAccessibilityHandler())
            detail::AccessibilityHelpers::notifyAccessibilityEvent (*handler, detail::AccessibilityHelpers::Event::elementMovedOrResized);
}

void Component::setSize (int w, int h)                  { setBounds (getX(), getY(), w, h); }

void Component::setTopLeftPosition (int x, int y)       { setTopLeftPosition ({ x, y }); }
void Component::setTopLeftPosition (Point<int> pos)     { setBounds (pos.x, pos.y, getWidth(), getHeight()); }

void Component::setTopRightPosition (int x, int y)      { setTopRightPosition ({ x, y }); }
void Component::setTopRightPosition (Point<int> pos)    { setTopLeftPosition (pos.x - getWidth(), pos.y); }
void Component::setBounds (Rectangle<int> r)            { setBounds (r.getX(), r.getY(), r.getWidth(), r.getHeight()); }

void Component::setCentrePosition (Point<int> p)        { setBounds (getBounds().withCentre (p.transformedBy (getTransform().inverted()))); }
void Component::setCentrePosition (int x, int y)        { setCentrePosition ({ x, y }); }

void Component::setCentreRelative (float x, float y)
{
    setCentrePosition (roundToInt ((float) getParentWidth()  * x),
                       roundToInt ((float) getParentHeight() * y));
}

void Component::setBoundsRelative (Rectangle<float> target)
{
    setBounds ((target * Point<float> ((float) getParentWidth(),
                                       (float) getParentHeight())).toNearestInt());
}

void Component::setBoundsRelative (float x, float y, float w, float h)
{
    setBoundsRelative ({ x, y, w, h });
}

void Component::centreWithSize (int width, int height)
{
    auto parentArea = detail::ComponentHelpers::getParentOrMainMonitorBounds (*this)
                          .transformedBy (getTransform().inverted());

    setBounds (parentArea.getCentreX() - width / 2,
               parentArea.getCentreY() - height / 2,
               width, height);
}

void Component::setBoundsInset (BorderSize<int> borders)
{
    setBounds (borders.subtractedFrom (detail::ComponentHelpers::getParentOrMainMonitorBounds (*this)));
}

void Component::setBoundsToFit (Rectangle<int> targetArea, Justification justification, bool onlyReduceInSize)
{
    if (getLocalBounds().isEmpty() || targetArea.isEmpty())
    {
        // it's no good calling this method unless both the component and
        // target rectangle have a finite size.
        jassertfalse;
        return;
    }

    auto sourceArea = targetArea.withZeroOrigin();

    if (onlyReduceInSize
         && getWidth() <= targetArea.getWidth()
         && getHeight() <= targetArea.getHeight())
    {
        sourceArea = getLocalBounds();
    }
    else
    {
        auto sourceRatio = getHeight() / (double) getWidth();
        auto targetRatio = targetArea.getHeight() / (double) targetArea.getWidth();

        if (sourceRatio <= targetRatio)
            sourceArea.setHeight (jmin (targetArea.getHeight(),
                                        roundToInt (targetArea.getWidth() * sourceRatio)));
        else
            sourceArea.setWidth (jmin (targetArea.getWidth(),
                                       roundToInt (targetArea.getHeight() / sourceRatio)));
    }

    if (! sourceArea.isEmpty())
        setBounds (justification.appliedToRectangle (sourceArea, targetArea));
}

//==============================================================================
void Component::setTransform (const AffineTransform& newTransform)
{
    // If you pass in a transform with no inverse, the component will have no dimensions,
    // and there will be all sorts of maths errors when converting coordinates.
    jassert (! newTransform.isSingularity());

    if (newTransform.isIdentity())
    {
        if (affineTransform != nullptr)
        {
            repaint();
            affineTransform.reset();
            repaint();
            sendMovedResizedMessages (false, false);
        }
    }
    else if (affineTransform == nullptr)
    {
        repaint();
        affineTransform.reset (new AffineTransform (newTransform));
        repaint();
        sendMovedResizedMessages (false, false);
    }
    else if (*affineTransform != newTransform)
    {
        repaint();
        *affineTransform = newTransform;
        repaint();
        sendMovedResizedMessages (false, false);
    }
}

bool Component::isTransformed() const noexcept
{
    return affineTransform != nullptr;
}

AffineTransform Component::getTransform() const
{
    return affineTransform != nullptr ? *affineTransform : AffineTransform();
}

float Component::getApproximateScaleFactorForComponent (const Component* targetComponent)
{
    AffineTransform transform;

    for (auto* target = targetComponent; target != nullptr; target = target->getParentComponent())
    {
        transform = transform.followedBy (target->getTransform());

        if (target->isOnDesktop())
            transform = transform.scaled (target->getDesktopScaleFactor());
    }

    auto transformScale = std::sqrt (std::abs (transform.getDeterminant()));
    return transformScale / Desktop::getInstance().getGlobalScaleFactor();
}

//==============================================================================
bool Component::hitTest (int x, int y)
{
    if (! flags.ignoresMouseClicksFlag)
        return true;

    if (flags.allowChildMouseClicksFlag)
    {
        for (int i = childComponentList.size(); --i >= 0;)
        {
            auto& child = *childComponentList.getUnchecked (i);

            if (child.isVisible()
                 && detail::ComponentHelpers::hitTest (child, detail::ComponentHelpers::convertFromParentSpace (child, Point<int> (x, y).toFloat())))
                return true;
        }
    }

    return false;
}

void Component::setInterceptsMouseClicks (bool allowClicks,
                                          bool allowClicksOnChildComponents) noexcept
{
    flags.ignoresMouseClicksFlag = ! allowClicks;
    flags.allowChildMouseClicksFlag = allowClicksOnChildComponents;
}

void Component::getInterceptsMouseClicks (bool& allowsClicksOnThisComponent,
                                          bool& allowsClicksOnChildComponents) const noexcept
{
    allowsClicksOnThisComponent = ! flags.ignoresMouseClicksFlag;
    allowsClicksOnChildComponents = flags.allowChildMouseClicksFlag;
}

bool Component::contains (Point<int> point)
{
    return contains (point.toFloat());
}

bool Component::contains (Point<float> point)
{
    if (detail::ComponentHelpers::hitTest (*this, point))
    {
        if (parentComponent != nullptr)
            return parentComponent->contains (detail::ComponentHelpers::convertToParentSpace (*this, point));

        if (flags.hasHeavyweightPeerFlag)
            if (auto* peer = getPeer())
                return peer->contains (detail::ComponentHelpers::localPositionToRawPeerPos (*this, point).roundToInt(), true);
    }

    return false;
}

bool Component::reallyContains (Point<int> point, bool returnTrueIfWithinAChild)
{
    return reallyContains (point.toFloat(), returnTrueIfWithinAChild);
}

bool Component::reallyContains (Point<float> point, bool returnTrueIfWithinAChild)
{
    if (! contains (point))
        return false;

    auto* top = getTopLevelComponent();
    auto* compAtPosition = top->getComponentAt (top->getLocalPoint (this, point));

    return (compAtPosition == this) || (returnTrueIfWithinAChild && isParentOf (compAtPosition));
}

Component* Component::getComponentAt (Point<int> position)
{
    return getComponentAt (position.toFloat());
}

Component* Component::getComponentAt (Point<float> position)
{
    if (flags.visibleFlag && detail::ComponentHelpers::hitTest (*this, position))
    {
        for (int i = childComponentList.size(); --i >= 0;)
        {
            auto* child = childComponentList.getUnchecked (i);

            child = child->getComponentAt (detail::ComponentHelpers::convertFromParentSpace (*child, position));

            if (child != nullptr)
                return child;
        }

        return this;
    }

    return nullptr;
}

Component* Component::getComponentAt (int x, int y)
{
    return getComponentAt (Point<int> { x, y });
}

//==============================================================================
void Component::addChildComponent (Component& child, int zOrder)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    jassert (this != &child); // adding a component to itself!?

    if (child.parentComponent != this)
    {
        if (child.parentComponent != nullptr)
            child.parentComponent->removeChildComponent (&child);
        else
            child.removeFromDesktop();

        child.parentComponent = this;

        if (child.isVisible())
            child.repaintParent();

        if (! child.isAlwaysOnTop())
        {
            if (zOrder < 0 || zOrder > childComponentList.size())
                zOrder = childComponentList.size();

            while (zOrder > 0)
            {
                if (! childComponentList.getUnchecked (zOrder - 1)->isAlwaysOnTop())
                    break;

                --zOrder;
            }
        }

        childComponentList.insert (zOrder, &child);

        child.internalHierarchyChanged();
        internalChildrenChanged();
    }
}

void Component::addAndMakeVisible (Component& child, int zOrder)
{
    child.setVisible (true);
    addChildComponent (child, zOrder);
}

void Component::addChildComponent (Component* child, int zOrder)
{
    if (child != nullptr)
        addChildComponent (*child, zOrder);
}

void Component::addAndMakeVisible (Component* child, int zOrder)
{
    if (child != nullptr)
        addAndMakeVisible (*child, zOrder);
}

void Component::addChildAndSetID (Component* child, const String& childID)
{
    if (child != nullptr)
    {
        child->setComponentID (childID);
        addAndMakeVisible (child);
    }
}

void Component::removeChildComponent (Component* child)
{
    removeChildComponent (childComponentList.indexOf (child), true, true);
}

Component* Component::removeChildComponent (int index)
{
    return removeChildComponent (index, true, true);
}

Component* Component::removeChildComponent (int index, bool sendParentEvents, bool sendChildEvents)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    if (auto* child = childComponentList [index])
    {
        sendParentEvents = sendParentEvents && child->isShowing();

        if (sendParentEvents)
        {
            sendFakeMouseMove();

            if (child->isVisible())
                child->repaintParent();
        }

        childComponentList.remove (index);
        child->parentComponent = nullptr;

        detail::ComponentHelpers::releaseAllCachedImageResources (*child);

        // (NB: there are obscure situations where child->isShowing() = false, but it still has the focus)
        if (child->hasKeyboardFocus (true))
        {
            const WeakReference<Component> safeThis (this);

            child->giveAwayKeyboardFocusInternal (sendChildEvents || currentlyFocusedComponent != child);

            if (sendParentEvents)
            {
                if (safeThis == nullptr)
                    return child;

                grabKeyboardFocus();
            }
        }

        if (sendChildEvents)
            child->internalHierarchyChanged();

        if (sendParentEvents)
            internalChildrenChanged();

        return child;
    }

    return nullptr;
}

//==============================================================================
void Component::removeAllChildren()
{
    while (! childComponentList.isEmpty())
        removeChildComponent (childComponentList.size() - 1);
}

void Component::deleteAllChildren()
{
    while (! childComponentList.isEmpty())
        delete (removeChildComponent (childComponentList.size() - 1));
}

int Component::getNumChildComponents() const noexcept
{
    return childComponentList.size();
}

Component* Component::getChildComponent (int index) const noexcept
{
    return childComponentList[index];
}

int Component::getIndexOfChildComponent (const Component* child) const noexcept
{
    return childComponentList.indexOf (const_cast<Component*> (child));
}

Component* Component::findChildWithID (StringRef targetID) const noexcept
{
    for (auto* c : childComponentList)
        if (c->componentID == targetID)
            return c;

    return nullptr;
}

Component* Component::getTopLevelComponent() const noexcept
{
    auto* comp = this;

    while (comp->parentComponent != nullptr)
        comp = comp->parentComponent;

    return const_cast<Component*> (comp);
}

bool Component::isParentOf (const Component* possibleChild) const noexcept
{
    while (possibleChild != nullptr)
    {
        possibleChild = possibleChild->parentComponent;

        if (possibleChild == this)
            return true;
    }

    return false;
}

//==============================================================================
void Component::parentHierarchyChanged() {}
void Component::childrenChanged() {}

void Component::internalChildrenChanged()
{
    if (componentListeners.isEmpty())
    {
        childrenChanged();
    }
    else
    {
        BailOutChecker checker (this);

        childrenChanged();

        if (! checker.shouldBailOut())
            componentListeners.callChecked (checker, [this] (ComponentListener& l) { l.componentChildrenChanged (*this); });
    }
}

void Component::internalHierarchyChanged()
{
    BailOutChecker checker (this);

    parentHierarchyChanged();

    if (checker.shouldBailOut())
        return;

    componentListeners.callChecked (checker, [this] (ComponentListener& l) { l.componentParentHierarchyChanged (*this); });

    if (checker.shouldBailOut())
        return;

    for (int i = childComponentList.size(); --i >= 0;)
    {
        childComponentList.getUnchecked (i)->internalHierarchyChanged();

        if (checker.shouldBailOut())
        {
            // you really shouldn't delete the parent component during a callback telling you
            // that it's changed..
            jassertfalse;
            return;
        }

        i = jmin (i, childComponentList.size());
    }

    if (flags.hasHeavyweightPeerFlag)
        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::structureChanged);
}

//==============================================================================
#if JUCE_MODAL_LOOPS_PERMITTED
int Component::runModalLoop()
{
    if (! MessageManager::getInstance()->isThisTheMessageThread())
    {
        // use a callback so this can be called from non-gui threads
        return (int) (pointer_sized_int) MessageManager::getInstance()
                                           ->callFunctionOnMessageThread (&detail::ComponentHelpers::runModalLoopCallback, this);
    }

    if (! isCurrentlyModal (false))
        enterModalState (true);

    return ModalComponentManager::getInstance()->runEventLoopForCurrentComponent();
}
#endif

//==============================================================================
void Component::enterModalState (bool shouldTakeKeyboardFocus,
                                 ModalComponentManager::Callback* callback,
                                 bool deleteWhenDismissed)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    SafePointer safeReference { this };

    if (! isCurrentlyModal (false))
    {
        // While this component is in modal state it may block other components from receiving
        // mouseExit events. To keep mouseEnter and mouseExit calls balanced on these components,
        // we must manually force the mouse to "leave" blocked components.
        detail::ComponentHelpers::sendMouseEventToComponentsThatAreBlockedByModal (*this, &Component::internalMouseExit);

        if (safeReference == nullptr)
        {
            // If you hit this assertion, the mouse-exit event above has caused the modal component to be deleted.
            jassertfalse;
            return;
        }

        auto& mcm = *ModalComponentManager::getInstance();
        mcm.startModal ({}, this, deleteWhenDismissed);
        mcm.attachCallback (this, callback);

        setVisible (true);

        if (shouldTakeKeyboardFocus)
            grabKeyboardFocus();
    }
    else
    {
        // Probably a bad idea to try to make a component modal twice!
        jassertfalse;
    }
}

void Component::exitModalState (int returnValue)
{
    WeakReference<Component> deletionChecker (this);

    if (isCurrentlyModal (false))
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            auto& mcm = *ModalComponentManager::getInstance();
            mcm.endModal ({}, this, returnValue);
            mcm.bringModalComponentsToFront();

            // While this component is in modal state it may block other components from receiving
            // mouseEnter events. To keep mouseEnter and mouseExit calls balanced on these components,
            // we must manually force the mouse to "enter" blocked components.
            if (deletionChecker != nullptr)
                detail::ComponentHelpers::sendMouseEventToComponentsThatAreBlockedByModal (*deletionChecker, &Component::internalMouseEnter);
        }
        else
        {
            MessageManager::callAsync ([target = WeakReference<Component> { this }, returnValue]
            {
                if (target != nullptr)
                    target->exitModalState (returnValue);
            });
        }
    }
}

bool Component::isCurrentlyModal (bool onlyConsiderForemostModalComponent) const noexcept
{
    auto& mcm = *ModalComponentManager::getInstance();

    return onlyConsiderForemostModalComponent ? mcm.isFrontModalComponent (this)
                                              : mcm.isModal (this);
}

bool Component::isCurrentlyBlockedByAnotherModalComponent() const
{
    return detail::ComponentHelpers::modalWouldBlockComponent (*this, getCurrentlyModalComponent());
}

int JUCE_CALLTYPE Component::getNumCurrentlyModalComponents() noexcept
{
    if (auto* manager = ModalComponentManager::getInstanceWithoutCreating())
        return manager->getNumModalComponents();

    return {};
}

Component* JUCE_CALLTYPE Component::getCurrentlyModalComponent (int index) noexcept
{
    if (auto* manager = ModalComponentManager::getInstanceWithoutCreating())
        return manager->getModalComponent (index);

    return {};
}

//==============================================================================
void Component::setBroughtToFrontOnMouseClick (bool shouldBeBroughtToFront) noexcept
{
    flags.bringToFrontOnClickFlag = shouldBeBroughtToFront;
}

bool Component::isBroughtToFrontOnMouseClick() const noexcept
{
    return flags.bringToFrontOnClickFlag;
}

//==============================================================================
void Component::setMouseCursor (const MouseCursor& newCursor)
{
    if (cursor != newCursor)
    {
        cursor = newCursor;

        if (flags.visibleFlag)
            updateMouseCursor();
    }
}

MouseCursor Component::getMouseCursor()
{
    return cursor;
}

void Component::updateMouseCursor() const
{
    Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
}

//==============================================================================
void Component::setRepaintsOnMouseActivity (bool shouldRepaint) noexcept
{
    flags.repaintOnMouseActivityFlag = shouldRepaint;
}

//==============================================================================
float Component::getAlpha() const noexcept
{
    return (255 - componentTransparency) / 255.0f;
}

void Component::setAlpha (float newAlpha)
{
    auto newIntAlpha = (uint8) (255 - jlimit (0, 255, roundToInt (newAlpha * 255.0)));

    if (componentTransparency != newIntAlpha)
    {
        componentTransparency = newIntAlpha;
        alphaChanged();
    }
}

void Component::alphaChanged()
{
    if (flags.hasHeavyweightPeerFlag)
    {
        if (auto* peer = getPeer())
            peer->setAlpha (getAlpha());
    }
    else
    {
        repaint();
    }
}

//==============================================================================
void Component::repaint()
{
    internalRepaintUnchecked (getLocalBounds(), true);
}

void Component::repaint (int x, int y, int w, int h)
{
    internalRepaint ({ x, y, w, h });
}

void Component::repaint (Rectangle<int> area)
{
    internalRepaint (area);
}

void Component::repaintParent()
{
    if (parentComponent != nullptr)
        parentComponent->internalRepaint (detail::ComponentHelpers::convertToParentSpace (*this, getLocalBounds()));
}

void Component::internalRepaint (Rectangle<int> area)
{
    area = area.getIntersection (getLocalBounds());

    if (! area.isEmpty())
        internalRepaintUnchecked (area, false);
}

void Component::internalRepaintUnchecked (Rectangle<int> area, bool isEntireComponent)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (flags.visibleFlag)
    {
        if (cachedImage != nullptr)
            if (! (isEntireComponent ? cachedImage->invalidateAll()
                                     : cachedImage->invalidate (area)))
                return;

        if (area.isEmpty())
            return;

        if (flags.hasHeavyweightPeerFlag)
        {
            if (auto* peer = getPeer())
            {
                // Tweak the scaling so that the component's integer size exactly aligns with the peer's scaled size
                auto peerBounds = peer->getBounds();
                auto scaled = area * Point<float> ((float) peerBounds.getWidth()  / (float) getWidth(),
                                                   (float) peerBounds.getHeight() / (float) getHeight());

                peer->repaint (affineTransform != nullptr ? scaled.transformedBy (*affineTransform) : scaled);
            }
        }
        else
        {
            if (parentComponent != nullptr)
                parentComponent->internalRepaint (detail::ComponentHelpers::convertToParentSpace (*this, area));
        }
    }
}

//==============================================================================
void Component::paint (Graphics&)
{
    // if your component is marked as opaque, you must implement a paint
    // method and ensure that its entire area is completely painted.
    jassert (getBounds().isEmpty() || ! isOpaque());
}

void Component::paintOverChildren (Graphics&)
{
    // all painting is done in the subclasses
}

//==============================================================================
void Component::paintWithinParentContext (Graphics& g)
{
    g.setOrigin (getPosition());

    if (cachedImage != nullptr)
        cachedImage->paint (g);
    else
        paintEntireComponent (g, false);
}

void Component::paintComponentAndChildren (Graphics& g)
{
   #if JUCE_ETW_TRACELOGGING
    {
        int depth = 0;
        auto parent = getParentComponent();
        while (parent)
        {
            parent = parent->getParentComponent();
            depth++;
        }

        JUCE_TRACE_LOG_PAINT_COMPONENT_AND_CHILDREN (depth);
    }
   #endif

    auto clipBounds = g.getClipBounds();

    if (flags.dontClipGraphicsFlag && getNumChildComponents() == 0)
    {
        paint (g);
    }
    else
    {
        Graphics::ScopedSaveState ss (g);

        if (! (detail::ComponentHelpers::clipObscuredRegions (*this, g, clipBounds, {}) && g.isClipEmpty()))
            paint (g);
    }

    for (int i = 0; i < childComponentList.size(); ++i)
    {
        auto& child = *childComponentList.getUnchecked (i);

        if (child.isVisible())
        {
            if (child.affineTransform != nullptr)
            {
                Graphics::ScopedSaveState ss (g);

                g.addTransform (*child.affineTransform);

                if ((child.flags.dontClipGraphicsFlag && ! g.isClipEmpty()) || g.reduceClipRegion (child.getBounds()))
                    child.paintWithinParentContext (g);
            }
            else if (clipBounds.intersects (child.getBounds()))
            {
                Graphics::ScopedSaveState ss (g);

                if (child.flags.dontClipGraphicsFlag)
                {
                    child.paintWithinParentContext (g);
                }
                else if (g.reduceClipRegion (child.getBounds()))
                {
                    bool nothingClipped = true;

                    for (int j = i + 1; j < childComponentList.size(); ++j)
                    {
                        auto& sibling = *childComponentList.getUnchecked (j);

                        if (sibling.flags.opaqueFlag && sibling.isVisible() && sibling.affineTransform == nullptr)
                        {
                            nothingClipped = false;
                            g.excludeClipRegion (sibling.getBounds());
                        }
                    }

                    if (nothingClipped || ! g.isClipEmpty())
                        child.paintWithinParentContext (g);
                }
            }
        }
    }

    Graphics::ScopedSaveState ss (g);
    paintOverChildren (g);
}

void Component::paintEntireComponent (Graphics& g, bool ignoreAlphaLevel)
{
    // If sizing a top-level-window and the OS paint message is delivered synchronously
    // before resized() is called, then we'll invoke the callback here, to make sure
    // the components inside have had a chance to sort their sizes out..
   #if JUCE_DEBUG
    if (! flags.isInsidePaintCall) // (avoids an assertion in plugins hosted in WaveLab)
   #endif
        sendMovedResizedMessagesIfPending();

   #if JUCE_DEBUG
    flags.isInsidePaintCall = true;
   #endif

    if (effectState != nullptr)
    {
        effectState->paint (g, *this, ignoreAlphaLevel);
    }
    else if (componentTransparency > 0 && ! ignoreAlphaLevel)
    {
        if (componentTransparency < 255)
        {
            g.beginTransparencyLayer (getAlpha());
            paintComponentAndChildren (g);
            g.endTransparencyLayer();
        }
    }
    else
    {
        paintComponentAndChildren (g);
    }

   #if JUCE_DEBUG
    flags.isInsidePaintCall = false;
   #endif
}

void Component::setPaintingIsUnclipped (bool shouldPaintWithoutClipping) noexcept
{
    flags.dontClipGraphicsFlag = shouldPaintWithoutClipping;
}

bool Component::isPaintingUnclipped() const noexcept
{
    return flags.dontClipGraphicsFlag;
}

//==============================================================================
Image Component::createComponentSnapshot (Rectangle<int> areaToGrab,
                                          bool clipImageToComponentBounds,
                                          float scaleFactor,
                                          const ImageType& imageType)
{
    auto r = areaToGrab;

    if (clipImageToComponentBounds)
        r = r.getIntersection (getLocalBounds());

    if (r.isEmpty())
        return {};

    auto w = roundToInt (scaleFactor * (float) r.getWidth());
    auto h = roundToInt (scaleFactor * (float) r.getHeight());

    Image image (flags.opaqueFlag ? Image::RGB : Image::ARGB, w, h, true, imageType);

    Graphics g (image);

    if (w != getWidth() || h != getHeight())
        g.addTransform (AffineTransform::scale ((float) w / (float) r.getWidth(),
                                                (float) h / (float) r.getHeight()));
    g.setOrigin (-r.getPosition());

    paintEntireComponent (g, true);

    return image;
}

ImageEffectFilter* Component::getComponentEffect() const noexcept
{
    return effectState != nullptr ? &effectState->getEffect() : nullptr;
}

void Component::setComponentEffect (ImageEffectFilter* newEffect)
{
    if (newEffect == nullptr && effectState == nullptr)
        return;

    const auto needsRepaint = [&]
    {
        if (newEffect == nullptr)
        {
            effectState.reset();
            return true;
        }

        if (effectState == nullptr)
        {
            effectState = std::make_unique<EffectState> (*newEffect);
            return true;
        }

        return effectState->setEffect (*newEffect);
    }();

    if (needsRepaint)
        repaint();
}

//==============================================================================
LookAndFeel& Component::getLookAndFeel() const noexcept
{
    for (auto* c = this; c != nullptr; c = c->parentComponent)
        if (auto lf = c->lookAndFeel.get())
            return *lf;

    return LookAndFeel::getDefaultLookAndFeel();
}

void Component::setLookAndFeel (LookAndFeel* newLookAndFeel)
{
    if (lookAndFeel != newLookAndFeel)
    {
        lookAndFeel = newLookAndFeel;
        sendLookAndFeelChange();
    }
}

FontOptions Component::withDefaultMetrics (FontOptions opt) const
{
    return getLookAndFeel().withDefaultMetrics (std::move (opt));
}

void Component::lookAndFeelChanged() {}
void Component::colourChanged() {}

void Component::sendLookAndFeelChange()
{
    const WeakReference<Component> safePointer (this);
    repaint();
    lookAndFeelChanged();

    if (safePointer != nullptr)
    {
        colourChanged();

        if (safePointer != nullptr)
        {
            for (int i = childComponentList.size(); --i >= 0;)
            {
                childComponentList.getUnchecked (i)->sendLookAndFeelChange();

                if (safePointer == nullptr)
                    return;

                i = jmin (i, childComponentList.size());
            }
        }
    }
}

Colour Component::findColour (int colourID, bool inheritFromParent) const
{
    if (auto* v = properties.getVarPointer (detail::ComponentHelpers::getColourPropertyID (colourID)))
        return Colour ((uint32) static_cast<int> (*v));

    if (inheritFromParent && parentComponent != nullptr
         && (lookAndFeel == nullptr || ! lookAndFeel->isColourSpecified (colourID)))
        return parentComponent->findColour (colourID, true);

    return getLookAndFeel().findColour (colourID);
}

bool Component::isColourSpecified (int colourID) const
{
    return properties.contains (detail::ComponentHelpers::getColourPropertyID (colourID));
}

void Component::removeColour (int colourID)
{
    if (properties.remove (detail::ComponentHelpers::getColourPropertyID (colourID)))
        colourChanged();
}

void Component::setColour (int colourID, Colour colour)
{
    if (properties.set (detail::ComponentHelpers::getColourPropertyID (colourID), (int) colour.getARGB()))
        colourChanged();
}

void Component::copyAllExplicitColoursTo (Component& target) const
{
    bool changed = false;

    for (int i = properties.size(); --i >= 0;)
    {
        auto name = properties.getName (i);

        if (name.toString().startsWith (detail::colourPropertyPrefix))
            if (target.properties.set (name, properties [name]))
                changed = true;
    }

    if (changed)
        target.colourChanged();
}

//==============================================================================
Component::Positioner::Positioner (Component& c) noexcept  : component (c)
{
}

Component::Positioner* Component::getPositioner() const noexcept
{
    return positioner.get();
}

void Component::setPositioner (Positioner* newPositioner)
{
    // You can only assign a positioner to the component that it was created for!
    jassert (newPositioner == nullptr || this == &(newPositioner->getComponent()));
    positioner.reset (newPositioner);
}

//==============================================================================
Rectangle<int> Component::getLocalBounds() const noexcept
{
    return boundsRelativeToParent.withZeroOrigin();
}

Rectangle<int> Component::getBoundsInParent() const noexcept
{
    return affineTransform == nullptr ? boundsRelativeToParent
                                      : boundsRelativeToParent.transformedBy (*affineTransform);
}

//==============================================================================
void Component::mouseEnter (const MouseEvent&)          {}
void Component::mouseExit  (const MouseEvent&)          {}
void Component::mouseDown  (const MouseEvent&)          {}
void Component::mouseUp    (const MouseEvent&)          {}
void Component::mouseDrag  (const MouseEvent&)          {}
void Component::mouseMove  (const MouseEvent&)          {}
void Component::mouseDoubleClick (const MouseEvent&)    {}

void Component::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    // the base class just passes this event up to the nearest enabled ancestor
    if (auto* enabledComponent = findFirstEnabledAncestor (getParentComponent()))
        enabledComponent->mouseWheelMove (e.getEventRelativeTo (enabledComponent), wheel);
}

void Component::mouseMagnify (const MouseEvent& e, float magnifyAmount)
{
    // the base class just passes this event up to the nearest enabled ancestor
    if (auto* enabledComponent = findFirstEnabledAncestor (getParentComponent()))
        enabledComponent->mouseMagnify (e.getEventRelativeTo (enabledComponent), magnifyAmount);
}

//==============================================================================
void Component::resized()                       {}
void Component::moved()                         {}
void Component::childBoundsChanged (Component*) {}
void Component::parentSizeChanged()             {}

void Component::addComponentListener (ComponentListener* newListener)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
   #if JUCE_DEBUG || JUCE_LOG_ASSERTIONS
    if (getParentComponent() != nullptr)
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED
    }
   #endif

    componentListeners.add (newListener);
}

void Component::removeComponentListener (ComponentListener* listenerToRemove)
{
    componentListeners.remove (listenerToRemove);
}

//==============================================================================
void Component::inputAttemptWhenModal()
{
    ModalComponentManager::getInstance()->bringModalComponentsToFront();
    getLookAndFeel().playAlertSound();
}

bool Component::canModalEventBeSentToComponent (const Component*)
{
    return false;
}

void Component::internalModalInputAttempt()
{
    if (auto* current = getCurrentlyModalComponent())
        current->inputAttemptWhenModal();
}

//==============================================================================
void Component::postCommandMessage (int commandID)
{
    MessageManager::callAsync ([target = WeakReference<Component> { this }, commandID]
    {
        if (target != nullptr)
            target->handleCommandMessage (commandID);
    });
}

void Component::handleCommandMessage (int)
{
    // used by subclasses
}

//==============================================================================
void Component::addMouseListener (MouseListener* newListener,
                                  bool wantsEventsForAllNestedChildComponents)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    // If you register a component as a mouselistener for itself, it'll receive all the events
    // twice - once via the direct callback that all components get anyway, and then again as a listener!
    jassert ((newListener != this) || wantsEventsForAllNestedChildComponents);

    if (mouseListeners == nullptr)
        mouseListeners.reset (new MouseListenerList());

    mouseListeners->addListener (newListener, wantsEventsForAllNestedChildComponents);
}

void Component::removeMouseListener (MouseListener* listenerToRemove)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (mouseListeners != nullptr)
        mouseListeners->removeListener (listenerToRemove);
}

//==============================================================================
void Component::internalMouseEnter (SafePointer<Component> target, MouseInputSource source, Point<float> relativePos, Time time)
{
    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        // if something else is modal, always just show a normal mouse cursor
        source.showMouseCursor (MouseCursor::NormalCursor);
        return;
    }

    if (target->flags.repaintOnMouseActivityFlag)
        target->repaint();

    const auto me = makeMouseEvent (source,
                                    detail::PointerState().withPosition (relativePos),
                                    source.getCurrentModifiers(),
                                    target,
                                    target,
                                    time,
                                    relativePos,
                                    time,
                                    0,
                                    false);

    HierarchyChecker checker (&target, me);
    target->mouseEnter (me);

    if (checker.shouldBailOut())
        return;

    target->flags.cachedMouseInsideComponent = true;

    if (checker.shouldBailOut())
        return;

    Desktop::getInstance().getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseEnter (me); });
    MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseEnter);
}

void Component::internalMouseExit (SafePointer<Component> target, MouseInputSource source, Point<float> relativePos, Time time)
{
    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        // if something else is modal, always just show a normal mouse cursor
        source.showMouseCursor (MouseCursor::NormalCursor);
        return;
    }

    if (target->flags.repaintOnMouseActivityFlag)
        target->repaint();

    target->flags.cachedMouseInsideComponent = false;

    const auto me = makeMouseEvent (source,
                                    detail::PointerState().withPosition (relativePos),
                                    source.getCurrentModifiers(),
                                    target,
                                    target,
                                    time,
                                    relativePos,
                                    time,
                                    0,
                                    false);

    HierarchyChecker checker (&target, me);
    target->mouseExit (me);

    if (checker.shouldBailOut())
        return;

    Desktop::getInstance().getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseExit (me); });
    MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseExit);
}

void Component::internalMouseDown (SafePointer<Component> target,
                                   MouseInputSource source,
                                   const detail::PointerState& relativePointerState,
                                   Time time)
{
    auto& desktop = Desktop::getInstance();

    const auto me = makeMouseEvent (source,
                                    relativePointerState,
                                    source.getCurrentModifiers(),
                                    target,
                                    target,
                                    time,
                                    relativePointerState.position,
                                    time,
                                    source.getNumberOfMultipleClicks(),
                                    false);

    HierarchyChecker checker (&target, me);

    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        target->flags.mouseDownWasBlocked = true;
        target->internalModalInputAttempt();

        if (checker.shouldBailOut())
            return;

        // If processing the input attempt has exited the modal loop, we'll allow the event
        // to be delivered..
        if (target->isCurrentlyBlockedByAnotherModalComponent())
        {
            // allow blocked mouse-events to go to global listeners..
            desktop.getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseDown (checker.eventWithNearestParent()); });
            return;
        }
    }

    target->flags.mouseDownWasBlocked = false;

    checker.forEach ([] (auto& comp)
    {
        if (comp.isBroughtToFrontOnMouseClick())
            comp.toFront (true);
    });

    if (checker.shouldBailOut())
        return;

    target->grabKeyboardFocusInternal (focusChangedByMouseClick, true, FocusChangeDirection::unknown);

    if (checker.shouldBailOut())
        return;

    if (target->flags.repaintOnMouseActivityFlag)
        target->repaint();

    target->mouseDown (me);

    if (checker.shouldBailOut())
        return;

    desktop.getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseDown (checker.eventWithNearestParent()); });

    MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseDown);
}

void Component::internalMouseUp (SafePointer<Component> target,
                                 MouseInputSource source,
                                 const detail::PointerState& relativePointerState,
                                 Time time,
                                 const ModifierKeys oldModifiers)
{
    const auto originalTarget = target;

    const auto me = makeMouseEvent (source,
                                    relativePointerState,
                                    oldModifiers,
                                    target,
                                    target,
                                    time,
                                    target->getLocalPoint (nullptr, source.getLastMouseDownPosition()),
                                    source.getLastMouseDownTime(),
                                    source.getNumberOfMultipleClicks(),
                                    source.isLongPressOrDrag());

    HierarchyChecker checker (&target, me);

    if (target->flags.mouseDownWasBlocked && target->isCurrentlyBlockedByAnotherModalComponent())
    {
        // Global listeners still need to know about the mouse up
        auto& desktop = Desktop::getInstance();
        desktop.getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseUp (checker.eventWithNearestParent()); });
        return;
    }

    if (target->flags.repaintOnMouseActivityFlag)
        target->repaint();

    target->mouseUp (me);

    if (checker.shouldBailOut())
        return;

    auto& desktop = Desktop::getInstance();
    desktop.getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseUp (checker.eventWithNearestParent()); });

    MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseUp);

    if (checker.shouldBailOut())
        return;

    // check for double-click
    if (me.getNumberOfClicks() >= 2)
    {
        if (target == originalTarget)
            target->mouseDoubleClick (checker.eventWithNearestParent());

        if (checker.shouldBailOut())
            return;

        desktop.mouseListeners.callChecked (checker, [&] (MouseListener& l) { l.mouseDoubleClick (checker.eventWithNearestParent()); });
        MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseDoubleClick);
    }
}

void Component::internalMouseDrag (SafePointer<Component> target, MouseInputSource source, const detail::PointerState& relativePointerState, Time time)
{
    if (! target->isCurrentlyBlockedByAnotherModalComponent())
    {
        const auto me = makeMouseEvent (source,
                                        relativePointerState,
                                        source.getCurrentModifiers(),
                                        target,
                                        target,
                                        time,
                                        target->getLocalPoint (nullptr, source.getLastMouseDownPosition()),
                                        source.getLastMouseDownTime(),
                                        source.getNumberOfMultipleClicks(),
                                        source.isLongPressOrDrag());

        HierarchyChecker checker (&target, me);

        target->mouseDrag (me);

        if (checker.shouldBailOut())
            return;

        Desktop::getInstance().getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseDrag (checker.eventWithNearestParent()); });
        MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseDrag);
    }
}

void Component::internalMouseMove (SafePointer<Component> target, MouseInputSource source, Point<float> relativePos, Time time)
{
    auto& desktop = Desktop::getInstance();

    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        // allow blocked mouse-events to go to global listeners..
        desktop.sendMouseMove();
    }
    else
    {
        const auto me = makeMouseEvent (source,
                                        detail::PointerState().withPosition (relativePos),
                                        source.getCurrentModifiers(),
                                        target,
                                        target,
                                        time,
                                        relativePos,
                                        time,
                                        0,
                                        false);

        HierarchyChecker checker (&target, me);

        target->mouseMove (me);

        if (checker.shouldBailOut())
            return;

        desktop.getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseMove (checker.eventWithNearestParent()); });
        MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseMove);
    }
}

void Component::internalMouseWheel (SafePointer<Component> target, MouseInputSource source, Point<float> relativePos,
                                    Time time, const MouseWheelDetails& wheel)
{
    auto& desktop = Desktop::getInstance();

    const auto me = makeMouseEvent (source,
                                    detail::PointerState().withPosition (relativePos),
                                    source.getCurrentModifiers(),
                                    target,
                                    target,
                                    time,
                                    relativePos,
                                    time,
                                    0,
                                    false);

    HierarchyChecker checker (&target, me);

    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        // allow blocked mouse-events to go to global listeners..
        desktop.mouseListeners.callChecked (checker, [&] (MouseListener& l) { l.mouseWheelMove (me, wheel); });
    }
    else
    {
        target->mouseWheelMove (me, wheel);

        if (checker.shouldBailOut())
            return;

        desktop.mouseListeners.callChecked (checker, [&] (MouseListener& l) { l.mouseWheelMove (checker.eventWithNearestParent(), wheel); });

        if (! checker.shouldBailOut())
            MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseWheelMove, wheel);
    }
}

void Component::internalMagnifyGesture (SafePointer<Component> target, MouseInputSource source, Point<float> relativePos,
                                        Time time, float amount)
{
    auto& desktop = Desktop::getInstance();

    const auto me = makeMouseEvent (source,
                                    detail::PointerState().withPosition (relativePos),
                                    source.getCurrentModifiers(),
                                    target,
                                    target,
                                    time,
                                    relativePos,
                                    time,
                                    0,
                                    false);

    HierarchyChecker checker (&target, me);

    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        // allow blocked mouse-events to go to global listeners..
        desktop.mouseListeners.callChecked (checker, [&] (MouseListener& l) { l.mouseMagnify (me, amount); });
    }
    else
    {
        target->mouseMagnify (me, amount);

        if (checker.shouldBailOut())
            return;

        desktop.mouseListeners.callChecked (checker, [&] (MouseListener& l) { l.mouseMagnify (checker.eventWithNearestParent(), amount); });

        if (! checker.shouldBailOut())
            MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseMagnify, amount);
    }
}

void Component::sendFakeMouseMove() const
{
    if (flags.ignoresMouseClicksFlag && ! flags.allowChildMouseClicksFlag)
        return;

    auto mainMouse = Desktop::getInstance().getMainMouseSource();

    if (! mainMouse.isDragging())
        mainMouse.triggerFakeMove();
}

void JUCE_CALLTYPE Component::beginDragAutoRepeat (int interval)
{
    Desktop::getInstance().beginDragAutoRepeat (interval);
}

//==============================================================================
void Component::broughtToFront()
{
}

void Component::internalBroughtToFront()
{
    if (flags.hasHeavyweightPeerFlag)
        Desktop::getInstance().componentBroughtToFront (this);

    BailOutChecker checker (this);
    broughtToFront();

    if (checker.shouldBailOut())
        return;

    componentListeners.callChecked (checker, [this] (ComponentListener& l) { l.componentBroughtToFront (*this); });

    if (checker.shouldBailOut())
        return;

    // When brought to the front and there's a modal component blocking this one,
    // we need to bring the modal one to the front instead..
    if (auto* cm = getCurrentlyModalComponent())
        if (cm->getTopLevelComponent() != getTopLevelComponent())
            ModalComponentManager::getInstance()->bringModalComponentsToFront (false); // very important that this is false, otherwise in Windows,
                                                                                       // non-front components can't get focus when another modal comp is
                                                                                       // active, and therefore can't receive mouse-clicks
}

//==============================================================================
void Component::focusGained (FocusChangeType)   {}
void Component::focusGainedWithDirection (FocusChangeType, FocusChangeDirection) {}
void Component::focusLost (FocusChangeType)     {}
void Component::focusOfChildComponentChanged (FocusChangeType) {}

void Component::internalKeyboardFocusGain (FocusChangeType cause)
{
    internalKeyboardFocusGain (cause, WeakReference<Component> (this), FocusChangeDirection::unknown);
}

void Component::internalKeyboardFocusGain (FocusChangeType cause,
                                           const WeakReference<Component>& safePointer,
                                           FocusChangeDirection direction)
{
    focusGainedWithDirection (cause, direction);
    focusGained (cause);

    if (safePointer == nullptr)
        return;

    if (hasKeyboardFocus (false))
        if (auto* handler = getAccessibilityHandler())
            handler->grabFocus();

    if (safePointer == nullptr)
        return;

    internalChildKeyboardFocusChange (cause, safePointer);
}

void Component::internalKeyboardFocusLoss (FocusChangeType cause)
{
    const WeakReference<Component> safePointer (this);

    focusLost (cause);

    if (safePointer != nullptr)
    {
        if (auto* handler = getAccessibilityHandler())
            handler->giveAwayFocus();

        internalChildKeyboardFocusChange (cause, safePointer);
    }
}

void Component::internalChildKeyboardFocusChange (FocusChangeType cause,
                                                  const WeakReference<Component>& safePointer)
{
    const bool childIsNowKeyboardFocused = hasKeyboardFocus (true);

    if (flags.childKeyboardFocusedFlag != childIsNowKeyboardFocused)
    {
        flags.childKeyboardFocusedFlag = childIsNowKeyboardFocused;

        focusOfChildComponentChanged (cause);

        if (safePointer == nullptr)
            return;
    }

    if (parentComponent != nullptr)
        parentComponent->internalChildKeyboardFocusChange (cause, parentComponent);
}

void Component::setWantsKeyboardFocus (bool wantsFocus) noexcept
{
    flags.wantsKeyboardFocusFlag = wantsFocus;
}

void Component::setMouseClickGrabsKeyboardFocus (bool shouldGrabFocus)
{
    flags.dontFocusOnMouseClickFlag = ! shouldGrabFocus;
}

bool Component::getMouseClickGrabsKeyboardFocus() const noexcept
{
    return ! flags.dontFocusOnMouseClickFlag;
}

bool Component::getWantsKeyboardFocus() const noexcept
{
    return flags.wantsKeyboardFocusFlag && ! flags.isDisabledFlag;
}

void Component::setFocusContainerType (FocusContainerType containerType) noexcept
{
    flags.isFocusContainerFlag = (containerType == FocusContainerType::focusContainer
                                  || containerType == FocusContainerType::keyboardFocusContainer);

    flags.isKeyboardFocusContainerFlag = (containerType == FocusContainerType::keyboardFocusContainer);
}

bool Component::isFocusContainer() const noexcept
{
    return flags.isFocusContainerFlag;
}

bool Component::isKeyboardFocusContainer() const noexcept
{
    return flags.isKeyboardFocusContainerFlag;
}

template <typename FocusContainerFn>
static Component* findContainer (const Component* child, FocusContainerFn isFocusContainer)
{
    if (auto* parent = child->getParentComponent())
    {
        if ((parent->*isFocusContainer)() || parent->getParentComponent() == nullptr)
            return parent;

        return findContainer (parent, isFocusContainer);
    }

    return nullptr;
}

Component* Component::findFocusContainer() const
{
    return findContainer (this, &Component::isFocusContainer);
}

Component* Component::findKeyboardFocusContainer() const
{
    return findContainer (this, &Component::isKeyboardFocusContainer);
}

static const Identifier explicitFocusOrderId ("_jexfo");

int Component::getExplicitFocusOrder() const
{
    return properties [explicitFocusOrderId];
}

void Component::setExplicitFocusOrder (int newFocusOrderIndex)
{
    properties.set (explicitFocusOrderId, newFocusOrderIndex);
}

std::unique_ptr<ComponentTraverser> Component::createFocusTraverser()
{
    if (flags.isFocusContainerFlag || parentComponent == nullptr)
        return std::make_unique<FocusTraverser>();

    return parentComponent->createFocusTraverser();
}

std::unique_ptr<ComponentTraverser> Component::createKeyboardFocusTraverser()
{
    if (flags.isKeyboardFocusContainerFlag || parentComponent == nullptr)
        return std::make_unique<KeyboardFocusTraverser>();

    return parentComponent->createKeyboardFocusTraverser();
}

void Component::takeKeyboardFocus (FocusChangeType cause, FocusChangeDirection direction)
{
    if (currentlyFocusedComponent == this)
        return;

    if (auto* peer = getPeer())
    {
        const WeakReference<Component> safePointer (this);
        peer->grabFocus();

        if (! peer->isFocused() || currentlyFocusedComponent == this)
            return;

        WeakReference<Component> componentLosingFocus (currentlyFocusedComponent);

        if (auto* losingFocus = componentLosingFocus.get())
            if (auto* otherPeer = losingFocus->getPeer())
                otherPeer->closeInputMethodContext();

        currentlyFocusedComponent = this;

        Desktop::getInstance().triggerFocusCallback();

        // call this after setting currentlyFocusedComponent so that the one that's
        // losing it has a chance to see where focus is going
        if (componentLosingFocus != nullptr)
            componentLosingFocus->internalKeyboardFocusLoss (cause);

        if (currentlyFocusedComponent == this)
            internalKeyboardFocusGain (cause, safePointer, direction);
    }
}

void Component::grabKeyboardFocusInternal (FocusChangeType cause, bool canTryParent, FocusChangeDirection direction)
{
    if (flags.dontFocusOnMouseClickFlag && cause == FocusChangeType::focusChangedByMouseClick)
        return;

    if (! isShowing())
        return;

    if (flags.wantsKeyboardFocusFlag
        && (isEnabled() || parentComponent == nullptr))
    {
        takeKeyboardFocus (cause, direction);
        return;
    }

    if (isParentOf (currentlyFocusedComponent) && currentlyFocusedComponent->isShowing())
        return;

    if (auto traverser = createKeyboardFocusTraverser())
    {
        if (auto* defaultComp = traverser->getDefaultComponent (this))
        {
            defaultComp->grabKeyboardFocusInternal (cause, false, direction);
            return;
        }
    }

    // if no children want it and we're allowed to try our parent comp,
    // then pass up to parent, which will try our siblings.
    if (canTryParent && parentComponent != nullptr)
        parentComponent->grabKeyboardFocusInternal (cause, true, direction);
}

void Component::grabKeyboardFocus()
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    grabKeyboardFocusInternal (focusChangedDirectly, true, FocusChangeDirection::unknown);

    // A component can only be focused when it's actually on the screen!
    // If this fails then you're probably trying to grab the focus before you've
    // added the component to a parent or made it visible. Or maybe one of its parent
    // components isn't yet visible.
    jassert (isShowing() || isOnDesktop());
}

void Component::giveAwayKeyboardFocusInternal (bool sendFocusLossEvent)
{
    if (hasKeyboardFocus (true))
    {
        if (auto* componentLosingFocus = currentlyFocusedComponent)
        {
            if (auto* otherPeer = componentLosingFocus->getPeer())
                otherPeer->closeInputMethodContext();

            currentlyFocusedComponent = nullptr;

            if (sendFocusLossEvent && componentLosingFocus != nullptr)
                componentLosingFocus->internalKeyboardFocusLoss (focusChangedDirectly);

            Desktop::getInstance().triggerFocusCallback();
        }
    }
}

void Component::giveAwayKeyboardFocus()
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    giveAwayKeyboardFocusInternal (true);
}

void Component::moveKeyboardFocusToSibling (bool moveToNext)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (parentComponent != nullptr)
    {
        if (auto traverser = createKeyboardFocusTraverser())
        {
            auto findComponentToFocus = [&]() -> Component*
            {
                if (auto* comp = (moveToNext ? traverser->getNextComponent (this)
                                             : traverser->getPreviousComponent (this)))
                    return comp;

                if (auto* focusContainer = findKeyboardFocusContainer())
                {
                    auto allFocusableComponents = traverser->getAllComponents (focusContainer);

                    if (! allFocusableComponents.empty())
                        return moveToNext ? allFocusableComponents.front()
                                          : allFocusableComponents.back();
                }

                return nullptr;
            };

            if (auto* nextComp = findComponentToFocus())
            {
                if (nextComp->isCurrentlyBlockedByAnotherModalComponent())
                {
                    const WeakReference<Component> nextCompPointer (nextComp);
                    internalModalInputAttempt();

                    if (nextCompPointer == nullptr || nextComp->isCurrentlyBlockedByAnotherModalComponent())
                        return;
                }

                nextComp->grabKeyboardFocusInternal (focusChangedByTabKey,
                                                     true,
                                                     moveToNext ? FocusChangeDirection::forward
                                                                : FocusChangeDirection::backward);
                return;
            }
        }

        parentComponent->moveKeyboardFocusToSibling (moveToNext);
    }
}

bool Component::hasKeyboardFocus (bool trueIfChildIsFocused) const
{
    return (currentlyFocusedComponent == this)
            || (trueIfChildIsFocused && isParentOf (currentlyFocusedComponent));
}

Component* JUCE_CALLTYPE Component::getCurrentlyFocusedComponent() noexcept
{
    return currentlyFocusedComponent;
}

void JUCE_CALLTYPE Component::unfocusAllComponents()
{
    if (currentlyFocusedComponent != nullptr)
        currentlyFocusedComponent->giveAwayKeyboardFocus();
}

//==============================================================================
bool Component::isEnabled() const noexcept
{
    return (! flags.isDisabledFlag)
            && (parentComponent == nullptr || parentComponent->isEnabled());
}

void Component::setEnabled (bool shouldBeEnabled)
{
    if (flags.isDisabledFlag == shouldBeEnabled)
    {
        flags.isDisabledFlag = ! shouldBeEnabled;

        // if any parent components are disabled, setting our flag won't make a difference,
        // so no need to send a change message
        if (parentComponent == nullptr || parentComponent->isEnabled())
            sendEnablementChangeMessage();

        BailOutChecker checker (this);
        componentListeners.callChecked (checker, [this] (ComponentListener& l) { l.componentEnablementChanged (*this); });

        if (! shouldBeEnabled && hasKeyboardFocus (true))
        {
            if (parentComponent != nullptr)
                parentComponent->grabKeyboardFocus();

            // ensure that keyboard focus is given away if it wasn't taken by parent
            giveAwayKeyboardFocus();
        }
    }
}

void Component::enablementChanged() {}

void Component::sendEnablementChangeMessage()
{
    const WeakReference<Component> safePointer (this);

    enablementChanged();

    if (safePointer == nullptr)
        return;

    for (int i = getNumChildComponents(); --i >= 0;)
    {
        if (auto* c = getChildComponent (i))
        {
            c->sendEnablementChangeMessage();

            if (safePointer == nullptr)
                return;
        }
    }
}

//==============================================================================
bool Component::isMouseOver (bool includeChildren) const
{
    if (! MessageManager::getInstance()->isThisTheMessageThread())
        return flags.cachedMouseInsideComponent;

    for (auto& ms : Desktop::getInstance().getMouseSources())
    {
        auto* c = ms.getComponentUnderMouse();

        if (c != nullptr && (c == this || (includeChildren && isParentOf (c))))
            if (ms.isDragging() || ! (ms.isTouch() || ms.isPen()))
                if (c->reallyContains (c->getLocalPoint (nullptr, ms.getScreenPosition()), false))
                    return true;
    }

    return false;
}

bool Component::isMouseButtonDown (bool includeChildren) const
{
    for (auto& ms : Desktop::getInstance().getMouseSources())
    {
        auto* c = ms.getComponentUnderMouse();

        if (c == this || (includeChildren && isParentOf (c)))
            if (ms.isDragging())
                return true;
    }

    return false;
}

bool Component::isMouseOverOrDragging (bool includeChildren) const
{
    for (auto& ms : Desktop::getInstance().getMouseSources())
    {
        auto* c = ms.getComponentUnderMouse();

        if (c == this || (includeChildren && isParentOf (c)))
            if (ms.isDragging() || ! ms.isTouch())
                return true;
    }

    return false;
}

bool JUCE_CALLTYPE Component::isMouseButtonDownAnywhere() noexcept
{
    return ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown();
}

Point<int> Component::getMouseXYRelative() const
{
    return getLocalPoint (nullptr, Desktop::getMousePositionFloat()).roundToInt();
}

//==============================================================================
void Component::addKeyListener (KeyListener* newListener)
{
    if (keyListeners == nullptr)
        keyListeners.reset (new Array<KeyListener*>());

    keyListeners->addIfNotAlreadyThere (newListener);
}

void Component::removeKeyListener (KeyListener* listenerToRemove)
{
    if (keyListeners != nullptr)
        keyListeners->removeFirstMatchingValue (listenerToRemove);
}

bool Component::keyPressed (const KeyPress&)            { return false; }
bool Component::keyStateChanged (bool /*isKeyDown*/)    { return false; }

void Component::modifierKeysChanged (const ModifierKeys& modifiers)
{
    if (parentComponent != nullptr)
        parentComponent->modifierKeysChanged (modifiers);
}

void Component::internalModifierKeysChanged()
{
    sendFakeMouseMove();
    modifierKeysChanged (ModifierKeys::currentModifiers);
}

//==============================================================================
Component::BailOutChecker::BailOutChecker (Component* component)
    : safePointer (component)
{
    jassert (component != nullptr);
}

bool Component::BailOutChecker::shouldBailOut() const noexcept
{
    return safePointer == nullptr;
}

//==============================================================================
void Component::setTitle (const String& newTitle)
{
    componentTitle = newTitle;
}

void Component::setDescription (const String& newDescription)
{
    componentDescription = newDescription;
}

void Component::setHelpText (const String& newHelpText)
{
    componentHelpText = newHelpText;
}

void Component::setAccessible (bool shouldBeAccessible)
{
    flags.accessibilityIgnoredFlag = ! shouldBeAccessible;

    if (flags.accessibilityIgnoredFlag)
        invalidateAccessibilityHandler();
}

bool Component::isAccessible() const noexcept
{
    return (! flags.accessibilityIgnoredFlag
            && (parentComponent == nullptr || parentComponent->isAccessible()));
}

std::unique_ptr<AccessibilityHandler> Component::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::unspecified);
}

std::unique_ptr<AccessibilityHandler> Component::createIgnoredAccessibilityHandler (Component& comp)
{
    return std::make_unique<AccessibilityHandler> (comp, AccessibilityRole::ignored);
}

void Component::invalidateAccessibilityHandler()
{
    accessibilityHandler = nullptr;
}

AccessibilityHandler* Component::getAccessibilityHandler()
{
    if (! isAccessible() || getWindowHandle() == nullptr)
        return nullptr;

    if (accessibilityHandler == nullptr
        || accessibilityHandler->getTypeIndex() != std::type_index (typeid (*this)))
    {
        accessibilityHandler = createAccessibilityHandler();

        // On Android, notifying that an element was created can cause the system to request
        // the accessibility node info for the new element. If we're not careful, this will lead
        // to recursive calls, as each time an element is created, new node info will be requested,
        // causing an element to be created, causing a new info request...
        // By assigning the accessibility handler before notifying the system that an element was
        // created, the if() predicate above should evaluate to false on recursive calls,
        // terminating the recursion.
        if (accessibilityHandler != nullptr)
            detail::AccessibilityHelpers::notifyAccessibilityEvent (*accessibilityHandler, detail::AccessibilityHelpers::Event::elementCreated);
        else
            jassertfalse; // createAccessibilityHandler must return non-null
    }

    return accessibilityHandler.get();
}

} // namespace juce
