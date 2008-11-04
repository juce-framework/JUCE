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

#include "../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AudioDataConverters.h"


//==============================================================================
void AudioDataConverters::convertFloatToInt16LE (const float* source, void* dest, int numSamples, const int destBytesPerSample)
{
    const double maxVal = (double) 0x7fff;
    char* intData = (char*) dest;

    if (dest != (void*) source || destBytesPerSample <= 4)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            *(uint16*)intData = swapIfBigEndian ((uint16) (short) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
            intData += destBytesPerSample;
        }
    }
    else
    {
        intData += destBytesPerSample * numSamples;

        for (int i = numSamples; --i >= 0;)
        {
            intData -= destBytesPerSample;
            *(uint16*)intData = swapIfBigEndian ((uint16) (short) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
        }
    }
}

void AudioDataConverters::convertFloatToInt16BE (const float* source, void* dest, int numSamples, const int destBytesPerSample)
{
    const double maxVal = (double) 0x7fff;
    char* intData = (char*) dest;

    if (dest != (void*) source || destBytesPerSample <= 4)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            *(uint16*)intData = swapIfLittleEndian ((uint16) (short) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
            intData += destBytesPerSample;
        }
    }
    else
    {
        intData += destBytesPerSample * numSamples;

        for (int i = numSamples; --i >= 0;)
        {
            intData -= destBytesPerSample;
            *(uint16*)intData = swapIfLittleEndian ((uint16) (short) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
        }
    }
}

void AudioDataConverters::convertFloatToInt24LE (const float* source, void* dest, int numSamples, const int destBytesPerSample)
{
    const double maxVal = (double) 0x7fffff;
    char* intData = (char*) dest;

    if (dest != (void*) source || destBytesPerSample <= 4)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            littleEndian24BitToChars ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])), intData);
            intData += destBytesPerSample;
        }
    }
    else
    {
        intData += destBytesPerSample * numSamples;

        for (int i = numSamples; --i >= 0;)
        {
            intData -= destBytesPerSample;
            littleEndian24BitToChars ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])), intData);
        }
    }
}

void AudioDataConverters::convertFloatToInt24BE (const float* source, void* dest, int numSamples, const int destBytesPerSample)
{
    const double maxVal = (double) 0x7fffff;
    char* intData = (char*) dest;

    if (dest != (void*) source || destBytesPerSample <= 4)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            bigEndian24BitToChars ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])), intData);
            intData += destBytesPerSample;
        }
    }
    else
    {
        intData += destBytesPerSample * numSamples;

        for (int i = numSamples; --i >= 0;)
        {
            intData -= destBytesPerSample;
            bigEndian24BitToChars ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])), intData);
        }
    }
}

void AudioDataConverters::convertFloatToInt32LE (const float* source, void* dest, int numSamples, const int destBytesPerSample)
{
    const double maxVal = (double) 0x7fffffff;
    char* intData = (char*) dest;

    if (dest != (void*) source || destBytesPerSample <= 4)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            *(uint32*)intData = swapIfBigEndian ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
            intData += destBytesPerSample;
        }
    }
    else
    {
        intData += destBytesPerSample * numSamples;

        for (int i = numSamples; --i >= 0;)
        {
            intData -= destBytesPerSample;
            *(uint32*)intData = swapIfBigEndian ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
        }
    }
}

void AudioDataConverters::convertFloatToInt32BE (const float* source, void* dest, int numSamples, const int destBytesPerSample)
{
    const double maxVal = (double) 0x7fffffff;
    char* intData = (char*) dest;

    if (dest != (void*) source || destBytesPerSample <= 4)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            *(uint32*)intData = swapIfLittleEndian ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
            intData += destBytesPerSample;
        }
    }
    else
    {
        intData += destBytesPerSample * numSamples;

        for (int i = numSamples; --i >= 0;)
        {
            intData -= destBytesPerSample;
            *(uint32*)intData = swapIfLittleEndian ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
        }
    }
}

void AudioDataConverters::convertFloatToFloat32LE (const float* source, void* dest, int numSamples, const int destBytesPerSample)
{
    jassert (dest != (void*) source || destBytesPerSample <= 4); // This op can't be performed on in-place data!

    char* d = (char*) dest;

    for (int i = 0; i < numSamples; ++i)
    {
        *(float*)d = source[i];

#if JUCE_BIG_ENDIAN
        *(uint32*)d = swapByteOrder (*(uint32*)d);
#endif

        d += destBytesPerSample;
    }
}

void AudioDataConverters::convertFloatToFloat32BE (const float* source, void* dest, int numSamples, const int destBytesPerSample)
{
    jassert (dest != (void*) source || destBytesPerSample <= 4); // This op can't be performed on in-place data!

    char* d = (char*) dest;

    for (int i = 0; i < numSamples; ++i)
    {
        *(float*)d = source[i];

#if JUCE_LITTLE_ENDIAN
        *(uint32*)d = swapByteOrder (*(uint32*)d);
#endif

        d += destBytesPerSample;
    }
}

//==============================================================================
void AudioDataConverters::convertInt16LEToFloat (const void* const source, float* const dest, int numSamples, const int srcBytesPerSample)
{
    const float scale = 1.0f / 0x7fff;
    const char* intData = (const char*) source;

    if (source != (void*) dest || srcBytesPerSample >= 4)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            dest[i] = scale * (short) swapIfBigEndian (*(uint16*)intData);
            intData += srcBytesPerSample;
        }
    }
    else
    {
        intData += srcBytesPerSample * numSamples;

        for (int i = numSamples; --i >= 0;)
        {
            intData -= srcBytesPerSample;
            dest[i] = scale * (short) swapIfBigEndian (*(uint16*)intData);
        }
    }
}

void AudioDataConverters::convertInt16BEToFloat (const void* const source, float* const dest, int numSamples, const int srcBytesPerSample)
{
    const float scale = 1.0f / 0x7fff;
    const char* intData = (const char*) source;

    if (source != (void*) dest || srcBytesPerSample >= 4)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            dest[i] = scale * (short) swapIfLittleEndian (*(uint16*)intData);
            intData += srcBytesPerSample;
        }
    }
    else
    {
        intData += srcBytesPerSample * numSamples;

        for (int i = numSamples; --i >= 0;)
        {
            intData -= srcBytesPerSample;
            dest[i] = scale * (short) swapIfLittleEndian (*(uint16*)intData);
        }
    }
}

void AudioDataConverters::convertInt24LEToFloat (const void* const source, float* const dest, int numSamples, const int srcBytesPerSample)
{
    const float scale = 1.0f / 0x7fffff;
    const char* intData = (const char*) source;

    if (source != (void*) dest || srcBytesPerSample >= 4)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            dest[i] = scale * (short) littleEndian24Bit (intData);
            intData += srcBytesPerSample;
        }
    }
    else
    {
        intData += srcBytesPerSample * numSamples;

        for (int i = numSamples; --i >= 0;)
        {
            intData -= srcBytesPerSample;
            dest[i] = scale * (short) littleEndian24Bit (intData);
        }
    }
}

void AudioDataConverters::convertInt24BEToFloat (const void* const source, float* const dest, int numSamples, const int srcBytesPerSample)
{
    const float scale = 1.0f / 0x7fffff;
    const char* intData = (const char*) source;

    if (source != (void*) dest || srcBytesPerSample >= 4)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            dest[i] = scale * (short) bigEndian24Bit (intData);
            intData += srcBytesPerSample;
        }
    }
    else
    {
        intData += srcBytesPerSample * numSamples;

        for (int i = numSamples; --i >= 0;)
        {
            intData -= srcBytesPerSample;
            dest[i] = scale * (short) bigEndian24Bit (intData);
        }
    }
}

void AudioDataConverters::convertInt32LEToFloat (const void* const source, float* const dest, int numSamples, const int srcBytesPerSample)
{
    const float scale = 1.0f / 0x7fffffff;
    const char* intData = (const char*) source;

    if (source != (void*) dest || srcBytesPerSample >= 4)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            dest[i] = scale * (int) swapIfBigEndian (*(uint32*) intData);
            intData += srcBytesPerSample;
        }
    }
    else
    {
        intData += srcBytesPerSample * numSamples;

        for (int i = numSamples; --i >= 0;)
        {
            intData -= srcBytesPerSample;
            dest[i] = scale * (int) swapIfBigEndian (*(uint32*) intData);
        }
    }
}

void AudioDataConverters::convertInt32BEToFloat (const void* const source, float* const dest, int numSamples, const int srcBytesPerSample)
{
    const float scale = 1.0f / 0x7fffffff;
    const char* intData = (const char*) source;

    if (source != (void*) dest || srcBytesPerSample >= 4)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            dest[i] = scale * (int) (swapIfLittleEndian (*(uint32*) intData));
            intData += srcBytesPerSample;
        }
    }
    else
    {
        intData += srcBytesPerSample * numSamples;

        for (int i = numSamples; --i >= 0;)
        {
            intData -= srcBytesPerSample;
            dest[i] = scale * (int) (swapIfLittleEndian (*(uint32*) intData));
        }
    }
}

void AudioDataConverters::convertFloat32LEToFloat (const void* const source, float* const dest, int numSamples, const int srcBytesPerSample)
{
    const char* s = (const char*) source;

    for (int i = 0; i < numSamples; ++i)
    {
        dest[i] = *(float*)s;

#if JUCE_BIG_ENDIAN
        uint32* const d = (uint32*) (dest + i);
        *d = swapByteOrder (*d);
#endif

        s += srcBytesPerSample;
    }
}

void AudioDataConverters::convertFloat32BEToFloat (const void* const source, float* const dest, int numSamples, const int srcBytesPerSample)
{
    const char* s = (const char*) source;

    for (int i = 0; i < numSamples; ++i)
    {
        dest[i] = *(float*)s;

#if JUCE_LITTLE_ENDIAN
        uint32* const d = (uint32*) (dest + i);
        *d = swapByteOrder (*d);
#endif

        s += srcBytesPerSample;
    }
}


//==============================================================================
void AudioDataConverters::convertFloatToFormat (const DataFormat destFormat,
                                                const float* const source,
                                                void* const dest,
                                                const int numSamples)
{
    switch (destFormat)
    {
    case int16LE:
        convertFloatToInt16LE (source, dest, numSamples);
        break;

    case int16BE:
        convertFloatToInt16BE (source, dest, numSamples);
        break;

    case int24LE:
        convertFloatToInt24LE (source, dest, numSamples);
        break;

    case int24BE:
        convertFloatToInt24BE (source, dest, numSamples);
        break;

    case int32LE:
        convertFloatToInt32LE (source, dest, numSamples);
        break;

    case int32BE:
        convertFloatToInt32BE (source, dest, numSamples);
        break;

    case float32LE:
        convertFloatToFloat32LE (source, dest, numSamples);
        break;

    case float32BE:
        convertFloatToFloat32BE (source, dest, numSamples);
        break;

    default:
        jassertfalse
        break;
    }
}

void AudioDataConverters::convertFormatToFloat (const DataFormat sourceFormat,
                                                const void* const source,
                                                float* const dest,
                                                const int numSamples)
{
    switch (sourceFormat)
    {
    case int16LE:
        convertInt16LEToFloat (source, dest, numSamples);
        break;

    case int16BE:
        convertInt16BEToFloat (source, dest, numSamples);
        break;

    case int24LE:
        convertInt24LEToFloat (source, dest, numSamples);
        break;

    case int24BE:
        convertInt24BEToFloat (source, dest, numSamples);
        break;

    case int32LE:
        convertInt32LEToFloat (source, dest, numSamples);
        break;

    case int32BE:
        convertInt32BEToFloat (source, dest, numSamples);
        break;

    case float32LE:
        convertFloat32LEToFloat (source, dest, numSamples);
        break;

    case float32BE:
        convertFloat32BEToFloat (source, dest, numSamples);
        break;

    default:
        jassertfalse
        break;
    }
}

//==============================================================================
void AudioDataConverters::interleaveSamples (const float** const source,
                                             float* const dest,
                                             const int numSamples,
                                             const int numChannels)
{
    for (int chan = 0; chan < numChannels; ++chan)
    {
        int i = chan;
        const float* src = source [chan];

        for (int j = 0; j < numSamples; ++j)
        {
            dest [i] = src [j];
            i += numChannels;
        }
    }
}

void AudioDataConverters::deinterleaveSamples (const float* const source,
                                               float** const dest,
                                               const int numSamples,
                                               const int numChannels)
{
    for (int chan = 0; chan < numChannels; ++chan)
    {
        int i = chan;
        float* dst = dest [chan];

        for (int j = 0; j < numSamples; ++j)
        {
            dst [j] = source [i];
            i += numChannels;
        }
    }
}


END_JUCE_NAMESPACE
