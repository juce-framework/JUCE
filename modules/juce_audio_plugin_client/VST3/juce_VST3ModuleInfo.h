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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

#pragma once

#ifndef DOXYGEN

namespace juce
{

[[maybe_unused]] static Steinberg::FUID toSteinbergUID (const VST3Interface::Id& uid)
{
    return Steinberg::FUID::fromTUID ((const char*) (uid.data()));
}

[[maybe_unused]] static VST3Interface::Id toVST3InterfaceId (const Steinberg::TUID uid)
{
    VST3Interface::Id iid;
    std::memcpy (iid.data(), uid, iid.size());
    return iid;
}

[[maybe_unused]] static VST3Interface::Id getVST3InterfaceId (VST3Interface::Type interfaceType)
{
   #if JUCE_VST3_CAN_REPLACE_VST2
    if (interfaceType == VST3Interface::Type::controller || interfaceType == VST3Interface::Type::component)
        return VST3Interface::vst2PluginId (JucePlugin_VSTUniqueID, JucePlugin_Name, interfaceType);
   #endif

    return VST3Interface::jucePluginId (JucePlugin_ManufacturerCode, JucePlugin_PluginCode, interfaceType);
}

[[maybe_unused]] static std::vector<VST3Interface::Id> getAllVST3CompatibleClasses()
{
    return
    {
       #if JUCE_VST3_CAN_REPLACE_VST2
        getVST3InterfaceId (VST3Interface::Type::component),
       #endif

       #ifdef JUCE_VST3_COMPATIBLE_CLASSES
        JUCE_VST3_COMPATIBLE_CLASSES
       #endif
    };
}

// PClassInfoW::fromAscii widens each byte on its own, which garbles any UTF-8 text that isn't
// plain ASCII. This decodes the UTF-8 properly into a fixed-size UTF-16 buffer. We must avoid
// relying on juce_core here, so this is a small self-contained decoder.
[[maybe_unused]] static void widenNonAsciiUtf8 (Steinberg::char16* dest, size_t destSize, const char* source) noexcept
{
    const auto* p = reinterpret_cast<const unsigned char*> (source);

    // Plain ASCII was already widened correctly, so leave it untouched.
    auto isAscii = true;

    for (auto* q = p; *q != 0; ++q)
        isAscii = isAscii && *q < 0x80;

    if (isAscii)
        return;

    static constexpr char32_t minValueForLength[] { 0, 0, 0x80, 0x800, 0x10000 };
    size_t numWritten = 0;

    // Decode one code point at a time, stopping before the buffer is full.
    while (*p != 0)
    {
        char32_t codePoint = 0xfffd; // Replacement character, used for malformed input.
        size_t numBytes = 1;

        if (*p < 0x80)
        {
            codePoint = *p;
        }
        else
        {
            const size_t expected = (*p & 0xe0) == 0xc0 ? 2
                                  : (*p & 0xf0) == 0xe0 ? 3
                                  : (*p & 0xf8) == 0xf0 ? 4 : 0;

            if (expected != 0)
            {
                char32_t value = (char32_t) (*p & (0xff >> (expected + 1)));
                size_t i = 1;

                for (; i < expected && (p[i] & 0xc0) == 0x80; ++i)
                    value = (value << 6) | (char32_t) (p[i] & 0x3f);

                const auto isValid = i == expected
                                  && value >= minValueForLength[expected]
                                  && value <= 0x10ffff
                                  && ! (value >= 0xd800 && value <= 0xdfff);

                if (isValid)
                {
                    codePoint = value;
                    numBytes = expected;
                }
            }
        }

        const size_t numNeeded = codePoint >= 0x10000 ? 2 : 1;

        // Leave room for the terminator, and never split a surrogate pair.
        if (numWritten + numNeeded + 1 > destSize)
            break;

        if (numNeeded == 2)
        {
            codePoint -= 0x10000;
            dest[numWritten++] = (Steinberg::char16) (0xd800 + (codePoint >> 10));
            dest[numWritten++] = (Steinberg::char16) (0xdc00 + (codePoint & 0x3ff));
        }
        else
        {
            dest[numWritten++] = (Steinberg::char16) codePoint;
        }

        p += numBytes;
    }

    // Clear whatever fromAscii left behind, including the terminator.
    while (numWritten < destSize)
        dest[numWritten++] = 0;
}

//==============================================================================
// See https://steinbergmedia.github.io/vst3_dev_portal/pages/FAQ/Compatibility+with+VST+2.x+or+VST+1.html

#ifdef JUCE_VST3_COMPATIBLE_CLASSES
class JucePluginCompatibility final : public Steinberg::IPluginCompatibility
{
public:
    JUCE_DECLARE_VST3_COM_REF_METHODS

    Steinberg::tresult PLUGIN_API getCompatibilityJSON (Steinberg::IBStream* stream) override
    {
        // We must avoid relying on any dependencies here including anything in juce_core
        std::string json = std::invoke ([]
        {
            static const auto newId = toSteinbergUID (getVST3InterfaceId (VST3Interface::Type::component));
            static const auto oldIds = getAllVST3CompatibleClasses();

            std::stringstream str;
            str << "[{"
                << "\"New\": \"" << VST3::UID { newId }.toString() << "\", "
                << "\"Old\": [";

            for (size_t i = 0; i < oldIds.size(); ++i)
            {
                str << "\""
                    << std::hex
                    << std::setfill ('0')
                    << std::uppercase;

                for (auto byte : oldIds[i])
                    str << std::setw (2) << (int) byte;

                str.clear();
                str << "\"";

                if (i < oldIds.size() - 1)
                    str << ", ";
            }

            str << "]}]";
            return str.str();
        });

        return stream->write (json.data(), (Steinberg::int32) json.size());
    }

    Steinberg::tresult PLUGIN_API queryInterface (const Steinberg::TUID targetIID,
                                                  void** obj) override
    {
        const auto result = testForMultiple (*this,
                                             targetIID,
                                             UniqueBase<IPluginCompatibility>{},
                                             UniqueBase<FUnknown>{});

        if (result.isOk())
            return result.extract (obj);

        jassertfalse; // Something new?
        *obj = nullptr;
        return Steinberg::kNotImplemented;
    }

    inline static const Steinberg::FUID iid = toSteinbergUID (getVST3InterfaceId (VST3Interface::Type::compatibility));

private:
    std::atomic<int> refCount { 1 };
};
#endif // JUCE_VST3_COMPATIBLE_CLASSES

//==============================================================================
class JucePluginFactoryBase : public Steinberg::IPluginFactory3
{
public:
    JucePluginFactoryBase() = default;
    virtual ~JucePluginFactoryBase() = default;

    //==============================================================================
    JUCE_DECLARE_VST3_COM_REF_METHODS

    Steinberg::tresult PLUGIN_API queryInterface (const Steinberg::TUID targetIID, void** obj) final
    {
        const auto result = testForMultiple (*this,
                                             targetIID,
                                             UniqueBase<IPluginFactory3>{},
                                             UniqueBase<IPluginFactory2>{},
                                             UniqueBase<IPluginFactory>{},
                                             UniqueBase<FUnknown>{});

        if (result.isOk())
            return result.extract (obj);

        jassertfalse; // Something new?
        *obj = nullptr;
        return Steinberg::kNotImplemented;
    }

    //==============================================================================
    Steinberg::int32 PLUGIN_API countClasses() final
    {
        return (Steinberg::int32) getClassEntries().size;
    }

    Steinberg::tresult PLUGIN_API getFactoryInfo (Steinberg::PFactoryInfo* info) final
    {
        if (info == nullptr)
            return Steinberg::kInvalidArgument;

        std::memcpy (info, &factoryInfo, sizeof (factoryInfo));
        return Steinberg::kResultOk;
    }

    Steinberg::tresult PLUGIN_API getClassInfo (Steinberg::int32 index,
                                                Steinberg::PClassInfo* info) final
    {
        return getPClassInfo (index, info, &ClassEntry::info2);
    }

    Steinberg::tresult PLUGIN_API getClassInfo2 (Steinberg::int32 index,
                                                 Steinberg::PClassInfo2* info) final
    {
        return getPClassInfo (index, info, &ClassEntry::info2);
    }

    Steinberg::tresult PLUGIN_API getClassInfoUnicode (Steinberg::int32 index,
                                                       Steinberg::PClassInfoW* info) final
    {
        return getPClassInfo (index, info, &ClassEntry::infoW);
    }

    Steinberg::tresult PLUGIN_API setHostContext (Steinberg::FUnknown*) override
    {
        jassertfalse;
        return Steinberg::kNotImplemented;
    }

    Steinberg::tresult PLUGIN_API createInstance (Steinberg::FIDString cid,
                                                  Steinberg::FIDString sourceIid,
                                                  void** obj) override
    {
        *obj = nullptr;

        Steinberg::TUID tuid;
        std::memcpy (tuid, sourceIid, sizeof (tuid));

       #if VST_VERSION >= 0x030608
        auto sourceFuid = Steinberg::FUID::fromTUID (tuid);
       #else
        FUID sourceFuid;
        sourceFuid = tuid;
       #endif

        if (cid == nullptr || sourceIid == nullptr || ! sourceFuid.isValid())
        {
            jassertfalse; // The host you're running in has severe implementation issues!
            return Steinberg::kInvalidArgument;
        }

        Steinberg::TUID iidToQuery;
        sourceFuid.toTUID (iidToQuery);

        for (auto& entry : getClassEntries())
        {
            if (doUIDsMatch (entry.infoW.cid, cid))
            {
                if (VSTComSmartPtr instance { createInstance (entry), IncrementRef::no })
                {
                    if (instance->queryInterface (iidToQuery, obj) == Steinberg::kResultOk)
                        return Steinberg::kResultOk;
                }

                break;
            }
        }

        return Steinberg::kNoInterface;
    }

protected:
    //==============================================================================
    struct ClassEntry
    {
      #ifndef JucePlugin_Vst3ComponentFlags
       #if JucePlugin_IsSynth
        #define JucePlugin_Vst3ComponentFlags Steinberg::Vst::kSimpleModeSupported
       #else
        #define JucePlugin_Vst3ComponentFlags 0
       #endif
      #endif

      #ifndef JucePlugin_Vst3Category
       #if JucePlugin_IsSynth
        #define JucePlugin_Vst3Category Steinberg::Vst::PlugType::kInstrumentSynth
       #else
        #define JucePlugin_Vst3Category Steinberg::Vst::PlugType::kFx
       #endif
      #endif

        ClassEntry (VST3Interface::Type interfaceType,
                    const char* name,
                    bool includeFlagsAndCategory) noexcept
            : ClassEntry (getVST3InterfaceId (interfaceType), name, includeFlagsAndCategory)
        {}

        ClassEntry (VST3Interface::Id interfaceId,
                    const char* name,
                    bool includeFlagsAndCategory) noexcept
            : info2 ((const char*) interfaceId.data(),
                     Steinberg::PClassInfo::kManyInstances,
                     name,
                     JucePlugin_Name,
                     includeFlagsAndCategory ? JucePlugin_Vst3ComponentFlags : 0,
                     includeFlagsAndCategory ? JucePlugin_Vst3Category : "",
                     JucePlugin_Manufacturer,
                     JucePlugin_VersionString,
                     kVstVersionString)
        {
            infoW.fromAscii (info2);

            // fromAscii garbles non-ASCII text, such as a plugin name containing a Greek letter.
            widenNonAsciiUtf8 (infoW.name,       std::size (infoW.name),       info2.name);
            widenNonAsciiUtf8 (infoW.vendor,     std::size (infoW.vendor),     info2.vendor);
            widenNonAsciiUtf8 (infoW.version,    std::size (infoW.version),    info2.version);
            widenNonAsciiUtf8 (infoW.sdkVersion, std::size (infoW.sdkVersion), info2.sdkVersion);
        }

        Steinberg::PClassInfo2 info2;
        Steinberg::PClassInfoW infoW;

    private:
        JUCE_DECLARE_NON_COPYABLE (ClassEntry)
    };

    struct ClassEntrySpan
    {
        const ClassEntry* data{};
        size_t size{};

        const ClassEntry* begin() const { return data; }
        const ClassEntry* end() const { return data + size; }
    };

    static ClassEntrySpan getClassEntries()
    {
        static const ClassEntry classEntries[]
        {
           #ifdef JUCE_VST3_COMPATIBLE_CLASSES
            { VST3Interface::Type::compatibility, kPluginCompatibilityClass,    false },
           #endif
            { VST3Interface::Type::component,     kVstAudioEffectClass,         true },
            { VST3Interface::Type::controller,    kVstComponentControllerClass, true },
           #if JucePlugin_Enable_ARA
            { VST3Interface::Type::ara,           kARAMainFactoryClass,         true }
           #endif
        };

        return { classEntries, std::size (classEntries) };
    }

    virtual Steinberg::FUnknown* createInstance (const ClassEntry& entry)
    {
       #ifdef JUCE_VST3_COMPATIBLE_CLASSES
        if (doUIDsMatch (entry.info2.cid, JucePluginCompatibility::iid.toTUID()))
            return new JucePluginCompatibility();
       #else
        (void) entry;
       #endif

        jassertfalse;
        return {};
    }

private:
    const ClassEntry& getClassEntry (Steinberg::int32 index) const
    {
        return getClassEntries().data[index];
    }

    //==============================================================================
    template <typename PClassInfoTargetType, typename PClassInfoSourceType>
    Steinberg::tresult PLUGIN_API getPClassInfo (Steinberg::int32 index,
                                                 PClassInfoTargetType* info,
                                                 PClassInfoSourceType ClassEntry::*source)
    {
        if (info == nullptr)
        {
            jassertfalse;
            return Steinberg::kInvalidArgument;
        }

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wclass-memaccess")
        std::memcpy (info, &(getClassEntry (index).*source), sizeof (*info));
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        return Steinberg::kResultOk;
    }

    //==============================================================================
    std::atomic<int> refCount { 1 };
    const Steinberg::PFactoryInfo factoryInfo
    {
        JucePlugin_Manufacturer,
        JucePlugin_ManufacturerWebsite,
        JucePlugin_ManufacturerEmail,
        Steinberg::Vst::kDefaultFactoryFlags
    };

    //==============================================================================
    // no leak detector here to prevent it firing on shutdown when running in hosts that
    // don't release the factory object correctly...
    JUCE_DECLARE_NON_COPYABLE (JucePluginFactoryBase)
};

} // namespace juce

#endif // DOXYGEN
