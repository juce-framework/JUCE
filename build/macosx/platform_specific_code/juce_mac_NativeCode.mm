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

/*
    This file wraps together all the mac-specific code, so that
    we can include all the native headers just once, and compile all our
    platform-specific stuff in one big lump, keeping it out of the way of
    the rest of the codebase.
*/

#include "juce_mac_NativeIncludes.h"


BEGIN_JUCE_NAMESPACE

//==============================================================================
#include "../../../src/juce_core/basics/juce_Singleton.h"
#include "../../../src/juce_core/basics/juce_Random.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/threads/juce_Process.h"
#include "../../../src/juce_core/threads/juce_Thread.h"
#include "../../../src/juce_core/threads/juce_InterProcessLock.h"
#include "../../../src/juce_core/io/files/juce_FileInputStream.h"
#include "../../../src/juce_core/io/network/juce_URL.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"
#include "../../../src/juce_core/text/juce_LocalisedStrings.h"
#include "../../../src/juce_appframework/application/juce_DeletedAtShutdown.h"
#include "../../../src/juce_appframework/application/juce_Application.h"
#include "../../../src/juce_appframework/application/juce_SystemClipboard.h"
#include "../../../src/juce_appframework/events/juce_MessageManager.h"
#include "../../../src/juce_appframework/gui/graphics/contexts/juce_LowLevelGraphicsSoftwareRenderer.h"
#include "../../../src/juce_appframework/gui/graphics/imaging/juce_ImageFileFormat.h"
#include "../../../src/juce_appframework/gui/graphics/imaging/juce_CameraDevice.h"
#include "../../../src/juce_appframework/gui/components/windows/juce_AlertWindow.h"
#include "../../../src/juce_appframework/gui/components/juce_Desktop.h"
#include "../../../src/juce_appframework/gui/components/menus/juce_MenuBarModel.h"
#include "../../../src/juce_appframework/gui/components/special/juce_OpenGLComponent.h"
#include "../../../src/juce_appframework/gui/components/special/juce_QuickTimeMovieComponent.h"
#include "../../../src/juce_appframework/gui/components/mouse/juce_DragAndDropContainer.h"
#include "../../../src/juce_appframework/gui/components/keyboard/juce_KeyPressMappingSet.h"
#include "../../../src/juce_appframework/gui/components/special/juce_NSViewComponent.h"
#include "../../../src/juce_appframework/gui/components/layout/juce_ComponentMovementWatcher.h"
#include "../../../src/juce_appframework/gui/components/special/juce_WebBrowserComponent.h"
#include "../../../src/juce_appframework/gui/components/filebrowser/juce_FileChooser.h"
#include "../../../src/juce_appframework/audio/audio_file_formats/juce_AudioCDBurner.h"
#include "../../../src/juce_appframework/audio/audio_file_formats/juce_AudioCDReader.h"
#include "../../../src/juce_appframework/audio/audio_sources/juce_AudioSource.h"
#include "../../../src/juce_appframework/audio/dsp/juce_AudioDataConverters.h"
#include "../../../src/juce_appframework/audio/devices/juce_AudioIODeviceType.h"
#include "../../../src/juce_appframework/audio/devices/juce_MidiOutput.h"
#include "../../../src/juce_appframework/audio/devices/juce_MidiInput.h"
#undef Point

//==============================================================================
/** This suffix is used for naming all Obj-C classes that are used inside juce.

    Because of the flat naming structure used by Obj-C, you can get horrible situations where
    two DLLs are loaded into a host, each of which uses classes with the same names, and these get
    cross-linked so that when you make a call to a class that you thought was private, it ends up
    actually calling into a similarly named class in the other module's address space.

    By changing this macro to a unique value, you ensure that all the obj-C classes in your app
    have unique names, and should avoid this problem.

    If you're using the amalgamated version, you can just set this macro to something unique before
    you include juce_amalgamated.cpp.
*/
#ifndef JUCE_ObjCExtraSuffix
 #define JUCE_ObjCExtraSuffix 3
#endif

#define appendMacro1(a, b, c, d) a ## _ ## b ## _ ## c ## _ ## d
#define appendMacro2(a, b, c, d) appendMacro1(a, b, c, d)
#define MakeObjCClassName(rootName)  appendMacro2 (rootName, JUCE_MAJOR_VERSION, JUCE_MINOR_VERSION, JUCE_ObjCExtraSuffix)

//==============================================================================
#define JUCE_INCLUDED_FILE 1

// Now include the actual code files..

#include "juce_mac_Strings.mm"
#include "juce_mac_SystemStats.mm"
#include "juce_mac_Network.mm"
#include "juce_mac_Threads.mm"
#include "juce_posix_SharedCode.h"
#include "juce_mac_Files.mm"
#include "juce_mac_MiscUtilities.mm"
#include "juce_mac_Debugging.mm"

#if ! JUCE_ONLY_BUILD_CORE_LIBRARY
#include "juce_mac_NSViewComponentPeer.mm"
#include "juce_mac_MouseCursor.mm"
#include "juce_mac_NSViewComponent.mm"
#include "juce_mac_AppleRemote.mm"
#include "juce_mac_OpenGLComponent.mm"
#include "juce_mac_MainMenu.mm"
#include "juce_mac_FileChooser.mm"
#include "juce_mac_QuickTimeMovieComponent.mm"
#include "juce_mac_AudioCDBurner.mm"
#include "juce_mac_Fonts.mm"
#include "juce_mac_MessageManager.mm"
#include "juce_mac_WebBrowserComponent.mm"
#include "juce_mac_CoreAudio.cpp"
#include "juce_mac_CoreMidi.cpp"
#include "juce_mac_CameraDevice.mm"
#endif

END_JUCE_NAMESPACE
