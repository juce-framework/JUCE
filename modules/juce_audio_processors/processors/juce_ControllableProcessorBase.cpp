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

ControllableProcessorBase::ControllableProcessorBase()
{
}

ControllableProcessorBase::~ControllableProcessorBase()
{
    // ooh, nasty - the editor should have been deleted before its ControllableProcessorBase.
    jassert (activeEditor == nullptr);

#if JUCE_DEBUG && ! JUCE_DISABLE_AUDIOPROCESSOR_BEGIN_END_GESTURE_CHECKING
    // This will fail if you've called beginParameterChangeGesture() for one
    // or more parameters without having made a corresponding call to endParameterChangeGesture...
    jassert (changingParams.countNumberOfSetBits() == 0);
#endif

    // The parameters are owned by an AudioProcessorParameterGroup, but we need
    // to keep the managedParameters array populated to maintain backwards
    // compatibility.
    managedParameters.clearQuick (false);
}

//==============================================================================

void ControllableProcessorBase::editorBeingDeleted (AudioProcessorEditor* const editor) noexcept
{
    const ScopedLock sl (getCallbackLock());

    if (activeEditor == editor)
        activeEditor = nullptr;
}

AudioProcessorEditor* ControllableProcessorBase::createEditorIfNeeded()
{
    if (activeEditor != nullptr)
        return activeEditor;

    auto* ed = createEditor();

    if (ed != nullptr)
    {
        // you must give your editor comp a size before returning it..
        jassert (ed->getWidth() > 0 && ed->getHeight() > 0);

        const ScopedLock sl (getCallbackLock());
        activeEditor = ed;
    }

    // You must make your hasEditor() method return a consistent result!
    jassert (hasEditor() == (ed != nullptr));

    return ed;
}

//==============================================================================

void ControllableProcessorBase::setParameterNotifyingHost (int parameterIndex, float newValue)
{
    if (auto* param = getParameters()[parameterIndex])
    {
        param->setValueNotifyingHost (newValue);
    }
    else if (isPositiveAndBelow (parameterIndex, getNumParameters()))
    {
        setParameter (parameterIndex, newValue);
        sendParamChangeMessageToListeners (parameterIndex, newValue);
    }
}

void ControllableProcessorBase::beginParameterChangeGesture (int parameterIndex)
{
    if (auto* param = getParameters()[parameterIndex])
    {
        param->beginChangeGesture();
    }
    else
    {
        if (isPositiveAndBelow (parameterIndex, getNumParameters()))
        {
#if JUCE_DEBUG && ! JUCE_DISABLE_AUDIOPROCESSOR_BEGIN_END_GESTURE_CHECKING
            // This means you've called beginParameterChangeGesture twice in succession without a matching
            // call to endParameterChangeGesture. That might be fine in most hosts, but better to avoid doing it.
            jassert (! changingParams[parameterIndex]);
            changingParams.setBit (parameterIndex);
#endif

            sendParamChangeGestureBeginToListeners (parameterIndex);
        }
        else
        {
            jassertfalse; // called with an out-of-range parameter index!
        }
    }
}

void ControllableProcessorBase::endParameterChangeGesture (int parameterIndex)
{
    if (auto* param = getParameters()[parameterIndex])
    {
        param->endChangeGesture();
    }
    else
    {
        if (isPositiveAndBelow (parameterIndex, getNumParameters()))
        {
#if JUCE_DEBUG && ! JUCE_DISABLE_AUDIOPROCESSOR_BEGIN_END_GESTURE_CHECKING
            // This means you've called endParameterChangeGesture without having previously called
            // beginParameterChangeGesture. That might be fine in most hosts, but better to keep the
            // calls matched correctly.
            jassert (changingParams[parameterIndex]);
            changingParams.clearBit (parameterIndex);
#endif

            sendParamChangeGestureEndToListeners (parameterIndex);
        }
        else
        {
            jassertfalse; // called with an out-of-range parameter index!
        }
    }
}

String ControllableProcessorBase::getParameterName (int index, int maximumStringLength)
{
    if (auto* p = managedParameters[index])
        return p->getName (maximumStringLength);

    return isPositiveAndBelow (index, getNumParameters()) ? getParameterName (index).substring (0, maximumStringLength)
    : String();
}

const String ControllableProcessorBase::getParameterText (int index)
{
#if JUCE_DEBUG
    // if you hit this, then you're probably using the old parameter control methods,
    // but have forgotten to implement either of the getParameterText() methods.
    jassert (! textRecursionCheck);
    ScopedValueSetter<bool> sv (textRecursionCheck, true, false);
#endif

    return isPositiveAndBelow (index, getNumParameters()) ? getParameterText (index, 1024)
    : String();
}

String ControllableProcessorBase::getParameterText (int index, int maximumStringLength)
{
    if (auto* p = managedParameters[index])
        return p->getText (p->getValue(), maximumStringLength);

    return isPositiveAndBelow (index, getNumParameters()) ? getParameterText (index).substring (0, maximumStringLength)
    : String();
}

//==============================================================================

const OwnedArray<AudioProcessorParameter>& ControllableProcessorBase::getParameters() const noexcept
{
    return managedParameters;
}

int ControllableProcessorBase::getNumParameters()
{
    return managedParameters.size();
}

float ControllableProcessorBase::getParameter (int index)
{
    if (auto* p = getParamChecked (index))
        return p->getValue();

    return 0;
}

void ControllableProcessorBase::setParameter (int index, float newValue)
{
    if (auto* p = getParamChecked (index))
        p->setValue (newValue);
}

float ControllableProcessorBase::getParameterDefaultValue (int index)
{
    if (auto* p = managedParameters[index])
        return p->getDefaultValue();

    return 0;
}

const String ControllableProcessorBase::getParameterName (int index)
{
    if (auto* p = getParamChecked (index))
        return p->getName (512);

    return {};
}

String ControllableProcessorBase::getParameterID (int index)
{
    // Don't use getParamChecked here, as this must also work for legacy plug-ins
    if (auto* p = dynamic_cast<AudioProcessorParameterWithID*> (managedParameters[index]))
        return p->paramID;

    return String (index);
}

int ControllableProcessorBase::getParameterNumSteps (int index)
{
    if (auto* p = managedParameters[index])
        return p->getNumSteps();

    return AudioProcessor::getDefaultNumParameterSteps();
}

int ControllableProcessorBase::getDefaultNumParameterSteps() noexcept
{
    return 0x7fffffff;
}

bool ControllableProcessorBase::isParameterDiscrete (int index) const
{
    if (auto* p = managedParameters[index])
        return p->isDiscrete();

    return false;
}

String ControllableProcessorBase::getParameterLabel (int index) const
{
    if (auto* p = managedParameters[index])
        return p->getLabel();

    return {};
}

bool ControllableProcessorBase::isParameterAutomatable (int index) const
{
    if (auto* p = managedParameters[index])
        return p->isAutomatable();

    return true;
}

bool ControllableProcessorBase::isParameterOrientationInverted (int index) const
{
    if (auto* p = managedParameters[index])
        return p->isOrientationInverted();

    return false;
}

bool ControllableProcessorBase::isMetaParameter (int index) const
{
    if (auto* p = managedParameters[index])
        return p->isMetaParameter();

    return false;
}

AudioProcessorParameter::Category ControllableProcessorBase::getParameterCategory (int index) const
{
    if (auto* p = managedParameters[index])
        return p->getCategory();

    return AudioProcessorParameter::genericParameter;
}

AudioProcessorParameter* ControllableProcessorBase::getParamChecked (int index) const noexcept
{
    AudioProcessorParameter* p = managedParameters[index];

    // If you hit this, then you're either trying to access parameters that are out-of-range,
    // or you're not using addParameter and the managed parameter list, but have failed
    // to override some essential virtual methods and implement them appropriately.
    jassert (p != nullptr);
    return p;
}

void ControllableProcessorBase::addParameterInternal (AudioProcessorParameter* param)
{
    param->processor = this;
    param->parameterIndex = managedParameters.size();
    managedParameters.add (param);

#ifdef JUCE_DEBUG
    shouldCheckParamsForDupeIDs = true;
#endif
}

void ControllableProcessorBase::addParameter (AudioProcessorParameter* param)
{
    addParameterInternal (param);
    parameterTree.addChild (std::unique_ptr<AudioProcessorParameter> (param));
}

void ControllableProcessorBase::addParameterGroup (std::unique_ptr<AudioProcessorParameterGroup> group)
{
    for (auto* param : group->getParameters (true))
        addParameterInternal (param);

    parameterTree.addChild (std::move (group));
}

const AudioProcessorParameterGroup& ControllableProcessorBase::getParameterTree()
{
    return parameterTree;
}

#ifdef JUCE_DEBUG
void ControllableProcessorBase::checkForDupedParamIDs()
{
    if (shouldCheckParamsForDupeIDs)
    {
        shouldCheckParamsForDupeIDs = false;
        StringArray ids;

        for (auto p : managedParameters)
            if (auto* withID = dynamic_cast<AudioProcessorParameterWithID*> (p))
                ids.add (withID->paramID);

        ids.sort (false);

        for (int i = 1; i < ids.size(); ++i)
        {
            // This is triggered if you have two or more parameters with the same ID!
            jassert (ids[i - 1] != ids[i]);
        }
    }
}
#endif

} // namespace juce
