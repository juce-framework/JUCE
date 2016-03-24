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

Component* Component::currentlyFocusedComponent = nullptr;


//==============================================================================
class Component::MouseListenerList
{
public:
    MouseListenerList() noexcept
        : numDeepMouseListeners (0)
    {
    }

    void addListener (MouseListener* const newListener, const bool wantsEventsForAllNestedChildComponents)
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

    void removeListener (MouseListener* const listenerToRemove)
    {
        const int index = listeners.indexOf (listenerToRemove);

        if (index >= 0)
        {
            if (index < numDeepMouseListeners)
                --numDeepMouseListeners;

            listeners.remove (index);
        }
    }

    static void sendMouseEvent (Component& comp, Component::BailOutChecker& checker,
                                void (MouseListener::*eventMethod) (const MouseEvent&), const MouseEvent& e)
    {
        if (checker.shouldBailOut())
            return;

        if (MouseListenerList* const list = comp.mouseListeners)
        {
            for (int i = list->listeners.size(); --i >= 0;)
            {
                (list->listeners.getUnchecked(i)->*eventMethod) (e);

                if (checker.shouldBailOut())
                    return;

                i = jmin (i, list->listeners.size());
            }
        }

        for (Component* p = comp.parentComponent; p != nullptr; p = p->parentComponent)
        {
            MouseListenerList* const list = p->mouseListeners;

            if (list != nullptr && list->numDeepMouseListeners > 0)
            {
                BailOutChecker2 checker2 (checker, p);

                for (int i = list->numDeepMouseListeners; --i >= 0;)
                {
                    (list->listeners.getUnchecked(i)->*eventMethod) (e);

                    if (checker2.shouldBailOut())
                        return;

                    i = jmin (i, list->numDeepMouseListeners);
                }
            }
        }
    }

    static void sendWheelEvent (Component& comp, Component::BailOutChecker& checker,
                                const MouseEvent& e, const MouseWheelDetails& wheel)
    {
        if (MouseListenerList* const list = comp.mouseListeners)
        {
            for (int i = list->listeners.size(); --i >= 0;)
            {
                list->listeners.getUnchecked(i)->mouseWheelMove (e, wheel);

                if (checker.shouldBailOut())
                    return;

                i = jmin (i, list->listeners.size());
            }
        }

        for (Component* p = comp.parentComponent; p != nullptr; p = p->parentComponent)
        {
            MouseListenerList* const list = p->mouseListeners;

            if (list != nullptr && list->numDeepMouseListeners > 0)
            {
                BailOutChecker2 checker2 (checker, p);

                for (int i = list->numDeepMouseListeners; --i >= 0;)
                {
                    list->listeners.getUnchecked(i)->mouseWheelMove (e, wheel);

                    if (checker2.shouldBailOut())
                        return;

                    i = jmin (i, list->numDeepMouseListeners);
                }
            }
        }
    }

private:
    Array<MouseListener*> listeners;
    int numDeepMouseListeners;

    class BailOutChecker2
    {
    public:
        BailOutChecker2 (Component::BailOutChecker& boc, Component* const comp)
            : checker (boc), safePointer (comp)
        {
        }

        bool shouldBailOut() const noexcept
        {
            return checker.shouldBailOut() || safePointer == nullptr;
        }

    private:
        Component::BailOutChecker& checker;
        const WeakReference<Component> safePointer;

        JUCE_DECLARE_NON_COPYABLE (BailOutChecker2)
    };

    JUCE_DECLARE_NON_COPYABLE (MouseListenerList)
};

//==============================================================================
struct FocusRestorer
{
    FocusRestorer()  : lastFocus (Component::getCurrentlyFocusedComponent()) {}

    ~FocusRestorer()
    {
        if (lastFocus != nullptr && ! lastFocus->isCurrentlyBlockedByAnotherModalComponent())
            lastFocus->grabKeyboardFocus();
    }

    WeakReference<Component> lastFocus;

    JUCE_DECLARE_NON_COPYABLE (FocusRestorer)
};

//==============================================================================
struct ScalingHelpers
{
    template <typename PointOrRect>
    static PointOrRect unscaledScreenPosToScaled (float scale, PointOrRect pos) noexcept
    {
        return scale != 1.0f ? pos / scale : pos;
    }

    template <typename PointOrRect>
    static PointOrRect scaledScreenPosToUnscaled (float scale, PointOrRect pos) noexcept
    {
        return scale != 1.0f ? pos * scale : pos;
    }

    // For these, we need to avoid getSmallestIntegerContainer being used, which causes
    // judder when moving windows
    static Rectangle<int> unscaledScreenPosToScaled (float scale, Rectangle<int> pos) noexcept
    {
        return scale != 1.0f ? Rectangle<int> (roundToInt (pos.getX() / scale),
                                               roundToInt (pos.getY() / scale),
                                               roundToInt (pos.getWidth() / scale),
                                               roundToInt (pos.getHeight() / scale)) : pos;
    }

    static Rectangle<int> scaledScreenPosToUnscaled (float scale, Rectangle<int> pos) noexcept
    {
        return scale != 1.0f ? Rectangle<int> (roundToInt (pos.getX() * scale),
                                               roundToInt (pos.getY() * scale),
                                               roundToInt (pos.getWidth() * scale),
                                               roundToInt (pos.getHeight() * scale)) : pos;
    }

    template <typename PointOrRect>
    static PointOrRect unscaledScreenPosToScaled (PointOrRect pos) noexcept
    {
        return unscaledScreenPosToScaled (Desktop::getInstance().getGlobalScaleFactor(), pos);
    }

    template <typename PointOrRect>
    static PointOrRect scaledScreenPosToUnscaled (PointOrRect pos) noexcept
    {
        return scaledScreenPosToUnscaled (Desktop::getInstance().getGlobalScaleFactor(), pos);
    }

    template <typename PointOrRect>
    static PointOrRect unscaledScreenPosToScaled (const Component& comp, PointOrRect pos) noexcept
    {
        return unscaledScreenPosToScaled (comp.getDesktopScaleFactor(), pos);
    }

    template <typename PointOrRect>
    static PointOrRect scaledScreenPosToUnscaled (const Component& comp, PointOrRect pos) noexcept
    {
        return scaledScreenPosToUnscaled (comp.getDesktopScaleFactor(), pos);
    }

    static Point<int>       addPosition      (Point<int> p,       const Component& c) noexcept  { return p + c.getPosition(); }
    static Rectangle<int>   addPosition      (Rectangle<int> p,   const Component& c) noexcept  { return p + c.getPosition(); }
    static Point<float>     addPosition      (Point<float> p,     const Component& c) noexcept  { return p + c.getPosition().toFloat(); }
    static Rectangle<float> addPosition      (Rectangle<float> p, const Component& c) noexcept  { return p + c.getPosition().toFloat(); }
    static Point<int>       subtractPosition (Point<int> p,       const Component& c) noexcept  { return p - c.getPosition(); }
    static Rectangle<int>   subtractPosition (Rectangle<int> p,   const Component& c) noexcept  { return p - c.getPosition(); }
    static Point<float>     subtractPosition (Point<float> p,     const Component& c) noexcept  { return p - c.getPosition().toFloat(); }
    static Rectangle<float> subtractPosition (Rectangle<float> p, const Component& c) noexcept  { return p - c.getPosition().toFloat(); }
};

//==============================================================================
struct Component::ComponentHelpers
{
   #if JUCE_MODAL_LOOPS_PERMITTED
    static void* runModalLoopCallback (void* userData)
    {
        return (void*) (pointer_sized_int) static_cast<Component*> (userData)->runModalLoop();
    }
   #endif

    static Identifier getColourPropertyId (int colourId)
    {
        char reversedHex[32];
        char* t = reversedHex;

        for (unsigned int v = (unsigned int) colourId;;)
        {
            *t++ = "0123456789abcdef" [(int) (v & 15)];
            v >>= 4;

            if (v == 0)
                break;
        }

        char destBuffer[32];
        char* dest = destBuffer;
        memcpy (dest, "jcclr_", 6);
        dest += 6;

        while (t > reversedHex)
            *dest++ = *--t;

        *dest++ = 0;
        return destBuffer;
    }

    //==============================================================================
    static inline bool hitTest (Component& comp, Point<int> localPoint)
    {
        return isPositiveAndBelow (localPoint.x, comp.getWidth())
            && isPositiveAndBelow (localPoint.y, comp.getHeight())
            && comp.hitTest (localPoint.x, localPoint.y);
    }

    // converts an unscaled position within a peer to the local position within that peer's component
    template <typename PointOrRect>
    static PointOrRect rawPeerPositionToLocal (const Component& comp, PointOrRect pos) noexcept
    {
        if (comp.isTransformed())
            pos = pos.transformedBy (comp.getTransform().inverted());

        return ScalingHelpers::unscaledScreenPosToScaled (comp, pos);
    }

    // converts a position within a peer's component to the unscaled position within the peer
    template <typename PointOrRect>
    static PointOrRect localPositionToRawPeerPos (const Component& comp, PointOrRect pos) noexcept
    {
        if (comp.isTransformed())
            pos = pos.transformedBy (comp.getTransform());

        return ScalingHelpers::scaledScreenPosToUnscaled (comp, pos);
    }

    template <typename PointOrRect>
    static PointOrRect convertFromParentSpace (const Component& comp, PointOrRect pointInParentSpace)
    {
        if (comp.affineTransform != nullptr)
            pointInParentSpace = pointInParentSpace.transformedBy (comp.affineTransform->inverted());

        if (comp.isOnDesktop())
        {
            if (ComponentPeer* peer = comp.getPeer())
                pointInParentSpace = ScalingHelpers::unscaledScreenPosToScaled
                                        (comp, peer->globalToLocal (ScalingHelpers::scaledScreenPosToUnscaled (pointInParentSpace)));
            else
                jassertfalse;
        }
        else
        {
            pointInParentSpace = ScalingHelpers::subtractPosition (pointInParentSpace, comp);
        }

        return pointInParentSpace;
    }

    template <typename PointOrRect>
    static PointOrRect convertToParentSpace (const Component& comp, PointOrRect pointInLocalSpace)
    {
        if (comp.isOnDesktop())
        {
            if (ComponentPeer* peer = comp.getPeer())
                pointInLocalSpace = ScalingHelpers::unscaledScreenPosToScaled
                                        (peer->localToGlobal (ScalingHelpers::scaledScreenPosToUnscaled (comp, pointInLocalSpace)));
            else
                jassertfalse;
        }
        else
        {
            pointInLocalSpace = ScalingHelpers::addPosition (pointInLocalSpace, comp);
        }

        if (comp.affineTransform != nullptr)
            pointInLocalSpace = pointInLocalSpace.transformedBy (*comp.affineTransform);

        return pointInLocalSpace;
    }

    template <typename PointOrRect>
    static PointOrRect convertFromDistantParentSpace (const Component* parent, const Component& target, const PointOrRect& coordInParent)
    {
        const Component* const directParent = target.getParentComponent();
        jassert (directParent != nullptr);

        if (directParent == parent)
            return convertFromParentSpace (target, coordInParent);

        return convertFromParentSpace (target, convertFromDistantParentSpace (parent, *directParent, coordInParent));
    }

    template <typename PointOrRect>
    static PointOrRect convertCoordinate (const Component* target, const Component* source, PointOrRect p)
    {
        while (source != nullptr)
        {
            if (source == target)
                return p;

            if (source->isParentOf (target))
                return convertFromDistantParentSpace (source, *target, p);

            p = convertToParentSpace (*source, p);
            source = source->getParentComponent();
        }

        jassert (source == nullptr);
        if (target == nullptr)
            return p;

        const Component* const topLevelComp = target->getTopLevelComponent();

        p = convertFromParentSpace (*topLevelComp, p);

        if (topLevelComp == target)
            return p;

        return convertFromDistantParentSpace (topLevelComp, *target, p);
    }

    static bool clipObscuredRegions (const Component& comp, Graphics& g, const Rectangle<int> clipRect, Point<int> delta)
    {
        bool wasClipped = false;

        for (int i = comp.childComponentList.size(); --i >= 0;)
        {
            const Component& child = *comp.childComponentList.getUnchecked(i);

            if (child.isVisible() && ! child.isTransformed())
            {
                const Rectangle<int> newClip (clipRect.getIntersection (child.bounds));

                if (! newClip.isEmpty())
                {
                    if (child.isOpaque() && child.componentTransparency == 0)
                    {
                        g.excludeClipRegion (newClip + delta);
                        wasClipped = true;
                    }
                    else
                    {
                        const Point<int> childPos (child.getPosition());
                        if (clipObscuredRegions (child, g, newClip - childPos, childPos + delta))
                            wasClipped = true;
                    }
                }
            }
        }

        return wasClipped;
    }

    static Rectangle<int> getParentOrMainMonitorBounds (const Component& comp)
    {
        if (Component* p = comp.getParentComponent())
            return p->getLocalBounds();

        return Desktop::getInstance().getDisplays().getMainDisplay().userArea;
    }

    static void releaseAllCachedImageResources (Component& c)
    {
        if (CachedComponentImage* cached = c.getCachedComponentImage())
            cached->releaseResources();

        for (int i = c.getNumChildComponents(); --i >= 0;)
            releaseAllCachedImageResources (*c.getChildComponent (i));
    }
};

//==============================================================================
Component::Component() noexcept
  : parentComponent (nullptr),
    lookAndFeel (nullptr),
    effect (nullptr),
    componentFlags (0),
    componentTransparency (0)
{
}

Component::Component (const String& name) noexcept
  : componentName (name),
    parentComponent (nullptr),
    lookAndFeel (nullptr),
    effect (nullptr),
    componentFlags (0),
    componentTransparency (0)
{
}

Component::~Component()
{
    static_jassert (sizeof (flags) <= sizeof (componentFlags));

    componentListeners.call (&ComponentListener::componentBeingDeleted, *this);

    masterReference.clear();

    while (childComponentList.size() > 0)
        removeChildComponent (childComponentList.size() - 1, false, true);

    if (parentComponent != nullptr)
        parentComponent->removeChildComponent (parentComponent->childComponentList.indexOf (this), true, false);
    else if (currentlyFocusedComponent == this || isParentOf (currentlyFocusedComponent))
        giveAwayFocus (currentlyFocusedComponent != this);

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
    ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    if (componentName != name)
    {
        componentName = name;

        if (flags.hasHeavyweightPeerFlag)
            if (ComponentPeer* const peer = getPeer())
                peer->setTitle (name);

        BailOutChecker checker (this);
        componentListeners.callChecked (checker, &ComponentListener::componentNameChanged, *this);
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
        ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

        const WeakReference<Component> safePointer (this);
        flags.visibleFlag = shouldBeVisible;

        if (shouldBeVisible)
            repaint();
        else
            repaintParent();

        sendFakeMouseMove();

        if (! shouldBeVisible)
        {
            ComponentHelpers::releaseAllCachedImageResources (*this);

            if (currentlyFocusedComponent == this || isParentOf (currentlyFocusedComponent))
            {
                if (parentComponent != nullptr)
                    parentComponent->grabKeyboardFocus();
                else
                    giveAwayFocus (true);
            }
        }

        if (safePointer != nullptr)
        {
            sendVisibilityChangeMessage();

            if (safePointer != nullptr && flags.hasHeavyweightPeerFlag)
            {
                if (ComponentPeer* const peer = getPeer())
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
        componentListeners.callChecked (checker, &ComponentListener::componentVisibilityChanged, *this);
}

bool Component::isShowing() const
{
    if (! flags.visibleFlag)
        return false;

    if (parentComponent != nullptr)
        return parentComponent->isShowing();

    if (const ComponentPeer* const peer = getPeer())
        return ! peer->isMinimised();

    return false;
}

//==============================================================================
void* Component::getWindowHandle() const
{
    if (const ComponentPeer* const peer = getPeer())
        return peer->getNativeHandle();

    return nullptr;
}

//==============================================================================
void Component::addToDesktop (int styleWanted, void* nativeWindowToAttachTo)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (isOpaque())
        styleWanted &= ~ComponentPeer::windowIsSemiTransparent;
    else
        styleWanted |= ComponentPeer::windowIsSemiTransparent;

    // don't use getPeer(), so that we only get the peer that's specifically
    // for this comp, and not for one of its parents.
    ComponentPeer* peer = ComponentPeer::getPeerFor (this);

    if (peer == nullptr || styleWanted != peer->getStyleFlags())
    {
        const WeakReference<Component> safePointer (this);

       #if JUCE_LINUX
        // it's wise to give the component a non-zero size before
        // putting it on the desktop, as X windows get confused by this, and
        // a (1, 1) minimum size is enforced here.
        setSize (jmax (1, getWidth()),
                 jmax (1, getHeight()));
       #endif

        const Point<int> topLeft (getScreenPosition());

        bool wasFullscreen = false;
        bool wasMinimised = false;
        ComponentBoundsConstrainer* currentConstrainer = nullptr;
        Rectangle<int> oldNonFullScreenBounds;
        int oldRenderingEngine = -1;

        if (peer != nullptr)
        {
            ScopedPointer<ComponentPeer> oldPeerToDelete (peer);

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

            bounds.setPosition (topLeft);
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
            internalHierarchyChanged();
        }
    }
}

void Component::removeFromDesktop()
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    if (flags.hasHeavyweightPeerFlag)
    {
        ComponentPeer* const peer = ComponentPeer::getPeerFor (this);
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
void Component::setOpaque (const bool shouldBeOpaque)
{
    if (shouldBeOpaque != flags.opaqueFlag)
    {
        flags.opaqueFlag = shouldBeOpaque;

        if (flags.hasHeavyweightPeerFlag)
            if (const ComponentPeer* const peer = ComponentPeer::getPeerFor (this))
                addToDesktop (peer->getStyleFlags());  // recreates the heavyweight window

        repaint();
    }
}

bool Component::isOpaque() const noexcept
{
    return flags.opaqueFlag;
}

//==============================================================================
class StandardCachedComponentImage  : public CachedComponentImage
{
public:
    StandardCachedComponentImage (Component& c) noexcept : owner (c), scale (1.0f) {}

    void paint (Graphics& g) override
    {
        scale = g.getInternalContext().getPhysicalPixelScaleFactor();
        const Rectangle<int> compBounds (owner.getLocalBounds());
        const Rectangle<int> imageBounds (compBounds * scale);

        if (image.isNull() || image.getBounds() != imageBounds)
        {
            image = Image (owner.isOpaque() ? Image::RGB
                                            : Image::ARGB,
                           jmax (1, imageBounds.getWidth()),
                           jmax (1, imageBounds.getHeight()),
                           ! owner.isOpaque());

            validArea.clear();
        }

        if (! validArea.containsRectangle (compBounds))
        {
            Graphics imG (image);
            LowLevelGraphicsContext& lg = imG.getInternalContext();

            lg.addTransform (AffineTransform::scale (scale));

            for (const Rectangle<int>* i = validArea.begin(), * const e = validArea.end(); i != e; ++i)
                lg.excludeClipRectangle (*i);

            if (! owner.isOpaque())
            {
                lg.setFill (Colours::transparentBlack);
                lg.fillRect (imageBounds, true);
                lg.setFill (Colours::black);
            }

            owner.paintEntireComponent (imG, true);
        }

        validArea = compBounds;

        g.setColour (Colours::black.withAlpha (owner.getAlpha()));
        g.drawImageTransformed (image, AffineTransform::scale (compBounds.getWidth()  / (float) imageBounds.getWidth(),
                                                               compBounds.getHeight() / (float) imageBounds.getHeight()), false);
    }

    bool invalidateAll() override                            { validArea.clear(); return true; }
    bool invalidate (const Rectangle<int>& area) override    { validArea.subtract (area); return true; }
    void releaseResources() override                         { image = Image(); }

private:
    Image image;
    RectangleList<int> validArea;
    Component& owner;
    float scale;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandardCachedComponentImage)
};

void Component::setCachedComponentImage (CachedComponentImage* newCachedImage)
{
    if (cachedImage != newCachedImage)
    {
        cachedImage = newCachedImage;
        repaint();
    }
}

void Component::setBufferedToImage (const bool shouldBeBuffered)
{
    // This assertion means that this component is already using a custom CachedComponentImage,
    // so by calling setBufferedToImage, you'll be deleting the custom one - this is almost certainly
    // not what you wanted to happen... If you really do know what you're doing here, and want to
    // avoid this assertion, just call setCachedComponentImage (nullptr) before setBufferedToImage().
    jassert (cachedImage == nullptr || dynamic_cast<StandardCachedComponentImage*> (cachedImage.get()) != nullptr);

    if (shouldBeBuffered)
    {
        if (cachedImage == nullptr)
            cachedImage = new StandardCachedComponentImage (*this);
    }
    else
    {
        cachedImage = nullptr;
    }
}

//==============================================================================
void Component::reorderChildInternal (const int sourceIndex, const int destIndex)
{
    if (sourceIndex != destIndex)
    {
        Component* const c = childComponentList.getUnchecked (sourceIndex);
        jassert (c != nullptr);
        c->repaintParent();

        childComponentList.move (sourceIndex, destIndex);

        sendFakeMouseMove();
        internalChildrenChanged();
    }
}

void Component::toFront (const bool setAsForeground)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    if (flags.hasHeavyweightPeerFlag)
    {
        if (ComponentPeer* const peer = getPeer())
        {
            peer->toFront (setAsForeground);

            if (setAsForeground && ! hasKeyboardFocus (true))
                grabKeyboardFocus();
        }
    }
    else if (parentComponent != nullptr)
    {
        const Array<Component*>& childList = parentComponent->childComponentList;

        if (childList.getLast() != this)
        {
            const int index = childList.indexOf (this);

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

        if (setAsForeground)
        {
            internalBroughtToFront();
            grabKeyboardFocus();
        }
    }
}

void Component::toBehind (Component* const other)
{
    if (other != nullptr && other != this)
    {
        // the two components must belong to the same parent..
        jassert (parentComponent == other->parentComponent);

        if (parentComponent != nullptr)
        {
            const Array<Component*>& childList = parentComponent->childComponentList;
            const int index = childList.indexOf (this);

            if (index >= 0 && childList [index + 1] != other)
            {
                int otherIndex = childList.indexOf (other);

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
                ComponentPeer* const us = getPeer();
                ComponentPeer* const them = other->getPeer();

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
        const Array<Component*>& childList = parentComponent->childComponentList;

        if (childList.getFirst() != this)
        {
            const int index = childList.indexOf (this);

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

void Component::setAlwaysOnTop (const bool shouldStayOnTop)
{
    if (shouldStayOnTop != flags.alwaysOnTopFlag)
    {
        BailOutChecker checker (this);

        flags.alwaysOnTopFlag = shouldStayOnTop;

        if (isOnDesktop())
        {
            if (ComponentPeer* const peer = getPeer())
            {
                if (! peer->setAlwaysOnTop (shouldStayOnTop))
                {
                    // some kinds of peer can't change their always-on-top status, so
                    // for these, we'll need to create a new window
                    const int oldFlags = peer->getStyleFlags();
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
int Component::proportionOfWidth  (const float proportion) const noexcept   { return roundToInt (proportion * bounds.getWidth()); }
int Component::proportionOfHeight (const float proportion) const noexcept   { return roundToInt (proportion * bounds.getHeight()); }

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

int Component::getScreenX() const   { return getScreenPosition().x; }
int Component::getScreenY() const   { return getScreenPosition().y; }

Point<int> Component::getScreenPosition() const       { return localPointToGlobal (Point<int>()); }
Rectangle<int> Component::getScreenBounds() const     { return localAreaToGlobal (getLocalBounds()); }

Rectangle<int> Component::getParentMonitorArea() const
{
    return Desktop::getInstance().getDisplays().getDisplayContaining (getScreenBounds().getCentre()).userArea;
}

Point<int> Component::getLocalPoint (const Component* source, Point<int> point) const
{
    return ComponentHelpers::convertCoordinate (this, source, point);
}

Point<float> Component::getLocalPoint (const Component* source, Point<float> point) const
{
    return ComponentHelpers::convertCoordinate (this, source, point);
}

Rectangle<int> Component::getLocalArea (const Component* source, const Rectangle<int>& area) const
{
    return ComponentHelpers::convertCoordinate (this, source, area);
}

Point<int> Component::localPointToGlobal (Point<int> point) const
{
    return ComponentHelpers::convertCoordinate (nullptr, this, point);
}

Point<float> Component::localPointToGlobal (Point<float> point) const
{
    return ComponentHelpers::convertCoordinate (nullptr, this, point);
}

Rectangle<int> Component::localAreaToGlobal (const Rectangle<int>& area) const
{
    return ComponentHelpers::convertCoordinate (nullptr, this, area);
}

// Deprecated methods...
Point<int> Component::relativePositionToGlobal (Point<int> relativePosition) const
{
    return localPointToGlobal (relativePosition);
}

Point<int> Component::globalPositionToRelative (Point<int> screenPosition) const
{
    return getLocalPoint (nullptr, screenPosition);
}

Point<int> Component::relativePositionToOtherComponent (const Component* const targetComponent, Point<int> positionRelativeToThis) const
{
    return targetComponent == nullptr ? localPointToGlobal (positionRelativeToThis)
                                      : targetComponent->getLocalPoint (this, positionRelativeToThis);
}


//==============================================================================
void Component::setBounds (const int x, const int y, int w, int h)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

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

        bounds.setBounds (x, y, w, h);

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
            if (ComponentPeer* const peer = getPeer())
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

void Component::sendMovedResizedMessages (const bool wasMoved, const bool wasResized)
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
            childComponentList.getUnchecked(i)->parentSizeChanged();

            if (checker.shouldBailOut())
                return;

            i = jmin (i, childComponentList.size());
        }
    }

    if (parentComponent != nullptr)
        parentComponent->childBoundsChanged (this);

    if (! checker.shouldBailOut())
        componentListeners.callChecked (checker, &ComponentListener::componentMovedOrResized,
                                        *this, wasMoved, wasResized);
}

void Component::setSize (const int w, const int h)
{
    setBounds (getX(), getY(), w, h);
}

void Component::setTopLeftPosition (const int x, const int y)
{
    setBounds (x, y, getWidth(), getHeight());
}

void Component::setTopLeftPosition (Point<int> pos)
{
    setBounds (pos.x, pos.y, getWidth(), getHeight());
}

void Component::setTopRightPosition (const int x, const int y)
{
    setTopLeftPosition (x - getWidth(), y);
}

void Component::setBounds (const Rectangle<int>& r)
{
    setBounds (r.getX(), r.getY(), r.getWidth(), r.getHeight());
}

void Component::setBounds (const RelativeRectangle& newBounds)
{
    newBounds.applyToComponent (*this);
}

void Component::setBounds (const String& newBoundsExpression)
{
    setBounds (RelativeRectangle (newBoundsExpression));
}

void Component::setBoundsRelative (const float x, const float y,
                                   const float w, const float h)
{
    const int pw = getParentWidth();
    const int ph = getParentHeight();

    setBounds (roundToInt (x * pw),
               roundToInt (y * ph),
               roundToInt (w * pw),
               roundToInt (h * ph));
}

void Component::setCentrePosition (const int x, const int y)
{
    setTopLeftPosition (x - getWidth() / 2,
                        y - getHeight() / 2);
}

void Component::setCentreRelative (const float x, const float y)
{
    setCentrePosition (roundToInt (getParentWidth() * x),
                       roundToInt (getParentHeight() * y));
}

void Component::centreWithSize (const int width, const int height)
{
    const Rectangle<int> parentArea (ComponentHelpers::getParentOrMainMonitorBounds (*this));

    setBounds (parentArea.getCentreX() - width / 2,
               parentArea.getCentreY() - height / 2,
               width, height);
}

void Component::setBoundsInset (const BorderSize<int>& borders)
{
    setBounds (borders.subtractedFrom (ComponentHelpers::getParentOrMainMonitorBounds (*this)));
}

void Component::setBoundsToFit (int x, int y, int width, int height,
                                Justification justification,
                                const bool onlyReduceInSize)
{
    // it's no good calling this method unless both the component and
    // target rectangle have a finite size.
    jassert (getWidth() > 0 && getHeight() > 0 && width > 0 && height > 0);

    if (getWidth() > 0 && getHeight() > 0
         && width > 0 && height > 0)
    {
        int newW, newH;

        if (onlyReduceInSize && getWidth() <= width && getHeight() <= height)
        {
            newW = getWidth();
            newH = getHeight();
        }
        else
        {
            const double imageRatio = getHeight() / (double) getWidth();
            const double targetRatio = height / (double) width;

            if (imageRatio <= targetRatio)
            {
                newW = width;
                newH = jmin (height, roundToInt (newW * imageRatio));
            }
            else
            {
                newH = height;
                newW = jmin (width, roundToInt (newH / imageRatio));
            }
        }

        if (newW > 0 && newH > 0)
            setBounds (justification.appliedToRectangle (Rectangle<int> (newW, newH),
                                                         Rectangle<int> (x, y, width, height)));
    }
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
            affineTransform = nullptr;
            repaint();

            sendMovedResizedMessages (false, false);
        }
    }
    else if (affineTransform == nullptr)
    {
        repaint();
        affineTransform = new AffineTransform (newTransform);
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

//==============================================================================
bool Component::hitTest (int x, int y)
{
    if (! flags.ignoresMouseClicksFlag)
        return true;

    if (flags.allowChildMouseClicksFlag)
    {
        for (int i = childComponentList.size(); --i >= 0;)
        {
            Component& child = *childComponentList.getUnchecked (i);

            if (child.isVisible()
                 && ComponentHelpers::hitTest (child, ComponentHelpers::convertFromParentSpace (child, Point<int> (x, y))))
                return true;
        }
    }

    return false;
}

void Component::setInterceptsMouseClicks (const bool allowClicks,
                                          const bool allowClicksOnChildComponents) noexcept
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
    if (ComponentHelpers::hitTest (*this, point))
    {
        if (parentComponent != nullptr)
            return parentComponent->contains (ComponentHelpers::convertToParentSpace (*this, point));

        if (flags.hasHeavyweightPeerFlag)
            if (const ComponentPeer* const peer = getPeer())
                return peer->contains (ComponentHelpers::localPositionToRawPeerPos (*this, point), true);
    }

    return false;
}

bool Component::reallyContains (Point<int> point, const bool returnTrueIfWithinAChild)
{
    if (! contains (point))
        return false;

    Component* const top = getTopLevelComponent();
    const Component* const compAtPosition = top->getComponentAt (top->getLocalPoint (this, point));

    return (compAtPosition == this) || (returnTrueIfWithinAChild && isParentOf (compAtPosition));
}

Component* Component::getComponentAt (Point<int> position)
{
    if (flags.visibleFlag && ComponentHelpers::hitTest (*this, position))
    {
        for (int i = childComponentList.size(); --i >= 0;)
        {
            Component* child = childComponentList.getUnchecked(i);
            child = child->getComponentAt (ComponentHelpers::convertFromParentSpace (*child, position));

            if (child != nullptr)
                return child;
        }

        return this;
    }

    return nullptr;
}

Component* Component::getComponentAt (const int x, const int y)
{
    return getComponentAt (Point<int> (x, y));
}

//==============================================================================
void Component::addChildComponent (Component& child, int zOrder)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

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

void Component::addChildComponent (Component* const child, int zOrder)
{
    if (child != nullptr)
        addChildComponent (*child, zOrder);
}

void Component::addAndMakeVisible (Component* const child, int zOrder)
{
    if (child != nullptr)
        addAndMakeVisible (*child, zOrder);
}

void Component::addChildAndSetID (Component* const child, const String& childID)
{
    if (child != nullptr)
    {
        child->setComponentID (childID);
        addAndMakeVisible (child);
    }
}

void Component::removeChildComponent (Component* const child)
{
    removeChildComponent (childComponentList.indexOf (child), true, true);
}

Component* Component::removeChildComponent (const int index)
{
    return removeChildComponent (index, true, true);
}

Component* Component::removeChildComponent (const int index, bool sendParentEvents, const bool sendChildEvents)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    Component* const child = childComponentList [index];

    if (child != nullptr)
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

        ComponentHelpers::releaseAllCachedImageResources (*child);

        // (NB: there are obscure situations where child->isShowing() = false, but it still has the focus)
        if (currentlyFocusedComponent == child || child->isParentOf (currentlyFocusedComponent))
        {
            if (sendParentEvents)
            {
                const WeakReference<Component> thisPointer (this);

                giveAwayFocus (sendChildEvents || currentlyFocusedComponent != child);

                if (thisPointer == nullptr)
                    return child;

                grabKeyboardFocus();
            }
            else
            {
                giveAwayFocus (sendChildEvents || currentlyFocusedComponent != child);
            }
        }

        if (sendChildEvents)
            child->internalHierarchyChanged();

        if (sendParentEvents)
            internalChildrenChanged();
    }

    return child;
}

//==============================================================================
void Component::removeAllChildren()
{
    while (childComponentList.size() > 0)
        removeChildComponent (childComponentList.size() - 1);
}

void Component::deleteAllChildren()
{
    while (childComponentList.size() > 0)
        delete (removeChildComponent (childComponentList.size() - 1));
}

int Component::getNumChildComponents() const noexcept
{
    return childComponentList.size();
}

Component* Component::getChildComponent (const int index) const noexcept
{
    return childComponentList [index];
}

int Component::getIndexOfChildComponent (const Component* const child) const noexcept
{
    return childComponentList.indexOf (const_cast<Component*> (child));
}

Component* Component::findChildWithID (StringRef targetID) const noexcept
{
    for (int i = childComponentList.size(); --i >= 0;)
    {
        Component* const c = childComponentList.getUnchecked(i);
        if (c->componentID == targetID)
            return c;
    }

    return nullptr;
}

Component* Component::getTopLevelComponent() const noexcept
{
    const Component* comp = this;

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
            componentListeners.callChecked (checker, &ComponentListener::componentChildrenChanged, *this);
    }
}

void Component::internalHierarchyChanged()
{
    BailOutChecker checker (this);

    parentHierarchyChanged();

    if (checker.shouldBailOut())
        return;

    componentListeners.callChecked (checker, &ComponentListener::componentParentHierarchyChanged, *this);

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
}

//==============================================================================
#if JUCE_MODAL_LOOPS_PERMITTED
int Component::runModalLoop()
{
    if (! MessageManager::getInstance()->isThisTheMessageThread())
    {
        // use a callback so this can be called from non-gui threads
        return (int) (pointer_sized_int) MessageManager::getInstance()
                                           ->callFunctionOnMessageThread (&ComponentHelpers::runModalLoopCallback, this);
    }

    if (! isCurrentlyModal())
        enterModalState (true);

    return ModalComponentManager::getInstance()->runEventLoopForCurrentComponent();
}
#endif

//==============================================================================
void Component::enterModalState (const bool shouldTakeKeyboardFocus,
                                 ModalComponentManager::Callback* callback,
                                 const bool deleteWhenDismissed)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (! isCurrentlyModal())
    {
        ModalComponentManager& mcm = *ModalComponentManager::getInstance();
        mcm.startModal (this, deleteWhenDismissed);
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

void Component::exitModalState (const int returnValue)
{
    if (isCurrentlyModal())
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            ModalComponentManager& mcm = *ModalComponentManager::getInstance();
            mcm.endModal (this, returnValue);
            mcm.bringModalComponentsToFront();
        }
        else
        {
            struct ExitModalStateMessage   : public CallbackMessage
            {
                ExitModalStateMessage (Component* c, int res)  : target (c), result (res)  {}

                void messageCallback() override
                {
                    if (Component* c = target)
                        c->exitModalState (result);
                }

                WeakReference<Component> target;
                int result;
            };

            (new ExitModalStateMessage (this, returnValue))->post();
        }
    }
}

bool Component::isCurrentlyModal() const noexcept
{
    return getCurrentlyModalComponent() == this;
}

bool Component::isCurrentlyBlockedByAnotherModalComponent() const
{
    Component* const mc = getCurrentlyModalComponent();

    return ! (mc == nullptr || mc == this || mc->isParentOf (this)
               || mc->canModalEventBeSentToComponent (this));
}

int JUCE_CALLTYPE Component::getNumCurrentlyModalComponents() noexcept
{
    return ModalComponentManager::getInstance()->getNumModalComponents();
}

Component* JUCE_CALLTYPE Component::getCurrentlyModalComponent (int index) noexcept
{
    return ModalComponentManager::getInstance()->getModalComponent (index);
}

//==============================================================================
void Component::setBroughtToFrontOnMouseClick (const bool shouldBeBroughtToFront) noexcept
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
void Component::setRepaintsOnMouseActivity (const bool shouldRepaint) noexcept
{
    flags.repaintOnMouseActivityFlag = shouldRepaint;
}

//==============================================================================
float Component::getAlpha() const noexcept
{
    return (255 - componentTransparency) / 255.0f;
}

void Component::setAlpha (const float newAlpha)
{
    const uint8 newIntAlpha = (uint8) (255 - jlimit (0, 255, roundToInt (newAlpha * 255.0)));

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
        if (ComponentPeer* const peer = getPeer())
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

void Component::repaint (const int x, const int y, const int w, const int h)
{
    internalRepaint (Rectangle<int> (x, y, w, h));
}

void Component::repaint (const Rectangle<int>& area)
{
    internalRepaint (area);
}

void Component::repaintParent()
{
    if (parentComponent != nullptr)
        parentComponent->internalRepaint (ComponentHelpers::convertToParentSpace (*this, getLocalBounds()));
}

void Component::internalRepaint (Rectangle<int> area)
{
    area = area.getIntersection (getLocalBounds());

    if (! area.isEmpty())
        internalRepaintUnchecked (area, false);
}

void Component::internalRepaintUnchecked (Rectangle<int> area, const bool isEntireComponent)
{
    if (flags.visibleFlag)
    {
        if (cachedImage != nullptr)
            if (! (isEntireComponent ? cachedImage->invalidateAll()
                                     : cachedImage->invalidate (area)))
                return;

        if (flags.hasHeavyweightPeerFlag)
        {
            // if component methods are being called from threads other than the message
            // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
            ASSERT_MESSAGE_MANAGER_IS_LOCKED

            if (ComponentPeer* const peer = getPeer())
            {
                // Tweak the scaling so that the component's integer size exactly aligns with the peer's scaled size
                const Rectangle<int> peerBounds (peer->getBounds());
                const Rectangle<int> scaled (area * Point<float> (peerBounds.getWidth()  / (float) getWidth(),
                                                                  peerBounds.getHeight() / (float) getHeight()));

                peer->repaint (affineTransform != nullptr ? scaled.transformedBy (*affineTransform) : scaled);
            }
        }
        else
        {
            if (parentComponent != nullptr)
                parentComponent->internalRepaint (ComponentHelpers::convertToParentSpace (*this, area));
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
    const Rectangle<int> clipBounds (g.getClipBounds());

    if (flags.dontClipGraphicsFlag)
    {
        paint (g);
    }
    else
    {
        g.saveState();

        if (! (ComponentHelpers::clipObscuredRegions (*this, g, clipBounds, Point<int>()) && g.isClipEmpty()))
            paint (g);

        g.restoreState();
    }

    for (int i = 0; i < childComponentList.size(); ++i)
    {
        Component& child = *childComponentList.getUnchecked (i);

        if (child.isVisible())
        {
            if (child.affineTransform != nullptr)
            {
                g.saveState();
                g.addTransform (*child.affineTransform);

                if ((child.flags.dontClipGraphicsFlag && ! g.isClipEmpty()) || g.reduceClipRegion (child.getBounds()))
                    child.paintWithinParentContext (g);

                g.restoreState();
            }
            else if (clipBounds.intersects (child.getBounds()))
            {
                g.saveState();

                if (child.flags.dontClipGraphicsFlag)
                {
                    child.paintWithinParentContext (g);
                }
                else if (g.reduceClipRegion (child.getBounds()))
                {
                    bool nothingClipped = true;

                    for (int j = i + 1; j < childComponentList.size(); ++j)
                    {
                        const Component& sibling = *childComponentList.getUnchecked (j);

                        if (sibling.flags.opaqueFlag && sibling.isVisible() && sibling.affineTransform == nullptr)
                        {
                            nothingClipped = false;
                            g.excludeClipRegion (sibling.getBounds());
                        }
                    }

                    if (nothingClipped || ! g.isClipEmpty())
                        child.paintWithinParentContext (g);
                }

                g.restoreState();
            }
        }
    }

    g.saveState();
    paintOverChildren (g);
    g.restoreState();
}

void Component::paintEntireComponent (Graphics& g, const bool ignoreAlphaLevel)
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

    if (effect != nullptr)
    {
        const float scale = g.getInternalContext().getPhysicalPixelScaleFactor();

        const Rectangle<int> scaledBounds (getLocalBounds() * scale);

        Image effectImage (flags.opaqueFlag ? Image::RGB : Image::ARGB,
                           scaledBounds.getWidth(), scaledBounds.getHeight(), ! flags.opaqueFlag);
        {
            Graphics g2 (effectImage);
            g2.addTransform (AffineTransform::scale (scaledBounds.getWidth()  / (float) getWidth(),
                                                     scaledBounds.getHeight() / (float) getHeight()));
            paintComponentAndChildren (g2);
        }

        g.saveState();
        g.addTransform (AffineTransform::scale (1.0f / scale));
        effect->applyEffect (effectImage, g, scale, ignoreAlphaLevel ? 1.0f : getAlpha());
        g.restoreState();
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

void Component::setPaintingIsUnclipped (const bool shouldPaintWithoutClipping) noexcept
{
    flags.dontClipGraphicsFlag = shouldPaintWithoutClipping;
}

//==============================================================================
Image Component::createComponentSnapshot (const Rectangle<int>& areaToGrab,
                                          bool clipImageToComponentBounds, float scaleFactor)
{
    Rectangle<int> r (areaToGrab);

    if (clipImageToComponentBounds)
        r = r.getIntersection (getLocalBounds());

    if (r.isEmpty())
        return Image();

    const int w = roundToInt (scaleFactor * r.getWidth());
    const int h = roundToInt (scaleFactor * r.getHeight());

    Image image (flags.opaqueFlag ? Image::RGB : Image::ARGB, w, h, true);

    Graphics g (image);

    if (w != getWidth() || h != getHeight())
        g.addTransform (AffineTransform::scale (w / (float) r.getWidth(),
                                                h / (float) r.getHeight()));
    g.setOrigin (-r.getPosition());

    paintEntireComponent (g, true);

    return image;
}

void Component::setComponentEffect (ImageEffectFilter* const newEffect)
{
    if (effect != newEffect)
    {
        effect = newEffect;
        repaint();
    }
}

//==============================================================================
LookAndFeel& Component::getLookAndFeel() const noexcept
{
    for (const Component* c = this; c != nullptr; c = c->parentComponent)
        if (c->lookAndFeel != nullptr)
            return *(c->lookAndFeel);

    return LookAndFeel::getDefaultLookAndFeel();
}

void Component::setLookAndFeel (LookAndFeel* const newLookAndFeel)
{
    if (lookAndFeel != newLookAndFeel)
    {
        lookAndFeel = newLookAndFeel;
        sendLookAndFeelChange();
    }
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

Colour Component::findColour (const int colourId, const bool inheritFromParent) const
{
    if (const var* const v = properties.getVarPointer (ComponentHelpers::getColourPropertyId (colourId)))
        return Colour ((uint32) static_cast<int> (*v));

    if (inheritFromParent && parentComponent != nullptr
         && (lookAndFeel == nullptr || ! lookAndFeel->isColourSpecified (colourId)))
        return parentComponent->findColour (colourId, true);

    return getLookAndFeel().findColour (colourId);
}

bool Component::isColourSpecified (const int colourId) const
{
    return properties.contains (ComponentHelpers::getColourPropertyId (colourId));
}

void Component::removeColour (const int colourId)
{
    if (properties.remove (ComponentHelpers::getColourPropertyId (colourId)))
        colourChanged();
}

void Component::setColour (const int colourId, Colour colour)
{
    if (properties.set (ComponentHelpers::getColourPropertyId (colourId), (int) colour.getARGB()))
        colourChanged();
}

void Component::copyAllExplicitColoursTo (Component& target) const
{
    bool changed = false;

    for (int i = properties.size(); --i >= 0;)
    {
        const Identifier name (properties.getName(i));

        if (name.toString().startsWith ("jcclr_"))
            if (target.properties.set (name, properties [name]))
                changed = true;
    }

    if (changed)
        target.colourChanged();
}

//==============================================================================
MarkerList* Component::getMarkers (bool /*xAxis*/)
{
    return nullptr;
}

//==============================================================================
Component::Positioner::Positioner (Component& c) noexcept
    : component (c)
{
}

Component::Positioner* Component::getPositioner() const noexcept
{
    return positioner;
}

void Component::setPositioner (Positioner* newPositioner)
{
    // You can only assign a positioner to the component that it was created for!
    jassert (newPositioner == nullptr || this == &(newPositioner->getComponent()));
    positioner = newPositioner;
}

//==============================================================================
Rectangle<int> Component::getLocalBounds() const noexcept
{
    return bounds.withZeroOrigin();
}

Rectangle<int> Component::getBoundsInParent() const noexcept
{
    return affineTransform == nullptr ? bounds
                                      : bounds.transformedBy (*affineTransform);
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
    // the base class just passes this event up to its parent..
    if (parentComponent != nullptr)
        parentComponent->mouseWheelMove (e.getEventRelativeTo (parentComponent), wheel);
}

void Component::mouseMagnify (const MouseEvent& e, float magnifyAmount)
{
    // the base class just passes this event up to its parent..
    if (parentComponent != nullptr)
        parentComponent->mouseMagnify (e.getEventRelativeTo (parentComponent), magnifyAmount);
}

//==============================================================================
void Component::resized()                       {}
void Component::moved()                         {}
void Component::childBoundsChanged (Component*) {}
void Component::parentSizeChanged()             {}

void Component::addComponentListener (ComponentListener* const newListener)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    #if JUCE_DEBUG || JUCE_LOG_ASSERTIONS
    if (getParentComponent() != nullptr)
        ASSERT_MESSAGE_MANAGER_IS_LOCKED;
    #endif

    componentListeners.add (newListener);
}

void Component::removeComponentListener (ComponentListener* const listenerToRemove)
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
    if (Component* const current = getCurrentlyModalComponent())
        current->inputAttemptWhenModal();
}

//==============================================================================
void Component::postCommandMessage (const int commandId)
{
    struct CustomCommandMessage   : public CallbackMessage
    {
        CustomCommandMessage (Component* const c, const int command)
            : target (c), commandId (command) {}

        void messageCallback() override
        {
            if (Component* c = target.get())
                c->handleCommandMessage (commandId);
        }

    private:
        WeakReference<Component> target;
        int commandId;
    };

    (new CustomCommandMessage (this, commandId))->post();
}

void Component::handleCommandMessage (int)
{
    // used by subclasses
}

//==============================================================================
void Component::addMouseListener (MouseListener* const newListener,
                                  const bool wantsEventsForAllNestedChildComponents)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    ASSERT_MESSAGE_MANAGER_IS_LOCKED

    // If you register a component as a mouselistener for itself, it'll receive all the events
    // twice - once via the direct callback that all components get anyway, and then again as a listener!
    jassert ((newListener != this) || wantsEventsForAllNestedChildComponents);

    if (mouseListeners == nullptr)
        mouseListeners = new MouseListenerList();

    mouseListeners->addListener (newListener, wantsEventsForAllNestedChildComponents);
}

void Component::removeMouseListener (MouseListener* const listenerToRemove)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (mouseListeners != nullptr)
        mouseListeners->removeListener (listenerToRemove);
}

//==============================================================================
void Component::internalMouseEnter (MouseInputSource source, Point<float> relativePos, Time time)
{
    if (isCurrentlyBlockedByAnotherModalComponent())
    {
        // if something else is modal, always just show a normal mouse cursor
        source.showMouseCursor (MouseCursor::NormalCursor);
        return;
    }

    if (flags.repaintOnMouseActivityFlag)
        repaint();

    BailOutChecker checker (this);

    const MouseEvent me (source, relativePos, source.getCurrentModifiers(), MouseInputSource::invalidPressure,
                         this, this, time, relativePos, time, 0, false);
    mouseEnter (me);

    if (checker.shouldBailOut())
        return;

    Desktop::getInstance().getMouseListeners().callChecked (checker, &MouseListener::mouseEnter, me);

    MouseListenerList::sendMouseEvent (*this, checker, &MouseListener::mouseEnter, me);
}

void Component::internalMouseExit (MouseInputSource source, Point<float> relativePos, Time time)
{
    if (flags.repaintOnMouseActivityFlag)
        repaint();

    BailOutChecker checker (this);

    const MouseEvent me (source, relativePos, source.getCurrentModifiers(), MouseInputSource::invalidPressure,
                         this, this, time, relativePos, time, 0, false);

    mouseExit (me);

    if (checker.shouldBailOut())
        return;

    Desktop::getInstance().getMouseListeners().callChecked (checker, &MouseListener::mouseExit, me);

    MouseListenerList::sendMouseEvent (*this, checker, &MouseListener::mouseExit, me);
}

void Component::internalMouseDown (MouseInputSource source, Point<float> relativePos, Time time, float pressure)
{
    Desktop& desktop = Desktop::getInstance();
    BailOutChecker checker (this);

    if (isCurrentlyBlockedByAnotherModalComponent())
    {
        flags.mouseDownWasBlocked = true;
        internalModalInputAttempt();

        if (checker.shouldBailOut())
            return;

        // If processing the input attempt has exited the modal loop, we'll allow the event
        // to be delivered..
        if (isCurrentlyBlockedByAnotherModalComponent())
        {
            // allow blocked mouse-events to go to global listeners..
            const MouseEvent me (source, relativePos, source.getCurrentModifiers(),
                                 pressure, this, this, time, relativePos, time,
                                 source.getNumberOfMultipleClicks(), false);

            desktop.getMouseListeners().callChecked (checker, &MouseListener::mouseDown, me);
            return;
        }
    }

    flags.mouseDownWasBlocked = false;

    for (Component* c = this; c != nullptr; c = c->parentComponent)
    {
        if (c->isBroughtToFrontOnMouseClick())
        {
            c->toFront (true);

            if (checker.shouldBailOut())
                return;
        }
    }

    if (! flags.dontFocusOnMouseClickFlag)
    {
        grabFocusInternal (focusChangedByMouseClick, true);

        if (checker.shouldBailOut())
            return;
    }

    if (flags.repaintOnMouseActivityFlag)
        repaint();

    const MouseEvent me (source, relativePos, source.getCurrentModifiers(),
                         pressure, this, this, time, relativePos, time,
                         source.getNumberOfMultipleClicks(), false);
    mouseDown (me);

    if (checker.shouldBailOut())
        return;

    desktop.getMouseListeners().callChecked (checker, &MouseListener::mouseDown, me);

    MouseListenerList::sendMouseEvent (*this, checker, &MouseListener::mouseDown, me);
}

void Component::internalMouseUp (MouseInputSource source, Point<float> relativePos,
                                 Time time, const ModifierKeys oldModifiers)
{
    if (flags.mouseDownWasBlocked && isCurrentlyBlockedByAnotherModalComponent())
        return;

    BailOutChecker checker (this);

    if (flags.repaintOnMouseActivityFlag)
        repaint();

    const MouseEvent me (source, relativePos,
                         oldModifiers, MouseInputSource::invalidPressure, this, this, time,
                         getLocalPoint (nullptr, source.getLastMouseDownPosition()),
                         source.getLastMouseDownTime(),
                         source.getNumberOfMultipleClicks(),
                         source.hasMouseMovedSignificantlySincePressed());
    mouseUp (me);

    if (checker.shouldBailOut())
        return;

    Desktop& desktop = Desktop::getInstance();
    desktop.getMouseListeners().callChecked (checker, &MouseListener::mouseUp, me);

    MouseListenerList::sendMouseEvent (*this, checker, &MouseListener::mouseUp, me);

    if (checker.shouldBailOut())
        return;

    // check for double-click
    if (me.getNumberOfClicks() >= 2)
    {
        mouseDoubleClick (me);

        if (checker.shouldBailOut())
            return;

        desktop.mouseListeners.callChecked (checker, &MouseListener::mouseDoubleClick, me);
        MouseListenerList::sendMouseEvent (*this, checker, &MouseListener::mouseDoubleClick, me);
    }
}

void Component::internalMouseDrag (MouseInputSource source, Point<float> relativePos, Time time, float pressure)
{
    if (! isCurrentlyBlockedByAnotherModalComponent())
    {
        BailOutChecker checker (this);

        const MouseEvent me (source, relativePos, source.getCurrentModifiers(),
                             pressure, this, this, time,
                             getLocalPoint (nullptr, source.getLastMouseDownPosition()),
                             source.getLastMouseDownTime(),
                             source.getNumberOfMultipleClicks(),
                             source.hasMouseMovedSignificantlySincePressed());
        mouseDrag (me);

        if (checker.shouldBailOut())
            return;

        Desktop::getInstance().getMouseListeners().callChecked (checker, &MouseListener::mouseDrag, me);

        MouseListenerList::sendMouseEvent (*this, checker, &MouseListener::mouseDrag, me);
    }
}

void Component::internalMouseMove (MouseInputSource source, Point<float> relativePos, Time time)
{
    Desktop& desktop = Desktop::getInstance();

    if (isCurrentlyBlockedByAnotherModalComponent())
    {
        // allow blocked mouse-events to go to global listeners..
        desktop.sendMouseMove();
    }
    else
    {
        BailOutChecker checker (this);

        const MouseEvent me (source, relativePos, source.getCurrentModifiers(), MouseInputSource::invalidPressure,
                             this, this, time, relativePos, time, 0, false);
        mouseMove (me);

        if (checker.shouldBailOut())
            return;

        desktop.getMouseListeners().callChecked (checker, &MouseListener::mouseMove, me);

        MouseListenerList::sendMouseEvent (*this, checker, &MouseListener::mouseMove, me);
    }
}

void Component::internalMouseWheel (MouseInputSource source, Point<float> relativePos,
                                    Time time, const MouseWheelDetails& wheel)
{
    Desktop& desktop = Desktop::getInstance();
    BailOutChecker checker (this);

    const MouseEvent me (source, relativePos, source.getCurrentModifiers(), MouseInputSource::invalidPressure,
                         this, this, time, relativePos, time, 0, false);

    if (isCurrentlyBlockedByAnotherModalComponent())
    {
        // allow blocked mouse-events to go to global listeners..
        desktop.mouseListeners.callChecked (checker, &MouseListener::mouseWheelMove, me, wheel);
    }
    else
    {
        mouseWheelMove (me, wheel);

        if (checker.shouldBailOut())
            return;

        desktop.mouseListeners.callChecked (checker, &MouseListener::mouseWheelMove, me, wheel);

        if (! checker.shouldBailOut())
            MouseListenerList::sendWheelEvent (*this, checker, me, wheel);
    }
}

void Component::internalMagnifyGesture (MouseInputSource source, Point<float> relativePos,
                                        Time time, float amount)
{
    if (! isCurrentlyBlockedByAnotherModalComponent())
    {
        const MouseEvent me (source, relativePos, source.getCurrentModifiers(), MouseInputSource::invalidPressure,
                             this, this, time, relativePos, time, 0, false);

        mouseMagnify (me, amount);
    }
}

void Component::sendFakeMouseMove() const
{
    MouseInputSource mainMouse = Desktop::getInstance().getMainMouseSource();

    if (! mainMouse.isDragging())
        mainMouse.triggerFakeMove();
}

void JUCE_CALLTYPE Component::beginDragAutoRepeat (const int interval)
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

    componentListeners.callChecked (checker, &ComponentListener::componentBroughtToFront, *this);

    if (checker.shouldBailOut())
        return;

    // When brought to the front and there's a modal component blocking this one,
    // we need to bring the modal one to the front instead..
    if (Component* const cm = getCurrentlyModalComponent())
        if (cm->getTopLevelComponent() != getTopLevelComponent())
            ModalComponentManager::getInstance()->bringModalComponentsToFront (false); // very important that this is false, otherwise in Windows,
                                                                                       // non-front components can't get focus when another modal comp is
                                                                                       // active, and therefore can't receive mouse-clicks
}

//==============================================================================
void Component::focusGained (FocusChangeType)   {}
void Component::focusLost (FocusChangeType)     {}
void Component::focusOfChildComponentChanged (FocusChangeType) {}

void Component::internalFocusGain (const FocusChangeType cause)
{
    internalFocusGain (cause, WeakReference<Component> (this));
}

void Component::internalFocusGain (const FocusChangeType cause, const WeakReference<Component>& safePointer)
{
    focusGained (cause);

    if (safePointer != nullptr)
        internalChildFocusChange (cause, safePointer);
}

void Component::internalFocusLoss (const FocusChangeType cause)
{
    const WeakReference<Component> safePointer (this);

    focusLost (focusChangedDirectly);

    if (safePointer != nullptr)
        internalChildFocusChange (cause, safePointer);
}

void Component::internalChildFocusChange (FocusChangeType cause, const WeakReference<Component>& safePointer)
{
    const bool childIsNowFocused = hasKeyboardFocus (true);

    if (flags.childCompFocusedFlag != childIsNowFocused)
    {
        flags.childCompFocusedFlag = childIsNowFocused;

        focusOfChildComponentChanged (cause);

        if (safePointer == nullptr)
            return;
    }

    if (parentComponent != nullptr)
        parentComponent->internalChildFocusChange (cause, WeakReference<Component> (parentComponent));
}

void Component::setWantsKeyboardFocus (const bool wantsFocus) noexcept
{
    flags.wantsFocusFlag = wantsFocus;
}

void Component::setMouseClickGrabsKeyboardFocus (const bool shouldGrabFocus)
{
    flags.dontFocusOnMouseClickFlag = ! shouldGrabFocus;
}

bool Component::getMouseClickGrabsKeyboardFocus() const noexcept
{
    return ! flags.dontFocusOnMouseClickFlag;
}

bool Component::getWantsKeyboardFocus() const noexcept
{
    return flags.wantsFocusFlag && ! flags.isDisabledFlag;
}

void Component::setFocusContainer (const bool shouldBeFocusContainer) noexcept
{
    flags.isFocusContainerFlag = shouldBeFocusContainer;
}

bool Component::isFocusContainer() const noexcept
{
    return flags.isFocusContainerFlag;
}

static const Identifier juce_explicitFocusOrderId ("_jexfo");

int Component::getExplicitFocusOrder() const
{
    return properties [juce_explicitFocusOrderId];
}

void Component::setExplicitFocusOrder (const int newFocusOrderIndex)
{
    properties.set (juce_explicitFocusOrderId, newFocusOrderIndex);
}

KeyboardFocusTraverser* Component::createFocusTraverser()
{
    if (flags.isFocusContainerFlag || parentComponent == nullptr)
        return new KeyboardFocusTraverser();

    return parentComponent->createFocusTraverser();
}

void Component::takeKeyboardFocus (const FocusChangeType cause)
{
    // give the focus to this component
    if (currentlyFocusedComponent != this)
    {
        // get the focus onto our desktop window
        if (ComponentPeer* const peer = getPeer())
        {
            const WeakReference<Component> safePointer (this);
            peer->grabFocus();

            if (peer->isFocused() && currentlyFocusedComponent != this)
            {
                WeakReference<Component> componentLosingFocus (currentlyFocusedComponent);
                currentlyFocusedComponent = this;

                Desktop::getInstance().triggerFocusCallback();

                // call this after setting currentlyFocusedComponent so that the one that's
                // losing it has a chance to see where focus is going
                if (componentLosingFocus != nullptr)
                    componentLosingFocus->internalFocusLoss (cause);

                if (currentlyFocusedComponent == this)
                    internalFocusGain (cause, safePointer);
            }
        }
    }
}

void Component::grabFocusInternal (const FocusChangeType cause, const bool canTryParent)
{
    if (isShowing())
    {
        if (flags.wantsFocusFlag && (isEnabled() || parentComponent == nullptr))
        {
            takeKeyboardFocus (cause);
        }
        else
        {
            if (isParentOf (currentlyFocusedComponent)
                 && currentlyFocusedComponent->isShowing())
            {
                // do nothing if the focused component is actually a child of ours..
            }
            else
            {
                // find the default child component..
                ScopedPointer<KeyboardFocusTraverser> traverser (createFocusTraverser());

                if (traverser != nullptr)
                {
                    Component* const defaultComp = traverser->getDefaultComponent (this);
                    traverser = nullptr;

                    if (defaultComp != nullptr)
                    {
                        defaultComp->grabFocusInternal (cause, false);
                        return;
                    }
                }

                if (canTryParent && parentComponent != nullptr)
                {
                    // if no children want it and we're allowed to try our parent comp,
                    // then pass up to parent, which will try our siblings.
                    parentComponent->grabFocusInternal (cause, true);
                }
            }
        }
    }
}

void Component::grabKeyboardFocus()
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    ASSERT_MESSAGE_MANAGER_IS_LOCKED

    grabFocusInternal (focusChangedDirectly, true);
}

void Component::moveKeyboardFocusToSibling (const bool moveToNext)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (parentComponent != nullptr)
    {
        ScopedPointer<KeyboardFocusTraverser> traverser (createFocusTraverser());

        if (traverser != nullptr)
        {
            Component* const nextComp = moveToNext ? traverser->getNextComponent (this)
                                                   : traverser->getPreviousComponent (this);
            traverser = nullptr;

            if (nextComp != nullptr)
            {
                if (nextComp->isCurrentlyBlockedByAnotherModalComponent())
                {
                    const WeakReference<Component> nextCompPointer (nextComp);
                    internalModalInputAttempt();

                    if (nextCompPointer == nullptr || nextComp->isCurrentlyBlockedByAnotherModalComponent())
                        return;
                }

                nextComp->grabFocusInternal (focusChangedByTabKey, true);
                return;
            }
        }

        parentComponent->moveKeyboardFocusToSibling (moveToNext);
    }
}

bool Component::hasKeyboardFocus (const bool trueIfChildIsFocused) const
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
    if (Component* c = getCurrentlyFocusedComponent())
        c->giveAwayFocus (true);
}

void Component::giveAwayFocus (const bool sendFocusLossEvent)
{
    Component* const componentLosingFocus = currentlyFocusedComponent;
    currentlyFocusedComponent = nullptr;

    if (sendFocusLossEvent && componentLosingFocus != nullptr)
        componentLosingFocus->internalFocusLoss (focusChangedDirectly);

    Desktop::getInstance().triggerFocusCallback();
}

//==============================================================================
bool Component::isEnabled() const noexcept
{
    return (! flags.isDisabledFlag)
            && (parentComponent == nullptr || parentComponent->isEnabled());
}

void Component::setEnabled (const bool shouldBeEnabled)
{
    if (flags.isDisabledFlag == shouldBeEnabled)
    {
        flags.isDisabledFlag = ! shouldBeEnabled;

        // if any parent components are disabled, setting our flag won't make a difference,
        // so no need to send a change message
        if (parentComponent == nullptr || parentComponent->isEnabled())
            sendEnablementChangeMessage();
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
        if (Component* const c = getChildComponent (i))
        {
            c->sendEnablementChangeMessage();

            if (safePointer == nullptr)
                return;
        }
    }
}

//==============================================================================
bool Component::isMouseOver (const bool includeChildren) const
{
    const Array<MouseInputSource>& mouseSources = Desktop::getInstance().getMouseSources();

    for (MouseInputSource* mi = mouseSources.begin(), * const e = mouseSources.end(); mi != e; ++mi)
    {
        Component* const c = mi->getComponentUnderMouse();

        if ((c == this || (includeChildren && isParentOf (c)))
              && c->reallyContains (c->getLocalPoint (nullptr, mi->getScreenPosition()).roundToInt(), false)
              && (mi->isMouse() || mi->isDragging()))
            return true;
    }

    return false;
}

bool Component::isMouseButtonDown() const
{
    const Array<MouseInputSource>& mouseSources = Desktop::getInstance().getMouseSources();

    for (MouseInputSource* mi = mouseSources.begin(), * const e = mouseSources.end(); mi != e; ++mi)
        if (mi->isDragging() && mi->getComponentUnderMouse() == this)
            return true;

    return false;
}

bool Component::isMouseOverOrDragging (const bool includeChildren) const
{
    const Array<MouseInputSource>& mouseSources = Desktop::getInstance().getMouseSources();

    for (MouseInputSource* mi = mouseSources.begin(), * const e = mouseSources.end(); mi != e; ++mi)
    {
        Component* const c = mi->getComponentUnderMouse();

        if ((c == this || (includeChildren && isParentOf (c)))
              && (mi->isMouse() || mi->isDragging()))
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
    return getLocalPoint (nullptr, Desktop::getMousePosition());
}

//==============================================================================
void Component::addKeyListener (KeyListener* const newListener)
{
    if (keyListeners == nullptr)
        keyListeners = new Array<KeyListener*>();

    keyListeners->addIfNotAlreadyThere (newListener);
}

void Component::removeKeyListener (KeyListener* const listenerToRemove)
{
    if (keyListeners != nullptr)
        keyListeners->removeFirstMatchingValue (listenerToRemove);
}

bool Component::keyPressed (const KeyPress&)                { return false; }
bool Component::keyStateChanged (const bool /*isKeyDown*/)  { return false; }

void Component::modifierKeysChanged (const ModifierKeys& modifiers)
{
    if (parentComponent != nullptr)
        parentComponent->modifierKeysChanged (modifiers);
}

void Component::internalModifierKeysChanged()
{
    sendFakeMouseMove();
    modifierKeysChanged (ModifierKeys::getCurrentModifiers());
}

//==============================================================================
Component::BailOutChecker::BailOutChecker (Component* const component)
    : safePointer (component)
{
    jassert (component != nullptr);
}

bool Component::BailOutChecker::shouldBailOut() const noexcept
{
    return safePointer == nullptr;
}
