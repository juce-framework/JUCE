## ARAPluginDemo

In addition to the sample ARATestPlugIn provided in the ARA SDK, we've worked with SoundRadix to create 
a sample project showcasing the ARA additions to the JUCE API. The sample project can be found at 
[JUCE_ARA/examples/ARA/ARAPluginDemo](https://github.com/Celemony/JUCE_ARA/tree/develop/examples/ARA/ARAPluginDemo). 

Below is an example of the plugin being hosted by Studio One. 

<img src="https://i.imgur.com/NaMNBol.gif"/>

The plug-in has a few features designed to be educational for both host and plug-in developers that we will outline below. 

### Model Object Property Visualiztion

The [RegionSequenceHeaderView](Source/RegionSequenceHeaderView.h) and [PlaybackRegionView](Source/PlaybackRegionView.h) classes will draw the name and color of their respective model objects by listening to changes in their properties. See their `ARARegionSequence` and `ARAPlaybackRegion` callbacks for more details. 

<img src="https://i.imgur.com/3Cc4uEq.gif"/>

### Selection

By listening to the `ARAEditorView` instances, the [RegionSequenceHeaderView](Source/RegionSequenceHeaderView.h) and [PlaybackRegionView](Source/PlaybackRegionView.h) classes respond to changes in the host selection. Selected objects will be bordered by a yellow rectangle. See their `ARAEditorView` callbacks for more details. 

`ARARegionSequence` Selection:
<br>
<img src="https://i.imgur.com/mouUUXp.gif"/>

`ARAPlaybackRegion` Selection:
<br>
<img src="https://i.imgur.com/YaGCZbT.gif"/>

### Musical Context Content

The [MusicalContextView](Source/MusicalContextView.h) class draws a musical ruler containing chord, bar signature, and tempo data. 

<img src="https://i.imgur.com/6uUq5QH.gif"/>

It uses the pitch interpretation and timeline conversion utilities provided with the ARA SDK Library to properly draw the rulers at the correct bar signature and tempo. 

### Audio Modification State Persistence

Our sample project [ARAPluginDemoAudioModification](Source/ARAPluginDemoAudioModification.h) class stores a 'reverse' state indicating that it's playback regions should render audio in reverse. To toggle the reverse state, double click the playback regions in the plug-in UI. 

<img src="https://i.imgur.com/UIhfW9f.gif"/>

This state gets persisted as a part of the ARA document; see the stream persistence functions of our [ARAPluginDemoDocumentController](Source/ARAPluginDemoDocumentController.h) for more details. 

### Host Playback Control

Our plug-in can use the host supplied ARAPlaybackControllerInterface to control host playback. The host transport position can be set by clicking on the musical timeline ruler, and the host will start playback if the ruler is double-clicked. See the [MusicalContextView mouse handlers](Source/MusicalContextView.cpp) for further details. 

<img src="https://i.imgur.com/cVNRNfj.gif"/>
