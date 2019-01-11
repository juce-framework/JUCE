## Understanding the ARA Sample Project


In addition to the sample plugin provided in the ARA SDK we've created a sample project showcasing the 
ARA additions to the JUCE API. The sample project can be found at 
[JUCE_ARA/examples/ARA/ARASampleProject](https://github.com/Celemony/JUCE_ARA/tree/develop/examples/ARA/ARASampleProject). Below is an example
of the plugin being hosted by Studio One. 

<img src="https://i.imgur.com/gK7GZq8.png"/>


The sample can be broken into five important classes:


#### `ARASampleProjectDocumentController`

This is the central point of communication between our plugin and the ARA host. A single instance of this 
class will be constructed by the host per project, and it will be used by the host whenever ARA objects and 
plugin instances need to be be constructed. Because only one instance exists per project, 
it's also a good place to store resources that don't need to be duplicated for each plugin instance. 

The host creates our `ARASampleProjectDocumentController` instance using our `doCreateDocumentController` 
override:
```
ARA::PlugIn::DocumentController* ARA::PlugIn::DocumentController::doCreateDocumentController() noexcept
{
    return new ARASampleProjectDocumentController();
}
```

In this case, we're using our document controller to do two important things:
- construct an instance of our ARAPlaybackRenderer class by overriding `doCreatePlaybackRenderer`
```
ARA::PlugIn::PlaybackRenderer* ARASampleProjectDocumentController::doCreatePlaybackRenderer() noexcept
{
    return new ARASampleProjectPlaybackRenderer (this);
}
```
- manage a time slice thread that we'll use to read audio samples, initialized in our constructor
```
ARASampleProjectDocumentController::ARASampleProjectDocumentController() noexcept
    : ARADocumentController(),
      audioSourceReadingThread (String (JucePlugin_Name) + " ARA Sample Reading Thread")
{
    audioSourceReadingThread.startThread();
}
```


#### `ARASampleProjectPlaybackRenderer`

Our document controller gets used to construct an instance of our `ARASampleProjectPlaybackRenderer` class. 
This class takes on the `ARAPlaybackRenderer` plugin instance role as defined by the ARA SDK, meaning it will
be used to render audio samples by the host during playback in realtime and non-realtime contexts. 

Because an individual plugin instance may or may not be required to fulfill the `ARAPlaybackRenderer` role, 
our `AudioProcessor` must subclass `AudioProcessorARAExtension` and delegate to any renderers bound to it:
```
// our processor subclasses AudioProcessorARAExtension 
class ARASampleProjectAudioProcessor    : public AudioProcessor,
                                          public AudioProcessorARAExtension
```
```
// which we use to determine if we need to delegate to our renderers
void ARASampleProjectAudioProcessor::prepareToPlay (double newSampleRate, int samplesPerBlock)
{
    if (isARAPlaybackRenderer())
        getARAPlaybackRenderer()->prepareToPlay (newSampleRate, getTotalNumOutputChannels(), samplesPerBlock, true);
    if (isARAEditorRenderer())
        getARAEditorRenderer()->prepareToPlay (newSampleRate, getTotalNumOutputChannels(), samplesPerBlock);
}
```
Functionally our renderer does nothing more than render the original audio data, but it does so by reading
the source audio samples from the host via ARA and playing them back using a `juce::AudioFormatReader` 
instance (making it an ARA enabled pass through renderer.) 

To do this, playback renderer overrides all three virtual renderer functions described in the Plugin Instance Role section:
- `ARASampleProjectPlaybackRenderer::prepareToPlay` \- in addition to allocating some temporary buffers, this function constructs an `AudioFormatReader`
instance used to read audio samples from the ARA host. To do this we'll use an `ARAAudioSourceReader` instance, 
though when rendering in realtime we'll wrap that instance in a `BufferingAudioReader` so that we can
read host samples ahead of time on the document controller's time slice thread. 
- `ARASampleProjectPlaybackRenderer::processBlock` \- because each playback renderer instance is assigned specific playback regions that it's responsible
for rendering, the bulk of this code is concerned with determining when and where playback region samples should go 
within incoming audio buffers. Once this is determined we can use our `ARAAudioSourceReader` to 
read samples and render them into the supplied buffer. 
- `ARASampleProjectPlaybackRenderer::releaseResources` \- this function cleans up all reader instance and temporary buffers used for reading audio source samples


#### `TrackHeaderView`

This view displays the "tracks" (or `ARARegionSequences`) in our ARA host's document. The track color, as
specified by the user inside the host, is represented as a colored rectangle. If a name is provided this is
shown as well. Because this view is closely tied to a particular `ARARegionSequence` instance, it subclasses
`ARARegionSequence::Listener` in order to properly handle region sequence property updates and lifetime. 

```
void TrackHeaderView::didUpdateRegionSequenceProperties (ARARegionSequence* sequence)
{
    repaint();
}

void TrackHeaderView::willDestroyRegionSequence (ARARegionSequence* sequence)
{
    detachFromRegionSequence();
}
```

We also display selection state by subclassing `ARAEditorView::Listener` - if the user selects a particular
track in the host, we visualize this with a yellow border around the track header. 

```
void TrackHeaderView::onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection)
{
    bool isOurRegionSequenceSelected = ARA::contains (currentSelection.getRegionSequences(), regionSequence);
    if (isOurRegionSequenceSelected != isSelected)
    {
        isSelected = isOurRegionSequenceSelected;
        repaint();
    }
}
```


<img src="https://i.imgur.com/mouUUXp.gif"/>


#### `PlaybackRegionView`

The `PlaybackRegionView` class is responsible for visualizing the waveform of a playback region in our
ARA document. Each `PlaybackRegionView` is responsible for drawing a single `ARAPlaybackRegion`, which is done
using a `juce::AudioThumbnail` - we pass the `juce::AudioThumbnail::setReader` function an 
`ARAPlaybackRegionReader` instance tasked with reading the view's playback region and let the thumbnail
do the rest of the work, taking care to draw only the visible portion of the region to avoid reading too
many samples per draw call. 

```
// create a non-realtime playback region reader for our audio thumb
playbackRegionReader = documentView.getARADocumentController()->createPlaybackRegionReader({playbackRegion}, true);
audioThumb.setReader (playbackRegionReader, reinterpret_cast<intptr_t> (playbackRegion));
```
	
Like `ARARegionSequences`, individual `ARAPlaybackRegions` also have a selection state. By subclassing
`ARAEditorView::Listener` we can receive selection notifications and determine whether or not our given
region is selected. The selection state is visualized by a yellow rectangular border around the waveform,
just like the `TrackHeaderView`. 
```
void PlaybackRegionView::onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection)
{
    bool isOurPlaybackRegionSelected = ARA::contains (currentSelection.getPlaybackRegions(), playbackRegion);
    if (isOurPlaybackRegionSelected != isSelected)
    {
        isSelected = isOurPlaybackRegionSelected;
        repaint();
    }
}
```

The regions can also be zoomed horizontally and vertically:

<img src="https://i.imgur.com/G30nSLA.gif"/>


#### `RulersView`

The `RulersView` draws a representation of the musical grid in our ARA document. We use the first 
`ARAMusicalContext` in the document to extract tempo and bar signature content as well as any info on the
changing chord stucture in the song using content reader classes provided by the ARA SDK:
```
const TempoContentReader tempoReader (musicalContext);
const BarSignaturesContentReader barSignaturesReader (musicalContext);
const ChordsContentReader chordsReader (musicalContext);
```
To begin we draw our seconds ruler representing "time on the clock" - we draw a tick mark at each second,
emphasizing every 10th second with a longer tick mark and every minute with a bolder tick mark. 

Next comes our musical timing grid - using convenience classes provided by the ARA C++ Library we can draw
tick marks at each  musical beat along the ARA document's timeline, representing downbeats with a thicker mark. 

<img src="https://i.imgur.com/6uUq5QH.gif"/>

Finally we draw our chord ruler, representing the chord structure of the ARA document. In a host like
Studio One you can define a changing chord progression for your song mapped to musical beats. This ruler
visualizes those chords above the musical timing grid as colored rectangles showing the name of the chord. 

The RulersView also displays the host's current playhead position as a vertical line - by clicking on the grid
of the RulersView you can instruct the host to modify its current play position - double clicking the grid will
instruct the host to start playback. 

<img src="https://i.imgur.com/cVNRNfj.gif"/>

<!-- 
TODO JUCE_ARA
These views are mostly concerned with UI organization and placement, they may not
be worth getting in to in the README. 
#### DocumentView

#### RegionSequenceView
-->