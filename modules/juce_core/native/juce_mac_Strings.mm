/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

String String::fromCFString (CFStringRef cfString)
{
    if (cfString == 0)
        return String::empty;

    CFRange range = { 0, CFStringGetLength (cfString) };
    HeapBlock <UniChar> u ((size_t) range.length + 1);
    CFStringGetCharacters (cfString, range, u);
    u[range.length] = 0;

    return String (CharPointer_UTF16 ((const CharPointer_UTF16::CharType*) u.getData()));
}

CFStringRef String::toCFString() const
{
    CharPointer_UTF16 utf16 (toUTF16());
    return CFStringCreateWithCharacters (kCFAllocatorDefault, (const UniChar*) utf16.getAddress(), (CFIndex) utf16.length());
}

String String::convertToPrecomposedUnicode() const
{
   #if JUCE_IOS
    JUCE_AUTORELEASEPOOL
    return nsStringToJuce ([juceStringToNS (*this) precomposedStringWithCanonicalMapping]);
   #else
    UnicodeMapping map;

    map.unicodeEncoding = CreateTextEncoding (kTextEncodingUnicodeDefault,
                                              kUnicodeNoSubset,
                                              kTextEncodingDefaultFormat);

    map.otherEncoding = CreateTextEncoding (kTextEncodingUnicodeDefault,
                                            kUnicodeCanonicalCompVariant,
                                            kTextEncodingDefaultFormat);

    map.mappingVersion = kUnicodeUseLatestMapping;

    UnicodeToTextInfo conversionInfo = 0;
    String result;

    if (CreateUnicodeToTextInfo (&map, &conversionInfo) == noErr)
    {
        const size_t bytesNeeded = CharPointer_UTF16::getBytesRequiredFor (getCharPointer());

        HeapBlock <char> tempOut;
        tempOut.calloc (bytesNeeded + 4);

        ByteCount bytesRead = 0;
        ByteCount outputBufferSize = 0;

        if (ConvertFromUnicodeToText (conversionInfo,
                                      bytesNeeded, (ConstUniCharArrayPtr) toUTF16().getAddress(),
                                      kUnicodeDefaultDirectionMask,
                                      0, 0, 0, 0,
                                      bytesNeeded, &bytesRead,
                                      &outputBufferSize, tempOut) == noErr)
        {
            result = String (CharPointer_UTF16 ((CharPointer_UTF16::CharType*) tempOut.getData()));
        }

        DisposeUnicodeToTextInfo (&conversionInfo);
    }

    return result;
   #endif
}
