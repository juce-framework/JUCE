/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A type of UI component that displays the parameters of an AudioProcessor as
    a simple list of sliders, combo boxes and switches.

    This can be used for showing an editor for a processor that doesn't supply
    its own custom editor.

    @see AudioProcessor

    @tags{Audio}
*/
class JUCE_API  GenericAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    //==============================================================================
    GenericAudioProcessorEditor (AudioProcessor&);
    ~GenericAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    /** @cond */
    [[deprecated ("This constructor has been changed to take a reference instead of a pointer.")]]
    GenericAudioProcessorEditor (AudioProcessor* p)  : GenericAudioProcessorEditor (*p) {}
    /** @endcond */

private:
    //==============================================================================
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericAudioProcessorEditor)
};

} // namespace juce
