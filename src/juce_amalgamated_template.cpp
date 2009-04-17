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
    This monolithic file contains the entire Juce source tree!

    To build an app which uses Juce, all you need to do is to add this
    file to your project, and include juce.h in your own cpp files.

*/

#ifdef __JUCE_JUCEHEADER__
 /* When you add the amalgamated cpp file to your project, you mustn't include it in
    a file where you've already included juce.h - just put it inside a file on its own,
    possibly with your config flags preceding it, but don't include anything else. */
 #error
#endif

#include "../juce_Config.h"

//==============================================================================
#ifdef _WIN32
 #include "../build/win32/platform_specific_code/juce_win32_NativeIncludes.h"
#elif defined (LINUX)
 //
#else
 #include "../build/macosx/platform_specific_code/juce_mac_NativeIncludes.h"
#endif

//==============================================================================
#define DONT_SET_USING_JUCE_NAMESPACE 1

#include "../juce_amalgamated.h"

#define NO_DUMMY_DECL

#if (defined(_MSC_VER) && (_MSC_VER <= 1200))
 #pragma warning (disable: 4309 4305)
#endif

#if JUCE_MAC && JUCE_32BIT && JUCE_SUPPORT_CARBON && ! JUCE_ONLY_BUILD_CORE_LIBRARY
 BEGIN_JUCE_NAMESPACE
 #include "../build/macosx/platform_specific_code/juce_mac_CarbonViewWrapperComponent.h"
 END_JUCE_NAMESPACE
#endif

//==============================================================================
#include "juce_core/basics/juce_FileLogger.cpp"
#include "juce_core/basics/juce_Logger.cpp"
#include "juce_core/basics/juce_Random.cpp"
#include "juce_core/basics/juce_RelativeTime.cpp"
#include "juce_core/basics/juce_SystemStats.cpp"
#include "juce_core/basics/juce_Time.cpp"
#include "juce_core/containers/juce_BitArray.cpp"
#include "juce_core/containers/juce_MemoryBlock.cpp"
#include "juce_core/containers/juce_PropertySet.cpp"
#include "juce_core/containers/juce_Variant.cpp"
#include "juce_core/cryptography/juce_BlowFish.cpp"
#include "juce_core/cryptography/juce_MD5.cpp"
#include "juce_core/cryptography/juce_Primes.cpp"
#include "juce_core/cryptography/juce_RSAKey.cpp"
#include "juce_core/io/juce_InputStream.cpp"
#include "juce_core/io/juce_OutputStream.cpp"
#include "juce_core/io/files/juce_DirectoryIterator.cpp"
#include "juce_core/io/files/juce_File.cpp"
#include "juce_core/io/files/juce_FileInputStream.cpp"
#include "juce_core/io/files/juce_FileOutputStream.cpp"
#include "juce_core/io/files/juce_FileSearchPath.cpp"
#include "juce_core/io/files/juce_NamedPipe.cpp"
#include "juce_core/io/network/juce_Socket.cpp"
#include "juce_core/io/network/juce_URL.cpp"
#include "juce_core/io/streams/juce_BufferedInputStream.cpp"
#include "juce_core/io/streams/juce_FileInputSource.cpp"
#include "juce_core/io/streams/juce_MemoryInputStream.cpp"
#include "juce_core/io/streams/juce_MemoryOutputStream.cpp"
#include "juce_core/io/streams/juce_SubregionStream.cpp"
#include "juce_core/misc/juce_PerformanceCounter.cpp"
#include "juce_core/misc/juce_Uuid.cpp"
#include "juce_core/misc/juce_ZipFile.cpp"
#include "juce_core/text/juce_CharacterFunctions.cpp"
#include "juce_core/text/juce_LocalisedStrings.cpp"
#include "juce_core/text/juce_String.cpp"
#include "juce_core/text/juce_StringArray.cpp"
#include "juce_core/text/juce_StringPairArray.cpp"
#include "juce_core/text/juce_XmlDocument.cpp"
#include "juce_core/text/juce_XmlElement.cpp"
#include "juce_core/threads/juce_InterProcessLock.cpp"
#include "juce_core/threads/juce_ReadWriteLock.cpp"
#include "juce_core/threads/juce_Thread.cpp"
#include "juce_core/threads/juce_ThreadPool.cpp"
#include "juce_core/threads/juce_TimeSliceThread.cpp"

#if ! JUCE_ONLY_BUILD_CORE_LIBRARY

#include "juce_appframework/application/juce_Application.cpp"
#include "juce_appframework/application/juce_ApplicationCommandInfo.cpp"
#include "juce_appframework/application/juce_ApplicationCommandManager.cpp"
#include "juce_appframework/application/juce_ApplicationCommandTarget.cpp"
#include "juce_appframework/application/juce_ApplicationProperties.cpp"
#include "juce_appframework/application/juce_DeletedAtShutdown.cpp"
#include "juce_appframework/application/juce_PropertiesFile.cpp"
#include "juce_appframework/audio/audio_file_formats/juce_AiffAudioFormat.cpp"
#include "juce_appframework/audio/audio_file_formats/juce_AudioCDReader.cpp"
#include "juce_appframework/audio/audio_file_formats/juce_AudioFormat.cpp"
#include "juce_appframework/audio/audio_file_formats/juce_AudioFormatManager.cpp"
#include "juce_appframework/audio/audio_file_formats/juce_AudioSubsectionReader.cpp"
#include "juce_appframework/audio/audio_file_formats/juce_AudioThumbnail.cpp"
#include "juce_appframework/audio/audio_file_formats/juce_AudioThumbnailCache.cpp"
#include "juce_appframework/audio/audio_file_formats/juce_QuickTimeAudioFormat.cpp"
#include "juce_appframework/audio/audio_file_formats/juce_WavAudioFormat.cpp"
#include "juce_appframework/audio/audio_sources/juce_AudioFormatReaderSource.cpp"
#include "juce_appframework/audio/audio_sources/juce_AudioSourcePlayer.cpp"
#include "juce_appframework/audio/audio_sources/juce_AudioTransportSource.cpp"
#include "juce_appframework/audio/audio_sources/juce_BufferingAudioSource.cpp"
#include "juce_appframework/audio/audio_sources/juce_ChannelRemappingAudioSource.cpp"
#include "juce_appframework/audio/audio_sources/juce_IIRFilterAudioSource.cpp"
#include "juce_appframework/audio/audio_sources/juce_MixerAudioSource.cpp"
#include "juce_appframework/audio/audio_sources/juce_ResamplingAudioSource.cpp"
#include "juce_appframework/audio/audio_sources/juce_ToneGeneratorAudioSource.cpp"
#include "juce_appframework/audio/devices/juce_AudioDeviceManager.cpp"
#include "juce_appframework/audio/devices/juce_AudioIODevice.cpp"
#include "juce_appframework/audio/devices/juce_AudioIODeviceType.cpp"
#include "juce_appframework/audio/devices/juce_MidiOutput.cpp"
#include "juce_appframework/audio/dsp/juce_AudioDataConverters.cpp"
#include "juce_appframework/audio/dsp/juce_AudioSampleBuffer.cpp"
#include "juce_appframework/audio/dsp/juce_IIRFilter.cpp"
#include "juce_appframework/audio/midi/juce_MidiBuffer.cpp"
#include "juce_appframework/audio/midi/juce_MidiFile.cpp"
#include "juce_appframework/audio/midi/juce_MidiKeyboardState.cpp"
#include "juce_appframework/audio/midi/juce_MidiMessage.cpp"
#include "juce_appframework/audio/midi/juce_MidiMessageCollector.cpp"
#include "juce_appframework/audio/midi/juce_MidiMessageSequence.cpp"
#include "juce_appframework/audio/plugins/juce_AudioPluginFormat.cpp"
#include "juce_appframework/audio/plugins/juce_AudioPluginFormatManager.cpp"
#include "juce_appframework/audio/plugins/juce_AudioPluginInstance.cpp"
#include "juce_appframework/audio/plugins/juce_KnownPluginList.cpp"
#include "juce_appframework/audio/plugins/juce_PluginDescription.cpp"
#include "juce_appframework/audio/plugins/juce_PluginDirectoryScanner.cpp"
#include "juce_appframework/audio/plugins/juce_PluginListComponent.cpp"
#include "juce_appframework/audio/plugins/formats/juce_AudioUnitPluginFormat.mm"
#include "juce_appframework/audio/plugins/formats/juce_VSTPluginFormat.mm"
#include "juce_appframework/audio/processors/juce_AudioProcessor.cpp"
#include "juce_appframework/audio/processors/juce_AudioProcessorEditor.cpp"
#include "juce_appframework/audio/processors/juce_AudioProcessorGraph.cpp"
#include "juce_appframework/audio/processors/juce_AudioProcessorPlayer.cpp"
#include "juce_appframework/audio/processors/juce_GenericAudioProcessorEditor.cpp"
#include "juce_appframework/audio/synthesisers/juce_Sampler.cpp"
#include "juce_appframework/audio/synthesisers/juce_Synthesiser.cpp"
#include "juce_appframework/documents/juce_FileBasedDocument.cpp"
#include "juce_appframework/documents/juce_RecentlyOpenedFilesList.cpp"
#include "juce_appframework/documents/juce_UndoManager.cpp"
#include "juce_appframework/events/juce_ActionBroadcaster.cpp"
#include "juce_appframework/events/juce_ActionListenerList.cpp"
#include "juce_appframework/events/juce_AsyncUpdater.cpp"
#include "juce_appframework/events/juce_ChangeBroadcaster.cpp"
#include "juce_appframework/events/juce_ChangeListenerList.cpp"
#include "juce_appframework/events/juce_InterprocessConnection.cpp"
#include "juce_appframework/events/juce_InterprocessConnectionServer.cpp"
#include "juce_appframework/events/juce_Message.cpp"
#include "juce_appframework/events/juce_MessageListener.cpp"
#include "juce_appframework/events/juce_MessageManager.cpp"
#include "juce_appframework/events/juce_MultiTimer.cpp"
#include "juce_appframework/events/juce_Timer.cpp"
#include "juce_appframework/gui/components/juce_Component.cpp"
#include "juce_appframework/gui/components/juce_ComponentListener.cpp"
#include "juce_appframework/gui/components/juce_Desktop.cpp"
#include "juce_appframework/gui/components/buttons/juce_ArrowButton.cpp"
#include "juce_appframework/gui/components/buttons/juce_Button.cpp"
#include "juce_appframework/gui/components/buttons/juce_DrawableButton.cpp"
#include "juce_appframework/gui/components/buttons/juce_HyperlinkButton.cpp"
#include "juce_appframework/gui/components/buttons/juce_ImageButton.cpp"
#include "juce_appframework/gui/components/buttons/juce_ShapeButton.cpp"
#include "juce_appframework/gui/components/buttons/juce_TextButton.cpp"
#include "juce_appframework/gui/components/buttons/juce_ToggleButton.cpp"
#include "juce_appframework/gui/components/buttons/juce_ToolbarButton.cpp"
#include "juce_appframework/gui/components/controls/juce_ComboBox.cpp"
#include "juce_appframework/gui/components/controls/juce_Label.cpp"
#include "juce_appframework/gui/components/controls/juce_ListBox.cpp"
#include "juce_appframework/gui/components/controls/juce_ProgressBar.cpp"
#include "juce_appframework/gui/components/controls/juce_Slider.cpp"
#include "juce_appframework/gui/components/controls/juce_TableHeaderComponent.cpp"
#include "juce_appframework/gui/components/controls/juce_TableListBox.cpp"
#include "juce_appframework/gui/components/controls/juce_TextEditor.cpp"
#include "juce_appframework/gui/components/controls/juce_Toolbar.cpp"
#include "juce_appframework/gui/components/controls/juce_ToolbarItemComponent.cpp"
#include "juce_appframework/gui/components/controls/juce_ToolbarItemPalette.cpp"
#include "juce_appframework/gui/components/controls/juce_TreeView.cpp"
#include "juce_appframework/gui/components/filebrowser/juce_DirectoryContentsDisplayComponent.cpp"
#include "juce_appframework/gui/components/filebrowser/juce_DirectoryContentsList.cpp"
#include "juce_appframework/gui/components/filebrowser/juce_FileBrowserComponent.cpp"
#include "juce_appframework/gui/components/filebrowser/juce_FileChooser.cpp"
#include "juce_appframework/gui/components/filebrowser/juce_FileChooserDialogBox.cpp"
#include "juce_appframework/gui/components/filebrowser/juce_FileFilter.cpp"
#include "juce_appframework/gui/components/filebrowser/juce_FileListComponent.cpp"
#include "juce_appframework/gui/components/filebrowser/juce_FilenameComponent.cpp"
#include "juce_appframework/gui/components/filebrowser/juce_FileSearchPathListComponent.cpp"
#include "juce_appframework/gui/components/filebrowser/juce_FileTreeComponent.cpp"
#include "juce_appframework/gui/components/filebrowser/juce_ImagePreviewComponent.cpp"
#include "juce_appframework/gui/components/filebrowser/juce_WildcardFileFilter.cpp"
#include "juce_appframework/gui/components/keyboard/juce_KeyboardFocusTraverser.cpp"
#include "juce_appframework/gui/components/keyboard/juce_KeyListener.cpp"
#include "juce_appframework/gui/components/keyboard/juce_KeyMappingEditorComponent.cpp"
#include "juce_appframework/gui/components/keyboard/juce_KeyPress.cpp"
#include "juce_appframework/gui/components/keyboard/juce_KeyPressMappingSet.cpp"
#include "juce_appframework/gui/components/keyboard/juce_ModifierKeys.cpp"
#include "juce_appframework/gui/components/layout/juce_ComponentAnimator.cpp"
#include "juce_appframework/gui/components/layout/juce_ComponentBoundsConstrainer.cpp"
#include "juce_appframework/gui/components/layout/juce_ComponentMovementWatcher.cpp"
#include "juce_appframework/gui/components/layout/juce_GroupComponent.cpp"
#include "juce_appframework/gui/components/layout/juce_MultiDocumentPanel.cpp"
#include "juce_appframework/gui/components/layout/juce_ResizableBorderComponent.cpp"
#include "juce_appframework/gui/components/layout/juce_ResizableCornerComponent.cpp"
#include "juce_appframework/gui/components/layout/juce_ScrollBar.cpp"
#include "juce_appframework/gui/components/layout/juce_StretchableLayoutManager.cpp"
#include "juce_appframework/gui/components/layout/juce_StretchableLayoutResizerBar.cpp"
#include "juce_appframework/gui/components/layout/juce_StretchableObjectResizer.cpp"
#include "juce_appframework/gui/components/layout/juce_TabbedButtonBar.cpp"
#include "juce_appframework/gui/components/layout/juce_TabbedComponent.cpp"
#include "juce_appframework/gui/components/layout/juce_Viewport.cpp"
#include "juce_appframework/gui/components/lookandfeel/juce_LookAndFeel.cpp"
#include "juce_appframework/gui/components/lookandfeel/juce_OldSchoolLookAndFeel.cpp"
#include "juce_appframework/gui/components/menus/juce_MenuBarComponent.cpp"
#include "juce_appframework/gui/components/menus/juce_MenuBarModel.cpp"
#include "juce_appframework/gui/components/menus/juce_PopupMenu.cpp"
#include "juce_appframework/gui/components/mouse/juce_ComponentDragger.cpp"
#include "juce_appframework/gui/components/mouse/juce_DragAndDropContainer.cpp"
#include "juce_appframework/gui/components/mouse/juce_MouseCursor.cpp"
#include "juce_appframework/gui/components/mouse/juce_MouseEvent.cpp"
#include "juce_appframework/gui/components/mouse/juce_MouseHoverDetector.cpp"
#include "juce_appframework/gui/components/mouse/juce_MouseListener.cpp"
#include "juce_appframework/gui/components/properties/juce_BooleanPropertyComponent.cpp"
#include "juce_appframework/gui/components/properties/juce_ButtonPropertyComponent.cpp"
#include "juce_appframework/gui/components/properties/juce_ChoicePropertyComponent.cpp"
#include "juce_appframework/gui/components/properties/juce_PropertyComponent.cpp"
#include "juce_appframework/gui/components/properties/juce_PropertyPanel.cpp"
#include "juce_appframework/gui/components/properties/juce_SliderPropertyComponent.cpp"
#include "juce_appframework/gui/components/properties/juce_TextPropertyComponent.cpp"
#include "juce_appframework/gui/components/special/juce_AudioDeviceSelectorComponent.cpp"
#include "juce_appframework/gui/components/special/juce_BubbleComponent.cpp"
#include "juce_appframework/gui/components/special/juce_BubbleMessageComponent.cpp"
#include "juce_appframework/gui/components/special/juce_ColourSelector.cpp"
#include "juce_appframework/gui/components/special/juce_DropShadower.cpp"
#include "juce_appframework/gui/components/special/juce_MagnifierComponent.cpp"
#include "juce_appframework/gui/components/special/juce_MidiKeyboardComponent.cpp"
#include "juce_appframework/gui/components/special/juce_OpenGLComponent.cpp"
#include "juce_appframework/gui/components/special/juce_PreferencesPanel.cpp"
#include "juce_appframework/gui/components/special/juce_SystemTrayIconComponent.cpp"
#include "juce_appframework/gui/components/windows/juce_AlertWindow.cpp"
#include "juce_appframework/gui/components/windows/juce_ComponentPeer.cpp"
#include "juce_appframework/gui/components/windows/juce_DialogWindow.cpp"
#include "juce_appframework/gui/components/windows/juce_DocumentWindow.cpp"
#include "juce_appframework/gui/components/windows/juce_ResizableWindow.cpp"
#include "juce_appframework/gui/components/windows/juce_SplashScreen.cpp"
#include "juce_appframework/gui/components/windows/juce_ThreadWithProgressWindow.cpp"
#include "juce_appframework/gui/components/windows/juce_TooltipWindow.cpp"
#include "juce_appframework/gui/components/windows/juce_TopLevelWindow.cpp"
#include "juce_appframework/gui/graphics/brushes/juce_Brush.cpp"
#include "juce_appframework/gui/graphics/brushes/juce_GradientBrush.cpp"
#include "juce_appframework/gui/graphics/brushes/juce_ImageBrush.cpp"
#include "juce_appframework/gui/graphics/brushes/juce_SolidColourBrush.cpp"
#include "juce_appframework/gui/graphics/colour/juce_Colour.cpp"
#include "juce_appframework/gui/graphics/colour/juce_ColourGradient.cpp"
#include "juce_appframework/gui/graphics/colour/juce_Colours.cpp"
#include "juce_appframework/gui/graphics/contexts/juce_EdgeTable.cpp"
#include "juce_appframework/gui/graphics/contexts/juce_Graphics.cpp"
#include "juce_appframework/gui/graphics/contexts/juce_Justification.cpp"
#include "juce_appframework/gui/graphics/contexts/juce_LowLevelGraphicsPostScriptRenderer.cpp"
#include "juce_appframework/gui/graphics/contexts/juce_LowLevelGraphicsSoftwareRenderer.cpp"
#include "juce_appframework/gui/graphics/contexts/juce_RectanglePlacement.cpp"
#include "juce_appframework/gui/graphics/drawables/juce_Drawable.cpp"
#include "juce_appframework/gui/graphics/drawables/juce_DrawableComposite.cpp"
#include "juce_appframework/gui/graphics/drawables/juce_DrawableImage.cpp"
#include "juce_appframework/gui/graphics/drawables/juce_DrawablePath.cpp"
#include "juce_appframework/gui/graphics/drawables/juce_DrawableText.cpp"
#include "juce_appframework/gui/graphics/drawables/juce_SVGParser.cpp"
#include "juce_appframework/gui/graphics/effects/juce_DropShadowEffect.cpp"
#include "juce_appframework/gui/graphics/effects/juce_GlowEffect.cpp"
#include "juce_appframework/gui/graphics/effects/juce_ReduceOpacityEffect.cpp"
#include "juce_appframework/gui/graphics/fonts/juce_Font.cpp"
#include "juce_appframework/gui/graphics/fonts/juce_GlyphArrangement.cpp"
#include "juce_appframework/gui/graphics/fonts/juce_TextLayout.cpp"
#include "juce_appframework/gui/graphics/fonts/juce_Typeface.cpp"
#include "juce_appframework/gui/graphics/geometry/juce_AffineTransform.cpp"
#include "juce_appframework/gui/graphics/geometry/juce_BorderSize.cpp"
#include "juce_appframework/gui/graphics/geometry/juce_Line.cpp"
#include "juce_appframework/gui/graphics/geometry/juce_Path.cpp"
#include "juce_appframework/gui/graphics/geometry/juce_PathIterator.cpp"
#include "juce_appframework/gui/graphics/geometry/juce_PathStrokeType.cpp"
#include "juce_appframework/gui/graphics/geometry/juce_Point.cpp"
#include "juce_appframework/gui/graphics/geometry/juce_PositionedRectangle.cpp"
#include "juce_appframework/gui/graphics/geometry/juce_Rectangle.cpp"
#include "juce_appframework/gui/graphics/geometry/juce_RectangleList.cpp"
#include "juce_appframework/gui/graphics/imaging/juce_Image.cpp"
#include "juce_appframework/gui/graphics/imaging/juce_ImageCache.cpp"
#include "juce_appframework/gui/graphics/imaging/juce_ImageConvolutionKernel.cpp"
#include "juce_appframework/gui/graphics/imaging/juce_ImageFileFormat.cpp"
#include "juce_appframework/gui/graphics/imaging/image_file_formats/juce_GIFLoader.cpp"

#endif

//==============================================================================
// some files include lots of library code, so leave them to the end to avoid cluttering
// up the build for the clean files.
#include "juce_core/io/streams/juce_GZIPCompressorOutputStream.cpp"
#include "juce_core/io/streams/juce_GZIPDecompressorInputStream.cpp"

#if ! JUCE_ONLY_BUILD_CORE_LIBRARY
#include "juce_appframework/audio/audio_file_formats/juce_FlacAudioFormat.cpp"
#include "juce_appframework/audio/audio_file_formats/juce_OggVorbisAudioFormat.cpp"
#include "juce_appframework/gui/graphics/imaging/image_file_formats/juce_JPEGLoader.cpp"
#include "juce_appframework/gui/graphics/imaging/image_file_formats/juce_PNGLoader.cpp"
#endif

//==============================================================================
#if JUCE_WIN32

#include "../build/win32/platform_specific_code/juce_win32_NativeCode.cpp"
#include "../build/win32/platform_specific_code/juce_win32_AutoLinkLibraries.h"

#endif

//==============================================================================
#if JUCE_LINUX

#include "../build/linux/platform_specific_code/juce_linux_Files.cpp"
#include "../build/linux/platform_specific_code/juce_linux_NamedPipe.cpp"
#include "../build/linux/platform_specific_code/juce_linux_Network.cpp"
#include "../build/linux/platform_specific_code/juce_linux_SystemStats.cpp"
#include "../build/linux/platform_specific_code/juce_linux_Threads.cpp"

#if ! JUCE_ONLY_BUILD_CORE_LIBRARY
 #include "../build/linux/platform_specific_code/juce_linux_Audio.cpp"
 #include "../build/linux/platform_specific_code/juce_linux_AudioCDReader.cpp"
 #include "../build/linux/platform_specific_code/juce_linux_FileChooser.cpp"
 #include "../build/linux/platform_specific_code/juce_linux_Fonts.cpp"
 #include "../build/linux/platform_specific_code/juce_linux_Messaging.cpp"
 #include "../build/linux/platform_specific_code/juce_linux_Midi.cpp"
 #include "../build/linux/platform_specific_code/juce_linux_WebBrowserComponent.cpp"
 #include "../build/linux/platform_specific_code/juce_linux_Windowing.cpp"
#endif

#endif

//==============================================================================
#if JUCE_MAC

#include "../build/macosx/platform_specific_code/juce_mac_NativeCode.mm"
#include "../build/macosx/platform_specific_code/juce_mac_NamedPipe.cpp"

#endif
