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

#ifndef JUCE_GENERICAUDIOPROCESSOREDITOR_H_INCLUDED
#define JUCE_GENERICAUDIOPROCESSOREDITOR_H_INCLUDED


//==============================================================================
/**
    A type of UI component that displays the parameters of an AudioProcessor as
    a simple list of sliders.

    This can be used for showing an editor for a processor that doesn't supply
    its own custom editor.

    @see AudioProcessor
*/
class JUCE_API  GenericAudioProcessorEditor      : public AudioProcessorEditor
{
public:
    //==============================================================================
    GenericAudioProcessorEditor (AudioProcessor* owner);
    ~GenericAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    PropertyPanel panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericAudioProcessorEditor)
};


#endif   // JUCE_GENERICAUDIOPROCESSOREDITOR_H_INCLUDED
