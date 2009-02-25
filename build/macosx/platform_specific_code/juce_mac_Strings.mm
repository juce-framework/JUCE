/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#ifdef JUCE_INCLUDED_FILE

//==============================================================================
static const String nsStringToJuce (NSString* s)
{
    return String::fromUTF8 ((uint8*) [s UTF8String]);
}

static NSString* juceStringToNS (const String& s)
{
    return [NSString stringWithUTF8String: (const char*) s.toUTF8()];
}


//==============================================================================
static const String convertUTF16ToString (const UniChar* utf16)
{
    String s;

    while (*utf16 != 0)
        s += (juce_wchar) *utf16++;

    return s;
}

const String PlatformUtilities::cfStringToJuceString (CFStringRef cfString)
{
    String result;

    if (cfString != 0)
    {
#if JUCE_STRINGS_ARE_UNICODE
        CFRange range = { 0, CFStringGetLength (cfString) };
        UniChar* const u = (UniChar*) juce_malloc (sizeof (UniChar) * (range.length + 1));

        CFStringGetCharacters (cfString, range, u);
        u[range.length] = 0;

        result = convertUTF16ToString (u);

        juce_free (u);
#else
        const int len = CFStringGetLength (cfString);
        char* buffer = (char*) juce_malloc (len + 1);
        CFStringGetCString (cfString, buffer, len + 1, CFStringGetSystemEncoding());
        result = buffer;
        juce_free (buffer);
#endif
    }

    return result;
}

CFStringRef PlatformUtilities::juceStringToCFString (const String& s)
{
#if JUCE_STRINGS_ARE_UNICODE
    const int len = s.length();
    const juce_wchar* t = (const juce_wchar*) s;

    UniChar* temp = (UniChar*) juce_malloc (sizeof (UniChar) * len + 4);

    for (int i = 0; i <= len; ++i)
        temp[i] = t[i];

    CFStringRef result = CFStringCreateWithCharacters (kCFAllocatorDefault, temp, len);
    juce_free (temp);

    return result;

#else
    return CFStringCreateWithCString (kCFAllocatorDefault,
                                      (const char*) s,
                                      CFStringGetSystemEncoding());
#endif
}

const String PlatformUtilities::convertToPrecomposedUnicode (const String& s)
{
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
        const int len = s.length();

        UniChar* const tempIn = (UniChar*) juce_calloc (sizeof (UniChar) * len + 4);
        UniChar* const tempOut = (UniChar*) juce_calloc (sizeof (UniChar) * len + 4);

        for (int i = 0; i <= len; ++i)
            tempIn[i] = s[i];

        ByteCount bytesRead = 0;
        ByteCount outputBufferSize = 0;

        if (ConvertFromUnicodeToText (conversionInfo,
                                      len * sizeof (UniChar), tempIn,
                                      kUnicodeDefaultDirectionMask,
                                      0, 0, 0, 0,
                                      len * sizeof (UniChar), &bytesRead,
                                      &outputBufferSize, tempOut) == noErr)
        {
            result.preallocateStorage (bytesRead / sizeof (UniChar) + 2);

            tchar* t = const_cast <tchar*> ((const tchar*) result);

            unsigned int i;
            for (i = 0; i < bytesRead / sizeof (UniChar); ++i)
                t[i] = (tchar) tempOut[i];

            t[i] = 0;
        }

        juce_free (tempIn);
        juce_free (tempOut);

        DisposeUnicodeToTextInfo (&conversionInfo);
    }

    return result;
}

//==============================================================================
#if ! JUCE_ONLY_BUILD_CORE_LIBRARY

void SystemClipboard::copyTextToClipboard (const String& text) throw()
{
    [[NSPasteboard generalPasteboard] declareTypes: [NSArray arrayWithObject: NSStringPboardType]
                                             owner: nil];

    [[NSPasteboard generalPasteboard] setString: juceStringToNS (text)
                                        forType: NSStringPboardType];
}

const String SystemClipboard::getTextFromClipboard() throw()
{
    NSString* text = [[NSPasteboard generalPasteboard] stringForType: NSStringPboardType];

    return text == 0 ? String::empty
                     : nsStringToJuce (text);
}

#endif

#endif
