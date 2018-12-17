#pragma once

#include "JuceHeader.h"

class ARASampleProjectAudioProcessorEditor;

//==============================================================================
/**
    RulersView
    JUCE component used to display rulers for song time (in seconds and musical beats) and chords
*/
class RulersView  : public Component,
                    private ARADocument::Listener,
                    private ARAMusicalContext::Listener
{
public:
    RulersView (ARASampleProjectAudioProcessorEditor& owner);
    ~RulersView();

    enum ColourIds
    {
        borderColourId                 = 0x1009110,  /**< The colour to use for the rulers border */
        musicalRulerBackgroundColourId = 0x1009111,  /**< The colour to use for the musical ruler background */
        timeRulerBackgroundColourId    = 0x1009112,  /**< The colour to use for the time    ruler background */
        chordsRulerBackgroundColourId  = 0x1009113,  /**< The colour to use for the chords  ruler background */
        musicalGridColourId            = 0x1009114,  /**< The colour to use for the musical grid */
        timeGridColourId               = 0x1009115,  /**< The colour to use for the time grid    */
        chordsColourId                 = 0x1009116   /**< The colour to use for chords           */

    };

    void paint (Graphics&) override;

    // ARADocument::Listener overrides
    void didEndEditing (ARADocument* document) override;
    void willRemoveMusicalContextFromDocument (ARADocument* document, ARAMusicalContext* musicalContext) override;
    void didReorderMusicalContextsInDocument (ARADocument* document) override;
    void willDestroyDocument (ARADocument* document) override;

    // ARAMusicalContext::Listener overrides
    void doUpdateMusicalContextContent (ARAMusicalContext* context, ARAContentUpdateScopes scopeFlags) override;

private:
    void detachFromDocument();
    void detachFromMusicalContext();
    bool findMusicalContext ();

private:
    ARASampleProjectAudioProcessorEditor& owner;
    ARADocument* document;
    ARAMusicalContext* musicalContext;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RulersView)
};
