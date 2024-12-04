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

PluginDescription AudioPluginInstance::getPluginDescription() const
{
    PluginDescription desc;
    fillInPluginDescription (desc);
    return desc;
}

void* AudioPluginInstance::getPlatformSpecificData() { return nullptr; }

void AudioPluginInstance::getExtensions (ExtensionsVisitor& visitor) const { visitor.visitUnknown ({}); }

String AudioPluginInstance::getParameterID (int parameterIndex)
{
    assertOnceOnDeprecatedMethodUse();

    // Currently there is no corresponding method available in the
    // AudioProcessorParameter class, and the previous behaviour of JUCE's
    // plug-in hosting code simply returns a string version of the index; to
    // maintain backwards compatibility you should perform the operation below
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
    : onStrings  { TRANS ("on"),  TRANS ("yes"), TRANS ("true") },
      offStrings { TRANS ("off"), TRANS ("no"),  TRANS ("false") }
{
}

String AudioPluginInstance::Parameter::getText (float value, int maximumStringLength) const
{
    if (isBoolean())
        return value < 0.5f ? TRANS ("Off") : TRANS ("On");

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

void AudioPluginInstance::addHostedParameter (std::unique_ptr<HostedParameter> param)
{
    addParameter (param.release());
}

void AudioPluginInstance::addHostedParameterGroup (std::unique_ptr<AudioProcessorParameterGroup> group)
{
   #if JUCE_DEBUG
    // All parameters *must* be HostedParameters, otherwise getHostedParameter will return
    // garbage and your host will crash and burn
    for (auto* param : group->getParameters (true))
    {
        jassertquiet (dynamic_cast<HostedParameter*> (param) != nullptr);
    }
   #endif

    addParameterGroup (std::move (group));
}

void AudioPluginInstance::setHostedParameterTree (AudioProcessorParameterGroup group)
{
   #if JUCE_DEBUG
    // All parameters *must* be HostedParameters, otherwise getHostedParameter will return
    // garbage and your host will crash and burn
    for (auto* param : group.getParameters (true))
    {
        jassertquiet (dynamic_cast<HostedParameter*> (param) != nullptr);
    }
   #endif

    setParameterTree (std::move (group));
}

AudioPluginInstance::HostedParameter* AudioPluginInstance::getHostedParameter (int index) const
{
    // It's important that all AudioPluginInstance implementations
    // only ever own HostedParameters!
    return static_cast<HostedParameter*> (getParameters()[index]);
}

} // namespace juce
