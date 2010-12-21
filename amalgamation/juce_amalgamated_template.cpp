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

#include "../src/core/juce_TargetPlatform.h" // FORCE_AMALGAMATOR_INCLUDE
#include "../juce_Config.h"  // FORCE_AMALGAMATOR_INCLUDE

#ifndef JUCE_BUILD_CORE
 #define JUCE_BUILD_CORE 1
#endif
#ifndef JUCE_BUILD_MISC
 #define JUCE_BUILD_MISC 1
#endif
#ifndef JUCE_BUILD_GUI
 #define JUCE_BUILD_GUI 1
#endif
#ifndef JUCE_BUILD_NATIVE
 #define JUCE_BUILD_NATIVE 1
#endif

#if JUCE_ONLY_BUILD_CORE_LIBRARY
 #undef JUCE_BUILD_MISC
 #undef JUCE_BUILD_GUI
#endif

//==============================================================================
#if JUCE_BUILD_NATIVE || JUCE_BUILD_CORE || (JUCE_BUILD_MISC && (JUCE_PLUGINHOST_VST || JUCE_PLUGINHOST_AU))
 #if JUCE_WINDOWS
  #include "../src/native/windows/juce_win32_NativeIncludes.h"
 #elif JUCE_LINUX
  #include "../src/native/linux/juce_linux_NativeIncludes.h"
 #elif JUCE_MAC || JUCE_IPHONE
  #include "../src/native/mac/juce_mac_NativeIncludes.h"
 #else
  #error "Unknown platform!"
 #endif
#endif

//==============================================================================
#define DONT_SET_USING_JUCE_NAMESPACE 1
#undef max
#undef min
#define NO_DUMMY_DECL

#if JUCE_BUILD_NATIVE
 #include "../juce_amalgamated.h"  // FORCE_AMALGAMATOR_INCLUDE
#endif

#if (defined(_MSC_VER) && (_MSC_VER <= 1200))
 #pragma warning (disable: 4309 4305)
#endif

#if JUCE_MAC && JUCE_32BIT && JUCE_SUPPORT_CARBON && JUCE_BUILD_NATIVE && ! JUCE_ONLY_BUILD_CORE_LIBRARY
 BEGIN_JUCE_NAMESPACE
 #include "../src/native/mac/juce_mac_CarbonViewWrapperComponent.h"
 END_JUCE_NAMESPACE
#endif

#define JUCE_AMALGAMATED_TEMPLATE 1

//==============================================================================
#if JUCE_BUILD_CORE
 #include "../src/core/juce_FileLogger.cpp"
 #include "../src/core/juce_Logger.cpp"
 #include "../src/maths/juce_Random.cpp"
 #include "../src/core/juce_RelativeTime.cpp"
 #include "../src/core/juce_SystemStats.cpp"
 #include "../src/core/juce_Time.cpp"
 #include "../src/core/juce_Initialisation.cpp"
 #include "../src/containers/juce_AbstractFifo.cpp"
 #include "../src/maths/juce_BigInteger.cpp"
 #include "../src/memory/juce_MemoryBlock.cpp"
 #include "../src/containers/juce_PropertySet.cpp"
 #include "../src/text/juce_Identifier.cpp"
 #include "../src/containers/juce_Variant.cpp"
 #include "../src/containers/juce_NamedValueSet.cpp"
 #include "../src/containers/juce_DynamicObject.cpp"
 #include "../src/maths/juce_Expression.cpp"
 #include "../src/cryptography/juce_BlowFish.cpp"
 #include "../src/cryptography/juce_MD5.cpp"
 #include "../src/cryptography/juce_Primes.cpp"
 #include "../src/cryptography/juce_RSAKey.cpp"
 #include "../src/io/streams/juce_InputStream.cpp"
 #include "../src/io/streams/juce_OutputStream.cpp"
 #include "../src/io/files/juce_DirectoryIterator.cpp"
 #include "../src/io/files/juce_File.cpp"
 #include "../src/io/files/juce_FileInputStream.cpp"
 #include "../src/io/files/juce_FileOutputStream.cpp"
 #include "../src/io/files/juce_FileSearchPath.cpp"
 #include "../src/io/files/juce_NamedPipe.cpp"
 #include "../src/io/files/juce_TemporaryFile.cpp"
 #include "../src/io/network/juce_Socket.cpp"
 #include "../src/io/network/juce_URL.cpp"
 #include "../src/io/network/juce_MACAddress.cpp"
 #include "../src/io/streams/juce_BufferedInputStream.cpp"
 #include "../src/io/streams/juce_FileInputSource.cpp"
 #include "../src/io/streams/juce_MemoryInputStream.cpp"
 #include "../src/io/streams/juce_MemoryOutputStream.cpp"
 #include "../src/io/streams/juce_SubregionStream.cpp"
 #include "../src/core/juce_PerformanceCounter.cpp"
 #include "../src/core/juce_Uuid.cpp"
 #include "../src/io/files/juce_ZipFile.cpp"
 #include "../src/text/juce_CharacterFunctions.cpp"
 #include "../src/text/juce_LocalisedStrings.cpp"
 #include "../src/text/juce_String.cpp"
 #include "../src/text/juce_StringArray.cpp"
 #include "../src/text/juce_StringPairArray.cpp"
 #include "../src/text/juce_StringPool.cpp"
 #include "../src/text/juce_XmlDocument.cpp"
 #include "../src/text/juce_XmlElement.cpp"
 #include "../src/threads/juce_ReadWriteLock.cpp"
 #include "../src/threads/juce_Thread.cpp"
 #include "../src/threads/juce_ThreadPool.cpp"
 #include "../src/threads/juce_TimeSliceThread.cpp"
 #include "../src/utilities/juce_DeletedAtShutdown.cpp"
 #include "../src/utilities/juce_UnitTest.cpp"
#endif

#if JUCE_BUILD_MISC
 #include "../src/containers/juce_ValueTree.cpp"
 #include "../src/containers/juce_Value.cpp"
 #include "../src/application/juce_Application.cpp"
 #include "../src/application/juce_ApplicationCommandInfo.cpp"
 #include "../src/application/juce_ApplicationCommandManager.cpp"
 #include "../src/application/juce_ApplicationCommandTarget.cpp"
 #include "../src/application/juce_ApplicationProperties.cpp"
 #include "../src/utilities/juce_PropertiesFile.cpp"
 #include "../src/utilities/juce_FileBasedDocument.cpp"
 #include "../src/utilities/juce_RecentlyOpenedFilesList.cpp"
 #include "../src/utilities/juce_UndoManager.cpp"
 #include "../src/audio/audio_file_formats/juce_AiffAudioFormat.cpp"
 #include "../src/audio/audio_file_formats/juce_AudioFormat.cpp"
 #include "../src/audio/audio_file_formats/juce_AudioFormatReader.cpp"
 #include "../src/audio/audio_file_formats/juce_AudioFormatWriter.cpp"
 #include "../src/audio/audio_file_formats/juce_AudioFormatManager.cpp"
 #include "../src/audio/audio_file_formats/juce_AudioSubsectionReader.cpp"
 #include "../src/audio/audio_file_formats/juce_AudioThumbnail.cpp"
 #include "../src/audio/audio_file_formats/juce_AudioThumbnailCache.cpp"
 #include "../src/audio/audio_file_formats/juce_QuickTimeAudioFormat.cpp"
 #include "../src/audio/audio_file_formats/juce_WavAudioFormat.cpp"
 #include "../src/audio/audio_file_formats/juce_AudioCDReader.cpp"
 #include "../src/audio/audio_sources/juce_AudioFormatReaderSource.cpp"
 #include "../src/audio/audio_sources/juce_AudioSourcePlayer.cpp"
 #include "../src/audio/audio_sources/juce_AudioTransportSource.cpp"
 #include "../src/audio/audio_sources/juce_BufferingAudioSource.cpp"
 #include "../src/audio/audio_sources/juce_ChannelRemappingAudioSource.cpp"
 #include "../src/audio/audio_sources/juce_IIRFilterAudioSource.cpp"
 #include "../src/audio/audio_sources/juce_MixerAudioSource.cpp"
 #include "../src/audio/audio_sources/juce_ResamplingAudioSource.cpp"
 #include "../src/audio/audio_sources/juce_ToneGeneratorAudioSource.cpp"
 #include "../src/audio/devices/juce_AudioDeviceManager.cpp"
 #include "../src/audio/devices/juce_AudioIODevice.cpp"
 #include "../src/audio/devices/juce_AudioIODeviceType.cpp"
 #include "../src/audio/devices/juce_MidiOutput.cpp"
 #include "../src/audio/dsp/juce_AudioDataConverters.cpp"
 #include "../src/audio/dsp/juce_AudioSampleBuffer.cpp"
 #include "../src/audio/dsp/juce_IIRFilter.cpp"
 #include "../src/audio/midi/juce_MidiBuffer.cpp"
 #include "../src/audio/midi/juce_MidiFile.cpp"
 #include "../src/audio/midi/juce_MidiKeyboardState.cpp"
 #include "../src/audio/midi/juce_MidiMessage.cpp"
 #include "../src/audio/midi/juce_MidiMessageCollector.cpp"
 #include "../src/audio/midi/juce_MidiMessageSequence.cpp"
 #include "../src/audio/plugins/juce_AudioPluginFormat.cpp"
 #include "../src/audio/plugins/juce_AudioPluginFormatManager.cpp"
 #include "../src/audio/plugins/juce_AudioPluginInstance.cpp"
 #include "../src/audio/plugins/juce_KnownPluginList.cpp"
 #include "../src/audio/plugins/juce_PluginDescription.cpp"
 #include "../src/audio/plugins/juce_PluginDirectoryScanner.cpp"
 #include "../src/audio/plugins/juce_PluginListComponent.cpp"
 #include "../src/audio/plugins/formats/juce_AudioUnitPluginFormat.mm"
 #include "../src/audio/plugins/formats/juce_VSTPluginFormat.mm"
 #include "../src/audio/processors/juce_AudioProcessor.cpp"
 #include "../src/audio/processors/juce_AudioProcessorEditor.cpp"
 #include "../src/audio/processors/juce_AudioProcessorGraph.cpp"
 #include "../src/audio/processors/juce_AudioProcessorPlayer.cpp"
 #include "../src/audio/processors/juce_GenericAudioProcessorEditor.cpp"
 #include "../src/audio/synthesisers/juce_Sampler.cpp"
 #include "../src/audio/synthesisers/juce_Synthesiser.cpp"
 #include "../src/events/juce_ActionBroadcaster.cpp"
 #include "../src/events/juce_AsyncUpdater.cpp"
 #include "../src/events/juce_ChangeBroadcaster.cpp"
 #include "../src/events/juce_InterprocessConnection.cpp"
 #include "../src/events/juce_InterprocessConnectionServer.cpp"
 #include "../src/events/juce_Message.cpp"
 #include "../src/events/juce_MessageListener.cpp"
 #include "../src/events/juce_MessageManager.cpp"
 #include "../src/events/juce_MultiTimer.cpp"
 #include "../src/events/juce_Timer.cpp"
#endif

#if JUCE_BUILD_GUI
 #include "../src/gui/components/juce_Component.cpp"
 #include "../src/gui/components/juce_ComponentListener.cpp"
 #include "../src/gui/components/juce_Desktop.cpp"
 #include "../src/gui/components/juce_ModalComponentManager.cpp"
 #include "../src/gui/components/buttons/juce_ArrowButton.cpp"
 #include "../src/gui/components/buttons/juce_Button.cpp"
 #include "../src/gui/components/buttons/juce_DrawableButton.cpp"
 #include "../src/gui/components/buttons/juce_HyperlinkButton.cpp"
 #include "../src/gui/components/buttons/juce_ImageButton.cpp"
 #include "../src/gui/components/buttons/juce_ShapeButton.cpp"
 #include "../src/gui/components/buttons/juce_TextButton.cpp"
 #include "../src/gui/components/buttons/juce_ToggleButton.cpp"
 #include "../src/gui/components/buttons/juce_ToolbarButton.cpp"
 #include "../src/gui/components/code_editor/juce_CodeDocument.cpp"
 #include "../src/gui/components/code_editor/juce_CodeEditorComponent.cpp"
 #include "../src/gui/components/code_editor/juce_CPlusPlusCodeTokeniser.cpp"
 #include "../src/gui/components/controls/juce_ComboBox.cpp"
 #include "../src/gui/components/controls/juce_Label.cpp"
 #include "../src/gui/components/controls/juce_ListBox.cpp"
 #include "../src/gui/components/controls/juce_ProgressBar.cpp"
 #include "../src/gui/components/controls/juce_Slider.cpp"
 #include "../src/gui/components/controls/juce_TableHeaderComponent.cpp"
 #include "../src/gui/components/controls/juce_TableListBox.cpp"
 #include "../src/gui/components/controls/juce_TextEditor.cpp"
 #include "../src/gui/components/controls/juce_Toolbar.cpp"
 #include "../src/gui/components/controls/juce_ToolbarItemComponent.cpp"
 #include "../src/gui/components/controls/juce_ToolbarItemPalette.cpp"
 #include "../src/gui/components/controls/juce_TreeView.cpp"
 #include "../src/gui/components/filebrowser/juce_DirectoryContentsDisplayComponent.cpp"
 #include "../src/gui/components/filebrowser/juce_DirectoryContentsList.cpp"
 #include "../src/gui/components/filebrowser/juce_FileBrowserComponent.cpp"
 #include "../src/gui/components/filebrowser/juce_FileChooser.cpp"
 #include "../src/gui/components/filebrowser/juce_FileChooserDialogBox.cpp"
 #include "../src/gui/components/filebrowser/juce_FileFilter.cpp"
 #include "../src/gui/components/filebrowser/juce_FileListComponent.cpp"
 #include "../src/gui/components/filebrowser/juce_FilenameComponent.cpp"
 #include "../src/gui/components/filebrowser/juce_FileSearchPathListComponent.cpp"
 #include "../src/gui/components/filebrowser/juce_FileTreeComponent.cpp"
 #include "../src/gui/components/filebrowser/juce_ImagePreviewComponent.cpp"
 #include "../src/gui/components/filebrowser/juce_WildcardFileFilter.cpp"
 #include "../src/gui/components/keyboard/juce_KeyboardFocusTraverser.cpp"
 #include "../src/gui/components/keyboard/juce_KeyListener.cpp"
 #include "../src/gui/components/keyboard/juce_KeyMappingEditorComponent.cpp"
 #include "../src/gui/components/keyboard/juce_KeyPress.cpp"
 #include "../src/gui/components/keyboard/juce_KeyPressMappingSet.cpp"
 #include "../src/gui/components/keyboard/juce_ModifierKeys.cpp"
 #include "../src/gui/components/layout/juce_ComponentAnimator.cpp"
 #include "../src/gui/components/layout/juce_ComponentBoundsConstrainer.cpp"
 #include "../src/gui/components/layout/juce_ComponentMovementWatcher.cpp"
 #include "../src/gui/components/layout/juce_GroupComponent.cpp"
 #include "../src/gui/components/layout/juce_MultiDocumentPanel.cpp"
 #include "../src/gui/components/layout/juce_ResizableBorderComponent.cpp"
 #include "../src/gui/components/layout/juce_ResizableCornerComponent.cpp"
 #include "../src/gui/components/layout/juce_ScrollBar.cpp"
 #include "../src/gui/components/layout/juce_StretchableLayoutManager.cpp"
 #include "../src/gui/components/layout/juce_StretchableLayoutResizerBar.cpp"
 #include "../src/gui/components/layout/juce_StretchableObjectResizer.cpp"
 #include "../src/gui/components/layout/juce_TabbedButtonBar.cpp"
 #include "../src/gui/components/layout/juce_TabbedComponent.cpp"
 #include "../src/gui/components/layout/juce_Viewport.cpp"
 #include "../src/gui/components/lookandfeel/juce_LookAndFeel.cpp"
 #include "../src/gui/components/lookandfeel/juce_OldSchoolLookAndFeel.cpp"
 #include "../src/gui/components/menus/juce_MenuBarComponent.cpp"
 #include "../src/gui/components/menus/juce_MenuBarModel.cpp"
 #include "../src/gui/components/menus/juce_PopupMenu.cpp"
 #include "../src/gui/components/mouse/juce_ComponentDragger.cpp"
 #include "../src/gui/components/mouse/juce_DragAndDropContainer.cpp"
 #include "../src/gui/components/mouse/juce_MouseCursor.cpp"
 #include "../src/gui/components/mouse/juce_MouseEvent.cpp"
 #include "../src/gui/components/mouse/juce_MouseInputSource.cpp"
 #include "../src/gui/components/mouse/juce_MouseHoverDetector.cpp"
 #include "../src/gui/components/mouse/juce_MouseListener.cpp"
 #include "../src/gui/components/properties/juce_BooleanPropertyComponent.cpp"
 #include "../src/gui/components/properties/juce_ButtonPropertyComponent.cpp"
 #include "../src/gui/components/properties/juce_ChoicePropertyComponent.cpp"
 #include "../src/gui/components/properties/juce_PropertyComponent.cpp"
 #include "../src/gui/components/properties/juce_PropertyPanel.cpp"
 #include "../src/gui/components/properties/juce_SliderPropertyComponent.cpp"
 #include "../src/gui/components/properties/juce_TextPropertyComponent.cpp"
 #include "../src/gui/components/special/juce_AudioDeviceSelectorComponent.cpp"
 #include "../src/gui/components/special/juce_BubbleComponent.cpp"
 #include "../src/gui/components/special/juce_BubbleMessageComponent.cpp"
 #include "../src/gui/components/special/juce_ColourSelector.cpp"
 #include "../src/gui/components/special/juce_DropShadower.cpp"
 #include "../src/gui/components/special/juce_MagnifierComponent.cpp"
 #include "../src/gui/components/special/juce_MidiKeyboardComponent.cpp"
 #include "../src/gui/components/special/juce_OpenGLComponent.cpp"
 #include "../src/gui/components/special/juce_PreferencesPanel.cpp"
 #include "../src/gui/components/special/juce_SystemTrayIconComponent.cpp"
 #include "../src/gui/components/windows/juce_AlertWindow.cpp"
 #include "../src/gui/components/windows/juce_CallOutBox.cpp"
 #include "../src/gui/components/windows/juce_ComponentPeer.cpp"
 #include "../src/gui/components/windows/juce_DialogWindow.cpp"
 #include "../src/gui/components/windows/juce_DocumentWindow.cpp"
 #include "../src/gui/components/windows/juce_ResizableWindow.cpp"
 #include "../src/gui/components/windows/juce_SplashScreen.cpp"
 #include "../src/gui/components/windows/juce_ThreadWithProgressWindow.cpp"
 #include "../src/gui/components/windows/juce_TooltipWindow.cpp"
 #include "../src/gui/components/windows/juce_TopLevelWindow.cpp"
 #include "../src/gui/graphics/geometry/juce_RelativeCoordinate.cpp"
#endif

#if JUCE_BUILD_MISC  // (put these in misc to balance the file sizes and avoid problems in iphone build)
 #include "../src/gui/graphics/colour/juce_Colour.cpp"
 #include "../src/gui/graphics/colour/juce_ColourGradient.cpp"
 #include "../src/gui/graphics/colour/juce_Colours.cpp"
 #include "../src/gui/graphics/contexts/juce_EdgeTable.cpp"
 #include "../src/gui/graphics/contexts/juce_FillType.cpp"
 #include "../src/gui/graphics/contexts/juce_Graphics.cpp"
 #include "../src/gui/graphics/contexts/juce_Justification.cpp"
 #include "../src/gui/graphics/contexts/juce_LowLevelGraphicsPostScriptRenderer.cpp"
 #include "../src/gui/graphics/contexts/juce_LowLevelGraphicsSoftwareRenderer.cpp"
 #include "../src/gui/graphics/contexts/juce_RectanglePlacement.cpp"
 #include "../src/gui/graphics/drawables/juce_Drawable.cpp"
 #include "../src/gui/graphics/drawables/juce_DrawableShape.cpp"
 #include "../src/gui/graphics/drawables/juce_DrawableComposite.cpp"
 #include "../src/gui/graphics/drawables/juce_DrawableImage.cpp"
 #include "../src/gui/graphics/drawables/juce_DrawablePath.cpp"
 #include "../src/gui/graphics/drawables/juce_DrawableRectangle.cpp"
 #include "../src/gui/graphics/drawables/juce_DrawableText.cpp"
 #include "../src/gui/graphics/drawables/juce_SVGParser.cpp"
 #include "../src/gui/graphics/effects/juce_DropShadowEffect.cpp"
 #include "../src/gui/graphics/effects/juce_GlowEffect.cpp"
 #include "../src/gui/graphics/fonts/juce_Font.cpp"
 #include "../src/gui/graphics/fonts/juce_GlyphArrangement.cpp"
 #include "../src/gui/graphics/fonts/juce_TextLayout.cpp"
 #include "../src/gui/graphics/fonts/juce_Typeface.cpp"
 #include "../src/gui/graphics/geometry/juce_AffineTransform.cpp"
 #include "../src/gui/graphics/geometry/juce_BorderSize.cpp"
 #include "../src/gui/graphics/geometry/juce_Path.cpp"
 #include "../src/gui/graphics/geometry/juce_PathIterator.cpp"
 #include "../src/gui/graphics/geometry/juce_PathStrokeType.cpp"
 #include "../src/gui/graphics/geometry/juce_PositionedRectangle.cpp"
 #include "../src/gui/graphics/geometry/juce_RectangleList.cpp"
 #include "../src/gui/graphics/imaging/juce_Image.cpp"
 #include "../src/gui/graphics/imaging/juce_ImageCache.cpp"
 #include "../src/gui/graphics/imaging/juce_ImageConvolutionKernel.cpp"
 #include "../src/gui/graphics/imaging/juce_ImageFileFormat.cpp"
 #include "../src/gui/graphics/imaging/image_file_formats/juce_GIFLoader.cpp"
#endif

//==============================================================================
// some files include lots of library code, so leave them to the end to avoid cluttering
// up the build for the clean files.
#if JUCE_BUILD_CORE
 #include "../src/io/streams/juce_GZIPCompressorOutputStream.cpp"
 #include "../src/io/streams/juce_GZIPDecompressorInputStream.cpp"
#endif

#if JUCE_BUILD_NATIVE && ! JUCE_ONLY_BUILD_CORE_LIBRARY
 #include "../src/audio/audio_file_formats/juce_FlacAudioFormat.cpp"
 #include "../src/audio/audio_file_formats/juce_OggVorbisAudioFormat.cpp"
#endif

#if JUCE_BUILD_CORE && ! JUCE_ONLY_BUILD_CORE_LIBRARY // do these in the core section to help balance the sizes
 #include "../src/gui/graphics/imaging/image_file_formats/juce_JPEGLoader.cpp"
 #include "../src/gui/graphics/imaging/image_file_formats/juce_PNGLoader.cpp"
#endif

//==============================================================================
#if JUCE_BUILD_NATIVE

 // Non-public headers that are needed by more than one platform must be included
 // before the platform-specific sections..
 BEGIN_JUCE_NAMESPACE
  #include "../src/native/common/juce_MidiDataConcatenator.h"
 END_JUCE_NAMESPACE

 #if JUCE_WINDOWS
  #include "../src/native/juce_win32_NativeCode.cpp"
 #endif

 #if JUCE_LINUX
  #include "../src/native/juce_linux_NativeCode.cpp"
 #endif

 #if JUCE_MAC || JUCE_IPHONE
  #include "../src/native/juce_mac_NativeCode.mm"
 #endif
#endif
