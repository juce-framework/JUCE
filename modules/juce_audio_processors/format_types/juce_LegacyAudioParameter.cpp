/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4996)

class LegacyAudioParameter :   public AudioProcessorParameter
{
public:
    LegacyAudioParameter (AudioProcessor& audioProcessorToUse, int audioParameterIndex)
    {
        processor = &audioProcessorToUse;

        parameterIndex = audioParameterIndex;
        jassert (parameterIndex < processor->getNumParameters());
    }

    //==============================================================================
    float getValue() const override                    { return processor->getParameter (parameterIndex); }
    void setValue (float newValue) override            { processor->setParameter (parameterIndex, newValue); }
    float getDefaultValue() const override             { return processor->getParameterDefaultValue (parameterIndex); }
    String getName (int maxLen) const override         { return processor->getParameterName (parameterIndex, maxLen); }
    String getLabel() const override                   { return processor->getParameterLabel (parameterIndex); }
    int getNumSteps() const override                   { return processor->getParameterNumSteps (parameterIndex); }
    bool isDiscrete() const override                   { return processor->isParameterDiscrete (parameterIndex); }
    bool isBoolean() const override                    { return false; }
    bool isOrientationInverted() const override        { return processor->isParameterOrientationInverted (parameterIndex); }
    bool isAutomatable() const override                { return processor->isParameterAutomatable (parameterIndex); }
    bool isMetaParameter() const override              { return processor->isMetaParameter (parameterIndex); }
    Category getCategory() const override              { return processor->getParameterCategory (parameterIndex); }
    String getCurrentValueAsText() const override      { return processor->getParameterText (parameterIndex); }
    String getParamID() const                          { return processor->getParameterID (parameterIndex); }

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

    static int getParamIndex (AudioProcessor& processor, AudioProcessorParameter* param) noexcept
    {
        if (auto* legacy = dynamic_cast<LegacyAudioParameter*> (param))
        {
            return legacy->parameterIndex;
        }
        else
        {
            auto n = processor.getNumParameters();
            jassert (n == processor.getParameters().size());

            for (int i = 0; i < n; ++i)
            {
                if (processor.getParameters()[i] == param)
                    return i;
            }
        }

        return -1;
    }

    static String getParamID (const AudioProcessorParameter* param, bool forceLegacyParamIDs) noexcept
    {
        if (auto* legacy = dynamic_cast<const LegacyAudioParameter*> (param))
            return forceLegacyParamIDs ? String (legacy->parameterIndex) : legacy->getParamID();

        if (auto* paramWithID = dynamic_cast<const AudioProcessorParameterWithID*> (param))
        {
            if (! forceLegacyParamIDs)
                return paramWithID->paramID;
        }

        if (param != nullptr)
            return String (param->getParameterIndex());

        return {};
    }
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

        legacyParamIDs = forceLegacyParamIDs;

        auto numParameters = audioProcessor.getNumParameters();
        usingManagedParameters = audioProcessor.getParameters().size() == numParameters;

        for (int i = 0; i < numParameters; ++i)
        {
            auto* param = [&]() -> AudioProcessorParameter*
            {
                if (usingManagedParameters)
                    return audioProcessor.getParameters()[i];

                auto newParam = std::make_unique<LegacyAudioParameter> (audioProcessor, i);
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
    bool legacyParamIDs = false, usingManagedParameters = false;
};

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

} // namespace juce
