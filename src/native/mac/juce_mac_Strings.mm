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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

//==============================================================================
namespace
{
    String nsStringToJuce (NSString* s)
    {
        return CharPointer_UTF8 ([s UTF8String]);
    }

    NSString* juceStringToNS (const String& s)
    {
        return [NSString stringWithUTF8String: s.toUTF8()];
    }
}

String PlatformUtilities::cfStringToJuceString (CFStringRef cfString)
{
    if (cfString == 0)
        return String::empty;

    CFRange range = { 0, CFStringGetLength (cfString) };
    HeapBlock <UniChar> u (range.length + 1);
    CFStringGetCharacters (cfString, range, u);
    u[range.length] = 0;

    return String (CharPointer_UTF16 ((const CharPointer_UTF16::CharType*) u.getData()));
}

CFStringRef PlatformUtilities::juceStringToCFString (const String& s)
{
    CharPointer_UTF16 utf16 (s.toUTF16());

    return CFStringCreateWithCharacters (kCFAllocatorDefault, (const UniChar*) utf16.getAddress(), utf16.length());
}

String PlatformUtilities::convertToPrecomposedUnicode (const String& s)
{
#if JUCE_IOS
    JUCE_AUTORELEASEPOOL
    return nsStringToJuce ([juceStringToNS (s) precomposedStringWithCanonicalMapping]);
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
        const int bytesNeeded = CharPointer_UTF16::getBytesRequiredFor (s.getCharPointer());

        HeapBlock <char> tempOut;
        tempOut.calloc (bytesNeeded + 4);

        ByteCount bytesRead = 0;
        ByteCount outputBufferSize = 0;

        if (ConvertFromUnicodeToText (conversionInfo,
                                      bytesNeeded, (ConstUniCharArrayPtr) s.toUTF16().getAddress(),
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

//==============================================================================
#if ! JUCE_ONLY_BUILD_CORE_LIBRARY

void SystemClipboard::copyTextToClipboard (const String& text)
{
   #if JUCE_IOS
    [[UIPasteboard generalPasteboard] setValue: juceStringToNS (text)
                             forPasteboardType: @"public.text"];
   #else
    [[NSPasteboard generalPasteboard] declareTypes: [NSArray arrayWithObject: NSStringPboardType]
                                             owner: nil];

    [[NSPasteboard generalPasteboard] setString: juceStringToNS (text)
                                        forType: NSStringPboardType];
   #endif
}

String SystemClipboard::getTextFromClipboard()
{
   #if JUCE_IOS
    NSString* text = [[UIPasteboard generalPasteboard] valueForPasteboardType: @"public.text"];
   #else
    NSString* text = [[NSPasteboard generalPasteboard] stringForType: NSStringPboardType];
   #endif

    return text == nil ? String::empty
                       : nsStringToJuce (text);
}

#endif

#endif
