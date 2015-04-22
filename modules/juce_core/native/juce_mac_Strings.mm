/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

String String::fromCFString (CFStringRef cfString)
{
    if (cfString == 0)
        return String();

    CFRange range = { 0, CFStringGetLength (cfString) };
    CFIndex bytesNeeded = 0;
    CFStringGetBytes (cfString, range, kCFStringEncodingUTF8, 0, false, nullptr, 0, &bytesNeeded);

    HeapBlock<UInt8> utf8 ((size_t) bytesNeeded + 1);
    CFStringGetBytes (cfString, range, kCFStringEncodingUTF8, 0, false, utf8, bytesNeeded + 1, nullptr);

    return String (CharPointer_UTF8 ((const CharPointer_UTF8::CharType*) utf8.getData()),
                   CharPointer_UTF8 ((const CharPointer_UTF8::CharType*) utf8.getData() + bytesNeeded));
}

CFStringRef String::toCFString() const
{
    const char* const utf8 = toRawUTF8();
    return CFStringCreateWithBytes (kCFAllocatorDefault, (const UInt8*) utf8,
                                    (CFIndex) strlen (utf8), kCFStringEncodingUTF8, false);
}

String String::convertToPrecomposedUnicode() const
{
   #if JUCE_IOS
    JUCE_AUTORELEASEPOOL
    {
        return nsStringToJuce ([juceStringToNS (*this) precomposedStringWithCanonicalMapping]);
    }
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

        HeapBlock<char> tempOut;
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
