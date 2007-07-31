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

#ifndef __JUCE_AUDIOSUBSECTIONREADER_JUCEHEADER__
#define __JUCE_AUDIOSUBSECTIONREADER_JUCEHEADER__

#include "juce_AudioFormatReader.h"


//==============================================================================
/**
    This class is used to wrap an AudioFormatReader and only read from a
    subsection of the file.

    So if you have a reader which can read a 1000 sample file, you could wrap it
    in one of these to only access, e.g. samples 100 to 200, and any samples
    outside that will come back as 0. Accessing sample 0 from this reader will
    actually read the first sample from the other's subsection, which might
    be at a non-zero position.

    @see AudioFormatReader
*/
class JUCE_API  AudioSubsectionReader  : public AudioFormatReader
{
public:
    //==============================================================================
    /** Creates a AudioSubsectionReader for a given data source.

        @param sourceReader             the source reader from which we'll be taking data
        @param subsectionStartSample    the sample within the source reader which will be
                                        mapped onto sample 0 for this reader.
        @param subsectionLength         the number of samples from the source that will
                                        make up the subsection. If this reader is asked for
                                        any samples beyond this region, it will return zero.
        @param deleteSourceWhenDeleted  if true, the sourceReader object will be deleted when
                                        this object is deleted.
    */
    AudioSubsectionReader (AudioFormatReader* const sourceReader,
                           const int64 subsectionStartSample,
                           const int64 subsectionLength,
                           const bool deleteSourceWhenDeleted);

    /** Destructor. */
    ~AudioSubsectionReader();


    //==============================================================================
    bool read (int** destSamples,
               int64 startSample,
               int numSamples);

    void readMaxLevels (int64 startSample,
                        int64 numSamples,
                        float& lowestLeft,
                        float& highestLeft,
                        float& lowestRight,
                        float& highestRight);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioFormatReader* const source;
    int64 startSample, length;
    const bool deleteSourceWhenDeleted;

    AudioSubsectionReader (const AudioSubsectionReader&);
    const AudioSubsectionReader& operator= (const AudioSubsectionReader&);
};

#endif   // __JUCE_AUDIOSUBSECTIONREADER_JUCEHEADER__
