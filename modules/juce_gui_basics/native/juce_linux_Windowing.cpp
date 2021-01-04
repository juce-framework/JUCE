/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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
static int numAlwaysOnTopPeers = 0;
bool juce_areThereAnyAlwaysOnTopWindows()  { return numAlwaysOnTopPeers > 0; }

//==============================================================================
template<typename WindowHandleType>
class LinuxComponentPeer  : public ComponentPeer
{
public:
    LinuxComponentPeer (Component& comp, int windowStyleFlags, WindowHandleType parentToAddTo)
        : ComponentPeer (comp, windowStyleFlags),
          isAlwaysOnTop (comp.isAlwaysOnTop())
    {
        // it's dangerous to create a window on a thread other than the message thread..
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        if (isAlwaysOnTop)
            ++numAlwaysOnTopPeers;

        repainter = std::make_unique<LinuxRepaintManager> (*this);

        windowH = XWindowSystem::getInstance()->createWindow (parentToAddTo, this);
        parentWindow = parentToAddTo;

        setTitle (component.getName());

        getNativeRealtimeModifiers = []() -> ModifierKeys { return XWindowSystem::getInstance()->getNativeRealtimeModifiers(); };
    }

    ~LinuxComponentPeer() override
    {
        // it's dangerous to delete a window on a thread other than the message thread..
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        repainter = nullptr;
        XWindowSystem::getInstance()->destroyWindow (windowH);

        if (isAlwaysOnTop)
            --numAlwaysOnTopPeers;
    }

    //==============================================================================
    void* getNativeHandle() const override
    {
        return (void*) windowH;
    }

    //==============================================================================
    void setBounds (const Rectangle<int>& newBounds, bool isNowFullScreen) override
    {
        bounds = newBounds.withSize (jmax (1, newBounds.getWidth()),
                                     jmax (1, newBounds.getHeight()));

        updateScaleFactorFromNewBounds (bounds, false);

        auto physicalBounds = (parentWindow == 0 ? Desktop::getInstance().getDisplays().logicalToPhysical (bounds)
                               : bounds * currentScaleFactor);

        WeakReference<Component> deletionChecker (&component);

        XWindowSystem::getInstance()->setBounds (windowH, physicalBounds, isNowFullScreen);

        fullScreen = isNowFullScreen;

        if (deletionChecker != nullptr)
        {
            updateBorderSize();
            handleMovedOrResized();
        }
    }

    Point<int> getScreenPosition (bool physical) const
    {
        auto parentPosition = XWindowSystem::getInstance()->getParentScreenPosition();

        auto screenBounds = (parentWindow == 0 ? bounds
                                               : bounds.translated (parentPosition.x, parentPosition.y));

        if (physical)
            return Desktop::getInstance().getDisplays().logicalToPhysical (screenBounds.getTopLeft());

        return screenBounds.getTopLeft();
    }

    Rectangle<int> getBounds() const override
    {
        return bounds;
    }

    BorderSize<int> getFrameSize() const override
    {
        return windowBorder;
    }

    Point<float> localToGlobal (Point<float> relativePosition) override
    {
        return relativePosition + getScreenPosition (false).toFloat();
    }

    Point<float> globalToLocal (Point<float> screenPosition) override
    {
        return screenPosition - getScreenPosition (false).toFloat();
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
            if (shouldBeFullScreen)
                r = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;

            if (! r.isEmpty())
                setBounds (ScalingHelpers::scaledScreenPosToUnscaled (component, r), shouldBeFullScreen);

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

            if (auto* peer = c->getPeer())
                if (peer->contains (localPos + bounds.getPosition() - peer->getBounds().getPosition(), true))
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
        repainter->repaint (area.getIntersection (bounds.withZeroOrigin()));
    }

    void performAnyPendingRepaintsNow() override
    {
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
    WindowHandleType getParentWindow()                 { return parentWindow; }
    void setParentWindow (WindowHandleType newParent)  { parentWindow = newParent; }

    //==============================================================================
    void updateWindowBounds()
    {
        jassert (windowH != 0);
        if (windowH != 0)
        {
            auto physicalBounds = XWindowSystem::getInstance()->getWindowBounds (windowH, parentWindow);

            updateScaleFactorFromNewBounds (physicalBounds, true);

            bounds = (parentWindow == 0 ? Desktop::getInstance().getDisplays().physicalToLogical (physicalBounds)
                                        : physicalBounds / currentScaleFactor);
        }
    }

    void updateBorderSize()
    {
        if ((styleFlags & windowHasTitleBar) == 0)
            windowBorder = {};
        else if (windowBorder.getTopAndBottom() == 0 && windowBorder.getLeftAndRight() == 0)
            windowBorder = XWindowSystem::getInstance()->getBorderSize (windowH);
    }

    //==============================================================================
    static bool isActiveApplication;
    bool focused = false;

private:
    //==============================================================================
    class LinuxRepaintManager   : public Timer
    {
    public:
        LinuxRepaintManager (LinuxComponentPeer& p)
            : peer (p),
              isSemiTransparentWindow ((peer.getStyleFlags() & ComponentPeer::windowIsSemiTransparent) != 0)
        {
        }

        void timerCallback() override
        {
            XWindowSystem::getInstance()->processPendingPaintsForWindow (peer.windowH);

            if (XWindowSystem::getInstance()->getNumPaintsPendingForWindow (peer.windowH) > 0)
                return;

            if (! regionsNeedingRepaint.isEmpty())
            {
                stopTimer();
                performAnyPendingRepaintsNow();
            }
            else if (Time::getApproximateMillisecondCounter() > lastTimeImageUsed + 3000)
            {
                stopTimer();
                image = Image();
            }
        }

        void repaint (Rectangle<int> area)
        {
            if (! isTimerRunning())
                startTimer (repaintTimerPeriod);

            regionsNeedingRepaint.add (area * peer.currentScaleFactor);
        }

        void performAnyPendingRepaintsNow()
        {
            if (XWindowSystem::getInstance()->getNumPaintsPendingForWindow (peer.windowH) > 0)
            {
                startTimer (repaintTimerPeriod);
                return;
            }

            auto originalRepaintRegion = regionsNeedingRepaint;
            regionsNeedingRepaint.clear();
            auto totalArea = originalRepaintRegion.getBounds();

            if (! totalArea.isEmpty())
            {
                if (image.isNull() || image.getWidth() < totalArea.getWidth()
                     || image.getHeight() < totalArea.getHeight())
                {
                    image = XWindowSystem::getInstance()->createImage (isSemiTransparentWindow,
                                                                       totalArea.getWidth(), totalArea.getHeight(),
                                                                       useARGBImagesForRendering);
                }

                startTimer (repaintTimerPeriod);

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
            startTimer (repaintTimerPeriod);
        }

    private:
        enum { repaintTimerPeriod = 1000 / 100 };

        LinuxComponentPeer& peer;
        const bool isSemiTransparentWindow;
        Image image;
        uint32 lastTimeImageUsed = 0;
        RectangleList<int> regionsNeedingRepaint;

        bool useARGBImagesForRendering = XWindowSystem::getInstance()->canUseARGBImages();

        JUCE_DECLARE_NON_COPYABLE (LinuxRepaintManager)
    };

    //==============================================================================
    void updateScaleFactorFromNewBounds (const Rectangle<int>& newBounds, bool isPhysical)
    {
        if (! JUCEApplicationBase::isStandaloneApp())
            return;

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

    //==============================================================================
    std::unique_ptr<LinuxRepaintManager> repainter;

    WindowHandleType windowH = {}, parentWindow = {}, keyProxy = {};
    Rectangle<int> bounds;
    BorderSize<int> windowBorder;
    bool fullScreen = false, isAlwaysOnTop = false;
    double currentScaleFactor = 1.0;
    Array<Component*> glRepaintListeners;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinuxComponentPeer)
};

template<typename WindowHandleType>
bool LinuxComponentPeer<WindowHandleType>::isActiveApplication = false;

//==============================================================================
ComponentPeer* Component::createNewPeer (int styleFlags, void* nativeWindowToAttachTo)
{
    return new LinuxComponentPeer<::Window> (*this, styleFlags, (::Window) nativeWindowToAttachTo);
}

//==============================================================================
JUCE_API bool JUCE_CALLTYPE Process::isForegroundProcess()    { return LinuxComponentPeer<::Window>::isActiveApplication; }

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
bool MouseInputSource::SourceList::addSource()
{
    if (sources.isEmpty())
    {
        addSource (0, MouseInputSource::InputSourceType::mouse);
        return true;
    }

    return false;
}

bool MouseInputSource::SourceList::canUseTouch()
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
void* CustomMouseCursorInfo::create() const
{
    return XWindowSystem::getInstance()->createCustomMouseCursorInfo (image, hotspot);
}

void MouseCursor::deleteMouseCursor (void* cursorHandle, bool)
{
    if (cursorHandle != nullptr)
        XWindowSystem::getInstance()->deleteMouseCursor (cursorHandle);
}

void* MouseCursor::createStandardMouseCursor (MouseCursor::StandardCursorType type)
{
    return XWindowSystem::getInstance()->createStandardMouseCursor (type);
}

void MouseCursor::showInWindow (ComponentPeer* peer) const
{
    if (peer != nullptr)
        XWindowSystem::getInstance()->showCursor ((::Window) peer->getNativeHandle(), getHandle());
}

//==============================================================================
template<typename WindowHandleType>
static LinuxComponentPeer<WindowHandleType>* getPeerForDragEvent (Component* sourceComp)
{
    if (sourceComp == nullptr)
        if (auto* draggingSource = Desktop::getInstance().getDraggingMouseSource (0))
            sourceComp = draggingSource->getComponentUnderMouse();

    if (sourceComp != nullptr)
        if (auto* lp = dynamic_cast<LinuxComponentPeer<::Window>*> (sourceComp->getPeer()))
            return lp;

    jassertfalse;  // This method must be called in response to a component's mouseDown or mouseDrag event!
    return nullptr;
}

bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, bool canMoveFiles,
                                                           Component* sourceComp, std::function<void()> callback)
{
    if (files.isEmpty())
        return false;

    if (auto* peer = getPeerForDragEvent<::Window> (sourceComp))
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

    if (auto* peer = getPeerForDragEvent<::Window> (sourceComp))
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
#if JUCE_MODAL_LOOPS_PERMITTED
void JUCE_CALLTYPE NativeMessageBox::showMessageBox (AlertWindow::AlertIconType iconType,
                                                     const String& title, const String& message,
                                                     Component*)
{
    AlertWindow::showMessageBox (iconType, title, message);
}
#endif

void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (AlertWindow::AlertIconType iconType,
                                                          const String& title, const String& message,
                                                          Component* associatedComponent,
                                                          ModalComponentManager::Callback* callback)
{
    AlertWindow::showMessageBoxAsync (iconType, title, message, {}, associatedComponent, callback);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (AlertWindow::AlertIconType iconType,
                                                      const String& title, const String& message,
                                                      Component* associatedComponent,
                                                      ModalComponentManager::Callback* callback)
{
    return AlertWindow::showOkCancelBox (iconType, title, message, {}, {}, associatedComponent, callback);
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoCancelBox (AlertWindow::AlertIconType iconType,
                                                        const String& title, const String& message,
                                                        Component* associatedComponent,
                                                        ModalComponentManager::Callback* callback)
{
    return AlertWindow::showYesNoCancelBox (iconType, title, message, {}, {}, {},
                                            associatedComponent, callback);
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoBox (AlertWindow::AlertIconType iconType,
                                                  const String& title, const String& message,
                                                  Component* associatedComponent,
                                                  ModalComponentManager::Callback* callback)
{
    return AlertWindow::showOkCancelBox (iconType, title, message, TRANS ("Yes"), TRANS ("No"),
                                         associatedComponent, callback);
}

//==============================================================================
Image juce_createIconForFile (const File&)
{
    return {};
}

void juce_LinuxAddRepaintListener (ComponentPeer* peer, Component* dummy)
{
    if (auto* linuxPeer = dynamic_cast<LinuxComponentPeer<::Window>*> (peer))
        linuxPeer->addOpenGLRepaintListener (dummy);
}

void juce_LinuxRemoveRepaintListener (ComponentPeer* peer, Component* dummy)
{
    if (auto* linuxPeer = dynamic_cast<LinuxComponentPeer<::Window>*> (peer))
        linuxPeer->removeOpenGLRepaintListener (dummy);
}

} // namespace juce
