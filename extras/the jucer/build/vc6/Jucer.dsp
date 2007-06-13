# Microsoft Developer Studio Project File - Name="Jucer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Jucer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Jucer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Jucer.mak" CFG="Jucer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Jucer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Jucer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Jucer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "Jucer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Jucer - Win32 Release"
# Name "Jucer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "model"

# PROP Default_Filter ""
# Begin Group "components"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\model\components\jucer_ButtonHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_ComboBoxHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_ComponentNameProperty.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_ComponentTypeHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_ComponentTypeHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_ComponentUndoableAction.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_GenericComponentHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_GroupComponentHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_HyperlinkButtonHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_JucerComponentHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_LabelHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_SliderHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_TabbedComponentHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_TextButtonHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_TextEditorHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_ToggleButtonHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_TreeViewHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\components\jucer_ViewportHandler.h
# End Source File
# End Group
# Begin Group "paintelements"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_ColouredElement.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_ColouredElement.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_ElementSiblingComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_FillType.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_FillType.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_GradientPointComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_ImageResourceProperty.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_PaintElement.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_PaintElement.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_PaintElementEllipse.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_PaintElementImage.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_PaintElementPath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_PaintElementPath.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_PaintElementRectangle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_PaintElementRoundedRectangle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_PaintElementText.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_PaintElementUndoableAction.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_PointComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_StrokeType.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\model\paintelements\jucer_StrokeType.h
# End Source File
# End Group
# Begin Group "documents"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\model\documents\jucer_ButtonDocument.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\model\documents\jucer_ButtonDocument.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\documents\jucer_ComponentDocument.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\model\documents\jucer_ComponentDocument.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\src\model\jucer_BinaryResources.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\model\jucer_BinaryResources.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\jucer_ComponentLayout.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\model\jucer_ComponentLayout.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\jucer_GeneratedCode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\model\jucer_GeneratedCode.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\jucer_JucerDocument.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\model\jucer_JucerDocument.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\jucer_ObjectTypes.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\model\jucer_ObjectTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\src\model\jucer_PaintRoutine.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\model\jucer_PaintRoutine.h
# End Source File
# End Group
# Begin Group "properties"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\properties\jucer_ColourPropertyComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\properties\jucer_ComponentChoiceProperty.h
# End Source File
# Begin Source File

SOURCE=..\..\src\properties\jucer_ComponentColourProperty.h
# End Source File
# Begin Source File

SOURCE=..\..\src\properties\jucer_ComponentTextProperty.h
# End Source File
# Begin Source File

SOURCE=..\..\src\properties\jucer_FilePropertyComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\properties\jucer_FilePropertyComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\properties\jucer_FontPropertyComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\properties\jucer_FontPropertyComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\properties\jucer_JustificationProperty.h
# End Source File
# Begin Source File

SOURCE=..\..\src\properties\jucer_PositionPropertyBase.h
# End Source File
# End Group
# Begin Group "templates"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\templates\jucer_ComponentTemplate.h
# End Source File
# End Group
# Begin Group "ui"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\ui\jucer_CommandIDs.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_ComponentLayoutEditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_ComponentLayoutEditor.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_ComponentLayoutPanel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_ComponentLayoutPanel.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_ComponentOverlayComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_ComponentOverlayComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_EditingPanelBase.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_EditingPanelBase.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_JucerDocumentHolder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_JucerDocumentHolder.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_MainWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_MainWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_PaintRoutineEditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_PaintRoutineEditor.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_PaintRoutinePanel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_PaintRoutinePanel.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_PrefsPanel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_PrefsPanel.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_ResourceEditorPanel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_ResourceEditorPanel.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_SnapGridPainter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_SnapGridPainter.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_TestComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ui\jucer_TestComponent.h
# End Source File
# End Group
# Begin Group "utility"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\utility\jucer_ColourEditorComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\utility\jucer_Colours.h
# End Source File
# Begin Source File

SOURCE=..\..\src\utility\jucer_StoredSettings.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\utility\jucer_StoredSettings.h
# End Source File
# Begin Source File

SOURCE=..\..\src\utility\jucer_UtilityFunctions.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\utility\jucer_UtilityFunctions.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\src\BinaryData.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\BinaryData.h
# End Source File
# Begin Source File

SOURCE=..\..\src\jucer_Headers.h
# End Source File
# Begin Source File

SOURCE=..\..\src\jucer_Main.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\jucer.rc
# End Source File
# Begin Source File

SOURCE=.\jucer_icon.ico
# End Source File
# End Group
# Begin Source File

SOURCE="..\..\Jucer To Do list.txt"
# End Source File
# End Target
# End Project
