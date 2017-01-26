/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#if JUCE_PLUGINHOST_VST && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_IOS)

//==============================================================================
#if JUCE_MAC && JUCE_SUPPORT_CARBON
 #include "../../juce_gui_extra/native/juce_mac_CarbonViewWrapperComponent.h"
#endif

//==============================================================================
#undef PRAGMA_ALIGN_SUPPORTED

#if JUCE_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4996)
#elif ! JUCE_MINGW
 #define __cdecl
#endif

namespace Vst2
{
#include "juce_VSTInterface.h"
}

using namespace Vst2;

#include "juce_VSTCommon.h"

#if JUCE_MSVC
 #pragma warning (pop)
 #pragma warning (disable: 4355) // ("this" used in initialiser list warning)
#endif

//==============================================================================
#include "juce_VSTMidiEventList.h"

#if JUCE_MINGW
 #ifndef WM_APPCOMMAND
  #define WM_APPCOMMAND 0x0319
 #endif

 extern "C" void _fpreset();
 extern "C" void _clearfp();
#elif ! JUCE_WINDOWS
 static void _fpreset() {}
 static void _clearfp() {}
#endif

#ifndef JUCE_VST_WRAPPER_LOAD_CUSTOM_MAIN
 #define JUCE_VST_WRAPPER_LOAD_CUSTOM_MAIN
#endif

#ifndef JUCE_VST_WRAPPER_INVOKE_MAIN
 #define JUCE_VST_WRAPPER_INVOKE_MAIN  effect = module->moduleMain (&audioMaster);
#endif

//==============================================================================
namespace
{
    const int fxbVersionNum = 1;

    struct fxProgram
    {
        int32 chunkMagic;    // 'CcnK'
        int32 byteSize;      // of this chunk, excl. magic + byteSize
        int32 fxMagic;       // 'FxCk'
        int32 version;
        int32 fxID;          // fx unique id
        int32 fxVersion;
        int32 numParams;
        char prgName[28];
        float params[1];        // variable no. of parameters
    };

    struct fxSet
    {
        int32 chunkMagic;    // 'CcnK'
        int32 byteSize;      // of this chunk, excl. magic + byteSize
        int32 fxMagic;       // 'FxBk'
        int32 version;
        int32 fxID;          // fx unique id
        int32 fxVersion;
        int32 numPrograms;
        char future[128];
        fxProgram programs[1];  // variable no. of programs
    };

    struct fxChunkSet
    {
        int32 chunkMagic;    // 'CcnK'
        int32 byteSize;      // of this chunk, excl. magic + byteSize
        int32 fxMagic;       // 'FxCh', 'FPCh', or 'FBCh'
        int32 version;
        int32 fxID;          // fx unique id
        int32 fxVersion;
        int32 numPrograms;
        char future[128];
        int32 chunkSize;
        char chunk[8];          // variable
    };

    struct fxProgramSet
    {
        int32 chunkMagic;    // 'CcnK'
        int32 byteSize;      // of this chunk, excl. magic + byteSize
        int32 fxMagic;       // 'FxCh', 'FPCh', or 'FBCh'
        int32 version;
        int32 fxID;          // fx unique id
        int32 fxVersion;
        int32 numPrograms;
        char name[28];
        int32 chunkSize;
        char chunk[8];          // variable
    };

    // Compares a magic value in either endianness.
    static bool compareMagic (int32 magic, const char* name) noexcept
    {
        return magic == (int32) ByteOrder::littleEndianInt (name)
            || magic == (int32) ByteOrder::bigEndianInt (name);
    }

    static int32 fxbName (const char* name) noexcept   { return (int32) ByteOrder::littleEndianInt (name); }
    static int32 fxbSwap (const int32 x) noexcept   { return (int32) ByteOrder::swapIfLittleEndian ((uint32) x); }

    static float fxbSwapFloat (const float x) noexcept
    {
       #ifdef JUCE_LITTLE_ENDIAN
        union { uint32 asInt; float asFloat; } n;
        n.asFloat = x;
        n.asInt = ByteOrder::swap (n.asInt);
        return n.asFloat;
       #else
        return x;
       #endif
    }
}

//==============================================================================
namespace
{
    static double getVSTHostTimeNanoseconds() noexcept
    {
       #if JUCE_WINDOWS
        return timeGetTime() * 1000000.0;
       #elif JUCE_LINUX || JUCE_IOS
        timeval micro;
        gettimeofday (&micro, 0);
        return micro.tv_usec * 1000.0;
       #elif JUCE_MAC
        UnsignedWide micro;
        Microseconds (&micro);
        return micro.lo * 1000.0;
       #endif
    }

    static int shellUIDToCreate = 0;
    static int insideVSTCallback = 0;

    struct IdleCallRecursionPreventer
    {
        IdleCallRecursionPreventer()  : isMessageThread (MessageManager::getInstance()->isThisTheMessageThread())
        {
            if (isMessageThread)
                ++insideVSTCallback;
        }

        ~IdleCallRecursionPreventer()
        {
            if (isMessageThread)
                --insideVSTCallback;
        }

        const bool isMessageThread;
        JUCE_DECLARE_NON_COPYABLE (IdleCallRecursionPreventer)
    };

   #if JUCE_MAC
    static bool makeFSRefFromPath (FSRef* destFSRef, const String& path)
    {
        return FSPathMakeRef (reinterpret_cast<const UInt8*> (path.toRawUTF8()), destFSRef, 0) == noErr;
    }
   #endif
}

//==============================================================================
typedef VstEffectInterface* (VSTINTERFACECALL *MainCall) (VstHostCallback);
static pointer_sized_int VSTINTERFACECALL audioMaster (VstEffectInterface* effect, int32 opcode, int32 index, pointer_sized_int value, void* ptr, float opt);

//==============================================================================
// Change this to disable logging of various VST activities
#ifndef VST_LOGGING
 #define VST_LOGGING 1
#endif

#if VST_LOGGING
 #define JUCE_VST_LOG(a) Logger::writeToLog(a);
#else
 #define JUCE_VST_LOG(a)
#endif

//==============================================================================
#if JUCE_LINUX

extern Display* display;
extern XContext windowHandleXContext;

namespace
{
    static bool xErrorTriggered = false;

    static int temporaryErrorHandler (Display*, XErrorEvent*)
    {
        xErrorTriggered = true;
        return 0;
    }

    typedef void (*EventProcPtr) (XEvent* ev);

    static EventProcPtr getPropertyFromXWindow (Window handle, Atom atom)
    {
        XErrorHandler oldErrorHandler = XSetErrorHandler (temporaryErrorHandler);
        xErrorTriggered = false;

        int userSize;
        unsigned long bytes, userCount;
        unsigned char* data;
        Atom userType;

        XGetWindowProperty (display, handle, atom, 0, 1, false, AnyPropertyType,
                            &userType,  &userSize, &userCount, &bytes, &data);

        XSetErrorHandler (oldErrorHandler);

        return (userCount == 1 && ! xErrorTriggered) ? *reinterpret_cast<EventProcPtr*> (data) : nullptr;
    }

    Window getChildWindow (Window windowToCheck)
    {
        Window rootWindow, parentWindow;
        Window* childWindows;
        unsigned int numChildren = 0;

        XQueryTree (display, windowToCheck, &rootWindow, &parentWindow, &childWindows, &numChildren);

        if (numChildren > 0)
            return childWindows [0];

        return 0;
    }

    static void translateJuceToXButtonModifiers (const MouseEvent& e, XEvent& ev) noexcept
    {
        if (e.mods.isLeftButtonDown())
        {
            ev.xbutton.button = Button1;
            ev.xbutton.state |= Button1Mask;
        }
        else if (e.mods.isRightButtonDown())
        {
            ev.xbutton.button = Button3;
            ev.xbutton.state |= Button3Mask;
        }
        else if (e.mods.isMiddleButtonDown())
        {
            ev.xbutton.button = Button2;
            ev.xbutton.state |= Button2Mask;
        }
    }

    static void translateJuceToXMotionModifiers (const MouseEvent& e, XEvent& ev) noexcept
    {
        if (e.mods.isLeftButtonDown())          ev.xmotion.state |= Button1Mask;
        else if (e.mods.isRightButtonDown())    ev.xmotion.state |= Button3Mask;
        else if (e.mods.isMiddleButtonDown())   ev.xmotion.state |= Button2Mask;
    }

    static void translateJuceToXCrossingModifiers (const MouseEvent& e, XEvent& ev) noexcept
    {
        if (e.mods.isLeftButtonDown())          ev.xcrossing.state |= Button1Mask;
        else if (e.mods.isRightButtonDown())    ev.xcrossing.state |= Button3Mask;
        else if (e.mods.isMiddleButtonDown())   ev.xcrossing.state |= Button2Mask;
    }

    static void translateJuceToXMouseWheelModifiers (const MouseEvent& e, const float increment, XEvent& ev) noexcept
    {
        ignoreUnused (e);

        if (increment < 0)
        {
            ev.xbutton.button = Button5;
            ev.xbutton.state |= Button5Mask;
        }
        else if (increment > 0)
        {
            ev.xbutton.button = Button4;
            ev.xbutton.state |= Button4Mask;
        }
    }
}

#endif

//==============================================================================
class ModuleHandle    : public ReferenceCountedObject
{
public:
    //==============================================================================
    File file;
    MainCall moduleMain, customMain;
    String pluginName;
    ScopedPointer<XmlElement> vstXml;

    typedef ReferenceCountedObjectPtr<ModuleHandle> Ptr;

    static Array<ModuleHandle*>& getActiveModules()
    {
        static Array<ModuleHandle*> activeModules;
        return activeModules;
    }

    //==============================================================================
    static Ptr findOrCreateModule (const File& file)
    {
        for (int i = getActiveModules().size(); --i >= 0;)
        {
            ModuleHandle* const module = getActiveModules().getUnchecked(i);

            if (module->file == file)
                return module;
        }

        const IdleCallRecursionPreventer icrp;
        shellUIDToCreate = 0;
        _fpreset();

        JUCE_VST_LOG ("Attempting to load VST: " + file.getFullPathName());

        Ptr m = new ModuleHandle (file, nullptr);

        if (m->open())
        {
            _fpreset();
            return m;
        }

        return nullptr;
    }

    //==============================================================================
    ModuleHandle (const File& f, MainCall customMainCall)
        : file (f), moduleMain (customMainCall), customMain (nullptr)
         #if JUCE_MAC || JUCE_IOS
          , resHandle (0), bundleRef (0)
         #if JUCE_MAC
          , resFileId (0)
         #endif
         #endif
    {
        getActiveModules().add (this);

       #if JUCE_WINDOWS || JUCE_LINUX || JUCE_IOS
        fullParentDirectoryPathName = f.getParentDirectory().getFullPathName();
       #elif JUCE_MAC
        FSRef ref;
        makeFSRefFromPath (&ref, f.getParentDirectory().getFullPathName());
        FSGetCatalogInfo (&ref, kFSCatInfoNone, 0, 0, &parentDirFSSpec, 0);
       #endif
    }

    ~ModuleHandle()
    {
        getActiveModules().removeFirstMatchingValue (this);
        close();
    }

    //==============================================================================
   #if ! JUCE_MAC
    String fullParentDirectoryPathName;
   #endif

  #if JUCE_WINDOWS || JUCE_LINUX
    DynamicLibrary module;

    bool open()
    {
        if (moduleMain != nullptr)
            return true;

        pluginName = file.getFileNameWithoutExtension();

        module.open (file.getFullPathName());

        moduleMain = (MainCall) module.getFunction ("VSTPluginMain");

        if (moduleMain == nullptr)
            moduleMain = (MainCall) module.getFunction ("main");

        JUCE_VST_WRAPPER_LOAD_CUSTOM_MAIN

        if (moduleMain != nullptr)
        {
            vstXml = XmlDocument::parse (file.withFileExtension ("vstxml"));

           #if JUCE_WINDOWS
            if (vstXml == nullptr)
                vstXml = XmlDocument::parse (getDLLResource (file, "VSTXML", 1));
           #endif
        }

        return moduleMain != nullptr;
    }

    void close()
    {
        _fpreset(); // (doesn't do any harm)

        module.close();
    }

    void closeEffect (VstEffectInterface* eff)
    {
        eff->dispatchFunction (eff, plugInOpcodeClose, 0, 0, 0, 0);
    }

   #if JUCE_WINDOWS
    static String getDLLResource (const File& dllFile, const String& type, int resID)
    {
        DynamicLibrary dll (dllFile.getFullPathName());
        HMODULE dllModule = (HMODULE) dll.getNativeHandle();

        if (dllModule != INVALID_HANDLE_VALUE)
        {
            if (HRSRC res = FindResource (dllModule, MAKEINTRESOURCE (resID), type.toWideCharPointer()))
            {
                if (HGLOBAL hGlob = LoadResource (dllModule, res))
                {
                    const char* data = static_cast<const char*> (LockResource (hGlob));
                    return String::fromUTF8 (data, SizeofResource (dllModule, res));
                }
            }
        }

        return String();
    }
   #endif
  #else
    Handle resHandle;
    CFBundleRef bundleRef;

   #if JUCE_MAC
    CFBundleRefNum resFileId;
    FSSpec parentDirFSSpec;
   #endif

    bool open()
    {
        if (moduleMain != nullptr)
            return true;

        bool ok = false;

        if (file.hasFileExtension (".vst"))
        {
            const char* const utf8 = file.getFullPathName().toRawUTF8();

            if (CFURLRef url = CFURLCreateFromFileSystemRepresentation (0, (const UInt8*) utf8,
                                                                        (CFIndex) strlen (utf8), file.isDirectory()))
            {
                bundleRef = CFBundleCreate (kCFAllocatorDefault, url);
                CFRelease (url);

                if (bundleRef != 0)
                {
                    if (CFBundleLoadExecutable (bundleRef))
                    {
                        moduleMain = (MainCall) CFBundleGetFunctionPointerForName (bundleRef, CFSTR("main_macho"));

                        if (moduleMain == nullptr)
                            moduleMain = (MainCall) CFBundleGetFunctionPointerForName (bundleRef, CFSTR("VSTPluginMain"));

                        JUCE_VST_WRAPPER_LOAD_CUSTOM_MAIN

                        if (moduleMain != nullptr)
                        {
                            if (CFTypeRef name = CFBundleGetValueForInfoDictionaryKey (bundleRef, CFSTR("CFBundleName")))
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

                           #if JUCE_MAC
                            resFileId = CFBundleOpenBundleResourceMap (bundleRef);
                           #endif

                            ok = true;

                            Array<File> vstXmlFiles;
                            file
                               #if JUCE_MAC
                                .getChildFile ("Contents")
                                .getChildFile ("Resources")
                               #endif
                                .findChildFiles (vstXmlFiles, File::findFiles, false, "*.vstxml");

                            if (vstXmlFiles.size() > 0)
                                vstXml = XmlDocument::parse (vstXmlFiles.getReference(0));
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

        return ok;
    }

    void close()
    {
        if (bundleRef != 0)
        {
           #if JUCE_MAC
            CFBundleCloseBundleResourceMap (bundleRef, resFileId);
           #endif

            if (CFGetRetainCount (bundleRef) == 1)
                CFBundleUnloadExecutable (bundleRef);

            if (CFGetRetainCount (bundleRef) > 0)
                CFRelease (bundleRef);
        }
    }

    void closeEffect (VstEffectInterface* eff)
    {
        eff->dispatchFunction (eff, plugInOpcodeClose, 0, 0, 0, 0);
    }

  #endif

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModuleHandle)
};

static const int defaultVSTSampleRateValue = 44100;
static const int defaultVSTBlockSizeValue = 512;

#if JUCE_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4996) // warning about overriding deprecated methods
#endif

//==============================================================================
//==============================================================================
class VSTPluginInstance     : public AudioPluginInstance,
                              private Timer,
                              private AsyncUpdater
{
private:
    VSTPluginInstance (const ModuleHandle::Ptr& mh, const BusesProperties& ioConfig, VstEffectInterface* effect)
        : AudioPluginInstance (ioConfig),
          vstEffect (effect),
          vstModule (mh),
          usesCocoaNSView (false),
          name (mh->pluginName),
          wantsMidiMessages (false),
          initialised (false),
          isPowerOn (false)
    {}

public:
    ~VSTPluginInstance()
    {
        if (vstEffect != nullptr && vstEffect->interfaceIdentifier == juceVstInterfaceIdentifier)
        {
            struct VSTDeleter : public CallbackMessage
            {
                VSTDeleter (VSTPluginInstance& inInstance, WaitableEvent& inEvent)
                    : vstInstance (inInstance), completionSignal (inEvent)
                {}

                void messageCallback() override
                {
                    vstInstance.cleanup();
                    completionSignal.signal();
                }

                VSTPluginInstance& vstInstance;
                WaitableEvent& completionSignal;
            };

            if (MessageManager::getInstance()->isThisTheMessageThread())
            {
                cleanup();
            }
            else
            {
                WaitableEvent completionEvent;
                (new VSTDeleter (*this, completionEvent))->post();
                completionEvent.wait();
            }
        }
    }

    void cleanup()
    {
        if (vstEffect != nullptr && vstEffect->interfaceIdentifier == juceVstInterfaceIdentifier)
        {
           #if JUCE_MAC
            if (vstModule->resFileId != 0)
                UseResFile (vstModule->resFileId);
           #endif

            // Must delete any editors before deleting the plugin instance!
            jassert (getActiveEditor() == 0);

            _fpreset(); // some dodgy plugs fuck around with this

            vstModule->closeEffect (vstEffect);
        }

        vstModule = nullptr;
        vstEffect = nullptr;
    }

    static VSTPluginInstance* create (const ModuleHandle::Ptr& newModule,
                                      double initialSampleRate,
                                      int initialBlockSize)
    {
        if (VstEffectInterface* newEffect = constructEffect (newModule))
        {
            newEffect->hostSpace2 = 0;

            newEffect->dispatchFunction (newEffect, plugInOpcodeIdentify, 0, 0, 0, 0);

            newEffect->dispatchFunction (newEffect, plugInOpcodeSetSampleRate, 0, 0, 0, static_cast<float> (initialSampleRate));
            newEffect->dispatchFunction (newEffect, plugInOpcodeSetBlockSize,  0, jmax (32, initialBlockSize), 0, 0);

            newEffect->dispatchFunction (newEffect, plugInOpcodeOpen, 0, 0, 0, 0);
            BusesProperties ioConfig = queryBusIO (newEffect);
            newEffect->dispatchFunction (newEffect, plugInOpcodeClose, 0, 0, 0, 0);

            newEffect = constructEffect (newModule);

            if (newEffect != nullptr)
                return new VSTPluginInstance (newModule, ioConfig, newEffect);
        }

        return nullptr;
    }

    //==============================================================================
    void fillInPluginDescription (PluginDescription& desc) const override
    {
        desc.name = name;

        {
            char buffer[512] = { 0 };
            dispatch (plugInOpcodeGetPlugInName, 0, 0, buffer, 0);

            desc.descriptiveName = String::createStringFromData (buffer, (int) sizeof (buffer)).trim();

            if (desc.descriptiveName.isEmpty())
                desc.descriptiveName = name;
        }

        desc.fileOrIdentifier = vstModule->file.getFullPathName();
        desc.uid = getUID();
        desc.lastFileModTime = vstModule->file.getLastModificationTime();
        desc.lastInfoUpdateTime = Time::getCurrentTime();
        desc.pluginFormatName = "VST";
        desc.category = getCategory();

        {
            char buffer[512] = { 0 };
            dispatch (plugInOpcodeGetManufacturerName, 0, 0, buffer, 0);
            desc.manufacturerName = String::createStringFromData (buffer, (int) sizeof (buffer)).trim();
        }

        desc.version = getVersion();
        desc.numInputChannels = getTotalNumInputChannels();
        desc.numOutputChannels = getTotalNumOutputChannels();
        desc.isInstrument = (vstEffect != nullptr && (vstEffect->flags & vstEffectFlagIsSynth) != 0);
    }

    bool initialiseEffect (double initialSampleRate, int initialBlockSize)
    {
        if (vstEffect != nullptr)
        {
            vstEffect->hostSpace2 = (pointer_sized_int) (pointer_sized_int) this;
            initialise (initialSampleRate, initialBlockSize);
            return true;
        }

        return false;
    }

    void initialise (double initialSampleRate, int initialBlockSize)
    {
        if (initialised || vstEffect == nullptr)
            return;

       #if JUCE_WINDOWS
        // On Windows it's highly advisable to create your plugins using the message thread,
        // because many plugins need a chance to create HWNDs that will get their
        // messages delivered by the main message thread, and that's not possible from
        // a background thread.
        jassert (MessageManager::getInstance()->isThisTheMessageThread());
       #endif

        JUCE_VST_LOG ("Initialising VST: " + vstModule->pluginName + " (" + getVersion() + ")");
        initialised = true;

        setRateAndBufferSizeDetails (initialSampleRate, initialBlockSize);

        dispatch (plugInOpcodeIdentify, 0, 0, 0, 0);

        if (getSampleRate() > 0)
            dispatch (plugInOpcodeSetSampleRate, 0, 0, 0, (float) getSampleRate());

        if (getBlockSize() > 0)
            dispatch (plugInOpcodeSetBlockSize, 0, jmax (32, getBlockSize()), 0, 0);

        dispatch (plugInOpcodeOpen, 0, 0, 0, 0);

        setRateAndBufferSizeDetails (getSampleRate(), getBlockSize());

        if (getNumPrograms() > 1)
            setCurrentProgram (0);
        else
            dispatch (plugInOpcodeSetCurrentProgram, 0, 0, 0, 0);

        for (int i = vstEffect->numInputChannels;  --i >= 0;)  dispatch (plugInOpcodeConnectInput,  i, 1, 0, 0);
        for (int i = vstEffect->numOutputChannels; --i >= 0;)  dispatch (plugInOpcodeConnectOutput, i, 1, 0, 0);

        if (getVstCategory() != kPlugCategShell) // (workaround for Waves 5 plugins which crash during this call)
            updateStoredProgramNames();

        wantsMidiMessages = pluginCanDo ("receiveVstMidiEvent") > 0;

       #if JUCE_MAC && JUCE_SUPPORT_CARBON
        usesCocoaNSView = ((unsigned int) pluginCanDo ("hasCockosViewAsConfig") & 0xffff0000ul) == 0xbeef0000ul;
       #endif

        setLatencySamples (vstEffect->latency);
    }

    void* getPlatformSpecificData() override    { return vstEffect; }

    const String getName() const override
    {
        if (vstEffect != nullptr)
        {
            char buffer[512] = { 0 };

            if (dispatch (plugInOpcodeGetManufacturerProductName, 0, 0, buffer, 0) != 0)
            {
                String productName = String::createStringFromData (buffer, (int) sizeof (buffer));

                if (productName.isNotEmpty())
                    return productName;
            }
        }

        return name;
    }

    int getUID() const
    {
        int uid = vstEffect != nullptr ? vstEffect->plugInIdentifier : 0;

        if (uid == 0)
            uid = vstModule->file.hashCode();

        return uid;
    }

    double getTailLengthSeconds() const override
    {
        if (vstEffect == nullptr)
            return 0.0;

        const double sampleRate = getSampleRate();

        if (sampleRate <= 0)
            return 0.0;

        pointer_sized_int samples = dispatch (plugInOpcodeGetTailSize, 0, 0, 0, 0);
        return samples / sampleRate;
    }

    bool acceptsMidi() const override    { return wantsMidiMessages; }
    bool producesMidi() const override   { return pluginCanDo ("sendVstMidiEvent") > 0; }
    bool supportsMPE() const override    { return pluginCanDo ("MPE") > 0; }

    VstPlugInCategory getVstCategory() const noexcept     { return (VstPlugInCategory) dispatch (plugInOpcodeGetPlugInCategory, 0, 0, 0, 0); }

    int pluginCanDo (const char* text) const     { return (int) dispatch (plugInOpcodeCanPlugInDo, 0, 0, (void*) text,  0); }

    //==============================================================================
    void prepareToPlay (double rate, int samplesPerBlockExpected) override
    {
        setRateAndBufferSizeDetails (rate, samplesPerBlockExpected);

        VstSpeakerConfiguration inArr, outArr;

        SpeakerMappings::channelSetToVstArrangement (getChannelLayoutOfBus (true,  0), inArr);
        SpeakerMappings::channelSetToVstArrangement (getChannelLayoutOfBus (false, 0), outArr);

        dispatch (plugInOpcodeSetSpeakerConfiguration, 0, reinterpret_cast<pointer_sized_int> (&inArr), &outArr, 0.0f);

        vstHostTime.tempoBPM = 120.0;
        vstHostTime.timeSignatureNumerator = 4;
        vstHostTime.timeSignatureDenominator    = 4;
        vstHostTime.sampleRate = rate;
        vstHostTime.samplePosition = 0;
        vstHostTime.flags = vstTimingInfoFlagNanosecondsValid
                              | vstTimingInfoFlagAutomationWriteModeActive
                              | vstTimingInfoFlagAutomationReadModeActive;

        initialise (rate, samplesPerBlockExpected);

        if (initialised)
        {
            wantsMidiMessages = wantsMidiMessages || (pluginCanDo ("receiveVstMidiEvent") > 0);

            if (wantsMidiMessages)
                midiEventsToSend.ensureSize (256);
            else
                midiEventsToSend.freeEvents();

            incomingMidi.clear();

            dispatch (plugInOpcodeSetSampleRate, 0, 0, 0, (float) rate);
            dispatch (plugInOpcodeSetBlockSize, 0, jmax (16, samplesPerBlockExpected), 0, 0);

            if (supportsDoublePrecisionProcessing())
            {
                int32 vstPrecision = isUsingDoublePrecision() ? vstProcessingSampleTypeDouble
                                                              : vstProcessingSampleTypeFloat;

                dispatch (plugInOpcodeSetSampleFloatType, 0, (pointer_sized_int) vstPrecision, 0, 0);
            }

            tempBuffer.setSize (jmax (1, vstEffect->numInputChannels), samplesPerBlockExpected);

            if (! isPowerOn)
                setPower (true);

            // dodgy hack to force some plugins to initialise the sample rate..
            if ((! hasEditor()) && getNumParameters() > 0)
            {
                const float old = getParameter (0);
                setParameter (0, (old < 0.5f) ? 1.0f : 0.0f);
                setParameter (0, old);
            }

            dispatch (plugInOpcodeStartProcess, 0, 0, 0, 0);

            setLatencySamples (vstEffect->latency);
        }
    }

    void releaseResources() override
    {
        if (initialised)
        {
            dispatch (plugInOpcodeStopProcess, 0, 0, 0, 0);
            setPower (false);
        }

        tempBuffer.setSize (1, 1);
        incomingMidi.clear();

        midiEventsToSend.freeEvents();
    }

    void reset() override
    {
        if (isPowerOn)
        {
            setPower (false);
            setPower (true);
        }
    }

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (! isUsingDoublePrecision());
        processAudio (buffer, midiMessages);
    }

    void processBlock (AudioBuffer<double>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (isUsingDoublePrecision());
        processAudio (buffer, midiMessages);
    }

    bool supportsDoublePrecisionProcessing() const override
    {
        return ((vstEffect->flags & vstEffectFlagInplaceAudio) != 0
             && (vstEffect->flags & vstEffectFlagInplaceDoubleAudio) != 0);
    }

    //==============================================================================
    bool canAddBus (bool) const override                                       { return false; }
    bool canRemoveBus (bool) const override                                    { return false; }

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        const int numInputBuses  = getBusCount (true);
        const int numOutputBuses = getBusCount (false);

        // it's not possible to change layout if there are sidechains/aux buses
        if (numInputBuses > 1 || numOutputBuses > 1)
            return (layouts == getBusesLayout());

        return (layouts.getNumChannels (true,  0) <= vstEffect->numInputChannels
             && layouts.getNumChannels (false, 0) <= vstEffect->numOutputChannels);
    }

    //==============================================================================
   #if JUCE_IOS
    bool hasEditor() const override                  { return false; }
   #else
    bool hasEditor() const override                  { return vstEffect != nullptr && (vstEffect->flags & vstEffectFlagHasEditor) != 0; }
   #endif

    AudioProcessorEditor* createEditor() override;

    //==============================================================================
    const String getInputChannelName (int index) const override
    {
        if (isValidChannel (index, true))
        {
            VstPinInfo pinProps;
            if (dispatch (plugInOpcodeGetInputPinProperties, index, 0, &pinProps, 0.0f) != 0)
                return String (pinProps.text, sizeof (pinProps.text));
        }

        return String();
    }

    bool isInputChannelStereoPair (int index) const override
    {
        if (! isValidChannel (index, true))
            return false;

        VstPinInfo pinProps;
        if (dispatch (plugInOpcodeGetInputPinProperties, index, 0, &pinProps, 0.0f) != 0)
            return (pinProps.flags & vstPinInfoFlagIsStereo) != 0;

        return true;
    }

    const String getOutputChannelName (int index) const override
    {
        if (isValidChannel (index, false))
        {
            VstPinInfo pinProps;
            if (dispatch (plugInOpcodeGetOutputPinProperties, index, 0, &pinProps, 0.0f) != 0)
                return String (pinProps.text, sizeof (pinProps.text));
        }

        return String();
    }

    bool isOutputChannelStereoPair (int index) const override
    {
        if (! isValidChannel (index, false))
            return false;

        VstPinInfo pinProps;
        if (dispatch (plugInOpcodeGetOutputPinProperties, index, 0, &pinProps, 0.0f) != 0)
            return (pinProps.flags & vstPinInfoFlagIsStereo) != 0;

        return true;
    }

    bool isValidChannel (int index, bool isInput) const noexcept
    {
        return isPositiveAndBelow (index, isInput ? getTotalNumInputChannels()
                                                  : getTotalNumOutputChannels());
    }

    //==============================================================================
    int getNumParameters() override      { return vstEffect != nullptr ? vstEffect->numParameters : 0; }

    float getParameter (int index) override
    {
        if (vstEffect != nullptr && isPositiveAndBelow (index, (int) vstEffect->numParameters))
        {
            const ScopedLock sl (lock);
            return vstEffect->getParameterValueFunction (vstEffect, index);
        }

        return 0.0f;
    }

    void setParameter (int index, float newValue) override
    {
        if (vstEffect != nullptr && isPositiveAndBelow (index, (int) vstEffect->numParameters))
        {
            const ScopedLock sl (lock);

            if (vstEffect->getParameterValueFunction (vstEffect, index) != newValue)
                vstEffect->setParameterValueFunction (vstEffect, index, newValue);
        }
    }

    const String getParameterName (int index) override       { return getTextForOpcode (index, plugInOpcodeGetParameterName); }
    const String getParameterText (int index) override       { return getTextForOpcode (index, plugInOpcodeGetParameterText); }
    String getParameterLabel (int index) const override      { return getTextForOpcode (index, plugInOpcodeGetParameterLabel); }

    bool isParameterAutomatable (int index) const override
    {
        if (vstEffect != nullptr)
        {
            jassert (index >= 0 && index < vstEffect->numParameters);
            return dispatch (plugInOpcodeIsParameterAutomatable, index, 0, 0, 0) != 0;
        }

        return false;
    }

    //==============================================================================
    int getNumPrograms() override          { return vstEffect != nullptr ? jmax (0, vstEffect->numPrograms) : 0; }

    // NB: some plugs return negative numbers from this function.
    int getCurrentProgram() override       { return (int) dispatch (plugInOpcodeGetCurrentProgram, 0, 0, 0, 0); }

    void setCurrentProgram (int newIndex) override
    {
        if (getNumPrograms() > 0 && newIndex != getCurrentProgram())
            dispatch (plugInOpcodeSetCurrentProgram, 0, jlimit (0, getNumPrograms() - 1, newIndex), 0, 0);
    }

    const String getProgramName (int index) override
    {
        if (index >= 0)
        {
            if (index == getCurrentProgram())
                return getCurrentProgramName();

            if (vstEffect != nullptr)
            {
                char nm[264] = { 0 };

                if (dispatch (plugInOpcodeGetProgramName, jlimit (0, getNumPrograms(), index), -1, nm, 0) != 0)
                    return String::fromUTF8 (nm).trim();
            }
        }

        return programNames [index];
    }

    void changeProgramName (int index, const String& newName) override
    {
        if (index >= 0 && index == getCurrentProgram())
        {
            if (getNumPrograms() > 0 && newName != getCurrentProgramName())
                dispatch (plugInOpcodeSetCurrentProgramName, 0, 0, (void*) newName.substring (0, 24).toRawUTF8(), 0.0f);
        }
        else
        {
            jassertfalse; // xxx not implemented!
        }
    }

    //==============================================================================
    void getStateInformation (MemoryBlock& mb) override                  { saveToFXBFile (mb, true); }
    void getCurrentProgramStateInformation (MemoryBlock& mb) override    { saveToFXBFile (mb, false); }

    void setStateInformation (const void* data, int size) override               { loadFromFXBFile (data, (size_t) size); }
    void setCurrentProgramStateInformation (const void* data, int size) override { loadFromFXBFile (data, (size_t) size); }

    //==============================================================================
    void timerCallback() override
    {
        if (dispatch (plugInOpcodeIdle, 0, 0, 0, 0) == 0)
            stopTimer();
    }

    void handleAsyncUpdate() override
    {
        // indicates that something about the plugin has changed..
        updateHostDisplay();
    }

    pointer_sized_int handleCallback (int32 opcode, int32 index, pointer_sized_int value, void* ptr, float opt)
    {
        switch (opcode)
        {
            case hostOpcodeParameterChanged:            sendParamChangeMessageToListeners (index, opt); break;
            case hostOpcodePreAudioProcessingEvents:    handleMidiFromPlugin ((const VstEventBlock*) ptr); break;
            case hostOpcodeGetTimingInfo:               return getVSTTime();
            case hostOpcodeIdle:                        handleIdle(); break;
            case hostOpcodeWindowSize:                  setWindowSize (index, (int) value); return 1;
            case hostOpcodeUpdateView:                  triggerAsyncUpdate(); break;
            case hostOpcodeIOModified:                  setLatencySamples (vstEffect->latency); break;
            case hostOpcodeNeedsIdle:                   startTimer (50); break;

            case hostOpcodeGetSampleRate:               return (pointer_sized_int) (getSampleRate() > 0 ? getSampleRate() : defaultVSTSampleRateValue);
            case hostOpcodeGetBlockSize:                return (pointer_sized_int) (getBlockSize() > 0  ? getBlockSize()  : defaultVSTBlockSizeValue);
            case hostOpcodePlugInWantsMidi:             wantsMidiMessages = true; break;
            case hostOpcodeGetDirectory:                return getVstDirectory();

            case hostOpcodeTempoAt:                     return (pointer_sized_int) (extraFunctions != nullptr ? extraFunctions->getTempoAt ((int64) value) : 0);
            case hostOpcodeGetAutomationState:          return (pointer_sized_int) (extraFunctions != nullptr ? extraFunctions->getAutomationState() : 0);

            case hostOpcodeParameterChangeGestureBegin: beginParameterChangeGesture (index); break;
            case hostOpcodeParameterChangeGestureEnd:   endParameterChangeGesture (index); break;

            case hostOpcodePinConnected:                    return isValidChannel (index, value == 0) ? 0 : 1; // (yes, 0 = true)
            case hostOpcodeGetCurrentAudioProcessingLevel:  return isNonRealtime() ? 4 : 0;

            // none of these are handled (yet)...
            case hostOpcodeSetTime:
            case hostOpcodeGetParameterInterval:
            case hostOpcodeGetInputLatency:
            case hostOpcodeGetOutputLatency:
            case hostOpcodeGetPreviousPlugIn:
            case hostOpcodeGetNextPlugIn:
            case hostOpcodeWillReplace:
            case hostOpcodeOfflineStart:
            case hostOpcodeOfflineReadSource:
            case hostOpcodeOfflineWrite:
            case hostOpcodeOfflineGetCurrentPass:
            case hostOpcodeOfflineGetCurrentMetaPass:
            case hostOpcodeGetOutputSpeakerConfiguration:
            case hostOpcodeManufacturerSpecific:
            case hostOpcodeSetIcon:
            case hostOpcodeGetLanguage:
            case hostOpcodeOpenEditorWindow:
            case hostOpcodeCloseEditorWindow:
                break;

            default:
                return handleGeneralCallback (opcode, index, value, ptr, opt);
        }

        return 0;
    }

    // handles non plugin-specific callbacks..
    static pointer_sized_int handleGeneralCallback (int32 opcode, int32 /*index*/, pointer_sized_int /*value*/, void* ptr, float /*opt*/)
    {
        switch (opcode)
        {
            case hostOpcodeCanHostDo:                         return handleCanDo ((const char*) ptr);
            case hostOpcodeVstVersion:                        return 2400;
            case hostOpcodeCurrentId:                         return shellUIDToCreate;
            case hostOpcodeGetNumberOfAutomatableParameters:  return 0;
            case hostOpcodeGetAutomationState:                return 1;
            case hostOpcodeGetManufacturerVersion:            return 0x0101;

            case hostOpcodeGetManufacturerName:
            case hostOpcodeGetProductName:                    return getHostName ((char*) ptr);

            case hostOpcodeGetSampleRate:                     return (pointer_sized_int) defaultVSTSampleRateValue;
            case hostOpcodeGetBlockSize:                      return (pointer_sized_int) defaultVSTBlockSizeValue;
            case hostOpcodeSetOutputSampleRate:               return 0;

            default:
                DBG ("*** Unhandled VST Callback: " + String ((int) opcode));
                break;
        }

        return 0;
    }

    //==============================================================================
    pointer_sized_int dispatch (const int opcode, const int index, const pointer_sized_int value, void* const ptr, float opt) const
    {
        pointer_sized_int result = 0;

        if (vstEffect != nullptr)
        {
            const ScopedLock sl (lock);
            const IdleCallRecursionPreventer icrp;

            try
            {
               #if JUCE_MAC
                const ResFileRefNum oldResFile = CurResFile();

                if (vstModule->resFileId != 0)
                    UseResFile (vstModule->resFileId);
               #endif

                result = vstEffect->dispatchFunction (vstEffect, opcode, index, value, ptr, opt);

               #if JUCE_MAC
                const ResFileRefNum newResFile = CurResFile();
                if (newResFile != oldResFile)  // avoid confusing the parent app's resource file with the plug-in's
                {
                    vstModule->resFileId = newResFile;
                    UseResFile (oldResFile);
                }
               #endif
            }
            catch (...)
            {}
        }

        return result;
    }

    bool loadFromFXBFile (const void* const data, const size_t dataSize)
    {
        if (dataSize < 28)
            return false;

        const fxSet* const set = (const fxSet*) data;

        if ((! compareMagic (set->chunkMagic, "CcnK")) || fxbSwap (set->version) > fxbVersionNum)
            return false;

        if (compareMagic (set->fxMagic, "FxBk"))
        {
            // bank of programs
            if (fxbSwap (set->numPrograms) >= 0)
            {
                const int oldProg = getCurrentProgram();
                const int numParams = fxbSwap (((const fxProgram*) (set->programs))->numParams);
                const int progLen = (int) sizeof (fxProgram) + (numParams - 1) * (int) sizeof (float);

                for (int i = 0; i < fxbSwap (set->numPrograms); ++i)
                {
                    if (i != oldProg)
                    {
                        const fxProgram* const prog = (const fxProgram*) (((const char*) (set->programs)) + i * progLen);
                        if (((const char*) prog) - ((const char*) set) >= (ssize_t) dataSize)
                            return false;

                        if (fxbSwap (set->numPrograms) > 0)
                            setCurrentProgram (i);

                        if (! restoreProgramSettings (prog))
                            return false;
                    }
                }

                if (fxbSwap (set->numPrograms) > 0)
                    setCurrentProgram (oldProg);

                const fxProgram* const prog = (const fxProgram*) (((const char*) (set->programs)) + oldProg * progLen);
                if (((const char*) prog) - ((const char*) set) >= (ssize_t) dataSize)
                    return false;

                if (! restoreProgramSettings (prog))
                    return false;
            }
        }
        else if (compareMagic (set->fxMagic, "FxCk"))
        {
            // single program
            const fxProgram* const prog = (const fxProgram*) data;

            if (! compareMagic (prog->chunkMagic, "CcnK"))
                return false;

            changeProgramName (getCurrentProgram(), prog->prgName);

            for (int i = 0; i < fxbSwap (prog->numParams); ++i)
                setParameter (i, fxbSwapFloat (prog->params[i]));
        }
        else if (compareMagic (set->fxMagic, "FBCh"))
        {
            // non-preset chunk
            const fxChunkSet* const cset = (const fxChunkSet*) data;

            if ((size_t) fxbSwap (cset->chunkSize) + sizeof (fxChunkSet) - 8 > (size_t) dataSize)
                return false;

            setChunkData (cset->chunk, fxbSwap (cset->chunkSize), false);
        }
        else if (compareMagic (set->fxMagic, "FPCh"))
        {
            // preset chunk
            const fxProgramSet* const cset = (const fxProgramSet*) data;

            if ((size_t) fxbSwap (cset->chunkSize) + sizeof (fxProgramSet) - 8 > (size_t) dataSize)
                return false;

            setChunkData (cset->chunk, fxbSwap (cset->chunkSize), true);

            changeProgramName (getCurrentProgram(), cset->name);
        }
        else
        {
            return false;
        }

        return true;
    }

    bool saveToFXBFile (MemoryBlock& dest, bool isFXB, int maxSizeMB = 128)
    {
        const int numPrograms = getNumPrograms();
        const int numParams = getNumParameters();

        if (usesChunks())
        {
            MemoryBlock chunk;
            getChunkData (chunk, ! isFXB, maxSizeMB);

            if (isFXB)
            {
                const size_t totalLen = sizeof (fxChunkSet) + chunk.getSize() - 8;
                dest.setSize (totalLen, true);

                fxChunkSet* const set = (fxChunkSet*) dest.getData();
                set->chunkMagic = fxbName ("CcnK");
                set->byteSize = 0;
                set->fxMagic = fxbName ("FBCh");
                set->version = fxbSwap (fxbVersionNum);
                set->fxID = fxbSwap (getUID());
                set->fxVersion = fxbSwap (getVersionNumber());
                set->numPrograms = fxbSwap (numPrograms);
                set->chunkSize = fxbSwap ((int32) chunk.getSize());

                chunk.copyTo (set->chunk, 0, chunk.getSize());
            }
            else
            {
                const size_t totalLen = sizeof (fxProgramSet) + chunk.getSize() - 8;
                dest.setSize (totalLen, true);

                fxProgramSet* const set = (fxProgramSet*) dest.getData();
                set->chunkMagic = fxbName ("CcnK");
                set->byteSize = 0;
                set->fxMagic = fxbName ("FPCh");
                set->version = fxbSwap (fxbVersionNum);
                set->fxID = fxbSwap (getUID());
                set->fxVersion = fxbSwap (getVersionNumber());
                set->numPrograms = fxbSwap (numPrograms);
                set->chunkSize = fxbSwap ((int32) chunk.getSize());

                getCurrentProgramName().copyToUTF8 (set->name, sizeof (set->name) - 1);
                chunk.copyTo (set->chunk, 0, chunk.getSize());
            }
        }
        else
        {
            if (isFXB)
            {
                const int progLen = (int) sizeof (fxProgram) + (numParams - 1) * (int) sizeof (float);
                const size_t len = (sizeof (fxSet) - sizeof (fxProgram)) + (size_t) (progLen * jmax (1, numPrograms));
                dest.setSize (len, true);

                fxSet* const set = (fxSet*) dest.getData();
                set->chunkMagic = fxbName ("CcnK");
                set->byteSize = 0;
                set->fxMagic = fxbName ("FxBk");
                set->version = fxbSwap (fxbVersionNum);
                set->fxID = fxbSwap (getUID());
                set->fxVersion = fxbSwap (getVersionNumber());
                set->numPrograms = fxbSwap (numPrograms);

                MemoryBlock oldSettings;
                createTempParameterStore (oldSettings);

                const int oldProgram = getCurrentProgram();

                if (oldProgram >= 0)
                    setParamsInProgramBlock ((fxProgram*) (((char*) (set->programs)) + oldProgram * progLen));

                for (int i = 0; i < numPrograms; ++i)
                {
                    if (i != oldProgram)
                    {
                        setCurrentProgram (i);
                        setParamsInProgramBlock ((fxProgram*) (((char*) (set->programs)) + i * progLen));
                    }
                }

                if (oldProgram >= 0)
                    setCurrentProgram (oldProgram);

                restoreFromTempParameterStore (oldSettings);
            }
            else
            {
                dest.setSize (sizeof (fxProgram) + (size_t) ((numParams - 1) * (int) sizeof (float)), true);
                setParamsInProgramBlock ((fxProgram*) dest.getData());
            }
        }

        return true;
    }

    bool usesChunks() const noexcept        { return vstEffect != nullptr && (vstEffect->flags & vstEffectFlagDataInChunks) != 0; }

    bool getChunkData (MemoryBlock& mb, bool isPreset, int maxSizeMB) const
    {
        if (usesChunks())
        {
            void* data = nullptr;
            const size_t bytes = (size_t) dispatch (plugInOpcodeGetData, isPreset ? 1 : 0, 0, &data, 0.0f);

            if (data != nullptr && bytes <= (size_t) maxSizeMB * 1024 * 1024)
            {
                mb.setSize (bytes);
                mb.copyFrom (data, 0, bytes);

                return true;
            }
        }

        return false;
    }

    bool setChunkData (const void* data, const int size, bool isPreset)
    {
        if (size > 0 && usesChunks())
        {
            dispatch (plugInOpcodeSetData, isPreset ? 1 : 0, size, (void*) data, 0.0f);

            if (! isPreset)
                updateStoredProgramNames();

            return true;
        }

        return false;
    }

    VstEffectInterface* vstEffect;
    ModuleHandle::Ptr vstModule;

    ScopedPointer<VSTPluginFormat::ExtraFunctions> extraFunctions;
    bool usesCocoaNSView;

private:
    String name;
    CriticalSection lock;
    bool wantsMidiMessages, initialised, isPowerOn;
    mutable StringArray programNames;
    AudioBuffer<float> tempBuffer;
    CriticalSection midiInLock;
    MidiBuffer incomingMidi;
    VSTMidiEventList midiEventsToSend;
    VstTimingInformation vstHostTime;

    static pointer_sized_int handleCanDo (const char* name)
    {
        static const char* canDos[] = { "supplyIdle",
                                        "sendVstEvents",
                                        "sendVstMidiEvent",
                                        "sendVstTimeInfo",
                                        "receiveVstEvents",
                                        "receiveVstMidiEvent",
                                        "supportShell",
                                        "sizeWindow",
                                        "shellCategory" };

        for (int i = 0; i < numElementsInArray (canDos); ++i)
            if (strcmp (canDos[i], name) == 0)
                return 1;

        return 0;
    }

    static pointer_sized_int getHostName (char* name)
    {
        String hostName ("Juce VST Host");

        if (JUCEApplicationBase* app = JUCEApplicationBase::getInstance())
            hostName = app->getApplicationName();

        hostName.copyToUTF8 (name, (size_t) jmin (vstMaxManufacturerStringLength, vstMaxPlugInNameStringLength) - 1);
        return 1;
    }

    pointer_sized_int getVSTTime() noexcept
    {
       #if JUCE_MSVC
        #pragma warning (push)
        #pragma warning (disable: 4311)
       #endif

        return (pointer_sized_int) &vstHostTime;

       #if JUCE_MSVC
        #pragma warning (pop)
       #endif
    }

    void handleIdle()
    {
        if (insideVSTCallback == 0 && MessageManager::getInstance()->isThisTheMessageThread())
        {
            const IdleCallRecursionPreventer icrp;

           #if JUCE_MAC
            if (getActiveEditor() != nullptr)
                dispatch (plugInOpcodeEditorIdle, 0, 0, 0, 0);
           #endif

            Timer::callPendingTimersSynchronously();
            handleUpdateNowIfNeeded();

            for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
                if (ComponentPeer* p = ComponentPeer::getPeer(i))
                    p->performAnyPendingRepaintsNow();
        }
    }

    void setWindowSize (int width, int height)
    {
        if (AudioProcessorEditor* ed = getActiveEditor())
        {
           #if JUCE_LINUX
            const MessageManagerLock mmLock;
           #endif
            ed->setSize (width, height);
        }
    }

    //==============================================================================
    static VstEffectInterface* constructEffect (const ModuleHandle::Ptr& module)
    {
        VstEffectInterface* effect = nullptr;
        try
        {
            const IdleCallRecursionPreventer icrp;
            _fpreset();

            JUCE_VST_LOG ("Creating VST instance: " + module->pluginName);

           #if JUCE_MAC
            if (module->resFileId != 0)
                UseResFile (module->resFileId);
           #endif

            {
                JUCE_VST_WRAPPER_INVOKE_MAIN
            }

            if (effect != nullptr && effect->interfaceIdentifier == juceVstInterfaceIdentifier)
            {
                jassert (effect->hostSpace2 == 0);
                jassert (effect->effectPointer != 0);

                _fpreset(); // some dodgy plugs mess around with this
            }
            else
            {
                effect = nullptr;
            }
        }
        catch (...)
        {}

        return effect;
    }

    static BusesProperties queryBusIO (VstEffectInterface* effect)
    {
        BusesProperties returnValue;

        VstSpeakerConfiguration* inArr = nullptr, *outArr = nullptr;
        if (effect->dispatchFunction (effect, plugInOpcodeGetSpeakerArrangement, 0, reinterpret_cast<pointer_sized_int> (&inArr), &outArr, 0.0f) == 0)
            inArr = outArr = nullptr;

        for (int dir = 0; dir < 2; ++dir)
        {
            const bool isInput = (dir == 0);
            const int opcode = (isInput ? plugInOpcodeGetInputPinProperties : plugInOpcodeGetOutputPinProperties);
            const int maxChannels = (isInput ? effect->numInputChannels : effect->numOutputChannels);
            const VstSpeakerConfiguration* arr = (isInput ? inArr : outArr);
            bool busAdded = false;

            VstPinInfo pinProps;
            AudioChannelSet layout;
            for (int ch = 0; ch < maxChannels; ch += layout.size())
            {
                if (effect->dispatchFunction (effect, opcode, ch, 0, &pinProps, 0.0f) == 0)
                    break;

                if ((pinProps.flags & vstPinInfoFlagValid) != 0)
                {
                    layout = SpeakerMappings::vstArrangementTypeToChannelSet (pinProps.configurationType, 0);
                    if (layout.isDisabled())
                        break;
                }
                else
                {
                    layout = ((pinProps.flags & vstPinInfoFlagIsStereo) != 0 ? AudioChannelSet::stereo() : AudioChannelSet::mono());
                }

                busAdded = true;
                returnValue.addBus (isInput, pinProps.text, layout, true);
            }

            // no buses?
            if (! busAdded && maxChannels > 0)
            {
                String busName = (isInput ? "Input" : "Output");
                if (effect->dispatchFunction (effect, opcode, 0, 0, &pinProps, 0.0f) != 0)
                    busName = pinProps.text;

                if (arr != nullptr)
                    layout = SpeakerMappings::vstArrangementTypeToChannelSet (*arr);
                else
                    layout = AudioChannelSet::canonicalChannelSet (maxChannels);

                returnValue.addBus (isInput, busName, layout, true);
            }
        }

        return returnValue;
    }

    //==============================================================================
    template <typename FloatType>
    void processAudio (AudioBuffer<FloatType>& buffer, MidiBuffer& midiMessages)
    {
        const int numSamples = buffer.getNumSamples();

        if (initialised)
        {
            if (AudioPlayHead* const currentPlayHead = getPlayHead())
            {
                AudioPlayHead::CurrentPositionInfo position;

                if (currentPlayHead->getCurrentPosition (position))
                {

                    vstHostTime.samplePosition           = (double) position.timeInSamples;
                    vstHostTime.tempoBPM                 = position.bpm;
                    vstHostTime.timeSignatureNumerator   = position.timeSigNumerator;
                    vstHostTime.timeSignatureDenominator = position.timeSigDenominator;
                    vstHostTime.musicalPosition          = position.ppqPosition;
                    vstHostTime.lastBarPosition          = position.ppqPositionOfLastBarStart;
                    vstHostTime.flags |= vstTimingInfoFlagTempoValid
                                           | vstTimingInfoFlagTimeSignatureValid
                                           | vstTimingInfoFlagMusicalPositionValid
                                           | vstTimingInfoFlagLastBarPositionValid;

                    int32 newTransportFlags = 0;
                    if (position.isPlaying)     newTransportFlags |= vstTimingInfoFlagCurrentlyPlaying;
                    if (position.isRecording)   newTransportFlags |= vstTimingInfoFlagCurrentlyRecording;

                    if (newTransportFlags != (vstHostTime.flags & (vstTimingInfoFlagCurrentlyPlaying
                                                                   | vstTimingInfoFlagCurrentlyRecording)))
                        vstHostTime.flags = (vstHostTime.flags & ~(vstTimingInfoFlagCurrentlyPlaying | vstTimingInfoFlagCurrentlyRecording)) | newTransportFlags | vstTimingInfoFlagTransportChanged;
                    else
                        vstHostTime.flags &= ~vstTimingInfoFlagTransportChanged;

                    switch (position.frameRate)
                    {
                        case AudioPlayHead::fps24:       setHostTimeFrameRate (0, 24.0,  position.timeInSeconds); break;
                        case AudioPlayHead::fps25:       setHostTimeFrameRate (1, 25.0,  position.timeInSeconds); break;
                        case AudioPlayHead::fps2997:     setHostTimeFrameRate (2, 29.97, position.timeInSeconds); break;
                        case AudioPlayHead::fps30:       setHostTimeFrameRate (3, 30.0,  position.timeInSeconds); break;
                        case AudioPlayHead::fps2997drop: setHostTimeFrameRate (4, 29.97, position.timeInSeconds); break;
                        case AudioPlayHead::fps30drop:   setHostTimeFrameRate (5, 29.97, position.timeInSeconds); break;
                        default: break;
                    }

                    if (position.isLooping)
                    {
                        vstHostTime.loopStartPosition = position.ppqLoopStart;
                        vstHostTime.loopEndPosition   = position.ppqLoopEnd;
                        vstHostTime.flags |= (vstTimingInfoFlagLoopPositionValid | vstTimingInfoFlagLoopActive);
                    }
                    else
                    {
                        vstHostTime.flags &= ~(vstTimingInfoFlagLoopPositionValid | vstTimingInfoFlagLoopActive);
                    }
                }
            }

            vstHostTime.systemTimeNanoseconds = getVSTHostTimeNanoseconds();

            if (wantsMidiMessages)
            {
                midiEventsToSend.clear();
                midiEventsToSend.ensureSize (1);

                MidiBuffer::Iterator iter (midiMessages);
                const uint8* midiData;
                int numBytesOfMidiData, samplePosition;

                while (iter.getNextEvent (midiData, numBytesOfMidiData, samplePosition))
                {
                    midiEventsToSend.addEvent (midiData, numBytesOfMidiData,
                                               jlimit (0, numSamples - 1, samplePosition));
                }

                vstEffect->dispatchFunction (vstEffect, plugInOpcodePreAudioProcessingEvents, 0, 0, midiEventsToSend.events, 0);
            }

            _clearfp();

            invokeProcessFunction (buffer, numSamples);
        }
        else
        {
            // Not initialised, so just bypass..
            for (int i = getTotalNumOutputChannels(); --i >= 0;)
                buffer.clear (i, 0, buffer.getNumSamples());
        }

        {
            // copy any incoming midi..
            const ScopedLock sl (midiInLock);

            midiMessages.swapWith (incomingMidi);
            incomingMidi.clear();
        }
    }

    //==============================================================================
    inline void invokeProcessFunction (AudioBuffer<float>& buffer, int32 sampleFrames)
    {
        if ((vstEffect->flags & vstEffectFlagInplaceAudio) != 0)
        {
            vstEffect->processAudioInplaceFunction (vstEffect, buffer.getArrayOfWritePointers(), buffer.getArrayOfWritePointers(), sampleFrames);
        }
        else
        {
            tempBuffer.setSize (vstEffect->numOutputChannels, sampleFrames);
            tempBuffer.clear();

            vstEffect->processAudioFunction (vstEffect, buffer.getArrayOfWritePointers(), tempBuffer.getArrayOfWritePointers(), sampleFrames);

            for (int i = vstEffect->numOutputChannels; --i >= 0;)
                buffer.copyFrom (i, 0, tempBuffer.getReadPointer (i), sampleFrames);
        }
    }

    inline void invokeProcessFunction (AudioBuffer<double>& buffer, int32 sampleFrames)
    {
        vstEffect->processDoubleAudioInplaceFunction (vstEffect, buffer.getArrayOfWritePointers(), buffer.getArrayOfWritePointers(), sampleFrames);
    }

    //==============================================================================
    void setHostTimeFrameRate (long frameRateIndex, double frameRate, double currentTime) noexcept
    {
        vstHostTime.flags |= vstTimingInfoFlagSmpteValid;
        vstHostTime.smpteRate       = (int32) frameRateIndex;
        vstHostTime.smpteOffset     = (int32) (currentTime * 80.0 * frameRate + 0.5);
    }

    bool restoreProgramSettings (const fxProgram* const prog)
    {
        if (compareMagic (prog->chunkMagic, "CcnK")
             && compareMagic (prog->fxMagic, "FxCk"))
        {
            changeProgramName (getCurrentProgram(), prog->prgName);

            for (int i = 0; i < fxbSwap (prog->numParams); ++i)
                setParameter (i, fxbSwapFloat (prog->params[i]));

            return true;
        }

        return false;
    }

    String getTextForOpcode (const int index, const VstHostToPlugInOpcodes opcode) const
    {
        if (vstEffect == nullptr)
            return String();

        jassert (index >= 0 && index < vstEffect->numParameters);
        char nm[256] = { 0 };
        dispatch (opcode, index, 0, nm, 0);
        return String::createStringFromData (nm, (int) sizeof (nm)).trim();
    }

    String getCurrentProgramName()
    {
        String progName;

        if (vstEffect != nullptr)
        {
            {
                char nm[256] = { 0 };
                dispatch (plugInOpcodeGetCurrentProgramName, 0, 0, nm, 0);
                progName = String::createStringFromData (nm, (int) sizeof (nm)).trim();
            }

            const int index = getCurrentProgram();

            if (index >= 0 && programNames[index].isEmpty())
            {
                while (programNames.size() < index)
                    programNames.add (String());

                programNames.set (index, progName);
            }
        }

        return progName;
    }

    void setParamsInProgramBlock (fxProgram* const prog)
    {
        const int numParams = getNumParameters();

        prog->chunkMagic = fxbName ("CcnK");
        prog->byteSize = 0;
        prog->fxMagic = fxbName ("FxCk");
        prog->version = fxbSwap (fxbVersionNum);
        prog->fxID = fxbSwap (getUID());
        prog->fxVersion = fxbSwap (getVersionNumber());
        prog->numParams = fxbSwap (numParams);

        getCurrentProgramName().copyToUTF8 (prog->prgName, sizeof (prog->prgName) - 1);

        for (int i = 0; i < numParams; ++i)
            prog->params[i] = fxbSwapFloat (getParameter (i));
    }

    void updateStoredProgramNames()
    {
        if (vstEffect != nullptr && getNumPrograms() > 0)
        {
            char nm[256] = { 0 };

            // only do this if the plugin can't use indexed names..
            if (dispatch (plugInOpcodeGetProgramName, 0, -1, nm, 0) == 0)
            {
                const int oldProgram = getCurrentProgram();
                MemoryBlock oldSettings;
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

    void handleMidiFromPlugin (const VstEventBlock* const events)
    {
        if (events != nullptr)
        {
            const ScopedLock sl (midiInLock);
            VSTMidiEventList::addEventsToMidiBuffer (events, incomingMidi);
        }
    }

    //==============================================================================
    void createTempParameterStore (MemoryBlock& dest)
    {
        dest.setSize (64 + 4 * (size_t) getNumParameters());
        dest.fillWith (0);

        getCurrentProgramName().copyToUTF8 ((char*) dest.getData(), 63);

        float* const p = (float*) (((char*) dest.getData()) + 64);
        for (int i = 0; i < getNumParameters(); ++i)
            p[i] = getParameter(i);
    }

    void restoreFromTempParameterStore (const MemoryBlock& m)
    {
        changeProgramName (getCurrentProgram(), (const char*) m.getData());

        float* p = (float*) (((char*) m.getData()) + 64);
        for (int i = 0; i < getNumParameters(); ++i)
            setParameter (i, p[i]);
    }

    pointer_sized_int getVstDirectory() const
    {
       #if JUCE_MAC
        return (pointer_sized_int) (void*) &vstModule->parentDirFSSpec;
       #else
        return (pointer_sized_int) (pointer_sized_uint) vstModule->fullParentDirectoryPathName.toRawUTF8();
       #endif
    }

    //==============================================================================
    int getVersionNumber() const noexcept   { return vstEffect != nullptr ? vstEffect->plugInVersion : 0; }

    String getVersion() const
    {
        unsigned int v = (unsigned int) dispatch (plugInOpcodeGetManufacturerVersion, 0, 0, 0, 0);

        String s;

        if (v == 0 || (int) v == -1)
            v = (unsigned int) getVersionNumber();

        if (v != 0)
        {
            int versionBits[32];
            int n = 0;

            for (unsigned int vv = v; vv != 0; vv /= 10)
                versionBits [n++] = vv % 10;

            if (n > 4) // if the number ends up silly, it's probably encoded as hex instead of decimal..
            {
                n = 0;

                for (unsigned int vv = v; vv != 0; vv >>= 8)
                    versionBits [n++] = vv & 255;
            }

            while (n > 1 && versionBits [n - 1] == 0)
                --n;

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

    const char* getCategory() const
    {
        switch (getVstCategory())
        {
            case kPlugCategEffect:       return "Effect";
            case kPlugCategSynth:        return "Synth";
            case kPlugCategAnalysis:     return "Analysis";
            case kPlugCategMastering:    return "Mastering";
            case kPlugCategSpacializer:  return "Spacial";
            case kPlugCategRoomFx:       return "Reverb";
            case kPlugSurroundFx:        return "Surround";
            case kPlugCategRestoration:  return "Restoration";
            case kPlugCategGenerator:    return "Tone generation";
            default:                     break;
        }

        return nullptr;
    }

    void setPower (const bool on)
    {
        dispatch (plugInOpcodeResumeSuspend, 0, on ? 1 : 0, 0, 0);
        isPowerOn = on;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VSTPluginInstance)
};

//==============================================================================
#if ! JUCE_IOS
class VSTPluginWindow;
static Array<VSTPluginWindow*> activeVSTWindows;

//==============================================================================
class VSTPluginWindow   : public AudioProcessorEditor,
                         #if ! JUCE_MAC
                          public ComponentMovementWatcher,
                         #endif
                          public Timer
{
public:
    VSTPluginWindow (VSTPluginInstance& plug)
        : AudioProcessorEditor (&plug),
         #if ! JUCE_MAC
          ComponentMovementWatcher (this),
         #endif
          plugin (plug),
          isOpen (false),
          recursiveResize (false),
          pluginWantsKeys (false),
          pluginRefusesToResize (false),
          alreadyInside (false)
    {
       #if JUCE_WINDOWS
        pluginHWND = 0;
        sizeCheckCount = 0;

       #elif JUCE_LINUX
        pluginWindow = None;
        pluginProc = None;

       #elif JUCE_MAC
        ignoreUnused (recursiveResize, pluginRefusesToResize, alreadyInside);

        #if JUCE_SUPPORT_CARBON
        if (! plug.usesCocoaNSView)
            addAndMakeVisible (carbonWrapper = new CarbonWrapperComponent (*this));
        else
        #endif
            addAndMakeVisible (cocoaWrapper = new AutoResizingNSViewComponentWithParent());
       #endif

        activeVSTWindows.add (this);

        setSize (1, 1);
        setOpaque (true);
        setVisible (true);
    }

    ~VSTPluginWindow()
    {
        closePluginWindow();

       #if JUCE_MAC
        #if JUCE_SUPPORT_CARBON
        carbonWrapper = nullptr;
        #endif
        cocoaWrapper = nullptr;
       #endif

        activeVSTWindows.removeFirstMatchingValue (this);
        plugin.editorBeingDeleted (this);
    }

    //==============================================================================
   #if ! JUCE_MAC
    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        if (recursiveResize)
            return;

        Component* const topComp = getTopLevelComponent();

        if (topComp->getPeer() != nullptr)
        {
            const Point<int> pos (topComp->getLocalPoint (this, Point<int>()));

            recursiveResize = true;

           #if JUCE_WINDOWS
            if (pluginHWND != 0)
                MoveWindow (pluginHWND, pos.getX(), pos.getY(), getWidth(), getHeight(), TRUE);
           #elif JUCE_LINUX
            if (pluginWindow != 0)
            {
                XMoveResizeWindow (display, pluginWindow,
                                   pos.getX(), pos.getY(),
                                   (unsigned int) getWidth(),
                                   (unsigned int) getHeight());

                XMapRaised (display, pluginWindow);
                XFlush (display);
            }
           #endif

            recursiveResize = false;
        }
    }

    void componentVisibilityChanged() override
    {
        if (isShowing())
            openPluginWindow();
        else if (! shouldAvoidDeletingWindow())
            closePluginWindow();

        componentMovedOrResized (true, true);
    }

    void componentPeerChanged() override
    {
        closePluginWindow();
        openPluginWindow();
    }
   #endif

   #if JUCE_MAC
    void visibilityChanged() override
    {
        if (cocoaWrapper != nullptr)
        {
            if (isVisible())
                openPluginWindow ((NSView*) cocoaWrapper->getView());
            else
                closePluginWindow();
        }
    }

    void childBoundsChanged (Component*) override
    {
        if (cocoaWrapper != nullptr)
        {
            int w = cocoaWrapper->getWidth();
            int h = cocoaWrapper->getHeight();

            if (w != getWidth() || h != getHeight())
                setSize (w, h);
        }
    }
   #endif

    //==============================================================================
    bool keyStateChanged (bool) override                 { return pluginWantsKeys; }
    bool keyPressed (const juce::KeyPress&) override     { return pluginWantsKeys; }

    //==============================================================================
   #if JUCE_MAC
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }
   #else
    void paint (Graphics& g) override
    {
        if (isOpen)
        {
           #if JUCE_LINUX
            if (pluginWindow != 0)
            {
                const Rectangle<int> clip (g.getClipBounds());

                XEvent ev = { 0 };
                ev.xexpose.type = Expose;
                ev.xexpose.display = display;
                ev.xexpose.window = pluginWindow;
                ev.xexpose.x = clip.getX();
                ev.xexpose.y = clip.getY();
                ev.xexpose.width = clip.getWidth();
                ev.xexpose.height = clip.getHeight();

                sendEventToChild (ev);
            }
           #endif
        }
        else
        {
            g.fillAll (Colours::black);
        }
    }
   #endif

    //==============================================================================
    void timerCallback() override
    {
        if (isShowing())
        {
           #if JUCE_WINDOWS
            if (--sizeCheckCount <= 0)
            {
                sizeCheckCount = 10;
                checkPluginWindowSize();
            }
           #endif

            static bool reentrantGuard = false;

            if (! reentrantGuard)
            {
                reentrantGuard = true;
                plugin.dispatch (plugInOpcodeEditorIdle, 0, 0, 0, 0);
                reentrantGuard = false;
            }

           #if JUCE_LINUX
            if (pluginWindow == 0)
            {
                updatePluginWindowHandle();

                if (pluginWindow != 0)
                    componentMovedOrResized (true, true);
            }
           #endif
        }
    }

    //==============================================================================
    void mouseDown (const MouseEvent& e) override
    {
        ignoreUnused (e);

       #if JUCE_LINUX
        if (pluginWindow == 0)
            return;

        toFront (true);

        XEvent ev;
        prepareXEvent (ev, e);
        ev.xbutton.type = ButtonPress;
        translateJuceToXButtonModifiers (e, ev);
        sendEventToChild (ev);

       #elif JUCE_WINDOWS
        toFront (true);
       #endif
    }

    void broughtToFront() override
    {
        activeVSTWindows.removeFirstMatchingValue (this);
        activeVSTWindows.add (this);

       #if JUCE_MAC
        dispatch (plugInOpcodeeffEditorTop, 0, 0, 0, 0);
       #endif
    }

    //==============================================================================
private:
    VSTPluginInstance& plugin;
    bool isOpen, recursiveResize;
    bool pluginWantsKeys, pluginRefusesToResize, alreadyInside;

   #if JUCE_WINDOWS
    HWND pluginHWND;
    void* originalWndProc;
    int sizeCheckCount;
   #elif JUCE_LINUX
    Window pluginWindow;
    EventProcPtr pluginProc;
   #endif

    // This is a workaround for old Mackie plugins that crash if their
    // window is deleted more than once.
    bool shouldAvoidDeletingWindow() const
    {
        return plugin.getPluginDescription()
            .manufacturerName.containsIgnoreCase ("Loud Technologies");
    }

    // This is an old workaround for some plugins that need a repaint when their
    // windows are first created, but it breaks some Izotope plugins..
    bool shouldRepaintCarbonWindowWhenCreated()
    {
        return ! plugin.getName().containsIgnoreCase ("izotope");
    }

    //==============================================================================
#if JUCE_MAC
    void openPluginWindow (void* parentWindow)
    {
        if (isOpen || parentWindow == 0)
            return;

        isOpen = true;

        VstEditorBounds* rect = nullptr;
        dispatch (plugInOpcodeGetEditorBounds, 0, 0, &rect, 0);
        dispatch (plugInOpcodeOpenEditor, 0, 0, parentWindow, 0);

        // do this before and after like in the steinberg example
        dispatch (plugInOpcodeGetEditorBounds, 0, 0, &rect, 0);
        dispatch (plugInOpcodeGetCurrentProgram, 0, 0, 0, 0); // also in steinberg code

        // Install keyboard hooks
        pluginWantsKeys = (dispatch (plugInOpcodeKeyboardFocusRequired, 0, 0, 0, 0) == 0);

        // double-check it's not too tiny
        int w = 250, h = 150;

        if (rect != nullptr)
        {
            w = rect->rightmost - rect->leftmost;
            h = rect->lower - rect->upper;

            if (w == 0 || h == 0)
            {
                w = 250;
                h = 150;
            }
        }

        w = jmax (w, 32);
        h = jmax (h, 32);

        setSize (w, h);

        startTimer (18 + juce::Random::getSystemRandom().nextInt (5));
        repaint();
    }

#else
    void openPluginWindow()
    {
        if (isOpen || getWindowHandle() == 0)
            return;

        JUCE_VST_LOG ("Opening VST UI: " + plugin.getName());
        isOpen = true;

        VstEditorBounds* rect = nullptr;
        dispatch (plugInOpcodeGetEditorBounds, 0, 0, &rect, 0);
        dispatch (plugInOpcodeOpenEditor, 0, 0, getWindowHandle(), 0);

        // do this before and after like in the steinberg example
        dispatch (plugInOpcodeGetEditorBounds, 0, 0, &rect, 0);
        dispatch (plugInOpcodeGetCurrentProgram, 0, 0, 0, 0); // also in steinberg code

        // Install keyboard hooks
        pluginWantsKeys = (dispatch (plugInOpcodeKeyboardFocusRequired, 0, 0, 0, 0) == 0);

       #if JUCE_WINDOWS
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

        if (! pluginWantsKeys)
        {
            originalWndProc = (void*) GetWindowLongPtr (pluginHWND, GWLP_WNDPROC);
            SetWindowLongPtr (pluginHWND, GWLP_WNDPROC, (LONG_PTR) vstHookWndProc);
        }

        #pragma warning (pop)

        RECT r;
        GetWindowRect (pluginHWND, &r);
        int w = r.right - r.left;
        int h = r.bottom - r.top;

        if (rect != nullptr)
        {
            const int rw = rect->rightmost - rect->leftmost;
            const int rh = rect->lower - rect->upper;

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

       #elif JUCE_LINUX
        updatePluginWindowHandle();

        int w = 250, h = 150;

        if (rect != nullptr)
        {
            w = rect->rightmost - rect->leftmost;
            h = rect->lower - rect->upper;

            if (w == 0 || h == 0)
            {
                w = 250;
                h = 150;
            }
        }

        if (pluginWindow != 0)
            XMapRaised (display, pluginWindow);
       #endif

        // double-check it's not too tiny
        w = jmax (w, 32);
        h = jmax (h, 32);

        setSize (w, h);

       #if JUCE_WINDOWS
        checkPluginWindowSize();
       #endif

        startTimer (18 + juce::Random::getSystemRandom().nextInt (5));
        repaint();
    }
#endif

    //==============================================================================
    void closePluginWindow()
    {
        if (isOpen)
        {
            // You shouldn't end up hitting this assertion unless the host is trying to do GUI
            // cleanup on a non-GUI thread.. If it does that, bad things could happen in here..
            jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

            JUCE_VST_LOG ("Closing VST UI: " + plugin.getName());
            isOpen = false;
            dispatch (plugInOpcodeCloseEditor, 0, 0, 0, 0);
            stopTimer();

           #if JUCE_WINDOWS
            #pragma warning (push)
            #pragma warning (disable: 4244)
            if (originalWndProc != 0 && pluginHWND != 0 && IsWindow (pluginHWND))
                SetWindowLongPtr (pluginHWND, GWLP_WNDPROC, (LONG_PTR) originalWndProc);
            #pragma warning (pop)

            originalWndProc = 0;
            pluginHWND = 0;
           #elif JUCE_LINUX
            pluginWindow = 0;
            pluginProc = 0;
           #endif
        }
    }

    //==============================================================================
    pointer_sized_int dispatch (const int opcode, const int index, const int value, void* const ptr, float opt)
    {
        return plugin.dispatch (opcode, index, value, ptr, opt);
    }

    //==============================================================================
#if JUCE_WINDOWS
    void checkPluginWindowSize()
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

    // hooks to get keyboard events from VST windows..
    static LRESULT CALLBACK vstHookWndProc (HWND hW, UINT message, WPARAM wParam, LPARAM lParam)
    {
        for (int i = activeVSTWindows.size(); --i >= 0;)
        {
            Component::SafePointer<VSTPluginWindow> w (activeVSTWindows[i]);

            if (w != nullptr && w->pluginHWND == hW)
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

                if (w != nullptr) // (may have been deleted in SendMessage callback)
                    return CallWindowProc ((WNDPROC) w->originalWndProc,
                                           (HWND) w->pluginHWND,
                                           message, wParam, lParam);
            }
        }

        return DefWindowProc (hW, message, wParam, lParam);
    }
#endif

#if JUCE_LINUX
    //==============================================================================
    // overload mouse/keyboard events to forward them to the plugin's inner window..
    void sendEventToChild (XEvent& event)
    {
        if (pluginProc != 0)
        {
            // if the plugin publishes an event procedure, pass the event directly..
            pluginProc (&event);
        }
        else if (pluginWindow != 0)
        {
            // if the plugin has a window, then send the event to the window so that
            // its message thread will pick it up..
            XSendEvent (display, pluginWindow, False, NoEventMask, &event);
            XFlush (display);
        }
    }

    void prepareXEvent (XEvent& ev, const MouseEvent& e) const noexcept
    {
        zerostruct (ev);
        ev.xcrossing.display = display;
        ev.xcrossing.window = pluginWindow;
        ev.xcrossing.root = RootWindow (display, DefaultScreen (display));
        ev.xcrossing.time = CurrentTime;
        ev.xcrossing.x = e.x;
        ev.xcrossing.y = e.y;
        ev.xcrossing.x_root = e.getScreenX();
        ev.xcrossing.y_root = e.getScreenY();
    }

    void mouseEnter (const MouseEvent& e) override
    {
        if (pluginWindow != 0)
        {
            XEvent ev;
            prepareXEvent (ev, e);
            ev.xcrossing.type = EnterNotify;
            ev.xcrossing.mode = NotifyNormal;
            ev.xcrossing.detail = NotifyAncestor;
            translateJuceToXCrossingModifiers (e, ev);
            sendEventToChild (ev);
        }
    }

    void mouseExit (const MouseEvent& e) override
    {
        if (pluginWindow != 0)
        {
            XEvent ev;
            prepareXEvent (ev, e);
            ev.xcrossing.type = LeaveNotify;
            ev.xcrossing.mode = NotifyNormal;
            ev.xcrossing.detail = NotifyAncestor;
            ev.xcrossing.focus = hasKeyboardFocus (true);
            translateJuceToXCrossingModifiers (e, ev);
            sendEventToChild (ev);
        }
    }

    void mouseMove (const MouseEvent& e) override
    {
        if (pluginWindow != 0)
        {
            XEvent ev;
            prepareXEvent (ev, e);
            ev.xmotion.type = MotionNotify;
            ev.xmotion.is_hint = NotifyNormal;
            sendEventToChild (ev);
        }
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (pluginWindow != 0)
        {
            XEvent ev;
            prepareXEvent (ev, e);
            ev.xmotion.type = MotionNotify;
            ev.xmotion.is_hint = NotifyNormal;
            translateJuceToXMotionModifiers (e, ev);
            sendEventToChild (ev);
        }
    }

    void mouseUp (const MouseEvent& e) override
    {
        if (pluginWindow != 0)
        {
            XEvent ev;
            prepareXEvent (ev, e);
            ev.xbutton.type = ButtonRelease;
            translateJuceToXButtonModifiers (e, ev);
            sendEventToChild (ev);
        }
    }

    void mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel) override
    {
        if (pluginWindow != 0)
        {
            XEvent ev;
            prepareXEvent (ev, e);
            ev.xbutton.type = ButtonPress;
            translateJuceToXMouseWheelModifiers (e, wheel.deltaY, ev);
            sendEventToChild (ev);

            ev.xbutton.type = ButtonRelease;
            sendEventToChild (ev);
        }
    }

    void updatePluginWindowHandle()
    {
        pluginWindow = getChildWindow ((Window) getWindowHandle());

        if (pluginWindow != 0)
            pluginProc = (EventProcPtr) getPropertyFromXWindow (pluginWindow,
                                                                XInternAtom (display, "_XEventProc", False));
    }
#endif

    //==============================================================================
#if JUCE_MAC
   #if JUCE_SUPPORT_CARBON
    class CarbonWrapperComponent   : public CarbonViewWrapperComponent
    {
    public:
        CarbonWrapperComponent (VSTPluginWindow& w)
            : owner (w), alreadyInside (false)
        {
            keepPluginWindowWhenHidden = w.shouldAvoidDeletingWindow();
            setRepaintsChildHIViewWhenCreated (w.shouldRepaintCarbonWindowWhenCreated());
        }

        ~CarbonWrapperComponent()
        {
            deleteWindow();
        }

        HIViewRef attachView (WindowRef windowRef, HIViewRef /*rootView*/) override
        {
            owner.openPluginWindow (windowRef);
            return 0;
        }

        void removeView (HIViewRef) override
        {
            if (owner.isOpen)
            {
                owner.isOpen = false;
                owner.dispatch (plugInOpcodeCloseEditor, 0, 0, 0, 0);
                owner.dispatch (plugInOpcodeSleepEditor, 0, 0, 0, 0);
            }
        }

        bool getEmbeddedViewSize (int& w, int& h) override
        {
            VstEditorBounds* rect = nullptr;
            owner.dispatch (plugInOpcodeGetEditorBounds, 0, 0, &rect, 0);
            w = rect->rightmost - rect->leftmost;
            h = rect->lower - rect->upper;
            return true;
        }

        void handleMouseDown (int x, int y) override
        {
            if (! alreadyInside)
            {
                alreadyInside = true;
                getTopLevelComponent()->toFront (true);
                owner.dispatch (plugInOpcodeGetMouse, x, y, 0, 0);
                alreadyInside = false;
            }
            else
            {
                PostEvent (::mouseDown, 0);
            }
        }

        void handlePaint() override
        {
            if (ComponentPeer* const peer = getPeer())
            {
                const Point<int> pos (peer->globalToLocal (getScreenPosition()));
                VstEditorBounds r;
                r.leftmost  = (int16) pos.getX();
                r.upper     = (int16) pos.getY();
                r.rightmost = (int16) (r.leftmost + getWidth());
                r.lower     = (int16) (r.upper + getHeight());

                owner.dispatch (plugInOpcodeDrawEditor, 0, 0, &r, 0);
            }
        }

    private:
        VSTPluginWindow& owner;
        bool alreadyInside;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CarbonWrapperComponent)
    };

    friend class CarbonWrapperComponent;
    ScopedPointer<CarbonWrapperComponent> carbonWrapper;
   #endif

    ScopedPointer<AutoResizingNSViewComponentWithParent> cocoaWrapper;

    void resized() override
    {
       #if JUCE_SUPPORT_CARBON
        if (carbonWrapper != nullptr)
            carbonWrapper->setSize (getWidth(), getHeight());
       #endif

        if (cocoaWrapper != nullptr)
            cocoaWrapper->setSize (getWidth(), getHeight());
    }
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VSTPluginWindow)
};
#endif
#if JUCE_MSVC
 #pragma warning (pop)
#endif

//==============================================================================
AudioProcessorEditor* VSTPluginInstance::createEditor()
{
   #if JUCE_IOS
    return nullptr;
   #else
    return hasEditor() ? new VSTPluginWindow (*this)
                       : nullptr;
   #endif
}

//==============================================================================
// entry point for all callbacks from the plugin
static pointer_sized_int VSTINTERFACECALL audioMaster (VstEffectInterface* effect, int32 opcode, int32 index, pointer_sized_int value, void* ptr, float opt)
{
    if (effect != nullptr)
        if (VSTPluginInstance* instance = (VSTPluginInstance*) (effect->hostSpace2))
            return instance->handleCallback (opcode, index, value, ptr, opt);

    return VSTPluginInstance::handleGeneralCallback (opcode, index, value, ptr, opt);
}

//==============================================================================
VSTPluginFormat::VSTPluginFormat() {}
VSTPluginFormat::~VSTPluginFormat() {}

static VSTPluginInstance* createAndUpdateDesc (VSTPluginFormat& format, PluginDescription& desc)
{
    if (AudioPluginInstance* p = format.createInstanceFromDescription (desc, 44100.0, 512))
    {
        if (VSTPluginInstance* instance = dynamic_cast<VSTPluginInstance*> (p))
        {
           #if JUCE_MAC
            if (instance->vstModule->resFileId != 0)
                UseResFile (instance->vstModule->resFileId);
           #endif

            instance->fillInPluginDescription (desc);
            return instance;
        }

        jassertfalse;
    }

    return nullptr;
}

void VSTPluginFormat::findAllTypesForFile (OwnedArray<PluginDescription>& results,
                                           const String& fileOrIdentifier)
{
    if (! fileMightContainThisPluginType (fileOrIdentifier))
        return;

    PluginDescription desc;
    desc.fileOrIdentifier = fileOrIdentifier;
    desc.uid = 0;

    ScopedPointer<VSTPluginInstance> instance (createAndUpdateDesc (*this, desc));

    if (instance == nullptr)
        return;

    if (instance->getVstCategory() != kPlugCategShell)
    {
        // Normal plugin...
        results.add (new PluginDescription (desc));

        instance->dispatch (plugInOpcodeOpen, 0, 0, 0, 0);
    }
    else
    {
        // It's a shell plugin, so iterate all the subtypes...
        for (;;)
        {
            char shellEffectName [256] = { 0 };
            const int uid = (int) instance->dispatch (plugInOpcodeNextPlugInUniqueID, 0, 0, shellEffectName, 0);

            if (uid == 0)
                break;

            desc.uid = uid;
            desc.name = shellEffectName;

            aboutToScanVSTShellPlugin (desc);

            ScopedPointer<VSTPluginInstance> shellInstance (createAndUpdateDesc (*this, desc));

            if (shellInstance != nullptr)
            {
                jassert (desc.uid == uid);
                desc.hasSharedContainer = true;
                desc.name = shellEffectName;

                if (! arrayContainsPlugin (results, desc))
                    results.add (new PluginDescription (desc));
            }
        }
    }
}

void VSTPluginFormat::createPluginInstance (const PluginDescription& desc,
                                            double sampleRate,
                                            int blockSize,
                                            void* userData,
                                            void (*callback) (void*, AudioPluginInstance*, const String&))
{
    ScopedPointer<VSTPluginInstance> result;

    if (fileMightContainThisPluginType (desc.fileOrIdentifier))
    {
        File file (desc.fileOrIdentifier);

        const File previousWorkingDirectory (File::getCurrentWorkingDirectory());
        file.getParentDirectory().setAsCurrentWorkingDirectory();

        if (ModuleHandle::Ptr module = ModuleHandle::findOrCreateModule (file))
        {
            shellUIDToCreate = desc.uid;

            result = VSTPluginInstance::create (module, sampleRate, blockSize);

            if (result != nullptr && ! result->initialiseEffect (sampleRate, blockSize))
                result = nullptr;
        }

        previousWorkingDirectory.setAsCurrentWorkingDirectory();
    }

    String errorMsg;

    if (result == nullptr)
        errorMsg = String (NEEDS_TRANS ("Unable to load XXX plug-in file")).replace ("XXX", "VST-2");

    callback (userData, result.release(), errorMsg);
}

bool VSTPluginFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const noexcept
{
    return false;
}

bool VSTPluginFormat::fileMightContainThisPluginType (const String& fileOrIdentifier)
{
    const File f (File::createFileWithoutCheckingPath (fileOrIdentifier));

  #if JUCE_MAC || JUCE_IOS
    return f.isDirectory() && f.hasFileExtension (".vst");
  #elif JUCE_WINDOWS
    return f.existsAsFile() && f.hasFileExtension (".dll");
  #elif JUCE_LINUX
    return f.existsAsFile() && f.hasFileExtension (".so");
  #endif
}

String VSTPluginFormat::getNameOfPluginFromIdentifier (const String& fileOrIdentifier)
{
    return fileOrIdentifier;
}

bool VSTPluginFormat::pluginNeedsRescanning (const PluginDescription& desc)
{
    return File (desc.fileOrIdentifier).getLastModificationTime() != desc.lastFileModTime;
}

bool VSTPluginFormat::doesPluginStillExist (const PluginDescription& desc)
{
    return File (desc.fileOrIdentifier).exists();
}

StringArray VSTPluginFormat::searchPathsForPlugins (const FileSearchPath& directoriesToSearch, const bool recursive, bool)
{
    StringArray results;

    for (int j = 0; j < directoriesToSearch.getNumPaths(); ++j)
        recursiveFileSearch (results, directoriesToSearch [j], recursive);

    return results;
}

void VSTPluginFormat::recursiveFileSearch (StringArray& results, const File& dir, const bool recursive)
{
    // avoid allowing the dir iterator to be recursive, because we want to avoid letting it delve inside
    // .component or .vst directories.
    DirectoryIterator iter (dir, false, "*", File::findFilesAndDirectories);

    while (iter.next())
    {
        const File f (iter.getFile());
        bool isPlugin = false;

        if (fileMightContainThisPluginType (f.getFullPathName()))
        {
            isPlugin = true;
            results.add (f.getFullPathName());
        }

        if (recursive && (! isPlugin) && f.isDirectory())
            recursiveFileSearch (results, f, true);
    }
}

FileSearchPath VSTPluginFormat::getDefaultLocationsToSearch()
{
   #if JUCE_MAC
    return FileSearchPath ("~/Library/Audio/Plug-Ins/VST;/Library/Audio/Plug-Ins/VST");
   #elif JUCE_LINUX
    return FileSearchPath (SystemStats::getEnvironmentVariable ("VST_PATH",
                                                                "/usr/lib/vst;/usr/local/lib/vst;~/.vst")
                             .replace (":", ";"));
   #elif JUCE_WINDOWS
    const String programFiles (File::getSpecialLocation (File::globalApplicationsDirectory).getFullPathName());

    FileSearchPath paths;
    paths.add (WindowsRegistry::getValue ("HKEY_LOCAL_MACHINE\\Software\\VST\\VSTPluginsPath",
                                          programFiles + "\\Steinberg\\VstPlugins"));
    paths.removeNonExistentPaths();

    paths.add (WindowsRegistry::getValue ("HKEY_LOCAL_MACHINE\\Software\\VST\\VSTPluginsPath",
                                          programFiles + "\\VstPlugins"));
    return paths;
   #elif JUCE_IOS
    // on iOS you can only load plug-ins inside the hosts bundle folder
    CFURLRef relativePluginDir = CFBundleCopyBuiltInPlugInsURL (CFBundleGetMainBundle());
    CFURLRef pluginDir = CFURLCopyAbsoluteURL (relativePluginDir);
    CFRelease (relativePluginDir);

    CFStringRef path = CFURLCopyFileSystemPath (pluginDir, kCFURLPOSIXPathStyle);
    CFRelease (pluginDir);

    FileSearchPath retval (String (CFStringGetCStringPtr (path, kCFStringEncodingUTF8)));
    CFRelease (path);

    return retval;
   #endif
}

const XmlElement* VSTPluginFormat::getVSTXML (AudioPluginInstance* plugin)
{
    if (VSTPluginInstance* const vst = dynamic_cast<VSTPluginInstance*> (plugin))
        if (vst->vstModule != nullptr)
            return vst->vstModule->vstXml.get();

    return nullptr;
}

bool VSTPluginFormat::loadFromFXBFile (AudioPluginInstance* plugin, const void* data, size_t dataSize)
{
    if (VSTPluginInstance* vst = dynamic_cast<VSTPluginInstance*> (plugin))
        return vst->loadFromFXBFile (data, dataSize);

    return false;
}

bool VSTPluginFormat::saveToFXBFile (AudioPluginInstance* plugin, MemoryBlock& dest, bool asFXB)
{
    if (VSTPluginInstance* vst = dynamic_cast<VSTPluginInstance*> (plugin))
        return vst->saveToFXBFile (dest, asFXB);

    return false;
}

bool VSTPluginFormat::getChunkData (AudioPluginInstance* plugin, MemoryBlock& result, bool isPreset)
{
    if (VSTPluginInstance* vst = dynamic_cast<VSTPluginInstance*> (plugin))
        return vst->getChunkData (result, isPreset, 128);

    return false;
}

bool VSTPluginFormat::setChunkData (AudioPluginInstance* plugin, const void* data, int size, bool isPreset)
{
    if (VSTPluginInstance* vst = dynamic_cast<VSTPluginInstance*> (plugin))
        return vst->setChunkData (data, size, isPreset);

    return false;
}

AudioPluginInstance* VSTPluginFormat::createCustomVSTFromMainCall (void* entryPointFunction,
                                                                   double initialSampleRate, int initialBufferSize)
{
    ModuleHandle::Ptr module = new ModuleHandle (File(), (MainCall) entryPointFunction);

    if (module->open())
    {
        ScopedPointer<VSTPluginInstance> result (VSTPluginInstance::create (module, initialSampleRate, initialBufferSize));

        if (result != nullptr && result->initialiseEffect (initialSampleRate, initialBufferSize))
            return result.release();
    }

    return nullptr;
}

void VSTPluginFormat::setExtraFunctions (AudioPluginInstance* plugin, ExtraFunctions* functions)
{
    ScopedPointer<ExtraFunctions> f (functions);

    if (VSTPluginInstance* vst = dynamic_cast<VSTPluginInstance*> (plugin))
        vst->extraFunctions = f;
}

AudioPluginInstance* VSTPluginFormat::getPluginInstanceFromVstEffectInterface (void* aEffect)
{
    if (VstEffectInterface* vstAEffect = reinterpret_cast<VstEffectInterface*> (aEffect))
        if (VSTPluginInstance* instanceVST = reinterpret_cast<VSTPluginInstance*> (vstAEffect->hostSpace2))
            return dynamic_cast<AudioPluginInstance*> (instanceVST);

    return nullptr;
}

pointer_sized_int JUCE_CALLTYPE VSTPluginFormat::dispatcher (AudioPluginInstance* plugin, int32 opcode, int32 index, pointer_sized_int value, void* ptr, float opt)
{
    if (VSTPluginInstance* vst = dynamic_cast<VSTPluginInstance*> (plugin))
        return vst->dispatch (opcode, index, value, ptr, opt);

    return 0;
}

void VSTPluginFormat::aboutToScanVSTShellPlugin (const PluginDescription&) {}

#endif
