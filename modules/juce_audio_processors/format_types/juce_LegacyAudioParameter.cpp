/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

    static String getParamID (AudioProcessorParameter* param, bool forceLegacyParamIDs) noexcept
    {
        if (auto* legacy = dynamic_cast<LegacyAudioParameter*> (param))
        {
            return forceLegacyParamIDs ? String (legacy->parameterIndex) : legacy->getParamID();
        }
        else if (auto* paramWithID = dynamic_cast<AudioProcessorParameterWithID*> (param))
        {
            if (! forceLegacyParamIDs)
                return paramWithID->paramID;
        }

        return String (param->getParameterIndex());
    }
};

//==============================================================================
class LegacyAudioParametersWrapper
{
public:
    void update (AudioProcessor& audioProcessor, bool forceLegacyParamIDs)
    {
        clear();

        legacyParamIDs = forceLegacyParamIDs;

        auto numParameters = audioProcessor.getNumParameters();
        usingManagedParameters = audioProcessor.getParameters().size() == numParameters;

        for (int i = 0; i < numParameters; ++i)
        {
            AudioProcessorParameter* param = usingManagedParameters ? audioProcessor.getParameters()[i]
                                                                    : (legacy.add (new LegacyAudioParameter (audioProcessor, i)));
            params.add (param);
        }
    }

    void clear()
    {
        legacy.clear();
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

    bool isUsingManagedParameters() const noexcept    { return usingManagedParameters; }
    int getNumParameters() const noexcept             { return params.size(); }

    Array<AudioProcessorParameter*> params;

private:
    OwnedArray<LegacyAudioParameter> legacy;
    bool legacyParamIDs = false, usingManagedParameters = false;
};

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

} // namespace juce
