/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#include "../../juce_core/system/juce_TargetPlatform.h"
#include "../utility/juce_CheckSettingMacros.h"

#if JucePlugin_Build_AAX && (JUCE_INCLUDED_AAX_IN_MM || defined (_WIN32) || defined (_WIN64))

#include "../utility/juce_IncludeSystemHeaders.h"
#include "../utility/juce_IncludeModuleHeaders.h"
#include "../utility/juce_WindowsHooks.h"

#ifdef __clang__
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wnon-virtual-dtor"
 #pragma clang diagnostic ignored "-Wsign-conversion"
#endif

#ifdef _MSC_VER
 #pragma warning (push)
 #pragma warning (disable : 4127 4512)
#endif

#include <AAX_Exports.cpp>
#include <AAX_ICollection.h>
#include <AAX_IComponentDescriptor.h>
#include <AAX_IEffectDescriptor.h>
#include <AAX_IPropertyMap.h>
#include <AAX_CEffectParameters.h>
#include <AAX_Errors.h>
#include <AAX_CBinaryTaperDelegate.h>
#include <AAX_CBinaryDisplayDelegate.h>
#include <AAX_CLinearTaperDelegate.h>
#include <AAX_CNumberDisplayDelegate.h>
#include <AAX_CEffectGUI.h>
#include <AAX_IViewContainer.h>
#include <AAX_ITransport.h>
#include <AAX_IMIDINode.h>
#include <AAX_UtilsNative.h>
#include <AAX_Enums.h>

#ifdef _MSC_VER
 #pragma warning (pop)
#endif

#ifdef __clang__
 #pragma clang diagnostic pop
#endif

#if JUCE_WINDOWS
 #ifndef JucePlugin_AAXLibs_path
  #error "You need to define the JucePlugin_AAXLibs_path macro. (This is best done via the Projucer)"
 #endif

 #if JUCE_64BIT
  #define JUCE_AAX_LIB "AAXLibrary_x64"
 #else
  #define JUCE_AAX_LIB "AAXLibrary"
 #endif

 #if JUCE_DEBUG
  #define JUCE_AAX_LIB_PATH   "\\Debug\\"
  #define JUCE_AAX_LIB_SUFFIX "_D"
 #else
  #define JUCE_AAX_LIB_PATH   "\\Release\\"
  #define JUCE_AAX_LIB_SUFFIX ""
 #endif

 #pragma comment(lib, JucePlugin_AAXLibs_path JUCE_AAX_LIB_PATH JUCE_AAX_LIB JUCE_AAX_LIB_SUFFIX ".lib")
#endif

#undef check

namespace juce
{
 #include "juce_AAX_Modifier_Injector.h"
}

const int32_t juceChunkType = 'juce';
const int maxAAXChannels = 8;

//==============================================================================
namespace AAXClasses
{
    static int32 getAAXParamHash (AAX_CParamID paramID) noexcept
    {
        int32 result = 0;

        while (*paramID != 0)
            result = (31 * result) + (*paramID++);

        return result;
    }

    static void check (AAX_Result result)
    {
        jassert (result == AAX_SUCCESS); ignoreUnused (result);
    }

    static bool isBypassParam (AAX_CParamID paramID) noexcept
    {
        return AAX::IsParameterIDEqual (paramID, cDefaultMasterBypassID) != 0;
    }

    // maps a channel index of an AAX format to an index of a juce format
    struct AAXChannelStreamOrder
    {
        AAX_EStemFormat aaxStemFormat;
        AudioChannelSet::ChannelType speakerOrder[8];
    };

    static AAXChannelStreamOrder aaxChannelOrder[] =
    {
        {AAX_eStemFormat_Mono,     {AudioChannelSet::centre, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown}},
        {AAX_eStemFormat_Stereo,   {AudioChannelSet::left, AudioChannelSet::right, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown}},
        {AAX_eStemFormat_LCR,      {AudioChannelSet::left, AudioChannelSet::centre, AudioChannelSet::right, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown}},
        {AAX_eStemFormat_LCRS,     {AudioChannelSet::left, AudioChannelSet::centre, AudioChannelSet::right, AudioChannelSet::centreSurround, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown}},
        {AAX_eStemFormat_Quad,     {AudioChannelSet::left, AudioChannelSet::right,  AudioChannelSet::leftSurround, AudioChannelSet::rightSurround, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown}},
        {AAX_eStemFormat_5_0,      {AudioChannelSet::left, AudioChannelSet::centre, AudioChannelSet::right, AudioChannelSet::leftSurround, AudioChannelSet::rightSurround, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown}},
        {AAX_eStemFormat_5_1,      {AudioChannelSet::left, AudioChannelSet::centre, AudioChannelSet::right, AudioChannelSet::leftSurround, AudioChannelSet::rightSurround, AudioChannelSet::LFE, AudioChannelSet::unknown, AudioChannelSet::unknown}},
        {AAX_eStemFormat_6_0,      {AudioChannelSet::left, AudioChannelSet::centre, AudioChannelSet::right, AudioChannelSet::leftSurround, AudioChannelSet::centreSurround, AudioChannelSet::rightSurround, AudioChannelSet::unknown, AudioChannelSet::unknown}},
        {AAX_eStemFormat_6_1,      {AudioChannelSet::left, AudioChannelSet::centre, AudioChannelSet::right, AudioChannelSet::leftSurround, AudioChannelSet::centreSurround, AudioChannelSet::rightSurround, AudioChannelSet::LFE, AudioChannelSet::unknown}},
        {AAX_eStemFormat_7_0_SDDS, {AudioChannelSet::left, AudioChannelSet::leftCentre, AudioChannelSet::centre, AudioChannelSet::rightCentre, AudioChannelSet::right, AudioChannelSet::leftSurround, AudioChannelSet::rightSurround, AudioChannelSet::unknown}},
        {AAX_eStemFormat_7_0_DTS,  {AudioChannelSet::left, AudioChannelSet::centre, AudioChannelSet::right, AudioChannelSet::leftSurroundSide, AudioChannelSet::rightSurroundSide, AudioChannelSet::leftSurroundRear, AudioChannelSet::rightSurroundRear, AudioChannelSet::unknown}},
        {AAX_eStemFormat_7_1_SDDS, {AudioChannelSet::left, AudioChannelSet::leftCentre, AudioChannelSet::centre, AudioChannelSet::rightCentre, AudioChannelSet::right, AudioChannelSet::leftSurround, AudioChannelSet::rightSurround, AudioChannelSet::LFE}},
        {AAX_eStemFormat_7_1_DTS,  {AudioChannelSet::left, AudioChannelSet::centre, AudioChannelSet::right, AudioChannelSet::leftSurroundSide, AudioChannelSet::rightSurroundSide, AudioChannelSet::leftSurroundRear, AudioChannelSet::rightSurroundRear, AudioChannelSet::LFE}},
        {AAX_eStemFormat_None,     {AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown, AudioChannelSet::unknown}},
    };

    static AAX_EStemFormat aaxFormats[AAX_eStemFormatNum] =
    {
        AAX_eStemFormat_Mono,
        AAX_eStemFormat_Stereo,
        AAX_eStemFormat_LCR,
        AAX_eStemFormat_LCRS,
        AAX_eStemFormat_Quad,
        AAX_eStemFormat_5_0,
        AAX_eStemFormat_5_1,
        AAX_eStemFormat_6_0,
        AAX_eStemFormat_6_1,
        AAX_eStemFormat_7_0_SDDS,
        AAX_eStemFormat_7_1_SDDS,
        AAX_eStemFormat_7_0_DTS,
        AAX_eStemFormat_7_1_DTS
    };

    static AAX_EStemFormat getFormatForAudioChannelSet (const AudioChannelSet& set, bool ignoreLayout) noexcept
    {
        // if the plug-in ignores layout, it is ok to convert between formats only by their numchannnels
        if (ignoreLayout)
        {
            switch (set.size())
            {
                case 0: return AAX_eStemFormat_None;
                case 1: return AAX_eStemFormat_Mono;
                case 2: return AAX_eStemFormat_Stereo;
                case 3: return AAX_eStemFormat_LCR;
                case 4: return AAX_eStemFormat_Quad;
                case 5: return AAX_eStemFormat_5_0;
                case 6: return AAX_eStemFormat_5_1;
                case 7: return AAX_eStemFormat_7_0_DTS;
                case 8: return AAX_eStemFormat_7_1_DTS;
                default:
                    break;
            }

            return AAX_eStemFormat_INT32_MAX;
        }

        if (set == AudioChannelSet::disabled())           return AAX_eStemFormat_None;
        if (set == AudioChannelSet::mono())               return AAX_eStemFormat_Mono;
        if (set == AudioChannelSet::stereo())             return AAX_eStemFormat_Stereo;
        if (set == AudioChannelSet::createLCR())          return AAX_eStemFormat_LCR;
        if (set == AudioChannelSet::createLCRS())         return AAX_eStemFormat_LCRS;
        if (set == AudioChannelSet::quadraphonic())       return AAX_eStemFormat_Quad;
        if (set == AudioChannelSet::create5point0())      return AAX_eStemFormat_5_0;
        if (set == AudioChannelSet::create5point1())      return AAX_eStemFormat_5_1;
        if (set == AudioChannelSet::create6point0())      return AAX_eStemFormat_6_0;
        if (set == AudioChannelSet::create6point1())      return AAX_eStemFormat_6_1;
        if (set == AudioChannelSet::create7point0())      return AAX_eStemFormat_7_0_DTS;
        if (set == AudioChannelSet::create7point1())      return AAX_eStemFormat_7_1_DTS;
        if (set == AudioChannelSet::create7point0SDDS())  return AAX_eStemFormat_7_0_SDDS;
        if (set == AudioChannelSet::create7point1SDDS())  return AAX_eStemFormat_7_1_SDDS;

        return AAX_eStemFormat_INT32_MAX;
    }

    static AudioChannelSet channelSetFromStemFormat (AAX_EStemFormat format, bool ignoreLayout) noexcept
    {
        if (! ignoreLayout)
        {
            switch (format)
            {
                case AAX_eStemFormat_None:     return AudioChannelSet::disabled();
                case AAX_eStemFormat_Mono:     return AudioChannelSet::mono();
                case AAX_eStemFormat_Stereo:   return AudioChannelSet::stereo();
                case AAX_eStemFormat_LCR:      return AudioChannelSet::createLCR();
                case AAX_eStemFormat_LCRS:     return AudioChannelSet::createLCRS();
                case AAX_eStemFormat_Quad:     return AudioChannelSet::quadraphonic();
                case AAX_eStemFormat_5_0:      return AudioChannelSet::create5point0();
                case AAX_eStemFormat_5_1:      return AudioChannelSet::create5point1();
                case AAX_eStemFormat_6_0:      return AudioChannelSet::create6point0();
                case AAX_eStemFormat_6_1:      return AudioChannelSet::create6point1();
                case AAX_eStemFormat_7_0_SDDS: return AudioChannelSet::create7point0SDDS();
                case AAX_eStemFormat_7_0_DTS:  return AudioChannelSet::create7point0();
                case AAX_eStemFormat_7_1_SDDS: return AudioChannelSet::create7point1SDDS();
                case AAX_eStemFormat_7_1_DTS:  return AudioChannelSet::create7point1();
                default:
                    break;
            }

            return AudioChannelSet::disabled();
        }

        return AudioChannelSet::discreteChannels (jmax (0, static_cast<int> (AAX_STEM_FORMAT_CHANNEL_COUNT (format))));
    }

    static Colour getColourFromHighlightEnum (AAX_EHighlightColor colour) noexcept
    {
        switch (colour)
        {
            case AAX_eHighlightColor_Red:       return Colours::red;
            case AAX_eHighlightColor_Blue:      return Colours::blue;
            case AAX_eHighlightColor_Green:     return Colours::green;
            case AAX_eHighlightColor_Yellow:    return Colours::yellow;
            default:                            jassertfalse; break;
        }

        return Colours::black;
    }

    static int juceChannelIndexToAax (int juceIndex, const AudioChannelSet& channelSet)
    {
        AAX_EStemFormat currentLayout = getFormatForAudioChannelSet (channelSet, false);

        int layoutIndex;
        for (layoutIndex = 0; aaxChannelOrder[layoutIndex].aaxStemFormat != currentLayout; ++layoutIndex)
            if (aaxChannelOrder[layoutIndex].aaxStemFormat == 0) return juceIndex;

        const AAXChannelStreamOrder& channelOrder = aaxChannelOrder[layoutIndex];
        const AudioChannelSet::ChannelType channelType = channelSet.getTypeOfChannel (static_cast<int> (juceIndex));

        for (int i = 0; i < 8 && channelOrder.speakerOrder[i] != 0; ++i)
            if (channelOrder.speakerOrder[i] == channelType)
                return i;

        return juceIndex;
    }

    //==============================================================================
    class JuceAAX_Processor;

    struct PluginInstanceInfo
    {
        PluginInstanceInfo (JuceAAX_Processor& p)  : parameters (p) {}

        JuceAAX_Processor& parameters;

        JUCE_DECLARE_NON_COPYABLE (PluginInstanceInfo)
    };

    //==============================================================================
    struct JUCEAlgorithmContext
    {
        float** inputChannels;
        float** outputChannels;
        int32_t* bufferSize;
        int32_t* bypass;

       #if JucePlugin_WantsMidiInput || JucePlugin_IsMidiEffect
        AAX_IMIDINode* midiNodeIn;
       #endif

       #if JucePlugin_ProducesMidiOutput || JucePlugin_IsSynth || JucePlugin_IsMidiEffect
        AAX_IMIDINode* midiNodeOut;
       #endif

        PluginInstanceInfo* pluginInstance;
        int32_t* isPrepared;
        int32_t* sideChainBuffers;

        float* const* meterTapBuffers;
    };

    struct JUCEAlgorithmIDs
    {
        enum
        {
            inputChannels     = AAX_FIELD_INDEX (JUCEAlgorithmContext, inputChannels),
            outputChannels    = AAX_FIELD_INDEX (JUCEAlgorithmContext, outputChannels),
            bufferSize        = AAX_FIELD_INDEX (JUCEAlgorithmContext, bufferSize),
            bypass            = AAX_FIELD_INDEX (JUCEAlgorithmContext, bypass),

           #if JucePlugin_WantsMidiInput || JucePlugin_IsMidiEffect
            midiNodeIn        = AAX_FIELD_INDEX (JUCEAlgorithmContext, midiNodeIn),
           #endif

           #if JucePlugin_ProducesMidiOutput || JucePlugin_IsSynth || JucePlugin_IsMidiEffect
            midiNodeOut       = AAX_FIELD_INDEX (JUCEAlgorithmContext, midiNodeOut),
           #endif

            pluginInstance    = AAX_FIELD_INDEX (JUCEAlgorithmContext, pluginInstance),
            preparedFlag      = AAX_FIELD_INDEX (JUCEAlgorithmContext, isPrepared),

            sideChainBuffers  = AAX_FIELD_INDEX (JUCEAlgorithmContext, sideChainBuffers),

            meterTapBuffers   = AAX_FIELD_INDEX (JUCEAlgorithmContext, meterTapBuffers)
        };
    };

   #if JucePlugin_WantsMidiInput || JucePlugin_IsMidiEffect
    static AAX_IMIDINode* getMidiNodeIn (const JUCEAlgorithmContext& c) noexcept   { return c.midiNodeIn; }
   #else
    static AAX_IMIDINode* getMidiNodeIn (const JUCEAlgorithmContext&) noexcept     { return nullptr; }
   #endif

   #if JucePlugin_ProducesMidiOutput || JucePlugin_IsSynth || JucePlugin_IsMidiEffect
    AAX_IMIDINode* midiNodeOut;
    static AAX_IMIDINode* getMidiNodeOut (const JUCEAlgorithmContext& c) noexcept  { return c.midiNodeOut; }
   #else
    static AAX_IMIDINode* getMidiNodeOut (const JUCEAlgorithmContext&) noexcept    { return nullptr; }
   #endif

    //==============================================================================
    class JuceAAX_Processor;

    class JuceAAX_GUI   : public AAX_CEffectGUI, public ModifierKeyProvider
    {
    public:
        JuceAAX_GUI() {}
        ~JuceAAX_GUI()  { DeleteViewContainer(); }

        static AAX_IEffectGUI* AAX_CALLBACK Create()   { return new JuceAAX_GUI(); }

        void CreateViewContents() override;

        void CreateViewContainer() override
        {
            CreateViewContents();

            if (void* nativeViewToAttachTo = GetViewContainerPtr())
            {
               #if JUCE_MAC
                if (GetViewContainerType() == AAX_eViewContainer_Type_NSView)
               #else
                if (GetViewContainerType() == AAX_eViewContainer_Type_HWND)
               #endif
                {
                    component->setVisible (true);
                    component->addToDesktop (0, nativeViewToAttachTo);

                    if (ModifierKeyReceiver* modReceiver = dynamic_cast<ModifierKeyReceiver*> (component->getPeer()))
                        modReceiver->setModifierKeyProvider (this);
                }
            }
        }

        void DeleteViewContainer() override
        {
            if (component != nullptr)
            {
                JUCE_AUTORELEASEPOOL
                {
                    if (ModifierKeyReceiver* modReceiver = dynamic_cast<ModifierKeyReceiver*> (component->getPeer()))
                        modReceiver->removeModifierKeyProvider();

                    component->removeFromDesktop();
                    component = nullptr;
                }
            }
        }

        AAX_Result GetViewSize (AAX_Point* viewSize) const override
        {
            if (component != nullptr)
            {
                viewSize->horz = (float) component->getWidth();
                viewSize->vert = (float) component->getHeight();
                return AAX_SUCCESS;
            }

            return AAX_ERROR_NULL_OBJECT;
        }

        AAX_Result ParameterUpdated (AAX_CParamID) override
        {
            return AAX_SUCCESS;
        }

        AAX_Result SetControlHighlightInfo (AAX_CParamID paramID, AAX_CBoolean isHighlighted, AAX_EHighlightColor colour) override
        {
            if (component != nullptr && component->pluginEditor != nullptr)
            {
                if (! isBypassParam (paramID))
                {
                    AudioProcessorEditor::ParameterControlHighlightInfo info;
                    info.parameterIndex  = getParamIndexFromID (paramID);
                    info.isHighlighted   = (isHighlighted != 0);
                    info.suggestedColour = getColourFromHighlightEnum (colour);

                    component->pluginEditor->setControlHighlight (info);
                }

                return AAX_SUCCESS;
            }

            return AAX_ERROR_NULL_OBJECT;
        }

        int getWin32Modifiers() const override
        {
            int modifierFlags = 0;

            if (const AAX_IViewContainer* viewContainer = GetViewContainer())
            {
                uint32 aaxViewMods = 0;
                const_cast<AAX_IViewContainer*>(viewContainer)->GetModifiers (&aaxViewMods);

                if ((aaxViewMods & AAX_eModifiers_Shift) != 0) modifierFlags |= ModifierKeys::shiftModifier;
                if ((aaxViewMods & AAX_eModifiers_Alt )  != 0) modifierFlags |= ModifierKeys::altModifier;
            }

            return modifierFlags;
        }

    private:
        //==============================================================================
        int getParamIndexFromID (AAX_CParamID paramID) const noexcept;
        AAX_CParamID getAAXParamIDFromJuceIndex (int index) const noexcept;

        //==============================================================================
        struct ContentWrapperComponent  : public Component
        {
            ContentWrapperComponent (JuceAAX_GUI& gui, AudioProcessor& plugin)
                : owner (gui)
            {
                setOpaque (true);
                setBroughtToFrontOnMouseClick (true);

                addAndMakeVisible (pluginEditor = plugin.createEditorIfNeeded());

                if (pluginEditor != nullptr)
                {
                    setBounds (pluginEditor->getLocalBounds());
                    pluginEditor->addMouseListener (this, true);
                }
            }

            ~ContentWrapperComponent()
            {
                if (pluginEditor != nullptr)
                {
                    PopupMenu::dismissAllActiveMenus();
                    pluginEditor->removeMouseListener (this);
                    pluginEditor->processor.editorBeingDeleted (pluginEditor);
                }
            }

            void paint (Graphics& g) override
            {
                g.fillAll (Colours::black);
            }

            template <typename MethodType>
            void callMouseMethod (const MouseEvent& e, MethodType method)
            {
                if (AAX_IViewContainer* vc = owner.GetViewContainer())
                {
                    const int parameterIndex = pluginEditor->getControlParameterIndex (*e.eventComponent);

                    if (AAX_CParamID aaxParamID = owner.getAAXParamIDFromJuceIndex (parameterIndex))
                    {
                        uint32_t mods = 0;
                        vc->GetModifiers (&mods);

                        (vc->*method) (aaxParamID, mods);
                    }
                }
            }

            void mouseDown (const MouseEvent& e) override  { callMouseMethod (e, &AAX_IViewContainer::HandleParameterMouseDown); }
            void mouseUp   (const MouseEvent& e) override  { callMouseMethod (e, &AAX_IViewContainer::HandleParameterMouseUp); }
            void mouseDrag (const MouseEvent& e) override  { callMouseMethod (e, &AAX_IViewContainer::HandleParameterMouseDrag); }

            void childBoundsChanged (Component*) override
            {
                if (pluginEditor != nullptr)
                {
                    const int w = pluginEditor->getWidth();
                    const int h = pluginEditor->getHeight();
                    setSize (w, h);

                    AAX_Point newSize ((float) h, (float) w);
                    owner.GetViewContainer()->SetViewSize (newSize);
                }
            }

            ScopedPointer<AudioProcessorEditor> pluginEditor;
            JuceAAX_GUI& owner;

           #if JUCE_WINDOWS
            WindowsHooks hooks;
           #endif

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentWrapperComponent)
        };

        ScopedPointer<ContentWrapperComponent> component;

        ScopedJuceInitialiser_GUI libraryInitialiser;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceAAX_GUI)
    };

    static void AAX_CALLBACK algorithmProcessCallback (JUCEAlgorithmContext* const instancesBegin[], const void* const instancesEnd);

    //==============================================================================
    class JuceAAX_Processor   : public AAX_CEffectParameters,
                                public juce::AudioPlayHead,
                                public AudioProcessorListener,
                                private AsyncUpdater
    {
    public:
        JuceAAX_Processor()
            : pluginInstance (createPluginFilterOfType (AudioProcessor::wrapperType_AAX)),
              isPrepared (false),
              sampleRate (0), lastBufferSize (1024), maxBufferSize (1024),
              hasSidechain (false), canDisableSidechain (false)
        {
            pluginInstance->setPlayHead (this);
            pluginInstance->addListener (this);

            rebuildChannelMapArrays();

            AAX_CEffectParameters::GetNumberOfChunks (&juceChunkIndex);
        }

        static AAX_CEffectParameters* AAX_CALLBACK Create()
        {
            PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_AAX;
            return new JuceAAX_Processor();
        }

        AAX_Result Uninitialize() override
        {
            cancelPendingUpdate();

            if (isPrepared && pluginInstance != nullptr)
            {
                isPrepared = false;
                processingSidechainChange.set (0);

                pluginInstance->releaseResources();
            }

            return AAX_CEffectParameters::Uninitialize();
        }

        AAX_Result EffectInit() override
        {
            cancelPendingUpdate();

            AAX_Result err;
            check (Controller()->GetSampleRate (&sampleRate));

            processingSidechainChange.set (0);

            if ((err = preparePlugin()) != AAX_SUCCESS)
                return err;

            addBypassParameter();
            addAudioProcessorParameters();

            return AAX_SUCCESS;
        }

        AAX_Result GetNumberOfChunks (int32_t* numChunks) const override
        {
            // The juceChunk is the last chunk.
            *numChunks = juceChunkIndex + 1;
            return AAX_SUCCESS;
        }

        AAX_Result GetChunkIDFromIndex (int32_t index, AAX_CTypeID* chunkID) const override
        {
            if (index != juceChunkIndex)
                return AAX_CEffectParameters::GetChunkIDFromIndex (index, chunkID);

            *chunkID = juceChunkType;
            return AAX_SUCCESS;
        }

        juce::MemoryBlock& getTemporaryChunkMemory() const
        {
            ScopedLock sl (perThreadDataLock);
            const Thread::ThreadID currentThread = Thread::getCurrentThreadId();

            if (ChunkMemoryBlock::Ptr m = perThreadFilterData [currentThread])
                return m->data;

            ChunkMemoryBlock::Ptr m (new ChunkMemoryBlock());
            perThreadFilterData.set (currentThread, m);
            return m->data;
        }

        AAX_Result GetChunkSize (AAX_CTypeID chunkID, uint32_t* oSize) const override
        {
            if (chunkID != juceChunkType)
                return AAX_CEffectParameters::GetChunkSize (chunkID, oSize);

            juce::MemoryBlock& tempFilterData = getTemporaryChunkMemory();
            tempFilterData.reset();
            pluginInstance->getStateInformation (tempFilterData);

            *oSize = (uint32_t) tempFilterData.getSize();
            return AAX_SUCCESS;
        }

        AAX_Result GetChunk (AAX_CTypeID chunkID, AAX_SPlugInChunk* oChunk) const override
        {
            if (chunkID != juceChunkType)
                return AAX_CEffectParameters::GetChunk (chunkID, oChunk);

            juce::MemoryBlock& tempFilterData = getTemporaryChunkMemory();

            if (tempFilterData.getSize() == 0)
                return 20700; // AAX_ERROR_PLUGIN_API_INVALID_THREAD

            oChunk->fSize = (int32_t) tempFilterData.getSize();
            tempFilterData.copyTo (oChunk->fData, 0, tempFilterData.getSize());
            tempFilterData.reset();

            return AAX_SUCCESS;
        }

        AAX_Result SetChunk (AAX_CTypeID chunkID, const AAX_SPlugInChunk* chunk) override
        {
            if (chunkID != juceChunkType)
                return AAX_CEffectParameters::SetChunk (chunkID, chunk);

            pluginInstance->setStateInformation ((void*) chunk->fData, chunk->fSize);

            // Notify Pro Tools that the parameters were updated.
            // Without it a bug happens in these circumstances:
            // * A preset is saved with the RTAS version of the plugin (".tfx" preset format).
            // * The preset is loaded in PT 10 using the AAX version.
            // * The session is then saved, and closed.
            // * The saved session is loaded, but acting as if the preset was never loaded.
            const int numParameters = pluginInstance->getNumParameters();

            for (int i = 0; i < numParameters; ++i)
                if (AAX_CParamID paramID = getAAXParamIDFromJuceIndex(i))
                    SetParameterNormalizedValue (paramID, (double) pluginInstance->getParameter(i));

            return AAX_SUCCESS;
        }

        AAX_Result ResetFieldData (AAX_CFieldIndex fieldIndex, void* data, uint32_t dataSize) const override
        {
            switch (fieldIndex)
            {
                case JUCEAlgorithmIDs::pluginInstance:
                {
                    const size_t numObjects = dataSize / sizeof (PluginInstanceInfo);
                    PluginInstanceInfo* const objects = static_cast<PluginInstanceInfo*> (data);

                    jassert (numObjects == 1); // not sure how to handle more than one..

                    for (size_t i = 0; i < numObjects; ++i)
                        new (objects + i) PluginInstanceInfo (const_cast<JuceAAX_Processor&> (*this));

                    break;
                }

                case JUCEAlgorithmIDs::preparedFlag:
                {
                    const_cast<JuceAAX_Processor*>(this)->preparePlugin();

                    const size_t numObjects = dataSize / sizeof (uint32_t);
                    uint32_t* const objects = static_cast<uint32_t*> (data);

                    for (size_t i = 0; i < numObjects; ++i)
                        new (objects + i) uint32_t (1);

                    break;
                }
            }

            return AAX_SUCCESS;
        }

        AAX_Result UpdateParameterNormalizedValue (AAX_CParamID paramID, double value, AAX_EUpdateSource source) override
        {
            AAX_Result result = AAX_CEffectParameters::UpdateParameterNormalizedValue (paramID, value, source);

            if (! isBypassParam (paramID))
                pluginInstance->setParameter (getParamIndexFromID (paramID), (float) value);

            return result;
        }

        AAX_Result GetParameterValueFromString (AAX_CParamID paramID, double* result, const AAX_IString& text) const override
        {
            if (isBypassParam (paramID))
            {
                *result = (text.Get()[0] == 'B') ? 1 : 0;
                return AAX_SUCCESS;
            }

            if (AudioProcessorParameter* param = pluginInstance->getParameters() [getParamIndexFromID (paramID)])
            {
                *result = param->getValueForText (text.Get());
                return AAX_SUCCESS;
            }

            return AAX_CEffectParameters::GetParameterValueFromString (paramID, result, text);
        }

        AAX_Result GetParameterStringFromValue (AAX_CParamID paramID, double value, AAX_IString* result, int32_t maxLen) const override
        {
            if (isBypassParam (paramID))
            {
                result->Set (value == 0 ? "Off" : (maxLen >= 8 ? "Bypassed" : "Byp"));
            }
            else
            {
                const int paramIndex = getParamIndexFromID (paramID);
                juce::String text;

                if (AudioProcessorParameter* param = pluginInstance->getParameters() [paramIndex])
                    text = param->getText ((float) value, maxLen);
                else
                    text = pluginInstance->getParameterText (paramIndex, maxLen);

                result->Set (text.toRawUTF8());
            }

            return AAX_SUCCESS;
        }

        AAX_Result GetParameterNumberofSteps (AAX_CParamID paramID, int32_t* result) const
        {
            if (isBypassParam (paramID))
                *result = 2;
            else
                *result = pluginInstance->getParameterNumSteps (getParamIndexFromID (paramID));

            return AAX_SUCCESS;
        }

        AAX_Result GetParameterNormalizedValue (AAX_CParamID paramID, double* result) const override
        {
            if (isBypassParam (paramID))
                return AAX_CEffectParameters::GetParameterNormalizedValue (paramID, result);

            *result = pluginInstance->getParameter (getParamIndexFromID (paramID));
            return AAX_SUCCESS;
        }

        AAX_Result SetParameterNormalizedValue (AAX_CParamID paramID, double newValue) override
        {
            if (isBypassParam (paramID))
                return AAX_CEffectParameters::SetParameterNormalizedValue (paramID, newValue);

            if (AAX_IParameter* p = const_cast<AAX_IParameter*> (mParameterManager.GetParameterByID (paramID)))
                p->SetValueWithFloat ((float) newValue);

            pluginInstance->setParameter (getParamIndexFromID (paramID), (float) newValue);
            return AAX_SUCCESS;
        }

        AAX_Result SetParameterNormalizedRelative (AAX_CParamID paramID, double newDeltaValue) override
        {
            if (isBypassParam (paramID))
                return AAX_CEffectParameters::SetParameterNormalizedRelative (paramID, newDeltaValue);

            const int paramIndex = getParamIndexFromID (paramID);
            const float newValue = pluginInstance->getParameter (paramIndex) + (float) newDeltaValue;
            pluginInstance->setParameter (paramIndex, jlimit (0.0f, 1.0f, newValue));

            if (AAX_IParameter* p = const_cast<AAX_IParameter*> (mParameterManager.GetParameterByID (paramID)))
                p->SetValueWithFloat (newValue);

            return AAX_SUCCESS;
        }

        AAX_Result GetParameterNameOfLength (AAX_CParamID paramID, AAX_IString* result, int32_t maxLen) const override
        {
            if (isBypassParam (paramID))
                result->Set (maxLen >= 13 ? "Master Bypass"
                                          : (maxLen >= 8 ? "Mast Byp"
                                                         : (maxLen >= 6 ? "MstByp" : "MByp")));
            else
                result->Set (pluginInstance->getParameterName (getParamIndexFromID (paramID), maxLen).toRawUTF8());

            return AAX_SUCCESS;
        }

        AAX_Result GetParameterName (AAX_CParamID paramID, AAX_IString* result) const override
        {
            if (isBypassParam (paramID))
                result->Set ("Master Bypass");
            else
                result->Set (pluginInstance->getParameterName (getParamIndexFromID (paramID), 31).toRawUTF8());

            return AAX_SUCCESS;
        }

        AAX_Result GetParameterDefaultNormalizedValue (AAX_CParamID paramID, double* result) const override
        {
            if (! isBypassParam (paramID))
            {
                *result = (double) pluginInstance->getParameterDefaultValue (getParamIndexFromID (paramID));

                jassert (*result >= 0 && *result <= 1.0f);
            }

            return AAX_SUCCESS;
        }

        AudioProcessor& getPluginInstance() const noexcept   { return *pluginInstance; }

        bool getCurrentPosition (juce::AudioPlayHead::CurrentPositionInfo& info) override
        {
            const AAX_ITransport& transport = *Transport();

            info.bpm = 0.0;
            check (transport.GetCurrentTempo (&info.bpm));

            int32_t num = 4, den = 4;
            transport.GetCurrentMeter (&num, &den);
            info.timeSigNumerator   = (int) num;
            info.timeSigDenominator = (int) den;

            info.timeInSamples = 0;

            if (transport.IsTransportPlaying (&info.isPlaying) != AAX_SUCCESS)
                info.isPlaying = false;

            if (info.isPlaying
                 || transport.GetTimelineSelectionStartPosition (&info.timeInSamples) != AAX_SUCCESS)
                check (transport.GetCurrentNativeSampleLocation (&info.timeInSamples));

            info.timeInSeconds = info.timeInSamples / sampleRate;

            int64_t ticks = 0;
            check (transport.GetCurrentTickPosition (&ticks));
            info.ppqPosition = ticks / 960000.0;

            info.isLooping = false;
            int64_t loopStartTick = 0, loopEndTick = 0;
            check (transport.GetCurrentLoopPosition (&info.isLooping, &loopStartTick, &loopEndTick));
            info.ppqLoopStart = loopStartTick / 960000.0;
            info.ppqLoopEnd   = loopEndTick   / 960000.0;

            info.editOriginTime = 0;
            info.frameRate = AudioPlayHead::fpsUnknown;

            AAX_EFrameRate frameRate;
            int32_t offset;

            if (transport.GetTimeCodeInfo (&frameRate, &offset) == AAX_SUCCESS)
            {
                double framesPerSec = 24.0;

                switch (frameRate)
                {
                    case AAX_eFrameRate_Undeclared:    break;
                    case AAX_eFrameRate_24Frame:       info.frameRate = AudioPlayHead::fps24;       break;
                    case AAX_eFrameRate_25Frame:       info.frameRate = AudioPlayHead::fps25;       framesPerSec = 25.0; break;
                    case AAX_eFrameRate_2997NonDrop:   info.frameRate = AudioPlayHead::fps2997;     framesPerSec = 29.97002997; break;
                    case AAX_eFrameRate_2997DropFrame: info.frameRate = AudioPlayHead::fps2997drop; framesPerSec = 29.97002997; break;
                    case AAX_eFrameRate_30NonDrop:     info.frameRate = AudioPlayHead::fps30;       framesPerSec = 30.0; break;
                    case AAX_eFrameRate_30DropFrame:   info.frameRate = AudioPlayHead::fps30drop;   framesPerSec = 30.0; break;
                    case AAX_eFrameRate_23976:         info.frameRate = AudioPlayHead::fps24;       framesPerSec = 23.976; break;
                    default:                           break;
                }

                info.editOriginTime = offset / framesPerSec;
            }

            // No way to get these: (?)
            info.isRecording = false;
            info.ppqPositionOfLastBarStart = 0;

            return true;
        }

        void audioProcessorParameterChanged (AudioProcessor* /*processor*/, int parameterIndex, float newValue) override
        {
            if (AAX_CParamID paramID = getAAXParamIDFromJuceIndex (parameterIndex))
                SetParameterNormalizedValue (paramID, (double) newValue);
        }

        void audioProcessorChanged (AudioProcessor* processor) override
        {
            ++mNumPlugInChanges;
            check (Controller()->SetSignalLatency (processor->getLatencySamples()));
        }

        void audioProcessorParameterChangeGestureBegin (AudioProcessor* /*processor*/, int parameterIndex) override
        {
            if (AAX_CParamID paramID = getAAXParamIDFromJuceIndex (parameterIndex))
                TouchParameter (paramID);
        }

        void audioProcessorParameterChangeGestureEnd (AudioProcessor* /*processor*/, int parameterIndex) override
        {
            if (AAX_CParamID paramID = getAAXParamIDFromJuceIndex (parameterIndex))
                ReleaseParameter (paramID);
        }

        AAX_Result NotificationReceived (AAX_CTypeID type, const void* data, uint32_t size) override
        {
            if (type == AAX_eNotificationEvent_EnteringOfflineMode)  pluginInstance->setNonRealtime (true);
            if (type == AAX_eNotificationEvent_ExitingOfflineMode)   pluginInstance->setNonRealtime (false);

            return AAX_CEffectParameters::NotificationReceived (type, data, size);
        }

        const float* getAudioBufferForInput (const float* const* inputs, const int sidechain, const int mainNumIns, int idx) const noexcept
        {
            jassert (idx < (mainNumIns + 1));

            if (idx < mainNumIns)
                return inputs[inputLayoutMap[idx]];

            return (sidechain != -1 ? inputs[sidechain] : sideChainBuffer.getData());
        }

        void process (const float* const* inputs, float* const* outputs, const int sideChainBufferIdx,
                      const int bufferSize, const bool bypass,
                      AAX_IMIDINode* midiNodeIn, AAX_IMIDINode* midiNodesOut,
                      float* const meterBuffers)
        {
            const int numIns    = pluginInstance->getTotalNumInputChannels();
            const int numOuts   = pluginInstance->getTotalNumOutputChannels();
            const int numMeters = aaxMeters.size();

            const bool processWantsSidechain = (sideChainBufferIdx != -1);
            bool isSuspended = pluginInstance->isSuspended();

            if (processingSidechainChange.get() == 0)
            {
                if (hasSidechain && canDisableSidechain
                 && (sidechainDesired.get() != 0) != processWantsSidechain)
                {
                    isSuspended = true;
                    sidechainDesired.set (processWantsSidechain ? 1 : 0);
                    processingSidechainChange.set (1);
                    triggerAsyncUpdate();
                }
            }
            else
                isSuspended = true;

            if (isSuspended)
            {
                for (int i = 0; i < numOuts; ++i)
                    FloatVectorOperations::clear (outputs[i], bufferSize);

                if (meterBuffers != nullptr)
                    FloatVectorOperations::clear (meterBuffers, numMeters);
            }
            else
            {
                const int mainNumIns = pluginInstance->getMainBusNumInputChannels();
                const int sidechain = (pluginInstance->getChannelCountOfBus (true, 1) > 0 ? sideChainBufferIdx : -1);
                const int numChans = jmax (numIns, numOuts);

                if (numChans == 0) return;

                if (channelList.size() <= numChans)
                    channelList.insertMultiple (-1, nullptr, 1 + numChans - channelList.size());

                float** channels = channelList.getRawDataPointer();

                if (numOuts >= numIns)
                {
                    for (int i = 0; i < numOuts; ++i)
                        channels[i] = outputs[outputLayoutMap[i]];

                    for (int i = 0; i < numIns; ++i)
                        memcpy (channels[i], getAudioBufferForInput (inputs, sidechain, mainNumIns, i), (size_t) bufferSize * sizeof (float));

                    for (int i = numIns; i < numOuts; ++i)
                        zeromem (channels[i], (size_t) bufferSize * sizeof (float));

                    process (channels, numOuts, bufferSize, bypass, midiNodeIn, midiNodesOut);
                }
                else
                {
                    for (int i = 0; i < numOuts; ++i)
                        channels[i] = outputs[outputLayoutMap[i]];

                    for (int i = 0; i < numOuts; ++i)
                        memcpy (channels[i], getAudioBufferForInput (inputs, sidechain, mainNumIns, i), (size_t) bufferSize * sizeof (float));

                    for (int i = numOuts; i < numIns; ++i)
                        channels[i] = const_cast<float*> (getAudioBufferForInput (inputs, sidechain, mainNumIns, i));

                    process (channels, numIns, bufferSize, bypass, midiNodeIn, midiNodesOut);
                }

                if (meterBuffers != nullptr)
                {
                    for (int i = 0; i < numMeters; ++i)
                        meterBuffers[i] = pluginInstance->getParameter (aaxMeters[i]);
                }
            }
        }

        //==============================================================================
        // In aax, the format of the aux and sidechain buses need to be fully determined
        // by the format on the main buses. This function tried to provide such a mapping.
        // Returns false if the in/out main layout is not supported
        static bool fullBusesLayoutFromMainLayout (const AudioProcessor& p,
                                                   const AudioChannelSet& mainInput, const AudioChannelSet& mainOutput,
                                                   AudioProcessor::BusesLayout& fullLayout)
        {
            AudioProcessor::BusesLayout currentLayout = getDefaultLayout (p, true);
            bool success = p.checkBusesLayoutSupported (currentLayout);
            jassert (success);
            ignoreUnused (success);

            const int numInputBuses  = p.getBusCount (true);
            const int numOutputBuses = p.getBusCount (false);

            if (const AudioProcessor::Bus* bus = p.getBus (true, 0))
                if (! bus->isLayoutSupported (mainInput, &currentLayout))
                    return false;

            if (const AudioProcessor::Bus* bus = p.getBus (false, 0))
                if (! bus->isLayoutSupported (mainOutput, &currentLayout))
                    return false;

            // did this change the input again
            if (numInputBuses > 0 && currentLayout.inputBuses.getReference (0) != mainInput)
                return false;

           #ifdef JucePlugin_PreferredChannelConfigurations
            short configs[][2] = {JucePlugin_PreferredChannelConfigurations};
            if (! AudioProcessor::containsLayout (currentLayout, configs))
                return false;
           #endif

            bool foundValid = false;
            {
                AudioProcessor::BusesLayout onlyMains = currentLayout;

                for (int i = 1; i < numInputBuses; ++i)
                    onlyMains.inputBuses.getReference  (i) = AudioChannelSet::disabled();

                for (int i = 1; i < numOutputBuses; ++i)
                    onlyMains.outputBuses.getReference (i) = AudioChannelSet::disabled();

                if (p.checkBusesLayoutSupported (onlyMains))
                {
                    foundValid = true;
                    fullLayout = onlyMains;
                }
            }

            if (numInputBuses > 1)
            {
                // can the first bus be a sidechain or disabled, if not then we can't use this layout combination
                if (const AudioProcessor::Bus* bus = p.getBus (true, 1))
                    if (! bus->isLayoutSupported (AudioChannelSet::mono(), &currentLayout) && ! bus->isLayoutSupported (AudioChannelSet::disabled(), &currentLayout))
                        return foundValid;

                // can all the other inputs be disabled, if not then we can't use this layout combination
                for (int i = 2; i < numInputBuses; ++i)
                    if (const AudioProcessor::Bus* bus = p.getBus (true, i))
                        if (! bus->isLayoutSupported (AudioChannelSet::disabled(), &currentLayout))
                            return foundValid;

                if (const AudioProcessor::Bus* bus = p.getBus (true, 0))
                    if (! bus->isLayoutSupported (mainInput, &currentLayout))
                        return foundValid;

                if (const AudioProcessor::Bus* bus = p.getBus (false, 0))
                    if (! bus->isLayoutSupported (mainOutput, &currentLayout))
                        return foundValid;

                // recheck if the format is correct
                if ((numInputBuses  > 0 && currentLayout.inputBuses .getReference (0) != mainInput)
                 || (numOutputBuses > 0 && currentLayout.outputBuses.getReference (0) != mainOutput))
                    return foundValid;

                const AudioChannelSet& sidechainBus = currentLayout.inputBuses.getReference (1);
                if (sidechainBus != AudioChannelSet::mono() && sidechainBus != AudioChannelSet::disabled())
                    return foundValid;

                for (int i = 2; i < numInputBuses; ++i)
                    if (currentLayout.outputBuses.getReference (i) != AudioChannelSet::disabled())
                        return foundValid;
            }

            const bool hasSidechain = (numInputBuses > 1 && currentLayout.inputBuses.getReference (1) == AudioChannelSet::mono());

            if (hasSidechain)
            {
                AudioProcessor::BusesLayout onlyMainsAndSidechain = currentLayout;

                for (int i = 1; i < numOutputBuses; ++i)
                    onlyMainsAndSidechain.outputBuses.getReference (i) = AudioChannelSet::disabled();

                if (p.checkBusesLayoutSupported (onlyMainsAndSidechain))
                {
                    foundValid = true;
                    fullLayout = onlyMainsAndSidechain;
                }
            }

            if (numOutputBuses > 1)
            {
                AudioProcessor::BusesLayout copy = currentLayout;

                int maxAuxBuses = jmin (16, numOutputBuses);
                for (int i = 1; i < maxAuxBuses; ++i)
                    copy.outputBuses.getReference (i) = mainOutput;

                for (int i = maxAuxBuses; i < numOutputBuses; ++i)
                    copy.outputBuses.getReference (i) = AudioChannelSet::disabled();

                if (p.checkBusesLayoutSupported (copy))
                {
                    fullLayout = copy;
                    foundValid = true;
                }
                else
                {
                    for (int i = 1; i < maxAuxBuses; ++i)
                        if (currentLayout.outputBuses.getReference (i).isDisabled())
                            return foundValid;

                    for (int i = maxAuxBuses; i < numOutputBuses; ++i)
                        if (const AudioProcessor::Bus* bus = p.getBus (false, i))
                            if (! bus->isLayoutSupported (AudioChannelSet::disabled(), &currentLayout))
                                return foundValid;

                    if (const AudioProcessor::Bus* bus = p.getBus (true, 0))
                        if (! bus->isLayoutSupported (mainInput, &currentLayout))
                            return foundValid;

                    if (const AudioProcessor::Bus* bus = p.getBus (false, 0))
                        if (! bus->isLayoutSupported (mainOutput, &currentLayout))
                            return foundValid;

                    if ((numInputBuses  > 0 && currentLayout.inputBuses .getReference (0) != mainInput)
                     || (numOutputBuses > 0 && currentLayout.outputBuses.getReference (0) != mainOutput))
                        return foundValid;

                    if (numInputBuses > 1)
                    {
                        const AudioChannelSet& sidechainBus = currentLayout.inputBuses.getReference (1);
                        if (sidechainBus != AudioChannelSet::mono() && sidechainBus != AudioChannelSet::disabled())
                            return foundValid;
                    }

                    for (int i = maxAuxBuses; i < numOutputBuses; ++i)
                        if (! currentLayout.outputBuses.getReference (i).isDisabled())
                            return foundValid;

                    fullLayout = currentLayout;
                    foundValid = true;
                }
            }

            return foundValid;
        }

    private:
        friend class JuceAAX_GUI;
        friend void AAX_CALLBACK AAXClasses::algorithmProcessCallback (JUCEAlgorithmContext* const instancesBegin[], const void* const instancesEnd);

        void process (float* const* channels, const int numChans, const int bufferSize,
                      const bool bypass, AAX_IMIDINode* midiNodeIn, AAX_IMIDINode* midiNodesOut)
        {
            AudioSampleBuffer buffer (channels, numChans, bufferSize);

            midiBuffer.clear();

            ignoreUnused (midiNodeIn, midiNodesOut);

           #if JucePlugin_WantsMidiInput || JucePlugin_IsMidiEffect
            {
                AAX_CMidiStream* const midiStream = midiNodeIn->GetNodeBuffer();
                const uint32_t numMidiEvents = midiStream->mBufferSize;

                for (uint32_t i = 0; i < numMidiEvents; ++i)
                {
                    const AAX_CMidiPacket& m = midiStream->mBuffer[i];

                    jassert ((int) m.mTimestamp < bufferSize);
                    midiBuffer.addEvent (m.mData, (int) m.mLength,
                                         jlimit (0, (int) bufferSize - 1, (int) m.mTimestamp));
                }
            }
           #endif

            {
                if (lastBufferSize != bufferSize)
                {
                    lastBufferSize = bufferSize;
                    pluginInstance->setRateAndBufferSizeDetails (sampleRate, bufferSize);

                    if (bufferSize > maxBufferSize)
                    {
                        // we only call prepareToPlay here if the new buffer size is larger than
                        // the one used last time prepareToPlay was called.
                        // currently, this should never actually happen, because as of Pro Tools 12,
                        // the maximum possible value is 1024, and we call prepareToPlay with that
                        // value during initialisation.
                        pluginInstance->prepareToPlay (sampleRate, bufferSize);
                        maxBufferSize = bufferSize;
                        sideChainBuffer.calloc (static_cast<size_t> (maxBufferSize));
                    }
                }

                const ScopedLock sl (pluginInstance->getCallbackLock());

                if (bypass)
                    pluginInstance->processBlockBypassed (buffer, midiBuffer);
                else
                    pluginInstance->processBlock (buffer, midiBuffer);
            }

           #if JucePlugin_ProducesMidiOutput || JucePlugin_IsMidiEffect
            {
                const juce::uint8* midiEventData;
                int midiEventSize, midiEventPosition;
                MidiBuffer::Iterator i (midiBuffer);

                AAX_CMidiPacket packet;
                packet.mIsImmediate = false;

                while (i.getNextEvent (midiEventData, midiEventSize, midiEventPosition))
                {
                    jassert (isPositiveAndBelow (midiEventPosition, bufferSize));

                    if (midiEventSize <= 4)
                    {
                        packet.mTimestamp   = (uint32_t) midiEventPosition;
                        packet.mLength      = (uint32_t) midiEventSize;
                        memcpy (packet.mData, midiEventData, (size_t) midiEventSize);

                        check (midiNodesOut->PostMIDIPacket (&packet));
                    }
                }
            }
           #endif
        }

        void addBypassParameter()
        {
            AAX_IParameter* masterBypass = new AAX_CParameter<bool> (cDefaultMasterBypassID,
                                                                     AAX_CString ("Master Bypass"),
                                                                     false,
                                                                     AAX_CBinaryTaperDelegate<bool>(),
                                                                     AAX_CBinaryDisplayDelegate<bool> ("bypass", "on"),
                                                                     true);
            masterBypass->SetNumberOfSteps (2);
            masterBypass->SetType (AAX_eParameterType_Discrete);
            mParameterManager.AddParameter (masterBypass);
            mPacketDispatcher.RegisterPacket (cDefaultMasterBypassID, JUCEAlgorithmIDs::bypass);
        }

        void addAudioProcessorParameters()
        {
            AudioProcessor& audioProcessor = getPluginInstance();

            const int numParameters = audioProcessor.getNumParameters();

           #if JUCE_FORCE_USE_LEGACY_PARAM_IDS
            const bool usingManagedParameters = false;
           #else
            const bool usingManagedParameters = (audioProcessor.getParameters().size() == numParameters);
           #endif

            for (int parameterIndex = 0; parameterIndex < numParameters; ++parameterIndex)
            {
                const AudioProcessorParameter::Category category = audioProcessor.getParameterCategory (parameterIndex);

                aaxParamIDs.add (usingManagedParameters ? audioProcessor.getParameterID (parameterIndex)
                                                        : String (parameterIndex));

                AAX_CString paramName (audioProcessor.getParameterName (parameterIndex, 31).toRawUTF8());
                AAX_CParamID paramID = aaxParamIDs.getReference (parameterIndex).getCharPointer();

                paramMap.set (AAXClasses::getAAXParamHash (paramID), parameterIndex);

                // is this a meter?
                if (((category & 0xffff0000) >> 16) == 2)
                {
                    aaxMeters.add (parameterIndex);
                    continue;
                }

                AAX_IParameter* parameter
                    = new AAX_CParameter<float> (paramID,
                                                 paramName,
                                                 audioProcessor.getParameterDefaultValue (parameterIndex),
                                                 AAX_CLinearTaperDelegate<float, 0>(),
                                                 AAX_CNumberDisplayDelegate<float, 3>(),
                                                 audioProcessor.isParameterAutomatable (parameterIndex));

                parameter->AddShortenedName (audioProcessor.getParameterName (parameterIndex, 4).toRawUTF8());

                const int parameterNumSteps = audioProcessor.getParameterNumSteps (parameterIndex);
                parameter->SetNumberOfSteps ((uint32_t) parameterNumSteps);
                parameter->SetType (parameterNumSteps > 1000 ? AAX_eParameterType_Continuous
                                                             : AAX_eParameterType_Discrete);

                parameter->SetOrientation (audioProcessor.isParameterOrientationInverted (parameterIndex)
                                            ? (AAX_eParameterOrientation_RightMinLeftMax | AAX_eParameterOrientation_TopMinBottomMax
                                                | AAX_eParameterOrientation_RotarySingleDotMode | AAX_eParameterOrientation_RotaryRightMinLeftMax)
                                            : (AAX_eParameterOrientation_LeftMinRightMax | AAX_eParameterOrientation_BottomMinTopMax
                                                | AAX_eParameterOrientation_RotarySingleDotMode | AAX_eParameterOrientation_RotaryLeftMinRightMax));

                mParameterManager.AddParameter (parameter);
            }
        }

        bool getMainBusFormats (AudioChannelSet& inputSet, AudioChannelSet& outputSet)
        {
            AudioProcessor& audioProcessor = getPluginInstance();
           #if ! JucePlugin_IsMidiEffect
            const int inputBuses  = audioProcessor.getBusCount (true);
            const int outputBuses = audioProcessor.getBusCount (false);
           #endif

           #if JucePlugin_IsMidiEffect
            // MIDI effect plug-ins do not support any audio channels
            jassert (audioProcessor.getTotalNumInputChannels()  == 0
                  && audioProcessor.getTotalNumOutputChannels() == 0);

            inputSet = outputSet = AudioChannelSet();

            return true;
           #else
            AAX_EStemFormat inputStemFormat = AAX_eStemFormat_None;
            check (Controller()->GetInputStemFormat (&inputStemFormat));

            AAX_EStemFormat outputStemFormat = AAX_eStemFormat_None;
            check (Controller()->GetOutputStemFormat (&outputStemFormat));

           #if JucePlugin_IsSynth
            if (inputBuses == 0) inputStemFormat = AAX_eStemFormat_None;
           #endif

            inputSet  = (inputBuses  > 0 ? channelSetFromStemFormat (inputStemFormat,  false) : AudioChannelSet());
            outputSet = (outputBuses > 0 ? channelSetFromStemFormat (outputStemFormat, false) : AudioChannelSet());

            if (  (inputSet  == AudioChannelSet::disabled() && inputStemFormat  != AAX_eStemFormat_None)
                || (outputSet == AudioChannelSet::disabled() && outputStemFormat != AAX_eStemFormat_None)
                || (inputSet  != AudioChannelSet::disabled() && inputBuses  == 0)
                || (outputSet != AudioChannelSet::disabled() && outputBuses == 0))
                return false;

            return true;
           #endif
        }

        AAX_Result preparePlugin()
        {
            AudioProcessor& audioProcessor = getPluginInstance();
            AudioProcessor::BusesLayout oldLayout = audioProcessor.getBusesLayout();

            AudioChannelSet inputSet, outputSet;
            if (! getMainBusFormats (inputSet, outputSet))
            {
                if (isPrepared)
                {
                    isPrepared = false;
                    audioProcessor.releaseResources();
                }

                return AAX_ERROR_UNIMPLEMENTED;
            }

            AudioProcessor::BusesLayout newLayout;
            if (! fullBusesLayoutFromMainLayout (audioProcessor, inputSet, outputSet, newLayout))
            {
                if (isPrepared)
                {
                    isPrepared = false;
                    audioProcessor.releaseResources();
                }

                return AAX_ERROR_UNIMPLEMENTED;
            }

            hasSidechain = (newLayout.getNumChannels (true, 1) == 1);
            if (hasSidechain)
            {
                sidechainDesired.set (1);

                AudioProcessor::BusesLayout disabledSidechainLayout (newLayout);
                disabledSidechainLayout.inputBuses.getReference (1) = AudioChannelSet::disabled();

                canDisableSidechain = audioProcessor.checkBusesLayoutSupported (disabledSidechainLayout);

                if (canDisableSidechain)
                {
                    sidechainDesired.set (0);
                    newLayout = disabledSidechainLayout;
                }
            }

            const bool layoutChanged = (oldLayout != newLayout);

            if (layoutChanged)
            {
                if (! audioProcessor.setBusesLayout (newLayout))
                {
                    if (isPrepared)
                    {
                        isPrepared = false;
                        audioProcessor.releaseResources();
                    }

                    return AAX_ERROR_UNIMPLEMENTED;
                }

                rebuildChannelMapArrays();
            }

            if (layoutChanged || (! isPrepared))
            {
                if (isPrepared)
                {
                    isPrepared = false;
                    audioProcessor.releaseResources();
                }

                audioProcessor.setRateAndBufferSizeDetails (sampleRate, lastBufferSize);
                audioProcessor.prepareToPlay (sampleRate, lastBufferSize);
                maxBufferSize = lastBufferSize;

                sideChainBuffer.calloc (static_cast<size_t> (maxBufferSize));
            }

            check (Controller()->SetSignalLatency (audioProcessor.getLatencySamples()));

            isPrepared = true;

            return AAX_SUCCESS;
        }

        void rebuildChannelMapArrays()
        {
            AudioProcessor& audioProcessor = getPluginInstance();

            for (int dir = 0; dir < 2; ++dir)
            {
                const bool isInput = (dir == 0);
                Array<int>& layoutMap = isInput ? inputLayoutMap : outputLayoutMap;
                layoutMap.clear();

                const int n  = audioProcessor.getBusCount (isInput);

                int chOffset = 0;
                for (int busIdx = 0; busIdx < n; ++busIdx)
                {
                    const AudioChannelSet channelFormat = audioProcessor.getChannelLayoutOfBus (isInput, busIdx);

                    if (channelFormat != AudioChannelSet::disabled())
                    {
                        const int numChannels = channelFormat.size();

                        for (int ch = 0; ch < numChannels; ++ch)
                            layoutMap.add (juceChannelIndexToAax (ch, channelFormat) + chOffset);

                        chOffset += numChannels;
                    }
                }
            }
        }

        static void algorithmCallback (JUCEAlgorithmContext* const instancesBegin[], const void* const instancesEnd)
        {

            for (JUCEAlgorithmContext* const* iter = instancesBegin; iter < instancesEnd; ++iter)
            {
                const JUCEAlgorithmContext& i = **iter;

                int sideChainBufferIdx = i.pluginInstance->parameters.hasSidechain && i.sideChainBuffers != nullptr
                                             ? static_cast<int> (*i.sideChainBuffers) : -1;

                // sidechain index of zero is an invalid index
                if (sideChainBufferIdx <= 0)
                    sideChainBufferIdx = -1;

                const int numMeters = i.pluginInstance->parameters.aaxMeters.size();

                float* const meterTapBuffers = (i.meterTapBuffers != nullptr && numMeters > 0 ? *i.meterTapBuffers : nullptr);

                i.pluginInstance->parameters.process (i.inputChannels, i.outputChannels, sideChainBufferIdx,
                                                      *(i.bufferSize), *(i.bypass) != 0,
                                                      getMidiNodeIn(i), getMidiNodeOut(i),
                                                      meterTapBuffers);
            }
        }
        //==============================================================================
        void handleAsyncUpdate() override
        {
            if (processingSidechainChange.get() == 0)
                return;

            AudioProcessor& audioProcessor = getPluginInstance();

            const bool sidechainActual = (audioProcessor.getChannelCountOfBus (true, 1) > 0);
            if (hasSidechain && canDisableSidechain && (sidechainDesired.get() != 0) != sidechainActual)
            {
                if (isPrepared)
                {
                    isPrepared = false;
                    audioProcessor.releaseResources();
                }

                if (AudioProcessor::Bus* bus = audioProcessor.getBus (true, 1))
                    bus->setCurrentLayout (sidechainDesired.get() != 0 ? AudioChannelSet::mono() : AudioChannelSet::disabled());

                audioProcessor.prepareToPlay (audioProcessor.getSampleRate(), audioProcessor.getBlockSize());

                isPrepared = true;
            }

            processingSidechainChange.set (0);
        }

        //==============================================================================
        inline int getParamIndexFromID (AAX_CParamID paramID) const noexcept
        {
            return paramMap [AAXClasses::getAAXParamHash (paramID)];
        }

        inline AAX_CParamID getAAXParamIDFromJuceIndex (int index) const noexcept
        {
            if (! isPositiveAndBelow (index, aaxParamIDs.size())) return nullptr;

            return aaxParamIDs.getReference (index).getCharPointer();
        }

        //==============================================================================
        static AudioProcessor::BusesLayout getDefaultLayout (const AudioProcessor& p, bool enableAll)
        {
            AudioProcessor::BusesLayout defaultLayout;

            for (int dir = 0; dir < 2; ++dir)
            {
                const bool isInput = (dir == 0);
                const int n = p.getBusCount (isInput);
                Array<AudioChannelSet>& layouts = (isInput ? defaultLayout.inputBuses : defaultLayout.outputBuses);

                for (int i = 0; i < n; ++i)
                    if (const AudioProcessor::Bus* bus = p.getBus (isInput, i))
                        layouts.add (enableAll || bus->isEnabledByDefault() ? bus->getDefaultLayout() : AudioChannelSet());
            }

            return defaultLayout;
        }

        static AudioProcessor::BusesLayout getDefaultLayout (AudioProcessor& p)
        {
            AudioProcessor::BusesLayout defaultLayout;

            defaultLayout = getDefaultLayout (p, true);

            if (! p.checkBusesLayoutSupported (defaultLayout))
                defaultLayout = getDefaultLayout (p, false);

            // Your processor must support the default layout
            jassert (p.checkBusesLayoutSupported (defaultLayout));
            return defaultLayout;
        }

        //==============================================================================
        ScopedJuceInitialiser_GUI libraryInitialiser;

        ScopedPointer<AudioProcessor> pluginInstance;

        bool isPrepared;
        MidiBuffer midiBuffer;
        Array<float*> channelList;
        int32_t juceChunkIndex;
        AAX_CSampleRate sampleRate;
        int lastBufferSize, maxBufferSize;
        bool hasSidechain, canDisableSidechain;

        Atomic<int> processingSidechainChange, sidechainDesired;

        HeapBlock<float> sideChainBuffer;
        Array<int> inputLayoutMap, outputLayoutMap;

        Array<String> aaxParamIDs;
        HashMap<int32, int> paramMap;

        Array<int> aaxMeters;

        struct ChunkMemoryBlock  : public ReferenceCountedObject
        {
            juce::MemoryBlock data;

            typedef ReferenceCountedObjectPtr<ChunkMemoryBlock> Ptr;
        };

        // temporary filter data is generated in GetChunkSize
        // and the size of the data returned. To avoid generating
        // it again in GetChunk, we need to store it somewhere.
        // However, as GetChunkSize and GetChunk can be called
        // on different threads, we store it in thread dependant storage
        // in a hash map with the thread id as a key.
        mutable HashMap<Thread::ThreadID, ChunkMemoryBlock::Ptr> perThreadFilterData;
        CriticalSection perThreadDataLock;

        JUCE_DECLARE_NON_COPYABLE (JuceAAX_Processor)
    };

    //==============================================================================
    void JuceAAX_GUI::CreateViewContents()
    {
        if (component == nullptr)
        {
            if (JuceAAX_Processor* params = dynamic_cast<JuceAAX_Processor*> (GetEffectParameters()))
                component = new ContentWrapperComponent (*this, params->getPluginInstance());
            else
                jassertfalse;
        }
    }

    int JuceAAX_GUI::getParamIndexFromID (AAX_CParamID paramID) const noexcept
    {
        if (const JuceAAX_Processor* params = dynamic_cast<const JuceAAX_Processor*> (GetEffectParameters()))
            return params->getParamIndexFromID (paramID);

        return -1;
    }

    AAX_CParamID JuceAAX_GUI::getAAXParamIDFromJuceIndex (int index) const noexcept
    {
        if (const JuceAAX_Processor* params = dynamic_cast<const JuceAAX_Processor*> (GetEffectParameters()))
            return params->getAAXParamIDFromJuceIndex (index);

        return nullptr;
    }

    //==============================================================================
    struct AAXFormatConfiguration
    {
        AAXFormatConfiguration() noexcept
            : inputFormat (AAX_eStemFormat_None), outputFormat (AAX_eStemFormat_None) {}

        AAXFormatConfiguration (AAX_EStemFormat inFormat, AAX_EStemFormat outFormat) noexcept
            : inputFormat (inFormat), outputFormat (outFormat) {}


        AAX_EStemFormat inputFormat, outputFormat;

        bool operator== (const AAXFormatConfiguration other) const noexcept { return (inputFormat == other.inputFormat) && (outputFormat == other.outputFormat); }
        bool operator< (const AAXFormatConfiguration other) const noexcept
        {
            return (inputFormat == other.inputFormat) ? (outputFormat < other.outputFormat) : (inputFormat < other.inputFormat);
        }
    };

    //==============================================================================
    static int addAAXMeters (AudioProcessor& p, AAX_IEffectDescriptor& descriptor)
    {
        const int n = p.getNumParameters();

        int meterIdx = 0;
        for (int i = 0; i < n; ++i)
        {
            const AudioProcessorParameter::Category category = p.getParameterCategory (i);

            // is this a meter?
            if (((category & 0xffff0000) >> 16) == 2)
            {
                if (AAX_IPropertyMap* meterProperties = descriptor.NewPropertyMap())
                {
                    AAX_EMeterType aaxMeterType;

                    switch (category)
                    {
                        case AudioProcessorParameter::inputMeter:
                            aaxMeterType = AAX_eMeterType_Input;
                            break;
                        case AudioProcessorParameter::outputMeter:
                            aaxMeterType = AAX_eMeterType_Output;
                            break;
                        case AudioProcessorParameter::compressorLimiterGainReductionMeter:
                            aaxMeterType = AAX_eMeterType_CLGain;
                            break;
                        case AudioProcessorParameter::expanderGateGainReductionMeter:
                            aaxMeterType = AAX_eMeterType_EGGain;
                            break;
                        case AudioProcessorParameter::analysisMeter:
                            aaxMeterType = AAX_eMeterType_Analysis;
                            break;
                        default:
                            aaxMeterType = AAX_eMeterType_Other;
                    }

                    meterProperties->AddProperty (AAX_eProperty_Meter_Type,        aaxMeterType);
                    meterProperties->AddProperty (AAX_eProperty_Meter_Orientation, AAX_eMeterOrientation_TopRight);
                    descriptor.AddMeterDescription ('Metr' + static_cast<AAX_CTypeID> (meterIdx++),
                                                    p.getParameterName (i).toRawUTF8(), meterProperties);
                }
            }
        }

        return meterIdx;
    }

    static void createDescriptor (AAX_IComponentDescriptor& desc, int configIndex,
                                  const AudioProcessor::BusesLayout& fullLayout, AudioProcessor& processor,
                                  const int numMeters)
    {
        AAX_EStemFormat aaxInputFormat  = getFormatForAudioChannelSet (fullLayout.getMainInputChannelSet(),  false);
        AAX_EStemFormat aaxOutputFormat = getFormatForAudioChannelSet (fullLayout.getMainOutputChannelSet(), false);

       #if JucePlugin_IsSynth
        if (aaxInputFormat == AAX_eStemFormat_None)
            aaxInputFormat = aaxOutputFormat;
       #endif

       #if JucePlugin_IsMidiEffect
        aaxInputFormat = aaxOutputFormat = AAX_eStemFormat_Mono;
       #endif

        check (desc.AddAudioIn  (JUCEAlgorithmIDs::inputChannels));
        check (desc.AddAudioOut (JUCEAlgorithmIDs::outputChannels));

        check (desc.AddAudioBufferLength (JUCEAlgorithmIDs::bufferSize));
        check (desc.AddDataInPort (JUCEAlgorithmIDs::bypass, sizeof (int32_t)));

       #if JucePlugin_WantsMidiInput || JucePlugin_IsMidiEffect
        check (desc.AddMIDINode (JUCEAlgorithmIDs::midiNodeIn, AAX_eMIDINodeType_LocalInput,
                                 JucePlugin_Name, 0xffff));
       #endif

       #if JucePlugin_ProducesMidiOutput || JucePlugin_IsSynth || JucePlugin_IsMidiEffect
        check (desc.AddMIDINode (JUCEAlgorithmIDs::midiNodeOut, AAX_eMIDINodeType_LocalOutput,
                                 JucePlugin_Name " Out", 0xffff));
       #endif

        check (desc.AddPrivateData (JUCEAlgorithmIDs::pluginInstance, sizeof (PluginInstanceInfo)));
        check (desc.AddPrivateData (JUCEAlgorithmIDs::preparedFlag, sizeof (int32_t)));

        if (numMeters > 0)
        {
            HeapBlock<AAX_CTypeID> meterIDs (static_cast<size_t> (numMeters));

            for (int i = 0; i < numMeters; ++i)
                meterIDs[i] = 'Metr' + static_cast<AAX_CTypeID> (i);

            check (desc.AddMeters (JUCEAlgorithmIDs::meterTapBuffers, meterIDs.getData(), static_cast<uint32_t> (numMeters)));
        }

        // Create a property map
        AAX_IPropertyMap* const properties = desc.NewPropertyMap();
        jassert (properties != nullptr);

        properties->AddProperty (AAX_eProperty_ManufacturerID,      JucePlugin_AAXManufacturerCode);
        properties->AddProperty (AAX_eProperty_ProductID,           JucePlugin_AAXProductId);

       #if JucePlugin_AAXDisableBypass
        properties->AddProperty (AAX_eProperty_CanBypass,           false);
       #else
        properties->AddProperty (AAX_eProperty_CanBypass,           true);
       #endif

        properties->AddProperty (AAX_eProperty_InputStemFormat,     static_cast<AAX_CPropertyValue> (aaxInputFormat));
        properties->AddProperty (AAX_eProperty_OutputStemFormat,    static_cast<AAX_CPropertyValue> (aaxOutputFormat));

        // This value needs to match the RTAS wrapper's Type ID, so that
        // the host knows that the RTAS/AAX plugins are equivalent.
        properties->AddProperty (AAX_eProperty_PlugInID_Native,     'jcaa' + configIndex);

       #if ! JucePlugin_AAXDisableAudioSuite
        properties->AddProperty (AAX_eProperty_PlugInID_AudioSuite, 'jyaa' + configIndex);
       #endif

       #if JucePlugin_AAXDisableMultiMono
        properties->AddProperty (AAX_eProperty_Constraint_MultiMonoSupport, false);
       #else
        properties->AddProperty (AAX_eProperty_Constraint_MultiMonoSupport, true);
       #endif

       #if JucePlugin_AAXDisableDynamicProcessing
        properties->AddProperty (AAX_eProperty_Constraint_AlwaysProcess, true);
       #endif

       #if JucePlugin_AAXDisableSaveRestore
        properties->AddProperty (AAX_eProperty_SupportsSaveRestore, false);
       #endif

        if (fullLayout.getChannelSet (true, 1) == AudioChannelSet::mono())
        {
            check (desc.AddSideChainIn (JUCEAlgorithmIDs::sideChainBuffers));
            properties->AddProperty (AAX_eProperty_SupportsSideChainInput, true);
        }

        const int maxAuxBuses = jmax (0, jmin (15, fullLayout.outputBuses.size() - 1));

        // add the output buses
        // This is incrdibly dumb: the output bus format must be well defined
        // for every main bus in/out format pair. This means that there cannot
        // be two configurations with different aux formats but
        // identical main bus in/out formats.
        for (int busIdx = 1; busIdx < maxAuxBuses + 1; ++busIdx)
        {
            const AudioChannelSet& set = fullLayout.getChannelSet (false, busIdx);
            if (set.isDisabled()) break;

            AAX_EStemFormat auxFormat  = getFormatForAudioChannelSet (set, true);
            if (auxFormat != AAX_eStemFormat_INT32_MAX && auxFormat != AAX_eStemFormat_None)
            {
                const String& name = processor.getBus (false, busIdx)->getName();
                check (desc.AddAuxOutputStem (0, static_cast<int32_t> (auxFormat), name.toRawUTF8()));
            }
        }

        check (desc.AddProcessProc_Native (algorithmProcessCallback, properties));
    }

    static void getPlugInDescription (AAX_IEffectDescriptor& descriptor)
    {
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_AAX;
        ScopedPointer<AudioProcessor> plugin = createPluginFilterOfType (AudioProcessor::wrapperType_AAX);
        const int numInputBuses  = plugin->getBusCount (true);
        const int numOutputBuses = plugin->getBusCount (false);

        descriptor.AddName (JucePlugin_Desc);
        descriptor.AddName (JucePlugin_Name);
        descriptor.AddCategory (JucePlugin_AAXCategory);

        const int numMeters = addAAXMeters (*plugin, descriptor);

       #ifdef JucePlugin_AAXPageTableFile
        // optional page table setting - define this macro in your project if you want
        // to set this value - see Avid documentation for details about its format.
        descriptor.AddResourceInfo (AAX_eResourceType_PageTable, JucePlugin_AAXPageTableFile);
       #endif

        check (descriptor.AddProcPtr ((void*) JuceAAX_GUI::Create,        kAAX_ProcPtrID_Create_EffectGUI));
        check (descriptor.AddProcPtr ((void*) JuceAAX_Processor::Create,  kAAX_ProcPtrID_Create_EffectParameters));

       #if JucePlugin_IsMidiEffect
        // MIDI effect plug-ins do not support any audio channels
        jassert (numInputBuses == 0 && numOutputBuses == 0);

        if (AAX_IComponentDescriptor* const desc = descriptor.NewComponentDescriptor())
        {
            createDescriptor (*desc, 0, plugin->getBusesLayout(), *plugin, numMeters);
            check (descriptor.AddComponent (desc));
        }

       #else
        int configIndex = 0;

        const int numIns  = numInputBuses  > 0 ? AAX_eStemFormatNum : 0;
        const int numOuts = numOutputBuses > 0 ? AAX_eStemFormatNum : 0;

        for (int inIdx = 0; inIdx < jmax (numIns, 1); ++inIdx)
        {
            AAX_EStemFormat aaxInFormat  = numIns > 0 ? aaxFormats[inIdx] : AAX_eStemFormat_None;
            AudioChannelSet inLayout  = channelSetFromStemFormat (aaxInFormat, false);

            for (int outIdx = 0; outIdx < jmax (numOuts, 1); ++outIdx)
            {
                AAX_EStemFormat aaxOutFormat = numOuts > 0 ? aaxFormats[outIdx] : AAX_eStemFormat_None;
                AudioChannelSet outLayout = channelSetFromStemFormat (aaxOutFormat, false);

                AudioProcessor::BusesLayout fullLayout;
                if (! JuceAAX_Processor::fullBusesLayoutFromMainLayout (*plugin, inLayout, outLayout, fullLayout))
                    continue;

                if (AAX_IComponentDescriptor* const desc = descriptor.NewComponentDescriptor())
                {
                    createDescriptor (*desc, configIndex++, fullLayout, *plugin, numMeters);
                    check (descriptor.AddComponent (desc));
                }
            }
        }

        // You don't have any supported layouts
        jassert (configIndex > 0);
       #endif
    }
}

void AAX_CALLBACK AAXClasses::algorithmProcessCallback (JUCEAlgorithmContext* const instancesBegin[], const void* const instancesEnd)
{
    AAXClasses::JuceAAX_Processor::algorithmCallback (instancesBegin, instancesEnd);
}

//==============================================================================
AAX_Result JUCE_CDECL GetEffectDescriptions (AAX_ICollection*);
AAX_Result JUCE_CDECL GetEffectDescriptions (AAX_ICollection* collection)
{
    ScopedJuceInitialiser_GUI libraryInitialiser;

    if (AAX_IEffectDescriptor* const descriptor = collection->NewDescriptor())
    {
        AAXClasses::getPlugInDescription (*descriptor);
        collection->AddEffect (JUCE_STRINGIFY (JucePlugin_AAXIdentifier), descriptor);

        collection->SetManufacturerName (JucePlugin_Manufacturer);
        collection->AddPackageName (JucePlugin_Desc);
        collection->AddPackageName (JucePlugin_Name);
        collection->SetPackageVersion (JucePlugin_VersionCode);

        return AAX_SUCCESS;
    }

    return AAX_ERROR_NULL_OBJECT;
}

#endif
