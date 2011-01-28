/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
    This file wraps together all the android-specific code, so that
    we can include all the native headers just once, and compile all our
    platform-specific stuff in one big lump, keeping it out of the way of
    the rest of the codebase.
*/

#include "../../core/juce_TargetPlatform.h"

#if JUCE_ANDROID

#undef JUCE_BUILD_NATIVE
#define JUCE_BUILD_NATIVE 1

#include "juce_android_NativeIncludes.h"
#include "../../core/juce_StandardHeader.h"

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
#include "../../io/network/juce_URL.h"
#include "../../io/network/juce_MACAddress.h"
#include "../../core/juce_PlatformUtilities.h"
#include "../../text/juce_LocalisedStrings.h"
#include "../../utilities/juce_DeletedAtShutdown.h"
#include "../../application/juce_Application.h"
#include "../../utilities/juce_SystemClipboard.h"
#include "../../events/juce_MessageManager.h"
#include "../../gui/graphics/contexts/juce_LowLevelGraphicsSoftwareRenderer.h"
#include "../../gui/graphics/imaging/juce_ImageFileFormat.h"
#include "../../gui/graphics/imaging/juce_CameraDevice.h"
#include "../../gui/components/windows/juce_ComponentPeer.h"
#include "../../gui/components/windows/juce_AlertWindow.h"
#include "../../gui/components/juce_Desktop.h"
#include "../../gui/components/menus/juce_MenuBarModel.h"
#include "../../gui/components/special/juce_OpenGLComponent.h"
#include "../../gui/components/special/juce_QuickTimeMovieComponent.h"
#include "../../gui/components/mouse/juce_DragAndDropContainer.h"
#include "../../gui/components/mouse/juce_MouseInputSource.h"
#include "../../gui/components/keyboard/juce_KeyPressMappingSet.h"
#include "../../gui/components/layout/juce_ComponentMovementWatcher.h"
#include "../../gui/components/special/juce_ActiveXControlComponent.h"
#include "../../gui/components/special/juce_WebBrowserComponent.h"
#include "../../gui/components/special/juce_DropShadower.h"
#include "../../gui/components/special/juce_SystemTrayIconComponent.h"
#include "../../gui/components/filebrowser/juce_FileChooser.h"
#include "../../gui/components/lookandfeel/juce_LookAndFeel.h"
#include "../../audio/audio_file_formats/juce_AudioCDBurner.h"
#include "../../audio/audio_file_formats/juce_AudioCDReader.h"
#include "../../audio/audio_sources/juce_AudioSource.h"
#include "../../audio/dsp/juce_AudioDataConverters.h"
#include "../../audio/devices/juce_AudioIODeviceType.h"
#include "../../audio/devices/juce_MidiOutput.h"
#include "../../audio/devices/juce_MidiInput.h"
#include "../../containers/juce_ScopedValueSetter.h"
#include "../common/juce_MidiDataConcatenator.h"

//==============================================================================
#define JUCE_INCLUDED_FILE 1

// Now include the actual code files..
#include "juce_android_SystemStats.cpp" // (must be first)
#include "../common/juce_posix_SharedCode.h"
#include "juce_android_Files.cpp"
#include "../common/juce_posix_NamedPipe.cpp"
#include "juce_android_Threads.cpp"
#include "juce_android_Network.cpp"
#include "juce_android_Messaging.cpp"
#include "juce_android_Fonts.cpp"
#include "juce_android_GraphicsContext.cpp"
#include "juce_android_Windowing.cpp"
#include "juce_android_FileChooser.cpp"
#include "juce_android_Misc.cpp"
#include "juce_android_WebBrowserComponent.cpp"
#include "juce_android_OpenGLComponent.cpp"
#include "juce_android_Midi.cpp"
#include "juce_android_Audio.cpp"
#include "juce_android_CameraDevice.cpp"

END_JUCE_NAMESPACE

#endif
