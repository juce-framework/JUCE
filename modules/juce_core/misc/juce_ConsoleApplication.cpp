/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

File ArgumentList::Argument::resolveAsFile() const
{
    return File::getCurrentWorkingDirectory().getChildFile (text.unquoted());
}

File ArgumentList::Argument::resolveAsExistingFile() const
{
    auto f = resolveAsFile();

    if (! f.exists())
        ConsoleApplication::fail ("Could not find file: " + f.getFullPathName());

    return f;
}

File ArgumentList::Argument::resolveAsExistingFolder() const
{
    auto f = resolveAsFile();

    if (! f.isDirectory())
        ConsoleApplication::fail ("Could not find folder: " + f.getFullPathName());

    return f;
}

bool ArgumentList::Argument::isLongOption() const     { return text[0] == '-' && text[1] == '-' && text[2] != '-'; }
bool ArgumentList::Argument::isShortOption() const    { return text[0] == '-' && text[1] != '-'; }

bool ArgumentList::Argument::isLongOption (const String& option) const
{
    if (option.startsWith ("--"))
        return text == option;

    jassert (! option.startsWithChar ('-')); // this will always fail to match

    return text == "--" + option;
}

bool ArgumentList::Argument::isShortOption (char option) const
{
    jassert (option != '-'); // this is probably not what you intended to pass in

    return isShortOption() && text.containsChar (option);
}

static bool compareOptionStrings (StringRef s1, StringRef s2)
{
    if (s1 == s2)
        return true;

    auto toks1 = StringArray::fromTokens (s1, "|", {});
    auto toks2 = StringArray::fromTokens (s2, "|", {});

    for (auto& part1 : toks1)
        for (auto& part2 : toks2)
            if (part1.trim() == part2.trim())
                return true;

    return false;
}

bool ArgumentList::Argument::operator== (StringRef s) const   { return compareOptionStrings (text, s); }
bool ArgumentList::Argument::operator!= (StringRef s) const   { return ! operator== (s); }

//==============================================================================
ArgumentList::ArgumentList (String exeName, StringArray args)
    : executableName (std::move (exeName))
{
    args.trim();

    for (auto& a : args)
        arguments.add ({ a });
}

ArgumentList::ArgumentList (int argc, char* argv[])
    : ArgumentList (argv[0], StringArray (argv + 1, argc - 1))
{
}

ArgumentList::ArgumentList (const String& exeName, const String& args)
    : ArgumentList (exeName, StringArray::fromTokens (args, true))
{
}

int ArgumentList::size() const                                      { return arguments.size(); }
ArgumentList::Argument ArgumentList::operator[] (int index) const   { return arguments[index]; }

void ArgumentList::checkMinNumArguments (int expectedMinNumberOfArgs) const
{
    if (size() < expectedMinNumberOfArgs)
        ConsoleApplication::fail ("Not enough arguments!");
}

int ArgumentList::indexOfOption (StringRef option) const
{
    jassert (option == String (option).trim()); // passing non-trimmed strings will always fail to find a match!

    for (int i = 0; i < arguments.size(); ++i)
        if (arguments.getReference(i) == option)
            return i;

    return -1;
}

bool ArgumentList::containsOption (StringRef option) const
{
    return indexOfOption (option) >= 0;
}

void ArgumentList::failIfOptionIsMissing (StringRef option) const
{
    if (! containsOption (option))
        ConsoleApplication::fail ("Expected the option " + option);
}

ArgumentList::Argument ArgumentList::getArgumentAfterOption (StringRef option) const
{
    for (int i = 0; i < arguments.size() - 1; ++i)
        if (arguments.getReference(i) == option)
            return arguments.getReference (i + 1);

    return {};
}

File ArgumentList::getFileAfterOption (StringRef option) const
{
    failIfOptionIsMissing (option);
    auto arg = getArgumentAfterOption (option);

    if (arg.text.isEmpty() || arg.text.startsWithChar ('-'))
        ConsoleApplication::fail ("Expected a filename after the " + option + " option");

    return arg.resolveAsFile();
}

File ArgumentList::getExistingFileAfterOption (StringRef option) const
{
    failIfOptionIsMissing (option);
    auto arg = getArgumentAfterOption (option);

    if (arg.text.isEmpty())
        ConsoleApplication::fail ("Expected a filename after the " + option + " option");

    return arg.resolveAsExistingFile();
}

File ArgumentList::getExistingFolderAfterOption (StringRef option) const
{
    failIfOptionIsMissing (option);
    auto arg = getArgumentAfterOption (option);

    if (arg.text.isEmpty())
        ConsoleApplication::fail ("Expected a folder name after the " + option + " option");

    return arg.resolveAsExistingFolder();
}

//==============================================================================
struct ConsoleAppFailureCode
{
    String errorMessage;
    int returnCode;
};

void ConsoleApplication::fail (String errorMessage, int returnCode)
{
    throw ConsoleAppFailureCode { std::move (errorMessage), returnCode };
}

int ConsoleApplication::invokeCatchingFailures (std::function<int()>&& f)
{
    int returnCode = 0;

    try
    {
        returnCode = f();
    }
    catch (const ConsoleAppFailureCode& error)
    {
        std::cout << error.errorMessage << std::endl;
        returnCode = error.returnCode;
    }

    return returnCode;
}

int ConsoleApplication::findAndRunCommand (const ArgumentList& args) const
{
    for (auto& c : commands)
        if (args.containsOption (c.commandOption))
            return invokeCatchingFailures ([&] { c.command (args); return 0; });

    if (commandIfNoOthersRecognised.isNotEmpty())
        for (auto& c : commands)
            if (compareOptionStrings (c.commandOption, commandIfNoOthersRecognised))
                return invokeCatchingFailures ([&] { c.command (args); return 0; });

    fail ("Unrecognised arguments");
    return 0;
}

int ConsoleApplication::findAndRunCommand (int argc, char* argv[]) const
{
    return findAndRunCommand (ArgumentList (argc, argv));
}

void ConsoleApplication::addCommand (Command c)
{
    commands.emplace_back (std::move (c));
}

void ConsoleApplication::addHelpCommand (String arg, String helpMessage, bool invokeIfNoOtherCommandRecognised)
{
    addCommand ({ arg, arg, "Prints this message",
                  [this, helpMessage] (const ArgumentList& args) { printHelp (helpMessage, args); }});

    if (invokeIfNoOtherCommandRecognised)
        commandIfNoOthersRecognised = arg;
}

void ConsoleApplication::addVersionCommand (String arg, String versionText)
{
    addCommand ({ arg, arg, "Prints the current version number",
                  [versionText] (const ArgumentList&)
                  {
                      std::cout << versionText << std::endl;
                  }});
}

void ConsoleApplication::printHelp (const String& preamble, const ArgumentList& args) const
{
    std::cout << preamble << std::endl;

    auto exeName = args.executableName.fromLastOccurrenceOf ("/", false, false)
                                      .fromLastOccurrenceOf ("\\", false, false);

    StringArray namesAndArgs;
    int maxLength = 0;

    for (auto& c : commands)
    {
        auto nameAndArgs = exeName + " " + c.argumentDescription;
        namesAndArgs.add (nameAndArgs);
        maxLength = std::max (maxLength, nameAndArgs.length());
    }

    for (size_t i = 0; i < commands.size(); ++i)
        std::cout << " " << namesAndArgs[(int) i].paddedRight (' ', maxLength + 2)
                  << commands[i].commandDescription << std::endl;

    std::cout << std::endl;
}

} // namespace juce
