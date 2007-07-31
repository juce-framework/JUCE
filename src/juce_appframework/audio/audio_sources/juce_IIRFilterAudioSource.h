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

#ifndef __JUCE_IIRFILTERAUDIOSOURCE_JUCEHEADER__
#define __JUCE_IIRFILTERAUDIOSOURCE_JUCEHEADER__

#include "juce_AudioSource.h"
#include "../dsp/juce_IIRFilter.h"
#include "../../../juce_core/containers/juce_OwnedArray.h"


//==============================================================================
/**
    An AudioSource that performs an IIR filter on another source.
*/
class JUCE_API  IIRFilterAudioSource  : public AudioSource
{
public:
    //==============================================================================
    /** Creates a IIRFilterAudioSource for a given input source.

        @param inputSource              the input source to read from
        @param deleteInputWhenDeleted   if true, the input source will be deleted when
                                        this object is deleted
    */
    IIRFilterAudioSource (AudioSource* const inputSource,
                          const bool deleteInputWhenDeleted);

    /** Destructor. */
    ~IIRFilterAudioSource();

    //==============================================================================
    /** Changes the filter to use the same parameters as the one being passed in.
    */
    void setFilterParameters (const IIRFilter& newSettings);

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
    void releaseResources();
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    AudioSource* const input;
    const bool deleteInputWhenDeleted;
    OwnedArray <IIRFilter> iirFilters;

    IIRFilterAudioSource (const IIRFilterAudioSource&);
    const IIRFilterAudioSource& operator= (const IIRFilterAudioSource&);
};


#endif   // __JUCE_IIRFILTERAUDIOSOURCE_JUCEHEADER__
