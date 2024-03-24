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

#pragma once

#ifndef DOXYGEN

#include "juce_lv2_config.h"

#ifdef Bool
 #undef Bool // previously defined in X11/Xlib.h
#endif

#ifdef verify
 #undef verify // previously defined in macOS 10.11 SDK
#endif

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant",
                                     "-Wcast-align",
                                     "-Wparentheses",
                                     "-Wnullability-extension",
                                     "-Wsign-conversion")

extern "C"
{
    #include "lilv/lilv/lilv.h"
    #include "lv2/atom/atom.h"
    #include "lv2/atom/forge.h"
    #include "lv2/atom/util.h"
    #include "lv2/buf-size/buf-size.h"
    #include "lv2/data-access/data-access.h"
    #include "lv2/dynmanifest/dynmanifest.h"
    #include "lv2/instance-access/instance-access.h"
    #include "lv2/log/log.h"
    #include "lv2/midi/midi.h"
    #include "lv2/options/options.h"
    #include "lv2/parameters/parameters.h"
    #include "lv2/patch/patch.h"
    #include "lv2/port-groups/port-groups.h"
    #include "lv2/presets/presets.h"
    #include "lv2/resize-port/resize-port.h"
    #include "lv2/state/state.h"
    #include "lv2/time/time.h"
    #include "lv2/ui/ui.h"
    #include "lv2/units/units.h"
    #include "lv2/worker/worker.h"
    #include "serd/serd/serd.h"
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#include <map>
#include <type_traits>

namespace juce::lv2_shared
{

class AtomForge
{
public:
    explicit AtomForge (LV2_URID_Map m)
        : map { m },
          chunk { map.map (map.handle, LV2_ATOM__Chunk) }
    {
        lv2_atom_forge_init (&forge, &map);
    }

    void setBuffer (void* buf, size_t size)
    {
        lv2_atom_forge_set_buffer (&forge, static_cast<uint8_t*> (buf), size);
    }

          LV2_Atom_Forge* get()       { return &forge; }
    const LV2_Atom_Forge* get() const { return &forge; }

    void writeChunk (uint32_t size)
    {
        lv2_atom_forge_atom (&forge, size, chunk);
    }

private:
    LV2_URID_Map map;
    LV2_Atom_Forge forge;
    const LV2_URID chunk;

    JUCE_LEAK_DETECTOR (AtomForge)
};

template <typename Constructor>
struct ScopedFrame
{
    template <typename... Args>
    explicit ScopedFrame (LV2_Atom_Forge* f, Args&&... args)
        : forge (f)
    {
        Constructor::construct (forge, &frame, std::forward<Args> (args)...);
    }

    ~ScopedFrame() { lv2_atom_forge_pop (forge, &frame); }

    LV2_Atom_Forge_Frame frame;
    LV2_Atom_Forge* forge = nullptr;

    JUCE_DECLARE_NON_COPYABLE (ScopedFrame)
    JUCE_DECLARE_NON_MOVEABLE (ScopedFrame)
    JUCE_LEAK_DETECTOR (ScopedFrame)
};

struct SequenceTraits { static constexpr auto construct = lv2_atom_forge_sequence_head; };
struct ObjectTraits   { static constexpr auto construct = lv2_atom_forge_object; };

using SequenceFrame = ScopedFrame<SequenceTraits>;
using ObjectFrame   = ScopedFrame<ObjectTraits>;

struct NumericAtomParser
{
    explicit NumericAtomParser (LV2_URID_Map mapFeatureIn)
        : mapFeature (mapFeatureIn) {}

    template <typename T> struct Tag { LV2_URID urid; };

    template <typename Target, typename... Types>
    static Optional<Target> tryParse (const LV2_Atom&, const void*)
    {
        return {};
    }

    template <typename Target, typename Head, typename... Tail>
    static Optional<Target> tryParse (const LV2_Atom& atom, const void* data, Tag<Head> head, Tag<Tail>... tail)
    {
        if (atom.type == head.urid && atom.size == sizeof (Head))
            return static_cast<Target> (*reinterpret_cast<const Head*> (data));

        return tryParse<Target> (atom, data, tail...);
    }

    template <typename Target>
    Optional<Target> parseNumericAtom (const LV2_Atom* atom, const void* data) const
    {
        if (atom == nullptr)
            return {};

        return tryParse<Target> (*atom,
                                 data,
                                 Tag<int32_t> { mLV2_ATOM__Bool },
                                 Tag<int32_t> { mLV2_ATOM__Int },
                                 Tag<int64_t> { mLV2_ATOM__Long },
                                 Tag<float>   { mLV2_ATOM__Float },
                                 Tag<double>  { mLV2_ATOM__Double });
    }

    template <typename Target>
    Optional<Target> parseNumericAtom (const LV2_Atom* atom) const
    {
        return parseNumericAtom<Target> (atom, atom + 1);
    }

    template <typename Target>
    Optional<Target> parseNumericOption (const LV2_Options_Option* option) const
    {
        if (option != nullptr)
        {
            const LV2_Atom atom { option->size, option->type };
            return parseNumericAtom<Target> (&atom, option->value);
        }

        return {};
    }

    LV2_URID map (const char* str) const { return mapFeature.map (mapFeature.handle, str); }

    const LV2_URID_Map mapFeature;
   #define X(str) const LV2_URID m##str = map (str);
    X (LV2_ATOM__Double)
    X (LV2_ATOM__Float)
    X (LV2_ATOM__Int)
    X (LV2_ATOM__Long)
    X (LV2_ATOM__Bool)
   #undef X

    JUCE_LEAK_DETECTOR (NumericAtomParser)
};

//==============================================================================
struct PatchSetHelper
{
    PatchSetHelper (LV2_URID_Map mapFeatureIn, const char* pluginUri)
        : parser (mapFeatureIn),
          pluginUrid (parser.map (pluginUri))
    {}

    bool isPlugin (const LV2_Atom* subject) const
    {
        if (subject == nullptr)
            return true;

        return subject->type == mLV2_ATOM__URID
               && reinterpret_cast<const LV2_Atom_URID*> (subject)->body == pluginUrid;
    }

    template <typename Callback>
    void processPatchSet (const LV2_Atom_Object* object, Callback&& callback)
    {
        if (object->body.otype != mLV2_PATCH__Set)
            return;

        const LV2_Atom* subject  = nullptr;
        const LV2_Atom* property = nullptr;
        const LV2_Atom* value    = nullptr;

        LV2_Atom_Object_Query query[] { { mLV2_PATCH__subject,  &subject },
                                        { mLV2_PATCH__property, &property },
                                        { mLV2_PATCH__value,    &value },
                                        LV2_ATOM_OBJECT_QUERY_END };

        lv2_atom_object_query (object, query);

        if (! isPlugin (subject))
            return;

        setPluginProperty (property, value, std::forward<Callback> (callback));
    }

    template <typename Callback>
    void processPatchSet (const LV2_Atom_Event* event, Callback&& callback)
    {
        if (event->body.type == mLV2_ATOM__Object)
            processPatchSet (reinterpret_cast<const LV2_Atom_Object*> (&event->body), std::forward<Callback> (callback));
    }

    template <typename Callback>
    void setPluginProperty (const LV2_Atom* property, const LV2_Atom* value, Callback&& callback)
    {
        if (property == nullptr)
        {
            // No property was supplied.
            jassertfalse;
            return;
        }

        if (property->type != mLV2_ATOM__URID)
        {
            // Set property is not a URID.
            jassertfalse;
            return;
        }

        const auto parseResult = parser.parseNumericAtom<float> (value);

        if (! parseResult.hasValue())
        {
            // Didn't understand the type of this atom.
            jassertfalse;
            return;
        }

        callback.setParameter (reinterpret_cast<const LV2_Atom_URID*> (property)->body, *parseResult);
    }

    NumericAtomParser parser;
    LV2_URID pluginUrid;
   #define X(str) const LV2_URID m##str = parser.map (str);
    X (LV2_ATOM__Bool)
    X (LV2_ATOM__Object)
    X (LV2_ATOM__URID)
    X (LV2_PATCH__Set)
    X (LV2_PATCH__property)
    X (LV2_PATCH__subject)
    X (LV2_PATCH__value)
   #undef X

    JUCE_LEAK_DETECTOR (PatchSetHelper)
};

//==============================================================================
template <typename Traits>
class Iterator
{
    using Container = typename Traits::Container;
    using Iter = typename Traits::Iter;

public:
    using difference_type = ptrdiff_t;
    using value_type = decltype (Traits::get (std::declval<Container>(), std::declval<Iter>()));
    using pointer = value_type*;
    using reference = value_type;
    using iterator_category = std::input_iterator_tag;

    /** Create iterator in end/sentinel state */
    Iterator() = default;

    /** Create iterator pointing to the beginning of this collection */
    explicit Iterator (Container p) noexcept
        : container (p), iter (testEnd (Traits::begin (container))) {}

    Iterator begin() const noexcept { return *this; }
    Iterator end()   const noexcept { return {}; }

    bool operator== (const Iterator& other) const noexcept
    {
        return iter == nullptr && other.iter == nullptr;
    }

    bool operator!= (const Iterator& other) const noexcept
    {
        return ! operator== (other);
    }

    Iterator& operator++()
    {
        iter = testEnd (Traits::next (container, iter));
        return *this;
    }

    Iterator operator++ (int)
    {
        auto copy = *this;
        ++(*this);
        return copy;
    }

    reference operator*() const noexcept
    {
        return Traits::get (container, iter);
    }

private:
    Iter testEnd (Iter i) const noexcept
    {
        return Traits::isEnd (container, i) ? Iter{} : i;
    }

    Container container{};
    Iter iter{};
};

//==============================================================================
struct SequenceWithSize
{
    SequenceWithSize() = default;

    SequenceWithSize (const LV2_Atom_Sequence_Body* bodyIn, size_t sizeIn)
        : body (bodyIn), size (sizeIn) {}

    explicit SequenceWithSize (const LV2_Atom_Sequence* sequence)
        : body (&sequence->body), size (sequence->atom.size) {}

    const LV2_Atom_Sequence_Body* body = nullptr;
    size_t size = 0;

    JUCE_LEAK_DETECTOR (SequenceWithSize)
};

struct SequenceIteratorTraits
{
    using Container     = SequenceWithSize;
    using Iter          = LV2_Atom_Event*;

    static LV2_Atom_Event* begin (const Container& s) noexcept
    {
        return lv2_atom_sequence_begin (s.body);
    }

    static LV2_Atom_Event* next (const Container&, Iter it) noexcept
    {
        return lv2_atom_sequence_next (it);
    }

    static bool isEnd (const Container& s, Iter it) noexcept
    {
        return lv2_atom_sequence_is_end (s.body, static_cast<uint32_t> (s.size), it);
    }

    static LV2_Atom_Event* get (const Container&, Iter e) noexcept { return e; }
};

using SequenceIterator = Iterator<SequenceIteratorTraits>;

const std::map<String, AudioChannelSet::ChannelType> channelDesignationMap
{
    { LV2_PORT_GROUPS__center,              AudioChannelSet::ChannelType::centre },
    { LV2_PORT_GROUPS__centerLeft,          AudioChannelSet::ChannelType::leftCentre },
    { LV2_PORT_GROUPS__centerRight,         AudioChannelSet::ChannelType::rightCentre },
    { LV2_PORT_GROUPS__left,                AudioChannelSet::ChannelType::left },
    { LV2_PORT_GROUPS__lowFrequencyEffects, AudioChannelSet::ChannelType::LFE },
    { LV2_PORT_GROUPS__rearCenter,          AudioChannelSet::ChannelType::surround },
    { LV2_PORT_GROUPS__rearLeft,            AudioChannelSet::ChannelType::leftSurroundRear },
    { LV2_PORT_GROUPS__rearRight,           AudioChannelSet::ChannelType::rightSurroundRear },
    { LV2_PORT_GROUPS__right,               AudioChannelSet::ChannelType::right },
    { LV2_PORT_GROUPS__sideLeft,            AudioChannelSet::ChannelType::leftSurroundSide },
    { LV2_PORT_GROUPS__sideRight,           AudioChannelSet::ChannelType::rightSurroundSide }
};

/*  Useful for converting a `void*` to another type (X11 Window, function pointer etc.) without
    invoking UB.
*/
template <typename OtherWordType, typename Word>
static auto wordCast (Word word)
{
    static_assert (sizeof (word) == sizeof (OtherWordType), "Word sizes must match");
    return readUnaligned<OtherWordType> (&word);
}

//==============================================================================
struct SinglePortInfo
{
    uint32_t index;
    AudioChannelSet::ChannelType designation;
    bool optional;

    auto tie() const { return std::tie (index, designation, optional); }
    bool operator== (const SinglePortInfo& other) const { return tie() == other.tie(); }
    bool operator<  (const SinglePortInfo& other) const { return index < other.index; }
};

struct ParsedGroup
{
    String uid;
    std::set<SinglePortInfo> info;

    auto tie() const { return std::tie (uid, info); }
    bool operator== (const ParsedGroup& other) const { return tie() == other.tie(); }

    static AudioChannelSet getEquivalentSet (const std::set<SinglePortInfo>& info)
    {
        const auto hasUnknownKind = [] (const SinglePortInfo& i) { return i.designation == AudioChannelSet::unknown; };

        if (std::any_of (info.begin(), info.end(), hasUnknownKind))
            return AudioChannelSet::discreteChannels ((int) info.size());

        AudioChannelSet result;

        for (const auto& port : info)
            result.addChannel (port.designation);

        return result;
    }

    AudioChannelSet getEquivalentSet() const noexcept { return getEquivalentSet (info); }

    bool isRequired() const
    {
        const auto getRequired = [] (const SinglePortInfo& i) { return ! i.optional; };
        return std::any_of (info.begin(), info.end(), getRequired);
    }

    bool isCompatible (const AudioChannelSet& requestedBus) const
    {
        return requestedBus == getEquivalentSet() || (! isRequired() && requestedBus.isDisabled());
    }
};

struct ParsedBuses
{
    std::vector<ParsedGroup> inputs, outputs;
};

class PortToAudioBufferMap
{
public:
    PortToAudioBufferMap (const AudioProcessor::BusesLayout& layout, const ParsedBuses& buses)
        : PortToAudioBufferMap ({ layout.inputBuses,  buses.inputs },
                                { layout.outputBuses, buses.outputs })
    {}

    int getChannelForPort (uint32_t port) const
    {
        const auto it = ports.find (port);
        return it != ports.end() ? it->second : -1;
    }

private:
    PortToAudioBufferMap (const Array<AudioChannelSet>& host,
                          const std::vector<ParsedGroup>& client)
        : ports (getPortLayout (host, client))
    {}

    PortToAudioBufferMap (const PortToAudioBufferMap& inputs,
                          const PortToAudioBufferMap& outputs)
    {
        ports.insert (inputs.ports.begin(),  inputs.ports.end());
        ports.insert (outputs.ports.begin(), outputs.ports.end());

        // If this assertion is hit, some ports have duplicate indices
        jassert (ports.size() == inputs.ports.size() + outputs.ports.size());
    }

    static std::map<uint32_t, int> getPortLayout (const Array<AudioChannelSet>& layout,
                                                  const std::vector<ParsedGroup>& parsedGroup)
    {
        if ((int) parsedGroup.size() != layout.size())
        {
            // Something has gone very wrong when computing/applying bus layouts!
            jassertfalse;
            return {};
        }

        std::map<uint32_t, int> result;
        int channelOffsetOfBus = 0;
        auto groupIterator = parsedGroup.begin();

        for (const auto& bus : layout)
        {
            const auto& group = groupIterator->info;

            for (const auto& port : group)
            {
                const auto index = bus.getChannelIndexForType (port.designation);

                if (index >= 0)
                    result.emplace (port.index, channelOffsetOfBus + index);
            }

            channelOffsetOfBus += bus.size();
            ++groupIterator;
        }

        if ((int) result.size() != channelOffsetOfBus)
        {
            jassertfalse;
            return {};
        }

        return result;
    }

    std::map<uint32_t, int> ports;

    JUCE_LEAK_DETECTOR (PortToAudioBufferMap)
};

// This will convert some grouped and ungrouped ports into a single collection of
// buses with a stable order.
// If any group has been marked as the main group, this will be placed first in the
// collection of results. The remaining groups will be sorted according to the
// indices of their ports.
// If there are no groups, all mandatory ports will be grouped into the first bus,
// and all remaining optional ports will have a separate bus each.
static inline std::vector<ParsedGroup> findStableBusOrder (const String& mainGroupUid,
                                                           const std::map<String, std::set<SinglePortInfo>>& groupedPorts,
                                                           const std::set<SinglePortInfo>& ungroupedPorts)
{
    if (groupedPorts.empty())
    {
        std::vector<ParsedGroup> result;

        ParsedGroup mandatoryPorts;

        for (const auto& port : ungroupedPorts)
            if (! port.optional)
                mandatoryPorts.info.insert (port);

        if (! mandatoryPorts.info.empty())
            result.push_back (std::move (mandatoryPorts));

        for (const auto& port : ungroupedPorts)
            if (port.optional)
                result.push_back ({ String{}, { port } });

        return result;
    }

    std::vector<ParsedGroup> result;

    const auto pushGroup = [&] (const std::pair<String, std::set<SinglePortInfo>>& info)
    {
        result.push_back ({ info.first, info.second });
    };

    const auto mainGroupIterator = groupedPorts.find (mainGroupUid);

    if (mainGroupIterator != groupedPorts.end())
        pushGroup (*mainGroupIterator);

    for (auto it = groupedPorts.begin(); it != groupedPorts.end(); ++it)
        if (it != mainGroupIterator)
            pushGroup (*it);

    for (const auto& info : ungroupedPorts)
        result.push_back ({ String{}, { info } });

    if (! result.empty())
    {
        // It is an error for the same port to be a member of multiple groups.
        // Therefore, a standard sort will always be stable, and we don't need to
        // use an explicitly stable sort.
        const auto compare = [] (const ParsedGroup& a, const ParsedGroup& b) { return a.info < b.info; };
        std::sort (std::next (result.begin()), result.end(), compare);
    }

    return result;
}

/*  See https://www.w3.org/TeamSubmission/turtle/#sec-grammar-grammar
*/
static inline bool isNameStartChar (juce_wchar input)
{
    return ('A' <= input && input <= 'Z')
        || input == '_'
        || ('a' <= input && input <= 'z')
        || (0x000c0 <= input && input <= 0x000d6)
        || (0x000d8 <= input && input <= 0x000f6)
        || (0x000f8 <= input && input <= 0x000ff)
        || (0x00370 <= input && input <= 0x0037d)
        || (0x0037f <= input && input <= 0x01fff)
        || (0x0200c <= input && input <= 0x0200d)
        || (0x02070 <= input && input <= 0x0218f)
        || (0x02c00 <= input && input <= 0x02fef)
        || (0x03001 <= input && input <= 0x0d7ff)
        || (0x0f900 <= input && input <= 0x0fdcf)
        || (0x0fdf0 <= input && input <= 0x0fffd)
        || (0x10000 <= input && input <= 0xeffff);
}

static inline bool isNameChar (juce_wchar input)
{
    return isNameStartChar (input)
        || input == '-'
        || ('0' <= input && input <= '9')
        || input == 0x000b7
        || (0x00300 <= input && input <= 0x0036f)
        || (0x0203f <= input && input <= 0x02040);
}

static inline String sanitiseStringAsTtlName (const String& input)
{
    if (input.isEmpty())
        return {};

    std::vector<juce_wchar> sanitised;
    sanitised.reserve (static_cast<size_t> (input.length()));

    sanitised.push_back (isNameStartChar (input[0]) ? input[0] : '_');

    std::for_each (std::begin (input) + 1, std::end (input), [&] (juce_wchar x)
    {
        sanitised.push_back (isNameChar (x) ? x : '_');
    });

    return String (CharPointer_UTF32 { sanitised.data() }, sanitised.size());
}

} // namespace juce::lv2_shared

#endif
