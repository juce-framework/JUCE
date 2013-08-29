/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_AUDIOPROCESSOREDITOR_H_INCLUDED
#define JUCE_AUDIOPROCESSOREDITOR_H_INCLUDED


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


#endif   // JUCE_AUDIOPROCESSOREDITOR_H_INCLUDED
