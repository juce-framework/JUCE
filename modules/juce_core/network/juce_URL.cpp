/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

URL::URL()
{
}

URL::URL (const String& u)  : url (u)
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
                    addParameter (removeEscapeChars (url.substring (i + 1, equalsPos)),
                                  removeEscapeChars (url.substring (equalsPos + 1)));
                }
                else if (nextAmp > 0 && equalsPos < nextAmp)
                {
                    addParameter (removeEscapeChars (url.substring (i + 1, equalsPos)),
                                  removeEscapeChars (url.substring (equalsPos + 1, nextAmp)));
                }
            }

            i = nextAmp;
        }
        while (i >= 0);

        url = url.upToFirstOccurrenceOf ("?", false, false);
    }
}

URL::URL (const String& u, int)  : url (u) {}

URL URL::createWithoutParsing (const String& u)
{
    return URL (u, 0);
}

URL::URL (const URL& other)
    : url (other.url),
      postData (other.postData),
      parameterNames (other.parameterNames),
      parameterValues (other.parameterValues),
      filesToUpload (other.filesToUpload)
{
}

URL& URL::operator= (const URL& other)
{
    url = other.url;
    postData = other.postData;
    parameterNames = other.parameterNames;
    parameterValues = other.parameterValues;
    filesToUpload = other.filesToUpload;

    return *this;
}

bool URL::operator== (const URL& other) const
{
    return url == other.url
        && postData == other.postData
        && parameterNames == other.parameterNames
        && parameterValues == other.parameterValues
        && filesToUpload == other.filesToUpload;
}

bool URL::operator!= (const URL& other) const
{
    return ! operator== (other);
}

URL::~URL()
{
}

namespace URLHelpers
{
    static String getMangledParameters (const URL& url)
    {
        jassert (url.getParameterNames().size() == url.getParameterValues().size());
        String p;

        for (int i = 0; i < url.getParameterNames().size(); ++i)
        {
            if (i > 0)
                p << '&';

            p << URL::addEscapeChars (url.getParameterNames()[i], true)
              << '='
              << URL::addEscapeChars (url.getParameterValues()[i], true);
        }

        return p;
    }

    static int findEndOfScheme (const String& url)
    {
        int i = 0;

        while (CharacterFunctions::isLetterOrDigit (url[i])
                || url[i] == '+' || url[i] == '-' || url[i] == '.')
            ++i;

        return url[i] == ':' ? i + 1 : 0;
    }

    static int findStartOfNetLocation (const String& url)
    {
        int start = findEndOfScheme (url);
        while (url[start] == '/')
            ++start;

        return start;
    }

    static int findStartOfPath (const String& url)
    {
        return url.indexOfChar (findStartOfNetLocation (url), '/') + 1;
    }

    static void concatenatePaths (String& path, const String& suffix)
    {
        if (! path.endsWithChar ('/'))
            path << '/';

        if (suffix.startsWithChar ('/'))
            path += suffix.substring (1);
        else
            path += suffix;
    }
}

void URL::addParameter (const String& name, const String& value)
{
    parameterNames.add (name);
    parameterValues.add (value);
}

String URL::toString (const bool includeGetParameters) const
{
    if (includeGetParameters && parameterNames.size() > 0)
        return url + "?" + URLHelpers::getMangledParameters (*this);

    return url;
}

bool URL::isEmpty() const noexcept
{
    return url.isEmpty();
}

bool URL::isWellFormed() const
{
    //xxx TODO
    return url.isNotEmpty();
}

String URL::getDomain() const
{
    const int start = URLHelpers::findStartOfNetLocation (url);
    const int end1 = url.indexOfChar (start, '/');
    const int end2 = url.indexOfChar (start, ':');

    const int end = (end1 < 0 && end2 < 0) ? std::numeric_limits<int>::max()
                                           : ((end1 < 0 || end2 < 0) ? jmax (end1, end2)
                                                                     : jmin (end1, end2));
    return url.substring (start, end);
}

String URL::getSubPath() const
{
    const int startOfPath = URLHelpers::findStartOfPath (url);

    return startOfPath <= 0 ? String()
                            : url.substring (startOfPath);
}

String URL::getScheme() const
{
    return url.substring (0, URLHelpers::findEndOfScheme (url) - 1);
}

int URL::getPort() const
{
    const int colonPos = url.indexOfChar (URLHelpers::findStartOfNetLocation (url), ':');

    return colonPos > 0 ? url.substring (colonPos + 1).getIntValue() : 0;
}

URL URL::withNewSubPath (const String& newPath) const
{
    const int startOfPath = URLHelpers::findStartOfPath (url);

    URL u (*this);

    if (startOfPath > 0)
        u.url = url.substring (0, startOfPath);

    URLHelpers::concatenatePaths (u.url, newPath);
    return u;
}

URL URL::getChildURL (const String& subPath) const
{
    URL u (*this);
    URLHelpers::concatenatePaths (u.url, subPath);
    return u;
}

void URL::createHeadersAndPostData (String& headers, MemoryBlock& headersAndPostData) const
{
    MemoryOutputStream data (headersAndPostData, false);

    if (filesToUpload.size() > 0)
    {
        // (this doesn't currently support mixing custom post-data with uploads..)
        jassert (postData.isEmpty());

        const String boundary (String::toHexString (Random::getSystemRandom().nextInt64()));

        headers << "Content-Type: multipart/form-data; boundary=" << boundary << "\r\n";

        data << "--" << boundary;

        for (int i = 0; i < parameterNames.size(); ++i)
        {
            data << "\r\nContent-Disposition: form-data; name=\"" << parameterNames[i]
                 << "\"\r\n\r\n" << parameterValues[i]
                 << "\r\n--" << boundary;
        }

        for (int i = 0; i < filesToUpload.size(); ++i)
        {
            const Upload& f = *filesToUpload.getObjectPointerUnchecked(i);

            data << "\r\nContent-Disposition: form-data; name=\"" << f.parameterName
                 << "\"; filename=\"" << f.filename << "\"\r\n";

            if (f.mimeType.isNotEmpty())
                data << "Content-Type: " << f.mimeType << "\r\n";

            data << "Content-Transfer-Encoding: binary\r\n\r\n";

            if (f.data != nullptr)
                data << *f.data;
            else
                data << f.file;

            data << "\r\n--" << boundary;
        }

        data << "--\r\n";
    }
    else
    {
        data << URLHelpers::getMangledParameters (*this)
             << postData;

        // if the user-supplied headers didn't contain a content-type, add one now..
        if (! headers.containsIgnoreCase ("Content-Type"))
            headers << "Content-Type: application/x-www-form-urlencoded\r\n";

        headers << "Content-length: " << (int) data.getDataSize() << "\r\n";
    }
}

//==============================================================================
bool URL::isProbablyAWebsiteURL (const String& possibleURL)
{
    static const char* validProtocols[] = { "http:", "ftp:", "https:" };

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
            && ! possibleEmailAddress.endsWithChar ('.');
}

//==============================================================================
InputStream* URL::createInputStream (const bool usePostCommand,
                                     OpenStreamProgressCallback* const progressCallback,
                                     void* const progressCallbackContext,
                                     String headers,
                                     const int timeOutMs,
                                     StringPairArray* const responseHeaders,
                                     int* statusCode,
                                     const int numRedirectsToFollow) const
{
    MemoryBlock headersAndPostData;

    if (! headers.endsWithChar ('\n'))
        headers << "\r\n";

    if (usePostCommand)
        createHeadersAndPostData (headers, headersAndPostData);

    if (! headers.endsWithChar ('\n'))
        headers << "\r\n";

    ScopedPointer<WebInputStream> wi (new WebInputStream (toString (! usePostCommand),
                                                          usePostCommand, headersAndPostData,
                                                          progressCallback, progressCallbackContext,
                                                          headers, timeOutMs, responseHeaders,
                                                          numRedirectsToFollow));

    if (statusCode != nullptr)
        *statusCode = wi->statusCode;

    return wi->isError() ? nullptr : wi.release();
}

//==============================================================================
bool URL::readEntireBinaryStream (MemoryBlock& destData,
                                  const bool usePostCommand) const
{
    const ScopedPointer<InputStream> in (createInputStream (usePostCommand));

    if (in != nullptr)
    {
        in->readIntoMemoryBlock (destData);
        return true;
    }

    return false;
}

String URL::readEntireTextStream (const bool usePostCommand) const
{
    const ScopedPointer<InputStream> in (createInputStream (usePostCommand));

    if (in != nullptr)
        return in->readEntireStreamAsString();

    return String();
}

XmlElement* URL::readEntireXmlStream (const bool usePostCommand) const
{
    return XmlDocument::parse (readEntireTextStream (usePostCommand));
}

//==============================================================================
URL URL::withParameter (const String& parameterName,
                        const String& parameterValue) const
{
    URL u (*this);
    u.addParameter (parameterName, parameterValue);
    return u;
}

URL URL::withParameters (const StringPairArray& parametersToAdd) const
{
    URL u (*this);

    for (int i = 0; i < parametersToAdd.size(); ++i)
        u.addParameter (parametersToAdd.getAllKeys()[i],
                        parametersToAdd.getAllValues()[i]);

    return u;
}

URL URL::withPOSTData (const String& newPostData) const
{
    URL u (*this);
    u.postData = newPostData;
    return u;
}

URL::Upload::Upload (const String& param, const String& name,
                     const String& mime, const File& f, MemoryBlock* mb)
    : parameterName (param), filename (name), mimeType (mime), file (f), data (mb)
{
    jassert (mimeType.isNotEmpty()); // You need to supply a mime type!
}

URL URL::withUpload (Upload* const f) const
{
    URL u (*this);

    for (int i = u.filesToUpload.size(); --i >= 0;)
        if (u.filesToUpload.getObjectPointerUnchecked(i)->parameterName == f->parameterName)
            u.filesToUpload.remove (i);

    u.filesToUpload.add (f);
    return u;
}

URL URL::withFileToUpload (const String& parameterName, const File& fileToUpload,
                           const String& mimeType) const
{
    return withUpload (new Upload (parameterName, fileToUpload.getFileName(),
                                   mimeType, fileToUpload, nullptr));
}

URL URL::withDataToUpload (const String& parameterName, const String& filename,
                           const MemoryBlock& fileContentToUpload, const String& mimeType) const
{
    return withUpload (new Upload (parameterName, filename, mimeType, File(),
                                   new MemoryBlock (fileContentToUpload)));
}

//==============================================================================
String URL::removeEscapeChars (const String& s)
{
    String result (s.replaceCharacter ('+', ' '));

    if (! result.containsChar ('%'))
        return result;

    // We need to operate on the string as raw UTF8 chars, and then recombine them into unicode
    // after all the replacements have been made, so that multi-byte chars are handled.
    Array<char> utf8 (result.toRawUTF8(), (int) result.getNumBytesAsUTF8());

    for (int i = 0; i < utf8.size(); ++i)
    {
        if (utf8.getUnchecked(i) == '%')
        {
            const int hexDigit1 = CharacterFunctions::getHexDigitValue ((juce_wchar) (uint8) utf8 [i + 1]);
            const int hexDigit2 = CharacterFunctions::getHexDigitValue ((juce_wchar) (uint8) utf8 [i + 2]);

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

    Array<char> utf8 (s.toRawUTF8(), (int) s.getNumBytesAsUTF8());

    for (int i = 0; i < utf8.size(); ++i)
    {
        const char c = utf8.getUnchecked(i);

        if (! (CharacterFunctions::isLetterOrDigit (c)
                 || legalChars.indexOf ((juce_wchar) c) >= 0))
        {
            utf8.set (i, '%');
            utf8.insert (++i, "0123456789ABCDEF" [((uint8) c) >> 4]);
            utf8.insert (++i, "0123456789ABCDEF" [c & 15]);
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

    return Process::openDocument (u, String());
}
