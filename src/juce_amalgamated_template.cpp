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

#include "core/juce_TargetPlatform.h" // FORCE_AMALGAMATOR_INCLUDE
#include "../juce_Config.h"  // FORCE_AMALGAMATOR_INCLUDE

//==============================================================================
#if JUCE_WINDOWS
 #include "native/windows/juce_win32_NativeIncludes.h"
#elif JUCE_LINUX
 #include "native/linux/juce_linux_NativeIncludes.h"
#elif JUCE_MAC || JUCE_IPHONE
 #include "native/mac/juce_mac_NativeIncludes.h"
#else
 #error "Unknown platform!"
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
 #include "native/mac/juce_mac_CarbonViewWrapperComponent.h"
 END_JUCE_NAMESPACE
#endif

#define JUCE_AMALGAMATED_TEMPLATE 1

//==============================================================================
#include "core/juce_FileLogger.cpp"
#include "core/juce_Logger.cpp"
#include "core/juce_Random.cpp"
#include "core/juce_RelativeTime.cpp"
#include "core/juce_SystemStats.cpp"
#include "core/juce_Time.cpp"
#include "containers/juce_BitArray.cpp"
#include "containers/juce_MemoryBlock.cpp"
#include "containers/juce_PropertySet.cpp"
#include "containers/juce_Variant.cpp"
#include "cryptography/juce_BlowFish.cpp"
#include "cryptography/juce_MD5.cpp"
#include "cryptography/juce_Primes.cpp"
#include "cryptography/juce_RSAKey.cpp"
#include "io/streams/juce_InputStream.cpp"
#include "io/streams/juce_OutputStream.cpp"
#include "io/files/juce_DirectoryIterator.cpp"
#include "io/files/juce_File.cpp"
#include "io/files/juce_FileInputStream.cpp"
#include "io/files/juce_FileOutputStream.cpp"
#include "io/files/juce_FileSearchPath.cpp"
#include "io/files/juce_NamedPipe.cpp"
#include "io/network/juce_Socket.cpp"
#include "io/network/juce_URL.cpp"
#include "io/streams/juce_BufferedInputStream.cpp"
#include "io/streams/juce_FileInputSource.cpp"
#include "io/streams/juce_MemoryInputStream.cpp"
#include "io/streams/juce_MemoryOutputStream.cpp"
#include "io/streams/juce_SubregionStream.cpp"
#include "core/juce_PerformanceCounter.cpp"
#include "core/juce_Uuid.cpp"
#include "io/files/juce_ZipFile.cpp"
#include "text/juce_CharacterFunctions.cpp"
#include "text/juce_LocalisedStrings.cpp"
#include "text/juce_String.cpp"
#include "text/juce_StringArray.cpp"
#include "text/juce_StringPairArray.cpp"
#include "text/juce_XmlDocument.cpp"
#include "text/juce_XmlElement.cpp"
#include "threads/juce_InterProcessLock.cpp"
#include "threads/juce_ReadWriteLock.cpp"
#include "threads/juce_Thread.cpp"
#include "threads/juce_ThreadPool.cpp"
#include "threads/juce_TimeSliceThread.cpp"

#if ! JUCE_ONLY_BUILD_CORE_LIBRARY

#include "application/juce_Application.cpp"
#include "application/juce_ApplicationCommandInfo.cpp"
#include "application/juce_ApplicationCommandManager.cpp"
#include "application/juce_ApplicationCommandTarget.cpp"
#include "application/juce_ApplicationProperties.cpp"
#include "utilities/juce_DeletedAtShutdown.cpp"
#include "utilities/juce_PropertiesFile.cpp"
#include "utilities/juce_FileBasedDocument.cpp"
#include "utilities/juce_RecentlyOpenedFilesList.cpp"
#include "utilities/juce_UndoManager.cpp"
#include "audio/audio_file_formats/juce_AiffAudioFormat.cpp"
#include "audio/audio_file_formats/juce_AudioCDReader.cpp"
#include "audio/audio_file_formats/juce_AudioFormat.cpp"
#include "audio/audio_file_formats/juce_AudioFormatManager.cpp"
#include "audio/audio_file_formats/juce_AudioSubsectionReader.cpp"
#include "audio/audio_file_formats/juce_AudioThumbnail.cpp"
#include "audio/audio_file_formats/juce_AudioThumbnailCache.cpp"
#include "audio/audio_file_formats/juce_QuickTimeAudioFormat.cpp"
#include "audio/audio_file_formats/juce_WavAudioFormat.cpp"
#include "audio/audio_sources/juce_AudioFormatReaderSource.cpp"
#include "audio/audio_sources/juce_AudioSourcePlayer.cpp"
#include "audio/audio_sources/juce_AudioTransportSource.cpp"
#include "audio/audio_sources/juce_BufferingAudioSource.cpp"
#include "audio/audio_sources/juce_ChannelRemappingAudioSource.cpp"
#include "audio/audio_sources/juce_IIRFilterAudioSource.cpp"
#include "audio/audio_sources/juce_MixerAudioSource.cpp"
#include "audio/audio_sources/juce_ResamplingAudioSource.cpp"
#include "audio/audio_sources/juce_ToneGeneratorAudioSource.cpp"
#include "audio/devices/juce_AudioDeviceManager.cpp"
#include "audio/devices/juce_AudioIODevice.cpp"
#include "audio/devices/juce_AudioIODeviceType.cpp"
#include "audio/devices/juce_MidiOutput.cpp"
#include "audio/dsp/juce_AudioDataConverters.cpp"
#include "audio/dsp/juce_AudioSampleBuffer.cpp"
#include "audio/dsp/juce_IIRFilter.cpp"
#include "audio/midi/juce_MidiBuffer.cpp"
#include "audio/midi/juce_MidiFile.cpp"
#include "audio/midi/juce_MidiKeyboardState.cpp"
#include "audio/midi/juce_MidiMessage.cpp"
#include "audio/midi/juce_MidiMessageCollector.cpp"
#include "audio/midi/juce_MidiMessageSequence.cpp"
#include "audio/plugins/juce_AudioPluginFormat.cpp"
#include "audio/plugins/juce_AudioPluginFormatManager.cpp"
#include "audio/plugins/juce_AudioPluginInstance.cpp"
#include "audio/plugins/juce_KnownPluginList.cpp"
#include "audio/plugins/juce_PluginDescription.cpp"
#include "audio/plugins/juce_PluginDirectoryScanner.cpp"
#include "audio/plugins/juce_PluginListComponent.cpp"
#include "audio/plugins/formats/juce_AudioUnitPluginFormat.mm"
#include "audio/plugins/formats/juce_VSTPluginFormat.mm"
#include "audio/processors/juce_AudioProcessor.cpp"
#include "audio/processors/juce_AudioProcessorEditor.cpp"
#include "audio/processors/juce_AudioProcessorGraph.cpp"
#include "audio/processors/juce_AudioProcessorPlayer.cpp"
#include "audio/processors/juce_GenericAudioProcessorEditor.cpp"
#include "audio/synthesisers/juce_Sampler.cpp"
#include "audio/synthesisers/juce_Synthesiser.cpp"
#include "events/juce_ActionBroadcaster.cpp"
#include "events/juce_ActionListenerList.cpp"
#include "events/juce_AsyncUpdater.cpp"
#include "events/juce_ChangeBroadcaster.cpp"
#include "events/juce_ChangeListenerList.cpp"
#include "events/juce_InterprocessConnection.cpp"
#include "events/juce_InterprocessConnectionServer.cpp"
#include "events/juce_Message.cpp"
#include "events/juce_MessageListener.cpp"
#include "events/juce_MessageManager.cpp"
#include "events/juce_MultiTimer.cpp"
#include "events/juce_Timer.cpp"
#include "gui/components/juce_Component.cpp"
#include "gui/components/juce_ComponentListener.cpp"
#include "gui/components/juce_Desktop.cpp"
#include "gui/components/buttons/juce_ArrowButton.cpp"
#include "gui/components/buttons/juce_Button.cpp"
#include "gui/components/buttons/juce_DrawableButton.cpp"
#include "gui/components/buttons/juce_HyperlinkButton.cpp"
#include "gui/components/buttons/juce_ImageButton.cpp"
#include "gui/components/buttons/juce_ShapeButton.cpp"
#include "gui/components/buttons/juce_TextButton.cpp"
#include "gui/components/buttons/juce_ToggleButton.cpp"
#include "gui/components/buttons/juce_ToolbarButton.cpp"
#include "gui/components/code_editor/juce_CodeDocument.cpp"
#include "gui/components/code_editor/juce_CodeEditorComponent.cpp"
#include "gui/components/code_editor/juce_CPlusPlusCodeTokeniser.cpp"
#include "gui/components/controls/juce_ComboBox.cpp"
#include "gui/components/controls/juce_Label.cpp"
#include "gui/components/controls/juce_ListBox.cpp"
#include "gui/components/controls/juce_ProgressBar.cpp"
#include "gui/components/controls/juce_Slider.cpp"
#include "gui/components/controls/juce_TableHeaderComponent.cpp"
#include "gui/components/controls/juce_TableListBox.cpp"
#include "gui/components/controls/juce_TextEditor.cpp"
#include "gui/components/controls/juce_Toolbar.cpp"
#include "gui/components/controls/juce_ToolbarItemComponent.cpp"
#include "gui/components/controls/juce_ToolbarItemPalette.cpp"
#include "gui/components/controls/juce_TreeView.cpp"
#include "gui/components/filebrowser/juce_DirectoryContentsDisplayComponent.cpp"
#include "gui/components/filebrowser/juce_DirectoryContentsList.cpp"
#include "gui/components/filebrowser/juce_FileBrowserComponent.cpp"
#include "gui/components/filebrowser/juce_FileChooser.cpp"
#include "gui/components/filebrowser/juce_FileChooserDialogBox.cpp"
#include "gui/components/filebrowser/juce_FileFilter.cpp"
#include "gui/components/filebrowser/juce_FileListComponent.cpp"
#include "gui/components/filebrowser/juce_FilenameComponent.cpp"
#include "gui/components/filebrowser/juce_FileSearchPathListComponent.cpp"
#include "gui/components/filebrowser/juce_FileTreeComponent.cpp"
#include "gui/components/filebrowser/juce_ImagePreviewComponent.cpp"
#include "gui/components/filebrowser/juce_WildcardFileFilter.cpp"
#include "gui/components/keyboard/juce_KeyboardFocusTraverser.cpp"
#include "gui/components/keyboard/juce_KeyListener.cpp"
#include "gui/components/keyboard/juce_KeyMappingEditorComponent.cpp"
#include "gui/components/keyboard/juce_KeyPress.cpp"
#include "gui/components/keyboard/juce_KeyPressMappingSet.cpp"
#include "gui/components/keyboard/juce_ModifierKeys.cpp"
#include "gui/components/layout/juce_ComponentAnimator.cpp"
#include "gui/components/layout/juce_ComponentBoundsConstrainer.cpp"
#include "gui/components/layout/juce_ComponentMovementWatcher.cpp"
#include "gui/components/layout/juce_GroupComponent.cpp"
#include "gui/components/layout/juce_MultiDocumentPanel.cpp"
#include "gui/components/layout/juce_ResizableBorderComponent.cpp"
#include "gui/components/layout/juce_ResizableCornerComponent.cpp"
#include "gui/components/layout/juce_ScrollBar.cpp"
#include "gui/components/layout/juce_StretchableLayoutManager.cpp"
#include "gui/components/layout/juce_StretchableLayoutResizerBar.cpp"
#include "gui/components/layout/juce_StretchableObjectResizer.cpp"
#include "gui/components/layout/juce_TabbedButtonBar.cpp"
#include "gui/components/layout/juce_TabbedComponent.cpp"
#include "gui/components/layout/juce_Viewport.cpp"
#include "gui/components/lookandfeel/juce_LookAndFeel.cpp"
#include "gui/components/lookandfeel/juce_OldSchoolLookAndFeel.cpp"
#include "gui/components/menus/juce_MenuBarComponent.cpp"
#include "gui/components/menus/juce_MenuBarModel.cpp"
#include "gui/components/menus/juce_PopupMenu.cpp"
#include "gui/components/mouse/juce_ComponentDragger.cpp"
#include "gui/components/mouse/juce_DragAndDropContainer.cpp"
#include "gui/components/mouse/juce_MouseCursor.cpp"
#include "gui/components/mouse/juce_MouseEvent.cpp"
#include "gui/components/mouse/juce_MouseHoverDetector.cpp"
#include "gui/components/mouse/juce_MouseListener.cpp"
#include "gui/components/properties/juce_BooleanPropertyComponent.cpp"
#include "gui/components/properties/juce_ButtonPropertyComponent.cpp"
#include "gui/components/properties/juce_ChoicePropertyComponent.cpp"
#include "gui/components/properties/juce_PropertyComponent.cpp"
#include "gui/components/properties/juce_PropertyPanel.cpp"
#include "gui/components/properties/juce_SliderPropertyComponent.cpp"
#include "gui/components/properties/juce_TextPropertyComponent.cpp"
#include "gui/components/special/juce_AudioDeviceSelectorComponent.cpp"
#include "gui/components/special/juce_BubbleComponent.cpp"
#include "gui/components/special/juce_BubbleMessageComponent.cpp"
#include "gui/components/special/juce_ColourSelector.cpp"
#include "gui/components/special/juce_DropShadower.cpp"
#include "gui/components/special/juce_MagnifierComponent.cpp"
#include "gui/components/special/juce_MidiKeyboardComponent.cpp"
#include "gui/components/special/juce_OpenGLComponent.cpp"
#include "gui/components/special/juce_PreferencesPanel.cpp"
#include "gui/components/special/juce_SystemTrayIconComponent.cpp"
#include "gui/components/windows/juce_AlertWindow.cpp"
#include "gui/components/windows/juce_ComponentPeer.cpp"
#include "gui/components/windows/juce_DialogWindow.cpp"
#include "gui/components/windows/juce_DocumentWindow.cpp"
#include "gui/components/windows/juce_ResizableWindow.cpp"
#include "gui/components/windows/juce_SplashScreen.cpp"
#include "gui/components/windows/juce_ThreadWithProgressWindow.cpp"
#include "gui/components/windows/juce_TooltipWindow.cpp"
#include "gui/components/windows/juce_TopLevelWindow.cpp"
#include "gui/graphics/colour/juce_Colour.cpp"
#include "gui/graphics/colour/juce_ColourGradient.cpp"
#include "gui/graphics/colour/juce_Colours.cpp"
#include "gui/graphics/contexts/juce_EdgeTable.cpp"
#include "gui/graphics/contexts/juce_FillType.cpp"
#include "gui/graphics/contexts/juce_Graphics.cpp"
#include "gui/graphics/contexts/juce_Justification.cpp"
#include "gui/graphics/contexts/juce_LowLevelGraphicsPostScriptRenderer.cpp"
#include "gui/graphics/contexts/juce_LowLevelGraphicsSoftwareRenderer.cpp"
#include "gui/graphics/contexts/juce_RectanglePlacement.cpp"
#include "gui/graphics/drawables/juce_Drawable.cpp"
#include "gui/graphics/drawables/juce_DrawableComposite.cpp"
#include "gui/graphics/drawables/juce_DrawableImage.cpp"
#include "gui/graphics/drawables/juce_DrawablePath.cpp"
#include "gui/graphics/drawables/juce_DrawableText.cpp"
#include "gui/graphics/drawables/juce_SVGParser.cpp"
#include "gui/graphics/effects/juce_DropShadowEffect.cpp"
#include "gui/graphics/effects/juce_GlowEffect.cpp"
#include "gui/graphics/effects/juce_ReduceOpacityEffect.cpp"
#include "gui/graphics/fonts/juce_Font.cpp"
#include "gui/graphics/fonts/juce_GlyphArrangement.cpp"
#include "gui/graphics/fonts/juce_TextLayout.cpp"
#include "gui/graphics/fonts/juce_Typeface.cpp"
#include "gui/graphics/geometry/juce_AffineTransform.cpp"
#include "gui/graphics/geometry/juce_BorderSize.cpp"
#include "gui/graphics/geometry/juce_Line.cpp"
#include "gui/graphics/geometry/juce_Path.cpp"
#include "gui/graphics/geometry/juce_PathIterator.cpp"
#include "gui/graphics/geometry/juce_PathStrokeType.cpp"
#include "gui/graphics/geometry/juce_Point.cpp"
#include "gui/graphics/geometry/juce_PositionedRectangle.cpp"
#include "gui/graphics/geometry/juce_Rectangle.cpp"
#include "gui/graphics/geometry/juce_RectangleList.cpp"
#include "gui/graphics/imaging/juce_Image.cpp"
#include "gui/graphics/imaging/juce_ImageCache.cpp"
#include "gui/graphics/imaging/juce_ImageConvolutionKernel.cpp"
#include "gui/graphics/imaging/juce_ImageFileFormat.cpp"
#include "gui/graphics/imaging/image_file_formats/juce_GIFLoader.cpp"

#endif

//==============================================================================
// some files include lots of library code, so leave them to the end to avoid cluttering
// up the build for the clean files.
#include "io/streams/juce_GZIPCompressorOutputStream.cpp"
#include "io/streams/juce_GZIPDecompressorInputStream.cpp"

#if ! JUCE_ONLY_BUILD_CORE_LIBRARY
 #include "audio/audio_file_formats/juce_FlacAudioFormat.cpp"
 #include "audio/audio_file_formats/juce_OggVorbisAudioFormat.cpp"
 #include "gui/graphics/imaging/image_file_formats/juce_JPEGLoader.cpp"
 #include "gui/graphics/imaging/image_file_formats/juce_PNGLoader.cpp"
#endif

//==============================================================================
#if JUCE_WINDOWS
 #include "native/juce_win32_NativeCode.cpp"
#endif

#if JUCE_LINUX
 #include "native/juce_linux_NativeCode.cpp"
#endif

#if JUCE_MAC || JUCE_IPHONE
 #include "native/juce_mac_NativeCode.mm"
#endif
