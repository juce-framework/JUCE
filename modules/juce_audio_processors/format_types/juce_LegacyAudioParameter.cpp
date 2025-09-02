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

JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS

class LegacyAudioParameter final : public HostedAudioProcessorParameter
{
public:
    LegacyAudioParameter (AudioProcessorParameter::Listener& listener,
                          AudioProcessor& audioProcessorToUse,
                          int audioParameterIndex)
        : processor (&audioProcessorToUse)
    {
        setOwner (&listener);
        setParameterIndex (audioParameterIndex);
        jassert (getParameterIndex() < processor->getNumParameters());
    }

    //==============================================================================
    float getValue() const override                    { return processor->getParameter (getParameterIndex()); }
    void setValue (float newValue) override            { processor->setParameter (getParameterIndex(), newValue); }
    float getDefaultValue() const override             { return processor->getParameterDefaultValue (getParameterIndex()); }
    String getName (int maxLen) const override         { return processor->getParameterName (getParameterIndex(), maxLen); }
    String getLabel() const override                   { return processor->getParameterLabel (getParameterIndex()); }
    int getNumSteps() const override                   { return processor->getParameterNumSteps (getParameterIndex()); }
    bool isDiscrete() const override                   { return processor->isParameterDiscrete (getParameterIndex()); }
    bool isBoolean() const override                    { return false; }
    bool isOrientationInverted() const override        { return processor->isParameterOrientationInverted (getParameterIndex()); }
    bool isAutomatable() const override                { return processor->isParameterAutomatable (getParameterIndex()); }
    bool isMetaParameter() const override              { return processor->isMetaParameter (getParameterIndex()); }
    Category getCategory() const override              { return processor->getParameterCategory (getParameterIndex()); }
    String getCurrentValueAsText() const override      { return processor->getParameterText (getParameterIndex()); }
    String getParameterID() const override             { return processor->getParameterID (getParameterIndex()); }

    //==============================================================================
    float getValueForText (const String&) const override
    {
        // legacy parameters do not support this method
        jassertfalse;
        return 0.0f;
    }

    String getText (float, int) const override
    {
        // legacy parameters do not support this method
        jassertfalse;
        return {};
    }

    //==============================================================================
    static bool isLegacy (AudioProcessorParameter* param) noexcept
    {
        return (dynamic_cast<LegacyAudioParameter*> (param) != nullptr);
    }

    static int getParamIndex (AudioProcessor& proc, AudioProcessorParameter* param) noexcept
    {
        if (auto* legacy = dynamic_cast<LegacyAudioParameter*> (param))
            return legacy->getParameterIndex();

        auto n = proc.getNumParameters();
        jassert (n == proc.getParameters().size());

        for (int i = 0; i < n; ++i)
        {
            if (proc.getParameters()[i] == param)
                return i;
        }

        return -1;
    }

    static String getParamID (const AudioProcessorParameter* param, bool forceLegacyParamIDs) noexcept
    {
        if (auto* legacy = dynamic_cast<const LegacyAudioParameter*> (param))
            return forceLegacyParamIDs ? String (legacy->getParameterIndex()) : legacy->getParameterID();

        if (auto* paramWithID = dynamic_cast<const HostedAudioProcessorParameter*> (param))
        {
            if (! forceLegacyParamIDs)
                return paramWithID->getParameterID();
        }

        if (param != nullptr)
            return String (param->getParameterIndex());

        return {};
    }

private:
    AudioProcessor* processor = nullptr;
};

//==============================================================================
class LegacyAudioParametersWrapper
{
public:
    LegacyAudioParametersWrapper() = default;

    LegacyAudioParametersWrapper (AudioProcessor& audioProcessor, bool forceLegacyParamIDs)
    {
        update (audioProcessor, forceLegacyParamIDs);
    }

    void update (AudioProcessor& audioProcessor, bool forceLegacyParamIDs)
    {
        clear();

        forwarder = AudioProcessor::ParameterChangeForwarder { &audioProcessor };
        legacyParamIDs = forceLegacyParamIDs;

        auto numParameters = audioProcessor.getNumParameters();
        usingManagedParameters = audioProcessor.getParameters().size() == numParameters;

        for (int i = 0; i < numParameters; ++i)
        {
            auto* param = [&]() -> AudioProcessorParameter*
            {
                if (usingManagedParameters)
                    return audioProcessor.getParameters()[i];

                auto newParam = std::make_unique<LegacyAudioParameter> (forwarder, audioProcessor, i);
                auto* result = newParam.get();
                ownedGroup.addChild (std::move (newParam));

                return result;
            }();

            params.add (param);
        }

        processorGroup = usingManagedParameters ? &audioProcessor.getParameterTree()
                                                : nullptr;
    }

    void clear()
    {
        forwarder = AudioProcessor::ParameterChangeForwarder { nullptr };
        ownedGroup = AudioProcessorParameterGroup();
        params.clear();
    }

    AudioProcessorParameter* getParamForIndex (int index) const
    {
        if (isPositiveAndBelow (index, params.size()))
            return params[index];

        return nullptr;
    }

    String getParamID (AudioProcessor& processor, int idx) const noexcept
    {
        if (usingManagedParameters && ! legacyParamIDs)
            return processor.getParameterID (idx);

        return String (idx);
    }

    const AudioProcessorParameterGroup& getGroup() const
    {
        return processorGroup != nullptr ? *processorGroup
                                         : ownedGroup;
    }

    void addNonOwning (AudioProcessorParameter* param)
    {
        params.add (param);
    }

    size_t size() const noexcept { return (size_t) params.size(); }

    bool isUsingManagedParameters() const noexcept    { return usingManagedParameters; }
    int getNumParameters() const noexcept             { return params.size(); }

    AudioProcessorParameter* const* begin() const { return params.begin(); }
    AudioProcessorParameter* const* end()   const { return params.end(); }

    bool contains (AudioProcessorParameter* param) const
    {
        return params.contains (param);
    }

private:
    const AudioProcessorParameterGroup* processorGroup = nullptr;
    AudioProcessorParameterGroup ownedGroup;
    Array<AudioProcessorParameter*> params;
    AudioProcessor::ParameterChangeForwarder forwarder { nullptr };
    bool legacyParamIDs = false, usingManagedParameters = false;
};

JUCE_END_IGNORE_DEPRECATION_WARNINGS

} // namespace juce
