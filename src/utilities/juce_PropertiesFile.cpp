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
#include "../memory/juce_ScopedPointer.h"
#include "../core/juce_SystemStats.h"
#include "../threads/juce_InterProcessLock.h"
#include "../threads/juce_ScopedLock.h"
#include "../text/juce_XmlDocument.h"


//==============================================================================
namespace PropertyFileConstants
{
    static const int magicNumber            = (int) ByteOrder::littleEndianInt ("PROP");
    static const int magicNumberCompressed  = (int) ByteOrder::littleEndianInt ("CPRP");

    static const char* const fileTag        = "PROPERTIES";
    static const char* const valueTag       = "VALUE";
    static const char* const nameAttribute  = "name";
    static const char* const valueAttribute = "val";
}

//==============================================================================
PropertiesFile::PropertiesFile (const File& f, const int millisecondsBeforeSaving,
                                const int options_, InterProcessLock* const processLock_)
    : PropertySet (ignoreCaseOfKeyNames),
      file (f),
      timerInterval (millisecondsBeforeSaving),
      options (options_),
      loadedOk (false),
      needsWriting (false),
      processLock (processLock_)
{
    // You need to correctly specify just one storage format for the file
    jassert ((options_ & (storeAsBinary | storeAsCompressedBinary | storeAsXML)) == storeAsBinary
             || (options_ & (storeAsBinary | storeAsCompressedBinary | storeAsXML)) == storeAsCompressedBinary
             || (options_ & (storeAsBinary | storeAsCompressedBinary | storeAsXML)) == storeAsXML);

    ProcessScopedLock pl (createProcessLock());

    if (pl != 0 && ! pl->isLocked())
        return; // locking failure..

    ScopedPointer<InputStream> fileStream (f.createInputStream());

    if (fileStream != 0)
    {
        int magicNumber = fileStream->readInt();

        if (magicNumber == PropertyFileConstants::magicNumberCompressed)
        {
            fileStream = new GZIPDecompressorInputStream (new SubregionStream (fileStream.release(), 4, -1, true), true);
            magicNumber = PropertyFileConstants::magicNumber;
        }

        if (magicNumber == PropertyFileConstants::magicNumber)
        {
            loadedOk = true;
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
            ScopedPointer<XmlElement> doc (parser.getDocumentElement (true));

            if (doc != 0 && doc->hasTagName (PropertyFileConstants::fileTag))
            {
                doc = parser.getDocumentElement();

                if (doc != 0)
                {
                    loadedOk = true;

                    forEachXmlChildElementWithTagName (*doc, e, PropertyFileConstants::valueTag)
                    {
                        const String name (e->getStringAttribute (PropertyFileConstants::nameAttribute));

                        if (name.isNotEmpty())
                        {
                            getAllProperties().set (name,
                                                    e->getFirstChildElement() != 0
                                                        ? e->getFirstChildElement()->createDocument (String::empty, true)
                                                        : e->getStringAttribute (PropertyFileConstants::valueAttribute));
                        }
                    }
                }
                else
                {
                    // must be a pretty broken XML file we're trying to parse here,
                    // or a sign that this object needs an InterProcessLock,
                    // or just a failure reading the file.  This last reason is why
                    // we don't jassertfalse here.
                }
            }
        }
    }
    else
    {
        loadedOk = ! f.exists();
    }
}

PropertiesFile::~PropertiesFile()
{
    if (! saveIfNeeded())
        jassertfalse;
}

InterProcessLock::ScopedLockType* PropertiesFile::createProcessLock() const
{
    return processLock != 0 ? new InterProcessLock::ScopedLockType (*processLock) : 0;
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

void PropertiesFile::setNeedsToBeSaved (const bool needsToBeSaved_)
{
    const ScopedLock sl (getLock());
    needsWriting = needsToBeSaved_;
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
        XmlElement doc (PropertyFileConstants::fileTag);

        for (int i = 0; i < getAllProperties().size(); ++i)
        {
            XmlElement* const e = doc.createNewChildElement (PropertyFileConstants::valueTag);
            e->setAttribute (PropertyFileConstants::nameAttribute, getAllProperties().getAllKeys() [i]);

            // if the value seems to contain xml, store it as such..
            XmlElement* const childElement = XmlDocument::parse (getAllProperties().getAllValues() [i]);

            if (childElement != 0)
                e->addChildElement (childElement);
            else
                e->setAttribute (PropertyFileConstants::valueAttribute,
                                 getAllProperties().getAllValues() [i]);
        }

        ProcessScopedLock pl (createProcessLock());

        if (pl != 0 && ! pl->isLocked())
            return false; // locking failure..

        if (doc.writeToFile (file, String::empty))
        {
            needsWriting = false;
            return true;
        }
    }
    else
    {
        ProcessScopedLock pl (createProcessLock());

        if (pl != 0 && ! pl->isLocked())
            return false; // locking failure..

        TemporaryFile tempFile (file);
        ScopedPointer <OutputStream> out (tempFile.getFile().createOutputStream());

        if (out != 0)
        {
            if ((options & storeAsCompressedBinary) != 0)
            {
                out->writeInt (PropertyFileConstants::magicNumberCompressed);
                out->flush();

                out = new GZIPCompressorOutputStream (out.release(), 9, true);
            }
            else
            {
                // have you set up the storage option flags correctly?
                jassert ((options & storeAsBinary) != 0);

                out->writeInt (PropertyFileConstants::magicNumber);
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
    sendChangeMessage();

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

#if JUCE_MAC || JUCE_IOS
    File dir (commonToAllUsers ? "/Library/Preferences"
                               : "~/Library/Preferences");

    if (folderName.isNotEmpty())
        dir = dir.getChildFile (folderName);
#endif

#ifdef JUCE_LINUX
    const File dir ((commonToAllUsers ? "/var/" : "~/")
                        + (folderName.isNotEmpty() ? folderName
                                                   : ("." + applicationName)));
#endif

#if JUCE_WINDOWS
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
                                                                const int propertiesFileOptions,
                                                                InterProcessLock* processLock_)
{
    const File file (getDefaultAppSettingsFile (applicationName,
                                                fileNameSuffix,
                                                folderName,
                                                commonToAllUsers));

    jassert (file != File::nonexistent);

    if (file == File::nonexistent)
        return 0;

    return new PropertiesFile (file, millisecondsBeforeSaving, propertiesFileOptions,processLock_);
}


END_JUCE_NAMESPACE
