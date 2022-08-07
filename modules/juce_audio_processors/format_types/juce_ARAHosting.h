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

#pragma once

#if (JUCE_PLUGINHOST_ARA && (JUCE_PLUGINHOST_VST3 || JUCE_PLUGINHOST_AU) && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX)) || DOXYGEN

// Include ARA SDK headers
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wgnu-zero-variadic-macro-arguments")

#include <ARA_API/ARAInterface.h>
#include <ARA_Library/Dispatch/ARAHostDispatch.h>

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

//==============================================================================
namespace juce
{
/** Reference counting helper class to ensure that the DocumentController is in editable state.

    When adding, removing or modifying %ARA model objects the enclosing DocumentController must be
    in editable state.

    You can achieve this by using the %ARA Library calls
    ARA::Host::DocumentController::beginEditing() and ARA::Host::DocumentController::endEditing().

    However, putting the DocumentController in and out of editable state is a potentially costly
    operation, thus it makes sense to group multiple modifications together and change the editable
    state only once.

    ARAEditGuard keeps track of all scopes that want to edit a particular DocumentController and
    will trigger beginEditing() and endEditing() only for the outermost scope. This allows you to
    merge multiple editing operations into one by putting ARAEditGuard in their enclosing scope.

    @tags{ARA}
*/
class ARAEditGuard
{
public:
    explicit ARAEditGuard (ARA::Host::DocumentController& dcIn);
    ~ARAEditGuard();

private:
    ARA::Host::DocumentController& dc;
};

namespace ARAHostModel
{

//==============================================================================
/** Allows converting, without warnings, between pointers of two unrelated types.

    This is a bit like ARA_MAP_HOST_REF, but not macro-based.

    To use it, add a line like this to a type that needs to deal in host references:
    @code
    using Converter = ConversionFunctions<ThisType*, ARAHostRef>;
    @endcode

    Now, you can convert back and forth with host references by calling
    Converter::toHostRef() and Converter::fromHostRef().

    @tags{ARA}
*/
template <typename A, typename B>
struct ConversionFunctions
{
    static_assert (sizeof (A) <= sizeof (B),
                   "It is only possible to convert between types of the same size");

    static B toHostRef (A value)
    {
        return readUnaligned<B> (&value);
    }

    static A fromHostRef (B value)
    {
        return readUnaligned<A> (&value);
    }
};

//==============================================================================
/** This class is used by the various ARA model object helper classes, such as MusicalContext,
    AudioSource etc. It helps with deregistering the model objects from the DocumentController
    when the lifetime of the helper class object ends.

    You shouldn't use this class directly but instead inherit from the helper classes.
*/
template <typename Base, typename PtrIn>
class ManagedARAHandle
{
public:
    using Ptr = PtrIn;

    /** Constructor. */
    ManagedARAHandle (ARA::Host::DocumentController& dc, Ptr ptr) noexcept
        : handle (ptr, Deleter { dc }) {}

    /** Returns the host side DocumentController reference. */
    auto& getDocumentController() const { return handle.get_deleter().documentController; }

    /** Returns the plugin side reference to the model object. */
    Ptr getPluginRef() const { return handle.get(); }

private:
    struct Deleter
    {
        void operator() (Ptr ptr) const noexcept
        {
            const ARAEditGuard guard (documentController);
            Base::destroy (documentController, ptr);
        }

        ARA::Host::DocumentController& documentController;
    };

    std::unique_ptr<std::remove_pointer_t<Ptr>, Deleter> handle;
};

//==============================================================================
/** Helper class for the host side implementation of the %ARA %AudioSource model object.

    Its intended use is to add a member variable of this type to your host side %AudioSource
    implementation. Then it provides a RAII approach to managing the lifetime of the corresponding
    objects created inside the DocumentController. When the host side object is instantiated an ARA
    model object is also created in the DocumentController. When the host side object is deleted it
    will be removed from the DocumentController as well.

    The class will automatically put the DocumentController into editable state for operations that
    mandate this e.g. creation, deletion or updating.

    You can encapsulate multiple such operations into a scope with an ARAEditGuard in order to invoke
    the editable state of the DocumentController only once.

    @tags{ARA}
*/
class AudioSource : public ManagedARAHandle<AudioSource, ARA::ARAAudioSourceRef>
{
public:
    /** Returns an %ARA versioned struct with the `structSize` correctly set for the currently
        used SDK version.

        You should leave `structSize` unchanged, and fill out the rest of the fields appropriately
        for the host implementation of the %ARA model object.
    */
    static constexpr auto getEmptyProperties() { return makeARASizedStruct (&ARA::ARAAudioSourceProperties::merits64BitSamples); }

    /** Creates an AudioSource object. During construction it registers an %ARA %AudioSource model
        object with the DocumentController that refers to the provided hostRef. When this object
        is deleted the corresponding DocumentController model object will also be deregistered.

        You can acquire a correctly versioned `ARA::ARAAudioSourceProperties` struct by calling
        getEmptyProperties().

        Places the DocumentController in editable state.

        @see ARAEditGuard
    */
    AudioSource (ARA::ARAAudioSourceHostRef hostRef,
                 ARA::Host::DocumentController& dc,
                 const ARA::ARAAudioSourceProperties& props);

    /** Destructor. Temporarily places the DocumentController in an editable state. */
    ~AudioSource() = default;

    /** Updates the state of the corresponding %ARA model object.

        Places the DocumentController in editable state.

        You can use getEmptyProperties() to acquire a properties struct where the `structSize`
        field has already been correctly set.
    */
    void update (const ARA::ARAAudioSourceProperties& props);

    /** Changes the plugin's access to the %AudioSource samples through the DocumentController.

        Places the DocumentController in editable state.
    */
    void enableAudioSourceSamplesAccess (bool);

    /** Called by ManagedARAHandle to deregister the model object during the destruction of
        AudioSource.

        You shouldn't call this function manually.
    */
    static void destroy (ARA::Host::DocumentController&, Ptr);
};

/** Helper class for the host side implementation of the %ARA %AudioModification model object.

    Its intended use is to add a member variable of this type to your host side %AudioModification
    implementation. Then it provides a RAII approach to managing the lifetime of the corresponding
    objects created inside the DocumentController. When the host side object is instantiated an ARA
    model object is also created in the DocumentController. When the host side object is deleted it
    will be removed from the DocumentController as well.

    The class will automatically put the DocumentController into editable state for operations that
    mandate this e.g. creation, deletion or updating.

    You can encapsulate multiple such operations into a scope with an ARAEditGuard in order to invoke
    the editable state of the DocumentController only once.

    @tags{ARA}
*/
class AudioModification : public ManagedARAHandle<AudioModification, ARA::ARAAudioModificationRef>
{
public:
    /** Returns an %ARA versioned struct with the `structSize` correctly set for the currently
        used SDK version.

        You should leave `structSize` unchanged, and fill out the rest of the fields appropriately
        for the host implementation of the %ARA model object.
    */
    static constexpr auto getEmptyProperties()
    {
        return makeARASizedStruct (&ARA::ARAAudioModificationProperties::persistentID);
    }

    /** Creates an AudioModification object. During construction it registers an %ARA %AudioModification model
        object with the DocumentController that refers to the provided hostRef. When this object
        is deleted the corresponding DocumentController model object will also be deregistered.

        You can acquire a correctly versioned `ARA::ARAAudioModificationProperties` struct by calling
        getEmptyProperties().

        Places the DocumentController in editable state.

        @see ARAEditGuard
    */
    AudioModification (ARA::ARAAudioModificationHostRef hostRef,
                       ARA::Host::DocumentController& dc,
                       AudioSource& s,
                       const ARA::ARAAudioModificationProperties& props);

    /** Updates the state of the corresponding %ARA model object.

        Places the DocumentController in editable state.

        You can use getEmptyProperties() to acquire a properties struct where the `structSize`
        field has already been correctly set.
    */
    void update (const ARA::ARAAudioModificationProperties& props);

    /** Returns the AudioSource containing this AudioModification. */
    auto& getAudioSource() const { return source; }

    /** Called by ManagedARAHandle to deregister the model object during the destruction of
        AudioModification.

        You shouldn't call this function manually.
    */
    static void destroy (ARA::Host::DocumentController&, Ptr);

private:
    AudioSource& source;
};

/** This class is used internally by PlaybackRegionRegistry to be notified when a PlaybackRegion
    object is deleted.
*/
struct DeletionListener
{
    /** Destructor. */
    virtual ~DeletionListener() = default;

    /** Removes another DeletionListener object from this DeletionListener. */
    virtual void removeListener (DeletionListener& other) noexcept = 0;
};

/** Helper class for the host side implementation of the %ARA %PlaybackRegion model object.

    Its intended use is to add a member variable of this type to your host side %PlaybackRegion
    implementation. Then it provides a RAII approach to managing the lifetime of the corresponding
    objects created inside the DocumentController. When the host side object is instantiated an ARA
    model object is also created in the DocumentController. When the host side object is deleted it
    will be removed from the DocumentController as well.

    The class will automatically put the DocumentController into editable state for operations that
    mandate this e.g. creation, deletion or updating.

    You can encapsulate multiple such operations into a scope with an ARAEditGuard in order to invoke
    the editable state of the DocumentController only once.

    @tags{ARA}
*/
struct PlaybackRegion
{
public:
    /** Returns an %ARA versioned struct with the `structSize` correctly set for the currently
        used SDK version.

        You should leave `structSize` unchanged, and fill out the rest of the fields appropriately
        for the host implementation of the %ARA model object.
    */
    static constexpr auto getEmptyProperties()
    {
        return makeARASizedStruct (&ARA::ARAPlaybackRegionProperties::color);
    }

    PlaybackRegion (ARA::ARAPlaybackRegionHostRef hostRef,
                    ARA::Host::DocumentController& dc,
                    AudioModification& m,
                    const ARA::ARAPlaybackRegionProperties& props);

    ~PlaybackRegion();

    /** Updates the state of the corresponding %ARA model object.

        Places the DocumentController in editable state.

        You can use getEmptyProperties() to acquire a properties struct where the `structSize`
        field has already been correctly set.
    */
    void update (const ARA::ARAPlaybackRegionProperties& props);

    /** Adds a DeletionListener object that will be notified when the PlaybackRegion object
        is deleted.

        Used by the PlaybackRegionRegistry.

        @see PlaybackRendererInterface, EditorRendererInterface
    */
    void addListener (DeletionListener& x);

    /** Returns the AudioModification containing this PlaybackRegion. */
    auto& getAudioModification() const;

    /** Returns the plugin side reference to the PlaybackRegion */
    ARA::ARAPlaybackRegionRef getPluginRef() const noexcept;
    DeletionListener& getDeletionListener() const noexcept;

private:
    class Impl;

    std::unique_ptr<Impl> impl;
};

/** Helper class for the host side implementation of the %ARA %MusicalContext model object.

    Its intended use is to add a member variable of this type to your host side %MusicalContext
    implementation. Then it provides a RAII approach to managing the lifetime of the corresponding
    objects created inside the DocumentController. When the host side object is instantiated an ARA
    model object is also created in the DocumentController. When the host side object is deleted it
    will be removed from the DocumentController as well.

    The class will automatically put the DocumentController into editable state for operations that
    mandate this e.g. creation, deletion or updating.

    You can encapsulate multiple such operations into a scope with an ARAEditGuard in order to invoke
    the editable state of the DocumentController only once.

    @tags{ARA}
*/
class MusicalContext : public ManagedARAHandle<MusicalContext, ARA::ARAMusicalContextRef>
{
public:
    /** Returns an %ARA versioned struct with the `structSize` correctly set for the currently
        used SDK version.

        You should leave `structSize` unchanged, and fill out the rest of the fields appropriately
        for the host implementation of the %ARA model object.
    */
    static constexpr auto getEmptyProperties()
    {
        return makeARASizedStruct (&ARA::ARAMusicalContextProperties::color);
    }

    /** Creates a MusicalContext object. During construction it registers an %ARA %MusicalContext model
        object with the DocumentController that refers to the provided hostRef. When this object
        is deleted the corresponding DocumentController model object will also be deregistered.

        You can acquire a correctly versioned `ARA::ARAMusicalContextProperties` struct by calling
        getEmptyProperties().

        Places the DocumentController in editable state.

        @see ARAEditGuard
    */
    MusicalContext (ARA::ARAMusicalContextHostRef hostRef,
                    ARA::Host::DocumentController& dc,
                    const ARA::ARAMusicalContextProperties& props);

    /** Updates the state of the corresponding %ARA model object.

        Places the DocumentController in editable state.

        You can use getEmptyProperties() to acquire a properties struct where the `structSize`
        field has already been correctly set.
    */
    void update (const ARA::ARAMusicalContextProperties& props);

    /** Called by ManagedARAHandle to deregister the model object during the destruction of
        AudioModification.

        You shouldn't call this function manually.
    */
    static void destroy (ARA::Host::DocumentController&, Ptr);
};

/** Helper class for the host side implementation of the %ARA %RegionSequence model object.

    Its intended use is to add a member variable of this type to your host side %RegionSequence
    implementation. Then it provides a RAII approach to managing the lifetime of the corresponding
    objects created inside the DocumentController. When the host side object is instantiated an ARA
    model object is also created in the DocumentController. When the host side object is deleted it
    will be removed from the DocumentController as well.

    The class will automatically put the DocumentController into editable state for operations that
    mandate this e.g. creation, deletion or updating.

    You can encapsulate multiple such operations into a scope with an ARAEditGuard in order to invoke
    the editable state of the DocumentController only once.

    @tags{ARA}
*/
class RegionSequence : public ManagedARAHandle<RegionSequence, ARA::ARARegionSequenceRef>
{
public:
    /** Returns an %ARA versioned struct with the `structSize` correctly set for the currently
        used SDK version.

        You should leave `structSize` unchanged, and fill out the rest of the fields appropriately
        for the host implementation of the %ARA model object.
    */
    static constexpr auto getEmptyProperties()
    {
        return makeARASizedStruct (&ARA::ARARegionSequenceProperties::color);
    }

    /** Creates a RegionSequence object. During construction it registers an %ARA %RegionSequence model
        object with the DocumentController that refers to the provided hostRef. When this object
        is deleted the corresponding DocumentController model object will also be deregistered.

        You can acquire a correctly versioned `ARA::ARARegionSequenceProperties` struct by calling
        getEmptyProperties().

        Places the DocumentController in editable state.

        @see ARAEditGuard
    */
    RegionSequence (ARA::ARARegionSequenceHostRef hostRef,
                    ARA::Host::DocumentController& dc,
                    const ARA::ARARegionSequenceProperties& props);

    /** Updates the state of the corresponding %ARA model object.

        Places the DocumentController in editable state.

        You can use getEmptyProperties() to acquire a properties struct where the `structSize`
        field has already been correctly set.
    */
    void update (const ARA::ARARegionSequenceProperties& props);

    /** Called by ManagedARAHandle to deregister the model object during the destruction of
        AudioModification.

        You shouldn't call this function manually.
    */
    static void destroy (ARA::Host::DocumentController&, Ptr);
};

//==============================================================================
/** Base class used by the ::PlaybackRendererInterface and ::EditorRendererInterface
    plugin extension interfaces.

    Hosts will want to create one or typically more %ARA plugin extension instances per plugin for
    the purpose of playback and editor rendering. The PlaybackRegions created by the host then have
    to be assigned to these instances through the appropriate interfaces.

    Whether a PlaybackRegion or an assigned RendererInterface is deleted first depends on the host
    implementation and exact use case.

    By using these helper classes you can ensure that the %ARA DocumentController remains in a
    valid state in both situations. In order to use them acquire an object from
    PlugInExtensionInstance::getPlaybackRendererInterface() or
    PlugInExtensionInstance::getEditorRendererInterface().

    Then call add() to register a PlaybackRegion with that particular PlugInExtensionInstance's
    interface.

    Now when you delete that PlaybackRegion it will be deregistered from that extension instance.
    If however you want to delete the plugin extension instance before the PlaybackRegion, you can
    delete the PlaybackRegionRegistry instance before deleting the plugin extension instance, which
    takes care of deregistering all PlaybackRegions.

    When adding or removing PlaybackRegions the plugin instance must be in an unprepared state i.e.
    before AudioProcessor::prepareToPlay() or after AudioProcessor::releaseResources().

    @code
    auto playbackRenderer = std::make_unique<PlaybackRendererInterface> (plugInExtensionInstance.getPlaybackRendererInterface());
    auto playbackRegion   = std::make_unique<PlaybackRegion> (documentController, regionSequence, audioModification, audioSource);

    // Either of the following three code variations are valid
    // (1) ===================================================
    playbackRenderer.add (playbackRegion);
    playbackRenderer.remove (playbackRegion);

    // (2) ===================================================
    playbackRenderer.add (playbackRegion);
    playbackRegion.reset();

    // (3) ===================================================
    playbackRenderer.add (playbackRegion);
    playbackRenderer.reset();
    @endcode

    @see PluginExtensionInstance

    @tags{ARA}
*/
template <typename RendererRef, typename Interface>
class PlaybackRegionRegistry
{
public:
    PlaybackRegionRegistry() = default;

    PlaybackRegionRegistry (RendererRef rendererRefIn, const Interface* interfaceIn)
        : registry (std::make_unique<Registry> (rendererRefIn, interfaceIn))
    {
    }

    /** Adds a PlaybackRegion to the corresponding ::PlaybackRendererInterface or ::EditorRendererInterface.

        The plugin instance must be in an unprepared state i.e. before AudioProcessor::prepareToPlay() or
        after AudioProcessor::releaseResources().
     */
    void add    (PlaybackRegion& region) { registry->add    (region); }

    /** Removes a PlaybackRegion from the corresponding ::PlaybackRendererInterface or ::EditorRendererInterface.

        The plugin instance must be in an unprepared state i.e. before AudioProcessor::prepareToPlay() or
        after AudioProcessor::releaseResources().
     */
    void remove (PlaybackRegion& region) { registry->remove (region); }

    /** Returns true if the underlying %ARA plugin extension instance fulfills the corresponding role. */
    bool isValid() { return registry->isValid(); }

private:
    class Registry : private DeletionListener
    {
    public:
        Registry (RendererRef rendererRefIn, const Interface* interfaceIn)
            : rendererRef (rendererRefIn), rendererInterface (interfaceIn)
        {
        }

        Registry (const Registry&) = delete;
        Registry (Registry&&) noexcept = delete;

        Registry& operator= (const Registry&) = delete;
        Registry& operator= (Registry&&) noexcept = delete;

        ~Registry() override
        {
            for (const auto& region : regions)
                doRemoveListener (*region.first);
        }

        bool isValid() { return rendererRef != nullptr && rendererInterface != nullptr; }

        void add (PlaybackRegion& region)
        {
            if (isValid())
                rendererInterface->addPlaybackRegion (rendererRef, region.getPluginRef());

            regions.emplace (&region.getDeletionListener(), region.getPluginRef());
            region.addListener (*this);
        }

        void remove (PlaybackRegion& region)
        {
            doRemoveListener (region.getDeletionListener());
        }

    private:
        void doRemoveListener (DeletionListener& listener) noexcept
        {
            listener.removeListener (*this);
            removeListener (listener);
        }

        void removeListener (DeletionListener& listener) noexcept override
        {
            const auto it = regions.find (&listener);

            if (it == regions.end())
            {
                jassertfalse;
                return;
            }

            if (isValid())
                rendererInterface->removePlaybackRegion (rendererRef, it->second);

            regions.erase (it);
        }

        RendererRef rendererRef = nullptr;
        const Interface* rendererInterface = nullptr;
        std::map<DeletionListener*, ARA::ARAPlaybackRegionRef> regions;
    };

    std::unique_ptr<Registry> registry;
};

//==============================================================================
/** Helper class for managing the lifetimes of %ARA plugin extension instances and PlaybackRegions.

    You can read more about its usage at PlaybackRegionRegistry.

    @see PlaybackRegion, PlaybackRegionRegistry

    @tags{ARA}
*/
using PlaybackRendererInterface = PlaybackRegionRegistry<ARA::ARAPlaybackRendererRef, ARA::ARAPlaybackRendererInterface>;

//==============================================================================
/** Helper class for managing the lifetimes of %ARA plugin extension instances and PlaybackRegions.

    You can read more about its usage at PlaybackRegionRegistry.

    @see PlaybackRegion, PlaybackRegionRegistry

    @tags{ARA}
*/
using EditorRendererInterface   = PlaybackRegionRegistry<ARA::ARAEditorRendererRef,   ARA::ARAEditorRendererInterface>;

//==============================================================================
/** Wrapper class for `ARA::ARAPlugInExtensionInstance*`.

    Returned by ARAHostDocumentController::bindDocumentToPluginInstance(). The corresponding
    ARAHostDocumentController must remain valid as long as the plugin extension is in use.
*/
class PlugInExtensionInstance final
{
public:
    /** Creates an empty PlugInExtensionInstance object.

        Calling isValid() on such an object will return false.
    */
    PlugInExtensionInstance() = default;

    /** Creates a PlugInExtensionInstance object that wraps a `const ARA::ARAPlugInExtensionInstance*`.

        The intended way to obtain a PlugInExtensionInstance object is to call
        ARAHostDocumentController::bindDocumentToPluginInstance(), which is using this constructor.
    */
    explicit PlugInExtensionInstance (const ARA::ARAPlugInExtensionInstance* instanceIn)
        : instance (instanceIn)
    {
    }

    /** Returns the PlaybackRendererInterface for the extension instance.

        Depending on what roles were passed into
        ARAHostDocumentController::bindDocumentToPluginInstance() one particular instance may not
        fulfill a given role. You can use PlaybackRendererInterface::isValid() to see if this
        interface was provided by the instance.
     */
    PlaybackRendererInterface getPlaybackRendererInterface() const;

    /** Returns the EditorRendererInterface for the extension instance.

        Depending on what roles were passed into
        ARAHostDocumentController::bindDocumentToPluginInstance() one particular instance may not
        fulfill a given role. You can use EditorRendererInterface::isValid() to see if this
        interface was provided by the instance.
     */
    EditorRendererInterface   getEditorRendererInterface() const;

    /** Returns false if the PlugInExtensionInstance was default constructed and represents
        no binding to an ARAHostDocumentController.
    */
    bool isValid() const noexcept { return instance != nullptr; }

private:
    const ARA::ARAPlugInExtensionInstance* instance = nullptr;
};

} // namespace ARAHostModel

//==============================================================================
/** Wrapper class for `ARA::Host::DocumentController`.

    In order to create an ARAHostDocumentController from an ARAFactoryWrapper you must
    provide at least two mandatory host side interfaces. You can create these implementations
    by inheriting from the base classes in the `ARA::Host` namespace.

    @tags{ARA}
*/
class ARAHostDocumentController final
{
public:
    /** Factory function.

        You must check if the returned pointer is valid.
    */
    static std::unique_ptr<ARAHostDocumentController>
        create (ARAFactoryWrapper factory,
                const String& documentName,
                std::unique_ptr<ARA::Host::AudioAccessControllerInterface> audioAccessController,
                std::unique_ptr<ARA::Host::ArchivingControllerInterface> archivingController,
                std::unique_ptr<ARA::Host::ContentAccessControllerInterface> contentAccessController = nullptr,
                std::unique_ptr<ARA::Host::ModelUpdateControllerInterface> modelUpdateController = nullptr,
                std::unique_ptr<ARA::Host::PlaybackControllerInterface> playbackController = nullptr);

    ~ARAHostDocumentController();

    /** Returns the underlying ARA::Host::DocumentController reference. */
    ARA::Host::DocumentController& getDocumentController() const;

    /** Binds the ARAHostDocumentController and its enclosed document to a plugin instance.

        The resulting ARAHostModel::PlugInExtensionInstance is responsible for fulfilling the
        ARA specific roles of the plugin.

        A single DocumentController can be bound to multiple plugin instances, which is a typical
        practice among hosts.
    */
    ARAHostModel::PlugInExtensionInstance bindDocumentToPluginInstance (AudioPluginInstance& instance,
                                                                        ARA::ARAPlugInInstanceRoleFlags knownRoles,
                                                                        ARA::ARAPlugInInstanceRoleFlags assignedRoles);

private:
    class Impl;
    std::unique_ptr<Impl> impl;

    explicit ARAHostDocumentController (std::unique_ptr<Impl>&& implIn);
};

/** Calls the provided callback with an ARAFactoryWrapper object obtained from the provided
    AudioPluginInstance.

    If the provided AudioPluginInstance has no ARA extensions, the callback will be called with an
    ARAFactoryWrapper that wraps a nullptr.

    The object passed to the callback must be checked even if the plugin instance reports having
    ARA extensions.
*/
void createARAFactoryAsync (AudioPluginInstance& instance, std::function<void (ARAFactoryWrapper)> cb);

} // namespace juce

//==============================================================================
#undef ARA_REF
#undef ARA_HOST_REF

#endif
