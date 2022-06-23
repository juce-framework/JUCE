/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class ModuleDescription
{
public:
    ModuleDescription() = default;

    ModuleDescription (const File& folder)
       : moduleFolder (folder),
         moduleInfo (parseJUCEHeaderMetadata (getHeader()))
    {
    }

    bool isValid() const                    { return getID().isNotEmpty(); }

    String getID() const                    { return moduleInfo [Ids::ID_uppercase].toString(); }
    String getVendor() const                { return moduleInfo [Ids::vendor].toString(); }
    String getVersion() const               { return moduleInfo [Ids::version].toString(); }
    String getName() const                  { return moduleInfo [Ids::name].toString(); }
    String getDescription() const           { return moduleInfo [Ids::description].toString(); }
    String getLicense() const               { return moduleInfo [Ids::license].toString(); }
    String getMinimumCppStandard() const    { return moduleInfo [Ids::minimumCppStandard].toString(); }
    String getPreprocessorDefs() const      { return moduleInfo [Ids::defines].toString(); }
    String getExtraSearchPaths() const      { return moduleInfo [Ids::searchpaths].toString(); }
    var getModuleInfo() const               { return moduleInfo; }
    File getModuleFolder() const            { return moduleFolder; }

    File getFolder() const
    {
        jassert (moduleFolder != File());

        return moduleFolder;
    }

    File getHeader() const
    {
        if (moduleFolder != File())
        {
            static const char* extensions[] = { ".h", ".hpp", ".hxx" };

            for (auto e : extensions)
            {
                auto header = moduleFolder.getChildFile (moduleFolder.getFileName() + e);

                if (header.existsAsFile())
                    return header;
            }
        }

        return {};
    }

    StringArray getDependencies() const
    {
        auto moduleDependencies = StringArray::fromTokens (moduleInfo ["dependencies"].toString(), " \t;,", "\"'");
        moduleDependencies.trim();
        moduleDependencies.removeEmptyStrings();

        return moduleDependencies;
    }

private:
    File moduleFolder;
    var moduleInfo;
    URL url;
};
