/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

namespace juce
{

struct BlocksVersion
{
public:
    /** The main value in a version number x.0.0 */
    int major = 0;

    /** The secondary value in a version number 1.x.0 */
    int minor = 0;

    /** The tertiary value in a version number 1.0.x */
    int patch = 0;

    /** The release tag for this version, such as "beta", "alpha", "rc", etc */
    juce::String releaseType;

    /** A numberical value assosiated with the release tag, such as "beta 4" */
    int releaseCount = 0;

    /** The assosiated git commit that generated this firmware version */
    juce::String commit;

    /** Identify "forced" firmware builds **/
    bool forced = false;

    juce::String toString (bool extended = false) const;

    /** Constructs a version number from an formatted juce::String */
    BlocksVersion (const juce::String&);

    /** Constructs a version number from another BlocksVersion */
    BlocksVersion (const BlocksVersion& other) = default;

    /** Creates an empty version number **/
    BlocksVersion() = default;

    /** Returns true if string format is valid */
    static bool isValidVersion (const juce::String& versionString);

    bool operator == (const BlocksVersion&) const;
    bool operator != (const BlocksVersion&) const;
    bool operator <  (const BlocksVersion&) const;
    bool operator >  (const BlocksVersion&) const;
    bool operator <= (const BlocksVersion&) const;
    bool operator >= (const BlocksVersion&) const;

private:
    /** @internal */
    bool evaluate (const juce::String& versionString);
    bool releaseTypeGreaterThan (const BlocksVersion& otherReleaseType) const;

    bool isGreaterThan (const BlocksVersion& other) const;
    bool isEqualTo (const BlocksVersion& other) const;
};

} // namespace juce
