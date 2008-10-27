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

//==============================================================================
/*
    This file wraps together all the win32-specific code, so that
    we can include all the native headers just once, and compile all our
    platform-specific stuff in one big lump, keeping it out of the way of
    the rest of the codebase.
*/

//==============================================================================
#include "juce_win32_NativeIncludes.h"

#include "../../../src/juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

//==============================================================================
#include "../../../src/juce_core/basics/juce_Singleton.h"
#include "../../../src/juce_core/basics/juce_Random.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/threads/juce_Process.h"
#include "../../../src/juce_core/threads/juce_Thread.h"
#include "../../../src/juce_core/threads/juce_InterProcessLock.h"
#include "../../../src/juce_core/io/files/juce_FileInputStream.h"
#include "../../../src/juce_core/io/files/juce_NamedPipe.h"
#include "../../../src/juce_core/io/network/juce_URL.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"
#include "../../../src/juce_core/text/juce_LocalisedStrings.h"
#include "../../../src/juce_appframework/application/juce_DeletedAtShutdown.h"
#include "../../../src/juce_appframework/application/juce_Application.h"
#include "../../../src/juce_appframework/application/juce_SystemClipboard.h"
#include "../../../src/juce_appframework/events/juce_MessageManager.h"
#include "../../../src/juce_appframework/gui/graphics/contexts/juce_LowLevelGraphicsSoftwareRenderer.h"
#include "../../../src/juce_appframework/gui/graphics/imaging/juce_ImageFileFormat.h"
#include "../../../src/juce_appframework/gui/components/windows/juce_AlertWindow.h"
#include "../../../src/juce_appframework/gui/components/juce_Desktop.h"
#include "../../../src/juce_appframework/gui/components/menus/juce_MenuBarModel.h"
#include "../../../src/juce_appframework/gui/components/special/juce_OpenGLComponent.h"
#include "../../../src/juce_appframework/gui/components/special/juce_QuickTimeMovieComponent.h"
#include "../../../src/juce_appframework/gui/components/mouse/juce_DragAndDropContainer.h"
#include "../../../src/juce_appframework/gui/components/keyboard/juce_KeyPressMappingSet.h"
#include "../../../src/juce_appframework/gui/components/layout/juce_ComponentMovementWatcher.h"
#include "../../../src/juce_appframework/gui/components/special/juce_ActiveXControlComponent.h"
#include "../../../src/juce_appframework/gui/components/special/juce_WebBrowserComponent.h"
#include "../../../src/juce_appframework/gui/components/special/juce_DropShadower.h"
#include "../../../src/juce_appframework/gui/components/special/juce_SystemTrayIconComponent.h"
#include "../../../src/juce_appframework/gui/components/filebrowser/juce_FileChooser.h"
#include "../../../src/juce_appframework/gui/components/lookandfeel/juce_LookAndFeel.h"
#include "../../../src/juce_appframework/audio/audio_file_formats/juce_AudioCDBurner.h"
#include "../../../src/juce_appframework/audio/audio_file_formats/juce_AudioCDReader.h"
#include "../../../src/juce_appframework/audio/audio_sources/juce_AudioSource.h"
#include "../../../src/juce_appframework/audio/dsp/juce_AudioDataConverters.h"
#include "../../../src/juce_appframework/audio/devices/juce_AudioIODeviceType.h"
#include "../../../src/juce_appframework/audio/devices/juce_MidiOutput.h"
#include "../../../src/juce_appframework/audio/devices/juce_MidiInput.h"


//==============================================================================
#define JUCE_INCLUDED_FILE 1

// Now include the actual code files..
#include "juce_win32_DynamicLibraryLoader.cpp"
#include "juce_win32_SystemStats.cpp"
#include "juce_win32_Threads.cpp"
#include "juce_win32_Files.cpp"
#include "juce_win32_Network.cpp"
#include "juce_win32_PlatformUtils.cpp"

#if ! JUCE_ONLY_BUILD_CORE_LIBRARY
 #include "juce_win32_Messaging.cpp"
 #include "juce_win32_Windowing.cpp"
 #include "juce_win32_Fonts.cpp"
 #include "juce_win32_FileChooser.cpp"
 #include "juce_win32_Misc.cpp"
 #include "juce_win32_ActiveXComponent.cpp"
 #include "juce_win32_QuickTimeMovieComponent.cpp"
 #include "juce_win32_WebBrowserComponent.cpp"
 #include "juce_win32_OpenGLComponent.cpp"
 #include "juce_win32_AudioCDReader.cpp"
 #include "juce_win32_Midi.cpp"
 #include "juce_win32_ASIO.cpp"
 #include "juce_win32_DirectSound.cpp"
#endif

//==============================================================================
// Auto-link the other win32 libs that are needed by library calls..
#if defined (JUCE_DLL_BUILD) && JUCE_MSVC
 #include "juce_win32_AutoLinkLibraries.h"
#endif


END_JUCE_NAMESPACE
