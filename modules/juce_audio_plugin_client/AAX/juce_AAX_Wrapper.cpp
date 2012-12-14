/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

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

#include "AAX_Exports.cpp"
#include "AAX_ICollection.h"
#include "AAX_IComponentDescriptor.h"
#include "AAX_IEffectDescriptor.h"
#include "AAX_IPropertyMap.h"
#include "AAX_CEffectParameters.h"
#include "AAX_Errors.h"
#include "AAX_CBinaryTaperDelegate.h"
#include "AAX_CBinaryDisplayDelegate.h"
#include "AAX_CEffectGUI.h"
#include "AAX_IViewContainer.h"

using juce::Component;

//==============================================================================
struct AAXClasses
{
    static void check (AAX_Result result)
    {
        jassert (result == AAX_SUCCESS); (void) result;
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
            case 7:   return AAX_eStemFormat_6_1;
            case 8:   return AAX_eStemFormat_7_1_DTS;

            default:  jassertfalse; break; // hmm - not a valid number of chans..
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
            case AAX_eStemFormat_Quad:      return 4;
            case AAX_eStemFormat_5_0:       return 5;
            case AAX_eStemFormat_5_1:       return 6;
            case AAX_eStemFormat_6_1:       return 7;
            case AAX_eStemFormat_7_1_DTS:   return 8;
            default:  jassertfalse; break; // hmm - not a valid number of chans..
        }

        return 0;
    }

    //==============================================================================
    struct JUCELibraryRefCount
    {
        JUCELibraryRefCount()    { if (getCount()++ == 0) initialise(); }
        ~JUCELibraryRefCount()   { if (--getCount() == 0) shutdown(); }

    private:
        static void initialise()
        {
            initialiseJuce_GUI();
        }

        static void shutdown()
        {
            shutdownJuce_GUI();
        }

        int& getCount() noexcept
        {
            static int count = 0;
            return count;
        }
    };

    //==============================================================================
    struct PluginInstanceInfo
    {
        PluginInstanceInfo (AudioProcessor& p)  : pluginInstance (p) {}

        void process (const float* const* inputs, float* const* outputs, const int bufferSize, const bool bypass)
        {
            const int numIns  = pluginInstance.getNumInputChannels();
            const int numOuts = pluginInstance.getNumOutputChannels();

            if (numOuts >= numIns)
            {
                for (int i = 0; i < numIns; ++i)
                    memcpy (outputs[i], inputs[i], bufferSize * sizeof (float));

                process (outputs, numOuts, bufferSize, bypass);
            }
            else
            {
                if (channelList.size() <= numIns)
                    channelList.insertMultiple (-1, nullptr, 1 + numIns - channelList.size());

                float** channels = channelList.getRawDataPointer();

                for (int i = 0; i < numOuts; ++i)
                {
                    memcpy (outputs[i], inputs[i], bufferSize * sizeof (float));
                    channels[i] = outputs[i];
                }

                for (int i = numOuts; i < numIns; ++i)
                    channels[i] = const_cast <float*> (inputs[i]);

                process (channels, numIns, bufferSize, bypass);
            }
        }

        void process (float* const* channels, const int numChans, const int bufferSize, const bool bypass)
        {
            AudioSampleBuffer buffer (channels, numChans, bufferSize);

            // XXX need to do midi..
            midiBuffer.clear();

            {
                const ScopedLock sl (pluginInstance.getCallbackLock());

                if (bypass)
                    pluginInstance.processBlockBypassed (buffer, midiBuffer);
                else
                    pluginInstance.processBlock (buffer, midiBuffer);
            }
        }

        AudioProcessor& pluginInstance;
        MidiBuffer midiBuffer;
        Array<float*> channelList;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginInstanceInfo)
    };

    //==============================================================================
    struct JUCEAlgorithmContext
    {
        float** inputChannels;
        float** outputChannels;
        int32_t* bufferSize;
        int32_t* bypass;

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
            pluginInstance  = AAX_FIELD_INDEX (JUCEAlgorithmContext, pluginInstance),
            preparedFlag    = AAX_FIELD_INDEX (JUCEAlgorithmContext, isPrepared)
        };
    };

    //==============================================================================
    class JuceAAX_GUI   : public AAX_CEffectGUI
    {
    public:
        JuceAAX_GUI() {}
        virtual ~JuceAAX_GUI()  { DeleteViewContainer(); }

        static AAX_IEffectGUI* AAX_CALLBACK Create()   { return new JuceAAX_GUI(); }

        void CreateViewContents()
        {
            if (component == nullptr)
            {
                if (JuceAAX_Parameters* params = dynamic_cast <JuceAAX_Parameters*> (GetEffectParameters()))
                    component = new ContentWrapperComponent (*this, params->getPluginInstance());
                else
                    jassertfalse;
            }
        }

        void CreateViewContainer()
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

        void DeleteViewContainer()
        {
            if (component != nullptr)
            {
                JUCE_AUTORELEASEPOOL
                component->removeFromDesktop();
                component = nullptr;
            }
        }

        virtual AAX_Result GetViewSize (AAX_Point* const viewSize) const
        {
            if (component != nullptr)
            {
                viewSize->horz = (float) component->getWidth();
                viewSize->vert = (float) component->getHeight();
                return AAX_SUCCESS;
            }

            return AAX_ERROR_NULL_OBJECT;
        }

        AAX_Result ParameterUpdated (AAX_CParamID iParameterID)
        {

            return AAX_SUCCESS;
        }

        AAX_Result SetControlHighlightInfo (AAX_CParamID iParameterID, AAX_CBoolean iIsHighlighted, AAX_EHighlightColor iColor)
        {

            return AAX_SUCCESS;
        }

    private:
        class ContentWrapperComponent  : public juce::Component
        {
        public:
            ContentWrapperComponent (JuceAAX_GUI& gui, AudioProcessor& plugin)
                : owner (gui)
            {
                setOpaque (true);
                addAndMakeVisible (pluginEditor = plugin.createEditorIfNeeded());
                setBounds (pluginEditor->getLocalBounds());
                setBroughtToFrontOnMouseClick (true);
            }

            ~ContentWrapperComponent()
            {
                if (pluginEditor != nullptr)
                {
                    PopupMenu::dismissAllActiveMenus();
                    pluginEditor->getAudioProcessor()->editorBeingDeleted (pluginEditor);
                }
            }

            void paint (Graphics& g)
            {
                g.fillAll (Colours::black);
            }

            void childBoundsChanged (Component*)
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

        private:
            ScopedPointer<AudioProcessorEditor> pluginEditor;
            JuceAAX_GUI& owner;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentWrapperComponent)
        };

        ScopedPointer<ContentWrapperComponent> component;

        JUCELibraryRefCount juceCount;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceAAX_GUI)
    };

    //==============================================================================
    class JuceAAX_Parameters   : public AAX_CEffectParameters
    {
    public:
        JuceAAX_Parameters()
        {
            pluginInstance = createPluginFilterOfType (AudioProcessor::wrapperType_AAX);
        }

        static AAX_CEffectParameters* AAX_CALLBACK Create()   { return new JuceAAX_Parameters(); }

        AAX_Result EffectInit()
        {
            addBypassParameter();

            // add other params..

            preparePlugin();

            return AAX_SUCCESS;
        }

        AAX_Result ResetFieldData (AAX_CFieldIndex fieldIndex, void* data, uint32_t dataSize) const
        {
            switch (fieldIndex)
            {
                case JUCEAlgorithmIDs::pluginInstance:
                {
                    const size_t numObjects = dataSize / sizeof (PluginInstanceInfo);
                    PluginInstanceInfo* const objects = static_cast <PluginInstanceInfo*> (data);

                    jassert (numObjects == 1); // not sure how to handle more than one..

                    for (size_t i = 0; i < numObjects; ++i)
                        new (objects + i) PluginInstanceInfo (*pluginInstance);

                    break;
                }

                case JUCEAlgorithmIDs::preparedFlag:
                {
                    preparePlugin();

                    const size_t numObjects = dataSize / sizeof (uint32_t);
                    uint32_t* const objects = static_cast <uint32_t*> (data);

                    for (size_t i = 0; i < numObjects; ++i)
                        new (objects + i) uint32_t (1);

                    break;
                }
            }

            return AAX_SUCCESS;
            //return AAX_ERROR_INVALID_FIELD_INDEX;
        }

        AudioProcessor& getPluginInstance() const noexcept   { return *pluginInstance; }

    private:
        void addBypassParameter()
        {
            AAX_CString bypassID;
            GetMasterBypassParameter (&bypassID);

            AAX_IParameter* masterBypass = new AAX_CParameter<bool> (bypassID.CString(),
                                                                     AAX_CString ("Master Bypass"),
                                                                     false,
                                                                     AAX_CBinaryTaperDelegate<bool>(),
                                                                     AAX_CBinaryDisplayDelegate<bool> ("bypass", "on"),
                                                                     true);
            masterBypass->SetNumberOfSteps (2);
            masterBypass->SetType (AAX_eParameterType_Discrete);
            mParameterManager.AddParameter (masterBypass);
            mPacketDispatcher.RegisterPacket (bypassID.CString(), JUCEAlgorithmIDs::bypass);
        }

        void preparePlugin() const
        {
            AAX_CSampleRate sampleRate;
            check (Controller()->GetSampleRate (&sampleRate));

            AAX_EStemFormat inputStemFormat = AAX_eStemFormat_None;
            check (Controller()->GetInputStemFormat (&inputStemFormat));
            const int numberOfInputChannels = getNumChannelsForStemFormat (inputStemFormat);

            AAX_EStemFormat outputStemFormat = AAX_eStemFormat_None;
            check (Controller()->GetOutputStemFormat (&outputStemFormat));
            const int numberOfOutputChannels = getNumChannelsForStemFormat (outputStemFormat);

            int32_t bufferSize = 0;
            check (Controller()->GetSignalLatency (&bufferSize));

            AudioProcessor& audioProcessor = getPluginInstance();
            audioProcessor.setPlayConfigDetails (numberOfInputChannels, numberOfOutputChannels, sampleRate, bufferSize);
            audioProcessor.prepareToPlay (sampleRate, bufferSize);
        }

        JUCELibraryRefCount juceCount;

        ScopedPointer<AudioProcessor> pluginInstance;

        JUCE_DECLARE_NON_COPYABLE (JuceAAX_Parameters)
    };

    //==============================================================================
    static void AAX_CALLBACK algorithmProcessCallback (JUCEAlgorithmContext* const instancesBegin[],
                                                       const void* const instancesEnd)
    {
        for (JUCEAlgorithmContext* const* iter = instancesBegin; iter < instancesEnd; ++iter)
        {
            const JUCEAlgorithmContext& i = **iter;

            i.pluginInstance->process (i.inputChannels, i.outputChannels,
                                       *(i.bufferSize), *(i.bypass) != 0);
        }
    }

    //==============================================================================
    static void createDescriptor (AAX_IComponentDescriptor& desc, int numInputs, int numOutputs)
    {
        check (desc.AddAudioIn  (JUCEAlgorithmIDs::inputChannels));
        check (desc.AddAudioOut (JUCEAlgorithmIDs::outputChannels));
        check (desc.AddAudioBufferLength (JUCEAlgorithmIDs::bufferSize));
        check (desc.AddDataInPort (JUCEAlgorithmIDs::bypass, sizeof (int32_t)));

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

        properties->AddProperty (AAX_eProperty_PlugInID_Native,     JucePlugin_AAXPluginId + (numInputs + 256 * numOutputs));

        check (desc.AddProcessProc_Native (algorithmProcessCallback, properties));
    }

    static void getPlugInDescription (AAX_IEffectDescriptor& descriptor)
    {
        descriptor.AddName (JucePlugin_Desc);
        descriptor.AddName (JucePlugin_Name);
        descriptor.AddCategory (JucePlugin_AAXCategory);

        check (descriptor.AddProcPtr ((void*) JuceAAX_GUI::Create,        kAAX_ProcPtrID_Create_EffectGUI));
        check (descriptor.AddProcPtr ((void*) JuceAAX_Parameters::Create, kAAX_ProcPtrID_Create_EffectParameters));

        const short channelConfigs[][2] = { JucePlugin_PreferredChannelConfigurations };
        const int numConfigs = numElementsInArray (channelConfigs);

        // You need to actually add some configurations to the JucePlugin_PreferredChannelConfigurations
        // value in your JucePluginCharacteristics.h file..
        jassert (numConfigs > 0);

        for (int i = 0; i < numConfigs; ++i)
        {
            if (AAX_IComponentDescriptor* const desc = descriptor.NewComponentDescriptor())
            {
                createDescriptor (*desc,
                                  channelConfigs [i][0],
                                  channelConfigs [i][1]);

                check (descriptor.AddComponent (desc));
            }
        }
    }
};

//==============================================================================
AAX_Result JUCE_CDECL GetEffectDescriptions (AAX_ICollection* collection)
{
    AAXClasses::JUCELibraryRefCount libraryRefCount;

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
