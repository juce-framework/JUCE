# JUCE_ARA

This repository is an experimental fork of the [JUCE develop branch](https://github.com/juce-framework/JUCE) with additions that enable it to generate [ARA plugins](https://www.celemony.com/en/service1/about-celemony/technologies) in the VST3 or AudioUnit format.
It is currently being maintained by [Celemony](https://www.celemony.com) and [SoundRadix](https://www.soundradix.com), with the goal of being picked up eventually for main line [JUCE](https://www.juce.com) once the code is stable.

Note that Celemony is not endorsing the use of JUCE by providing this fork. JUCE_ARA is a fairly thin adapter to integrate ARA into JUCE, it does not provide any features that would be relevant when using ARA with a different framework. Consequently, the decision whether or not to use JUCE for any given project should be made independently of JUCE_ARA.

In order to use this branch you'll need access to the [ARA SDK](http://www.celemony.com/en/service1/about-celemony/technologies) - if you're a developer and would like access to the latest ARA SDK release, send an email to [ara@celemony.com](mailto:ara@celemony.com?Subject=JUCE%20ARA%20integration).
You'll also need to use our ARA-enabled build of the Projucer that's part of this fork.
For feedback and questions, please contact Celemony via [ara@celemony.com](mailto:ara@celemony.com?Subject=JUCE%20ARA%20integration).

Current ARA SDK compatibility version: 1.9.14


## JUCE and the ARA API

### ARADocumentController

As is the case in the ARA C++ library, the `juce::ARADocumentController` class must be subclassed by your
ARA plugin. The ARA host will use your document controller to build a representation of the ARA document for
your plugin - by default the `juce::ARADocumentController` will create the ARA model object classes outlined below, 
but your plugin should override the `doCreate` functions in order to create model objects that suit your purpose. 

For example, in the 
[`ARAPluginDemoDocumentController`](https://github.com/Celemony/JUCE_ARA/tree/develop/examples/Plugins/ARAPluginDemo/Source/ARAPluginDemoDocumentController.h)
we override `doCreateAudioModification` in order to return a [custom `juce::ARAAudioModification` subclass](https://github.com/Celemony/JUCE_ARA/tree/develop/examples/Plugins/ARAPluginDemo/Source/ARAPluginDemoAudioModification.h). 

### ARA Model Objects

To make ARA easier to integrate with existing JUCE code, we've subclassed the C++ classes provided 
in the ARA SDK into counterparts in the `juce` namespace:
- `ARA::PlugIn::Document` ==> `juce::ARADocument`
- `ARA::PlugIn::MusicalContext` ==> `juce::ARAMusicalContext`
- `ARA::PlugIn::RegionSequence` ==> `juce::ARARegionSequence`
- `ARA::PlugIn::AudioSource` ==> `juce::ARAAudioSource`
- `ARA::PlugIn::AudioModification` ==> `juce::ARAAudioModification`
- `ARA::PlugIn::PlaybackRegion` ==> `juce::ARAPlaybackRegion`

These model objects can be subclassed further by overriding the `ARADocumentController::doCreate` 
functions as needed, i.e
```
// declare an ARAAudioSource subclass
class MyCustomAudioSource : public juce::ARAAudioSource
{
    // inherit the juce::ARAAudioSource constructor
    using juce::ARAAudioSource::ARAAudioSource;
};

// override doCreateAudioSource() to return your custom subclass
class MyCustomDocumentController : public juce::ARADocumentController
{
    ARA::PlugIn::AudioSource* doCreateAudioSource (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef) 
    noexcept override
    {
        return new MyCustomAudioSource (static_cast<ARADocument*> (document), hostRef);
    }
};
```


### Listeners

To make things feel more "JUCEy", we've given each JUCE model object a `Listener` base class 
with virtual callbacks that can be overridden to receive notifications related to the ARA model graph 
and host rendering / UI state. These callbacks are meant to replace the `will/did` functions that would 
be overridden by an `ARA::PlugIn::DocumentController` implementation using the ARA C++ library. 

We've also added this `Listener` pattern to other classes, such as the 
`ARAEditorView` instance role which helps manage selection and UI state, see [below](#ara-plugin-instance-roles).

Listener updates are subscribed to on a per-object basis. For example, if you need to 
keep track of the properties of a particular `ARAPlaybackRegion`, you must subclass 
`ARAPlaybackRegion::Listener` and and add/remove yourself as a listener of that playback region
via `addListener` and `removeListener`.  

```
class PlaybackRegionManager  : public ARAPlaybackRegion::Listener
{
public:
    // use constructor/destructor to manage Listener subscription
    PlaybackRegionManager (ARAPlaybackRegion* region)
        : myRegion (region)
    {
        myRegion->addListener (this);
    }
    ~PlaybackRegionManager() 
    {
        myRegion->removeListener (this);
    }

    // ARAPlaybackRegion::Listener overrides
    void willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) override {}
    void didUpdatePlaybackRegionContent (ARAPlaybackRegion* playbackRegion, ARAContentUpdateScopes scopeFlags) override {}
    
private:
    ARAPlaybackRegion* myRegion;
};
```

See the [ARA Plugin Demo PlaybackRegionView class](https://github.com/Celemony/JUCE_ARA/tree/develop/examples/Plugins/ARAPluginDemo/Source/PlaybackRegionView.h)
for an example of several `Listener` implementations in action. 

### ARA PlugIn Instance Roles

When an ARA plug-in is instantiated by the host it will take on one or more instance roles 
as defined by the ARA API. To represent this concept in JUCE we've created two extension classes:

The `AudioProcessorARAExtension` class, meant to be subclassed by the JUCE plugin's `AudioProcessor`
implementation, allows access to all three plugin instance roles bound to the JUCE plugin instance.

Your plug-in can then fulfill its plug-in instance roles as needed, for example

```
class MyAudioProcessor    : public AudioProcessor,
                            public AudioProcessorARAExtension
{
public:
    // ...
    
    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        // if this plug-in instance is bound to ARA
        if (isBoundToARA())
        {
            // if this plug-in instance is an ARAPlaybackRenderer, render assigned playback regions
            if (auto araPlaybackRenderer = getARAPlaybackRenderer())
                for (auto playbackRegion : araPlaybackRenderer->getPlaybackRegions())
                    renderPlaybackRegion (buffer, playbackRegion);

            // if this plug-in instance is an ARAEditorRenderer, render assigned playback regions
            if (auto araEditorRenderer = getARAEditorRenderer())
                for (auto playbackRegion : araEditorRenderer->getPlaybackRegions())
                    renderPlaybackRegion (buffer, playbackRegion);
        }
    }
    
private:
    void renderPlaybackRegion (AudioBuffer<float>& buffer, ARAPlaybackRegion* playbackRegion) 
    {
        // perform your rendering here
    }
};
```

The `AudioProcessorEditorARAExtension` class, meant to be subclassed by the JUCE plugin's `AudioProcessorEditor`
implementation, allows access to the `ARAEditorView` role and helps the plugin interact with host selection
and UI state. Our `ARAEditorView` class also has a Listener class that can be used to recieve UI related callbacks. 

```
class MyAudioProcessorEditor  : public AudioProcessorEditor,
                                public AudioProcessorEditorARAExtension,
                                public ARAEditorView::Listener
{
public:
    MyAudioProcessorEditor (MyAudioProcessor& p)
        : AudioProcessorEditor (&p),
          AudioProcessorEditorARAExtension (&p)
    {
        if (isARAEditorView())
            getARAEditorView()->addListener (this);
    }
    ~MyAudioProcessorEditor()
    {
        if (isARAEditorView())
            getARAEditorView()->removeListener (this);
    }
    
    // ARAEditorView::Listener overrides
    void onNewSelection (const ARA::PlugIn::ViewSelection& viewSelection) override
    {
        for (auto playbackRegion : viewSelection.getPlaybackRegions())
            juce::Logger::writeToLog (playbackRegion->getName() + " is selected. ");
    }
};
```

### Audio Readers

Reading large buffers of audio samples at will is a key component of the ARA API. Because the
concept of audio readers already exists in JUCE, we've subclassed the existing `juce::AudioFormatReader` 
to read audio samples via ARA in a class called `ARAAudioSourceReader` that allows reading samples
from a single `ARAAudioSource`.

```
std::vector<float> readAudioSourceSamples (ARAAudioSource* audioSource)
{
    jassert (audioSource->isSampleAccessEnabled());

    // prepare output buffer
    ARA::ARASampleCount sampleCount = audioSource->getSampleCount();
    std::vector<float> sampleBuffer (sampleCount);
    float* sampleBufPointer = sampleBuffer.data();

    // read the first channel of audio source sample data
    ARAAudioSourceReader sourceReader(audioSource);
    sourceReader.read (&sampleBufPointer, 1, 0, sampleCount);
    
    return sampleBuffer;
}
```

We've also created an `ARAPlaybackRegionReader` class that can read samples as if they were being output by 
an `ARAPlaybackRenderer` instance. This is useful if you want to deal with playback regions instead of the original audio source samples. 

```
class MyAudioProcessor    : public AudioProcessor,
                            public AudioProcessorARAExtension
{
    MyAudioProcessor();
    // ...
};

std::vector<float> readPlaybackRegionSamples (ARAPlaybackRegion* playbackRegion)
{
    // prepare output buffer
    ARA::ARASampleCount sampleCount = 1024;
    std::vector<float> sampleBuffer (sampleCount);
    float* sampleBufPointer = sampleBuffer.data();

    // read the first channel of playback region sample data
    ARAPlaybackRegionReader regionReader (std::make_unique<MyAudioProcessor>(), { playbackRegion });
    regionReader.read (&sampleBufPointer, 1, 0, sampleCount);

    return sampleBuffer;
}
```

Once created, the our readers can be treated like any other `AudioFormatReader` - the 
[ARA Plugin Demo PlaybackRegionView class](https://github.com/Celemony/JUCE_ARA/tree/develop/examples/Plugins/ARAPluginDemo/Source/PlaybackRegionView.h)
takes advantage of this by giving a `juce::AudioThubnail` instance an `ARAPlaybackRegionReader` to draw a
visualization of playback region waveform. 

## Getting Started

### Using the ARA enabled Projucer

We've made modifications to the Projucer to enable it to work with the ARA 2.0 SDK. These modifications allow building VST3 and AudioUnit plugins that can be loaded as ARA plugins by a compatible host. 

To create an ARA plugin, clone the develop branch of Celemony's [JUCE fork](https://github.com/Celemony/JUCE_ARA/tree/develop) and build the [Projucer](https://github.com/Celemony/JUCE_ARA/tree/develop/extras/Projucer) (projects for Visual Studio and Xcode are checked in to the repository.)

Once you have the ARA version of the Projucer built, you must specify your global `ARA SDK` path by navigating to `File->Global Paths...`:

<img src="https://i.imgur.com/fRjU8kB.png"/>

This will become the default path for new ARA plug-in projects, but like other SDK paths it can  be configured per project.

### Creating an ARA enabled audio plug-in

Once the SDK has been located we can create a new ARA Audio Plug-In project like so

<img src="https://i.imgur.com/kCfoPbo.png"/>

The generated plug-in will have, in addition to the standard `PluginProcessor` and `PluginEditor` classes, a `PluginARADocumentController` file containing a class that implements an `ARADocumentController`. 

<img src="https://i.imgur.com/JqNIe2b.png"/>

This version of the Projucer supports ARA for VST3 and AudioUnit, so make sure at least one of these plugin formats is checked. 

<img src="https://i.imgur.com/TexUBqU.png"/>

### Enabling ARA in an existing JUCE audio plug-in

If you've already got a JUCE audio plug-in project that you'd like to convert to an ARA plug-in, you can enable ARA in our version of the Projucer using the `Enable ARA` checkbox. 

However, you'll have to add your own document`ARADocumentController` implementation - referencing the `PluginARADocumentController` class generated for new ARA Audio Plug-In projects is a good starting point.

<img src="https://i.imgur.com/VGcDWEc.png"/>

### Configuring the ARA plug-in properties

With ARA enabled we can edit various `ARAFactory` properties such as available content types and transformation flags as well as the plugin factory identifier.

<img src="https://i.imgur.com/mJoXIxG.png"/>

This should be enough to generate an empty ARA plugin that will appear in an ARA host, such as Studio One or SONAR. 
For a more complete example of an ARA plugin see the [ARA Plugin Demo](https://github.com/Celemony/JUCE_ARA/tree/develop/examples/Plugins/ARAPluginDemo) checked in to this repository. 

## Further Additions

Our goal with this fork is to make ARA plugin development as easy and accessible as possible. We'll 
continue to update this repository as new features are designed and developed. Some possible future additions:
- minimizing the casts required from `ARA::PlugIn` classes to their `juce` equivalents
- better UI integration with JUCE using `juce::LookAndFeel`
