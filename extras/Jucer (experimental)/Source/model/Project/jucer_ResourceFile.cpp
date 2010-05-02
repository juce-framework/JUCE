/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "jucer_ResourceFile.h"
#include "../../ui/Project Editor/jucer_ProjectTreeViewBase.h"
#include "../../ui/jucer_OpenDocumentManager.h"

static const char* resourceFileIdentifierString = "JUCER_BINARY_RESOURCE";


//==============================================================================
ResourceFile::ResourceFile (Project& project_)
    : project (project_),
      className ("BinaryData")
{
    addResourcesFromProjectItem (project.getMainGroup());
}

ResourceFile::~ResourceFile()
{
}

bool ResourceFile::isResourceFile (const File& file)
{
    if (file.hasFileExtension ("cpp;h"))
    {
        ScopedPointer <InputStream> in (file.createInputStream());

        if (in != 0)
        {
            MemoryBlock mb;
            in->readIntoMemoryBlock (mb, 256);
            return mb.toString().contains (resourceFileIdentifierString);
        }
    }

    return false;
}

//==============================================================================
void ResourceFile::addResourcesFromProjectItem (const Project::Item& projectItem)
{
    if (projectItem.isGroup())
    {
        for (int i = 0; i < projectItem.getNumChildren(); ++i)
            addResourcesFromProjectItem (projectItem.getChild(i));
    }
    else
    {
        if (projectItem.shouldBeAddedToBinaryResources())
            addFile (projectItem.getFile());
    }
}

//==============================================================================
void ResourceFile::setJuceHeaderToInclude (const File& header)
{
    juceHeader = header;
}

void ResourceFile::setClassName (const String& className_)
{
    className = className_;
}

void ResourceFile::addFile (const File& file)
{
    files.add (new File (file));
}

int64 ResourceFile::getTotalDataSize() const
{
    int64 total = 0;

    for (int i = 0; i < files.size(); ++i)
        total += files.getUnchecked(i)->getSize();

    return total;
}

static void writeCppData (InputStream& in, OutputStream& out)
{
    const int maxCharsOnLine = 250;

    MemoryBlock mb;
    in.readIntoMemoryBlock (mb);
    const unsigned char* data = (const unsigned char*) mb.getData();
    int charsOnLine = 0;

    bool canUseStringLiteral = mb.getSize() < 65535; // MS compilers can't handle strings bigger than 65536 chars..

    if (canUseStringLiteral)
    {
        for (size_t i = 0; i < mb.getSize(); ++i)
        {
            const unsigned int num = (unsigned int) data[i];
            if (! ((num >= 32 && num < 127) || num == '\t' || num == '\r' || num == '\n'))
            {
                canUseStringLiteral = false;
                break;
            }
        }
    }

    if (! canUseStringLiteral)
    {
        out << "{ ";

        for (size_t i = 0; i < mb.getSize(); ++i)
        {
            const int num = (int) (unsigned int) data[i];
            out << num << ',';

            charsOnLine += 2;
            if (num >= 10)
                ++charsOnLine;
            if (num >= 100)
                ++charsOnLine;

            if (charsOnLine >= maxCharsOnLine)
            {
                charsOnLine = 0;
                out << newLine;
            }
        }

        out << "0,0 };";
    }
    else
    {
        out << "\"";

        for (size_t i = 0; i < mb.getSize(); ++i)
        {
            const unsigned int num = (unsigned int) data[i];

            switch (num)
            {
                case '\t':      out << "\\t"; break;
                case '\r':      out << "\\r"; break;
                case '\n':      out << "\\n"; charsOnLine = maxCharsOnLine; break;
                case '"':       out << "\\\""; break;
                case '\\':      out << "\\\\"; break;
                default:        out << (char) num; break;
            }

            if (++charsOnLine >= maxCharsOnLine && i < mb.getSize() - 1)
            {
                charsOnLine = 0;
                out << "\"" << newLine << "\"";
            }
        }

        out << "\";";
    }
}

static int calcResourceHashCode (const String& s)
{
    const char* t = s.toUTF8();
    int hash = 0;
    while (*t != 0)
        hash = 31 * hash + *t++;

    return hash;
}

bool ResourceFile::write (const File& cppFile, OutputStream& cpp, OutputStream& header)
{
    String comment;
    comment << newLine << newLine
            << "   This is an auto-generated file, created by " << JUCEApplication::getInstance()->getApplicationName() << newLine
            << "   Do not edit anything in this file!" << newLine << newLine
            << "*/" << newLine << newLine;

    header << "/* ========================================================================================="
           << comment;

    cpp << "/* ==================================== " << resourceFileIdentifierString << " ===================================="
        << comment;

    if (juceHeader.exists())
        header << createIncludeStatement (juceHeader, cppFile) << newLine;

    const String namespaceName (className);
    StringArray variableNames;

    int i;
    for (i = 0; i < files.size(); ++i)
    {
        String variableNameRoot (makeValidCppIdentifier (files.getUnchecked(i)->getFileName()
                                                          .replaceCharacters (" .", "__")
                                                          .retainCharacters ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789"),
                                                         false, true, false));
        String variableName (variableNameRoot);

        int suffix = 2;
        while (variableNames.contains (variableName))
            variableName = variableNameRoot + String (suffix++);

        variableNames.add (variableName);
    }

    cpp << createIncludeStatement (cppFile.withFileExtension (".h"), cppFile) << newLine
        << newLine
        << newLine
        << "const char* " << namespaceName << "::getNamedResource (const char* resourceNameUTF8, int& numBytes) throw()" << newLine
        << "{" << newLine
        << "    int hash = 0;" << newLine
        << "    if (resourceNameUTF8 != 0)" << newLine
        << "        while (*resourceNameUTF8 != 0)" << newLine
        << "            hash = 31 * hash + *resourceNameUTF8++;" << newLine
        << newLine
        << "    switch (hash)" << newLine
        << "    {" << newLine;

    for (i = 0; i < files.size(); ++i)
    {
        cpp << "        case 0x" << hexString8Digits (calcResourceHashCode (variableNames[i]))
            << ":  numBytes = " << namespaceName << "::" << variableNames[i] << "Size; return "
            << namespaceName << "::" << variableNames[i] << ";" << newLine;
    }

    cpp << "        default: break;" << newLine
        << "    }" << newLine
        << newLine
        << "    numBytes = 0;" << newLine
        << "    return 0;" << newLine
        << "}" << newLine
        << newLine;

    header << "namespace " << namespaceName << newLine << "{" << newLine;

    for (i = 0; i < files.size(); ++i)
    {
        const File file (*files.getUnchecked(i));
        const int64 dataSize = file.getSize();

        ScopedPointer <InputStream> fileStream (file.createInputStream());
        jassert (fileStream != 0);

        if (fileStream != 0)
        {
            const String variableName (variableNames[i]);
            const String tempVariable ("temp_" + String::toHexString (file.hashCode()));

            header << "    extern const char*   " << variableName << ";" << newLine;
            header << "    const int            " << variableName << "Size = " << (int) dataSize << ";" << newLine << newLine;

            cpp  << newLine << "//================== " << file.getFileName() << " ==================" << newLine
                << "static const unsigned char " << tempVariable
                << "[] =" << newLine;

            writeCppData (*fileStream, cpp);

            cpp << newLine << newLine
                << "const char* " << namespaceName << "::" << variableName << " = (const char*) "
                << tempVariable << ";" << newLine;
        }
    }

    header << "    // If you provide the name of one of the binary resource variables above, this function will" << newLine
           << "    // return the corresponding data and its size (or a null pointer if the name isn't found)." << newLine
           << "    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) throw();" << newLine
           << "}" << newLine;

    return true;
}

bool ResourceFile::write (const File& cppFile)
{
    TemporaryFile tempH (cppFile.withFileExtension (".h"), TemporaryFile::useHiddenFile);
    TemporaryFile tempCpp (cppFile, TemporaryFile::useHiddenFile);

    ScopedPointer <FileOutputStream> cppOut (tempCpp.getFile().createOutputStream (32768));
    ScopedPointer <FileOutputStream> hppOut (tempH.getFile().createOutputStream (32768));

    if (cppOut != 0 && hppOut != 0)
    {
        if (write (cppFile, *cppOut, *hppOut))
        {
            cppOut = 0;
            hppOut = 0;

            return (areFilesIdentical (tempCpp.getFile(), tempCpp.getTargetFile()) || tempCpp.overwriteTargetFileWithTemporary())
                && (areFilesIdentical (tempH.getFile(), tempH.getTargetFile()) || tempH.overwriteTargetFileWithTemporary());
        }
    }

    return false;
}
