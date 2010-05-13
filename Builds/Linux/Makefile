# Automatically generated makefile, created by the Jucer
# Don't edit this file! Your changes will be overwritten when you re-save the Jucer project!

ifndef CONFIG
  CONFIG=Debug
endif

# (this disables dependency generation if multiple architectures are set)
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

ifeq ($(CONFIG),Debug)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Debug
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O0
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L"/usr/X11R6/lib/" -L"../.././bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  TARGET := libjucedebug.a
  BLDCMD = ar -rcs $(OUTDIR)/$(TARGET) $(OBJECTS) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -Os
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L"/usr/X11R6/lib/" -L"../.././bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "NDEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  TARGET := libjuce.a
  BLDCMD = ar -rcs $(OUTDIR)/$(TARGET) $(OBJECTS) $(TARGET_ARCH)
endif

OBJECTS := \
  $(OBJDIR)/juce_Application.o \
  $(OBJDIR)/juce_ApplicationCommandInfo.o \
  $(OBJDIR)/juce_ApplicationCommandManager.o \
  $(OBJDIR)/juce_ApplicationCommandTarget.o \
  $(OBJDIR)/juce_ApplicationProperties.o \
  $(OBJDIR)/juce_AiffAudioFormat.o \
  $(OBJDIR)/juce_AudioCDReader.o \
  $(OBJDIR)/juce_AudioFormat.o \
  $(OBJDIR)/juce_AudioFormatManager.o \
  $(OBJDIR)/juce_AudioSubsectionReader.o \
  $(OBJDIR)/juce_AudioThumbnail.o \
  $(OBJDIR)/juce_AudioThumbnailCache.o \
  $(OBJDIR)/juce_FlacAudioFormat.o \
  $(OBJDIR)/juce_OggVorbisAudioFormat.o \
  $(OBJDIR)/juce_QuickTimeAudioFormat.o \
  $(OBJDIR)/juce_WavAudioFormat.o \
  $(OBJDIR)/juce_AudioFormatReaderSource.o \
  $(OBJDIR)/juce_AudioSourcePlayer.o \
  $(OBJDIR)/juce_AudioTransportSource.o \
  $(OBJDIR)/juce_BufferingAudioSource.o \
  $(OBJDIR)/juce_ChannelRemappingAudioSource.o \
  $(OBJDIR)/juce_IIRFilterAudioSource.o \
  $(OBJDIR)/juce_MixerAudioSource.o \
  $(OBJDIR)/juce_ResamplingAudioSource.o \
  $(OBJDIR)/juce_ToneGeneratorAudioSource.o \
  $(OBJDIR)/juce_AudioDeviceManager.o \
  $(OBJDIR)/juce_AudioIODevice.o \
  $(OBJDIR)/juce_AudioIODeviceType.o \
  $(OBJDIR)/juce_MidiOutput.o \
  $(OBJDIR)/juce_AudioDataConverters.o \
  $(OBJDIR)/juce_AudioSampleBuffer.o \
  $(OBJDIR)/juce_IIRFilter.o \
  $(OBJDIR)/juce_MidiBuffer.o \
  $(OBJDIR)/juce_MidiFile.o \
  $(OBJDIR)/juce_MidiKeyboardState.o \
  $(OBJDIR)/juce_MidiMessage.o \
  $(OBJDIR)/juce_MidiMessageCollector.o \
  $(OBJDIR)/juce_MidiMessageSequence.o \
  $(OBJDIR)/juce_VSTPluginFormat.o \
  $(OBJDIR)/juce_AudioPluginFormat.o \
  $(OBJDIR)/juce_AudioPluginFormatManager.o \
  $(OBJDIR)/juce_AudioPluginInstance.o \
  $(OBJDIR)/juce_KnownPluginList.o \
  $(OBJDIR)/juce_PluginDescription.o \
  $(OBJDIR)/juce_PluginDirectoryScanner.o \
  $(OBJDIR)/juce_PluginListComponent.o \
  $(OBJDIR)/juce_AudioProcessor.o \
  $(OBJDIR)/juce_AudioProcessorEditor.o \
  $(OBJDIR)/juce_AudioProcessorGraph.o \
  $(OBJDIR)/juce_AudioProcessorPlayer.o \
  $(OBJDIR)/juce_GenericAudioProcessorEditor.o \
  $(OBJDIR)/juce_Sampler.o \
  $(OBJDIR)/juce_Synthesiser.o \
  $(OBJDIR)/juce_BitArray.o \
  $(OBJDIR)/juce_DynamicObject.o \
  $(OBJDIR)/juce_MemoryBlock.o \
  $(OBJDIR)/juce_NamedValueSet.o \
  $(OBJDIR)/juce_PropertySet.o \
  $(OBJDIR)/juce_Value.o \
  $(OBJDIR)/juce_ValueTree.o \
  $(OBJDIR)/juce_Variant.o \
  $(OBJDIR)/juce_FileLogger.o \
  $(OBJDIR)/juce_Logger.o \
  $(OBJDIR)/juce_PerformanceCounter.o \
  $(OBJDIR)/juce_Random.o \
  $(OBJDIR)/juce_RelativeTime.o \
  $(OBJDIR)/juce_SystemStats.o \
  $(OBJDIR)/juce_Time.o \
  $(OBJDIR)/juce_Uuid.o \
  $(OBJDIR)/juce_BlowFish.o \
  $(OBJDIR)/juce_MD5.o \
  $(OBJDIR)/juce_Primes.o \
  $(OBJDIR)/juce_RSAKey.o \
  $(OBJDIR)/juce_ActionBroadcaster.o \
  $(OBJDIR)/juce_ActionListenerList.o \
  $(OBJDIR)/juce_AsyncUpdater.o \
  $(OBJDIR)/juce_ChangeBroadcaster.o \
  $(OBJDIR)/juce_ChangeListenerList.o \
  $(OBJDIR)/juce_InterprocessConnection.o \
  $(OBJDIR)/juce_InterprocessConnectionServer.o \
  $(OBJDIR)/juce_Message.o \
  $(OBJDIR)/juce_MessageListener.o \
  $(OBJDIR)/juce_MessageManager.o \
  $(OBJDIR)/juce_MultiTimer.o \
  $(OBJDIR)/juce_Timer.o \
  $(OBJDIR)/juce_Component.o \
  $(OBJDIR)/juce_ComponentListener.o \
  $(OBJDIR)/juce_Desktop.o \
  $(OBJDIR)/juce_ArrowButton.o \
  $(OBJDIR)/juce_Button.o \
  $(OBJDIR)/juce_DrawableButton.o \
  $(OBJDIR)/juce_HyperlinkButton.o \
  $(OBJDIR)/juce_ImageButton.o \
  $(OBJDIR)/juce_ShapeButton.o \
  $(OBJDIR)/juce_TextButton.o \
  $(OBJDIR)/juce_ToggleButton.o \
  $(OBJDIR)/juce_ToolbarButton.o \
  $(OBJDIR)/juce_CodeDocument.o \
  $(OBJDIR)/juce_CodeEditorComponent.o \
  $(OBJDIR)/juce_CPlusPlusCodeTokeniser.o \
  $(OBJDIR)/juce_ComboBox.o \
  $(OBJDIR)/juce_Label.o \
  $(OBJDIR)/juce_ListBox.o \
  $(OBJDIR)/juce_ProgressBar.o \
  $(OBJDIR)/juce_Slider.o \
  $(OBJDIR)/juce_TableHeaderComponent.o \
  $(OBJDIR)/juce_TableListBox.o \
  $(OBJDIR)/juce_TextEditor.o \
  $(OBJDIR)/juce_Toolbar.o \
  $(OBJDIR)/juce_ToolbarItemComponent.o \
  $(OBJDIR)/juce_ToolbarItemPalette.o \
  $(OBJDIR)/juce_TreeView.o \
  $(OBJDIR)/juce_DirectoryContentsDisplayComponent.o \
  $(OBJDIR)/juce_DirectoryContentsList.o \
  $(OBJDIR)/juce_FileBrowserComponent.o \
  $(OBJDIR)/juce_FileChooser.o \
  $(OBJDIR)/juce_FileChooserDialogBox.o \
  $(OBJDIR)/juce_FileFilter.o \
  $(OBJDIR)/juce_FileListComponent.o \
  $(OBJDIR)/juce_FilenameComponent.o \
  $(OBJDIR)/juce_FileSearchPathListComponent.o \
  $(OBJDIR)/juce_FileTreeComponent.o \
  $(OBJDIR)/juce_ImagePreviewComponent.o \
  $(OBJDIR)/juce_WildcardFileFilter.o \
  $(OBJDIR)/juce_KeyboardFocusTraverser.o \
  $(OBJDIR)/juce_KeyListener.o \
  $(OBJDIR)/juce_KeyMappingEditorComponent.o \
  $(OBJDIR)/juce_KeyPress.o \
  $(OBJDIR)/juce_KeyPressMappingSet.o \
  $(OBJDIR)/juce_ModifierKeys.o \
  $(OBJDIR)/juce_ComponentAnimator.o \
  $(OBJDIR)/juce_ComponentBoundsConstrainer.o \
  $(OBJDIR)/juce_ComponentMovementWatcher.o \
  $(OBJDIR)/juce_GroupComponent.o \
  $(OBJDIR)/juce_MultiDocumentPanel.o \
  $(OBJDIR)/juce_ResizableBorderComponent.o \
  $(OBJDIR)/juce_ResizableCornerComponent.o \
  $(OBJDIR)/juce_ScrollBar.o \
  $(OBJDIR)/juce_StretchableLayoutManager.o \
  $(OBJDIR)/juce_StretchableLayoutResizerBar.o \
  $(OBJDIR)/juce_StretchableObjectResizer.o \
  $(OBJDIR)/juce_TabbedButtonBar.o \
  $(OBJDIR)/juce_TabbedComponent.o \
  $(OBJDIR)/juce_Viewport.o \
  $(OBJDIR)/juce_LookAndFeel.o \
  $(OBJDIR)/juce_OldSchoolLookAndFeel.o \
  $(OBJDIR)/juce_MenuBarComponent.o \
  $(OBJDIR)/juce_MenuBarModel.o \
  $(OBJDIR)/juce_PopupMenu.o \
  $(OBJDIR)/juce_ComponentDragger.o \
  $(OBJDIR)/juce_DragAndDropContainer.o \
  $(OBJDIR)/juce_MouseCursor.o \
  $(OBJDIR)/juce_MouseEvent.o \
  $(OBJDIR)/juce_MouseHoverDetector.o \
  $(OBJDIR)/juce_MouseInputSource.o \
  $(OBJDIR)/juce_MouseListener.o \
  $(OBJDIR)/juce_BooleanPropertyComponent.o \
  $(OBJDIR)/juce_ButtonPropertyComponent.o \
  $(OBJDIR)/juce_ChoicePropertyComponent.o \
  $(OBJDIR)/juce_PropertyComponent.o \
  $(OBJDIR)/juce_PropertyPanel.o \
  $(OBJDIR)/juce_SliderPropertyComponent.o \
  $(OBJDIR)/juce_TextPropertyComponent.o \
  $(OBJDIR)/juce_AudioDeviceSelectorComponent.o \
  $(OBJDIR)/juce_BubbleComponent.o \
  $(OBJDIR)/juce_BubbleMessageComponent.o \
  $(OBJDIR)/juce_ColourSelector.o \
  $(OBJDIR)/juce_DropShadower.o \
  $(OBJDIR)/juce_MagnifierComponent.o \
  $(OBJDIR)/juce_MidiKeyboardComponent.o \
  $(OBJDIR)/juce_OpenGLComponent.o \
  $(OBJDIR)/juce_PreferencesPanel.o \
  $(OBJDIR)/juce_SystemTrayIconComponent.o \
  $(OBJDIR)/juce_AlertWindow.o \
  $(OBJDIR)/juce_ComponentPeer.o \
  $(OBJDIR)/juce_DialogWindow.o \
  $(OBJDIR)/juce_DocumentWindow.o \
  $(OBJDIR)/juce_ResizableWindow.o \
  $(OBJDIR)/juce_SplashScreen.o \
  $(OBJDIR)/juce_ThreadWithProgressWindow.o \
  $(OBJDIR)/juce_TooltipWindow.o \
  $(OBJDIR)/juce_TopLevelWindow.o \
  $(OBJDIR)/juce_Colour.o \
  $(OBJDIR)/juce_ColourGradient.o \
  $(OBJDIR)/juce_Colours.o \
  $(OBJDIR)/juce_EdgeTable.o \
  $(OBJDIR)/juce_FillType.o \
  $(OBJDIR)/juce_Graphics.o \
  $(OBJDIR)/juce_Justification.o \
  $(OBJDIR)/juce_LowLevelGraphicsPostScriptRenderer.o \
  $(OBJDIR)/juce_LowLevelGraphicsSoftwareRenderer.o \
  $(OBJDIR)/juce_RectanglePlacement.o \
  $(OBJDIR)/juce_Drawable.o \
  $(OBJDIR)/juce_DrawableComposite.o \
  $(OBJDIR)/juce_DrawableImage.o \
  $(OBJDIR)/juce_DrawablePath.o \
  $(OBJDIR)/juce_DrawableText.o \
  $(OBJDIR)/juce_SVGParser.o \
  $(OBJDIR)/juce_DropShadowEffect.o \
  $(OBJDIR)/juce_GlowEffect.o \
  $(OBJDIR)/juce_ReduceOpacityEffect.o \
  $(OBJDIR)/juce_Font.o \
  $(OBJDIR)/juce_GlyphArrangement.o \
  $(OBJDIR)/juce_TextLayout.o \
  $(OBJDIR)/juce_Typeface.o \
  $(OBJDIR)/juce_AffineTransform.o \
  $(OBJDIR)/juce_BorderSize.o \
  $(OBJDIR)/juce_Line.o \
  $(OBJDIR)/juce_Path.o \
  $(OBJDIR)/juce_PathIterator.o \
  $(OBJDIR)/juce_PathStrokeType.o \
  $(OBJDIR)/juce_PositionedRectangle.o \
  $(OBJDIR)/juce_RectangleList.o \
  $(OBJDIR)/juce_GIFLoader.o \
  $(OBJDIR)/juce_JPEGLoader.o \
  $(OBJDIR)/juce_PNGLoader.o \
  $(OBJDIR)/juce_Image.o \
  $(OBJDIR)/juce_ImageCache.o \
  $(OBJDIR)/juce_ImageConvolutionKernel.o \
  $(OBJDIR)/juce_ImageFileFormat.o \
  $(OBJDIR)/juce_DirectoryIterator.o \
  $(OBJDIR)/juce_File.o \
  $(OBJDIR)/juce_FileInputStream.o \
  $(OBJDIR)/juce_FileOutputStream.o \
  $(OBJDIR)/juce_FileSearchPath.o \
  $(OBJDIR)/juce_NamedPipe.o \
  $(OBJDIR)/juce_TemporaryFile.o \
  $(OBJDIR)/juce_ZipFile.o \
  $(OBJDIR)/juce_Socket.o \
  $(OBJDIR)/juce_URL.o \
  $(OBJDIR)/juce_BufferedInputStream.o \
  $(OBJDIR)/juce_FileInputSource.o \
  $(OBJDIR)/juce_GZIPCompressorOutputStream.o \
  $(OBJDIR)/juce_GZIPDecompressorInputStream.o \
  $(OBJDIR)/juce_InputStream.o \
  $(OBJDIR)/juce_MemoryInputStream.o \
  $(OBJDIR)/juce_MemoryOutputStream.o \
  $(OBJDIR)/juce_OutputStream.o \
  $(OBJDIR)/juce_SubregionStream.o \
  $(OBJDIR)/juce_linux_NativeCode.o \
  $(OBJDIR)/juce_win32_NativeCode.o \
  $(OBJDIR)/juce_posix_NamedPipe.o \
  $(OBJDIR)/juce_linux_Audio.o \
  $(OBJDIR)/juce_linux_AudioCDReader.o \
  $(OBJDIR)/juce_linux_Clipboard.o \
  $(OBJDIR)/juce_linux_FileChooser.o \
  $(OBJDIR)/juce_linux_Files.o \
  $(OBJDIR)/juce_linux_Fonts.o \
  $(OBJDIR)/juce_linux_JackAudio.o \
  $(OBJDIR)/juce_linux_Messaging.o \
  $(OBJDIR)/juce_linux_Midi.o \
  $(OBJDIR)/juce_linux_Network.o \
  $(OBJDIR)/juce_linux_SystemStats.o \
  $(OBJDIR)/juce_linux_Threads.o \
  $(OBJDIR)/juce_linux_WebBrowserComponent.o \
  $(OBJDIR)/juce_linux_Windowing.o \
  $(OBJDIR)/juce_iphone_Audio.o \
  $(OBJDIR)/juce_mac_CoreAudio.o \
  $(OBJDIR)/juce_mac_CoreMidi.o \
  $(OBJDIR)/juce_win32_ActiveXComponent.o \
  $(OBJDIR)/juce_win32_ASIO.o \
  $(OBJDIR)/juce_win32_AudioCDReader.o \
  $(OBJDIR)/juce_win32_CameraDevice.o \
  $(OBJDIR)/juce_win32_DirectSound.o \
  $(OBJDIR)/juce_win32_DynamicLibraryLoader.o \
  $(OBJDIR)/juce_win32_FileChooser.o \
  $(OBJDIR)/juce_win32_Files.o \
  $(OBJDIR)/juce_win32_Fonts.o \
  $(OBJDIR)/juce_win32_Messaging.o \
  $(OBJDIR)/juce_win32_Midi.o \
  $(OBJDIR)/juce_win32_Misc.o \
  $(OBJDIR)/juce_win32_Network.o \
  $(OBJDIR)/juce_win32_OpenGLComponent.o \
  $(OBJDIR)/juce_win32_PlatformUtils.o \
  $(OBJDIR)/juce_win32_QuickTimeMovieComponent.o \
  $(OBJDIR)/juce_win32_SystemStats.o \
  $(OBJDIR)/juce_win32_Threads.o \
  $(OBJDIR)/juce_win32_WASAPI.o \
  $(OBJDIR)/juce_win32_WebBrowserComponent.o \
  $(OBJDIR)/juce_win32_Windowing.o \
  $(OBJDIR)/juce_CharacterFunctions.o \
  $(OBJDIR)/juce_LocalisedStrings.o \
  $(OBJDIR)/juce_String.o \
  $(OBJDIR)/juce_StringArray.o \
  $(OBJDIR)/juce_StringPairArray.o \
  $(OBJDIR)/juce_XmlDocument.o \
  $(OBJDIR)/juce_XmlElement.o \
  $(OBJDIR)/juce_ReadWriteLock.o \
  $(OBJDIR)/juce_Thread.o \
  $(OBJDIR)/juce_ThreadPool.o \
  $(OBJDIR)/juce_TimeSliceThread.o \
  $(OBJDIR)/juce_DeletedAtShutdown.o \
  $(OBJDIR)/juce_FileBasedDocument.o \
  $(OBJDIR)/juce_PropertiesFile.o \
  $(OBJDIR)/juce_RecentlyOpenedFilesList.o \
  $(OBJDIR)/juce_UndoManager.o \

.PHONY: clean

$(OUTDIR)/$(TARGET): $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking Juce
	-@mkdir -p $(BINDIR)
	-@mkdir -p $(LIBDIR)
	-@mkdir -p $(OUTDIR)
	@$(BLDCMD)

clean:
	@echo Cleaning Juce
	-@rm -f $(OUTDIR)/$(TARGET)
	-@rm -rf $(OBJDIR)/*
	-@rm -rf $(OBJDIR)

$(OBJDIR)/juce_Application.o: ../../src/application/juce_Application.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ApplicationCommandInfo.o: ../../src/application/juce_ApplicationCommandInfo.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ApplicationCommandManager.o: ../../src/application/juce_ApplicationCommandManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ApplicationCommandTarget.o: ../../src/application/juce_ApplicationCommandTarget.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ApplicationProperties.o: ../../src/application/juce_ApplicationProperties.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AiffAudioFormat.o: ../../src/audio/audio_file_formats/juce_AiffAudioFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioCDReader.o: ../../src/audio/audio_file_formats/juce_AudioCDReader.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioFormat.o: ../../src/audio/audio_file_formats/juce_AudioFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioFormatManager.o: ../../src/audio/audio_file_formats/juce_AudioFormatManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioSubsectionReader.o: ../../src/audio/audio_file_formats/juce_AudioSubsectionReader.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioThumbnail.o: ../../src/audio/audio_file_formats/juce_AudioThumbnail.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioThumbnailCache.o: ../../src/audio/audio_file_formats/juce_AudioThumbnailCache.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FlacAudioFormat.o: ../../src/audio/audio_file_formats/juce_FlacAudioFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_OggVorbisAudioFormat.o: ../../src/audio/audio_file_formats/juce_OggVorbisAudioFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_QuickTimeAudioFormat.o: ../../src/audio/audio_file_formats/juce_QuickTimeAudioFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_WavAudioFormat.o: ../../src/audio/audio_file_formats/juce_WavAudioFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioFormatReaderSource.o: ../../src/audio/audio_sources/juce_AudioFormatReaderSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioSourcePlayer.o: ../../src/audio/audio_sources/juce_AudioSourcePlayer.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioTransportSource.o: ../../src/audio/audio_sources/juce_AudioTransportSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_BufferingAudioSource.o: ../../src/audio/audio_sources/juce_BufferingAudioSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ChannelRemappingAudioSource.o: ../../src/audio/audio_sources/juce_ChannelRemappingAudioSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_IIRFilterAudioSource.o: ../../src/audio/audio_sources/juce_IIRFilterAudioSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MixerAudioSource.o: ../../src/audio/audio_sources/juce_MixerAudioSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ResamplingAudioSource.o: ../../src/audio/audio_sources/juce_ResamplingAudioSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ToneGeneratorAudioSource.o: ../../src/audio/audio_sources/juce_ToneGeneratorAudioSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioDeviceManager.o: ../../src/audio/devices/juce_AudioDeviceManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioIODevice.o: ../../src/audio/devices/juce_AudioIODevice.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioIODeviceType.o: ../../src/audio/devices/juce_AudioIODeviceType.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiOutput.o: ../../src/audio/devices/juce_MidiOutput.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioDataConverters.o: ../../src/audio/dsp/juce_AudioDataConverters.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioSampleBuffer.o: ../../src/audio/dsp/juce_AudioSampleBuffer.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_IIRFilter.o: ../../src/audio/dsp/juce_IIRFilter.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiBuffer.o: ../../src/audio/midi/juce_MidiBuffer.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiFile.o: ../../src/audio/midi/juce_MidiFile.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiKeyboardState.o: ../../src/audio/midi/juce_MidiKeyboardState.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiMessage.o: ../../src/audio/midi/juce_MidiMessage.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiMessageCollector.o: ../../src/audio/midi/juce_MidiMessageCollector.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiMessageSequence.o: ../../src/audio/midi/juce_MidiMessageSequence.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_VSTPluginFormat.o: ../../src/audio/plugins/formats/juce_VSTPluginFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioPluginFormat.o: ../../src/audio/plugins/juce_AudioPluginFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioPluginFormatManager.o: ../../src/audio/plugins/juce_AudioPluginFormatManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioPluginInstance.o: ../../src/audio/plugins/juce_AudioPluginInstance.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_KnownPluginList.o: ../../src/audio/plugins/juce_KnownPluginList.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PluginDescription.o: ../../src/audio/plugins/juce_PluginDescription.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PluginDirectoryScanner.o: ../../src/audio/plugins/juce_PluginDirectoryScanner.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PluginListComponent.o: ../../src/audio/plugins/juce_PluginListComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioProcessor.o: ../../src/audio/processors/juce_AudioProcessor.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioProcessorEditor.o: ../../src/audio/processors/juce_AudioProcessorEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioProcessorGraph.o: ../../src/audio/processors/juce_AudioProcessorGraph.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioProcessorPlayer.o: ../../src/audio/processors/juce_AudioProcessorPlayer.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_GenericAudioProcessorEditor.o: ../../src/audio/processors/juce_GenericAudioProcessorEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Sampler.o: ../../src/audio/synthesisers/juce_Sampler.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Synthesiser.o: ../../src/audio/synthesisers/juce_Synthesiser.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_BitArray.o: ../../src/containers/juce_BitArray.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DynamicObject.o: ../../src/containers/juce_DynamicObject.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MemoryBlock.o: ../../src/containers/juce_MemoryBlock.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_NamedValueSet.o: ../../src/containers/juce_NamedValueSet.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PropertySet.o: ../../src/containers/juce_PropertySet.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Value.o: ../../src/containers/juce_Value.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ValueTree.o: ../../src/containers/juce_ValueTree.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Variant.o: ../../src/containers/juce_Variant.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileLogger.o: ../../src/core/juce_FileLogger.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Logger.o: ../../src/core/juce_Logger.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PerformanceCounter.o: ../../src/core/juce_PerformanceCounter.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Random.o: ../../src/core/juce_Random.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RelativeTime.o: ../../src/core/juce_RelativeTime.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_SystemStats.o: ../../src/core/juce_SystemStats.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Time.o: ../../src/core/juce_Time.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Uuid.o: ../../src/core/juce_Uuid.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_BlowFish.o: ../../src/cryptography/juce_BlowFish.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MD5.o: ../../src/cryptography/juce_MD5.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Primes.o: ../../src/cryptography/juce_Primes.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RSAKey.o: ../../src/cryptography/juce_RSAKey.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ActionBroadcaster.o: ../../src/events/juce_ActionBroadcaster.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ActionListenerList.o: ../../src/events/juce_ActionListenerList.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AsyncUpdater.o: ../../src/events/juce_AsyncUpdater.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ChangeBroadcaster.o: ../../src/events/juce_ChangeBroadcaster.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ChangeListenerList.o: ../../src/events/juce_ChangeListenerList.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_InterprocessConnection.o: ../../src/events/juce_InterprocessConnection.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_InterprocessConnectionServer.o: ../../src/events/juce_InterprocessConnectionServer.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Message.o: ../../src/events/juce_Message.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MessageListener.o: ../../src/events/juce_MessageListener.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MessageManager.o: ../../src/events/juce_MessageManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MultiTimer.o: ../../src/events/juce_MultiTimer.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Timer.o: ../../src/events/juce_Timer.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Component.o: ../../src/gui/components/juce_Component.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ComponentListener.o: ../../src/gui/components/juce_ComponentListener.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Desktop.o: ../../src/gui/components/juce_Desktop.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ArrowButton.o: ../../src/gui/components/buttons/juce_ArrowButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Button.o: ../../src/gui/components/buttons/juce_Button.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DrawableButton.o: ../../src/gui/components/buttons/juce_DrawableButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_HyperlinkButton.o: ../../src/gui/components/buttons/juce_HyperlinkButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ImageButton.o: ../../src/gui/components/buttons/juce_ImageButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ShapeButton.o: ../../src/gui/components/buttons/juce_ShapeButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TextButton.o: ../../src/gui/components/buttons/juce_TextButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ToggleButton.o: ../../src/gui/components/buttons/juce_ToggleButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ToolbarButton.o: ../../src/gui/components/buttons/juce_ToolbarButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_CodeDocument.o: ../../src/gui/components/code_editor/juce_CodeDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_CodeEditorComponent.o: ../../src/gui/components/code_editor/juce_CodeEditorComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_CPlusPlusCodeTokeniser.o: ../../src/gui/components/code_editor/juce_CPlusPlusCodeTokeniser.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ComboBox.o: ../../src/gui/components/controls/juce_ComboBox.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Label.o: ../../src/gui/components/controls/juce_Label.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ListBox.o: ../../src/gui/components/controls/juce_ListBox.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ProgressBar.o: ../../src/gui/components/controls/juce_ProgressBar.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Slider.o: ../../src/gui/components/controls/juce_Slider.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TableHeaderComponent.o: ../../src/gui/components/controls/juce_TableHeaderComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TableListBox.o: ../../src/gui/components/controls/juce_TableListBox.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TextEditor.o: ../../src/gui/components/controls/juce_TextEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Toolbar.o: ../../src/gui/components/controls/juce_Toolbar.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ToolbarItemComponent.o: ../../src/gui/components/controls/juce_ToolbarItemComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ToolbarItemPalette.o: ../../src/gui/components/controls/juce_ToolbarItemPalette.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TreeView.o: ../../src/gui/components/controls/juce_TreeView.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DirectoryContentsDisplayComponent.o: ../../src/gui/components/filebrowser/juce_DirectoryContentsDisplayComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DirectoryContentsList.o: ../../src/gui/components/filebrowser/juce_DirectoryContentsList.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileBrowserComponent.o: ../../src/gui/components/filebrowser/juce_FileBrowserComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileChooser.o: ../../src/gui/components/filebrowser/juce_FileChooser.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileChooserDialogBox.o: ../../src/gui/components/filebrowser/juce_FileChooserDialogBox.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileFilter.o: ../../src/gui/components/filebrowser/juce_FileFilter.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileListComponent.o: ../../src/gui/components/filebrowser/juce_FileListComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FilenameComponent.o: ../../src/gui/components/filebrowser/juce_FilenameComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileSearchPathListComponent.o: ../../src/gui/components/filebrowser/juce_FileSearchPathListComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileTreeComponent.o: ../../src/gui/components/filebrowser/juce_FileTreeComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ImagePreviewComponent.o: ../../src/gui/components/filebrowser/juce_ImagePreviewComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_WildcardFileFilter.o: ../../src/gui/components/filebrowser/juce_WildcardFileFilter.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_KeyboardFocusTraverser.o: ../../src/gui/components/keyboard/juce_KeyboardFocusTraverser.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_KeyListener.o: ../../src/gui/components/keyboard/juce_KeyListener.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_KeyMappingEditorComponent.o: ../../src/gui/components/keyboard/juce_KeyMappingEditorComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_KeyPress.o: ../../src/gui/components/keyboard/juce_KeyPress.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_KeyPressMappingSet.o: ../../src/gui/components/keyboard/juce_KeyPressMappingSet.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ModifierKeys.o: ../../src/gui/components/keyboard/juce_ModifierKeys.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ComponentAnimator.o: ../../src/gui/components/layout/juce_ComponentAnimator.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ComponentBoundsConstrainer.o: ../../src/gui/components/layout/juce_ComponentBoundsConstrainer.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ComponentMovementWatcher.o: ../../src/gui/components/layout/juce_ComponentMovementWatcher.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_GroupComponent.o: ../../src/gui/components/layout/juce_GroupComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MultiDocumentPanel.o: ../../src/gui/components/layout/juce_MultiDocumentPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ResizableBorderComponent.o: ../../src/gui/components/layout/juce_ResizableBorderComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ResizableCornerComponent.o: ../../src/gui/components/layout/juce_ResizableCornerComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ScrollBar.o: ../../src/gui/components/layout/juce_ScrollBar.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_StretchableLayoutManager.o: ../../src/gui/components/layout/juce_StretchableLayoutManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_StretchableLayoutResizerBar.o: ../../src/gui/components/layout/juce_StretchableLayoutResizerBar.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_StretchableObjectResizer.o: ../../src/gui/components/layout/juce_StretchableObjectResizer.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TabbedButtonBar.o: ../../src/gui/components/layout/juce_TabbedButtonBar.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TabbedComponent.o: ../../src/gui/components/layout/juce_TabbedComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Viewport.o: ../../src/gui/components/layout/juce_Viewport.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_LookAndFeel.o: ../../src/gui/components/lookandfeel/juce_LookAndFeel.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_OldSchoolLookAndFeel.o: ../../src/gui/components/lookandfeel/juce_OldSchoolLookAndFeel.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MenuBarComponent.o: ../../src/gui/components/menus/juce_MenuBarComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MenuBarModel.o: ../../src/gui/components/menus/juce_MenuBarModel.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PopupMenu.o: ../../src/gui/components/menus/juce_PopupMenu.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ComponentDragger.o: ../../src/gui/components/mouse/juce_ComponentDragger.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DragAndDropContainer.o: ../../src/gui/components/mouse/juce_DragAndDropContainer.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MouseCursor.o: ../../src/gui/components/mouse/juce_MouseCursor.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MouseEvent.o: ../../src/gui/components/mouse/juce_MouseEvent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MouseHoverDetector.o: ../../src/gui/components/mouse/juce_MouseHoverDetector.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MouseInputSource.o: ../../src/gui/components/mouse/juce_MouseInputSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MouseListener.o: ../../src/gui/components/mouse/juce_MouseListener.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_BooleanPropertyComponent.o: ../../src/gui/components/properties/juce_BooleanPropertyComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ButtonPropertyComponent.o: ../../src/gui/components/properties/juce_ButtonPropertyComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ChoicePropertyComponent.o: ../../src/gui/components/properties/juce_ChoicePropertyComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PropertyComponent.o: ../../src/gui/components/properties/juce_PropertyComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PropertyPanel.o: ../../src/gui/components/properties/juce_PropertyPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_SliderPropertyComponent.o: ../../src/gui/components/properties/juce_SliderPropertyComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TextPropertyComponent.o: ../../src/gui/components/properties/juce_TextPropertyComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioDeviceSelectorComponent.o: ../../src/gui/components/special/juce_AudioDeviceSelectorComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_BubbleComponent.o: ../../src/gui/components/special/juce_BubbleComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_BubbleMessageComponent.o: ../../src/gui/components/special/juce_BubbleMessageComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ColourSelector.o: ../../src/gui/components/special/juce_ColourSelector.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DropShadower.o: ../../src/gui/components/special/juce_DropShadower.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MagnifierComponent.o: ../../src/gui/components/special/juce_MagnifierComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiKeyboardComponent.o: ../../src/gui/components/special/juce_MidiKeyboardComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_OpenGLComponent.o: ../../src/gui/components/special/juce_OpenGLComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PreferencesPanel.o: ../../src/gui/components/special/juce_PreferencesPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_SystemTrayIconComponent.o: ../../src/gui/components/special/juce_SystemTrayIconComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AlertWindow.o: ../../src/gui/components/windows/juce_AlertWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ComponentPeer.o: ../../src/gui/components/windows/juce_ComponentPeer.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DialogWindow.o: ../../src/gui/components/windows/juce_DialogWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DocumentWindow.o: ../../src/gui/components/windows/juce_DocumentWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ResizableWindow.o: ../../src/gui/components/windows/juce_ResizableWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_SplashScreen.o: ../../src/gui/components/windows/juce_SplashScreen.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ThreadWithProgressWindow.o: ../../src/gui/components/windows/juce_ThreadWithProgressWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TooltipWindow.o: ../../src/gui/components/windows/juce_TooltipWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TopLevelWindow.o: ../../src/gui/components/windows/juce_TopLevelWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Colour.o: ../../src/gui/graphics/colour/juce_Colour.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ColourGradient.o: ../../src/gui/graphics/colour/juce_ColourGradient.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Colours.o: ../../src/gui/graphics/colour/juce_Colours.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_EdgeTable.o: ../../src/gui/graphics/contexts/juce_EdgeTable.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FillType.o: ../../src/gui/graphics/contexts/juce_FillType.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Graphics.o: ../../src/gui/graphics/contexts/juce_Graphics.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Justification.o: ../../src/gui/graphics/contexts/juce_Justification.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_LowLevelGraphicsPostScriptRenderer.o: ../../src/gui/graphics/contexts/juce_LowLevelGraphicsPostScriptRenderer.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_LowLevelGraphicsSoftwareRenderer.o: ../../src/gui/graphics/contexts/juce_LowLevelGraphicsSoftwareRenderer.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RectanglePlacement.o: ../../src/gui/graphics/contexts/juce_RectanglePlacement.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Drawable.o: ../../src/gui/graphics/drawables/juce_Drawable.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DrawableComposite.o: ../../src/gui/graphics/drawables/juce_DrawableComposite.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DrawableImage.o: ../../src/gui/graphics/drawables/juce_DrawableImage.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DrawablePath.o: ../../src/gui/graphics/drawables/juce_DrawablePath.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DrawableText.o: ../../src/gui/graphics/drawables/juce_DrawableText.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_SVGParser.o: ../../src/gui/graphics/drawables/juce_SVGParser.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DropShadowEffect.o: ../../src/gui/graphics/effects/juce_DropShadowEffect.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_GlowEffect.o: ../../src/gui/graphics/effects/juce_GlowEffect.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ReduceOpacityEffect.o: ../../src/gui/graphics/effects/juce_ReduceOpacityEffect.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Font.o: ../../src/gui/graphics/fonts/juce_Font.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_GlyphArrangement.o: ../../src/gui/graphics/fonts/juce_GlyphArrangement.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TextLayout.o: ../../src/gui/graphics/fonts/juce_TextLayout.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Typeface.o: ../../src/gui/graphics/fonts/juce_Typeface.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AffineTransform.o: ../../src/gui/graphics/geometry/juce_AffineTransform.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_BorderSize.o: ../../src/gui/graphics/geometry/juce_BorderSize.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Line.o: ../../src/gui/graphics/geometry/juce_Line.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Path.o: ../../src/gui/graphics/geometry/juce_Path.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PathIterator.o: ../../src/gui/graphics/geometry/juce_PathIterator.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PathStrokeType.o: ../../src/gui/graphics/geometry/juce_PathStrokeType.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PositionedRectangle.o: ../../src/gui/graphics/geometry/juce_PositionedRectangle.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RectangleList.o: ../../src/gui/graphics/geometry/juce_RectangleList.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_GIFLoader.o: ../../src/gui/graphics/imaging/image_file_formats/juce_GIFLoader.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_JPEGLoader.o: ../../src/gui/graphics/imaging/image_file_formats/juce_JPEGLoader.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PNGLoader.o: ../../src/gui/graphics/imaging/image_file_formats/juce_PNGLoader.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Image.o: ../../src/gui/graphics/imaging/juce_Image.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ImageCache.o: ../../src/gui/graphics/imaging/juce_ImageCache.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ImageConvolutionKernel.o: ../../src/gui/graphics/imaging/juce_ImageConvolutionKernel.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ImageFileFormat.o: ../../src/gui/graphics/imaging/juce_ImageFileFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DirectoryIterator.o: ../../src/io/files/juce_DirectoryIterator.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_File.o: ../../src/io/files/juce_File.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileInputStream.o: ../../src/io/files/juce_FileInputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileOutputStream.o: ../../src/io/files/juce_FileOutputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileSearchPath.o: ../../src/io/files/juce_FileSearchPath.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_NamedPipe.o: ../../src/io/files/juce_NamedPipe.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TemporaryFile.o: ../../src/io/files/juce_TemporaryFile.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ZipFile.o: ../../src/io/files/juce_ZipFile.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Socket.o: ../../src/io/network/juce_Socket.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_URL.o: ../../src/io/network/juce_URL.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_BufferedInputStream.o: ../../src/io/streams/juce_BufferedInputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileInputSource.o: ../../src/io/streams/juce_FileInputSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_GZIPCompressorOutputStream.o: ../../src/io/streams/juce_GZIPCompressorOutputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_GZIPDecompressorInputStream.o: ../../src/io/streams/juce_GZIPDecompressorInputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_InputStream.o: ../../src/io/streams/juce_InputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MemoryInputStream.o: ../../src/io/streams/juce_MemoryInputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MemoryOutputStream.o: ../../src/io/streams/juce_MemoryOutputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_OutputStream.o: ../../src/io/streams/juce_OutputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_SubregionStream.o: ../../src/io/streams/juce_SubregionStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_NativeCode.o: ../../src/native/juce_linux_NativeCode.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_NativeCode.o: ../../src/native/juce_win32_NativeCode.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_posix_NamedPipe.o: ../../src/native/common/juce_posix_NamedPipe.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Audio.o: ../../src/native/linux/juce_linux_Audio.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_AudioCDReader.o: ../../src/native/linux/juce_linux_AudioCDReader.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Clipboard.o: ../../src/native/linux/juce_linux_Clipboard.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_FileChooser.o: ../../src/native/linux/juce_linux_FileChooser.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Files.o: ../../src/native/linux/juce_linux_Files.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Fonts.o: ../../src/native/linux/juce_linux_Fonts.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_JackAudio.o: ../../src/native/linux/juce_linux_JackAudio.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Messaging.o: ../../src/native/linux/juce_linux_Messaging.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Midi.o: ../../src/native/linux/juce_linux_Midi.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Network.o: ../../src/native/linux/juce_linux_Network.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_SystemStats.o: ../../src/native/linux/juce_linux_SystemStats.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Threads.o: ../../src/native/linux/juce_linux_Threads.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_WebBrowserComponent.o: ../../src/native/linux/juce_linux_WebBrowserComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Windowing.o: ../../src/native/linux/juce_linux_Windowing.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_iphone_Audio.o: ../../src/native/mac/juce_iphone_Audio.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_mac_CoreAudio.o: ../../src/native/mac/juce_mac_CoreAudio.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_mac_CoreMidi.o: ../../src/native/mac/juce_mac_CoreMidi.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_ActiveXComponent.o: ../../src/native/windows/juce_win32_ActiveXComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_ASIO.o: ../../src/native/windows/juce_win32_ASIO.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_AudioCDReader.o: ../../src/native/windows/juce_win32_AudioCDReader.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_CameraDevice.o: ../../src/native/windows/juce_win32_CameraDevice.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_DirectSound.o: ../../src/native/windows/juce_win32_DirectSound.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_DynamicLibraryLoader.o: ../../src/native/windows/juce_win32_DynamicLibraryLoader.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_FileChooser.o: ../../src/native/windows/juce_win32_FileChooser.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Files.o: ../../src/native/windows/juce_win32_Files.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Fonts.o: ../../src/native/windows/juce_win32_Fonts.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Messaging.o: ../../src/native/windows/juce_win32_Messaging.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Midi.o: ../../src/native/windows/juce_win32_Midi.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Misc.o: ../../src/native/windows/juce_win32_Misc.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Network.o: ../../src/native/windows/juce_win32_Network.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_OpenGLComponent.o: ../../src/native/windows/juce_win32_OpenGLComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_PlatformUtils.o: ../../src/native/windows/juce_win32_PlatformUtils.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_QuickTimeMovieComponent.o: ../../src/native/windows/juce_win32_QuickTimeMovieComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_SystemStats.o: ../../src/native/windows/juce_win32_SystemStats.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Threads.o: ../../src/native/windows/juce_win32_Threads.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_WASAPI.o: ../../src/native/windows/juce_win32_WASAPI.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_WebBrowserComponent.o: ../../src/native/windows/juce_win32_WebBrowserComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Windowing.o: ../../src/native/windows/juce_win32_Windowing.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_CharacterFunctions.o: ../../src/text/juce_CharacterFunctions.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_LocalisedStrings.o: ../../src/text/juce_LocalisedStrings.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_String.o: ../../src/text/juce_String.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_StringArray.o: ../../src/text/juce_StringArray.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_StringPairArray.o: ../../src/text/juce_StringPairArray.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_XmlDocument.o: ../../src/text/juce_XmlDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_XmlElement.o: ../../src/text/juce_XmlElement.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ReadWriteLock.o: ../../src/threads/juce_ReadWriteLock.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Thread.o: ../../src/threads/juce_Thread.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ThreadPool.o: ../../src/threads/juce_ThreadPool.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TimeSliceThread.o: ../../src/threads/juce_TimeSliceThread.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DeletedAtShutdown.o: ../../src/utilities/juce_DeletedAtShutdown.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileBasedDocument.o: ../../src/utilities/juce_FileBasedDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PropertiesFile.o: ../../src/utilities/juce_PropertiesFile.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RecentlyOpenedFilesList.o: ../../src/utilities/juce_RecentlyOpenedFilesList.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_UndoManager.o: ../../src/utilities/juce_UndoManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
