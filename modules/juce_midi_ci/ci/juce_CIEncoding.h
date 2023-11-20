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

#ifndef DOXYGEN

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

#endif  // ifndef DOXYGEN
