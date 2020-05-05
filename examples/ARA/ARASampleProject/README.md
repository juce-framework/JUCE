## Understanding the ARA Sample Project

In addition to the sample ARATestPlugIn provided in the ARA SDK, we've created a sample project showcasing the 
ARA additions to the JUCE API. The sample project can be found at 
[JUCE_ARA/examples/ARA/ARASampleProject](https://github.com/Celemony/JUCE_ARA/tree/develop/examples/ARA/ARASampleProject). 
Below is an example of the plugin being hosted by Studio One. 

<img src="https://i.imgur.com/gK7GZq8.png"/>

As seen above, the plug-in shows the region sequences and playback regions within their musical context in a zoomable/scrollable arrangement view. 
Time in seconds and in musical beats and chords are drawn in rulers above the arrangement. 
Name and color of the arrangement objects are used if provided. Selected objects and time range are highlighted, and hidden region sequences are not displayed.

The current playback position reported by the ARA companion API host is shown.
By clicking or double-clicking on the rulers, the plug-in requests the host to reposition or start playback.

During playback, the plug-in passes through the underlying audio source samples without further processing. 
Playback can be reversed by double clicking a playback region - this reverse state is stored in a custom
subclass of `ARAAudioModification`, and is persisted as a part of the ARA document archive. 

In addition to demonstrating ARA integration with JUCE, the sample is also a valuable tool when implementing ARA hosts - it provides visual feedback for the model graph published by the host, 
and supports many view integration features as described above.
Furthermore, playing with the project settings with the Projucer can be a good way to verify a hosts ability
to detect ARA plugin properties.

Below we'll go over several important classes and how they interact with the ARA API. 

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

In this case, we're using our document controller to create our custom `ARAAudioModification` subclass
and manage the persistence of our ARA document / model graph state. This is done in the following overrides:

```
// our doCreateAudioModification() override, used to return an ARASampleProjectAudioModification instance
ARA::PlugIn::AudioModification* doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef, const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept override;

// our persistence overrides, used to store and restore data saved by the host (in this case, our ARASampleProjectAudioModification reverse state)
bool doRestoreObjectsFromStream (ARAInputStream& input, const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept override;
bool doStoreObjectsToStream (ARAOutputStream& output, const ARA::PlugIn::StoreObjectsFilter* filter) noexcept override;
```

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
// Create a playback region reader using this processor for our audio thumb
playbackRegionReader = new ARAPlaybackRegionReader (std::make_unique<ARASampleProjectAudioProcessor>(), { playbackRegion });
audioThumb.setReader (playbackRegionReader, reinterpret_cast<intptr_t> (playbackRegionReader));
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

<img src="https://i.imgur.com/YaGCZbT.gif"/>

When a playback region is double clicked, we toggle a flag in its parent `ARASampleProjectAudioModification`
indicating that playback should be reversed. 

```
void PlaybackRegionView::mouseDoubleClick (const MouseEvent& /*event*/)
{
    // set the reverse flag on our region's audio modification when double clicked
    auto audioModification = playbackRegion->getAudioModification<ARASampleProjectAudioModification>();
    audioModification->setReversePlayback (! audioModification->getReversePlayback());

    // send a content change notification for the modification and all associated playback regions
    audioModification->notifyContentChanged (ARAContentUpdateScopes::samplesAreAffected());
    for (auto araPlaybackRegion : audioModification->getPlaybackRegions<ARAPlaybackRegion>())
        araPlaybackRegion->notifyContentChanged (ARAContentUpdateScopes::samplesAreAffected());
}
```

<img src="https://i.imgur.com/UIhfW9f.gif"/>

The regions can also be zoomed horizontally:

<img src="https://i.imgur.com/SGwHhBe.gif"/>


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