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

static inline File resolveFilename (const String& name)
{
    return File::getCurrentWorkingDirectory().getChildFile (name.unquoted());
}

static inline void checkFileExists (const File& f)
{
    if (! f.exists())
        ConsoleApplication::fail ("Could not find file: " + f.getFullPathName());
}

static inline void checkFolderExists (const File& f)
{
    if (! f.isDirectory())
        ConsoleApplication::fail ("Could not find folder: " + f.getFullPathName());
}

File ArgumentList::Argument::resolveAsFile() const
{
    return resolveFilename (text);
}

File ArgumentList::Argument::resolveAsExistingFile() const
{
    auto f = resolveAsFile();
    checkFileExists (f);
    return f;
}

File ArgumentList::Argument::resolveAsExistingFolder() const
{
    auto f = resolveAsFile();

    if (! f.isDirectory())
        ConsoleApplication::fail ("Could not find folder: " + f.getFullPathName());

    return f;
}

static inline bool isShortOptionFormat (StringRef s)  { return s[0] == '-' && s[1] != '-'; }
static inline bool isLongOptionFormat  (StringRef s)  { return s[0] == '-' && s[1] == '-' && s[2] != '-'; }
static inline bool isOptionFormat      (StringRef s)  { return s[0] == '-'; }

bool ArgumentList::Argument::isLongOption() const     { return isLongOptionFormat (text); }
bool ArgumentList::Argument::isShortOption() const    { return isShortOptionFormat (text); }
bool ArgumentList::Argument::isOption() const         { return isOptionFormat (text); }

bool ArgumentList::Argument::isLongOption (const String& option) const
{
    if (! isLongOptionFormat (option))
    {
        jassert (! isShortOptionFormat (option)); // this will always fail to match
        return isLongOption ("--" + option);
    }

    return text.upToFirstOccurrenceOf ("=", false, false) == option;
}

String ArgumentList::Argument::getLongOptionValue() const
{
    if (isLongOption())
        if (auto equalsIndex = text.indexOfChar ('='))
            return text.substring (equalsIndex + 1);

    return {};
}

bool ArgumentList::Argument::isShortOption (char option) const
{
    jassert (option != '-'); // this is probably not what you intended to pass in

    return isShortOption() && text.containsChar (option);
}

bool ArgumentList::Argument::operator== (StringRef wildcard) const
{
    for (auto& o : StringArray::fromTokens (wildcard, "|", {}))
    {
        if (text == o)
            return true;

        if (isShortOptionFormat (o) && o.length() == 2 && isShortOption ((char) o[1]))
            return true;

        if (isLongOptionFormat (o) && isLongOption (o))
            return true;
    }

    return false;
}

bool ArgumentList::Argument::operator!= (StringRef s) const   { return ! operator== (s); }

//==============================================================================
ArgumentList::ArgumentList (String exeName, StringArray args)
    : executableName (std::move (exeName))
{
    args.trim();
    args.removeEmptyStrings();

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

String ArgumentList::getValueForOption (StringRef option) const
{
    jassert (isOptionFormat (option)); // the thing you're searching for must be an option

    for (int i = 0; i < arguments.size(); ++i)
    {
        auto& arg = arguments.getReference(i);

        if (arg == option)
        {
            if (arg.isShortOption())
            {
                if (i < arguments.size() - 1 && ! arguments.getReference (i + 1).isOption())
                    return arguments.getReference (i + 1).text;

                return {};
            }

            if (arg.isLongOption())
                return arg.getLongOptionValue();
        }
    }

    return {};
}

File ArgumentList::getFileForOption (StringRef option) const
{
    auto text = getValueForOption (option);

    if (text.isEmpty())
    {
        failIfOptionIsMissing (option);
        ConsoleApplication::fail ("Expected a filename after the " + option + " option");
    }

    return resolveFilename (text);
}

File ArgumentList::getExistingFileForOption (StringRef option) const
{
    auto file = getFileForOption (option);
    checkFileExists (file);
    return file;
}

File ArgumentList::getExistingFolderForOption (StringRef option) const
{
    auto file = getFileForOption (option);
    checkFolderExists (file);
    return file;
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

const ConsoleApplication::Command* ConsoleApplication::findCommand (const ArgumentList& args, bool optionMustBeFirstArg) const
{
    for (auto& c : commands)
    {
        auto index = args.indexOfOption (c.commandOption);

        if (optionMustBeFirstArg ? (index == 0) : (index >= 0))
            return &c;
    }

    if (commandIfNoOthersRecognised >= 0)
        return &commands[(size_t) commandIfNoOthersRecognised];

    return {};
}

int ConsoleApplication::findAndRunCommand (const ArgumentList& args, bool optionMustBeFirstArg) const
{
    if (auto c = findCommand (args, optionMustBeFirstArg))
        return invokeCatchingFailures ([=] { c->command (args); return 0; });

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

void ConsoleApplication::addDefaultCommand (Command c)
{
    commandIfNoOthersRecognised = (int) commands.size();
    addCommand (std::move (c));
}

void ConsoleApplication::addHelpCommand (String arg, String helpMessage, bool makeDefaultCommand)
{
    Command c { arg, arg, "Prints the list of commands", {},
                [this, helpMessage] (const ArgumentList& args)
                {
                    std::cout << helpMessage << std::endl;
                    printCommandList (args);
                }};

    if (makeDefaultCommand)
        addDefaultCommand (std::move (c));
    else
        addCommand (std::move (c));
}

void ConsoleApplication::addVersionCommand (String arg, String versionText)
{
    addCommand ({ arg, arg, "Prints the current version number", {},
                  [versionText] (const ArgumentList&)
                  {
                      std::cout << versionText << std::endl;
                  }});
}

const std::vector<ConsoleApplication::Command>& ConsoleApplication::getCommands() const
{
    return commands;
}

void ConsoleApplication::printCommandList (const ArgumentList& args) const
{
    auto exeName = args.executableName.fromLastOccurrenceOf ("/", false, false)
                                      .fromLastOccurrenceOf ("\\", false, false);

    StringArray namesAndArgs;
    int descriptionIndent = 0;

    for (auto& c : commands)
    {
        auto nameAndArgs = exeName + " " + c.argumentDescription;
        namesAndArgs.add (nameAndArgs);
        descriptionIndent = std::max (descriptionIndent, nameAndArgs.length());
    }

    descriptionIndent = std::min (descriptionIndent + 1, 40);

    for (size_t i = 0; i < commands.size(); ++i)
    {
        auto nameAndArgs = namesAndArgs[(int) i];
        std::cout << ' ';

        if (nameAndArgs.length() > descriptionIndent)
            std::cout << nameAndArgs << std::endl << String::repeatedString (" ", descriptionIndent + 1);
        else
            std::cout << nameAndArgs.paddedRight (' ', descriptionIndent);

        std::cout << commands[i].shortDescription << std::endl;
    }

    std::cout << std::endl;
}

} // namespace juce
