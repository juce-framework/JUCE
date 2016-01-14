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
#include "../utility/juce_PluginBusUtilities.h"
#undef Component

#ifdef __clang__
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wnon-virtual-dtor"
 #pragma clang diagnostic ignored "-Wsign-conversion"
#endif

#ifdef _MSC_VER
 #pragma warning (push)
 #pragma warning (disable : 4127)
#endif

#include "AAX_Exports.cpp"
#include "AAX_ICollection.h"
#include "AAX_IComponentDescriptor.h"
#include "AAX_IEffectDescriptor.h"
#include "AAX_IPropertyMap.h"
#include "AAX_CEffectParameters.h"
#include "AAX_Errors.h"
#include "AAX_CBinaryTaperDelegate.h"
#include "AAX_CBinaryDisplayDelegate.h"
#include "AAX_CLinearTaperDelegate.h"
#include "AAX_CNumberDisplayDelegate.h"
#include "AAX_CEffectGUI.h"
#include "AAX_IViewContainer.h"
#include "AAX_ITransport.h"
#include "AAX_IMIDINode.h"
#include "AAX_UtilsNative.h"
#include "AAX_Enums.h"

#ifdef _MSC_VER
 #pragma warning (pop)
#endif

#ifdef __clang__
 #pragma clang diagnostic pop
#endif

#if JUCE_WINDOWS
 #ifndef JucePlugin_AAXLibs_path
  #error "You need to define the JucePlugin_AAXLibs_path macro. (This is best done via the introjucer)"
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

using juce::Component;

const int32_t juceChunkType = 'juce';

//==============================================================================
struct AAXClasses
{
    static void check (AAX_Result result)
    {
        jassert (result == AAX_SUCCESS); ignoreUnused (result);
    }

    static int getParamIndexFromID (AAX_CParamID paramID) noexcept
    {
        return atoi (paramID);
    }

    static bool isBypassParam (AAX_CParamID paramID) noexcept
    {
        return AAX::IsParameterIDEqual (paramID, cDefaultMasterBypassID) != 0;
    }

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
        if (set == AudioChannelSet::createFront7point0()) return AAX_eStemFormat_7_0_SDDS;
        if (set == AudioChannelSet::createFront7point1()) return AAX_eStemFormat_7_1_SDDS;

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
                case AAX_eStemFormat_7_0_SDDS: return AudioChannelSet::createFront7point0();
                case AAX_eStemFormat_7_0_DTS:  return AudioChannelSet::create7point0();
                case AAX_eStemFormat_7_1_SDDS: return AudioChannelSet::createFront7point1();
                case AAX_eStemFormat_7_1_DTS:  return AudioChannelSet::create7point1();
                default:
                    break;
            }

            return AudioChannelSet::disabled();
        }

        return AudioChannelSet::discreteChannels (jmax (0, static_cast<int> (AAX_STEM_FORMAT_CHANNEL_COUNT (format))));
    }

    static const char* getSpeakerArrangementString (AAX_EStemFormat format) noexcept
    {
        switch (format)
        {
            case AAX_eStemFormat_Mono:      return "M";
            case AAX_eStemFormat_Stereo:    return "L R";
            case AAX_eStemFormat_LCR:       return "L C R";
            case AAX_eStemFormat_LCRS:      return "L C R S";
            case AAX_eStemFormat_Quad:      return "L R Ls Rs";
            case AAX_eStemFormat_5_0:       return "L C R Ls Rs";
            case AAX_eStemFormat_5_1:       return "L C R Ls Rs LFE";
            case AAX_eStemFormat_6_0:       return "L C R Ls Cs Rs";
            case AAX_eStemFormat_6_1:       return "L C R Ls Cs Rs LFE";
            case AAX_eStemFormat_7_0_SDDS:  return "L Lc C Rc R Ls Rs";
            case AAX_eStemFormat_7_1_SDDS:  return "L Lc C Rc R Ls Rs LFE";
            case AAX_eStemFormat_7_0_DTS:   return "L C R Lss Rss Lsr Rsr";
            case AAX_eStemFormat_7_1_DTS:   return "L C R Lss Rss Lsr Rsr LFE";
            default:                        break;
        }

        return nullptr;
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
    };

    struct JUCEAlgorithmIDs
    {
        enum
        {
            inputChannels   = AAX_FIELD_INDEX (JUCEAlgorithmContext, inputChannels),
            outputChannels  = AAX_FIELD_INDEX (JUCEAlgorithmContext, outputChannels),
            bufferSize      = AAX_FIELD_INDEX (JUCEAlgorithmContext, bufferSize),
            bypass          = AAX_FIELD_INDEX (JUCEAlgorithmContext, bypass),

           #if JucePlugin_WantsMidiInput || JucePlugin_IsMidiEffect
            midiNodeIn      = AAX_FIELD_INDEX (JUCEAlgorithmContext, midiNodeIn),
           #endif

           #if JucePlugin_ProducesMidiOutput || JucePlugin_IsSynth || JucePlugin_IsMidiEffect
            midiNodeOut     = AAX_FIELD_INDEX (JUCEAlgorithmContext, midiNodeOut),
           #endif

            pluginInstance  = AAX_FIELD_INDEX (JUCEAlgorithmContext, pluginInstance),
            preparedFlag    = AAX_FIELD_INDEX (JUCEAlgorithmContext, isPrepared),

            sideChainBuffers  = AAX_FIELD_INDEX (JUCEAlgorithmContext, sideChainBuffers)
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
    class JuceAAX_GUI   : public AAX_CEffectGUI
    {
    public:
        JuceAAX_GUI() {}
        ~JuceAAX_GUI()  { DeleteViewContainer(); }

        static AAX_IEffectGUI* AAX_CALLBACK Create()   { return new JuceAAX_GUI(); }

        void CreateViewContents() override
        {
            if (component == nullptr)
            {
                if (JuceAAX_Processor* params = dynamic_cast<JuceAAX_Processor*> (GetEffectParameters()))
                    component = new ContentWrapperComponent (*this, params->getPluginInstance());
                else
                    jassertfalse;
            }
        }

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
                }
            }
        }

        void DeleteViewContainer() override
        {
            if (component != nullptr)
            {
                JUCE_AUTORELEASEPOOL
                {
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

    private:
        struct ContentWrapperComponent  : public juce::Component
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

                    if (parameterIndex >= 0)
                    {
                        uint32_t mods = 0;
                        vc->GetModifiers (&mods);
                        (vc->*method) (IndexAsParamID (parameterIndex), mods);
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

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentWrapperComponent)
        };

        ScopedPointer<ContentWrapperComponent> component;

        ScopedJuceInitialiser_GUI libraryInitialiser;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceAAX_GUI)
    };

    //==============================================================================
    class JuceAAX_Processor   : public AAX_CEffectParameters,
                                public juce::AudioPlayHead,
                                public AudioProcessorListener
    {
    public:
        JuceAAX_Processor()  : pluginInstance (createPluginFilterOfType (AudioProcessor::wrapperType_AAX)),
                               busUtils (*pluginInstance, false),
                               sampleRate (0), lastBufferSize (1024), maxBufferSize (1024),
                               hasSidechain (false)
        {
            pluginInstance->setPlayHead (this);
            pluginInstance->addListener (this);

            busUtils.findAllCompatibleLayouts();

            AAX_CEffectParameters::GetNumberOfChunks (&juceChunkIndex);
        }

        static AAX_CEffectParameters* AAX_CALLBACK Create()   { return new JuceAAX_Processor(); }

        AAX_Result EffectInit() override
        {
            AAX_Result err;

            check (Controller()->GetSampleRate (&sampleRate));

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
                return 20700 /*AAX_ERROR_PLUGIN_API_INVALID_THREAD*/;

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
                SetParameterNormalizedValue (IndexAsParamID (i), (double) pluginInstance->getParameter(i));

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
            SetParameterNormalizedValue (IndexAsParamID (parameterIndex), (double) newValue);
        }

        void audioProcessorChanged (AudioProcessor* processor) override
        {
            ++mNumPlugInChanges;
            check (Controller()->SetSignalLatency (processor->getLatencySamples()));
        }

        void audioProcessorParameterChangeGestureBegin (AudioProcessor* /*processor*/, int parameterIndex) override
        {
            TouchParameter (IndexAsParamID (parameterIndex));
        }

        void audioProcessorParameterChangeGestureEnd (AudioProcessor* /*processor*/, int parameterIndex) override
        {
            ReleaseParameter (IndexAsParamID (parameterIndex));
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
                return inputs[idx];

            return (sidechain != -1 ? inputs[sidechain] : sideChainBuffer.getData());
        }

        void process (const float* const* inputs, float* const* outputs, const int sideChainBufferIdx,
                      const int bufferSize, const bool bypass,
                      AAX_IMIDINode* midiNodeIn, AAX_IMIDINode* midiNodesOut)
        {
            const int numIns  = pluginInstance->getTotalNumInputChannels();
            const int numOuts = pluginInstance->getTotalNumOutputChannels();

            if (pluginInstance->isSuspended())
            {
                for (int i = 0; i < numOuts; ++i)
                    FloatVectorOperations::clear (outputs[i], bufferSize);
            }
            else
            {
                const int mainNumIns = numIns > 0 ? pluginInstance->busArrangement.inputBuses.getReference (0).channels.size() : 0;
                const int sidechain = busUtils.getNumEnabledBuses (true) >= 2 ? sideChainBufferIdx : -1;

                if (numOuts >= numIns)
                {
                    for (int i = 0; i < numIns; ++i)
                        memcpy (outputs[i], getAudioBufferForInput (inputs, sidechain, mainNumIns, i), (size_t) bufferSize * sizeof (float));

                    process (outputs, numOuts, bufferSize, bypass, midiNodeIn, midiNodesOut);
                }
                else
                {
                    if (channelList.size() <= numIns)
                        channelList.insertMultiple (-1, nullptr, 1 + numIns - channelList.size());

                    float** channels = channelList.getRawDataPointer();

                    for (int i = 0; i < numOuts; ++i)
                    {
                        memcpy (outputs[i], getAudioBufferForInput (inputs, sidechain, mainNumIns, i), (size_t) bufferSize * sizeof (float));
                        channels[i] = outputs[i];
                    }

                    for (int i = numOuts; i < numIns; ++i)
                        channels[i] = const_cast<float*> (getAudioBufferForInput (inputs, sidechain, mainNumIns, i));

                    process (channels, numIns, bufferSize, bypass, midiNodeIn, midiNodesOut);
                }
            }
        }

        bool supportsSidechain() const noexcept { return hasSidechain; };

    private:
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
                        sideChainBuffer.realloc (static_cast<size_t> (maxBufferSize));
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

            for (int parameterIndex = 0; parameterIndex < numParameters; ++parameterIndex)
            {
                AAX_CString paramName (audioProcessor.getParameterName (parameterIndex, 31).toRawUTF8());

                AAX_IParameter* parameter
                    = new AAX_CParameter<float> (IndexAsParamID (parameterIndex),
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

        AAX_Result preparePlugin()
        {
            AudioProcessor& audioProcessor = getPluginInstance();

           #if JucePlugin_IsMidiEffect
            // MIDI effect plug-ins do not support any audio channels
            jassert (audioProcessor.busArrangement.getTotalNumInputChannels()  == 0
                  && audioProcessor.busArrangement.getTotalNumOutputChannels() == 0);
           #else
            AAX_EStemFormat inputStemFormat = AAX_eStemFormat_None;
            check (Controller()->GetInputStemFormat (&inputStemFormat));

            AAX_EStemFormat outputStemFormat = AAX_eStemFormat_None;
            check (Controller()->GetOutputStemFormat (&outputStemFormat));

            const AudioChannelSet inputSet  = channelSetFromStemFormat (inputStemFormat,  busUtils.busIgnoresLayout (true, 0));
            const AudioChannelSet outputSet = channelSetFromStemFormat (outputStemFormat, busUtils.busIgnoresLayout (false, 0));

            if (  (inputSet  == AudioChannelSet::disabled() && inputStemFormat  != AAX_eStemFormat_None)
               || (outputSet == AudioChannelSet::disabled() && outputStemFormat != AAX_eStemFormat_None))
                return AAX_ERROR_UNIMPLEMENTED;

            bool success = true;

            if (busUtils.getBusCount (true) > 0)
                success = audioProcessor.setPreferredBusArrangement (true, 0, inputSet);

            if (success && busUtils.getBusCount (false) > 0)
                success = audioProcessor.setPreferredBusArrangement (false, 0, outputSet);

            // This should never happen as the plugin reported that this layout is supported
            jassert (success);

            hasSidechain = enableAuxBusesForCurrentFormat (busUtils, inputSet, outputSet);
            if (hasSidechain)
                sideChainBuffer.realloc (static_cast<size_t> (maxBufferSize));

            // recheck the format
            if ( (busUtils.getBusCount (true)  > 0 && busUtils.getChannelSet (true, 0)  != inputSet)
              || (busUtils.getBusCount (false) > 0 && busUtils.getChannelSet (false, 0) != outputSet)
              || (hasSidechain && busUtils.getNumChannels(true, 1) != 1))
                return AAX_ERROR_UNIMPLEMENTED;
           #endif

            audioProcessor.setRateAndBufferSizeDetails (sampleRate, maxBufferSize);
            audioProcessor.prepareToPlay (sampleRate, lastBufferSize);
            maxBufferSize = lastBufferSize;

            check (Controller()->SetSignalLatency (audioProcessor.getLatencySamples()));

            return AAX_SUCCESS;
        }

        ScopedJuceInitialiser_GUI libraryInitialiser;

        ScopedPointer<AudioProcessor> pluginInstance;
        PluginBusUtilities busUtils;
        MidiBuffer midiBuffer;
        Array<float*> channelList;
        int32_t juceChunkIndex;
        AAX_CSampleRate sampleRate;
        int lastBufferSize, maxBufferSize;
        bool hasSidechain;
        HeapBlock<float> sideChainBuffer;

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
    struct IndexAsParamID
    {
        inline explicit IndexAsParamID (int i) noexcept : index (i) {}

        operator AAX_CParamID() noexcept
        {
            jassert (index >= 0);

            char* t = name + sizeof (name);
            *--t = 0;
            int v = index;

            do
            {
                *--t = (char) ('0' + (v % 10));
                v /= 10;

            } while (v > 0);

            return static_cast<AAX_CParamID> (t);
        }

    private:
        int index;
        char name[32];

        JUCE_DECLARE_NON_COPYABLE (IndexAsParamID)
    };

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
    static void AAX_CALLBACK algorithmProcessCallback (JUCEAlgorithmContext* const instancesBegin[],
                                                       const void* const instancesEnd)
    {
        for (JUCEAlgorithmContext* const* iter = instancesBegin; iter < instancesEnd; ++iter)
        {
            const JUCEAlgorithmContext& i = **iter;

            int sideChainBufferIdx = i.pluginInstance->parameters.supportsSidechain() && i.sideChainBuffers != nullptr
                                         ? static_cast<int> (*i.sideChainBuffers)
                                         : -1;

            i.pluginInstance->parameters.process (i.inputChannels, i.outputChannels, sideChainBufferIdx,
                                                  *(i.bufferSize), *(i.bypass) != 0,
                                                  getMidiNodeIn(i), getMidiNodeOut(i));
        }
    }

    static bool enableAuxBusesForCurrentFormat (PluginBusUtilities& busUtils, const AudioChannelSet& inputLayout,
                                                                              const AudioChannelSet& outputLayout)
    {
        const int numOutBuses = busUtils.getBusCount (false);
        const int numInputBuses = busUtils.getBusCount(true);

        if (numOutBuses > 1)
        {
            PluginBusUtilities::ScopedBusRestorer layoutRestorer (busUtils);

            // enable all possible output buses
            for (int busIdx = 1; busIdx < busUtils.getBusCount (false); ++busIdx)
            {
                AudioChannelSet layout = busUtils.getChannelSet (false, busIdx);

                // bus disabled by default? try to enable it with the default layout
                if (layout == AudioChannelSet::disabled())
                {
                    layout = busUtils.getDefaultLayoutForBus (false, busIdx);
                    busUtils.processor.setPreferredBusArrangement (false, busIdx, layout);
                }
            }

            // changing output buses may have changed main bus layout
            bool success = true;

            if (numInputBuses > 0)
                success = busUtils.processor.setPreferredBusArrangement (true, 0, inputLayout);

            if (success)
                success = busUtils.processor.setPreferredBusArrangement (false, 0, outputLayout);

            // was the above successful
            if (success && (numInputBuses == 0 || busUtils.getChannelSet (true,  0) == inputLayout)
                        && busUtils.getChannelSet (false, 0) == outputLayout)
                layoutRestorer.release();
        }

        // does the plug-in have side-chain support? Check the following:
        // 1) does it have an input bus with index = 1 which supports mono
        // 2) can all other input buses be disabled
        // 3) does the format of the main buses not change when enabling the first bus
        if (numInputBuses > 1)
        {
            bool success = true;
            bool hasSidechain = false;

            if (const AudioChannelSet* set = busUtils.getSupportedBusLayouts (true, 1).getDefaultLayoutForChannelNum (1))
                hasSidechain = busUtils.processor.setPreferredBusArrangement (true, 1, *set);

            if (! hasSidechain)
                success = busUtils.processor.setPreferredBusArrangement (true, 1, AudioChannelSet::disabled());

            // AAX requires your processor's first sidechain to be either mono or that
            // it can be disabled
            jassert(success);

            // disable all other input buses
            for (int busIdx = 2; busIdx < numInputBuses; ++busIdx)
            {
                success = busUtils.processor.setPreferredBusArrangement (true, busIdx, AudioChannelSet::disabled());

                // AAX can only have a single side-chain input. Therefore, your processor must either
                // only have a single side-chain input or allow disabling all other side-chains
                jassert (success);
            }

            if (hasSidechain)
            {
                if (busUtils.getBusCount (false) == 0 || busUtils.getBusCount (true) == 0 ||
                   (busUtils.getChannelSet (true, 0)  == inputLayout && busUtils.getChannelSet (false, 0) == outputLayout))
                    return true;

                // restore the old layout
                if (busUtils.getBusCount(true) > 0)
                    busUtils.processor.setPreferredBusArrangement (true,  0, inputLayout);

                if (busUtils.getBusCount (false) > 0)
                    busUtils.processor.setPreferredBusArrangement (false, 0, outputLayout);
            }
        }

        return false;
    }

    //==============================================================================
    static void createDescriptor (AAX_IComponentDescriptor& desc, int configIndex, PluginBusUtilities& busUtils,
                                  const AudioChannelSet& inputLayout, const AudioChannelSet& outputLayout,
                                  const AAX_EStemFormat aaxInputFormat, const AAX_EStemFormat aaxOutputFormat)
    {
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

        if (enableAuxBusesForCurrentFormat (busUtils, inputLayout, outputLayout))
        {
            check (desc.AddSideChainIn (JUCEAlgorithmIDs::sideChainBuffers));
            properties->AddProperty (AAX_eProperty_SupportsSideChainInput, true);
        }

        // add the output buses
        // This is incrdibly dumb: the output bus format must be well defined
        // for every main bus in/out format pair. This means that there cannot
        // be two configurations with different aux formats but
        // identical main bus in/out formats.
        for (int busIdx = 1; busIdx < busUtils.getBusCount (false); ++busIdx)
        {
            AudioChannelSet outBusLayout = busUtils.getChannelSet (false, busIdx);

            if (outBusLayout != AudioChannelSet::disabled())
            {
                AAX_EStemFormat auxFormat  = getFormatForAudioChannelSet (outBusLayout, busUtils.busIgnoresLayout (false,  busIdx));
                if (auxFormat != AAX_eStemFormat_INT32_MAX && auxFormat != AAX_eStemFormat_None)
                {
                    const String& name = busUtils.processor.busArrangement.outputBuses.getReference (busIdx).name;
                    check (desc.AddAuxOutputStem (0, static_cast<int32_t> (auxFormat), name.toRawUTF8()));
                }
            }
        }

        // this assertion should be covered by the assertions above
        // if not please report a bug
        jassert (busUtils.getNumEnabledBuses (true) <= 2);

        check (desc.AddProcessProc_Native (algorithmProcessCallback, properties));
    }

    static void getPlugInDescription (AAX_IEffectDescriptor& descriptor)
    {
        ScopedPointer<AudioProcessor> plugin = createPluginFilterOfType (AudioProcessor::wrapperType_AAX);
        PluginBusUtilities busUtils (*plugin, false);

        busUtils.findAllCompatibleLayouts();

        descriptor.AddName (JucePlugin_Desc);
        descriptor.AddName (JucePlugin_Name);
        descriptor.AddCategory (JucePlugin_AAXCategory);

       #ifdef JucePlugin_AAXPageTableFile
        // optional page table setting - define this macro in your project if you want
        // to set this value - see Avid documentation for details about its format.
        descriptor.AddResourceInfo (AAX_eResourceType_PageTable, JucePlugin_AAXPageTableFile);
       #endif

        check (descriptor.AddProcPtr ((void*) JuceAAX_GUI::Create,        kAAX_ProcPtrID_Create_EffectGUI));
        check (descriptor.AddProcPtr ((void*) JuceAAX_Processor::Create,  kAAX_ProcPtrID_Create_EffectParameters));

        SortedSet<AAXFormatConfiguration> aaxFormats;
        SortedSet<AudioChannelSet> inLayouts =  busUtils.getBusCount (true)  > 0 ? busUtils.getSupportedBusLayouts (true,  0).supportedLayouts : SortedSet<AudioChannelSet>();
        SortedSet<AudioChannelSet> outLayouts = busUtils.getBusCount (false) > 0 ? busUtils.getSupportedBusLayouts (false, 0).supportedLayouts : SortedSet<AudioChannelSet>();

        const int numIns  = inLayouts. size();
        const int numOuts = outLayouts.size();

       #if JucePlugin_IsMidiEffect
        // MIDI effect plug-ins do not support any audio channels
        jassert (numIns == 0 && numOuts == 0);

        if (AAX_IComponentDescriptor* const desc = descriptor.NewComponentDescriptor())
        {
            createDescriptor (*desc, 0, busUtils,
                              AudioChannelSet::disabled(), AudioChannelSet::disabled(),
                              AAX_eStemFormat_Mono, AAX_eStemFormat_Mono);
            check (descriptor.AddComponent (desc));
        }

       #else
        int configIndex = 0;

        for (int inIdx = 0; inIdx < jmax (numIns, 1); ++inIdx)
        {
            for (int outIdx = 0; outIdx < jmax (numOuts, 1); ++outIdx)
            {
                bool success = true;

                if (numIns > 0)
                    success = busUtils.processor.setPreferredBusArrangement (true, 0, inLayouts.getReference (inIdx));

                if (numOuts > 0 && success)
                    success = busUtils.processor.setPreferredBusArrangement (false, 0, outLayouts.getReference (outIdx));

                // We should never hit this assertion: PluginBusUtilities reported this as supported.
                // Please report this as a bug!
                jassert (success);

                AudioChannelSet inLayout  = numIns  > 0 ? busUtils.getChannelSet (true,  0) : AudioChannelSet();
                AudioChannelSet outLayout = numOuts > 0 ? busUtils.getChannelSet (false, 0) : AudioChannelSet();

                // if we can't set both in AND out formats simultaneously then ignore this format!
                if (numIns > 0 && numOuts > 0 && (inLayout != inLayouts.getReference (inIdx) || (outLayout != outLayouts.getReference (outIdx))))
                    continue;

                AAX_EStemFormat aaxInFormat  = getFormatForAudioChannelSet (inLayout,  busUtils.busIgnoresLayout (true,  0));
                AAX_EStemFormat aaxOutFormat = getFormatForAudioChannelSet (outLayout, busUtils.busIgnoresLayout (false, 0));

                // does AAX support this layout?
                if (aaxInFormat == AAX_eStemFormat_INT32_MAX || aaxOutFormat == AAX_eStemFormat_INT32_MAX)
                    continue;

                // AAX requires a single input if this plug-in is a synth
               #if JucePlugin_IsSynth
                if (numIns == 0)
                    aaxInFormat = aaxOutFormat;
               #endif

                if (aaxInFormat == AAX_eStemFormat_None && aaxOutFormat == AAX_eStemFormat_None)
                    continue;

                AAXFormatConfiguration aaxFormat (aaxInFormat, aaxOutFormat);
                if (aaxFormats.indexOf (aaxFormat) < 0)
                {
                    aaxFormats.add (aaxFormat);

                    if (AAX_IComponentDescriptor* const desc = descriptor.NewComponentDescriptor())
                    {
                        createDescriptor (*desc, configIndex++, busUtils, inLayout, outLayout, aaxInFormat, aaxOutFormat);
                        check (descriptor.AddComponent (desc));
                    }
                }
            }
        }

        // You don't have any supported layouts
        jassert (configIndex > 0);
       #endif
    }
};

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
