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

#if JUCE_PLUGINHOST_LADSPA && JUCE_LINUX

} // (juce namespace)

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
class LADSPAModuleHandle    : public ReferenceCountedObject
{
public:
    LADSPAModuleHandle (const File& f)
        : file (f), moduleMain (nullptr)
    {
        getActiveModules().add (this);
    }

    ~LADSPAModuleHandle()
    {
        getActiveModules().removeFirstMatchingValue (this);
        close();
    }

    typedef ReferenceCountedObjectPtr<LADSPAModuleHandle> Ptr;

    static Array <LADSPAModuleHandle*>& getActiveModules()
    {
        static Array <LADSPAModuleHandle*> activeModules;
        return activeModules;
    }

    static LADSPAModuleHandle* findOrCreateModule (const File& file)
    {
        for (int i = getActiveModules().size(); --i >= 0;)
        {
            LADSPAModuleHandle* const module = getActiveModules().getUnchecked(i);

            if (module->file == file)
                return module;
        }

        ++insideLADSPACallback;
        shellLADSPAUIDToCreate = 0;

        JUCE_LADSPA_LOG ("Loading LADSPA module: " + file.getFullPathName());

        ScopedPointer<LADSPAModuleHandle> m (new LADSPAModuleHandle (file));

        if (! m->open())
            m = nullptr;

        --insideLADSPACallback;

        return m.release();
    }

    File file;
    LADSPA_Descriptor_Function moduleMain;

private:
    DynamicLibrary module;

    bool open()
    {
        module.open (file.getFullPathName());
        moduleMain = (LADSPA_Descriptor_Function) module.getFunction ("ladspa_descriptor");
        return moduleMain != nullptr;
    }

    void close()
    {
        module.close();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LADSPAModuleHandle)
};


//==============================================================================
class LADSPAPluginInstance     : public AudioPluginInstance
{
public:
    LADSPAPluginInstance (const LADSPAModuleHandle::Ptr& m)
        : plugin (nullptr), handle (nullptr), initialised (false),
          tempBuffer (1, 1), module (m)
    {
        ++insideLADSPACallback;

        name = module->file.getFileNameWithoutExtension();

        JUCE_LADSPA_LOG ("Creating LADSPA instance: " + name);

        if (module->moduleMain != nullptr)
        {
            plugin = module->moduleMain (shellLADSPAUIDToCreate);

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

        const double sampleRate = getSampleRate() > 0 ? getSampleRate() : 44100.0;

        handle = plugin->instantiate (plugin, (uint32) sampleRate);

        --insideLADSPACallback;
    }

    ~LADSPAPluginInstance()
    {
        const ScopedLock sl (lock);

        jassert (insideLADSPACallback == 0);

        if (handle != nullptr && plugin != nullptr && plugin->cleanup != nullptr)
            plugin->cleanup (handle);

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
        parameters.clear();

        for (unsigned int i = 0; i < plugin->PortCount; ++i)
        {
            const LADSPA_PortDescriptor portDesc = plugin->PortDescriptors[i];

            if ((portDesc & LADSPA_PORT_CONTROL) != 0)
                parameters.add (i);

            if ((portDesc & LADSPA_PORT_AUDIO) != 0)
            {
                if ((portDesc & LADSPA_PORT_INPUT) != 0)    inputs.add (i);
                if ((portDesc & LADSPA_PORT_OUTPUT) != 0)   outputs.add (i);
            }
        }

        parameterValues.calloc (parameters.size());

        for (int i = 0; i < parameters.size(); ++i)
            plugin->connect_port (handle, parameters[i], &(parameterValues[i].scaled));

        setPlayConfigDetails (inputs.size(), outputs.size(), initialSampleRate, initialBlockSize);

        setCurrentProgram (0);
        setLatencySamples (0);

        // Some plugins crash if this doesn't happen:
        if (plugin->activate   != nullptr)   plugin->activate (handle);
        if (plugin->deactivate != nullptr)   plugin->deactivate (handle);
    }

    //==============================================================================
    // AudioPluginInstance methods:

    void fillInPluginDescription (PluginDescription& desc) const
    {
        desc.name = getName();
        desc.fileOrIdentifier = module->file.getFullPathName();
        desc.uid = getUID();
        desc.lastFileModTime = module->file.getLastModificationTime();
        desc.pluginFormatName = "LADSPA";
        desc.category = getCategory();
        desc.manufacturerName = plugin != nullptr ? String (plugin->Maker) : String::empty;
        desc.version = getVersion();
        desc.numInputChannels  = getNumInputChannels();
        desc.numOutputChannels = getNumOutputChannels();
        desc.isInstrument = false;
    }

    const String getName() const
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

    String getVersion() const                 { return LADSPA_VERSION; }
    String getCategory() const                { return "Effect"; }

    bool acceptsMidi() const                  { return false; }
    bool producesMidi() const                 { return false; }

    bool silenceInProducesSilenceOut() const  { return plugin == nullptr; } // ..any way to get a proper answer for these?
    double getTailLengthSeconds() const       { return 0.0; }

    //==============================================================================
    void prepareToPlay (double newSampleRate, int samplesPerBlockExpected)
    {
        setLatencySamples (0);

        initialise (newSampleRate, samplesPerBlockExpected);

        if (initialised)
        {
            tempBuffer.setSize (jmax (1, outputs.size()), samplesPerBlockExpected);

            // dodgy hack to force some plugins to initialise the sample rate..
            if (getNumParameters() > 0)
            {
                const float old = getParameter (0);
                setParameter (0, (old < 0.5f) ? 1.0f : 0.0f);
                setParameter (0, old);
            }

            if (plugin->activate != nullptr)
                plugin->activate (handle);
        }
    }

    void releaseResources()
    {
        if (handle != nullptr && plugin->deactivate != nullptr)
            plugin->deactivate (handle);

        tempBuffer.setSize (1, 1);
    }

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
    {
        const int numSamples = buffer.getNumSamples();

        if (initialised && plugin != nullptr && handle != nullptr)
        {
            for (int i = 0; i < inputs.size(); ++i)
                plugin->connect_port (handle, inputs[i],
                                      i < buffer.getNumChannels() ? buffer.getSampleData (i) : nullptr);

            if (plugin->run != nullptr)
            {
                for (int i = 0; i < outputs.size(); ++i)
                    plugin->connect_port (handle, outputs.getUnchecked(i),
                                          i < buffer.getNumChannels() ? buffer.getSampleData (i) : nullptr);

                plugin->run (handle, numSamples);
                return;
            }

            if (plugin->run_adding != nullptr)
            {
                tempBuffer.setSize (outputs.size(), numSamples);
                tempBuffer.clear();

                for (int i = 0; i < outputs.size(); ++i)
                    plugin->connect_port (handle, outputs.getUnchecked(i), tempBuffer.getSampleData (i));

                plugin->run_adding (handle, numSamples);

                for (int i = 0; i < outputs.size(); ++i)
                    if (i < buffer.getNumChannels())
                        buffer.copyFrom (i, 0, tempBuffer, i, 0, numSamples);

                return;
            }

            jassertfalse; // no callback to use?
        }

        for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
            buffer.clear (i, 0, numSamples);
    }

    bool isInputChannelStereoPair (int index) const    { return isPositiveAndBelow (index, getNumInputChannels()); }
    bool isOutputChannelStereoPair (int index) const   { return isPositiveAndBelow (index, getNumInputChannels()); }

    const String getInputChannelName (const int index) const
    {
        if (isPositiveAndBelow (index, getNumInputChannels()))
            return String (plugin->PortNames [inputs [index]]).trim();

        return String::empty;
    }

    const String getOutputChannelName (const int index) const
    {
        if (isPositiveAndBelow (index, getNumInputChannels()))
            return String (plugin->PortNames [outputs [index]]).trim();

        return String::empty;
    }

    //==============================================================================
    int getNumParameters()                              { return handle != nullptr ? parameters.size() : 0; }

    bool isParameterAutomatable (int index) const
    {
        return plugin != nullptr
                 && (plugin->PortDescriptors [parameters[index]] & LADSPA_PORT_INPUT) != 0;
    }

    float getParameter (int index)
    {
        if (plugin != nullptr && isPositiveAndBelow (index, parameters.size()))
        {
            const ScopedLock sl (lock);
            return parameterValues[index].unscaled;
        }

        return 0.0f;
    }

    void setParameter (int index, float newValue)
    {
        if (plugin != nullptr && isPositiveAndBelow (index, parameters.size()))
        {
            const ScopedLock sl (lock);

            ParameterValue& p = parameterValues[index];

            if (p.unscaled != newValue)
                p = ParameterValue (getNewParamScaled (plugin->PortRangeHints [parameters[index]], newValue), newValue);
        }
    }

    const String getParameterName (int index)
    {
        if (plugin != nullptr)
        {
            jassert (isPositiveAndBelow (index, parameters.size()));
            return String (plugin->PortNames [parameters [index]]).trim();
        }

        return String::empty;
    }

    const String getParameterText (int index)
    {
        if (plugin != nullptr)
        {
            jassert (index >= 0 && index < parameters.size());

            const LADSPA_PortRangeHint& hint = plugin->PortRangeHints [parameters [index]];

            if (LADSPA_IS_HINT_INTEGER (hint.HintDescriptor))
                return String ((int) parameterValues[index].scaled);

            return String (parameterValues[index].scaled, 4);
        }

        return String::empty;
    }

    //==============================================================================
    int getNumPrograms()                                { return 0; }
    int getCurrentProgram()                             { return 0; }

    void setCurrentProgram (int newIndex)
    {
        if (plugin != nullptr)
            for (int i = 0; i < parameters.size(); ++i)
                parameterValues[i] = getParamValue (plugin->PortRangeHints [parameters[i]]);
    }

    const String getProgramName (int index)
    {
        // XXX
        return String::empty;
    }

    void changeProgramName (int index, const String& newName)
    {
        // XXX
    }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData)
    {
        destData.setSize (sizeof (float) * getNumParameters());
        destData.fillWith (0);

        float* const p = (float*) ((char*) destData.getData());
        for (int i = 0; i < getNumParameters(); ++i)
            p[i] = getParameter(i);
    }

    void getCurrentProgramStateInformation (MemoryBlock& destData)
    {
        getStateInformation (destData);
    }

    void setStateInformation (const void* data, int sizeInBytes)
    {
        const float* p = static_cast <const float*> (data);

        for (int i = 0; i < getNumParameters(); ++i)
            setParameter (i, p[i]);
    }

    void setCurrentProgramStateInformation (const void* data, int sizeInBytes)
    {
        setStateInformation (data, sizeInBytes);
    }

    bool hasEditor() const
    {
        return false;
    }

    AudioProcessorEditor* createEditor()
    {
        return nullptr;
    }

    bool isValid() const
    {
        return handle != nullptr;
    }

    LADSPAModuleHandle::Ptr module;
    const LADSPA_Descriptor* plugin;

private:
    LADSPA_Handle handle;
    String name;
    CriticalSection lock;
    bool initialised;
    AudioSampleBuffer tempBuffer;
    Array<int> inputs, outputs, parameters;

    struct ParameterValue
    {
        inline ParameterValue() noexcept                   : scaled (0), unscaled (0) {}
        inline ParameterValue (float s, float u) noexcept  : scaled (s), unscaled (u) {}

        float scaled, unscaled;
    };

    HeapBlock<ParameterValue> parameterValues;

    //==============================================================================
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
        const LADSPA_PortRangeHintDescriptor& desc = hint.HintDescriptor;

        if (LADSPA_IS_HINT_TOGGLED (desc))
            return (newValue < 0.5f) ? 0.0f : 1.0f;

        const float scale = LADSPA_IS_HINT_SAMPLE_RATE (desc) ? (float) getSampleRate() : 1.0f;
        const float lower = hint.LowerBound * scale;
        const float upper = hint.UpperBound * scale;

        if (LADSPA_IS_HINT_BOUNDED_BELOW (desc) && LADSPA_IS_HINT_BOUNDED_ABOVE (desc))
            return toIntIfNecessary (desc, scaledValue (lower, upper, newValue, LADSPA_IS_HINT_LOGARITHMIC (desc)));

        if (LADSPA_IS_HINT_BOUNDED_BELOW (desc))   return toIntIfNecessary (desc, newValue);
        if (LADSPA_IS_HINT_BOUNDED_ABOVE (desc))   return toIntIfNecessary (desc, newValue * upper);

        return 0.0f;
    }

    ParameterValue getParamValue (const LADSPA_PortRangeHint& hint) const
    {
        const LADSPA_PortRangeHintDescriptor& desc = hint.HintDescriptor;

        if (LADSPA_IS_HINT_HAS_DEFAULT (desc))
        {
            if (LADSPA_IS_HINT_DEFAULT_0 (desc))    return ParameterValue();
            if (LADSPA_IS_HINT_DEFAULT_1 (desc))    return ParameterValue (1.0f, 1.0f);
            if (LADSPA_IS_HINT_DEFAULT_100 (desc))  return ParameterValue (100.0f, 0.5f);
            if (LADSPA_IS_HINT_DEFAULT_440 (desc))  return ParameterValue (440.0f, 0.5f);

            const float scale = LADSPA_IS_HINT_SAMPLE_RATE (desc) ? (float) getSampleRate() : 1.0f;
            const float lower = hint.LowerBound * scale;
            const float upper = hint.UpperBound * scale;

            if (LADSPA_IS_HINT_BOUNDED_BELOW (desc) && LADSPA_IS_HINT_DEFAULT_MINIMUM (desc))   return ParameterValue (lower, 0.0f);
            if (LADSPA_IS_HINT_BOUNDED_ABOVE (desc) && LADSPA_IS_HINT_DEFAULT_MAXIMUM (desc))   return ParameterValue (upper, 1.0f);

            if (LADSPA_IS_HINT_BOUNDED_BELOW (desc))
            {
                const bool useLog = LADSPA_IS_HINT_LOGARITHMIC (desc);

                if (LADSPA_IS_HINT_DEFAULT_LOW    (desc))  return ParameterValue (scaledValue (lower, upper, 0.25f, useLog), 0.25f);
                if (LADSPA_IS_HINT_DEFAULT_MIDDLE (desc))  return ParameterValue (scaledValue (lower, upper, 0.50f, useLog), 0.50f);
                if (LADSPA_IS_HINT_DEFAULT_HIGH   (desc))  return ParameterValue (scaledValue (lower, upper, 0.75f, useLog), 0.75f);
            }
        }

        return ParameterValue();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LADSPAPluginInstance)
};


//==============================================================================
//==============================================================================
LADSPAPluginFormat::LADSPAPluginFormat() {}
LADSPAPluginFormat::~LADSPAPluginFormat() {}

void LADSPAPluginFormat::findAllTypesForFile (OwnedArray <PluginDescription>& results,
                                              const String& fileOrIdentifier)
{
    if (! fileMightContainThisPluginType (fileOrIdentifier))
        return;

    PluginDescription desc;
    desc.fileOrIdentifier = fileOrIdentifier;
    desc.uid = 0;

    ScopedPointer<LADSPAPluginInstance> instance (dynamic_cast<LADSPAPluginInstance*> (createInstanceFromDescription (desc, 44100.0, 512)));

    if (instance == nullptr || ! instance->isValid())
        return;

    instance->initialise (44100.0, 512);

    instance->fillInPluginDescription (desc);

    if (instance->module->moduleMain != nullptr)
    {
        for (int uid = 0;; ++uid)
        {
            if (const LADSPA_Descriptor* plugin = instance->module->moduleMain (uid))
            {
                desc.uid = uid;
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

AudioPluginInstance* LADSPAPluginFormat::createInstanceFromDescription (const PluginDescription& desc,
                                                                        double sampleRate, int blockSize)
{
    ScopedPointer<LADSPAPluginInstance> result;

    if (fileMightContainThisPluginType (desc.fileOrIdentifier))
    {
        File file (desc.fileOrIdentifier);

        const File previousWorkingDirectory (File::getCurrentWorkingDirectory());
        file.getParentDirectory().setAsCurrentWorkingDirectory();

        const LADSPAModuleHandle::Ptr module (LADSPAModuleHandle::findOrCreateModule (file));

        if (module != nullptr)
        {
            shellLADSPAUIDToCreate = desc.uid;

            result = new LADSPAPluginInstance (module);

            if (result->plugin != nullptr && result->isValid())
                result->initialise (sampleRate, blockSize);
            else
                result = nullptr;
        }

        previousWorkingDirectory.setAsCurrentWorkingDirectory();
    }

    return result.release();
}

bool LADSPAPluginFormat::fileMightContainThisPluginType (const String& fileOrIdentifier)
{
    const File f (File::createFileWithoutCheckingPath (fileOrIdentifier));
    return f.existsAsFile() && f.hasFileExtension (".so");
}

String LADSPAPluginFormat::getNameOfPluginFromIdentifier (const String& fileOrIdentifier)
{
    return fileOrIdentifier;
}

bool LADSPAPluginFormat::pluginNeedsRescanning (const PluginDescription& desc)
{
    return File (desc.fileOrIdentifier).getLastModificationTime() != desc.lastFileModTime;
}

bool LADSPAPluginFormat::doesPluginStillExist (const PluginDescription& desc)
{
    return File::createFileWithoutCheckingPath (desc.fileOrIdentifier).exists();
}

StringArray LADSPAPluginFormat::searchPathsForPlugins (const FileSearchPath& directoriesToSearch, const bool recursive)
{
    StringArray results;

    for (int j = 0; j < directoriesToSearch.getNumPaths(); ++j)
        recursiveFileSearch (results, directoriesToSearch[j], recursive);

    return results;
}

void LADSPAPluginFormat::recursiveFileSearch (StringArray& results, const File& dir, const bool recursive)
{
    DirectoryIterator iter (dir, false, "*", File::findFilesAndDirectories);

    while (iter.next())
    {
        const File f (iter.getFile());
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

FileSearchPath LADSPAPluginFormat::getDefaultLocationsToSearch()
{
    return FileSearchPath (SystemStats::getEnvironmentVariable ("LADSPA_PATH",
                                                                "/usr/lib/ladspa;/usr/local/lib/ladspa;~/.ladspa")
                             .replace (":", ";"));
}

#endif
