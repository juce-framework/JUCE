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
void AudioDataConverters::convertFloatToInt16LE (const float* source, void* dest, int numSamples)
{
    const double maxVal = (double) 0x7fff;
    uint16* const intData = (uint16*) dest;

    for (int i = 0; i < numSamples; ++i)
        intData[i] = swapIfBigEndian ((uint16) (short) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
}

void AudioDataConverters::convertFloatToInt16BE (const float* source, void* dest, int numSamples)
{
    const double maxVal = (double) 0x7fff;
    uint16* const intData = (uint16*) dest;

    for (int i = 0; i < numSamples; ++i)
        intData[i] = swapIfLittleEndian ((uint16) (short) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
}

void AudioDataConverters::convertFloatToInt24LE (const float* source, void* dest, int numSamples)
{
    const double maxVal = (double) 0x7fffff;
    char* intData = (char*) dest;

    for (int i = 0; i < numSamples; ++i)
    {
        littleEndian24BitToChars ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])), intData);
        intData += 3;
    }
}

void AudioDataConverters::convertFloatToInt24BE (const float* source, void* dest, int numSamples)
{
    const double maxVal = (double) 0x7fffff;
    char* intData = (char*) dest;

    for (int i = 0; i < numSamples; ++i)
    {
        bigEndian24BitToChars ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])), intData);
        intData += 3;
    }
}

void AudioDataConverters::convertFloatToInt32LE (const float* source, void* dest, int numSamples)
{
    const double maxVal = (double) 0x7fffffff;
    uint32* const intData = (uint32*) dest;

    for (int i = 0; i < numSamples; ++i)
        intData[i] = swapIfBigEndian ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
}

void AudioDataConverters::convertFloatToInt32BE (const float* source, void* dest, int numSamples)
{
    const double maxVal = (double) 0x7fffffff;
    uint32* const intData = (uint32*) dest;

    for (int i = 0; i < numSamples; ++i)
        intData[i] = swapIfLittleEndian ((uint32) roundDoubleToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
}

void AudioDataConverters::convertFloatToFloat32LE (const float* source, void* dest, int numSamples)
{
    if (source != (const float*) dest)
        memcpy (dest, source, numSamples * sizeof (float));

#if JUCE_BIG_ENDIAN
    uint32* const data = (uint32*) dest;

    for (int i = 0; i < numSamples; ++i)
        data[i] = swapByteOrder (data [i]);
#endif
}

void AudioDataConverters::convertFloatToFloat32BE (const float* source, void* dest, int numSamples)
{
    if (source != (const float*) dest)
        memcpy (dest, source, numSamples * sizeof (float));

#if JUCE_LITTLE_ENDIAN
    uint32* const data = (uint32*) dest;

    for (int i = 0; i < numSamples; ++i)
        data[i] = swapByteOrder (data [i]);
#endif
}

//==============================================================================
void AudioDataConverters::convertInt16LEToFloat (const void* const source, float* const dest, int numSamples)
{
    const float scale = 1.0f / 0x7fff;
    uint16* const intData = (uint16*) source;

    while (--numSamples >= 0)
        dest [numSamples] = scale * (short) swapIfBigEndian (intData [numSamples]);
}

void AudioDataConverters::convertInt16BEToFloat (const void* const source, float* const dest, int numSamples)
{
    const float scale = 1.0f / 0x7fff;
    uint16* const intData = (uint16*) source;

    while (--numSamples >= 0)
        dest [numSamples] = scale * (short) swapIfLittleEndian (intData [numSamples]);
}

void AudioDataConverters::convertInt24LEToFloat (const void* const source, float* const dest, int numSamples)
{
    const float scale = 1.0f / 0x7fffff;
    char* const intData = (char*) source;

    while (--numSamples >= 0)
        dest [numSamples] = scale * littleEndian24Bit (intData + (numSamples + numSamples + numSamples));
}

void AudioDataConverters::convertInt24BEToFloat (const void* const source, float* const dest, int numSamples)
{
    const float scale = 1.0f / 0x7fffff;
    char* const intData = (char*) source;

    while (--numSamples >= 0)
        dest [numSamples] = scale * bigEndian24Bit (intData + (numSamples + numSamples + numSamples));
}

void AudioDataConverters::convertInt32LEToFloat (const void* const source, float* const dest, int numSamples)
{
    const float scale = 1.0f / 0x7fffffff;
    uint32* const intData = (uint32*) source;

    for (int i = 0; i < numSamples; ++i)
        dest [numSamples] = scale * (int) swapIfBigEndian (intData [numSamples]);
}

void AudioDataConverters::convertInt32BEToFloat (const void* const source, float* const dest, int numSamples)
{
    const float scale = 1.0f / 0x7fffffff;
    uint32* const intData = (uint32*) source;

    for (int i = 0; i < numSamples; ++i)
        dest [numSamples] = scale * (int) swapIfLittleEndian (intData [numSamples]);
}

void AudioDataConverters::convertFloat32LEToFloat (const void* const source, float* const dest, int numSamples)
{
    convertFloatToFloat32LE ((float*) source, dest, numSamples);
}

void AudioDataConverters::convertFloat32BEToFloat (const void* const source, float* const dest, int numSamples)
{
    convertFloatToFloat32BE ((float*) source, dest, numSamples);
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
