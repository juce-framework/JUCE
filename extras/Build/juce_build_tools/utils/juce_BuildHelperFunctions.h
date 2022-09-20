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

namespace juce
{
namespace build_tools
{
    void overwriteFileIfDifferentOrThrow (const File& file, const MemoryOutputStream& newData);
    void overwriteFileIfDifferentOrThrow (const File& file, const String& newData);

    class SaveError
    {
    public:
        SaveError (const String& error) : message (error)
        {}

        SaveError (const File& fileThatFailedToWrite)
            : message ("Can't write to the file: " + fileThatFailedToWrite.getFullPathName())
        {}

        String message;
    };

    String replacePreprocessorDefs (const StringPairArray& definitions, String sourceString);

    String getXcodePackageType (ProjectType::Target::Type);
    String getXcodeBundleSignature (ProjectType::Target::Type);

    inline String hexString8Digits (int value)
    {
      return String::toHexString (value).paddedLeft ('0', 8);
    }

    String makeValidIdentifier (String s,
                                bool makeCamelCase,
                                bool removeColons,
                                bool allowTemplates,
                                bool allowAsterisks = false);

    String makeBinaryDataIdentifierName (const File& file);

    void writeDataAsCppLiteral (const MemoryBlock& mb,
                                OutputStream& out,
                                bool breakAtNewLines,
                                bool allowStringBreaks);

    void createStringMatcher (OutputStream& out,
                              const String& utf8PointerVariable,
                              const StringArray& strings,
                              const StringArray& codeToExecute,
                              const int indentLevel);

    String unixStylePath (const String& path);
    String windowsStylePath (const String& path);
    String currentOSStylePath (const String& path);

    bool isAbsolutePath (const String& path);

    // A windows-aware version of File::getRelativePath()
    String getRelativePathFrom (const File& file, const File& sourceFolder);

    void writeStreamToFile (const File& file, const std::function<void (MemoryOutputStream&)>& writer);
}
}
