/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_AUDIODATACONVERTERS_JUCEHEADER__
#define __JUCE_AUDIODATACONVERTERS_JUCEHEADER__


//==============================================================================
/**
    A set of routines to convert buffers of 32-bit floating point data to and from
    various integer formats.

*/
class JUCE_API  AudioDataConverters
{
public:
    //==============================================================================
    static void convertFloatToInt16LE (const float* source, void* dest, int numSamples, const int destBytesPerSample = 2);
    static void convertFloatToInt16BE (const float* source, void* dest, int numSamples, const int destBytesPerSample = 2);

    static void convertFloatToInt24LE (const float* source, void* dest, int numSamples, const int destBytesPerSample = 3);
    static void convertFloatToInt24BE (const float* source, void* dest, int numSamples, const int destBytesPerSample = 3);

    static void convertFloatToInt32LE (const float* source, void* dest, int numSamples, const int destBytesPerSample = 4);
    static void convertFloatToInt32BE (const float* source, void* dest, int numSamples, const int destBytesPerSample = 4);

    static void convertFloatToFloat32LE (const float* source, void* dest, int numSamples, const int destBytesPerSample = 4);
    static void convertFloatToFloat32BE (const float* source, void* dest, int numSamples, const int destBytesPerSample = 4);

    //==============================================================================
    static void convertInt16LEToFloat (const void* source, float* dest, int numSamples, const int srcBytesPerSample = 2);
    static void convertInt16BEToFloat (const void* source, float* dest, int numSamples, const int srcBytesPerSample = 2);

    static void convertInt24LEToFloat (const void* source, float* dest, int numSamples, const int srcBytesPerSample = 3);
    static void convertInt24BEToFloat (const void* source, float* dest, int numSamples, const int srcBytesPerSample = 3);

    static void convertInt32LEToFloat (const void* source, float* dest, int numSamples, const int srcBytesPerSample = 4);
    static void convertInt32BEToFloat (const void* source, float* dest, int numSamples, const int srcBytesPerSample = 4);

    static void convertFloat32LEToFloat (const void* source, float* dest, int numSamples, const int srcBytesPerSample = 4);
    static void convertFloat32BEToFloat (const void* source, float* dest, int numSamples, const int srcBytesPerSample = 4);

    //==============================================================================
    enum DataFormat
    {
        int16LE,
        int16BE,
        int24LE,
        int24BE,
        int32LE,
        int32BE,
        float32LE,
        float32BE,
    };

    static void convertFloatToFormat (const DataFormat destFormat,
                                      const float* source, void* dest, int numSamples);

    static void convertFormatToFloat (const DataFormat sourceFormat,
                                      const void* source, float* dest, int numSamples);

    //==============================================================================
    static void interleaveSamples (const float** source, float* dest,
                                   const int numSamples, const int numChannels);

    static void deinterleaveSamples (const float* source, float** dest,
                                     const int numSamples, const int numChannels);
};


#endif   // __JUCE_AUDIODATACONVERTERS_JUCEHEADER__
