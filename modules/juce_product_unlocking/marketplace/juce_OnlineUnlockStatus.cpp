/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/* Note: there's a bit of light obfuscation in this code, just to make things
   a bit more annoying for crackers who try to reverse-engineer your binaries, but
   nothing particularly foolproof.
*/

struct KeyFileUtils
{
    static XmlElement createKeyFileContent (const String& appName,
                                            const String& userEmail,
                                            const String& userName,
                                            const String& machineNumbers,
                                            const String& machineNumbersAttributeName)
    {
        XmlElement xml ("key");

        xml.setAttribute ("user", userName);
        xml.setAttribute ("email", userEmail);
        xml.setAttribute (machineNumbersAttributeName, machineNumbers);
        xml.setAttribute ("app", appName);
        xml.setAttribute ("date", String::toHexString (Time::getCurrentTime().toMilliseconds()));

        return xml;
    }

    static String createKeyFileComment (const String& appName,
                                        const String& userEmail,
                                        const String& userName,
                                        const String& machineNumbers)
    {
        String comment;
        comment << "Keyfile for " << appName << newLine;

        if (userName.isNotEmpty())
            comment << "User: " << userName << newLine;

        comment << "Email: " << userEmail << newLine
                << "Machine numbers: " << machineNumbers << newLine
                << "Created: " << Time::getCurrentTime().toString (true, true);

        return comment;
    }

    //==============================================================================
    static String encryptXML (const XmlElement& xml, RSAKey privateKey)
    {
        MemoryOutputStream text;
        text << xml.createDocument (StringRef(), true);

        BigInteger val;
        val.loadFromMemoryBlock (text.getMemoryBlock());

        privateKey.applyToValue (val);

        return val.toString (16);
    }

    static String createKeyFile (String comment,
                                 const XmlElement& xml,
                                 RSAKey rsaPrivateKey)
    {
        String asHex ("#" + encryptXML (xml, rsaPrivateKey));

        StringArray lines;
        lines.add (comment);
        lines.add (String());

        const int charsPerLine = 70;
        while (asHex.length() > 0)
        {
            lines.add (asHex.substring (0, charsPerLine));
            asHex = asHex.substring (charsPerLine);
        }

        lines.add (String());

        return lines.joinIntoString ("\r\n");
    }

    //==============================================================================
    static XmlElement decryptXML (String hexData, RSAKey rsaPublicKey)
    {
        BigInteger val;
        val.parseString (hexData, 16);

        RSAKey key (rsaPublicKey);
        jassert (key.isValid());

        std::unique_ptr<XmlElement> xml;

        if (! val.isZero())
        {
            key.applyToValue (val);

            const MemoryBlock mb (val.toMemoryBlock());

            if (CharPointer_UTF8::isValidString (static_cast<const char*> (mb.getData()), (int) mb.getSize()))
                xml.reset (XmlDocument::parse (mb.toString()));
        }

        return xml != nullptr ? *xml : XmlElement("key");
    }

    static XmlElement getXmlFromKeyFile (String keyFileText, RSAKey rsaPublicKey)
    {
        return decryptXML (keyFileText.fromLastOccurrenceOf ("#", false, false).trim(), rsaPublicKey);
    }

    static StringArray getMachineNumbers (XmlElement xml, StringRef attributeName)
    {
        StringArray numbers;
        numbers.addTokens (xml.getStringAttribute (attributeName), ",; ", StringRef());
        numbers.trim();
        numbers.removeEmptyStrings();
        return numbers;
    }

    static String getLicensee (const XmlElement& xml)       { return xml.getStringAttribute ("user"); }
    static String getEmail (const XmlElement& xml)          { return xml.getStringAttribute ("email"); }
    static String getAppID (const XmlElement& xml)          { return xml.getStringAttribute ("app"); }

    struct KeyFileData
    {
        String licensee, email, appID;
        StringArray machineNumbers;

        bool keyFileExpires;
        Time expiryTime;
    };

    static KeyFileData getDataFromKeyFile (XmlElement xml)
    {
        KeyFileData data;

        data.licensee = getLicensee (xml);
        data.email = getEmail (xml);
        data.appID = getAppID (xml);

        if (xml.hasAttribute ("expiryTime") && xml.hasAttribute ("expiring_mach"))
        {
            data.keyFileExpires = true;
            data.machineNumbers.addArray (getMachineNumbers (xml, "expiring_mach"));
            data.expiryTime = Time (xml.getStringAttribute ("expiryTime").getHexValue64());
        }
        else
        {
            data.keyFileExpires = false;
            data.machineNumbers.addArray (getMachineNumbers (xml, "mach"));
        }

        return data;
    }
};

//==============================================================================
#if JUCE_MODULE_AVAILABLE_juce_data_structures
const char* OnlineUnlockStatus::unlockedProp = "u";
const char* OnlineUnlockStatus::expiryTimeProp = "t";
static const char* stateTagName = "REG";
static const char* userNameProp = "user";
static const char* keyfileDataProp = "key";

static var machineNumberAllowed (StringArray numbersFromKeyFile,
                                 StringArray localMachineNumbers)
{
    var result;

    for (int i = 0; i < localMachineNumbers.size(); ++i)
    {
        String localNumber (localMachineNumbers[i].trim());

        if (localNumber.isNotEmpty())
        {
            for (int j = numbersFromKeyFile.size(); --j >= 0;)
            {
                var ok (localNumber.trim().equalsIgnoreCase (numbersFromKeyFile[j].trim()));
                result.swapWith (ok);

                if (result)
                    break;
            }
        }
    }

    return result;
}

//==============================================================================
OnlineUnlockStatus::OnlineUnlockStatus()  : status (stateTagName)
{
}

OnlineUnlockStatus::~OnlineUnlockStatus()
{
}

void OnlineUnlockStatus::load()
{
    MemoryBlock mb;
    mb.fromBase64Encoding (getState());

    if (mb.getSize() > 0)
        status = ValueTree::readFromGZIPData (mb.getData(), mb.getSize());
    else
        status = ValueTree (stateTagName);

    StringArray localMachineNums (getLocalMachineIDs());

    if (machineNumberAllowed (StringArray ("1234"), localMachineNums))
        status.removeProperty (unlockedProp, nullptr);

    KeyFileUtils::KeyFileData data;
    data = KeyFileUtils::getDataFromKeyFile (KeyFileUtils::getXmlFromKeyFile (status[keyfileDataProp], getPublicKey()));

    if (data.keyFileExpires)
    {
        if (! doesProductIDMatch (data.appID))
            status.removeProperty (expiryTimeProp, nullptr);

        if (! machineNumberAllowed (data.machineNumbers, localMachineNums))
            status.removeProperty (expiryTimeProp, nullptr);
    }
    else
    {
        if (! doesProductIDMatch (data.appID))
            status.removeProperty (unlockedProp, nullptr);

        if (! machineNumberAllowed (data.machineNumbers, localMachineNums))
            status.removeProperty (unlockedProp, nullptr);
    }
}

void OnlineUnlockStatus::save()
{
    MemoryOutputStream mo;

    {
        GZIPCompressorOutputStream gzipStream (mo, 9);
        status.writeToStream (gzipStream);
    }

    saveState (mo.getMemoryBlock().toBase64Encoding());
}

char OnlineUnlockStatus::MachineIDUtilities::getPlatformPrefix()
{
   #if JUCE_MAC
    return 'M';
   #elif JUCE_WINDOWS
    return 'W';
   #elif JUCE_LINUX
    return 'L';
   #elif JUCE_IOS
    return 'I';
   #elif JUCE_ANDROID
    return 'A';
   #endif
}

String OnlineUnlockStatus::MachineIDUtilities::getEncodedIDString (const String& input)
{
    const String platform (String::charToString (static_cast<juce_wchar> (getPlatformPrefix())));

    return platform + MD5 ((input + "salt_1" + platform).toUTF8())
                        .toHexString().substring (0, 9).toUpperCase();
}

bool OnlineUnlockStatus::MachineIDUtilities::addFileIDToList (StringArray& ids, const File& f)
{
    if (uint64 num = f.getFileIdentifier())
    {
        ids.add (getEncodedIDString (String::toHexString ((int64) num)));
        return true;
    }

    return false;
}

void OnlineUnlockStatus::MachineIDUtilities::addMACAddressesToList (StringArray& ids)
{
    Array<MACAddress> addresses;
    MACAddress::findAllAddresses (addresses);

    for (int i = 0; i < addresses.size(); ++i)
        ids.add (getEncodedIDString (addresses.getReference(i).toString()));
}

StringArray OnlineUnlockStatus::MachineIDUtilities::getLocalMachineIDs()
{
    auto identifiers = SystemStats::getDeviceIdentifiers();
    for (auto& identifier : identifiers)
        identifier = getEncodedIDString (identifier);

    return identifiers;
}

StringArray OnlineUnlockStatus::getLocalMachineIDs()
{
    return MachineIDUtilities::getLocalMachineIDs();
}

void OnlineUnlockStatus::userCancelled()
{
}

void OnlineUnlockStatus::setUserEmail (const String& usernameOrEmail)
{
    status.setProperty (userNameProp, usernameOrEmail, nullptr);
}

String OnlineUnlockStatus::getUserEmail() const
{
    return status[userNameProp].toString();
}

bool OnlineUnlockStatus::applyKeyFile (String keyFileContent)
{
    KeyFileUtils::KeyFileData data;
    data = KeyFileUtils::getDataFromKeyFile (KeyFileUtils::getXmlFromKeyFile (keyFileContent, getPublicKey()));

    if (data.licensee.isNotEmpty() && data.email.isNotEmpty() && doesProductIDMatch (data.appID))
    {
        setUserEmail (data.email);
        status.setProperty (keyfileDataProp, keyFileContent, nullptr);
        status.removeProperty (data.keyFileExpires ? expiryTimeProp : unlockedProp, nullptr);

        var actualResult (0), dummyResult (1.0);
        var v (machineNumberAllowed (data.machineNumbers, getLocalMachineIDs()));
        actualResult.swapWith (v);
        v = machineNumberAllowed (StringArray ("01"), getLocalMachineIDs());
        dummyResult.swapWith (v);
        jassert (! dummyResult);

        if (data.keyFileExpires)
        {
            if ((! dummyResult) && actualResult)
                status.setProperty (expiryTimeProp, data.expiryTime.toMilliseconds(), nullptr);

            return getExpiryTime().toMilliseconds() > 0;
        }

        if ((! dummyResult) && actualResult)
            status.setProperty (unlockedProp, actualResult, nullptr);

        return isUnlocked();
    }

    return false;
}

static bool canConnectToWebsite (const URL& url)
{
    std::unique_ptr<InputStream> in (url.createInputStream (false, nullptr, nullptr, String(), 2000, nullptr));
    return in != nullptr;
}

static bool areMajorWebsitesAvailable()
{
    const char* urlsToTry[] = { "http://google.com",  "http://bing.com",  "http://amazon.com",
                                "https://google.com", "https://bing.com", "https://amazon.com", nullptr};

    for (const char** url = urlsToTry; *url != nullptr; ++url)
        if (canConnectToWebsite (URL (*url)))
            return true;

    return false;
}

OnlineUnlockStatus::UnlockResult OnlineUnlockStatus::handleXmlReply (XmlElement xml)
{
    UnlockResult r;

    if (const XmlElement* keyNode = xml.getChildByName ("KEY"))
    {
        const String keyText (keyNode->getAllSubText().trim());
        r.succeeded = keyText.length() > 10 && applyKeyFile (keyText);
    }
    else
    {
        r.succeeded = false;
    }

    if (xml.hasTagName ("MESSAGE"))
        r.informativeMessage = xml.getStringAttribute ("message").trim();

    if (xml.hasTagName ("ERROR"))
        r.errorMessage = xml.getStringAttribute ("error").trim();

    if (xml.getStringAttribute ("url").isNotEmpty())
        r.urlToLaunch = xml.getStringAttribute ("url").trim();

    if (r.errorMessage.isEmpty() && r.informativeMessage.isEmpty() && r.urlToLaunch.isEmpty() && ! r.succeeded)
        r.errorMessage = TRANS ("Unexpected or corrupted reply from XYZ").replace ("XYZ", getWebsiteName()) + "...\n\n"
                            + TRANS("Please try again in a few minutes, and contact us for support if this message appears again.");

    return r;
}

OnlineUnlockStatus::UnlockResult OnlineUnlockStatus::handleFailedConnection()
{
    UnlockResult r;
    r.succeeded = false;

    r.errorMessage = TRANS("Couldn't connect to XYZ").replace ("XYZ", getWebsiteName()) + "...\n\n";

    if (areMajorWebsitesAvailable())
        r.errorMessage << TRANS("Your internet connection seems to be OK, but our webserver "
                                "didn't respond... This is most likely a temporary problem, so try "
                                "again in a few minutes, but if it persists, please contact us for support!");
    else
        r.errorMessage << TRANS("No internet sites seem to be accessible from your computer.. Before trying again, "
                                "please check that your network is working correctly, and make sure "
                                "that any firewall/security software installed on your machine isn't "
                                "blocking your web connection.");

    return r;
}

OnlineUnlockStatus::UnlockResult OnlineUnlockStatus::attemptWebserverUnlock (const String& email,
                                                                             const String& password)
{
    // This method will block while it contacts the server, so you must run it on a background thread!
    jassert (! MessageManager::getInstance()->isThisTheMessageThread());

    String reply (readReplyFromWebserver (email, password));

    DBG ("Reply from server: " << reply);

    std::unique_ptr<XmlElement> xml (XmlDocument::parse (reply));

    if (xml != nullptr)
        return handleXmlReply (*xml);

    return handleFailedConnection();
}

#endif // JUCE_MODULE_AVAILABLE_juce_data_structures

//==============================================================================
String KeyGeneration::generateKeyFile (const String& appName,
                                       const String& userEmail,
                                       const String& userName,
                                       const String& machineNumbers,
                                       const RSAKey& privateKey)
{
    XmlElement xml (KeyFileUtils::createKeyFileContent (appName, userEmail, userName, machineNumbers, "mach"));
    const String comment (KeyFileUtils::createKeyFileComment (appName, userEmail, userName, machineNumbers));

    return KeyFileUtils::createKeyFile (comment, xml, privateKey);
}

String KeyGeneration::generateExpiringKeyFile (const String& appName,
                                               const String& userEmail,
                                               const String& userName,
                                               const String& machineNumbers,
                                               const Time expiryTime,
                                               const RSAKey& privateKey)
{
    XmlElement xml (KeyFileUtils::createKeyFileContent (appName, userEmail, userName, machineNumbers, "expiring_mach"));
    xml.setAttribute ("expiryTime", String::toHexString (expiryTime.toMilliseconds()));

    String comment (KeyFileUtils::createKeyFileComment (appName, userEmail, userName, machineNumbers));
    comment << newLine << "Expires: " << expiryTime.toString (true, true);

    return KeyFileUtils::createKeyFile (comment, xml, privateKey);
}

} // namespace juce
