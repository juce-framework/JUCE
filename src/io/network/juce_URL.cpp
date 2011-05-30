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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_URL.h"
#include "../streams/juce_InputStream.h"
#include "../../maths/juce_Random.h"
#include "../../core/juce_PlatformUtilities.h"
#include "../../text/juce_XmlDocument.h"
#include "../../io/streams/juce_MemoryOutputStream.h"


//==============================================================================
URL::URL()
{
}

URL::URL (const String& url_)
    : url (url_)
{
    int i = url.indexOfChar ('?');

    if (i >= 0)
    {
        do
        {
            const int nextAmp   = url.indexOfChar (i + 1, '&');
            const int equalsPos = url.indexOfChar (i + 1, '=');

            if (equalsPos > i + 1)
            {
                if (nextAmp < 0)
                {
                    parameters.set (removeEscapeChars (url.substring (i + 1, equalsPos)),
                                    removeEscapeChars (url.substring (equalsPos + 1)));
                }
                else if (nextAmp > 0 && equalsPos < nextAmp)
                {
                    parameters.set (removeEscapeChars (url.substring (i + 1, equalsPos)),
                                    removeEscapeChars (url.substring (equalsPos + 1, nextAmp)));
                }
            }

            i = nextAmp;
        }
        while (i >= 0);

        url = url.upToFirstOccurrenceOf ("?", false, false);
    }
}

URL::URL (const URL& other)
    : url (other.url),
      postData (other.postData),
      parameters (other.parameters),
      filesToUpload (other.filesToUpload),
      mimeTypes (other.mimeTypes)
{
}

URL& URL::operator= (const URL& other)
{
    url = other.url;
    postData = other.postData;
    parameters = other.parameters;
    filesToUpload = other.filesToUpload;
    mimeTypes = other.mimeTypes;

    return *this;
}

URL::~URL()
{
}

namespace URLHelpers
{
    String getMangledParameters (const StringPairArray& parameters)
    {
        String p;

        for (int i = 0; i < parameters.size(); ++i)
        {
            if (i > 0)
                p << '&';

            p << URL::addEscapeChars (parameters.getAllKeys() [i], true)
              << '='
              << URL::addEscapeChars (parameters.getAllValues() [i], true);
        }

        return p;
    }

    int findStartOfDomain (const String& url)
    {
        int i = 0;

        while (CharacterFunctions::isLetterOrDigit (url[i])
                || url[i] == '+' || url[i] == '-' || url[i] == '.')
            ++i;

        return url[i] == ':' ? i + 1 : 0;
    }

    void createHeadersAndPostData (const URL& url, String& headers, MemoryBlock& postData)
    {
        MemoryOutputStream data (postData, false);

        if (url.getFilesToUpload().size() > 0)
        {
            // need to upload some files, so do it as multi-part...
            const String boundary (String::toHexString (Random::getSystemRandom().nextInt64()));

            headers << "Content-Type: multipart/form-data; boundary=" << boundary << "\r\n";

            data << "--" << boundary;

            int i;
            for (i = 0; i < url.getParameters().size(); ++i)
            {
                data << "\r\nContent-Disposition: form-data; name=\""
                     << url.getParameters().getAllKeys() [i]
                     << "\"\r\n\r\n"
                     << url.getParameters().getAllValues() [i]
                     << "\r\n--"
                     << boundary;
            }

            for (i = 0; i < url.getFilesToUpload().size(); ++i)
            {
                const File file (url.getFilesToUpload().getAllValues() [i]);
                const String paramName (url.getFilesToUpload().getAllKeys() [i]);

                data << "\r\nContent-Disposition: form-data; name=\"" << paramName
                     << "\"; filename=\"" << file.getFileName() << "\"\r\n";

                const String mimeType (url.getMimeTypesOfUploadFiles()
                                          .getValue (paramName, String::empty));

                if (mimeType.isNotEmpty())
                    data << "Content-Type: " << mimeType << "\r\n";

                data << "Content-Transfer-Encoding: binary\r\n\r\n"
                     << file << "\r\n--" << boundary;
            }

            data << "--\r\n";
            data.flush();
        }
        else
        {
            data << getMangledParameters (url.getParameters()) << url.getPostData();
            data.flush();

            // just a short text attachment, so use simple url encoding..
            headers << "Content-Type: application/x-www-form-urlencoded\r\nContent-length: "
                    << (int) postData.getSize() << "\r\n";
        }
    }
}

String URL::toString (const bool includeGetParameters) const
{
    if (includeGetParameters && parameters.size() > 0)
        return url + "?" + URLHelpers::getMangledParameters (parameters);
    else
        return url;
}

bool URL::isWellFormed() const
{
    //xxx TODO
    return url.isNotEmpty();
}

String URL::getDomain() const
{
    int start = URLHelpers::findStartOfDomain (url);
    while (url[start] == '/')
        ++start;

    const int end1 = url.indexOfChar (start, '/');
    const int end2 = url.indexOfChar (start, ':');

    const int end = (end1 < 0 || end2 < 0) ? jmax (end1, end2)
                                           : jmin (end1, end2);

    return url.substring (start, end);
}

String URL::getSubPath() const
{
    int start = URLHelpers::findStartOfDomain (url);
    while (url[start] == '/')
        ++start;

    const int startOfPath = url.indexOfChar (start, '/') + 1;

    return startOfPath <= 0 ? String::empty
                            : url.substring (startOfPath);
}

String URL::getScheme() const
{
    return url.substring (0, URLHelpers::findStartOfDomain (url) - 1);
}

const URL URL::withNewSubPath (const String& newPath) const
{
    int start = URLHelpers::findStartOfDomain (url);
    while (url[start] == '/')
        ++start;

    const int startOfPath = url.indexOfChar (start, '/') + 1;

    URL u (*this);

    if (startOfPath > 0)
        u.url = url.substring (0, startOfPath);

    if (! u.url.endsWithChar ('/'))
        u.url << '/';

    if (newPath.startsWithChar ('/'))
        u.url << newPath.substring (1);
    else
        u.url << newPath;

    return u;
}

//==============================================================================
bool URL::isProbablyAWebsiteURL (const String& possibleURL)
{
    const char* validProtocols[] = { "http:", "ftp:", "https:" };

    for (int i = 0; i < numElementsInArray (validProtocols); ++i)
        if (possibleURL.startsWithIgnoreCase (validProtocols[i]))
            return true;

    if (possibleURL.containsChar ('@')
         || possibleURL.containsChar (' '))
        return false;

    const String topLevelDomain (possibleURL.upToFirstOccurrenceOf ("/", false, false)
                                            .fromLastOccurrenceOf (".", false, false));

    return topLevelDomain.isNotEmpty() && topLevelDomain.length() <= 3;
}

bool URL::isProbablyAnEmailAddress (const String& possibleEmailAddress)
{
    const int atSign = possibleEmailAddress.indexOfChar ('@');

    return atSign > 0
            && possibleEmailAddress.lastIndexOfChar ('.') > (atSign + 1)
            && (! possibleEmailAddress.endsWithChar ('.'));
}

//==============================================================================
InputStream* URL::createInputStream (const bool usePostCommand,
                                     OpenStreamProgressCallback* const progressCallback,
                                     void* const progressCallbackContext,
                                     const String& extraHeaders,
                                     const int timeOutMs,
                                     StringPairArray* const responseHeaders) const
{
    String headers;
    MemoryBlock headersAndPostData;

    if (usePostCommand)
        URLHelpers::createHeadersAndPostData (*this, headers, headersAndPostData);

    headers += extraHeaders;

    if (! headers.endsWithChar ('\n'))
        headers << "\r\n";

    return createNativeStream (toString (! usePostCommand), usePostCommand, headersAndPostData,
                               progressCallback, progressCallbackContext,
                               headers, timeOutMs, responseHeaders);
}

//==============================================================================
bool URL::readEntireBinaryStream (MemoryBlock& destData,
                                  const bool usePostCommand) const
{
    const ScopedPointer <InputStream> in (createInputStream (usePostCommand));

    if (in != nullptr)
    {
        in->readIntoMemoryBlock (destData);
        return true;
    }

    return false;
}

String URL::readEntireTextStream (const bool usePostCommand) const
{
    const ScopedPointer <InputStream> in (createInputStream (usePostCommand));

    if (in != nullptr)
        return in->readEntireStreamAsString();

    return String::empty;
}

XmlElement* URL::readEntireXmlStream (const bool usePostCommand) const
{
    return XmlDocument::parse (readEntireTextStream (usePostCommand));
}

//==============================================================================
const URL URL::withParameter (const String& parameterName,
                              const String& parameterValue) const
{
    URL u (*this);
    u.parameters.set (parameterName, parameterValue);
    return u;
}

const URL URL::withFileToUpload (const String& parameterName,
                                 const File& fileToUpload,
                                 const String& mimeType) const
{
    jassert (mimeType.isNotEmpty()); // You need to supply a mime type!

    URL u (*this);
    u.filesToUpload.set (parameterName, fileToUpload.getFullPathName());
    u.mimeTypes.set (parameterName, mimeType);
    return u;
}

const URL URL::withPOSTData (const String& postData_) const
{
    URL u (*this);
    u.postData = postData_;
    return u;
}

const StringPairArray& URL::getParameters() const
{
    return parameters;
}

const StringPairArray& URL::getFilesToUpload() const
{
    return filesToUpload;
}

const StringPairArray& URL::getMimeTypesOfUploadFiles() const
{
    return mimeTypes;
}

//==============================================================================
String URL::removeEscapeChars (const String& s)
{
    String result (s.replaceCharacter ('+', ' '));

    if (! result.containsChar ('%'))
        return result;

    // We need to operate on the string as raw UTF8 chars, and then recombine them into unicode
    // after all the replacements have been made, so that multi-byte chars are handled.
    Array<char> utf8 (result.toUTF8().getAddress(), result.getNumBytesAsUTF8());

    for (int i = 0; i < utf8.size(); ++i)
    {
        if (utf8.getUnchecked(i) == '%')
        {
            const int hexDigit1 = CharacterFunctions::getHexDigitValue (utf8 [i + 1]);
            const int hexDigit2 = CharacterFunctions::getHexDigitValue (utf8 [i + 2]);

            if (hexDigit1 >= 0 && hexDigit2 >= 0)
            {
                utf8.set (i, (char) ((hexDigit1 << 4) + hexDigit2));
                utf8.removeRange (i + 1, 2);
            }
        }
    }

    return String::fromUTF8 (utf8.getRawDataPointer(), utf8.size());
}

String URL::addEscapeChars (const String& s, const bool isParameter)
{
    const CharPointer_UTF8 legalChars (isParameter ? "_-.*!'()"
                                                   : ",$_-.*!'()");

    Array<char> utf8 (s.toUTF8().getAddress(), s.getNumBytesAsUTF8());

    for (int i = 0; i < utf8.size(); ++i)
    {
        const char c = utf8.getUnchecked(i);

        if (! (CharacterFunctions::isLetterOrDigit (c)
                 || legalChars.indexOf ((juce_wchar) c) >= 0))
        {
            if (c == ' ')
            {
                utf8.set (i, '+');
            }
            else
            {
                static const char* const hexDigits = "0123456789abcdef";

                utf8.set (i, '%');
                utf8.insert (++i, hexDigits [((uint8) c) >> 4]);
                utf8.insert (++i, hexDigits [c & 15]);
            }
        }
    }

    return String::fromUTF8 (utf8.getRawDataPointer(), utf8.size());
}

//==============================================================================
bool URL::launchInDefaultBrowser() const
{
    String u (toString (true));

    if (u.containsChar ('@') && ! u.containsChar (':'))
        u = "mailto:" + u;

    return PlatformUtilities::openDocument (u, String::empty);
}


END_JUCE_NAMESPACE
