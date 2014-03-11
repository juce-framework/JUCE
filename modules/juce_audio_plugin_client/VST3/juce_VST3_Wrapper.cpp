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

// Your project must contain an AppConfig.h file with your project-specific settings in it,
// and your header search path must make it accessible to the module's files.
#include "AppConfig.h"

//==============================================================================
#if JucePlugin_Build_VST3 && (__APPLE_CPP__ || __APPLE_CC__ || _WIN32 || _WIN64)

#include "../../juce_audio_processors/format_types/juce_VST3Headers.h"
#include "../utility/juce_CheckSettingMacros.h"
#include "../utility/juce_IncludeModuleHeaders.h"
#include "../../juce_audio_processors/format_types/juce_VST3Common.h"

#undef Point
#undef Component

using namespace Steinberg;

//==============================================================================
class JuceLibraryRefCount
{
public:
    JuceLibraryRefCount()   { if ((getCount()++) == 0) initialiseJuce_GUI(); }
    ~JuceLibraryRefCount()  { if ((--getCount()) == 0) shutdownJuce_GUI(); }

private:
    int& getCount() noexcept
    {
        static int count = 0;
        return count;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceLibraryRefCount)
};

//==============================================================================
namespace juce
{
 #if JUCE_MAC
  extern void initialiseMac();
  extern void* attachComponentToWindowRef (Component*, void* parent, bool isNSView);
  extern void detachComponentFromWindowRef (Component*, void* window, bool isNSView);
  extern void setNativeHostWindowSize (void* window, Component*, int newWidth, int newHeight, bool isNSView);
 #endif
}

//==============================================================================
class JuceAudioProcessor  : public FUnknown
{
public:
    JuceAudioProcessor (AudioProcessor* source) noexcept
        : refCount (0), audioProcessor (source) {}

    virtual ~JuceAudioProcessor() {}

    AudioProcessor* get() const noexcept      { return audioProcessor; }

    JUCE_DECLARE_VST3_COM_QUERY_METHODS
    JUCE_DECLARE_VST3_COM_REF_METHODS

    static const FUID iid;

private:
    Atomic<int> refCount;
    ScopedPointer<AudioProcessor> audioProcessor;

    JuceAudioProcessor() JUCE_DELETED_FUNCTION;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceAudioProcessor)
};

#define TEST_FOR_COMMON_BASE_AND_RETURN_IF_VALID(CommonClassType, SourceClassType) \
    if (doUIDsMatch (iid, CommonClassType::iid)) \
    { \
        addRef(); \
        *obj = (CommonClassType*) static_cast<SourceClassType*> (this); \
        return Steinberg::kResultOk; \
    }

//==============================================================================
class JuceVST3EditController : public Vst::EditController,
                               public Vst::IMidiMapping,
                               public AudioProcessorListener
{
public:
    JuceVST3EditController (Vst::IHostApplication* host)
    {
        if (host != nullptr)
            host->queryInterface (FUnknown::iid, (void**) &hostContext);
    }

    //==============================================================================
    static const FUID iid;

    //==============================================================================
    REFCOUNT_METHODS (ComponentBase)

    tresult PLUGIN_API queryInterface (const TUID iid, void** obj) override
    {
        TEST_FOR_AND_RETURN_IF_VALID (FObject)
        TEST_FOR_AND_RETURN_IF_VALID (JuceVST3EditController)
        TEST_FOR_AND_RETURN_IF_VALID (Vst::IEditController)
        TEST_FOR_AND_RETURN_IF_VALID (Vst::IEditController2)
        TEST_FOR_AND_RETURN_IF_VALID (Vst::IConnectionPoint)
        TEST_FOR_AND_RETURN_IF_VALID (Vst::IMidiMapping)
        TEST_FOR_COMMON_BASE_AND_RETURN_IF_VALID (IPluginBase, Vst::IEditController)
        TEST_FOR_COMMON_BASE_AND_RETURN_IF_VALID (IDependent, Vst::IEditController)
        TEST_FOR_COMMON_BASE_AND_RETURN_IF_VALID (FUnknown, Vst::IEditController)

        if (doUIDsMatch (iid, JuceAudioProcessor::iid))
        {
            audioProcessor->addRef();
            *obj = audioProcessor;
            return kResultOk;
        }

        *obj = nullptr;
        return kNoInterface;
    }

    //==============================================================================
    tresult PLUGIN_API initialize (FUnknown* context) override
    {
        if (hostContext != context)
        {
            if (hostContext != nullptr)
                hostContext->release();

            hostContext = context;

            if (hostContext != nullptr)
                hostContext->addRef();
        }

        return kResultTrue;
    }

    tresult PLUGIN_API terminate() override
    {
        if (AudioProcessor* const pluginInstance = getPluginInstance())
            pluginInstance->removeListener (this);

        audioProcessor = nullptr;

        return EditController::terminate();
    }

    //==============================================================================
    struct Param  : public Vst::Parameter
    {
        Param (AudioProcessor& p, int index)  : owner (p), paramIndex (index)
        {
            info.id = (Vst::ParamID) index;
            toString128 (info.title, p.getParameterName (index));
            toString128 (info.shortTitle, p.getParameterName (index, 8));
            toString128 (info.units, p.getParameterLabel (index));

            const int numSteps = p.getParameterNumSteps (index);
            info.stepCount = (Steinberg::int32) (numSteps > 0 && numSteps < 0x7fffffff ? numSteps - 1 : 0);
            info.defaultNormalizedValue = p.getParameterDefaultValue (index);
            info.unitId = Vst::kRootUnitId;
            info.flags = p.isParameterAutomatable (index) ? Vst::ParameterInfo::kCanAutomate : 0;
        }

        virtual ~Param() {}

        bool setNormalized (Vst::ParamValue v) override
        {
            v = jlimit (0.0, 1.0, v);

            if (v != valueNormalized)
            {
                valueNormalized = v;
                changed();
                return true;
            }

            return false;
        }

        void toString (Vst::ParamValue, Vst::String128 result) const override
        {
            toString128 (result, owner.getParameterText (paramIndex, 128));
        }

        Vst::ParamValue toPlain (Vst::ParamValue v) const override       { return v; }
        Vst::ParamValue toNormalized (Vst::ParamValue v) const override  { return v; }

    private:
        AudioProcessor& owner;
        int paramIndex;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Param)
    };

    //==============================================================================
    void setAudioProcessor (JuceAudioProcessor* audioProc)
    {
        if (audioProcessor != audioProc)
        {
            audioProcessor = audioProc;
            setupParameters();
        }
    }

    tresult PLUGIN_API connect (IConnectionPoint* other) override
    {
        if (other != nullptr && audioProcessor == nullptr)
        {
            const tresult result = ComponentBase::connect (other);

            if (! audioProcessor.loadFrom (other))
                sendIntMessage ("JuceVST3EditController", (Steinberg::int64) (pointer_sized_int) this);
            else
                setupParameters();

            return result;
        }

        jassertfalse;
        return kResultFalse;
    }

    //==============================================================================
    tresult PLUGIN_API getMidiControllerAssignment (Steinberg::int32, Steinberg::int16,
                                                    Vst::CtrlNumber,
                                                    Vst::ParamID& id) override
    {
        //TODO
        id = 0;
        return kNotImplemented;
    }

    //==============================================================================
    IPlugView* PLUGIN_API createView (const char* name) override
    {
        if (AudioProcessor* const pluginInstance = getPluginInstance())
        {
            if (pluginInstance->hasEditor() && name != nullptr
                 && strcmp (name, Vst::ViewType::kEditor) == 0)
            {
                return new JuceVST3Editor (*this, *pluginInstance);
            }
        }

        return nullptr;
    }

    //==============================================================================
    void audioProcessorParameterChangeGestureBegin (AudioProcessor*, int index) override        { beginEdit ((Steinberg::uint32) index); }
    void audioProcessorParameterChanged (AudioProcessor*, int index, float newValue) override   { performEdit ((Steinberg::uint32) index, (double) newValue); }
    void audioProcessorParameterChangeGestureEnd (AudioProcessor*, int index) override          { endEdit ((Steinberg::uint32) index); }

    void audioProcessorChanged (AudioProcessor*) override
    {
        if (componentHandler != nullptr)
            componentHandler->restartComponent (Vst::kLatencyChanged & Vst::kParamValuesChanged);
    }

    //==============================================================================
    AudioProcessor* getPluginInstance() const noexcept
    {
        if (audioProcessor != nullptr)
            return audioProcessor->get();

        return nullptr;
    }

private:
    //==============================================================================
    ComSmartPtr<JuceAudioProcessor> audioProcessor;
    const JuceLibraryRefCount juceCount;

    //==============================================================================
    void setupParameters()
    {
        if (AudioProcessor* const pluginInstance = getPluginInstance())
        {
            pluginInstance->addListener (this);

            if (parameters.getParameterCount() <= 0)
                for (int i = 0; i < pluginInstance->getNumParameters(); ++i)
                    parameters.addParameter (new Param (*pluginInstance, i));

            audioProcessorChanged (pluginInstance);
        }
    }

    void sendIntMessage (const char* idTag, const Steinberg::int64 value)
    {
        jassert (hostContext != nullptr);

        if (Vst::IMessage* message = allocateMessage())
        {
            const FReleaser releaser (message);
            message->setMessageID (idTag);
            message->getAttributes()->setInt (idTag, value);
            sendMessage (message);
        }
    }

    //==============================================================================
    class JuceVST3Editor : public Vst::EditorView
    {
    public:
        JuceVST3Editor (JuceVST3EditController& ec, AudioProcessor& p)
          : Vst::EditorView (&ec, nullptr),
            owner (&ec), pluginInstance (p)
        {
           #if JUCE_MAC
            macHostWindow = nullptr;
            isNSView = false;
           #endif

            component = new ContentWrapperComponent (*this, p);
        }

        //==============================================================================
        tresult PLUGIN_API isPlatformTypeSupported (FIDString type) override
        {
            if (type != nullptr && pluginInstance.hasEditor())
            {
               #if JUCE_WINDOWS
                if (strcmp (type, kPlatformTypeHWND) == 0)
               #else
                if (strcmp (type, kPlatformTypeNSView) == 0 || strcmp (type, kPlatformTypeHIView) == 0)
               #endif
                    return kResultTrue;
            }

            return kResultFalse;
        }

        tresult PLUGIN_API attached (void* parent, FIDString type) override
        {
            if (parent == nullptr || isPlatformTypeSupported (type) == kResultFalse)
                return kResultFalse;

           #if JUCE_WINDOWS
            component->addToDesktop (0, parent);
            component->setOpaque (true);
            component->setVisible (true);
           #else
            isNSView = (strcmp (type, kPlatformTypeNSView) == 0);
            macHostWindow = juce::attachComponentToWindowRef (component, parent, isNSView);
           #endif

            component->resizeHostWindow();
            systemWindow = parent;
            attachedToParent();

            return kResultTrue;
        }

        tresult PLUGIN_API removed() override
        {
            if (component != nullptr)
            {
               #if JUCE_WINDOWS
                component->removeFromDesktop();
               #else
                if (macHostWindow != nullptr)
                {
                    juce::detachComponentFromWindowRef (component, macHostWindow, isNSView);
                    macHostWindow = nullptr;
                }
               #endif

                component = nullptr;
            }

            return CPluginView::removed();
        }

        tresult PLUGIN_API onSize (ViewRect* newSize) override
        {
            if (newSize != nullptr)
            {
                rect = *newSize;

                if (component != nullptr)
                    component->setSize (rect.getWidth(), rect.getHeight());

                return kResultTrue;
            }

            jassertfalse;
            return kResultFalse;
        }

        tresult PLUGIN_API getSize (ViewRect* size) override
        {
            if (size != nullptr && component != nullptr)
            {
                *size = ViewRect (0, 0, component->getWidth(), component->getHeight());
                return kResultTrue;
            }

            return kResultFalse;
        }

        tresult PLUGIN_API canResize() override         { return kResultTrue; }

        tresult PLUGIN_API checkSizeConstraint (ViewRect* rect) override
        {
            if (rect != nullptr && component != nullptr)
            {
                rect->right  = rect->left + component->getWidth();
                rect->bottom = rect->top  + component->getHeight();
                return kResultTrue;
            }

            jassertfalse;
            return kResultFalse;
        }

    private:
        //==============================================================================
        class ContentWrapperComponent : public juce::Component
        {
        public:
            ContentWrapperComponent (JuceVST3Editor& editor, AudioProcessor& plugin)
               : owner (editor),
                 pluginEditor (plugin.createEditorIfNeeded())
            {
                setOpaque (true);
                setBroughtToFrontOnMouseClick (true);

                // if hasEditor() returns true then createEditorIfNeeded has to return a valid editor
                jassert (pluginEditor != nullptr);

                if (pluginEditor != nullptr)
                {
                    addAndMakeVisible (pluginEditor);
                    setBounds (pluginEditor->getLocalBounds());
                    resizeHostWindow();
                }
            }

            ~ContentWrapperComponent()
            {
                if (pluginEditor != nullptr)
                {
                    PopupMenu::dismissAllActiveMenus();
                    pluginEditor->getAudioProcessor()->editorBeingDeleted (pluginEditor);
                }
            }

            void paint (Graphics& g) override
            {
                g.fillAll (Colours::black);
            }

            void childBoundsChanged (Component*) override
            {
                resizeHostWindow();
            }

            void resized() override
            {
                if (pluginEditor != nullptr)
                    pluginEditor->setBounds (getLocalBounds());
            }

            void resizeHostWindow()
            {
                if (pluginEditor != nullptr)
                {
                    const int w = pluginEditor->getWidth();
                    const int h = pluginEditor->getHeight();

                   #if JUCE_WINDOWS
                    setSize (w, h);
                   #else
                    if (owner.macHostWindow != nullptr)
                        juce::setNativeHostWindowSize (owner.macHostWindow, this, w, h, owner.isNSView);
                   #endif

                    if (owner.plugFrame != nullptr)
                    {
                        ViewRect newSize (0, 0, w, h);
                        owner.plugFrame->resizeView (&owner, &newSize);
                    }
                }
            }

        private:
            JuceVST3Editor& owner;
            ScopedPointer<AudioProcessorEditor> pluginEditor;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentWrapperComponent)
        };

        //==============================================================================
        ComSmartPtr<JuceVST3EditController> owner;
        AudioProcessor& pluginInstance;

        ScopedPointer<ContentWrapperComponent> component;
        friend class ContentWrapperComponent;

       #if JUCE_MAC
        void* macHostWindow;
        bool isNSView;
       #endif

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceVST3Editor)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceVST3EditController)
};

//==============================================================================
class JuceVST3Component : public Vst::IComponent,
                          public Vst::IAudioProcessor,
                          public Vst::IUnitInfo,
                          public Vst::IConnectionPoint,
                          public AudioPlayHead
{
public:
    JuceVST3Component (Vst::IHostApplication* h)
      : refCount (1),
        host (h),
        audioInputs  (Vst::kAudio, Vst::kInput),
        audioOutputs (Vst::kAudio, Vst::kOutput),
        eventInputs  (Vst::kEvent, Vst::kInput),
        eventOutputs (Vst::kEvent, Vst::kOutput)
    {
        pluginInstance = createPluginFilterOfType (AudioProcessor::wrapperType_VST3);
        comPluginInstance = new JuceAudioProcessor (pluginInstance);

        zerostruct (processContext);

        processSetup.maxSamplesPerBlock = 1024;
        processSetup.processMode = Vst::kRealtime;
        processSetup.sampleRate = 44100.0;
        processSetup.symbolicSampleSize = Vst::kSample32;
    }

    ~JuceVST3Component()
    {
        if (pluginInstance != nullptr)
            if (pluginInstance->getPlayHead() == this)
                pluginInstance->setPlayHead (nullptr);

        audioInputs.removeAll();
        audioOutputs.removeAll();
        eventInputs.removeAll();
        eventOutputs.removeAll();
    }

    //==============================================================================
    AudioProcessor& getPluginInstance() const noexcept { return *pluginInstance; }

    //==============================================================================
    static const FUID iid;

    JUCE_DECLARE_VST3_COM_REF_METHODS

    tresult PLUGIN_API queryInterface (const TUID iid, void** obj) override
    {
        TEST_FOR_AND_RETURN_IF_VALID (IPluginBase)
        TEST_FOR_AND_RETURN_IF_VALID (JuceVST3Component)
        TEST_FOR_AND_RETURN_IF_VALID (Vst::IComponent)
        TEST_FOR_AND_RETURN_IF_VALID (Vst::IAudioProcessor)
        TEST_FOR_AND_RETURN_IF_VALID (Vst::IUnitInfo)
        TEST_FOR_AND_RETURN_IF_VALID (Vst::IConnectionPoint)
        TEST_FOR_COMMON_BASE_AND_RETURN_IF_VALID (FUnknown, Vst::IComponent)

        if (doUIDsMatch (iid, JuceAudioProcessor::iid))
        {
            comPluginInstance->addRef();
            *obj = comPluginInstance;
            return kResultOk;
        }

        *obj = nullptr;
        return kNoInterface;
    }

    //==============================================================================
    tresult PLUGIN_API initialize (FUnknown* hostContext) override
    {
        if (host != hostContext)
            host.loadFrom (hostContext);

       #if JucePlugin_MaxNumInputChannels > 0
        addAudioBusTo (audioInputs, TRANS("Audio Input"),
                       getArrangementForNumChannels (JucePlugin_MaxNumInputChannels));
       #endif

       #if JucePlugin_MaxNumOutputChannels > 0
        addAudioBusTo (audioOutputs, TRANS("Audio Output"),
                       getArrangementForNumChannels (JucePlugin_MaxNumOutputChannels));
       #endif

       #if JucePlugin_WantsMidiInput
        addEventBusTo (eventInputs, TRANS("MIDI Input"));
       #endif

       #if JucePlugin_ProducesMidiOutput
        addEventBusTo (eventOutputs, TRANS("MIDI Output"));
       #endif

        processContext.sampleRate = processSetup.sampleRate;

        preparePlugin (processSetup.sampleRate, (int) processSetup.maxSamplesPerBlock);

        return kResultTrue;
    }

    tresult PLUGIN_API terminate() override
    {
        getPluginInstance().releaseResources();
        return kResultTrue;
    }

    //==============================================================================
    tresult PLUGIN_API connect (IConnectionPoint* other) override
    {
        if (other != nullptr && juceVST3EditController == nullptr)
            juceVST3EditController.loadFrom (other);

        return kResultTrue;
    }

    tresult PLUGIN_API disconnect (IConnectionPoint*) override
    {
        juceVST3EditController = nullptr;
        return kResultTrue;
    }

    tresult PLUGIN_API notify (Vst::IMessage* message) override
    {
        if (message != nullptr && juceVST3EditController == nullptr)
        {
            Steinberg::int64 value = 0;

            if (message->getAttributes()->getInt ("JuceVST3EditController", value) == kResultTrue)
            {
                juceVST3EditController = (JuceVST3EditController*) (pointer_sized_int) value;

                if (juceVST3EditController != nullptr)
                    juceVST3EditController->setAudioProcessor (comPluginInstance);
                else
                    jassertfalse;
            }
        }

        return kResultTrue;
    }

    //==============================================================================
    tresult PLUGIN_API getControllerClassId (TUID classID) override
    {
        memcpy (classID, JuceVST3EditController::iid, sizeof (TUID));
        return kResultTrue;
    }

    Steinberg::int32 PLUGIN_API getBusCount (Vst::MediaType type, Vst::BusDirection dir) override
    {
        if (Vst::BusList* const busList = getBusListFor (type, dir))
            return busList->total();

        return 0;
    }

    tresult PLUGIN_API getBusInfo (Vst::MediaType type, Vst::BusDirection dir,
                                   Steinberg::int32 index, Vst::BusInfo& info) override
    {
        if (Vst::BusList* const busList = getBusListFor (type, dir))
        {
            if (Vst::Bus* const bus = (Vst::Bus*) busList->at (index))
            {
                info.mediaType = type;
                info.direction = dir;

                if (bus->getInfo (info))
                    return kResultTrue;
            }
        }

        zerostruct (info);
        return kResultFalse;
    }

    tresult PLUGIN_API activateBus (Vst::MediaType type, Vst::BusDirection dir,
                                    Steinberg::int32 index, TBool state) override
    {
        if (Vst::BusList* const busList = getBusListFor (type, dir))
        {
            if (Vst::Bus* const bus = (Vst::Bus*) busList->at (index))
            {
                bus->setActive (state);
                return kResultTrue;
            }
        }

        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API setActive (TBool state) override
    {
        if (state == kResultFalse)
        {
            getPluginInstance().releaseResources();
        }
        else
        {
            double sampleRate = getPluginInstance().getSampleRate();
            int bufferSize = getPluginInstance().getBlockSize();

            sampleRate = processSetup.sampleRate > 0.0
                            ? processSetup.sampleRate
                            : sampleRate;

            bufferSize = processSetup.maxSamplesPerBlock > 0
                            ? (int) processSetup.maxSamplesPerBlock
                            : bufferSize;

            channelList.clear();
            channelList.insertMultiple (0, nullptr, jmax (JucePlugin_MaxNumInputChannels, JucePlugin_MaxNumOutputChannels) + 1);

            preparePlugin (sampleRate, bufferSize);
        }

        return kResultOk;
    }

    tresult PLUGIN_API setIoMode (Vst::IoMode) override { return kNotImplemented; }
    tresult PLUGIN_API getRoutingInfo (Vst::RoutingInfo&, Vst::RoutingInfo&) override { return kNotImplemented; }

    bool readFromMemoryStream (IBStream* state) const
    {
        FUnknownPtr<MemoryStream> s (state);

        if (s != nullptr
             && s->getData() != nullptr
             && s->getSize() > 0
             && s->getSize() < 1024 * 1024 * 100) // (some hosts seem to return junk for the size)
        {
            // Adobe Audition CS6 hack to avoid trying to use corrupted streams:
            if (getHostType().isAdobeAudition())
                if (s->getSize() >= 5 && memcmp (s->getData(), "VC2!E", 5) == 0)
                    return false;

            pluginInstance->setStateInformation (s->getData(), (int) s->getSize());
            return true;
        }

        return false;
    }

    bool readFromUnknownStream (IBStream* state) const
    {
        MemoryOutputStream allData;

        {
            const size_t bytesPerBlock = 4096;
            HeapBlock<char> buffer (bytesPerBlock);

            for (;;)
            {
                Steinberg::int32 bytesRead = 0;

                if (state->read (buffer, (Steinberg::int32) bytesPerBlock, &bytesRead) == kResultTrue && bytesRead > 0)
                {
                    allData.write (buffer, bytesRead);
                    continue;
                }

                break;
            }
        }

        const size_t dataSize = allData.getDataSize();

        if (dataSize > 0 && dataSize < 0x7fffffff)
        {
            pluginInstance->setStateInformation (allData.getData(), (int) dataSize);
            return true;
        }

        return false;
    }

    tresult PLUGIN_API setState (IBStream* state) override
    {
        if (state == nullptr)
            return kInvalidArgument;

        FUnknownPtr<IBStream> stateRefHolder (state); // just in case the caller hasn't properly ref-counted the stream object

        if (state->seek (0, IBStream::kIBSeekSet, nullptr) == kResultTrue)
            if (readFromMemoryStream (state) || readFromUnknownStream (state))
                return kResultTrue;

        return kResultFalse;
    }

    tresult PLUGIN_API getState (IBStream* state) override
    {
        if (state != nullptr)
        {
            juce::MemoryBlock mem;
            pluginInstance->getStateInformation (mem);
            return state->write (mem.getData(), (Steinberg::int32) mem.getSize());
        }

        return kInvalidArgument;
    }

    //==============================================================================
    Steinberg::int32 PLUGIN_API getUnitCount() override
    {
        return 1;
    }

    tresult PLUGIN_API getUnitInfo (Steinberg::int32 unitIndex, Vst::UnitInfo& info) override
    {
        if (unitIndex == 0)
        {
            info.id             = Vst::kRootUnitId;
            info.parentUnitId   = Vst::kNoParentUnitId;
            info.programListId  = Vst::kNoProgramListId;

            toString128 (info.name, TRANS("Root Unit"));

            return kResultTrue;
        }

        zerostruct (info);
        return kResultFalse;
    }

    Steinberg::int32 PLUGIN_API getProgramListCount() override
    {
        if (getPluginInstance().getNumPrograms() > 0)
            return 1;

        return 0;
    }

    tresult PLUGIN_API getProgramListInfo (Steinberg::int32 listIndex, Vst::ProgramListInfo& info) override
    {
        if (listIndex == 0)
        {
            info.id = paramPreset;
            info.programCount = (Steinberg::int32) getPluginInstance().getNumPrograms();

            toString128 (info.name, TRANS("Factory Presets"));

            return kResultTrue;
        }

        jassertfalse;
        zerostruct (info);
        return kResultFalse;
    }

    tresult PLUGIN_API getProgramName (Vst::ProgramListID listId, Steinberg::int32 programIndex, Vst::String128 name) override
    {
        if (listId == paramPreset
            && isPositiveAndBelow ((int) programIndex, getPluginInstance().getNumPrograms()))
        {
            toString128 (name, getPluginInstance().getProgramName ((int) programIndex));
            return kResultTrue;
        }

        jassertfalse;
        toString128 (name, juce::String());
        return kResultFalse;
    }

    tresult PLUGIN_API getProgramInfo (Vst::ProgramListID, Steinberg::int32, Vst::CString, Vst::String128) override             { return kNotImplemented; }
    tresult PLUGIN_API hasProgramPitchNames (Vst::ProgramListID, Steinberg::int32) override                                     { return kNotImplemented; }
    tresult PLUGIN_API getProgramPitchName (Vst::ProgramListID, Steinberg::int32, Steinberg::int16, Vst::String128) override    { return kNotImplemented; }
    tresult PLUGIN_API selectUnit (Vst::UnitID) override                                                                        { return kNotImplemented; }
    tresult PLUGIN_API setUnitProgramData (Steinberg::int32, Steinberg::int32, IBStream*) override                              { return kNotImplemented; }
    Vst::UnitID PLUGIN_API getSelectedUnit() override                                                                           { return Vst::kRootUnitId; }

    tresult PLUGIN_API getUnitByBus (Vst::MediaType, Vst::BusDirection,
                                     Steinberg::int32, Steinberg::int32,
                                     Vst::UnitID& unitId) override
    {
        zerostruct (unitId);
        return kNotImplemented;
    }

    //==============================================================================
    bool getCurrentPosition (CurrentPositionInfo& info) override
    {
        info.timeInSamples              = jmax ((juce::int64) 0, processContext.projectTimeSamples);
        info.timeInSeconds              = processContext.projectTimeMusic;
        info.bpm                        = jmax (1.0, processContext.tempo);
        info.timeSigNumerator           = jmax (1, (int) processContext.timeSigNumerator);
        info.timeSigDenominator         = jmax (1, (int) processContext.timeSigDenominator);
        info.ppqPositionOfLastBarStart  = processContext.barPositionMusic;
        info.ppqPosition                = processContext.projectTimeMusic;
        info.ppqLoopStart               = processContext.cycleStartMusic;
        info.ppqLoopEnd                 = processContext.cycleEndMusic;
        info.isRecording                = (processContext.state & Vst::ProcessContext::kRecording) != 0;
        info.isPlaying                  = (processContext.state & Vst::ProcessContext::kPlaying) != 0;
        info.isLooping                  = (processContext.state & Vst::ProcessContext::kCycleActive) != 0;
        info.editOriginTime             = 0.0;
        info.frameRate                  = AudioPlayHead::fpsUnknown;

        if ((processContext.state & Vst::ProcessContext::kSmpteValid) != 0)
        {
            switch (processContext.frameRate.framesPerSecond)
            {
                case 24: info.frameRate = AudioPlayHead::fps24; break;
                case 25: info.frameRate = AudioPlayHead::fps25; break;
                case 29: info.frameRate = AudioPlayHead::fps30drop; break;

                case 30:
                {
                    if ((processContext.frameRate.flags & Vst::FrameRate::kDropRate) != 0)
                        info.frameRate = AudioPlayHead::fps30drop;
                    else
                        info.frameRate = AudioPlayHead::fps30;
                }
                break;

                default: break;
            }
        }

        return true;
    }

    //==============================================================================
    static tresult setBusArrangementFor (Vst::BusList& list,
                                         Vst::SpeakerArrangement* arrangement,
                                         Steinberg::int32 numBusses)
    {
        if (arrangement != nullptr && numBusses == 1) //Should only be 1 bus per BusList
        {
            Steinberg::int32 counter = 0;

            FOREACH_CAST (IPtr<Vst::Bus>, Vst::AudioBus, bus, list)
                if (counter < numBusses)
                    bus->setArrangement (arrangement[counter]);

                counter++;
            ENDFOR

            return kResultTrue;
        }

        return kResultFalse;
    }

    tresult PLUGIN_API setBusArrangements (Vst::SpeakerArrangement* inputs, Steinberg::int32 numIns,
                                           Vst::SpeakerArrangement* outputs, Steinberg::int32 numOuts) override
    {
       #if JucePlugin_MaxNumInputChannels > 0
        if (setBusArrangementFor (audioInputs, inputs, numIns) != kResultTrue)
            return kResultFalse;
       #else
        if (numIns != 0)
            return kResultFalse;
       #endif

       #if JucePlugin_MaxNumOutputChannels > 0
        if (setBusArrangementFor (audioOutputs, outputs, numOuts) != kResultTrue)
            return kResultFalse;
       #else
        if (numOuts != 0)
            return kResultFalse;
       #endif

        return kResultTrue;
    }

    tresult PLUGIN_API getBusArrangement (Vst::BusDirection dir, Steinberg::int32 index, Vst::SpeakerArrangement& arr) override
    {
        if (Vst::BusList* const busList = getBusListFor (Vst::kAudio, dir))
        {
            if (Vst::AudioBus* const audioBus = FCast<Vst::AudioBus> (busList->at (index)))
            {
                arr = audioBus->getArrangement();
                return kResultTrue;
            }
        }

        return kResultFalse;
    }

    tresult PLUGIN_API canProcessSampleSize (Steinberg::int32 symbolicSampleSize) override
    {
        return symbolicSampleSize == Vst::kSample32 ? kResultTrue : kResultFalse;
    }

    Steinberg::uint32 PLUGIN_API getLatencySamples() override
    {
        return (Steinberg::uint32) jmax (0, getPluginInstance().getLatencySamples());
    }

    tresult PLUGIN_API setupProcessing (Vst::ProcessSetup& newSetup) override
    {
        if (canProcessSampleSize (newSetup.symbolicSampleSize) != kResultTrue)
            return kResultFalse;

        processSetup = newSetup;
        processContext.sampleRate = processSetup.sampleRate;

        preparePlugin (processSetup.sampleRate, processSetup.maxSamplesPerBlock);

        return kResultTrue;
    }

    tresult PLUGIN_API setProcessing (TBool state) override
    {
        if (state == kResultFalse)
            getPluginInstance().reset();

        return kResultTrue;
    }

    Steinberg::uint32 PLUGIN_API getTailSamples() override
    {
        const double tailLengthSeconds = getPluginInstance().getTailLengthSeconds();

        if (tailLengthSeconds <= 0.0 || processSetup.sampleRate > 0.0)
            return Vst::kNoTail;

        return (Steinberg::uint32) roundToIntAccurate (tailLengthSeconds * processSetup.sampleRate);
    }

    //==============================================================================
    void processParameterChanges (Vst::IParameterChanges& paramChanges)
    {
        jassert (pluginInstance != nullptr);

        const Steinberg::int32 numParamsChanged = paramChanges.getParameterCount();

        for (Steinberg::int32 i = 0; i < numParamsChanged; ++i)
        {
            if (Vst::IParamValueQueue* paramQueue = paramChanges.getParameterData (i))
            {
                const Steinberg::int32 numPoints = paramQueue->getPointCount();

                Steinberg::int32 offsetSamples;
                double value = 0.0;

                if (paramQueue->getPoint (numPoints - 1,  offsetSamples, value) == kResultTrue)
                {
                    const int id = (int) paramQueue->getParameterId();
                    jassert (isPositiveAndBelow (id, pluginInstance->getNumParameters()));
                    pluginInstance->setParameter (id, (float) value);
                }
            }
        }
    }

    tresult PLUGIN_API process (Vst::ProcessData& data) override
    {
        if (pluginInstance == nullptr)
            return kResultFalse;

        if (data.processContext != nullptr)
            processContext = *data.processContext;
        else
            zerostruct (processContext);

        midiBuffer.clear();

       #if JucePlugin_WantsMidiInput
        if (data.inputEvents != nullptr)
            MidiEventList::toMidiBuffer (midiBuffer, *data.inputEvents);
       #endif

       #if JUCE_DEBUG && ! JucePlugin_ProducesMidiOutput
        const int numMidiEventsComingIn = midiBuffer.getNumEvents();
       #endif

        const int numInputChans  = data.inputs  != nullptr ? (int) data.inputs[0].numChannels : 0;
        const int numOutputChans = data.outputs != nullptr ? (int) data.outputs[0].numChannels : 0;

        int totalChans = 0;

        while (totalChans < numInputChans)
        {
            channelList.set (totalChans, data.inputs[0].channelBuffers32[totalChans]);
            ++totalChans;
        }

        while (totalChans < numOutputChans)
        {
            channelList.set (totalChans, data.outputs[0].channelBuffers32[totalChans]);
            ++totalChans;
        }

        AudioSampleBuffer buffer (channelList.getRawDataPointer(), totalChans, (int) data.numSamples);

        {
            const ScopedLock sl (pluginInstance->getCallbackLock());

            pluginInstance->setNonRealtime (data.processMode == Vst::kOffline);

            if (data.inputParameterChanges != nullptr)
                processParameterChanges (*data.inputParameterChanges);

            if (pluginInstance->isSuspended())
                buffer.clear();
            else
                pluginInstance->processBlock (buffer, midiBuffer);
        }

        for (int i = 0; i < numOutputChans; ++i)
            FloatVectorOperations::copy (data.outputs[0].channelBuffers32[i], buffer.getSampleData (i), (int) data.numSamples);

        // clear extra busses..
        if (data.outputs != nullptr)
            for (int i = 1; i < data.numOutputs; ++i)
                for (int f = 0; f < data.outputs[i].numChannels; ++f)
                    FloatVectorOperations::clear (data.outputs[i].channelBuffers32[f], (int) data.numSamples);

       #if JucePlugin_ProducesMidiOutput
        if (data.outputEvents != nullptr)
            MidiEventList::toEventList (*data.outputEvents, midiBuffer);
       #elif JUCE_DEBUG
        /*  This assertion is caused when you've added some events to the
            midiMessages array in your processBlock() method, which usually means
            that you're trying to send them somewhere. But in this case they're
            getting thrown away.

            If your plugin does want to send MIDI messages, you'll need to set
            the JucePlugin_ProducesMidiOutput macro to 1 in your
            JucePluginCharacteristics.h file.

            If you don't want to produce any MIDI output, then you should clear the
            midiMessages array at the end of your processBlock() method, to
            indicate that you don't want any of the events to be passed through
            to the output.
        */
        jassert (midiBuffer.getNumEvents() <= numMidiEventsComingIn);
       #endif

        return kResultTrue;
    }

private:
    //==============================================================================
    Atomic<int> refCount;

    AudioProcessor* pluginInstance;
    ComSmartPtr<Vst::IHostApplication> host;
    ComSmartPtr<JuceAudioProcessor> comPluginInstance;
    ComSmartPtr<JuceVST3EditController> juceVST3EditController;

    /**
        Since VST3 does not provide a way of knowing the buffer size and sample rate at any point,
        this object needs to be copied on every call to process() to be up-to-date...
    */
    Vst::ProcessContext processContext;
    Vst::ProcessSetup processSetup;

    Vst::BusList audioInputs, audioOutputs, eventInputs, eventOutputs;
    MidiBuffer midiBuffer;
    Array<float*> channelList;

    const JuceLibraryRefCount juceCount;

    //==============================================================================
    void addBusTo (Vst::BusList& busList, Vst::Bus* newBus)
    {
        busList.append (IPtr<Vst::Bus> (newBus, false));
    }

    void addAudioBusTo (Vst::BusList& busList, const juce::String& name, Vst::SpeakerArrangement arr)
    {
        addBusTo (busList, new Vst::AudioBus (toString (name), Vst::kMain, Vst::BusInfo::kDefaultActive, arr));
    }

    void addEventBusTo (Vst::BusList& busList, const juce::String& name)
    {
        addBusTo (busList, new Vst::EventBus (toString (name), 16, Vst::kMain, Vst::BusInfo::kDefaultActive));
    }

    Vst::BusList* getBusListFor (Vst::MediaType type, Vst::BusDirection dir)
    {
        if (type == Vst::kAudio)  return dir == Vst::kInput ? &audioInputs : &audioOutputs;
        if (type == Vst::kEvent)  return dir == Vst::kInput ? &eventInputs : &eventOutputs;

        return nullptr;
    }

    //==============================================================================
    enum InternalParameters
    {
        paramPreset = 'prst'
    };

    void preparePlugin (double sampleRate, int bufferSize)
    {
        getPluginInstance().setPlayConfigDetails (JucePlugin_MaxNumInputChannels,
                                                  JucePlugin_MaxNumOutputChannels,
                                                  sampleRate, bufferSize);

        getPluginInstance().prepareToPlay (sampleRate, bufferSize);
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceVST3Component)
};

//==============================================================================
#if JUCE_MSVC
 #pragma warning (push, 0)
 #pragma warning (disable: 4310)
#elif JUCE_CLANG
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-w"
#endif

DECLARE_CLASS_IID (JuceAudioProcessor, 0x0101ABAB, 0xABCDEF01, JucePlugin_ManufacturerCode, JucePlugin_PluginCode)
DEF_CLASS_IID (JuceAudioProcessor)

DECLARE_CLASS_IID (JuceVST3Component, 0xABCDEF01, 0x9182FAEB, JucePlugin_ManufacturerCode, JucePlugin_PluginCode)
DEF_CLASS_IID (JuceVST3Component)

DECLARE_CLASS_IID (JuceVST3EditController, 0xABCDEF01, 0x1234ABCD, JucePlugin_ManufacturerCode, JucePlugin_PluginCode)
DEF_CLASS_IID (JuceVST3EditController)

#if JUCE_MSVC
 #pragma warning (pop)
#elif JUCE_CLANG
 #pragma clang diagnostic pop
#endif

//==============================================================================
bool initModule()
{
   #if JUCE_MAC
    initialiseMac();
   #endif

    return true;
}

bool shutdownModule()
{
    return true;
}

#undef JUCE_EXPORTED_FUNCTION

#if JUCE_WINDOWS
    extern "C" __declspec (dllexport) bool InitDll()   { return initModule(); }
    extern "C" __declspec (dllexport) bool ExitDll()   { return shutdownModule(); }
    #define JUCE_EXPORTED_FUNCTION

#else
    #define JUCE_EXPORTED_FUNCTION extern "C" __attribute__ ((visibility ("default")))

    CFBundleRef globalBundleInstance = nullptr;
    juce::uint32 numBundleRefs = 0;
    juce::Array<CFBundleRef> bundleRefs;

    enum { MaxPathLength = 2048 };
    char modulePath[MaxPathLength] = { 0 };
    void* moduleHandle = nullptr;

    JUCE_EXPORTED_FUNCTION bool bundleEntry (CFBundleRef ref)
    {
        if (ref != nullptr)
        {
            ++numBundleRefs;
            CFRetain (ref);

            bundleRefs.add (ref);

            if (moduleHandle == nullptr)
            {
                globalBundleInstance = ref;
                moduleHandle = ref;

                CFURLRef tempURL = CFBundleCopyBundleURL (ref);
                CFURLGetFileSystemRepresentation (tempURL, true, (UInt8*) modulePath, MaxPathLength);
                CFRelease (tempURL);
            }
        }

        return initModule();
    }

    JUCE_EXPORTED_FUNCTION bool bundleExit()
    {
        if (shutdownModule())
        {
            if (--numBundleRefs == 0)
            {
                for (size_t i = 0; i < bundleRefs.size(); ++i)
                    CFRelease (bundleRefs.getUnchecked (i));

                bundleRefs.clear();
            }

            return true;
        }

        return false;
    }
#endif

//==============================================================================
/** This typedef represents VST3's createInstance() function signature */
typedef FUnknown* (*CreateFunction) (Vst::IHostApplication*);

static FUnknown* createComponentInstance (Vst::IHostApplication* host)
{
    return (Vst::IAudioProcessor*) new JuceVST3Component (host);
}

static FUnknown* createControllerInstance (Vst::IHostApplication* host)
{
    return (Vst::IEditController*) new JuceVST3EditController (host);
}

//==============================================================================
class JucePluginFactory;
JucePluginFactory* globalFactory = nullptr;

//==============================================================================
class JucePluginFactory : public IPluginFactory3
{
public:
    JucePluginFactory()
        : refCount (1),
          factoryInfo (JucePlugin_Manufacturer, JucePlugin_ManufacturerWebsite,
                       JucePlugin_ManufacturerEmail, Vst::kDefaultFactoryFlags)
    {
    }

    virtual ~JucePluginFactory()
    {
        if (globalFactory == this)
            globalFactory = nullptr;
    }

    //==============================================================================
    bool registerClass (const PClassInfo2& info, CreateFunction createFunction)
    {
        if (createFunction == nullptr)
        {
            jassertfalse;
            return false;
        }

        ClassEntry* entry = classes.add (new ClassEntry (info, createFunction));
        entry->infoW.fromAscii (info);

        return true;
    }

    bool isClassRegistered (const FUID& cid) const
    {
        for (int i = 0; i < classes.size(); ++i)
            if (classes.getUnchecked (i)->infoW.cid == cid)
                return true;

        return false;
    }

    //==============================================================================
    JUCE_DECLARE_VST3_COM_REF_METHODS

    tresult PLUGIN_API queryInterface (const TUID iid, void** obj) override
    {
        TEST_FOR_AND_RETURN_IF_VALID (IPluginFactory3)
        TEST_FOR_AND_RETURN_IF_VALID (IPluginFactory2)
        TEST_FOR_AND_RETURN_IF_VALID (IPluginFactory)
        TEST_FOR_AND_RETURN_IF_VALID (FUnknown)

        jassertfalse; // Something new?
        *obj = nullptr;
        return kNotImplemented;
    }

    //==============================================================================
    Steinberg::int32 PLUGIN_API countClasses() override
    {
        return (Steinberg::int32) classes.size();
    }

    tresult PLUGIN_API getFactoryInfo (PFactoryInfo* info) override
    {
        if (info == nullptr)
            return kInvalidArgument;

        memcpy (info, &factoryInfo, sizeof (PFactoryInfo));
        return kResultOk;
    }

    tresult PLUGIN_API getClassInfo (Steinberg::int32 index, PClassInfo* info) override
    {
        return getPClassInfo<PClassInfo> (index, info);
    }

    tresult PLUGIN_API getClassInfo2 (Steinberg::int32 index, PClassInfo2* info) override
    {
        return getPClassInfo<PClassInfo2> (index, info);
    }

    tresult PLUGIN_API getClassInfoUnicode (Steinberg::int32 index, PClassInfoW* info) override
    {
        if (info != nullptr)
        {
            if (ClassEntry* entry = classes[(int) index])
            {
                memcpy (info, &entry->infoW, sizeof (PClassInfoW));
                return kResultOk;
            }
        }

        return kInvalidArgument;
    }

    tresult PLUGIN_API createInstance (FIDString cid, FIDString sourceIid, void** obj) override
    {
        *obj = nullptr;

        FUID sourceFuid = sourceIid;

        if (cid == nullptr || sourceIid == nullptr || ! sourceFuid.isValid())
        {
            jassertfalse; // The host you're running in has severe implementation issues!
            return kInvalidArgument;
        }

        TUID iidToQuery;
        sourceFuid.toTUID (iidToQuery);

        for (int i = 0; i < classes.size(); ++i)
        {
            const ClassEntry& entry = *classes.getUnchecked (i);

            if (doUIDsMatch (entry.infoW.cid, cid))
            {
                if (FUnknown* const instance = entry.createFunction (host))
                {
                    const FReleaser releaser (instance);

                    if (instance->queryInterface (iidToQuery, obj) == kResultOk)
                        return kResultOk;
                }

                break;
            }
        }

        return kNoInterface;
    }

    tresult PLUGIN_API setHostContext (FUnknown* context) override
    {
        host.loadFrom (context);

        if (host != nullptr)
        {
            Vst::String128 name;
            host->getName (name);

            return kResultTrue;
        }

        return kNotImplemented;
    }

private:
    //==============================================================================
    const JuceLibraryRefCount juceCount;
    Atomic<int> refCount;
    const PFactoryInfo factoryInfo;
    ComSmartPtr<Vst::IHostApplication> host;

    //==============================================================================
    struct ClassEntry
    {
        ClassEntry() noexcept  : createFunction (nullptr), isUnicode (false) {}

        ClassEntry (const PClassInfo2& info, CreateFunction fn) noexcept
            : info2 (info), createFunction (fn), isUnicode (false) {}

        PClassInfo2 info2;
        PClassInfoW infoW;
        CreateFunction createFunction;
        bool isUnicode;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClassEntry)
    };

    OwnedArray<ClassEntry> classes;

    //==============================================================================
    template<class PClassInfoType>
    tresult PLUGIN_API getPClassInfo (Steinberg::int32 index, PClassInfoType* info)
    {
        if (info != nullptr)
        {
            zerostruct (*info);

            if (ClassEntry* entry = classes[(int) index])
            {
                if (entry->isUnicode)
                    return kResultFalse;

                memcpy (info, &entry->info2, sizeof (PClassInfoType));
                return kResultOk;
            }
        }

        jassertfalse;
        return kInvalidArgument;
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucePluginFactory)
};

//==============================================================================
#ifndef JucePlugin_Vst3ComponentFlags
   #if JucePlugin_IsSynth
    #define JucePlugin_Vst3ComponentFlags Vst::kSimpleModeSupported
   #else
    #define JucePlugin_Vst3ComponentFlags 0
   #endif
#endif

#ifndef JucePlugin_Vst3Category
   #if JucePlugin_IsSynth
    #define JucePlugin_Vst3Category Vst::PlugType::kInstrumentSynth
   #else
    #define JucePlugin_Vst3Category Vst::PlugType::kFx
   #endif
#endif

//==============================================================================
// The VST3 plugin entry point.
JUCE_EXPORTED_FUNCTION IPluginFactory* PLUGIN_API GetPluginFactory()
{
   #if JUCE_WINDOWS
    // Cunning trick to force this function to be exported. Life's too short to
    // faff around creating .def files for this kind of thing.
    #pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
   #endif

    if (globalFactory == nullptr)
    {
        globalFactory = new JucePluginFactory();

        static const PClassInfo2 componentClass (JuceVST3Component::iid,
                                                 PClassInfo::kManyInstances,
                                                 kVstAudioEffectClass,
                                                 JucePlugin_Name,
                                                 JucePlugin_Vst3ComponentFlags,
                                                 JucePlugin_Vst3Category,
                                                 JucePlugin_Manufacturer,
                                                 JucePlugin_VersionString,
                                                 kVstVersionString);

        globalFactory->registerClass (componentClass, createComponentInstance);

        static const PClassInfo2 controllerClass (JuceVST3EditController::iid,
                                                  PClassInfo::kManyInstances,
                                                  kVstComponentControllerClass,
                                                  JucePlugin_Name,
                                                  JucePlugin_Vst3ComponentFlags,
                                                  JucePlugin_Vst3Category,
                                                  JucePlugin_Manufacturer,
                                                  JucePlugin_VersionString,
                                                  kVstVersionString);

        globalFactory->registerClass (controllerClass, createControllerInstance);
    }
    else
    {
        globalFactory->addRef();
    }

    return dynamic_cast<IPluginFactory*> (globalFactory);
}

#endif //JucePlugin_Build_VST3
