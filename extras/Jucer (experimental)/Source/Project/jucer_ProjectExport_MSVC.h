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

#ifndef __JUCER_PROJECTEXPORT_MSVC_JUCEHEADER__
#define __JUCER_PROJECTEXPORT_MSVC_JUCEHEADER__

#include "jucer_ProjectExporter.h"


//==============================================================================
class MSVCProjectExporterBase   : public ProjectExporter
{
public:
    //==============================================================================
    MSVCProjectExporterBase (Project& project_, const ValueTree& settings_, const char* const folderName)
        : ProjectExporter (project_, settings_), hasIcon (false)
    {
        if (getTargetLocation().toString().isEmpty())
            getTargetLocation() = getDefaultBuildsRootFolder() + folderName;

        if (getVSTFolder().toString().isEmpty())
            getVSTFolder() = "c:\\SDKs\\vstsdk2.4";

        if (getRTASFolder().toString().isEmpty())
            getRTASFolder() = "c:\\SDKs\\PT_80_SDK";

        if ((int) getLibraryType().getValue() <= 0)
            getLibraryType() = 1;

        projectGUID = createGUID (project.getProjectUID());
    }

    ~MSVCProjectExporterBase()  {}

    //==============================================================================
    bool isPossibleForCurrentProject()          { return true; }
    bool usesMMFiles() const                    { return false; }

    void createPropertyEditors (Array <PropertyComponent*>& props)
    {
        ProjectExporter::createPropertyEditors (props);

        if (project.isLibrary())
        {
            const char* const libTypes[] = { "Static Library (.lib)", "Dynamic Library (.dll)", 0 };
            const int libTypeValues[] = { 1, 2, 0 };
            props.add (new ChoicePropertyComponent (getLibraryType(), "Library Type", StringArray (libTypes), Array<var> (libTypeValues)));

            props.add (new TextPropertyComponent (getSetting (Ids::libraryName_Debug), "Library Name (Debug)", 128, false));
            props.getLast()->setTooltip ("If set, this name will override the binary name specified in the configuration settings, for a debug build. You must include the .lib or .dll suffix on this filename.");

            props.add (new TextPropertyComponent (getSetting (Ids::libraryName_Release), "Library Name (Release)", 128, false));
            props.getLast()->setTooltip ("If set, this name will override the binary name specified in the configuration settings, for a release build. You must include the .lib or .dll suffix on this filename.");
        }
    }

protected:
    String projectGUID;
    File rcFile, iconFile;
    bool hasIcon;

    const File getProjectFile (const String& extension) const   { return getTargetFolder().getChildFile (project.getProjectFilenameRoot()).withFileExtension (extension); }

    Value getLibraryType() const        { return getSetting (Ids::libraryType); }
    bool isLibraryDLL() const           { return project.isLibrary() && getLibraryType() == 2; }

    //==============================================================================
    const Array<RelativePath> getRTASFilesRequired() const
    {
        Array<RelativePath> s;

        if (isRTAS())
        {
            static const char* files[] = { "extras/audio plugins/wrapper/RTAS/juce_RTAS_DigiCode1.cpp",
                                           "extras/audio plugins/wrapper/RTAS/juce_RTAS_DigiCode2.cpp",
                                           "extras/audio plugins/wrapper/RTAS/juce_RTAS_DigiCode3.cpp",
                                           "extras/audio plugins/wrapper/RTAS/juce_RTAS_DigiCode_Header.h",
                                           "extras/audio plugins/wrapper/RTAS/juce_RTAS_WinUtilities.cpp",
                                           "extras/audio plugins/wrapper/RTAS/juce_RTAS_Wrapper.cpp" };

            for (int i = 0; i < numElementsInArray (files); ++i)
                s.add (getJucePathFromTargetFolder().getChildFile (files[i]));
        }

        return s;
    }

    const String getIntermediatesPath (const Project::BuildConfiguration& config) const
    {
        return ".\\" + File::createLegalFileName (config.getName().toString().trim());
    }

    const String getConfigTargetPath (const Project::BuildConfiguration& config) const
    {
        const String binaryPath (config.getTargetBinaryRelativePath().toString().trim());
        if (binaryPath.isEmpty())
            return getIntermediatesPath (config);

        return ".\\" + RelativePath (binaryPath, RelativePath::projectFolder)
                             .rebased (project.getFile().getParentDirectory(), getTargetFolder(), RelativePath::buildTargetFolder)
                             .toWindowsStyle();
    }

    const String getTargetBinarySuffix() const
    {
        if (project.isLibrary())
            return ".lib";
        else if (isRTAS())
            return ".dpm";
        else if (project.isAudioPlugin() || project.isBrowserPlugin())
            return ".dll";

        return ".exe";
    }

    const String getPreprocessorDefs (const Project::BuildConfiguration& config, const String& joinString) const
    {
        StringPairArray defines;
        defines.set ("WIN32", "");
        defines.set ("_WINDOWS", "");

        if (config.isDebug().getValue())
        {
            defines.set ("DEBUG", "");
            defines.set ("_DEBUG", "");
        }
        else
        {
            defines.set ("NDEBUG", "");
        }

        if (project.isCommandLineApp())
            defines.set ("_CONSOLE", "");

        if (project.isLibrary())
            defines.set ("_LIB", "");

        if (isRTAS())
        {
            RelativePath rtasFolder (getRTASFolder().toString(), RelativePath::unknown);
            defines.set ("JucePlugin_WinBag_path", CodeHelpers::addEscapeChars (rtasFolder.getChildFile ("WinBag")
                                                                                .toWindowsStyle().quoted()));
        }

        defines = mergePreprocessorDefs (defines, getAllPreprocessorDefs (config));

        StringArray result;

        for (int i = 0; i < defines.size(); ++i)
        {
            String def (defines.getAllKeys()[i]);
            const String value (defines.getAllValues()[i]);
            if (value.isNotEmpty())
                def << "=" << value;

            result.add (def);
        }

        return result.joinIntoString (joinString);
    }

    const StringArray getHeaderSearchPaths (const Project::BuildConfiguration& config) const
    {
        StringArray searchPaths (config.getHeaderSearchPaths());

        if (project.shouldAddVSTFolderToPath() && getVSTFolder().toString().isNotEmpty())
            searchPaths.add (rebaseFromProjectFolderToBuildTarget (RelativePath (getVSTFolder().toString(), RelativePath::projectFolder)).toWindowsStyle());

        if (project.isAudioPlugin())
            searchPaths.add (juceWrapperFolder.toWindowsStyle());

        if (isRTAS())
        {
            static const char* rtasIncludePaths[] = { "AlturaPorts/TDMPlugins/PluginLibrary/EffectClasses",
                                                      "AlturaPorts/TDMPlugins/PluginLibrary/ProcessClasses",
                                                      "AlturaPorts/TDMPlugins/PluginLibrary/ProcessClasses/Interfaces",
                                                      "AlturaPorts/TDMPlugins/PluginLibrary/Utilities",
                                                      "AlturaPorts/TDMPlugins/PluginLibrary/RTASP_Adapt",
                                                      "AlturaPorts/TDMPlugins/PluginLibrary/CoreClasses",
                                                      "AlturaPorts/TDMPlugins/PluginLibrary/Controls",
                                                      "AlturaPorts/TDMPlugins/PluginLibrary/Meters",
                                                      "AlturaPorts/TDMPlugins/PluginLibrary/ViewClasses",
                                                      "AlturaPorts/TDMPlugins/PluginLibrary/DSPClasses",
                                                      "AlturaPorts/TDMPlugins/PluginLibrary/Interfaces",
                                                      "AlturaPorts/TDMPlugins/common",
                                                      "AlturaPorts/TDMPlugins/common/Platform",
                                                      "AlturaPorts/TDMPlugins/SignalProcessing/Public",
                                                      "AlturaPorts/TDMPlugIns/DSPManager/Interfaces",
                                                      "AlturaPorts/SADriver/Interfaces",
                                                      "AlturaPorts/DigiPublic/Interfaces",
                                                      "AlturaPorts/Fic/Interfaces/DAEClient",
                                                      "AlturaPorts/NewFileLibs/Cmn",
                                                      "AlturaPorts/NewFileLibs/DOA",
                                                      "AlturaPorts/AlturaSource/PPC_H",
                                                      "AlturaPorts/AlturaSource/AppSupport",
                                                      "AvidCode/AVX2sdk/AVX/avx2/avx2sdk/inc",
                                                      "xplat/AVX/avx2/avx2sdk/inc" };

            RelativePath sdkFolder (getRTASFolder().toString(), RelativePath::projectFolder);

            for (int i = 0; i < numElementsInArray (rtasIncludePaths); ++i)
                searchPaths.add (rebaseFromProjectFolderToBuildTarget (sdkFolder.getChildFile (rtasIncludePaths[i])).toWindowsStyle());
        }

        return searchPaths;
    }

    const String getBinaryFileForConfig (const Project::BuildConfiguration& config) const
    {
        const String targetBinary (getSetting (config.isDebug().getValue() ? Ids::libraryName_Debug : Ids::libraryName_Release).toString().trim());
        if (targetBinary.isNotEmpty())
            return targetBinary;

        return config.getTargetBinaryName().toString() + getTargetBinarySuffix();
    }

    static const String createConfigName (const Project::BuildConfiguration& config)
    {
        return config.getName().toString() + "|Win32";
    }

    //==============================================================================
    void writeSolutionFile (OutputStream& out, const String& versionString, const File& vcProject)
    {
        out << "Microsoft Visual Studio Solution File, Format Version " << versionString << newLine
            << "Project(\"" << createGUID (project.getProjectName().toString() + "sln_guid") << "\") = \"" << project.getProjectName().toString() << "\", \""
            << vcProject.getFileName() << "\", \"" << projectGUID << '"' << newLine
            << "EndProject" << newLine
            << "Global" << newLine
            << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution" << newLine;

        int i;
        for (i = 0; i < project.getNumConfigurations(); ++i)
        {
            Project::BuildConfiguration config (project.getConfiguration (i));
            out << "\t\t" << createConfigName (config) << " = " << createConfigName (config) << newLine;
        }

        out << "\tEndGlobalSection" << newLine
            << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution" << newLine;

        for (i = 0; i < project.getNumConfigurations(); ++i)
        {
            Project::BuildConfiguration config (project.getConfiguration (i));
            out << "\t\t" << projectGUID << "." << createConfigName (config) << ".ActiveCfg = " << createConfigName (config) << newLine;
            out << "\t\t" << projectGUID << "." << createConfigName (config) << ".Build.0 = " << createConfigName (config) << newLine;
        }

        out << "\tEndGlobalSection" << newLine
            << "\tGlobalSection(SolutionProperties) = preSolution" << newLine
            << "\t\tHideSolutionNode = FALSE" << newLine
            << "\tEndGlobalSection" << newLine
            << "EndGlobal" << newLine;
    }

    //==============================================================================
    static bool writeRCFile (const File& file, const File& iconFile)
    {
        return file.deleteFile()
                && file.appendText ("IDI_ICON1 ICON DISCARDABLE "
                                        + iconFile.getFileName().quoted(), false, false);
    }

    static void writeIconFile (const Array<Image>& images, OutputStream& out)
    {
        out.writeShort (0); // reserved
        out.writeShort (1); // .ico tag
        out.writeShort ((short) images.size());

        MemoryOutputStream dataBlock;

        const int imageDirEntrySize = 16;
        const int dataBlockStart = 6 + images.size() * imageDirEntrySize;

        for (int i = 0; i < images.size(); ++i)
        {
            const Image& image = images.getReference (i);
            const int w = image.getWidth();
            const int h = image.getHeight();
            const int maskStride = (w / 8 + 3) & ~3;

            const size_t oldDataSize = dataBlock.getDataSize();
            dataBlock.writeInt (40); // bitmapinfoheader size
            dataBlock.writeInt (w);
            dataBlock.writeInt (h * 2);
            dataBlock.writeShort (1); // planes
            dataBlock.writeShort (32); // bits
            dataBlock.writeInt (0); // compression
            dataBlock.writeInt ((h * w * 4) + (h * maskStride)); // size image
            dataBlock.writeInt (0); // x pixels per meter
            dataBlock.writeInt (0); // y pixels per meter
            dataBlock.writeInt (0); // clr used
            dataBlock.writeInt (0); // clr important

            const Image::BitmapData bitmap (image, false);
            const int alphaThreshold = 5;

            int y;
            for (y = h; --y >= 0;)
            {
                for (int x = 0; x < w; ++x)
                {
                    const Colour pixel (bitmap.getPixelColour (x, y));

                    if (pixel.getAlpha() <= alphaThreshold)
                    {
                        dataBlock.writeInt (0);
                    }
                    else
                    {
                        dataBlock.writeByte ((char) pixel.getBlue());
                        dataBlock.writeByte ((char) pixel.getGreen());
                        dataBlock.writeByte ((char) pixel.getRed());
                        dataBlock.writeByte ((char) pixel.getAlpha());
                    }
                }
            }

            for (y = h; --y >= 0;)
            {
                int mask = 0, count = 0;

                for (int x = 0; x < w; ++x)
                {
                    const Colour pixel (bitmap.getPixelColour (x, y));

                    mask <<= 1;
                    if (pixel.getAlpha() <= alphaThreshold)
                        mask |= 1;

                    if (++count == 8)
                    {
                        dataBlock.writeByte ((char) mask);
                        count = 0;
                        mask = 0;
                    }
                }

                if (mask != 0)
                    dataBlock.writeByte ((char) mask);

                for (int i = maskStride - w / 8; --i >= 0;)
                    dataBlock.writeByte (0);
            }

            out.writeByte ((char) w);
            out.writeByte ((char) h);
            out.writeByte (0);
            out.writeByte (0);
            out.writeShort (1); // colour planes
            out.writeShort (32); // bits per pixel
            out.writeInt ((int) (dataBlock.getDataSize() - oldDataSize));
            out.writeInt (dataBlockStart + oldDataSize);
        }

        jassert (out.getPosition() == dataBlockStart);
        out << dataBlock;
    }

    static const Image getBestIconImage (const Image& im1, const Image& im2, int size)
    {
        Image im;

        if (im1.isValid() && im2.isValid())
        {
            if (im1.getWidth() >= size && im2.getWidth() >= size)
                im = im1.getWidth() < im2.getWidth() ? im1 : im2;
            else if (im1.getWidth() >= size)
                im = im1;
            else if (im2.getWidth() >= size)
                im = im2;
            else
                return Image();
        }
        else
        {
            im = im1.isValid() ? im1 : im2;
        }

        if (size == im.getWidth() && size == im.getHeight())
            return im;

        if (im.getWidth() < size && im.getHeight() < size)
            return Image();

        Image newIm (Image::ARGB, size, size, true);
        Graphics g (newIm);
        g.drawImageWithin (im, 0, 0, size, size,
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, false);
        return newIm;
    }

    bool createIconFile()
    {
        Array<Image> images;

        const Image smallIcon (project.getSmallIcon());
        const Image bigIcon (project.getBigIcon());

        Image im (getBestIconImage (smallIcon, bigIcon, 16));
        if (im.isValid())
            images.add (im);

        im = getBestIconImage (smallIcon, bigIcon, 32);
        if (im.isValid())
            images.add (im);

        im = getBestIconImage (smallIcon, bigIcon, 48);
        if (im.isValid())
            images.add (im);

        im = getBestIconImage (smallIcon, bigIcon, 128);
        if (im.isValid())
            images.add (im);

        if (images.size() == 0)
            return true;

        MemoryOutputStream mo;
        writeIconFile (images, mo);

        iconFile = getTargetFolder().getChildFile ("icon.ico");
        rcFile = getTargetFolder().getChildFile ("resources.rc");

        hasIcon = FileHelpers::overwriteFileWithNewDataIfDifferent (iconFile, mo)
                    && writeRCFile (rcFile, iconFile);
        return hasIcon;
    }

    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterBase);
};


//==============================================================================
class MSVCProjectExporterVC2008   : public MSVCProjectExporterBase
{
public:
    //==============================================================================
    MSVCProjectExporterVC2008 (Project& project_, const ValueTree& settings_, const char* folderName = "VisualStudio2008")
        : MSVCProjectExporterBase (project_, settings_, folderName)
    {
        name = getName();
    }

    ~MSVCProjectExporterVC2008()  {}

    static const char* getName()                    { return "Visual Studio 2008"; }
    static const char* getValueTreeTypeName()       { return "VS2008"; }

    void launchProject()                            { getSLNFile().startAsProcess(); }

    bool isDefaultFormatForCurrentOS()
    {
      #if JUCE_WINDOWS
        return true;
      #else
        return false;
      #endif
    }

    static MSVCProjectExporterVC2008* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2008 (project, settings);

        return 0;
    }

    //==============================================================================
    const String create()
    {
        createIconFile();

        if (hasIcon)
        {
            juceWrapperFiles.add (RelativePath (iconFile.getFileName(), RelativePath::buildTargetFolder));
            juceWrapperFiles.add (RelativePath (rcFile.getFileName(), RelativePath::buildTargetFolder));
        }

        {
            XmlElement projectXml ("VisualStudioProject");
            fillInProjectXml (projectXml);

            MemoryOutputStream mo;
            projectXml.writeToStream (mo, String::empty, false, true, "UTF-8", 10);

            if (! FileHelpers::overwriteFileWithNewDataIfDifferent (getVCProjFile(), mo))
                return "Can't write to the VC project file: " + getVCProjFile().getFullPathName();
        }

        {
            MemoryOutputStream mo;
            writeSolutionFile (mo, getSolutionVersionString(), getVCProjFile());

            if (! FileHelpers::overwriteFileWithNewDataIfDifferent (getSLNFile(), mo))
                return "Can't write to the VC solution file: " + getSLNFile().getFullPathName();
        }

        return String::empty;
    }

protected:
    virtual const String getProjectVersionString() const    { return "9.00"; }
    virtual const String getSolutionVersionString() const   { return "10.00" + newLine + "# Visual C++ Express 2008"; }

    const File getVCProjFile() const    { return getProjectFile (".vcproj"); }
    const File getSLNFile() const       { return getProjectFile (".sln"); }

    //==============================================================================
    void fillInProjectXml (XmlElement& projectXml)
    {
        projectXml.setAttribute ("ProjectType", "Visual C++");
        projectXml.setAttribute ("Version", getProjectVersionString());
        projectXml.setAttribute ("Name", project.getProjectName().toString());
        projectXml.setAttribute ("ProjectGUID", projectGUID);
        projectXml.setAttribute ("TargetFrameworkVersion", "131072");

        {
            XmlElement* platforms = projectXml.createNewChildElement ("Platforms");
            XmlElement* platform = platforms->createNewChildElement ("Platform");
            platform->setAttribute ("Name", "Win32");
        }

        projectXml.createNewChildElement ("ToolFiles");
        createConfigs (*projectXml.createNewChildElement ("Configurations"));
        projectXml.createNewChildElement ("References");
        createFiles (*projectXml.createNewChildElement ("Files"));
        projectXml.createNewChildElement ("Globals");
    }

    //==============================================================================
    void addFile (const RelativePath& file, XmlElement& parent, const bool excludeFromBuild, const bool useStdcall)
    {
        jassert (file.getRoot() == RelativePath::buildTargetFolder);

        XmlElement* fileXml = parent.createNewChildElement ("File");
        fileXml->setAttribute ("RelativePath", file.toWindowsStyle());

        if (excludeFromBuild || useStdcall)
        {
            for (int i = 0; i < project.getNumConfigurations(); ++i)
            {
                Project::BuildConfiguration config (project.getConfiguration (i));

                XmlElement* fileConfig = fileXml->createNewChildElement ("FileConfiguration");
                fileConfig->setAttribute ("Name", createConfigName (config));

                if (excludeFromBuild)
                    fileConfig->setAttribute ("ExcludedFromBuild", "true");

                XmlElement* tool = createToolElement (*fileConfig, "VCCLCompilerTool");

                if (useStdcall)
                    tool->setAttribute ("CallingConvention", "2");
            }
        }
    }

    XmlElement* createGroup (const String& groupName, XmlElement& parent)
    {
        XmlElement* filter = parent.createNewChildElement ("Filter");
        filter->setAttribute ("Name", groupName);
        return filter;
    }

    void addFiles (const Project::Item& projectItem, XmlElement& parent)
    {
        if (projectItem.isGroup())
        {
            XmlElement* filter = createGroup (projectItem.getName().toString(), parent);

            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                addFiles (projectItem.getChild(i), *filter);
        }
        else
        {
            if (projectItem.shouldBeAddedToTargetProject())
            {
                const RelativePath path (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);

                addFile (path, parent,
                         projectItem.shouldBeAddedToBinaryResources() || (shouldFileBeCompiledByDefault (path) && ! projectItem.shouldBeCompiled()),
                         false);
            }
        }
    }

    void addGroup (XmlElement& parent, const String& groupName, const Array<RelativePath>& files, const bool useStdcall)
    {
        if (files.size() > 0)
        {
            XmlElement* const group = createGroup (groupName, parent);

            for (int i = 0; i < files.size(); ++i)
                if (files.getReference(i).hasFileExtension ("cpp;c;cc;cxx;h;hpp;hxx;rc;ico"))
                    addFile (files.getReference(i), *group, false,
                             useStdcall && shouldFileBeCompiledByDefault (files.getReference(i)));
        }
    }

    void createFiles (XmlElement& files)
    {
        addFiles (project.getMainGroup(), files);

        addGroup (files, project.getJuceCodeGroupName(), juceWrapperFiles, false);
        addGroup (files, "Juce VST Wrapper", getVSTFilesRequired(), false);
        addGroup (files, "Juce RTAS Wrapper", getRTASFilesRequired(), true);
    }

    //==============================================================================
    XmlElement* createToolElement (XmlElement& parent, const String& toolName) const
    {
        XmlElement* const e = parent.createNewChildElement ("Tool");
        e->setAttribute ("Name", toolName);
        return e;
    }

    void createConfig (XmlElement& xml, const Project::BuildConfiguration& config) const
    {
        String binariesPath (getConfigTargetPath (config));
        String intermediatesPath (getIntermediatesPath (config));
        const bool isDebug = (bool) config.isDebug().getValue();
        const String binaryName (File::createLegalFileName (config.getTargetBinaryName().toString()));

        xml.setAttribute ("Name", createConfigName (config));
        xml.setAttribute ("OutputDirectory", FileHelpers::windowsStylePath (binariesPath));
        xml.setAttribute ("IntermediateDirectory", FileHelpers::windowsStylePath (intermediatesPath));
        xml.setAttribute ("ConfigurationType", (project.isAudioPlugin() || project.isBrowserPlugin() || isLibraryDLL())
                                                    ? "2" : (project.isLibrary() ? "4" : "1"));
        xml.setAttribute ("UseOfMFC", "0");
        xml.setAttribute ("ATLMinimizesCRunTimeLibraryUsage", "false");
        xml.setAttribute ("CharacterSet", "2");

        if (! isDebug)
            xml.setAttribute ("WholeProgramOptimization", "1");

        createToolElement (xml, "VCPreBuildEventTool");

        XmlElement* customBuild = createToolElement (xml, "VCCustomBuildTool");

        if (isRTAS())
        {
            RelativePath rsrFile (getJucePathFromTargetFolder().getChildFile ("extras/audio plugins/wrapper/RTAS/juce_RTAS_WinResources.rsr"));

            customBuild->setAttribute ("CommandLine", "copy /Y \"" + rsrFile.toWindowsStyle() + "\" \"$(TargetPath)\".rsr");
            customBuild->setAttribute ("Outputs", "\"$(TargetPath)\".rsr");
        }

        createToolElement (xml, "VCXMLDataGeneratorTool");
        createToolElement (xml, "VCWebServiceProxyGeneratorTool");

        if (! project.isLibrary())
        {
            XmlElement* midl = createToolElement (xml, "VCMIDLTool");
            midl->setAttribute ("PreprocessorDefinitions", isDebug ? "_DEBUG" : "NDEBUG");
            midl->setAttribute ("MkTypLibCompatible", "true");
            midl->setAttribute ("SuppressStartupBanner", "true");
            midl->setAttribute ("TargetEnvironment", "1");
            midl->setAttribute ("TypeLibraryName", FileHelpers::windowsStylePath (intermediatesPath + "/" + binaryName + ".tlb"));
            midl->setAttribute ("HeaderFileName", "");
        }

        {
            XmlElement* compiler = createToolElement (xml, "VCCLCompilerTool");

            const int optimiseLevel = (int) config.getOptimisationLevel().getValue();
            compiler->setAttribute ("Optimization", optimiseLevel <= 1 ? "0" : (optimiseLevel == 2 ? "2" : "3"));

            if (isDebug)
            {
                compiler->setAttribute ("BufferSecurityCheck", "");
                compiler->setAttribute ("DebugInformationFormat", project.isLibrary() ? "3" : "4");
            }
            else
            {
                compiler->setAttribute ("InlineFunctionExpansion", "1");
                compiler->setAttribute ("StringPooling", "true");
            }

            compiler->setAttribute ("AdditionalIncludeDirectories", replacePreprocessorTokens (config, getHeaderSearchPaths (config).joinIntoString (";")));
            compiler->setAttribute ("PreprocessorDefinitions", getPreprocessorDefs (config, ";"));
            compiler->setAttribute ("RuntimeLibrary", isRTAS() ? (isDebug ? 3 : 2) // MT DLL
                                                               : (isDebug ? 1 : 0)); // MT static
            compiler->setAttribute ("RuntimeTypeInfo", "true");
            compiler->setAttribute ("UsePrecompiledHeader", "0");
            compiler->setAttribute ("PrecompiledHeaderFile", FileHelpers::windowsStylePath (intermediatesPath + "/" + binaryName + ".pch"));
            compiler->setAttribute ("AssemblerListingLocation", FileHelpers::windowsStylePath (intermediatesPath + "/"));
            compiler->setAttribute ("ObjectFile", FileHelpers::windowsStylePath (intermediatesPath + "/"));
            compiler->setAttribute ("ProgramDataBaseFileName", FileHelpers::windowsStylePath (intermediatesPath + "/"));
            compiler->setAttribute ("WarningLevel", "4");
            compiler->setAttribute ("SuppressStartupBanner", "true");

            const String extraFlags (replacePreprocessorTokens (config, getExtraCompilerFlags().toString()).trim());
            if (extraFlags.isNotEmpty())
                compiler->setAttribute ("AdditionalOptions", extraFlags);
        }

        createToolElement (xml, "VCManagedResourceCompilerTool");

        {
            XmlElement* resCompiler = createToolElement (xml, "VCResourceCompilerTool");
            resCompiler->setAttribute ("PreprocessorDefinitions", isDebug ? "_DEBUG" : "NDEBUG");
        }

        createToolElement (xml, "VCPreLinkEventTool");

        const String outputFileName (getBinaryFileForConfig (config));

        if (! project.isLibrary())
        {
            XmlElement* linker = createToolElement (xml, "VCLinkerTool");

            linker->setAttribute ("OutputFile", FileHelpers::windowsStylePath (binariesPath + "/" + outputFileName));
            linker->setAttribute ("SuppressStartupBanner", "true");

            if (project.getJuceLinkageMode() == Project::useLinkedJuce)
                linker->setAttribute ("AdditionalLibraryDirectories", getJucePathFromTargetFolder().getChildFile ("bin").toWindowsStyle());

            linker->setAttribute ("IgnoreDefaultLibraryNames", isDebug ? "libcmt.lib, msvcrt.lib" : "");
            linker->setAttribute ("GenerateDebugInformation", isDebug ? "true" : "false");
            linker->setAttribute ("ProgramDatabaseFile", FileHelpers::windowsStylePath (intermediatesPath + "/" + binaryName + ".pdb"));
            linker->setAttribute ("SubSystem", project.isCommandLineApp() ? "1" : "2");

            if (! isDebug)
            {
                linker->setAttribute ("GenerateManifest", "false");
                linker->setAttribute ("OptimizeReferences", "2");
                linker->setAttribute ("EnableCOMDATFolding", "2");
            }

            linker->setAttribute ("TargetMachine", "1"); // (64-bit build = 5)

            String extraLinkerOptions (getExtraLinkerFlags().toString());

            if (isRTAS())
            {
                extraLinkerOptions += " /FORCE:multiple";
                linker->setAttribute ("DelayLoadDLLs", "DAE.dll; DigiExt.dll; DSI.dll; PluginLib.dll; DSPManager.dll");
                linker->setAttribute ("ModuleDefinitionFile", getJucePathFromTargetFolder()
                                            .getChildFile ("extras/audio plugins/wrapper/RTAS/juce_RTAS_WinExports.def")
                                            .toWindowsStyle());
            }

            if (extraLinkerOptions.isNotEmpty())
                linker->setAttribute ("AdditionalOptions", replacePreprocessorTokens (config, extraLinkerOptions).trim());
        }
        else
        {
            if (isLibraryDLL())
            {
                XmlElement* linker = createToolElement (xml, "VCLinkerTool");

                String extraLinkerOptions (getExtraLinkerFlags().toString());
                extraLinkerOptions << " /IMPLIB:" << FileHelpers::windowsStylePath (binariesPath + "/" + outputFileName.upToLastOccurrenceOf (".", false, false) + ".lib");
                linker->setAttribute ("AdditionalOptions", replacePreprocessorTokens (config, extraLinkerOptions).trim());

                linker->setAttribute ("OutputFile", FileHelpers::windowsStylePath (binariesPath + "/" + outputFileName));
                linker->setAttribute ("IgnoreDefaultLibraryNames", isDebug ? "libcmt.lib, msvcrt.lib" : "");
            }
            else
            {
                XmlElement* librarian = createToolElement (xml, "VCLibrarianTool");

                librarian->setAttribute ("OutputFile", FileHelpers::windowsStylePath (binariesPath + "/" + outputFileName));
                librarian->setAttribute ("IgnoreDefaultLibraryNames", isDebug ? "libcmt.lib, msvcrt.lib" : "");
            }
        }

        createToolElement (xml, "VCALinkTool");
        createToolElement (xml, "VCManifestTool");
        createToolElement (xml, "VCXDCMakeTool");

        {
            XmlElement* bscMake = createToolElement (xml, "VCBscMakeTool");
            bscMake->setAttribute ("SuppressStartupBanner", "true");
            bscMake->setAttribute ("OutputFile", FileHelpers::windowsStylePath (intermediatesPath + "/" + binaryName + ".bsc"));
        }

        createToolElement (xml, "VCFxCopTool");

        if (! project.isLibrary())
            createToolElement (xml, "VCAppVerifierTool");

        createToolElement (xml, "VCPostBuildEventTool");
    }

    void createConfigs (XmlElement& configs)
    {
        for (int i = 0; i < project.getNumConfigurations(); ++i)
        {
            Project::BuildConfiguration config (project.getConfiguration (i));
            createConfig (*configs.createNewChildElement ("Configuration"), config);
        }
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2008);
};


//==============================================================================
class MSVCProjectExporterVC2005   : public MSVCProjectExporterVC2008
{
public:
    MSVCProjectExporterVC2005 (Project& project_, const ValueTree& settings_)
        : MSVCProjectExporterVC2008 (project_, settings_, "VisualStudio2005")
    {
        name = getName();
    }

    ~MSVCProjectExporterVC2005()  {}

    static const char* getName()                    { return "Visual Studio 2005"; }
    static const char* getValueTreeTypeName()       { return "VS2005"; }

    bool isDefaultFormatForCurrentOS()              { return false; }

    static MSVCProjectExporterVC2005* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2005 (project, settings);

        return 0;
    }

protected:
    const String getProjectVersionString() const    { return "8.00"; }
    const String getSolutionVersionString() const   { return "8.00" + newLine + "# Visual C++ Express 2005"; }

    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2005);
};


//==============================================================================
class MSVCProjectExporterVC6   : public MSVCProjectExporterBase
{
public:
    //==============================================================================
    MSVCProjectExporterVC6 (Project& project_, const ValueTree& settings_)
        : MSVCProjectExporterBase (project_, settings_, "MSVC6")
    {
        name = getName();
    }

    ~MSVCProjectExporterVC6()  {}

    static const char* getName()                   { return "Visual C++ 6.0"; }
    static const char* getValueTreeTypeName()      { return "MSVC6"; }

    bool isDefaultFormatForCurrentOS()              { return false; }
    void launchProject()                            { getDSWFile().startAsProcess(); }

    static MSVCProjectExporterVC6* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC6 (project, settings);

        return 0;
    }

    //==============================================================================
    const String create()
    {
        {
            MemoryOutputStream mo;
            writeProject (mo);

            if (! FileHelpers::overwriteFileWithNewDataIfDifferent (getDSPFile(), mo))
                return "Can't write to the VC project file: " + getDSPFile().getFullPathName();
        }

        {
            MemoryOutputStream mo;
            writeDSWFile (mo);

            if (! FileHelpers::overwriteFileWithNewDataIfDifferent (getDSWFile(), mo))
                return "Can't write to the VC solution file: " + getDSWFile().getFullPathName();
        }

        return String::empty;
    }

private:
    const File getDSPFile() const       { return getProjectFile (".dsp"); }
    const File getDSWFile() const       { return getProjectFile (".dsw"); }

    //==============================================================================
    const String createConfigName (const Project::BuildConfiguration& config) const
    {
        return project.getProjectName().toString() + " - Win32 " + config.getName().toString();
    }

    void writeProject (OutputStream& out)
    {
        const String defaultConfigName (createConfigName (project.getConfiguration (0)));

        const bool isDLL = project.isAudioPlugin() || project.isBrowserPlugin();
        String targetType, targetCode;

        if (isDLL)                              { targetType = "\"Win32 (x86) Dynamic-Link Library\"";  targetCode = "0x0102"; }
        else if (project.isLibrary())           { targetType = "\"Win32 (x86) Static Library\"";        targetCode = "0x0104"; }
        else if (project.isCommandLineApp())    { targetType = "\"Win32 (x86) Console Application\"";   targetCode = "0x0103"; }
        else                                    { targetType = "\"Win32 (x86) Application\"";           targetCode = "0x0101"; }

        out << "# Microsoft Developer Studio Project File - Name=\"" << project.getProjectName()
            << "\" - Package Owner=<4>" << newLine
            << "# Microsoft Developer Studio Generated Build File, Format Version 6.00" << newLine
            << "# ** DO NOT EDIT **" << newLine
            << "# TARGTYPE " << targetType << " " << targetCode << newLine
            << "CFG=" << defaultConfigName << newLine
            << "!MESSAGE This is not a valid makefile. To build this project using NMAKE," << newLine
            << "!MESSAGE use the Export Makefile command and run" << newLine
            << "!MESSAGE " << newLine
            << "!MESSAGE NMAKE /f \"" << project.getProjectName() << ".mak.\"" << newLine
            << "!MESSAGE " << newLine
            << "!MESSAGE You can specify a configuration when running NMAKE" << newLine
            << "!MESSAGE by defining the macro CFG on the command line. For example:" << newLine
            << "!MESSAGE " << newLine
            << "!MESSAGE NMAKE /f \"" << project.getProjectName() << ".mak\" CFG=\"" << defaultConfigName << '"' << newLine
            << "!MESSAGE " << newLine
            << "!MESSAGE Possible choices for configuration are:" << newLine
            << "!MESSAGE " << newLine;

        int i;
        for (i = 0; i < project.getNumConfigurations(); ++i)
            out << "!MESSAGE \"" << createConfigName (project.getConfiguration (i)) << "\" (based on " << targetType << ")" << newLine;

        out << "!MESSAGE " << newLine
            << "# Begin Project" << newLine
            << "# PROP AllowPerConfigDependencies 0" << newLine
            << "# PROP Scc_ProjName \"\"" << newLine
            << "# PROP Scc_LocalPath \"\"" << newLine
            << "CPP=cl.exe" << newLine
            << "MTL=midl.exe" << newLine
            << "RSC=rc.exe" << newLine;

        String targetList;

        for (i = 0; i < project.getNumConfigurations(); ++i)
        {
            const Project::BuildConfiguration config (project.getConfiguration (i));
            const String configName (createConfigName (config));
            targetList << "# Name \"" << configName << '"' << newLine;

            const String binariesPath (getConfigTargetPath (config));
            const String targetBinary (FileHelpers::windowsStylePath (binariesPath + "/" + getBinaryFileForConfig (config)));
            const String optimisationFlag (((int) config.getOptimisationLevel().getValue() <= 1) ? "Od" : (config.getOptimisationLevel() == 2 ? "O2" : "O3"));
            const String defines (getPreprocessorDefs (config, " /D "));
            const bool isDebug = (bool) config.isDebug().getValue();
            const String extraDebugFlags (isDebug ? "/Gm /ZI /GZ" : "");

            out << (i == 0 ? "!IF" : "!ELSEIF") << "  \"$(CFG)\" == \"" << configName << '"' << newLine
                << "# PROP BASE Use_MFC 0" << newLine
                << "# PROP BASE Use_Debug_Libraries " << (isDebug ? "1" : "0") << newLine
                << "# PROP BASE Output_Dir \"" << binariesPath << '"' << newLine
                << "# PROP BASE Intermediate_Dir \"" << getIntermediatesPath (config) << '"' << newLine
                << "# PROP BASE Target_Dir \"\"" << newLine
                << "# PROP Use_MFC 0" << newLine
                << "# PROP Use_Debug_Libraries " << (isDebug ? "1" : "0") << newLine
                << "# PROP Output_Dir \"" << binariesPath << '"' << newLine
                << "# PROP Intermediate_Dir \"" << getIntermediatesPath (config) << '"' << newLine
                << "# PROP Ignore_Export_Lib 0" << newLine
                << "# PROP Target_Dir \"\"" << newLine
                << "# ADD BASE CPP /nologo /W3 /GX /" << optimisationFlag << " /D " << defines
                << " /YX /FD /c " << extraDebugFlags << " /Zm1024" << newLine
                << "# ADD CPP /nologo " << (isDebug ? "/MTd" : "/MT") << " /W3 /GR /GX /" << optimisationFlag
                << " /I " << replacePreprocessorTokens (config, getHeaderSearchPaths (config).joinIntoString (" /I "))
                << " /D " << defines << " /D \"_UNICODE\" /D \"UNICODE\" /FD /c /Zm1024 " << extraDebugFlags
                << " " << replacePreprocessorTokens (config, getExtraCompilerFlags().toString()).trim() << newLine;

            if (! isDebug)
                out << "# SUBTRACT CPP /YX" << newLine;

            if (! project.isLibrary())
                out << "# ADD BASE MTL /nologo /D " << defines << " /mktyplib203 /win32" << newLine
                    << "# ADD MTL /nologo /D " << defines << " /mktyplib203 /win32" << newLine;

            out << "# ADD BASE RSC /l 0x40c /d " << defines << newLine
                << "# ADD RSC /l 0x40c /d " << defines << newLine
                << "BSC32=bscmake.exe" << newLine
                << "# ADD BASE BSC32 /nologo" << newLine
                << "# ADD BSC32 /nologo" << newLine;

            if (project.isLibrary())
            {
                out << "LIB32=link.exe -lib" << newLine
                    << "# ADD BASE LIB32 /nologo" << newLine
                    << "# ADD LIB32 /nologo /out:\"" << targetBinary << '"' << newLine;
            }
            else
            {
                out << "LINK32=link.exe" << newLine
                    << "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386" << newLine
                    << "# ADD LINK32 \"C:\\Program Files\\Microsoft Visual Studio\\VC98\\LIB\\shell32.lib\" " // This is avoid debug information corruption when mixing Platform SDK
                    << "kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib "
                    << (isDebug ? " /debug" : "")
                    << " /nologo /machine:I386 /out:\"" << targetBinary << "\" "
                    << (isDLL ? "/dll" : (project.isCommandLineApp() ? "/subsystem:console "
                                                                     : "/subsystem:windows "))
                    << replacePreprocessorTokens (config, getExtraLinkerFlags().toString()).trim() << newLine;
            }
        }

        out << "!ENDIF" << newLine
            << "# Begin Target"  << newLine
            << targetList;

        writeFiles (out, project.getMainGroup());
        writeGroup (out, project.getJuceCodeGroupName(), juceWrapperFiles);
        writeGroup (out, "Juce VST Wrapper", getVSTFilesRequired());

        out << "# End Target" << newLine
            << "# End Project" << newLine;
    }

    void writeFile (OutputStream& out, const RelativePath& file, const bool excludeFromBuild)
    {
        jassert (file.getRoot() == RelativePath::buildTargetFolder);

        out << "# Begin Source File" << newLine
            << "SOURCE=" << file.toWindowsStyle().quoted() << newLine;

        if (excludeFromBuild)
            out << "# PROP Exclude_From_Build 1" << newLine;

        out << "# End Source File" << newLine;
    }

    void writeFiles (OutputStream& out, const Project::Item& projectItem)
    {
        if (projectItem.isGroup())
        {
            out << "# Begin Group \"" << projectItem.getName() << '"' << newLine
                << "# PROP Default_Filter \"cpp;c;cc;cxx;rc;def;r;odl;idl;hpj;bat\"" << newLine;

            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                writeFiles (out, projectItem.getChild (i));

            out << "# End Group" << newLine;
        }
        else if (projectItem.shouldBeAddedToTargetProject())
        {
            const RelativePath path (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);
            writeFile (out, path, projectItem.shouldBeAddedToBinaryResources() || (shouldFileBeCompiledByDefault (path) && ! projectItem.shouldBeCompiled()));
        }
    }

    void writeGroup (OutputStream& out, const String& groupName, const Array<RelativePath>& files)
    {
        if (files.size() > 0)
        {
            out << "# Begin Group \"" << groupName << '"' << newLine;
            for (int i = 0; i < files.size(); ++i)
                if (files.getReference(i).hasFileExtension ("cpp;cc;c;cxx;h;hpp;hxx"))
                    writeFile (out, files.getReference(i), false);

            out << "# End Group" << newLine;
        }
    }

    void writeDSWFile (OutputStream& out)
    {
        out << "Microsoft Developer Studio Workspace File, Format Version 6.00 " << newLine;

        if (! project.isUsingWrapperFiles())
        {
            out << "Project: \"JUCE\"= ..\\JUCE.dsp - Package Owner=<4>" << newLine
                << "Package=<5>" << newLine
                << "{{{" << newLine
                << "}}}" << newLine
                << "Package=<4>" << newLine
                << "{{{" << newLine
                << "}}}" << newLine;
        }

        out << "Project: \"" << project.getProjectName() << "\" = .\\" << getDSPFile().getFileName() << " - Package Owner=<4>" << newLine
            << "Package=<5>" << newLine
            << "{{{" << newLine
            << "}}}" << newLine
            << "Package=<4>" << newLine
            << "{{{" << newLine;

        if (! project.isUsingWrapperFiles())
        {
            out << "    Begin Project Dependency" << newLine
                << "    Project_Dep_Name JUCE" << newLine
                << "    End Project Dependency" << newLine;
        }

        out << "}}}" << newLine
            << "Global:" << newLine
            << "Package=<5>" << newLine
            << "{{{" << newLine
            << "}}}" << newLine
            << "Package=<3>" << newLine
            << "{{{" << newLine
            << "}}}" << newLine;
    }

    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC6);
};


//==============================================================================
class MSVCProjectExporterVC2010   : public MSVCProjectExporterBase
{
public:
    MSVCProjectExporterVC2010 (Project& project_, const ValueTree& settings_)
        : MSVCProjectExporterBase (project_, settings_, "VisualStudio2010")
    {
        name = getName();
    }

    ~MSVCProjectExporterVC2010()  {}

    static const char* getName()                    { return "Visual Studio 2010"; }
    static const char* getValueTreeTypeName()       { return "VS2010"; }

    bool isDefaultFormatForCurrentOS()              { return false; }
    void launchProject()                            { getSLNFile().startAsProcess(); }

    static MSVCProjectExporterVC2010* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2010 (project, settings);

        return 0;
    }

    //==============================================================================
    const String create()
    {
        createIconFile();

        {
            XmlElement projectXml ("Project");
            fillInProjectXml (projectXml);

            MemoryOutputStream mo;
            projectXml.writeToStream (mo, String::empty, false, true, "utf-8", 100);

            if (! FileHelpers::overwriteFileWithNewDataIfDifferent (getVCProjFile(), mo))
                return "Can't write to the VC project file: " + getVCProjFile().getFullPathName();
        }

        {
            XmlElement filtersXml ("Project");
            fillInFiltersXml (filtersXml);

            MemoryOutputStream mo;
            filtersXml.writeToStream (mo, String::empty, false, true, "utf-8", 100);

            if (! FileHelpers::overwriteFileWithNewDataIfDifferent (getVCProjFiltersFile(), mo))
                return "Can't write to the VC project file: " + getVCProjFiltersFile().getFullPathName();
        }

        {
            MemoryOutputStream mo;
            writeSolutionFile (mo, "11.00", getVCProjFile());

            if (! FileHelpers::overwriteFileWithNewDataIfDifferent (getSLNFile(), mo))
                return "Can't write to the VC solution file: " + getSLNFile().getFullPathName();
        }

        return String::empty;
    }

protected:
    const File getVCProjFile() const            { return getProjectFile (".vcxproj"); }
    const File getVCProjFiltersFile() const     { return getProjectFile (".vcxproj.filters"); }
    const File getSLNFile() const               { return getProjectFile (".sln"); }

    static const String createConfigName (const Project::BuildConfiguration& config)
    {
        return config.getName().toString() + "|Win32";
    }

    static void setConditionAttribute (XmlElement& xml, const Project::BuildConfiguration& config)
    {
        xml.setAttribute ("Condition", "'$(Configuration)|$(Platform)'=='" + createConfigName (config) + "'");
    }

    //==============================================================================
    void fillInProjectXml (XmlElement& projectXml)
    {
        projectXml.setAttribute ("DefaultTargets", "Build");
        projectXml.setAttribute ("ToolsVersion", "4.0");
        projectXml.setAttribute ("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

        {
            XmlElement* configsGroup = projectXml.createNewChildElement ("ItemGroup");
            configsGroup->setAttribute ("Label", "ProjectConfigurations");

            for (int i = 0; i < project.getNumConfigurations(); ++i)
            {
                Project::BuildConfiguration config (project.getConfiguration (i));

                XmlElement* e = configsGroup->createNewChildElement ("ProjectConfiguration");
                e->setAttribute ("Include", createConfigName (config));
                e->createNewChildElement ("Configuration")->addTextElement (config.getName().toString());
                e->createNewChildElement ("Platform")->addTextElement ("Win32");
            }
        }

        {
            XmlElement* globals = projectXml.createNewChildElement ("PropertyGroup");
            globals->setAttribute ("Label", "Globals");
            globals->createNewChildElement ("ProjectGuid")->addTextElement (projectGUID);
        }

        {
            XmlElement* imports = projectXml.createNewChildElement ("Import");
            imports->setAttribute ("Project", "$(VCTargetsPath)\\Microsoft.Cpp.Default.props");
        }

        for (int i = 0; i < project.getNumConfigurations(); ++i)
        {
            Project::BuildConfiguration config (project.getConfiguration (i));

            XmlElement* e = projectXml.createNewChildElement ("PropertyGroup");
            setConditionAttribute (*e, config);
            e->setAttribute ("Label", "Configuration");
            e->createNewChildElement ("ConfigurationType")->addTextElement (getProjectType());
            e->createNewChildElement ("UseOfMfc")->addTextElement ("false");
            e->createNewChildElement ("CharacterSet")->addTextElement ("MultiByte");

            if (! config.isDebug().getValue())
                e->createNewChildElement ("WholeProgramOptimization")->addTextElement ("true");
        }

        {
            XmlElement* e = projectXml.createNewChildElement ("Import");
            e->setAttribute ("Project", "$(VCTargetsPath)\\Microsoft.Cpp.props");
        }

        {
            XmlElement* e = projectXml.createNewChildElement ("ImportGroup");
            e->setAttribute ("Label", "ExtensionSettings");
        }

        {
            XmlElement* e = projectXml.createNewChildElement ("ImportGroup");
            e->setAttribute ("Label", "PropertySheets");
            XmlElement* p = e->createNewChildElement ("Import");
            p->setAttribute ("Project", "$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props");
            p->setAttribute ("Condition", "exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')");
            p->setAttribute ("Label", "LocalAppDataPlatform");
        }

        {
            XmlElement* e = projectXml.createNewChildElement ("PropertyGroup");
            e->setAttribute ("Label", "UserMacros");
        }

        {
            XmlElement* props = projectXml.createNewChildElement ("PropertyGroup");
            props->createNewChildElement ("_ProjectFileVersion")->addTextElement ("10.0.30319.1");

            for (int i = 0; i < project.getNumConfigurations(); ++i)
            {
                Project::BuildConfiguration config (project.getConfiguration (i));

                XmlElement* outdir = props->createNewChildElement ("OutDir");
                setConditionAttribute (*outdir, config);
                outdir->addTextElement (getConfigTargetPath (config) + "\\");

                XmlElement* intdir = props->createNewChildElement ("IntDir");
                setConditionAttribute (*intdir, config);
                intdir->addTextElement (getConfigTargetPath (config) + "\\");

                XmlElement* name = props->createNewChildElement ("TargetName");
                setConditionAttribute (*name, config);
                name->addTextElement (getBinaryFileForConfig (config).upToLastOccurrenceOf (".", false, false));
            }
        }

        for (int i = 0; i < project.getNumConfigurations(); ++i)
        {
            Project::BuildConfiguration config (project.getConfiguration (i));
            String binariesPath (getConfigTargetPath (config));
            String intermediatesPath (getIntermediatesPath (config));
            const bool isDebug = (bool) config.isDebug().getValue();
            const String binaryName (File::createLegalFileName (config.getTargetBinaryName().toString()));
            const String outputFileName (getBinaryFileForConfig (config));

            XmlElement* group = projectXml.createNewChildElement ("ItemDefinitionGroup");
            setConditionAttribute (*group, config);

            XmlElement* midl = group->createNewChildElement ("Midl");
            midl->createNewChildElement ("PreprocessorDefinitions")->addTextElement (isDebug ? "_DEBUG;%(PreprocessorDefinitions)"
                                                                                             : "NDEBUG;%(PreprocessorDefinitions)");
            midl->createNewChildElement ("MkTypLibCompatible")->addTextElement ("true");
            midl->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
            midl->createNewChildElement ("TargetEnvironment")->addTextElement ("Win32");
            //midl->createNewChildElement ("TypeLibraryName")->addTextElement ("");
            midl->createNewChildElement ("HeaderFileName");

            XmlElement* cl = group->createNewChildElement ("ClCompile");
            cl->createNewChildElement ("Optimization")->addTextElement (isDebug ? "Disabled" : "MaxSpeed");

            if (isDebug)
                cl->createNewChildElement ("DebugInformationFormat")->addTextElement ("EditAndContinue");

            StringArray includePaths (getHeaderSearchPaths (config));
            includePaths.add ("%(AdditionalIncludeDirectories)");
            cl->createNewChildElement ("AdditionalIncludeDirectories")->addTextElement (includePaths.joinIntoString (";"));
            cl->createNewChildElement ("PreprocessorDefinitions")->addTextElement (getPreprocessorDefs (config, ";") + ";%(PreprocessorDefinitions)");
            cl->createNewChildElement ("RuntimeLibrary")->addTextElement (isRTAS() ? (isDebug ? "MultiThreadedDLLDebug" : "MultiThreadedDLL")
                                                                                   : (isDebug ? "MultiThreadedDebug" : "MultiThreaded"));
            cl->createNewChildElement ("RuntimeTypeInfo")->addTextElement ("true");
            cl->createNewChildElement ("PrecompiledHeader");
            cl->createNewChildElement ("AssemblerListingLocation")->addTextElement (FileHelpers::windowsStylePath (intermediatesPath + "/"));
            cl->createNewChildElement ("ObjectFileName")->addTextElement (FileHelpers::windowsStylePath (intermediatesPath + "/"));
            cl->createNewChildElement ("ProgramDataBaseFileName")->addTextElement (FileHelpers::windowsStylePath (intermediatesPath + "/"));
            cl->createNewChildElement ("WarningLevel")->addTextElement ("Level4");
            cl->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");

            XmlElement* res = group->createNewChildElement ("ResourceCompile");
            res->createNewChildElement ("PreprocessorDefinitions")->addTextElement (isDebug ? "_DEBUG;%(PreprocessorDefinitions)"
                                                                                            : "NDEBUG;%(PreprocessorDefinitions)");

            XmlElement* link = group->createNewChildElement ("Link");
            link->createNewChildElement ("OutputFile")->addTextElement (FileHelpers::windowsStylePath (binariesPath + "/" + outputFileName));
            link->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
            link->createNewChildElement ("IgnoreSpecificDefaultLibraries")->addTextElement (isDebug ? "libcmt.lib; msvcrt.lib;;%(IgnoreSpecificDefaultLibraries)"
                                                                                                    : "%(IgnoreSpecificDefaultLibraries)");
            link->createNewChildElement ("GenerateDebugInformation")->addTextElement (isDebug ? "true" : "false");
            link->createNewChildElement ("ProgramDatabaseFile")->addTextElement (FileHelpers::windowsStylePath (intermediatesPath + "/" + binaryName + ".pdb"));
            link->createNewChildElement ("SubSystem")->addTextElement (project.isCommandLineApp() ? "Console" : "Windows");
            link->createNewChildElement ("TargetMachine")->addTextElement ("MachineX86");

            if (! isDebug)
            {
                link->createNewChildElement ("OptimizeReferences")->addTextElement ("true");
                link->createNewChildElement ("EnableCOMDATFolding")->addTextElement ("true");
            }

            XmlElement* bsc = group->createNewChildElement ("Bscmake");
            bsc->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
            bsc->createNewChildElement ("OutputFile")->addTextElement (FileHelpers::windowsStylePath (intermediatesPath + "/" + binaryName + ".bsc"));
        }

        {
            XmlElement* cppFiles = projectXml.createNewChildElement ("ItemGroup");
            XmlElement* headerFiles = projectXml.createNewChildElement ("ItemGroup");

            addFilesToCompile (project.getMainGroup(), *cppFiles, *headerFiles);
            addFilesToCompile (juceWrapperFiles, *cppFiles, *headerFiles, false);
            addFilesToCompile (getVSTFilesRequired(), *cppFiles, *headerFiles, false);
            addFilesToCompile (getRTASFilesRequired(), *cppFiles, *headerFiles, true);
        }

        if (hasIcon)
        {
            {
                XmlElement* iconGroup = projectXml.createNewChildElement ("ItemGroup");
                XmlElement* e = iconGroup->createNewChildElement ("None");
                e->setAttribute ("Include", ".\\" + iconFile.getFileName());
            }

            {
                XmlElement* rcGroup = projectXml.createNewChildElement ("ItemGroup");
                XmlElement* e = rcGroup->createNewChildElement ("ResourceCompile");
                e->setAttribute ("Include", ".\\" + rcFile.getFileName());
            }
        }

        {
            XmlElement* e = projectXml.createNewChildElement ("Import");
            e->setAttribute ("Project", "$(VCTargetsPath)\\Microsoft.Cpp.targets");
        }

        {
            XmlElement* e = projectXml.createNewChildElement ("ImportGroup");
            e->setAttribute ("Label", "ExtensionTargets");
        }
    }

    const String getProjectType() const
    {
        if (project.isGUIApplication() || project.isCommandLineApp())
            return "Application";
        else if (project.isAudioPlugin() || project.isBrowserPlugin())
            return "DynamicLibrary";
        else if (project.isLibrary())
            return "StaticLibrary";

        jassertfalse;
        return String::empty;
    }

    //==============================================================================
    void addFileToCompile (const RelativePath& file, XmlElement& cpps, XmlElement& headers, const bool excludeFromBuild, const bool useStdcall)
    {
        jassert (file.getRoot() == RelativePath::buildTargetFolder);

        if (file.hasFileExtension ("cpp;cc;cxx;c"))
        {
            XmlElement* e = cpps.createNewChildElement ("ClCompile");
            e->setAttribute ("Include", file.toWindowsStyle());

            if (excludeFromBuild)
                e->createNewChildElement ("ExcludedFromBuild")->addTextElement ("true");

            if (useStdcall)
            {
                jassertfalse;
            }
        }
        else if (file.hasFileExtension (headerFileExtensions))
        {
            headers.createNewChildElement ("ClInclude")->setAttribute ("Include", file.toWindowsStyle());
        }
    }

    void addFilesToCompile (const Array<RelativePath>& files, XmlElement& cpps, XmlElement& headers, bool useStdCall)
    {
        for (int i = 0; i < files.size(); ++i)
            addFileToCompile (files.getReference(i), cpps, headers, false,
                              useStdCall && shouldFileBeCompiledByDefault (files.getReference(i)));
    }

    void addFilesToCompile (const Project::Item& projectItem, XmlElement& cpps, XmlElement& headers)
    {
        if (projectItem.isGroup())
        {
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                addFilesToCompile (projectItem.getChild(i), cpps, headers);
        }
        else
        {
            if (projectItem.shouldBeAddedToTargetProject())
            {
                const RelativePath path (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);

                if (path.hasFileExtension (headerFileExtensions) || (path.hasFileExtension ("cpp;cc;c;cxx") && projectItem.shouldBeCompiled()))
                    addFileToCompile (path, cpps, headers, false, false);
            }
        }
    }

    //==============================================================================
    void addFilterGroup (XmlElement& groups, const String& path)
    {
        XmlElement* e = groups.createNewChildElement ("Filter");
        e->setAttribute ("Include", path);
        e->createNewChildElement ("UniqueIdentifier")->addTextElement (createGUID (path + "_guidpathsaltxhsdf"));
    }

    void addFileToFilter (const RelativePath& file, const String& groupPath, XmlElement& cpps, XmlElement& headers)
    {
        XmlElement* e;

        if (file.hasFileExtension (headerFileExtensions))
            e = headers.createNewChildElement ("ClInclude");
        else
            e = cpps.createNewChildElement ("ClCompile");

        jassert (file.getRoot() == RelativePath::buildTargetFolder);
        e->setAttribute ("Include", file.toWindowsStyle());
        e->createNewChildElement ("Filter")->addTextElement (groupPath);
    }

    void addFilesToFilter (const Project::Item& projectItem, const String& path, XmlElement& cpps, XmlElement& headers, XmlElement& groups)
    {
        if (projectItem.isGroup())
        {
            addFilterGroup (groups, path);

            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                addFilesToFilter (projectItem.getChild(i),
                                  (path.isEmpty() ? String::empty : (path + "\\")) + projectItem.getChild(i).getName().toString(),
                                  cpps, headers, groups);
        }
        else
        {
            if (projectItem.shouldBeAddedToTargetProject())
            {
                addFileToFilter (RelativePath (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder),
                                 path.upToLastOccurrenceOf ("\\", false, false), cpps, headers);
            }
        }
    }

    void addFilesToFilter (const Array<RelativePath>& files, const String& path, XmlElement& cpps, XmlElement& headers, XmlElement& groups)
    {
        if (files.size() > 0)
        {
            addFilterGroup (groups, path);

            for (int i = 0; i < files.size(); ++i)
                addFileToFilter (files.getReference(i), path, cpps, headers);
        }
    }

    void fillInFiltersXml (XmlElement& filterXml)
    {
        filterXml.setAttribute ("ToolsVersion", "4.0");
        filterXml.setAttribute ("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

        XmlElement* groups = filterXml.createNewChildElement ("ItemGroup");
        XmlElement* cpps = filterXml.createNewChildElement ("ItemGroup");
        XmlElement* headers = filterXml.createNewChildElement ("ItemGroup");

        addFilesToFilter (project.getMainGroup(), project.getProjectName().toString(), *cpps, *headers, *groups);

        addFilesToFilter (juceWrapperFiles, project.getJuceCodeGroupName(), *cpps, *headers, *groups);
        addFilesToFilter (getVSTFilesRequired(), "Juce VST Wrapper", *cpps, *headers, *groups);
        addFilesToFilter (getRTASFilesRequired(), "Juce RTAS Wrapper", *cpps, *headers, *groups);

        if (iconFile.exists())
        {
            {
                XmlElement* iconGroup = filterXml.createNewChildElement ("ItemGroup");
                XmlElement* e = iconGroup->createNewChildElement ("None");
                e->setAttribute ("Include", ".\\" + iconFile.getFileName());
                e->createNewChildElement ("Filter")->addTextElement (project.getJuceCodeGroupName());
            }

            {
                XmlElement* rcGroup = filterXml.createNewChildElement ("ItemGroup");
                XmlElement* e = rcGroup->createNewChildElement ("ResourceCompile");
                e->setAttribute ("Include", ".\\" + rcFile.getFileName());
                e->createNewChildElement ("Filter")->addTextElement (project.getJuceCodeGroupName());
            }
        }
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2010);
};


#endif   // __JUCER_PROJECTEXPORT_MSVC_JUCEHEADER__
