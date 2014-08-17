/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

/* Note: there's a bit of light obfuscation in this code, just to make things
   a bit more annoying for crackers who try to reverse-engineer your binaries, but
   nothing particularly foolproof.
*/

struct KeyFileUtils
{
    static String encryptXML (const XmlElement& xml, RSAKey privateKey)
    {
        MemoryOutputStream text;
        text << xml.createDocument (String::empty, true);

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
        lines.add (String::empty);

        const int charsPerLine = 70;
        while (asHex.length() > 0)
        {
            lines.add (asHex.substring (0, charsPerLine));
            asHex = asHex.substring (charsPerLine);
        }

        lines.add (String::empty);

        return lines.joinIntoString ("\r\n");
    }

    //==============================================================================
    static XmlElement decryptXML (String hexData, RSAKey rsaPublicKey)
    {
        BigInteger val;
        val.parseString (hexData, 16);

        RSAKey key (rsaPublicKey);
        key.applyToValue (val);

        ScopedPointer<XmlElement> xml (XmlDocument::parse (val.toMemoryBlock().toString()));

        return xml != nullptr ? *xml : XmlElement("key");
    }

    static XmlElement getXmlFromKeyFile (String keyFileText, RSAKey rsaPublicKey)
    {
        return decryptXML (keyFileText.fromLastOccurrenceOf ("#", false, false).trim(), rsaPublicKey);
    }

    static StringArray getMachineNumbers (XmlElement xml)
    {
        StringArray numbers;
        numbers.addTokens (xml.getStringAttribute ("mach"), ",; ", String::empty);
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
    };

    static KeyFileData getDataFromKeyFile (XmlElement xml)
    {
        KeyFileData data;

        data.licensee = getLicensee (xml);
        data.email = getEmail (xml);
        data.appID = getAppID (xml);
        data.machineNumbers.addArray (getMachineNumbers (xml));

        return data;
    }
};

//==============================================================================
const char* TracktionMarketplaceStatus::unlockedProp = "u";
static const char* stateTagName = "REG";
static const char* userNameProp = "user";
static const char* passwordProp = "pw";

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
TracktionMarketplaceStatus::TracktionMarketplaceStatus()  : status (stateTagName)
{
}

TracktionMarketplaceStatus::~TracktionMarketplaceStatus()
{
}

void TracktionMarketplaceStatus::load()
{
    MemoryBlock mb;
    mb.fromBase64Encoding (getState());

    if (mb.getSize() > 0)
        status = ValueTree::readFromGZIPData (mb.getData(), mb.getSize());
    else
        status = ValueTree (stateTagName);

    if (machineNumberAllowed (StringArray ("1234"), getLocalMachineIDs()))
        status.removeProperty (unlockedProp, nullptr);
}

void TracktionMarketplaceStatus::save()
{
    MemoryOutputStream mo;

    {
        GZIPCompressorOutputStream gzipStream (&mo, 9);
        status.writeToStream (gzipStream);
    }

    saveState (mo.getMemoryBlock().toBase64Encoding());
}

static String getEncodedIDString (const String& input)
{
    static const char* const platform =
       #if JUCE_MAC
        "M";
       #elif JUCE_WINDOWS
        "W";
       #elif JUCE_LINUX
        "L";
       #elif JUCE_IOS
        "I";
       #elif JUCE_ANDROID
        "A";
       #endif

    return platform + MD5 ((input + "salt_1" + platform).toUTF8())
                        .toHexString().substring (0, 9).toUpperCase();
}

StringArray TracktionMarketplaceStatus::getLocalMachineIDs()
{
    StringArray nums;

    // First choice for an ID number is a filesystem ID for the user's home
    // folder or windows directory.

   #if JUCE_WINDOWS
    uint64 num = File::getSpecialLocation (File::windowsSystemDirectory).getFileIdentifier();
   #else
    uint64 num = File ("~").getFileIdentifier();
   #endif

    if (num != 0)
    {
        nums.add (getEncodedIDString (String::toHexString ((int64) num)));
        return nums;
    }

    // ..if that fails, use the MAC addresses..

    Array<MACAddress> addresses;
    MACAddress::findAllAddresses (addresses);

    for (int i = 0; i < addresses.size(); ++i)
        nums.add (getEncodedIDString (addresses[i].toString()));

    jassert (nums.size() > 0); // failed to create any IDs!
    return nums;
}

URL TracktionMarketplaceStatus::getServerAuthenticationURL()
{
    return URL ("https://www.tracktion.com/marketplace/authenticate.php");
}

String TracktionMarketplaceStatus::getWebsiteName()
{
    return "tracktion.com";
}

void TracktionMarketplaceStatus::setUserEmail (const String& usernameOrEmail)
{
    status.setProperty (userNameProp, usernameOrEmail, nullptr);
}

String TracktionMarketplaceStatus::getUserEmail() const
{
    return status[userNameProp].toString();
}

bool TracktionMarketplaceStatus::applyKeyFile (String keyFileContent)
{
    KeyFileUtils::KeyFileData data;
    data = KeyFileUtils::getDataFromKeyFile (KeyFileUtils::getXmlFromKeyFile (keyFileContent, getPublicKey()));

    if (data.licensee.isNotEmpty() && data.email.isNotEmpty() && data.appID == getMarketplaceProductID())
    {
        setUserEmail (data.email);

        if (! isUnlocked())
        {
            var actualResult (0), dummyResult (1.0);
            var v (machineNumberAllowed (data.machineNumbers, getLocalMachineIDs()));
            actualResult.swapWith (v);
            v = machineNumberAllowed (StringArray ("01"), getLocalMachineIDs());
            dummyResult.swapWith (v);
            jassert (! dummyResult);

            if ((! dummyResult) && actualResult)
                status.setProperty (unlockedProp, actualResult, nullptr);
        }

        return true;
    }

    return false;
}

static bool canConnectToWebsite (const URL& url)
{
    ScopedPointer<InputStream> in (url.createInputStream (false, nullptr, nullptr, String(), 2000, nullptr));
    return in != nullptr;
}

static bool areMajorWebsitesAvailable()
{
    const char* urlsToTry[] = { "http://google.com", "http://bing.com", "http://amazon.com", nullptr};

    for (const char** url = urlsToTry; *url != nullptr; ++url)
        if (canConnectToWebsite (URL (*url)))
            return true;

    return false;
}

TracktionMarketplaceStatus::UnlockResult TracktionMarketplaceStatus::handleXmlReply (XmlElement xml)
{
    UnlockResult r;

    if (const XmlElement* keyNode = xml.getChildByName ("KEY"))
    {
        const String keyText (keyNode->getAllSubText().trim());
        r.succeeded = keyText.length() > 10 && applyKeyFile (keyText);
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

TracktionMarketplaceStatus::UnlockResult TracktionMarketplaceStatus::handleFailedConnection()
{
    UnlockResult r;

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

TracktionMarketplaceStatus::UnlockResult TracktionMarketplaceStatus::attemptWebserverUnlock (const String& email,
                                                                                             const String& password)
{
    // This method will block while it contacts the server, so you must run it on a background thread!
    jassert (! MessageManager::getInstance()->isThisTheMessageThread());

    URL url (getServerAuthenticationURL()
                .withParameter ("product", getMarketplaceProductID())
                .withParameter ("email", email)
                .withParameter ("pw", password)
                .withParameter ("os", SystemStats::getOperatingSystemName())
                .withParameter ("mach", getLocalMachineIDs()[0]));

    DBG ("Trying to unlock via URL: " << url.toString (true));

    const String reply (url.readEntireTextStream());

    DBG ("Reply from server: " << reply);

    ScopedPointer<XmlElement> xml (XmlDocument::parse (reply));

    if (xml != nullptr)
        return handleXmlReply (*xml);

    return handleFailedConnection();
}

//==============================================================================
String TracktionMarketplaceKeyGeneration::generateKeyFile (const String& appName,
                                                           const String& userEmail,
                                                           const String& userName,
                                                           const String& machineNumbers,
                                                           const RSAKey& privateKey)
{
    XmlElement xml ("key");

    xml.setAttribute ("user", userName);
    xml.setAttribute ("email", userEmail);
    xml.setAttribute ("mach", machineNumbers);
    xml.setAttribute ("app", appName);
    xml.setAttribute ("date", String::toHexString (Time::getCurrentTime().toMilliseconds()));

    String comment;
    comment << "Keyfile for " << appName << newLine;

    if (userName.isNotEmpty())
        comment << "User: " << userName << newLine;

    comment << "Email: " << userEmail << newLine
            << "Machine numbers: " << machineNumbers << newLine
            << "Created: " << Time::getCurrentTime().toString (true, true);

    return KeyFileUtils::createKeyFile (comment, xml, privateKey);
}
