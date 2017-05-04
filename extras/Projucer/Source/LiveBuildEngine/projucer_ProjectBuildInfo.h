/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

struct ProjectBuildInfo
{
    ProjectBuildInfo()                      : tree (MessageTypes::BUILDINFO)  {}
    ProjectBuildInfo (const ValueTree& t)   : tree (t)  {}

    Array<File> getCompileUnits() const
    {
        Array<File> files;

        for (int i = 0; i < tree.getNumChildren(); ++i)
            if (tree.getChild(i).hasType (MessageTypes::COMPILEUNIT))
                files.add (File (tree.getChild(i) [Ids::file].toString()));

        return files;
    }

    // This is a list of all cpp and header files that are actually "user" code
    // rather than system or internal files
    Array<File> getUserFiles() const
    {
        Array<File> files;

        for (int i = 0; i < tree.getNumChildren(); ++i)
            if (tree.getChild(i).hasType (MessageTypes::USERFILE))
                files.add (File (tree.getChild(i) [Ids::file].toString()));

        return files;
    }

    void setFiles (const Array<File>& compileUnits, const Array<File>& allUserFiles)
    {
        for (const File& f : compileUnits)
        {
            ValueTree file (MessageTypes::COMPILEUNIT);
            file.setProperty (Ids::file, f.getFullPathName(), nullptr);
            tree.addChild (file, -1, nullptr);
        }

        for (const File& f : allUserFiles)
        {
            ValueTree file (MessageTypes::USERFILE);
            file.setProperty (Ids::file, f.getFullPathName(), nullptr);
            tree.addChild (file, -1, nullptr);
        }
    }

    StringArray getSystemIncludes() const           { return separateJoinedStrings (tree [Ids::systempath]); }
    StringArray getUserIncludes() const             { return separateJoinedStrings (tree [Ids::userpath]); }

    void setSystemIncludes (const StringArray& s)   { tree.setProperty (Ids::systempath, concatenateListOfStrings (s), nullptr); }
    void setUserIncludes (const StringArray& s)     { tree.setProperty (Ids::userpath,   concatenateListOfStrings (s), nullptr); }

    String getGlobalDefs() const                    { return tree [Ids::defines]; }
    void setGlobalDefs (const String& defs)         { tree.setProperty (Ids::defines, defs, nullptr); }

    String getCompileFlags() const                  { return tree [Ids::extraCompilerFlags]; }
    void setCompileFlags (const String& f)          { tree.setProperty (Ids::extraCompilerFlags, f, nullptr); }

    String getUtilsCppInclude() const               { return tree [Ids::utilsCppInclude]; }
    void setUtilsCppInclude (const String& s)       { tree.setProperty (Ids::utilsCppInclude, s, nullptr); }

    String getJuceModulesFolder() const             { return tree [Ids::juceModulesFolder]; }
    void setJuceModulesFolder (const String& s)     { tree.setProperty (Ids::juceModulesFolder, s, nullptr); }

    StringArray getExtraDLLs() const                { return separateJoinedStrings (tree [Ids::extraDLLs]); }
    void setExtraDLLs (const StringArray& s)        { tree.setProperty (Ids::extraDLLs, concatenateListOfStrings (s), nullptr); }

    String getWindowsTargetPlatformVersion() const            { return tree [Ids::liveWindowsTargetPlatformVersion]; }
    void setWindowsTargetPlatformVersion (const String& s)    { tree.setProperty (Ids::liveWindowsTargetPlatformVersion, s, nullptr); }

    ValueTree tree;
};
