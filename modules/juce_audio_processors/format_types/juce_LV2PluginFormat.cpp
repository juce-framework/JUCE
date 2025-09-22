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

#if JUCE_INTERNAL_HAS_LV2

#include <juce_audio_processors_headless/format_types/juce_LV2PluginFormatImpl.h>
#include <juce_gui_extra/native/juce_NSViewFrameWatcher_mac.h>
#include <juce_audio_processors/utilities/juce_NSViewComponentWithParent.h>

namespace juce
{
namespace lv2_host
{

class UiFeaturesData
{
public:
    UiFeaturesData (PhysicalResizeListener& rl,
                    TouchListener& tl,
                    LV2_Handle instanceIn,
                    LV2UI_Widget parentIn,
                    Instance::GetExtensionData getExtensionData,
                    const Ports& ports,
                    SymbolMap& symapIn,
                    const UiFeaturesDataOptions& optIn)
        : opts (optIn),
          resizeListener (rl),
          touchListener (tl),
          instance (instanceIn),
          parent (parentIn),
          symap (symapIn),
          dataAccess { getExtensionData },
          portIndices (makePortIndices (ports))
    {
    }

    const LV2_Feature* const* getFeatureArray() const noexcept { return features.pointers.data(); }

    Rectangle<int> getLastRequestedBounds() const   { return { lastRequestedWidth, lastRequestedHeight }; }

private:
    int resizeCallback (int width, int height)
    {
        lastRequestedWidth = width;
        lastRequestedHeight = height;
        resizeListener.viewRequestedResizeInPhysicalPixels (width, height);
        return 0;
    }

    static int resizeCallback (LV2UI_Feature_Handle handle, int width, int height)
    {
        return static_cast<UiFeaturesData*> (handle)->resizeCallback (width, height);
    }

    uint32_t portIndexCallback (const char* symbol) const
    {
        const auto it = portIndices.find (symbol);
        return it != portIndices.cend() ? it->second : LV2UI_INVALID_PORT_INDEX;
    }

    static uint32_t portIndexCallback (LV2UI_Feature_Handle handle, const char* symbol)
    {
        return static_cast<const UiFeaturesData*> (handle)->portIndexCallback (symbol);
    }

    void touchCallback (uint32_t portIndex, bool grabbed) const
    {
        touchListener.controlGrabbed (portIndex, grabbed);
    }

    static void touchCallback (LV2UI_Feature_Handle handle, uint32_t index, bool b)
    {
        return static_cast<const UiFeaturesData*> (handle)->touchCallback (index, b);
    }

    static std::map<String, uint32_t> makePortIndices (const Ports& ports)
    {
        std::map<String, uint32_t> result;

        ports.forEachPort ([&] (const PortHeader& header)
        {
            [[maybe_unused]] const auto emplaced = result.emplace (header.symbol, header.index);

            // This will complain if there are duplicate port symbols.
            jassert (emplaced.second);
        });

        return result;
    }

    const UiFeaturesDataOptions opts;
    PhysicalResizeListener& resizeListener;
    TouchListener& touchListener;
    LV2_Handle instance{};
    LV2UI_Widget parent{};
    SymbolMap& symap;
    const UsefulUrids urids { symap };
    Log log { &urids };
    int lastRequestedWidth = 0, lastRequestedHeight = 0;
    std::vector<LV2_Options_Option> options { { LV2_OPTIONS_INSTANCE,
                                                0,
                                                symap.map (LV2_UI__scaleFactor),
                                                sizeof (float),
                                                symap.map (LV2_ATOM__Float),
                                                &opts.initialScaleFactor },
                                              { LV2_OPTIONS_INSTANCE,
                                                0,
                                                symap.map (LV2_PARAMETERS__sampleRate),
                                                sizeof (float),
                                                symap.map (LV2_ATOM__Float),
                                                &opts.sampleRate },
                                              { LV2_OPTIONS_INSTANCE, 0, 0, 0, 0, nullptr } }; // The final entry must be nulled out
    LV2UI_Resize resize { this, resizeCallback };
    LV2_URID_Map map        = symap.getMapFeature();
    LV2_URID_Unmap unmap    = symap.getUnmapFeature();
    LV2UI_Port_Map portMap { this, portIndexCallback };
    LV2UI_Touch touch { this, touchCallback };
    LV2_Extension_Data_Feature dataAccess;
    std::map<String, uint32_t> portIndices;
    Features features { UiFeatureUris::makeFeatures (&resize,
                                                     parent,
                                                     instance,
                                                     &dataAccess,
                                                     &map,
                                                     &unmap,
                                                     &portMap,
                                                     &touch,
                                                     options.data(),
                                                     log.getLogFeature()) };

    JUCE_LEAK_DETECTOR (UiFeaturesData)
};

/*
    Creates and holds a UI instance for a plugin with a specific URI, using the provided descriptor.
*/
class UiInstance
{
public:
    UiInstance (World& world,
                const UiDescriptor* descriptorIn,
                const UiInstanceArgs& args,
                const LV2_Feature* const* features,
                MessageBufferInterface<MessageHeader>& messagesIn,
                SymbolMap& map,
                PhysicalResizeListener& rl)
        : descriptor (descriptorIn),
          resizeListener (rl),
          uiToProcessor (messagesIn),
          mLV2_UI__floatProtocol   (map.map (LV2_UI__floatProtocol)),
          mLV2_ATOM__atomTransfer  (map.map (LV2_ATOM__atomTransfer)),
          mLV2_ATOM__eventTransfer (map.map (LV2_ATOM__eventTransfer)),
          instance (makeInstance (args, features)),
          idleCallback (getExtensionData<LV2UI_Idle_Interface> (world, LV2_UI__idleInterface))
    {
        jassert (descriptor != nullptr);
        jassert (widget != nullptr);

        ignoreUnused (resizeListener);
    }

    LV2UI_Handle getHandle() const noexcept { return instance.get(); }

    void pushMessage (MessageHeader header, uint32_t size, const void* buffer)
    {
        descriptor->portEvent (getHandle(), header.portIndex, size, header.protocol, buffer);
    }

    int idle()
    {
        if (idleCallback.valid && idleCallback.extension.idle != nullptr)
            return idleCallback.extension.idle (getHandle());

        return 0;
    }

    template <typename Extension>
    OptionalExtension<Extension> getExtensionData (World& world, const char* uid) const
    {
        return descriptor->getExtensionData<Extension> (world, uid);
    }

    Rectangle<int> getDetectedViewBounds() const
    {
       #if JUCE_MAC
        const auto frame = [(NSView*) widget frame];
        return { (int) frame.size.width, (int) frame.size.height };
       #elif JUCE_LINUX || JUCE_BSD
        Window root = 0;
        int wx = 0, wy = 0;
        unsigned int ww = 0, wh = 0, bw = 0, bitDepth = 0;

        XWindowSystemUtilities::ScopedXLock xLock;
        auto* display = XWindowSystem::getInstance()->getDisplay();
        X11Symbols::getInstance()->xGetGeometry (display,
                                                 (::Drawable) widget,
                                                 &root,
                                                 &wx,
                                                 &wy,
                                                 &ww,
                                                 &wh,
                                                 &bw,
                                                 &bitDepth);

        return { (int) ww, (int) wh };
       #elif JUCE_WINDOWS
        RECT rect;
        GetWindowRect ((HWND) widget, &rect);
        return { rect.right - rect.left, rect.bottom - rect.top };
       #else
        return {};
       #endif
    }

    const UiDescriptor* descriptor = nullptr;

private:
    using Instance = std::unique_ptr<void, void (*) (LV2UI_Handle)>;
    using Idle = int (*) (LV2UI_Handle);

    Instance makeInstance (const UiInstanceArgs& args, const LV2_Feature* const* features)
    {
        if (descriptor->get() == nullptr)
            return { nullptr, [] (LV2UI_Handle) {} };

        return Instance { descriptor->get()->instantiate (descriptor->get(),
                                                          args.pluginUri.toString (true).toRawUTF8(),
                                                          File::addTrailingSeparator (args.bundlePath.getFullPathName()).toRawUTF8(),
                                                          writeFunction,
                                                          this,
                                                          &widget,
                                                          features),
                          descriptor->get()->cleanup };
    }

    void write (uint32_t portIndex, uint32_t bufferSize, uint32_t protocol, const void* buffer)
    {
        const LV2_URID protocols[] { 0, mLV2_UI__floatProtocol, mLV2_ATOM__atomTransfer, mLV2_ATOM__eventTransfer };
        const auto it = std::find (std::begin (protocols), std::end (protocols), protocol);

        if (it != std::end (protocols))
        {
            uiToProcessor.pushMessage ({ portIndex, protocol }, bufferSize, buffer);
        }
    }

    static void writeFunction (LV2UI_Controller controller,
                               uint32_t portIndex,
                               uint32_t bufferSize,
                               uint32_t portProtocol,
                               const void* buffer)
    {
        jassert (controller != nullptr);
        static_cast<UiInstance*> (controller)->write (portIndex, bufferSize, portProtocol, buffer);
    }

    PhysicalResizeListener& resizeListener;
    MessageBufferInterface<MessageHeader>& uiToProcessor;
    LV2UI_Widget widget = nullptr;
    const LV2_URID mLV2_UI__floatProtocol;
    const LV2_URID mLV2_ATOM__atomTransfer;
    const LV2_URID mLV2_ATOM__eventTransfer;
    Instance instance;
    OptionalExtension<LV2UI_Idle_Interface> idleCallback;

   #if JUCE_MAC
    NSViewFrameWatcher frameWatcher { (NSView*) widget, [this]
    {
        const auto bounds = getDetectedViewBounds();
        resizeListener.viewRequestedResizeInPhysicalPixels (bounds.getWidth(), bounds.getHeight());
    } };
   #elif JUCE_WINDOWS
    WindowSizeChangeListener frameWatcher { (HWND) widget, resizeListener };
   #endif

    JUCE_LEAK_DETECTOR (UiInstance)
};

class AsyncFn final : public AsyncUpdater
{
public:
    explicit AsyncFn (std::function<void()> callbackIn)
        : callback (std::move (callbackIn)) {}

    ~AsyncFn() override { cancelPendingUpdate(); }

    void handleAsyncUpdate() override { callback(); }

private:
    std::function<void()> callback;
};

class UiInstanceWithSupports
{
public:
    UiInstanceWithSupports (World& world,
                            PhysicalResizeListener& resizeListener,
                            TouchListener& touchListener,
                            const UiDescriptor* descriptor,
                            const UiInstanceArgs& args,
                            LV2UI_Widget parent,
                            InstanceWithSupports& engineInstance,
                            const UiFeaturesDataOptions& opts)
        : features (resizeListener,
                    touchListener,
                    engineInstance.instance.getHandle(),
                    parent,
                    engineInstance.instance.getExtensionDataCallback(),
                    engineInstance.ports,
                    *engineInstance.symap,
                    opts),
          instance (world,
                    descriptor,
                    args,
                    features.getFeatureArray(),
                    engineInstance.uiToProcessor,
                    *engineInstance.symap,
                    resizeListener)
    {}

    UiFeaturesData features;
    UiInstance instance;

    JUCE_LEAK_DETECTOR (UiInstanceWithSupports)
};

class PeerChangedListener final : private ComponentMovementWatcher
{
public:
    PeerChangedListener (Component& c, std::function<void()> peerChangedIn)
        : ComponentMovementWatcher (&c), peerChanged (std::move (peerChangedIn))
    {
    }

    void componentMovedOrResized (bool, bool) override {}
    void componentPeerChanged() override { NullCheckedInvocation::invoke (peerChanged); }
    void componentVisibilityChanged() override {}

    using ComponentMovementWatcher::componentVisibilityChanged;
    using ComponentMovementWatcher::componentMovedOrResized;

private:
    std::function<void()> peerChanged;
};

struct ViewSizeListener final : private ComponentMovementWatcher
{
    ViewSizeListener (Component& c, PhysicalResizeListener& l)
        : ComponentMovementWatcher (&c), listener (l)
    {
    }

    void componentMovedOrResized (bool, bool wasResized) override
    {
        if (wasResized)
        {
            const auto physicalSize = Desktop::getInstance().getDisplays()
                                                            .logicalToPhysical (getComponent()->localAreaToGlobal (getComponent()->getLocalBounds()));
            const auto width  = physicalSize.getWidth();
            const auto height = physicalSize.getHeight();

            if (width > 10 && height > 10)
                listener.viewRequestedResizeInPhysicalPixels (width, height);
        }
    }

    void componentPeerChanged() override {}
    void componentVisibilityChanged() override {}

    using ComponentMovementWatcher::componentVisibilityChanged;
    using ComponentMovementWatcher::componentMovedOrResized;

    PhysicalResizeListener& listener;
};

class ConfiguredEditorComponent final : public Component,
                                        private PhysicalResizeListener
{
public:
    ConfiguredEditorComponent (World& world,
                               InstanceWithSupports& instance,
                               UiDescriptor& uiDescriptor,
                               LogicalResizeListener& resizeListenerIn,
                               TouchListener& touchListener,
                               const String& uiBundleUri,
                               const UiFeaturesDataOptions& opts)
        : resizeListener (resizeListenerIn),
          floatUrid (instance.symap->map (LV2_ATOM__Float)),
          scaleFactorUrid (instance.symap->map (LV2_UI__scaleFactor)),
          uiInstance (new UiInstanceWithSupports (world,
                                                  *this,
                                                  touchListener,
                                                  &uiDescriptor,
                                                  UiInstanceArgs{}.withBundlePath (bundlePathFromUri (uiBundleUri.toRawUTF8()))
                                                                  .withPluginUri (URL (instance.instance.getUri())),
                                                  viewComponent.getWidget(),
                                                  instance,
                                                  opts)),
          resizeClient (uiInstance->instance.getExtensionData<LV2UI_Resize> (world, LV2_UI__resize)),
          optionsInterface (uiInstance->instance.getExtensionData<LV2_Options_Interface> (world, LV2_OPTIONS__interface))
    {
        jassert (uiInstance != nullptr);

        setOpaque (true);
        addAndMakeVisible (viewComponent);

        const auto boundsToUse = [&]
        {
            const auto requested = uiInstance->features.getLastRequestedBounds();

            if (requested.getWidth() > 10 && requested.getHeight() > 10)
                return requested;

            return uiInstance->instance.getDetectedViewBounds();
        }();

        const auto scaled = lv2ToComponentRect (boundsToUse);
        lastWidth  = scaled.getWidth();
        lastHeight = scaled.getHeight();
        setSize (lastWidth, lastHeight);
    }

    ~ConfiguredEditorComponent() override
    {
        viewComponent.prepareForDestruction();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void resized() override
    {
        viewComponent.setBounds (getLocalBounds());
    }

    void updateViewBounds()
    {
        // If the editor changed size as a result of a request from the client,
        // we shouldn't send a notification back to the client.
        if (uiInstance != nullptr)
        {
            if (resizeClient.valid && resizeClient.extension.ui_resize != nullptr)
            {
                const auto physicalSize = componentToLv2Rect (getLocalBounds());

                resizeClient.extension.ui_resize (uiInstance->instance.getHandle(),
                                                  physicalSize.getWidth(),
                                                  physicalSize.getHeight());
            }
        }
    }

    void pushMessage (MessageHeader header, uint32_t size, const void* buffer)
    {
        if (uiInstance != nullptr)
            uiInstance->instance.pushMessage (header, size, buffer);
    }

    int idle()
    {
        if (uiInstance != nullptr)
            return uiInstance->instance.idle();

        return 0;
    }

    void childBoundsChanged (Component* c) override
    {
        if (c == nullptr)
            resizeToFitView();
    }

    void setUserScaleFactor (float userScale) { userScaleFactor = userScale; }

    void sendScaleFactorToPlugin()
    {
        const auto factor = getEffectiveScale();

        const LV2_Options_Option options[]
        {
            { LV2_OPTIONS_INSTANCE, 0, scaleFactorUrid, sizeof (float), floatUrid, &factor },
            { {}, {}, {}, {}, {}, {} }
        };

        if (optionsInterface.valid)
            optionsInterface.extension.set (uiInstance->instance.getHandle(), options);

        applyLastRequestedPhysicalSize();
    }

private:
    void viewRequestedResizeInPhysicalPixels (int width, int height) override
    {
        lastWidth = width;
        lastHeight = height;
        const auto logical = lv2ToComponentRect ({ width, height });
        resizeListener.viewRequestedResizeInLogicalPixels (logical.getWidth(), logical.getHeight());
    }

    void resizeToFitView()
    {
        viewComponent.fitToView();
        resizeListener.viewRequestedResizeInLogicalPixels (viewComponent.getWidth(), viewComponent.getHeight());
    }

    void applyLastRequestedPhysicalSize()
    {
        viewRequestedResizeInPhysicalPixels (lastWidth, lastHeight);
        viewComponent.forceViewToSize();
    }

    /*  Convert from the component's coordinate system to the hosted LV2's coordinate system. */
    Rectangle<int> componentToLv2Rect (Rectangle<int> r) const
    {
        return localAreaToGlobal (r) * nativeScaleFactor * getDesktopScaleFactor();
    }

    /*  Convert from the hosted LV2's coordinate system to the component's coordinate system. */
    Rectangle<int> lv2ToComponentRect (Rectangle<int> vr) const
    {
        return getLocalArea (nullptr, vr / (nativeScaleFactor * getDesktopScaleFactor()));
    }

    float getEffectiveScale() const     { return nativeScaleFactor * userScaleFactor; }

    // If possible, try to keep platform-specific handing restricted to the implementation of
    // ViewComponent. Keep the interface of ViewComponent consistent on all platforms.
   #if JUCE_LINUX || JUCE_BSD
    struct InnerHolder
    {
        struct Inner final : public XEmbedComponent
        {
            Inner() : XEmbedComponent (true, true)
            {
                setOpaque (true);
                setVisible (true);
                addToDesktop (0);
            }
        };

        Inner inner;
    };

    struct ViewComponent final : public InnerHolder,
                                 public XEmbedComponent
    {
        explicit ViewComponent (PhysicalResizeListener& l)
            : XEmbedComponent ((unsigned long) inner.getPeer()->getNativeHandle(), true, false),
              listener (inner, l)
        {
            setOpaque (true);
        }

        ~ViewComponent()
        {
            removeClient();
        }

        void prepareForDestruction()
        {
            inner.removeClient();
        }

        LV2UI_Widget getWidget() { return lv2_shared::wordCast<LV2UI_Widget> (inner.getHostWindowID()); }
        void forceViewToSize() {}
        void fitToView() {}

        ViewSizeListener listener;
    };
   #elif JUCE_MAC
    struct ViewComponent final : public NSViewComponentWithParent
    {
        explicit ViewComponent (PhysicalResizeListener&)
            : NSViewComponentWithParent (WantsNudge::no) {}
        LV2UI_Widget getWidget() { return getView(); }
        void forceViewToSize() {}
        void fitToView() { resizeToFitView(); }
        void prepareForDestruction() {}
    };
   #elif JUCE_WINDOWS
    struct ViewComponent final : public HWNDComponent
    {
        explicit ViewComponent (PhysicalResizeListener&)
        {
            setOpaque (true);
            inner.addToDesktop (0);

            if (auto* peer = inner.getPeer())
                setHWND (peer->getNativeHandle());
        }

        void paint (Graphics& g) override { g.fillAll (Colours::black); }

        LV2UI_Widget getWidget() { return getHWND(); }

        void forceViewToSize() { updateHWNDBounds(); }
        void fitToView() { resizeToFit(); }

        void prepareForDestruction() {}

    private:
        struct Inner final : public Component
        {
            Inner() { setOpaque (true); }
            void paint (Graphics& g) override { g.fillAll (Colours::black); }
        };

        Inner inner;
    };
   #else
    struct ViewComponent final : public Component
    {
        explicit ViewComponent (PhysicalResizeListener&) {}
        void* getWidget() { return nullptr; }
        void forceViewToSize() {}
        void fitToView() {}
        void prepareForDestruction() {}
    };
   #endif

    struct ScaleNotifierCallback
    {
        ConfiguredEditorComponent& window;

        void operator() (float platformScale) const
        {
            MessageManager::callAsync ([ref = Component::SafePointer<ConfiguredEditorComponent> (&window), platformScale]
            {
                if (auto* r = ref.getComponent())
                {
                    if (approximatelyEqual (std::exchange (r->nativeScaleFactor, platformScale), platformScale))
                        return;

                    r->nativeScaleFactor = platformScale;
                    r->sendScaleFactorToPlugin();
                }
            });
        }
    };

    LogicalResizeListener& resizeListener;
    int lastWidth = 0, lastHeight = 0;
    float nativeScaleFactor = 1.0f, userScaleFactor = 1.0f;
    NativeScaleFactorNotifier scaleNotifier { this, ScaleNotifierCallback { *this } };
    ViewComponent viewComponent { *this };
    LV2_URID floatUrid, scaleFactorUrid;
    std::unique_ptr<UiInstanceWithSupports> uiInstance;
    OptionalExtension<LV2UI_Resize> resizeClient;
    OptionalExtension<LV2_Options_Interface> optionsInterface;
    PeerChangedListener peerListener { *this, [this]
    {
        applyLastRequestedPhysicalSize();
    } };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfiguredEditorComponent)
};

//==============================================================================
/*  Interface to receive notifications when the Editor changes. */
struct EditorListener
{
    virtual ~EditorListener() = default;

    /*  The editor needs to be recreated in a few different scenarios, such as:
        - When the scale factor of the window changes, because we can only provide the
          scale factor to the view during construction
        - When the sample rate changes, because the processor also needs to be destroyed
          and recreated in this case

        This function will be called whenever the editor has been recreated, in order to
        allow the processor (or other listeners) to respond, e.g. by sending all of the
        current port/parameter values to the view.
    */
    virtual void viewCreated (UiEventListener* newListener) = 0;

    virtual void notifyEditorBeingDeleted() = 0;
};

class Editor final : public AudioProcessorEditor,
                     public UiEventListener,
                     private LogicalResizeListener
{
public:
    Editor (World& worldIn,
            AudioPluginInstance& p,
            UiDescriptor& uiDescriptorIn,
            TouchListener& touchListenerIn,
            EditorListener& listenerIn,
            ProcessorToUi& channelIn,
            InstanceWithSupports& instanceIn,
            const String& uiBundleUriIn,
            RequiredFeatures requiredIn,
            OptionalFeatures optionalIn)
        : AudioProcessorEditor (p),
          world (worldIn),
          uiDescriptor (&uiDescriptorIn),
          touchListener (&touchListenerIn),
          listener (&listenerIn),
          uiBundleUri (uiBundleUriIn),
          channel (channelIn),
          required (std::move (requiredIn)),
          optional (std::move (optionalIn))
    {
        setResizable (isResizable (required, optional), false);
        setSize (10, 10);
        setOpaque (true);

        createView (instanceIn);

        channel.addUi (*this);
    }

    ~Editor() noexcept override
    {
        channel.removeUi (*this);

        listener->notifyEditorBeingDeleted();
    }

    void createView (InstanceWithSupports& instance)
    {
        const auto initialScale = userScaleFactor * (float) [&]
        {
            if (auto* p = getPeer())
                return p->getPlatformScaleFactor();

            return 1.0;
        }();

        const auto opts = UiFeaturesDataOptions{}.withInitialScaleFactor (initialScale)
                                                 .withSampleRate ((float) processor.getSampleRate());
        configuredEditor = nullptr;
        configuredEditor = rawToUniquePtr (new ConfiguredEditorComponent (world,
                                                                          instance,
                                                                          *uiDescriptor,
                                                                          *this,
                                                                          *touchListener,
                                                                          uiBundleUri,
                                                                          opts));
        parentHierarchyChanged();
        const auto initialSize = configuredEditor->getBounds();
        setSize (initialSize.getWidth(), initialSize.getHeight());

        listener->viewCreated (this);
    }

    void destroyView()
    {
        configuredEditor = nullptr;
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void resized() override
    {
        const ScopedValueSetter<bool> scope (resizeFromHost, true);

        if (auto* inner = configuredEditor.get())
        {
            inner->setBounds (getLocalBounds());
            inner->updateViewBounds();
        }
    }

    void parentHierarchyChanged() override
    {
        if (auto* comp = configuredEditor.get())
        {
            if (isShowing())
                addAndMakeVisible (comp);
            else
                removeChildComponent (comp);
        }
    }

    void pushMessage (MessageHeader header, uint32_t size, const void* buffer) override
    {
        if (auto* comp = configuredEditor.get())
            comp->pushMessage (header, size, buffer);
    }

    int idle() override
    {
        if (auto* comp = configuredEditor.get())
            return comp->idle();

        return 0;
    }

    void setScaleFactor (float newScale) override
    {
        userScaleFactor = newScale;

        if (configuredEditor != nullptr)
        {
            configuredEditor->setUserScaleFactor (userScaleFactor);
            configuredEditor->sendScaleFactorToPlugin();
        }
    }

private:
    bool isResizable (const RequiredFeatures& requiredFeatures,
                      const OptionalFeatures& optionalFeatures) const
    {
        const auto uriMatches = [] (const LilvNode* node)
        {
            const auto* uri = lilv_node_as_uri (node);
            return std::strcmp (uri, LV2_UI__noUserResize) == 0;
        };

        return uiDescriptor->hasExtensionData (world, LV2_UI__resize)
               && ! uiDescriptor->hasExtensionData (world, LV2_UI__noUserResize)
               && noneOf (requiredFeatures.values, uriMatches)
               && noneOf (optionalFeatures.values, uriMatches);
    }

    bool isScalable() const
    {
        return uiDescriptor->hasExtensionData (world, LV2_OPTIONS__interface);
    }

    void viewRequestedResizeInLogicalPixels (int width, int height) override
    {
        if (! resizeFromHost)
            setSize (width, height);
    }

    World& world;
    UiDescriptor* uiDescriptor;
    TouchListener* touchListener;
    EditorListener* listener;
    String uiBundleUri;
    ProcessorToUi& channel;
    const RequiredFeatures required;
    const OptionalFeatures optional;
    std::unique_ptr<ConfiguredEditorComponent> configuredEditor;
    float userScaleFactor = 1.0f;
    bool resizeFromHost = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)
};

template <bool editorEnabled = editorFunctionalityEnabled> class OptionalEditor;

template <>
class OptionalEditor<true>
{
public:
    OptionalEditor (World& worldIn, String uiBundleUriIn, UiDescriptor uiDescriptorIn, std::function<void()> timerCallback)
        : world (&worldIn),
          uiBundleUri (std::move (uiBundleUriIn)),
          uiDescriptor (std::move (uiDescriptorIn)),
          changedParameterFlusher (std::move (timerCallback)) {}

    void createView (InstanceWithSupports& instance)
    {
        if (auto* editor = editorPointer.getComponent())
            editor->createView (instance);
    }

    void destroyView()
    {
        if (auto* editor = editorPointer.getComponent())
            editor->destroyView();
    }

    std::unique_ptr<AudioProcessorEditor> createEditor (AudioPluginInstance& p,
                                                        InstanceWithSupports& instanceIn,
                                                        TouchListener& touchListenerIn,
                                                        EditorListener& listenerIn,
                                                        ProcessorToUi& channelIn)
    {
        if (! hasEditor())
            return nullptr;

        const auto queryFeatures = [this] (const char* kind)
        {
            return world->findNodes (world->newUri (uiDescriptor.get()->URI).get(),
                                     world->newUri (kind).get(),
                                     nullptr);
        };

        auto newEditor = std::make_unique<Editor> (*world,
                                                   p,
                                                   uiDescriptor,
                                                   touchListenerIn,
                                                   listenerIn,
                                                   channelIn,
                                                   instanceIn,
                                                   uiBundleUri,
                                                   RequiredFeatures { queryFeatures (LV2_CORE__requiredFeature) },
                                                   OptionalFeatures { queryFeatures (LV2_CORE__optionalFeature) });

        editorPointer = newEditor.get();

        changedParameterFlusher.startTimerHz (60);

        return newEditor;
    }

    bool hasEditor() const
    {
        return uiDescriptor.get() != nullptr;
    }

    void prepareToDestroyEditor()
    {
        changedParameterFlusher.stopTimer();
    }

private:
    World* world = nullptr;
    Component::SafePointer<Editor> editorPointer = nullptr;
    String uiBundleUri;
    UiDescriptor uiDescriptor;
    TimedCallback changedParameterFlusher;
};

template <>
class OptionalEditor<false>
{
public:
    OptionalEditor (String, UiDescriptor, std::function<void()>) {}

    void createView (InstanceWithSupports&) {}
    void destroyView() {}

    std::unique_ptr<AudioProcessorEditor> createEditor (AudioPluginInstance&,
                                                        InstanceWithSupports&,
                                                        TouchListener&,
                                                        EditorListener&,
                                                        ProcessorToUi&)
    {
        return nullptr;
    }

    bool hasEditor() const { return false; }
    void prepareToDestroyEditor() {}
};

class LV2AudioPluginInstance final : public LV2AudioPluginInstanceHeadless,
                                     private EditorListener,
                                     private TouchListener
{
public:
    using LV2AudioPluginInstanceHeadless::LV2AudioPluginInstanceHeadless;

    LV2AudioPluginInstance (std::shared_ptr<World> worldIn,
                            const Plugin& pluginIn,
                            const UsefulUris& uris,
                            std::unique_ptr<InstanceWithSupports>&& in,
                            PluginDescription&& desc,
                            std::vector<String> knownPresetUris,
                            PluginState stateToApply,
                            String uiBundleUriIn,
                            UiDescriptor uiDescriptorIn)
        : LV2AudioPluginInstanceHeadless (worldIn,
                                          pluginIn,
                                          uris,
                                          std::move (in),
                                          std::move (desc),
                                          knownPresetUris,
                                          std::move (stateToApply),
                                          uiBundleUriIn,
                                          uiDescriptorIn),
          optionalEditor (*worldIn,
                          std::move (uiBundleUriIn),
                          std::move (uiDescriptorIn),
                          [this] { postChangedParametersToUi (*processorToUi, uiEventListener); })
    {
        asyncFullUiParameterUpdate.triggerAsyncUpdate();
    }

    AudioProcessorEditor* createEditor() override
    {
        if (auto* i = getCurrentInstance())
            return optionalEditor.createEditor (*this, *i, *this, *this, *processorToUi).release();

        // No instance?
        jassertfalse;
        return nullptr;
    }

    bool hasEditor() const override
    {
        return optionalEditor.hasEditor();
    }

    void prepareToPlay (double sampleRate, int numSamples) override
    {
        // This does *not* destroy the editor component.
        // If we destroy the processor, the view must also be destroyed to avoid dangling pointers.
        // However, JUCE clients expect their editors to remain valid for the duration of the
        // AudioProcessor's lifetime.
        // As a compromise, this will create a new LV2 view into an existing editor component.
        optionalEditor.destroyView();

        LV2AudioPluginInstanceHeadless::prepareToPlay (sampleRate, numSamples);

        if (auto* i = getCurrentInstance())
            optionalEditor.createView (*i);
        else
            jassertfalse; // Unable to create instance?
    }

    void setStateInformation (const void *data, int size) override
    {
        LV2AudioPluginInstanceHeadless::setStateInformation (data, size);
        asyncFullUiParameterUpdate.triggerAsyncUpdate();
    }

    void setCurrentProgram (int newProgram) override
    {
        LV2AudioPluginInstanceHeadless::setCurrentProgram (newProgram);
        asyncFullUiParameterUpdate.triggerAsyncUpdate();
    }

private:
    void sendOutgoingPortMessageToUi (UiMessageHeader header, uint32_t size, const void* buffer) override
    {
        header.listener = uiEventListener;
        processorToUi->pushMessage (header, size, buffer);
    }

    void viewCreated (UiEventListener* newListener) override
    {
        uiEventListener = newListener;
        postAllParametersToUi (*processorToUi, uiEventListener);
    }

    void notifyEditorBeingDeleted() override
    {
        optionalEditor.prepareToDestroyEditor();
        uiEventListener = nullptr;
        editorBeingDeleted (getActiveEditor());
    }

    void controlGrabbed (uint32_t port, bool grabbed) override
    {
        if (auto* param = getParamByPortIndex (port))
        {
            if (grabbed)
                param->beginChangeGesture();
            else
                param->endChangeGesture();
        }
    }

    SharedResourcePointer<ProcessorToUi> processorToUi;

    std::atomic<UiEventListener*> uiEventListener { nullptr };
    OptionalEditor<> optionalEditor;
    AsyncFn asyncFullUiParameterUpdate { [this] { postAllParametersToUi (*processorToUi, uiEventListener); } };
};

} // namespace lv2_host

void LV2PluginFormat::createPluginInstance (const PluginDescription& desc,
                                            double sampleRate,
                                            int bufferSize,
                                            PluginCreationCallback callback)
{
    Pimpl::createPluginInstance<lv2_host::LV2AudioPluginInstance> (*this, desc, sampleRate, bufferSize, std::move (callback));
}

} // namespace juce

#endif
