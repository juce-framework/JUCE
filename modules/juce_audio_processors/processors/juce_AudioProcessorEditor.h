/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_AUDIOPROCESSOREDITOR_JUCEHEADER__
#define __JUCE_AUDIOPROCESSOREDITOR_JUCEHEADER__

class AudioProcessor;


//==============================================================================
/**
    Base class for the component that acts as the GUI for an AudioProcessor.

    Derive your editor component from this class, and create an instance of it
    by overriding the AudioProcessor::createEditor() method.

    @see AudioProcessor, GenericAudioProcessorEditor
*/
class JUCE_API  AudioProcessorEditor  : public Component
{
protected:
    //==============================================================================
    /** Creates an editor for the specified processor.
    */
    AudioProcessorEditor (AudioProcessor* owner);

public:
    /** Destructor. */
    ~AudioProcessorEditor();


    //==============================================================================
    /** Returns a pointer to the processor that this editor represents. */
    AudioProcessor* getAudioProcessor() const noexcept        { return owner; }


private:
    //==============================================================================
    AudioProcessor* const owner;

    JUCE_DECLARE_NON_COPYABLE (AudioProcessorEditor)
};


#endif   // __JUCE_AUDIOPROCESSOREDITOR_JUCEHEADER__
