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

#include "juce_AudioFormat.h"
#include "../dsp/juce_AudioSampleBuffer.h"


//==============================================================================
AudioFormatReader::AudioFormatReader (InputStream* const in,
                                      const String& formatName_)
    : sampleRate (0),
      bitsPerSample (0),
      lengthInSamples (0),
      numChannels (0),
      usesFloatingPointData (false),
      input (in),
      formatName (formatName_)
{
}

AudioFormatReader::~AudioFormatReader()
{
    delete input;
}

static void findMaxMin (const float* src, const int num,
                        float& maxVal, float& minVal)
{
    float mn = src[0];
    float mx = mn;

    for (int i = 1; i < num; ++i)
    {
        const float s = src[i];
        if (s > mx)
            mx = s;
        if (s < mn)
            mn = s;
    }

    maxVal = mx;
    minVal = mn;
}

void AudioFormatReader::readMaxLevels (int64 startSampleInFile,
                                       int64 numSamples,
                                       float& lowestLeft, float& highestLeft,
                                       float& lowestRight, float& highestRight)
{
    if (numSamples <= 0)
    {
        lowestLeft = 0;
        lowestRight = 0;
        highestLeft = 0;
        highestRight = 0;
        return;
    }

    const int bufferSize = (int) jmin (numSamples, (int64) 4096);
    MemoryBlock tempSpace (bufferSize * sizeof (int) * 2 + 64);

    int* tempBuffer[3];
    tempBuffer[0] = (int*) tempSpace.getData();
    tempBuffer[1] = ((int*) tempSpace.getData()) + bufferSize;
    tempBuffer[2] = 0;

    if (usesFloatingPointData)
    {
        float lmin = 1.0e6;
        float lmax = -lmin;
        float rmin = lmin;
        float rmax = lmax;

        while (numSamples > 0)
        {
            const int numToDo = (int) jmin (numSamples, (int64) bufferSize);
            read ((int**) tempBuffer, startSampleInFile, numToDo);

            numSamples -= numToDo;

            float bufmin, bufmax;
            findMaxMin ((float*) tempBuffer[0], numToDo, bufmax, bufmin);
            lmin = jmin (lmin, bufmin);
            lmax = jmax (lmax, bufmax);

            if (numChannels > 1)
            {
                findMaxMin ((float*) tempBuffer[1], numToDo, bufmax, bufmin);
                rmin = jmin (rmin, bufmin);
                rmax = jmax (rmax, bufmax);
            }
        }

        if (numChannels <= 1)
        {
            rmax = lmax;
            rmin = lmin;
        }

        lowestLeft = lmin;
        highestLeft = lmax;
        lowestRight = rmin;
        highestRight = rmax;
    }
    else
    {
        int lmax = INT_MIN;
        int lmin = INT_MAX;
        int rmax = INT_MIN;
        int rmin = INT_MAX;

        while (numSamples > 0)
        {
            const int numToDo = (int) jmin (numSamples, (int64) bufferSize);
            read ((int**) tempBuffer, startSampleInFile, numToDo);

            numSamples -= numToDo;

            for (int j = numChannels; --j >= 0;)
            {
                int bufMax = INT_MIN;
                int bufMin = INT_MAX;

                const int* const b = tempBuffer[j];

                for (int i = 0; i < numToDo; ++i)
                {
                    const int samp = b[i];

                    if (samp < bufMin)
                        bufMin = samp;

                    if (samp > bufMax)
                        bufMax = samp;
                }

                if (j == 0)
                {
                    lmax = jmax (lmax, bufMax);
                    lmin = jmin (lmin, bufMin);
                }
                else
                {
                    rmax = jmax (rmax, bufMax);
                    rmin = jmin (rmin, bufMin);
                }
            }
        }

        if (numChannels <= 1)
        {
            rmax = lmax;
            rmin = lmin;
        }

        lowestLeft = lmin / (float)INT_MAX;
        highestLeft = lmax / (float)INT_MAX;
        lowestRight = rmin / (float)INT_MAX;
        highestRight = rmax / (float)INT_MAX;
    }
}

int64 AudioFormatReader::searchForLevel (int64 startSample,
                                         int64 numSamplesToSearch,
                                         const double magnitudeRangeMinimum,
                                         const double magnitudeRangeMaximum,
                                         const int minimumConsecutiveSamples)
{
    if (numSamplesToSearch == 0)
        return -1;

    const int bufferSize = 4096;
    MemoryBlock tempSpace (bufferSize * sizeof (int) * 2 + 64);

    int* tempBuffer[3];
    tempBuffer[0] = (int*) tempSpace.getData();
    tempBuffer[1] = ((int*) tempSpace.getData()) + bufferSize;
    tempBuffer[2] = 0;

    int consecutive = 0;
    int64 firstMatchPos = -1;

    jassert (magnitudeRangeMaximum > magnitudeRangeMinimum);

    const double doubleMin = jlimit (0.0, (double) INT_MAX, magnitudeRangeMinimum * INT_MAX);
    const double doubleMax = jlimit (doubleMin, (double) INT_MAX, magnitudeRangeMaximum * INT_MAX);
    const int intMagnitudeRangeMinimum = roundDoubleToInt (doubleMin);
    const int intMagnitudeRangeMaximum = roundDoubleToInt (doubleMax);

    while (numSamplesToSearch != 0)
    {
        const int numThisTime = (int) jmin (abs64 (numSamplesToSearch), (int64) bufferSize);
        int64 bufferStart = startSample;

        if (numSamplesToSearch < 0)
            bufferStart -= numThisTime;

        if (bufferStart >= (int) lengthInSamples)
            break;

        read ((int**) tempBuffer, bufferStart, numThisTime);

        int num = numThisTime;
        while (--num >= 0)
        {
            if (numSamplesToSearch < 0)
                --startSample;

            bool matches = false;
            const int index = (int) (startSample - bufferStart);

            if (usesFloatingPointData)
            {
                const float sample1 = fabsf (((float*) tempBuffer[0]) [index]);

                if (sample1 >= magnitudeRangeMinimum
                     && sample1 <= magnitudeRangeMaximum)
                {
                    matches = true;
                }
                else if (numChannels > 1)
                {
                    const float sample2 = fabsf (((float*) tempBuffer[1]) [index]);

                    matches = (sample2 >= magnitudeRangeMinimum
                                 && sample2 <= magnitudeRangeMaximum);
                }
            }
            else
            {
                const int sample1 = abs (tempBuffer[0] [index]);

                if (sample1 >= intMagnitudeRangeMinimum
                     && sample1 <= intMagnitudeRangeMaximum)
                {
                    matches = true;
                }
                else if (numChannels > 1)
                {
                    const int sample2 = abs (tempBuffer[1][index]);

                    matches = (sample2 >= intMagnitudeRangeMinimum
                                 && sample2 <= intMagnitudeRangeMaximum);
                }
            }

            if (matches)
            {
                if (firstMatchPos < 0)
                    firstMatchPos = startSample;

                if (++consecutive >= minimumConsecutiveSamples)
                {
                    if (firstMatchPos < 0 || firstMatchPos >= lengthInSamples)
                        return -1;

                    return firstMatchPos;
                }
            }
            else
            {
                consecutive = 0;
                firstMatchPos = -1;
            }

            if (numSamplesToSearch > 0)
                ++startSample;
        }

        if (numSamplesToSearch > 0)
            numSamplesToSearch -= numThisTime;
        else
            numSamplesToSearch += numThisTime;
    }

    return -1;
}

//==============================================================================
AudioFormatWriter::AudioFormatWriter (OutputStream* const out,
                                      const String& formatName_,
                                      const double rate,
                                      const unsigned int numChannels_,
                                      const unsigned int bitsPerSample_)
  : sampleRate (rate),
    numChannels (numChannels_),
    bitsPerSample (bitsPerSample_),
    usesFloatingPointData (false),
    output (out),
    formatName (formatName_)
{
}

AudioFormatWriter::~AudioFormatWriter()
{
    delete output;
}

bool AudioFormatWriter::writeFromAudioReader (AudioFormatReader& reader,
                                              int64 startSample,
                                              int numSamplesToRead)
{
    const int bufferSize = 16384;
    const int maxChans = 128;
    AudioSampleBuffer tempBuffer (reader.numChannels, bufferSize);
    int* buffers [maxChans];

    for (int i = maxChans; --i >= 0;)
        buffers[i] = 0;

    while (numSamplesToRead > 0)
    {
        const int numToDo = jmin (numSamplesToRead, bufferSize);

        for (int i = tempBuffer.getNumChannels(); --i >= 0;)
            buffers[i] = (int*) tempBuffer.getSampleData (i, 0);

        if (! reader.read (buffers, startSample, numToDo))
            return false;

        if (reader.usesFloatingPointData != isFloatingPoint())
        {
            int** bufferChan = buffers;

            while (*bufferChan != 0)
            {
                int* b = *bufferChan++;

                if (isFloatingPoint())
                {
                    // int -> float
                    const double factor = 1.0 / INT_MAX;

                    for (int i = 0; i < numToDo; ++i)
                        ((float*)b)[i] = (float) (factor * b[i]);
                }
                else
                {
                    // float -> int
                    for (int i = 0; i < numToDo; ++i)
                    {
                        const double samp = *(const float*) b;

                        if (samp <= -1.0)
                            *b++ = INT_MIN;
                        else if (samp >= 1.0)
                            *b++ = INT_MAX;
                        else
                            *b++ = roundDoubleToInt (INT_MAX * samp);
                    }
                }
            }
        }

        if (! write ((const int**) buffers, numToDo))
            return false;

        numSamplesToRead -= numToDo;
        startSample += numToDo;
    }

    return true;
}

bool AudioFormatWriter::writeFromAudioSource (AudioSource& source,
                                              int numSamplesToRead,
                                              const int samplesPerBlock)
{
    const int maxChans = 128;
    AudioSampleBuffer tempBuffer (getNumChannels(), samplesPerBlock);
    int* buffers [maxChans];

    while (numSamplesToRead > 0)
    {
        const int numToDo = jmin (numSamplesToRead, samplesPerBlock);

        AudioSourceChannelInfo info;
        info.buffer = &tempBuffer;
        info.startSample = 0;
        info.numSamples = numToDo;
        info.clearActiveBufferRegion();

        source.getNextAudioBlock (info);

        int i;
        for (i = maxChans; --i >= 0;)
            buffers[i] = 0;

        for (i = tempBuffer.getNumChannels(); --i >= 0;)
            buffers[i] = (int*) tempBuffer.getSampleData (i, 0);

        if (! isFloatingPoint())
        {
            int** bufferChan = buffers;

            while (*bufferChan != 0)
            {
                int* b = *bufferChan++;

                // float -> int
                for (int j = numToDo; --j >= 0;)
                {
                    const double samp = *(const float*) b;

                    if (samp <= -1.0)
                        *b++ = INT_MIN;
                    else if (samp >= 1.0)
                        *b++ = INT_MAX;
                    else
                        *b++ = roundDoubleToInt (INT_MAX * samp);
                }
            }
        }

        if (! write ((const int**) buffers, numToDo))
            return false;

        numSamplesToRead -= numToDo;
    }

    return true;
}

//==============================================================================
AudioFormat::AudioFormat (const String& name,
                          const tchar** const extensions)
  : formatName (name),
    fileExtensions (extensions)
{
}

AudioFormat::~AudioFormat()
{
}

//==============================================================================
const String& AudioFormat::getFormatName() const
{
    return formatName;
}

const StringArray& AudioFormat::getFileExtensions() const
{
    return fileExtensions;
}

bool AudioFormat::canHandleFile (const File& f)
{
    for (int i = 0; i < fileExtensions.size(); ++i)
        if (f.hasFileExtension (fileExtensions[i]))
            return true;

    return false;
}

bool AudioFormat::isCompressed()
{
    return false;
}

const StringArray AudioFormat::getQualityOptions()
{
    return StringArray();
}


END_JUCE_NAMESPACE
