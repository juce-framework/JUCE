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

//==============================================================================
/**
    Holds a list of command-line arguments, and provides useful methods for searching
    and operating on them.

    You can create an ArgumentList manually, or give it some argv/argc values from a
    main() function to parse.

    @see ConsoleApplication
*/
struct ArgumentList
{
    /** Creates an argument list for a given executable. */
    ArgumentList (String executable, StringArray arguments);

    /** Parses a standard argv/argc pair to create an argument list. */
    ArgumentList (int argc, char* argv[]);

    /** Tokenises a string containing all the arguments to create an argument list. */
    ArgumentList (const String& executable, const String& arguments);

    ArgumentList (const ArgumentList&) = default;
    ArgumentList& operator= (const ArgumentList&) = default;

    //==============================================================================
    /**
        One of the arguments in an ArgumentList.
    */
    struct Argument
    {
        /** The original text of this argument. */
        String text;

        /** Resolves this argument as an absolute File, using the current working
            directory as a base for resolving relative paths, and stripping quotes, etc.
        */
        File resolveAsFile() const;

        /** Resolves this argument as an absolute File, using the current working
            directory as a base for resolving relative paths, and also doing a check to
            make sure the file exists.
            If the file doesn't exist, this will call fail() with a suitable error.
            @see resolveAsFile, resolveAsExistingFolder
        */
        File resolveAsExistingFile() const;

        /** Resolves a user-supplied folder name into an absolute File, using the current working
            directory as a base for resolving relative paths, and also doing a check to make
            sure the folder exists.
            If the folder doesn't exist, this will call fail() with a suitable error.
            @see resolveAsFile, resolveAsExistingFile
        */
        File resolveAsExistingFolder() const;

        /** Returns true if this argument starts with a double dash. */
        bool isLongOption() const;

        /** Returns true if this argument starts with a single dash. */
        bool isShortOption() const;

        /** Returns true if this argument starts with a double dash, followed by the given string. */
        bool isLongOption (const String& optionRoot) const;

        /** Returns true if this argument starts with a single dash and then contains the given character somewhere inside it. */
        bool isShortOption (char shortOptionCharacter) const;

        /** Compares this argument against a string.
            The string may be a pipe-separated list of options, e.g. "--help|-h"
        */
        bool operator== (StringRef stringToCompare) const;

        /** Compares this argument against a string.
            The string may be a pipe-separated list of options, e.g. "--help|-h"
        */
        bool operator!= (StringRef stringToCompare) const;
    };

    //==============================================================================
    /** Returns the number of arguments in the list. */
    int size() const;

    /** Returns one of the arguments */
    Argument operator[] (int index) const;

    /** Throws an error unless there are at least the given number of arguments. */
    void checkMinNumArguments (int expectedMinNumberOfArgs) const;

    /** Returns true if the given string matches one of the arguments.
        The option can also be a list of different versions separated by pipes, e.g. "--help|-h"
    */
    bool containsOption (StringRef option) const;

    /** Returns the index of the given string if it matches one of the arguments, or -1 if it doesn't.
        The option can also be a list of different versions separated by pipes, e.g. "--help|-h"
    */
    int indexOfOption (StringRef option) const;

    /** Throws an error unless the given option is found in the argument list. */
    void failIfOptionIsMissing (StringRef option) const;

    /** Looks for the given argument and returns the one that follows it in the list.
        The option can also be a list of different versions separated by pipes, e.g. "--help|-h"
        If the argument isn't found, this returns an empty string.
    */
    Argument getArgumentAfterOption (StringRef option) const;

    /** Looks for a given argument and tries to parse the following argument as a file.
        The option can also be a list of different versions separated by pipes, e.g. "--help|-h"
        If the option isn't found, or if the next argument isn't a filename, it will throw
        an error.
    */
    File getFileAfterOption (StringRef option) const;

    /** Looks for a given argument and tries to parse the following argument as a file
        which must exist for this to succeed.
        The option can also be a list of different versions separated by pipes, e.g. "--help|-h"
        If the option isn't found, or if the next argument isn't a filename, or if the file
        doesn't exist, or if it's a folder rather than a file, then it will throw a suitable error.
    */
    File getExistingFileAfterOption (StringRef option) const;

    /** Looks for a given argument and tries to parse the following argument as a folder
        which must exist for this to succeed.
        The option can also be a list of different versions separated by pipes, e.g. "--help|-h"
        If the option isn't found, or if the next argument isn't a filename, or if it doesn't
        point to a folder, then it will throw a suitable error.
    */
    File getExistingFolderAfterOption (StringRef option) const;

    /** The name or path of the executable that was invoked, as it was specified on the command-line. */
    String executableName;

    /** The list of arguments (not including the name of the executable that was invoked). */
    Array<Argument> arguments;
};


//==============================================================================
/**
    Represents a the set of commands that a console app can perform, and provides
    helper functions for performing them.

    When using these helper classes to implement a console app, you probably want to
    do something along these lines:

    @code
    int main (int argc, char* argv[])
    {
        ConsoleApplication app;

        app.addHelpCommand ("--help|-h", "Usage:", true);
        app.addVersionCommand ("--version|-v", "MyApp version 1.2.3");

        app.addCommand ({ "--foo",
                          "--foo filename",
                          "Performs a foo operation on the given file",
                          [] (const auto& args) { doFoo (args); }});

        return app.findAndRunCommand (argc, argv);
    }
    @endcode

    @see ArgumentList
*/
struct ConsoleApplication
{
    //==============================================================================
    /**
        Represents a command that can be executed if its command-line arguments are matched.
        @see ConsoleApplication::addCommand(), ConsoleApplication::findAndRunCommand()
    */
    struct Command
    {
        /** The option string that must appear in the argument list for this command to be invoked.
            This can also be a list of different versions separated by pipes, e.g. "--help|-h"
        */
        String commandOption;

        /** A description of the command-line arguments needed for this command, which will be
            printed as part of the help text.
        */
        String argumentDescription;

        /** A description of the meaning of this command, for use in the help text. */
        String commandDescription;

        /** The actual command that should be invoked to perform this action. */
        std::function<void(const ArgumentList&)> command;
    };

    //==============================================================================
    /** Adds a command to the list. */
    void addCommand (Command);

    /** Adds a help command to the list.
        This command will print the user-supplied message that's passed in here as an
        argument, followed by a list of all the registered commands.
    */
    void addHelpCommand (String helpArgument, String helpMessage,
                         bool invokeIfNoOtherCommandRecognised);

    /** Adds a command that will print the given text in response to the "--version" option. */
    void addVersionCommand (String versionArgument, String versionText);

    //==============================================================================
    /** Throws a failure exception to cause a command-line app to terminate.
        This is intended to be called from code in a Command, so that the
        exception will be automatically caught and turned into a printed error message
        and a return code which will be returned from main().
        @see ConsoleApplication::invokeCatchingFailures()
    */
    static void fail (String errorMessage, int returnCode = 1);

    /** Invokes a function, catching any fail() calls that it might trigger, and handling
        them by printing their error message and returning their error code.
        @see ConsoleApplication::fail()
    */
    static int invokeCatchingFailures (std::function<int()>&& functionToCall);

    //==============================================================================
    /** Looks for the first command in the list which matches the given arguments, and
        tries to invoke it.

        If no command is found, it prints a help message listing the available commands.
        If the command calls the fail() function, this will throw an exception that gets
        automatically caught and handled, and this method will return the error code that
        was passed into the fail() call.
    */
    int findAndRunCommand (const ArgumentList&) const;

    /** Creates an ArgumentList object from the argc and argv variablrs, and invokes
        findAndRunCommand() using it.
    */
    int findAndRunCommand (int argc, char* argv[]) const;

private:
    //==============================================================================
    std::vector<Command> commands;
    String commandIfNoOthersRecognised;

    void printHelp (const String& preamble, const ArgumentList&) const;
};

} // namespace juce
