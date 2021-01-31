/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

namespace PropertyFileConstants
{
    constexpr static const int magicNumber            = (int) ByteOrder::makeInt ('P', 'R', 'O', 'P');
    constexpr static const int magicNumberCompressed  = (int) ByteOrder::makeInt ('C', 'P', 'R', 'P');

    constexpr static const char* const fileTag        = "PROPERTIES";
    constexpr static const char* const valueTag       = "VALUE";
    constexpr static const char* const nameAttribute  = "name";
    constexpr static const char* const valueAttribute = "val";
}

//==============================================================================
PropertiesFile::Options::Options()
    : commonToAllUsers (false),
      ignoreCaseOfKeyNames (false),
      doNotSave (false),
      millisecondsBeforeSaving (3000),
      storageFormat (PropertiesFile::storeAsXML),
      processLock (nullptr)
{
}

File PropertiesFile::Options::getDefaultFile() const
{
    // mustn't have illegal characters in this name..
    jassert (applicationName == File::createLegalFileName (applicationName));

   #if JUCE_MAC || JUCE_IOS
    File dir (commonToAllUsers ?  "/Library/"
                               : "~/Library/");

    if (osxLibrarySubFolder != "Preferences"
        && ! osxLibrarySubFolder.startsWith ("Application Support")
        && ! osxLibrarySubFolder.startsWith ("Containers"))
    {
        /* The PropertiesFile class always used to put its settings files in "Library/Preferences", but Apple
           have changed their advice, and now stipulate that settings should go in "Library/Application Support",
           or Library/Containers/[app_bundle_id] for a sandboxed app.

           Because older apps would be broken by a silent change in this class's behaviour, you must now
           explicitly set the osxLibrarySubFolder value to indicate which path you want to use.

           In newer apps, you should always set this to "Application Support"
           or "Application Support/YourSubFolderName".

           If your app needs to load settings files that were created by older versions of juce and
           you want to maintain backwards-compatibility, then you can set this to "Preferences".
           But.. for better Apple-compliance, the recommended approach would be to write some code that
           finds your old settings files in ~/Library/Preferences, moves them to ~/Library/Application Support,
           and then uses the new path.
        */
        jassertfalse;

        dir = dir.getChildFile ("Application Support");
    }
    else
    {
        dir = dir.getChildFile (osxLibrarySubFolder);
    }

    if (folderName.isNotEmpty())
        dir = dir.getChildFile (folderName);

   #elif JUCE_LINUX || JUCE_ANDROID
    auto dir = File (commonToAllUsers ? "/var" : "~")
                      .getChildFile (folderName.isNotEmpty() ? folderName
                                                             : ("." + applicationName));

   #elif JUCE_WINDOWS
    auto dir = File::getSpecialLocation (commonToAllUsers ? File::commonApplicationDataDirectory
                                                          : File::userApplicationDataDirectory);

    if (dir == File())
        return {};

    dir = dir.getChildFile (folderName.isNotEmpty() ? folderName
                                                    : applicationName);
   #endif

    return (filenameSuffix.startsWithChar (L'.')
               ? dir.getChildFile (applicationName).withFileExtension (filenameSuffix)
               : dir.getChildFile (applicationName + "." + filenameSuffix));
}


//==============================================================================
PropertiesFile::PropertiesFile (const File& f, const Options& o)
    : PropertySet (o.ignoreCaseOfKeyNames),
      file (f), options (o)
{
    reload();
}

PropertiesFile::PropertiesFile (const Options& o)
    : PropertySet (o.ignoreCaseOfKeyNames),
      file (o.getDefaultFile()), options (o)
{
    reload();
}

bool PropertiesFile::reload()
{
    ProcessScopedLock pl (createProcessLock());

    if (pl != nullptr && ! pl->isLocked())
        return false; // locking failure..

    loadedOk = (! file.exists()) || loadAsBinary() || loadAsXml();
    return loadedOk;
}

PropertiesFile::~PropertiesFile()
{
    saveIfNeeded();
}

InterProcessLock::ScopedLockType* PropertiesFile::createProcessLock() const
{
    return options.processLock != nullptr ? new InterProcessLock::ScopedLockType (*options.processLock) : nullptr;
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

    if (options.doNotSave
         || file == File()
         || file.isDirectory()
         || ! file.getParentDirectory().createDirectory())
        return false;

    if (options.storageFormat == storeAsXML)
        return saveAsXml();

    return saveAsBinary();
}

bool PropertiesFile::loadAsXml()
{
    if (auto doc = parseXMLIfTagMatches (file, PropertyFileConstants::fileTag))
    {
        for (auto* e : doc->getChildWithTagNameIterator (PropertyFileConstants::valueTag))
        {
            auto name = e->getStringAttribute (PropertyFileConstants::nameAttribute);

            if (name.isNotEmpty())
                getAllProperties().set (name,
                                        e->getFirstChildElement() != nullptr
                                            ? e->getFirstChildElement()->toString (XmlElement::TextFormat().singleLine().withoutHeader())
                                            : e->getStringAttribute (PropertyFileConstants::valueAttribute));
        }

        return true;
    }

    return false;
}

bool PropertiesFile::saveAsXml()
{
    XmlElement doc (PropertyFileConstants::fileTag);
    auto& props = getAllProperties();

    for (int i = 0; i < props.size(); ++i)
    {
        auto* e = doc.createNewChildElement (PropertyFileConstants::valueTag);
        e->setAttribute (PropertyFileConstants::nameAttribute, props.getAllKeys() [i]);

        // if the value seems to contain xml, store it as such..
        if (auto childElement = parseXML (props.getAllValues() [i]))
            e->addChildElement (childElement.release());
        else
            e->setAttribute (PropertyFileConstants::valueAttribute, props.getAllValues() [i]);
    }

    ProcessScopedLock pl (createProcessLock());

    if (pl != nullptr && ! pl->isLocked())
        return false; // locking failure..

    if (doc.writeTo (file, {}))
    {
        needsWriting = false;
        return true;
    }

    return false;
}

bool PropertiesFile::loadAsBinary()
{
    FileInputStream fileStream (file);

    if (fileStream.openedOk())
    {
        auto magicNumber = fileStream.readInt();

        if (magicNumber == PropertyFileConstants::magicNumberCompressed)
        {
            SubregionStream subStream (&fileStream, 4, -1, false);
            GZIPDecompressorInputStream gzip (subStream);
            return loadAsBinary (gzip);
        }

        if (magicNumber == PropertyFileConstants::magicNumber)
            return loadAsBinary (fileStream);
    }

    return false;
}

bool PropertiesFile::loadAsBinary (InputStream& input)
{
    BufferedInputStream in (input, 2048);

    int numValues = in.readInt();

    while (--numValues >= 0 && ! in.isExhausted())
    {
        auto key = in.readString();
        auto value = in.readString();
        jassert (key.isNotEmpty());

        if (key.isNotEmpty())
            getAllProperties().set (key, value);
    }

    return true;
}

bool PropertiesFile::saveAsBinary()
{
    ProcessScopedLock pl (createProcessLock());

    if (pl != nullptr && ! pl->isLocked())
        return false; // locking failure..

    TemporaryFile tempFile (file);

    {
        FileOutputStream out (tempFile.getFile());

        if (! out.openedOk())
            return false;

        if (options.storageFormat == storeAsCompressedBinary)
        {
            out.writeInt (PropertyFileConstants::magicNumberCompressed);
            out.flush();

            GZIPCompressorOutputStream zipped (out, 9);

            if (! writeToStream (zipped))
                return false;
        }
        else
        {
            // have you set up the storage option flags correctly?
            jassert (options.storageFormat == storeAsBinary);

            out.writeInt (PropertyFileConstants::magicNumber);

            if (! writeToStream (out))
                return false;
        }
    }

    if (! tempFile.overwriteTargetFileWithTemporary())
        return false;

    needsWriting = false;
    return true;
}

bool PropertiesFile::writeToStream (OutputStream& out)
{
    auto& props  = getAllProperties();
    auto& keys   = props.getAllKeys();
    auto& values = props.getAllValues();
    auto numProperties = props.size();

    if (! out.writeInt (numProperties))
        return false;

    for (int i = 0; i < numProperties; ++i)
    {
        if (! out.writeString (keys[i]))   return false;
        if (! out.writeString (values[i])) return false;
    }

    return true;
}

void PropertiesFile::timerCallback()
{
    saveIfNeeded();
}

void PropertiesFile::propertyChanged()
{
    sendChangeMessage();
    needsWriting = true;

    if (options.millisecondsBeforeSaving > 0)
        startTimer (options.millisecondsBeforeSaving);
    else if (options.millisecondsBeforeSaving == 0)
        saveIfNeeded();
}

} // namespace juce
