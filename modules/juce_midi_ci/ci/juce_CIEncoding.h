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

namespace juce::midi_ci
{

#define JUCE_ENCODINGS X(ascii, "ASCII") X(mcoded7, "Mcoded7") X(zlibAndMcoded7, "zlib+Mcoded7")

/**
    Identifies different encodings that may be used by property exchange messages.

    @tags{Audio}
*/
enum class Encoding
{
   #define X(name, unused) name,
    JUCE_ENCODINGS
   #undef X
};

/**
    Utility functions for working with the Encoding enum.

    @tags{Audio}
*/
struct EncodingUtils
{
    EncodingUtils() = delete;

    /** Converts an Encoding to a human-readable string. */
    static const char* toString (Encoding e)
    {
        switch (e)
        {
           #define X(name, string) case Encoding::name: return string;
            JUCE_ENCODINGS
           #undef X
        }

        return nullptr;
    }

    /** Converts an encoding string from a property exchange JSON header to
        an Encoding.
    */
    static std::optional<Encoding> toEncoding (const char* str)
    {
       #define X(name, string) if (std::string_view (str) == std::string_view (string)) return Encoding::name;
        JUCE_ENCODINGS
       #undef X

        return {};
    }
};

#undef JUCE_ENCODINGS

} // namespace juce::midi_ci

/** @cond */
namespace juce
{
    template <>
    struct SerialisationTraits<midi_ci::Encoding>
    {
        static constexpr auto marshallingVersion = std::nullopt;

        template <typename Archive>
        void load (Archive& archive, midi_ci::Encoding& t)
        {
            String encoding;
            archive (encoding);
            t = midi_ci::EncodingUtils::toEncoding (encoding.toRawUTF8()).value_or (midi_ci::Encoding{});
        }

        template <typename Archive>
        void save (Archive& archive, const midi_ci::Encoding& t)
        {
            archive (midi_ci::EncodingUtils::toString (t));
        }
    };

} // namespace juce
/** @endcond */
