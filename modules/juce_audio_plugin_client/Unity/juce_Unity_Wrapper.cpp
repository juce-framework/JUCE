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

#include <juce_core/system/juce_TargetPlatform.h>

#if JucePlugin_Build_Unity

#include "../utility/juce_IncludeModuleHeaders.h"
#include <juce_audio_processors/format_types/juce_LegacyAudioParameter.cpp>

#if JUCE_WINDOWS
 #include "../utility/juce_IncludeSystemHeaders.h"
#endif

#include "juce_UnityPluginInterface.h"

//==============================================================================
namespace juce
{

typedef ComponentPeer* (*createUnityPeerFunctionType) (Component&);
extern createUnityPeerFunctionType juce_createUnityPeerFn;

//==============================================================================
class UnityPeer    : public ComponentPeer,
                     public AsyncUpdater
{
public:
    UnityPeer (Component& ed)
        : ComponentPeer (ed, 0),
          mouseWatcher (*this)
    {
        getEditor().setResizable (false, false);
    }

    //==============================================================================
    Rectangle<int> getBounds() const override                              { return bounds; }
    Point<float> localToGlobal (Point<float> relativePosition) override    { return relativePosition + getBounds().getPosition().toFloat(); }
    Point<float> globalToLocal (Point<float> screenPosition) override      { return screenPosition - getBounds().getPosition().toFloat(); }

    using ComponentPeer::localToGlobal;
    using ComponentPeer::globalToLocal;

    StringArray getAvailableRenderingEngines() override                    { return StringArray ("Software Renderer"); }

    void setBounds (const Rectangle<int>& newBounds, bool) override
    {
        bounds = newBounds;
        mouseWatcher.setBoundsToWatch (bounds);
    }

    bool contains (Point<int> localPos, bool) const override
    {
        if (isPositiveAndBelow (localPos.getX(), getBounds().getWidth())
               && isPositiveAndBelow (localPos.getY(), getBounds().getHeight()))
            return true;

        return false;
    }

    void handleAsyncUpdate() override
    {
        fillPixels();
    }

    //==============================================================================
    AudioProcessorEditor& getEditor()    { return *dynamic_cast<AudioProcessorEditor*> (&getComponent()); }

    void setPixelDataHandle (uint8* handle, int width, int height)
    {
        pixelData = handle;

        textureWidth = width;
        textureHeight = height;

        renderImage = Image (new UnityBitmapImage (pixelData, width, height));
    }

    // N.B. This is NOT an efficient way to do this and you shouldn't use this method in your own code.
    // It works for our purposes here but a much more efficient way would be to use a GL texture.
    void fillPixels()
    {
        if (pixelData == nullptr)
            return;

        LowLevelGraphicsSoftwareRenderer renderer (renderImage);
        renderer.addTransform (AffineTransform::verticalFlip ((float) getComponent().getHeight()));

        handlePaint (renderer);

        for (int i = 0; i < textureWidth * textureHeight * 4; i += 4)
        {
            auto r = pixelData[i + 2];
            auto g = pixelData[i + 1];
            auto b = pixelData[i + 0];

            pixelData[i + 0] = r;
            pixelData[i + 1] = g;
            pixelData[i + 2] = b;
        }
    }

    void forwardMouseEvent (Point<float> position, ModifierKeys mods)
    {
        ModifierKeys::currentModifiers = mods;

        handleMouseEvent (juce::MouseInputSource::mouse, position, mods, juce::MouseInputSource::defaultPressure,
                          juce::MouseInputSource::defaultOrientation, juce::Time::currentTimeMillis());
    }

    void forwardKeyPress (int code, String name, ModifierKeys mods)
    {
        ModifierKeys::currentModifiers = mods;

        handleKeyPress (getKeyPress (code, name));
    }

private:
    //==============================================================================
    struct UnityBitmapImage    : public ImagePixelData
    {
        UnityBitmapImage (uint8* data, int w, int h)
            : ImagePixelData (Image::PixelFormat::ARGB, w, h),
              imageData (data),
              lineStride (width * pixelStride)
        {
        }

        std::unique_ptr<ImageType> createType() const override
        {
            return std::make_unique<SoftwareImageType>();
        }

        std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override
        {
            return std::make_unique<LowLevelGraphicsSoftwareRenderer> (Image (this));
        }

        void initialiseBitmapData (Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode mode) override
        {
            ignoreUnused (mode);

            const auto offset = (size_t) x * (size_t) pixelStride + (size_t) y * (size_t) lineStride;
            bitmap.data = imageData + offset;
            bitmap.size = (size_t) (lineStride * height) - offset;
            bitmap.pixelFormat = pixelFormat;
            bitmap.lineStride = lineStride;
            bitmap.pixelStride = pixelStride;
        }

        ImagePixelData::Ptr clone() override
        {
            auto im = new UnityBitmapImage (imageData, width, height);

            for (int i = 0; i < height; ++i)
                memcpy (im->imageData + i * lineStride, imageData + i * lineStride, (size_t) lineStride);

            return im;
        }

        uint8* imageData;
        int pixelStride = 4, lineStride;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnityBitmapImage)
    };

    //==============================================================================
    struct MouseWatcher    : public Timer
    {
        MouseWatcher (ComponentPeer& o)    : owner (o)    {}

        void timerCallback() override
        {
            auto pos = Desktop::getMousePosition();

            if (boundsToWatch.contains (pos) && pos != lastMousePos)
            {
                auto ms = Desktop::getInstance().getMainMouseSource();

                if (! ms.getCurrentModifiers().isLeftButtonDown())
                    owner.handleMouseEvent (juce::MouseInputSource::mouse, owner.globalToLocal (pos.toFloat()), {},
                                            juce::MouseInputSource::defaultPressure, juce::MouseInputSource::defaultOrientation, juce::Time::currentTimeMillis());

                lastMousePos = pos;
            }

        }

        void setBoundsToWatch (Rectangle<int> b)
        {
            if (boundsToWatch != b)
                boundsToWatch = b;

            startTimer (250);
        }

        ComponentPeer& owner;
        Rectangle<int> boundsToWatch;
        Point<int> lastMousePos;
    };

    //==============================================================================
    KeyPress getKeyPress (int keyCode, String name)
    {
        if (keyCode >= 32 && keyCode <= 64)
            return { keyCode, ModifierKeys::currentModifiers, juce::juce_wchar (keyCode) };

        if (keyCode >= 91 && keyCode <= 122)
            return { keyCode, ModifierKeys::currentModifiers, name[0] };

        if (keyCode >= 256 && keyCode <= 265)
            return { juce::KeyPress::numberPad0 + (keyCode - 256), ModifierKeys::currentModifiers, juce::String (keyCode - 256).getCharPointer()[0] };

        if (keyCode == 8)      return { juce::KeyPress::backspaceKey,          ModifierKeys::currentModifiers, {} };
        if (keyCode == 127)    return { juce::KeyPress::deleteKey,             ModifierKeys::currentModifiers, {} };
        if (keyCode == 9)      return { juce::KeyPress::tabKey,                ModifierKeys::currentModifiers, {} };
        if (keyCode == 13)     return { juce::KeyPress::returnKey,             ModifierKeys::currentModifiers, {} };
        if (keyCode == 27)     return { juce::KeyPress::escapeKey,             ModifierKeys::currentModifiers, {} };
        if (keyCode == 32)     return { juce::KeyPress::spaceKey,              ModifierKeys::currentModifiers, {} };
        if (keyCode == 266)    return { juce::KeyPress::numberPadDecimalPoint, ModifierKeys::currentModifiers, {} };
        if (keyCode == 267)    return { juce::KeyPress::numberPadDivide,       ModifierKeys::currentModifiers, {} };
        if (keyCode == 268)    return { juce::KeyPress::numberPadMultiply,     ModifierKeys::currentModifiers, {} };
        if (keyCode == 269)    return { juce::KeyPress::numberPadSubtract,     ModifierKeys::currentModifiers, {} };
        if (keyCode == 270)    return { juce::KeyPress::numberPadAdd,          ModifierKeys::currentModifiers, {} };
        if (keyCode == 272)    return { juce::KeyPress::numberPadEquals,       ModifierKeys::currentModifiers, {} };
        if (keyCode == 273)    return { juce::KeyPress::upKey,                 ModifierKeys::currentModifiers, {} };
        if (keyCode == 274)    return { juce::KeyPress::downKey,               ModifierKeys::currentModifiers, {} };
        if (keyCode == 275)    return { juce::KeyPress::rightKey,              ModifierKeys::currentModifiers, {} };
        if (keyCode == 276)    return { juce::KeyPress::leftKey,               ModifierKeys::currentModifiers, {} };

        return {};
    }

    //==============================================================================
    Rectangle<int> bounds;
    MouseWatcher mouseWatcher;

    uint8* pixelData = nullptr;
    int textureWidth, textureHeight;
    Image renderImage;

    //==============================================================================
    void setMinimised (bool) override                                 {}
    bool isMinimised() const override                                 { return false; }
    void setFullScreen (bool) override                                {}
    bool isFullScreen() const override                                { return false; }
    bool setAlwaysOnTop (bool) override                               { return false; }
    void toFront (bool) override                                      {}
    void toBehind (ComponentPeer*) override                           {}
    bool isFocused() const override                                   { return true; }
    void grabFocus() override                                         {}
    void* getNativeHandle() const override                            { return nullptr; }
    OptionalBorderSize getFrameSizeIfPresent() const override         { return {}; }
    BorderSize<int> getFrameSize() const override                     { return {}; }
    void setVisible (bool) override                                   {}
    void setTitle (const String&) override                            {}
    void setIcon (const Image&) override                              {}
    void textInputRequired (Point<int>, TextInputTarget&) override    {}
    void setAlpha (float) override                                    {}
    void performAnyPendingRepaintsNow() override                      {}
    void repaint (const Rectangle<int>&) override                     {}

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnityPeer)
};

ComponentPeer* createUnityPeer (Component& c)    { return new UnityPeer (c); }

//==============================================================================
class AudioProcessorUnityWrapper
{
public:
    AudioProcessorUnityWrapper (bool isTemporary)
    {
        pluginInstance.reset (createPluginFilterOfType (AudioProcessor::wrapperType_Unity));

        if (! isTemporary && pluginInstance->hasEditor())
        {
            pluginInstanceEditor.reset (pluginInstance->createEditorIfNeeded());
            pluginInstanceEditor->setVisible (true);
            pluginInstanceEditor->addToDesktop (0);
        }

        juceParameters.update (*pluginInstance, false);
    }

    ~AudioProcessorUnityWrapper()
    {
        if (pluginInstanceEditor != nullptr)
        {
            pluginInstanceEditor->removeFromDesktop();

            PopupMenu::dismissAllActiveMenus();
            pluginInstanceEditor->processor.editorBeingDeleted (pluginInstanceEditor.get());
            pluginInstanceEditor = nullptr;
        }
    }

    void create (UnityAudioEffectState* state)
    {
        // only supported in Unity plugin API > 1.0
        if (state->structSize >= sizeof (UnityAudioEffectState))
            samplesPerBlock = static_cast<int> (state->dspBufferSize);

       #ifdef JucePlugin_PreferredChannelConfigurations
        short configs[][2] = { JucePlugin_PreferredChannelConfigurations };
        const int numConfigs = sizeof (configs) / sizeof (short[2]);

        jassertquiet (numConfigs > 0 && (configs[0][0] > 0 || configs[0][1] > 0));

        pluginInstance->setPlayConfigDetails (configs[0][0], configs[0][1], state->sampleRate, samplesPerBlock);
       #else
        pluginInstance->setRateAndBufferSizeDetails (state->sampleRate, samplesPerBlock);
       #endif

        pluginInstance->prepareToPlay (state->sampleRate, samplesPerBlock);

        scratchBuffer.setSize (jmax (pluginInstance->getTotalNumInputChannels(), pluginInstance->getTotalNumOutputChannels()), samplesPerBlock);
    }

    void release()
    {
        pluginInstance->releaseResources();
    }

    void reset()
    {
        pluginInstance->reset();
    }

    void process (float* inBuffer, float* outBuffer, int bufferSize, int numInChannels, int numOutChannels, bool isBypassed)
    {
        // If the plugin has a bypass parameter, set it to the current bypass state
        if (auto* param = pluginInstance->getBypassParameter())
            if (isBypassed != (param->getValue() >= 0.5f))
                param->setValueNotifyingHost (isBypassed ? 1.0f : 0.0f);

        for (int pos = 0; pos < bufferSize;)
        {
            auto max = jmin (bufferSize - pos, samplesPerBlock);
            processBuffers (inBuffer + (pos * numInChannels), outBuffer + (pos * numOutChannels), max, numInChannels, numOutChannels, isBypassed);

            pos += max;
        }
    }

    void declareParameters (UnityAudioEffectDefinition& definition)
    {
        static std::unique_ptr<UnityAudioParameterDefinition> parametersPtr;
        static int numParams = 0;

        if (parametersPtr == nullptr)
        {
            numParams = (int) juceParameters.size();

            parametersPtr.reset (static_cast<UnityAudioParameterDefinition*> (std::calloc (static_cast<size_t> (numParams),
                                                                              sizeof (UnityAudioParameterDefinition))));

            parameterDescriptions.clear();

            for (int i = 0; i < numParams; ++i)
            {
                auto* parameter = juceParameters.getParamForIndex (i);
                auto& paramDef = parametersPtr.get()[i];

                const auto nameLength = (size_t) numElementsInArray (paramDef.name);
                const auto unitLength = (size_t) numElementsInArray (paramDef.unit);

                parameter->getName ((int) nameLength - 1).copyToUTF8 (paramDef.name, nameLength);

                if (parameter->getLabel().isNotEmpty())
                    parameter->getLabel().copyToUTF8 (paramDef.unit, unitLength);

                parameterDescriptions.add (parameter->getName (15));
                paramDef.description = parameterDescriptions[i].toRawUTF8();

                paramDef.defaultVal = parameter->getDefaultValue();
                paramDef.min = 0.0f;
                paramDef.max = 1.0f;
                paramDef.displayScale = 1.0f;
                paramDef.displayExponent = 1.0f;
            }
        }

        definition.numParameters = static_cast<uint32> (numParams);
        definition.parameterDefintions = parametersPtr.get();
    }

    void setParameter (int index, float value)       { juceParameters.getParamForIndex (index)->setValueNotifyingHost (value); }
    float getParameter (int index) const noexcept    { return juceParameters.getParamForIndex (index)->getValue(); }

    String getParameterString (int index) const noexcept
    {
        auto* param = juceParameters.getParamForIndex (index);
        return param->getText (param->getValue(), 16);
    }

    int getNumInputChannels() const noexcept         { return pluginInstance->getTotalNumInputChannels(); }
    int getNumOutputChannels() const noexcept        { return pluginInstance->getTotalNumOutputChannels(); }

    bool hasEditor() const noexcept                  { return pluginInstance->hasEditor(); }

    UnityPeer& getEditorPeer() const
    {
        auto* peer = dynamic_cast<UnityPeer*> (pluginInstanceEditor->getPeer());

        jassert (peer != nullptr);
        return *peer;
    }

private:
    //==============================================================================
    void processBuffers (float* inBuffer, float* outBuffer, int bufferSize, int numInChannels, int numOutChannels, bool isBypassed)
    {
        int ch;
        for (ch = 0; ch < numInChannels; ++ch)
        {
            using DstSampleType = AudioData::Pointer<AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::NonConst>;
            using SrcSampleType = AudioData::Pointer<AudioData::Float32, AudioData::NativeEndian, AudioData::Interleaved,    AudioData::Const>;

            DstSampleType dstData (scratchBuffer.getWritePointer (ch));
            SrcSampleType srcData (inBuffer + ch, numInChannels);
            dstData.convertSamples (srcData, bufferSize);
        }

        for (; ch < numOutChannels; ++ch)
            scratchBuffer.clear (ch, 0, bufferSize);

        {
            const ScopedLock sl (pluginInstance->getCallbackLock());

            if (pluginInstance->isSuspended())
            {
                scratchBuffer.clear();
            }
            else
            {
                MidiBuffer mb;

                if (isBypassed && pluginInstance->getBypassParameter() == nullptr)
                    pluginInstance->processBlockBypassed (scratchBuffer, mb);
                else
                    pluginInstance->processBlock (scratchBuffer, mb);
            }
        }

        for (ch = 0; ch < numOutChannels; ++ch)
        {
            using DstSampleType = AudioData::Pointer<AudioData::Float32, AudioData::NativeEndian, AudioData::Interleaved,    AudioData::NonConst>;
            using SrcSampleType = AudioData::Pointer<AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::Const>;

            DstSampleType dstData (outBuffer + ch, numOutChannels);
            SrcSampleType srcData (scratchBuffer.getReadPointer (ch));
            dstData.convertSamples (srcData, bufferSize);
        }
    }

    //==============================================================================
    std::unique_ptr<AudioProcessor> pluginInstance;
    std::unique_ptr<AudioProcessorEditor> pluginInstanceEditor;

    int samplesPerBlock = 1024;
    StringArray parameterDescriptions;

    AudioBuffer<float> scratchBuffer;

    LegacyAudioParametersWrapper juceParameters;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorUnityWrapper)
};

//==============================================================================
HashMap<int, AudioProcessorUnityWrapper*>& getWrapperMap()
{
    static HashMap<int, AudioProcessorUnityWrapper*> wrapperMap;
    return wrapperMap;
}

static void onWrapperCreation (AudioProcessorUnityWrapper* wrapperToAdd)
{
    getWrapperMap().set (std::abs (Random::getSystemRandom().nextInt (65536)), wrapperToAdd);
}

static void onWrapperDeletion (AudioProcessorUnityWrapper* wrapperToRemove)
{
    getWrapperMap().removeValue (wrapperToRemove);
}

//==============================================================================
namespace UnityCallbacks
{
    int UNITY_INTERFACE_API createCallback (UnityAudioEffectState* state)
    {
        auto* pluginInstance = new AudioProcessorUnityWrapper (false);
        pluginInstance->create (state);

        state->effectData = pluginInstance;

        onWrapperCreation (pluginInstance);

        return 0;
    }

    int UNITY_INTERFACE_API releaseCallback (UnityAudioEffectState* state)
    {
        auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();
        pluginInstance->release();

        onWrapperDeletion (pluginInstance);
        delete pluginInstance;

        if (getWrapperMap().size() == 0)
            shutdownJuce_GUI();

        return 0;
    }

    int UNITY_INTERFACE_API resetCallback (UnityAudioEffectState* state)
    {
        auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();
        pluginInstance->reset();

        return 0;
    }

    int UNITY_INTERFACE_API setPositionCallback (UnityAudioEffectState* state, unsigned int pos)
    {
        ignoreUnused (state, pos);

        return 0;
    }

    int UNITY_INTERFACE_API setFloatParameterCallback (UnityAudioEffectState* state, int index, float value)
    {
        auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();
        pluginInstance->setParameter (index, value);

        return 0;
    }

    int UNITY_INTERFACE_API getFloatParameterCallback (UnityAudioEffectState* state, int index, float* value, char* valueStr)
    {
        auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();
        *value = pluginInstance->getParameter (index);

        pluginInstance->getParameterString (index).copyToUTF8 (valueStr, 15);

        return 0;
    }

    int UNITY_INTERFACE_API getFloatBufferCallback (UnityAudioEffectState* state, const char* name, float* buffer, int numSamples)
    {
        ignoreUnused (numSamples);

        auto nameStr = String (name);

        if (nameStr == "Editor")
        {
            auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();

            buffer[0] = pluginInstance->hasEditor() ? 1.0f : 0.0f;
        }
        else if (nameStr == "ID")
        {
            auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();

            for (HashMap<int, AudioProcessorUnityWrapper*>::Iterator i (getWrapperMap()); i.next();)
            {
                if (i.getValue() == pluginInstance)
                {
                    buffer[0] = (float) i.getKey();
                    break;
                }
            }

            return 0;
        }
        else if (nameStr == "Size")
        {
            auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();

            auto& editor = pluginInstance->getEditorPeer().getEditor();

            buffer[0] = (float) editor.getBounds().getWidth();
            buffer[1] = (float) editor.getBounds().getHeight();
            buffer[2] = (float) editor.getConstrainer()->getMinimumWidth();
            buffer[3] = (float) editor.getConstrainer()->getMinimumHeight();
            buffer[4] = (float) editor.getConstrainer()->getMaximumWidth();
            buffer[5] = (float) editor.getConstrainer()->getMaximumHeight();
        }

        return 0;
    }

    int UNITY_INTERFACE_API processCallback (UnityAudioEffectState* state, float* inBuffer, float* outBuffer,
                                             unsigned int bufferSize, int numInChannels, int numOutChannels)
    {
        auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();

        if (pluginInstance != nullptr)
        {
            auto isPlaying = ((state->flags & stateIsPlaying) != 0);
            auto isMuted   = ((state->flags & stateIsMuted)   != 0);
            auto isPaused  = ((state->flags & stateIsPaused)  != 0);

            const auto bypassed = ! isPlaying || (isMuted || isPaused);
            pluginInstance->process (inBuffer, outBuffer, static_cast<int> (bufferSize), numInChannels, numOutChannels, bypassed);
        }
        else
        {
            FloatVectorOperations::clear (outBuffer, static_cast<int> (bufferSize) * numOutChannels);
        }

        return 0;
    }
}

//==============================================================================
static void declareEffect (UnityAudioEffectDefinition& definition)
{
    memset (&definition, 0, sizeof (definition));

    std::unique_ptr<AudioProcessorUnityWrapper> wrapper = std::make_unique<AudioProcessorUnityWrapper> (true);

    String name (JucePlugin_Name);
    if (! name.startsWithIgnoreCase ("audioplugin"))
        name = "audioplugin_" + name;

    name.copyToUTF8 (definition.name, (size_t) numElementsInArray (definition.name));

    definition.structSize = sizeof (UnityAudioEffectDefinition);
    definition.parameterStructSize = sizeof (UnityAudioParameterDefinition);

    definition.apiVersion = UNITY_AUDIO_PLUGIN_API_VERSION;
    definition.pluginVersion = JucePlugin_VersionCode;

    // effects must set this to 0, generators > 0
    definition.channels = (wrapper->getNumInputChannels() != 0 ? 0
                                                               : static_cast<uint32> (wrapper->getNumOutputChannels()));

    wrapper->declareParameters (definition);

    definition.create            = UnityCallbacks::createCallback;
    definition.release           = UnityCallbacks::releaseCallback;
    definition.reset             = UnityCallbacks::resetCallback;
    definition.setPosition       = UnityCallbacks::setPositionCallback;
    definition.process           = UnityCallbacks::processCallback;
    definition.setFloatParameter = UnityCallbacks::setFloatParameterCallback;
    definition.getFloatParameter = UnityCallbacks::getFloatParameterCallback;
    definition.getFloatBuffer    = UnityCallbacks::getFloatBufferCallback;
}

} // namespace juce

UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API UnityGetAudioEffectDefinitions (UnityAudioEffectDefinition*** definitionsPtr)
{
    if (juce::getWrapperMap().size() == 0)
        juce::initialiseJuce_GUI();

    static bool hasInitialised = false;

    if (! hasInitialised)
    {
        juce::PluginHostType::jucePlugInClientCurrentWrapperType = juce::AudioProcessor::wrapperType_Unity;
        juce::juce_createUnityPeerFn = juce::createUnityPeer;

        hasInitialised = true;
    }

    auto* definition = new UnityAudioEffectDefinition();
    juce::declareEffect (*definition);

    *definitionsPtr = &definition;

    return 1;
}

//==============================================================================
static juce::ModifierKeys unityModifiersToJUCE (UnityEventModifiers mods, bool mouseDown, int mouseButton = -1)
{
    int flags = 0;

    if (mouseDown)
    {
        if (mouseButton == 0)
            flags |= juce::ModifierKeys::leftButtonModifier;
        else if (mouseButton == 1)
            flags |= juce::ModifierKeys::rightButtonModifier;
        else if (mouseButton == 2)
            flags |= juce::ModifierKeys::middleButtonModifier;
    }

    if (mods == 0)
        return flags;

    if ((mods & UnityEventModifiers::shift) != 0)        flags |= juce::ModifierKeys::shiftModifier;
    if ((mods & UnityEventModifiers::control) != 0)      flags |= juce::ModifierKeys::ctrlModifier;
    if ((mods & UnityEventModifiers::alt) != 0)          flags |= juce::ModifierKeys::altModifier;
    if ((mods & UnityEventModifiers::command) != 0)      flags |= juce::ModifierKeys::commandModifier;

    return { flags };
}

//==============================================================================
static juce::AudioProcessorUnityWrapper* getWrapperChecked (int id)
{
    auto* wrapper = juce::getWrapperMap()[id];
    jassert (wrapper != nullptr);

    return wrapper;
}

//==============================================================================
static void UNITY_INTERFACE_API onRenderEvent (int id)
{
    getWrapperChecked (id)->getEditorPeer().triggerAsyncUpdate();
}

UNITY_INTERFACE_EXPORT renderCallback UNITY_INTERFACE_API getRenderCallback()
{
    return onRenderEvent;
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API unityInitialiseTexture (int id, void* data, int w, int h)
{
    getWrapperChecked (id)->getEditorPeer().setPixelDataHandle (reinterpret_cast<juce::uint8*> (data), w, h);
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API unityMouseDown (int id, float x, float y, UnityEventModifiers unityMods, int button)
{
    getWrapperChecked (id)->getEditorPeer().forwardMouseEvent ({ x, y }, unityModifiersToJUCE (unityMods, true, button));
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API unityMouseDrag (int id, float x, float y, UnityEventModifiers unityMods, int button)
{
    getWrapperChecked (id)->getEditorPeer().forwardMouseEvent ({ x, y }, unityModifiersToJUCE (unityMods, true, button));
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API unityMouseUp (int id, float x, float y, UnityEventModifiers unityMods)
{
    getWrapperChecked (id)->getEditorPeer().forwardMouseEvent ({ x, y }, unityModifiersToJUCE (unityMods, false));
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API unityKeyEvent (int id, int code, UnityEventModifiers mods, const char* name)
{
    getWrapperChecked (id)->getEditorPeer().forwardKeyPress (code, name, unityModifiersToJUCE (mods, false));
}

UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API unitySetScreenBounds (int id, float x, float y, float w, float h)
{
    getWrapperChecked (id)->getEditorPeer().getEditor().setBounds ({ (int) x, (int) y, (int) w, (int) h });
}

//==============================================================================
#if JUCE_WINDOWS
 extern "C" BOOL WINAPI DllMain (HINSTANCE instance, DWORD reason, LPVOID)
 {
     if (reason == DLL_PROCESS_ATTACH)
         juce::Process::setCurrentModuleInstanceHandle (instance);

     return true;
 }
#endif

#endif
