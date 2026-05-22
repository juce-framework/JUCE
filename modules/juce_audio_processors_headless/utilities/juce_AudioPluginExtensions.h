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

/** @cond */

// Forward declarations to avoid leaking implementation details.
namespace Steinberg::Vst
{
    class IComponent;
} // namespace Steinberg::Vst

/** @endcond */

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

/** A collection of interfaces for interacting with hosted plugins in a plugin-format-specific way.

    @tags{Audio}
*/
struct AudioPluginExtensions
{
    AudioPluginExtensions() = delete;

    /** Can be used to retrieve information about a VST3 that is wrapped by an AudioProcessor. */
    struct VST3Client
    {
        virtual ~VST3Client() = default;
        virtual Steinberg::Vst::IComponent* getIComponentPtr() const noexcept = 0;
        virtual MemoryBlock getPreset() const = 0;
        virtual bool setPreset (const MemoryBlock&) = 0;
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

        /** Attempts to retrieve the VSTXML data from a plugin.
            Will return nullptr if the plugin doesn't have any VSTXML.
        */
        virtual const XmlElement* getVSTXML() const = 0;

        /** Attempts to reload a VST plugin's state from some FXB or FXP data. */
        virtual bool loadFromFXBFile (Span<const std::byte> data) = 0;

        /** Attempts to save a VST's state to some FXP or FXB data. */
        virtual bool saveToFXBFile (MemoryBlock& result, bool asFXB) = 0;

        /** Attempts to set a VST's state from a chunk of memory. */
        virtual bool setChunkData (Span<const std::byte> data, bool isPreset) = 0;

        /** Attempts to get a VST's state as a chunk of memory. */
        virtual bool getChunkData (MemoryBlock& result, bool isPreset) const = 0;

        //==============================================================================
        /** Base class for some extra functions that can be attached to a VST plugin instance. */
        class ExtraFunctions
        {
        public:
            virtual ~ExtraFunctions() = default;

            /** This should return 10000 * the BPM at this position in the current edit. */
            virtual int64 getTempoAt (int64 samplePos) = 0;

            /** This should return the host's automation state.
                @returns 0 = not supported, 1 = off, 2 = read, 3 = write, 4 = read/write
            */
            virtual int getAutomationState() = 0;
        };

        /** Provides an ExtraFunctions callback object for a plugin to use.
            The plugin will take ownership of the object and will delete it automatically.
        */
        virtual void setExtraFunctions (std::unique_ptr<ExtraFunctions> functions) = 0;

        /** This simply calls directly to the VST's AEffect::dispatcher() function. */
        virtual pointer_sized_int dispatcher (int32, int32, pointer_sized_int, void*, float) = 0;
    };

    /** Can be used to retrieve information about a plugin that provides ARA extensions. */
    struct ARAClient
    {
        virtual ~ARAClient() = default;
        virtual void createARAFactoryAsync (std::function<void (ARAFactoryWrapper)>) const = 0;
    };
};

} // namespace juce
