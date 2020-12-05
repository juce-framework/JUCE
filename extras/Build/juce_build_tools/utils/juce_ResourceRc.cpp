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

namespace juce
{
namespace build_tools
{
    static String getCommaSeparatedVersionNumber (const String& version)
    {
        auto versionParts = StringArray::fromTokens (version, ",.", "");
        versionParts.trim();
        versionParts.removeEmptyStrings();
        while (versionParts.size() < 4)
            versionParts.add ("0");

        return versionParts.joinIntoString (",");
    }

    void ResourceRcOptions::write (const File& resourceRcFile) const
    {
        MemoryOutputStream mo;

        mo << "#pragma code_page(65001)" << newLine
           << newLine
           << "#ifdef JUCE_USER_DEFINED_RC_FILE" << newLine
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

        const auto writeRCValue = [&] (const String& n, const String& value)
        {
            if (value.isNotEmpty())
                mo << "      VALUE \"" << n << "\",  \""
                   << value.replace ("\"", "\"\"") << "\\0\"" << newLine;
        };

        writeRCValue ("CompanyName",     companyName);
        writeRCValue ("LegalCopyright",  companyCopyright);
        writeRCValue ("FileDescription", projectName);
        writeRCValue ("FileVersion",     version);
        writeRCValue ("ProductName",     projectName);
        writeRCValue ("ProductVersion",  version);

        mo << "    END" << newLine
           << "  END" << newLine
           << newLine
           << "  BLOCK \"VarFileInfo\"" << newLine
           << "  BEGIN" << newLine
           << "    VALUE \"Translation\", 0x409, 1252" << newLine
           << "  END" << newLine
           << "END" << newLine
           << newLine
           << "#endif" << newLine;

        if (icon.existsAsFile())
            mo << newLine
               << "IDI_ICON1 ICON DISCARDABLE " << icon.getFileName().quoted()
               << newLine
               << "IDI_ICON2 ICON DISCARDABLE " << icon.getFileName().quoted();

        overwriteFileIfDifferentOrThrow (resourceRcFile, mo);
    }
}
}
