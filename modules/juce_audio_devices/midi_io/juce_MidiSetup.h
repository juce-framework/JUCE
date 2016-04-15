

#ifndef JUCE_MIDISETUP_H_INCLUDED
#define JUCE_MIDISETUP_H_INCLUDED


//==============================================================================
/**
 
 */
class JUCE_API  MidiSetupListener
{
public:
    virtual void midiDevicesChanged() = 0;
};

//==============================================================================
/**

 */
class JUCE_API  MidiSetup
{
public:
    
    #if JUCE_MAC || JUCE_IOS || DOXYGEN
    /** Listen to setup changes (Only available on iOS and OS X).
     
     */
    static void addListener (MidiSetupListener * const listener);
    static void removeListener (MidiSetupListener * const listener);
    
    #endif
};


#endif /* JUCE_MIDISETUP_H_INCLUDED */
