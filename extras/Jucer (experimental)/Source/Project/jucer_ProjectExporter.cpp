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

#include "jucer_ProjectExporter.h"
#include "jucer_ProjectExport_Make.h"
#include "jucer_ProjectExport_MSVC.h"
#include "jucer_ProjectExport_XCode.h"


//==============================================================================
ProjectExporter::ProjectExporter (Project& project_, const ValueTree& settings_)
    : project (project_), settings (settings_)
{
}

ProjectExporter::~ProjectExporter()
{
}

//==============================================================================
int ProjectExporter::getNumExporters()
{
    return 6;
}

const StringArray ProjectExporter::getExporterNames()
{
    StringArray s;
    s.add (XCodeProjectExporter::getNameMac());
    s.add (XCodeProjectExporter::getNameiOS());
    s.add (MSVCProjectExporterVC6::getName());
    s.add (MSVCProjectExporterVC2005::getName());
    s.add (MSVCProjectExporterVC2008::getName());
    s.add (MSVCProjectExporterVC2010::getName());
    s.add (MakefileProjectExporter::getNameLinux());
    return s;
}

ProjectExporter* ProjectExporter::createNewExporter (Project& project, const int index)
{
    ProjectExporter* exp = 0;

    switch (index)
    {
        case 0:     exp = new XCodeProjectExporter (project, ValueTree (XCodeProjectExporter::getValueTreeTypeName (false)), false); break;
        case 1:     exp = new XCodeProjectExporter (project, ValueTree (XCodeProjectExporter::getValueTreeTypeName (true)), true); break;
        case 2:     exp = new MSVCProjectExporterVC6 (project, ValueTree (MSVCProjectExporterVC6::getValueTreeTypeName())); break;
        case 3:     exp = new MSVCProjectExporterVC2005 (project, ValueTree (MSVCProjectExporterVC2005::getValueTreeTypeName())); break;
        case 4:     exp = new MSVCProjectExporterVC2008 (project, ValueTree (MSVCProjectExporterVC2008::getValueTreeTypeName())); break;
        case 5:     exp = new MSVCProjectExporterVC2010 (project, ValueTree (MSVCProjectExporterVC2010::getValueTreeTypeName())); break;
        case 6:     exp = new MakefileProjectExporter (project, ValueTree (MakefileProjectExporter::getValueTreeTypeName())); break;
        default:    jassertfalse; return 0;
    }

    File juceFolder (StoredSettings::getInstance()->getLastKnownJuceFolder());
    File target (exp->getTargetFolder());

    if (FileHelpers::shouldPathsBeRelative (juceFolder.getFullPathName(), project.getFile().getFullPathName()))
        exp->getJuceFolder() = juceFolder.getRelativePathFrom (project.getFile().getParentDirectory());
    else
        exp->getJuceFolder() = juceFolder.getFullPathName();

    return exp;
}

ProjectExporter* ProjectExporter::createExporter (Project& project, const ValueTree& settings)
{
    ProjectExporter* exp = MSVCProjectExporterVC6::createForSettings (project, settings);
    if (exp == 0)    exp = MSVCProjectExporterVC2005::createForSettings (project, settings);
    if (exp == 0)    exp = MSVCProjectExporterVC2008::createForSettings (project, settings);
    if (exp == 0)    exp = MSVCProjectExporterVC2010::createForSettings (project, settings);
    if (exp == 0)    exp = XCodeProjectExporter::createForSettings (project, settings);
    if (exp == 0)    exp = MakefileProjectExporter::createForSettings (project, settings);

    jassert (exp != 0);
    return exp;
}

ProjectExporter* ProjectExporter::createPlatformDefaultExporter (Project& project)
{
    for (int i = 0; i < project.getNumExporters(); ++i)
    {
        ScopedPointer <ProjectExporter> exp (project.createExporter (i));

        if (exp->isDefaultFormatForCurrentOS())
            return exp.release();
    }

    return 0;
}

const File ProjectExporter::getTargetFolder() const
{
    return project.resolveFilename (getTargetLocation().toString());
}

const String ProjectExporter::getIncludePathForFileInJuceFolder (const String& pathFromJuceFolder, const File& targetIncludeFile) const
{
    String juceFolderPath (getJuceFolder().toString());

    if (juceFolderPath.startsWithChar ('<'))
    {
        juceFolderPath = FileHelpers::unixStylePath (File::addTrailingSeparator (juceFolderPath.substring (1).dropLastCharacters(1)));
        if (juceFolderPath == "/")
            juceFolderPath = String::empty;

        return "<" + juceFolderPath + pathFromJuceFolder + ">";
    }
    else
    {
        const RelativePath juceFromProject (juceFolderPath, RelativePath::projectFolder);
        const RelativePath fileFromProject (juceFromProject.getChildFile (pathFromJuceFolder));
        const RelativePath fileFromHere (fileFromProject.rebased (project.getFile().getParentDirectory(),
                                                                  targetIncludeFile.getParentDirectory(), RelativePath::unknown));
        return fileFromHere.toUnixStyle().quoted();
    }
}

const RelativePath ProjectExporter::getJucePathFromTargetFolder() const
{
    return rebaseFromProjectFolderToBuildTarget (RelativePath (getJuceFolder().toString(), RelativePath::projectFolder));
}

const RelativePath ProjectExporter::rebaseFromProjectFolderToBuildTarget (const RelativePath& path) const
{
    return path.rebased (project.getFile().getParentDirectory(), getTargetFolder(), RelativePath::buildTargetFolder);
}

bool ProjectExporter::shouldFileBeCompiledByDefault (const RelativePath& file) const
{
    return file.hasFileExtension ("cpp;cc;c;cxx");
}

void ProjectExporter::createPropertyEditors (Array <PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (getTargetLocation(), "Target Project Folder", 1024, false));
    props.getLast()->setTooltip ("The location of the folder in which the " + name + " project will be created. This path can be absolute, but it's much more sensible to make it relative to the jucer project directory.");

    props.add (new TextPropertyComponent (getJuceFolder(), "Juce Location", 1024, false));
    props.getLast()->setTooltip ("The location of the Juce library folder that the " + name + " project will use to when compiling. This can be an absolute path, or relative to the jucer project folder, but it must be valid on the filesystem of the machine you use to actually do the compiling.");

    if (project.isAudioPlugin())
    {
        if (project.shouldAddVSTFolderToPath())
        {
            props.add (new TextPropertyComponent (getVSTFolder(), "VST Folder", 1024, false));
            props.getLast()->setTooltip ("If you're building a VST, this must be the folder containing the VST SDK. This should be an absolute path.");
        }

        if (isRTAS())
        {
            props.add (new TextPropertyComponent (getRTASFolder(), "RTAS Folder", 1024, false));
            props.getLast()->setTooltip ("If you're building an RTAS, this must be the folder containing the RTAS SDK. This should be an absolute path.");
        }
    }

    props.add (new TextPropertyComponent (getExporterPreprocessorDefs(), "Extra Preprocessor Definitions", 32768, false));
    props.getLast()->setTooltip ("Extra preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace or commas to separate the items - to include a space or comma in a definition, precede it with a backslash.");

    props.add (new TextPropertyComponent (getExtraCompilerFlags(), "Extra compiler flags", 2048, false));
    props.getLast()->setTooltip ("Extra command-line flags to be passed to the compiler. This string can contain references to preprocessor definitions in the form ${NAME_OF_DEFINITION}, which will be replaced with their values.");
    props.add (new TextPropertyComponent (getExtraLinkerFlags(), "Extra linker flags", 2048, false));
    props.getLast()->setTooltip ("Extra command-line flags to be passed to the linker. You might want to use this for adding additional libraries. This string can contain references to preprocessor definitions in the form ${NAME_OF_VALUE}, which will be replaced with their values.");
}

const Array<RelativePath> ProjectExporter::getVSTFilesRequired() const
{
    Array<RelativePath> s;
    if (isVST())
    {
        const char* files[] = { "extras/audio plugins/wrapper/VST/juce_VST_Wrapper.cpp",
                                "extras/audio plugins/wrapper/VST/juce_VST_Wrapper.mm" };

        for (int i = 0; i < numElementsInArray (files); ++i)
            s.add (getJucePathFromTargetFolder().getChildFile (files[i]));
    }

    return s;
}

const StringPairArray ProjectExporter::getAllPreprocessorDefs (const Project::BuildConfiguration& config) const
{
    StringPairArray defs (mergePreprocessorDefs (config.getAllPreprocessorDefs(),
                                                 parsePreprocessorDefs (getExporterPreprocessorDefs().toString())));
    defs.set (getExporterIdentifierMacro(), "1");
    return defs;
}

const String ProjectExporter::replacePreprocessorTokens (const Project::BuildConfiguration& config, const String& sourceString) const
{
    return replacePreprocessorDefs (getAllPreprocessorDefs (config), sourceString);
}
