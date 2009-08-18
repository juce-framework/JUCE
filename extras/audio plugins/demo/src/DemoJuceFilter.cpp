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

#include "includes.h"
#include "DemoJuceFilter.h"
#include "DemoEditorComponent.h"


//==============================================================================
/**
    This function must be implemented to create a new instance of your
    plugin object.
*/
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DemoJuceFilter();
}

//==============================================================================
DemoJuceFilter::DemoJuceFilter()
{
    gain = 1.0f;
    lastUIWidth = 400;
    lastUIHeight = 140;

    zeromem (&lastPosInfo, sizeof (lastPosInfo));
    lastPosInfo.timeSigNumerator = 4;
    lastPosInfo.timeSigDenominator = 4;
    lastPosInfo.bpm = 120;
}

DemoJuceFilter::~DemoJuceFilter()
{
}

//==============================================================================
const String DemoJuceFilter::getName() const
{
    return "Juce Demo Filter";
}

int DemoJuceFilter::getNumParameters()
{
    return 1;
}

float DemoJuceFilter::getParameter (int index)
{
    return (index == 0) ? gain
                        : 0.0f;
}

void DemoJuceFilter::setParameter (int index, float newValue)
{
    if (index == 0)
    {
        if (gain != newValue)
        {
            gain = newValue;

            // if this is changing the gain, broadcast a change message which
            // our editor will pick up.
            sendChangeMessage (this);
        }
    }
}

const String DemoJuceFilter::getParameterName (int index)
{
    if (index == 0)
        return T("gain");

    return String::empty;
}

const String DemoJuceFilter::getParameterText (int index)
{
    if (index == 0)
        return String (gain, 2);

    return String::empty;
}

const String DemoJuceFilter::getInputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

const String DemoJuceFilter::getOutputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

bool DemoJuceFilter::isInputChannelStereoPair (int index) const
{
    return false;
}

bool DemoJuceFilter::isOutputChannelStereoPair (int index) const
{
    return false;
}

bool DemoJuceFilter::acceptsMidi() const
{
    return true;
}

bool DemoJuceFilter::producesMidi() const
{
    return true;
}

//==============================================================================
void DemoJuceFilter::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // do your pre-playback setup stuff here..
    keyboardState.reset();
}

void DemoJuceFilter::releaseResources()
{
    // when playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void DemoJuceFilter::processBlock (AudioSampleBuffer& buffer,
                                   MidiBuffer& midiMessages)
{
    // for each of our input channels, we'll attenuate its level by the
    // amount that our volume parameter is set to.
    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        buffer.applyGain (channel, 0, buffer.getNumSamples(), gain);
    }

    // in case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }

    // if any midi messages come in, use them to update the keyboard state object. This
    // object sends notification to the UI component about key up/down changes
    keyboardState.processNextMidiBuffer (midiMessages,
                                         0, buffer.getNumSamples(),
                                         true);

    // have a go at getting the current time from the host, and if it's changed, tell
    // our UI to update itself.
    AudioPlayHead::CurrentPositionInfo pos;

    if (getPlayHead() != 0 && getPlayHead()->getCurrentPosition (pos))
    {
        if (memcmp (&pos, &lastPosInfo, sizeof (pos)) != 0)
        {
            lastPosInfo = pos;
            sendChangeMessage (this);
        }
    }
    else
    {
        zeromem (&lastPosInfo, sizeof (lastPosInfo));
        lastPosInfo.timeSigNumerator = 4;
        lastPosInfo.timeSigDenominator = 4;
        lastPosInfo.bpm = 120;
    }
}

//==============================================================================
AudioProcessorEditor* DemoJuceFilter::createEditor()
{
    return new DemoEditorComponent (this);
}

//==============================================================================
void DemoJuceFilter::getStateInformation (MemoryBlock& destData)
{
    // you can store your parameters as binary data if you want to or if you've got
    // a load of binary to put in there, but if you're not doing anything too heavy,
    // XML is a much cleaner way of doing it - here's an example of how to store your
    // params as XML..

    // create an outer XML element..
    XmlElement xmlState (T("MYPLUGINSETTINGS"));

    // add some attributes to it..
    xmlState.setAttribute (T("pluginVersion"), 1);
    xmlState.setAttribute (T("gainLevel"), gain);
    xmlState.setAttribute (T("uiWidth"), lastUIWidth);
    xmlState.setAttribute (T("uiHeight"), lastUIHeight);

    // you could also add as many child elements as you need to here..


    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xmlState, destData);
}

void DemoJuceFilter::setStateInformation (const void* data, int sizeInBytes)
{
    // use this helper function to get the XML from this binary blob..
    XmlElement* const xmlState = getXmlFromBinary (data, sizeInBytes);

    if (xmlState != 0)
    {
        // check that it's the right type of xml..
        if (xmlState->hasTagName (T("MYPLUGINSETTINGS")))
        {
            // ok, now pull out our parameters..
            gain = (float) xmlState->getDoubleAttribute (T("gainLevel"), gain);

            lastUIWidth = xmlState->getIntAttribute (T("uiWidth"), lastUIWidth);
            lastUIHeight = xmlState->getIntAttribute (T("uiHeight"), lastUIHeight);

            sendChangeMessage (this);
        }

        delete xmlState;
    }
}
