/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/* Note: There's a bit of light obfuscation in this code, just to make things
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
        text << xml.toString (XmlElement::TextFormat().singleLine());

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

            auto mb = val.toMemoryBlock();

            if (CharPointer_UTF8::isValidString (static_cast<const char*> (mb.getData()), (int) mb.getSize()))
                xml = parseXML (mb.toString());
        }

        return xml != nullptr ? *xml : XmlElement ("key");
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
        auto localNumber = localMachineNumbers[i].trim();

        if (localNumber.isNotEmpty())
        {
            for (int j = numbersFromKeyFile.size(); --j >= 0;)
            {
                var ok (localNumber.trim().equalsIgnoreCase (numbersFromKeyFile[j].trim()));
                result.swapWith (ok);

                if (result)
                    break;
            }

            if (result)
                break;
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

    if (! mb.isEmpty())
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
   #elif JUCE_BSD
    return 'B';
   #elif JUCE_IOS
    return 'I';
   #elif JUCE_ANDROID
    return 'A';
   #endif
}

String OnlineUnlockStatus::MachineIDUtilities::getEncodedIDString (const String& input)
{
    auto platform = String::charToString (static_cast<juce_wchar> (getPlatformPrefix()));

    return platform + MD5 ((input + "salt_1" + platform).toUTF8())
                        .toHexString().substring (0, 9).toUpperCase();
}

bool OnlineUnlockStatus::MachineIDUtilities::addFileIDToList (StringArray& ids, const File& f)
{
    if (auto num = f.getFileIdentifier())
    {
        ids.add (getEncodedIDString (String::toHexString ((int64) num)));
        return true;
    }

    return false;
}

void OnlineUnlockStatus::MachineIDUtilities::addMACAddressesToList (StringArray& ids)
{
    for (auto& address : MACAddress::getAllAddresses())
        ids.add (getEncodedIDString (address.toString()));
}

String OnlineUnlockStatus::MachineIDUtilities::getUniqueMachineID()
{
    return getEncodedIDString (SystemStats::getUniqueDeviceID());
}

JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS

StringArray OnlineUnlockStatus::MachineIDUtilities::getLocalMachineIDs()
{
    auto flags = SystemStats::MachineIdFlags::macAddresses
               | SystemStats::MachineIdFlags::fileSystemId
               | SystemStats::MachineIdFlags::legacyUniqueId
               | SystemStats::MachineIdFlags::uniqueId;
    auto identifiers = SystemStats::getMachineIdentifiers (flags);

    for (auto& identifier : identifiers)
        identifier = getEncodedIDString (identifier);

    return identifiers;
}

StringArray OnlineUnlockStatus::getLocalMachineIDs()
{
    return MachineIDUtilities::getLocalMachineIDs();
}

JUCE_END_IGNORE_DEPRECATION_WARNINGS

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

Result OnlineUnlockStatus::applyKeyFile (const String& keyFileContent)
{
    KeyFileUtils::KeyFileData data;
    data = KeyFileUtils::getDataFromKeyFile (KeyFileUtils::getXmlFromKeyFile (keyFileContent, getPublicKey()));

    if (data.licensee.isEmpty() || data.email.isEmpty())
        return Result::fail (LicenseResult::badCredentials);

    if (! doesProductIDMatch (data.appID))
        return Result::fail (LicenseResult::badProductID);

    if (MachineIDUtilities::getUniqueMachineID().isEmpty())
        return Result::fail (LicenseResult::notReady);

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

        return getExpiryTime().toMilliseconds() > 0 ? Result::ok()
                                                    : Result::fail (LicenseResult::licenseExpired);
    }

    if ((! dummyResult) && actualResult)
        status.setProperty (unlockedProp, actualResult, nullptr);

    return isUnlocked() ? Result::ok()
                        : Result::fail (LicenseResult::unlockFailed);
}

static bool areMajorWebsitesAvailable()
{
    static constexpr const char* const urlsToTry[] = { "http://google.com",  "http://bing.com",  "http://amazon.com",
                                                       "https://google.com", "https://bing.com", "https://amazon.com" };
    const auto canConnectToWebsite = [] (auto url)
    {
        return URL (url).createInputStream (URL::InputStreamOptions (URL::ParameterHandling::inAddress)
                                                .withConnectionTimeoutMs (2000)) != nullptr;
    };

    return std::any_of (std::begin (urlsToTry),
                        std::end   (urlsToTry),
                        canConnectToWebsite);
}

OnlineUnlockStatus::UnlockResult OnlineUnlockStatus::handleXmlReply (XmlElement xml)
{
    UnlockResult r;

    r.succeeded = false;

    if (const auto keyNode = xml.getChildByName ("KEY"))
    {
        if (const auto keyText = keyNode->getAllSubText().trim(); keyText.length() > 10)
        {
            const auto keyFileResult = applyKeyFile (keyText);

            if (keyFileResult.failed())
            {
                r.errorMessage = keyFileResult.getErrorMessage();
                return r;
            }

            r.succeeded = true;
        }
    }

    if (xml.hasTagName ("MESSAGE"))
        r.informativeMessage = xml.getStringAttribute ("message").trim();

    if (xml.hasTagName ("ERROR"))
        r.errorMessage = xml.getStringAttribute ("error").trim();

    if (xml.getStringAttribute ("url").isNotEmpty())
        r.urlToLaunch = xml.getStringAttribute ("url").trim();

    if (r.errorMessage.isEmpty() && r.informativeMessage.isEmpty() && r.urlToLaunch.isEmpty() && ! r.succeeded)
        r.errorMessage = getMessageForUnexpectedReply();

    return r;
}

OnlineUnlockStatus::UnlockResult OnlineUnlockStatus::handleFailedConnection()
{
    UnlockResult r;
    r.succeeded = false;
    r.errorMessage = getMessageForConnectionFailure (areMajorWebsitesAvailable());
    return r;
}

String OnlineUnlockStatus::getMessageForConnectionFailure (bool isInternetConnectionWorking)
{
    String message = TRANS ("Couldn't connect to XYZ").replace ("XYZ", getWebsiteName()) + "...\n\n";

    if (isInternetConnectionWorking)
        message << TRANS ("Your internet connection seems to be OK, but our webserver "
                          "didn't respond... This is most likely a temporary problem, so try "
                          "again in a few minutes, but if it persists, please contact us for support!");
    else
        message << TRANS ("No internet sites seem to be accessible from your computer.. Before trying again, "
                          "please check that your network is working correctly, and make sure "
                          "that any firewall/security software installed on your machine isn't "
                          "blocking your web connection.");

    return message;
}

String OnlineUnlockStatus::getMessageForUnexpectedReply()
{
    return TRANS ("Unexpected or corrupted reply from XYZ").replace ("XYZ", getWebsiteName()) + "...\n\n"
                    + TRANS ("Please try again in a few minutes, and contact us for support if this message appears again.");
}

OnlineUnlockStatus::UnlockResult OnlineUnlockStatus::attemptWebserverUnlock (const String& email,
                                                                             const String& password)
{
    // This method will block while it contacts the server, so you must run it on a background thread!
    jassert (! MessageManager::getInstance()->isThisTheMessageThread());

    auto reply = readReplyFromWebserver (email, password);

    DBG ("Reply from server: " << reply);

    if (auto xml = parseXML (reply))
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
    auto xml = KeyFileUtils::createKeyFileContent (appName, userEmail, userName, machineNumbers, "mach");
    auto comment = KeyFileUtils::createKeyFileComment (appName, userEmail, userName, machineNumbers);

    return KeyFileUtils::createKeyFile (comment, xml, privateKey);
}

String KeyGeneration::generateExpiringKeyFile (const String& appName,
                                               const String& userEmail,
                                               const String& userName,
                                               const String& machineNumbers,
                                               const Time expiryTime,
                                               const RSAKey& privateKey)
{
    auto xml = KeyFileUtils::createKeyFileContent (appName, userEmail, userName, machineNumbers, "expiring_mach");
    xml.setAttribute ("expiryTime", String::toHexString (expiryTime.toMilliseconds()));

    auto comment = KeyFileUtils::createKeyFileComment (appName, userEmail, userName, machineNumbers);
    comment << newLine << "Expires: " << expiryTime.toString (true, true);

    return KeyFileUtils::createKeyFile (comment, xml, privateKey);
}

} // namespace juce
