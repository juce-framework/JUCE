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

#define JUCE_PLUGIN_HOST 1

//==============================================================================
// (Just a quick way of getting these files into the project)
#include "../../../audio plugins/wrapper/juce_AudioFilterBase.cpp"
#include "../../../audio plugins/wrapper/juce_AudioFilterEditor.cpp"

#include "juce_AudioPluginInstance.h"


//==============================================================================
AudioPluginInstance::AudioPluginInstance()
{
    internalAsyncUpdater = new InternalAsyncUpdater (*this);

    initialiseInternal (this);
}

AudioPluginInstance::~AudioPluginInstance()
{
    delete internalAsyncUpdater;
}

void AudioPluginInstance::addListener (AudioPluginParameterListener* const newListener) throw()
{
    listeners.addIfNotAlreadyThere (newListener);
}

void AudioPluginInstance::removeListener (AudioPluginParameterListener* const listenerToRemove) throw()
{
    listeners.removeValue (listenerToRemove);
}

void AudioPluginInstance::internalAsyncCallback()
{
    changedParamLock.enter();
    Array <int> changed;
    changed.swapWithArray (changedParams);
    changedParamLock.exit();

    for (int j = 0; j < changed.size(); ++j)
    {
        const int paramIndex = changed.getUnchecked (j);

        for (int i = listeners.size(); --i >= 0;)
        {
            AudioPluginParameterListener* const l = (AudioPluginParameterListener*) listeners.getUnchecked(i);

            if (paramIndex >= 0)
                l->audioPluginParameterChanged (this, paramIndex);
            else
                l->audioPluginChanged (this);

            i = jmin (i, listeners.size());
        }
    }
}

//==============================================================================
bool JUCE_CALLTYPE AudioPluginInstance::getCurrentPositionInfo (AudioFilterBase::CurrentPositionInfo& info)
{
    info.bpm = 120.0;
    info.timeSigNumerator = 4;
    info.timeSigDenominator = 4;

    info.timeInSeconds = 0;

    /** For timecode, the position of the start of the edit, in seconds from 00:00:00:00. */
    info.editOriginTime = 0;

    /** The current play position in pulses-per-quarter-note.
        This is the number of quarter notes since the edit start.
    */
    info.ppqPosition = 0;

    /** The position of the start of the last bar, in pulses-per-quarter-note.

        This is the number of quarter notes from the start of the edit to the
        start of the current bar.

        Note - this value may be unavailable on some hosts, e.g. Pro-Tools. If
        it's not available, the value will be 0.
    */
    info.ppqPositionOfLastBarStart = 0;

    info.frameRate = AudioFilterBase::CurrentPositionInfo::fpsUnknown;

    info.isPlaying = false;
    info.isRecording = false;

    return true;
}

void JUCE_CALLTYPE AudioPluginInstance::informHostOfParameterChange (int index, float /*newValue*/)
{
    queueChangeMessage (index);
}

void JUCE_CALLTYPE AudioPluginInstance::updateHostDisplay()
{
    queueChangeMessage (-1);
}

void AudioPluginInstance::queueChangeMessage (const int index) throw()
{
    const ScopedLock sl (changedParamLock);
    changedParams.addIfNotAlreadyThere (index);

    if (! internalAsyncUpdater->isTimerRunning())
        internalAsyncUpdater->startTimer (1);
}

//==============================================================================
AudioPluginInstance::InternalAsyncUpdater::InternalAsyncUpdater (AudioPluginInstance& owner_)
    : owner (owner_)
{
}

void AudioPluginInstance::InternalAsyncUpdater::timerCallback()
{
    stopTimer();
    owner.internalAsyncCallback();
}

