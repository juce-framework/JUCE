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

#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <vector>

//==============================================================================
struct FileHelpers
{
    static std::string getCurrentWorkingDirectory()
    {
        std::vector<char> buffer (1024);

        while (getcwd (buffer.data(), buffer.size() - 1) == nullptr && errno == ERANGE)
            buffer.resize (buffer.size() * 2 / 3);

        return { buffer.data() };
    }

    static bool endsWith (const std::string& s, char c)
    {
        if (s.length() == 0)
            return false;

        return *s.rbegin() == c;
    }

    static std::string appendedPaths (const std::string& first, const std::string& second)
    {
        return endsWith (first, '/') ? first + second : first + "/" + second;
    }

    static bool exists (const std::string& path)
    {
        return ! path.empty() && access (path.c_str(), F_OK) == 0;
    }

    static bool deleteFile (const std::string& path)
    {
        if (! exists (path))
            return true;

        return remove (path.c_str()) == 0;
    }

    static std::string getFilename (const std::string& path)
    {
        return { std::find_if (path.rbegin(), path.rend(), [] (auto c) { return c == '/'; }).base(),
                 path.end() };
    }

    static bool isDirectory (const std::string& path)
    {
       #if defined (__FreeBSD__) || defined (__OpenBSD__)
        #define JUCE_STAT stat
       #else
        #define JUCE_STAT stat64
       #endif

        struct JUCE_STAT info;

        return    ! path.empty()
               && JUCE_STAT (path.c_str(), &info) == 0
               && ((info.st_mode & S_IFDIR) != 0);
    }

    static std::string getParentDirectory (const std::string& path)
    {
        std::string p { path.begin(),
                        std::find_if (path.rbegin(),
                                      path.rend(),
                                      [] (auto c) { return c == '/'; }).base() };

        // Trim the ending slash, but only if not root
        if (endsWith (p, '/') && p.length() > 1)
            return { p.begin(), p.end() - 1 };

        return p;
    }

    static bool createDirectory (const std::string& path)
    {
        if (isDirectory (path))
            return true;

        const auto parentDir = getParentDirectory (path);

        if (path == parentDir)
            return false;

        if (createDirectory (parentDir))
            return mkdir (path.c_str(), 0777) != -1;

        return false;
    }
};

//==============================================================================
struct StringHelpers
{
    static bool isQuoteCharacter (char c)
    {
        return c == '"' || c == '\'';
    }

    static std::string unquoted (const std::string& str)
    {
        if (str.length() == 0 || (! isQuoteCharacter (str[0])))
            return str;

        return str.substr (1, str.length() - (isQuoteCharacter (str[str.length() - 1]) ? 1 : 0));
    }

    static void ltrim (std::string& s)
    {
        s.erase (s.begin(), std::find_if (s.begin(), s.end(), [] (int c) { return ! std::isspace (c); }));
    }

    static void rtrim (std::string& s)
    {
        s.erase (std::find_if (s.rbegin(), s.rend(), [] (int c) { return ! std::isspace (c); }).base(), s.end());
    }

    static std::string trimmed (const std::string& str)
    {
        auto result = str;
        ltrim (result);
        rtrim (result);
        return result;
    }

    static std::string replaced (const std::string& str, char charToReplace, char replaceWith)
    {
        auto result = str;
        std::replace (result.begin(), result.end(), charToReplace, replaceWith);
        return result;
    }
};

//==============================================================================
static bool addFile (const std::string& filePath,
                     const std::string& binaryNamespace,
                     std::ofstream& headerStream,
                     std::ofstream& cppStream,
                     bool verbose)
{
    std::ifstream fileStream (filePath, std::ios::in | std::ios::binary | std::ios::ate);

    if (! fileStream.is_open())
    {
        std::cerr << "Failed to open input file " << filePath << std::endl;
        return false;
    }

    std::vector<char> buffer ((size_t) fileStream.tellg());
    fileStream.seekg (0);
    fileStream.read (buffer.data(), static_cast<std::streamsize> (buffer.size()));

    const auto variableName = StringHelpers::replaced (StringHelpers::replaced (FileHelpers::getFilename (filePath),
                                                                                ' ',
                                                                                '_'),
                                                       '.',
                                                       '_');

    if (verbose)
    {
        std::cout << "Adding " << variableName << ": "
                  << buffer.size() << " bytes" << std::endl;
    }

    headerStream << "    extern const char*  " << variableName << ";" << std::endl
                 << "    const int           " << variableName << "Size = "
                 << buffer.size() << ";" << std::endl;

    cppStream << "static const unsigned char temp0[] = {";

    auto* data = (const uint8_t*) buffer.data();

    for (size_t i = 0; i < buffer.size() - 1; ++i)
    {
        cppStream << (int) data[i] << ",";

        if ((i % 40) == 39)
            cppStream << std::endl << "  ";
    }

    cppStream << (int) data[buffer.size() - 1] << ",0,0};" << std::endl;
    cppStream << "const char* " << binaryNamespace << "::" << variableName
              << " = (const char*) temp0" << ";" << std::endl << std::endl;

    return true;
}

//==============================================================================
class Arguments
{
public:
    enum class PositionalArguments
    {
        sourceFile = 0,
        targetDirectory,
        targetFilename,
        binaryNamespace
    };

    static std::optional<Arguments> create (int argc, char* argv[])
    {
        std::vector<std::string> arguments;
        bool verbose = false;

        for (int i = 1; i < argc; ++i)
        {
            std::string arg { argv[i] };

            if (arg == "-v" || arg == "--verbose")
                verbose = true;
            else
                arguments.emplace_back (std::move (arg));
        }

        if (arguments.size() != static_cast<size_t> (PositionalArguments::binaryNamespace) + 1)
            return std::nullopt;

        return Arguments { std::move (arguments), verbose };
    }

    std::string get (PositionalArguments argument) const
    {
        return arguments[static_cast<size_t> (argument)];
    }

    bool isVerbose() const
    {
        return verbose;
    }

private:
    Arguments (std::vector<std::string> args, bool verboseIn)
        : arguments (std::move (args)), verbose (verboseIn)
    {
    }

    std::vector<std::string> arguments;
    bool verbose = false;
};

//==============================================================================
int main (int argc, char* argv[])
{
    const auto arguments = Arguments::create (argc, argv);

    if (! arguments.has_value())
    {
        std::cout << " Usage: SimpleBinaryBuilder  [-v | --verbose] sourcefile targetdirectory targetfilename namespace"
                                                                                                        << std::endl << std::endl
                  << " SimpleBinaryBuilder will encode the provided source file into"                   << std::endl
                  << " two files called (targetfilename).cpp and (targetfilename).h,"                   << std::endl
                  << " which it will write into the specified target directory."                        << std::endl
                  << " The target directory will be automatically created if necessary. The binary"     << std::endl
                  << " resource will be available in the given namespace."                              << std::endl << std::endl;

        return 0;
    }

    const auto currentWorkingDirectory = FileHelpers::getCurrentWorkingDirectory();

    using ArgType = Arguments::PositionalArguments;

    const auto sourceFile = FileHelpers::appendedPaths (currentWorkingDirectory,
                                                        StringHelpers::unquoted (arguments->get (ArgType::sourceFile)));

    if (! FileHelpers::exists (sourceFile))
    {
        std::cerr << "Source file doesn't exist: "
                  << sourceFile
                  << std::endl << std::endl;

        return 1;
    }

    const auto targetDirectory = FileHelpers::appendedPaths (currentWorkingDirectory,
                                                             StringHelpers::unquoted (arguments->get (ArgType::targetDirectory)));

    if (! FileHelpers::exists (targetDirectory))
    {
        if (! FileHelpers::createDirectory (targetDirectory))
        {
            std::cerr << "Failed to create target directory: " << targetDirectory << std::endl;
            return 1;
        }
    }

    const auto className = StringHelpers::trimmed (arguments->get (ArgType::targetFilename));
    const auto binaryNamespace = StringHelpers::trimmed (arguments->get (ArgType::binaryNamespace));

    const auto headerFilePath = FileHelpers::appendedPaths (targetDirectory, className + ".h");
    const auto cppFilePath    = FileHelpers::appendedPaths (targetDirectory, className + ".cpp");

    if (arguments->isVerbose())
    {
        std::cout << "Creating " << headerFilePath
                  << " and " << cppFilePath
                  << " from file " << sourceFile
                  << "..." << std::endl << std::endl;
    }

    if (! FileHelpers::deleteFile (headerFilePath))
    {
        std::cerr << "Failed to remove old header file: " << headerFilePath << std::endl;
        return 1;
    }

    if (! FileHelpers::deleteFile (cppFilePath))
    {
        std::cerr << "Failed to remove old source file: " << cppFilePath << std::endl;
        return 1;
    }

    std::ofstream header (headerFilePath);

    if (! header.is_open())
    {
        std::cerr << "Failed to open " << headerFilePath << std::endl;

        return 1;
    }

    std::ofstream cpp (cppFilePath);

    if (! cpp.is_open())
    {
        std::cerr << "Failed to open " << headerFilePath << std::endl;

        return 1;
    }

    header << "/* (Auto-generated binary data file). */" << std::endl << std::endl
           << "#pragma once" << std::endl << std::endl
           << "namespace " << binaryNamespace << std::endl
           << "{" << std::endl;

    cpp << "/* (Auto-generated binary data file). */" << std::endl << std::endl
        << "#include " << std::quoted (className + ".h") << std::endl << std::endl;

    if (! addFile (sourceFile, binaryNamespace, header, cpp, arguments->isVerbose()))
        return 1;

    header << "}" << std::endl << std::endl;

    return 0;
}
