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

#ifndef DOXYGEN

// Forward declarations to avoid leaking implementation details.
namespace Steinberg::Vst
{
    class IComponent;
} // namespace Steinberg::Vst

#endif

//==============================================================================
#if TARGET_OS_IPHONE
struct OpaqueAudioComponentInstance;
typedef struct OpaqueAudioComponentInstance* AudioComponentInstance;
#else
struct ComponentInstanceRecord;
typedef struct ComponentInstanceRecord* AudioComponentInstance;
#endif

typedef AudioComponentInstance AudioUnit;

//==============================================================================
/*  If you are including the VST headers inside a namespace this forward
    declaration may cause a collision with the contents of `aeffect.h`.

    If that is the case you can avoid the collision by placing a `struct AEffect;`
    forward declaration inside the namespace and before the inclusion of the VST
    headers, e.g. @code

    namespace Vst2
    {
    struct AEffect;
    #include <pluginterfaces/vst2.x/aeffect.h>
    #include <pluginterfaces/vst2.x/aeffectx.h>
    }
    @endcode
*/
struct AEffect;

//==============================================================================
namespace juce
{

//==============================================================================
/** Create a derived implementation of this class and pass it to
    AudioPluginInstance::getExtensions() to retrieve format-specific
    information about a plugin instance.

    Note that the references passed to the visit member functions are only
    guaranteed to live for the duration of the function call, so don't
    store pointers to these objects! If you need to store and reuse
    format-specific information, it is recommended to copy the result
    of the function calls that you care about. For example, you should
    store the result of VST::getAEffectPtr() rather than storing a pointer
    to the VST instance.

    @tags{Audio}
*/
struct ExtensionsVisitor
{
    /** Indicates that there is no platform specific information available. */
    struct Unknown {};

    /** Can be used to retrieve information about a VST3 that is wrapped by an AudioProcessor. */
    struct VST3Client
    {
        virtual ~VST3Client() = default;
        virtual Steinberg::Vst::IComponent* getIComponentPtr() const noexcept = 0;

        virtual MemoryBlock getPreset() const = 0;
        virtual bool setPreset (const MemoryBlock&) const = 0;
    };

    /** Can be used to retrieve information about an AudioUnit that is wrapped by an AudioProcessor. */
    struct AudioUnitClient
    {
        virtual ~AudioUnitClient() = default;
        virtual AudioUnit getAudioUnitHandle() const noexcept = 0;
    };

    /** Can be used to retrieve information about a VST that is wrapped by an AudioProcessor. */
    struct VSTClient
    {
        virtual ~VSTClient() = default;
        virtual AEffect* getAEffectPtr() const noexcept = 0;
    };

    /** Can be used to retrieve information about a plugin that provides ARA extensions. */
    struct ARAClient
    {
        virtual ~ARAClient() = default;
        virtual void createARAFactoryAsync (std::function<void (ARAFactoryWrapper)>) const = 0;
    };

    ExtensionsVisitor() = default;

    ExtensionsVisitor (const ExtensionsVisitor&) = default;
    ExtensionsVisitor (ExtensionsVisitor&&) = default;

    ExtensionsVisitor& operator= (const ExtensionsVisitor&) = default;
    ExtensionsVisitor& operator= (ExtensionsVisitor&&) = default;

    virtual ~ExtensionsVisitor() = default;

    /** Will be called if there is no platform specific information available. */
    virtual void visitUnknown           (const Unknown&)         {}

    /** Called with VST3-specific information. */
    virtual void visitVST3Client        (const VST3Client&)      {}

    /** Called with VST-specific information. */
    virtual void visitVSTClient         (const VSTClient&)       {}

    /** Called with AU-specific information. */
    virtual void visitAudioUnitClient   (const AudioUnitClient&) {}

    /** Called with ARA-specific information. */
    virtual void visitARAClient         (const ARAClient&)       {}
};

} // namespace juce
