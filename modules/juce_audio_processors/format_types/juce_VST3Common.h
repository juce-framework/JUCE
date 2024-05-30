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

namespace juce
{

JUCE_BEGIN_NO_SANITIZE ("vptr")

//==============================================================================
#define JUCE_DECLARE_VST3_COM_REF_METHODS \
    Steinberg::uint32 PLUGIN_API addRef() override   { return (Steinberg::uint32) ++refCount; } \
    Steinberg::uint32 PLUGIN_API release() override  { const int r = --refCount; if (r == 0) delete this; return (Steinberg::uint32) r; }

#define JUCE_DECLARE_VST3_COM_QUERY_METHODS \
    Steinberg::tresult PLUGIN_API queryInterface (const Steinberg::TUID, void** obj) override \
    { \
        jassertfalse; \
        *obj = nullptr; \
        return Steinberg::kNotImplemented; \
    }

inline bool doUIDsMatch (const Steinberg::TUID a, const Steinberg::TUID b) noexcept
{
    return std::memcmp (a, b, sizeof (Steinberg::TUID)) == 0;
}

/*  Holds a tresult and a pointer to an object.

    Useful for holding intermediate results of calls to queryInterface.
*/
class QueryInterfaceResult
{
public:
    QueryInterfaceResult() = default;

    QueryInterfaceResult (Steinberg::tresult resultIn, void* ptrIn)
        : result (resultIn), ptr (ptrIn) {}

    bool isOk() const noexcept   { return result == Steinberg::kResultOk; }

    Steinberg::tresult extract (void** obj) const
    {
        *obj = result == Steinberg::kResultOk ? ptr : nullptr;
        return result;
    }

private:
    Steinberg::tresult result = Steinberg::kResultFalse;
    void* ptr = nullptr;
};

/*  Holds a tresult and a pointer to an object.

    Calling InterfaceResultWithDeferredAddRef::extract() will also call addRef
    on the pointed-to object. It is expected that users will use
    InterfaceResultWithDeferredAddRef to hold intermediate results of a queryInterface
    call. When a suitable interface is found, the function can be exited with
    `return suitableInterface.extract (obj)`, which will set the obj pointer,
    add a reference to the interface, and return the appropriate result code.
*/
class InterfaceResultWithDeferredAddRef
{
public:
    InterfaceResultWithDeferredAddRef() = default;

    template <typename Ptr>
    InterfaceResultWithDeferredAddRef (Steinberg::tresult resultIn, Ptr* ptrIn)
        : result (resultIn, ptrIn),
          addRefFn (doAddRef<Ptr>) {}

    bool isOk() const noexcept   { return result.isOk(); }

    Steinberg::tresult extract (void** obj) const
    {
        const auto toReturn = result.extract (obj);

        if (result.isOk() && *obj != nullptr)
            NullCheckedInvocation::invoke (addRefFn, *obj);

        return toReturn;
    }

private:
    template <typename Ptr>
    static void doAddRef (void* obj)   { static_cast<Ptr*> (obj)->addRef(); }

    QueryInterfaceResult result;
    void (*addRefFn) (void*) = nullptr;
};

template <typename ClassType>                                   struct UniqueBase {};
template <typename CommonClassType, typename SourceClassType>   struct SharedBase {};

template <typename ToTest, typename CommonClassType, typename SourceClassType>
InterfaceResultWithDeferredAddRef testFor (ToTest& toTest,
                                           const Steinberg::TUID targetIID,
                                           SharedBase<CommonClassType, SourceClassType>)
{
    if (! doUIDsMatch (targetIID, CommonClassType::iid))
        return {};

    return { Steinberg::kResultOk, static_cast<CommonClassType*> (static_cast<SourceClassType*> (std::addressof (toTest))) };
}

template <typename ToTest, typename ClassType>
InterfaceResultWithDeferredAddRef testFor (ToTest& toTest,
                                           const Steinberg::TUID targetIID,
                                           UniqueBase<ClassType>)
{
    return testFor (toTest, targetIID, SharedBase<ClassType, ClassType>{});
}

template <typename ToTest>
InterfaceResultWithDeferredAddRef testForMultiple (ToTest&, const Steinberg::TUID) { return {}; }

template <typename ToTest, typename Head, typename... Tail>
InterfaceResultWithDeferredAddRef testForMultiple (ToTest& toTest, const Steinberg::TUID targetIID, Head head, Tail... tail)
{
    const auto result = testFor (toTest, targetIID, head);

    if (result.isOk())
        return result;

    return testForMultiple (toTest, targetIID, tail...);
}

//==============================================================================
#if VST_VERSION < 0x030608
 #define kAmbi1stOrderACN kBFormat
#endif

//==============================================================================
inline juce::String toString (const Steinberg::char8* string) noexcept       { return juce::String (juce::CharPointer_UTF8  ((juce::CharPointer_UTF8::CharType*)  string)); }
inline juce::String toString (const Steinberg::char16* string) noexcept      { return juce::String (juce::CharPointer_UTF16 ((juce::CharPointer_UTF16::CharType*) string)); }

// NB: The casts are handled by a Steinberg::UString operator
inline juce::String toString (const Steinberg::UString128& string) noexcept  { return toString (static_cast<const Steinberg::char16*> (string)); }
inline juce::String toString (const Steinberg::UString256& string) noexcept  { return toString (static_cast<const Steinberg::char16*> (string)); }

inline Steinberg::Vst::TChar* toString (const juce::String& source) noexcept { return reinterpret_cast<Steinberg::Vst::TChar*> (source.toUTF16().getAddress()); }

inline void toString128 (Steinberg::Vst::String128 result, const char* source)
{
    Steinberg::UString (result, 128).fromAscii (source);
}

inline void toString128 (Steinberg::Vst::String128 result, const juce::String& source)
{
    Steinberg::UString (result, 128).assign (toString (source));
}

#if JUCE_WINDOWS
 static const Steinberg::FIDString defaultVST3WindowType = Steinberg::kPlatformTypeHWND;
#elif JUCE_MAC
 static const Steinberg::FIDString defaultVST3WindowType = Steinberg::kPlatformTypeNSView;
#elif JUCE_LINUX || JUCE_BSD
 static const Steinberg::FIDString defaultVST3WindowType = Steinberg::kPlatformTypeX11EmbedWindowID;
#endif


//==============================================================================
static inline Steinberg::Vst::SpeakerArrangement getArrangementForBus (Steinberg::Vst::IAudioProcessor* processor,
                                                                       bool isInput, int busIndex)
{
    Steinberg::Vst::SpeakerArrangement arrangement = Steinberg::Vst::SpeakerArr::kEmpty;

    if (processor != nullptr)
        processor->getBusArrangement (isInput ? Steinberg::Vst::kInput : Steinberg::Vst::kOutput,
                                      (Steinberg::int32) busIndex, arrangement);

    return arrangement;
}

static std::optional<Steinberg::Vst::Speaker> getSpeakerType (const AudioChannelSet& set, AudioChannelSet::ChannelType type) noexcept
{
    switch (type)
    {
        case AudioChannelSet::left:              return Steinberg::Vst::kSpeakerL;
        case AudioChannelSet::right:             return Steinberg::Vst::kSpeakerR;
        case AudioChannelSet::centre:            return (set == AudioChannelSet::mono() ? Steinberg::Vst::kSpeakerM : Steinberg::Vst::kSpeakerC);

        case AudioChannelSet::LFE:               return Steinberg::Vst::kSpeakerLfe;
        case AudioChannelSet::leftSurround:      return Steinberg::Vst::kSpeakerLs;
        case AudioChannelSet::rightSurround:     return Steinberg::Vst::kSpeakerRs;
        case AudioChannelSet::leftCentre:        return Steinberg::Vst::kSpeakerLc;
        case AudioChannelSet::rightCentre:       return Steinberg::Vst::kSpeakerRc;
        case AudioChannelSet::centreSurround:    return Steinberg::Vst::kSpeakerCs;
        case AudioChannelSet::leftSurroundSide:  return Steinberg::Vst::kSpeakerSl;
        case AudioChannelSet::rightSurroundSide: return Steinberg::Vst::kSpeakerSr;
        case AudioChannelSet::topMiddle:         return Steinberg::Vst::kSpeakerTc; /* kSpeakerTm */
        case AudioChannelSet::topFrontLeft:      return Steinberg::Vst::kSpeakerTfl;
        case AudioChannelSet::topFrontCentre:    return Steinberg::Vst::kSpeakerTfc;
        case AudioChannelSet::topFrontRight:     return Steinberg::Vst::kSpeakerTfr;
        case AudioChannelSet::topRearLeft:       return Steinberg::Vst::kSpeakerTrl;
        case AudioChannelSet::topRearCentre:     return Steinberg::Vst::kSpeakerTrc;
        case AudioChannelSet::topRearRight:      return Steinberg::Vst::kSpeakerTrr;
        case AudioChannelSet::LFE2:              return Steinberg::Vst::kSpeakerLfe2;
        case AudioChannelSet::leftSurroundRear:  return Steinberg::Vst::kSpeakerLcs;
        case AudioChannelSet::rightSurroundRear: return Steinberg::Vst::kSpeakerRcs;
        case AudioChannelSet::proximityLeft:     return Steinberg::Vst::kSpeakerPl;
        case AudioChannelSet::proximityRight:    return Steinberg::Vst::kSpeakerPr;
        case AudioChannelSet::ambisonicACN0:     return Steinberg::Vst::kSpeakerACN0;
        case AudioChannelSet::ambisonicACN1:     return Steinberg::Vst::kSpeakerACN1;
        case AudioChannelSet::ambisonicACN2:     return Steinberg::Vst::kSpeakerACN2;
        case AudioChannelSet::ambisonicACN3:     return Steinberg::Vst::kSpeakerACN3;
        case AudioChannelSet::ambisonicACN4:     return Steinberg::Vst::kSpeakerACN4;
        case AudioChannelSet::ambisonicACN5:     return Steinberg::Vst::kSpeakerACN5;
        case AudioChannelSet::ambisonicACN6:     return Steinberg::Vst::kSpeakerACN6;
        case AudioChannelSet::ambisonicACN7:     return Steinberg::Vst::kSpeakerACN7;
        case AudioChannelSet::ambisonicACN8:     return Steinberg::Vst::kSpeakerACN8;
        case AudioChannelSet::ambisonicACN9:     return Steinberg::Vst::kSpeakerACN9;
        case AudioChannelSet::ambisonicACN10:    return Steinberg::Vst::kSpeakerACN10;
        case AudioChannelSet::ambisonicACN11:    return Steinberg::Vst::kSpeakerACN11;
        case AudioChannelSet::ambisonicACN12:    return Steinberg::Vst::kSpeakerACN12;
        case AudioChannelSet::ambisonicACN13:    return Steinberg::Vst::kSpeakerACN13;
        case AudioChannelSet::ambisonicACN14:    return Steinberg::Vst::kSpeakerACN14;
        case AudioChannelSet::ambisonicACN15:    return Steinberg::Vst::kSpeakerACN15;
        case AudioChannelSet::ambisonicACN16:    return Steinberg::Vst::kSpeakerACN16;
        case AudioChannelSet::ambisonicACN17:    return Steinberg::Vst::kSpeakerACN17;
        case AudioChannelSet::ambisonicACN18:    return Steinberg::Vst::kSpeakerACN18;
        case AudioChannelSet::ambisonicACN19:    return Steinberg::Vst::kSpeakerACN19;
        case AudioChannelSet::ambisonicACN20:    return Steinberg::Vst::kSpeakerACN20;
        case AudioChannelSet::ambisonicACN21:    return Steinberg::Vst::kSpeakerACN21;
        case AudioChannelSet::ambisonicACN22:    return Steinberg::Vst::kSpeakerACN22;
        case AudioChannelSet::ambisonicACN23:    return Steinberg::Vst::kSpeakerACN23;
        case AudioChannelSet::ambisonicACN24:    return Steinberg::Vst::kSpeakerACN24;
        case AudioChannelSet::topSideLeft:       return Steinberg::Vst::kSpeakerTsl;
        case AudioChannelSet::topSideRight:      return Steinberg::Vst::kSpeakerTsr;
        case AudioChannelSet::bottomFrontLeft:   return Steinberg::Vst::kSpeakerBfl;
        case AudioChannelSet::bottomFrontCentre: return Steinberg::Vst::kSpeakerBfc;
        case AudioChannelSet::bottomFrontRight:  return Steinberg::Vst::kSpeakerBfr;
        case AudioChannelSet::bottomSideLeft:    return Steinberg::Vst::kSpeakerBsl;
        case AudioChannelSet::bottomSideRight:   return Steinberg::Vst::kSpeakerBsr;
        case AudioChannelSet::bottomRearLeft:    return Steinberg::Vst::kSpeakerBrl;
        case AudioChannelSet::bottomRearCentre:  return Steinberg::Vst::kSpeakerBrc;
        case AudioChannelSet::bottomRearRight:   return Steinberg::Vst::kSpeakerBrr;
        case AudioChannelSet::wideLeft:          return Steinberg::Vst::kSpeakerLw;
        case AudioChannelSet::wideRight:         return Steinberg::Vst::kSpeakerRw;

        case AudioChannelSet::discreteChannel0:  return Steinberg::Vst::kSpeakerM;

        case AudioChannelSet::ambisonicACN25:
        case AudioChannelSet::ambisonicACN26:
        case AudioChannelSet::ambisonicACN27:
        case AudioChannelSet::ambisonicACN28:
        case AudioChannelSet::ambisonicACN29:
        case AudioChannelSet::ambisonicACN30:
        case AudioChannelSet::ambisonicACN31:
        case AudioChannelSet::ambisonicACN32:
        case AudioChannelSet::ambisonicACN33:
        case AudioChannelSet::ambisonicACN34:
        case AudioChannelSet::ambisonicACN35:
        case AudioChannelSet::ambisonicACN36:
        case AudioChannelSet::ambisonicACN37:
        case AudioChannelSet::ambisonicACN38:
        case AudioChannelSet::ambisonicACN39:
        case AudioChannelSet::ambisonicACN40:
        case AudioChannelSet::ambisonicACN41:
        case AudioChannelSet::ambisonicACN42:
        case AudioChannelSet::ambisonicACN43:
        case AudioChannelSet::ambisonicACN44:
        case AudioChannelSet::ambisonicACN45:
        case AudioChannelSet::ambisonicACN46:
        case AudioChannelSet::ambisonicACN47:
        case AudioChannelSet::ambisonicACN48:
        case AudioChannelSet::ambisonicACN49:
        case AudioChannelSet::ambisonicACN50:
        case AudioChannelSet::ambisonicACN51:
        case AudioChannelSet::ambisonicACN52:
        case AudioChannelSet::ambisonicACN53:
        case AudioChannelSet::ambisonicACN54:
        case AudioChannelSet::ambisonicACN55:
        case AudioChannelSet::ambisonicACN56:
        case AudioChannelSet::ambisonicACN57:
        case AudioChannelSet::ambisonicACN58:
        case AudioChannelSet::ambisonicACN59:
        case AudioChannelSet::ambisonicACN60:
        case AudioChannelSet::ambisonicACN61:
        case AudioChannelSet::ambisonicACN62:
        case AudioChannelSet::ambisonicACN63:
        case AudioChannelSet::unknown:
            break;
    }

    return {};
}

static std::optional<AudioChannelSet::ChannelType> getChannelType (Steinberg::Vst::SpeakerArrangement arr, Steinberg::Vst::Speaker type) noexcept
{
    switch (type)
    {
        case Steinberg::Vst::kSpeakerL:     return AudioChannelSet::left;
        case Steinberg::Vst::kSpeakerR:     return AudioChannelSet::right;
        case Steinberg::Vst::kSpeakerC:     return AudioChannelSet::centre;
        case Steinberg::Vst::kSpeakerLfe:   return AudioChannelSet::LFE;
        case Steinberg::Vst::kSpeakerLs:    return AudioChannelSet::leftSurround;
        case Steinberg::Vst::kSpeakerRs:    return AudioChannelSet::rightSurround;
        case Steinberg::Vst::kSpeakerLc:    return AudioChannelSet::leftCentre;
        case Steinberg::Vst::kSpeakerRc:    return AudioChannelSet::rightCentre;
        case Steinberg::Vst::kSpeakerCs:    return AudioChannelSet::centreSurround;
        case Steinberg::Vst::kSpeakerSl:    return AudioChannelSet::leftSurroundSide;
        case Steinberg::Vst::kSpeakerSr:    return AudioChannelSet::rightSurroundSide;
        case Steinberg::Vst::kSpeakerTc:    return AudioChannelSet::topMiddle;  /* kSpeakerTm */
        case Steinberg::Vst::kSpeakerTfl:   return AudioChannelSet::topFrontLeft;
        case Steinberg::Vst::kSpeakerTfc:   return AudioChannelSet::topFrontCentre;
        case Steinberg::Vst::kSpeakerTfr:   return AudioChannelSet::topFrontRight;
        case Steinberg::Vst::kSpeakerTrl:   return AudioChannelSet::topRearLeft;
        case Steinberg::Vst::kSpeakerTrc:   return AudioChannelSet::topRearCentre;
        case Steinberg::Vst::kSpeakerTrr:   return AudioChannelSet::topRearRight;
        case Steinberg::Vst::kSpeakerLfe2:  return AudioChannelSet::LFE2;
        case Steinberg::Vst::kSpeakerM:     return ((arr & Steinberg::Vst::kSpeakerC) != 0 ? AudioChannelSet::discreteChannel0 : AudioChannelSet::centre);
        case Steinberg::Vst::kSpeakerACN0:  return AudioChannelSet::ambisonicACN0;
        case Steinberg::Vst::kSpeakerACN1:  return AudioChannelSet::ambisonicACN1;
        case Steinberg::Vst::kSpeakerACN2:  return AudioChannelSet::ambisonicACN2;
        case Steinberg::Vst::kSpeakerACN3:  return AudioChannelSet::ambisonicACN3;
        case Steinberg::Vst::kSpeakerACN4:  return AudioChannelSet::ambisonicACN4;
        case Steinberg::Vst::kSpeakerACN5:  return AudioChannelSet::ambisonicACN5;
        case Steinberg::Vst::kSpeakerACN6:  return AudioChannelSet::ambisonicACN6;
        case Steinberg::Vst::kSpeakerACN7:  return AudioChannelSet::ambisonicACN7;
        case Steinberg::Vst::kSpeakerACN8:  return AudioChannelSet::ambisonicACN8;
        case Steinberg::Vst::kSpeakerACN9:  return AudioChannelSet::ambisonicACN9;
        case Steinberg::Vst::kSpeakerACN10: return AudioChannelSet::ambisonicACN10;
        case Steinberg::Vst::kSpeakerACN11: return AudioChannelSet::ambisonicACN11;
        case Steinberg::Vst::kSpeakerACN12: return AudioChannelSet::ambisonicACN12;
        case Steinberg::Vst::kSpeakerACN13: return AudioChannelSet::ambisonicACN13;
        case Steinberg::Vst::kSpeakerACN14: return AudioChannelSet::ambisonicACN14;
        case Steinberg::Vst::kSpeakerACN15: return AudioChannelSet::ambisonicACN15;
        case Steinberg::Vst::kSpeakerACN16: return AudioChannelSet::ambisonicACN16;
        case Steinberg::Vst::kSpeakerACN17: return AudioChannelSet::ambisonicACN17;
        case Steinberg::Vst::kSpeakerACN18: return AudioChannelSet::ambisonicACN18;
        case Steinberg::Vst::kSpeakerACN19: return AudioChannelSet::ambisonicACN19;
        case Steinberg::Vst::kSpeakerACN20: return AudioChannelSet::ambisonicACN20;
        case Steinberg::Vst::kSpeakerACN21: return AudioChannelSet::ambisonicACN21;
        case Steinberg::Vst::kSpeakerACN22: return AudioChannelSet::ambisonicACN22;
        case Steinberg::Vst::kSpeakerACN23: return AudioChannelSet::ambisonicACN23;
        case Steinberg::Vst::kSpeakerACN24: return AudioChannelSet::ambisonicACN24;
        case Steinberg::Vst::kSpeakerTsl:   return AudioChannelSet::topSideLeft;
        case Steinberg::Vst::kSpeakerTsr:   return AudioChannelSet::topSideRight;
        case Steinberg::Vst::kSpeakerLcs:   return AudioChannelSet::leftSurroundRear;
        case Steinberg::Vst::kSpeakerRcs:   return AudioChannelSet::rightSurroundRear;
        case Steinberg::Vst::kSpeakerBfl:   return AudioChannelSet::bottomFrontLeft;
        case Steinberg::Vst::kSpeakerBfc:   return AudioChannelSet::bottomFrontCentre;
        case Steinberg::Vst::kSpeakerBfr:   return AudioChannelSet::bottomFrontRight;
        case Steinberg::Vst::kSpeakerPl:    return AudioChannelSet::proximityLeft;
        case Steinberg::Vst::kSpeakerPr:    return AudioChannelSet::proximityRight;
        case Steinberg::Vst::kSpeakerBsl:   return AudioChannelSet::bottomSideLeft;
        case Steinberg::Vst::kSpeakerBsr:   return AudioChannelSet::bottomSideRight;
        case Steinberg::Vst::kSpeakerBrl:   return AudioChannelSet::bottomRearLeft;
        case Steinberg::Vst::kSpeakerBrc:   return AudioChannelSet::bottomRearCentre;
        case Steinberg::Vst::kSpeakerBrr:   return AudioChannelSet::bottomRearRight;
        case Steinberg::Vst::kSpeakerLw:    return AudioChannelSet::wideLeft;
        case Steinberg::Vst::kSpeakerRw:    return AudioChannelSet::wideRight;
    }

    return {};
}

namespace detail
{
    struct LayoutPair
    {
        Steinberg::Vst::SpeakerArrangement arrangement;
        std::initializer_list<AudioChannelSet::ChannelType> channelOrder;
    };

    using namespace Steinberg::Vst::SpeakerArr;
    using X = AudioChannelSet;

    /*  Maps VST3 layouts to the equivalent JUCE channels, in VST3 order.

        The channel types are taken from the equivalent JUCE AudioChannelSet, and then reordered to
        match the VST3 speaker positions.
    */
    const LayoutPair layoutTable[]
    {
        { kEmpty,                       {} },
        { kMono,                        { X::centre } },
        { kStereo,                      { X::left, X::right } },
        { k30Cine,                      { X::left, X::right, X::centre } },
        { k30Music,                     { X::left, X::right, X::surround } },
        { k40Cine,                      { X::left, X::right, X::centre, X::surround } },
        { k50,                          { X::left, X::right, X::centre, X::leftSurround, X::rightSurround } },
        { k51,                          { X::left, X::right, X::centre, X::LFE, X::leftSurround, X::rightSurround } },
        { k60Cine,                      { X::left, X::right, X::centre, X::leftSurround, X::rightSurround, X::centreSurround } },
        { k61Cine,                      { X::left, X::right, X::centre, X::LFE, X::leftSurround, X::rightSurround, X::centreSurround } },
        { k60Music,                     { X::left, X::right, X::leftSurround, X::rightSurround, X::leftSurroundSide, X::rightSurroundSide } },
        { k61Music,                     { X::left, X::right, X::LFE, X::leftSurround, X::rightSurround, X::leftSurroundSide, X::rightSurroundSide } },
        { k70Music,                     { X::left, X::right, X::centre, X::leftSurroundRear, X::rightSurroundRear, X::leftSurroundSide, X::rightSurroundSide } },
        { k70Cine,                      { X::left, X::right, X::centre, X::leftSurround, X::rightSurround, X::leftCentre, X::rightCentre } },
        { k71Music,                     { X::left, X::right, X::centre, X::LFE, X::leftSurroundRear, X::rightSurroundRear, X::leftSurroundSide, X::rightSurroundSide } },
        { k71Cine,                      { X::left, X::right, X::centre, X::LFE, X::leftSurround, X::rightSurround, X::leftCentre, X::rightCentre } },
        { k40Music,                     { X::left, X::right, X::leftSurround, X::rightSurround } },

        { k51_4,                        { X::left, X::right, X::centre, X::LFE, X::leftSurround, X::rightSurround, X::topFrontLeft, X::topFrontRight, X::topRearLeft, X::topRearRight } },
        { k50_4,                        { X::left, X::right, X::centre,         X::leftSurround, X::rightSurround, X::topFrontLeft, X::topFrontRight, X::topRearLeft, X::topRearRight } },
        { k71_2,                        { X::left, X::right, X::centre, X::LFE, X::leftSurroundRear, X::rightSurroundRear, X::leftSurroundSide, X::rightSurroundSide, X::topSideLeft, X::topSideRight } },
        { k70_2,                        { X::left, X::right, X::centre,         X::leftSurroundRear, X::rightSurroundRear, X::leftSurroundSide, X::rightSurroundSide, X::topSideLeft, X::topSideRight } },
        { k71_4,                        { X::left, X::right, X::centre, X::LFE, X::leftSurroundRear, X::rightSurroundRear, X::leftSurroundSide, X::rightSurroundSide, X::topFrontLeft, X::topFrontRight, X::topRearLeft, X::topRearRight } },
        { k70_4,                        { X::left, X::right, X::centre,         X::leftSurroundRear, X::rightSurroundRear, X::leftSurroundSide, X::rightSurroundSide, X::topFrontLeft, X::topFrontRight, X::topRearLeft, X::topRearRight } },
        { k71_6,                        { X::left, X::right, X::centre, X::LFE, X::leftSurroundRear, X::rightSurroundRear, X::leftSurroundSide, X::rightSurroundSide, X::topFrontLeft, X::topFrontRight, X::topRearLeft, X::topRearRight, X::topSideLeft, X::topSideRight } },
        { k70_6,                        { X::left, X::right, X::centre,         X::leftSurroundRear, X::rightSurroundRear, X::leftSurroundSide, X::rightSurroundSide, X::topFrontLeft, X::topFrontRight, X::topRearLeft, X::topRearRight, X::topSideLeft, X::topSideRight } },

        { k90_4_W,                      { X::left, X::right, X::centre,         X::leftSurroundRear, X::rightSurroundRear, X::leftSurroundSide, X::rightSurroundSide, X::topFrontLeft, X::topFrontRight, X::topRearLeft, X::topRearRight, X::wideLeft, X::wideRight } },
        { k91_4_W,                      { X::left, X::right, X::centre, X::LFE, X::leftSurroundRear, X::rightSurroundRear, X::leftSurroundSide, X::rightSurroundSide, X::topFrontLeft, X::topFrontRight, X::topRearLeft, X::topRearRight, X::wideLeft, X::wideRight } },
        { k90_6_W,                      { X::left, X::right, X::centre,         X::leftSurroundRear, X::rightSurroundRear, X::leftSurroundSide, X::rightSurroundSide, X::topFrontLeft, X::topFrontRight, X::topRearLeft, X::topRearRight, X::topSideLeft, X::topSideRight, X::wideLeft, X::wideRight } },
        { k91_6_W,                      { X::left, X::right, X::centre, X::LFE, X::leftSurroundRear, X::rightSurroundRear, X::leftSurroundSide, X::rightSurroundSide, X::topFrontLeft, X::topFrontRight, X::topRearLeft, X::topRearRight, X::topSideLeft, X::topSideRight, X::wideLeft, X::wideRight } },

        { k90_4,                        { X::left, X::right, X::centre,         X::leftSurround, X::rightSurround, X::leftCentre, X::rightCentre, X::leftSurroundSide, X::rightSurroundSide, X::topFrontLeft, X::topFrontRight, X::topRearLeft, X::topRearRight } },
        { k91_4,                        { X::left, X::right, X::centre, X::LFE, X::leftSurround, X::rightSurround, X::leftCentre, X::rightCentre, X::leftSurroundSide, X::rightSurroundSide, X::topFrontLeft, X::topFrontRight, X::topRearLeft, X::topRearRight } },
        { k90_6,                        { X::left, X::right, X::centre,         X::leftSurround, X::rightSurround, X::leftCentre, X::rightCentre, X::leftSurroundSide, X::rightSurroundSide, X::topFrontLeft, X::topFrontRight, X::topRearLeft, X::topRearRight, X::topSideLeft, X::topSideRight } },
        { k91_6,                        { X::left, X::right, X::centre, X::LFE, X::leftSurround, X::rightSurround, X::leftCentre, X::rightCentre, X::leftSurroundSide, X::rightSurroundSide, X::topFrontLeft, X::topFrontRight, X::topRearLeft, X::topRearRight, X::topSideLeft, X::topSideRight } },
    };

   #if JUCE_DEBUG
    static std::once_flag layoutTableCheckedFlag;
   #endif
}

inline bool isLayoutTableValid()
{
    for (const auto& item : detail::layoutTable)
        if ((size_t) countNumberOfBits ((uint64) item.arrangement) != item.channelOrder.size())
            return false;

    std::set<Steinberg::Vst::SpeakerArrangement> arrangements;

    for (const auto& item : detail::layoutTable)
        arrangements.insert (item.arrangement);

    if (arrangements.size() != (size_t) numElementsInArray (detail::layoutTable))
        return false; // There's a duplicate speaker arrangement

    return std::all_of (std::begin (detail::layoutTable), std::end (detail::layoutTable), [] (const auto& item)
    {
        return std::set<AudioChannelSet::ChannelType> (item.channelOrder).size() == item.channelOrder.size();
    });
}

static std::optional<Array<AudioChannelSet::ChannelType>> getSpeakerOrder (Steinberg::Vst::SpeakerArrangement arr)
{
    using namespace Steinberg::Vst;
    using namespace Steinberg::Vst::SpeakerArr;

   #if JUCE_DEBUG
    std::call_once (detail::layoutTableCheckedFlag, [] { jassert (isLayoutTableValid()); });
   #endif

    // Check if this is a layout with a hard-coded conversion
    const auto arrangementMatches = [arr] (const auto& layoutPair) { return layoutPair.arrangement == arr; };
    const auto iter = std::find_if (std::begin (detail::layoutTable), std::end (detail::layoutTable), arrangementMatches);

    if (iter != std::end (detail::layoutTable))
        return iter->channelOrder;

    // There's no hard-coded conversion, so assume that the channels are in the same orders in both layouts.
    const auto channels = getChannelCount (arr);
    Array<AudioChannelSet::ChannelType> result;
    result.ensureStorageAllocated (channels);

    for (auto i = 0; i < channels; ++i)
        if (const auto t = getChannelType (arr, getSpeaker (arr, i)))
            result.add (*t);

    if (getChannelCount (arr) == result.size())
        return result;

    return {};
}

struct Ambisonics
{
    struct Mapping
    {
        Steinberg::Vst::SpeakerArrangement arrangement;
        AudioChannelSet channelSet;
    };

    inline static const Mapping mappings[]
    {
        { Steinberg::Vst::SpeakerArr::kAmbi5thOrderACN, AudioChannelSet::ambisonic (5) },
        { Steinberg::Vst::SpeakerArr::kAmbi6thOrderACN, AudioChannelSet::ambisonic (6) },
        { Steinberg::Vst::SpeakerArr::kAmbi7thOrderACN, AudioChannelSet::ambisonic (7) },
    };

    Ambisonics() = delete;
};

static std::optional<Steinberg::Vst::SpeakerArrangement> getVst3SpeakerArrangement (const AudioChannelSet& channels) noexcept
{
    using namespace Steinberg::Vst::SpeakerArr;

   #if JUCE_DEBUG
    std::call_once (detail::layoutTableCheckedFlag, [] { jassert (isLayoutTableValid()); });
   #endif

    for (const auto& mapping : Ambisonics::mappings)
        if (channels == mapping.channelSet)
            return mapping.arrangement;

    const auto channelSetMatches = [&channels] (const auto& layoutPair)
    {
        return AudioChannelSet::channelSetWithChannels (layoutPair.channelOrder) == channels;
    };
    const auto iter = std::find_if (std::begin (detail::layoutTable), std::end (detail::layoutTable), channelSetMatches);

    if (iter != std::end (detail::layoutTable))
        return iter->arrangement;

    Steinberg::Vst::SpeakerArrangement result = 0;

    for (const auto& type : channels.getChannelTypes())
        if (const auto t = getSpeakerType (channels, type))
            result |= *t;

    if (getChannelCount (result) == channels.size())
        return result;

    return {};
}

inline std::optional<AudioChannelSet> getChannelSetForSpeakerArrangement (Steinberg::Vst::SpeakerArrangement arr) noexcept
{
    using namespace Steinberg::Vst::SpeakerArr;

    for (const auto& mapping : Ambisonics::mappings)
        if (arr == mapping.arrangement)
            return mapping.channelSet;

    if (const auto order = getSpeakerOrder (arr))
        return AudioChannelSet::channelSetWithChannels (*order);

    // VST3 <-> JUCE layout conversion error: report this bug to the JUCE forum
    return {};
}

//==============================================================================
/*
    Provides fast remapping of the channels on a single bus, from VST3 order to JUCE order.

    For multi-bus plugins, you'll need several instances of this, one per bus.
*/
struct ChannelMapping
{
    ChannelMapping (const AudioChannelSet& layout, bool activeIn)
        : indices (makeChannelIndices (layout)), active (activeIn) {}

    explicit ChannelMapping (const AudioChannelSet& layout)
        : ChannelMapping (layout, true) {}

    explicit ChannelMapping (const AudioProcessor::Bus& bus)
        : ChannelMapping (bus.getLastEnabledLayout(), bus.isEnabled()) {}

    int getJuceChannelForVst3Channel (int vst3Channel) const { return indices[(size_t) vst3Channel]; }

    size_t size() const { return indices.size(); }

    void setActive (bool x) { active = x; }
    bool isActive() const { return active; }

private:
    /*  Builds a table that provides the index of the corresponding JUCE channel, given a VST3 channel.

        Depending on the mapping, the VST3 arrangement and JUCE arrangement may not contain channels
        that map 1:1 via getChannelType. For example, the VST3 7.1 layout contains
        'kSpeakerLs' which maps to the 'leftSurround' channel type, but the JUCE 7.1 layout does not
        contain this channel type. As a result, we need to try to map the channels sensibly, even
        if there's not a 'direct' mapping.
    */
    static std::vector<int> makeChannelIndices (const AudioChannelSet& juceArrangement)
    {
        const auto order = [&]
        {
            const auto fallback = juceArrangement.getChannelTypes();
            const auto vst3Arrangement = getVst3SpeakerArrangement (juceArrangement);

            if (! vst3Arrangement.has_value())
                return fallback;

            const auto reordered = getSpeakerOrder (*vst3Arrangement);

            if (! reordered.has_value() || AudioChannelSet::channelSetWithChannels (*reordered) != juceArrangement)
                return fallback;

            return *reordered;
        }();

        std::vector<int> result;

        for (const auto& type : order)
            result.push_back (juceArrangement.getChannelIndexForType (type));

        return result;
    }

    std::vector<int> indices;
    bool active = true;
};

class DynamicChannelMapping
{
public:
    DynamicChannelMapping (const AudioChannelSet& channelSet, bool active)
        : set (channelSet), map (channelSet, active) {}

    explicit DynamicChannelMapping (const AudioChannelSet& channelSet)
        : DynamicChannelMapping (channelSet, true) {}

    explicit DynamicChannelMapping (const AudioProcessor::Bus& bus)
        : DynamicChannelMapping (bus.getLastEnabledLayout(), bus.isEnabled()) {}

    AudioChannelSet getAudioChannelSet() const { return set; }
    int getJuceChannelForVst3Channel (int vst3Channel) const { return map.getJuceChannelForVst3Channel (vst3Channel); }
    size_t size() const { return map.size(); }

    /*  Returns true if the host has activated this bus. */
    bool isHostActive()   const { return hostActive; }
    /*  Returns true if the AudioProcessor expects this bus to be active. */
    bool isClientActive() const { return map.isActive(); }

    void setHostActive   (bool active)   { hostActive   = active; }
    void setClientActive (bool active)   { map.setActive (active); }

private:
    AudioChannelSet set;
    ChannelMapping map;
    bool hostActive = false;
};

//==============================================================================
inline auto& getAudioBusPointer (detail::Tag<float>,  Steinberg::Vst::AudioBusBuffers& data) { return data.channelBuffers32; }
inline auto& getAudioBusPointer (detail::Tag<double>, Steinberg::Vst::AudioBusBuffers& data) { return data.channelBuffers64; }

static inline int countUsedClientChannels (const std::vector<DynamicChannelMapping>& inputMap,
                                           const std::vector<DynamicChannelMapping>& outputMap)
{
    const auto countUsedChannelsInVector = [] (const std::vector<DynamicChannelMapping>& map)
    {
        return std::accumulate (map.begin(), map.end(), 0, [] (auto acc, const auto& item)
        {
            return acc + (item.isClientActive() ? (int) item.size() : 0);
        });
    };

    return jmax (countUsedChannelsInVector (inputMap), countUsedChannelsInVector (outputMap));
}

template <typename FloatType>
class ScratchBuffer
{
public:
    void setSize (int numChannels, int blockSize)
    {
        buffer.setSize (numChannels, blockSize);
    }

    void clear() { channelCounter = 0; }

    auto* getNextChannelBuffer() { return buffer.getWritePointer (channelCounter++); }

    auto getArrayOfWritePointers() { return buffer.getArrayOfWritePointers(); }

private:
    AudioBuffer<FloatType> buffer;
    int channelCounter = 0;
};

template <typename FloatType>
static int countValidBuses (Steinberg::Vst::AudioBusBuffers* buffers, int32 num)
{
    return (int) std::distance (buffers, std::find_if (buffers, buffers + num, [] (auto& buf)
    {
        return getAudioBusPointer (detail::Tag<FloatType>{}, buf) == nullptr && buf.numChannels > 0;
    }));
}

enum class Direction { input, output };

template <Direction direction, typename FloatType, typename Iterator>
static bool validateLayouts (Iterator first, Iterator last, const std::vector<DynamicChannelMapping>& map)
{
    if ((size_t) std::distance (first, last) > map.size())
        return false;

    auto mapIterator = map.begin();

    for (auto it = first; it != last; ++it, ++mapIterator)
    {
        auto& bus = *it;
        auto** busPtr = getAudioBusPointer (detail::Tag<FloatType>{}, bus);
        const auto expectedJuceChannels = (int) mapIterator->size();
        const auto actualVstChannels = (int) bus.numChannels;
        const auto limit = jmin (expectedJuceChannels, actualVstChannels);
        const auto anyChannelIsNull = std::any_of (busPtr, busPtr + limit, [] (auto* ptr) { return ptr == nullptr; });
        constexpr auto isInput = direction == Direction::input;

        const auto channelCountIsUsable = isInput ? expectedJuceChannels <= actualVstChannels
                                                  : actualVstChannels <= expectedJuceChannels;

        // Null channels are allowed if the bus is inactive
        if (mapIterator->isHostActive() && (anyChannelIsNull || ! channelCountIsUsable))
            return false;

        // If this is hit, the destination bus has fewer channels than the source bus.
        // As a result, some channels will 'go missing', and channel layouts may be invalid.
        jassert (actualVstChannels == expectedJuceChannels);
    }

    // If the host didn't provide the full complement of buses, it must be because the other
    // buses are all deactivated.
    return std::none_of (mapIterator, map.end(), [] (const auto& item) { return item.isHostActive(); });
}

/*
    The main purpose of this class is to remap a set of buffers provided by the VST3 host into an
    equivalent JUCE AudioBuffer using the JUCE channel layout/order.

    An instance of this class handles input and output remapping for a single data type (float or
    double), matching the FloatType template parameter.

    This is in VST3Common.h, rather than in the VST3_Wrapper.cpp, so that we can test it.

    @see ClientBufferMapper
*/
template <typename FloatType>
class ClientBufferMapperData
{
public:
    void prepare (int numChannels, int blockSize)
    {
        scratchBuffer.setSize (numChannels, blockSize);
        channels.reserve ((size_t) jmin (128, numChannels));
    }

    AudioBuffer<FloatType> getMappedBuffer (Steinberg::Vst::ProcessData& data,
                                            const std::vector<DynamicChannelMapping>& inputMap,
                                            const std::vector<DynamicChannelMapping>& outputMap)
    {
        scratchBuffer.clear();
        channels.clear();

        const auto usedChannels = countUsedClientChannels (inputMap, outputMap);

        // WaveLab workaround: This host may report the wrong number of inputs/outputs so re-count here
        const auto vstInputs = countValidBuses<FloatType> (data.inputs, data.numInputs);

        if (! validateLayouts<Direction::input, FloatType> (data.inputs, data.inputs + vstInputs, inputMap))
            return getBlankBuffer (usedChannels, (int) data.numSamples);

        setUpInputChannels (data, (size_t) vstInputs, scratchBuffer, inputMap, channels);
        setUpOutputChannels (scratchBuffer, outputMap, channels);

        const auto channelPtr = channels.empty() ? scratchBuffer.getArrayOfWritePointers()
                                                 : channels.data();

        return { channelPtr, (int) channels.size(), (int) data.numSamples };
    }

private:
    static void setUpInputChannels (Steinberg::Vst::ProcessData& data,
                                    size_t vstInputs,
                                    ScratchBuffer<FloatType>& scratchBuffer,
                                    const std::vector<DynamicChannelMapping>& map,
                                    std::vector<FloatType*>& channels)
    {
        for (size_t busIndex = 0; busIndex < map.size(); ++busIndex)
        {
            const auto& mapping = map[busIndex];

            if (! mapping.isClientActive())
                continue;

            const auto originalSize = channels.size();

            for (size_t channelIndex = 0; channelIndex < mapping.size(); ++channelIndex)
                channels.push_back (scratchBuffer.getNextChannelBuffer());

            if (mapping.isHostActive() && busIndex < vstInputs)
            {
                auto& bus = data.inputs[busIndex];

                // Every JUCE channel must have a VST3 channel counterpart
                jassert (mapping.size() <= static_cast<size_t> (bus.numChannels));

                auto** busPtr = getAudioBusPointer (detail::Tag<FloatType>{}, bus);

                for (size_t channelIndex = 0; channelIndex < mapping.size(); ++channelIndex)
                {
                    FloatVectorOperations::copy (channels[(size_t) mapping.getJuceChannelForVst3Channel ((int) channelIndex) + originalSize],
                                                 busPtr[channelIndex],
                                                 (size_t) data.numSamples);
                }
            }
            else
            {
                for (size_t channelIndex = 0; channelIndex < mapping.size(); ++channelIndex)
                    FloatVectorOperations::clear (channels[originalSize + channelIndex], (size_t) data.numSamples);
            }
        }
    }

    static void setUpOutputChannels (ScratchBuffer<FloatType>& scratchBuffer,
                                     const std::vector<DynamicChannelMapping>& map,
                                     std::vector<FloatType*>& channels)
    {
        for (size_t i = 0, initialBusIndex = 0; i < (size_t) map.size(); ++i)
        {
            const auto& mapping = map[i];

            if (mapping.isClientActive())
            {
                for (size_t j = 0; j < mapping.size(); ++j)
                {
                    if (channels.size() <= initialBusIndex + j)
                        channels.push_back (scratchBuffer.getNextChannelBuffer());
                }

                initialBusIndex += mapping.size();
            }
        }
    }

    AudioBuffer<FloatType> getBlankBuffer (int usedChannels, int usedSamples)
    {
        // The host is ignoring the bus layout we requested, so we can't process sensibly!
        jassertfalse;

        // Return a silent buffer for the AudioProcessor to process
        for (auto i = 0; i < usedChannels; ++i)
        {
            channels.push_back (scratchBuffer.getNextChannelBuffer());
            FloatVectorOperations::clear (channels.back(), usedSamples);
        }

        return { channels.data(), (int) channels.size(), usedSamples };
    }

    std::vector<FloatType*> channels;
    ScratchBuffer<FloatType> scratchBuffer;
};

//==============================================================================
/*
    Remaps a set of buffers provided by the VST3 host into an equivalent JUCE AudioBuffer using the
    JUCE channel layout/order.

    An instance of this class can remap to either a float or double JUCE buffer, as necessary.

    Although the VST3 spec requires that the bus layout does not change while the plugin is
    activated and processing, some hosts get this wrong and try to enable/disable buses during
    playback. This class attempts to be resilient, and should cope with buses being switched on and
    off during processing.

    This is in VST3Common.h, rather than in the VST3_Wrapper.cpp, so that we can test it.

    @see ClientBufferMapper
*/
class ClientBufferMapper
{
public:
    void updateFromProcessor (const AudioProcessor& processor)
    {
        struct Pair
        {
            std::vector<DynamicChannelMapping>& map;
            bool isInput;
        };

        for (const auto& pair : { Pair { inputMap, true }, Pair { outputMap, false } })
        {
            if (pair.map.empty())
            {
                for (auto i = 0; i < processor.getBusCount (pair.isInput); ++i)
                    pair.map.emplace_back (*processor.getBus (pair.isInput, i));
            }
            else
            {
                // The number of buses cannot change after creating a VST3 plugin!
                jassert ((size_t) processor.getBusCount (pair.isInput) == pair.map.size());

                for (size_t i = 0; i < (size_t) processor.getBusCount (pair.isInput); ++i)
                {
                    pair.map[i] = [&]
                    {
                        DynamicChannelMapping replacement { *processor.getBus (pair.isInput, (int) i) };
                        replacement.setHostActive (pair.map[i].isHostActive());
                        return replacement;
                    }();
                }
            }
        }
    }

    void prepare (int blockSize)
    {
        const auto findNumChannelsWhenAllBusesEnabled = [] (const auto& map)
        {
            return std::accumulate (map.cbegin(), map.cend(), 0, [] (auto acc, const auto& item)
            {
                return acc + (int) item.size();
            });
        };

        const auto numChannels = jmax (findNumChannelsWhenAllBusesEnabled (inputMap),
                                       findNumChannelsWhenAllBusesEnabled (outputMap));

        floatData .prepare (numChannels, blockSize);
        doubleData.prepare (numChannels, blockSize);
    }

    void updateActiveClientBuses (const AudioProcessor::BusesLayout& clientBuses)
    {
        if (   (size_t) clientBuses.inputBuses .size() != inputMap .size()
            || (size_t) clientBuses.outputBuses.size() != outputMap.size())
        {
            jassertfalse;
            return;
        }

        const auto sync = [] (auto& map, auto& client)
        {
            for (size_t i = 0; i < map.size(); ++i)
            {
                jassert (client[(int) i] == AudioChannelSet::disabled() || client[(int) i] == map[i].getAudioChannelSet());
                map[i].setClientActive (client[(int) i] != AudioChannelSet::disabled());
            }
        };

        sync (inputMap,  clientBuses.inputBuses);
        sync (outputMap, clientBuses.outputBuses);
    }

    void setInputBusHostActive  (size_t bus, bool state) { setHostActive (inputMap,  bus, state); }
    void setOutputBusHostActive (size_t bus, bool state) { setHostActive (outputMap, bus, state); }

    auto& getData (detail::Tag<float>)  { return floatData; }
    auto& getData (detail::Tag<double>) { return doubleData; }

    AudioChannelSet getRequestedLayoutForInputBus (size_t bus) const
    {
        return getRequestedLayoutForBus (inputMap, bus);
    }

    AudioChannelSet getRequestedLayoutForOutputBus (size_t bus) const
    {
        return getRequestedLayoutForBus (outputMap, bus);
    }

    const std::vector<DynamicChannelMapping>& getInputMap()  const { return inputMap; }
    const std::vector<DynamicChannelMapping>& getOutputMap() const { return outputMap; }

private:
    static void setHostActive (std::vector<DynamicChannelMapping>& map, size_t bus, bool state)
    {
        if (bus < map.size())
            map[bus].setHostActive (state);
    }

    static AudioChannelSet getRequestedLayoutForBus (const std::vector<DynamicChannelMapping>& map, size_t bus)
    {
        if (bus < map.size() && map[bus].isHostActive())
            return map[bus].getAudioChannelSet();

        return AudioChannelSet::disabled();
    }

    ClientBufferMapperData<float> floatData;
    ClientBufferMapperData<double> doubleData;

    std::vector<DynamicChannelMapping> inputMap;
    std::vector<DynamicChannelMapping> outputMap;
};

//==============================================================================
/*  Holds a buffer in the JUCE channel layout, and a reference to a Vst ProcessData struct, and
    copies each JUCE channel to the appropriate host output channel when this object goes
    out of scope.
*/
template <typename FloatType>
class ClientRemappedBuffer
{
public:
    ClientRemappedBuffer (ClientBufferMapperData<FloatType>& mapperData,
                          const std::vector<DynamicChannelMapping>* inputMapIn,
                          const std::vector<DynamicChannelMapping>* outputMapIn,
                          Steinberg::Vst::ProcessData& hostData)
        : buffer (mapperData.getMappedBuffer (hostData, *inputMapIn, *outputMapIn)),
          outputMap (outputMapIn),
          data (hostData)
    {}

    ClientRemappedBuffer (ClientBufferMapper& mapperIn, Steinberg::Vst::ProcessData& hostData)
        : ClientRemappedBuffer (mapperIn.getData (detail::Tag<FloatType>{}),
                                &mapperIn.getInputMap(),
                                &mapperIn.getOutputMap(),
                                hostData)
    {}

    ~ClientRemappedBuffer()
    {
        // WaveLab workaround: This host may report the wrong number of inputs/outputs so re-count here
        const auto vstOutputs = (size_t) countValidBuses<FloatType> (data.outputs, data.numOutputs);

        if (validateLayouts<Direction::output, FloatType> (data.outputs, data.outputs + vstOutputs, *outputMap))
            copyToHostOutputBuses (vstOutputs);
        else
            clearHostOutputBuses (vstOutputs);
    }

    AudioBuffer<FloatType> buffer;

private:
    void copyToHostOutputBuses (size_t vstOutputs) const
    {
        for (size_t i = 0, juceBusOffset = 0; i < outputMap->size(); ++i)
        {
            const auto& mapping = (*outputMap)[i];

            if (mapping.isHostActive() && i < vstOutputs)
            {
                auto& bus = data.outputs[i];

                // Every VST3 channel must have a JUCE channel counterpart
                jassert (static_cast<size_t> (bus.numChannels) <= mapping.size());

                if (mapping.isClientActive())
                {
                    for (size_t j = 0; j < static_cast<size_t> (bus.numChannels); ++j)
                    {
                        auto* hostChannel = getAudioBusPointer (detail::Tag<FloatType>{}, bus)[j];
                        const auto juceChannel = juceBusOffset + (size_t) mapping.getJuceChannelForVst3Channel ((int) j);
                        FloatVectorOperations::copy (hostChannel, buffer.getReadPointer ((int) juceChannel), (size_t) data.numSamples);
                    }
                }
                else
                {
                    for (size_t j = 0; j < static_cast<size_t> (bus.numChannels); ++j)
                    {
                        auto* hostChannel = getAudioBusPointer (detail::Tag<FloatType>{}, bus)[j];
                        FloatVectorOperations::clear (hostChannel, (size_t) data.numSamples);
                    }
                }
            }

            if (mapping.isClientActive())
                juceBusOffset += mapping.size();
        }
    }

    void clearHostOutputBuses (size_t vstOutputs) const
    {
        // The host provided us with an unexpected bus layout.
        jassertfalse;

        std::for_each (data.outputs, data.outputs + vstOutputs, [this] (auto& bus)
        {
            auto** busPtr = getAudioBusPointer (detail::Tag<FloatType>{}, bus);
            std::for_each (busPtr, busPtr + bus.numChannels, [this] (auto* ptr)
            {
                if (ptr != nullptr)
                    FloatVectorOperations::clear (ptr, (int) data.numSamples);
            });
        });
    }

    const std::vector<DynamicChannelMapping>* outputMap = nullptr;
    Steinberg::Vst::ProcessData& data;

    JUCE_DECLARE_NON_COPYABLE (ClientRemappedBuffer)
    JUCE_DECLARE_NON_MOVEABLE (ClientRemappedBuffer)
};

//==============================================================================
/*
    Remaps a JUCE buffer to an equivalent VST3 layout.

    An instance of this class handles mappings for both float and double buffers, but in a single
    direction (input or output).
*/
class HostBufferMapper
{
public:
    /*  Builds a cached map of juce <-> vst3 channel mappings. */
    void prepare (std::vector<ChannelMapping> arrangements)
    {
        mappings = std::move (arrangements);

        floatBusMap .resize (mappings.size());
        doubleBusMap.resize (mappings.size());
        busBuffers  .resize (mappings.size());
    }

    /*  Applies the mapping to an AudioBuffer using JUCE channel layout. */
    template <typename FloatType>
    Steinberg::Vst::AudioBusBuffers* getVst3LayoutForJuceBuffer (AudioBuffer<FloatType>& source)
    {
        int channelIndexOffset = 0;

        for (size_t i = 0; i < mappings.size(); ++i)
        {
            const auto& mapping = mappings[i];
            associateBufferTo (busBuffers[i], get (detail::Tag<FloatType>{})[i], source, mapping, channelIndexOffset);
            channelIndexOffset += mapping.isActive() ? (int) mapping.size() : 0;
        }

        return busBuffers.data();
    }

private:
    template <typename FloatType>
    using Bus = std::vector<FloatType*>;

    template <typename FloatType>
    using BusMap = std::vector<Bus<FloatType>>;

    static void assignRawPointer (Steinberg::Vst::AudioBusBuffers& vstBuffers, float** raw)  { vstBuffers.channelBuffers32 = raw; }
    static void assignRawPointer (Steinberg::Vst::AudioBusBuffers& vstBuffers, double** raw) { vstBuffers.channelBuffers64 = raw; }

    template <typename FloatType>
    void associateBufferTo (Steinberg::Vst::AudioBusBuffers& vstBuffers,
                            Bus<FloatType>& bus,
                            AudioBuffer<FloatType>& buffer,
                            const ChannelMapping& busMap,
                            int channelStartOffset) const
    {
        bus.clear();

        for (size_t i = 0; i < busMap.size(); ++i)
        {
            bus.push_back (busMap.isActive() ? buffer.getWritePointer (channelStartOffset + busMap.getJuceChannelForVst3Channel ((int) i))
                                             : nullptr);
        }

        assignRawPointer (vstBuffers, bus.data());
        vstBuffers.numChannels      = (Steinberg::int32) busMap.size();
        vstBuffers.silenceFlags     = busMap.isActive() ? 0 : std::numeric_limits<Steinberg::uint64>::max();
    }

    auto& get (detail::Tag<float>)    { return floatBusMap; }
    auto& get (detail::Tag<double>)   { return doubleBusMap; }

    BusMap<float>  floatBusMap;
    BusMap<double> doubleBusMap;

    std::vector<Steinberg::Vst::AudioBusBuffers> busBuffers;
    std::vector<ChannelMapping> mappings;
};

//==============================================================================
// We have to trust that Steinberg won't double-delete
// NOLINTBEGIN(clang-analyzer-cplusplus.NewDelete)
template <class ObjectType>
class VSTComSmartPtr
{
public:
    VSTComSmartPtr() = default;
    VSTComSmartPtr (const VSTComSmartPtr& other) noexcept : source (other.source)            { if (source != nullptr) source->addRef(); }
    ~VSTComSmartPtr()                                                                        { if (source != nullptr) source->release(); }

    explicit operator bool() const noexcept           { return operator!= (nullptr); }
    ObjectType* get() const noexcept                  { return source; }
    ObjectType& operator*() const noexcept            { return *source; }
    ObjectType* operator->() const noexcept           { return source; }

    VSTComSmartPtr& operator= (const VSTComSmartPtr& other)
    {
        auto p = other;
        std::swap (p.source, source);
        return *this;
    }

    VSTComSmartPtr& operator= (std::nullptr_t)
    {
        return operator= (VSTComSmartPtr{});
    }

    bool operator== (std::nullptr_t) const noexcept { return source == nullptr; }
    bool operator!= (std::nullptr_t) const noexcept { return source != nullptr; }

    bool loadFrom (Steinberg::FUnknown* o)
    {
        *this = nullptr;
        return o != nullptr && o->queryInterface (ObjectType::iid, (void**) &source) == Steinberg::kResultOk;
    }

    bool loadFrom (Steinberg::IPluginFactory* factory, const Steinberg::TUID& uuid)
    {
        jassert (factory != nullptr);
        *this = nullptr;
        return factory->createInstance (uuid, ObjectType::iid, (void**) &source) == Steinberg::kResultOk;
    }

    /** Increments refcount. */
    static auto addOwner (ObjectType* t)
    {
        return VSTComSmartPtr (t, true);
    }

    /** Does not initially increment refcount; assumes t has a positive refcount. */
    static auto becomeOwner (ObjectType* t)
    {
        return VSTComSmartPtr (t, false);
    }

private:
    explicit VSTComSmartPtr (ObjectType* object, bool autoAddRef) noexcept  : source (object)  { if (source != nullptr && autoAddRef) source->addRef(); }
    ObjectType* source = nullptr;
};

/** Increments refcount. */
template <class ObjectType>
auto addVSTComSmartPtrOwner (ObjectType* t)
{
    return VSTComSmartPtr<ObjectType>::addOwner (t);
}

/** Does not initially increment refcount; assumes t has a positive refcount. */
template <class ObjectType>
auto becomeVSTComSmartPtrOwner (ObjectType* t)
{
    return VSTComSmartPtr<ObjectType>::becomeOwner (t);
}

// NOLINTEND(clang-analyzer-cplusplus.NewDelete)

//==============================================================================
/*  This class stores a plugin's preferred MIDI mappings.

    The IMidiMapping is normally an extension of the IEditController which
    should only be accessed from the UI thread. If we're being strict about
    things, then we shouldn't call IMidiMapping functions from the audio thread.

    This code is very similar to that found in the audioclient demo code in the
    VST3 SDK repo.
*/
class StoredMidiMapping
{
public:
    StoredMidiMapping()
    {
        for (auto& channel : channels)
            channel.resize (Steinberg::Vst::kCountCtrlNumber);
    }

    void storeMappings (Steinberg::Vst::IMidiMapping& mapping)
    {
        for (size_t channelIndex = 0; channelIndex < channels.size(); ++channelIndex)
            storeControllers (mapping, channels[channelIndex], channelIndex);
    }

    /* Returns kNoParamId if there is no mapping for this controller. */
    Steinberg::Vst::ParamID getMapping (Steinberg::int16 channel,
                                        Steinberg::Vst::CtrlNumber controller) const noexcept
    {
        return channels[(size_t) channel][(size_t) controller];
    }

private:
    // Maps controller numbers to ParamIDs
    using Controllers = std::vector<Steinberg::Vst::ParamID>;

    // Each channel may have a different CC mapping
    using Channels = std::array<Controllers, 16>;

    static void storeControllers (Steinberg::Vst::IMidiMapping& mapping, Controllers& channel, size_t channelIndex)
    {
        for (size_t controllerIndex = 0; controllerIndex < channel.size(); ++controllerIndex)
            channel[controllerIndex] = getSingleMapping (mapping, channelIndex, controllerIndex);
    }

    static Steinberg::Vst::ParamID getSingleMapping (Steinberg::Vst::IMidiMapping& mapping,
                                                     size_t channelIndex,
                                                     size_t controllerIndex)
    {
        Steinberg::Vst::ParamID result{};
        const auto returnCode = mapping.getMidiControllerAssignment (0,
                                                                     (int16) channelIndex,
                                                                     (Steinberg::Vst::CtrlNumber) controllerIndex,
                                                                     result);

        return returnCode == Steinberg::kResultTrue ? result : Steinberg::Vst::kNoParamId;
    }

    Channels channels;
};

//==============================================================================
class MidiEventList  : public Steinberg::Vst::IEventList
{
public:
    MidiEventList() = default;
    virtual ~MidiEventList() = default;

    JUCE_DECLARE_VST3_COM_REF_METHODS
    JUCE_DECLARE_VST3_COM_QUERY_METHODS

    //==============================================================================
    void clear()
    {
        events.clearQuick();
    }

    Steinberg::int32 PLUGIN_API getEventCount() override
    {
        return (Steinberg::int32) events.size();
    }

    // NB: This has to cope with out-of-range indexes from some plugins.
    Steinberg::tresult PLUGIN_API getEvent (Steinberg::int32 index, Steinberg::Vst::Event& e) override
    {
        if (isPositiveAndBelow ((int) index, events.size()))
        {
            e = events.getReference ((int) index);
            return Steinberg::kResultTrue;
        }

        return Steinberg::kResultFalse;
    }

    Steinberg::tresult PLUGIN_API addEvent (Steinberg::Vst::Event& e) override
    {
        events.add (e);
        return Steinberg::kResultTrue;
    }

    //==============================================================================
    static void toMidiBuffer (MidiBuffer& result, Steinberg::Vst::IEventList& eventList)
    {
        const auto numEvents = eventList.getEventCount();

        for (Steinberg::int32 i = 0; i < numEvents; ++i)
        {
            Steinberg::Vst::Event e;

            if (eventList.getEvent (i, e) != Steinberg::kResultOk)
                continue;

            if (const auto message = toMidiMessage (e))
                result.addEvent (*message, e.sampleOffset);
        }
    }

    template <typename Callback>
    static void hostToPluginEventList (Steinberg::Vst::IEventList& result,
                                       MidiBuffer& midiBuffer,
                                       StoredMidiMapping& mapping,
                                       Callback&& callback)
    {
        toEventList (result, midiBuffer, &mapping, callback);
    }

    static void pluginToHostEventList (Steinberg::Vst::IEventList& result, MidiBuffer& midiBuffer)
    {
        toEventList (result, midiBuffer, nullptr, [] (auto&&...) {});
    }

private:
    enum class EventConversionKind
    {
        // Hosted plugins don't expect to receive LegacyMIDICCEvents messages from the host,
        // so if we're converting midi from the host to an eventlist, this mode will avoid
        // converting to Legacy events where possible.
        hostToPlugin,

        // If plugins generate MIDI internally, then where possible we should preserve
        // these messages as LegacyMIDICCOut events.
        pluginToHost
    };

    template <typename Callback>
    static bool sendMappedParameter (const MidiMessage& msg,
                                     StoredMidiMapping* midiMapping,
                                     Callback&& callback)
    {
        if (midiMapping == nullptr)
            return false;

        const auto controlEvent = toVst3ControlEvent (msg);

        if (! controlEvent.has_value())
            return false;

        const auto controlParamID = midiMapping->getMapping (createSafeChannel (msg.getChannel()),
                                                             controlEvent->controllerNumber);

        if (controlParamID != Steinberg::Vst::kNoParamId)
            callback (controlParamID, controlEvent->paramValue);

        return true;
    }

    template <typename Callback>
    static void processMidiMessage (Steinberg::Vst::IEventList& result,
                                    const MidiMessageMetadata metadata,
                                    StoredMidiMapping* midiMapping,
                                    Callback&& callback)
    {
        const auto msg = metadata.getMessage();

        if (sendMappedParameter (msg, midiMapping, std::forward<Callback> (callback)))
            return;

        const auto kind = midiMapping != nullptr ? EventConversionKind::hostToPlugin
                                                 : EventConversionKind::pluginToHost;

        auto maybeEvent = createVstEvent (msg, metadata.data, kind);

        if (! maybeEvent.hasValue())
            return;

        maybeEvent->busIndex = 0;
        maybeEvent->sampleOffset = metadata.samplePosition;
        result.addEvent (*maybeEvent);
    }

    /*  If mapping is non-null, the conversion is assumed to be host-to-plugin, or otherwise
        plugin-to-host.
    */
    template <typename Callback>
    static void toEventList (Steinberg::Vst::IEventList& result,
                             MidiBuffer& midiBuffer,
                             StoredMidiMapping* midiMapping,
                             Callback&& callback)
    {
        enum { maxNumEvents = 2048 }; // Steinberg's Host Checker states that no more than 2048 events are allowed at once
        int numEvents = 0;

        for (const auto metadata : midiBuffer)
        {
            if (++numEvents > maxNumEvents)
                break;

            processMidiMessage (result, metadata, midiMapping, std::forward<Callback> (callback));
        }
    }

    Array<Steinberg::Vst::Event, CriticalSection> events;
    Atomic<int> refCount;

    static Steinberg::int16 createSafeChannel (int channel) noexcept  { return (Steinberg::int16) jlimit (0, 15, channel - 1); }
    static int createSafeChannel (Steinberg::int16 channel) noexcept  { return (int) jlimit (1, 16, channel + 1); }

    static Steinberg::int16 createSafeNote (int note) noexcept        { return (Steinberg::int16) jlimit (0, 127, note); }
    static int createSafeNote (Steinberg::int16 note) noexcept        { return jlimit (0, 127, (int) note); }

    static float normaliseMidiValue (int value) noexcept              { return jlimit (0.0f, 1.0f, (float) value / 127.0f); }
    static int denormaliseToMidiValue (float value) noexcept          { return roundToInt (jlimit (0.0f, 127.0f, value * 127.0f)); }

    static Steinberg::Vst::Event createNoteOnEvent (const MidiMessage& msg) noexcept
    {
        Steinberg::Vst::Event e{};
        e.type              = Steinberg::Vst::Event::kNoteOnEvent;
        e.noteOn.channel    = createSafeChannel (msg.getChannel());
        e.noteOn.pitch      = createSafeNote (msg.getNoteNumber());
        e.noteOn.velocity   = normaliseMidiValue (msg.getVelocity());
        e.noteOn.length     = 0;
        e.noteOn.tuning     = 0.0f;
        e.noteOn.noteId     = -1;
        return e;
    }

    static Steinberg::Vst::Event createNoteOffEvent (const MidiMessage& msg) noexcept
    {
        Steinberg::Vst::Event e{};
        e.type              = Steinberg::Vst::Event::kNoteOffEvent;
        e.noteOff.channel   = createSafeChannel (msg.getChannel());
        e.noteOff.pitch     = createSafeNote (msg.getNoteNumber());
        e.noteOff.velocity  = normaliseMidiValue (msg.getVelocity());
        e.noteOff.tuning    = 0.0f;
        e.noteOff.noteId    = -1;
        return e;
    }

    static Steinberg::Vst::Event createSysExEvent (const MidiMessage& msg, const uint8* data) noexcept
    {
        jassert (msg.isSysEx());

        Steinberg::Vst::Event e{};
        e.type          = Steinberg::Vst::Event::kDataEvent;
        e.data.bytes    = data;
        e.data.size     = (uint32) msg.getRawDataSize();
        e.data.type     = Steinberg::Vst::DataEvent::kMidiSysEx;
        return e;
    }

    static Steinberg::Vst::Event createLegacyMIDIEvent (int channel, int controlNumber, int value, int value2 = 0)
    {
        Steinberg::Vst::Event e{};
        e.type                      = Steinberg::Vst::Event::kLegacyMIDICCOutEvent;
        e.midiCCOut.channel         = Steinberg::int8 (createSafeChannel (channel));
        e.midiCCOut.controlNumber   = uint8 (jlimit (0, 255, controlNumber));
        e.midiCCOut.value           = Steinberg::int8 (createSafeNote (value));
        e.midiCCOut.value2          = Steinberg::int8 (createSafeNote (value2));
        return e;
    }

    static Steinberg::Vst::Event createPolyPressureEvent (const MidiMessage& msg)
    {
        Steinberg::Vst::Event e{};
        e.type                      = Steinberg::Vst::Event::kPolyPressureEvent;
        e.polyPressure.channel      = createSafeChannel (msg.getChannel());
        e.polyPressure.pitch        = createSafeNote (msg.getNoteNumber());
        e.polyPressure.pressure     = normaliseMidiValue (msg.getAfterTouchValue());
        e.polyPressure.noteId       = -1;
        return e;
    }

    static Steinberg::Vst::Event createChannelPressureEvent (const MidiMessage& msg) noexcept
    {
        return createLegacyMIDIEvent (msg.getChannel(),
                                      Steinberg::Vst::kAfterTouch,
                                      msg.getChannelPressureValue());
    }

    static Steinberg::Vst::Event createControllerEvent (const MidiMessage& msg) noexcept
    {
        return createLegacyMIDIEvent (msg.getChannel(),
                                      msg.getControllerNumber(),
                                      msg.getControllerValue());
    }

    static Steinberg::Vst::Event createCtrlPolyPressureEvent (const MidiMessage& msg) noexcept
    {
        return createLegacyMIDIEvent (msg.getChannel(),
                                      Steinberg::Vst::kCtrlPolyPressure,
                                      msg.getNoteNumber(),
                                      msg.getAfterTouchValue());
    }

    static Steinberg::Vst::Event createPitchWheelEvent (const MidiMessage& msg) noexcept
    {
        return createLegacyMIDIEvent (msg.getChannel(),
                                      Steinberg::Vst::kPitchBend,
                                      msg.getRawData()[1],
                                      msg.getRawData()[2]);
    }

    static Steinberg::Vst::Event createProgramChangeEvent (const MidiMessage& msg) noexcept
    {
        return createLegacyMIDIEvent (msg.getChannel(),
                                      Steinberg::Vst::kCtrlProgramChange,
                                      msg.getProgramChangeNumber());
    }

    static Steinberg::Vst::Event createCtrlQuarterFrameEvent (const MidiMessage& msg) noexcept
    {
        return createLegacyMIDIEvent (msg.getChannel(),
                                      Steinberg::Vst::kCtrlQuarterFrame,
                                      msg.getQuarterFrameValue());
    }

    static Optional<Steinberg::Vst::Event> createVstEvent (const MidiMessage& msg,
                                                           const uint8* midiEventData,
                                                           EventConversionKind kind) noexcept
    {
        if (msg.isNoteOn())
            return createNoteOnEvent (msg);

        if (msg.isNoteOff())
            return createNoteOffEvent (msg);

        if (msg.isSysEx())
            return createSysExEvent (msg, midiEventData);

        if (msg.isChannelPressure())
            return createChannelPressureEvent (msg);

        if (msg.isPitchWheel())
            return createPitchWheelEvent (msg);

        if (msg.isProgramChange())
            return createProgramChangeEvent (msg);

        if (msg.isController())
            return createControllerEvent (msg);

        if (msg.isQuarterFrame())
            return createCtrlQuarterFrameEvent (msg);

        if (msg.isAftertouch())
        {
            switch (kind)
            {
                case EventConversionKind::hostToPlugin:
                    return createPolyPressureEvent (msg);

                case EventConversionKind::pluginToHost:
                    return createCtrlPolyPressureEvent (msg);
            }

            jassertfalse;
            return {};
        }

        return {};
    }

    static Optional<MidiMessage> toMidiMessage (const Steinberg::Vst::LegacyMIDICCOutEvent& e)
    {
        if (e.controlNumber <= 127)
            return MidiMessage::controllerEvent (createSafeChannel (int16 (e.channel)),
                                                 createSafeNote (int16 (e.controlNumber)),
                                                 createSafeNote (int16 (e.value)));

        switch (e.controlNumber)
        {
            case Steinberg::Vst::kAfterTouch:
                return MidiMessage::channelPressureChange (createSafeChannel (int16 (e.channel)),
                                                           createSafeNote (int16 (e.value)));

            case Steinberg::Vst::kPitchBend:
                return MidiMessage::pitchWheel (createSafeChannel (int16 (e.channel)),
                                                (e.value & 0x7f) | ((e.value2 & 0x7f) << 7));

            case Steinberg::Vst::kCtrlProgramChange:
                return MidiMessage::programChange (createSafeChannel (int16 (e.channel)),
                                                   createSafeNote (int16 (e.value)));

            case Steinberg::Vst::kCtrlQuarterFrame:
                return MidiMessage::quarterFrame (createSafeChannel (int16 (e.channel)),
                                                  createSafeNote (int16 (e.value)));

            case Steinberg::Vst::kCtrlPolyPressure:
                return MidiMessage::aftertouchChange (createSafeChannel (int16 (e.channel)),
                                                      createSafeNote (int16 (e.value)),
                                                      createSafeNote (int16 (e.value2)));

            default:
                // If this is hit, we're trying to convert a LegacyMIDICCOutEvent with an unknown controlNumber.
                jassertfalse;
                return {};
        }
    }

    static Optional<MidiMessage> toMidiMessage (const Steinberg::Vst::DataEvent& e)
    {
        if (e.type != Steinberg::Vst::DataEvent::kMidiSysEx || e.size < 2)
        {
            // Only sysex data messages can be converted to MIDI
            jassertfalse;
            return {};
        }

        const auto header = e.bytes[0];
        const auto footer = e.bytes[e.size - 1];

        if (header != 0xf0 || footer != 0xf7)
        {
            // The sysex header/footer bytes are missing
            jassertfalse;
            return {};
        }

        return MidiMessage::createSysExMessage (e.bytes + 1, (int) e.size - 2);
    }

    static Optional<MidiMessage> toMidiMessage (const Steinberg::Vst::Event& e)
    {
        switch (e.type)
        {
            case Steinberg::Vst::Event::kNoteOnEvent:
                return MidiMessage::noteOn (createSafeChannel (e.noteOn.channel),
                                            createSafeNote (e.noteOn.pitch),
                                            (Steinberg::uint8) denormaliseToMidiValue (e.noteOn.velocity));

            case Steinberg::Vst::Event::kNoteOffEvent:
                return MidiMessage::noteOff (createSafeChannel (e.noteOff.channel),
                                             createSafeNote (e.noteOff.pitch),
                                             (Steinberg::uint8) denormaliseToMidiValue (e.noteOff.velocity));

            case Steinberg::Vst::Event::kPolyPressureEvent:
                return MidiMessage::aftertouchChange (createSafeChannel (e.polyPressure.channel),
                                                      createSafeNote (e.polyPressure.pitch),
                                                      (Steinberg::uint8) denormaliseToMidiValue (e.polyPressure.pressure));

            case Steinberg::Vst::Event::kDataEvent:
                return toMidiMessage (e.data);

            case Steinberg::Vst::Event::kLegacyMIDICCOutEvent:
                return toMidiMessage (e.midiCCOut);

            case Steinberg::Vst::Event::kNoteExpressionValueEvent:
            case Steinberg::Vst::Event::kNoteExpressionTextEvent:
            case Steinberg::Vst::Event::kChordEvent:
            case Steinberg::Vst::Event::kScaleEvent:
                return {};

            default:
                break;
        }

        // If this is hit, we've been sent an event type that doesn't exist in the VST3 spec.
        jassertfalse;
        return {};
    }

    //==============================================================================
    struct Vst3MidiControlEvent
    {
        Steinberg::Vst::CtrlNumber controllerNumber;
        Steinberg::Vst::ParamValue paramValue;
    };

    static std::optional<Vst3MidiControlEvent> toVst3ControlEvent (const MidiMessage& msg)
    {
        if (msg.isController())
            return Vst3MidiControlEvent { (Steinberg::Vst::CtrlNumber) msg.getControllerNumber(), msg.getControllerValue() / 127.0 };

        if (msg.isPitchWheel())
            return Vst3MidiControlEvent { Steinberg::Vst::kPitchBend, msg.getPitchWheelValue() / 16383.0};

        if (msg.isChannelPressure())
            return Vst3MidiControlEvent { Steinberg::Vst::kAfterTouch, msg.getChannelPressureValue() / 127.0};

        return {};
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiEventList)
};

//==============================================================================
/*  Provides very quick polling of all parameter states.

    We must iterate all parameters on each processBlock call to check whether any
    parameter value has changed. This class attempts to make this polling process
    as quick as possible.

    The indices here are of type Steinberg::int32, as they are expected to correspond
    to parameter information obtained from the IEditController. These indices may not
    match the indices of parameters returned from AudioProcessor::getParameters(), so
    be careful!
*/
class CachedParamValues
{
public:
    CachedParamValues() = default;

    explicit CachedParamValues (std::vector<Steinberg::Vst::ParamID> paramIdsIn)
        : paramIds (std::move (paramIdsIn)), floatCache (paramIds.size()) {}

    size_t size() const noexcept { return floatCache.size(); }

    Steinberg::Vst::ParamID getParamID (Steinberg::int32 index) const noexcept { return paramIds[(size_t) index]; }

    void set (Steinberg::int32 index, float value)
    {
        floatCache.setValueAndBits ((size_t) index, value, 1);
    }

    float exchangeWithoutNotifying (Steinberg::int32 index, float value)
    {
        return floatCache.exchangeValue ((size_t) index, value);
    }

    float get (Steinberg::int32 index) const noexcept { return floatCache.get ((size_t) index); }

    template <typename Callback>
    void ifSet (Callback&& callback)
    {
        floatCache.ifSet ([&] (size_t index, float value, uint32_t)
        {
            callback ((Steinberg::int32) index, value);
        });
    }

private:
    std::vector<Steinberg::Vst::ParamID> paramIds;
    FlaggedFloatCache<1> floatCache;
};

//==============================================================================
/*  Ensures that a 'restart' call only ever happens on the main thread. */
class ComponentRestarter : private AsyncUpdater
{
public:
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void restartComponentOnMessageThread (int32 flags) = 0;
    };

    explicit ComponentRestarter (Listener& listenerIn)
        : listener (listenerIn) {}

    ~ComponentRestarter() noexcept override
    {
        cancelPendingUpdate();
    }

    void restart (int32 newFlags)
    {
        if (newFlags == 0)
            return;

        flags.fetch_or (newFlags);

        if (MessageManager::getInstance()->isThisTheMessageThread())
            handleAsyncUpdate();
        else
            triggerAsyncUpdate();
    }

private:
    void handleAsyncUpdate() override
    {
        listener.restartComponentOnMessageThread (flags.exchange (0));
    }

    Listener& listener;
    std::atomic<int32> flags { 0 };
};

JUCE_END_NO_SANITIZE

} // namespace juce

#endif
