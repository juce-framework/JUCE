/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "juce_linux_NativeIncludes.h"


BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/io/files/juce_FileInputStream.h"
#include "../../../src/juce_core/io/files/juce_FileOutputStream.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/basics/juce_Time.h"
#include "../../../src/juce_core/basics/juce_Random.h"
#include "../../../src/juce_core/io/network/juce_URL.h"
#include "../../../src/juce_core/io/files/juce_NamedPipe.h"
#include "../../../src/juce_core/threads/juce_InterProcessLock.h"
#include "../../../src/juce_appframework/audio/devices/juce_AudioIODeviceType.h"
#include "../../../src/juce_core/threads/juce_Thread.h"
#include "../../../src/juce_core/threads/juce_ScopedLock.h"
#include "../../../src/juce_core/io/files/juce_File.h"
#include "../../../src/juce_core/basics/juce_Singleton.h"
#include "../../../src/juce_appframework/audio/dsp/juce_AudioDataConverters.h"
#include "../../../src/juce_appframework/audio/audio_file_formats/juce_AudioCDReader.h"
#include "../../../src/juce_appframework/gui/graphics/fonts/juce_Font.h"
#include "../../../src/juce_core/io/streams/juce_MemoryInputStream.h"
#include "../../../src/juce_core/io/files/juce_DirectoryIterator.h"
#include "../../../src/juce_core/text/juce_XmlDocument.h"
#include "../../../src/juce_appframework/application/juce_DeletedAtShutdown.h"
#include "../../../src/juce_appframework/events/juce_MessageManager.h"
#include "../../../src/juce_core/threads/juce_WaitableEvent.h"
#include "../../../src/juce_core/threads/juce_Process.h"
#include "../../../src/juce_appframework/gui/components/filebrowser/juce_FileChooser.h"
#include "../../../src/juce_appframework/audio/devices/juce_MidiOutput.h"
#include "../../../src/juce_appframework/audio/devices/juce_MidiInput.h"
#include "../../../src/juce_core/text/juce_StringArray.h"
#include "../../../src/juce_core/containers/juce_MemoryBlock.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"
#include "../../../src/juce_core/threads/juce_CriticalSection.h"
#include "../../../src/juce_appframework/events/juce_Timer.h"
#include "../../../src/juce_appframework/gui/components/special/juce_WebBrowserComponent.h"
#include "../../../src/juce_appframework/gui/components/keyboard/juce_KeyPress.h"
#include "../../../src/juce_appframework/application/juce_SystemClipboard.h"
#include "../../../src/juce_appframework/gui/components/windows/juce_AlertWindow.h"
#include "../../../src/juce_appframework/gui/components/special/juce_OpenGLComponent.h"
#include "../../../src/juce_appframework/gui/components/juce_Desktop.h"
#include "../../../src/juce_appframework/gui/components/juce_ComponentDeletionWatcher.h"
#include "../../../src/juce_appframework/gui/graphics/geometry/juce_RectangleList.h"
#include "../../../src/juce_appframework/gui/graphics/imaging/juce_ImageFileFormat.h"
#include "../../../src/juce_appframework/gui/graphics/contexts/juce_LowLevelGraphicsSoftwareRenderer.h"
#include "../../../src/juce_appframework/gui/components/mouse/juce_DragAndDropContainer.h"
#include "../../../src/juce_appframework/gui/components/special/juce_SystemTrayIconComponent.h"
#include "../../../src/juce_appframework/application/juce_Application.h"

/* Remove this macro if you're having problems compiling the cpu affinity
   calls (the API for these has changed about quite a bit in various Linux
   versions, and a lot of distros seem to ship with obsolete versions)
*/
#if defined (CPU_ISSET) && ! defined (SUPPORT_AFFINITIES)
  #define SUPPORT_AFFINITIES 1
#endif

//==============================================================================
#define JUCE_INCLUDED_FILE 1

// Now include the actual code files..

#include "../../macosx/platform_specific_code/juce_posix_SharedCode.h"
#include "juce_linux_Files.cpp"
#include "../../macosx/platform_specific_code/juce_mac_NamedPipe.cpp"
#include "juce_linux_Network.cpp"
#include "juce_linux_SystemStats.cpp"
#include "juce_linux_Threads.cpp"

#if ! JUCE_ONLY_BUILD_CORE_LIBRARY
 #include "juce_linux_Messaging.cpp"
 #include "juce_linux_Fonts.cpp"
 #include "juce_linux_Windowing.cpp"
 #include "juce_linux_Audio.cpp"
 #include "juce_linux_Midi.cpp"
 #include "juce_linux_AudioCDReader.cpp"
 #include "juce_linux_FileChooser.cpp"
 #include "juce_linux_WebBrowserComponent.cpp"
#endif

END_JUCE_NAMESPACE
