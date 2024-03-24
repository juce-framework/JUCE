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

namespace juce
{

const OSCTimeTag OSCTimeTag::immediately;

static const uint64 millisecondsBetweenOscAndJuceEpochs = 2208988800000ULL;
static const uint64 rawTimeTagRepresentingImmediately = 0x0000000000000001ULL;

//==============================================================================
OSCTimeTag::OSCTimeTag() noexcept  : rawTimeTag (rawTimeTagRepresentingImmediately)
{
}

OSCTimeTag::OSCTimeTag (uint64 t) noexcept  : rawTimeTag (t)
{
}

OSCTimeTag::OSCTimeTag (Time time) noexcept
{
    const uint64 milliseconds = (uint64) time.toMilliseconds() + millisecondsBetweenOscAndJuceEpochs;

    uint64 seconds = milliseconds / 1000;
    uint32 fractionalPart = uint32 (4294967.296 * (milliseconds % 1000));

    rawTimeTag = (seconds << 32) + fractionalPart;
}

//==============================================================================
Time OSCTimeTag::toTime() const noexcept
{
    const uint64 seconds = rawTimeTag >> 32;
    const uint32 fractionalPart = (rawTimeTag & 0x00000000FFFFFFFFULL);

    const auto fractionalPartInMillis = (double) fractionalPart / 4294967.296;

    // now using signed integer, because this is allowed to become negative:
    const auto juceTimeInMillis = (int64) (seconds * 1000)
                                + (int64) roundToInt (fractionalPartInMillis)
                                - (int64) millisecondsBetweenOscAndJuceEpochs;

    return Time (juceTimeInMillis);
}

//==============================================================================
bool OSCTimeTag::isImmediately() const noexcept
{
    return rawTimeTag == rawTimeTagRepresentingImmediately;
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class OSCTimeTagTests final : public UnitTest
{
public:
    OSCTimeTagTests()
        : UnitTest ("OSCTimeTag class", UnitTestCategories::osc)
    {}

    void runTest() override
    {
        beginTest ("Basics");

        {
            OSCTimeTag tag;
            expect (tag.isImmediately());
        }
        {
            OSCTimeTag tag (3535653);
            expect (! tag.isImmediately());

            OSCTimeTag otherTag;
            otherTag = tag;
            expect (! otherTag.isImmediately());

            OSCTimeTag copyTag (tag);
            expect (! copyTag.isImmediately());
        }

        beginTest ("Conversion to/from JUCE Time");

        {
            Time time;
            OSCTimeTag tag (time);
            expect (! tag.isImmediately());
        }
        {
            OSCTimeTag tag;
            Time time = tag.toTime();
            expect (time < Time::getCurrentTime());
        }
        {
            Time currentTime (Time::currentTimeMillis());
            double deltaInSeconds = 1.234;
            RelativeTime delta (deltaInSeconds);
            Time laterTime = currentTime + delta;

            OSCTimeTag currentTimeTag (currentTime);
            OSCTimeTag laterTimeTag (laterTime);

            uint64 currentTimeTagRaw = currentTimeTag.getRawTimeTag();
            uint64 laterTimeTagRaw = laterTimeTag.getRawTimeTag();

            // in the raw time tag, the most significant 32 bits are seconds,
            // so let's verify that the difference is right:
            uint64 diff = laterTimeTagRaw - currentTimeTagRaw;
            double acceptableErrorInSeconds = 0.000001; // definitely not audible anymore.

            expect ((float) diff / float (1ULL << 32) < deltaInSeconds + acceptableErrorInSeconds );
            expect ((float) diff / float (1ULL << 32) > deltaInSeconds - acceptableErrorInSeconds );

            // round trip:

            Time currentTime2 = currentTimeTag.toTime();
            Time laterTime2 = laterTimeTag.toTime();
            RelativeTime delta2 = laterTime2 - currentTime2;

            expect (currentTime2 == currentTime);
            expect (laterTime2 == laterTime);
            expect (delta2 == delta);
        }
    }
};

static OSCTimeTagTests OSCTimeTagUnitTests;

#endif

} // namespace juce
