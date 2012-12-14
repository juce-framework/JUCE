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

#ifndef __JUCER_PROJECTEXPORT_MSVC_JUCEHEADER__
#define __JUCER_PROJECTEXPORT_MSVC_JUCEHEADER__

#include "jucer_ProjectExporter.h"
#include "jucer_ProjectSaver.h"

//==============================================================================
class MSVCProjectExporterBase   : public ProjectExporter
{
public:
    MSVCProjectExporterBase (Project& p, const ValueTree& t, const char* const folderName)
        : ProjectExporter (p, t)
    {
        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + folderName;

        if ((int) getLibraryType().getValue() <= 0)
            getLibraryType() = 1;

        projectGUID = createGUID (project.getProjectUID());

        updateOldSettings();
    }

    //==============================================================================
    bool usesMMFiles() const                    { return false; }
    bool isVisualStudio() const                 { return true; }
    bool canCopeWithDuplicateFiles()            { return false; }

    bool launchProject()
    {
       #if JUCE_WINDOWS
        return getSLNFile().startAsProcess();
       #else
        return false;
       #endif
    }

    void createExporterProperties (PropertyListBuilder& props)
    {
        if (projectType.isLibrary())
        {
            const char* const libTypes[] = { "Static Library (.lib)", "Dynamic Library (.dll)", 0 };
            const int libTypeValues[] = { 1, 2, 0 };
            props.add (new ChoicePropertyComponent (getLibraryType(), "Library Type",
                                                    StringArray (libTypes), Array<var> (libTypeValues)));
        }
    }

protected:
    String projectGUID;
    mutable File rcFile, iconFile;

    File getProjectFile (const String& extension) const   { return getTargetFolder().getChildFile (project.getProjectFilenameRoot()).withFileExtension (extension); }
    File getSLNFile() const             { return getProjectFile (".sln"); }

    Value getLibraryType()              { return getSetting (Ids::libraryType); }
    String getLibraryString() const     { return getSettingString (Ids::libraryType); }
    bool isLibraryDLL() const           { return msvcIsDLL || (projectType.isLibrary() && (int) settings [Ids::libraryType] == 2); }

    static String prependIfNotAbsolute (const String& file, const char* prefix)
    {
        if (File::isAbsolutePath (file) || file.startsWithChar ('$'))
            prefix = "";

        return prefix + FileHelpers::windowsStylePath (file);
    }

    static String getIntDirFile (const String& file)  { return prependIfNotAbsolute (file, "$(IntDir)\\"); }
    static String getOutDirFile (const String& file)  { return prependIfNotAbsolute (file, "$(OutDir)\\"); }

    void updateOldSettings()
    {
        {
            const String oldStylePrebuildCommand (getSettingString (Ids::prebuildCommand));
            settings.removeProperty (Ids::prebuildCommand, nullptr);

            if (oldStylePrebuildCommand.isNotEmpty())
                for (ConfigIterator config (*this); config.next();)
                    dynamic_cast <MSVCBuildConfiguration&> (*config).getPrebuildCommand() = oldStylePrebuildCommand;
        }

        {
            const String oldStyleLibName (getSettingString ("libraryName_Debug"));
            settings.removeProperty ("libraryName_Debug", nullptr);

            if (oldStyleLibName.isNotEmpty())
                for (ConfigIterator config (*this); config.next();)
                    if (config->isDebug())
                        config->getTargetBinaryName() = oldStyleLibName;
        }

        {
            const String oldStyleLibName (getSettingString ("libraryName_Release"));
            settings.removeProperty ("libraryName_Release", nullptr);

            if (oldStyleLibName.isNotEmpty())
                for (ConfigIterator config (*this); config.next();)
                    if (! config->isDebug())
                        config->getTargetBinaryName() = oldStyleLibName;
        }
    }

    //==============================================================================
    class MSVCBuildConfiguration  : public BuildConfiguration
    {
    public:
        MSVCBuildConfiguration (Project& project, const ValueTree& settings)
            : BuildConfiguration (project, settings)
        {
            if (getWarningLevel() == 0)
                getWarningLevelValue() = 4;

            setValueIfVoid (shouldGenerateManifestValue(), true);
        }

        Value getWarningLevelValue()            { return getValue (Ids::winWarningLevel); }
        int getWarningLevel() const             { return config [Ids::winWarningLevel]; }

        Value getPrebuildCommand()                  { return getValue (Ids::prebuildCommand); }
        String getPrebuildCommandString() const     { return config [Ids::prebuildCommand]; }
        Value getPostbuildCommand()                 { return getValue (Ids::postbuildCommand); }
        String getPostbuildCommandString() const    { return config [Ids::postbuildCommand]; }

        Value shouldGenerateManifestValue()         { return getValue (Ids::generateManifest); }
        bool shouldGenerateManifest() const         { return config [Ids::generateManifest]; }

        Value getWholeProgramOptValue()             { return getValue (Ids::wholeProgramOptimisation); }
        bool shouldDisableWholeProgramOpt() const   { return static_cast<int> (config [Ids::wholeProgramOptimisation]) > 0; }

        Value getUsingRuntimeLibDLL()               { return getValue (Ids::useRuntimeLibDLL); }
        bool isUsingRuntimeLibDLL() const           { return config [Ids::useRuntimeLibDLL]; }

        String getOutputFilename (const String& suffix, bool forceSuffix) const
        {
            const String target (File::createLegalFileName (getTargetBinaryNameString().trim()));

            if (forceSuffix || ! target.containsChar ('.'))
                return target.upToLastOccurrenceOf (".", false, false) + suffix;

            return target;
        }

        void createConfigProperties (PropertyListBuilder& props)
        {
            const char* const warningLevelNames[] = { "Low", "Medium", "High", nullptr };
            const int warningLevels[] = { 2, 3, 4, 0 };

            props.add (new ChoicePropertyComponent (getWarningLevelValue(), "Warning Level",
                                                    StringArray (warningLevelNames), Array<var> (warningLevels)));

            {
                const char* const runtimeNames[] = { "(Default)", "Use static runtime", "Use DLL runtime", nullptr };
                const var runtimeValues[] = { var(), var (false), var (true) };

                props.add (new ChoicePropertyComponent (getUsingRuntimeLibDLL(), "Runtime Library",
                                                        StringArray (runtimeNames), Array<var> (runtimeValues, numElementsInArray (runtimeValues))));
            }

            {
                const char* const wpoNames[] = { "Enable link-time code generation when possible",
                                                 "Always disable link-time code generation", nullptr };
                const var wpoValues[] = { var(), var (1) };

                props.add (new ChoicePropertyComponent (getWholeProgramOptValue(), "Whole Program Optimisation",
                                                        StringArray (wpoNames), Array<var> (wpoValues, numElementsInArray (wpoValues))));
            }

            props.add (new TextPropertyComponent (getPrebuildCommand(),  "Pre-build Command",  2048, false));
            props.add (new TextPropertyComponent (getPostbuildCommand(), "Post-build Command", 2048, false));
            props.add (new BooleanPropertyComponent (shouldGenerateManifestValue(), "Manifest", "Generate Manifest"));
        }
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& settings) const
    {
        return new MSVCBuildConfiguration (project, settings);
    }

    static int getWarningLevel (const BuildConfiguration& config)
    {
        return dynamic_cast <const MSVCBuildConfiguration&> (config).getWarningLevel();
    }

    //==============================================================================
    String getIntermediatesPath (const BuildConfiguration& config) const
    {
        return prependDot (File::createLegalFileName (config.getName().trim()));
    }

    String getConfigTargetPath (const BuildConfiguration& config) const
    {
        const String binaryPath (config.getTargetBinaryRelativePathString().trim());
        if (binaryPath.isEmpty())
            return getIntermediatesPath (config);

        RelativePath binaryRelPath (binaryPath, RelativePath::projectFolder);

        if (binaryRelPath.isAbsolute())
            return binaryRelPath.toWindowsStyle();

        return prependDot (binaryRelPath.rebased (projectFolder, getTargetFolder(), RelativePath::buildTargetFolder)
                                        .toWindowsStyle());
    }

    String getPreprocessorDefs (const BuildConfiguration& config, const String& joinString) const
    {
        StringPairArray defines (msvcExtraPreprocessorDefs);
        defines.set ("WIN32", "");
        defines.set ("_WINDOWS", "");

        if (config.isDebug())
        {
            defines.set ("DEBUG", "");
            defines.set ("_DEBUG", "");
        }
        else
        {
            defines.set ("NDEBUG", "");
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

    StringArray getHeaderSearchPaths (const BuildConfiguration& config) const
    {
        StringArray searchPaths (extraSearchPaths);
        searchPaths.addArray (config.getHeaderSearchPaths());
        searchPaths.removeDuplicates (false);
        return searchPaths;
    }

    virtual String createConfigName (const BuildConfiguration& config) const
    {
        return config.getName() + "|Win32";
    }

    //==============================================================================
    void writeSolutionFile (OutputStream& out, const String& versionString, String commentString, const File& vcProject) const
    {
        if (commentString.isNotEmpty())
            commentString += newLine;

        out << "Microsoft Visual Studio Solution File, Format Version " << versionString << newLine
            << commentString
            << "Project(\"" << createGUID (projectName + "sln_guid") << "\") = \"" << projectName << "\", \""
            << vcProject.getFileName() << "\", \"" << projectGUID << '"' << newLine
            << "EndProject" << newLine
            << "Global" << newLine
            << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution" << newLine;

        for (ConstConfigIterator i (*this); i.next();)
        {
            const String configName (createConfigName (*i));
            out << "\t\t" << configName << " = " << configName << newLine;
        }

        out << "\tEndGlobalSection" << newLine
            << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution" << newLine;

        for (ConstConfigIterator i (*this); i.next();)
        {
            const String configName (createConfigName (*i));
            out << "\t\t" << projectGUID << "." << configName << ".ActiveCfg = " << configName << newLine;
            out << "\t\t" << projectGUID << "." << configName << ".Build.0 = " << configName << newLine;
        }

        out << "\tEndGlobalSection" << newLine
            << "\tGlobalSection(SolutionProperties) = preSolution" << newLine
            << "\t\tHideSolutionNode = FALSE" << newLine
            << "\tEndGlobalSection" << newLine
            << "EndGlobal" << newLine;
    }

    //==============================================================================
    static void writeBMPImage (const Image& image, const int w, const int h, MemoryOutputStream& out)
    {
        const int maskStride = (w / 8 + 3) & ~3;

        out.writeInt (40); // bitmapinfoheader size
        out.writeInt (w);
        out.writeInt (h * 2);
        out.writeShort (1); // planes
        out.writeShort (32); // bits
        out.writeInt (0); // compression
        out.writeInt ((h * w * 4) + (h * maskStride)); // size image
        out.writeInt (0); // x pixels per meter
        out.writeInt (0); // y pixels per meter
        out.writeInt (0); // clr used
        out.writeInt (0); // clr important

        const Image::BitmapData bitmap (image, Image::BitmapData::readOnly);
        const int alphaThreshold = 5;

        int y;
        for (y = h; --y >= 0;)
        {
            for (int x = 0; x < w; ++x)
            {
                const Colour pixel (bitmap.getPixelColour (x, y));

                if (pixel.getAlpha() <= alphaThreshold)
                {
                    out.writeInt (0);
                }
                else
                {
                    out.writeByte ((char) pixel.getBlue());
                    out.writeByte ((char) pixel.getGreen());
                    out.writeByte ((char) pixel.getRed());
                    out.writeByte ((char) pixel.getAlpha());
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
                    out.writeByte ((char) mask);
                    count = 0;
                    mask = 0;
                }
            }

            if (mask != 0)
                out.writeByte ((char) mask);

            for (int i = maskStride - w / 8; --i >= 0;)
                out.writeByte (0);
        }
    }

    static void writeIconFile (const Array<Image>& images, MemoryOutputStream& out)
    {
        out.writeShort (0); // reserved
        out.writeShort (1); // .ico tag
        out.writeShort ((short) images.size());

        MemoryOutputStream dataBlock;

        const int imageDirEntrySize = 16;
        const int dataBlockStart = 6 + images.size() * imageDirEntrySize;

        for (int i = 0; i < images.size(); ++i)
        {
            const size_t oldDataSize = dataBlock.getDataSize();

            const Image& image = images.getReference (i);
            const int w = image.getWidth();
            const int h = image.getHeight();

            if (w >= 256 || h >= 256)
            {
                PNGImageFormat pngFormat;
                pngFormat.writeImageToStream (image, dataBlock);
            }
            else
            {
                writeBMPImage (image, w, h, dataBlock);
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

    bool hasResourceFile() const
    {
        return ! projectType.isLibrary();
    }

    void createResourcesAndIcon() const
    {
        if (hasResourceFile())
        {
            Array<Image> images;
            const int sizes[] = { 16, 32, 48, 256 };

            for (int i = 0; i < numElementsInArray (sizes); ++i)
            {
                Image im (getBestIconForSize (sizes[i], true));
                if (im.isValid())
                    images.add (im);
            }

            if (images.size() > 0)
            {
                iconFile = getTargetFolder().getChildFile ("icon.ico");

                MemoryOutputStream mo;
                writeIconFile (images, mo);
                overwriteFileIfDifferentOrThrow (iconFile, mo);
            }

            createRCFile();
        }
    }

    void createRCFile() const
    {
        rcFile = getTargetFolder().getChildFile ("resources.rc");

        const String version (project.getVersionString());

        MemoryOutputStream mo;

        mo << "#ifdef JUCE_USER_DEFINED_RC_FILE" << newLine
           << " #include JUCE_USER_DEFINED_RC_FILE" << newLine
           << "#else" << newLine
           << newLine
           << "#undef  WIN32_LEAN_AND_MEAN" << newLine
           << "#define WIN32_LEAN_AND_MEAN" << newLine
           << "#include <windows.h>" << newLine
           << newLine
           << "VS_VERSION_INFO VERSIONINFO" << newLine
           << "FILEVERSION  " << getCommaSeparatedVersionNumber (version) << newLine
           << "BEGIN" << newLine
           << "  BLOCK \"StringFileInfo\"" << newLine
           << "  BEGIN" << newLine
           << "    BLOCK \"040904E4\"" << newLine
           << "    BEGIN" << newLine;

        writeRCValue (mo, "CompanyName", project.getCompanyName().toString());
        writeRCValue (mo, "FileDescription", project.getTitle());
        writeRCValue (mo, "FileVersion", version);
        writeRCValue (mo, "ProductName", project.getTitle());
        writeRCValue (mo, "ProductVersion", version);

        mo << "    END" << newLine
           << "  END" << newLine
           << newLine
           << "  BLOCK \"VarFileInfo\"" << newLine
           << "  BEGIN" << newLine
           << "    VALUE \"Translation\", 0x409, 65001" << newLine
           << "  END" << newLine
           << "END" << newLine
           << newLine
           << "#endif" << newLine;

        if (iconFile != File::nonexistent)
           mo << newLine
              << "IDI_ICON1 ICON DISCARDABLE " << iconFile.getFileName().quoted()
              << newLine
              << "IDI_ICON2 ICON DISCARDABLE " << iconFile.getFileName().quoted();

        overwriteFileIfDifferentOrThrow (rcFile, mo);
    }

    static void writeRCValue (MemoryOutputStream& mo, const String& name, const String& value)
    {
        if (value.isNotEmpty())
            mo << "      VALUE \"" << name << "\",  \""
               << CodeHelpers::addEscapeChars (value) << "\\0\"" << newLine;
    }

    static String getCommaSeparatedVersionNumber (const String& version)
    {
        StringArray versionParts;
        versionParts.addTokens (version, ",.", "");
        versionParts.trim();
        versionParts.removeEmptyStrings();
        while (versionParts.size() < 4)
            versionParts.add ("0");

        return versionParts.joinIntoString (",");
    }

    static String prependDot (const String& filename)
    {
        return FileHelpers::isAbsolutePath (filename) ? filename
                                                      : (".\\" + filename);
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

    static const char* getName()                    { return "Visual Studio 2008"; }
    static const char* getValueTreeTypeName()       { return "VS2008"; }
    int getVisualStudioVersion() const              { return 9; }

    static MSVCProjectExporterVC2008* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2008 (project, settings);

        return nullptr;
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const
    {
        createResourcesAndIcon();

        if (hasResourceFile())
        {
            for (int i = 0; i < getAllGroups().size(); ++i)
            {
                Project::Item& group = getAllGroups().getReference(i);

                if (group.getID() == ProjectSaver::getGeneratedGroupID())
                {
                    if (iconFile != File::nonexistent)
                    {
                        group.addFile (iconFile, -1, true);
                        group.findItemForFile (iconFile).getShouldAddToResourceValue() = false;
                    }

                    group.addFile (rcFile, -1, true);
                    group.findItemForFile (rcFile).getShouldAddToResourceValue() = false;

                    break;
                }
            }
        }

        {
            XmlElement projectXml ("VisualStudioProject");
            fillInProjectXml (projectXml);
            writeXmlOrThrow (projectXml, getVCProjFile(), "UTF-8", 10);
        }

        {
            MemoryOutputStream mo;
            writeSolutionFile (mo, getSolutionVersionString(), String::empty, getVCProjFile());

            overwriteFileIfDifferentOrThrow (getSLNFile(), mo);
        }
    }

protected:
    virtual String getProjectVersionString() const    { return "9.00"; }
    virtual String getSolutionVersionString() const   { return "10.00" + newLine + "# Visual C++ Express 2008"; }

    File getVCProjFile() const    { return getProjectFile (".vcproj"); }

    //==============================================================================
    void fillInProjectXml (XmlElement& projectXml) const
    {
        projectXml.setAttribute ("ProjectType", "Visual C++");
        projectXml.setAttribute ("Version", getProjectVersionString());
        projectXml.setAttribute ("Name", projectName);
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
    void addFile (const RelativePath& file, XmlElement& parent, const bool excludeFromBuild, const bool useStdcall) const
    {
        jassert (file.getRoot() == RelativePath::buildTargetFolder);

        XmlElement* fileXml = parent.createNewChildElement ("File");
        fileXml->setAttribute ("RelativePath", file.toWindowsStyle());

        if (excludeFromBuild || useStdcall)
        {
            for (ConstConfigIterator i (*this); i.next();)
            {
                XmlElement* fileConfig = fileXml->createNewChildElement ("FileConfiguration");
                fileConfig->setAttribute ("Name", createConfigName (*i));

                if (excludeFromBuild)
                    fileConfig->setAttribute ("ExcludedFromBuild", "true");

                XmlElement* tool = createToolElement (*fileConfig, "VCCLCompilerTool");

                if (useStdcall)
                    tool->setAttribute ("CallingConvention", "2");
            }
        }
    }

    XmlElement* createGroup (const String& groupName, XmlElement& parent) const
    {
        XmlElement* filter = parent.createNewChildElement ("Filter");
        filter->setAttribute ("Name", groupName);
        return filter;
    }

    void addFiles (const Project::Item& projectItem, XmlElement& parent) const
    {
        if (projectItem.isGroup())
        {
            XmlElement* filter = createGroup (projectItem.getName(), parent);

            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                addFiles (projectItem.getChild(i), *filter);
        }
        else if (projectItem.shouldBeAddedToTargetProject())
        {
            const RelativePath path (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);

            addFile (path, parent,
                     projectItem.shouldBeAddedToBinaryResources() || (shouldFileBeCompiledByDefault (path) && ! projectItem.shouldBeCompiled()),
                     shouldFileBeCompiledByDefault (path) && (bool) projectItem.shouldUseStdCall());
        }
    }

    void createFiles (XmlElement& files) const
    {
        for (int i = 0; i < getAllGroups().size(); ++i)
        {
            const Project::Item& group = getAllGroups().getReference(i);

            if (group.getNumChildren() > 0)
                addFiles (group, files);
        }
    }

    //==============================================================================
    XmlElement* createToolElement (XmlElement& parent, const String& toolName) const
    {
        XmlElement* const e = parent.createNewChildElement ("Tool");
        e->setAttribute ("Name", toolName);
        return e;
    }

    void createConfig (XmlElement& xml, const MSVCBuildConfiguration& config) const
    {
        const bool isDebug = config.isDebug();

        xml.setAttribute ("Name", createConfigName (config));
        xml.setAttribute ("OutputDirectory", FileHelpers::windowsStylePath (getConfigTargetPath (config)));
        xml.setAttribute ("IntermediateDirectory", FileHelpers::windowsStylePath (getIntermediatesPath (config)));
        xml.setAttribute ("ConfigurationType", isLibraryDLL() ? "2" : (projectType.isLibrary() ? "4" : "1"));
        xml.setAttribute ("UseOfMFC", "0");
        xml.setAttribute ("ATLMinimizesCRunTimeLibraryUsage", "false");
        xml.setAttribute ("CharacterSet", "2");

        if (! (isDebug || config.shouldDisableWholeProgramOpt()))
            xml.setAttribute ("WholeProgramOptimization", "1");

        XmlElement* preBuildEvent = createToolElement (xml, "VCPreBuildEventTool");

        if (config.getPrebuildCommandString().isNotEmpty())
        {
            preBuildEvent->setAttribute ("Description", "Pre-build");
            preBuildEvent->setAttribute ("CommandLine", config.getPrebuildCommandString());
        }

        createToolElement (xml, "VCCustomBuildTool");
        createToolElement (xml, "VCXMLDataGeneratorTool");
        createToolElement (xml, "VCWebServiceProxyGeneratorTool");

        if (! projectType.isLibrary())
        {
            XmlElement* midl = createToolElement (xml, "VCMIDLTool");
            midl->setAttribute ("PreprocessorDefinitions", isDebug ? "_DEBUG" : "NDEBUG");
            midl->setAttribute ("MkTypLibCompatible", "true");
            midl->setAttribute ("SuppressStartupBanner", "true");
            midl->setAttribute ("TargetEnvironment", "1");
            midl->setAttribute ("TypeLibraryName", getIntDirFile (config.getOutputFilename (".tlb", true)));
            midl->setAttribute ("HeaderFileName", "");
        }

        {
            XmlElement* compiler = createToolElement (xml, "VCCLCompilerTool");

            const int optimiseLevel = config.getOptimisationLevelInt();
            compiler->setAttribute ("Optimization", optimiseLevel <= 1 ? "0" : (optimiseLevel == 2 ? "1" : "2"));

            if (isDebug)
            {
                compiler->setAttribute ("BufferSecurityCheck", "");
                compiler->setAttribute ("DebugInformationFormat", projectType.isLibrary() ? "3" : "4");
            }
            else
            {
                compiler->setAttribute ("InlineFunctionExpansion", "1");
                compiler->setAttribute ("StringPooling", "true");
            }

            compiler->setAttribute ("AdditionalIncludeDirectories", replacePreprocessorTokens (config, getHeaderSearchPaths (config).joinIntoString (";")));
            compiler->setAttribute ("PreprocessorDefinitions", getPreprocessorDefs (config, ";"));
            compiler->setAttribute ("RuntimeLibrary", config.isUsingRuntimeLibDLL() ? (isDebug ? 3 : 2) // MT DLL
                                                                                    : (isDebug ? 1 : 0)); // MT static
            compiler->setAttribute ("RuntimeTypeInfo", "true");
            compiler->setAttribute ("UsePrecompiledHeader", "0");
            compiler->setAttribute ("PrecompiledHeaderFile", getIntDirFile (config.getOutputFilename (".pch", true)));
            compiler->setAttribute ("AssemblerListingLocation", "$(IntDir)\\");
            compiler->setAttribute ("ObjectFile", "$(IntDir)\\");
            compiler->setAttribute ("ProgramDataBaseFileName", "$(IntDir)\\");
            compiler->setAttribute ("WarningLevel", String (getWarningLevel (config)));
            compiler->setAttribute ("SuppressStartupBanner", "true");

            const String extraFlags (replacePreprocessorTokens (config, getExtraCompilerFlagsString()).trim());
            if (extraFlags.isNotEmpty())
                compiler->setAttribute ("AdditionalOptions", extraFlags);
        }

        createToolElement (xml, "VCManagedResourceCompilerTool");

        {
            XmlElement* resCompiler = createToolElement (xml, "VCResourceCompilerTool");
            resCompiler->setAttribute ("PreprocessorDefinitions", isDebug ? "_DEBUG" : "NDEBUG");
        }

        createToolElement (xml, "VCPreLinkEventTool");

        if (! projectType.isLibrary())
        {
            XmlElement* linker = createToolElement (xml, "VCLinkerTool");

            linker->setAttribute ("OutputFile", getOutDirFile (config.getOutputFilename (msvcTargetSuffix, false)));
            linker->setAttribute ("SuppressStartupBanner", "true");

            linker->setAttribute ("IgnoreDefaultLibraryNames", isDebug ? "libcmt.lib, msvcrt.lib" : "");
            linker->setAttribute ("GenerateDebugInformation", isDebug ? "true" : "false");
            linker->setAttribute ("ProgramDatabaseFile", getIntDirFile (config.getOutputFilename (".pdb", true)));
            linker->setAttribute ("SubSystem", msvcIsWindowsSubsystem ? "2" : "1");

            const StringArray librarySearchPaths (config.getLibrarySearchPaths());
            if (librarySearchPaths.size() > 0)
                linker->setAttribute ("AdditionalLibraryDirectories", librarySearchPaths.joinIntoString (";"));

            linker->setAttribute ("GenerateManifest", config.shouldGenerateManifest() ? "true" : "false");

            if (! isDebug)
            {
                linker->setAttribute ("OptimizeReferences", "2");
                linker->setAttribute ("EnableCOMDATFolding", "2");
            }

            linker->setAttribute ("TargetMachine", "1"); // (64-bit build = 5)

            if (msvcDelayLoadedDLLs.isNotEmpty())
                linker->setAttribute ("DelayLoadDLLs", msvcDelayLoadedDLLs);

            if (config.config [Ids::msvcModuleDefinitionFile].toString().isNotEmpty())
                linker->setAttribute ("ModuleDefinitionFile", config.config [Ids::msvcModuleDefinitionFile].toString());

            String externalLibraries (getExternalLibrariesString());
            if (externalLibraries.isNotEmpty())
                linker->setAttribute ("AdditionalDependencies", replacePreprocessorTokens (config, externalLibraries).trim());

            String extraLinkerOptions (getExtraLinkerFlagsString());
            if (extraLinkerOptions.isNotEmpty())
                linker->setAttribute ("AdditionalOptions", replacePreprocessorTokens (config, extraLinkerOptions).trim());
        }
        else
        {
            if (isLibraryDLL())
            {
                XmlElement* linker = createToolElement (xml, "VCLinkerTool");

                String extraLinkerOptions (getExtraLinkerFlagsString());
                extraLinkerOptions << " /IMPLIB:" << getOutDirFile (config.getOutputFilename (".lib", true));
                linker->setAttribute ("AdditionalOptions", replacePreprocessorTokens (config, extraLinkerOptions).trim());

                String externalLibraries (getExternalLibrariesString());
                if (externalLibraries.isNotEmpty())
                    linker->setAttribute ("AdditionalDependencies", replacePreprocessorTokens (config, externalLibraries).trim());

                linker->setAttribute ("OutputFile", getOutDirFile (config.getOutputFilename (msvcTargetSuffix, false)));
                linker->setAttribute ("IgnoreDefaultLibraryNames", isDebug ? "libcmt.lib, msvcrt.lib" : "");
            }
            else
            {
                XmlElement* librarian = createToolElement (xml, "VCLibrarianTool");

                librarian->setAttribute ("OutputFile", getOutDirFile (config.getOutputFilename (msvcTargetSuffix, false)));
                librarian->setAttribute ("IgnoreDefaultLibraryNames", isDebug ? "libcmt.lib, msvcrt.lib" : "");
            }
        }

        createToolElement (xml, "VCALinkTool");
        createToolElement (xml, "VCManifestTool");
        createToolElement (xml, "VCXDCMakeTool");

        {
            XmlElement* bscMake = createToolElement (xml, "VCBscMakeTool");
            bscMake->setAttribute ("SuppressStartupBanner", "true");
            bscMake->setAttribute ("OutputFile", getIntDirFile (config.getOutputFilename (".bsc", true)));
        }

        createToolElement (xml, "VCFxCopTool");

        if (! projectType.isLibrary())
            createToolElement (xml, "VCAppVerifierTool");

        XmlElement* postBuildEvent = createToolElement (xml, "VCPostBuildEventTool");

        if (config.getPostbuildCommandString().isNotEmpty())
        {
            postBuildEvent->setAttribute ("Description", "Post-build");
            postBuildEvent->setAttribute ("CommandLine", config.getPostbuildCommandString());
        }
    }

    void createConfigs (XmlElement& xml) const
    {
        for (ConstConfigIterator config (*this); config.next();)
            createConfig (*xml.createNewChildElement ("Configuration"),
                          dynamic_cast <const MSVCBuildConfiguration&> (*config));
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2008);
};


//==============================================================================
class MSVCProjectExporterVC2005   : public MSVCProjectExporterVC2008
{
public:
    MSVCProjectExporterVC2005 (Project& p, const ValueTree& t)
        : MSVCProjectExporterVC2008 (p, t, "VisualStudio2005")
    {
        name = getName();
    }

    static const char* getName()                    { return "Visual Studio 2005"; }
    static const char* getValueTreeTypeName()       { return "VS2005"; }
    int getVisualStudioVersion() const              { return 8; }

    static MSVCProjectExporterVC2005* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2005 (project, settings);

        return nullptr;
    }

protected:
    String getProjectVersionString() const    { return "8.00"; }
    String getSolutionVersionString() const   { return "9.00" + newLine + "# Visual C++ Express 2005"; }

    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2005);
};

//==============================================================================
class MSVCProjectExporterVC2010   : public MSVCProjectExporterBase
{
public:
    MSVCProjectExporterVC2010 (Project& p, const ValueTree& t)
        : MSVCProjectExporterBase (p, t, "VisualStudio2010")
    {
        name = getName();
    }

    static const char* getName()                    { return "Visual Studio 2010"; }
    static const char* getValueTreeTypeName()       { return "VS2010"; }
    int getVisualStudioVersion() const              { return 10; }

    static MSVCProjectExporterVC2010* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2010 (project, settings);

        return nullptr;
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const
    {
        createResourcesAndIcon();

        {
            XmlElement projectXml ("Project");
            fillInProjectXml (projectXml);
            writeXmlOrThrow (projectXml, getVCProjFile(), "utf-8", 100);
        }

        {
            XmlElement filtersXml ("Project");
            fillInFiltersXml (filtersXml);
            writeXmlOrThrow (filtersXml, getVCProjFiltersFile(), "utf-8", 100);
        }

        {
            MemoryOutputStream mo;
            writeSolutionFile (mo, "11.00", "# Visual Studio 2010", getVCProjFile());

            overwriteFileIfDifferentOrThrow (getSLNFile(), mo);
        }
    }

protected:
    //==============================================================================
    class VC2010BuildConfiguration  : public MSVCBuildConfiguration
    {
    public:
        VC2010BuildConfiguration (Project& project, const ValueTree& settings)
            : MSVCBuildConfiguration (project, settings)
        {
            if (getArchitectureType().toString().isEmpty())
                getArchitectureType() = get32BitArchName();
        }

        //==============================================================================
        static const char* get32BitArchName()   { return "32-bit"; }
        static const char* get64BitArchName()   { return "x64"; }

        Value getArchitectureType()             { return getValue (Ids::winArchitecture); }
        bool is64Bit() const                    { return config [Ids::winArchitecture].toString() == get64BitArchName(); }

        //==============================================================================
        void createConfigProperties (PropertyListBuilder& props)
        {
            MSVCBuildConfiguration::createConfigProperties (props);

            const char* const archTypes[] = { get32BitArchName(), get64BitArchName(), nullptr };
            props.add (new ChoicePropertyComponent (getArchitectureType(), "Architecture",
                                                    StringArray (archTypes), Array<var> (archTypes)));
        }
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& settings) const
    {
        return new VC2010BuildConfiguration (project, settings);
    }

    static bool is64Bit (const BuildConfiguration& config)
    {
        return dynamic_cast <const VC2010BuildConfiguration&> (config).is64Bit();
    }

    //==============================================================================
    File getVCProjFile() const            { return getProjectFile (".vcxproj"); }
    File getVCProjFiltersFile() const     { return getProjectFile (".vcxproj.filters"); }

    String createConfigName (const BuildConfiguration& config) const
    {
        return config.getName() + (is64Bit (config) ? "|x64"
                                                    : "|Win32");
    }

    void setConditionAttribute (XmlElement& xml, const BuildConfiguration& config) const
    {
        xml.setAttribute ("Condition", "'$(Configuration)|$(Platform)'=='" + createConfigName (config) + "'");
    }

    //==============================================================================
    void fillInProjectXml (XmlElement& projectXml) const
    {
        projectXml.setAttribute ("DefaultTargets", "Build");
        projectXml.setAttribute ("ToolsVersion", "4.0");
        projectXml.setAttribute ("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

        {
            XmlElement* configsGroup = projectXml.createNewChildElement ("ItemGroup");
            configsGroup->setAttribute ("Label", "ProjectConfigurations");

            for (ConstConfigIterator config (*this); config.next();)
            {
                XmlElement* e = configsGroup->createNewChildElement ("ProjectConfiguration");
                e->setAttribute ("Include", createConfigName (*config));
                e->createNewChildElement ("Configuration")->addTextElement (config->getName());
                e->createNewChildElement ("Platform")->addTextElement (is64Bit (*config) ? "x64" : "Win32");
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

        for (ConstConfigIterator i (*this); i.next();)
        {
            const MSVCBuildConfiguration& config = dynamic_cast <const MSVCBuildConfiguration&> (*i);

            XmlElement* e = projectXml.createNewChildElement ("PropertyGroup");
            setConditionAttribute (*e, config);
            e->setAttribute ("Label", "Configuration");
            e->createNewChildElement ("ConfigurationType")->addTextElement (getProjectType());
            e->createNewChildElement ("UseOfMfc")->addTextElement ("false");
            e->createNewChildElement ("CharacterSet")->addTextElement ("MultiByte");

            if (! (config.isDebug() || config.shouldDisableWholeProgramOpt()))
                e->createNewChildElement ("WholeProgramOptimization")->addTextElement ("true");

            if (is64Bit (config))
                e->createNewChildElement ("PlatformToolset")->addTextElement ("Windows7.1SDK");
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

            for (ConstConfigIterator i (*this); i.next();)
            {
                const MSVCBuildConfiguration& config = dynamic_cast <const MSVCBuildConfiguration&> (*i);

                {
                    XmlElement* outdir = props->createNewChildElement ("OutDir");
                    setConditionAttribute (*outdir, config);
                    outdir->addTextElement (getConfigTargetPath (config) + "\\");
                }

                {
                    XmlElement* intdir = props->createNewChildElement ("IntDir");
                    setConditionAttribute (*intdir, config);
                    intdir->addTextElement (getIntermediatesPath (config) + "\\");
                }

                {
                    XmlElement* name = props->createNewChildElement ("TargetName");
                    setConditionAttribute (*name, config);
                    name->addTextElement (config.getOutputFilename (String::empty, true));
                }

                {
                    XmlElement* manifest = props->createNewChildElement ("GenerateManifest");
                    setConditionAttribute (*manifest, config);
                    manifest->addTextElement (config.shouldGenerateManifest() ? "true" : "false");
                }

                const StringArray librarySearchPaths (config.getLibrarySearchPaths());

                if (librarySearchPaths.size() > 0)
                {
                    XmlElement* libPath = props->createNewChildElement ("LibraryPath");
                    setConditionAttribute (*libPath, config);
                    libPath->addTextElement ("$(LibraryPath);" + librarySearchPaths.joinIntoString (";"));
                }
            }
        }

        for (ConstConfigIterator i (*this); i.next();)
        {
            const MSVCBuildConfiguration& config = dynamic_cast <const MSVCBuildConfiguration&> (*i);

            const bool isDebug = config.isDebug();

            XmlElement* group = projectXml.createNewChildElement ("ItemDefinitionGroup");
            setConditionAttribute (*group, config);

            {
                XmlElement* midl = group->createNewChildElement ("Midl");
                midl->createNewChildElement ("PreprocessorDefinitions")->addTextElement (isDebug ? "_DEBUG;%(PreprocessorDefinitions)"
                                                                                                 : "NDEBUG;%(PreprocessorDefinitions)");
                midl->createNewChildElement ("MkTypLibCompatible")->addTextElement ("true");
                midl->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
                midl->createNewChildElement ("TargetEnvironment")->addTextElement ("Win32");
                midl->createNewChildElement ("HeaderFileName");
            }

            {
                XmlElement* cl = group->createNewChildElement ("ClCompile");

                const int optimiseLevel = config.getOptimisationLevelInt();
                cl->createNewChildElement ("Optimization")->addTextElement (optimiseLevel <= 1 ? "Disabled"
                                                                                               : optimiseLevel == 2 ? "MinSpace"
                                                                                                                    : "MaxSpeed");

                if (isDebug && optimiseLevel <= 1)
                    cl->createNewChildElement ("DebugInformationFormat")->addTextElement (is64Bit (config) ? "ProgramDatabase"
                                                                                                           : "EditAndContinue");

                StringArray includePaths (getHeaderSearchPaths (config));
                includePaths.add ("%(AdditionalIncludeDirectories)");
                cl->createNewChildElement ("AdditionalIncludeDirectories")->addTextElement (includePaths.joinIntoString (";"));
                cl->createNewChildElement ("PreprocessorDefinitions")->addTextElement (getPreprocessorDefs (config, ";") + ";%(PreprocessorDefinitions)");
                cl->createNewChildElement ("RuntimeLibrary")->addTextElement (config.isUsingRuntimeLibDLL() ? (isDebug ? "MultiThreadedDebugDLL" : "MultiThreadedDLL")
                                                                                                            : (isDebug ? "MultiThreadedDebug"    : "MultiThreaded"));
                cl->createNewChildElement ("RuntimeTypeInfo")->addTextElement ("true");
                cl->createNewChildElement ("PrecompiledHeader");
                cl->createNewChildElement ("AssemblerListingLocation")->addTextElement ("$(IntDir)\\");
                cl->createNewChildElement ("ObjectFileName")->addTextElement ("$(IntDir)\\");
                cl->createNewChildElement ("ProgramDataBaseFileName")->addTextElement ("$(IntDir)\\");
                cl->createNewChildElement ("WarningLevel")->addTextElement ("Level" + String (getWarningLevel (config)));
                cl->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
                cl->createNewChildElement ("MultiProcessorCompilation")->addTextElement ("true");

                const String extraFlags (replacePreprocessorTokens (config, getExtraCompilerFlagsString()).trim());
                if (extraFlags.isNotEmpty())
                    cl->createNewChildElement ("AdditionalOptions")->addTextElement (extraFlags + " %(AdditionalOptions)");
            }

            {
                XmlElement* res = group->createNewChildElement ("ResourceCompile");
                res->createNewChildElement ("PreprocessorDefinitions")->addTextElement (isDebug ? "_DEBUG;%(PreprocessorDefinitions)"
                                                                                                : "NDEBUG;%(PreprocessorDefinitions)");
            }

            {
                XmlElement* link = group->createNewChildElement ("Link");
                link->createNewChildElement ("OutputFile")->addTextElement (getOutDirFile (config.getOutputFilename (msvcTargetSuffix, false)));
                link->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
                link->createNewChildElement ("IgnoreSpecificDefaultLibraries")->addTextElement (isDebug ? "libcmt.lib; msvcrt.lib;;%(IgnoreSpecificDefaultLibraries)"
                                                                                                        : "%(IgnoreSpecificDefaultLibraries)");
                link->createNewChildElement ("GenerateDebugInformation")->addTextElement (isDebug ? "true" : "false");
                link->createNewChildElement ("ProgramDatabaseFile")->addTextElement (getIntDirFile (config.getOutputFilename (".pdb", true)));
                link->createNewChildElement ("SubSystem")->addTextElement (msvcIsWindowsSubsystem ? "Windows" : "Console");

                if (! is64Bit (config))
                    link->createNewChildElement ("TargetMachine")->addTextElement ("MachineX86");

                if (! isDebug)
                {
                    link->createNewChildElement ("OptimizeReferences")->addTextElement ("true");
                    link->createNewChildElement ("EnableCOMDATFolding")->addTextElement ("true");
                }

                String externalLibraries (getExternalLibrariesString());
                if (externalLibraries.isNotEmpty())
                    link->createNewChildElement ("AdditionalDependencies")->addTextElement (replacePreprocessorTokens (config, externalLibraries).trim()
                                                                                              + ";%(AdditionalDependencies)");

                String extraLinkerOptions (getExtraLinkerFlagsString());
                if (extraLinkerOptions.isNotEmpty())
                    link->createNewChildElement ("AdditionalOptions")->addTextElement (replacePreprocessorTokens (config, extraLinkerOptions).trim()
                                                                                         + " %(AdditionalOptions)");

                if (msvcDelayLoadedDLLs.isNotEmpty())
                    link->createNewChildElement ("DelayLoadDLLs")->addTextElement (msvcDelayLoadedDLLs);

                if (config.config [Ids::msvcModuleDefinitionFile].toString().isNotEmpty())
                    link->createNewChildElement ("ModuleDefinitionFile")
                        ->addTextElement (config.config [Ids::msvcModuleDefinitionFile].toString());
            }

            {
                XmlElement* bsc = group->createNewChildElement ("Bscmake");
                bsc->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
                bsc->createNewChildElement ("OutputFile")->addTextElement (getIntDirFile (config.getOutputFilename (".bsc", true)));
            }

            if (config.getPrebuildCommandString().isNotEmpty())
                group->createNewChildElement ("PreBuildEvent")
                     ->createNewChildElement ("Command")
                     ->addTextElement (config.getPrebuildCommandString());

            if (config.getPostbuildCommandString().isNotEmpty())
                group->createNewChildElement ("PostBuildEvent")
                     ->createNewChildElement ("Command")
                     ->addTextElement (config.getPostbuildCommandString());
        }

        ScopedPointer<XmlElement> otherFilesGroup (new XmlElement ("ItemGroup"));

        {
            XmlElement* cppFiles    = projectXml.createNewChildElement ("ItemGroup");
            XmlElement* headerFiles = projectXml.createNewChildElement ("ItemGroup");

            for (int i = 0; i < getAllGroups().size(); ++i)
            {
                const Project::Item& group = getAllGroups().getReference(i);

                if (group.getNumChildren() > 0)
                    addFilesToCompile (group, *cppFiles, *headerFiles, *otherFilesGroup);
            }
        }

        if (iconFile != File::nonexistent)
        {
            XmlElement* e = otherFilesGroup->createNewChildElement ("None");
            e->setAttribute ("Include", prependDot (iconFile.getFileName()));
        }

        if (otherFilesGroup->getFirstChildElement() != nullptr)
            projectXml.addChildElement (otherFilesGroup.release());

        if (hasResourceFile())
        {
            XmlElement* rcGroup = projectXml.createNewChildElement ("ItemGroup");
            XmlElement* e = rcGroup->createNewChildElement ("ResourceCompile");
            e->setAttribute ("Include", prependDot (rcFile.getFileName()));
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

    String getProjectType() const
    {
        if (projectType.isGUIApplication() || projectType.isCommandLineApp())   return "Application";
        else if (isLibraryDLL())                                                return "DynamicLibrary";
        else if (projectType.isLibrary())                                       return "StaticLibrary";

        jassertfalse;
        return String::empty;
    }

    //==============================================================================
    void addFilesToCompile (const Project::Item& projectItem, XmlElement& cpps, XmlElement& headers, XmlElement& otherFiles) const
    {
        if (projectItem.isGroup())
        {
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                addFilesToCompile (projectItem.getChild(i), cpps, headers, otherFiles);
        }
        else if (projectItem.shouldBeAddedToTargetProject())
        {
            const RelativePath path (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);

            jassert (path.getRoot() == RelativePath::buildTargetFolder);

            if (path.hasFileExtension ("cpp;cc;cxx;c"))
            {
                XmlElement* e = cpps.createNewChildElement ("ClCompile");
                e->setAttribute ("Include", path.toWindowsStyle());

                if (! projectItem.shouldBeCompiled())
                    e->createNewChildElement ("ExcludedFromBuild")->addTextElement ("true");

                if (projectItem.shouldUseStdCall())
                    e->createNewChildElement ("CallingConvention")->addTextElement ("StdCall");
            }
            else if (path.hasFileExtension (headerFileExtensions))
            {
                headers.createNewChildElement ("ClInclude")->setAttribute ("Include", path.toWindowsStyle());
            }
            else if (! path.hasFileExtension ("mm;m"))
            {
                otherFiles.createNewChildElement ("None")->setAttribute ("Include", path.toWindowsStyle());
            }
        }
    }

    //==============================================================================
    void addFilterGroup (XmlElement& groups, const String& path) const
    {
        XmlElement* e = groups.createNewChildElement ("Filter");
        e->setAttribute ("Include", path);
        e->createNewChildElement ("UniqueIdentifier")->addTextElement (createGUID (path + "_guidpathsaltxhsdf"));
    }

    void addFileToFilter (const RelativePath& file, const String& groupPath,
                          XmlElement& cpps, XmlElement& headers, XmlElement& otherFiles) const
    {
        XmlElement* e;

        if (file.hasFileExtension (headerFileExtensions))
            e = headers.createNewChildElement ("ClInclude");
        else if (file.hasFileExtension (sourceFileExtensions))
            e = cpps.createNewChildElement ("ClCompile");
        else
            e = otherFiles.createNewChildElement ("None");

        jassert (file.getRoot() == RelativePath::buildTargetFolder);
        e->setAttribute ("Include", file.toWindowsStyle());
        e->createNewChildElement ("Filter")->addTextElement (groupPath);
    }

    void addFilesToFilter (const Project::Item& projectItem, const String& path,
                           XmlElement& cpps, XmlElement& headers, XmlElement& otherFiles, XmlElement& groups) const
    {
        if (projectItem.isGroup())
        {
            addFilterGroup (groups, path);

            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                addFilesToFilter (projectItem.getChild(i),
                                  (path.isEmpty() ? String::empty : (path + "\\")) + projectItem.getChild(i).getName(),
                                  cpps, headers, otherFiles, groups);
        }
        else if (projectItem.shouldBeAddedToTargetProject())
        {
            addFileToFilter (RelativePath (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder),
                             path.upToLastOccurrenceOf ("\\", false, false), cpps, headers, otherFiles);
        }
    }

    void addFilesToFilter (const Array<RelativePath>& files, const String& path,
                           XmlElement& cpps, XmlElement& headers, XmlElement& otherFiles, XmlElement& groups)
    {
        if (files.size() > 0)
        {
            addFilterGroup (groups, path);

            for (int i = 0; i < files.size(); ++i)
                addFileToFilter (files.getReference(i), path, cpps, headers, otherFiles);
        }
    }

    void fillInFiltersXml (XmlElement& filterXml) const
    {
        filterXml.setAttribute ("ToolsVersion", "4.0");
        filterXml.setAttribute ("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

        XmlElement* groupsXml  = filterXml.createNewChildElement ("ItemGroup");
        XmlElement* cpps       = filterXml.createNewChildElement ("ItemGroup");
        XmlElement* headers    = filterXml.createNewChildElement ("ItemGroup");
        ScopedPointer<XmlElement> otherFilesGroup (new XmlElement ("ItemGroup"));

        for (int i = 0; i < getAllGroups().size(); ++i)
        {
            const Project::Item& group = getAllGroups().getReference(i);

            if (group.getNumChildren() > 0)
                addFilesToFilter (group, group.getName(), *cpps, *headers, *otherFilesGroup, *groupsXml);
        }

        if (iconFile.exists())
        {
            XmlElement* e = otherFilesGroup->createNewChildElement ("None");
            e->setAttribute ("Include", prependDot (iconFile.getFileName()));
            e->createNewChildElement ("Filter")->addTextElement (ProjectSaver::getJuceCodeGroupName());
        }

        if (otherFilesGroup->getFirstChildElement() != nullptr)
            filterXml.addChildElement (otherFilesGroup.release());

        if (hasResourceFile())
        {
            XmlElement* rcGroup = filterXml.createNewChildElement ("ItemGroup");
            XmlElement* e = rcGroup->createNewChildElement ("ResourceCompile");
            e->setAttribute ("Include", prependDot (rcFile.getFileName()));
            e->createNewChildElement ("Filter")->addTextElement (ProjectSaver::getJuceCodeGroupName());
        }
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2010);
};


#endif   // __JUCER_PROJECTEXPORT_MSVC_JUCEHEADER__
