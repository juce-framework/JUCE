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

#include "InternalFilters.h"
#include "FilterGraph.h"

//==============================================================================
class InternalFilterBase    : public AudioPluginInstance,
                              public FilterGraphPlayer::PlayerAwareFilter
{
public:
    InternalFilterBase()
        : player (0)
    {
    }

    ~InternalFilterBase()
    {
    }

    void setPlayer (FilterGraphPlayer* p)
    {
        player = p;
        rendererChanged();
    }

    virtual void rendererChanged()                                          {}

    const String getManufacturer() const                                    { return "Raw Material Software"; }
    const String getFormatName() const                                      { return "Internal"; }
    const File getFile() const                                              { return File::nonexistent; }
    int getUID() const                                                      { return getName().hashCode(); }
    int getSamplesLatency() const                                           { return 0; }

    AudioFilterEditor* JUCE_CALLTYPE createEditor()                         { return 0; }
    int JUCE_CALLTYPE getNumParameters()                                    { return 0; }
    const String JUCE_CALLTYPE getParameterName (int)                       { return String::empty; }
    float JUCE_CALLTYPE getParameter (int)                                  { return 0; }
    const String JUCE_CALLTYPE getParameterText (int)                       { return String::empty; }
    void JUCE_CALLTYPE setParameter (int, float)                            {}
    int JUCE_CALLTYPE getNumPrograms()                                      { return 0; }
    int JUCE_CALLTYPE getCurrentProgram()                                   { return 0; }
    void JUCE_CALLTYPE setCurrentProgram (int)                              {}
    const String JUCE_CALLTYPE getProgramName (int)                         { return String::empty; }
    void JUCE_CALLTYPE changeProgramName (int, const String&)               {}
    void JUCE_CALLTYPE getStateInformation (JUCE_NAMESPACE::MemoryBlock&)   {}
    void JUCE_CALLTYPE setStateInformation (const void*, int)               {}

protected:
    FilterGraphPlayer* player;

    AudioIODevice* getAudioDevice() const
    {
        if (player != 0)
        {
            AudioDeviceManager* const dm = player->getAudioDeviceManager();

            jassert (dm != 0);
            if (dm != 0)
                return dm->getCurrentAudioDevice();
        }

        return 0;
    }
};

//==============================================================================
class AudioInputDeviceFilter     : public InternalFilterBase
{
public:
    AudioInputDeviceFilter (const int numChannels)
    {
        numOutputChannels = numChannels;
    }

    ~AudioInputDeviceFilter() {}

    const String getName() const            { return "Audio Input"; }
    const String getVersion() const         { return "1.0"; }
    bool isInstrument() const               { return true; }
    const String getCategory() const        { return "I/O devices"; }
    bool acceptsMidi() const                { return false; }
    bool producesMidi() const               { return false; }

    void rendererChanged()
    {
        AudioIODevice* const dev = getAudioDevice();

        if (dev != 0)
            numOutputChannels = dev->getInputChannelNames().size();
    }

    void JUCE_CALLTYPE prepareToPlay (double /*sampleRate*/, int /*estimatedSamplesPerBlock*/)
    {
        rendererChanged();
    }

    void JUCE_CALLTYPE releaseResources()
    {
    }

    void JUCE_CALLTYPE processBlock (const AudioSampleBuffer& /*input*/,
                                     AudioSampleBuffer& output,
                                     const bool accumulateOutput,
                                     MidiBuffer&)
    {
        int n = 0;

        for (int i = 0; i < player->totalNumInputChannels; ++i)
        {
            if (n >= output.getNumChannels())
                break;

            if (player->inputChannelData [i] != 0)
            {
                if (accumulateOutput)
                    output.addFrom (n, 0, player->inputChannelData [i], output.getNumSamples());
                else
                    output.copyFrom (n, 0, player->inputChannelData [i], output.getNumSamples());

                ++n;
            }
        }
    }

    const String JUCE_CALLTYPE getInputChannelName (const int channelIndex) const
    {
        return String (channelIndex + 1);
    }

    const String JUCE_CALLTYPE getOutputChannelName (const int channelIndex) const
    {
        AudioIODevice* const dev = getAudioDevice();

        if (dev != 0)
            return dev->getInputChannelNames() [channelIndex];

        return String (channelIndex + 1);
    }

    bool JUCE_CALLTYPE isInputChannelStereoPair (int) const
    {
        return true;
    }

    bool JUCE_CALLTYPE isOutputChannelStereoPair (int) const
    {
        return true;
    }
};


//==============================================================================
class AudioOutputDeviceFilter     : public InternalFilterBase
{
public:
    AudioOutputDeviceFilter (const int numChannels)
    {
        numInputChannels = numChannels;
    }

    ~AudioOutputDeviceFilter() {}

    const String getName() const            { return "Audio Output"; }
    const String getVersion() const         { return "1.0"; }
    bool isInstrument() const               { return false; }
    const String getCategory() const        { return "I/O devices"; }
    bool acceptsMidi() const                { return false; }
    bool producesMidi() const               { return false; }

    void rendererChanged()
    {
        AudioIODevice* const dev = getAudioDevice();

        if (dev != 0)
            numInputChannels = dev->getOutputChannelNames().size();
    }

    void JUCE_CALLTYPE prepareToPlay (double /*sampleRate*/, int /*estimatedSamplesPerBlock*/)
    {
        rendererChanged();
    }

    void JUCE_CALLTYPE releaseResources()
    {
    }

    void JUCE_CALLTYPE processBlock (const AudioSampleBuffer& input,
                                     AudioSampleBuffer&,
                                     const bool accumulateOutput,
                                     MidiBuffer&)
    {
        int n = 0;

        for (int i = 0; i < player->totalNumOutputChannels; ++i)
        {
            if (n >= input.getNumChannels())
                break;

            float* dst = player->outputChannelData [i];

            if (dst != 0)
            {
                const float* src = input.getSampleData (n);

                if (accumulateOutput)
                {
                    for (int i = input.getNumSamples(); --i >= 0;)
                        *dst++ += *src++;
                }
                else
                {
                    memcpy (dst, src, sizeof (float) * input.getNumSamples());
                }

                ++n;
            }
        }
    }

    const String JUCE_CALLTYPE getInputChannelName (const int channelIndex) const
    {
        AudioIODevice* const dev = getAudioDevice();

        if (dev != 0)
            return dev->getOutputChannelNames() [channelIndex];

        return String (channelIndex + 1);
    }

    const String JUCE_CALLTYPE getOutputChannelName (const int channelIndex) const
    {
        return String (channelIndex + 1);
    }

    bool JUCE_CALLTYPE isInputChannelStereoPair (int) const
    {
        return true;
    }

    bool JUCE_CALLTYPE isOutputChannelStereoPair (int) const
    {
        return true;
    }
};


//==============================================================================
class MidiInputDeviceFilter     : public InternalFilterBase
{
public:
    MidiInputDeviceFilter()
    {
    }

    ~MidiInputDeviceFilter() {}

    const String getName() const            { return "Midi Input"; }
    const String getVersion() const         { return "1.0"; }
    bool isInstrument() const               { return true; }
    const String getCategory() const        { return "I/O devices"; }
    bool acceptsMidi() const                { return false; }
    bool producesMidi() const               { return true; }

    void rendererChanged()
    {
        if (player != 0)
        {
            AudioDeviceManager* const dm = player->getAudioDeviceManager();

            jassert (dm != 0);
        }
    }

    void JUCE_CALLTYPE prepareToPlay (double /*sampleRate*/, int /*estimatedSamplesPerBlock*/)
    {
        rendererChanged();
    }

    void JUCE_CALLTYPE releaseResources()
    {
    }

    void JUCE_CALLTYPE processBlock (const AudioSampleBuffer& input,
                                     AudioSampleBuffer&,
                                     const bool accumulateOutput,
                                     MidiBuffer& midiBuffer)
    {
        if (! accumulateOutput)
            midiBuffer.clear();

        midiBuffer.addEvents (player->incomingMidi, 0, input.getNumSamples(), 0);
    }

    const String JUCE_CALLTYPE getInputChannelName (const int channelIndex) const
    {
        return String (channelIndex + 1);
    }

    const String JUCE_CALLTYPE getOutputChannelName (const int channelIndex) const
    {
        AudioIODevice* const dev = getAudioDevice();

        if (dev != 0)
            return dev->getInputChannelNames() [channelIndex];

        return String (channelIndex + 1);
    }

    bool JUCE_CALLTYPE isInputChannelStereoPair (int) const
    {
        return true;
    }

    bool JUCE_CALLTYPE isOutputChannelStereoPair (int) const
    {
        return true;
    }
};


//==============================================================================
InternalPluginFormat::InternalPluginFormat()
{
    {
        AudioInputDeviceFilter f (2);
        audioInDesc.fillInFromInstance (f);
    }

    {
        AudioOutputDeviceFilter f (2);
        audioOutDesc.fillInFromInstance (f);
    }

    {
        MidiInputDeviceFilter f;
        midiInDesc.fillInFromInstance (f);
    }
}

AudioPluginInstance* InternalPluginFormat::createInstanceFromDescription (const PluginDescription& desc)
{
    if (desc.name == audioOutDesc.name)
    {
        return new AudioOutputDeviceFilter (desc.numInputChannels);
    }
    else if (desc.name == audioInDesc.name)
    {
        return new AudioInputDeviceFilter (desc.numOutputChannels);
    }
    else if (desc.name == midiInDesc.name)
    {
        return new MidiInputDeviceFilter();
    }

    return 0;
}

const PluginDescription* InternalPluginFormat::getDescriptionFor (const InternalFilterType type)
{
    switch (type)
    {
    case audioInputFilter:
        return &audioInDesc;
    case audioOutputFilter:
        return &audioOutDesc;
    case midiInputFilter:
        return &midiInDesc;
    }

    return 0;
}

void InternalPluginFormat::getAllTypes (OwnedArray <PluginDescription>& results)
{
    for (int i = 0; i < (int) endOfFilterTypes; ++i)
        results.add (new PluginDescription (*getDescriptionFor ((InternalFilterType) i)));
}
