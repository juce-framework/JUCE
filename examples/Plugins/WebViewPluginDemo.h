/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             WebViewPluginDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Filtering audio plugin using an HTML/JS user interface

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_plugin_client, juce_audio_processors, juce_dsp,
                   juce_audio_utils, juce_core, juce_data_structures,
                   juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1, JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1

 type:             AudioProcessor
 mainClass:        WebViewPluginAudioProcessorWrapper

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

namespace ID
{
#define PARAMETER_ID(str) static const ParameterID str { #str, 1 };

PARAMETER_ID (cutoffFreqHz)
PARAMETER_ID (mute)
PARAMETER_ID (filterType)

#undef PARAMETER_ID
}

class CircularBuffer
{
public:
    CircularBuffer (int numChannels, int numSamples)
        : buffer (data, (size_t) numChannels, (size_t) numSamples)
    {
    }

    template <typename T>
    void push (dsp::AudioBlock<T> b)
    {
        jassert (b.getNumChannels() == buffer.getNumChannels());

        const auto trimmed = b.getSubBlock (  b.getNumSamples()
                                            - std::min (b.getNumSamples(), buffer.getNumSamples()));

        const auto bufferLength = (int64) buffer.getNumSamples();

        for (auto samplesRemaining = (int64) trimmed.getNumSamples(); samplesRemaining > 0;)
        {
            const auto writeOffset = writeIx % bufferLength;
            const auto numSamplesToWrite = std::min (samplesRemaining, bufferLength - writeOffset);

            auto destSubBlock = buffer.getSubBlock ((size_t) writeOffset, (size_t) numSamplesToWrite);
            const auto sourceSubBlock = trimmed.getSubBlock (trimmed.getNumSamples() - (size_t) samplesRemaining,
                                                             (size_t) numSamplesToWrite);

            destSubBlock.copyFrom (sourceSubBlock);

            samplesRemaining -= numSamplesToWrite;
            writeIx += numSamplesToWrite;
        }
    }

    template <typename T>
    void push (Span<T> s)
    {
        auto* ptr = s.begin();
        dsp::AudioBlock<T> b (&ptr, 1, s.size());
        push (b);
    }

    void read (int64 readIx, dsp::AudioBlock<float> output) const
    {
        const auto numChannelsToUse = std::min (buffer.getNumChannels(), output.getNumChannels());

        jassert (output.getNumChannels() == buffer.getNumChannels());

        const auto bufferLength = (int64) buffer.getNumSamples();

        for (auto outputOffset = (size_t) 0; outputOffset < output.getNumSamples();)
        {
            const auto inputOffset = (size_t) ((readIx + (int64) outputOffset) % bufferLength);
            const auto numSamplesToRead = std::min (output.getNumSamples() - outputOffset,
                                                    (size_t) bufferLength - inputOffset);

            auto destSubBlock = output.getSubBlock (outputOffset, numSamplesToRead)
                                      .getSubsetChannelBlock (0, numChannelsToUse);

            destSubBlock.copyFrom (buffer.getSubBlock (inputOffset, numSamplesToRead)
                                         .getSubsetChannelBlock (0, numChannelsToUse));

            outputOffset += numSamplesToRead;
        }
    }

    int64 getWriteIndex() const noexcept { return writeIx; }

private:
    HeapBlock<char> data;
    dsp::AudioBlock<float> buffer;
    int64 writeIx = 0;
};

class SpectralBars
{
public:
    static constexpr int getNumBars() noexcept
    {
        return analysisWindowWidth / 2;
    }

    template <typename T>
    void push (Span<T> data)
    {
        buffer.push (data);
    }

    void compute (Span<float> output)
    {
        auto* ptr = output.begin();
        auto result = dsp::AudioBlock<float> (&ptr, 1, output.size());
        result.clear();
        auto analysisData = fftTmp.getSubBlock (0, analysisWindowWidth);

        for (int i = 0; i < numAnalysisWindows; ++i)
        {
            buffer.read (0 + i * analysisWindowOverlap, analysisData);
            fft.performFrequencyOnlyForwardTransform (fftTmp.getChannelPointer (0), true);
            result.add (analysisData);
        }

        result.multiplyBy (1.0f / numAnalysisWindows);
    }

    static inline constexpr int64 fftOrder            = 5;
    static inline constexpr int64 analysisWindowWidth = 1 << fftOrder;
    static inline constexpr int numAnalysisWindows    = 16;
    static inline constexpr int analysisWindowOverlap = analysisWindowWidth / 2;

private:
    dsp::FFT fft { fftOrder };

    HeapBlock<char> fftTmpData;
    dsp::AudioBlock<float> fftTmp { fftTmpData, 1, (size_t) (2 * fft.getSize()) };

    CircularBuffer buffer { 1,       (int) analysisWindowWidth
                                   + (numAnalysisWindows - 1) * analysisWindowOverlap };
};

//==============================================================================
class WebViewPluginAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    WebViewPluginAudioProcessor (AudioProcessorValueTreeState::ParameterLayout layout);

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    const String getName() const override        { return JucePlugin_Name; }

    bool acceptsMidi() const override            { return false; }
    bool producesMidi() const override           { return false; }
    bool isMidiEffect() const override           { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override                        { return 1; }
    int getCurrentProgram() override                     { return 0; }
    void setCurrentProgram (int) override                {}
    const String getProgramName (int) override           { return {}; }
    void changeProgramName (int, const String&) override {}

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    struct Parameters
    {
    public:
        explicit Parameters (AudioProcessorValueTreeState::ParameterLayout& layout)
            : cutoffFreqHz (addToLayout<AudioParameterFloat> (layout,
                                                              ID::cutoffFreqHz,
                                                              "Cutoff",
                                                              NormalisableRange<float> { 200.0f, 14000.0f, 1.0f, 0.5f },
                                                              11000.0f,
                                                              AudioParameterFloatAttributes{}.withLabel ("Hz"))),
              mute (addToLayout<AudioParameterBool> (layout, ID::mute, "Mute", false)),
              filterType (addToLayout<AudioParameterChoice> (layout,
                                                             ID::filterType,
                                                             "Filter type",
                                                             StringArray { "Low-pass", "High-pass", "Band-pass" },
                                                             0))
        {
        }

        AudioParameterFloat&  cutoffFreqHz;
        AudioParameterBool&   mute;
        AudioParameterChoice& filterType;

    private:
        template <typename Param>
        static void add (AudioProcessorParameterGroup& group, std::unique_ptr<Param> param)
        {
            group.addChild (std::move (param));
        }

        template <typename Param>
        static void add (AudioProcessorValueTreeState::ParameterLayout& group, std::unique_ptr<Param> param)
        {
            group.add (std::move (param));
        }

        template <typename Param, typename Group, typename... Ts>
        static Param& addToLayout (Group& layout, Ts&&... ts)
        {
            auto param = std::make_unique<Param> (std::forward<Ts> (ts)...);
            auto& ref = *param;
            add (layout, std::move (param));
            return ref;
        }
    };

    Parameters parameters;
    AudioProcessorValueTreeState state;

    std::vector<float> spectrumData = [] { return std::vector<float> (16, 0.0f); }();
    SpinLock spectrumDataLock;

    SpectralBars spectralBars;

    dsp::LadderFilter<float> filter;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebViewPluginAudioProcessor)
};

//==============================================================================
WebViewPluginAudioProcessor::WebViewPluginAudioProcessor (AudioProcessorValueTreeState::ParameterLayout layout)
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
      parameters (layout),
      state (*this, nullptr, "STATE", std::move (layout))
{
}

//==============================================================================
void WebViewPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto channels = std::max (getTotalNumInputChannels(), getTotalNumOutputChannels());

    if (channels == 0)
        return;

    filter.prepare ({ sampleRate, (uint32_t) samplesPerBlock, (uint32_t) channels });
    filter.reset();
}

bool WebViewPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void WebViewPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels  = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    filter.setCutoffFrequencyHz (parameters.cutoffFreqHz.get());

    const auto filterMode = [this]
    {
        switch (parameters.filterType.getIndex())
        {
            case 0:
                return dsp::LadderFilter<float>::Mode::LPF12;

            case 1:
                return dsp::LadderFilter<float>::Mode::HPF12;

            default:
                return dsp::LadderFilter<float>::Mode::BPF12;
        }
    }();

    filter.setMode (filterMode);

    auto outBlock = dsp::AudioBlock<float> { buffer }.getSubsetChannelBlock (0, (size_t) getTotalNumOutputChannels());

    if (parameters.mute.get())
        outBlock.clear();

    filter.process (dsp::ProcessContextReplacing<float> (outBlock));

    spectralBars.push (Span { buffer.getReadPointer (0), (size_t) buffer.getNumSamples() });

    {
        const SpinLock::ScopedTryLockType lock (spectrumDataLock);

        if (! lock.isLocked())
            return;

        spectralBars.compute ({ spectrumData.data(), spectrumData.size() });
    }
}

//==============================================================================
void WebViewPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ignoreUnused (destData);
}

void WebViewPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused (data, sizeInBytes);
}

extern const String localDevServerAddress;

std::optional<WebBrowserComponent::Resource> getResource (const String& url);

//==============================================================================
struct SinglePageBrowser : WebBrowserComponent
{
    using WebBrowserComponent::WebBrowserComponent;

    // Prevent page loads from navigating away from our single page web app
    bool pageAboutToLoad (const String& newURL) override;
};

//==============================================================================
class WebViewPluginAudioProcessorEditor  : public AudioProcessorEditor, private Timer
{
public:
    explicit WebViewPluginAudioProcessorEditor (WebViewPluginAudioProcessor&);

    std::optional<WebBrowserComponent::Resource> getResource (const String& url);

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    int getControlParameterIndex (Component&) override
    {
        return controlParameterIndexReceiver.getControlParameterIndex();
    }

    void timerCallback() override
    {
        static constexpr size_t numFramesBuffered = 5;

        SpinLock::ScopedLockType lock { processorRef.spectrumDataLock };

        Array<var> frame;

        for (size_t i = 1; i < processorRef.spectrumData.size(); ++i)
            frame.add (processorRef.spectrumData[i]);

        spectrumDataFrames.push_back (std::move (frame));

        while (spectrumDataFrames.size() > numFramesBuffered)
            spectrumDataFrames.pop_front();

        static int64 callbackCounter = 0;

        if (   spectrumDataFrames.size() == numFramesBuffered
            && callbackCounter++ % (int64) numFramesBuffered)
        {
            webComponent.emitEventIfBrowserIsVisible ("spectrumData", var{});
        }
    }

private:
    WebViewPluginAudioProcessor& processorRef;

    WebSliderRelay       cutoffSliderRelay    { "cutoffSlider" };
    WebToggleButtonRelay muteToggleRelay      { "muteToggle" };
    WebComboBoxRelay     filterTypeComboRelay { "filterTypeCombo" };

    WebControlParameterIndexReceiver controlParameterIndexReceiver;

    SinglePageBrowser webComponent { WebBrowserComponent::Options{}
                                         .withBackend (WebBrowserComponent::Options::Backend::webview2)
                                         .withWinWebView2Options (WebBrowserComponent::Options::WinWebView2{}
                                                                      .withUserDataFolder (File::getSpecialLocation (File::SpecialLocationType::tempDirectory)))
                                         .withNativeIntegrationEnabled()
                                         .withOptionsFrom (cutoffSliderRelay)
                                         .withOptionsFrom (muteToggleRelay)
                                         .withOptionsFrom (filterTypeComboRelay)
                                         .withOptionsFrom (controlParameterIndexReceiver)
                                         .withNativeFunction ("sayHello", [](auto& var, auto complete)
                                                              {
                                                                  complete ("Hello " + var[0].toString());
                                                              })
                                         .withResourceProvider ([this] (const auto& url)
                                                                {
                                                                    return getResource (url);
                                                                },
                                                                URL { localDevServerAddress }.getOrigin()) };

    WebSliderParameterAttachment       cutoffAttachment;
    WebToggleButtonParameterAttachment muteAttachment;
    WebComboBoxParameterAttachment     filterTypeAttachment;

    std::deque<Array<var>> spectrumDataFrames;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebViewPluginAudioProcessorEditor)
};

static ZipFile* getZipFile()
{
    static auto stream = createAssetInputStream ("webviewplugin-gui_1.0.0.zip", AssertAssetExists::no);

    if (stream == nullptr)
        return nullptr;

    static ZipFile f { stream.get(), false };
    return &f;
}

static const char* getMimeForExtension (const String& extension)
{
    static const std::unordered_map<String, const char*> mimeMap =
    {
        { { "htm"   },  "text/html"                },
        { { "html"  },  "text/html"                },
        { { "txt"   },  "text/plain"               },
        { { "jpg"   },  "image/jpeg"               },
        { { "jpeg"  },  "image/jpeg"               },
        { { "svg"   },  "image/svg+xml"            },
        { { "ico"   },  "image/vnd.microsoft.icon" },
        { { "json"  },  "application/json"         },
        { { "png"   },  "image/png"                },
        { { "css"   },  "text/css"                 },
        { { "map"   },  "application/json"         },
        { { "js"    },  "text/javascript"          },
        { { "woff2" },  "font/woff2"               }
    };

    if (const auto it = mimeMap.find (extension.toLowerCase()); it != mimeMap.end())
        return it->second;

    jassertfalse;
    return "";
}

static String getExtension (String filename)
{
    return filename.fromLastOccurrenceOf (".", false, false);
}

static auto streamToVector (InputStream& stream)
{
    std::vector<std::byte> result ((size_t) stream.getTotalLength());
    stream.setPosition (0);
    [[maybe_unused]] const auto bytesRead = stream.read (result.data(), result.size());
    jassert (bytesRead == (ssize_t) result.size());
    return result;
}

std::optional<WebBrowserComponent::Resource> WebViewPluginAudioProcessorEditor::getResource (const String& url)
{
    const auto urlToRetrive = url == "/" ? String { "index.html" }
                                         : url.fromFirstOccurrenceOf ("/", false, false);

    if (auto* archive = getZipFile())
    {
        if (auto* entry = archive->getEntry (urlToRetrive))
        {
            auto stream = rawToUniquePtr (archive->createStreamForEntry (*entry));
            auto v = streamToVector (*stream);
            auto mime = getMimeForExtension (getExtension (entry->filename).toLowerCase());
            return WebBrowserComponent::Resource { std::move (v),
                                                   std::move (mime) };
        }
    }

    if (urlToRetrive == "index.html")
    {
        auto fallbackIndexHtml = createAssetInputStream ("webviewplugin-gui-fallback.html");
        return WebBrowserComponent::Resource { streamToVector (*fallbackIndexHtml),
                                               String { "text/html" } };
    }

    if (urlToRetrive == "data.txt")
    {
        WebBrowserComponent::Resource resource;
        static constexpr char testData[] = "testdata";
        MemoryInputStream stream { testData, numElementsInArray (testData) - 1, false };
        return WebBrowserComponent::Resource { streamToVector (stream), String { "text/html" } };
    }

    if (urlToRetrive == "spectrumData.json")
    {
        Array<var> frames;

        for (const auto& frame : spectrumDataFrames)
            frames.add (frame);

        DynamicObject::Ptr d (new DynamicObject());
        d->setProperty ("timeResolutionMs", getTimerInterval());
        d->setProperty ("frames", std::move (frames));

        const auto s = JSON::toString (d.get());
        MemoryInputStream stream { s.getCharPointer(), s.getNumBytesAsUTF8(), false };
        return WebBrowserComponent::Resource { streamToVector (stream), String { "application/json" } };
    }

    return std::nullopt;
}

#if JUCE_ANDROID
// The localhost is available on this address to the emulator
const String localDevServerAddress = "http://10.0.2.2:3000/";
#else
const String localDevServerAddress = "http://localhost:3000/";
#endif

bool SinglePageBrowser::pageAboutToLoad (const String& newURL)
{
    return newURL == localDevServerAddress || newURL == getResourceProviderRoot();
}

//==============================================================================
WebViewPluginAudioProcessorEditor::WebViewPluginAudioProcessorEditor (WebViewPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
      cutoffAttachment (*processorRef.state.getParameter (ID::cutoffFreqHz.getParamID()),
                        cutoffSliderRelay,
                        processorRef.state.undoManager),
      muteAttachment (*processorRef.state.getParameter (ID::mute.getParamID()),
                      muteToggleRelay,
                      processorRef.state.undoManager),
      filterTypeAttachment (*processorRef.state.getParameter (ID::filterType.getParamID()),
                            filterTypeComboRelay,
                            processorRef.state.undoManager)
{
    addAndMakeVisible (webComponent);

    // webComponent.goToURL (localDevServerAddress);
    webComponent.goToURL (WebBrowserComponent::getResourceProviderRoot());

    setSize (500, 500);

    startTimerHz (20);
}

//==============================================================================
void WebViewPluginAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void WebViewPluginAudioProcessorEditor::resized()
{
    webComponent.setBounds (getLocalBounds());
}

class WebViewPluginAudioProcessorWrapper  : public WebViewPluginAudioProcessor
{
public:
    WebViewPluginAudioProcessorWrapper()  : WebViewPluginAudioProcessor ({})
    {}

    bool hasEditor() const override               { return true; }
    AudioProcessorEditor* createEditor() override { return new WebViewPluginAudioProcessorEditor (*this); }
};
