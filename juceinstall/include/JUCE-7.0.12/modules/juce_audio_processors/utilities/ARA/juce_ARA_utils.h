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

#if JucePlugin_Enable_ARA

// Include ARA SDK headers
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wgnu-zero-variadic-macro-arguments",
                                     "-Wunused-parameter",
                                     "-Wfloat-equal")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6387)

#include <ARA_Library/PlugIn/ARAPlug.h>

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

namespace juce
{

using ARAViewSelection = ARA::PlugIn::ViewSelection;
using ARAContentUpdateScopes = ARA::ContentUpdateScopes;
using ARARestoreObjectsFilter = ARA::PlugIn::RestoreObjectsFilter;
using ARAStoreObjectsFilter = ARA::PlugIn::StoreObjectsFilter;

/** Converts an ARA::ARAUtf8String to a JUCE String. */
inline String convertARAString (ARA::ARAUtf8String str)
{
    return String (CharPointer_UTF8 (str));
}

/** Converts a potentially NULL ARA::ARAUtf8String to a JUCE String.

    Returns the JUCE equivalent of the provided string if it's not nullptr, and the fallback string
    otherwise.
*/
inline String convertOptionalARAString (ARA::ARAUtf8String str, const String& fallbackString = String())
{
    return (str != nullptr) ? convertARAString (str) : fallbackString;
}

/** Converts an ARA::ARAColor* to a JUCE Colour. */
inline Colour convertARAColour (const ARA::ARAColor* colour)
{
    return Colour::fromFloatRGBA (colour->r, colour->g, colour->b, 1.0f);
}

/** Converts a potentially NULL ARA::ARAColor* to a JUCE Colour.

    Returns the JUCE equivalent of the provided colour if it's not nullptr, and the fallback colour
    otherwise.
*/
inline Colour convertOptionalARAColour (const ARA::ARAColor* colour, const Colour& fallbackColour = Colour())
{
    return (colour != nullptr) ? convertARAColour (colour) : fallbackColour;
}

} // namespace juce

#include "juce_ARAModelObjects.h"
#include "juce_ARADocumentController.h"
#include "juce_AudioProcessor_ARAExtensions.h"
#include "juce_ARAPlugInInstanceRoles.h"

#endif
