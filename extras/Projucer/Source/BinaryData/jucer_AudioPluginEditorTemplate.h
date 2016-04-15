/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#ifndef HEADERGUARD
#define HEADERGUARD

EDITORHEADERS


//==============================================================================
/**
*/
class EDITORCLASSNAME  : public AudioProcessorEditor
{
public:
    EDITORCLASSNAME (FILTERCLASSNAME&);
    ~EDITORCLASSNAME();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FILTERCLASSNAME& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EDITORCLASSNAME)
};


#endif  // HEADERGUARD
