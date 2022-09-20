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

#if (JUCE_PLUGINHOST_ARA && (JUCE_PLUGINHOST_VST3 || JUCE_PLUGINHOST_AU) && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX))

#include "juce_ARAHosting.h"

#include <ARA_Library/Dispatch/ARAHostDispatch.cpp>

namespace juce
{
struct ARAEditGuardState
{
public:
    /*  Returns true if this controller wasn't previously present. */
    bool add (ARA::Host::DocumentController& dc)
    {
        const std::lock_guard<std::mutex> lock (mutex);
        return ++counts[&dc] == 1;
    }

    /*  Returns true if this controller is no longer present. */
    bool remove (ARA::Host::DocumentController& dc)
    {
        const std::lock_guard<std::mutex> lock (mutex);
        return --counts[&dc] == 0;
    }

private:
    std::map<ARA::Host::DocumentController*, int> counts;
    std::mutex mutex;
};

static ARAEditGuardState editGuardState;

ARAEditGuard::ARAEditGuard (ARA::Host::DocumentController& dcIn) : dc (dcIn)
{
    if (editGuardState.add (dc))
        dc.beginEditing();
}

ARAEditGuard::~ARAEditGuard()
{
    if (editGuardState.remove (dc))
        dc.endEditing();
}

//==============================================================================
namespace ARAHostModel
{

//==============================================================================
AudioSource::AudioSource (ARA::ARAAudioSourceHostRef hostRef,
                          ARA::Host::DocumentController& dc,
                          const ARA::ARAAudioSourceProperties& props)
    : ManagedARAHandle (dc, [&]
                        {
                            const ARAEditGuard guard (dc);
                            return dc.createAudioSource (hostRef, &props);
                        }())
{
}

void AudioSource::update (const ARA::ARAAudioSourceProperties& props)
{
    const ARAEditGuard guard (getDocumentController());
    getDocumentController().updateAudioSourceProperties (getPluginRef(), &props);
}

void AudioSource::enableAudioSourceSamplesAccess (bool x)
{
    const ARAEditGuard guard (getDocumentController());
    getDocumentController().enableAudioSourceSamplesAccess (getPluginRef(), x);
}

void AudioSource::destroy (ARA::Host::DocumentController& dc, Ptr ptr)
{
    dc.destroyAudioSource (ptr);
}

//==============================================================================
AudioModification::AudioModification (ARA::ARAAudioModificationHostRef hostRef,
                                      ARA::Host::DocumentController& dc,
                                      AudioSource& s,
                                      const ARA::ARAAudioModificationProperties& props)
    : ManagedARAHandle (dc, [&]
                        {
                            const ARAEditGuard guard (dc);
                            return dc.createAudioModification (s.getPluginRef(), hostRef, &props);
                        }()),
      source (s)
{
}

void AudioModification::update (const ARA::ARAAudioModificationProperties& props)
{
    const ARAEditGuard guard (getDocumentController());
    getDocumentController().updateAudioModificationProperties (getPluginRef(), &props);
}

void AudioModification::destroy (ARA::Host::DocumentController& dc, Ptr ptr)
{
    dc.destroyAudioModification (ptr);
}

//==============================================================================
class PlaybackRegion::Impl  : public ManagedARAHandle<Impl, ARA::ARAPlaybackRegionRef>,
                              public DeletionListener
{
public:
    Impl (ARA::ARAPlaybackRegionHostRef hostRef,
          ARA::Host::DocumentController& dc,
          AudioModification& m,
          const ARA::ARAPlaybackRegionProperties& props);

    ~Impl() override
    {
        for (const auto& l : listeners)
            l->removeListener (*this);
    }

    /*  Updates the state of the corresponding %ARA model object.

        Places the DocumentController in editable state.

        You can use getEmptyProperties() to acquire a properties struct where the `structSize`
        field has already been correctly set.
    */
    void update (const ARA::ARAPlaybackRegionProperties& props);

    auto& getAudioModification() const { return modification; }

    static void destroy (ARA::Host::DocumentController&, Ptr);

    void addListener    (DeletionListener& l)                   { listeners.insert (&l); }
    void removeListener (DeletionListener& l) noexcept override { listeners.erase (&l); }

private:
    AudioModification* modification = nullptr;
    std::unordered_set<DeletionListener*> listeners;
};

PlaybackRegion::Impl::Impl (ARA::ARAPlaybackRegionHostRef hostRef,
                            ARA::Host::DocumentController& dc,
                            AudioModification& m,
                            const ARA::ARAPlaybackRegionProperties& props)
    : ManagedARAHandle (dc, [&]
                        {
                            const ARAEditGuard guard (dc);
                            return dc.createPlaybackRegion (m.getPluginRef(), hostRef, &props);
                        }()),
      modification (&m)
{
}

PlaybackRegion::~PlaybackRegion() = default;

void PlaybackRegion::Impl::update (const ARA::ARAPlaybackRegionProperties& props)
{
    const ARAEditGuard guard (getDocumentController());
    getDocumentController().updatePlaybackRegionProperties (getPluginRef(), &props);
}

void PlaybackRegion::Impl::destroy (ARA::Host::DocumentController& dc, Ptr ptr)
{
    dc.destroyPlaybackRegion (ptr);
}

PlaybackRegion::PlaybackRegion (ARA::ARAPlaybackRegionHostRef hostRef,
                                ARA::Host::DocumentController& dc,
                                AudioModification& m,
                                const ARA::ARAPlaybackRegionProperties& props)
    : impl (std::make_unique<Impl> (hostRef, dc, m, props))
{
}

void PlaybackRegion::update (const ARA::ARAPlaybackRegionProperties& props) { impl->update (props); }

void PlaybackRegion::addListener (DeletionListener& x) { impl->addListener (x); }

auto& PlaybackRegion::getAudioModification() const { return impl->getAudioModification(); }

ARA::ARAPlaybackRegionRef PlaybackRegion::getPluginRef() const noexcept { return impl->getPluginRef(); }

DeletionListener& PlaybackRegion::getDeletionListener() const noexcept { return *impl.get(); }

//==============================================================================
MusicalContext::MusicalContext (ARA::ARAMusicalContextHostRef hostRef,
                                ARA::Host::DocumentController& dc,
                                const ARA::ARAMusicalContextProperties& props)
    : ManagedARAHandle (dc, [&]
                        {
                            const ARAEditGuard guard (dc);
                            return dc.createMusicalContext (hostRef, &props);
                        }())
{
}

void MusicalContext::update (const ARA::ARAMusicalContextProperties& props)
{
    const ARAEditGuard guard (getDocumentController());
    return getDocumentController().updateMusicalContextProperties (getPluginRef(), &props);
}

void MusicalContext::destroy (ARA::Host::DocumentController& dc, Ptr ptr)
{
    dc.destroyMusicalContext (ptr);
}

//==============================================================================
RegionSequence::RegionSequence (ARA::ARARegionSequenceHostRef hostRef,
                                ARA::Host::DocumentController& dc,
                                const ARA::ARARegionSequenceProperties& props)
    : ManagedARAHandle (dc, [&]
                        {
                            const ARAEditGuard guard (dc);
                            return dc.createRegionSequence (hostRef, &props);
                        }())
{
}

void RegionSequence::update (const ARA::ARARegionSequenceProperties& props)
{
    const ARAEditGuard guard (getDocumentController());
    return getDocumentController().updateRegionSequenceProperties (getPluginRef(), &props);
}

void RegionSequence::destroy (ARA::Host::DocumentController& dc, Ptr ptr)
{
    dc.destroyRegionSequence (ptr);
}

//==============================================================================
PlaybackRendererInterface PlugInExtensionInstance::getPlaybackRendererInterface() const
{
    if (instance != nullptr)
        return PlaybackRendererInterface (instance->playbackRendererRef, instance->playbackRendererInterface);

    return {};
}

EditorRendererInterface PlugInExtensionInstance::getEditorRendererInterface() const
{
    if (instance != nullptr)
        return EditorRendererInterface (instance->editorRendererRef, instance->editorRendererInterface);

    return {};
}

} // namespace ARAHostModel

//==============================================================================
class ARAHostDocumentController::Impl
{
public:
    Impl (ARAFactoryWrapper araFactoryIn,
          std::unique_ptr<ARA::Host::DocumentControllerHostInstance>&& dcHostInstanceIn,
          const ARA::ARADocumentControllerInstance* documentControllerInstance,
          std::unique_ptr<ARA::Host::AudioAccessControllerInterface>&& audioAccessControllerIn,
          std::unique_ptr<ARA::Host::ArchivingControllerInterface>&& archivingControllerIn,
          std::unique_ptr<ARA::Host::ContentAccessControllerInterface>&& contentAccessControllerIn,
          std::unique_ptr<ARA::Host::ModelUpdateControllerInterface>&& modelUpdateControllerIn,
          std::unique_ptr<ARA::Host::PlaybackControllerInterface>&& playbackControllerIn)
        : araFactory (std::move (araFactoryIn)),
          audioAccessController (std::move (audioAccessControllerIn)),
          archivingController (std::move (archivingControllerIn)),
          contentAccessController (std::move (contentAccessControllerIn)),
          modelUpdateController (std::move (modelUpdateControllerIn)),
          playbackController (std::move (playbackControllerIn)),
          dcHostInstance (std::move (dcHostInstanceIn)),
          documentController (documentControllerInstance)
    {
    }

    ~Impl()
    {
        documentController.destroyDocumentController();
    }

    static std::unique_ptr<Impl>
        createImpl (ARAFactoryWrapper araFactory,
                    const String& documentName,
                    std::unique_ptr<ARA::Host::AudioAccessControllerInterface>&& audioAccessController,
                    std::unique_ptr<ARA::Host::ArchivingControllerInterface>&& archivingController,
                    std::unique_ptr<ARA::Host::ContentAccessControllerInterface>&& contentAccessController,
                    std::unique_ptr<ARA::Host::ModelUpdateControllerInterface>&& modelUpdateController,
                    std::unique_ptr<ARA::Host::PlaybackControllerInterface>&& playbackController)
    {
        std::unique_ptr<ARA::Host::DocumentControllerHostInstance> dcHostInstance =
            std::make_unique<ARA::Host::DocumentControllerHostInstance> (audioAccessController.get(),
                                                                         archivingController.get(),
                                                                         contentAccessController.get(),
                                                                         modelUpdateController.get(),
                                                                         playbackController.get());

        const auto documentProperties = makeARASizedStruct (&ARA::ARADocumentProperties::name, documentName.toRawUTF8());

        if (auto* dci = araFactory.get()->createDocumentControllerWithDocument (dcHostInstance.get(), &documentProperties))
            return std::make_unique<Impl> (std::move (araFactory),
                                           std::move (dcHostInstance),
                                           dci,
                                           std::move (audioAccessController),
                                           std::move (archivingController),
                                           std::move (contentAccessController),
                                           std::move (modelUpdateController),
                                           std::move (playbackController));

        return {};
    }

    ARAHostModel::PlugInExtensionInstance bindDocumentToPluginInstance (AudioPluginInstance& instance,
                                                                        ARA::ARAPlugInInstanceRoleFlags knownRoles,
                                                                        ARA::ARAPlugInInstanceRoleFlags assignedRoles)
    {

        const auto makeVisitor = [] (auto vst3Fn, auto auFn)
        {
            using Vst3Fn = decltype (vst3Fn);
            using AuFn = decltype (auFn);

            struct Visitor : ExtensionsVisitor, Vst3Fn, AuFn
            {
                explicit Visitor (Vst3Fn vst3Fn, AuFn auFn) : Vst3Fn (std::move (vst3Fn)), AuFn (std::move (auFn)) {}
                void visitVST3Client (const VST3Client& x) override { Vst3Fn::operator() (x); }
                void visitAudioUnitClient (const AudioUnitClient& x) override { AuFn::operator() (x); }
            };

            return Visitor { std::move (vst3Fn), std::move (auFn) };
        };

        const ARA::ARAPlugInExtensionInstance* pei = nullptr;
        auto visitor = makeVisitor ([this, &pei, knownRoles, assignedRoles] (const ExtensionsVisitor::VST3Client& vst3Client)
                                    {
                                        auto* iComponentPtr = vst3Client.getIComponentPtr();
                                        VSTComSmartPtr<ARA::IPlugInEntryPoint2> araEntryPoint;

                                        if (araEntryPoint.loadFrom (iComponentPtr))
                                            pei = araEntryPoint->bindToDocumentControllerWithRoles (documentController.getRef(), knownRoles, assignedRoles);
                                    },
                                   #if JUCE_PLUGINHOST_AU && JUCE_MAC
                                    [this, &pei, knownRoles, assignedRoles] (const ExtensionsVisitor::AudioUnitClient& auClient)
                                    {
                                        auto audioUnit = auClient.getAudioUnitHandle();
                                        auto propertySize = (UInt32) sizeof (ARA::ARAAudioUnitPlugInExtensionBinding);
                                        const auto expectedPropertySize = propertySize;
                                        ARA::ARAAudioUnitPlugInExtensionBinding audioUnitBinding { ARA::kARAAudioUnitMagic,
                                                                                                   documentController.getRef(),
                                                                                                   nullptr,
                                                                                                   knownRoles,
                                                                                                   assignedRoles };

                                        auto status = AudioUnitGetProperty (audioUnit,
                                                                            ARA::kAudioUnitProperty_ARAPlugInExtensionBindingWithRoles,
                                                                            kAudioUnitScope_Global,
                                                                            0,
                                                                            &audioUnitBinding,
                                                                            &propertySize);

                                        if (status == noErr
                                            && propertySize == expectedPropertySize
                                            && audioUnitBinding.inOutMagicNumber == ARA::kARAAudioUnitMagic
                                            && audioUnitBinding.inDocumentControllerRef == documentController.getRef()
                                            && audioUnitBinding.outPlugInExtension != nullptr)
                                        {
                                            pei = audioUnitBinding.outPlugInExtension;
                                        }
                                        else
                                            jassertfalse;
                                    }
                                   #else
                                    [] (const auto&) {}
                                   #endif
                                    );

        instance.getExtensions (visitor);
        return ARAHostModel::PlugInExtensionInstance { pei };
    }

    auto& getDocumentController()       { return documentController; }

private:
    ARAFactoryWrapper araFactory;

    std::unique_ptr<ARA::Host::AudioAccessControllerInterface>   audioAccessController;
    std::unique_ptr<ARA::Host::ArchivingControllerInterface>     archivingController;
    std::unique_ptr<ARA::Host::ContentAccessControllerInterface> contentAccessController;
    std::unique_ptr<ARA::Host::ModelUpdateControllerInterface>   modelUpdateController;
    std::unique_ptr<ARA::Host::PlaybackControllerInterface>      playbackController;

    std::unique_ptr<ARA::Host::DocumentControllerHostInstance> dcHostInstance;
    ARA::Host::DocumentController documentController;
};

ARAHostDocumentController::ARAHostDocumentController (std::unique_ptr<Impl>&& implIn)
    : impl { std::move (implIn) }
{}

std::unique_ptr<ARAHostDocumentController> ARAHostDocumentController::create (ARAFactoryWrapper factory,
                                                                              const String& documentName,
                                                                              std::unique_ptr<ARA::Host::AudioAccessControllerInterface> audioAccessController,
                                                                              std::unique_ptr<ARA::Host::ArchivingControllerInterface> archivingController,
                                                                              std::unique_ptr<ARA::Host::ContentAccessControllerInterface> contentAccessController,
                                                                              std::unique_ptr<ARA::Host::ModelUpdateControllerInterface> modelUpdateController,
                                                                              std::unique_ptr<ARA::Host::PlaybackControllerInterface> playbackController)
{
    if (auto impl = Impl::createImpl (std::move (factory),
                                      documentName,
                                      std::move (audioAccessController),
                                      std::move (archivingController),
                                      std::move (contentAccessController),
                                      std::move (modelUpdateController),
                                      std::move (playbackController)))
    {
        return rawToUniquePtr (new ARAHostDocumentController (std::move (impl)));
    }

    return {};
}

ARAHostDocumentController::~ARAHostDocumentController() = default;

ARA::Host::DocumentController& ARAHostDocumentController::getDocumentController() const
{
    return impl->getDocumentController();
}

ARAHostModel::PlugInExtensionInstance ARAHostDocumentController::bindDocumentToPluginInstance (AudioPluginInstance& instance,
                                                                                               ARA::ARAPlugInInstanceRoleFlags knownRoles,
                                                                                               ARA::ARAPlugInInstanceRoleFlags assignedRoles)
{
    return impl->bindDocumentToPluginInstance (instance, knownRoles, assignedRoles);
}

void createARAFactoryAsync (AudioPluginInstance& instance, std::function<void (ARAFactoryWrapper)> cb)
{
    if (! instance.getPluginDescription().hasARAExtension)
        cb (ARAFactoryWrapper{});

    struct Extensions : public ExtensionsVisitor
    {
        Extensions (std::function<void (ARAFactoryWrapper)> callbackIn)
            : callback (std::move (callbackIn))
        {}

        void visitARAClient (const ARAClient& araClient) override
        {
            araClient.createARAFactoryAsync (std::move (callback));
        }

        std::function<void (ARAFactoryWrapper)> callback;
    };

    Extensions extensions { std::move(cb) };
    instance.getExtensions (extensions);
}

} // namespace juce

#endif
