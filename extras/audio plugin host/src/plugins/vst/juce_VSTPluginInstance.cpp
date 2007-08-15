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

#ifdef _WIN32
 #define _WIN32_WINNT 0x500
 #define STRICT
 #include <windows.h>
 #include <float.h>
 #pragma warning (disable : 4312)
#else
 #include <Carbon/Carbon.h>
#endif

#include "../../../../../juce.h"
#include "juce_VSTPluginInstance.h"
#include "../juce_PluginDescription.h"

#if ! JUCE_WIN32
 #define _fpreset()
 #define _clearfp()
#endif

BEGIN_JUCE_NAMESPACE
 extern void juce_callAnyTimersSynchronously();
END_JUCE_NAMESPACE

//==============================================================================
const int fxbVersionNum = 1;

//==============================================================================
struct fxProgram
{
    long chunkMagic;        // 'CcnK'
    long byteSize;          // of this chunk, excl. magic + byteSize
    long fxMagic;           // 'FxCk'
    long version;
    long fxID;              // fx unique id
    long fxVersion;
    long numParams;
    char prgName[28];
    float params[1];        // variable no. of parameters
};

struct fxSet
{
    long chunkMagic;        // 'CcnK'
    long byteSize;          // of this chunk, excl. magic + byteSize
    long fxMagic;           // 'FxBk'
    long version;
    long fxID;              // fx unique id
    long fxVersion;
    long numPrograms;
    char future[128];
    fxProgram programs[1];  // variable no. of programs
};

struct fxChunkSet
{
    long chunkMagic;        // 'CcnK'
    long byteSize;          // of this chunk, excl. magic + byteSize
    long fxMagic;           // 'FxCh', 'FPCh', or 'FBCh'
    long version;
    long fxID;              // fx unique id
    long fxVersion;
    long numPrograms;
    char future[128];
    long chunkSize;
    char chunk[8];          // variable
};

struct fxProgramSet
{
    long chunkMagic;        // 'CcnK'
    long byteSize;          // of this chunk, excl. magic + byteSize
    long fxMagic;           // 'FxCh', 'FPCh', or 'FBCh'
    long version;
    long fxID;              // fx unique id
    long fxVersion;
    long numPrograms;
    char name[28];
    long chunkSize;
    char chunk[8];          // variable
};


#ifdef JUCE_LITTLE_ENDIAN
 static long swap (const long x) throw()    { return (long) swapByteOrder ((uint32) x); }

 static float swapFloat (const float x) throw()
 {
     union { uint32 asInt; float asFloat; } n;
     n.asFloat = x;
     n.asInt = swapByteOrder (n.asInt);
     return n.asFloat;
 }
#else
 #define swap(x) (x)
 #define swapFloat(x) (x)
#endif

//==============================================================================
typedef AEffect* (*MainCall) (audioMasterCallback);

static VstIntPtr VSTCALLBACK audioMaster (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);

#if JUCE_PPC
 static void* audioMasterCoerced = 0;
#endif

static int shellUIDToCreate = 0;
static int insideVSTCallback = 0;

static Array <VSTPluginWindow*> activeWindows;

//==============================================================================
// Change this to disable logging of various VST activities
#ifndef VST_LOGGING
  #define VST_LOGGING 1
#endif

#if VST_LOGGING
 #define log(a) Logger::writeToLog(a);
#else
 #define log(a)
#endif

//==============================================================================
#if JUCE_MAC
namespace JUCE_NAMESPACE
{
    extern bool juce_isHIViewCreatedByJuce (HIViewRef view);
    extern bool juce_isWindowCreatedByJuce (WindowRef window);
}

#if JUCE_PPC
static void* NewCFMFromMachO (void* const machofp) throw()
{
    void* result = juce_malloc (8);

    ((void**) result)[0] = machofp;
    ((void**) result)[1] = result;

    return result;
}
#endif
#endif

//==============================================================================
static VoidArray activeModules;

//==============================================================================
class ModuleHandle    : public ReferenceCountedObject
{
public:
    //==============================================================================
    File file;
    MainCall moduleMain;
    String pluginName;

    //==============================================================================
    static ModuleHandle* findOrCreateModule (const File& file)
    {
        for (int i = activeModules.size(); --i >= 0;)
        {
            ModuleHandle* const module = (ModuleHandle*) activeModules.getUnchecked(i);

            if (module->file == file)
                return module;
        }

        _fpreset(); // (doesn't do any harm)
        ++insideVSTCallback;
        shellUIDToCreate = 0;

        log ("Attempting to load VST: " + file.getFullPathName());

        ModuleHandle* m = new ModuleHandle (file);

        if (! m->open())
            deleteAndZero (m);

        --insideVSTCallback;
        _fpreset(); // (doesn't do any harm)

        return m;
    }

    //==============================================================================
    ModuleHandle (const File& file_)
        : file (file_),
          moduleMain (0),
#if JUCE_WIN32
          hModule (0)
#else
          fragId (0),
          resHandle (0),
          bundleRef (0),
          resFileId (0)
#endif
    {
        activeModules.add (this);

#if JUCE_WIN32
        fullParentDirectoryPathName = file_.getParentDirectory().getFullPathName();
#else
        PlatformUtilities::makeFSSpecFromPath (&parentDirFSSpec, file_.getParentDirectory().getFullPathName());
#endif
    }

    ~ModuleHandle()
    {
        activeModules.removeValue (this);

        close();
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

    //==============================================================================
#if JUCE_WIN32
    HMODULE hModule;
    String fullParentDirectoryPathName;

    static HMODULE loadDLL (const TCHAR* filename) throw()
    {
        HMODULE h = 0;

        __try
        {
            h = LoadLibrary (filename);
        }
        __finally
        {
        }

        return h;
    }

    bool open()
    {
        static bool timePeriodSet = false;

        if (! timePeriodSet)
        {
            timePeriodSet = true;
            timeBeginPeriod (2);
        }

        pluginName = file.getFileNameWithoutExtension();

        hModule = loadDLL (file.getFullPathName());

        if (hModule == 0)
            return false;

        moduleMain = (MainCall) GetProcAddress (hModule, "VSTPluginMain");

        if (moduleMain == 0)
            moduleMain = (MainCall) GetProcAddress (hModule, "main");

        return moduleMain != 0;
    }

    void close()
    {
        _fpreset(); // (doesn't do any harm)

        if (hModule != 0)
        {
            __try
            {
                FreeLibrary (hModule);
            }
            __finally
            {
            }
        }
    }

    void closeEffect (AEffect* eff)
    {
        eff->dispatcher (eff, effClose, 0, 0, 0, 0);
    }

#else
    CFragConnectionID fragId;
    Handle resHandle;
    CFBundleRef bundleRef;
    FSSpec parentDirFSSpec;
    short resFileId;

    bool open()
    {
        bool ok = false;
        const String filename (file.getFullPathName());

        if (file.hasFileExtension (T(".vst")))
        {
            CFURLRef url = CFURLCreateFromFileSystemRepresentation (0, (const UInt8*) (const char*) filename,
                                                                    filename.length(), file.isDirectory());

            if (url != 0)
            {
                bundleRef = CFBundleCreate (kCFAllocatorDefault, url);
                CFRelease (url);

                if (bundleRef != 0)
                {
                    if (CFBundleLoadExecutable (bundleRef))
                    {
                        moduleMain = (MainCall) CFBundleGetFunctionPointerForName (bundleRef, CFSTR("main_macho"));

                        if (moduleMain == 0)
                            moduleMain = (MainCall) CFBundleGetFunctionPointerForName (bundleRef, CFSTR("VSTPluginMain"));

                        if (moduleMain != 0)
                        {
                            CFTypeRef name = CFBundleGetValueForInfoDictionaryKey (bundleRef, CFSTR("CFBundleName"));

                            if (name != 0)
                            {
                                if (CFGetTypeID (name) == CFStringGetTypeID())
                                {
                                    char buffer[1024];

                                    if (CFStringGetCString ((CFStringRef) name, buffer, sizeof (buffer), CFStringGetSystemEncoding()))
                                        pluginName = buffer;
                                }
                            }

                            if (pluginName.isEmpty())
                                pluginName = file.getFileNameWithoutExtension();

                            resFileId = CFBundleOpenBundleResourceMap (bundleRef);

                            ok = true;
                        }
                    }

                    if (! ok)
                    {
                        CFBundleUnloadExecutable (bundleRef);
                        CFRelease (bundleRef);
                        bundleRef = 0;
                    }
                }
            }
        }
#if JUCE_PPC
        else
        {
            FSRef fn;

            if (FSPathMakeRef ((UInt8*) (const char*) filename, &fn, 0) == noErr)
            {
                resFileId = FSOpenResFile (&fn, fsRdPerm);

                if (resFileId != -1)
                {
                    const int numEffs = Count1Resources ('aEff');

                    for (int i = 0; i < numEffs; ++i)
                    {
                        resHandle = Get1IndResource ('aEff', i + 1);

                        if (resHandle != 0)
                        {
                            OSType type;
                            Str255 name;
                            SInt16 id;
                            GetResInfo (resHandle, &id, &type, name);
                            pluginName = String ((const char*) name + 1, name[0]);
                            DetachResource (resHandle);
                            HLock (resHandle);

                            Ptr ptr;
                            Str255 errorText;

                            OSErr err = GetMemFragment (*resHandle, GetHandleSize (resHandle),
                                                        name, kPrivateCFragCopy,
                                                        &fragId, &ptr, errorText);

                            if (err == noErr)
                            {
                                moduleMain = (MainCall) newMachOFromCFM (ptr);
                                ok = true;
                            }
                            else
                            {
                                HUnlock (resHandle);
                            }

                            break;
                        }
                    }

                    if (! ok)
                        CloseResFile (resFileId);
                }
            }
        }
#endif

        return ok;
    }

    void close()
    {
#if JUCE_PPC
        if (fragId != 0)
        {
            if (moduleMain != 0)
                disposeMachOFromCFM ((void*) moduleMain);

            CloseConnection (&fragId);
            HUnlock (resHandle);

            if (resFileId != 0)
                CloseResFile (resFileId);
        }
        else 
#endif
        if (bundleRef != 0)
        {
            CFBundleCloseBundleResourceMap (bundleRef, resFileId);

            if (CFGetRetainCount (bundleRef) == 1)
                CFBundleUnloadExecutable (bundleRef);

            if (CFGetRetainCount (bundleRef) > 0)
                CFRelease (bundleRef);
        }
    }

    void closeEffect (AEffect* eff)
    {
#if JUCE_PPC
        if (fragId != 0)
        {
            VoidArray thingsToDelete;
            thingsToDelete.add ((void*) eff->dispatcher);
            thingsToDelete.add ((void*) eff->process);
            thingsToDelete.add ((void*) eff->setParameter);
            thingsToDelete.add ((void*) eff->getParameter);
            thingsToDelete.add ((void*) eff->processReplacing);

            eff->dispatcher (eff, effClose, 0, 0, 0, 0);

            for (int i = thingsToDelete.size(); --i >= 0;)
                disposeMachOFromCFM (thingsToDelete[i]);
        }
        else
#endif
        {
            eff->dispatcher (eff, effClose, 0, 0, 0, 0);
        }
    }

#if JUCE_PPC
    static void* newMachOFromCFM (void* cfmfp)
    {
        if (cfmfp == 0)
            return 0;

        UInt32* const mfp = (UInt32*) juce_malloc (sizeof (UInt32) * 6);

        mfp[0] = 0x3d800000 | ((UInt32) cfmfp >> 16);
        mfp[1] = 0x618c0000 | ((UInt32) cfmfp & 0xffff);
        mfp[2] = 0x800c0000;
        mfp[3] = 0x804c0004;
        mfp[4] = 0x7c0903a6;
        mfp[5] = 0x4e800420;

        MakeDataExecutable (mfp, sizeof (UInt32) * 6);
        return mfp;
    }

    static void disposeMachOFromCFM (void* ptr)
    {
        juce_free (ptr);
    }

    void coerceAEffectFunctionCalls (AEffect* eff)
    {
        if (fragId != 0)
        {
            eff->dispatcher = (AEffectDispatcherProc) newMachOFromCFM ((void*) eff->dispatcher);
            eff->process = (AEffectProcessProc) newMachOFromCFM ((void*) eff->process);
            eff->setParameter = (AEffectSetParameterProc) newMachOFromCFM ((void*) eff->setParameter);
            eff->getParameter = (AEffectGetParameterProc) newMachOFromCFM ((void*) eff->getParameter);
            eff->processReplacing = (AEffectProcessProc) newMachOFromCFM ((void*) eff->processReplacing);
        }
    }
#endif

#endif
};


//==============================================================================
VSTPluginInstance::VSTPluginInstance (const ReferenceCountedObjectPtr <ModuleHandle>& module_)
    : effect (0),
      wantsMidiMessages (false),
      initialised (false),
      isPowerOn (false),
      numAllocatedMidiEvents (0),
      midiEventsToSend (0),
      tempBuffer (1, 1),
      channels (0),
      module (module_)
{
    try
    {
        _fpreset();

        ++insideVSTCallback;

        name = module->pluginName;
        log (T("Creating VST instance: ") + name);

#if JUCE_MAC
        if (module->resFileId != 0)
            UseResFile (module->resFileId);

#if JUCE_PPC
        if (module->fragId != 0)
        {
            static void* audioMasterCoerced = 0;
            if (audioMasterCoerced == 0)
                audioMasterCoerced = NewCFMFromMachO ((void*) &audioMaster);

            effect = module->moduleMain ((audioMasterCallback) audioMasterCoerced);
        }
        else
#endif
#endif
        {
            effect = module->moduleMain (&audioMaster);
        }

        --insideVSTCallback;

        if (effect != 0 && effect->magic == kEffectMagic)
        {
#if JUCE_PPC
            module->coerceAEffectFunctionCalls (effect);
#endif

            jassert (effect->resvd2 == 0);
            jassert (effect->object != 0);

            _fpreset(); // some dodgy plugs fuck around with this
        }
        else
        {
            effect = 0;
        }
    }
    catch (...)
    {
        --insideVSTCallback;
    }
}

VSTPluginInstance::~VSTPluginInstance()
{
    {
        const ScopedLock sl (lock);

        jassert (insideVSTCallback == 0);

        if (effect != 0 && effect->magic == kEffectMagic)
        {
            try
            {
#if JUCE_MAC
                if (module->resFileId != 0)
                    UseResFile (module->resFileId);
#endif

                // Must delete any editors before deleting the plugin instance!
                jassert (getActiveEditor() == 0);

                _fpreset(); // some dodgy plugs fuck around with this

                module->closeEffect (effect);
            }
            catch (...)
            {}
        }

        module = 0;
        effect = 0;
    }

    freeMidiEvents();

    juce_free (channels);
    channels = 0;
}

//==============================================================================
void VSTPluginInstance::initialise()
{
    if (initialised || effect == 0)
        return;

    log (T("Initialising VST: ") + module->pluginName);
    initialised = true;

    dispatch (effIdentify, 0, 0, 0, 0);

    {
        char buffer [kVstMaxEffectNameLen + 8];
        zerostruct (buffer);
        dispatch (effGetEffectName, 0, 0, buffer, 0);

        name = String (buffer);
        if (name.trim().isEmpty())
            name = module->pluginName;
    }

    dispatch (effSetSampleRate, 0, 0, 0, (float) sampleRate);
    dispatch (effSetBlockSize, 0, jmax (16, blockSize), 0, 0);

    dispatch (effOpen, 0, 0, 0, 0);

    numOutputChannels = effect->numOutputs;
    numInputChannels = effect->numInputs;

    if (getNumPrograms() > 1)
        setCurrentProgram (0);
    else
        dispatch (effSetProgram, 0, 0, 0, 0);

    int i;
    for (i = effect->numInputs; --i >= 0;)
        dispatch (effConnectInput, i, 1, 0, 0);

    for (i = effect->numOutputs; --i >= 0;)
        dispatch (effConnectOutput, i, 1, 0, 0);

    updateStoredProgramNames();

    wantsMidiMessages = dispatch (effCanDo, 0, 0, (void*) "receiveVstMidiEvent", 0) > 0;
}


//==============================================================================
void JUCE_CALLTYPE VSTPluginInstance::prepareToPlay (double sampleRate_, int samplesPerBlockExpected)
{
    sampleRate = sampleRate_;
    blockSize = samplesPerBlockExpected;
    midiCollector.reset (sampleRate);

    juce_free (channels);
    channels = (float**) juce_calloc (sizeof (float*) * jmax (16, getNumOutputChannels() + 2, getNumInputChannels() + 2));

    vstHostTime.tempo = 120.0;
    vstHostTime.timeSigNumerator = 4;
    vstHostTime.timeSigDenominator = 4;
    vstHostTime.sampleRate = sampleRate;
    vstHostTime.samplePos = 0;
    vstHostTime.flags = kVstNanosValid;  /*| kVstTransportPlaying | kVstTempoValid | kVstTimeSigValid*/;

    initialise();

    if (initialised)
    {
        wantsMidiMessages = wantsMidiMessages
                                || (dispatch (effCanDo, 0, 0, (void*) "receiveVstMidiEvent", 0) > 0);

        if (wantsMidiMessages)
            ensureMidiEventSize (256);
        else
            freeMidiEvents();

        incomingMidi.clear();

        dispatch (effSetSampleRate, 0, 0, 0, (float) sampleRate);
        dispatch (effSetBlockSize, 0, jmax (16, blockSize), 0, 0);

        tempBuffer.setSize (effect->numOutputs, blockSize);

        if (! isPowerOn)
            setPower (true);

        // dodgy hack to force some plugins to initialise the sample rate..
        if ((! hasEditor()) && getNumParameters() > 0)
        {
            const float old = getParameter (0);
            setParameter (0, (old < 0.5f) ? 1.0f : 0.0f);
            setParameter (0, old);
        }

        dispatch (effStartProcess, 0, 0, 0, 0);
    }
}

void JUCE_CALLTYPE VSTPluginInstance::releaseResources()
{
    if (initialised)
    {
        dispatch (effStopProcess, 0, 0, 0, 0);
        setPower (false);
    }

    midiCollector.reset (sampleRate);
    tempBuffer.setSize (1, 1);
    incomingMidi.clear();

    freeMidiEvents();
    juce_free (channels);
    channels = 0;
}

void JUCE_CALLTYPE VSTPluginInstance::processBlock (AudioSampleBuffer& buffer,
                                                    MidiBuffer& midiMessages)
{
    const int numSamples = buffer.getNumSamples();

    if (initialised)
    {
#if JUCE_WIN32
        vstHostTime.nanoSeconds = timeGetTime() * 1000000.0;
#else
        UnsignedWide micro;
        Microseconds (&micro);
        vstHostTime.nanoSeconds = micro.lo * 1000.0;
#endif

        if (wantsMidiMessages)
        {
            MidiBuffer::Iterator iter (midiMessages);

            int eventIndex = 0;
            const uint8* midiData;
            int numBytesOfMidiData, samplePosition;

            while (iter.getNextEvent (midiData, numBytesOfMidiData, samplePosition))
            {
                if (numBytesOfMidiData < 4)
                {
                    ensureMidiEventSize (eventIndex);
                    VstMidiEvent* const e
                        = (VstMidiEvent*) ((VstEvents*) midiEventsToSend)->events [eventIndex++];

                    // check that some plugin hasn't messed up our objects
                    jassert (e->type == kVstMidiType);
                    jassert (e->byteSize == 24);

                    e->deltaFrames = jlimit (0, numSamples - 1, samplePosition);
                    e->noteLength = 0;
                    e->noteOffset = 0;
                    e->midiData[0] = midiData[0];
                    e->midiData[1] = midiData[1];
                    e->midiData[2] = midiData[2];
                    e->detune = 0;
                    e->noteOffVelocity = 0;
                }
            }

            if (midiEventsToSend == 0)
                ensureMidiEventSize (1);

            ((VstEvents*) midiEventsToSend)->numEvents = eventIndex;

            try
            {
                effect->dispatcher (effect, effProcessEvents, 0, 0, midiEventsToSend, 0);
            }
            catch (...)
            {}
        }

        int i;
        const int maxChans = jmax (effect->numInputs, effect->numOutputs);

        for (i = 0; i < maxChans; ++i)
            channels[i] = buffer.getSampleData (i);

        channels [maxChans] = 0;

        _clearfp();

        if ((effect->flags & effFlagsCanReplacing) != 0)
        {
            try
            {
                effect->processReplacing (effect, channels, channels, numSamples);
            }
            catch (...)
            {}
        }
        else
        {
            tempBuffer.setSize (effect->numOutputs, numSamples);
            tempBuffer.clear();

            float* outs [64];

            for (i = effect->numOutputs; --i >= 0;)
                outs[i] = tempBuffer.getSampleData (i);

            outs [effect->numOutputs] = 0;

            try
            {
                effect->process (effect, channels, outs, numSamples);
            }
            catch (...)
            {}

            for (i = effect->numOutputs; --i >= 0;)
                buffer.copyFrom (i, 0, outs[i], numSamples);
        }
    }
    else
    {
        // Not initialised, so just bypass..
        for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
            buffer.clear (i, 0, buffer.getNumSamples());
    }

    {
        // copy any incoming midi..
        const ScopedLock sl (midiInLock);

        midiMessages = incomingMidi;
        incomingMidi.clear();
    }
}

//==============================================================================
void VSTPluginInstance::ensureMidiEventSize (int numEventsNeeded)
{
    if (numEventsNeeded > numAllocatedMidiEvents)
    {
        numEventsNeeded = (numEventsNeeded + 32) & ~31;

        const int size = 20 + sizeof (VstEvent*) * numEventsNeeded;

        if (midiEventsToSend == 0)
            midiEventsToSend = juce_calloc (size);
        else
            midiEventsToSend = juce_realloc (midiEventsToSend, size);

        for (int i = numAllocatedMidiEvents; i < numEventsNeeded; ++i)
        {
            VstMidiEvent* const e = (VstMidiEvent*) juce_calloc (sizeof (VstMidiEvent));
            e->type = kVstMidiType;
            e->byteSize = 24;

            ((VstEvents*) midiEventsToSend)->events[i] = (VstEvent*) e;
        }

        numAllocatedMidiEvents = numEventsNeeded;
    }
}

void VSTPluginInstance::freeMidiEvents()
{
    if (midiEventsToSend != 0)
    {
        for (int i = numAllocatedMidiEvents; --i >= 0;)
            juce_free (((VstEvents*) midiEventsToSend)->events[i]);

        juce_free (midiEventsToSend);
        midiEventsToSend = 0;
        numAllocatedMidiEvents = 0;
    }
}

void VSTPluginInstance::handleMidiFromPlugin (const VstEvents* const events)
{
    if (events != 0)
    {
        const ScopedLock sl (midiInLock);

        for (int i = 0; i < events->numEvents; ++i)
        {
            const VstEvent* const e = events->events[i];

            if (e->type == kVstMidiType)
            {
                incomingMidi.addEvent ((const uint8*) ((const VstMidiEvent*) e)->midiData,
                                       3, e->deltaFrames);
            }
        }
    }
}

//==============================================================================
class VSTPluginWindow   : public AudioFilterEditor,
                          public Timer
{
public:
    //==============================================================================
    VSTPluginWindow (VSTPluginInstance& plugin_)
        : AudioFilterEditor (&plugin_),
          plugin (plugin_),
          isOpen (false),
          wasShowing (false),
          pluginRefusesToResize (false),
          pluginWantsKeys (false),
          alreadyInside (false),
          recursiveResize (false)
    {
#if JUCE_WIN32
        sizeCheckCount = 0;
        pluginHWND = 0;
#else
        pluginViewRef = 0;
#endif

        movementWatcher = new CompMovementWatcher (this);

        activeWindows.add (this);

        setOpaque (true);
        setVisible (true);
    }

    ~VSTPluginWindow()
    {
        deleteAndZero (movementWatcher);

        closePluginWindow();

        activeWindows.removeValue (this);
        plugin.editorBeingDeleted  (this);
    }

    //==============================================================================
    void componentMovedOrResized()
    {
        if (recursiveResize)
            return;

        Component* const topComp = getTopLevelComponent();

        if (topComp->getPeer() != 0)
        {
            int x = 0, y = 0;
            relativePositionToOtherComponent (topComp, x, y);

            recursiveResize = true;

#if JUCE_MAC
            if (pluginViewRef != 0)
            {
                HIRect r;
                r.origin.x = (float) x;
                r.origin.y = (float) y;
                r.size.width = (float) getWidth();
                r.size.height = (float) getHeight();
                HIViewSetFrame (pluginViewRef, &r);
            }
            else if (pluginWindowRef != 0)
            {
                Rect r;
                r.left = getScreenX();
                r.top = getScreenY();
                r.right = r.left + getWidth();
                r.bottom = r.top + getHeight();

                WindowGroupRef group = GetWindowGroup (pluginWindowRef);
                WindowGroupAttributes atts;
                GetWindowGroupAttributes (group, &atts);
                ChangeWindowGroupAttributes (group, 0, kWindowGroupAttrMoveTogether);

                SetWindowBounds (pluginWindowRef, kWindowContentRgn, &r);

                if ((atts & kWindowGroupAttrMoveTogether) != 0)
                    ChangeWindowGroupAttributes (group, kWindowGroupAttrMoveTogether, 0);
            }
            else
            {
                repaint();
            }
#else
            if (pluginHWND != 0)
                MoveWindow (pluginHWND, x, y, getWidth(), getHeight(), TRUE);
#endif

            recursiveResize = false;
        }
    }

    void componentVisibilityChanged()
    {
        const bool isShowingNow = isShowing();

        if (wasShowing != isShowingNow)
        {
            wasShowing = isShowingNow;

            if (isShowingNow)
                openPluginWindow();
            else
                closePluginWindow();
        }

        componentMovedOrResized();
    }

    void componentPeerChanged()
    {
        closePluginWindow();
        openPluginWindow();
    }

    //==============================================================================
    bool keyStateChanged()
    {
        return pluginWantsKeys;
    }

    bool keyPressed (const KeyPress&)
    {
        return pluginWantsKeys;
    }

    //==============================================================================
    void paint (Graphics& g)
    {
        if (isOpen)
        {
            ComponentPeer* const peer = getPeer();

            if (peer != 0)
            {
                peer->addMaskedRegion (getScreenX() - peer->getScreenX(),
                                       getScreenY() - peer->getScreenY(),
                                       getWidth(), getHeight());

#if JUCE_MAC
                dispatch (effEditDraw, 0, 0, 0, 0);
#endif
            }
        }
        else
        {
            g.fillAll (Colours::black);
        }
    }

    //==============================================================================
    void timerCallback()
    {
#if JUCE_WIN32
        if (--sizeCheckCount <= 0)
        {
            sizeCheckCount = 10;

            checkPluginWindowSize();
        }
#endif

        try
        {
            static bool reentrant = false;

            if (! reentrant)
            {
                reentrant = true;
                plugin.dispatch (effEditIdle, 0, 0, 0, 0);
                reentrant = false;
            }
        }
        catch (...)
        {}
    }

    //==============================================================================
    void mouseDown (const MouseEvent& e)
    {
#if JUCE_MAC
        if (! alreadyInside)
        {
            alreadyInside = true;
            toFront (true);
            dispatch (effEditMouse, e.x, e.y, 0, 0);
            alreadyInside = false;
        }
        else
        {
            PostEvent (::mouseDown, 0);
        }
#else
        (void) e;

        toFront (true);
#endif
    }

    void broughtToFront()
    {
        activeWindows.removeValue (this);
        activeWindows.add (this);

#if JUCE_MAC
        dispatch (effEditTop, 0, 0, 0, 0);
#endif
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    VSTPluginInstance& plugin;
    bool isOpen, wasShowing, recursiveResize;
    bool pluginWantsKeys, pluginRefusesToResize, alreadyInside;

#if JUCE_WIN32
    HWND pluginHWND;
    void* originalWndProc;
    int sizeCheckCount;
#else
    HIViewRef pluginViewRef;
    WindowRef pluginWindowRef;
#endif

    //==============================================================================
    void openPluginWindow()
    {
        if (isOpen || getWindowHandle() == 0)
            return;

        log (T("Opening VST UI: ") + plugin.getName());
        isOpen = true;

        ERect* rect = 0;
        dispatch (effEditGetRect, 0, 0, &rect, 0);
        dispatch (effEditOpen, 0, 0, getWindowHandle(), 0);

        // do this before and after like in the steinberg example
        dispatch (effEditGetRect, 0, 0, &rect, 0);
        dispatch (effGetProgram, 0, 0, 0, 0); // also in steinberg code

        // Install keyboard hooks
        pluginWantsKeys = (dispatch (effKeysRequired, 0, 0, 0, 0) == 0);

#if JUCE_WIN32
        originalWndProc = 0;
        pluginHWND = GetWindow ((HWND) getWindowHandle(), GW_CHILD);

        if (pluginHWND == 0)
        {
            isOpen = false;
            setSize (300, 150);
            return;
        }

        #pragma warning (push)
        #pragma warning (disable: 4244)

        originalWndProc = (void*) GetWindowLongPtr (pluginHWND, GWL_WNDPROC);

        if (! pluginWantsKeys)
            SetWindowLongPtr (pluginHWND, GWL_WNDPROC, (LONG_PTR) vstHookWndProc);

        #pragma warning (pop)

        int w, h;
        RECT r;
        GetWindowRect (pluginHWND, &r);
        w = r.right - r.left;
        h = r.bottom - r.top;

        if (rect != 0)
        {
            const int rw = rect->right - rect->left;
            const int rh = rect->bottom - rect->top;

            if ((rw > 50 && rh > 50 && rw < 2000 && rh < 2000 && rw != w && rh != h)
                || ((w == 0 && rw > 0) || (h == 0 && rh > 0)))
            {
                // very dodgy logic to decide which size is right.
                if (abs (rw - w) > 350 || abs (rh - h) > 350)
                {
                    SetWindowPos (pluginHWND, 0,
                                  0, 0, rw, rh,
                                  SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);

                    GetWindowRect (pluginHWND, &r);

                    w = r.right - r.left;
                    h = r.bottom - r.top;

                    pluginRefusesToResize = (w != rw) || (h != rh);

                    w = rw;
                    h = rh;
                }
            }
        }
#else
        HIViewRef root = HIViewGetRoot ((WindowRef) getWindowHandle());
        HIViewFindByID (root, kHIViewWindowContentID, &root);
        pluginViewRef = HIViewGetFirstSubview (root);

        while (pluginViewRef != 0 && juce_isHIViewCreatedByJuce (pluginViewRef))
            pluginViewRef = HIViewGetNextView (pluginViewRef);

        pluginWindowRef = 0;

        if (pluginViewRef == 0)
        {
            WindowGroupRef ourGroup = GetWindowGroup ((WindowRef) getWindowHandle());
            //DebugPrintWindowGroup (ourGroup);
            //DebugPrintAllWindowGroups();

            GetIndexedWindow (ourGroup, 1,
                              kWindowGroupContentsVisible,
                              &pluginWindowRef);

            if (pluginWindowRef == (WindowRef) getWindowHandle()
                 || juce_isWindowCreatedByJuce (pluginWindowRef))
                pluginWindowRef = 0;
        }

        int w = 250, h = 150;

        if (rect != 0)
        {
            w = rect->right - rect->left;
            h = rect->bottom - rect->top;

            if (w == 0 || h == 0)
            {
                w = 250;
                h = 150;
            }
        }
#endif

        // double-check it's not too tiny
        w = jmax (w, 32);
        h = jmax (h, 32);

        setSize (w, h);

#if JUCE_WIN32
        checkPluginWindowSize();
#endif

        startTimer (18 + juce::Random::getSystemRandom().nextInt (5));
        repaint();
    }

    //==============================================================================
    void closePluginWindow()
    {
        if (isOpen)
        {
            log (T("Closing VST UI: ") + plugin.getName());
            isOpen = false;

            dispatch (effEditClose, 0, 0, 0, 0);

#if JUCE_WIN32
            #pragma warning (push)
            #pragma warning (disable: 4244)

            if (pluginHWND != 0 && IsWindow (pluginHWND))
                SetWindowLongPtr (pluginHWND, GWL_WNDPROC, (LONG_PTR) originalWndProc);

            #pragma warning (pop)

            stopTimer();

            if (pluginHWND != 0 && IsWindow (pluginHWND))
                DestroyWindow (pluginHWND);

            pluginHWND = 0;
#else
            dispatch (effEditSleep, 0, 0, 0, 0);
            pluginViewRef = 0;
            stopTimer();
#endif
        }
    }

    //==============================================================================
#if JUCE_WIN32
    void checkPluginWindowSize() throw()
    {
        RECT r;
        GetWindowRect (pluginHWND, &r);
        const int w = r.right - r.left;
        const int h = r.bottom - r.top;

        if (isShowing() && w > 0 && h > 0
             && (w != getWidth() || h != getHeight())
             && ! pluginRefusesToResize)
        {
            setSize (w, h);
            sizeCheckCount = 0;
        }
    }
#endif

    //==============================================================================
    class CompMovementWatcher  : public ComponentMovementWatcher
    {
    public:
        CompMovementWatcher (VSTPluginWindow* const owner_)
            : ComponentMovementWatcher (owner_),
              owner (owner_)
        {
        }

        //==============================================================================
        void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/)
        {
            owner->componentMovedOrResized();
        }

        void componentPeerChanged()
        {
            owner->componentPeerChanged();
        }

        void componentVisibilityChanged (Component&)
        {
            owner->componentVisibilityChanged();
        }

    private:
        VSTPluginWindow* const owner;
    };

    CompMovementWatcher* movementWatcher;

    //==============================================================================
    int dispatch (const int opcode, const int index, const int value, void* const ptr, float opt)
    {
        return plugin.dispatch (opcode, index, value, ptr, opt);
    }

    //==============================================================================
    // hooks to get keyboard events from VST windows..
#if JUCE_WIN32
    static LRESULT CALLBACK vstHookWndProc (HWND hW, UINT message, WPARAM wParam, LPARAM lParam)
    {
        for (int i = activeWindows.size(); --i >= 0;)
        {
            const VSTPluginWindow* const w = (const VSTPluginWindow*) activeWindows.getUnchecked (i);

            if (w->pluginHWND == hW)
            {
                if (message == WM_CHAR
                    || message == WM_KEYDOWN
                    || message == WM_SYSKEYDOWN
                    || message == WM_KEYUP
                    || message == WM_SYSKEYUP
                    || message == WM_APPCOMMAND)
                {
                    SendMessage ((HWND) w->getTopLevelComponent()->getWindowHandle(),
                                 message, wParam, lParam);
                }

                return CallWindowProc ((WNDPROC) (w->originalWndProc),
                                       (HWND) w->pluginHWND,
                                       message,
                                       wParam,
                                       lParam);
            }
        }

        return DefWindowProc (hW, message, wParam, lParam);
    }
#endif
};

//==============================================================================
AudioFilterEditor* JUCE_CALLTYPE VSTPluginInstance::createEditor()
{
    if (hasEditor())
        return new VSTPluginWindow (*this);

    return 0;
}


//==============================================================================
void VSTPluginInstance::handleAsyncUpdate()
{
    // indicates that something about the plugin has changed..
    if (callbacks != 0)
        callbacks->updateHostDisplay();
}

//==============================================================================
bool VSTPluginInstance::restoreProgramSettings (const fxProgram* const prog)
{
    if (swap (prog->chunkMagic) == 'CcnK' && swap (prog->fxMagic) == 'FxCk')
    {
        changeProgramName (getCurrentProgram(), prog->prgName);

        for (int i = 0; i < swap (prog->numParams); ++i)
            setParameter (i, swapFloat (prog->params[i]));

        return true;
    }

    return false;
}

bool VSTPluginInstance::loadFromFXBFile (const void* const data,
                                         const int dataSize)
{
    if (dataSize < 28)
        return false;

    const fxSet* const set = (const fxSet*) data;

    if ((swap (set->chunkMagic) != 'CcnK' && swap (set->chunkMagic) != 'KncC')
         || swap (set->version) > fxbVersionNum)
        return false;

    if (swap (set->fxMagic) == 'FxBk')
    {
        // bank of programs
        if (swap (set->numPrograms) >= 0)
        {
            const int oldProg = getCurrentProgram();
            const int numParams = swap (((const fxProgram*) (set->programs))->numParams);
            const int progLen = sizeof (fxProgram) + (numParams - 1) * sizeof (float);

            for (int i = 0; i < swap (set->numPrograms); ++i)
            {
                if (i != oldProg)
                {
                    const fxProgram* const prog = (const fxProgram*) (((const char*) (set->programs)) + i * progLen);
                    if (((const char*) prog) - ((const char*) set) >= dataSize)
                        return false;

                    if (swap (set->numPrograms) > 0)
                        setCurrentProgram (i);

                    if (! restoreProgramSettings (prog))
                        return false;
                }
            }

            if (swap (set->numPrograms) > 0)
                setCurrentProgram (oldProg);

            const fxProgram* const prog = (const fxProgram*) (((const char*) (set->programs)) + oldProg * progLen);
            if (((const char*) prog) - ((const char*) set) >= dataSize)
                return false;

            if (! restoreProgramSettings (prog))
                return false;
        }
    }
    else if (swap (set->fxMagic) == 'FxCk')
    {
        // single program
        const fxProgram* const prog = (const fxProgram*) data;

        if (swap (prog->chunkMagic) != 'CcnK')
            return false;

        changeProgramName (getCurrentProgram(), prog->prgName);

        for (int i = 0; i < swap (prog->numParams); ++i)
            setParameter (i, swapFloat (prog->params[i]));
    }
    else if (swap (set->fxMagic) == 'FBCh' || swap (set->fxMagic) == 'hCBF')
    {
        // non-preset chunk
        const fxChunkSet* const cset = (const fxChunkSet*) data;

        if (swap (cset->chunkSize) + sizeof (fxChunkSet) - 8 > (unsigned int) dataSize)
            return false;

        setChunkData (cset->chunk, swap (cset->chunkSize), false);
    }
    else if (swap (set->fxMagic) == 'FPCh' || swap (set->fxMagic) == 'hCPF')
    {
        // preset chunk
        const fxProgramSet* const cset = (const fxProgramSet*) data;

        if (swap (cset->chunkSize) + sizeof (fxProgramSet) - 8 > (unsigned int) dataSize)
            return false;

        setChunkData (cset->chunk, swap (cset->chunkSize), true);

        changeProgramName (getCurrentProgram(), cset->name);
    }
    else
    {
        return false;
    }

    return true;
}

//==============================================================================
void VSTPluginInstance::setParamsInProgramBlock (fxProgram* const prog) throw()
{
    const int numParams = getNumParameters();

    prog->chunkMagic = swap ('CcnK');
    prog->byteSize = 0;
    prog->fxMagic = swap ('FxCk');
    prog->version = swap (fxbVersionNum);
    prog->fxID = swap (getUID());
    prog->fxVersion = swap (getVersionNumber());
    prog->numParams = swap (numParams);

    getCurrentProgramName().copyToBuffer (prog->prgName, sizeof (prog->prgName) - 1);

    for (int i = 0; i < numParams; ++i)
        prog->params[i] = swapFloat (getParameter (i));
}

bool VSTPluginInstance::saveToFXBFile (juce::MemoryBlock& dest, bool isFXB, int maxSizeMB)
{
    const int numPrograms = getNumPrograms();
    const int numParams = getNumParameters();

    if (usesChunks())
    {
        if (isFXB)
        {
            juce::MemoryBlock chunk;
            getChunkData (chunk, false, maxSizeMB);

            const int totalLen = sizeof (fxChunkSet) + chunk.getSize() - 8;
            dest.setSize (totalLen, true);

            fxChunkSet* const set = (fxChunkSet*) dest.getData();
            set->chunkMagic = swap ('CcnK');
            set->byteSize = 0;
            set->fxMagic = swap ('FBCh');
            set->version = swap (fxbVersionNum);
            set->fxID = swap (getUID());
            set->fxVersion = swap (getVersionNumber());
            set->numPrograms = swap (numPrograms);
            set->chunkSize = swap (chunk.getSize());

            chunk.copyTo (set->chunk, 0, chunk.getSize());
        }
        else
        {
            juce::MemoryBlock chunk;
            getChunkData (chunk, true, maxSizeMB);

            const int totalLen = sizeof (fxProgramSet) + chunk.getSize() - 8;
            dest.setSize (totalLen, true);

            fxProgramSet* const set = (fxProgramSet*) dest.getData();
            set->chunkMagic = swap ('CcnK');
            set->byteSize = 0;
            set->fxMagic = swap ('FPCh');
            set->version = swap (fxbVersionNum);
            set->fxID = swap (getUID());
            set->fxVersion = swap (getVersionNumber());
            set->numPrograms = swap (numPrograms);
            set->chunkSize = swap (chunk.getSize());

            getCurrentProgramName().copyToBuffer (set->name, sizeof (set->name) - 1);
            chunk.copyTo (set->chunk, 0, chunk.getSize());
        }
    }
    else
    {
        if (isFXB)
        {
            const int progLen = sizeof (fxProgram) + (numParams - 1) * sizeof (float);
            const int len = (sizeof (fxSet) - sizeof (fxProgram)) + progLen * jmax (1, numPrograms);
            dest.setSize (len, true);

            fxSet* const set = (fxSet*) dest.getData();
            set->chunkMagic = swap ('CcnK');
            set->byteSize = 0;
            set->fxMagic = swap ('FxBk');
            set->version = swap (fxbVersionNum);
            set->fxID = swap (getUID());
            set->fxVersion = swap (getVersionNumber());
            set->numPrograms = swap (numPrograms);

            const int oldProgram = getCurrentProgram();
            juce::MemoryBlock oldSettings;
            createTempParameterStore (oldSettings);

            setParamsInProgramBlock ((fxProgram*) (((char*) (set->programs)) + oldProgram * progLen));

            for (int i = 0; i < numPrograms; ++i)
            {
                if (i != oldProgram)
                {
                    setCurrentProgram (i);
                    setParamsInProgramBlock ((fxProgram*) (((char*) (set->programs)) + i * progLen));
                }
            }

            setCurrentProgram (oldProgram);
            restoreFromTempParameterStore (oldSettings);
        }
        else
        {
            const int totalLen = sizeof (fxProgram) + (numParams - 1) * sizeof (float);
            dest.setSize (totalLen, true);

            setParamsInProgramBlock ((fxProgram*) dest.getData());
        }
    }

    return true;
}

void VSTPluginInstance::getChunkData (juce::MemoryBlock& mb, bool isPreset, int maxSizeMB) const
{
    if (usesChunks())
    {
        void* data = 0;
        const int bytes = dispatch (effGetChunk, isPreset ? 1 : 0, 0, &data, 0.0f);

        if (data != 0 && bytes <= maxSizeMB * 1024 * 1024)
        {
            mb.setSize (bytes);
            mb.copyFrom (data, 0, bytes);
        }
    }
}

void VSTPluginInstance::setChunkData (const char* data, int size, bool isPreset)
{
    if (size > 0 && usesChunks())
    {
        dispatch (effSetChunk, isPreset ? 1 : 0, size, (void*) data, 0.0f);

        if (! isPreset)
            updateStoredProgramNames();
    }
}

//==============================================================================
void VSTPluginInstance::timerCallback()
{
    if (dispatch (effIdle, 0, 0, 0, 0) == 0)
        stopTimer();
}

int VSTPluginInstance::dispatch (const int opcode, const int index, const int value, void* const ptr, float opt) const
{
    const ScopedLock sl (lock);

    ++insideVSTCallback;
    int result = 0;

    try
    {
        if (effect != 0)
        {
#if JUCE_MAC
            if (module->resFileId != 0)
                UseResFile (module->resFileId);

            CGrafPtr oldPort;

            if (getActiveEditor() != 0)
            {
                int x = 0, y = 0;
                getActiveEditor()->relativePositionToOtherComponent (getActiveEditor()->getTopLevelComponent(), x, y);

                GetPort (&oldPort);
                SetPortWindowPort ((WindowRef) getActiveEditor()->getWindowHandle());
                SetOrigin (-x, -y);
            }
#endif

            result = effect->dispatcher (effect, opcode, index, value, ptr, opt);

#if JUCE_MAC
            if (getActiveEditor() != 0)
                SetPort (oldPort);

            module->resFileId = CurResFile();
#endif

            --insideVSTCallback;
            return result;
        }
    }
    catch (...)
    {
        //char s[512];
        //sprintf (s, "dispatcher (%d, %d, %d, %x, %f)", opcode, index, value, (int)ptr, opt);
    }

    --insideVSTCallback;
    return result;
}

//==============================================================================
// handles non plugin-specific callbacks..
static VstIntPtr handleGeneralCallback (VstInt32 opcode, VstInt32 index, VstInt32 value, void *ptr, float opt)
{
    (void) index;
    (void) value;
    (void) opt;

    switch (opcode)
    {
    case audioMasterCanDo:
        {
            static const char* canDos[] = { "supplyIdle",
                                            "sendVstEvents",
                                            "sendVstMidiEvent",
                                            "sendVstTimeInfo",
                                            "receiveVstEvents",
                                            "receiveVstMidiEvent",
                                            "supportShell",
                                            "shellCategory" };

            for (int i = 0; i < numElementsInArray (canDos); ++i)
                if (strcmp (canDos[i], (const char*) ptr) == 0)
                    return 1;

            return 0;
        }

    case audioMasterVersion:
        return 0x2400;
    case audioMasterCurrentId:
        return shellUIDToCreate;
    case audioMasterGetNumAutomatableParameters:
        return 0;
    case audioMasterGetAutomationState:
        return 1;

    case audioMasterGetVendorVersion:
        return 1;
    case audioMasterGetVendorString:
    case audioMasterGetProductString:
        JUCEApplication::getInstance()
            ->getApplicationName().copyToBuffer ((char*) ptr, jmin (kVstMaxVendorStrLen, kVstMaxProductStrLen) - 1);
        break;

    case audioMasterGetSampleRate:
        return 44100;

    case audioMasterGetBlockSize:
        return 512;

    case audioMasterSetOutputSampleRate:
        return 0;

    default:
        DBG ("*** Unhandled VST Callback: " + String ((int) opcode));
        break;
    }

    return 0;
}

// handles callbacks for a specific plugin
VstIntPtr VSTPluginInstance::handleCallback (VstInt32 opcode, VstInt32 index, VstInt32 value, void *ptr, float opt)
{
    switch (opcode)
    {
    case audioMasterAutomate:
        if (callbacks != 0)
            callbacks->informHostOfParameterChange (index, opt);
        break;

    case audioMasterProcessEvents:
        handleMidiFromPlugin ((const VstEvents*) ptr);
        break;

    case audioMasterGetTime:
        #ifdef _MSC_VER
         #pragma warning (push)
         #pragma warning (disable: 4311)
        #endif

        return (VstIntPtr) &vstHostTime;

        #ifdef _MSC_VER
         #pragma warning (pop)
        #endif
        break;

    case audioMasterIdle:
        if (insideVSTCallback == 0 && MessageManager::getInstance()->isThisTheMessageThread())
        {
            ++insideVSTCallback;
#if JUCE_MAC
            if (getActiveEditor() != 0)
                dispatch (effEditIdle, 0, 0, 0, 0);
#endif
            const MessageManagerLock mml;

            juce_callAnyTimersSynchronously();

            handleUpdateNowIfNeeded();

            for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
                ComponentPeer::getPeer (i)->performAnyPendingRepaintsNow();

            --insideVSTCallback;
        }
        break;

    case audioMasterUpdateDisplay:
        triggerAsyncUpdate();
        break;

    case audioMasterTempoAt:
        // returns (10000 * bpm)
        break;

    case audioMasterNeedIdle:
        startTimer (50);
        break;

    case audioMasterSizeWindow:
        if (getActiveEditor() != 0)
            getActiveEditor()->setSize (index, value);

        return 1;

    case audioMasterGetSampleRate:
        return (VstIntPtr) sampleRate;

    case audioMasterGetBlockSize:
        return (VstIntPtr) blockSize;

    case audioMasterWantMidi:
        wantsMidiMessages = true;
        break;

    case audioMasterGetDirectory:
      #if JUCE_MAC
        return (VstIntPtr) (void*) &module->parentDirFSSpec;
      #else
        return (VstIntPtr) (pointer_sized_uint) (const char*) module->fullParentDirectoryPathName;
      #endif

    case audioMasterGetAutomationState:
        // returns 0: not supported, 1: off, 2:read, 3:write, 4:read/write
        break;

    // none of these are handled (yet)..
    case audioMasterBeginEdit:
    case audioMasterEndEdit:
    case audioMasterSetTime:
    case audioMasterPinConnected:
    case audioMasterGetParameterQuantization:
    case audioMasterIOChanged:
    case audioMasterGetInputLatency:
    case audioMasterGetOutputLatency:
    case audioMasterGetPreviousPlug:
    case audioMasterGetNextPlug:
    case audioMasterWillReplaceOrAccumulate:
    case audioMasterGetCurrentProcessLevel:
    case audioMasterOfflineStart:
    case audioMasterOfflineRead:
    case audioMasterOfflineWrite:
    case audioMasterOfflineGetCurrentPass:
    case audioMasterOfflineGetCurrentMetaPass:
    case audioMasterVendorSpecific:
    case audioMasterSetIcon:
    case audioMasterGetLanguage:
    case audioMasterOpenWindow:
    case audioMasterCloseWindow:
        break;

    default:
        return handleGeneralCallback (opcode, index, value, ptr, opt);
    }

    return 0;
}

// entry point for all callbacks from the plugin
static VstIntPtr VSTCALLBACK audioMaster (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
    try
    {
        if (effect != 0 && effect->resvd2 != 0)
        {
            return ((VSTPluginInstance*)(effect->resvd2))
                        ->handleCallback (opcode, index, value, ptr, opt);
        }

        return handleGeneralCallback (opcode, index, value, ptr, opt);
    }
    catch (...)
    {
        return 0;
    }
}

//==============================================================================
const String VSTPluginInstance::getName() const
{
    return name;
}

const String VSTPluginInstance::getManufacturer() const
{
    char buffer [kVstMaxVendorStrLen + 8];
    zerostruct (buffer);
    dispatch (effGetVendorString, 0, 0, buffer, 0);
    return buffer;
}

const String VSTPluginInstance::getVersion() const
{
    int v = dispatch (effGetVendorVersion, 0, 0, 0, 0);

    String s;

    if (v != 0)
    {
        int versionBits[4];
        int n = 0;

        while (v != 0)
        {
            versionBits [n++] = (v & 0xff);
            v >>= 8;
        }

        s << 'V';

        while (n > 0)
        {
            s << versionBits [--n];

            if (n > 0)
                s << '.';
        }
    }

    return s;
}

int VSTPluginInstance::getVersionNumber() const throw()
{
    return effect != 0 ? effect->version : 0;
}

const String VSTPluginInstance::getFormatName() const
{
    return "VST";
}

const File VSTPluginInstance::getFile() const
{
    return module->file;
}

int VSTPluginInstance::getUID() const
{
    int uid = effect != 0 ? effect->uniqueID : 0;

    if (uid == 0)
        uid = getFile().hashCode();

    return uid;
}

const String VSTPluginInstance::getCategory() const
{
    const char* result = 0;

    switch (dispatch (effGetPlugCategory, 0, 0, 0, 0))
    {
    case kPlugCategEffect:
        result = "Effect";
        break;

    case kPlugCategSynth:
        result = "Synth";
        break;

    case kPlugCategAnalysis:
        result = "Anaylsis";
        break;

    case kPlugCategMastering:
        result = "Mastering";
        break;

    case kPlugCategSpacializer:
        result = "Spacial";
        break;

    case kPlugCategRoomFx:
        result = "Reverb";
        break;

    case kPlugSurroundFx:
        result = "Surround";
        break;

    case kPlugCategRestoration:
        result = "Restoration";
        break;

    case kPlugCategGenerator:
        result = "Tone generation";
        break;

    default:
        break;
    }

    return result;
}

//==============================================================================
int JUCE_CALLTYPE VSTPluginInstance::getNumParameters()
{
    return effect != 0 ? effect->numParams : 0;
}

float JUCE_CALLTYPE VSTPluginInstance::getParameter (int index)
{
    if (effect != 0 && index >= 0 && index < effect->numParams)
    {
        try
        {
            const ScopedLock sl (lock);
            return effect->getParameter (effect, index);
        }
        catch (...)
        {
        }
    }

    return 0.0f;
}

void JUCE_CALLTYPE VSTPluginInstance::setParameter (int index, float newValue)
{
    if (effect != 0 && index >= 0 && index < effect->numParams)
    {
        try
        {
            const ScopedLock sl (lock);

            if (effect->getParameter (effect, index) != newValue)
                effect->setParameter (effect, index, newValue);
        }
        catch (...)
        {
        }
    }
}

const String JUCE_CALLTYPE VSTPluginInstance::getParameterName (int index)
{
    if (effect != 0)
    {
        jassert (index >= 0 && index < effect->numParams);

        char nm [256];
        zerostruct (nm);
        dispatch (effGetParamName, index, 0, nm, 0);
        return String (nm).trim();
    }

    return String::empty;
}

const String VSTPluginInstance::getParameterLabel (int index) const
{
    if (effect != 0)
    {
        jassert (index >= 0 && index < effect->numParams);

        char nm [256];
        zerostruct (nm);
        dispatch (effGetParamLabel, index, 0, nm, 0);
        return String (nm).trim();
    }

    return String::empty;
}

const String JUCE_CALLTYPE VSTPluginInstance::getParameterText (int index)
{
    if (effect != 0)
    {
        jassert (index >= 0 && index < effect->numParams);

        char nm [256];
        zerostruct (nm);
        dispatch (effGetParamDisplay, index, 0, nm, 0);
        return String (nm).trim();
    }

    return String::empty;
}

bool VSTPluginInstance::isParameterAutomatable (int index) const
{
    if (effect != 0)
    {
        jassert (index >= 0 && index < effect->numParams);
        return dispatch (effCanBeAutomated, index, 0, 0, 0) != 0;
    }

    return false;
}

void VSTPluginInstance::createTempParameterStore (juce::MemoryBlock& dest)
{
    dest.setSize (64 + 4 * getNumParameters());
    dest.fillWith (0);

    getCurrentProgramName().copyToBuffer ((char*) dest.getData(), 63);

    float* const p = (float*) (((char*) dest.getData()) + 64);
    for (int i = 0; i < getNumParameters(); ++i)
        p[i] = getParameter(i);
}

void VSTPluginInstance::restoreFromTempParameterStore (const juce::MemoryBlock& m)
{
    changeProgramName (getCurrentProgram(), (const char*) m);

    float* p = (float*) (((char*) m.getData()) + 64);
    for (int i = 0; i < getNumParameters(); ++i)
        setParameter (i, p[i]);
}

//==============================================================================
int JUCE_CALLTYPE VSTPluginInstance::getNumPrograms()
{
    return effect != 0 ? effect->numPrograms : 0;
}

int JUCE_CALLTYPE VSTPluginInstance::getCurrentProgram()
{
    return dispatch (effGetProgram, 0, 0, 0, 0);
}

void JUCE_CALLTYPE VSTPluginInstance::setCurrentProgram (int newIndex)
{
    if (getNumPrograms() > 0 && newIndex != getCurrentProgram())
        dispatch (effSetProgram, 0, jlimit (0, getNumPrograms() - 1, newIndex), 0, 0);
}

const String JUCE_CALLTYPE VSTPluginInstance::getProgramName (int index)
{
    if (index == getCurrentProgram())
    {
        return getCurrentProgramName();
    }
    else if (effect != 0)
    {
        char nm [256];
        zerostruct (nm);

        if (dispatch (effGetProgramNameIndexed,
                      jlimit (0, getNumPrograms(), index),
                      -1, nm, 0) != 0)
        {
            return String (nm).trim();
        }
    }

    return programNames [index];
}

void JUCE_CALLTYPE VSTPluginInstance::changeProgramName (int index, const String& newName)
{
    if (index == getCurrentProgram())
    {
        if (getNumPrograms() > 0 && newName != getCurrentProgramName())
            dispatch (effSetProgramName, 0, 0, (void*) (const char*) newName.substring (0, 24), 0.0f);
    }
    else
    {
        jassertfalse // xxx not implemented!
    }
}

void VSTPluginInstance::updateStoredProgramNames()
{
    if (effect != 0 && getNumPrograms() > 0)
    {
        char nm [256];
        zerostruct (nm);

        // only do this if the plugin can't use indexed names..
        if (dispatch (effGetProgramNameIndexed, 0, -1, nm, 0) == 0)
        {
            const int oldProgram = getCurrentProgram();
            juce::MemoryBlock oldSettings;
            createTempParameterStore (oldSettings);

            for (int i = 0; i < getNumPrograms(); ++i)
            {
                setCurrentProgram (i);
                getCurrentProgramName();  // (this updates the list)
            }

            setCurrentProgram (oldProgram);
            restoreFromTempParameterStore (oldSettings);
        }
    }
}

const String VSTPluginInstance::getCurrentProgramName()
{
    if (effect != 0)
    {
        char nm [256];
        zerostruct (nm);
        dispatch (effGetProgramName, 0, 0, nm, 0);

        const int index = getCurrentProgram();
        if (programNames[index].isEmpty())
        {
            while (programNames.size() < index)
                programNames.add (String::empty);

            programNames.set (index, String (nm).trim());
        }

        return String (nm).trim();
    }

    return String::empty;
}

//==============================================================================
const String VSTPluginInstance::getInputChannelName (const int index) const
{
    if (index >= 0 && index < getNumInputChannels())
    {
        VstPinProperties pinProps;
        if (dispatch (effGetInputProperties, index, 0, &pinProps, 0.0f) != 0)
            return String (pinProps.label, sizeof (pinProps.label));
    }

    return String::empty;
}

bool VSTPluginInstance::isInputChannelStereoPair (int index) const
{
    if (index < 0 || index >= getNumInputChannels())
        return false;

    VstPinProperties pinProps;
    if (dispatch (effGetInputProperties, index, 0, &pinProps, 0.0f) != 0)
        return (pinProps.flags & kVstPinIsStereo) != 0;

    return true;
}

const String VSTPluginInstance::getOutputChannelName (const int index) const
{
    if (index >= 0 && index < getNumOutputChannels())
    {
        VstPinProperties pinProps;
        if (dispatch (effGetOutputProperties, index, 0, &pinProps, 0.0f) != 0)
            return String (pinProps.label, sizeof (pinProps.label));
    }

    return String::empty;
}

bool VSTPluginInstance::isOutputChannelStereoPair (int index) const
{
    if (index < 0 || index >= getNumOutputChannels())
        return false;

    VstPinProperties pinProps;
    if (dispatch (effGetOutputProperties, index, 0, &pinProps, 0.0f) != 0)
        return (pinProps.flags & kVstPinIsStereo) != 0;

    return true;
}

//==============================================================================
bool VSTPluginInstance::acceptsMidi() const
{
    return wantsMidiMessages;
}

bool VSTPluginInstance::producesMidi() const
{
    return dispatch (effCanDo, 0, 0, (void*) "sendVstMidiEvent", 0) > 0;
}

int VSTPluginInstance::getSamplesLatency() const
{
    return effect != 0 ? effect->initialDelay : 0;
}

void VSTPluginInstance::setPower (const bool on)
{
    dispatch (effMainsChanged, 0, on ? 1 : 0, 0, 0);
    isPowerOn = on;
}

bool VSTPluginInstance::hasEditor() const throw()
{
    return effect != 0 && (effect->flags & effFlagsHasEditor) != 0;
}

bool VSTPluginInstance::canMono() const throw()
{
    return effect != 0 && (effect->flags & effFlagsCanMono) != 0;
}

bool VSTPluginInstance::canReplace() const throw()
{
    return effect != 0 && (effect->flags & effFlagsCanReplacing) != 0;
}

bool VSTPluginInstance::isOffline() const throw()
{
    return dispatch (effCanDo, 0, 0, (void*) "offline", 0) > 0;
}

bool VSTPluginInstance::isInstrument() const
{
    return effect != 0 && (effect->flags & effFlagsIsSynth) != 0;
}

bool VSTPluginInstance::usesChunks() const throw()
{
    return effect != 0 && (effect->flags & effFlagsProgramChunks) != 0;
}


//==============================================================================
const int defaultMaxSizeMB = 64;

void JUCE_CALLTYPE VSTPluginInstance::getStateInformation (JUCE_NAMESPACE::MemoryBlock& destData)
{
    saveToFXBFile (destData, true, defaultMaxSizeMB);
}

void JUCE_CALLTYPE VSTPluginInstance::getCurrentProgramStateInformation (JUCE_NAMESPACE::MemoryBlock& destData)
{
    saveToFXBFile (destData, false, defaultMaxSizeMB);
}

void JUCE_CALLTYPE VSTPluginInstance::setStateInformation (const void* data, int sizeInBytes)
{
    loadFromFXBFile (data, sizeInBytes);
}

void JUCE_CALLTYPE VSTPluginInstance::setCurrentProgramStateInformation (const void* data, int sizeInBytes)
{
    loadFromFXBFile (data, sizeInBytes);
}

//==============================================================================
//==============================================================================
VSTPluginFormat::VSTPluginFormat()
{
}

VSTPluginFormat::~VSTPluginFormat()
{
}

void VSTPluginFormat::findAllTypesForFile (OwnedArray <PluginDescription>& results,
                                           const File& file)
{
    if (! fileMightContainThisPluginType (file))
        return;

    PluginDescription desc;
    desc.file = file;
    desc.uid = 0;

    VSTPluginInstance* instance = dynamic_cast <VSTPluginInstance*> (createInstanceFromDescription (desc));

    if (instance == 0)
        return;

    try
    {
#if JUCE_MAC
        if (instance->module->resFileId != 0)
            UseResFile (instance->module->resFileId);
#endif

        desc.fillInFromInstance (*instance);

        VstPlugCategory category = (VstPlugCategory) instance->dispatch (effGetPlugCategory, 0, 0, 0, 0);

        if (category != kPlugCategShell)
        {
            // Normal plugin...
            results.add (new PluginDescription (desc));

            ++insideVSTCallback;
            instance->dispatch (effOpen, 0, 0, 0, 0);
            --insideVSTCallback;
        }
        else
        {
            // It's a shell plugin, so iterate all the subtypes...
            char shellEffectName [64];

            for (;;)
            {
                zerostruct (shellEffectName);
                const int uid = instance->dispatch (effShellGetNextPlugin, 0, 0, shellEffectName, 0);

                if (uid == 0)
                {
                    break;
                }
                else
                {
                    desc.uid = uid;
                    desc.name = shellEffectName;

                    bool alreadyThere = false;

                    for (int i = results.size(); --i >= 0;)
                    {
                        PluginDescription* const d = results.getUnchecked(i);

                        if (d->isDuplicateOf (desc))
                        {
                            alreadyThere = true;
                            break;
                        }
                    }

                    if (! alreadyThere)
                        results.add (new PluginDescription (desc));
                }
            }
        }
    }
    catch (...)
    {
        // crashed while loading...
    }

    deleteAndZero (instance);
}

AudioPluginInstance* VSTPluginFormat::createInstanceFromDescription (const PluginDescription& desc)
{
    VSTPluginInstance* result = 0;

    if (fileMightContainThisPluginType (desc.file))
    {
        const File previousWorkingDirectory (File::getCurrentWorkingDirectory());
        desc.file.getParentDirectory().setAsCurrentWorkingDirectory();

        const ReferenceCountedObjectPtr <ModuleHandle> module (ModuleHandle::findOrCreateModule (desc.file));

        if (module != 0)
        {
            shellUIDToCreate = desc.uid;

            result = new VSTPluginInstance (module);

            if (result->effect != 0)
            {
                result->effect->resvd2 = (VstIntPtr) (pointer_sized_int) result;
                result->initialise();
            }
            else
            {
                deleteAndZero (result);
            }
        }

        previousWorkingDirectory.setAsCurrentWorkingDirectory();
    }

    return result;
}

bool VSTPluginFormat::fileMightContainThisPluginType (const File& f)
{
#if JUCE_MAC
    if (f.isDirectory() && f.hasFileExtension (T(".vst")))
        return true;

#if JUCE_PPC
    FSRef fileRef;
    if (PlatformUtilities::makeFSRefFromPath (&fileRef, f.getFullPathName()))
    {
        const short resFileId = FSOpenResFile (&fileRef, fsRdPerm);

        if (resFileId != -1)
        {
            const int numEffects = Count1Resources ('aEff');
            CloseResFile (resFileId);

            if (numEffects > 0)
                return true;
        }
    }
#endif

    return false;
#else
    return f.existsAsFile()
            && f.hasFileExtension (T(".dll"));
#endif
}

const FileSearchPath VSTPluginFormat::getDefaultLocationsToSearch()
{
#if JUCE_MAC
    return FileSearchPath ("~/Library/Audio/Plug-Ins/VST;/Library/Audio/Plug-Ins/VST");
#else
    const String programFiles (File::getSpecialLocation (File::globalApplicationsDirectory).getFullPathName());

    return FileSearchPath (programFiles + "\\Steinberg\\VstPlugins");
#endif
}
