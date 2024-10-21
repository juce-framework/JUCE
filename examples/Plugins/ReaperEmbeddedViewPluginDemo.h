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

 name:             ReaperEmbeddedViewDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      An audio plugin which embeds a secondary view in VST2 and
                   VST3 formats in REAPER

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_plugin_client, juce_audio_processors,
                   juce_audio_utils, juce_core, juce_data_structures,
                   juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             AudioProcessor
 mainClass:        ReaperEmbeddedViewDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

/*  This demo shows how to use the VST2ClientExtensions and VST3ClientExtensions
    classes to provide extended functionality in compatible VST/VST3 hosts.

    If this project is built as a VST or VST3 plugin and loaded in REAPER
    6.29 or higher, it will provide an embedded level meter in the track
    control panel. To enable the embedded view, right-click on the plugin
    and select "Show embedded UI in TCP".

    The plugin's editor also include a button which can be used to toggle
    all inserts on and off.
*/

#pragma once

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wshadow-field-in-constructor",
                                     "-Wnon-virtual-dtor")

#include <pluginterfaces/base/ftypes.h>
#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/vst/ivsthostapplication.h>
#include <pluginterfaces/vst2.x/aeffect.h>

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

namespace reaper
{
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant",
                                         "-Wunused-parameter",
                                         "-Wnon-virtual-dtor")
    JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4100)

    using namespace Steinberg;
    using INT_PTR = pointer_sized_int;
    using uint32 = Steinberg::uint32;

    #include "extern/reaper_plugin_fx_embed.h"
    #include "extern/reaper_vst3_interfaces.h"

    //==============================================================================
    /*  These should live in a file which is guaranteed to be compiled only once
        (i.e. a .cpp file, normally). This demo is a bit special, because we know
        that this header will only be included in a single translation unit.
     */
    DEF_CLASS_IID (IReaperHostApplication)
    DEF_CLASS_IID (IReaperUIEmbedInterface)

    JUCE_END_IGNORE_WARNINGS_MSVC
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

//==============================================================================
struct EmbeddedViewListener
{
    virtual ~EmbeddedViewListener() = default;
    virtual Steinberg::TPtrInt handledEmbeddedUIMessage (int msg,
                                                         Steinberg::TPtrInt parm2,
                                                         Steinberg::TPtrInt parm3) = 0;

    virtual void setGlobalBypassFunction (void (*) (int)) = 0;
};

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wnon-virtual-dtor")

//==============================================================================
class EmbeddedUI final : public reaper::IReaperUIEmbedInterface
{
public:
    explicit EmbeddedUI (EmbeddedViewListener& demo) : listener (demo) {}

    Steinberg::TPtrInt embed_message (int msg,
                                      Steinberg::TPtrInt parm2,
                                      Steinberg::TPtrInt parm3) override
    {
        return listener.handledEmbeddedUIMessage (msg, parm2, parm3);
    }

    Steinberg::uint32 PLUGIN_API addRef() override   { return ++refCount; }
    Steinberg::uint32 PLUGIN_API release() override  { return --refCount; }

    Steinberg::tresult PLUGIN_API queryInterface (const Steinberg::TUID tuid, void** obj) override
    {
        if (std::memcmp (tuid, iid, sizeof (Steinberg::TUID)) == 0)
        {
            ++refCount;
            *obj = this;
            return Steinberg::kResultOk;
        }

        *obj = nullptr;
        return Steinberg::kNoInterface;
    }

private:
    EmbeddedViewListener& listener;
    std::atomic<Steinberg::uint32> refCount { 1 };
};

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

class VST2Extensions final : public VST2ClientExtensions
{
public:
    explicit VST2Extensions (EmbeddedViewListener& l)
        : listener (l) {}

    pointer_sized_int handleVstPluginCanDo (int32, pointer_sized_int, void* ptr, float) override
    {
        if (auto* str = static_cast<const char*> (ptr))
            for (auto* key : { "hasCockosEmbeddedUI", "hasCockosExtensions" })
                if (strcmp (str, key) == 0)
                    return (pointer_sized_int) 0xbeef0000;

        return 0;
    }

    pointer_sized_int handleVstManufacturerSpecific (int32 index,
                                                     pointer_sized_int value,
                                                     void* ptr,
                                                     float opt) override
    {
        // The docstring at the top of reaper_plugin_fx_embed.h specifies
        // that the index will always be effEditDraw, which is now deprecated.
        if (index != __effEditDrawDeprecated)
            return 0;

        return (pointer_sized_int) listener.handledEmbeddedUIMessage ((int) opt,
                                                                      (Steinberg::TPtrInt) value,
                                                                      (Steinberg::TPtrInt) ptr);
    }

    void handleVstHostCallbackAvailable (std::function<VstHostCallbackType>&& hostcb) override
    {
        char functionName[] = "BypassFxAllTracks";
        listener.setGlobalBypassFunction (reinterpret_cast<void (*) (int)> (hostcb ((int32_t) 0xdeadbeef, (int32_t) 0xdeadf00d, 0, functionName, 0.0)));
    }

private:
    EmbeddedViewListener& listener;
};

class VST3Extensions final : public VST3ClientExtensions
{
public:
    explicit VST3Extensions (EmbeddedViewListener& l)
        : listener (l) {}

    int32_t queryIEditController (const Steinberg::TUID tuid, void** obj) override
    {
        if (embeddedUi.queryInterface (tuid, obj) == Steinberg::kResultOk)
            return Steinberg::kResultOk;

        *obj = nullptr;
        return Steinberg::kNoInterface;
    }

    void setIHostApplication (Steinberg::FUnknown* ptr) override
    {
        if (ptr == nullptr)
            return;

        void* objPtr = nullptr;

        if (ptr->queryInterface (reaper::IReaperHostApplication::iid, &objPtr) == Steinberg::kResultOk)
        {
            if (void* fnPtr = static_cast<reaper::IReaperHostApplication*> (objPtr)->getReaperApi ("BypassFxAllTracks"))
                listener.setGlobalBypassFunction (reinterpret_cast<void (*) (int)> (fnPtr));
        }
    }

private:
    EmbeddedViewListener& listener;
    EmbeddedUI embeddedUi { listener };
};

//==============================================================================
class Editor final : public AudioProcessorEditor
{
public:
    explicit Editor (AudioProcessor& proc,
                     AudioParameterFloat& param,
                     void (*globalBypass) (int))
        : AudioProcessorEditor (proc), attachment (param, slider)
    {
        addAndMakeVisible (slider);
        addAndMakeVisible (bypassButton);

        // Clicking will bypass *everything*
        bypassButton.onClick = [globalBypass] { NullCheckedInvocation::invoke (globalBypass, -1); };

        setSize (300, 80);
    }

    void resized() override
    {
        auto b = getLocalBounds();
        slider.setBounds (b.removeFromTop (40));
        bypassButton.setBounds (b);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::darkgrey);
    }

private:
    Slider slider;
    TextButton bypassButton { "global bypass" };
    SliderParameterAttachment attachment;
};

//==============================================================================
class ReaperEmbeddedViewDemo final : public AudioProcessor,
                                     private EmbeddedViewListener,
                                     private Timer
{
public:
    ReaperEmbeddedViewDemo()
    {
        addParameter (gain = new AudioParameterFloat ({ "gain", 1 }, "Gain", 0.0f, 1.0f, 0.5f));
        startTimerHz (60);
    }

    void prepareToPlay (double, int) override {}
    void reset() override {}

    void releaseResources() override {}

    void processBlock (AudioBuffer<float>&  audio, MidiBuffer&) override { processBlockImpl (audio); }
    void processBlock (AudioBuffer<double>& audio, MidiBuffer&) override { processBlockImpl (audio); }

    //==============================================================================
    AudioProcessorEditor* createEditor() override { return new Editor (*this, *gain, globalBypassFn); }
    bool hasEditor() const override               { return true;   }

    //==============================================================================
    const String getName() const override { return "ReaperEmbeddedViewDemo"; }

    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms()    override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const String getProgramName (int) override { return "None"; }

    void changeProgramName (int, const String&) override {}

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        MemoryOutputStream (destData, true).writeFloat (*gain);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        gain->setValueNotifyingHost (MemoryInputStream (data,
                                                        static_cast<size_t> (sizeInBytes),
                                                        false).readFloat());
    }

    VST2ClientExtensions* getVST2ClientExtensions() override { return &vst2Extensions; }
    VST3ClientExtensions* getVST3ClientExtensions() override { return &vst3Extensions; }

private:
    template <typename Float>
    void processBlockImpl (AudioBuffer<Float>& audio)
    {
        audio.applyGain (*gain);

        const auto minMax = audio.findMinMax (0, 0, audio.getNumSamples());
        const auto newMax = (float) std::max (std::abs (minMax.getStart()), std::abs (minMax.getEnd()));

        auto loaded = storedLevel.load();
        while (loaded < newMax && ! storedLevel.compare_exchange_weak (loaded, newMax)) {}
    }

    void timerCallback() override
    {
        levelToDraw = std::max (levelToDraw * 0.95f, storedLevel.exchange (0.0f));
    }

    Steinberg::TPtrInt getSizeInfo (reaper::REAPER_FXEMBED_SizeHints* sizeHints)
    {
        if (sizeHints == nullptr)
            return 0;

        sizeHints->preferred_aspect = 1 << 16;
        sizeHints->minimum_aspect   = 1 << 16;
        sizeHints->min_height = sizeHints->min_width = 50;
        sizeHints->max_height = sizeHints->max_width = 1000;
        return 1;
    }

    Steinberg::TPtrInt doPaint (reaper::REAPER_FXEMBED_IBitmap* bitmap,
                                reaper::REAPER_FXEMBED_DrawInfo* drawInfo)
    {
        if (bitmap == nullptr || drawInfo == nullptr || bitmap->getWidth() <= 0 || bitmap->getHeight() <= 0)
            return 0;

        Image img (juce::Image::PixelFormat::ARGB, bitmap->getWidth(), bitmap->getHeight(), true);
        Graphics g (img);

        g.fillAll (Colours::black);

        const auto bounds = g.getClipBounds();
        const auto corner = 3.0f;

        g.setColour (Colours::darkgrey);
        g.fillRoundedRectangle (bounds.withSizeKeepingCentre (20, bounds.getHeight() - 6).toFloat(),
                                corner);

        const auto minDb = -50.0f;
        const auto maxDb = 6.0f;
        const auto levelInDb = Decibels::gainToDecibels (levelToDraw, minDb);
        const auto fractionOfHeight = jmap (levelInDb, minDb, maxDb, 0.0f, 1.0f);
        const auto trackBounds = bounds.withSizeKeepingCentre (16, bounds.getHeight() - 10).toFloat();

        g.setColour (Colours::black);
        const auto zeroDbIndicatorY = trackBounds.proportionOfHeight (jmap (0.0f,
                                                                            minDb,
                                                                            maxDb,
                                                                            0.0f,
                                                                            1.0f));
        g.drawHorizontalLine ((int) (trackBounds.getBottom() - zeroDbIndicatorY),
                              trackBounds.getX(),
                              trackBounds.getRight());

        g.setGradientFill (ColourGradient (Colours::darkgreen,
                                           { 0.0f, (float) bounds.getHeight() },
                                           Colours::darkred,
                                           { 0.0f, 0.0f },
                                           false));

        g.fillRoundedRectangle (trackBounds.withHeight (trackBounds.proportionOfHeight (fractionOfHeight))
                                           .withBottomY (trackBounds.getBottom()),
                                corner);

        Image::BitmapData imgData { img, Image::BitmapData::readOnly };
        const auto pixelsWidth = imgData.pixelStride * imgData.width;

        auto* px = bitmap->getBits();
        const auto rowSpan = bitmap->getRowSpan();
        const auto numRows = bitmap->getHeight();

        for (int y = 0; y < numRows; ++y)
            std::memcpy (px + (y * rowSpan), imgData.getLinePointer (y), (size_t) pixelsWidth);

        return 1;
    }

    Steinberg::TPtrInt handledEmbeddedUIMessage (int msg,
                                                 Steinberg::TPtrInt parm2,
                                                 Steinberg::TPtrInt parm3) override
    {
        switch (msg)
        {
            case REAPER_FXEMBED_WM_IS_SUPPORTED:
                return 1;

            case REAPER_FXEMBED_WM_PAINT:
                return doPaint (reinterpret_cast<reaper::REAPER_FXEMBED_IBitmap*> (parm2),
                                reinterpret_cast<reaper::REAPER_FXEMBED_DrawInfo*> (parm3));

            case REAPER_FXEMBED_WM_GETMINMAXINFO:
                return getSizeInfo (reinterpret_cast<reaper::REAPER_FXEMBED_SizeHints*> (parm3));

            // Implementing mouse behaviour is left as an exercise for the reaper, I mean reader
            case REAPER_FXEMBED_WM_CREATE:          break;
            case REAPER_FXEMBED_WM_DESTROY:         break;
            case REAPER_FXEMBED_WM_SETCURSOR:       break;
            case REAPER_FXEMBED_WM_MOUSEMOVE:       break;
            case REAPER_FXEMBED_WM_LBUTTONDOWN:     break;
            case REAPER_FXEMBED_WM_LBUTTONUP:       break;
            case REAPER_FXEMBED_WM_LBUTTONDBLCLK:   break;
            case REAPER_FXEMBED_WM_RBUTTONDOWN:     break;
            case REAPER_FXEMBED_WM_RBUTTONUP:       break;
            case REAPER_FXEMBED_WM_RBUTTONDBLCLK:   break;
            case REAPER_FXEMBED_WM_MOUSEWHEEL:      break;
        }

        return 0;
    }

    void setGlobalBypassFunction (void (*fn) (int)) override { globalBypassFn = fn; }

    AudioParameterFloat* gain = nullptr;
    void (*globalBypassFn) (int) = nullptr;

    std::atomic<float> storedLevel { 0.0f };
    float levelToDraw = 0.0f;

    VST2Extensions vst2Extensions { *this };
    VST3Extensions vst3Extensions { *this };
};
