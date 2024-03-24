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

//==============================================================================
/**
    This class represents a single value for any of the MPE
    dimensions of control. It supports values with 7-bit or 14-bit resolutions
    (corresponding to 1 or 2 MIDI bytes, respectively). It also offers helper
    functions to query the value in a variety of representations that can be
    useful in an audio or MIDI context.

    @tags{Audio}
*/
class JUCE_API  MPEValue
{
public:
    //==============================================================================
    /** Default constructor.

        Constructs an MPEValue corresponding to the centre value.
    */
    MPEValue() noexcept;

    /** Constructs an MPEValue from an integer between 0 and 127
        (using 7-bit precision).
    */
    static MPEValue from7BitInt (int value) noexcept;

    /** Constructs an MPEValue from an integer between 0 and 16383
        (using 14-bit precision).
    */
    static MPEValue from14BitInt (int value) noexcept;

    /** Constructs an MPEValue from a float between 0.0f and 1.0f. */
    static MPEValue fromUnsignedFloat (float value) noexcept;

    /** Constructs an MPEValue from a float between -1.0f and 1.0f. */
    static MPEValue fromSignedFloat (float value) noexcept;

    /** Constructs an MPEValue corresponding to the centre value. */
    static MPEValue centreValue() noexcept;

    /** Constructs an MPEValue corresponding to the minimum value. */
    static MPEValue minValue() noexcept;

    /** Constructs an MPEValue corresponding to the maximum value. */
    static MPEValue maxValue() noexcept;

    /** Retrieves the current value as an integer between 0 and 127.

        Information will be lost if the value was initialised with a precision
        higher than 7-bit.
    */
    int as7BitInt() const noexcept;

    /** Retrieves the current value as an integer between 0 and 16383.

        Resolution will be lost if the value was initialised with a precision
        higher than 14-bit.
    */
    int as14BitInt() const noexcept;

    /** Retrieves the current value mapped to a float between -1.0f and 1.0f. */
    float asSignedFloat() const noexcept;

    /** Retrieves the current value mapped to a float between 0.0f and 1.0f. */
    float asUnsignedFloat() const noexcept;

    /** Returns true if two values are equal. */
    bool operator== (const MPEValue& other) const noexcept;

    /** Returns true if two values are not equal. */
    bool operator!= (const MPEValue& other) const noexcept;

private:
    //==============================================================================
    MPEValue (int normalisedValue);
    int normalisedValue = 8192;
};

} // namespace juce
