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

#include "juce_AudioFilterBase.h"


//==============================================================================
AudioFilterBase::AudioFilterBase()
    : suspended (false),
      numInputChannels (0),
      numOutputChannels (0),
      callbacks (0),
      activeEditor (0)
{
    sampleRate = 0.0;
    blockSize = 0;
}

AudioFilterBase::~AudioFilterBase()
{
    // ooh, nasty - the editor should have been deleted before the filter
    // that it refers to is deleted..
    jassert (activeEditor == 0);
}

void AudioFilterBase::initialiseInternal (FilterNativeCallbacks* const callbacks_)
{
    callbacks = callbacks_;
}

void AudioFilterBase::setParameterNotifyingHost (const int parameterIndex,
                                                 const float newValue)
{
    jassert (parameterIndex >= 0 && parameterIndex < getNumParameters());

    if (callbacks != 0)
        callbacks->informHostOfParameterChange (parameterIndex, newValue);
    else
        setParameter (parameterIndex, newValue);
}

void AudioFilterBase::suspendProcessing (const bool shouldBeSuspended)
{
    const ScopedLock sl (callbackLock);
    suspended = shouldBeSuspended;
}

//==============================================================================
bool AudioFilterBase::getCurrentPositionInfo (CurrentPositionInfo& info)
{
    return callbacks != 0
            && callbacks->getCurrentPositionInfo (info);
}

const String AudioFilterBase::getInputChannelName (const int channelIndex) const
{
    String s (inputNames [channelIndex]);
    return s.isNotEmpty() ? s : String (channelIndex + 1);
}

const String AudioFilterBase::getOutputChannelName (const int channelIndex) const
{
    String s (outputNames [channelIndex]);
    return s.isNotEmpty() ? s : String (channelIndex + 1);
}

//==============================================================================
void AudioFilterBase::editorBeingDeleted (AudioFilterEditor* const editor)
{
    const ScopedLock sl (callbackLock);

    jassert (activeEditor == editor);

    if (activeEditor == editor)
        activeEditor = 0;
}

AudioFilterEditor* AudioFilterBase::createEditorIfNeeded()
{
    if (activeEditor != 0)
        return activeEditor;

    AudioFilterEditor* const ed = createEditor();

    if (ed != 0)
    {
        // you must give your editor comp a size before returning it..
        jassert (ed->getWidth() > 0 && ed->getHeight() > 0);

        const ScopedLock sl (callbackLock);
        activeEditor = ed;
    }

    return ed;
}

//==============================================================================
void AudioFilterBase::getCurrentProgramStateInformation (JUCE_NAMESPACE::MemoryBlock& destData)
{
    getStateInformation (destData);
}

void AudioFilterBase::setCurrentProgramStateInformation (const void* data, int sizeInBytes)
{
    setStateInformation (data, sizeInBytes);
}

//==============================================================================
// magic number to identify memory blocks that we've stored as XML
const uint32 magicXmlNumber = 0x21324356;

void AudioFilterBase::copyXmlToBinary (const XmlElement& xml,
                                       JUCE_NAMESPACE::MemoryBlock& destData)
{
    const String xmlString (xml.createDocument (String::empty, true, false));
    const int stringLength = xmlString.length();

    destData.setSize (stringLength + 10);

    char* const d = (char*) destData.getData();
    *(uint32*) d = swapIfBigEndian ((const uint32) magicXmlNumber);
    *(uint32*) (d + 4) = swapIfBigEndian ((const uint32) stringLength);

    xmlString.copyToBuffer (d + 8, stringLength);
}

XmlElement* AudioFilterBase::getXmlFromBinary (const void* data,
                                               const int sizeInBytes)
{
    if (sizeInBytes > 8
         && littleEndianInt ((const char*) data) == magicXmlNumber)
    {
        const uint32 stringLength = littleEndianInt (((const char*) data) + 4);

        if (stringLength > 0)
        {
            XmlDocument doc (String (((const char*) data) + 8,
                                     jmin ((sizeInBytes - 8), stringLength)));

            return doc.getDocumentElement();
        }
    }

    return 0;
}
