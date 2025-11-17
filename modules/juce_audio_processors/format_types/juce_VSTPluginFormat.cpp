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

#if JUCE_INTERNAL_HAS_VST

#include <juce_audio_processors_headless/format_types/juce_VSTPluginFormatImpl.h>
#include <juce_audio_processors/utilities/juce_NSViewComponentWithParent.h>

namespace juce
{

#if JUCE_LINUX || JUCE_BSD

using EventProcPtr = void (*)(XEvent*);

static Window getChildWindow (Window windowToCheck)
{
    Window rootWindow, parentWindow;
    Window* childWindows;
    unsigned int numChildren = 0;

    X11Symbols::getInstance()->xQueryTree (XWindowSystem::getInstance()->getDisplay(),
                                           windowToCheck, &rootWindow, &parentWindow, &childWindows, &numChildren);

    if (numChildren > 0)
        return childWindows [0];

    return 0;
}

#endif

//==============================================================================
#if ! (JUCE_IOS || JUCE_ANDROID)
struct VSTPluginWindow;
static Array<VSTPluginWindow*> activeVSTWindows;

//==============================================================================
struct VSTPluginWindow final : public AudioProcessorEditor,
                              #if ! JUCE_MAC
                               private ComponentMovementWatcher,
                              #endif
                               private Timer
{
public:
    explicit VSTPluginWindow (VSTPluginInstanceHeadless& plug)
        : AudioProcessorEditor (&plug),
         #if ! JUCE_MAC
          ComponentMovementWatcher (this),
         #endif
          plugin (plug)
    {
       #if JUCE_LINUX || JUCE_BSD
        pluginWindow = None;
        ignoreUnused (pluginRefusesToResize, alreadyInside);
       #elif JUCE_MAC
        ignoreUnused (recursiveResize, pluginRefusesToResize, alreadyInside);

        cocoaWrapper.reset (new NSViewComponentWithParent (plugin));
        addAndMakeVisible (cocoaWrapper.get());
       #endif

        activeVSTWindows.add (this);

        Vst2::ERect* rect = nullptr;
        dispatch (Vst2::effEditGetRect, 0, 0, &rect, 0);

        if (rect != nullptr)
            updateSizeFromEditor (rect->right - rect->left, rect->bottom - rect->top);
        else
            updateSizeFromEditor (1, 1);

        setOpaque (true);
        setVisible (true);

       #if JUCE_WINDOWS
        addAndMakeVisible (embeddedComponent);
       #endif
    }

    ~VSTPluginWindow() override
    {
        activeVSTWindows.removeFirstMatchingValue (this);

        closePluginWindow();

       #if JUCE_MAC
        cocoaWrapper.reset();
       #endif

        plugin.editorBeingDeleted (this);
    }

    //==============================================================================
    /*  Convert from the hosted VST's coordinate system to the component's coordinate system. */
    Rectangle<int> vstToComponentRect (Component& editor, const Rectangle<int>& vr) const
    {
        return editor.getLocalArea (nullptr, vr / (nativeScaleFactor * getDesktopScaleFactor()));
    }

    Rectangle<int> componentToVstRect (Component& editor, const Rectangle<int>& vr) const
    {
        if (auto* tl = editor.getTopLevelComponent())
            return tl->getLocalArea (&editor, vr) * nativeScaleFactor * tl->getDesktopScaleFactor();

        return {};
    }

    bool updateSizeFromEditor (int w, int h)
    {
        const auto correctedBounds = vstToComponentRect (*this, { w, h });
        setSize (correctedBounds.getWidth(), correctedBounds.getHeight());

      #if JUCE_MAC
        if (cocoaWrapper != nullptr)
            cocoaWrapper->setSize (correctedBounds.getWidth(), correctedBounds.getHeight());
      #endif

        return true;
    }

   #if JUCE_MAC
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void visibilityChanged() override
    {
        if (isShowing())
            openPluginWindow ((NSView*) cocoaWrapper->getView());
        else
            closePluginWindow();
    }

    void childBoundsChanged (Component*) override
    {
        auto w = cocoaWrapper->getWidth();
        auto h = cocoaWrapper->getHeight();

        if (w != getWidth() || h != getHeight())
            setSize (w, h);
    }

    void parentHierarchyChanged() override { visibilityChanged(); }
   #else
    float getEffectiveScale() const
    {
        return nativeScaleFactor * userScaleFactor;
    }

    void paint (Graphics& g) override
    {
       #if JUCE_LINUX || JUCE_BSD
        if (isOpen)
        {
            if (pluginWindow != 0)
            {
                auto clip = componentToVstRect (*this, g.getClipBounds().toNearestInt());

                X11Symbols::getInstance()->xClearArea (display, pluginWindow, clip.getX(), clip.getY(),
                                                       static_cast<unsigned int> (clip.getWidth()),
                                                       static_cast<unsigned int> (clip.getHeight()), True);
            }
        }
        else
       #endif
        {
            g.fillAll (Colours::black);
        }
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        if (recursiveResize)
            return;

        if (getPeer() != nullptr)
        {
            const ScopedValueSetter<bool> recursiveResizeSetter (recursiveResize, true);

           #if JUCE_WINDOWS
            embeddedComponent.setBounds (getLocalBounds());
           #elif JUCE_LINUX || JUCE_BSD
            const auto pos = componentToVstRect (*this, getLocalBounds());

            if (pluginWindow != 0)
            {
                auto* symbols = X11Symbols::getInstance();
                symbols->xMoveResizeWindow (display,
                                            pluginWindow,
                                            pos.getX(),
                                            pos.getY(),
                                            (unsigned int) pos.getWidth(),
                                            (unsigned int) pos.getHeight());
                symbols->xMapRaised (display, pluginWindow);
                symbols->xFlush (display);
            }
           #endif
        }
    }

    using ComponentMovementWatcher::componentMovedOrResized;

    void componentVisibilityChanged() override
    {
        if (isShowing())
            openPluginWindow();
        else if (! shouldAvoidDeletingWindow())
            closePluginWindow();

        setContentScaleFactor();

       #if JUCE_LINUX || JUCE_BSD
        MessageManager::callAsync ([safeThis = SafePointer<VSTPluginWindow> { this }]
        {
            if (safeThis != nullptr)
                safeThis->componentMovedOrResized (true, true);
        });
       #else
        componentMovedOrResized (true, true);
       #endif
    }

    using ComponentMovementWatcher::componentVisibilityChanged;

    void componentPeerChanged() override
    {
        closePluginWindow();

        if (getPeer() != nullptr)
        {
            openPluginWindow();
            componentMovedOrResized (true, true);
        }
    }

    void setContentScaleFactor()
    {
        if (pluginRespondsToDPIChanges)
            dispatch (Vst2::effVendorSpecific,
                      (int) ByteOrder::bigEndianInt ("PreS"),
                      (int) ByteOrder::bigEndianInt ("AeCs"),
                      nullptr, getEffectiveScale());
    }
   #endif

    void setScaleFactor (float scale) override
    {
        userScaleFactor = scale;

       #if ! JUCE_MAC
        setContentScaleFactor();
       #endif

       #if JUCE_WINDOWS
        resizeToFit();
       #endif
    }

    //==============================================================================
    bool keyStateChanged (bool) override                 { return pluginWantsKeys; }
    bool keyPressed (const juce::KeyPress&) override     { return pluginWantsKeys; }

    //==============================================================================
    void timerCallback() override
    {
        if (isShowing())
        {
           #if JUCE_WINDOWS
            if (--sizeCheckCount <= 0)
            {
                sizeCheckCount = 10;
                checkPluginWindowSize();
            }
           #endif

            static bool reentrantGuard = false;

            if (! reentrantGuard)
            {
                reentrantGuard = true;

               #if JUCE_WINDOWS
                // Some plugins may draw/resize inside their idle callback, so ensure that
                // DPI awareness is set correctly inside this call.
                ScopedThreadDPIAwarenessSetter scope (getPluginHWND());
               #endif
                plugin.dispatch (Vst2::effEditIdle, 0, 0, nullptr, 0);

                reentrantGuard = false;
            }

           #if JUCE_LINUX || JUCE_BSD
            if (pluginWindow == 0)
            {
                updatePluginWindowHandle();

                if (pluginWindow != 0)
                    componentMovedOrResized (true, true);
            }
           #endif
        }
    }

    //==============================================================================
    void mouseDown ([[maybe_unused]] const MouseEvent& e) override
    {
       #if JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD
        toFront (true);
       #endif
    }

    void broughtToFront() override
    {
        if (activeVSTWindows.removeFirstMatchingValue (this) != -1)
            activeVSTWindows.add (this);

       #if JUCE_MAC
        dispatch (Vst2::effEditTop, 0, 0, nullptr, 0);
       #endif
    }

private:
    //==============================================================================
    // This is a workaround for old Mackie plugins that crash if their
    // window is deleted more than once.
    bool shouldAvoidDeletingWindow() const
    {
        return plugin.getPluginDescription()
                .manufacturerName.containsIgnoreCase ("Loud Technologies");
    }

    // This is an old workaround for some plugins that need a repaint when their
    // windows are first created, but it breaks some Izotope plugins.
    bool shouldRepaintCarbonWindowWhenCreated()
    {
        return ! plugin.getName().containsIgnoreCase ("izotope");
    }

    //==============================================================================
   #if JUCE_MAC
    void openPluginWindow (void* parentWindow)
    {
        if (isOpen || parentWindow == nullptr)
            return;

        isOpen = true;

        Vst2::ERect* rect = nullptr;
        dispatch (Vst2::effEditGetRect, 0, 0, &rect, 0);
        dispatch (Vst2::effEditOpen, 0, 0, parentWindow, 0);

        // do this before and after like in the steinberg example
        dispatch (Vst2::effEditGetRect, 0, 0, &rect, 0);
        dispatch (Vst2::effGetProgram, 0, 0, nullptr, 0); // also in steinberg code

        // Install keyboard hooks
        pluginWantsKeys = (dispatch (Vst2::effKeysRequired, 0, 0, nullptr, 0) == 0);

        // double-check it's not too tiny
        int w = 250, h = 150;

        if (rect != nullptr)
        {
            w = rect->right - rect->left;
            h = rect->bottom - rect->top;

            if (w == 0 || h == 0)
            {
                w = 250;
                h = 150;
            }
        }

        w = jmax (w, 32);
        h = jmax (h, 32);

        updateSizeFromEditor (w, h);

        startTimer (18 + juce::Random::getSystemRandom().nextInt (5));
        repaint();
    }
   #else
    void openPluginWindow()
    {
        if (isOpen)
            return;

        JUCE_VST_LOG ("Opening VST UI: " + plugin.getName());
        isOpen = true;

        pluginRespondsToDPIChanges = plugin.pluginCanDo ("supportsViewDpiScaling") > 0;

        setContentScaleFactor();

        Vst2::ERect* rect = nullptr;

        dispatch (Vst2::effEditGetRect, 0, 0, &rect, 0);

       #if JUCE_WINDOWS
        auto* handle = embeddedComponent.getHWND();
       #else
        auto* handle = getWindowHandle();
       #endif
        dispatch (Vst2::effEditOpen, 0, 0, handle, 0);
        dispatch (Vst2::effEditGetRect, 0, 0, &rect, 0);  // do this before and after like in the steinberg example
        dispatch (Vst2::effGetProgram, 0, 0, nullptr, 0); // also in steinberg code

        pluginWantsKeys = (dispatch (Vst2::effKeysRequired, 0, 0, nullptr, 0) == 0);

       #if JUCE_WINDOWS
        originalWndProc = nullptr;
        auto* pluginHWND = getPluginHWND();

        if (pluginHWND == nullptr)
        {
            isOpen = false;
            setSize (300, 150);
            return;
        }

        JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4244)

        if (! pluginWantsKeys)
        {
            originalWndProc = (void*) GetWindowLongPtr (pluginHWND, GWLP_WNDPROC);
            SetWindowLongPtr (pluginHWND, GWLP_WNDPROC, (LONG_PTR) vstHookWndProc);
        }

        JUCE_END_IGNORE_WARNINGS_MSVC

        RECT r;

        {
            ScopedThreadDPIAwarenessSetter threadDpiAwarenessSetter { pluginHWND };
            GetWindowRect (pluginHWND, &r);
        }

        auto w = (int) (r.right - r.left);
        auto h = (int) (r.bottom - r.top);

        if (rect != nullptr)
        {
            auto rw = rect->right - rect->left;
            auto rh = rect->bottom - rect->top;

            if ((rw > 50 && rh > 50 && rw < 2000 && rh < 2000 && (! isWithin (w, rw, 2) || ! isWithin (h, rh, 2)))
                || ((w == 0 && rw > 0) || (h == 0 && rh > 0)))
            {
                // very dodgy logic to decide which size is right
                if (std::abs (rw - w) > 350 || std::abs (rh - h) > 350)
                {
                    ScopedThreadDPIAwarenessSetter threadDpiAwarenessSetter { pluginHWND };

                    SetWindowPos (pluginHWND, nullptr,
                                  0, 0, rw, rh,
                                  SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);

                    GetWindowRect (pluginHWND, &r);

                    w = r.right - r.left;
                    h = r.bottom - r.top;

                    pluginRefusesToResize = (w != rw) || (h != rh);

                    w = rw;
                    h = rh;
                }
            }
        }
       #elif JUCE_LINUX || JUCE_BSD
        updatePluginWindowHandle();

        int w = 250, h = 150;

        if (rect != nullptr)
        {
            w = rect->right - rect->left;
            h = rect->bottom - rect->top;

            if (w == 0 || h == 0)
            {
                w = 250;
                h = 150;
            }
        }

        if (pluginWindow != 0)
            X11Symbols::getInstance()->xMapRaised (display, pluginWindow);
       #endif

        // double-check it's not too tiny
        w = jmax (w, 32);
        h = jmax (h, 32);

        updateSizeFromEditor (w, h);

       #if JUCE_WINDOWS
        checkPluginWindowSize();
       #endif

        startTimer (18 + juce::Random::getSystemRandom().nextInt (5));
        repaint();
    }
   #endif

    //==============================================================================
    void closePluginWindow()
    {
        if (isOpen)
        {
            // You shouldn't end up hitting this assertion unless the host is trying to do GUI
            // cleanup on a non-GUI thread. If it does that, bad things could happen in here.
            JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

            JUCE_VST_LOG ("Closing VST UI: " + plugin.getName());
            isOpen = false;
            dispatch (Vst2::effEditClose, 0, 0, nullptr, 0);
            stopTimer();

           #if JUCE_WINDOWS
            JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4244)
            auto* pluginHWND = getPluginHWND();

            if (originalWndProc != nullptr && pluginHWND != nullptr && IsWindow (pluginHWND))
                SetWindowLongPtr (pluginHWND, GWLP_WNDPROC, (LONG_PTR) originalWndProc);
            JUCE_END_IGNORE_WARNINGS_MSVC

            originalWndProc = nullptr;
           #elif JUCE_LINUX || JUCE_BSD
            pluginWindow = 0;
           #endif
        }
    }

    //==============================================================================
    pointer_sized_int dispatch (const int opcode, const int index, const int value, void* const ptr, float opt)
    {
        return plugin.dispatch (opcode, index, value, ptr, opt);
    }

    //==============================================================================
   #if JUCE_WINDOWS
    bool isWindowSizeCorrectForPlugin (int w, int h)
    {
        if (pluginRefusesToResize)
            return true;

        const auto converted = vstToComponentRect (*this, { w, h });
        return (isWithin (converted.getWidth(), getWidth(), 5) && isWithin (converted.getHeight(), getHeight(), 5));
    }

    void resizeToFit()
    {
        Vst2::ERect* rect = nullptr;
        dispatch (Vst2::effEditGetRect, 0, 0, &rect, 0);

        auto w = rect->right - rect->left;
        auto h = rect->bottom - rect->top;

        if (! isWindowSizeCorrectForPlugin (w, h))
        {
            updateSizeFromEditor (w, h);
            embeddedComponent.updateHWNDBounds();
            sizeCheckCount = 0;
        }
    }

    void checkPluginWindowSize()
    {
        if (! pluginRespondsToDPIChanges)
            resizeToFit();
    }

    // hooks to get keyboard events from VST windows
    static LRESULT CALLBACK vstHookWndProc (HWND hW, UINT message, WPARAM wParam, LPARAM lParam)
    {
        for (int i = activeVSTWindows.size(); --i >= 0;)
        {
            Component::SafePointer<VSTPluginWindow> w (activeVSTWindows[i]);

            auto* pluginHWND = w->getPluginHWND();

            if (w != nullptr && pluginHWND == hW)
            {
                if (message == WM_CHAR
                    || message == WM_KEYDOWN
                    || message == WM_SYSKEYDOWN
                    || message == WM_KEYUP
                    || message == WM_SYSKEYUP
                    || message == WM_APPCOMMAND)
                {
                    SendMessage ((HWND) w->getTopLevelComponent()->getWindowHandle(),
                                 message, wParam, lParam);
                }

                if (w != nullptr) // (may have been deleted in SendMessage callback)
                    return CallWindowProc ((WNDPROC) w->originalWndProc,
                                           (HWND) pluginHWND,
                                           message, wParam, lParam);
            }
        }

        return DefWindowProc (hW, message, wParam, lParam);
    }
   #endif

   #if JUCE_LINUX || JUCE_BSD
    void updatePluginWindowHandle()
    {
        pluginWindow = getChildWindow ((Window) getWindowHandle());
    }
   #endif

    //==============================================================================
    #if JUCE_MAC
      std::unique_ptr<NSViewComponentWithParent> cocoaWrapper;

      void resized() override
      {
      }
    #endif

    //==============================================================================
    VSTPluginInstanceHeadless& plugin;
    float userScaleFactor = 1.0f;
    bool isOpen = false, recursiveResize = false;
    bool pluginWantsKeys = false, pluginRefusesToResize = false, alreadyInside = false;

   #if ! JUCE_MAC
    bool pluginRespondsToDPIChanges = false;

    float nativeScaleFactor = 1.0f;

    struct ScaleNotifierCallback
    {
        VSTPluginWindow& window;

        void operator() (float platformScale) const
        {
            MessageManager::callAsync ([ref = Component::SafePointer<VSTPluginWindow> (&window), platformScale]
            {
                if (auto* r = ref.getComponent())
                {
                    r->nativeScaleFactor = platformScale;
                    r->setContentScaleFactor();

                   #if JUCE_WINDOWS
                    r->resizeToFit();
                    r->embeddedComponent.updateHWNDBounds();
                   #endif
                    r->componentMovedOrResized (true, true);
                }
            });
        }
    };

    NativeScaleFactorNotifier scaleNotifier { this, ScaleNotifierCallback { *this } };

    #if JUCE_WINDOWS
     struct ViewComponent final : public HWNDComponent
     {
         ViewComponent()
         {
             setOpaque (true);
             inner.addToDesktop (0);

             if (auto* peer = inner.getPeer())
                 setHWND (peer->getNativeHandle());
         }

         void paint (Graphics& g) override { g.fillAll (Colours::black); }

     private:
         struct Inner final : public Component
         {
             Inner() { setOpaque (true); }
             void paint (Graphics& g) override { g.fillAll (Colours::black); }
         };

         Inner inner;
     };

     HWND getPluginHWND() const
     {
         return GetWindow ((HWND) embeddedComponent.getHWND(), GW_CHILD);
     }

     ViewComponent embeddedComponent;
     void* originalWndProc = {};
     int sizeCheckCount = 0;
    #elif JUCE_LINUX || JUCE_BSD
     ::Display* display = XWindowSystem::getInstance()->getDisplay();
     Window pluginWindow = 0;
    #endif
   #else
     static constexpr auto nativeScaleFactor = 1.0f;
   #endif

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VSTPluginWindow)
};
#endif

class VSTPluginInstance final : public VSTPluginInstanceHeadless,
                                private Timer,
                                private AsyncUpdater
{
public:
    using VSTPluginInstanceHeadless::VSTPluginInstanceHeadless;

   #if ! JUCE_IOS && ! JUCE_ANDROID
    bool hasEditor() const override
    {
        return vstEffect != nullptr && (vstEffect->flags & Vst2::effFlagsHasEditor) != 0;
    }

    VSTPluginWindow* createEditor() override
    {
        return hasEditor() ? new VSTPluginWindow (*this)
                           : nullptr;
    }

    void handleIdle() override
    {
        if (insideVSTCallback == 0 && MessageManager::getInstance()->isThisTheMessageThread())
        {
            const IdleCallRecursionPreventer icrp;

            #if JUCE_MAC
            if (getActiveEditor() != nullptr)
                dispatch (Vst2::effEditIdle, 0, 0, nullptr, 0);
            #endif

            Timer::callPendingTimersSynchronously();
            handleUpdateNowIfNeeded();

            for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
                if (auto* p = ComponentPeer::getPeer (i))
                    p->performAnyPendingRepaintsNow();
        }
    }

    void updateDisplay() override
    {
        triggerAsyncUpdate();
    }

    void needIdle() override
    {
        startTimer (50);
    }

    bool updateSizeFromEditor (int w, int h) override
    {
        if (auto* editor = dynamic_cast<VSTPluginWindow*> (getActiveEditor()))
            return editor->updateSizeFromEditor (w, h);

        return false;
    }
   #endif

private:
    void timerCallback() override
    {
        if (dispatch (Vst2::effIdle, 0, 0, nullptr, 0) == 0)
            stopTimer();
    }

    void handleAsyncUpdate() override
    {
        updateHostDisplay (AudioProcessorListener::ChangeDetails().withProgramChanged (true)
                                                                  .withParameterInfoChanged (true));
    }

};

//==============================================================================
void VSTPluginFormat::createPluginInstance (const PluginDescription& desc,
                                            double sampleRate,
                                            int blockSize,
                                            PluginCreationCallback callback)
{
    createVstPluginInstance<VSTPluginInstance> (*this, desc, sampleRate, blockSize, callback);
}

} // namespace juce

#endif
