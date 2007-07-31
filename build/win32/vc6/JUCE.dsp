# Microsoft Developer Studio Project File - Name="JUCE" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=JUCE - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "JUCE.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "JUCE.mak" CFG="JUCE - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "JUCE - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "JUCE - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "JUCE - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../bin/intermediate_win32/static"
# PROP Intermediate_Dir "../../../bin/intermediate_win32/static"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MT /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../../bin/jucelib_static_Win32.lib"

!ELSEIF  "$(CFG)" == "JUCE - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../bin/intermediate_win32/staticdebug"
# PROP Intermediate_Dir "../../../bin/intermediate_win32/staticdebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MTd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../../bin/jucelib_static_Win32_debug.lib"

!ENDIF 

# Begin Target

# Name "JUCE - Win32 Release"
# Name "JUCE - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "appframework"

# PROP Default_Filter ""
# Begin Group "application"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_Application.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_Application.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_ApplicationCommandID.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_ApplicationCommandInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_ApplicationCommandInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_ApplicationCommandManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_ApplicationCommandManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_ApplicationCommandTarget.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_ApplicationCommandTarget.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_ApplicationProperties.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_ApplicationProperties.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_DeletedAtShutdown.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_DeletedAtShutdown.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_PropertiesFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_PropertiesFile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\application\juce_SystemClipboard.h
# End Source File
# End Group
# Begin Group "audio"

# PROP Default_Filter ""
# Begin Group "audio_file_formats"

# PROP Default_Filter ""
# Begin Group "flac"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\flac\libFLAC\bitbuffer.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\flac\libFLAC\bitmath.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\flac\libFLAC\cpu.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\flac\libFLAC\crc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\flac\libFLAC\fixed.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\flac\libFLAC\float.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\flac\libFLAC\format.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\flac\libFLAC\juce_FlacHeader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\flac\libFLAC\lpc_flac.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\flac\libFLAC\md5.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\flac\libFLAC\memory.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\flac\libFLAC\stream_decoder.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\flac\libFLAC\stream_encoder.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\flac\libFLAC\stream_encoder_framing.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\flac\libFLAC\window_flac.c
# End Source File
# End Group
# Begin Group "ogg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\bitwise.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\framing.c
# End Source File
# End Group
# Begin Group "vorbis"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\analysis.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\bitrate.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\block.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\codebook.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\envelope.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\floor0.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\floor1.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\info.c"
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\juce_OggVorbisHeader.h
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\lpc.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\lsp.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\mapping0.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\mdct.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\psy.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\registry.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\res0.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\sharedbook.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\smallft.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\synthesis.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\vorbisenc.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\vorbisfile.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\src\juce_appframework\audio\audio_file_formats\oggvorbis\libvorbis-1.1.2\lib\window.c"
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_AiffAudioFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_AiffAudioFormat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_AudioCDReader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_AudioCDReader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_AudioFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_AudioFormat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_AudioFormatManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_AudioFormatManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_AudioFormatReader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_AudioFormatWriter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_AudioSubsectionReader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_AudioSubsectionReader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_FlacAudioFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_FlacAudioFormat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_OggVorbisAudioFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_OggVorbisAudioFormat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_WavAudioFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_file_formats\juce_WavAudioFormat.h
# End Source File
# End Group
# Begin Group "audio_sources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_AudioFormatReaderSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_AudioFormatReaderSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_AudioSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_AudioSourcePlayer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_AudioSourcePlayer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_AudioTransportSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_AudioTransportSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_BufferingAudioSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_BufferingAudioSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_ChannelRemappingAudioSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_ChannelRemappingAudioSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_MixerAudioSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_MixerAudioSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_PositionableAudioSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_ResamplingAudioSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_ResamplingAudioSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_ToneGeneratorAudioSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\audio_sources\juce_ToneGeneratorAudioSource.h
# End Source File
# End Group
# Begin Group "devices"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\devices\juce_AudioDeviceManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\devices\juce_AudioDeviceManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\devices\juce_AudioIODevice.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\devices\juce_AudioIODevice.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\devices\juce_AudioIODeviceType.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\devices\juce_AudioIODeviceType.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\devices\juce_MidiInput.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\devices\juce_MidiOutput.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\devices\juce_MidiOutput.h
# End Source File
# End Group
# Begin Group "midi"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\midi\juce_MidiBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\midi\juce_MidiBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\midi\juce_MidiFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\midi\juce_MidiFile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\midi\juce_MidiKeyboardState.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\midi\juce_MidiKeyboardState.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\midi\juce_MidiMessage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\midi\juce_MidiMessage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\midi\juce_MidiMessageCollector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\midi\juce_MidiMessageCollector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\midi\juce_MidiMessageSequence.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\midi\juce_MidiMessageSequence.h
# End Source File
# End Group
# Begin Group "synthesisers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\synthesisers\juce_Sampler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\synthesisers\juce_Sampler.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\synthesisers\juce_Synthesiser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\synthesisers\juce_Synthesiser.h
# End Source File
# End Group
# Begin Group "dsp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\dsp\juce_AudioDataConverters.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\dsp\juce_AudioDataConverters.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\dsp\juce_AudioSampleBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\dsp\juce_AudioSampleBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\dsp\juce_IIRFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\audio\dsp\juce_IIRFilter.h
# End Source File
# End Group
# End Group
# Begin Group "events"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_ActionBroadcaster.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_ActionBroadcaster.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_ActionListener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_ActionListenerList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_ActionListenerList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_AsyncUpdater.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_AsyncUpdater.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_ChangeBroadcaster.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_ChangeBroadcaster.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_ChangeListener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_ChangeListenerList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_ChangeListenerList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_InterprocessConnection.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_InterprocessConnection.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_InterprocessConnectionServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_InterprocessConnectionServer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_Message.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_Message.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_MessageListener.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_MessageListener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_MessageManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_MessageManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_MultiTimer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_MultiTimer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_Timer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\events\juce_Timer.h
# End Source File
# End Group
# Begin Group "gui"

# PROP Default_Filter ""
# Begin Group "graphics"

# PROP Default_Filter ""
# Begin Group "brushes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\brushes\juce_Brush.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\brushes\juce_Brush.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\brushes\juce_GradientBrush.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\brushes\juce_GradientBrush.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\brushes\juce_ImageBrush.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\brushes\juce_ImageBrush.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\brushes\juce_SolidColourBrush.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\brushes\juce_SolidColourBrush.h
# End Source File
# End Group
# Begin Group "colour"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\colour\juce_Colour.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\colour\juce_Colour.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\colour\juce_ColourGradient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\colour\juce_ColourGradient.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\colour\juce_Colours.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\colour\juce_Colours.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\colour\juce_PixelFormats.h
# End Source File
# End Group
# Begin Group "contexts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\contexts\juce_EdgeTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\contexts\juce_EdgeTable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\contexts\juce_Graphics.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\contexts\juce_Graphics.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\contexts\juce_Justification.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\contexts\juce_Justification.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\contexts\juce_LowLevelGraphicsContext.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\contexts\juce_LowLevelGraphicsPostScriptRenderer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\contexts\juce_LowLevelGraphicsPostScriptRenderer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\contexts\juce_LowLevelGraphicsSoftwareRenderer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\contexts\juce_LowLevelGraphicsSoftwareRenderer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\contexts\juce_RectanglePlacement.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\contexts\juce_RectanglePlacement.h
# End Source File
# End Group
# Begin Group "fonts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\fonts\juce_Font.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\fonts\juce_Font.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\fonts\juce_GlyphArrangement.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\fonts\juce_GlyphArrangement.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\fonts\juce_TextLayout.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\fonts\juce_TextLayout.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\fonts\juce_Typeface.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\fonts\juce_TypeFace.h
# End Source File
# End Group
# Begin Group "geometry"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_AffineTransform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_AffineTransform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_BorderSize.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_BorderSize.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_Line.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_Line.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_Path.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_Path.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_PathIterator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_PathIterator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_PathStrokeType.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_PathStrokeType.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_Point.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_Point.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_PositionedRectangle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_PositionedRectangle.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_Rectangle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_Rectangle.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_RectangleList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\geometry\juce_RectangleList.h
# End Source File
# End Group
# Begin Group "imaging"

# PROP Default_Filter ""
# Begin Group "image_file_formats"

# PROP Default_Filter ""
# Begin Group "jpglib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\cderror.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jcapimin.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jcapistd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jccoefct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jccolor.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jcdctmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jchuff.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jchuff.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jcinit.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jcmainct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jcmarker.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jcmaster.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jcomapi.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jconfig.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jcparam.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jcphuff.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jcprepct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jcsample.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jctrans.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdapimin.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdapistd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdatasrc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdcoefct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdcolor.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdct.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jddctmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdhuff.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdhuff.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdinput.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdmainct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdmarker.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdmaster.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdmerge.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdphuff.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdpostct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdsample.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jdtrans.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jerror.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jerror.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jfdctflt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jfdctfst.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jfdctint.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jidctflt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jidctfst.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jidctint.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jidctred.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jinclude.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jmemmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jmemnobs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jmemsys.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jmorecfg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jpegint.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jpeglib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jquant1.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jquant2.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jutils.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\jversion.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\transupp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\jpglib\transupp.h
# End Source File
# End Group
# Begin Group "pnglib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\png.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\png.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngconf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngerror.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pnggccrd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngget.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngmem.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngpread.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngread.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngrio.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngrtran.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngrutil.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngset.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngtrans.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngvcrd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngwio.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngwrite.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngwtran.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\pnglib\pngwutil.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\juce_GIFLoader.cpp
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\juce_JPEGLoader.cpp
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\image_file_formats\juce_PNGLoader.cpp
# ADD CPP /W1
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\juce_Image.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\juce_Image.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\juce_ImageCache.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\juce_ImageCache.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\juce_ImageConvolutionKernel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\juce_ImageConvolutionKernel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\juce_ImageFileFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\imaging\juce_ImageFileFormat.h
# End Source File
# End Group
# Begin Group "effects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\effects\juce_DropShadowEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\effects\juce_DropShadowEffect.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\effects\juce_GlowEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\effects\juce_GlowEffect.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\effects\juce_ImageEffectFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\effects\juce_ReduceOpacityEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\effects\juce_ReduceOpacityEffect.h
# End Source File
# End Group
# Begin Group "drawables"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\drawables\juce_Drawable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\drawables\juce_Drawable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\drawables\juce_DrawableComposite.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\drawables\juce_DrawableComposite.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\drawables\juce_DrawableImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\drawables\juce_DrawableImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\drawables\juce_DrawablePath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\drawables\juce_DrawablePath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\drawables\juce_DrawableText.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\drawables\juce_DrawableText.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\graphics\drawables\juce_SVGParser.cpp
# End Source File
# End Group
# End Group
# Begin Group "components"

# PROP Default_Filter ""
# Begin Group "buttons"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_ArrowButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_ArrowButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_Button.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_Button.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_DrawableButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_DrawableButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_HyperlinkButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_HyperlinkButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_ImageButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_ImageButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_ShapeButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_ShapeButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_TextButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_TextButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_ToggleButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_ToggleButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_ToolbarButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\buttons\juce_ToolbarButton.h
# End Source File
# End Group
# Begin Group "controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_ComboBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_ComboBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_Label.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_Label.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_ListBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_ListBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_ProgressBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_ProgressBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_Slider.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_Slider.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_TableHeaderComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_TableHeaderComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_TableListBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_TableListBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_TextEditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_TextEditor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_Toolbar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_Toolbar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_ToolbarItemComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_ToolbarItemComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_ToolbarItemFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_ToolbarItemPalette.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_ToolbarItemPalette.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_TreeView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\controls\juce_TreeView.h
# End Source File
# End Group
# Begin Group "keyboard"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\keyboard\juce_KeyboardFocusTraverser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\keyboard\juce_KeyboardFocusTraverser.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\keyboard\juce_KeyListener.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\keyboard\juce_KeyListener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\keyboard\juce_KeyMappingEditorComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\keyboard\juce_KeyMappingEditorComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\keyboard\juce_KeyPress.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\keyboard\juce_KeyPress.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\keyboard\juce_KeyPressMappingSet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\keyboard\juce_KeyPressMappingSet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\keyboard\juce_ModifierKeys.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\keyboard\juce_ModifierKeys.h
# End Source File
# End Group
# Begin Group "layout"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_ComponentAnimator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_ComponentAnimator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_ComponentBoundsConstrainer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_ComponentBoundsConstrainer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_ComponentMovementWatcher.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_ComponentMovementWatcher.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_GroupComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_GroupComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_MultiDocumentPanel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_MultiDocumentPanel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_ResizableBase.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_ResizableBorderComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_ResizableBorderComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_ResizableCornerComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_ResizableCornerComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_ScrollBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_ScrollBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_StretchableLayoutManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_StretchableLayoutManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_StretchableLayoutResizerBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_StretchableLayoutResizerBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_StretchableObjectResizer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_StretchableObjectResizer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_TabbedButtonBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_TabbedButtonBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_TabbedComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_TabbedComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_Viewport.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\layout\juce_Viewport.h
# End Source File
# End Group
# Begin Group "lookandfeel"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\lookandfeel\juce_LookAndFeel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\lookandfeel\juce_LookAndFeel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\lookandfeel\juce_OldSchoolLookAndFeel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\lookandfeel\juce_OldSchoolLookAndFeel.h
# End Source File
# End Group
# Begin Group "menus"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\menus\juce_MenuBarComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\menus\juce_MenuBarComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\menus\juce_MenuBarModel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\menus\juce_MenuBarModel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\menus\juce_PopupMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\menus\juce_PopupMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\menus\juce_PopupMenuCustomComponent.h
# End Source File
# End Group
# Begin Group "mouse"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\mouse\juce_ComponentDragger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\mouse\juce_ComponentDragger.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\mouse\juce_DragAndDropContainer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\mouse\juce_DragAndDropContainer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\mouse\juce_DragAndDropTarget.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\mouse\juce_LassoComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\mouse\juce_MouseCursor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\mouse\juce_MouseCursor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\mouse\juce_MouseEvent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\mouse\juce_MouseEvent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\mouse\juce_MouseHoverDetector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\mouse\juce_MouseHoverDetector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\mouse\juce_MouseListener.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\mouse\juce_MouseListener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\mouse\juce_TooltipClient.h
# End Source File
# End Group
# Begin Group "special"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_ActiveXControlComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_AudioDeviceSelectorComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_AudioDeviceSelectorComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_BubbleComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_BubbleComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_BubbleMessageComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_BubbleMessageComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_ColourSelector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_ColourSelector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_DropShadower.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_DropShadower.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_MagnifierComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_MagnifierComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_MidiKeyboardComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_MidiKeyboardComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_OpenGLComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_OpenGLComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_PreferencesPanel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_PreferencesPanel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_QuickTimeMovieComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_QuickTimeMovieComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_SystemTrayIconComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\special\juce_SystemTrayIconComponent.h
# End Source File
# End Group
# Begin Group "windows"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_AlertWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_AlertWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_ComponentPeer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_ComponentPeer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_DialogWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_DialogWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_DocumentWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_DocumentWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_ResizableWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_ResizableWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_SplashScreen.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_SplashScreen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_ThreadWithProgressWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_ThreadWithProgressWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_ToolTipWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_ToolTipWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_TopLevelWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\windows\juce_TopLevelWindow.h
# End Source File
# End Group
# Begin Group "filebrowser"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_DirectoryContentsDisplayComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_DirectoryContentsDisplayComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_DirectoryContentsList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_DirectoryContentsList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FileBrowserComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FileBrowserComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FileBrowserListener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FileChooser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FileChooser.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FileChooserDialogBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FileChooserDialogBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FileFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FileFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FileListComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FileListComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FilenameComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FilenameComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FilePreviewComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FileTreeComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_FileTreeComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_ImagePreviewComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_ImagePreviewComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_WildcardFileFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\filebrowser\juce_WildcardFileFilter.h
# End Source File
# End Group
# Begin Group "properties"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\properties\juce_BooleanPropertyComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\properties\juce_BooleanPropertyComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\properties\juce_ButtonPropertyComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\properties\juce_ButtonPropertyComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\properties\juce_ChoicePropertyComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\properties\juce_ChoicePropertyComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\properties\juce_PropertyComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\properties\juce_PropertyComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\properties\juce_PropertyPanel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\properties\juce_PropertyPanel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\properties\juce_SliderPropertyComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\properties\juce_SliderPropertyComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\properties\juce_TextPropertyComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\properties\juce_TextPropertyComponent.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\juce_Component.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\juce_Component.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\juce_ComponentDeletionWatcher.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\juce_ComponentListener.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\juce_ComponentListener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\juce_Desktop.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\gui\components\juce_Desktop.h
# End Source File
# End Group
# End Group
# Begin Group "documents"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\documents\juce_FileBasedDocument.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\documents\juce_FileBasedDocument.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\documents\juce_RecentlyOpenedFilesList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\documents\juce_RecentlyOpenedFilesList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\documents\juce_SelectedItemSet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\documents\juce_UndoableAction.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\documents\juce_UndoManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_appframework\documents\juce_UndoManager.h
# End Source File
# End Group
# End Group
# Begin Group "core"

# PROP Default_Filter ""
# Begin Group "basics"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_Atomic.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_DataConversions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_FileLogger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_FileLogger.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_Logger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_Logger.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_MathsFunctions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_Memory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_PlatformDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_Random.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_Random.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_RelativeTime.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_RelativeTime.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_Singleton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_StandardHeader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_SystemStats.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_SystemStats.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_Time.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\basics\juce_Time.h
# End Source File
# End Group
# Begin Group "containers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_core\containers\juce_Array.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\containers\juce_ArrayAllocationBase.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\containers\juce_BitArray.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\containers\juce_BitArray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\containers\juce_ElementComparator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\containers\juce_MemoryBlock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\containers\juce_MemoryBlock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\containers\juce_OwnedArray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\containers\juce_PropertySet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\containers\juce_PropertySet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\containers\juce_ReferenceCountedArray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\containers\juce_ReferenceCountedObject.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\containers\juce_SortedSet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\containers\juce_SparseSet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\containers\juce_VoidArray.h
# End Source File
# End Group
# Begin Group "io"

# PROP Default_Filter ""
# Begin Group "files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\files\juce_DirectoryIterator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\files\juce_DirectoryIterator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\files\juce_File.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\files\juce_File.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\files\juce_FileInputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\files\juce_FileInputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\files\juce_FileOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\files\juce_FileOutputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\files\juce_FileSearchPath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\files\juce_FileSearchPath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\files\juce_NamedPipe.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\files\juce_NamedPipe.h
# End Source File
# End Group
# Begin Group "network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\network\juce_Socket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\network\juce_Socket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\network\juce_URL.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\network\juce_URL.h
# End Source File
# End Group
# Begin Group "streams"

# PROP Default_Filter ""
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\zlib\compress.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\zlib\deflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\zlib\infback.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\zlib\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\zlib\zutil.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\juce_BufferedInputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\juce_BufferedInputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\juce_GZIPCompressorOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\juce_GZIPCompressorOutputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\juce_GZIPDecompressorInputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\juce_GZIPDecompressorInputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\juce_MemoryInputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\juce_MemoryInputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\juce_MemoryOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\juce_MemoryOutputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\juce_SubregionStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\streams\juce_SubregionStream.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\juce_InputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\juce_InputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\juce_OutputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\io\juce_OutputStream.h
# End Source File
# End Group
# Begin Group "misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_core\misc\juce_PerformanceCounter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\misc\juce_PerformanceCounter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\misc\juce_PlatformUtilities.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\misc\juce_Uuid.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\misc\juce_Uuid.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\misc\juce_ZipFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\misc\juce_ZipFile.h
# End Source File
# End Group
# Begin Group "text"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_core\text\juce_CharacterFunctions.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\text\juce_CharacterFunctions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\text\juce_LocalisedStrings.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\text\juce_LocalisedStrings.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\text\juce_String.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\text\juce_String.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\text\juce_StringArray.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\text\juce_StringArray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\text\juce_StringPairArray.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\text\juce_StringPairArray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\text\juce_TextFunctions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\text\juce_XmlDocument.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\text\juce_XmlDocument.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\text\juce_XmlElement.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\text\juce_XmlElement.h
# End Source File
# End Group
# Begin Group "threads"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_core\threads\juce_CriticalSection.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\threads\juce_InterProcessLock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\threads\juce_InterProcessLock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\threads\juce_Process.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\threads\juce_ReadWriteLock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\threads\juce_ReadWriteLock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\threads\juce_ScopedLock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\threads\juce_Thread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\threads\juce_Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\threads\juce_ThreadPool.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\threads\juce_ThreadPool.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\threads\juce_TimeSliceThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\threads\juce_TimeSliceThread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\threads\juce_WaitableEvent.h
# End Source File
# End Group
# Begin Group "cryptography"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\juce_core\cryptography\juce_BlowFish.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\cryptography\juce_BlowFish.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\cryptography\juce_MD5.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\cryptography\juce_MD5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\cryptography\juce_Primes.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\cryptography\juce_Primes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\cryptography\juce_RSAKey.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\juce_core\cryptography\juce_RSAKey.h
# End Source File
# End Group
# End Group
# Begin Group "win32_code"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\platform_specific_code\juce_win32_ASIO.cpp
# End Source File
# Begin Source File

SOURCE=..\platform_specific_code\juce_win32_AudioCDReader.cpp
# End Source File
# Begin Source File

SOURCE=..\platform_specific_code\juce_win32_DirectSound.cpp
# End Source File
# Begin Source File

SOURCE=..\platform_specific_code\juce_win32_DynamicLibraryLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\platform_specific_code\juce_win32_FileChooser.cpp
# End Source File
# Begin Source File

SOURCE=..\platform_specific_code\juce_win32_Files.cpp
# End Source File
# Begin Source File

SOURCE=..\platform_specific_code\juce_win32_Fonts.cpp
# End Source File
# Begin Source File

SOURCE=..\platform_specific_code\juce_win32_Messaging.cpp
# End Source File
# Begin Source File

SOURCE=..\platform_specific_code\juce_win32_Midi.cpp
# End Source File
# Begin Source File

SOURCE=..\platform_specific_code\juce_win32_Misc.cpp
# End Source File
# Begin Source File

SOURCE=..\platform_specific_code\juce_win32_Network.cpp
# End Source File
# Begin Source File

SOURCE=..\platform_specific_code\juce_win32_PlatformUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\platform_specific_code\juce_win32_SystemStats.cpp
# End Source File
# Begin Source File

SOURCE=..\platform_specific_code\juce_win32_Threads.cpp
# End Source File
# Begin Source File

SOURCE=..\platform_specific_code\juce_win32_Windowing.cpp
# End Source File
# Begin Source File

SOURCE=..\platform_specific_code\win32_headers.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\juce.h
# End Source File
# Begin Source File

SOURCE=..\..\..\juce_Config.h
# End Source File
# End Group
# Begin Source File

SOURCE="..\..\..\docs\JUCE changelist.txt"
# End Source File
# End Target
# End Project
