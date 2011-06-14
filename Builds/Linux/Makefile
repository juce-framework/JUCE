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
  OUTDIR := ../../bin
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O0
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L"/usr/X11R6/lib/" -L"../.././bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2"
  TARGET := libjucedebug.a
  BLDCMD = ar -rcs $(OUTDIR)/$(TARGET) $(OBJECTS) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := ../../bin
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -Os
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L"/usr/X11R6/lib/" -L"../.././bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2"
  TARGET := libjuce.a
  BLDCMD = ar -rcs $(OUTDIR)/$(TARGET) $(OBJECTS) $(TARGET_ARCH)
endif

OBJECTS := \
  $(OBJDIR)/juce_Application_2817465d.o \
  $(OBJDIR)/juce_ApplicationCommandInfo_80671406.o \
  $(OBJDIR)/juce_ApplicationCommandManager_5aac95f.o \
  $(OBJDIR)/juce_ApplicationCommandTarget_90a89aa9.o \
  $(OBJDIR)/juce_ApplicationProperties_a76b5c30.o \
  $(OBJDIR)/juce_AiffAudioFormat_ffbda971.o \
  $(OBJDIR)/juce_AudioCDReader_c730f7a6.o \
  $(OBJDIR)/juce_AudioFormat_6605d0f9.o \
  $(OBJDIR)/juce_AudioFormatManager_949148fe.o \
  $(OBJDIR)/juce_AudioFormatReader_36f0295c.o \
  $(OBJDIR)/juce_AudioFormatWriter_11461d0c.o \
  $(OBJDIR)/juce_AudioSubsectionReader_65f61a0a.o \
  $(OBJDIR)/juce_AudioThumbnail_cb99b4b4.o \
  $(OBJDIR)/juce_AudioThumbnailCache_89a7c678.o \
  $(OBJDIR)/juce_FlacAudioFormat_19c048f1.o \
  $(OBJDIR)/juce_OggVorbisAudioFormat_3f214fc9.o \
  $(OBJDIR)/juce_QuickTimeAudioFormat_8ad67271.o \
  $(OBJDIR)/juce_WavAudioFormat_4ff23a5f.o \
  $(OBJDIR)/juce_AudioFormatReaderSource_13d4af92.o \
  $(OBJDIR)/juce_AudioSourcePlayer_42464899.o \
  $(OBJDIR)/juce_AudioTransportSource_f3e60751.o \
  $(OBJDIR)/juce_BufferingAudioSource_58721b72.o \
  $(OBJDIR)/juce_ChannelRemappingAudioSource_e17f93c0.o \
  $(OBJDIR)/juce_IIRFilterAudioSource_807ee8aa.o \
  $(OBJDIR)/juce_MixerAudioSource_bc6f772b.o \
  $(OBJDIR)/juce_ResamplingAudioSource_8511875e.o \
  $(OBJDIR)/juce_ReverbAudioSource_3ae42aa6.o \
  $(OBJDIR)/juce_ToneGeneratorAudioSource_77f504b3.o \
  $(OBJDIR)/juce_AudioDeviceManager_c24db832.o \
  $(OBJDIR)/juce_AudioIODevice_f7da876b.o \
  $(OBJDIR)/juce_AudioIODeviceType_e5d402c5.o \
  $(OBJDIR)/juce_AudioDataConverters_dc0ece28.o \
  $(OBJDIR)/juce_AudioSampleBuffer_af6ff195.o \
  $(OBJDIR)/juce_IIRFilter_9a31e47f.o \
  $(OBJDIR)/juce_MidiBuffer_fa4db7fe.o \
  $(OBJDIR)/juce_MidiFile_3bdbc97a.o \
  $(OBJDIR)/juce_MidiKeyboardState_28313976.o \
  $(OBJDIR)/juce_MidiMessage_5b1f5753.o \
  $(OBJDIR)/juce_MidiMessageCollector_108abdc4.o \
  $(OBJDIR)/juce_MidiMessageSequence_a577dcb4.o \
  $(OBJDIR)/juce_MidiOutput_3cc0f43f.o \
  $(OBJDIR)/juce_VSTPluginFormat_af341d6.o \
  $(OBJDIR)/juce_AudioPluginFormat_5dc1eec2.o \
  $(OBJDIR)/juce_AudioPluginFormatManager_c336c715.o \
  $(OBJDIR)/juce_AudioPluginInstance_96637ee0.o \
  $(OBJDIR)/juce_KnownPluginList_cadece16.o \
  $(OBJDIR)/juce_PluginDescription_d9c0f10b.o \
  $(OBJDIR)/juce_PluginDirectoryScanner_c7652cec.o \
  $(OBJDIR)/juce_PluginListComponent_f08a630e.o \
  $(OBJDIR)/juce_AudioProcessor_babd0919.o \
  $(OBJDIR)/juce_AudioProcessorEditor_82a32d26.o \
  $(OBJDIR)/juce_AudioProcessorGraph_76dcb4ff.o \
  $(OBJDIR)/juce_AudioProcessorPlayer_5c53cc9a.o \
  $(OBJDIR)/juce_GenericAudioProcessorEditor_2e8ec30d.o \
  $(OBJDIR)/juce_Sampler_98f716a4.o \
  $(OBJDIR)/juce_Synthesiser_2bffa1dd.o \
  $(OBJDIR)/juce_AbstractFifo_dfc0bd23.o \
  $(OBJDIR)/juce_DynamicObject_69d02ab3.o \
  $(OBJDIR)/juce_NamedValueSet_6b0793df.o \
  $(OBJDIR)/juce_PropertySet_914b5922.o \
  $(OBJDIR)/juce_Value_16231646.o \
  $(OBJDIR)/juce_ValueTree_f822be84.o \
  $(OBJDIR)/juce_Variant_4f31c35a.o \
  $(OBJDIR)/juce_FileLogger_7858478e.o \
  $(OBJDIR)/juce_Initialisation_fec765a9.o \
  $(OBJDIR)/juce_Logger_4f4f7f72.o \
  $(OBJDIR)/juce_PerformanceCounter_6422080e.o \
  $(OBJDIR)/juce_RelativeTime_bc5ef35b.o \
  $(OBJDIR)/juce_Result_d8f437f.o \
  $(OBJDIR)/juce_SystemStats_c00d8758.o \
  $(OBJDIR)/juce_Time_16acdd6f.o \
  $(OBJDIR)/juce_Uuid_f277575d.o \
  $(OBJDIR)/juce_BlowFish_5d22a7d9.o \
  $(OBJDIR)/juce_MD5_56896819.o \
  $(OBJDIR)/juce_Primes_32e6603.o \
  $(OBJDIR)/juce_RSAKey_b60982ae.o \
  $(OBJDIR)/juce_ActionBroadcaster_7f997786.o \
  $(OBJDIR)/juce_AsyncUpdater_a7e1cb89.o \
  $(OBJDIR)/juce_ChangeBroadcaster_3eb8fecc.o \
  $(OBJDIR)/juce_InterprocessConnection_13086b6d.o \
  $(OBJDIR)/juce_InterprocessConnectionServer_a4d7b350.o \
  $(OBJDIR)/juce_Message_ac79f635.o \
  $(OBJDIR)/juce_MessageListener_93e51509.o \
  $(OBJDIR)/juce_MessageManager_c7e00842.o \
  $(OBJDIR)/juce_MultiTimer_d4d9eca8.o \
  $(OBJDIR)/juce_Timer_77c098b3.o \
  $(OBJDIR)/juce_Component_49f01dfa.o \
  $(OBJDIR)/juce_ComponentListener_e0eda7ce.o \
  $(OBJDIR)/juce_Desktop_e3b47b99.o \
  $(OBJDIR)/juce_ModalComponentManager_b87bddba.o \
  $(OBJDIR)/juce_ArrowButton_ebac7066.o \
  $(OBJDIR)/juce_Button_886d3491.o \
  $(OBJDIR)/juce_DrawableButton_e03899cf.o \
  $(OBJDIR)/juce_HyperlinkButton_af0a8983.o \
  $(OBJDIR)/juce_ImageButton_c4dda738.o \
  $(OBJDIR)/juce_ShapeButton_2359adfe.o \
  $(OBJDIR)/juce_TextButton_8cf49c9e.o \
  $(OBJDIR)/juce_ToggleButton_f69cb85.o \
  $(OBJDIR)/juce_ToolbarButton_884113d8.o \
  $(OBJDIR)/juce_CodeDocument_fdd87805.o \
  $(OBJDIR)/juce_CodeEditorComponent_6fe4a170.o \
  $(OBJDIR)/juce_CPlusPlusCodeTokeniser_6b8548a9.o \
  $(OBJDIR)/juce_ComboBox_bcb5e365.o \
  $(OBJDIR)/juce_ImageComponent_1f90572a.o \
  $(OBJDIR)/juce_Label_7af0aab6.o \
  $(OBJDIR)/juce_ListBox_d4a8704f.o \
  $(OBJDIR)/juce_ProgressBar_38bc3ba8.o \
  $(OBJDIR)/juce_Slider_8a0d91a9.o \
  $(OBJDIR)/juce_TableHeaderComponent_2228124a.o \
  $(OBJDIR)/juce_TableListBox_588d027.o \
  $(OBJDIR)/juce_TextEditor_abe7dfc2.o \
  $(OBJDIR)/juce_Toolbar_29fa8c7d.o \
  $(OBJDIR)/juce_ToolbarItemComponent_7fab8ff7.o \
  $(OBJDIR)/juce_ToolbarItemPalette_250807b5.o \
  $(OBJDIR)/juce_TreeView_f1a8d26b.o \
  $(OBJDIR)/juce_DirectoryContentsDisplayComponent_97e43c62.o \
  $(OBJDIR)/juce_DirectoryContentsList_31e06025.o \
  $(OBJDIR)/juce_FileBrowserComponent_1332771b.o \
  $(OBJDIR)/juce_FileChooser_c439271f.o \
  $(OBJDIR)/juce_FileChooserDialogBox_50a36f4e.o \
  $(OBJDIR)/juce_FileFilter_de71639e.o \
  $(OBJDIR)/juce_FileListComponent_3f41b6a3.o \
  $(OBJDIR)/juce_FilenameComponent_921ef5f6.o \
  $(OBJDIR)/juce_FileSearchPathListComponent_85ed8576.o \
  $(OBJDIR)/juce_FileTreeComponent_8cb2d3a3.o \
  $(OBJDIR)/juce_ImagePreviewComponent_a108ea50.o \
  $(OBJDIR)/juce_WildcardFileFilter_9337b18.o \
  $(OBJDIR)/juce_CaretComponent_26bf4433.o \
  $(OBJDIR)/juce_KeyboardFocusTraverser_6325fc08.o \
  $(OBJDIR)/juce_KeyListener_631ada24.o \
  $(OBJDIR)/juce_KeyMappingEditorComponent_a0183a92.o \
  $(OBJDIR)/juce_KeyPress_47fcc29d.o \
  $(OBJDIR)/juce_KeyPressMappingSet_26212371.o \
  $(OBJDIR)/juce_ModifierKeys_eea16d64.o \
  $(OBJDIR)/juce_ComponentAnimator_ad5663d2.o \
  $(OBJDIR)/juce_ComponentBoundsConstrainer_a33c10ae.o \
  $(OBJDIR)/juce_ComponentBuilder_2aa6c5a.o \
  $(OBJDIR)/juce_ComponentMovementWatcher_cbb3e7cc.o \
  $(OBJDIR)/juce_GroupComponent_456e237a.o \
  $(OBJDIR)/juce_MultiDocumentPanel_50e05aac.o \
  $(OBJDIR)/juce_ResizableBorderComponent_7f4f2182.o \
  $(OBJDIR)/juce_ResizableCornerComponent_1cac2df9.o \
  $(OBJDIR)/juce_ResizableEdgeComponent_b9c9291.o \
  $(OBJDIR)/juce_ScrollBar_89666ad4.o \
  $(OBJDIR)/juce_StretchableLayoutManager_6bf5d800.o \
  $(OBJDIR)/juce_StretchableLayoutResizerBar_a720084c.o \
  $(OBJDIR)/juce_StretchableObjectResizer_2edcfadc.o \
  $(OBJDIR)/juce_TabbedButtonBar_a760aaa3.o \
  $(OBJDIR)/juce_TabbedComponent_d618721f.o \
  $(OBJDIR)/juce_Viewport_6c983422.o \
  $(OBJDIR)/juce_LookAndFeel_d4e838ec.o \
  $(OBJDIR)/juce_OldSchoolLookAndFeel_5ece559f.o \
  $(OBJDIR)/juce_MenuBarComponent_c68c8dbb.o \
  $(OBJDIR)/juce_MenuBarModel_ede42267.o \
  $(OBJDIR)/juce_PopupMenu_ec4cf9a3.o \
  $(OBJDIR)/juce_ComponentDragger_fb3bdbe6.o \
  $(OBJDIR)/juce_DragAndDropContainer_553e412.o \
  $(OBJDIR)/juce_MouseCursor_afb68802.o \
  $(OBJDIR)/juce_MouseEvent_f6ca6078.o \
  $(OBJDIR)/juce_MouseInputSource_8904aa23.o \
  $(OBJDIR)/juce_MouseListener_eb73f8a0.o \
  $(OBJDIR)/juce_MarkerList_2fbb4ef.o \
  $(OBJDIR)/juce_RelativeCoordinate_ab2eee5b.o \
  $(OBJDIR)/juce_RelativeCoordinatePositioner_c1352791.o \
  $(OBJDIR)/juce_RelativeParallelogram_5d17dbe6.o \
  $(OBJDIR)/juce_RelativePoint_9d2cc957.o \
  $(OBJDIR)/juce_RelativePointPath_a4fed61c.o \
  $(OBJDIR)/juce_RelativeRectangle_cc5440b6.o \
  $(OBJDIR)/juce_BooleanPropertyComponent_92b54aa5.o \
  $(OBJDIR)/juce_ButtonPropertyComponent_3675d6bb.o \
  $(OBJDIR)/juce_ChoicePropertyComponent_b9b44bac.o \
  $(OBJDIR)/juce_PropertyComponent_df1eaecd.o \
  $(OBJDIR)/juce_PropertyPanel_c640c5d4.o \
  $(OBJDIR)/juce_SliderPropertyComponent_ba2eb86c.o \
  $(OBJDIR)/juce_TextPropertyComponent_977e6fa0.o \
  $(OBJDIR)/juce_AudioDeviceSelectorComponent_64b1a089.o \
  $(OBJDIR)/juce_BubbleComponent_5f51c164.o \
  $(OBJDIR)/juce_BubbleMessageComponent_22f737f9.o \
  $(OBJDIR)/juce_ColourSelector_ac8bb1e2.o \
  $(OBJDIR)/juce_DropShadower_5903f9f3.o \
  $(OBJDIR)/juce_MidiKeyboardComponent_896e2ac8.o \
  $(OBJDIR)/juce_OpenGLComponent_73944221.o \
  $(OBJDIR)/juce_PreferencesPanel_dbc7d503.o \
  $(OBJDIR)/juce_SystemTrayIconComponent_fa664512.o \
  $(OBJDIR)/juce_AlertWindow_94d8d1f5.o \
  $(OBJDIR)/juce_CallOutBox_61f42d7c.o \
  $(OBJDIR)/juce_ComponentPeer_1e4b4b08.o \
  $(OBJDIR)/juce_DialogWindow_535b27b9.o \
  $(OBJDIR)/juce_DocumentWindow_3cb9d4cc.o \
  $(OBJDIR)/juce_ResizableWindow_d2110a84.o \
  $(OBJDIR)/juce_SplashScreen_a0940094.o \
  $(OBJDIR)/juce_ThreadWithProgressWindow_d711ee6e.o \
  $(OBJDIR)/juce_TooltipWindow_45074edc.o \
  $(OBJDIR)/juce_TopLevelWindow_717cf1e0.o \
  $(OBJDIR)/juce_Colour_453f3d5.o \
  $(OBJDIR)/juce_ColourGradient_c63d0725.o \
  $(OBJDIR)/juce_Colours_89de9488.o \
  $(OBJDIR)/juce_EdgeTable_6a5fefda.o \
  $(OBJDIR)/juce_FillType_5805677e.o \
  $(OBJDIR)/juce_Graphics_27754bac.o \
  $(OBJDIR)/juce_Justification_dc284c3b.o \
  $(OBJDIR)/juce_LowLevelGraphicsPostScriptRenderer_2e8a92a.o \
  $(OBJDIR)/juce_LowLevelGraphicsSoftwareRenderer_97263906.o \
  $(OBJDIR)/juce_RectanglePlacement_5fc90ed7.o \
  $(OBJDIR)/juce_Drawable_8c1eccc6.o \
  $(OBJDIR)/juce_DrawableComposite_4cd2d2ab.o \
  $(OBJDIR)/juce_DrawableImage_53f04eff.o \
  $(OBJDIR)/juce_DrawablePath_fb538a0b.o \
  $(OBJDIR)/juce_DrawableRectangle_5f2f5993.o \
  $(OBJDIR)/juce_DrawableShape_26926245.o \
  $(OBJDIR)/juce_DrawableText_75eda713.o \
  $(OBJDIR)/juce_SVGParser_b79416c5.o \
  $(OBJDIR)/juce_DropShadowEffect_da52d75.o \
  $(OBJDIR)/juce_GlowEffect_c0959893.o \
  $(OBJDIR)/juce_CustomTypeface_b93a3eff.o \
  $(OBJDIR)/juce_Font_fa88db26.o \
  $(OBJDIR)/juce_GlyphArrangement_f0797295.o \
  $(OBJDIR)/juce_TextLayout_d18f700e.o \
  $(OBJDIR)/juce_Typeface_34b3026e.o \
  $(OBJDIR)/juce_AffineTransform_4a320188.o \
  $(OBJDIR)/juce_Path_dcdd4594.o \
  $(OBJDIR)/juce_PathIterator_4e116c22.o \
  $(OBJDIR)/juce_PathStrokeType_506dcb86.o \
  $(OBJDIR)/juce_RectangleList_b6ae7ee8.o \
  $(OBJDIR)/juce_GIFLoader_61887b90.o \
  $(OBJDIR)/juce_JPEGLoader_412d2ccc.o \
  $(OBJDIR)/juce_PNGLoader_7f80a3b5.o \
  $(OBJDIR)/juce_Image_790c9a1a.o \
  $(OBJDIR)/juce_ImageCache_e74a50d2.o \
  $(OBJDIR)/juce_ImageConvolutionKernel_4e9e7945.o \
  $(OBJDIR)/juce_ImageFileFormat_161e770d.o \
  $(OBJDIR)/juce_DirectoryIterator_24c60d54.o \
  $(OBJDIR)/juce_File_8244d8d.o \
  $(OBJDIR)/juce_FileInputStream_e14097a7.o \
  $(OBJDIR)/juce_FileOutputStream_d151050e.o \
  $(OBJDIR)/juce_FileSearchPath_30e3d71a.o \
  $(OBJDIR)/juce_NamedPipe_b3a205e0.o \
  $(OBJDIR)/juce_TemporaryFile_935f3fc6.o \
  $(OBJDIR)/juce_ZipFile_16fbb396.o \
  $(OBJDIR)/juce_MACAddress_b81987ed.o \
  $(OBJDIR)/juce_Socket_cf39c63b.o \
  $(OBJDIR)/juce_URL_d6b7cff1.o \
  $(OBJDIR)/juce_BufferedInputStream_c71b2b68.o \
  $(OBJDIR)/juce_FileInputSource_2a374046.o \
  $(OBJDIR)/juce_GZIPCompressorOutputStream_4409282d.o \
  $(OBJDIR)/juce_GZIPDecompressorInputStream_b026ffc7.o \
  $(OBJDIR)/juce_InputStream_5314e647.o \
  $(OBJDIR)/juce_MemoryInputStream_c5db0106.o \
  $(OBJDIR)/juce_MemoryOutputStream_8003c78f.o \
  $(OBJDIR)/juce_OutputStream_9a068a6e.o \
  $(OBJDIR)/juce_SubregionStream_9156f331.o \
  $(OBJDIR)/juce_BigInteger_b44f43d6.o \
  $(OBJDIR)/juce_Expression_6f910d50.o \
  $(OBJDIR)/juce_Random_a529cb7b.o \
  $(OBJDIR)/juce_MemoryBlock_52f17c52.o \
  $(OBJDIR)/juce_posix_NamedPipe_aa308c65.o \
  $(OBJDIR)/juce_linux_Audio_18d7e8b6.o \
  $(OBJDIR)/juce_linux_AudioCDReader_1263363a.o \
  $(OBJDIR)/juce_linux_Clipboard_44a6bcf6.o \
  $(OBJDIR)/juce_linux_FileChooser_3b3f403f.o \
  $(OBJDIR)/juce_linux_Files_c036ced7.o \
  $(OBJDIR)/juce_linux_Fonts_b2e303e4.o \
  $(OBJDIR)/juce_linux_JackAudio_15353e17.o \
  $(OBJDIR)/juce_linux_Messaging_daefaf64.o \
  $(OBJDIR)/juce_linux_Midi_4bad86cb.o \
  $(OBJDIR)/juce_linux_NativeCode_8a61cd0e.o \
  $(OBJDIR)/juce_linux_Network_f47a1c4e.o \
  $(OBJDIR)/juce_linux_SystemStats_9f6d6b50.o \
  $(OBJDIR)/juce_linux_Threads_dc4a4049.o \
  $(OBJDIR)/juce_linux_WebBrowserComponent_358c0a49.o \
  $(OBJDIR)/juce_linux_Windowing_abe04752.o \
  $(OBJDIR)/juce_ios_Audio_40e86914.o \
  $(OBJDIR)/juce_mac_CoreAudio_faf8c177.o \
  $(OBJDIR)/juce_mac_CoreMidi_e79e11ea.o \
  $(OBJDIR)/juce_win32_ActiveXComponent_7590131d.o \
  $(OBJDIR)/juce_win32_ASIO_60eb61ca.o \
  $(OBJDIR)/juce_win32_AudioCDReader_66c7252.o \
  $(OBJDIR)/juce_win32_CameraDevice_ea35306d.o \
  $(OBJDIR)/juce_win32_Direct2DGraphicsContext_9f1b6be1.o \
  $(OBJDIR)/juce_win32_DirectShowComponent_e4030c4f.o \
  $(OBJDIR)/juce_win32_DirectSound_3462415e.o \
  $(OBJDIR)/juce_win32_FileChooser_18a257.o \
  $(OBJDIR)/juce_win32_Files_f3e9a2ef.o \
  $(OBJDIR)/juce_win32_Fonts_e695d7fc.o \
  $(OBJDIR)/juce_win32_Messaging_926ab77c.o \
  $(OBJDIR)/juce_win32_Midi_fac3cfb3.o \
  $(OBJDIR)/juce_win32_Misc_1407f3fe.o \
  $(OBJDIR)/juce_win32_NativeCode_c247c7f6.o \
  $(OBJDIR)/juce_win32_Network_6c84a66.o \
  $(OBJDIR)/juce_win32_OpenGLComponent_d0d99a46.o \
  $(OBJDIR)/juce_win32_PlatformUtils_73e7e1f6.o \
  $(OBJDIR)/juce_win32_QuickTimeMovieComponent_be08e5f.o \
  $(OBJDIR)/juce_win32_SystemStats_6446cd68.o \
  $(OBJDIR)/juce_win32_Threads_ee986e61.o \
  $(OBJDIR)/juce_win32_WASAPI_c7c6c3c3.o \
  $(OBJDIR)/juce_win32_WebBrowserComponent_2a4dd461.o \
  $(OBJDIR)/juce_win32_Windowing_635b4f6a.o \
  $(OBJDIR)/juce_android_Audio_93f83bd6.o \
  $(OBJDIR)/juce_android_CameraDevice_d2743865.o \
  $(OBJDIR)/juce_android_FileChooser_205cbb5f.o \
  $(OBJDIR)/juce_android_Files_3b5721f7.o \
  $(OBJDIR)/juce_android_Fonts_2e035704.o \
  $(OBJDIR)/juce_android_GraphicsContext_d0fb27e4.o \
  $(OBJDIR)/juce_android_Messaging_59717284.o \
  $(OBJDIR)/juce_android_Midi_4fa64fab.o \
  $(OBJDIR)/juce_android_Misc_68ea73f6.o \
  $(OBJDIR)/juce_android_NativeCode_dc186dee.o \
  $(OBJDIR)/juce_android_Network_28d2276e.o \
  $(OBJDIR)/juce_android_OpenGLComponent_8fd9ef4e.o \
  $(OBJDIR)/juce_android_SystemStats_848ae670.o \
  $(OBJDIR)/juce_android_Threads_10a24b69.o \
  $(OBJDIR)/juce_android_WebBrowserComponent_178c6569.o \
  $(OBJDIR)/juce_android_Windowing_2a620a72.o \
  $(OBJDIR)/juce_CharacterFunctions_b1a5ac62.o \
  $(OBJDIR)/juce_Identifier_e99ee619.o \
  $(OBJDIR)/juce_JSON_bbf64af8.o \
  $(OBJDIR)/juce_LocalisedStrings_3ddfa55e.o \
  $(OBJDIR)/juce_String_80587b01.o \
  $(OBJDIR)/juce_StringArray_4aaa67a2.o \
  $(OBJDIR)/juce_StringPairArray_37130fe8.o \
  $(OBJDIR)/juce_StringPool_9e8d033d.o \
  $(OBJDIR)/juce_XmlDocument_6b9b0c2c.o \
  $(OBJDIR)/juce_XmlElement_51daa55.o \
  $(OBJDIR)/juce_ReadWriteLock_7acf3cc8.o \
  $(OBJDIR)/juce_Thread_69734d40.o \
  $(OBJDIR)/juce_ThreadPool_fd669dfc.o \
  $(OBJDIR)/juce_TimeSliceThread_e9db3803.o \
  $(OBJDIR)/juce_DeletedAtShutdown_b9105375.o \
  $(OBJDIR)/juce_FileBasedDocument_d507d0a5.o \
  $(OBJDIR)/juce_PropertiesFile_8faca026.o \
  $(OBJDIR)/juce_RecentlyOpenedFilesList_f791257.o \
  $(OBJDIR)/juce_UndoManager_33b197fc.o \
  $(OBJDIR)/juce_UnitTest_4a02c10d.o \

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

$(OBJDIR)/juce_Application_2817465d.o: ../../src/application/juce_Application.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Application.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ApplicationCommandInfo_80671406.o: ../../src/application/juce_ApplicationCommandInfo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ApplicationCommandInfo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ApplicationCommandManager_5aac95f.o: ../../src/application/juce_ApplicationCommandManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ApplicationCommandManager.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ApplicationCommandTarget_90a89aa9.o: ../../src/application/juce_ApplicationCommandTarget.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ApplicationCommandTarget.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ApplicationProperties_a76b5c30.o: ../../src/application/juce_ApplicationProperties.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ApplicationProperties.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AiffAudioFormat_ffbda971.o: ../../src/audio/audio_file_formats/juce_AiffAudioFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AiffAudioFormat.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioCDReader_c730f7a6.o: ../../src/audio/audio_file_formats/juce_AudioCDReader.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioCDReader.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioFormat_6605d0f9.o: ../../src/audio/audio_file_formats/juce_AudioFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioFormat.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioFormatManager_949148fe.o: ../../src/audio/audio_file_formats/juce_AudioFormatManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioFormatManager.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioFormatReader_36f0295c.o: ../../src/audio/audio_file_formats/juce_AudioFormatReader.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioFormatReader.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioFormatWriter_11461d0c.o: ../../src/audio/audio_file_formats/juce_AudioFormatWriter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioFormatWriter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioSubsectionReader_65f61a0a.o: ../../src/audio/audio_file_formats/juce_AudioSubsectionReader.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioSubsectionReader.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioThumbnail_cb99b4b4.o: ../../src/audio/audio_file_formats/juce_AudioThumbnail.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioThumbnail.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioThumbnailCache_89a7c678.o: ../../src/audio/audio_file_formats/juce_AudioThumbnailCache.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioThumbnailCache.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FlacAudioFormat_19c048f1.o: ../../src/audio/audio_file_formats/juce_FlacAudioFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FlacAudioFormat.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_OggVorbisAudioFormat_3f214fc9.o: ../../src/audio/audio_file_formats/juce_OggVorbisAudioFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_OggVorbisAudioFormat.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_QuickTimeAudioFormat_8ad67271.o: ../../src/audio/audio_file_formats/juce_QuickTimeAudioFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_QuickTimeAudioFormat.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_WavAudioFormat_4ff23a5f.o: ../../src/audio/audio_file_formats/juce_WavAudioFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_WavAudioFormat.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioFormatReaderSource_13d4af92.o: ../../src/audio/audio_sources/juce_AudioFormatReaderSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioFormatReaderSource.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioSourcePlayer_42464899.o: ../../src/audio/audio_sources/juce_AudioSourcePlayer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioSourcePlayer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioTransportSource_f3e60751.o: ../../src/audio/audio_sources/juce_AudioTransportSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioTransportSource.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_BufferingAudioSource_58721b72.o: ../../src/audio/audio_sources/juce_BufferingAudioSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_BufferingAudioSource.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ChannelRemappingAudioSource_e17f93c0.o: ../../src/audio/audio_sources/juce_ChannelRemappingAudioSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ChannelRemappingAudioSource.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_IIRFilterAudioSource_807ee8aa.o: ../../src/audio/audio_sources/juce_IIRFilterAudioSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_IIRFilterAudioSource.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MixerAudioSource_bc6f772b.o: ../../src/audio/audio_sources/juce_MixerAudioSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MixerAudioSource.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ResamplingAudioSource_8511875e.o: ../../src/audio/audio_sources/juce_ResamplingAudioSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ResamplingAudioSource.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ReverbAudioSource_3ae42aa6.o: ../../src/audio/audio_sources/juce_ReverbAudioSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ReverbAudioSource.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ToneGeneratorAudioSource_77f504b3.o: ../../src/audio/audio_sources/juce_ToneGeneratorAudioSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ToneGeneratorAudioSource.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioDeviceManager_c24db832.o: ../../src/audio/devices/juce_AudioDeviceManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioDeviceManager.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioIODevice_f7da876b.o: ../../src/audio/devices/juce_AudioIODevice.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioIODevice.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioIODeviceType_e5d402c5.o: ../../src/audio/devices/juce_AudioIODeviceType.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioIODeviceType.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioDataConverters_dc0ece28.o: ../../src/audio/dsp/juce_AudioDataConverters.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioDataConverters.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioSampleBuffer_af6ff195.o: ../../src/audio/dsp/juce_AudioSampleBuffer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioSampleBuffer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_IIRFilter_9a31e47f.o: ../../src/audio/dsp/juce_IIRFilter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_IIRFilter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiBuffer_fa4db7fe.o: ../../src/audio/midi/juce_MidiBuffer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MidiBuffer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiFile_3bdbc97a.o: ../../src/audio/midi/juce_MidiFile.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MidiFile.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiKeyboardState_28313976.o: ../../src/audio/midi/juce_MidiKeyboardState.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MidiKeyboardState.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiMessage_5b1f5753.o: ../../src/audio/midi/juce_MidiMessage.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MidiMessage.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiMessageCollector_108abdc4.o: ../../src/audio/midi/juce_MidiMessageCollector.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MidiMessageCollector.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiMessageSequence_a577dcb4.o: ../../src/audio/midi/juce_MidiMessageSequence.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MidiMessageSequence.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiOutput_3cc0f43f.o: ../../src/audio/midi/juce_MidiOutput.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MidiOutput.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_VSTPluginFormat_af341d6.o: ../../src/audio/plugin_host/formats/juce_VSTPluginFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_VSTPluginFormat.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioPluginFormat_5dc1eec2.o: ../../src/audio/plugin_host/juce_AudioPluginFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioPluginFormat.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioPluginFormatManager_c336c715.o: ../../src/audio/plugin_host/juce_AudioPluginFormatManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioPluginFormatManager.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioPluginInstance_96637ee0.o: ../../src/audio/plugin_host/juce_AudioPluginInstance.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioPluginInstance.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_KnownPluginList_cadece16.o: ../../src/audio/plugin_host/juce_KnownPluginList.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_KnownPluginList.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PluginDescription_d9c0f10b.o: ../../src/audio/plugin_host/juce_PluginDescription.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_PluginDescription.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PluginDirectoryScanner_c7652cec.o: ../../src/audio/plugin_host/juce_PluginDirectoryScanner.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_PluginDirectoryScanner.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PluginListComponent_f08a630e.o: ../../src/audio/plugin_host/juce_PluginListComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_PluginListComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioProcessor_babd0919.o: ../../src/audio/processors/juce_AudioProcessor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioProcessor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioProcessorEditor_82a32d26.o: ../../src/audio/processors/juce_AudioProcessorEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioProcessorEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioProcessorGraph_76dcb4ff.o: ../../src/audio/processors/juce_AudioProcessorGraph.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioProcessorGraph.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioProcessorPlayer_5c53cc9a.o: ../../src/audio/processors/juce_AudioProcessorPlayer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioProcessorPlayer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_GenericAudioProcessorEditor_2e8ec30d.o: ../../src/audio/processors/juce_GenericAudioProcessorEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_GenericAudioProcessorEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Sampler_98f716a4.o: ../../src/audio/synthesisers/juce_Sampler.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Sampler.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Synthesiser_2bffa1dd.o: ../../src/audio/synthesisers/juce_Synthesiser.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Synthesiser.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AbstractFifo_dfc0bd23.o: ../../src/containers/juce_AbstractFifo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AbstractFifo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DynamicObject_69d02ab3.o: ../../src/containers/juce_DynamicObject.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DynamicObject.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_NamedValueSet_6b0793df.o: ../../src/containers/juce_NamedValueSet.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_NamedValueSet.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PropertySet_914b5922.o: ../../src/containers/juce_PropertySet.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_PropertySet.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Value_16231646.o: ../../src/containers/juce_Value.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Value.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ValueTree_f822be84.o: ../../src/containers/juce_ValueTree.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ValueTree.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Variant_4f31c35a.o: ../../src/containers/juce_Variant.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Variant.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileLogger_7858478e.o: ../../src/core/juce_FileLogger.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FileLogger.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Initialisation_fec765a9.o: ../../src/core/juce_Initialisation.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Initialisation.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Logger_4f4f7f72.o: ../../src/core/juce_Logger.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Logger.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PerformanceCounter_6422080e.o: ../../src/core/juce_PerformanceCounter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_PerformanceCounter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RelativeTime_bc5ef35b.o: ../../src/core/juce_RelativeTime.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_RelativeTime.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Result_d8f437f.o: ../../src/core/juce_Result.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Result.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_SystemStats_c00d8758.o: ../../src/core/juce_SystemStats.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_SystemStats.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Time_16acdd6f.o: ../../src/core/juce_Time.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Time.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Uuid_f277575d.o: ../../src/core/juce_Uuid.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Uuid.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_BlowFish_5d22a7d9.o: ../../src/cryptography/juce_BlowFish.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_BlowFish.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MD5_56896819.o: ../../src/cryptography/juce_MD5.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MD5.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Primes_32e6603.o: ../../src/cryptography/juce_Primes.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Primes.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RSAKey_b60982ae.o: ../../src/cryptography/juce_RSAKey.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_RSAKey.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ActionBroadcaster_7f997786.o: ../../src/events/juce_ActionBroadcaster.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ActionBroadcaster.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AsyncUpdater_a7e1cb89.o: ../../src/events/juce_AsyncUpdater.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AsyncUpdater.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ChangeBroadcaster_3eb8fecc.o: ../../src/events/juce_ChangeBroadcaster.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ChangeBroadcaster.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_InterprocessConnection_13086b6d.o: ../../src/events/juce_InterprocessConnection.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_InterprocessConnection.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_InterprocessConnectionServer_a4d7b350.o: ../../src/events/juce_InterprocessConnectionServer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_InterprocessConnectionServer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Message_ac79f635.o: ../../src/events/juce_Message.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Message.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MessageListener_93e51509.o: ../../src/events/juce_MessageListener.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MessageListener.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MessageManager_c7e00842.o: ../../src/events/juce_MessageManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MessageManager.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MultiTimer_d4d9eca8.o: ../../src/events/juce_MultiTimer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MultiTimer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Timer_77c098b3.o: ../../src/events/juce_Timer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Timer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Component_49f01dfa.o: ../../src/gui/components/juce_Component.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Component.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ComponentListener_e0eda7ce.o: ../../src/gui/components/juce_ComponentListener.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ComponentListener.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Desktop_e3b47b99.o: ../../src/gui/components/juce_Desktop.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Desktop.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ModalComponentManager_b87bddba.o: ../../src/gui/components/juce_ModalComponentManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ModalComponentManager.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ArrowButton_ebac7066.o: ../../src/gui/components/buttons/juce_ArrowButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ArrowButton.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Button_886d3491.o: ../../src/gui/components/buttons/juce_Button.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Button.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DrawableButton_e03899cf.o: ../../src/gui/components/buttons/juce_DrawableButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DrawableButton.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_HyperlinkButton_af0a8983.o: ../../src/gui/components/buttons/juce_HyperlinkButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_HyperlinkButton.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ImageButton_c4dda738.o: ../../src/gui/components/buttons/juce_ImageButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ImageButton.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ShapeButton_2359adfe.o: ../../src/gui/components/buttons/juce_ShapeButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ShapeButton.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TextButton_8cf49c9e.o: ../../src/gui/components/buttons/juce_TextButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_TextButton.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ToggleButton_f69cb85.o: ../../src/gui/components/buttons/juce_ToggleButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ToggleButton.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ToolbarButton_884113d8.o: ../../src/gui/components/buttons/juce_ToolbarButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ToolbarButton.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_CodeDocument_fdd87805.o: ../../src/gui/components/code_editor/juce_CodeDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_CodeDocument.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_CodeEditorComponent_6fe4a170.o: ../../src/gui/components/code_editor/juce_CodeEditorComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_CodeEditorComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_CPlusPlusCodeTokeniser_6b8548a9.o: ../../src/gui/components/code_editor/juce_CPlusPlusCodeTokeniser.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_CPlusPlusCodeTokeniser.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ComboBox_bcb5e365.o: ../../src/gui/components/controls/juce_ComboBox.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ComboBox.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ImageComponent_1f90572a.o: ../../src/gui/components/controls/juce_ImageComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ImageComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Label_7af0aab6.o: ../../src/gui/components/controls/juce_Label.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Label.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ListBox_d4a8704f.o: ../../src/gui/components/controls/juce_ListBox.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ListBox.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ProgressBar_38bc3ba8.o: ../../src/gui/components/controls/juce_ProgressBar.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ProgressBar.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Slider_8a0d91a9.o: ../../src/gui/components/controls/juce_Slider.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Slider.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TableHeaderComponent_2228124a.o: ../../src/gui/components/controls/juce_TableHeaderComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_TableHeaderComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TableListBox_588d027.o: ../../src/gui/components/controls/juce_TableListBox.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_TableListBox.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TextEditor_abe7dfc2.o: ../../src/gui/components/controls/juce_TextEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_TextEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Toolbar_29fa8c7d.o: ../../src/gui/components/controls/juce_Toolbar.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Toolbar.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ToolbarItemComponent_7fab8ff7.o: ../../src/gui/components/controls/juce_ToolbarItemComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ToolbarItemComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ToolbarItemPalette_250807b5.o: ../../src/gui/components/controls/juce_ToolbarItemPalette.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ToolbarItemPalette.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TreeView_f1a8d26b.o: ../../src/gui/components/controls/juce_TreeView.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_TreeView.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DirectoryContentsDisplayComponent_97e43c62.o: ../../src/gui/components/filebrowser/juce_DirectoryContentsDisplayComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DirectoryContentsDisplayComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DirectoryContentsList_31e06025.o: ../../src/gui/components/filebrowser/juce_DirectoryContentsList.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DirectoryContentsList.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileBrowserComponent_1332771b.o: ../../src/gui/components/filebrowser/juce_FileBrowserComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FileBrowserComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileChooser_c439271f.o: ../../src/gui/components/filebrowser/juce_FileChooser.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FileChooser.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileChooserDialogBox_50a36f4e.o: ../../src/gui/components/filebrowser/juce_FileChooserDialogBox.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FileChooserDialogBox.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileFilter_de71639e.o: ../../src/gui/components/filebrowser/juce_FileFilter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FileFilter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileListComponent_3f41b6a3.o: ../../src/gui/components/filebrowser/juce_FileListComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FileListComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FilenameComponent_921ef5f6.o: ../../src/gui/components/filebrowser/juce_FilenameComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FilenameComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileSearchPathListComponent_85ed8576.o: ../../src/gui/components/filebrowser/juce_FileSearchPathListComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FileSearchPathListComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileTreeComponent_8cb2d3a3.o: ../../src/gui/components/filebrowser/juce_FileTreeComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FileTreeComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ImagePreviewComponent_a108ea50.o: ../../src/gui/components/filebrowser/juce_ImagePreviewComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ImagePreviewComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_WildcardFileFilter_9337b18.o: ../../src/gui/components/filebrowser/juce_WildcardFileFilter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_WildcardFileFilter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_CaretComponent_26bf4433.o: ../../src/gui/components/keyboard/juce_CaretComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_CaretComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_KeyboardFocusTraverser_6325fc08.o: ../../src/gui/components/keyboard/juce_KeyboardFocusTraverser.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_KeyboardFocusTraverser.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_KeyListener_631ada24.o: ../../src/gui/components/keyboard/juce_KeyListener.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_KeyListener.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_KeyMappingEditorComponent_a0183a92.o: ../../src/gui/components/keyboard/juce_KeyMappingEditorComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_KeyMappingEditorComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_KeyPress_47fcc29d.o: ../../src/gui/components/keyboard/juce_KeyPress.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_KeyPress.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_KeyPressMappingSet_26212371.o: ../../src/gui/components/keyboard/juce_KeyPressMappingSet.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_KeyPressMappingSet.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ModifierKeys_eea16d64.o: ../../src/gui/components/keyboard/juce_ModifierKeys.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ModifierKeys.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ComponentAnimator_ad5663d2.o: ../../src/gui/components/layout/juce_ComponentAnimator.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ComponentAnimator.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ComponentBoundsConstrainer_a33c10ae.o: ../../src/gui/components/layout/juce_ComponentBoundsConstrainer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ComponentBoundsConstrainer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ComponentBuilder_2aa6c5a.o: ../../src/gui/components/layout/juce_ComponentBuilder.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ComponentBuilder.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ComponentMovementWatcher_cbb3e7cc.o: ../../src/gui/components/layout/juce_ComponentMovementWatcher.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ComponentMovementWatcher.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_GroupComponent_456e237a.o: ../../src/gui/components/layout/juce_GroupComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_GroupComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MultiDocumentPanel_50e05aac.o: ../../src/gui/components/layout/juce_MultiDocumentPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MultiDocumentPanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ResizableBorderComponent_7f4f2182.o: ../../src/gui/components/layout/juce_ResizableBorderComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ResizableBorderComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ResizableCornerComponent_1cac2df9.o: ../../src/gui/components/layout/juce_ResizableCornerComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ResizableCornerComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ResizableEdgeComponent_b9c9291.o: ../../src/gui/components/layout/juce_ResizableEdgeComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ResizableEdgeComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ScrollBar_89666ad4.o: ../../src/gui/components/layout/juce_ScrollBar.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ScrollBar.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_StretchableLayoutManager_6bf5d800.o: ../../src/gui/components/layout/juce_StretchableLayoutManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_StretchableLayoutManager.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_StretchableLayoutResizerBar_a720084c.o: ../../src/gui/components/layout/juce_StretchableLayoutResizerBar.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_StretchableLayoutResizerBar.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_StretchableObjectResizer_2edcfadc.o: ../../src/gui/components/layout/juce_StretchableObjectResizer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_StretchableObjectResizer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TabbedButtonBar_a760aaa3.o: ../../src/gui/components/layout/juce_TabbedButtonBar.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_TabbedButtonBar.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TabbedComponent_d618721f.o: ../../src/gui/components/layout/juce_TabbedComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_TabbedComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Viewport_6c983422.o: ../../src/gui/components/layout/juce_Viewport.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Viewport.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_LookAndFeel_d4e838ec.o: ../../src/gui/components/lookandfeel/juce_LookAndFeel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_LookAndFeel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_OldSchoolLookAndFeel_5ece559f.o: ../../src/gui/components/lookandfeel/juce_OldSchoolLookAndFeel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_OldSchoolLookAndFeel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MenuBarComponent_c68c8dbb.o: ../../src/gui/components/menus/juce_MenuBarComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MenuBarComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MenuBarModel_ede42267.o: ../../src/gui/components/menus/juce_MenuBarModel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MenuBarModel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PopupMenu_ec4cf9a3.o: ../../src/gui/components/menus/juce_PopupMenu.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_PopupMenu.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ComponentDragger_fb3bdbe6.o: ../../src/gui/components/mouse/juce_ComponentDragger.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ComponentDragger.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DragAndDropContainer_553e412.o: ../../src/gui/components/mouse/juce_DragAndDropContainer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DragAndDropContainer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MouseCursor_afb68802.o: ../../src/gui/components/mouse/juce_MouseCursor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MouseCursor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MouseEvent_f6ca6078.o: ../../src/gui/components/mouse/juce_MouseEvent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MouseEvent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MouseInputSource_8904aa23.o: ../../src/gui/components/mouse/juce_MouseInputSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MouseInputSource.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MouseListener_eb73f8a0.o: ../../src/gui/components/mouse/juce_MouseListener.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MouseListener.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MarkerList_2fbb4ef.o: ../../src/gui/components/positioning/juce_MarkerList.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MarkerList.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RelativeCoordinate_ab2eee5b.o: ../../src/gui/components/positioning/juce_RelativeCoordinate.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_RelativeCoordinate.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RelativeCoordinatePositioner_c1352791.o: ../../src/gui/components/positioning/juce_RelativeCoordinatePositioner.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_RelativeCoordinatePositioner.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RelativeParallelogram_5d17dbe6.o: ../../src/gui/components/positioning/juce_RelativeParallelogram.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_RelativeParallelogram.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RelativePoint_9d2cc957.o: ../../src/gui/components/positioning/juce_RelativePoint.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_RelativePoint.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RelativePointPath_a4fed61c.o: ../../src/gui/components/positioning/juce_RelativePointPath.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_RelativePointPath.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RelativeRectangle_cc5440b6.o: ../../src/gui/components/positioning/juce_RelativeRectangle.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_RelativeRectangle.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_BooleanPropertyComponent_92b54aa5.o: ../../src/gui/components/properties/juce_BooleanPropertyComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_BooleanPropertyComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ButtonPropertyComponent_3675d6bb.o: ../../src/gui/components/properties/juce_ButtonPropertyComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ButtonPropertyComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ChoicePropertyComponent_b9b44bac.o: ../../src/gui/components/properties/juce_ChoicePropertyComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ChoicePropertyComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PropertyComponent_df1eaecd.o: ../../src/gui/components/properties/juce_PropertyComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_PropertyComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PropertyPanel_c640c5d4.o: ../../src/gui/components/properties/juce_PropertyPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_PropertyPanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_SliderPropertyComponent_ba2eb86c.o: ../../src/gui/components/properties/juce_SliderPropertyComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_SliderPropertyComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TextPropertyComponent_977e6fa0.o: ../../src/gui/components/properties/juce_TextPropertyComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_TextPropertyComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AudioDeviceSelectorComponent_64b1a089.o: ../../src/gui/components/special/juce_AudioDeviceSelectorComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AudioDeviceSelectorComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_BubbleComponent_5f51c164.o: ../../src/gui/components/special/juce_BubbleComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_BubbleComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_BubbleMessageComponent_22f737f9.o: ../../src/gui/components/special/juce_BubbleMessageComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_BubbleMessageComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ColourSelector_ac8bb1e2.o: ../../src/gui/components/special/juce_ColourSelector.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ColourSelector.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DropShadower_5903f9f3.o: ../../src/gui/components/special/juce_DropShadower.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DropShadower.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MidiKeyboardComponent_896e2ac8.o: ../../src/gui/components/special/juce_MidiKeyboardComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MidiKeyboardComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_OpenGLComponent_73944221.o: ../../src/gui/components/special/juce_OpenGLComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_OpenGLComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PreferencesPanel_dbc7d503.o: ../../src/gui/components/special/juce_PreferencesPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_PreferencesPanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_SystemTrayIconComponent_fa664512.o: ../../src/gui/components/special/juce_SystemTrayIconComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_SystemTrayIconComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AlertWindow_94d8d1f5.o: ../../src/gui/components/windows/juce_AlertWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AlertWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_CallOutBox_61f42d7c.o: ../../src/gui/components/windows/juce_CallOutBox.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_CallOutBox.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ComponentPeer_1e4b4b08.o: ../../src/gui/components/windows/juce_ComponentPeer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ComponentPeer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DialogWindow_535b27b9.o: ../../src/gui/components/windows/juce_DialogWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DialogWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DocumentWindow_3cb9d4cc.o: ../../src/gui/components/windows/juce_DocumentWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DocumentWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ResizableWindow_d2110a84.o: ../../src/gui/components/windows/juce_ResizableWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ResizableWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_SplashScreen_a0940094.o: ../../src/gui/components/windows/juce_SplashScreen.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_SplashScreen.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ThreadWithProgressWindow_d711ee6e.o: ../../src/gui/components/windows/juce_ThreadWithProgressWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ThreadWithProgressWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TooltipWindow_45074edc.o: ../../src/gui/components/windows/juce_TooltipWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_TooltipWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TopLevelWindow_717cf1e0.o: ../../src/gui/components/windows/juce_TopLevelWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_TopLevelWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Colour_453f3d5.o: ../../src/gui/graphics/colour/juce_Colour.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Colour.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ColourGradient_c63d0725.o: ../../src/gui/graphics/colour/juce_ColourGradient.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ColourGradient.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Colours_89de9488.o: ../../src/gui/graphics/colour/juce_Colours.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Colours.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_EdgeTable_6a5fefda.o: ../../src/gui/graphics/contexts/juce_EdgeTable.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_EdgeTable.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FillType_5805677e.o: ../../src/gui/graphics/contexts/juce_FillType.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FillType.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Graphics_27754bac.o: ../../src/gui/graphics/contexts/juce_Graphics.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Graphics.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Justification_dc284c3b.o: ../../src/gui/graphics/contexts/juce_Justification.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Justification.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_LowLevelGraphicsPostScriptRenderer_2e8a92a.o: ../../src/gui/graphics/contexts/juce_LowLevelGraphicsPostScriptRenderer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_LowLevelGraphicsPostScriptRenderer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_LowLevelGraphicsSoftwareRenderer_97263906.o: ../../src/gui/graphics/contexts/juce_LowLevelGraphicsSoftwareRenderer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_LowLevelGraphicsSoftwareRenderer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RectanglePlacement_5fc90ed7.o: ../../src/gui/graphics/contexts/juce_RectanglePlacement.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_RectanglePlacement.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Drawable_8c1eccc6.o: ../../src/gui/graphics/drawables/juce_Drawable.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Drawable.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DrawableComposite_4cd2d2ab.o: ../../src/gui/graphics/drawables/juce_DrawableComposite.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DrawableComposite.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DrawableImage_53f04eff.o: ../../src/gui/graphics/drawables/juce_DrawableImage.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DrawableImage.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DrawablePath_fb538a0b.o: ../../src/gui/graphics/drawables/juce_DrawablePath.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DrawablePath.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DrawableRectangle_5f2f5993.o: ../../src/gui/graphics/drawables/juce_DrawableRectangle.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DrawableRectangle.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DrawableShape_26926245.o: ../../src/gui/graphics/drawables/juce_DrawableShape.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DrawableShape.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DrawableText_75eda713.o: ../../src/gui/graphics/drawables/juce_DrawableText.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DrawableText.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_SVGParser_b79416c5.o: ../../src/gui/graphics/drawables/juce_SVGParser.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_SVGParser.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DropShadowEffect_da52d75.o: ../../src/gui/graphics/effects/juce_DropShadowEffect.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DropShadowEffect.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_GlowEffect_c0959893.o: ../../src/gui/graphics/effects/juce_GlowEffect.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_GlowEffect.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_CustomTypeface_b93a3eff.o: ../../src/gui/graphics/fonts/juce_CustomTypeface.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_CustomTypeface.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Font_fa88db26.o: ../../src/gui/graphics/fonts/juce_Font.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Font.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_GlyphArrangement_f0797295.o: ../../src/gui/graphics/fonts/juce_GlyphArrangement.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_GlyphArrangement.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TextLayout_d18f700e.o: ../../src/gui/graphics/fonts/juce_TextLayout.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_TextLayout.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Typeface_34b3026e.o: ../../src/gui/graphics/fonts/juce_Typeface.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Typeface.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_AffineTransform_4a320188.o: ../../src/gui/graphics/geometry/juce_AffineTransform.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_AffineTransform.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Path_dcdd4594.o: ../../src/gui/graphics/geometry/juce_Path.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Path.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PathIterator_4e116c22.o: ../../src/gui/graphics/geometry/juce_PathIterator.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_PathIterator.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PathStrokeType_506dcb86.o: ../../src/gui/graphics/geometry/juce_PathStrokeType.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_PathStrokeType.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RectangleList_b6ae7ee8.o: ../../src/gui/graphics/geometry/juce_RectangleList.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_RectangleList.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_GIFLoader_61887b90.o: ../../src/gui/graphics/imaging/image_file_formats/juce_GIFLoader.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_GIFLoader.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_JPEGLoader_412d2ccc.o: ../../src/gui/graphics/imaging/image_file_formats/juce_JPEGLoader.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_JPEGLoader.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PNGLoader_7f80a3b5.o: ../../src/gui/graphics/imaging/image_file_formats/juce_PNGLoader.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_PNGLoader.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Image_790c9a1a.o: ../../src/gui/graphics/imaging/juce_Image.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Image.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ImageCache_e74a50d2.o: ../../src/gui/graphics/imaging/juce_ImageCache.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ImageCache.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ImageConvolutionKernel_4e9e7945.o: ../../src/gui/graphics/imaging/juce_ImageConvolutionKernel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ImageConvolutionKernel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ImageFileFormat_161e770d.o: ../../src/gui/graphics/imaging/juce_ImageFileFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ImageFileFormat.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DirectoryIterator_24c60d54.o: ../../src/io/files/juce_DirectoryIterator.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DirectoryIterator.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_File_8244d8d.o: ../../src/io/files/juce_File.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_File.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileInputStream_e14097a7.o: ../../src/io/files/juce_FileInputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FileInputStream.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileOutputStream_d151050e.o: ../../src/io/files/juce_FileOutputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FileOutputStream.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileSearchPath_30e3d71a.o: ../../src/io/files/juce_FileSearchPath.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FileSearchPath.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_NamedPipe_b3a205e0.o: ../../src/io/files/juce_NamedPipe.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_NamedPipe.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TemporaryFile_935f3fc6.o: ../../src/io/files/juce_TemporaryFile.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_TemporaryFile.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ZipFile_16fbb396.o: ../../src/io/files/juce_ZipFile.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ZipFile.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MACAddress_b81987ed.o: ../../src/io/network/juce_MACAddress.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MACAddress.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Socket_cf39c63b.o: ../../src/io/network/juce_Socket.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Socket.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_URL_d6b7cff1.o: ../../src/io/network/juce_URL.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_URL.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_BufferedInputStream_c71b2b68.o: ../../src/io/streams/juce_BufferedInputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_BufferedInputStream.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileInputSource_2a374046.o: ../../src/io/streams/juce_FileInputSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FileInputSource.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_GZIPCompressorOutputStream_4409282d.o: ../../src/io/streams/juce_GZIPCompressorOutputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_GZIPCompressorOutputStream.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_GZIPDecompressorInputStream_b026ffc7.o: ../../src/io/streams/juce_GZIPDecompressorInputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_GZIPDecompressorInputStream.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_InputStream_5314e647.o: ../../src/io/streams/juce_InputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_InputStream.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MemoryInputStream_c5db0106.o: ../../src/io/streams/juce_MemoryInputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MemoryInputStream.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MemoryOutputStream_8003c78f.o: ../../src/io/streams/juce_MemoryOutputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MemoryOutputStream.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_OutputStream_9a068a6e.o: ../../src/io/streams/juce_OutputStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_OutputStream.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_SubregionStream_9156f331.o: ../../src/io/streams/juce_SubregionStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_SubregionStream.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_BigInteger_b44f43d6.o: ../../src/maths/juce_BigInteger.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_BigInteger.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Expression_6f910d50.o: ../../src/maths/juce_Expression.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Expression.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Random_a529cb7b.o: ../../src/maths/juce_Random.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Random.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_MemoryBlock_52f17c52.o: ../../src/memory/juce_MemoryBlock.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_MemoryBlock.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_posix_NamedPipe_aa308c65.o: ../../src/native/common/juce_posix_NamedPipe.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_posix_NamedPipe.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Audio_18d7e8b6.o: ../../src/native/linux/juce_linux_Audio.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_linux_Audio.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_AudioCDReader_1263363a.o: ../../src/native/linux/juce_linux_AudioCDReader.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_linux_AudioCDReader.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Clipboard_44a6bcf6.o: ../../src/native/linux/juce_linux_Clipboard.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_linux_Clipboard.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_FileChooser_3b3f403f.o: ../../src/native/linux/juce_linux_FileChooser.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_linux_FileChooser.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Files_c036ced7.o: ../../src/native/linux/juce_linux_Files.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_linux_Files.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Fonts_b2e303e4.o: ../../src/native/linux/juce_linux_Fonts.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_linux_Fonts.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_JackAudio_15353e17.o: ../../src/native/linux/juce_linux_JackAudio.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_linux_JackAudio.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Messaging_daefaf64.o: ../../src/native/linux/juce_linux_Messaging.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_linux_Messaging.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Midi_4bad86cb.o: ../../src/native/linux/juce_linux_Midi.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_linux_Midi.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_NativeCode_8a61cd0e.o: ../../src/native/linux/juce_linux_NativeCode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_linux_NativeCode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Network_f47a1c4e.o: ../../src/native/linux/juce_linux_Network.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_linux_Network.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_SystemStats_9f6d6b50.o: ../../src/native/linux/juce_linux_SystemStats.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_linux_SystemStats.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Threads_dc4a4049.o: ../../src/native/linux/juce_linux_Threads.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_linux_Threads.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_WebBrowserComponent_358c0a49.o: ../../src/native/linux/juce_linux_WebBrowserComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_linux_WebBrowserComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_linux_Windowing_abe04752.o: ../../src/native/linux/juce_linux_Windowing.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_linux_Windowing.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ios_Audio_40e86914.o: ../../src/native/mac/juce_ios_Audio.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ios_Audio.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_mac_CoreAudio_faf8c177.o: ../../src/native/mac/juce_mac_CoreAudio.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_mac_CoreAudio.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_mac_CoreMidi_e79e11ea.o: ../../src/native/mac/juce_mac_CoreMidi.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_mac_CoreMidi.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_ActiveXComponent_7590131d.o: ../../src/native/windows/juce_win32_ActiveXComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_ActiveXComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_ASIO_60eb61ca.o: ../../src/native/windows/juce_win32_ASIO.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_ASIO.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_AudioCDReader_66c7252.o: ../../src/native/windows/juce_win32_AudioCDReader.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_AudioCDReader.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_CameraDevice_ea35306d.o: ../../src/native/windows/juce_win32_CameraDevice.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_CameraDevice.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Direct2DGraphicsContext_9f1b6be1.o: ../../src/native/windows/juce_win32_Direct2DGraphicsContext.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_Direct2DGraphicsContext.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_DirectShowComponent_e4030c4f.o: ../../src/native/windows/juce_win32_DirectShowComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_DirectShowComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_DirectSound_3462415e.o: ../../src/native/windows/juce_win32_DirectSound.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_DirectSound.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_FileChooser_18a257.o: ../../src/native/windows/juce_win32_FileChooser.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_FileChooser.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Files_f3e9a2ef.o: ../../src/native/windows/juce_win32_Files.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_Files.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Fonts_e695d7fc.o: ../../src/native/windows/juce_win32_Fonts.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_Fonts.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Messaging_926ab77c.o: ../../src/native/windows/juce_win32_Messaging.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_Messaging.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Midi_fac3cfb3.o: ../../src/native/windows/juce_win32_Midi.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_Midi.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Misc_1407f3fe.o: ../../src/native/windows/juce_win32_Misc.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_Misc.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_NativeCode_c247c7f6.o: ../../src/native/windows/juce_win32_NativeCode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_NativeCode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Network_6c84a66.o: ../../src/native/windows/juce_win32_Network.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_Network.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_OpenGLComponent_d0d99a46.o: ../../src/native/windows/juce_win32_OpenGLComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_OpenGLComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_PlatformUtils_73e7e1f6.o: ../../src/native/windows/juce_win32_PlatformUtils.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_PlatformUtils.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_QuickTimeMovieComponent_be08e5f.o: ../../src/native/windows/juce_win32_QuickTimeMovieComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_QuickTimeMovieComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_SystemStats_6446cd68.o: ../../src/native/windows/juce_win32_SystemStats.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_SystemStats.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Threads_ee986e61.o: ../../src/native/windows/juce_win32_Threads.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_Threads.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_WASAPI_c7c6c3c3.o: ../../src/native/windows/juce_win32_WASAPI.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_WASAPI.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_WebBrowserComponent_2a4dd461.o: ../../src/native/windows/juce_win32_WebBrowserComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_WebBrowserComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_win32_Windowing_635b4f6a.o: ../../src/native/windows/juce_win32_Windowing.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_win32_Windowing.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_Audio_93f83bd6.o: ../../src/native/android/juce_android_Audio.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_Audio.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_CameraDevice_d2743865.o: ../../src/native/android/juce_android_CameraDevice.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_CameraDevice.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_FileChooser_205cbb5f.o: ../../src/native/android/juce_android_FileChooser.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_FileChooser.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_Files_3b5721f7.o: ../../src/native/android/juce_android_Files.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_Files.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_Fonts_2e035704.o: ../../src/native/android/juce_android_Fonts.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_Fonts.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_GraphicsContext_d0fb27e4.o: ../../src/native/android/juce_android_GraphicsContext.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_GraphicsContext.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_Messaging_59717284.o: ../../src/native/android/juce_android_Messaging.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_Messaging.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_Midi_4fa64fab.o: ../../src/native/android/juce_android_Midi.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_Midi.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_Misc_68ea73f6.o: ../../src/native/android/juce_android_Misc.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_Misc.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_NativeCode_dc186dee.o: ../../src/native/android/juce_android_NativeCode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_NativeCode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_Network_28d2276e.o: ../../src/native/android/juce_android_Network.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_Network.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_OpenGLComponent_8fd9ef4e.o: ../../src/native/android/juce_android_OpenGLComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_OpenGLComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_SystemStats_848ae670.o: ../../src/native/android/juce_android_SystemStats.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_SystemStats.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_Threads_10a24b69.o: ../../src/native/android/juce_android_Threads.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_Threads.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_WebBrowserComponent_178c6569.o: ../../src/native/android/juce_android_WebBrowserComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_WebBrowserComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_android_Windowing_2a620a72.o: ../../src/native/android/juce_android_Windowing.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_android_Windowing.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_CharacterFunctions_b1a5ac62.o: ../../src/text/juce_CharacterFunctions.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_CharacterFunctions.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Identifier_e99ee619.o: ../../src/text/juce_Identifier.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Identifier.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_JSON_bbf64af8.o: ../../src/text/juce_JSON.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_JSON.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_LocalisedStrings_3ddfa55e.o: ../../src/text/juce_LocalisedStrings.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_LocalisedStrings.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_String_80587b01.o: ../../src/text/juce_String.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_String.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_StringArray_4aaa67a2.o: ../../src/text/juce_StringArray.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_StringArray.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_StringPairArray_37130fe8.o: ../../src/text/juce_StringPairArray.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_StringPairArray.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_StringPool_9e8d033d.o: ../../src/text/juce_StringPool.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_StringPool.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_XmlDocument_6b9b0c2c.o: ../../src/text/juce_XmlDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_XmlDocument.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_XmlElement_51daa55.o: ../../src/text/juce_XmlElement.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_XmlElement.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ReadWriteLock_7acf3cc8.o: ../../src/threads/juce_ReadWriteLock.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ReadWriteLock.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_Thread_69734d40.o: ../../src/threads/juce_Thread.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_Thread.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_ThreadPool_fd669dfc.o: ../../src/threads/juce_ThreadPool.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_ThreadPool.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_TimeSliceThread_e9db3803.o: ../../src/threads/juce_TimeSliceThread.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_TimeSliceThread.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_DeletedAtShutdown_b9105375.o: ../../src/utilities/juce_DeletedAtShutdown.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_DeletedAtShutdown.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_FileBasedDocument_d507d0a5.o: ../../src/utilities/juce_FileBasedDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_FileBasedDocument.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PropertiesFile_8faca026.o: ../../src/utilities/juce_PropertiesFile.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_PropertiesFile.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RecentlyOpenedFilesList_f791257.o: ../../src/utilities/juce_RecentlyOpenedFilesList.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_RecentlyOpenedFilesList.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_UndoManager_33b197fc.o: ../../src/utilities/juce_UndoManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_UndoManager.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_UnitTest_4a02c10d.o: ../../src/utilities/juce_UnitTest.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_UnitTest.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
