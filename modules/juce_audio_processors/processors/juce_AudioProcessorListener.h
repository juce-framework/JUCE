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
    Base class for listeners that want to know about changes to an AudioProcessor.

    Use AudioProcessor::addListener() to register your listener with an AudioProcessor.

    @see AudioProcessor

    @tags{Audio}
*/
class JUCE_API  AudioProcessorListener
{
public:
    //==============================================================================
    /** Destructor. */
    virtual ~AudioProcessorListener() = default;

    //==============================================================================
    /** Receives a callback when a parameter is changed.

        IMPORTANT NOTE: This will be called synchronously when a parameter changes, and
        many audio processors will change their parameter during their audio callback.
        This means that not only has your handler code got to be completely thread-safe,
        but it's also got to be VERY fast, and avoid blocking. If you need to handle
        this event on your message thread, use this callback to trigger an AsyncUpdater
        or ChangeBroadcaster which you can respond to on the message thread.
    */
    virtual void audioProcessorParameterChanged (AudioProcessor* processor,
                                                 int parameterIndex,
                                                 float newValue) = 0;

    /** Provides details about aspects of an AudioProcessor which have changed.
    */
    struct JUCE_API  ChangeDetails
    {
        /** @see withLatencyChanged */
        bool latencyChanged           = false;
        /** @see withParameterInfoChanged */
        bool parameterInfoChanged     = false;
        /** @see withProgramChanged */
        bool programChanged           = false;
        /** @see withNonParameterStateChanged */
        bool nonParameterStateChanged = false;

        /** Indicates that the AudioProcessor's latency has changed.

            Most of the time, you won't need to use this function directly.
            AudioProcessor::setLatencySamples() will automatically call
            AudioProcessor::updateHostDisplay(), indicating that the latency has changed.

            @see latencyChanged
        */
        [[nodiscard]] ChangeDetails withLatencyChanged           (bool b) const noexcept { return with (&ChangeDetails::latencyChanged,           b); }

        /** Indicates that some attributes of the AudioProcessor's parameters have changed.

            When this flag is set, the host should rescan the AudioProcessor's parameters, and
            update its controls to match. This is often used to update the names of a plugin's
            parameters in the host.

            @see parameterInfoChanged
        */
        [[nodiscard]] ChangeDetails withParameterInfoChanged     (bool b) const noexcept { return with (&ChangeDetails::parameterInfoChanged,     b); }

        /** Indicates that the loaded program has changed.

            When this flag is set, the host should call AudioProcessor::getCurrentProgram() and
            update any preset list views to display the program that is currently in use.

            @see programChanged
        */
        [[nodiscard]] ChangeDetails withProgramChanged           (bool b) const noexcept { return with (&ChangeDetails::programChanged,           b); }

        /** Indicates that the plugin state has changed (but not its parameters!).

            An AudioProcessor can call updateHostDisplay with this flag set to notify the host that
            its state has changed in a way that requires re-saving.

            If a host receives a call to audioProcessorChanged with this flag set, it should offer
            to save the plugin state before taking any actions that might irrevocably destroy the
            current plugin state, such as closing the project.

            @see nonParameterStateChanged
        */
        [[nodiscard]] ChangeDetails withNonParameterStateChanged (bool b) const noexcept { return with (&ChangeDetails::nonParameterStateChanged, b); }

        /** Returns the default set of flags that will be used when
            AudioProcessor::updateHostDisplay() is called with no arguments.
        */
        static ChangeDetails getDefaultFlags()
        {
            return ChangeDetails{}.withLatencyChanged (true)
                                  .withParameterInfoChanged (true)
                                  .withProgramChanged (true);
        }

        [[deprecated ("The naming of this function is misleading. Use getDefaultFlags instead.")]]
        static ChangeDetails getAllChanged()
        {
            return getDefaultFlags();
        }

    private:
        template <typename Member, typename Value>
        ChangeDetails with (Member&& member, Value&& value) const noexcept
        {
            auto copy = *this;
            copy.*member = std::forward<Value> (value);
            return copy;
        }
    };

    /** Called to indicate that something else in the plugin has changed, like its
        program, number of parameters, etc.

        IMPORTANT NOTE: This will be called synchronously, and many audio processors will
        call it during their audio callback. This means that not only has your handler code
        got to be completely thread-safe, but it's also got to be VERY fast, and avoid
        blocking. If you need to handle this event on your message thread, use this callback
        to trigger an AsyncUpdater or ChangeBroadcaster which you can respond to later on the
        message thread.
    */
    virtual void audioProcessorChanged (AudioProcessor* processor, const ChangeDetails& details) = 0;

    /** Indicates that a parameter change gesture has started.

        E.g. if the user is dragging a slider, this would be called when they first
        press the mouse button, and audioProcessorParameterChangeGestureEnd would be
        called when they release it.

        IMPORTANT NOTE: This will be called synchronously, and many audio processors will
        call it during their audio callback. This means that not only has your handler code
        got to be completely thread-safe, but it's also got to be VERY fast, and avoid
        blocking. If you need to handle this event on your message thread, use this callback
        to trigger an AsyncUpdater or ChangeBroadcaster which you can respond to later on the
        message thread.

        @see audioProcessorParameterChangeGestureEnd
    */
    virtual void audioProcessorParameterChangeGestureBegin (AudioProcessor* processor,
                                                            int parameterIndex);

    /** Indicates that a parameter change gesture has finished.

        E.g. if the user is dragging a slider, this would be called when they release
        the mouse button.

        IMPORTANT NOTE: This will be called synchronously, and many audio processors will
        call it during their audio callback. This means that not only has your handler code
        got to be completely thread-safe, but it's also got to be VERY fast, and avoid
        blocking. If you need to handle this event on your message thread, use this callback
        to trigger an AsyncUpdater or ChangeBroadcaster which you can respond to later on the
        message thread.

        @see audioProcessorParameterChangeGestureBegin
    */
    virtual void audioProcessorParameterChangeGestureEnd (AudioProcessor* processor,
                                                          int parameterIndex);
};

} // namespace juce
