/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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
class CompileEngineSettings
{
public:
    CompileEngineSettings (ValueTree& projectRoot)
        : tree (projectRoot.getOrCreateChildWithName ("LIVE_SETTINGS", nullptr)
                           .getOrCreateChildWithName (getLiveSettingsSubType(), nullptr)),
          buildEnabledValue             (tree, Ids::buildEnabled,                 nullptr, false),
          continuousRebuildEnabledValue (tree, Ids::continuousRebuildEnabled,     nullptr, false),
          warningsEnabledValue          (tree, Ids::warningsEnabled,              nullptr, true),
          userHeaderPathValue           (tree, Ids::headerPath,                   nullptr),
          systemHeaderPathValue         (tree, Ids::systemHeaderPath,             nullptr),
          extraDLLsValue                (tree, Ids::extraDLLs,                    nullptr),
          extraCompilerFlagsValue       (tree, Ids::extraCompilerFlags,           nullptr),
          extraPreprocessorDefsValue    (tree, Ids::defines,                      nullptr),
          windowsTargetPlatformValue    (tree, Ids::windowsTargetPlatformVersion, nullptr, "10.0.16299.0")
    {
    }

    //==============================================================================
    void setBuildEnabled (bool enabled) noexcept                   { buildEnabledValue = enabled; }
    void setContinuousRebuildEnabled (bool enabled) noexcept       { continuousRebuildEnabledValue = enabled; }
    void setWarningsEnabled (bool enabled)                         { warningsEnabledValue = enabled; }

    bool isBuildEnabled() const noexcept                           { return buildEnabledValue.get(); }
    bool isContinuousRebuildEnabled() const noexcept               { return continuousRebuildEnabledValue.get(); }
    bool areWarningsEnabled() const noexcept                       { return warningsEnabledValue.get(); }

    String getUserHeaderPathString() const noexcept                { return userHeaderPathValue.get(); }
    String getSystemHeaderPathString() const noexcept              { return systemHeaderPathValue.get(); }
    String getExtraDLLsString() const noexcept                     { return extraDLLsValue.get(); }
    String getExtraCompilerFlagsString() const noexcept            { return extraCompilerFlagsValue.get(); }
    String getExtraPreprocessorDefsString() const noexcept         { return extraPreprocessorDefsValue.get(); }
    String getWindowsTargetPlatformVersionString() const noexcept  { return windowsTargetPlatformValue.get(); }

    //==============================================================================
    void getLiveSettings (PropertyListBuilder& props)
    {
        props.addSearchPathProperty (userHeaderPathValue, "User Header Paths", "User header search paths.");
        props.addSearchPathProperty (systemHeaderPathValue, "System Header Paths", "System header search paths.");

        props.add (new TextPropertyComponent (extraPreprocessorDefsValue, "Preprocessor Definitions", 32768, true),
                   "Extra preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace or commas "
                   "to separate the items - to include a space or comma in a definition, precede it with a backslash.");

        props.add (new TextPropertyComponent (extraCompilerFlagsValue, "Extra Compiler Flags", 2048, true),
                   "Extra command-line flags to be passed to the compiler. This string can contain references to preprocessor"
                   " definitions in the form ${NAME_OF_DEFINITION}, which will be replaced with their values.");

        props.add (new TextPropertyComponent (extraDLLsValue, "Extra Dynamic Libraries", 2048, true),
                   "Extra dynamic libs that the running code may require. Use new-lines or commas to separate the items.");

        props.add (new TextPropertyComponent (windowsTargetPlatformValue, "Windows Target Platform", 256, false),
                   "The Windows target platform to use.");
    }

private:
    ValueTree tree;

    ValueWithDefault buildEnabledValue, continuousRebuildEnabledValue, warningsEnabledValue, userHeaderPathValue, systemHeaderPathValue,
                     extraDLLsValue, extraCompilerFlagsValue, extraPreprocessorDefsValue, windowsTargetPlatformValue;

    //==============================================================================
    String getLiveSettingsSubType() const noexcept
    {
       #if JUCE_MAC
        return "OSX";
       #elif JUCE_WINDOWS
        return "WINDOWS";
       #elif JUCE_LINUX || JUCE_BSD
        return "LINUX";
       #else
        // unknown platform?!
        jassertfalse;
        return {};
       #endif
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompileEngineSettings)
};
