/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
class LinuxComponentPeer final : public ComponentPeer,
                                 private XWindowSystemUtilities::XSettings::Listener
{
public:
    LinuxComponentPeer (Component& comp, int windowStyleFlags, ::Window parentToAddTo)
        : ComponentPeer (comp, windowStyleFlags),
          isAlwaysOnTop (comp.isAlwaysOnTop())
    {
        // it's dangerous to create a window on a thread other than the message thread.
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        const auto* instance = XWindowSystem::getInstance();

        if (! instance->isX11Available())
            return;

        if (isAlwaysOnTop)
            ++WindowUtilsInternal::numAlwaysOnTopPeers;

        repainter = std::make_unique<LinuxRepaintManager> (*this);

        windowH = instance->createWindow (parentToAddTo, this);
        parentWindow = parentToAddTo;

        setTitle (component.getName());

        if (auto* xSettings = instance->getXSettings())
            xSettings->addListener (this);

        getNativeRealtimeModifiers = []() -> ModifierKeys { return XWindowSystem::getInstance()->getNativeRealtimeModifiers(); };

        updateVBlankTimer();
    }

    ~LinuxComponentPeer() override
    {
        // it's dangerous to delete a window on a thread other than the message thread.
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        auto* instance = XWindowSystem::getInstance();

        repainter = nullptr;
        instance->destroyWindow (windowH);

        if (auto* xSettings = instance->getXSettings())
            xSettings->removeListener (this);

        if (isAlwaysOnTop)
            --WindowUtilsInternal::numAlwaysOnTopPeers;
    }

    ::Window getWindowHandle() const noexcept
    {
        return windowH;
    }

    //==============================================================================
    void* getNativeHandle() const override
    {
        return reinterpret_cast<void*> (getWindowHandle());
    }

    //==============================================================================
    void forceSetBounds (const Rectangle<int>& correctedNewBounds, bool isNowFullScreen)
    {
        bounds = correctedNewBounds;

        updateScaleFactorFromNewBounds (bounds, false);

        auto physicalBounds = parentWindow == 0 ? Desktop::getInstance().getDisplays().logicalToPhysical (bounds)
                                                : bounds * currentScaleFactor;

        WeakReference<Component> deletionChecker (&component);

        XWindowSystem::getInstance()->setBounds (windowH, physicalBounds, isNowFullScreen);

        fullScreen = isNowFullScreen;

        if (deletionChecker != nullptr)
        {
            updateBorderSize();
            handleMovedOrResized();
        }
    }

    void setBounds (const Rectangle<int>& newBounds, bool isNowFullScreen) override
    {
        const auto correctedNewBounds = newBounds.withSize (jmax (1, newBounds.getWidth()),
                                                            jmax (1, newBounds.getHeight()));

        if (bounds != correctedNewBounds || fullScreen != isNowFullScreen)
            forceSetBounds (correctedNewBounds, isNowFullScreen);
    }

    Point<int> getScreenPosition (bool physical) const
    {
        auto physicalParentPosition = XWindowSystem::getInstance()->getPhysicalParentScreenPosition();
        auto parentPosition = parentWindow == 0 ? Desktop::getInstance().getDisplays().physicalToLogical (physicalParentPosition)
                                                : physicalParentPosition / currentScaleFactor;

        auto screenBounds = parentWindow == 0 ? bounds
                                              : bounds.translated (parentPosition.x, parentPosition.y);

        if (physical)
            return parentWindow == 0 ? Desktop::getInstance().getDisplays().logicalToPhysical (screenBounds.getTopLeft())
                                     : screenBounds.getTopLeft() * currentScaleFactor;

        return screenBounds.getTopLeft();
    }

    Rectangle<int> getBounds() const override
    {
        return bounds;
    }

    OptionalBorderSize getFrameSizeIfPresent() const override
    {
        return windowBorder;
    }

    BorderSize<int> getFrameSize() const override
    {
        const auto optionalBorderSize = getFrameSizeIfPresent();
        return optionalBorderSize ? (*optionalBorderSize) : BorderSize<int>();
    }

    Point<float> localToGlobal (Point<float> relativePosition) override
    {
        return localToGlobal (*this, relativePosition);
    }

    Point<float> globalToLocal (Point<float> screenPosition) override
    {
        return globalToLocal (*this, screenPosition);
    }

    using ComponentPeer::localToGlobal;
    using ComponentPeer::globalToLocal;

    //==============================================================================
    StringArray getAvailableRenderingEngines() override
    {
        return { "Software Renderer" };
    }

    void setVisible (bool shouldBeVisible) override
    {
        XWindowSystem::getInstance()->setVisible (windowH, shouldBeVisible);
    }

    void setTitle (const String& title) override
    {
        XWindowSystem::getInstance()->setTitle (windowH, title);
    }

    void setMinimised (bool shouldBeMinimised) override
    {
        if (shouldBeMinimised)
            XWindowSystem::getInstance()->setMinimised (windowH, shouldBeMinimised);
        else
            setVisible (true);
    }

    bool isMinimised() const override
    {
        return XWindowSystem::getInstance()->isMinimised (windowH);
    }

    void setFullScreen (bool shouldBeFullScreen) override
    {
        auto r = lastNonFullscreenBounds; // (get a copy of this before de-minimising)

        setMinimised (false);

        if (fullScreen != shouldBeFullScreen)
        {
            const auto usingNativeTitleBar = ((styleFlags & windowHasTitleBar) != 0);

            if (usingNativeTitleBar)
                XWindowSystem::getInstance()->setMaximised (windowH, shouldBeFullScreen);

            if (shouldBeFullScreen)
                r = usingNativeTitleBar ? XWindowSystem::getInstance()->getWindowBounds (windowH, parentWindow)
                                        : Desktop::getInstance().getDisplays().getDisplayForRect (bounds)->userArea;

            if (! r.isEmpty())
                setBounds (detail::ScalingHelpers::scaledScreenPosToUnscaled (component, r), shouldBeFullScreen);

            component.repaint();
        }
    }

    bool isFullScreen() const override
    {
        return fullScreen;
    }

    bool contains (Point<int> localPos, bool trueIfInAChildWindow) const override
    {
        if (! bounds.withZeroOrigin().contains (localPos))
            return false;

        for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
        {
            auto* c = Desktop::getInstance().getComponent (i);

            if (c == &component)
                break;

            if (! c->isVisible())
                continue;

            auto* otherPeer = c->getPeer();
            jassert (otherPeer == nullptr || dynamic_cast<LinuxComponentPeer*> (c->getPeer()) != nullptr);

            if (auto* peer = static_cast<LinuxComponentPeer*> (otherPeer))
                if (peer->contains (globalToLocal (*peer, localToGlobal (*this, localPos.toFloat())).roundToInt(), true))
                    return false;
        }

        if (trueIfInAChildWindow)
            return true;

        return XWindowSystem::getInstance()->contains (windowH, localPos * currentScaleFactor);
    }

    void toFront (bool makeActive) override
    {
        if (makeActive)
        {
            setVisible (true);
            grabFocus();
        }

        XWindowSystem::getInstance()->toFront (windowH, makeActive);
        handleBroughtToFront();
    }

    void toBehind (ComponentPeer* other) override
    {
        if (auto* otherPeer = dynamic_cast<LinuxComponentPeer*> (other))
        {
            if (otherPeer->styleFlags & windowIsTemporary)
                return;

            setMinimised (false);
            XWindowSystem::getInstance()->toBehind (windowH, otherPeer->windowH);
        }
        else
        {
            jassertfalse; // wrong type of window?
        }
    }

    bool isFocused() const override
    {
        return XWindowSystem::getInstance()->isFocused (windowH);
    }

    void grabFocus() override
    {
        if (XWindowSystem::getInstance()->grabFocus (windowH))
            isActiveApplication = true;
    }

    //==============================================================================
    void repaint (const Rectangle<int>& area) override
    {
        if (repainter != nullptr)
            repainter->repaint (area.getIntersection (bounds.withZeroOrigin()));
    }

    void performAnyPendingRepaintsNow() override
    {
        if (repainter != nullptr)
            repainter->performAnyPendingRepaintsNow();
    }

    void setIcon (const Image& newIcon) override
    {
        XWindowSystem::getInstance()->setIcon (windowH, newIcon);
    }

    double getPlatformScaleFactor() const noexcept override
    {
        return currentScaleFactor;
    }

    void setAlpha (float) override                                  {}
    bool setAlwaysOnTop (bool) override                             { return false; }
    void textInputRequired (Point<int>, TextInputTarget&) override  {}

    //==============================================================================
    void addOpenGLRepaintListener (Component* dummy)
    {
        if (dummy != nullptr)
            glRepaintListeners.addIfNotAlreadyThere (dummy);
    }

    void removeOpenGLRepaintListener (Component* dummy)
    {
        if (dummy != nullptr)
            glRepaintListeners.removeAllInstancesOf (dummy);
    }

    void repaintOpenGLContexts()
    {
        for (auto* c : glRepaintListeners)
            c->handleCommandMessage (0);
    }

    //==============================================================================
    ::Window getParentWindow()                         { return parentWindow; }
    void setParentWindow (::Window newParent)          { parentWindow = newParent; }

    //==============================================================================
    bool isConstrainedNativeWindow() const
    {
        return constrainer != nullptr
            && (styleFlags & (windowHasTitleBar | windowIsResizable)) == (windowHasTitleBar | windowIsResizable)
            && ! isKioskMode();
    }

    void updateWindowBounds()
    {
        if (windowH == 0)
        {
            jassertfalse;
            return;
        }

        if (isConstrainedNativeWindow())
            XWindowSystem::getInstance()->updateConstraints (windowH);

        auto physicalBounds = XWindowSystem::getInstance()->getWindowBounds (windowH, parentWindow);

        updateScaleFactorFromNewBounds (physicalBounds, true);

        bounds = parentWindow == 0 ? Desktop::getInstance().getDisplays().physicalToLogical (physicalBounds)
                                   : physicalBounds / currentScaleFactor;

        updateVBlankTimer();
    }

    void updateBorderSize()
    {
        if ((styleFlags & windowHasTitleBar) == 0)
        {
            windowBorder = ComponentPeer::OptionalBorderSize { BorderSize<int>() };
        }
        else if (! windowBorder
                 || ((*windowBorder).getTopAndBottom() == 0 && (*windowBorder).getLeftAndRight() == 0))
        {
            windowBorder = [&]()
            {
                if (auto unscaledBorderSize = XWindowSystem::getInstance()->getBorderSize (windowH))
                    return OptionalBorderSize { (*unscaledBorderSize).multipliedBy (1.0 / currentScaleFactor) };

                return OptionalBorderSize {};
            }();
        }
    }

    bool setWindowAssociation (::Window windowIn)
    {
        clearWindowAssociation();
        association = { this, windowIn };
        return association.isValid();
    }

    void clearWindowAssociation() { association = {}; }

    void startHostManagedResize (Point<int>, ResizableBorderComponent::Zone zone) override
    {
        XWindowSystem::getInstance()->startHostManagedResize (windowH, zone);
    }

    //==============================================================================
    static bool isActiveApplication;
    bool focused = false;

private:
    //==============================================================================
    class LinuxRepaintManager
    {
    public:
        LinuxRepaintManager (LinuxComponentPeer& p)
            : peer (p),
              isSemiTransparentWindow ((peer.getStyleFlags() & ComponentPeer::windowIsSemiTransparent) != 0)
        {
        }

        void dispatchDeferredRepaints()
        {
            XWindowSystem::getInstance()->processPendingPaintsForWindow (peer.windowH);

            if (XWindowSystem::getInstance()->getNumPaintsPendingForWindow (peer.windowH) > 0)
                return;

            if (! regionsNeedingRepaint.isEmpty())
                performAnyPendingRepaintsNow();
            else if (Time::getApproximateMillisecondCounter() > lastTimeImageUsed + 3000)
                image = Image();
        }

        void repaint (Rectangle<int> area)
        {
            regionsNeedingRepaint.add (area * peer.currentScaleFactor);
        }

        void performAnyPendingRepaintsNow()
        {
            if (XWindowSystem::getInstance()->getNumPaintsPendingForWindow (peer.windowH) > 0)
                return;

            auto originalRepaintRegion = regionsNeedingRepaint;
            regionsNeedingRepaint.clear();
            auto totalArea = originalRepaintRegion.getBounds();

            if (! totalArea.isEmpty())
            {
                const auto wasImageNull = image.isNull();

                if (wasImageNull || image.getWidth() < totalArea.getWidth()
                     || image.getHeight() < totalArea.getHeight())
                {
                    image = XWindowSystem::getInstance()->createImage (isSemiTransparentWindow,
                                                                       totalArea.getWidth(), totalArea.getHeight(),
                                                                       useARGBImagesForRendering);
                    if (wasImageNull)
                    {
                        // After calling createImage() XWindowSystem::getWindowBounds() will return
                        // changed coordinates that look like the result of some position
                        // defaulting mechanism. If we handle a configureNotifyEvent after
                        // createImage() and before we would issue new, valid coordinates, we will
                        // apply these default, unwanted coordinates to our window. To avoid that
                        // we immediately send another positioning message to guarantee that the
                        // next configureNotifyEvent will read valid values.
                        //
                        // This issue only occurs right after peer creation, when the image is
                        // null. Updating when only the width or height is changed would lead to
                        // incorrect behaviour.
                        peer.forceSetBounds (detail::ScalingHelpers::scaledScreenPosToUnscaled (peer.component, peer.component.getBoundsInParent()),
                                             peer.isFullScreen());
                    }
                }

                RectangleList<int> adjustedList (originalRepaintRegion);
                adjustedList.offsetAll (-totalArea.getX(), -totalArea.getY());

                if (XWindowSystem::getInstance()->canUseARGBImages())
                    for (auto& i : originalRepaintRegion)
                        image.clear (i - totalArea.getPosition());

                {
                    auto context = peer.getComponent().getLookAndFeel()
                                     .createGraphicsContext (image, -totalArea.getPosition(), adjustedList);

                    context->addTransform (AffineTransform::scale ((float) peer.currentScaleFactor));
                    peer.handlePaint (*context);
                }

                for (auto& i : originalRepaintRegion)
                   XWindowSystem::getInstance()->blitToWindow (peer.windowH, image, i, totalArea);
            }

            lastTimeImageUsed = Time::getApproximateMillisecondCounter();
        }

    private:
        LinuxComponentPeer& peer;
        const bool isSemiTransparentWindow;
        Image image;
        uint32 lastTimeImageUsed = 0;
        RectangleList<int> regionsNeedingRepaint;

        bool useARGBImagesForRendering = XWindowSystem::getInstance()->canUseARGBImages();

        JUCE_DECLARE_NON_COPYABLE (LinuxRepaintManager)
    };

    //==============================================================================
    template <typename This>
    static Point<float> localToGlobal (This& t, Point<float> relativePosition)
    {
        return relativePosition + t.getScreenPosition (false).toFloat();
    }

    template <typename This>
    static Point<float> globalToLocal (This& t, Point<float> screenPosition)
    {
        return screenPosition - t.getScreenPosition (false).toFloat();
    }

    //==============================================================================
    void settingChanged (const XWindowSystemUtilities::XSetting& settingThatHasChanged) override
    {
        static StringArray possibleSettings { XWindowSystem::getWindowScalingFactorSettingName(),
                                              "Gdk/UnscaledDPI",
                                              "Xft/DPI" };

        if (possibleSettings.contains (settingThatHasChanged.name))
            forceDisplayUpdate();
    }

    void updateScaleFactorFromNewBounds (const Rectangle<int>& newBounds, bool isPhysical)
    {
        Point<int> translation = (parentWindow != 0 ? getScreenPosition (isPhysical) : Point<int>());
        const auto& desktop = Desktop::getInstance();

        if (auto* display = desktop.getDisplays().getDisplayForRect (newBounds.translated (translation.x, translation.y),
                                                                     isPhysical))
        {
            auto newScaleFactor = display->scale / desktop.getGlobalScaleFactor();

            if (! approximatelyEqual (newScaleFactor, currentScaleFactor))
            {
                currentScaleFactor = newScaleFactor;
                scaleFactorListeners.call ([&] (ScaleFactorListener& l) { l.nativeScaleFactorChanged (currentScaleFactor); });
            }
        }
    }

    void onVBlank()
    {
        vBlankListeners.call ([] (auto& l) { l.onVBlank(); });

        if (repainter != nullptr)
            repainter->dispatchDeferredRepaints();
    }

    void updateVBlankTimer()
    {
        if (auto* display = Desktop::getInstance().getDisplays().getDisplayForRect (bounds))
        {
            // Some systems fail to set an explicit refresh rate, or ask for a refresh rate of 0
            // (observed on Raspbian Bullseye over VNC). In these situations, use a fallback value.
            const auto newIntFrequencyHz = roundToInt (display->verticalFrequencyHz.value_or (0.0));
            const auto frequencyToUse = newIntFrequencyHz != 0 ? newIntFrequencyHz : 100;

            if (vBlankManager.getTimerInterval() != frequencyToUse)
                vBlankManager.startTimerHz (frequencyToUse);
        }
    }

    //==============================================================================
    std::unique_ptr<LinuxRepaintManager> repainter;
    TimedCallback vBlankManager { [this]() { onVBlank(); } };

    ::Window windowH = {}, parentWindow = {};
    Rectangle<int> bounds;
    ComponentPeer::OptionalBorderSize windowBorder;
    bool fullScreen = false, isAlwaysOnTop = false;
    double currentScaleFactor = 1.0;
    Array<Component*> glRepaintListeners;
    ScopedWindowAssociation association;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinuxComponentPeer)
};

bool LinuxComponentPeer::isActiveApplication = false;

//==============================================================================
ComponentPeer* Component::createNewPeer (int styleFlags, void* nativeWindowToAttachTo)
{
    return new LinuxComponentPeer (*this, styleFlags, (::Window) nativeWindowToAttachTo);
}

//==============================================================================
JUCE_API bool JUCE_CALLTYPE Process::isForegroundProcess()    { return LinuxComponentPeer::isActiveApplication; }

JUCE_API void JUCE_CALLTYPE Process::makeForegroundProcess()  {}
JUCE_API void JUCE_CALLTYPE Process::hide()                   {}

//==============================================================================
void Desktop::setKioskComponent (Component* comp, bool enableOrDisable, bool)
{
    if (enableOrDisable)
        comp->setBounds (getDisplays().getDisplayForRect (comp->getScreenBounds())->totalArea);
}

void Displays::findDisplays (float masterScale)
{
    if (XWindowSystem::getInstance()->getDisplay() != nullptr)
    {
        displays = XWindowSystem::getInstance()->findDisplays (masterScale);

        if (! displays.isEmpty())
            updateToLogical();
    }
}

bool Desktop::canUseSemiTransparentWindows() noexcept
{
    return XWindowSystem::getInstance()->canUseSemiTransparentWindows();
}

class Desktop::NativeDarkModeChangeDetectorImpl  : private XWindowSystemUtilities::XSettings::Listener
{
public:
    NativeDarkModeChangeDetectorImpl()
    {
        const auto* windowSystem = XWindowSystem::getInstance();

        if (auto* xSettings = windowSystem->getXSettings())
            xSettings->addListener (this);

        darkModeEnabled = windowSystem->isDarkModeActive();
    }

    ~NativeDarkModeChangeDetectorImpl() override
    {
        if (auto* windowSystem = XWindowSystem::getInstanceWithoutCreating())
            if (auto* xSettings = windowSystem->getXSettings())
                xSettings->removeListener (this);
    }

    bool isDarkModeEnabled() const noexcept  { return darkModeEnabled; }

private:
    void settingChanged (const XWindowSystemUtilities::XSetting& settingThatHasChanged) override
    {
        if (settingThatHasChanged.name == XWindowSystem::getThemeNameSettingName())
        {
            const auto wasDarkModeEnabled = std::exchange (darkModeEnabled, XWindowSystem::getInstance()->isDarkModeActive());

            if (darkModeEnabled != wasDarkModeEnabled)
                Desktop::getInstance().darkModeChanged();
        }
    }

    bool darkModeEnabled = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeDarkModeChangeDetectorImpl)
};

std::unique_ptr<Desktop::NativeDarkModeChangeDetectorImpl> Desktop::createNativeDarkModeChangeDetectorImpl()
{
    return std::make_unique<NativeDarkModeChangeDetectorImpl>();
}

bool Desktop::isDarkModeActive() const
{
    return nativeDarkModeChangeDetectorImpl->isDarkModeEnabled();
}

static bool screenSaverAllowed = true;

void Desktop::setScreenSaverEnabled (bool isEnabled)
{
    if (screenSaverAllowed != isEnabled)
    {
        screenSaverAllowed = isEnabled;
        XWindowSystem::getInstance()->setScreenSaverEnabled (screenSaverAllowed);
    }
}

bool Desktop::isScreenSaverEnabled()
{
    return screenSaverAllowed;
}

double Desktop::getDefaultMasterScale()                             { return 1.0; }

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const  { return upright; }
void Desktop::allowedOrientationsChanged()                          {}

//==============================================================================
bool detail::MouseInputSourceList::addSource()
{
    if (sources.isEmpty())
    {
        addSource (0, MouseInputSource::InputSourceType::mouse);
        return true;
    }

    return false;
}

bool detail::MouseInputSourceList::canUseTouch() const
{
    return false;
}

Point<float> MouseInputSource::getCurrentRawMousePosition()
{
    return Desktop::getInstance().getDisplays().physicalToLogical (XWindowSystem::getInstance()->getCurrentMousePosition());
}

void MouseInputSource::setRawMousePosition (Point<float> newPosition)
{
    XWindowSystem::getInstance()->setMousePosition (Desktop::getInstance().getDisplays().logicalToPhysical (newPosition));
}

//==============================================================================
class MouseCursor::PlatformSpecificHandle
{
public:
    explicit PlatformSpecificHandle (const MouseCursor::StandardCursorType type)
        : cursorHandle (makeHandle (type)) {}

    explicit PlatformSpecificHandle (const detail::CustomMouseCursorInfo& info)
        : cursorHandle (makeHandle (info)) {}

    ~PlatformSpecificHandle()
    {
        if (cursorHandle != Cursor{})
            XWindowSystem::getInstance()->deleteMouseCursor (cursorHandle);
    }

    static void showInWindow (PlatformSpecificHandle* handle, ComponentPeer* peer)
    {
        const auto cursor = handle != nullptr ? handle->cursorHandle : Cursor{};

        if (peer != nullptr)
            XWindowSystem::getInstance()->showCursor ((::Window) peer->getNativeHandle(), cursor);
    }

private:
    static Cursor makeHandle (const detail::CustomMouseCursorInfo& info)
    {
        const auto image = info.image.getImage();
        return XWindowSystem::getInstance()->createCustomMouseCursorInfo (image.rescaled ((int) (image.getWidth()  / info.image.getScale()),
                                                                                          (int) (image.getHeight() / info.image.getScale())), info.hotspot);
    }

    static Cursor makeHandle (MouseCursor::StandardCursorType type)
    {
        return XWindowSystem::getInstance()->createStandardMouseCursor (type);
    }

    Cursor cursorHandle;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE (PlatformSpecificHandle)
    JUCE_DECLARE_NON_MOVEABLE (PlatformSpecificHandle)
};

//==============================================================================
static LinuxComponentPeer* getPeerForDragEvent (Component* sourceComp)
{
    if (sourceComp == nullptr)
        if (auto* draggingSource = Desktop::getInstance().getDraggingMouseSource (0))
            sourceComp = draggingSource->getComponentUnderMouse();

    if (sourceComp != nullptr)
        if (auto* lp = dynamic_cast<LinuxComponentPeer*> (sourceComp->getPeer()))
            return lp;

    jassertfalse;  // This method must be called in response to a component's mouseDown or mouseDrag event!
    return nullptr;
}

bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, bool canMoveFiles,
                                                           Component* sourceComp, std::function<void()> callback)
{
    if (files.isEmpty())
        return false;

    if (auto* peer = getPeerForDragEvent (sourceComp))
        return XWindowSystem::getInstance()->externalDragFileInit (peer, files, canMoveFiles, std::move (callback));

    // This method must be called in response to a component's mouseDown or mouseDrag event!
    jassertfalse;
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text, Component* sourceComp,
                                                          std::function<void()> callback)
{
    if (text.isEmpty())
        return false;

    if (auto* peer = getPeerForDragEvent (sourceComp))
        return XWindowSystem::getInstance()->externalDragTextInit (peer, text, std::move (callback));

    // This method must be called in response to a component's mouseDown or mouseDrag event!
    jassertfalse;
    return false;
}

//==============================================================================
void SystemClipboard::copyTextToClipboard (const String& clipText)
{
    XWindowSystem::getInstance()->copyTextToClipboard (clipText);
}

String SystemClipboard::getTextFromClipboard()
{
    return XWindowSystem::getInstance()->getTextFromClipboard();
}

//==============================================================================
bool KeyPress::isKeyCurrentlyDown (int keyCode)
{
    return XWindowSystem::getInstance()->isKeyCurrentlyDown (keyCode);
}

void LookAndFeel::playAlertSound()
{
    std::cout << "\a" << std::flush;
}

//==============================================================================
Image detail::WindowingHelpers::createIconForFile (const File&)
{
    return {};
}

void juce_LinuxAddRepaintListener (ComponentPeer* peer, Component* dummy);
void juce_LinuxAddRepaintListener (ComponentPeer* peer, Component* dummy)
{
    if (auto* linuxPeer = dynamic_cast<LinuxComponentPeer*> (peer))
        linuxPeer->addOpenGLRepaintListener (dummy);
}

void juce_LinuxRemoveRepaintListener (ComponentPeer* peer, Component* dummy);
void juce_LinuxRemoveRepaintListener (ComponentPeer* peer, Component* dummy)
{
    if (auto* linuxPeer = dynamic_cast<LinuxComponentPeer*> (peer))
        linuxPeer->removeOpenGLRepaintListener (dummy);
}

} // namespace juce
