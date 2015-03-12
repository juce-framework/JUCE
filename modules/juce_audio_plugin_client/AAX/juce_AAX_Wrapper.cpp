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

#include "../utility/juce_CheckSettingMacros.h"

#if JucePlugin_Build_AAX && (JUCE_INCLUDED_AAX_IN_MM || defined (_WIN32) || defined (_WIN64))

#ifdef _MSC_VER
 #include <windows.h>
#else
 #include <Cocoa/Cocoa.h>
#endif

#include "../utility/juce_IncludeModuleHeaders.h"
#undef Component

#ifdef __clang__
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wnon-virtual-dtor"
 #pragma clang diagnostic ignored "-Wsign-conversion"
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

using juce::Component;

const int32_t juceChunkType = 'juce';

//==============================================================================
struct AAXClasses
{
    static void check (AAX_Result result)
    {
        jassert (result == AAX_SUCCESS); (void) result;
    }

    static int getParamIndexFromID (AAX_CParamID paramID) noexcept
    {
        return atoi (paramID);
    }

    static bool isBypassParam (AAX_CParamID paramID) noexcept
    {
        return AAX::IsParameterIDEqual (paramID, cDefaultMasterBypassID) != 0;
    }

    static AAX_EStemFormat getFormatForChans (const int numChans) noexcept
    {
        switch (numChans)
        {
            case 0:   return AAX_eStemFormat_None;
            case 1:   return AAX_eStemFormat_Mono;
            case 2:   return AAX_eStemFormat_Stereo;
            case 3:   return AAX_eStemFormat_LCR;
            case 4:   return AAX_eStemFormat_Quad;
            case 5:   return AAX_eStemFormat_5_0;
            case 6:   return AAX_eStemFormat_5_1;
            case 7:   return AAX_eStemFormat_7_0_DTS;
            case 8:   return AAX_eStemFormat_7_1_DTS;
            default:  jassertfalse; break;
        }

        return AAX_eStemFormat_None;
    }

    static int getNumChannelsForStemFormat (AAX_EStemFormat format) noexcept
    {
        switch (format)
        {
            case AAX_eStemFormat_None:      return 0;
            case AAX_eStemFormat_Mono:      return 1;
            case AAX_eStemFormat_Stereo:    return 2;
            case AAX_eStemFormat_LCR:       return 3;
            case AAX_eStemFormat_LCRS:
            case AAX_eStemFormat_Quad:      return 4;
            case AAX_eStemFormat_5_0:       return 5;
            case AAX_eStemFormat_5_1:
            case AAX_eStemFormat_6_0:       return 6;
            case AAX_eStemFormat_6_1:
            case AAX_eStemFormat_7_0_SDDS:
            case AAX_eStemFormat_7_0_DTS:   return 7;
            case AAX_eStemFormat_7_1_SDDS:
            case AAX_eStemFormat_7_1_DTS:   return 8;
            default:                        jassertfalse; break;
        }

        return 0;
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

       #if JucePlugin_WantsMidiInput
        AAX_IMIDINode* midiNodeIn;
       #endif

       #if JucePlugin_ProducesMidiOutput
        AAX_IMIDINode* midiNodeOut;
       #endif

        PluginInstanceInfo* pluginInstance;
        int32_t* isPrepared;
    };

    struct JUCEAlgorithmIDs
    {
        enum
        {
            inputChannels   = AAX_FIELD_INDEX (JUCEAlgorithmContext, inputChannels),
            outputChannels  = AAX_FIELD_INDEX (JUCEAlgorithmContext, outputChannels),
            bufferSize      = AAX_FIELD_INDEX (JUCEAlgorithmContext, bufferSize),
            bypass          = AAX_FIELD_INDEX (JUCEAlgorithmContext, bypass),

           #if JucePlugin_WantsMidiInput
            midiNodeIn      = AAX_FIELD_INDEX (JUCEAlgorithmContext, midiNodeIn),
           #endif

           #if JucePlugin_ProducesMidiOutput
            midiNodeOut     = AAX_FIELD_INDEX (JUCEAlgorithmContext, midiNodeOut),
           #endif

            pluginInstance  = AAX_FIELD_INDEX (JUCEAlgorithmContext, pluginInstance),
            preparedFlag    = AAX_FIELD_INDEX (JUCEAlgorithmContext, isPrepared)
        };
    };

   #if JucePlugin_WantsMidiInput
    static AAX_IMIDINode* getMidiNodeIn (const JUCEAlgorithmContext& c) noexcept   { return c.midiNodeIn; }
   #else
    static AAX_IMIDINode* getMidiNodeIn (const JUCEAlgorithmContext&) noexcept     { return nullptr; }
   #endif

   #if JucePlugin_ProducesMidiOutput
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
                    info.isHighlighted   = isHighlighted;
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
        JuceAAX_Processor()  : sampleRate (0), lastBufferSize (1024)
        {
            pluginInstance = createPluginFilterOfType (AudioProcessor::wrapperType_AAX);
            pluginInstance->setPlayHead (this);
            pluginInstance->addListener (this);

            AAX_CEffectParameters::GetNumberOfChunks (&juceChunkIndex);
        }

        static AAX_CEffectParameters* AAX_CALLBACK Create()   { return new JuceAAX_Processor(); }

        AAX_Result EffectInit() override
        {
            check (Controller()->GetSampleRate (&sampleRate));

            preparePlugin();
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

        AAX_Result GetChunkSize (AAX_CTypeID chunkID, uint32_t* oSize) const override
        {
            if (chunkID != juceChunkType)
                return AAX_CEffectParameters::GetChunkSize (chunkID, oSize);

            tempFilterData.reset();
            pluginInstance->getStateInformation (tempFilterData);
            *oSize = (uint32_t) tempFilterData.getSize();
            return AAX_SUCCESS;
        }

        AAX_Result GetChunk (AAX_CTypeID chunkID, AAX_SPlugInChunk* oChunk) const override
        {
            if (chunkID != juceChunkType)
                return AAX_CEffectParameters::GetChunk (chunkID, oChunk);

            if (tempFilterData.getSize() == 0)
                pluginInstance->getStateInformation (tempFilterData);

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

        void process (const float* const* inputs, float* const* outputs, const int bufferSize,
                      const bool bypass, AAX_IMIDINode* midiNodeIn, AAX_IMIDINode* midiNodesOut)
        {
            const int numIns  = pluginInstance->getNumInputChannels();
            const int numOuts = pluginInstance->getNumOutputChannels();

            if (numOuts >= numIns)
            {
                for (int i = 0; i < numIns; ++i)
                    memcpy (outputs[i], inputs[i], (size_t) bufferSize * sizeof (float));

                process (outputs, numOuts, bufferSize, bypass, midiNodeIn, midiNodesOut);
            }
            else
            {
                if (channelList.size() <= numIns)
                    channelList.insertMultiple (-1, nullptr, 1 + numIns - channelList.size());

                float** channels = channelList.getRawDataPointer();

                for (int i = 0; i < numOuts; ++i)
                {
                    memcpy (outputs[i], inputs[i], (size_t) bufferSize * sizeof (float));
                    channels[i] = outputs[i];
                }

                for (int i = numOuts; i < numIns; ++i)
                    channels[i] = const_cast<float*> (inputs[i]);

                process (channels, numIns, bufferSize, bypass, midiNodeIn, midiNodesOut);
            }
        }

    private:
        void process (float* const* channels, const int numChans, const int bufferSize,
                      const bool bypass, AAX_IMIDINode* midiNodeIn, AAX_IMIDINode* midiNodesOut)
        {
            AudioSampleBuffer buffer (channels, numChans, bufferSize);

            midiBuffer.clear();

            (void) midiNodeIn;
            (void) midiNodesOut;

           #if JucePlugin_WantsMidiInput
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
                    pluginInstance->setPlayConfigDetails (pluginInstance->getNumInputChannels(),
                                                          pluginInstance->getNumOutputChannels(),
                                                          sampleRate, bufferSize);
                    pluginInstance->prepareToPlay (sampleRate, bufferSize);
                }

                const ScopedLock sl (pluginInstance->getCallbackLock());

                if (bypass)
                    pluginInstance->processBlockBypassed (buffer, midiBuffer);
                else
                    pluginInstance->processBlock (buffer, midiBuffer);
            }

           #if JucePlugin_ProducesMidiOutput
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
           #else
            (void) midiNodesOut;
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
                AAX_IParameter* parameter
                    = new AAX_CParameter<float> (IndexAsParamID (parameterIndex),
                                                 audioProcessor.getParameterName (parameterIndex, 31).toRawUTF8(),
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

        void preparePlugin()
        {
            AAX_EStemFormat inputStemFormat = AAX_eStemFormat_None;
            check (Controller()->GetInputStemFormat (&inputStemFormat));
            const int numberOfInputChannels = getNumChannelsForStemFormat (inputStemFormat);

            AAX_EStemFormat outputStemFormat = AAX_eStemFormat_None;
            check (Controller()->GetOutputStemFormat (&outputStemFormat));
            const int numberOfOutputChannels = getNumChannelsForStemFormat (outputStemFormat);

            AudioProcessor& audioProcessor = getPluginInstance();

            audioProcessor.setSpeakerArrangement (getSpeakerArrangementString (inputStemFormat),
                                                  getSpeakerArrangementString (outputStemFormat));

            audioProcessor.setPlayConfigDetails (numberOfInputChannels, numberOfOutputChannels, sampleRate, lastBufferSize);
            audioProcessor.prepareToPlay (sampleRate, lastBufferSize);

            check (Controller()->SetSignalLatency (audioProcessor.getLatencySamples()));
        }

        ScopedJuceInitialiser_GUI libraryInitialiser;

        ScopedPointer<AudioProcessor> pluginInstance;
        MidiBuffer midiBuffer;
        Array<float*> channelList;
        int32_t juceChunkIndex;
        AAX_CSampleRate sampleRate;
        int lastBufferSize;

        // tempFilterData is initialized in GetChunkSize.
        // To avoid generating it again in GetChunk, we keep it as a member.
        mutable juce::MemoryBlock tempFilterData;

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
    static void AAX_CALLBACK algorithmProcessCallback (JUCEAlgorithmContext* const instancesBegin[],
                                                       const void* const instancesEnd)
    {
        for (JUCEAlgorithmContext* const* iter = instancesBegin; iter < instancesEnd; ++iter)
        {
            const JUCEAlgorithmContext& i = **iter;

            i.pluginInstance->parameters.process (i.inputChannels, i.outputChannels,
                                                  *(i.bufferSize), *(i.bypass) != 0,
                                                  getMidiNodeIn(i), getMidiNodeOut(i));
        }
    }

    //==============================================================================
    static void createDescriptor (AAX_IComponentDescriptor& desc, int channelConfigIndex,
                                  int numInputs, int numOutputs)
    {
        check (desc.AddAudioIn  (JUCEAlgorithmIDs::inputChannels));
        check (desc.AddAudioOut (JUCEAlgorithmIDs::outputChannels));
        check (desc.AddAudioBufferLength (JUCEAlgorithmIDs::bufferSize));
        check (desc.AddDataInPort (JUCEAlgorithmIDs::bypass, sizeof (int32_t)));

       #if JucePlugin_WantsMidiInput
        check (desc.AddMIDINode (JUCEAlgorithmIDs::midiNodeIn, AAX_eMIDINodeType_LocalInput,
                                 JucePlugin_Name, 0xffff));
       #endif

       #if JucePlugin_ProducesMidiOutput
        check (desc.AddMIDINode (JUCEAlgorithmIDs::midiNodeOut, AAX_eMIDINodeType_LocalOutput,
                                 JucePlugin_Name " Out", 0xffff));
       #endif

        check (desc.AddPrivateData (JUCEAlgorithmIDs::pluginInstance, sizeof (PluginInstanceInfo)));

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

        properties->AddProperty (AAX_eProperty_InputStemFormat,     getFormatForChans (numInputs));
        properties->AddProperty (AAX_eProperty_OutputStemFormat,    getFormatForChans (numOutputs));

        // This value needs to match the RTAS wrapper's Type ID, so that
        // the host knows that the RTAS/AAX plugins are equivalent.
        properties->AddProperty (AAX_eProperty_PlugInID_Native,     'jcaa' + channelConfigIndex);

       #if ! JucePlugin_AAXDisableAudioSuite
        properties->AddProperty (AAX_eProperty_PlugInID_AudioSuite, 'jyaa' + channelConfigIndex);
       #endif

       #if JucePlugin_AAXDisableMultiMono
        properties->AddProperty (AAX_eProperty_Constraint_MultiMonoSupport, false);
       #else
        properties->AddProperty (AAX_eProperty_Constraint_MultiMonoSupport, true);
       #endif

        check (desc.AddProcessProc_Native (algorithmProcessCallback, properties));
    }

    static void getPlugInDescription (AAX_IEffectDescriptor& descriptor)
    {
        descriptor.AddName (JucePlugin_Desc);
        descriptor.AddName (JucePlugin_Name);
        descriptor.AddCategory (JucePlugin_AAXCategory);

       #ifdef JucePlugin_AAXPageTableFile
        // optional page table setting - define this macro in your AppConfig.h if you
        // want to set this value - see Avid documentation for details about its format.
        descriptor.AddResourceInfo (AAX_eResourceType_PageTable, JucePlugin_AAXPageTableFile);
       #endif

        check (descriptor.AddProcPtr ((void*) JuceAAX_GUI::Create,        kAAX_ProcPtrID_Create_EffectGUI));
        check (descriptor.AddProcPtr ((void*) JuceAAX_Processor::Create,  kAAX_ProcPtrID_Create_EffectParameters));

        const short channelConfigs[][2] = { JucePlugin_PreferredChannelConfigurations };
        const int numConfigs = numElementsInArray (channelConfigs);

        // You need to actually add some configurations to the JucePlugin_PreferredChannelConfigurations
        // value in your JucePluginCharacteristics.h file..
        jassert (numConfigs > 0);

        for (int i = 0; i < numConfigs; ++i)
        {
            if (AAX_IComponentDescriptor* const desc = descriptor.NewComponentDescriptor())
            {
                const int numIns  = channelConfigs [i][0];
                const int numOuts = channelConfigs [i][1];

                if (numIns <= 8 && numOuts <= 8) // AAX doesn't seem to handle more than 8 chans
                {
                    createDescriptor (*desc, i, numIns, numOuts);
                    check (descriptor.AddComponent (desc));
                }
            }
        }
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
