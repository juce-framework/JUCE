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

#ifndef JUCE_AUDIOPROCESSORPARAMETER_H_INCLUDED
#define JUCE_AUDIOPROCESSORPARAMETER_H_INCLUDED


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

    /** The host will call this method to change the value of one of the filter's parameters.

        The host may call this at any time, including during the audio processing
        callback, so the filter has to process this very fast and avoid blocking.

        If you want to set the value of a parameter internally, e.g. from your
        editor component, then don't call this directly - instead, use the
        setValueNotifyingHost() method, which will also send a message to
        the host telling it about the change. If the message isn't sent, the host
        won't be able to automate your parameters properly.

        The value passed will be between 0 and 1.0.
    */
    virtual void setValue (float newValue) = 0;

    /** Your filter can call this when it needs to change one of its parameters.

        This could happen when the editor or some other internal operation changes
        a parameter. This method will call the setParameter() method to change the
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

    /** Returns the number of discrete interval steps that this parameter's range
        should be quantised into.

        If you want a continuous range of values, don't override this method, and allow
        the default implementation to return AudioProcessor::getDefaultNumParameterSteps().
        If your parameter is boolean, then you may want to make this return 2.
        The value that is returned may or may not be used, depending on the host.
    */
    virtual int getNumSteps() const;

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

    /** Returns the index of this parameter in its parent processor's parameter list. */
    int getParameterIndex() const noexcept              { return parameterIndex; }

private:
    friend class AudioProcessor;
    AudioProcessor* processor;
    int parameterIndex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorParameter)
};


#endif   // JUCE_AUDIOPROCESSORPARAMETER_H_INCLUDED
