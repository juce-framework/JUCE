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

#if JUCE_WEB_BROWSER || DOXYGEN

/** This is a helper class for implementing AudioProcessorEditor::getControlParameterIndex with GUIs
    using a WebBrowserComponent.

    Create an instance of this class and attach it to the WebBrowserComponent by using
    WebBrowserComponent::Options::withOptionsFrom.

    In your frontend code you can use the ControlParameterIndexUpdater class, that emits
    controlParameterIndexChanged events based on the mouse movement, and control parameter index
    annotations attached to DOM elements.

    @tags{GUI}
*/
class JUCE_API  WebControlParameterIndexReceiver : public OptionsBuilder<WebBrowserComponent::Options>
{
public:
    /*  Returns the control parameter index last reported by the WebBrowserComponent GUI to be
        active.
    */
    int getControlParameterIndex() const { return controlParameterIndex; }

    //==============================================================================
    WebBrowserComponent::Options buildOptions (const WebBrowserComponent::Options& initialOptions) override
    {
        return initialOptions.withEventListener ("__juce__controlParameterIndexChanged",
                                                 [this] (auto newIndex)
                                                 {
                                                     controlParameterIndex = (int) newIndex;
                                                 });
    }

private:
    int controlParameterIndex = -1;
};

#endif

}
