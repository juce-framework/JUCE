/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

/*
    This file wraps together all the mac-specific code, so that
    we can include all the native headers just once, and compile all our
    platform-specific stuff in one big lump, keeping it out of the way of
    the rest of the codebase.
*/

#include "../../core/juce_TargetPlatform.h"

#if JUCE_MAC || JUCE_IOS

#undef JUCE_BUILD_NATIVE
#define JUCE_BUILD_NATIVE 1

#include "../../core/juce_StandardHeader.h"
#include "juce_mac_NativeIncludes.h"

BEGIN_JUCE_NAMESPACE

//==============================================================================
#include "../../core/juce_Singleton.h"
#include "../../maths/juce_Random.h"
#include "../../core/juce_SystemStats.h"
#include "../../threads/juce_Process.h"
#include "../../threads/juce_Thread.h"
#include "../../threads/juce_InterProcessLock.h"
#include "../../io/files/juce_FileInputStream.h"
#include "../../io/files/juce_FileOutputStream.h"
#include "../../io/files/juce_NamedPipe.h"
#include "../../io/files/juce_DirectoryIterator.h"
#include "../../io/files/juce_MemoryMappedFile.h"
#include "../../io/network/juce_URL.h"
#include "../../io/network/juce_MACAddress.h"
#include "../../io/streams/juce_MemoryInputStream.h"
#include "../../io/streams/juce_BufferedInputStream.h"
#include "../../core/juce_PlatformUtilities.h"
#include "../../core/juce_Initialisation.h"
#include "../../text/juce_LocalisedStrings.h"
#include "../../text/juce_XmlDocument.h"
#include "../../utilities/juce_DeletedAtShutdown.h"
#include "../../application/juce_Application.h"
#include "../../utilities/juce_SystemClipboard.h"
#include "../../events/juce_MessageManager.h"
#include "../../containers/juce_ReferenceCountedArray.h"
#include "../../gui/graphics/contexts/juce_LowLevelGraphicsSoftwareRenderer.h"
#include "../../gui/graphics/contexts/juce_EdgeTable.h"
#include "../../gui/graphics/imaging/juce_ImageFileFormat.h"
#include "../../gui/graphics/imaging/juce_CameraDevice.h"
#include "../../gui/components/windows/juce_AlertWindow.h"
#include "../../gui/components/windows/juce_NativeMessageBox.h"
#include "../../gui/components/windows/juce_ComponentPeer.h"
#include "../../gui/components/juce_Desktop.h"
#include "../../gui/components/menus/juce_MenuBarModel.h"
#include "../../gui/components/special/juce_OpenGLComponent.h"
#include "../../gui/components/special/juce_QuickTimeMovieComponent.h"
#include "../../gui/components/mouse/juce_DragAndDropContainer.h"
#include "../../gui/components/mouse/juce_MouseEvent.h"
#include "../../gui/components/mouse/juce_MouseInputSource.h"
#include "../../gui/components/keyboard/juce_KeyPressMappingSet.h"
#include "../../gui/components/special/juce_NSViewComponent.h"
#include "../../gui/components/layout/juce_ComponentMovementWatcher.h"
#include "../../gui/components/special/juce_WebBrowserComponent.h"
#include "../../gui/components/filebrowser/juce_FileChooser.h"
#include "../../audio/audio_file_formats/juce_AudioCDBurner.h"
#include "../../audio/audio_file_formats/juce_AudioCDReader.h"
#include "../../audio/audio_file_formats/juce_AiffAudioFormat.h"
#include "../../audio/audio_sources/juce_AudioSource.h"
#include "../../audio/dsp/juce_AudioDataConverters.h"
#include "../../audio/devices/juce_AudioIODeviceType.h"
#include "../../audio/midi/juce_MidiOutput.h"
#include "../../audio/midi/juce_MidiInput.h"
#include "../../containers/juce_ScopedValueSetter.h"
#include "../common/juce_MidiDataConcatenator.h"
#undef Point

#if ! JUCE_ONLY_BUILD_CORE_LIBRARY
namespace
{
    template <class RectType>
    const Rectangle<int> convertToRectInt (const RectType& r)
    {
        return Rectangle<int> ((int) r.origin.x, (int) r.origin.y, (int) r.size.width, (int) r.size.height);
    }

    template <class RectType>
    const Rectangle<float> convertToRectFloat (const RectType& r)
    {
        return Rectangle<float> (r.origin.x, r.origin.y, r.size.width, r.size.height);
    }

    template <class RectType>
    CGRect convertToCGRect (const RectType& r)
    {
        return CGRectMake ((CGFloat) r.getX(), (CGFloat) r.getY(), (CGFloat) r.getWidth(), (CGFloat) r.getHeight());
    }
}

//==============================================================================
class MessageQueue
{
public:
    MessageQueue()
    {
       #if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_4 && ! JUCE_IOS
        runLoop = CFRunLoopGetMain();
       #else
        runLoop = CFRunLoopGetCurrent();
       #endif

        CFRunLoopSourceContext sourceContext = { 0 };
        sourceContext.info = this;
        sourceContext.perform = runLoopSourceCallback;
        runLoopSource = CFRunLoopSourceCreate (kCFAllocatorDefault, 1, &sourceContext);
        CFRunLoopAddSource (runLoop, runLoopSource, kCFRunLoopCommonModes);
    }

    ~MessageQueue()
    {
        CFRunLoopRemoveSource (runLoop, runLoopSource, kCFRunLoopCommonModes);
        CFRunLoopSourceInvalidate (runLoopSource);
        CFRelease (runLoopSource);
    }

    void post (Message* const message)
    {
        messages.add (message);
        CFRunLoopSourceSignal (runLoopSource);
        CFRunLoopWakeUp (runLoop);
    }

private:
    ReferenceCountedArray <Message, CriticalSection> messages;
    CriticalSection lock;
    CFRunLoopRef runLoop;
    CFRunLoopSourceRef runLoopSource;

    bool deliverNextMessage()
    {
        const Message::Ptr nextMessage (messages.removeAndReturn (0));

        if (nextMessage == nullptr)
            return false;

        JUCE_AUTORELEASEPOOL
        MessageManager::getInstance()->deliverMessage (nextMessage);
        return true;
    }

    void runLoopCallback()
    {
        for (int i = 4; --i >= 0;)
            if (! deliverNextMessage())
                return;

        CFRunLoopSourceSignal (runLoopSource);
        CFRunLoopWakeUp (runLoop);
    }

    static void runLoopSourceCallback (void* info)
    {
        static_cast <MessageQueue*> (info)->runLoopCallback();
    }
};
#endif

//==============================================================================
#define JUCE_INCLUDED_FILE 1

// Now include the actual code files..

#include "juce_mac_ObjCSuffix.h"
#include "juce_mac_Strings.mm"
#include "juce_mac_SystemStats.mm"
#include "juce_mac_Network.mm"
#include "../common/juce_posix_NamedPipe.cpp"
#include "juce_mac_Threads.mm"
#include "../common/juce_posix_SharedCode.h"
#include "juce_mac_Files.mm"

#if JUCE_IOS
 #include "juce_ios_MiscUtilities.mm"
#else
 #include "juce_mac_MiscUtilities.mm"
#endif

#include "juce_mac_Debugging.mm"

#if ! JUCE_ONLY_BUILD_CORE_LIBRARY
 #if JUCE_IOS
  #include "juce_mac_Fonts.mm"
  #include "juce_mac_CoreGraphicsContext.mm"
  #include "juce_ios_UIViewComponentPeer.mm"
  #include "juce_ios_MessageManager.mm"
  #include "juce_mac_FileChooser.mm"
  #include "juce_mac_OpenGLComponent.mm"
  #include "juce_mac_MouseCursor.mm"
  #include "juce_mac_WebBrowserComponent.mm"
  #include "juce_ios_Audio.cpp"
  #include "juce_mac_CoreMidi.cpp"
 #else
  #include "juce_mac_Fonts.mm" // (must go before juce_mac_CoreGraphicsContext.mm)
  #include "juce_mac_CoreGraphicsContext.mm"
  #include "juce_mac_NSViewComponentPeer.mm"
  #include "juce_mac_MouseCursor.mm"
  #include "juce_mac_NSViewComponent.mm"
  #include "juce_mac_AppleRemote.mm"
  #include "juce_mac_OpenGLComponent.mm"
  #include "juce_mac_MainMenu.mm"
  #include "juce_mac_FileChooser.mm"
  #include "juce_mac_QuickTimeMovieComponent.mm"
  #include "juce_mac_AudioCDBurner.mm"
  #include "juce_mac_AudioCDReader.mm"
  #include "juce_mac_MessageManager.mm"
  #include "juce_mac_WebBrowserComponent.mm"
  #include "juce_mac_CoreAudio.cpp"
  #include "juce_mac_CoreMidi.cpp"
  #include "juce_mac_CameraDevice.mm"
 #endif
#endif

END_JUCE_NAMESPACE

#endif
