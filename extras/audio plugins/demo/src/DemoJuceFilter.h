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

#ifndef DEMOJUCEPLUGINFILTER_H
#define DEMOJUCEPLUGINFILTER_H

#include "../../wrapper/juce_AudioFilterBase.h"


//==============================================================================
/**
    A simple plugin filter that just applies a gain change to the audio
    passing through it.

*/
class DemoJuceFilter  : public AudioFilterBase,
                        public ChangeBroadcaster
{
public:
    //==============================================================================
    DemoJuceFilter();
    ~DemoJuceFilter();

    //==============================================================================
    void JUCE_CALLTYPE prepareToPlay (double sampleRate, int samplesPerBlock);
    void JUCE_CALLTYPE releaseResources();

	void JUCE_CALLTYPE processBlock (const AudioSampleBuffer& input,
                                     AudioSampleBuffer& output,
                                     const bool accumulateOutput,
                                     MidiBuffer& midiMessages);

    //==============================================================================
    AudioFilterEditor* JUCE_CALLTYPE createEditor();

    //==============================================================================
    int JUCE_CALLTYPE getNumParameters();

    float JUCE_CALLTYPE getParameter (int index);
    void JUCE_CALLTYPE setParameter (int index, float newValue);

    const String JUCE_CALLTYPE getParameterName (int index);
    const String JUCE_CALLTYPE getParameterText (int index);

    //==============================================================================
    int JUCE_CALLTYPE getNumPrograms()                                        { return 0; }
    int JUCE_CALLTYPE getCurrentProgram()                                     { return 0; }
    void JUCE_CALLTYPE setCurrentProgram (int index)                          { }
    const String JUCE_CALLTYPE getProgramName (int index)                     { return String::empty; }
    void JUCE_CALLTYPE changeProgramName (int index, const String& newName)   { }

    //==============================================================================
    void JUCE_CALLTYPE getStateInformation (JUCE_NAMESPACE::MemoryBlock& destData);
    void JUCE_CALLTYPE setStateInformation (const void* data, int sizeInBytes);

    //==============================================================================
    // These properties are public so that our editor component can access them
    //  - a bit of a hacky way to do it, but it's only a demo!

    // this is kept up to date with the midi messages that arrive, and the UI component
    // registers with it so it can represent the incoming messages
    MidiKeyboardState keyboardState;

    // this keeps a copy of the last set of time info that was acquired during an audio
    // callback - the UI component will read this and display it.
    AudioFilterBase::CurrentPositionInfo lastPosInfo;

    // these are used to persist the UI's size - the values are stored along with the
    // filter's other parameters, and the UI component will update them when it gets
    // resized.
    int lastUIWidth, lastUIHeight;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    // this is our gain - the UI and the host can access this by getting/setting
    // parameter 0.
    float gain;
};


#endif
