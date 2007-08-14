/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_VSTFORMAT_JUCEHEADER__
#define __JUCE_VSTFORMAT_JUCEHEADER__

#include "juce_VSTPluginFormat.h"

#undef PRAGMA_ALIGN_SUPPORTED
#define VST_FORCE_DEPRECATED 0

#ifdef _MSC_VER
  #pragma warning (push)
  #pragma warning (disable: 4996)
#endif

/* Obviously you're going to need the Steinberg vstsdk2.4 folder in
   your include path.
*/
#include "pluginterfaces/vst2.x/aeffectx.h"

#ifdef _MSC_VER
  #pragma warning (pop)
#endif

class ModuleHandle;
struct fxProgram;


//==============================================================================
/**
    An instance of a plugin, created by a VSTPluginType.

*/
class VSTPluginInstance     : public AudioPluginInstance,
                              private Timer,
                              private AsyncUpdater
{
public:
    //==============================================================================
    ~VSTPluginInstance();

    //==============================================================================
    // AudioPluginInstance methods:

    const String getName() const;
    const String getManufacturer() const;
    const String getVersion() const;
    bool isInstrument() const;
    const String getCategory() const;
    const String getFormatName() const;
    const File getFile() const;
    int getUID() const;
    bool acceptsMidi() const;
    bool producesMidi() const;
    int getSamplesLatency() const;

    //==============================================================================
    // AudioFilterBase methods:

    void JUCE_CALLTYPE prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
    void JUCE_CALLTYPE releaseResources();
    void JUCE_CALLTYPE processBlock (AudioSampleBuffer& buffer,
                                     MidiBuffer& midiMessages);

    AudioFilterEditor* JUCE_CALLTYPE createEditor();

    const String JUCE_CALLTYPE getInputChannelName (const int index) const;
    bool JUCE_CALLTYPE isInputChannelStereoPair (int index) const;

    const String JUCE_CALLTYPE getOutputChannelName (const int index) const;
    bool JUCE_CALLTYPE isOutputChannelStereoPair (int index) const;

    //==============================================================================
    int JUCE_CALLTYPE getNumParameters();
    float JUCE_CALLTYPE getParameter (int index);
    void JUCE_CALLTYPE setParameter (int index, float newValue);
    const String JUCE_CALLTYPE getParameterName (int index);
    const String JUCE_CALLTYPE getParameterText (int index);
    bool isParameterAutomatable (int index) const;

    //==============================================================================
    int JUCE_CALLTYPE getNumPrograms();
    int JUCE_CALLTYPE getCurrentProgram();
    void JUCE_CALLTYPE setCurrentProgram (int index);
    const String JUCE_CALLTYPE getProgramName (int index);
    void JUCE_CALLTYPE changeProgramName (int index, const String& newName);

    //==============================================================================
    void JUCE_CALLTYPE getStateInformation (JUCE_NAMESPACE::MemoryBlock& destData);
    void JUCE_CALLTYPE getCurrentProgramStateInformation (JUCE_NAMESPACE::MemoryBlock& destData);
    void JUCE_CALLTYPE setStateInformation (const void* data, int sizeInBytes);
    void JUCE_CALLTYPE setCurrentProgramStateInformation (const void* data, int sizeInBytes);

    //==============================================================================
    void timerCallback();
    void handleAsyncUpdate();
    VstIntPtr handleCallback (VstInt32 opcode, VstInt32 index, VstInt32 value, void *ptr, float opt);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class VSTPluginType;
    friend class VSTPluginWindow;
    friend class VSTPluginFormat;

    AEffect* effect;
    String name;
    CriticalSection lock;
    bool wantsMidiMessages, initialised, isPowerOn;
    mutable StringArray programNames;
    MidiMessageCollector midiCollector;
    AudioSampleBuffer tempBuffer;
    CriticalSection midiInLock;
    MidiBuffer incomingMidi;
    void* midiEventsToSend;
    int numAllocatedMidiEvents;
    VstTimeInfo vstHostTime;
    float** channels;

    ReferenceCountedObjectPtr <ModuleHandle> module;

    //==============================================================================
    int dispatch (const int opcode, const int index, const int value, void* const ptr, float opt) const;
    bool restoreProgramSettings (const fxProgram* const prog);
    const String getCurrentProgramName();
    void setParamsInProgramBlock (fxProgram* const prog) throw();
    void updateStoredProgramNames();
    void initialise();
    void ensureMidiEventSize (int numEventsNeeded);
    void freeMidiEvents();
    void handleMidiFromPlugin (const VstEvents* const events);
    void createTempParameterStore (juce::MemoryBlock& dest);
    void restoreFromTempParameterStore (const juce::MemoryBlock& mb);
    const String getParameterLabel (int index) const;

    bool usesChunks() const throw();
    void getChunkData (juce::MemoryBlock& mb, bool isPreset, int maxSizeMB) const;
    void setChunkData (const char* data, int size, bool isPreset);
    bool loadFromFXBFile (const void* data, int numBytes);
    bool saveToFXBFile (juce::MemoryBlock& dest, bool isFXB, int maxSizeMB);

    int getVersionNumber() const throw();
    bool hasEditor() const throw();
    bool canMono() const throw();
    bool canReplace() const throw();
    bool isOffline() const throw();
    void setPower (const bool on);

    VSTPluginInstance (const ReferenceCountedObjectPtr <ModuleHandle>& module);
};



#endif   // __JUCE_VSTFORMAT_JUCEHEADER__
