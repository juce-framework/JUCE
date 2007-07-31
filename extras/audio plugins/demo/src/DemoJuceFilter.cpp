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

#include "DemoJuceFilter.h"
#include "DemoEditorComponent.h"


//==============================================================================
/**
    This function must be implemented to create the actual plugin object that
    you want to use.
*/
AudioFilterBase* JUCE_CALLTYPE createPluginFilter()
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

void DemoJuceFilter::processBlock (const AudioSampleBuffer& input,
                                   AudioSampleBuffer& output,
                                   const bool accumulateOutput,
                                   MidiBuffer& midiMessages)
{
    if (accumulateOutput)
    {
        // if we're accumulating, we should add our results to the existing contents of the
        // output buffer..

        if (input.getNumChannels() > 0)
        {
            for (int channel = 0; channel < output.getNumChannels(); ++channel)
            {
                // for each output channel, add the contents of the corresponding
                // input channel (or if there are more outputs than inputs, just
                // keep using the last input channel)
                output.addFrom (channel,
                                0,
                                input,
                                jmin (channel, input.getNumChannels() - 1),
                                0,
                                input.getNumSamples(),
                                gain);
            }
        }
    }
    else
    {
        // if we're not accumulating, the output buffer's contents are undefined
        // (don't assume they're zero!) and we should overwrite it.

        if (input.getNumChannels() > 0)
        {
            for (int channel = 0; channel < output.getNumChannels(); ++channel)
            {
                // for each output channel, copy the contents of the corresponding
                // input channel (or if there are more outputs than inputs, just
                // keep using the last input channel)
                output.copyFrom (channel,
                                 0,
                                 input,
                                 jmin (channel, input.getNumChannels() - 1),
                                 0,
                                 input.getNumSamples());
            }

            output.applyGain (0, output.getNumSamples(), gain);
        }
        else
        {
            // when not accumulating, you always have to put something into
            // the output buffer, even if in this case we have no inputs to copy.
            output.clear();
        }
    }

    // if any midi messages come in, use them to update the keyboard state object. This
    // object sends notification to the UI component about key up/down changes
    keyboardState.processNextMidiBuffer (midiMessages,
                                         0, output.getNumSamples(),
                                         true);

    // have a go at getting the current time from the host, and if it's changed, tell
    // our UI to update itself.
    CurrentPositionInfo pos;

    if (getCurrentPositionInfo (pos))
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
AudioFilterEditor* DemoJuceFilter::createEditor()
{
    return new DemoEditorComponent (this);
}

//==============================================================================
void DemoJuceFilter::getStateInformation (JUCE_NAMESPACE::MemoryBlock& destData)
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
