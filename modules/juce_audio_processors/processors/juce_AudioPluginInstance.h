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

#if JUCE_MSVC
 #pragma warning (push, 0)

 // MSVC does not like it if you override a deprecated method even if you
 // keep the deprecation attribute. Other compilers are more forgiving.
 #pragma warning (disable: 4996)
#endif

//==============================================================================
/**
    Base class for an active instance of a plugin.

    This derives from the AudioProcessor class, and adds some extra functionality
    that helps when wrapping dynamically loaded plugins.

    This class is not needed when writing plugins, and you should never need to derive
    your own sub-classes from it. The plugin hosting classes use it internally and will
    return AudioPluginInstance objects which wrap external plugins.

    @see AudioProcessor, AudioPluginFormat

    @tags{Audio}
*/
class JUCE_API  AudioPluginInstance   : public AudioProcessor
{
public:
    //==============================================================================
    /** Destructor.

        Make sure that you delete any UI components that belong to this plugin before
        deleting the plugin.
    */
    virtual ~AudioPluginInstance() {}

    //==============================================================================
    /** Fills-in the appropriate parts of this plugin description object. */
    virtual void fillInPluginDescription (PluginDescription& description) const = 0;

    /** Returns a PluginDescription for this plugin.
        This is just a convenience method to avoid calling fillInPluginDescription.
    */
    PluginDescription getPluginDescription() const;

    /** Returns a pointer to some kind of platform-specific data about the plugin.
        E.g. For a VST, this value can be cast to an AEffect*. For an AudioUnit, it can be
        cast to an AudioUnit handle.
    */
    virtual void* getPlatformSpecificData()                 { return nullptr; }

    /** For some formats (currently AudioUnit), this forces a reload of the list of
        available parameters.
    */
    virtual void refreshParameterList() {}

    // Rather than using these methods you should call the corresponding methods
    // on the AudioProcessorParameter objects returned from getParameters().
    // See the implementations of the methods below for some examples of how to
    // do this.
    //
    // In addition to being marked as deprecated these methods will assert on
    // the first call.
    JUCE_DEPRECATED (String getParameterID (int index) override);
    JUCE_DEPRECATED (float getParameter (int parameterIndex) override);
    JUCE_DEPRECATED (void setParameter (int parameterIndex, float newValue) override);
    JUCE_DEPRECATED (const String getParameterName (int parameterIndex) override);
    JUCE_DEPRECATED (String getParameterName (int parameterIndex, int maximumStringLength) override);
    JUCE_DEPRECATED (const String getParameterText (int parameterIndex) override);
    JUCE_DEPRECATED (String getParameterText (int parameterIndex, int maximumStringLength) override);
    JUCE_DEPRECATED (int getParameterNumSteps (int parameterIndex) override);
    JUCE_DEPRECATED (bool isParameterDiscrete (int parameterIndex) const override);
    JUCE_DEPRECATED (bool isParameterAutomatable (int parameterIndex) const override);
    JUCE_DEPRECATED (float getParameterDefaultValue (int parameterIndex) override);
    JUCE_DEPRECATED (String getParameterLabel (int parameterIndex) const override);
    JUCE_DEPRECATED (bool isParameterOrientationInverted (int parameterIndex) const override);
    JUCE_DEPRECATED (bool isMetaParameter (int parameterIndex) const override);
    JUCE_DEPRECATED (AudioProcessorParameter::Category getParameterCategory (int parameterIndex) const override);

protected:
    //==============================================================================
    /** Structure used to describe plugin parameters */
    struct Parameter   : public AudioProcessorParameter
    {
        Parameter();
        virtual ~Parameter();

        virtual String getText (float value, int maximumStringLength) const override;
        virtual float getValueForText (const String& text) const override;

        StringArray onStrings, offStrings;
    };

    AudioPluginInstance() {}
    AudioPluginInstance (const BusesProperties& ioLayouts) : AudioProcessor (ioLayouts) {}
    template <int numLayouts>
    AudioPluginInstance (const short channelLayoutList[numLayouts][2]) : AudioProcessor (channelLayoutList) {}

private:
    void assertOnceOnDeprecatedMethodUse() const noexcept;

    static bool deprecationAssertiontriggered;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginInstance)
};

#if JUCE_MSVC
 #pragma warning (pop)
#endif

} // namespace juce
