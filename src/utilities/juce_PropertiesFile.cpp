/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_PropertiesFile.h"
#include "../io/files/juce_TemporaryFile.h"
#include "../io/files/juce_FileInputStream.h"
#include "../io/files/juce_FileOutputStream.h"
#include "../io/streams/juce_BufferedInputStream.h"
#include "../io/streams/juce_SubregionStream.h"
#include "../io/streams/juce_GZIPDecompressorInputStream.h"
#include "../io/streams/juce_GZIPCompressorOutputStream.h"
#include "../core/juce_SystemStats.h"
#include "../text/juce_XmlDocument.h"


//==============================================================================
static const int propFileMagicNumber            = ((int) ByteOrder::littleEndianInt ("PROP"));
static const int propFileMagicNumberCompressed  = ((int) ByteOrder::littleEndianInt ("CPRP"));

static const tchar* const propertyFileXmlTag    = T("PROPERTIES");
static const tchar* const propertyTagName       = T("VALUE");

//==============================================================================
PropertiesFile::PropertiesFile (const File& f,
                                const int millisecondsBeforeSaving,
                                const int options_)
    : PropertySet (ignoreCaseOfKeyNames),
      file (f),
      timerInterval (millisecondsBeforeSaving),
      options (options_),
      needsWriting (false)
{
    // You need to correctly specify just one storage format for the file
    jassert ((options_ & (storeAsBinary | storeAsCompressedBinary | storeAsXML)) == storeAsBinary
             || (options_ & (storeAsBinary | storeAsCompressedBinary | storeAsXML)) == storeAsCompressedBinary
             || (options_ & (storeAsBinary | storeAsCompressedBinary | storeAsXML)) == storeAsXML);

    ScopedPointer <InputStream> fileStream (f.createInputStream());

    if (fileStream != 0)
    {
        int magicNumber = fileStream->readInt();

        if (magicNumber == propFileMagicNumberCompressed)
        {
            fileStream = new GZIPDecompressorInputStream (new SubregionStream (fileStream.release(), 4, -1, true),
                                                          true);

            magicNumber = propFileMagicNumber;
        }

        if (magicNumber == propFileMagicNumber)
        {
            BufferedInputStream in (fileStream.release(), 2048, true);

            int numValues = in.readInt();

            while (--numValues >= 0 && ! in.isExhausted())
            {
                const String key (in.readString());
                const String value (in.readString());

                jassert (key.isNotEmpty());
                if (key.isNotEmpty())
                    getAllProperties().set (key, value);
            }
        }
        else
        {
            // Not a binary props file - let's see if it's XML..
            fileStream = 0;

            XmlDocument parser (f);
            ScopedPointer <XmlElement> doc (parser.getDocumentElement (true));

            if (doc != 0 && doc->hasTagName (propertyFileXmlTag))
            {
                doc = parser.getDocumentElement();

                if (doc != 0)
                {
                    forEachXmlChildElementWithTagName (*doc, e, propertyTagName)
                    {
                        const String name (e->getStringAttribute (T("name")));

                        if (name.isNotEmpty())
                        {
                            getAllProperties().set (name,
                                                    e->getFirstChildElement() != 0
                                                        ? e->getFirstChildElement()->createDocument (String::empty, true)
                                                        : e->getStringAttribute (T("val")));
                        }
                    }
                }
                else
                {
                    // must be a pretty broken XML file we're trying to parse here!
                    jassertfalse
                }
            }
        }
    }
}

PropertiesFile::~PropertiesFile()
{
    saveIfNeeded();
}

bool PropertiesFile::saveIfNeeded()
{
    const ScopedLock sl (getLock());
    return (! needsWriting) || save();
}

bool PropertiesFile::needsToBeSaved() const
{
    const ScopedLock sl (getLock());
    return needsWriting;
}


bool PropertiesFile::save()
{
    const ScopedLock sl (getLock());

    stopTimer();

    if (file == File::nonexistent
         || file.isDirectory()
         || ! file.getParentDirectory().createDirectory())
        return false;

    if ((options & storeAsXML) != 0)
    {
        XmlElement doc (propertyFileXmlTag);

        for (int i = 0; i < getAllProperties().size(); ++i)
        {
            XmlElement* const e = new XmlElement (propertyTagName);
            e->setAttribute (T("name"), getAllProperties().getAllKeys() [i]);

            // if the value seems to contain xml, store it as such..
            XmlDocument xmlContent (getAllProperties().getAllValues() [i]);
            XmlElement* const childElement = xmlContent.getDocumentElement();

            if (childElement != 0)
                e->addChildElement (childElement);
            else
                e->setAttribute (T("val"), getAllProperties().getAllValues() [i]);

            doc.addChildElement (e);
        }

        return doc.writeToFile (file, String::empty);
    }
    else
    {
        TemporaryFile tempFile (file);

        ScopedPointer <OutputStream> out (tempFile.getFile().createOutputStream());

        if (out != 0)
        {
            if ((options & storeAsCompressedBinary) != 0)
            {
                out->writeInt (propFileMagicNumberCompressed);
                out->flush();

                out = new GZIPCompressorOutputStream (out.release(), 9, true);
            }
            else
            {
                // have you set up the storage option flags correctly?
                jassert ((options & storeAsBinary) != 0);

                out->writeInt (propFileMagicNumber);
            }

            const int numProperties = getAllProperties().size();

            out->writeInt (numProperties);

            for (int i = 0; i < numProperties; ++i)
            {
                out->writeString (getAllProperties().getAllKeys() [i]);
                out->writeString (getAllProperties().getAllValues() [i]);
            }

            out = 0;

            if (tempFile.overwriteTargetFileWithTemporary())
            {
                needsWriting = false;
                return true;
            }
        }
    }

    return false;
}

void PropertiesFile::timerCallback()
{
    saveIfNeeded();
}

void PropertiesFile::propertyChanged()
{
    sendChangeMessage (this);

    needsWriting = true;

    if (timerInterval > 0)
        startTimer (timerInterval);
    else if (timerInterval == 0)
        saveIfNeeded();
}

//==============================================================================
const File PropertiesFile::getDefaultAppSettingsFile (const String& applicationName,
                                                      const String& fileNameSuffix,
                                                      const String& folderName,
                                                      const bool commonToAllUsers)
{
    // mustn't have illegal characters in this name..
    jassert (applicationName == File::createLegalFileName (applicationName));

#if JUCE_MAC || JUCE_IPHONE
    File dir (commonToAllUsers ? "/Library/Preferences"
                               : "~/Library/Preferences");

    if (folderName.isNotEmpty())
        dir = dir.getChildFile (folderName);
#endif

#ifdef JUCE_LINUX
    const File dir ((commonToAllUsers ? T("/var/") : T("~/"))
                        + (folderName.isNotEmpty() ? folderName
                                                   : (T(".") + applicationName)));
#endif

#if JUCE_WIN32
    File dir (File::getSpecialLocation (commonToAllUsers ? File::commonApplicationDataDirectory
                                                         : File::userApplicationDataDirectory));

    if (dir == File::nonexistent)
        return File::nonexistent;

    dir = dir.getChildFile (folderName.isNotEmpty() ? folderName
                                                    : applicationName);

#endif

    return dir.getChildFile (applicationName)
              .withFileExtension (fileNameSuffix);
}

PropertiesFile* PropertiesFile::createDefaultAppPropertiesFile (const String& applicationName,
                                                                const String& fileNameSuffix,
                                                                const String& folderName,
                                                                const bool commonToAllUsers,
                                                                const int millisecondsBeforeSaving,
                                                                const int propertiesFileOptions)
{
    const File file (getDefaultAppSettingsFile (applicationName,
                                                fileNameSuffix,
                                                folderName,
                                                commonToAllUsers));

    jassert (file != File::nonexistent);

    if (file == File::nonexistent)
        return 0;

    return new PropertiesFile (file, millisecondsBeforeSaving, propertiesFileOptions);
}


END_JUCE_NAMESPACE
