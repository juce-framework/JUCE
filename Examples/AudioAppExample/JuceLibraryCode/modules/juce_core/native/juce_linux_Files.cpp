/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

enum
{
    U_ISOFS_SUPER_MAGIC = 0x9660,   // linux/iso_fs.h
    U_MSDOS_SUPER_MAGIC = 0x4d44,   // linux/msdos_fs.h
    U_NFS_SUPER_MAGIC = 0x6969,     // linux/nfs_fs.h
    U_SMB_SUPER_MAGIC = 0x517B      // linux/smb_fs.h
};

bool File::isOnCDRomDrive() const
{
    struct statfs buf;

    return statfs (getFullPathName().toUTF8(), &buf) == 0
             && buf.f_type == (short) U_ISOFS_SUPER_MAGIC;
}

bool File::isOnHardDisk() const
{
    struct statfs buf;

    if (statfs (getFullPathName().toUTF8(), &buf) == 0)
    {
        switch (buf.f_type)
        {
            case U_ISOFS_SUPER_MAGIC:   // CD-ROM
            case U_MSDOS_SUPER_MAGIC:   // Probably floppy (but could be mounted FAT filesystem)
            case U_NFS_SUPER_MAGIC:     // Network NFS
            case U_SMB_SUPER_MAGIC:     // Network Samba
                return false;

            default: break;
        }
    }

    // Assume so if this fails for some reason
    return true;
}

bool File::isOnRemovableDrive() const
{
    jassertfalse; // xxx not implemented for linux!
    return false;
}

String File::getVersion() const
{
    return String(); // xxx not yet implemented
}

//==============================================================================
static File resolveXDGFolder (const char* const type, const char* const fallbackFolder)
{
    StringArray confLines;
    File ("~/.config/user-dirs.dirs").readLines (confLines);

    for (int i = 0; i < confLines.size(); ++i)
    {
        const String line (confLines[i].trimStart());

        if (line.startsWith (type))
        {
            // eg. resolve XDG_MUSIC_DIR="$HOME/Music" to /home/user/Music
            const File f (line.replace ("$HOME", File ("~").getFullPathName())
                              .fromFirstOccurrenceOf ("=", false, false)
                              .trim().unquoted());

            if (f.isDirectory())
                return f;
        }
    }

    return File (fallbackFolder);
}

const char* const* juce_argv = nullptr;
int juce_argc = 0;

File File::getSpecialLocation (const SpecialLocationType type)
{
    switch (type)
    {
        case userHomeDirectory:
        {
            if (const char* homeDir = getenv ("HOME"))
                return File (CharPointer_UTF8 (homeDir));

            if (struct passwd* const pw = getpwuid (getuid()))
                return File (CharPointer_UTF8 (pw->pw_dir));

            return File();
        }

        case userDocumentsDirectory:          return resolveXDGFolder ("XDG_DOCUMENTS_DIR", "~");
        case userMusicDirectory:              return resolveXDGFolder ("XDG_MUSIC_DIR",     "~");
        case userMoviesDirectory:             return resolveXDGFolder ("XDG_VIDEOS_DIR",    "~");
        case userPicturesDirectory:           return resolveXDGFolder ("XDG_PICTURES_DIR",  "~");
        case userDesktopDirectory:            return resolveXDGFolder ("XDG_DESKTOP_DIR",   "~/Desktop");
        case userApplicationDataDirectory:    return resolveXDGFolder ("XDG_CONFIG_HOME",   "~");
        case commonDocumentsDirectory:
        case commonApplicationDataDirectory:  return File ("/var");
        case globalApplicationsDirectory:     return File ("/usr");

        case tempDirectory:
        {
            File tmp ("/var/tmp");

            if (! tmp.isDirectory())
            {
                tmp = "/tmp";

                if (! tmp.isDirectory())
                    tmp = File::getCurrentWorkingDirectory();
            }

            return tmp;
        }

        case invokedExecutableFile:
            if (juce_argv != nullptr && juce_argc > 0)
                return File (CharPointer_UTF8 (juce_argv[0]));
            // deliberate fall-through...

        case currentExecutableFile:
        case currentApplicationFile:
            return juce_getExecutableFile();

        case hostApplicationPath:
        {
            const File f ("/proc/self/exe");
            return f.isLink() ? f.getLinkedTarget() : juce_getExecutableFile();
        }

        default:
            jassertfalse; // unknown type?
            break;
    }

    return File();
}

//==============================================================================
bool File::moveToTrash() const
{
    if (! exists())
        return true;

    File trashCan ("~/.Trash");

    if (! trashCan.isDirectory())
        trashCan = "~/.local/share/Trash/files";

    if (! trashCan.isDirectory())
        return false;

    return moveFileTo (trashCan.getNonexistentChildFile (getFileNameWithoutExtension(),
                                                         getFileExtension()));
}

//==============================================================================
static bool isFileExecutable (const String& filename)
{
    juce_statStruct info;

    return juce_stat (filename, info)
            && S_ISREG (info.st_mode)
            && access (filename.toUTF8(), X_OK) == 0;
}

bool Process::openDocument (const String& fileName, const String& parameters)
{
    String cmdString (fileName.replace (" ", "\\ ",false));
    cmdString << " " << parameters;

    if (URL::isProbablyAWebsiteURL (fileName)
         || cmdString.startsWithIgnoreCase ("file:")
         || URL::isProbablyAnEmailAddress (fileName)
         || File::createFileWithoutCheckingPath (fileName).isDirectory()
         || ! isFileExecutable (fileName))
    {
        // create a command that tries to launch a bunch of likely browsers
        static const char* const browserNames[] = { "xdg-open", "/etc/alternatives/x-www-browser", "firefox", "mozilla",
                                                    "google-chrome", "chromium-browser", "opera", "konqueror" };
        StringArray cmdLines;

        for (int i = 0; i < numElementsInArray (browserNames); ++i)
            cmdLines.add (String (browserNames[i]) + " " + cmdString.trim().quoted());

        cmdString = cmdLines.joinIntoString (" || ");
    }

    const char* const argv[4] = { "/bin/sh", "-c", cmdString.toUTF8(), 0 };

    const int cpid = fork();

    if (cpid == 0)
    {
        setsid();

        // Child process
        execve (argv[0], (char**) argv, environ);
        exit (0);
    }

    return cpid >= 0;
}

void File::revealToUser() const
{
    if (isDirectory())
        startAsProcess();
    else if (getParentDirectory().exists())
        getParentDirectory().startAsProcess();
}
