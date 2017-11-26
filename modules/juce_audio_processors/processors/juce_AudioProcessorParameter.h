/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/** An abstract base class for parameter objects that can be added to an
    AudioProcessor.

    @see AudioProcessor::addParameter
*/
class JUCE_API  AudioProcessorParameter
{
public:
    AudioProcessorParameter() noexcept;

    /** Destructor. */
    virtual ~AudioProcessorParameter();

    /** Called by the host to find out the value of this parameter.

        Hosts will expect the value returned to be between 0 and 1.0.

        This could be called quite frequently, so try to make your code efficient.
        It's also likely to be called by non-UI threads, so the code in here should
        be thread-aware.
    */
    virtual float getValue() const = 0;

    /** The host will call this method to change the value of a parameter.

        The host may call this at any time, including during the audio processing
        callback, so your implementation has to process this very efficiently and
        avoid any kind of locking.

        If you want to set the value of a parameter internally, e.g. from your
        editor component, then don't call this directly - instead, use the
        setValueNotifyingHost() method, which will also send a message to
        the host telling it about the change. If the message isn't sent, the host
        won't be able to automate your parameters properly.

        The value passed will be between 0 and 1.0.
    */
    virtual void setValue (float newValue) = 0;

    /** A processor should call this when it needs to change one of its parameters.

        This could happen when the editor or some other internal operation changes
        a parameter. This method will call the setValue() method to change the
        value, and will then send a message to the host telling it about the change.

        Note that to make sure the host correctly handles automation, you should call
        the beginChangeGesture() and endChangeGesture() methods to tell the host when
        the user has started and stopped changing the parameter.
    */
    void setValueNotifyingHost (float newValue);

    /** Sends a signal to the host to tell it that the user is about to start changing this
        parameter.
        This allows the host to know when a parameter is actively being held by the user, and
        it may use this information to help it record automation.
        If you call this, it must be matched by a later call to endChangeGesture().
    */
    void beginChangeGesture();

    /** Tells the host that the user has finished changing this parameter.
        This allows the host to know when a parameter is actively being held by the user,
        and it may use this information to help it record automation.
        A call to this method must follow a call to beginChangeGesture().
    */
    void endChangeGesture();

    /** This should return the default value for this parameter. */
    virtual float getDefaultValue() const = 0;

    /** Returns the name to display for this parameter, which should be made
        to fit within the given string length.
    */
    virtual String getName (int maximumStringLength) const = 0;

    /** Some parameters may be able to return a label string for
        their units. For example "Hz" or "%".
    */
    virtual String getLabel() const = 0;

    /** Returns the number of steps that this parameter's range should be quantised into.

        If you want a continuous range of values, don't override this method, and allow
        the default implementation to return AudioProcessor::getDefaultNumParameterSteps().

        If your parameter is boolean, then you may want to make this return 2.

        The value that is returned may or may not be used, depending on the host. If you
        want the host to display stepped automation values, rather than a continuous
        interpolation between successive values, you should override isDiscrete to return true.

        @see isDiscrete
    */
    virtual int getNumSteps() const;

    /** Returns whether the parameter uses discrete values, based on the result of
        getNumSteps, or allows the host to select values continuously.

        This information may or may not be used, depending on the host. If you
        want the host to display stepped automation values, rather than a continuous
        interpolation between successive values, override this method to return true.

        @see getNumSteps
    */
    virtual bool isDiscrete() const;

    /** Returns a textual version of the supplied parameter value.
        The default implementation just returns the floating point value
        as a string, but this could do anything you need for a custom type
        of value.
    */
    virtual String getText (float value, int /*maximumStringLength*/) const;

    /** Should parse a string and return the appropriate value for it. */
    virtual float getValueForText (const String& text) const = 0;

    /** This can be overridden to tell the host that this parameter operates in the
        reverse direction.
        (Not all plugin formats or hosts will actually use this information).
    */
    virtual bool isOrientationInverted() const;

    /** Returns true if the host can automate this parameter.
        By default, this returns true.
    */
    virtual bool isAutomatable() const;

    /** Should return true if this parameter is a "meta" parameter.
        A meta-parameter is a parameter that changes other params. It is used
        by some hosts (e.g. AudioUnit hosts).
        By default this returns false.
    */
    virtual bool isMetaParameter() const;

    enum Category
    {
        genericParameter = (0 << 16) | 0,        /** If your parameter is not a meter then you should use this category */

        inputGain        = (1 << 16) | 0,        /** Currently not used */
        outputGain       = (1 << 16) | 1,

        /** The following categories tell the host that this parameter is a meter level value
            and therefore read-only. Most hosts will display these type of parameters as
            a meter in the generic view of your plug-in. Pro-Tools will also show the meter
            in the mixer view.
        */
        inputMeter                          = (2 << 16) | 0,
        outputMeter                         = (2 << 16) | 1,
        compressorLimiterGainReductionMeter = (2 << 16) | 2,
        expanderGateGainReductionMeter      = (2 << 16) | 3,
        analysisMeter                       = (2 << 16) | 4,
        otherMeter                          = (2 << 16) | 5
    };

    /** Returns the parameter's category. */
    virtual Category getCategory() const;

    /** Returns the index of this parameter in its parent processor's parameter list. */
    int getParameterIndex() const noexcept              { return parameterIndex; }

private:
    friend class AudioProcessor;
    AudioProcessor* processor = nullptr;
    int parameterIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorParameter)
};

} // namespace juce
