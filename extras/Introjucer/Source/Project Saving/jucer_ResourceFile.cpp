/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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
#include "../Project/jucer_ProjectTreeViewBase.h"
#include "../Application/jucer_OpenDocumentManager.h"

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
    if (file.hasFileExtension ("cpp;cc;h"))
    {
        ScopedPointer <InputStream> in (file.createInputStream());

        if (in != nullptr)
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
            addFile (projectItem.getFile(), projectItem.getImageFileID());
    }
}

//==============================================================================
void ResourceFile::setClassName (const String& className_)
{
    className = className_;
}

void ResourceFile::addFile (const File& file, const String& imageProviderId)
{
    files.add (file);

    const String variableNameRoot (CodeHelpers::makeBinaryDataIdentifierName (file));
    String variableName (variableNameRoot);

    int suffix = 2;
    while (variableNames.contains (variableName))
        variableName = variableNameRoot + String (suffix++);

    if (imageProviderId.isNotEmpty())
        variableName << "|" << imageProviderId;

    variableNames.add (variableName);
}

String ResourceFile::getDataVariableFor (const File& file) const
{
    jassert (files.indexOf (file) >= 0);
    return variableNames [files.indexOf (file)];
}

String ResourceFile::getSizeVariableFor (const File& file) const
{
    jassert (files.indexOf (file) >= 0);
    return variableNames [files.indexOf (file)] + "Size";
}

int64 ResourceFile::getTotalDataSize() const
{
    int64 total = 0;

    for (int i = 0; i < files.size(); ++i)
        total += files.getReference(i).getSize();

    return total;
}

bool ResourceFile::write (const File& cppFile, OutputStream& cpp, OutputStream& header)
{
    String comment;
    comment << newLine << newLine
            << "   This is an auto-generated file: Any edits you make may be overwritten!" << newLine
            << newLine
            << "*/" << newLine
            << newLine;

    header << "/* ========================================================================================="
           << comment;

    cpp << "/* ==================================== " << resourceFileIdentifierString << " ===================================="
        << comment;

    const String namespaceName (className);

    cpp    << "namespace " << namespaceName << newLine << "{" << newLine;
    header << "namespace " << namespaceName << newLine << "{" << newLine;

    StringArray returnCodes;
    bool containsAnyImages = false;

    for (int i = 0; i < files.size(); ++i)
    {
        const File& file = files.getReference(i);
        const int64 dataSize = file.getSize();

        const String variableName (variableNames[i].upToFirstOccurrenceOf ("|", false, false));

        returnCodes.add ("numBytes = " + String (dataSize) + "; return " + variableName + ";");

        FileInputStream fileStream (file);
        jassert (fileStream.openedOk());

        if (fileStream.openedOk())
        {
            containsAnyImages = containsAnyImages
                                 || (ImageFileFormat::findImageFormatForStream (fileStream) != nullptr);

            const String tempVariable ("temp_" + String::toHexString (file.hashCode()));

            header << "    extern const char*   " << variableName << ";" << newLine;
            header << "    const int            " << variableName << "Size = " << (int) dataSize << ";" << newLine << newLine;

            cpp  << newLine << "//================== " << file.getFileName() << " ==================" << newLine
                << "static const unsigned char " << tempVariable << "[] =" << newLine;

            {
                MemoryBlock data;
                fileStream.readIntoMemoryBlock (data);
                CodeHelpers::writeDataAsCppLiteral (data, cpp, true, true);
            }

            cpp << newLine << newLine
                << "const char* " << variableName << " = (const char*) " << tempVariable << ";" << newLine;
        }
    }

    cpp << newLine
        << newLine
        << "const char* getNamedResource (const char*, int&) throw();" << newLine
        << "const char* getNamedResource (const char* resourceNameUTF8, int& numBytes) throw()" << newLine
        << "{" << newLine;

    CodeHelpers::createStringMatcher (cpp, "resourceNameUTF8", variableNames, returnCodes, 4);

    cpp << "    numBytes = 0;" << newLine
        << "    return 0;" << newLine
        << "}" << newLine
        << newLine
        << "}" << newLine;

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

    if (cppOut != nullptr && hppOut != nullptr)
    {
        if (write (cppFile, *cppOut, *hppOut))
        {
            cppOut = nullptr;
            hppOut = nullptr;

            return (tempCpp.getFile().hasIdenticalContentTo (tempCpp.getTargetFile()) || tempCpp.overwriteTargetFileWithTemporary())
                && (tempH.getFile().hasIdenticalContentTo (tempH.getTargetFile()) || tempH.overwriteTargetFileWithTemporary());
        }
    }

    return false;
}
