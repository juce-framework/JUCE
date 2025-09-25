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

#if JUCE_INTERNAL_HAS_LADSPA

#include <juce_audio_processors_headless/utilities/juce_CommonProcessorUtilities.h>

#include <ladspa.h>

namespace juce
{

static int shellLADSPAUIDToCreate = 0;
static int insideLADSPACallback = 0;

#define JUCE_LADSPA_LOGGING 1

#if JUCE_LADSPA_LOGGING
 #define JUCE_LADSPA_LOG(x) Logger::writeToLog (x);
#else
 #define JUCE_LADSPA_LOG(x)
#endif

//==============================================================================
class LADSPAModuleHandle final : public ReferenceCountedObject
{
public:
    LADSPAModuleHandle (const File& f)
        : file (f)
    {
        getActiveModules().add (this);
    }

    ~LADSPAModuleHandle()
    {
        getActiveModules().removeFirstMatchingValue (this);
        close();
    }

    using Ptr = ReferenceCountedObjectPtr<LADSPAModuleHandle>;

    static Array<LADSPAModuleHandle*>& getActiveModules()
    {
        static Array<LADSPAModuleHandle*> activeModules;
        return activeModules;
    }

    static LADSPAModuleHandle* findOrCreateModule (const File& file)
    {
        for (auto i = getActiveModules().size(); --i >= 0;)
        {
            auto* module = getActiveModules().getUnchecked (i);

            if (module->file == file)
                return module;
        }

        ++insideLADSPACallback;
        shellLADSPAUIDToCreate = 0;

        JUCE_LADSPA_LOG ("Loading LADSPA module: " + file.getFullPathName());

        std::unique_ptr<LADSPAModuleHandle> m (new LADSPAModuleHandle (file));

        if (! m->open())
            m = nullptr;

        --insideLADSPACallback;

        return m.release();
    }

    File file;
    LADSPA_Descriptor_Function moduleMain = nullptr;

private:
    DynamicLibrary module;

    bool open()
    {
        module.open (file.getFullPathName());
        moduleMain = (LADSPA_Descriptor_Function) module.getFunction ("ladspa_descriptor");

        return (moduleMain != nullptr);
    }

    void close()
    {
        module.close();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LADSPAModuleHandle)
};

//==============================================================================
class LADSPAPluginInstance final    : public AudioPluginInstance
{
public:
    LADSPAPluginInstance (const LADSPAModuleHandle::Ptr& m)
        : module (m)
    {
        ++insideLADSPACallback;

        name = module->file.getFileNameWithoutExtension();

        JUCE_LADSPA_LOG ("Creating LADSPA instance: " + name);

        if (module->moduleMain != nullptr)
        {
            plugin = module->moduleMain ((size_t) shellLADSPAUIDToCreate);

            if (plugin == nullptr)
            {
                JUCE_LADSPA_LOG ("Cannot find any valid descriptor in shared library");
                --insideLADSPACallback;
                return;
            }
        }
        else
        {
            JUCE_LADSPA_LOG ("Cannot find any valid plugin in shared library");
            --insideLADSPACallback;
            return;
        }

        const auto sampleRate = getSampleRate() > 0 ? getSampleRate()
                                                    : 44100.0;

        handle = plugin->instantiate (plugin, (uint32) sampleRate);

        --insideLADSPACallback;
    }

    ~LADSPAPluginInstance() override
    {
        const ScopedLock sl (lock);

        jassert (insideLADSPACallback == 0);

        if (handle != nullptr && plugin != nullptr)
            NullCheckedInvocation::invoke (plugin->cleanup, handle);

        initialised = false;
        module = nullptr;
        plugin = nullptr;
        handle = nullptr;
    }

    void initialise (double initialSampleRate, int initialBlockSize)
    {
        setPlayConfigDetails (inputs.size(), outputs.size(), initialSampleRate, initialBlockSize);

        if (initialised || plugin == nullptr || handle == nullptr)
            return;

        JUCE_LADSPA_LOG ("Initialising LADSPA: " + name);

        initialised = true;

        inputs.clear();
        outputs.clear();
        AudioProcessorParameterGroup newTree;

        for (unsigned int i = 0; i < plugin->PortCount; ++i)
        {
            const auto portDesc = plugin->PortDescriptors[i];

            if ((portDesc & LADSPA_PORT_CONTROL) != 0)
                newTree.addChild (std::make_unique<LADSPAParameter> (*this, (int) i,
                                                                     String (plugin->PortNames[i]).trim(),
                                                                     (portDesc & LADSPA_PORT_INPUT) != 0));

            if ((portDesc & LADSPA_PORT_AUDIO) != 0)
            {
                if ((portDesc & LADSPA_PORT_INPUT) != 0)    inputs.add ((int) i);
                if ((portDesc & LADSPA_PORT_OUTPUT) != 0)   outputs.add ((int) i);
            }
        }

        setHostedParameterTree (std::move (newTree));

        for (auto* param : getParameters())
            if (auto* ladspaParam = dynamic_cast<LADSPAParameter*> (param))
                plugin->connect_port (handle, (size_t) ladspaParam->paramID, &(ladspaParam->paramValue.scaled));

        setPlayConfigDetails (inputs.size(), outputs.size(), initialSampleRate, initialBlockSize);

        setCurrentProgram (0);
        setLatencySamples (0);

        // Some plugins crash if this doesn't happen:
        NullCheckedInvocation::invoke (plugin->activate, handle);
        NullCheckedInvocation::invoke (plugin->deactivate, handle);
    }

    //==============================================================================
    // AudioPluginInstance methods:

    void fillInPluginDescription (PluginDescription& desc) const override
    {
        desc.name = getName();
        desc.fileOrIdentifier = module->file.getFullPathName();
        desc.uniqueId = desc.deprecatedUid = getUID();
        desc.lastFileModTime = module->file.getLastModificationTime();
        desc.lastInfoUpdateTime = Time::getCurrentTime();
        desc.pluginFormatName = "LADSPA";
        desc.category = getCategory();
        desc.manufacturerName = plugin != nullptr ? String (plugin->Maker) : String();
        desc.version = getVersion();
        desc.numInputChannels  = getTotalNumInputChannels();
        desc.numOutputChannels = getTotalNumOutputChannels();
        desc.isInstrument = false;
    }

    const String getName() const override
    {
        if (plugin != nullptr && plugin->Label != nullptr)
            return plugin->Label;

        return name;
    }

    int getUID() const
    {
        if (plugin != nullptr && plugin->UniqueID != 0)
            return (int) plugin->UniqueID;

        return module->file.hashCode();
    }

    String getVersion() const                         { return LADSPA_VERSION; }
    String getCategory() const                        { return "Effect"; }

    bool acceptsMidi()  const override                { return false; }
    bool producesMidi() const override                { return false; }

    double getTailLengthSeconds() const override      { return 0.0; }

    //==============================================================================
    void prepareToPlay (double newSampleRate, int samplesPerBlockExpected) override
    {
        setLatencySamples (0);

        initialise (newSampleRate, samplesPerBlockExpected);

        if (initialised)
        {
            tempBuffer.setSize (jmax (1, outputs.size()), samplesPerBlockExpected);

            // dodgy hack to force some plugins to initialise the sample rate..
            if (auto* firstParam = getParameters()[0])
            {
                const auto old = firstParam->getValue();
                firstParam->setValue ((old < 0.5f) ? 1.0f : 0.0f);
                firstParam->setValue (old);
            }

            NullCheckedInvocation::invoke (plugin->activate, handle);
        }
    }

    void releaseResources() override
    {
        if (handle != nullptr && plugin->deactivate != nullptr)
            NullCheckedInvocation::invoke (plugin->deactivate, handle);

        tempBuffer.setSize (1, 1);
    }

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer&) override
    {
        auto numSamples = buffer.getNumSamples();

        if (initialised && plugin != nullptr && handle != nullptr)
        {
            for (int i = 0; i < inputs.size(); ++i)
                plugin->connect_port (handle, (size_t) inputs[i],
                                      i < buffer.getNumChannels() ? buffer.getWritePointer (i) : nullptr);

            if (plugin->run != nullptr)
            {
                for (int i = 0; i < outputs.size(); ++i)
                    plugin->connect_port (handle, (size_t) outputs.getUnchecked (i),
                                          i < buffer.getNumChannels() ? buffer.getWritePointer (i) : nullptr);

                plugin->run (handle, (size_t) numSamples);
                return;
            }

            if (plugin->run_adding != nullptr)
            {
                tempBuffer.setSize (outputs.size(), numSamples);
                tempBuffer.clear();

                for (int i = 0; i < outputs.size(); ++i)
                    plugin->connect_port (handle, (size_t) outputs.getUnchecked (i), tempBuffer.getWritePointer (i));

                plugin->run_adding (handle, (size_t) numSamples);

                for (int i = 0; i < outputs.size(); ++i)
                    if (i < buffer.getNumChannels())
                        buffer.copyFrom (i, 0, tempBuffer, i, 0, numSamples);

                return;
            }

            jassertfalse; // no callback to use?
        }

        for (auto i = getTotalNumInputChannels(), e = getTotalNumOutputChannels(); i < e; ++i)
            buffer.clear (i, 0, numSamples);
    }

    using AudioPluginInstance::processBlock;

    bool isInputChannelStereoPair  (int index) const override    { return isPositiveAndBelow (index, getTotalNumInputChannels()); }
    bool isOutputChannelStereoPair (int index) const override    { return isPositiveAndBelow (index, getTotalNumOutputChannels()); }

    const String getInputChannelName (const int index) const override
    {
        if (isPositiveAndBelow (index, getTotalNumInputChannels()))
            return String (plugin->PortNames [inputs [index]]).trim();

        return {};
    }

    const String getOutputChannelName (const int index) const override
    {
        if (isPositiveAndBelow (index, getTotalNumInputChannels()))
            return String (plugin->PortNames [outputs [index]]).trim();

        return {};
    }

    //==============================================================================
    int getNumPrograms() override                            { return 0; }
    int getCurrentProgram() override                         { return 0; }

    void setCurrentProgram (int) override
    {
        for (auto* param : getParameters())
            if (auto* ladspaParam = dynamic_cast<LADSPAParameter*> (param))
                ladspaParam->reset();
    }

    const String getProgramName (int) override             { return {}; }
    void changeProgramName (int, const String&) override   {}

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        auto numParameters = getParameters().size();
        destData.setSize ((size_t) numParameters * sizeof (float));
        destData.fillWith (0);

        auto* p = unalignedPointerCast<float*> (destData.getData());

        for (int i = 0; i < numParameters; ++i)
            if (auto* param = getParameters()[i])
                p[i] = param->getValue();
    }

    void getCurrentProgramStateInformation (MemoryBlock& destData) override               { getStateInformation (destData); }
    void setCurrentProgramStateInformation (const void* data, int sizeInBytes) override   { setStateInformation (data, sizeInBytes); }

    void setStateInformation (const void* data, [[maybe_unused]] int sizeInBytes) override
    {
        auto* p = static_cast<const float*> (data);

        for (int i = 0; i < getParameters().size(); ++i)
            if (auto* param = getParameters()[i])
                param->setValue (p[i]);
    }

    bool hasEditor() const override                 { return false; }
    AudioProcessorEditor* createEditor() override   { return nullptr; }

    bool isValid() const                            { return handle != nullptr; }

    //==============================================================================
    LADSPAModuleHandle::Ptr module;
    const LADSPA_Descriptor* plugin = nullptr;

private:
    //==============================================================================
    struct LADSPAParameter final   : public Parameter
    {
        struct ParameterValue
        {
            inline ParameterValue() noexcept                                               {}
            inline ParameterValue (float s, float u) noexcept  : scaled (s), unscaled (u)  {}

            float scaled = 0, unscaled = 0;
        };

        LADSPAParameter (LADSPAPluginInstance& parent, int parameterID,
                         const String& parameterName, bool parameterIsAutomatable)
            : pluginInstance (parent),
              paramID (parameterID),
              name (parameterName),
              automatable (parameterIsAutomatable)
        {
            reset();
        }

        float getValue() const override
        {
            if (pluginInstance.plugin != nullptr)
            {
                const ScopedLock sl (pluginInstance.lock);

                return paramValue.unscaled;
            }

            return 0.0f;
        }

        String getCurrentValueAsText() const override
        {
            if (auto* interface = pluginInstance.plugin)
            {
                const auto& hint = interface->PortRangeHints[paramID];

                if (LADSPA_IS_HINT_INTEGER (hint.HintDescriptor))
                    return String ((int) paramValue.scaled);

                return String (paramValue.scaled, 4);
            }

            return {};
        }

        void setValue (float newValue) override
        {
            if (auto* interface = pluginInstance.plugin)
            {
                const ScopedLock sl (pluginInstance.lock);

                if (! approximatelyEqual (paramValue.unscaled, newValue))
                    paramValue = ParameterValue (getNewParamScaled (interface->PortRangeHints [paramID], newValue), newValue);
            }
        }

        float getDefaultValue() const override
        {
            return defaultValue;
        }

        ParameterValue getDefaultParamValue() const
        {
            if (auto* interface = pluginInstance.plugin)
            {
                const auto& hint = interface->PortRangeHints[paramID];
                const auto& desc = hint.HintDescriptor;

                if (LADSPA_IS_HINT_HAS_DEFAULT (desc))
                {
                    if (LADSPA_IS_HINT_DEFAULT_0 (desc))    return {};
                    if (LADSPA_IS_HINT_DEFAULT_1 (desc))    return { 1.0f, 1.0f };
                    if (LADSPA_IS_HINT_DEFAULT_100 (desc))  return { 100.0f, 0.5f };
                    if (LADSPA_IS_HINT_DEFAULT_440 (desc))  return { 440.0f, 0.5f };

                    const auto scale = LADSPA_IS_HINT_SAMPLE_RATE (desc) ? (float) pluginInstance.getSampleRate()
                    : 1.0f;
                    const auto lower = hint.LowerBound * scale;
                    const auto upper = hint.UpperBound * scale;

                    if (LADSPA_IS_HINT_BOUNDED_BELOW (desc) && LADSPA_IS_HINT_DEFAULT_MINIMUM (desc))   return { lower, 0.0f };
                    if (LADSPA_IS_HINT_BOUNDED_ABOVE (desc) && LADSPA_IS_HINT_DEFAULT_MAXIMUM (desc))   return { upper, 1.0f };

                    if (LADSPA_IS_HINT_BOUNDED_BELOW (desc))
                    {
                        auto useLog = LADSPA_IS_HINT_LOGARITHMIC (desc);

                        if (LADSPA_IS_HINT_DEFAULT_LOW    (desc))  return { scaledValue (lower, upper, 0.25f, useLog), 0.25f };
                        if (LADSPA_IS_HINT_DEFAULT_MIDDLE (desc))  return { scaledValue (lower, upper, 0.50f, useLog), 0.50f };
                        if (LADSPA_IS_HINT_DEFAULT_HIGH   (desc))  return { scaledValue (lower, upper, 0.75f, useLog), 0.75f };
                    }
                }
            }

            return {};
        }

        void reset()
        {
            paramValue = getDefaultParamValue();
            defaultValue = paramValue.unscaled;
        }

        String getName (int /*maximumStringLength*/) const override    { return name; }
        String getLabel() const override                               { return {}; }

        bool isAutomatable() const override                            { return automatable; }

        String getParameterID() const override
        {
            return String (paramID);
        }

        static float scaledValue (float low, float high, float alpha, bool useLog) noexcept
        {
            if (useLog && low > 0 && high > 0)
                return expf (logf (low) * (1.0f - alpha) + logf (high) * alpha);

            return low + (high - low) * alpha;
        }

        static float toIntIfNecessary (const LADSPA_PortRangeHintDescriptor& desc, float value)
        {
            return LADSPA_IS_HINT_INTEGER (desc) ? ((float) (int) value) : value;
        }

        float getNewParamScaled (const LADSPA_PortRangeHint& hint, float newValue) const
        {
            const auto& desc = hint.HintDescriptor;

            if (LADSPA_IS_HINT_TOGGLED (desc))
                return (newValue < 0.5f) ? 0.0f : 1.0f;

            const auto scale = LADSPA_IS_HINT_SAMPLE_RATE (desc) ? (float) pluginInstance.getSampleRate()
            : 1.0f;
            const auto lower = hint.LowerBound * scale;
            const auto upper = hint.UpperBound * scale;

            if (LADSPA_IS_HINT_BOUNDED_BELOW (desc) && LADSPA_IS_HINT_BOUNDED_ABOVE (desc))
                return toIntIfNecessary (desc, scaledValue (lower, upper, newValue, LADSPA_IS_HINT_LOGARITHMIC (desc)));

            if (LADSPA_IS_HINT_BOUNDED_BELOW (desc))   return toIntIfNecessary (desc, newValue);
            if (LADSPA_IS_HINT_BOUNDED_ABOVE (desc))   return toIntIfNecessary (desc, newValue * upper);

            return 0.0f;
        }

        LADSPAPluginInstance& pluginInstance;
        const int paramID;
        const String name;
        const bool automatable;

        ParameterValue paramValue;
        float defaultValue = 0.0f;
    };

    //==============================================================================
    LADSPA_Handle handle = nullptr;
    String name;
    CriticalSection lock;
    bool initialised = false;
    AudioBuffer<float> tempBuffer { 1, 1 };
    Array<int> inputs, outputs;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LADSPAPluginInstance)
};


//==============================================================================
void LADSPAPluginFormatHeadless::findAllTypesForFile (OwnedArray<PluginDescription>& results, const String& fileOrIdentifier)
{
    if (! fileMightContainThisPluginType (fileOrIdentifier))
        return;

    PluginDescription desc;
    desc.fileOrIdentifier = fileOrIdentifier;
    desc.uniqueId = desc.deprecatedUid = 0;

    auto createdInstance = createInstanceFromDescription (desc, 44100.0, 512);
    auto instance = dynamic_cast<LADSPAPluginInstance*> (createdInstance.get());

    if (instance == nullptr || ! instance->isValid())
        return;

    instance->initialise (44100.0, 512);
    instance->fillInPluginDescription (desc);

    if (instance->module->moduleMain != nullptr)
    {
        for (int uid = 0;; ++uid)
        {
            if (auto* plugin = instance->module->moduleMain ((size_t) uid))
            {
                desc.uniqueId = desc.deprecatedUid = uid;
                desc.name = plugin->Name != nullptr ? plugin->Name : "Unknown";

                if (! arrayContainsPlugin (results, desc))
                    results.add (new PluginDescription (desc));
            }
            else
            {
                break;
            }
        }
    }
}

void LADSPAPluginFormatHeadless::createPluginInstance (const PluginDescription& desc,
                                                       double sampleRate,
                                                       int blockSize,
                                                       PluginCreationCallback callback)
{
    std::unique_ptr<LADSPAPluginInstance> result;

    if (fileMightContainThisPluginType (desc.fileOrIdentifier))
    {
        auto file = File (desc.fileOrIdentifier);

        auto previousWorkingDirectory = File::getCurrentWorkingDirectory();
        file.getParentDirectory().setAsCurrentWorkingDirectory();

        const LADSPAModuleHandle::Ptr module (LADSPAModuleHandle::findOrCreateModule (file));

        if (module != nullptr)
        {
            shellLADSPAUIDToCreate = desc.uniqueId != 0 ? desc.uniqueId : desc.deprecatedUid;

            result.reset (new LADSPAPluginInstance (module));

            if (result->plugin != nullptr && result->isValid())
                result->initialise (sampleRate, blockSize);
            else
                result = nullptr;
        }

        previousWorkingDirectory.setAsCurrentWorkingDirectory();
    }

    String errorMsg;

    if (result == nullptr)
        errorMsg = TRANS ("Unable to load XXX plug-in file").replace ("XXX", "LADSPA");

    callback (std::move (result), errorMsg);
}

bool LADSPAPluginFormatHeadless::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const
{
    return false;
}

bool LADSPAPluginFormatHeadless::fileMightContainThisPluginType (const String& fileOrIdentifier)
{
    auto f = File::createFileWithoutCheckingPath (fileOrIdentifier);
    return f.existsAsFile() && f.hasFileExtension (".so");
}

String LADSPAPluginFormatHeadless::getNameOfPluginFromIdentifier (const String& fileOrIdentifier)
{
    return fileOrIdentifier;
}

bool LADSPAPluginFormatHeadless::pluginNeedsRescanning (const PluginDescription& desc)
{
    return File (desc.fileOrIdentifier).getLastModificationTime() != desc.lastFileModTime;
}

bool LADSPAPluginFormatHeadless::doesPluginStillExist (const PluginDescription& desc)
{
    return File::createFileWithoutCheckingPath (desc.fileOrIdentifier).exists();
}

StringArray LADSPAPluginFormatHeadless::searchPathsForPlugins (const FileSearchPath& directoriesToSearch, const bool recursive, bool)
{
    StringArray results;

    for (int j = 0; j < directoriesToSearch.getNumPaths(); ++j)
        recursiveFileSearch (results, directoriesToSearch[j], recursive);

    return results;
}

void LADSPAPluginFormatHeadless::recursiveFileSearch (StringArray& results, const File& dir, const bool recursive)
{
    for (const auto& iter : RangedDirectoryIterator (dir, false, "*", File::findFilesAndDirectories))
    {
        auto f = iter.getFile();
        bool isPlugin = false;

        if (fileMightContainThisPluginType (f.getFullPathName()))
        {
            isPlugin = true;
            results.add (f.getFullPathName());
        }

        if (recursive && (! isPlugin) && f.isDirectory())
            recursiveFileSearch (results, f, true);
    }
}

FileSearchPath LADSPAPluginFormatHeadless::getDefaultLocationsToSearch()
{
    return  { SystemStats::getEnvironmentVariable ("LADSPA_PATH", "/usr/lib/ladspa;/usr/local/lib/ladspa;~/.ladspa").replace (":", ";") };
}

} // namespace juce

#endif
