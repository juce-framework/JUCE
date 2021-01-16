#pragma once

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

namespace juce
{

//==============================================================================
/** Base class for a renderer fulfulling the ARAPlaybackRenderer role as described in the ARA SDK. 

    Instances of this class are constructed by the DocumentController. 
    If you are subclassing ARAPlaybackRenderer, make sure to call the base class
    implementations of any overridden function, except for processBlock.

    @tags{ARA}
*/
class JUCE_API  ARAPlaybackRenderer   : public ARA::PlugIn::PlaybackRenderer
{
public:
    using ARA::PlugIn::PlaybackRenderer::PlaybackRenderer;

// TODO JUCE_ARA Should we keep this here for subclasses? There is also didBindToARA(), which
//               can provide this if needed. We could get rid of both renderer subclasses otherwise,
//               in fact we could even go further and hide the instance roles entirely behind
//               AudioProcessor(Editor)ARAExtension, which is maybe a good way reduce complexity?
    void setAudioProcessor (AudioProcessor* processor) { audioProcessor = processor; }
    AudioProcessor* getAudioProcessor() const { return audioProcessor; }

// TODO JUCE_ARA see definition of these in .cpp
//#if ARA_VALIDATE_API_CALLS
//    void addPlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept override;
//    void removePlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept override;
//#endif

private:
    AudioProcessor* audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPlaybackRenderer)
};

//==============================================================================
/** Base class for a renderer fulfulling the ARAEditorRenderer role as described in the ARA SDK. 
    
    Instances of this class are constructed by the DocumentController. 
    If you are subclassing ARAEditorRenderer, make sure to call the base class
    implementations of any overridden function, except for processBlock.

    @tags{ARA}
*/
class JUCE_API  ARAEditorRenderer     : public ARA::PlugIn::EditorRenderer
{
public:
    using ARA::PlugIn::EditorRenderer::EditorRenderer;

    void setAudioProcessor (AudioProcessor* processor) { audioProcessor = processor; }
    AudioProcessor* getAudioProcessor() const { return audioProcessor; }

private:
    AudioProcessor* audioProcessor { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAEditorRenderer)
};

//==============================================================================
/** Base class for a renderer fulfulling the ARAEditorView role as described in the ARA SDK. 
    
    Instances of this class are constructed by the DocumentController. 
    If you are subclassing ARAEditorView, make sure to call the base class
    implementations of all overridden functions.
    
    @tags{ARA}
*/
class JUCE_API  ARAEditorView     : public ARA::PlugIn::EditorView
{
public:
    using ARA::PlugIn::EditorView::EditorView;

    // these must be called by subclass implementations to properly to forward listener notifications
    void doNotifySelection (const ARA::PlugIn::ViewSelection* currentSelection) noexcept override;
    void doNotifyHideRegionSequences (std::vector<ARA::PlugIn::RegionSequence*> const& regionSequences) noexcept override;

    class JUCE_API  Listener
    {
    public:
        virtual ~Listener() = default;

       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
        /** Called when the editor view's selection changes.
            @param viewSelection The current selection state.
        */
        virtual void onNewSelection (const ARA::PlugIn::ViewSelection& viewSelection) {}

        /** Called when region sequences are flagged as hidden in the host UI.
            @param regionSequences A vector containing all hidden region sequences. 
        */
        virtual void onHideRegionSequences (std::vector<ARARegionSequence*> const& regionSequences) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    /** \copydoc ARAListenableModelClass::addListener */
    void addListener (Listener* l);

    /** \copydoc ARAListenableModelClass::removeListener */
    void removeListener (Listener* l);

private:
    ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAEditorView)
};

}
