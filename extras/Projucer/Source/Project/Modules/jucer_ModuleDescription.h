/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
