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

namespace juce
{

BlocksVersion::BlocksVersion (const String& versionString)
{
    evaluate (versionString);
}

String BlocksVersion::toString (bool extended) const
{
    String output = String (major) + "." + String (minor) + "." + String (patch);

    if (extended)
    {
        if (releaseType.isNotEmpty())
            output += "-" + releaseType + "-" + String (releaseCount);

        if (commit.isNotEmpty())
            output += "-" + commit;

        if (forced)
            output += "-f";
    }

    return output;
}

static std::regex getRegEx()
{
    static const std::string majorMinorPatchRegex { "([0-9]+)\\.([0-9]+)\\.([0-9]+)" };
    static const std::string releaseAndCommitDetailsRegex { "(?:-(alpha|beta|rc))?(?:-([0-9]+))?(?:-g([A-Za-z0-9]+))?" };
    static const std::string forcedUpdateRegex { "(-f)?" };

    static const std::regex regEx ("(?:.+)?" + majorMinorPatchRegex + releaseAndCommitDetailsRegex + forcedUpdateRegex + "(?:.+)?");

    return regEx;
}

bool BlocksVersion::isValidVersion (const String& versionString)
{
    return std::regex_match (versionString.toRawUTF8(), getRegEx());
}

bool BlocksVersion::evaluate (const String& versionString)
{
    std::cmatch groups;
    const bool result = std::regex_match (versionString.toRawUTF8(), groups, getRegEx());

    jassert (result);

    auto toInt = [](const std::sub_match<const char*> match)
    {
        return std::atoi (match.str().c_str());
    };

    enum tags { FULL, MAJOR, MINOR, PATCH, RELEASE, COUNT, COMMIT, FORCED};

    major        = toInt  (groups[MAJOR]);
    minor        = toInt  (groups[MINOR]);
    patch        = toInt  (groups[PATCH]);
    releaseType  = String (groups[RELEASE]);
    releaseCount = toInt  (groups[COUNT]);
    commit       = String (groups[COMMIT]);
    forced       = groups[FORCED].matched;

    return result;
}

bool BlocksVersion::isEqualTo (const BlocksVersion& other) const
{
    return major == other.major &&
           minor == other.minor &&
           patch == other.patch &&
           releaseType  == other.releaseType &&
           releaseCount == other.releaseCount;
}

bool BlocksVersion::isGreaterThan (const BlocksVersion& other) const
{
    if (major != other.major) return (major > other.major);
    if (minor != other.minor) return (minor > other.minor);
    if (patch != other.patch) return (patch > other.patch);

    return releaseTypeGreaterThan (other);
}

bool BlocksVersion::releaseTypeGreaterThan (const BlocksVersion& other) const
{
    auto getReleaseTypePriority = [](const BlocksVersion& version)
    {
        String releaseTypes[4] = { "alpha", "beta", "rc", {} };

        for (int i = 0; i < 4; ++i)
            if (version.releaseType == releaseTypes[i])
                return i;

        return -1;
    };

    if (releaseType != other.releaseType)
        return getReleaseTypePriority (*this) > getReleaseTypePriority (other);

    return releaseCount > other.releaseCount;
}

bool BlocksVersion::operator== (const BlocksVersion& other) const
{
    return isEqualTo (other);
}

bool BlocksVersion::operator!=(const BlocksVersion& other) const
{
    return ! (*this == other);
}

bool BlocksVersion::operator> (const BlocksVersion& other) const
{
    return isGreaterThan (other);
}

bool BlocksVersion::operator< (const BlocksVersion& other) const
{
    return ! (*this > other) && (*this != other);
}

bool BlocksVersion::operator<= (const BlocksVersion& other) const
{
    return (*this < other) || (*this == other);
}

bool BlocksVersion::operator>= (const BlocksVersion& other) const
{
    return (*this > other) || (*this == other);
}

#if JUCE_UNIT_TESTS
class BlocksVersionUnitTests  : public UnitTest
{
public:
    BlocksVersionUnitTests() : UnitTest ("BlocksVersionUnitTests", "Blocks") {}

    void runTest() override
    {
        beginTest ("Compare patch number");
        expect (BlocksVersion ("4.6.7") < BlocksVersion ("4.6.11"));
        expect (BlocksVersion ("4.6.6") > BlocksVersion ("4.6.2"));
        expect (BlocksVersion ("4.6.5") <= BlocksVersion ("4.6.8"));
        expect (BlocksVersion ("4.6.4") >= BlocksVersion ("4.6.3"));

        beginTest ("Compare minor number");
        expect (BlocksVersion ("4.5.9") < BlocksVersion ("4.6.7"));
        expect (BlocksVersion ("4.15.2") > BlocksVersion ("4.6.6"));
        expect (BlocksVersion ("4.4.8") <= BlocksVersion ("4.6.5"));
        expect (BlocksVersion ("4.7.4") >= BlocksVersion ("4.6.3"));

        beginTest ("Compare major number");
        expect (BlocksVersion ("4.6.9") < BlocksVersion ("8.5.7"));
        expect (BlocksVersion ("15.6.2") > BlocksVersion ("4.9.6"));
        expect (BlocksVersion ("4.6.8") <= BlocksVersion ("7.4.5"));
        expect (BlocksVersion ("5.6.4") >= BlocksVersion ("4.7.3"));

        beginTest ("Compare build number");
        expect (BlocksVersion ("0.3.2-alpha-3-gjduh") < BlocksVersion ("0.3.2-alpha-12-gjduh"));
        expect (BlocksVersion ("0.3.2-alpha-4-gjduh") > BlocksVersion ("0.3.2-alpha-1-gjduh"));
        expect (BlocksVersion ("0.3.2-beta-5-gjduh") <= BlocksVersion ("0.3.2-beta-6-gjduh"));
        expect (BlocksVersion ("0.3.2-beta-6-gjduh") >= BlocksVersion ("0.3.2-beta-3-gjduh"));

        beginTest ("Compare build type");
        expect (BlocksVersion ("0.3.2-alpha-3-gjduhenf") < BlocksVersion ("0.3.2-beta-1-gjduhenf"));
        expect (BlocksVersion ("0.3.2-beta-3-gjduhenf") < BlocksVersion ("0.3.2"));
        expect (BlocksVersion ("0.3.2") > BlocksVersion ("0.3.2-alpha-3-gjduhenf"));

        beginTest ("Compare equal numbers");
        expect (BlocksVersion ("4.6.7") == BlocksVersion ("4.6.7"));
        expect (BlocksVersion ("4.6.7-alpha-3-gsdfsf") == BlocksVersion ("4.6.7-alpha-3-gsdfsf"));

        beginTest ("Identify forced version");
        expect (BlocksVersion("0.2.2-2-g25eaec8a-f").forced);
        expect (BlocksVersion("0.2.2-2-f").forced);
        expect (! BlocksVersion("0.2.2-2-g25eaec8-d7").forced);

        beginTest ("Valid Strings");
        expect (BlocksVersion::isValidVersion ("Rainbow 0.4.5-beta-1-g4c36e"));
        expect (! BlocksVersion::isValidVersion ("0.4-beta-1-g4c36e"));
        expect (! BlocksVersion::isValidVersion ("a.0.4-beta-1-g4c36e"));
        expect (BlocksVersion::isValidVersion ("BLOCKS control 0.2.2-2-g25eaec8a-f.syx"));
        expect (BlocksVersion("BLOCKS control 0.2.2-2-g25eaec8a-f.syx") == BlocksVersion("0.2.2-2-g25eaec8a-f"));

        beginTest ("Default constructors");
        {
            BlocksVersion v1 ("4.5.9");
            BlocksVersion v2 (v1);
            BlocksVersion v3;
            v3 = v1;

            expect (v2 == v1);
            expect (v3 == v1);

            BlocksVersion emptyVersion;
            expect (emptyVersion == BlocksVersion ("0.0.0"));
        }
    }
};

static BlocksVersionUnitTests BlocksVersionUnitTests;
#endif

} // namespace juce
