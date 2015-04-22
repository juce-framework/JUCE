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
    /** Creates an editor for the specified processor. */
    AudioProcessorEditor (AudioProcessor&) noexcept;

    /** Creates an editor for the specified processor. */
    AudioProcessorEditor (AudioProcessor*) noexcept;

public:
    /** Destructor. */
    ~AudioProcessorEditor();


    //==============================================================================
    /** The AudioProcessor that this editor represents. */
    AudioProcessor& processor;

    /** Returns a pointer to the processor that this editor represents.
        This method is here to support legacy code, but it's easier to just use the
        AudioProcessorEditor::processor member variable directly to get this object.
    */
    AudioProcessor* getAudioProcessor() const noexcept        { return &processor; }

    //==============================================================================
    /** Used by the setParameterHighlighting() method. */
    struct ParameterControlHighlightInfo
    {
        int parameterIndex;
        bool isHighlighted;
        Colour suggestedColour;
    };

    /** Some types of plugin can call this to suggest that the control for a particular
        parameter should be highlighted.
        Currently only AAX plugins will call this, and implementing it is optional.
    */
    virtual void setControlHighlight (ParameterControlHighlightInfo);

    /** Called by certain plug-in wrappers to find out whether a component is used
        to control a parameter.

        If the given component represents a particular plugin parameter, then this
        method should return the index of that parameter. If not, it should return -1.
        Currently only AAX plugins will call this, and implementing it is optional.
    */
    virtual int getControlParameterIndex (Component&);

private:
    JUCE_DECLARE_NON_COPYABLE (AudioProcessorEditor)
};


#endif   // JUCE_AUDIOPROCESSOREDITOR_H_INCLUDED
