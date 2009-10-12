/*
  ==============================================================================

   Utility to turn a bunch of binary files into a .cpp file and .h file full of
   data so they can be built directly into an executable.

   Copyright 2007 by Julian Storer.

   Use this code at your own risk! It carries no warranty!

  ==============================================================================
*/

#include "juce_AppConfig.h"
#include "../../juce_amalgamated.h"


//==============================================================================
static int addFile (const File& file,
                    const String& classname,
                    OutputStream& headerStream,
                    OutputStream& cppStream)
{
    MemoryBlock mb;
    file.loadFileAsData (mb);

    const String name (file.getFileName().toLowerCase()
                           .replaceCharacter (' ', '_')
                           .replaceCharacter ('.', '_')
                           .retainCharacters (T("abcdefghijklmnopqrstuvwxyz_0123456789")));

    printf ("Adding %s: %d bytes\n",
            (const char*) name,
            mb.getSize());

    headerStream.printf ("    extern const char*  %s;\r\n"
                         "    const int           %sSize = %d;\r\n\r\n",
                         (const char*) name,
                         (const char*) name,
                         mb.getSize());

    static int tempNum = 0;

    cppStream.printf ("static const unsigned char temp%d[] = {", ++tempNum);

    int i = 0;
    const uint8* const data = (const uint8*) mb.getData();

    while (i < mb.getSize() - 1)
    {
        if ((i % 40) != 39)
            cppStream.printf ("%d,", (int) data[i]);
        else
            cppStream.printf ("%d,\r\n  ", (int) data[i]);

        ++i;
    }

    cppStream.printf ("%d,0,0};\r\n", (int) data[i]);

    cppStream.printf ("const char* %s::%s = (const char*) temp%d;\r\n\r\n",
                      (const char*) classname,
                      (const char*) name,
                      tempNum);

    return mb.getSize();
}

static bool isHiddenFile (const File& f, const File& root)
{
    return f.getFileName().endsWithIgnoreCase (T(".scc"))
         || f.getFileName() == T(".svn")
         || f.getFileName().startsWithChar (T('.'))
         || (f.getSize() == 0 && ! f.isDirectory())
         || (f.getParentDirectory() != root && isHiddenFile (f.getParentDirectory(), root));
}

//==============================================================================
int main (int argc, char* argv[])
{
    // If you're running a command-line app, you need to initialise juce manually
    // before calling any Juce functionality..
    initialiseJuce_NonGUI();

    printf ("\n BinaryBuilder! Copyright 2007 by Julian Storer - www.rawmaterialsoftware.com\n\n");

    if (argc < 4 || argc > 5)
    {
        printf (" Usage: BinaryBuilder  sourcedirectory targetdirectory targetclassname [optional wildcard pattern]\n\n");
        printf (" BinaryBuilder will find all files in the source directory, and encode them\n");
        printf (" into two files called (targetclassname).cpp and (targetclassname).h, which it\n");
        printf (" will write into the target directory supplied.\n\n");
        printf (" Any files in sub-directories of the source directory will be put into the\n");
        printf (" resultant class, but #ifdef'ed out using the name of the sub-directory (hard to\n");
        printf (" explain, but obvious when you try it...)\n");

        return 0;
    }

    const File sourceDirectory (File::getCurrentWorkingDirectory()
                                     .getChildFile (String (argv[1]).unquoted()));

    if (! sourceDirectory.isDirectory())
    {
        String error ("Source directory doesn't exist: ");
        error << sourceDirectory.getFullPathName() << "\n\n";

        printf ((const char*) error);
        return 0;
    }

    const File destDirectory (File::getCurrentWorkingDirectory()
                                   .getChildFile (String (argv[2]).unquoted()));

    if (! destDirectory.isDirectory())
    {
        String error ("Destination directory doesn't exist: ");
        error << destDirectory.getFullPathName() << "\n\n";

        printf ((const char*) error);
        return 0;
    }

    String className (argv[3]);
    className = className.trim();

    const File headerFile (destDirectory.getChildFile (className).withFileExtension (T(".h")));
    const File cppFile    (destDirectory.getChildFile (className).withFileExtension (T(".cpp")));

    String message;
    message << "Creating " << headerFile.getFullPathName()
            << " and " << cppFile.getFullPathName()
            << " from files in " << sourceDirectory.getFullPathName()
            << "...\n\n";

    printf ((const char*) message);

    OwnedArray <File> files;
    sourceDirectory.findChildFiles (files, File::findFiles, true,
                                    (argc > 4) ? argv[4] : "*");

    if (files.size() == 0)
    {
        String error ("Didn't find any source files in: ");
        error << sourceDirectory.getFullPathName() << "\n\n";
        printf ((const char*) error);
        return 0;
    }

    headerFile.deleteFile();
    cppFile.deleteFile();

    OutputStream* header = headerFile.createOutputStream();

    if (header == 0)
    {
        String error ("Couldn't open ");
        error << headerFile.getFullPathName() << " for writing\n\n";
        printf ((const char*) error);
        return 0;
    }

    OutputStream* cpp = cppFile.createOutputStream();

    if (cpp == 0)
    {
        String error ("Couldn't open ");
        error << cppFile.getFullPathName() << " for writing\n\n";
        printf ((const char*) error);
        return 0;
    }

    header->printf ("/* (Auto-generated binary data file). */\r\n\r\n"
                    "#ifndef BINARY_%s_H\r\n"
                    "#define BINARY_%s_H\r\n\r\n"
                    "namespace %s\r\n"
                    "{\r\n",
                    (const char*) className.toUpperCase(),
                    (const char*) className.toUpperCase(),
                    (const char*) className);

    cpp->printf ("/* (Auto-generated binary data file). */\r\n\r\n"
                 "#include \"%s.h\"\r\n\r\n",
                 (const char*) className);

    int totalBytes = 0;

    for (int i = 0; i < files.size(); ++i)
    {
        const File file (*(files[i]));

        // (avoid source control files and hidden files..)
        if (! isHiddenFile (file, sourceDirectory))
        {
            if (file.getParentDirectory() != sourceDirectory)
            {
                header->printf ("  #ifdef %s\r\n", (const char*) file.getParentDirectory().getFileName().toUpperCase());
                cpp->printf      ("#ifdef %s\r\n", (const char*) file.getParentDirectory().getFileName().toUpperCase());

                totalBytes += addFile (file, className, *header, *cpp);

                header->printf ("  #endif\r\n");
                cpp->printf ("#endif\r\n");
            }
            else
            {
                totalBytes += addFile (file, className, *header, *cpp);
            }
        }
    }

    header->printf ("};\r\n\r\n"
                    "#endif\r\n");

    delete header;
    delete cpp;

    printf ("\n Total size of binary data: %d bytes\n", totalBytes);

    shutdownJuce_NonGUI();

    return 0;
}
