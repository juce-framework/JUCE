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

PluginDescription AudioPluginInstance::getPluginDescription() const
{
    PluginDescription desc;
    fillInPluginDescription (desc);
    return desc;
}

String AudioPluginInstance::getParameterID (int parameterIndex)
{
    assertOnceOnDeprecatedMethodUse();

    // Currently there is no corresponding method available in the
    // AudioProcessorParameter class, and the previous behaviour of JUCE's
    // plug-in hosting code simply returns a string version of the index; to
    // maintain backwards compatibilty you should perform the operation below
    // this comment. However the caveat is that for plug-ins which change their
    // number of parameters dynamically at runtime you cannot rely upon the
    // returned parameter ID mapping to the correct parameter. A comprehensive
    // solution to this problem requires some additional work in JUCE's hosting
    // code.
    return String (parameterIndex);
}

float AudioPluginInstance::getParameter (int parameterIndex)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getValue();

    return 0.0f;
}

void AudioPluginInstance::setParameter (int parameterIndex, float newValue)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        param->setValue (newValue);
}

const String AudioPluginInstance::getParameterName (int parameterIndex)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getName (1024);

    return {};
}

String AudioPluginInstance::getParameterName (int parameterIndex, int maximumStringLength)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getName (maximumStringLength);

    return {};
}

const String AudioPluginInstance::getParameterText (int parameterIndex)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getCurrentValueAsText();

    return {};
}

String AudioPluginInstance::getParameterText (int parameterIndex, int maximumStringLength)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getCurrentValueAsText().substring (0, maximumStringLength);

    return {};
}

float AudioPluginInstance::getParameterDefaultValue (int parameterIndex)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getDefaultValue();

    return 0.0f;
}

int AudioPluginInstance::getParameterNumSteps (int parameterIndex)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getNumSteps();

    return AudioProcessor::getDefaultNumParameterSteps();
}

bool AudioPluginInstance::isParameterDiscrete (int parameterIndex) const
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->isDiscrete();

    return false;
}

bool AudioPluginInstance::isParameterAutomatable (int parameterIndex) const
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->isAutomatable();

    return true;
}

String AudioPluginInstance::getParameterLabel (int parameterIndex) const
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getLabel();

    return {};
}

bool AudioPluginInstance::isParameterOrientationInverted (int parameterIndex) const
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->isOrientationInverted();

    return false;
}

bool AudioPluginInstance::isMetaParameter (int parameterIndex) const
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->isMetaParameter();

    return false;
}

AudioProcessorParameter::Category AudioPluginInstance::getParameterCategory (int parameterIndex) const
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getCategory();

    return AudioProcessorParameter::genericParameter;
}

void AudioPluginInstance::assertOnceOnDeprecatedMethodUse() const noexcept
{
    if (! deprecationAssertiontriggered)
    {
        // If you hit this assertion then you are using at least one of the
        // methods marked as deprecated in this class. For now you can simply
        // continue past this point and subsequent uses of deprecated methods
        // will not trigger additional assertions. However, we will shortly be
        // removing these methods so you are strongly advised to look at the
        // implementation of the corresponding method in this class and use
        // that approach instead.
        jassertfalse;
    }

    deprecationAssertiontriggered = true;
}

bool AudioPluginInstance::deprecationAssertiontriggered = false;

AudioPluginInstance::Parameter::Parameter()
{
    onStrings.add (TRANS("on"));
    onStrings.add (TRANS("yes"));
    onStrings.add (TRANS("true"));

    offStrings.add (TRANS("off"));
    offStrings.add (TRANS("no"));
    offStrings.add (TRANS("false"));
}

AudioPluginInstance::Parameter::~Parameter() {}

String AudioPluginInstance::Parameter::getText (float value, int maximumStringLength) const
{
    if (isBoolean())
        return value < 0.5f ? TRANS("Off") : TRANS("On");

    return String (value).substring (0, maximumStringLength);
}

float AudioPluginInstance::Parameter::getValueForText (const String& text) const
{
    auto floatValue = text.retainCharacters ("-0123456789.").getFloatValue();

    if (isBoolean())
    {
        if (onStrings.contains (text, true))
            return 1.0f;

        if (offStrings.contains (text, true))
            return 0.0f;

        return floatValue < 0.5f ? 0.0f : 1.0f;
    }

    return floatValue;
}

} // namespace juce
