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

/**
    Contains some static methods for converting between binary and the
    standard base-64 encoding format.

    @tags{Core}
*/
struct JUCE_API Base64
{
    /** Converts a binary block of data into a base-64 string.
        This will write the resulting string data to the given stream.
        If a write error occurs with the stream, the method will terminate and return false.
    */
    static bool convertToBase64 (OutputStream& base64Result, const void* sourceData, size_t sourceDataSize);

    /** Converts a base-64 string back to its binary representation.
        This will write the decoded binary data to the given stream.
        If the string is not valid base-64, the method will terminate and return false.
    */
    static bool convertFromBase64 (OutputStream& binaryOutput, StringRef base64TextInput);

    /** Converts a block of binary data to a base-64 string. */
    static String toBase64 (const void* sourceData, size_t sourceDataSize);

    /** Converts a string's UTF-8 representation to a base-64 string. */
    static String toBase64 (const String& textToEncode);
};

} // namespace juce
