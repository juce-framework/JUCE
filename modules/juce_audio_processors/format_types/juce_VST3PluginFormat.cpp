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

#if JUCE_INTERNAL_HAS_VST3

#include <juce_audio_processors_headless/format_types/juce_VST3PluginFormatImpl.h>
#include <juce_audio_processors/utilities/juce_NSViewComponentWithParent.h>

namespace juce
{

// UB Sanitizer doesn't necessarily have instrumentation for loaded plugins, so
// it won't recognize the dynamic types of pointers to the plugin's interfaces.
JUCE_BEGIN_NO_SANITIZE ("vptr")

class VST3HostContextWithContextMenu : public VST3HostContextHeadless
{
public:
    Vst::IContextMenu* PLUGIN_API createContextMenu (IPlugView*, const Vst::ParamID*) override
    {
        if (auto* p = getPlugin())
        {
            auto* result = new ContextMenu (*p);
            result->addRef();
            return result;
        }

        return nullptr;
    }

private:
    struct ContextMenu final : public Vst::IContextMenu
    {
        explicit ContextMenu (VST3PluginInstanceHeadless& pluginInstance)  : owner (pluginInstance) {}
        ~ContextMenu() = default;

        JUCE_DECLARE_VST3_COM_REF_METHODS
        JUCE_DECLARE_VST3_COM_QUERY_METHODS

        Steinberg::int32 PLUGIN_API getItemCount() override     { return (Steinberg::int32) items.size(); }

        tresult PLUGIN_API addItem (const Item& item, IContextMenuTarget* target) override
        {
            jassert (target != nullptr);

            ItemAndTarget newItem;
            newItem.item = item;
            newItem.target = VSTComSmartPtr (target, IncrementRef::yes);

            items.add (newItem);
            return kResultOk;
        }

        tresult PLUGIN_API removeItem (const Item& toRemove, IContextMenuTarget* target) override
        {
            for (int i = items.size(); --i >= 0;)
            {
                auto& item = items.getReference (i);

                if (item.item.tag == toRemove.tag && item.target.get() == target)
                    items.remove (i);
            }

            return kResultOk;
        }

        tresult PLUGIN_API getItem (Steinberg::int32 tag, Item& result, IContextMenuTarget** target) override
        {
            for (int i = 0; i < items.size(); ++i)
            {
                auto& item = items.getReference (i);

                if (item.item.tag == tag)
                {
                    result = item.item;

                    if (target != nullptr)
                        *target = item.target.get();

                    return kResultTrue;
                }
            }

            zerostruct (result);
            return kResultFalse;
        }

        tresult PLUGIN_API popup (UCoord x, UCoord y) override
        {
            Array<const Item*> subItemStack;
            OwnedArray<PopupMenu> menuStack;
            PopupMenu* topLevelMenu = menuStack.add (new PopupMenu());

            for (int i = 0; i < items.size(); ++i)
            {
                auto& item = items.getReference (i).item;
                auto* menuToUse = menuStack.getLast();

                if (hasFlag (item.flags, Item::kIsGroupStart & ~Item::kIsDisabled))
                {
                    subItemStack.add (&item);
                    menuStack.add (new PopupMenu());
                }
                else if (hasFlag (item.flags, Item::kIsGroupEnd))
                {
                    if (auto* subItem = subItemStack.getLast())
                    {
                        if (auto* m = menuStack [menuStack.size() - 2])
                            m->addSubMenu (toString (subItem->name), *menuToUse,
                                           ! hasFlag (subItem->flags, Item::kIsDisabled),
                                           nullptr,
                                           hasFlag (subItem->flags, Item::kIsChecked));

                        menuStack.removeLast (1);
                        subItemStack.removeLast (1);
                    }
                }
                else if (hasFlag (item.flags, Item::kIsSeparator))
                {
                    menuToUse->addSeparator();
                }
                else
                {
                    menuToUse->addItem (item.tag != 0 ? (int) item.tag : (int) zeroTagReplacement,
                                        toString (item.name),
                                        ! hasFlag (item.flags, Item::kIsDisabled),
                                        hasFlag (item.flags, Item::kIsChecked));
                }
            }

            PopupMenu::Options options;

            if (auto* ed = owner.getActiveEditor())
            {
               #if JUCE_WINDOWS && JUCE_WIN_PER_MONITOR_DPI_AWARE
                if (auto* peer = ed->getPeer())
                {
                    auto scale = peer->getPlatformScaleFactor();

                    x = roundToInt (x / scale);
                    y = roundToInt (y / scale);
                }
               #endif

                options = options.withTargetScreenArea (ed->getScreenBounds().translated ((int) x, (int) y).withSize (1, 1));
            }

           #if JUCE_MODAL_LOOPS_PERMITTED
            // Unfortunately, Steinberg's docs explicitly say this should be modal.
            handleResult (topLevelMenu->showMenu (options));
           #else
            topLevelMenu->showMenuAsync (options,
                                         ModalCallbackFunction::create (menuFinished,
                                                                        VSTComSmartPtr (this, IncrementRef::yes)));
           #endif

            return kResultOk;
        }

       #if ! JUCE_MODAL_LOOPS_PERMITTED
        static void menuFinished (int modalResult, VSTComSmartPtr<ContextMenu> menu)  { menu->handleResult (modalResult); }
       #endif

    private:
        enum { zeroTagReplacement = 0x7fffffff };

        Atomic<int> refCount;
        VST3PluginInstanceHeadless& owner;

        struct ItemAndTarget
        {
            Item item;
            VSTComSmartPtr<IContextMenuTarget> target;
        };

        Array<ItemAndTarget> items;

        void handleResult (int result)
        {
            if (result == 0)
                return;

            if (result == zeroTagReplacement)
                result = 0;

            for (int i = 0; i < items.size(); ++i)
            {
                auto& item = items.getReference (i);

                if ((int) item.item.tag == result)
                {
                    if (item.target != nullptr)
                        item.target->executeMenuItem ((Steinberg::int32) result);

                    break;
                }
            }
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContextMenu)
    };
};

struct VST3PluginWindow final : public AudioProcessorEditor,
                                public RunLoop,
                                public IPlugFrame,
                                private ComponentMovementWatcher,
                                private ComponentBoundsConstrainer
{
    VST3PluginWindow (AudioPluginInstance* owner, VSTComSmartPtr<IPlugView> pluginView)
        : AudioProcessorEditor (owner),
          ComponentMovementWatcher (this),
          view (pluginView)
         #if JUCE_MAC
        , embeddedComponent (*owner)
         #endif
    {
        setSize (10, 10);
        setOpaque (true);
        setVisible (true);
        setConstrainer (this);

        warnOnFailure (view->setFrame (this));
        view->queryInterface (IPlugViewContentScaleSupport::iid, (void**) &scaleInterface);

        setContentScaleFactor();
        resizeToFit();

        setResizable (view->canResize() == kResultTrue, false);
    }

    ~VST3PluginWindow() override
    {
        if (scaleInterface != nullptr)
            scaleInterface->release();

        #if JUCE_LINUX || JUCE_BSD
         embeddedComponent.removeClient();
        #endif

        if (attachedCalled)
            warnOnFailure (view->removed());

        warnOnFailure (view->setFrame (nullptr));

        processor.editorBeingDeleted (this);

       #if JUCE_MAC
        embeddedComponent.setView (nullptr);
       #endif

        view = nullptr;
    }

    tresult PLUGIN_API queryInterface (const TUID queryIid, void** obj) override
    {
        return testForMultiple (*this,
                                queryIid,
                               #if JUCE_LINUX || JUCE_BSD
                                UniqueBase<Linux::IRunLoop>{},
                               #endif
                                UniqueBase<IPlugFrame>{}).extract (obj);
    }

    JUCE_DECLARE_VST3_COM_REF_METHODS

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel) override
    {
        view->onWheel (wheel.deltaY);
    }

    void focusGained (FocusChangeType) override     { view->onFocus (true); }
    void focusLost (FocusChangeType) override       { view->onFocus (false); }

    /** It seems that most, if not all, plugins do their own keyboard hooks,
        but IPlugView does have a set of keyboard related methods...
    */
    bool keyStateChanged (bool /*isKeyDown*/) override  { return true; }
    bool keyPressed (const KeyPress& /*key*/) override  { return true; }

private:
    void checkBounds (Rectangle<int>& bounds,
                      const Rectangle<int>&,
                      const Rectangle<int>&,
                      bool,
                      bool,
                      bool,
                      bool) override
    {
        auto rect = componentToVST3Rect (bounds);
        auto constrainedRect = rect;
        view->checkSizeConstraint (&constrainedRect);

        // Prevent inadvertent window growth while dragging; see componentMovedOrResized below
        if (constrainedRect.getWidth() != rect.getWidth() || constrainedRect.getHeight() != rect.getHeight())
            bounds = vst3ToComponentRect (constrainedRect);
    }

    //==============================================================================
    void componentPeerChanged() override {}

    /*  Convert from the component's coordinate system to the hosted VST3's coordinate system. */
    ViewRect componentToVST3Rect (Rectangle<int> r) const
    {
        const auto combinedScale = nativeScaleFactor * getDesktopScaleFactor();
        const auto physical = (localAreaToGlobal (r.toFloat()) * combinedScale).toNearestInt();
        return { 0, 0, physical.getWidth(), physical.getHeight() };
    }

    /*  Convert from the hosted VST3's coordinate system to the component's coordinate system. */
    Rectangle<int> vst3ToComponentRect (const ViewRect& vr) const
    {
        const auto combinedScale = nativeScaleFactor * getDesktopScaleFactor();
        const auto floatRect = Rectangle { (float) vr.right, (float) vr.bottom } / combinedScale;
        return getLocalArea (nullptr, floatRect).toNearestInt();
    }

    void componentMovedOrResized (bool, bool wasResized) override
    {
        if (recursiveResize || ! wasResized || getTopLevelComponent()->getPeer() == nullptr)
            return;

        if (view->canResize() == kResultTrue)
        {
            // componentToVST3Rect will apply DPI scaling and round to the nearest integer; vst3ToComponentRect
            // will invert the DPI scaling, but the logical size returned by vst3ToComponentRect may be
            // different from the original size due to floating point rounding if the scale factor is > 100%.
            // This can cause the window to unexpectedly grow while it's moving.
            auto scaledRect = componentToVST3Rect (getLocalBounds());

            auto constrainedRect = scaledRect;
            view->checkSizeConstraint (&constrainedRect);

            const auto tieRect = [] (const auto& x) { return std::tuple (x.getWidth(), x.getHeight()); };

            // Only update the size if the constrained size is actually different
            if (tieRect (constrainedRect) != tieRect (scaledRect))
            {
                const ScopedValueSetter recursiveResizeSetter (recursiveResize, true);

                const auto logicalSize = vst3ToComponentRect (constrainedRect);
                setSize (logicalSize.getWidth(), logicalSize.getHeight());
            }

            embeddedComponent.setBounds (getLocalBounds());

            view->onSize (&constrainedRect);
        }
        else
        {
            ViewRect rect;
            warnOnFailure (view->getSize (&rect));

            resizeWithRect (embeddedComponent, rect);
        }

        // Some plugins don't update their cursor correctly when mousing out the window
        Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
    }

    using ComponentMovementWatcher::componentMovedOrResized;

    void componentVisibilityChanged() override
    {
        attachPluginWindow();
        resizeToFit();
        componentMovedOrResized (true, true);
    }

    using ComponentMovementWatcher::componentVisibilityChanged;

    void resizeToFit()
    {
        ViewRect rect;
        warnOnFailure (view->getSize (&rect));
        resizeWithRect (*this, rect);
    }

    tresult PLUGIN_API resizeView (IPlugView* incomingView, ViewRect* newSize) override
    {
        const ScopedValueSetter<bool> recursiveResizeSetter (recursiveResize, true);

        if (incomingView != nullptr && newSize != nullptr && incomingView == view.get())
        {
            const auto oldPhysicalSize = componentToVST3Rect (getLocalBounds());
            const auto logicalSize = vst3ToComponentRect (*newSize);
            setSize (logicalSize.getWidth(), logicalSize.getHeight());
            embeddedComponent.setSize (logicalSize.getWidth(), logicalSize.getHeight());

           #if JUCE_WINDOWS
            embeddedComponent.updateHWNDBounds();
           #elif JUCE_LINUX || JUCE_BSD
            embeddedComponent.updateEmbeddedBounds();
           #endif

            // According to the VST3 Workflow Diagrams, a resizeView from the plugin should
            // always trigger a response from the host which confirms the new size.
            auto currentPhysicalSize = componentToVST3Rect (getLocalBounds());

            if (currentPhysicalSize.getWidth() != oldPhysicalSize.getWidth()
                || currentPhysicalSize.getHeight() != oldPhysicalSize.getHeight()
                || ! isInOnSize)
            {
                // Guard against plug-ins immediately calling resizeView() with the same size
                const ScopedValueSetter<bool> inOnSizeSetter (isInOnSize, true);
                view->onSize (&currentPhysicalSize);
            }

            return kResultTrue;
        }

        jassertfalse;
        return kInvalidArgument;
    }

    //==============================================================================
    void resizeWithRect (Component& comp, const ViewRect& rect) const
    {
        const auto logicalSize = vst3ToComponentRect (rect);
        comp.setSize (jmax (10, logicalSize.getWidth()),
                      jmax (10, logicalSize.getHeight()));
    }

    void attachPluginWindow()
    {
        if (pluginHandle == HandleFormat{})
        {
            #if JUCE_WINDOWS
             pluginHandle = static_cast<HWND> (embeddedComponent.getHWND());
            #endif

             embeddedComponent.setBounds (getLocalBounds());
             addAndMakeVisible (embeddedComponent);

            #if JUCE_MAC
             pluginHandle = (HandleFormat) embeddedComponent.getView();
            #elif JUCE_LINUX || JUCE_BSD
             pluginHandle = (HandleFormat) embeddedComponent.getHostWindowID();
            #endif

            if (pluginHandle == HandleFormat{})
            {
                jassertfalse;
                return;
            }

            [[maybe_unused]] const auto attachedResult = view->attached ((void*) pluginHandle, defaultVST3WindowType);
            [[maybe_unused]] const auto warning = warnOnFailure (attachedResult);

            if (attachedResult == kResultOk)
                attachedCalled = true;

            updatePluginScale();

           #if JUCE_WINDOWS
            // Make sure the embedded component window is the right size
            // and invalidate the embedded HWND and any child windows
            embeddedComponent.updateHWNDBounds();
           #endif
        }
    }

    void updatePluginScale()
    {
        if (scaleInterface != nullptr)
            setContentScaleFactor();
        else
            resizeToFit();
    }

    void setContentScaleFactor()
    {
        if (scaleInterface != nullptr)
        {
            [[maybe_unused]] const auto result = scaleInterface->setContentScaleFactor ((IPlugViewContentScaleSupport::ScaleFactor) getEffectiveScale());

           #if ! JUCE_MAC
            [[maybe_unused]] const auto warning = warnOnFailure (result);
           #endif
        }
    }

    void setScaleFactor (float s) override
    {
        userScaleFactor = s;
        setContentScaleFactor();
        resizeToFit();
    }

    float getEffectiveScale() const
    {
        return nativeScaleFactor * userScaleFactor;
    }

    //==============================================================================
    Atomic<int> refCount { 1 };
    VSTComSmartPtr<IPlugView> view;

   #if JUCE_WINDOWS
    using HandleFormat = HWND;

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

    ViewComponent embeddedComponent;
   #elif JUCE_MAC
    NSViewComponentWithParent embeddedComponent;
    using HandleFormat = NSView*;
   #elif JUCE_LINUX || JUCE_BSD
    XEmbedComponent embeddedComponent { true, false };
    using HandleFormat = Window;
   #else
    Component embeddedComponent;
    using HandleFormat = void*;
   #endif

    HandleFormat pluginHandle = {};
    bool recursiveResize = false, isInOnSize = false, attachedCalled = false;

    IPlugViewContentScaleSupport* scaleInterface = nullptr;
    float nativeScaleFactor = 1.0f;
    float userScaleFactor = 1.0f;

    struct ScaleNotifierCallback
    {
        VST3PluginWindow& window;

        void operator() (float platformScale) const
        {
            MessageManager::callAsync ([ref = Component::SafePointer<VST3PluginWindow> (&window), platformScale]
            {
                if (auto* r = ref.getComponent())
                {
                    r->nativeScaleFactor = platformScale;
                    r->setContentScaleFactor();
                    r->resizeToFit();

                   #if JUCE_WINDOWS
                    r->embeddedComponent.updateHWNDBounds();
                   #elif JUCE_LINUX || JUCE_BSD
                    r->embeddedComponent.updateEmbeddedBounds();
                   #endif
                }
            });
        }
    };

    NativeScaleFactorNotifier scaleNotifier { this, ScaleNotifierCallback { *this } };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3PluginWindow)
};

class VST3PluginInstance final : public VST3PluginInstanceHeadless
{
public:
    using VST3PluginInstanceHeadless::VST3PluginInstanceHeadless;

    bool hasEditor() const override
    {
        // (if possible, avoid creating a second instance of the editor, because that crashes some plugins)
        if (getActiveEditor() != nullptr)
            return true;

        VSTComSmartPtr view { tryCreatingView(), IncrementRef::no };
        return view != nullptr;
    }

    VST3PluginWindow* createEditor() override
    {
        if (VSTComSmartPtr view { tryCreatingView(), IncrementRef::no })
            return new VST3PluginWindow (this, view);

        return nullptr;
    }
};

void VST3PluginFormat::createPluginInstance (const PluginDescription& description,
                                             double,
                                             int,
                                             PluginCreationCallback callback)
{
    createVst3InstanceImpl<VST3PluginInstance> (*this,
                                                { new VST3HostContextWithContextMenu, IncrementRef::no },
                                                description,
                                                callback);
}

JUCE_END_NO_SANITIZE

} // namespace juce

#endif
