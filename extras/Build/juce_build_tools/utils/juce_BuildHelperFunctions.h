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

namespace juce::build_tools
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
                              int indentLevel);

    String unixStylePath (const String& path);
    String windowsStylePath (const String& path);
    String currentOSStylePath (const String& path);

    bool isAbsolutePath (const String& path);

    // A windows-aware version of File::getRelativePath()
    String getRelativePathFrom (const File& file, const File& sourceFolder);

    void writeStreamToFile (const File& file, const std::function<void (MemoryOutputStream&)>& writer);

} // namespace juce::build_tools
