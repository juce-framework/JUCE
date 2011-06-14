/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_JUCE_APP_INCLUDES_INCLUDEFILES__
#define __JUCE_JUCE_APP_INCLUDES_INCLUDEFILES__

#ifndef __JUCE_APPLICATION_JUCEHEADER__
 #include "application/juce_Application.h"
#endif
#ifndef __JUCE_APPLICATIONCOMMANDID_JUCEHEADER__
 #include "application/juce_ApplicationCommandID.h"
#endif
#ifndef __JUCE_APPLICATIONCOMMANDINFO_JUCEHEADER__
 #include "application/juce_ApplicationCommandInfo.h"
#endif
#ifndef __JUCE_APPLICATIONCOMMANDMANAGER_JUCEHEADER__
 #include "application/juce_ApplicationCommandManager.h"
#endif
#ifndef __JUCE_APPLICATIONCOMMANDTARGET_JUCEHEADER__
 #include "application/juce_ApplicationCommandTarget.h"
#endif
#ifndef __JUCE_APPLICATIONPROPERTIES_JUCEHEADER__
 #include "application/juce_ApplicationProperties.h"
#endif
#ifndef __JUCE_AIFFAUDIOFORMAT_JUCEHEADER__
 #include "audio/audio_file_formats/juce_AiffAudioFormat.h"
#endif
#ifndef __JUCE_AUDIOCDBURNER_JUCEHEADER__
 #include "audio/audio_file_formats/juce_AudioCDBurner.h"
#endif
#ifndef __JUCE_AUDIOCDREADER_JUCEHEADER__
 #include "audio/audio_file_formats/juce_AudioCDReader.h"
#endif
#ifndef __JUCE_AUDIOFORMAT_JUCEHEADER__
 #include "audio/audio_file_formats/juce_AudioFormat.h"
#endif
#ifndef __JUCE_AUDIOFORMATMANAGER_JUCEHEADER__
 #include "audio/audio_file_formats/juce_AudioFormatManager.h"
#endif
#ifndef __JUCE_AUDIOFORMATREADER_JUCEHEADER__
 #include "audio/audio_file_formats/juce_AudioFormatReader.h"
#endif
#ifndef __JUCE_AUDIOFORMATWRITER_JUCEHEADER__
 #include "audio/audio_file_formats/juce_AudioFormatWriter.h"
#endif
#ifndef __JUCE_AUDIOSUBSECTIONREADER_JUCEHEADER__
 #include "audio/audio_file_formats/juce_AudioSubsectionReader.h"
#endif
#ifndef __JUCE_AUDIOTHUMBNAIL_JUCEHEADER__
 #include "audio/audio_file_formats/juce_AudioThumbnail.h"
#endif
#ifndef __JUCE_AUDIOTHUMBNAILCACHE_JUCEHEADER__
 #include "audio/audio_file_formats/juce_AudioThumbnailCache.h"
#endif
#ifndef __JUCE_FLACAUDIOFORMAT_JUCEHEADER__
 #include "audio/audio_file_formats/juce_FlacAudioFormat.h"
#endif
#ifndef __JUCE_OGGVORBISAUDIOFORMAT_JUCEHEADER__
 #include "audio/audio_file_formats/juce_OggVorbisAudioFormat.h"
#endif
#ifndef __JUCE_QUICKTIMEAUDIOFORMAT_JUCEHEADER__
 #include "audio/audio_file_formats/juce_QuickTimeAudioFormat.h"
#endif
#ifndef __JUCE_WAVAUDIOFORMAT_JUCEHEADER__
 #include "audio/audio_file_formats/juce_WavAudioFormat.h"
#endif
#ifndef __JUCE_AUDIOFORMATREADERSOURCE_JUCEHEADER__
 #include "audio/audio_sources/juce_AudioFormatReaderSource.h"
#endif
#ifndef __JUCE_AUDIOSOURCE_JUCEHEADER__
 #include "audio/audio_sources/juce_AudioSource.h"
#endif
#ifndef __JUCE_AUDIOSOURCEPLAYER_JUCEHEADER__
 #include "audio/audio_sources/juce_AudioSourcePlayer.h"
#endif
#ifndef __JUCE_AUDIOTRANSPORTSOURCE_JUCEHEADER__
 #include "audio/audio_sources/juce_AudioTransportSource.h"
#endif
#ifndef __JUCE_BUFFERINGAUDIOSOURCE_JUCEHEADER__
 #include "audio/audio_sources/juce_BufferingAudioSource.h"
#endif
#ifndef __JUCE_CHANNELREMAPPINGAUDIOSOURCE_JUCEHEADER__
 #include "audio/audio_sources/juce_ChannelRemappingAudioSource.h"
#endif
#ifndef __JUCE_IIRFILTERAUDIOSOURCE_JUCEHEADER__
 #include "audio/audio_sources/juce_IIRFilterAudioSource.h"
#endif
#ifndef __JUCE_MIXERAUDIOSOURCE_JUCEHEADER__
 #include "audio/audio_sources/juce_MixerAudioSource.h"
#endif
#ifndef __JUCE_POSITIONABLEAUDIOSOURCE_JUCEHEADER__
 #include "audio/audio_sources/juce_PositionableAudioSource.h"
#endif
#ifndef __JUCE_RESAMPLINGAUDIOSOURCE_JUCEHEADER__
 #include "audio/audio_sources/juce_ResamplingAudioSource.h"
#endif
#ifndef __JUCE_REVERBAUDIOSOURCE_JUCEHEADER__
 #include "audio/audio_sources/juce_ReverbAudioSource.h"
#endif
#ifndef __JUCE_TONEGENERATORAUDIOSOURCE_JUCEHEADER__
 #include "audio/audio_sources/juce_ToneGeneratorAudioSource.h"
#endif
#ifndef __JUCE_AUDIODEVICEMANAGER_JUCEHEADER__
 #include "audio/devices/juce_AudioDeviceManager.h"
#endif
#ifndef __JUCE_AUDIOIODEVICE_JUCEHEADER__
 #include "audio/devices/juce_AudioIODevice.h"
#endif
#ifndef __JUCE_AUDIOIODEVICETYPE_JUCEHEADER__
 #include "audio/devices/juce_AudioIODeviceType.h"
#endif
#ifndef __JUCE_AUDIODATACONVERTERS_JUCEHEADER__
 #include "audio/dsp/juce_AudioDataConverters.h"
#endif
#ifndef __JUCE_AUDIOSAMPLEBUFFER_JUCEHEADER__
 #include "audio/dsp/juce_AudioSampleBuffer.h"
#endif
#ifndef __JUCE_DECIBELS_JUCEHEADER__
 #include "audio/dsp/juce_Decibels.h"
#endif
#ifndef __JUCE_IIRFILTER_JUCEHEADER__
 #include "audio/dsp/juce_IIRFilter.h"
#endif
#ifndef __JUCE_REVERB_JUCEHEADER__
 #include "audio/dsp/juce_Reverb.h"
#endif
#ifndef __JUCE_MIDIBUFFER_JUCEHEADER__
 #include "audio/midi/juce_MidiBuffer.h"
#endif
#ifndef __JUCE_MIDIFILE_JUCEHEADER__
 #include "audio/midi/juce_MidiFile.h"
#endif
#ifndef __JUCE_MIDIINPUT_JUCEHEADER__
 #include "audio/midi/juce_MidiInput.h"
#endif
#ifndef __JUCE_MIDIKEYBOARDSTATE_JUCEHEADER__
 #include "audio/midi/juce_MidiKeyboardState.h"
#endif
#ifndef __JUCE_MIDIMESSAGE_JUCEHEADER__
 #include "audio/midi/juce_MidiMessage.h"
#endif
#ifndef __JUCE_MIDIMESSAGECOLLECTOR_JUCEHEADER__
 #include "audio/midi/juce_MidiMessageCollector.h"
#endif
#ifndef __JUCE_MIDIMESSAGESEQUENCE_JUCEHEADER__
 #include "audio/midi/juce_MidiMessageSequence.h"
#endif
#ifndef __JUCE_MIDIOUTPUT_JUCEHEADER__
 #include "audio/midi/juce_MidiOutput.h"
#endif
#ifndef __JUCE_AUDIOUNITPLUGINFORMAT_JUCEHEADER__
 #include "audio/plugin_host/formats/juce_AudioUnitPluginFormat.h"
#endif
#ifndef __JUCE_DIRECTXPLUGINFORMAT_JUCEHEADER__
 #include "audio/plugin_host/formats/juce_DirectXPluginFormat.h"
#endif
#ifndef __JUCE_LADSPAPLUGINFORMAT_JUCEHEADER__
 #include "audio/plugin_host/formats/juce_LADSPAPluginFormat.h"
#endif
#ifndef __JUCE_VSTMIDIEVENTLIST_JUCEHEADER__
 #include "audio/plugin_host/formats/juce_VSTMidiEventList.h"
#endif
#ifndef __JUCE_VSTPLUGINFORMAT_JUCEHEADER__
 #include "audio/plugin_host/formats/juce_VSTPluginFormat.h"
#endif
#ifndef __JUCE_AUDIOPLUGINFORMAT_JUCEHEADER__
 #include "audio/plugin_host/juce_AudioPluginFormat.h"
#endif
#ifndef __JUCE_AUDIOPLUGINFORMATMANAGER_JUCEHEADER__
 #include "audio/plugin_host/juce_AudioPluginFormatManager.h"
#endif
#ifndef __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__
 #include "audio/plugin_host/juce_AudioPluginInstance.h"
#endif
#ifndef __JUCE_KNOWNPLUGINLIST_JUCEHEADER__
 #include "audio/plugin_host/juce_KnownPluginList.h"
#endif
#ifndef __JUCE_PLUGINDESCRIPTION_JUCEHEADER__
 #include "audio/plugin_host/juce_PluginDescription.h"
#endif
#ifndef __JUCE_PLUGINDIRECTORYSCANNER_JUCEHEADER__
 #include "audio/plugin_host/juce_PluginDirectoryScanner.h"
#endif
#ifndef __JUCE_PLUGINLISTCOMPONENT_JUCEHEADER__
 #include "audio/plugin_host/juce_PluginListComponent.h"
#endif
#ifndef __JUCE_AUDIOPLAYHEAD_JUCEHEADER__
 #include "audio/processors/juce_AudioPlayHead.h"
#endif
#ifndef __JUCE_AUDIOPROCESSOR_JUCEHEADER__
 #include "audio/processors/juce_AudioProcessor.h"
#endif
#ifndef __JUCE_AUDIOPROCESSOREDITOR_JUCEHEADER__
 #include "audio/processors/juce_AudioProcessorEditor.h"
#endif
#ifndef __JUCE_AUDIOPROCESSORGRAPH_JUCEHEADER__
 #include "audio/processors/juce_AudioProcessorGraph.h"
#endif
#ifndef __JUCE_AUDIOPROCESSORLISTENER_JUCEHEADER__
 #include "audio/processors/juce_AudioProcessorListener.h"
#endif
#ifndef __JUCE_AUDIOPROCESSORPLAYER_JUCEHEADER__
 #include "audio/processors/juce_AudioProcessorPlayer.h"
#endif
#ifndef __JUCE_GENERICAUDIOPROCESSOREDITOR_JUCEHEADER__
 #include "audio/processors/juce_GenericAudioProcessorEditor.h"
#endif
#ifndef __JUCE_SAMPLER_JUCEHEADER__
 #include "audio/synthesisers/juce_Sampler.h"
#endif
#ifndef __JUCE_SYNTHESISER_JUCEHEADER__
 #include "audio/synthesisers/juce_Synthesiser.h"
#endif
#ifndef __JUCE_ACTIONBROADCASTER_JUCEHEADER__
 #include "events/juce_ActionBroadcaster.h"
#endif
#ifndef __JUCE_ACTIONLISTENER_JUCEHEADER__
 #include "events/juce_ActionListener.h"
#endif
#ifndef __JUCE_ASYNCUPDATER_JUCEHEADER__
 #include "events/juce_AsyncUpdater.h"
#endif
#ifndef __JUCE_CALLBACKMESSAGE_JUCEHEADER__
 #include "events/juce_CallbackMessage.h"
#endif
#ifndef __JUCE_CHANGEBROADCASTER_JUCEHEADER__
 #include "events/juce_ChangeBroadcaster.h"
#endif
#ifndef __JUCE_CHANGELISTENER_JUCEHEADER__
 #include "events/juce_ChangeListener.h"
#endif
#ifndef __JUCE_INTERPROCESSCONNECTION_JUCEHEADER__
 #include "events/juce_InterprocessConnection.h"
#endif
#ifndef __JUCE_INTERPROCESSCONNECTIONSERVER_JUCEHEADER__
 #include "events/juce_InterprocessConnectionServer.h"
#endif
#ifndef __JUCE_LISTENERLIST_JUCEHEADER__
 #include "events/juce_ListenerList.h"
#endif
#ifndef __JUCE_MESSAGE_JUCEHEADER__
 #include "events/juce_Message.h"
#endif
#ifndef __JUCE_MESSAGELISTENER_JUCEHEADER__
 #include "events/juce_MessageListener.h"
#endif
#ifndef __JUCE_MESSAGEMANAGER_JUCEHEADER__
 #include "events/juce_MessageManager.h"
#endif
#ifndef __JUCE_MULTITIMER_JUCEHEADER__
 #include "events/juce_MultiTimer.h"
#endif
#ifndef __JUCE_TIMER_JUCEHEADER__
 #include "events/juce_Timer.h"
#endif
#ifndef __JUCE_ARROWBUTTON_JUCEHEADER__
 #include "gui/components/buttons/juce_ArrowButton.h"
#endif
#ifndef __JUCE_BUTTON_JUCEHEADER__
 #include "gui/components/buttons/juce_Button.h"
#endif
#ifndef __JUCE_DRAWABLEBUTTON_JUCEHEADER__
 #include "gui/components/buttons/juce_DrawableButton.h"
#endif
#ifndef __JUCE_HYPERLINKBUTTON_JUCEHEADER__
 #include "gui/components/buttons/juce_HyperlinkButton.h"
#endif
#ifndef __JUCE_IMAGEBUTTON_JUCEHEADER__
 #include "gui/components/buttons/juce_ImageButton.h"
#endif
#ifndef __JUCE_SHAPEBUTTON_JUCEHEADER__
 #include "gui/components/buttons/juce_ShapeButton.h"
#endif
#ifndef __JUCE_TEXTBUTTON_JUCEHEADER__
 #include "gui/components/buttons/juce_TextButton.h"
#endif
#ifndef __JUCE_TOGGLEBUTTON_JUCEHEADER__
 #include "gui/components/buttons/juce_ToggleButton.h"
#endif
#ifndef __JUCE_TOOLBARBUTTON_JUCEHEADER__
 #include "gui/components/buttons/juce_ToolbarButton.h"
#endif
#ifndef __JUCE_CODEDOCUMENT_JUCEHEADER__
 #include "gui/components/code_editor/juce_CodeDocument.h"
#endif
#ifndef __JUCE_CODEEDITORCOMPONENT_JUCEHEADER__
 #include "gui/components/code_editor/juce_CodeEditorComponent.h"
#endif
#ifndef __JUCE_CODETOKENISER_JUCEHEADER__
 #include "gui/components/code_editor/juce_CodeTokeniser.h"
#endif
#ifndef __JUCE_CPLUSPLUSCODETOKENISER_JUCEHEADER__
 #include "gui/components/code_editor/juce_CPlusPlusCodeTokeniser.h"
#endif
#ifndef __JUCE_COMBOBOX_JUCEHEADER__
 #include "gui/components/controls/juce_ComboBox.h"
#endif
#ifndef __JUCE_IMAGECOMPONENT_JUCEHEADER__
 #include "gui/components/controls/juce_ImageComponent.h"
#endif
#ifndef __JUCE_LABEL_JUCEHEADER__
 #include "gui/components/controls/juce_Label.h"
#endif
#ifndef __JUCE_LISTBOX_JUCEHEADER__
 #include "gui/components/controls/juce_ListBox.h"
#endif
#ifndef __JUCE_PROGRESSBAR_JUCEHEADER__
 #include "gui/components/controls/juce_ProgressBar.h"
#endif
#ifndef __JUCE_SLIDER_JUCEHEADER__
 #include "gui/components/controls/juce_Slider.h"
#endif
#ifndef __JUCE_TABLEHEADERCOMPONENT_JUCEHEADER__
 #include "gui/components/controls/juce_TableHeaderComponent.h"
#endif
#ifndef __JUCE_TABLELISTBOX_JUCEHEADER__
 #include "gui/components/controls/juce_TableListBox.h"
#endif
#ifndef __JUCE_TEXTEDITOR_JUCEHEADER__
 #include "gui/components/controls/juce_TextEditor.h"
#endif
#ifndef __JUCE_TOOLBAR_JUCEHEADER__
 #include "gui/components/controls/juce_Toolbar.h"
#endif
#ifndef __JUCE_TOOLBARITEMCOMPONENT_JUCEHEADER__
 #include "gui/components/controls/juce_ToolbarItemComponent.h"
#endif
#ifndef __JUCE_TOOLBARITEMFACTORY_JUCEHEADER__
 #include "gui/components/controls/juce_ToolbarItemFactory.h"
#endif
#ifndef __JUCE_TOOLBARITEMPALETTE_JUCEHEADER__
 #include "gui/components/controls/juce_ToolbarItemPalette.h"
#endif
#ifndef __JUCE_TREEVIEW_JUCEHEADER__
 #include "gui/components/controls/juce_TreeView.h"
#endif
#ifndef __JUCE_DIRECTORYCONTENTSDISPLAYCOMPONENT_JUCEHEADER__
 #include "gui/components/filebrowser/juce_DirectoryContentsDisplayComponent.h"
#endif
#ifndef __JUCE_DIRECTORYCONTENTSLIST_JUCEHEADER__
 #include "gui/components/filebrowser/juce_DirectoryContentsList.h"
#endif
#ifndef __JUCE_FILEBROWSERCOMPONENT_JUCEHEADER__
 #include "gui/components/filebrowser/juce_FileBrowserComponent.h"
#endif
#ifndef __JUCE_FILEBROWSERLISTENER_JUCEHEADER__
 #include "gui/components/filebrowser/juce_FileBrowserListener.h"
#endif
#ifndef __JUCE_FILECHOOSER_JUCEHEADER__
 #include "gui/components/filebrowser/juce_FileChooser.h"
#endif
#ifndef __JUCE_FILECHOOSERDIALOGBOX_JUCEHEADER__
 #include "gui/components/filebrowser/juce_FileChooserDialogBox.h"
#endif
#ifndef __JUCE_FILEFILTER_JUCEHEADER__
 #include "gui/components/filebrowser/juce_FileFilter.h"
#endif
#ifndef __JUCE_FILELISTCOMPONENT_JUCEHEADER__
 #include "gui/components/filebrowser/juce_FileListComponent.h"
#endif
#ifndef __JUCE_FILENAMECOMPONENT_JUCEHEADER__
 #include "gui/components/filebrowser/juce_FilenameComponent.h"
#endif
#ifndef __JUCE_FILEPREVIEWCOMPONENT_JUCEHEADER__
 #include "gui/components/filebrowser/juce_FilePreviewComponent.h"
#endif
#ifndef __JUCE_FILESEARCHPATHLISTCOMPONENT_JUCEHEADER__
 #include "gui/components/filebrowser/juce_FileSearchPathListComponent.h"
#endif
#ifndef __JUCE_FILETREECOMPONENT_JUCEHEADER__
 #include "gui/components/filebrowser/juce_FileTreeComponent.h"
#endif
#ifndef __JUCE_IMAGEPREVIEWCOMPONENT_JUCEHEADER__
 #include "gui/components/filebrowser/juce_ImagePreviewComponent.h"
#endif
#ifndef __JUCE_WILDCARDFILEFILTER_JUCEHEADER__
 #include "gui/components/filebrowser/juce_WildcardFileFilter.h"
#endif
#ifndef __JUCE_COMPONENT_JUCEHEADER__
 #include "gui/components/juce_Component.h"
#endif
#ifndef __JUCE_COMPONENTLISTENER_JUCEHEADER__
 #include "gui/components/juce_ComponentListener.h"
#endif
#ifndef __JUCE_DESKTOP_JUCEHEADER__
 #include "gui/components/juce_Desktop.h"
#endif
#ifndef __JUCE_MODALCOMPONENTMANAGER_JUCEHEADER__
 #include "gui/components/juce_ModalComponentManager.h"
#endif
#ifndef __JUCE_CARETCOMPONENT_JUCEHEADER__
 #include "gui/components/keyboard/juce_CaretComponent.h"
#endif
#ifndef __JUCE_KEYBOARDFOCUSTRAVERSER_JUCEHEADER__
 #include "gui/components/keyboard/juce_KeyboardFocusTraverser.h"
#endif
#ifndef __JUCE_KEYLISTENER_JUCEHEADER__
 #include "gui/components/keyboard/juce_KeyListener.h"
#endif
#ifndef __JUCE_KEYMAPPINGEDITORCOMPONENT_JUCEHEADER__
 #include "gui/components/keyboard/juce_KeyMappingEditorComponent.h"
#endif
#ifndef __JUCE_KEYPRESS_JUCEHEADER__
 #include "gui/components/keyboard/juce_KeyPress.h"
#endif
#ifndef __JUCE_KEYPRESSMAPPINGSET_JUCEHEADER__
 #include "gui/components/keyboard/juce_KeyPressMappingSet.h"
#endif
#ifndef __JUCE_MODIFIERKEYS_JUCEHEADER__
 #include "gui/components/keyboard/juce_ModifierKeys.h"
#endif
#ifndef __JUCE_TEXTEDITORKEYMAPPER_JUCEHEADER__
 #include "gui/components/keyboard/juce_TextEditorKeyMapper.h"
#endif
#ifndef __JUCE_TEXTINPUTTARGET_JUCEHEADER__
 #include "gui/components/keyboard/juce_TextInputTarget.h"
#endif
#ifndef __JUCE_COMPONENTANIMATOR_JUCEHEADER__
 #include "gui/components/layout/juce_ComponentAnimator.h"
#endif
#ifndef __JUCE_COMPONENTBOUNDSCONSTRAINER_JUCEHEADER__
 #include "gui/components/layout/juce_ComponentBoundsConstrainer.h"
#endif
#ifndef __JUCE_COMPONENTBUILDER_JUCEHEADER__
 #include "gui/components/layout/juce_ComponentBuilder.h"
#endif
#ifndef __JUCE_COMPONENTMOVEMENTWATCHER_JUCEHEADER__
 #include "gui/components/layout/juce_ComponentMovementWatcher.h"
#endif
#ifndef __JUCE_GROUPCOMPONENT_JUCEHEADER__
 #include "gui/components/layout/juce_GroupComponent.h"
#endif
#ifndef __JUCE_MULTIDOCUMENTPANEL_JUCEHEADER__
 #include "gui/components/layout/juce_MultiDocumentPanel.h"
#endif
#ifndef __JUCE_RESIZABLEBORDERCOMPONENT_JUCEHEADER__
 #include "gui/components/layout/juce_ResizableBorderComponent.h"
#endif
#ifndef __JUCE_RESIZABLECORNERCOMPONENT_JUCEHEADER__
 #include "gui/components/layout/juce_ResizableCornerComponent.h"
#endif
#ifndef __JUCE_RESIZABLEEDGECOMPONENT_JUCEHEADER__
 #include "gui/components/layout/juce_ResizableEdgeComponent.h"
#endif
#ifndef __JUCE_SCROLLBAR_JUCEHEADER__
 #include "gui/components/layout/juce_ScrollBar.h"
#endif
#ifndef __JUCE_STRETCHABLELAYOUTMANAGER_JUCEHEADER__
 #include "gui/components/layout/juce_StretchableLayoutManager.h"
#endif
#ifndef __JUCE_STRETCHABLELAYOUTRESIZERBAR_JUCEHEADER__
 #include "gui/components/layout/juce_StretchableLayoutResizerBar.h"
#endif
#ifndef __JUCE_STRETCHABLEOBJECTRESIZER_JUCEHEADER__
 #include "gui/components/layout/juce_StretchableObjectResizer.h"
#endif
#ifndef __JUCE_TABBEDBUTTONBAR_JUCEHEADER__
 #include "gui/components/layout/juce_TabbedButtonBar.h"
#endif
#ifndef __JUCE_TABBEDCOMPONENT_JUCEHEADER__
 #include "gui/components/layout/juce_TabbedComponent.h"
#endif
#ifndef __JUCE_VIEWPORT_JUCEHEADER__
 #include "gui/components/layout/juce_Viewport.h"
#endif
#ifndef __JUCE_LOOKANDFEEL_JUCEHEADER__
 #include "gui/components/lookandfeel/juce_LookAndFeel.h"
#endif
#ifndef __JUCE_OLDSCHOOLLOOKANDFEEL_JUCEHEADER__
 #include "gui/components/lookandfeel/juce_OldSchoolLookAndFeel.h"
#endif
#ifndef __JUCE_MENUBARCOMPONENT_JUCEHEADER__
 #include "gui/components/menus/juce_MenuBarComponent.h"
#endif
#ifndef __JUCE_MENUBARMODEL_JUCEHEADER__
 #include "gui/components/menus/juce_MenuBarModel.h"
#endif
#ifndef __JUCE_POPUPMENU_JUCEHEADER__
 #include "gui/components/menus/juce_PopupMenu.h"
#endif
#ifndef __JUCE_COMPONENTDRAGGER_JUCEHEADER__
 #include "gui/components/mouse/juce_ComponentDragger.h"
#endif
#ifndef __JUCE_DRAGANDDROPCONTAINER_JUCEHEADER__
 #include "gui/components/mouse/juce_DragAndDropContainer.h"
#endif
#ifndef __JUCE_DRAGANDDROPTARGET_JUCEHEADER__
 #include "gui/components/mouse/juce_DragAndDropTarget.h"
#endif
#ifndef __JUCE_FILEDRAGANDDROPTARGET_JUCEHEADER__
 #include "gui/components/mouse/juce_FileDragAndDropTarget.h"
#endif
#ifndef __JUCE_LASSOCOMPONENT_JUCEHEADER__
 #include "gui/components/mouse/juce_LassoComponent.h"
#endif
#ifndef __JUCE_MOUSECURSOR_JUCEHEADER__
 #include "gui/components/mouse/juce_MouseCursor.h"
#endif
#ifndef __JUCE_MOUSEEVENT_JUCEHEADER__
 #include "gui/components/mouse/juce_MouseEvent.h"
#endif
#ifndef __JUCE_MOUSEINPUTSOURCE_JUCEHEADER__
 #include "gui/components/mouse/juce_MouseInputSource.h"
#endif
#ifndef __JUCE_MOUSELISTENER_JUCEHEADER__
 #include "gui/components/mouse/juce_MouseListener.h"
#endif
#ifndef __JUCE_TOOLTIPCLIENT_JUCEHEADER__
 #include "gui/components/mouse/juce_TooltipClient.h"
#endif
#ifndef __JUCE_MARKERLIST_JUCEHEADER__
 #include "gui/components/positioning/juce_MarkerList.h"
#endif
#ifndef __JUCE_RELATIVECOORDINATE_JUCEHEADER__
 #include "gui/components/positioning/juce_RelativeCoordinate.h"
#endif
#ifndef __JUCE_RELATIVECOORDINATEPOSITIONER_JUCEHEADER__
 #include "gui/components/positioning/juce_RelativeCoordinatePositioner.h"
#endif
#ifndef __JUCE_RELATIVEPARALLELOGRAM_JUCEHEADER__
 #include "gui/components/positioning/juce_RelativeParallelogram.h"
#endif
#ifndef __JUCE_RELATIVEPOINT_JUCEHEADER__
 #include "gui/components/positioning/juce_RelativePoint.h"
#endif
#ifndef __JUCE_RELATIVEPOINTPATH_JUCEHEADER__
 #include "gui/components/positioning/juce_RelativePointPath.h"
#endif
#ifndef __JUCE_RELATIVERECTANGLE_JUCEHEADER__
 #include "gui/components/positioning/juce_RelativeRectangle.h"
#endif
#ifndef __JUCE_BOOLEANPROPERTYCOMPONENT_JUCEHEADER__
 #include "gui/components/properties/juce_BooleanPropertyComponent.h"
#endif
#ifndef __JUCE_BUTTONPROPERTYCOMPONENT_JUCEHEADER__
 #include "gui/components/properties/juce_ButtonPropertyComponent.h"
#endif
#ifndef __JUCE_CHOICEPROPERTYCOMPONENT_JUCEHEADER__
 #include "gui/components/properties/juce_ChoicePropertyComponent.h"
#endif
#ifndef __JUCE_PROPERTYCOMPONENT_JUCEHEADER__
 #include "gui/components/properties/juce_PropertyComponent.h"
#endif
#ifndef __JUCE_PROPERTYPANEL_JUCEHEADER__
 #include "gui/components/properties/juce_PropertyPanel.h"
#endif
#ifndef __JUCE_SLIDERPROPERTYCOMPONENT_JUCEHEADER__
 #include "gui/components/properties/juce_SliderPropertyComponent.h"
#endif
#ifndef __JUCE_TEXTPROPERTYCOMPONENT_JUCEHEADER__
 #include "gui/components/properties/juce_TextPropertyComponent.h"
#endif
#ifndef __JUCE_ACTIVEXCONTROLCOMPONENT_JUCEHEADER__
 #include "gui/components/special/juce_ActiveXControlComponent.h"
#endif
#ifndef __JUCE_AUDIODEVICESELECTORCOMPONENT_JUCEHEADER__
 #include "gui/components/special/juce_AudioDeviceSelectorComponent.h"
#endif
#ifndef __JUCE_BUBBLECOMPONENT_JUCEHEADER__
 #include "gui/components/special/juce_BubbleComponent.h"
#endif
#ifndef __JUCE_BUBBLEMESSAGECOMPONENT_JUCEHEADER__
 #include "gui/components/special/juce_BubbleMessageComponent.h"
#endif
#ifndef __JUCE_COLOURSELECTOR_JUCEHEADER__
 #include "gui/components/special/juce_ColourSelector.h"
#endif
#ifndef __JUCE_DIRECTSHOWCOMPONENT_JUCEHEADER__
 #include "gui/components/special/juce_DirectShowComponent.h"
#endif
#ifndef __JUCE_DROPSHADOWER_JUCEHEADER__
 #include "gui/components/special/juce_DropShadower.h"
#endif
#ifndef __JUCE_MIDIKEYBOARDCOMPONENT_JUCEHEADER__
 #include "gui/components/special/juce_MidiKeyboardComponent.h"
#endif
#ifndef __JUCE_NSVIEWCOMPONENT_JUCEHEADER__
 #include "gui/components/special/juce_NSViewComponent.h"
#endif
#ifndef __JUCE_OPENGLCOMPONENT_JUCEHEADER__
 #include "gui/components/special/juce_OpenGLComponent.h"
#endif
#ifndef __JUCE_PREFERENCESPANEL_JUCEHEADER__
 #include "gui/components/special/juce_PreferencesPanel.h"
#endif
#ifndef __JUCE_QUICKTIMEMOVIECOMPONENT_JUCEHEADER__
 #include "gui/components/special/juce_QuickTimeMovieComponent.h"
#endif
#ifndef __JUCE_SYSTEMTRAYICONCOMPONENT_JUCEHEADER__
 #include "gui/components/special/juce_SystemTrayIconComponent.h"
#endif
#ifndef __JUCE_WEBBROWSERCOMPONENT_JUCEHEADER__
 #include "gui/components/special/juce_WebBrowserComponent.h"
#endif
#ifndef __JUCE_ALERTWINDOW_JUCEHEADER__
 #include "gui/components/windows/juce_AlertWindow.h"
#endif
#ifndef __JUCE_CALLOUTBOX_JUCEHEADER__
 #include "gui/components/windows/juce_CallOutBox.h"
#endif
#ifndef __JUCE_COMPONENTPEER_JUCEHEADER__
 #include "gui/components/windows/juce_ComponentPeer.h"
#endif
#ifndef __JUCE_DIALOGWINDOW_JUCEHEADER__
 #include "gui/components/windows/juce_DialogWindow.h"
#endif
#ifndef __JUCE_DOCUMENTWINDOW_JUCEHEADER__
 #include "gui/components/windows/juce_DocumentWindow.h"
#endif
#ifndef __JUCE_NATIVEMESSAGEBOX_JUCEHEADER__
 #include "gui/components/windows/juce_NativeMessageBox.h"
#endif
#ifndef __JUCE_RESIZABLEWINDOW_JUCEHEADER__
 #include "gui/components/windows/juce_ResizableWindow.h"
#endif
#ifndef __JUCE_SPLASHSCREEN_JUCEHEADER__
 #include "gui/components/windows/juce_SplashScreen.h"
#endif
#ifndef __JUCE_THREADWITHPROGRESSWINDOW_JUCEHEADER__
 #include "gui/components/windows/juce_ThreadWithProgressWindow.h"
#endif
#ifndef __JUCE_TOOLTIPWINDOW_JUCEHEADER__
 #include "gui/components/windows/juce_TooltipWindow.h"
#endif
#ifndef __JUCE_TOPLEVELWINDOW_JUCEHEADER__
 #include "gui/components/windows/juce_TopLevelWindow.h"
#endif
#ifndef __JUCE_COLOUR_JUCEHEADER__
 #include "gui/graphics/colour/juce_Colour.h"
#endif
#ifndef __JUCE_COLOURGRADIENT_JUCEHEADER__
 #include "gui/graphics/colour/juce_ColourGradient.h"
#endif
#ifndef __JUCE_COLOURS_JUCEHEADER__
 #include "gui/graphics/colour/juce_Colours.h"
#endif
#ifndef __JUCE_PIXELFORMATS_JUCEHEADER__
 #include "gui/graphics/colour/juce_PixelFormats.h"
#endif
#ifndef __JUCE_EDGETABLE_JUCEHEADER__
 #include "gui/graphics/contexts/juce_EdgeTable.h"
#endif
#ifndef __JUCE_FILLTYPE_JUCEHEADER__
 #include "gui/graphics/contexts/juce_FillType.h"
#endif
#ifndef __JUCE_GRAPHICS_JUCEHEADER__
 #include "gui/graphics/contexts/juce_Graphics.h"
#endif
#ifndef __JUCE_JUSTIFICATION_JUCEHEADER__
 #include "gui/graphics/contexts/juce_Justification.h"
#endif
#ifndef __JUCE_LOWLEVELGRAPHICSCONTEXT_JUCEHEADER__
 #include "gui/graphics/contexts/juce_LowLevelGraphicsContext.h"
#endif
#ifndef __JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_JUCEHEADER__
 #include "gui/graphics/contexts/juce_LowLevelGraphicsPostScriptRenderer.h"
#endif
#ifndef __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__
 #include "gui/graphics/contexts/juce_LowLevelGraphicsSoftwareRenderer.h"
#endif
#ifndef __JUCE_RECTANGLEPLACEMENT_JUCEHEADER__
 #include "gui/graphics/contexts/juce_RectanglePlacement.h"
#endif
#ifndef __JUCE_DRAWABLE_JUCEHEADER__
 #include "gui/graphics/drawables/juce_Drawable.h"
#endif
#ifndef __JUCE_DRAWABLECOMPOSITE_JUCEHEADER__
 #include "gui/graphics/drawables/juce_DrawableComposite.h"
#endif
#ifndef __JUCE_DRAWABLEIMAGE_JUCEHEADER__
 #include "gui/graphics/drawables/juce_DrawableImage.h"
#endif
#ifndef __JUCE_DRAWABLEPATH_JUCEHEADER__
 #include "gui/graphics/drawables/juce_DrawablePath.h"
#endif
#ifndef __JUCE_DRAWABLERECTANGLE_JUCEHEADER__
 #include "gui/graphics/drawables/juce_DrawableRectangle.h"
#endif
#ifndef __JUCE_DRAWABLESHAPE_JUCEHEADER__
 #include "gui/graphics/drawables/juce_DrawableShape.h"
#endif
#ifndef __JUCE_DRAWABLETEXT_JUCEHEADER__
 #include "gui/graphics/drawables/juce_DrawableText.h"
#endif
#ifndef __JUCE_DROPSHADOWEFFECT_JUCEHEADER__
 #include "gui/graphics/effects/juce_DropShadowEffect.h"
#endif
#ifndef __JUCE_GLOWEFFECT_JUCEHEADER__
 #include "gui/graphics/effects/juce_GlowEffect.h"
#endif
#ifndef __JUCE_IMAGEEFFECTFILTER_JUCEHEADER__
 #include "gui/graphics/effects/juce_ImageEffectFilter.h"
#endif
#ifndef __JUCE_CUSTOMTYPEFACE_JUCEHEADER__
 #include "gui/graphics/fonts/juce_CustomTypeface.h"
#endif
#ifndef __JUCE_FONT_JUCEHEADER__
 #include "gui/graphics/fonts/juce_Font.h"
#endif
#ifndef __JUCE_GLYPHARRANGEMENT_JUCEHEADER__
 #include "gui/graphics/fonts/juce_GlyphArrangement.h"
#endif
#ifndef __JUCE_TEXTLAYOUT_JUCEHEADER__
 #include "gui/graphics/fonts/juce_TextLayout.h"
#endif
#ifndef __JUCE_TYPEFACE_JUCEHEADER__
 #include "gui/graphics/fonts/juce_Typeface.h"
#endif
#ifndef __JUCE_AFFINETRANSFORM_JUCEHEADER__
 #include "gui/graphics/geometry/juce_AffineTransform.h"
#endif
#ifndef __JUCE_BORDERSIZE_JUCEHEADER__
 #include "gui/graphics/geometry/juce_BorderSize.h"
#endif
#ifndef __JUCE_LINE_JUCEHEADER__
 #include "gui/graphics/geometry/juce_Line.h"
#endif
#ifndef __JUCE_PATH_JUCEHEADER__
 #include "gui/graphics/geometry/juce_Path.h"
#endif
#ifndef __JUCE_PATHITERATOR_JUCEHEADER__
 #include "gui/graphics/geometry/juce_PathIterator.h"
#endif
#ifndef __JUCE_PATHSTROKETYPE_JUCEHEADER__
 #include "gui/graphics/geometry/juce_PathStrokeType.h"
#endif
#ifndef __JUCE_POINT_JUCEHEADER__
 #include "gui/graphics/geometry/juce_Point.h"
#endif
#ifndef __JUCE_RECTANGLE_JUCEHEADER__
 #include "gui/graphics/geometry/juce_Rectangle.h"
#endif
#ifndef __JUCE_RECTANGLELIST_JUCEHEADER__
 #include "gui/graphics/geometry/juce_RectangleList.h"
#endif
#ifndef __JUCE_CAMERADEVICE_JUCEHEADER__
 #include "gui/graphics/imaging/juce_CameraDevice.h"
#endif
#ifndef __JUCE_IMAGE_JUCEHEADER__
 #include "gui/graphics/imaging/juce_Image.h"
#endif
#ifndef __JUCE_IMAGECACHE_JUCEHEADER__
 #include "gui/graphics/imaging/juce_ImageCache.h"
#endif
#ifndef __JUCE_IMAGECONVOLUTIONKERNEL_JUCEHEADER__
 #include "gui/graphics/imaging/juce_ImageConvolutionKernel.h"
#endif
#ifndef __JUCE_IMAGEFILEFORMAT_JUCEHEADER__
 #include "gui/graphics/imaging/juce_ImageFileFormat.h"
#endif
#ifndef __JUCE_DELETEDATSHUTDOWN_JUCEHEADER__
 #include "utilities/juce_DeletedAtShutdown.h"
#endif
#ifndef __JUCE_FILEBASEDDOCUMENT_JUCEHEADER__
 #include "utilities/juce_FileBasedDocument.h"
#endif
#ifndef __JUCE_PROPERTIESFILE_JUCEHEADER__
 #include "utilities/juce_PropertiesFile.h"
#endif
#ifndef __JUCE_RECENTLYOPENEDFILESLIST_JUCEHEADER__
 #include "utilities/juce_RecentlyOpenedFilesList.h"
#endif
#ifndef __JUCE_SELECTEDITEMSET_JUCEHEADER__
 #include "utilities/juce_SelectedItemSet.h"
#endif
#ifndef __JUCE_SYSTEMCLIPBOARD_JUCEHEADER__
 #include "utilities/juce_SystemClipboard.h"
#endif
#ifndef __JUCE_UNDOABLEACTION_JUCEHEADER__
 #include "utilities/juce_UndoableAction.h"
#endif
#ifndef __JUCE_UNDOMANAGER_JUCEHEADER__
 #include "utilities/juce_UndoManager.h"
#endif
#ifndef __JUCE_UNITTEST_JUCEHEADER__
 #include "utilities/juce_UnitTest.h"
#endif

#endif
