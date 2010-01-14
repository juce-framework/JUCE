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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_URL.h"
#include "../../core/juce_Random.h"
#include "../../text/juce_XmlDocument.h"


//==============================================================================
URL::URL()
{
}

URL::URL (const String& url_)
    : url (url_)
{
    int i = url.indexOfChar (T('?'));

    if (i >= 0)
    {
        do
        {
            const int nextAmp   = url.indexOfChar (i + 1, T('&'));
            const int equalsPos = url.indexOfChar (i + 1, T('='));

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

        url = url.upToFirstOccurrenceOf (T("?"), false, false);
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

const URL& URL::operator= (const URL& other)
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

static const String getMangledParameters (const StringPairArray& parameters)
{
    String p;

    for (int i = 0; i < parameters.size(); ++i)
    {
        if (i > 0)
            p += T("&");

        p << URL::addEscapeChars (parameters.getAllKeys() [i], true)
          << T("=")
          << URL::addEscapeChars (parameters.getAllValues() [i], true);
    }

    return p;
}

const String URL::toString (const bool includeGetParameters) const
{
    if (includeGetParameters && parameters.size() > 0)
        return url + T("?") + getMangledParameters (parameters);
    else
        return url;
}

bool URL::isWellFormed() const
{
    //xxx TODO
    return url.isNotEmpty();
}

static int findStartOfDomain (const String& url)
{
    int i = 0;

    while (CharacterFunctions::isLetterOrDigit (url[i])
           || CharacterFunctions::indexOfChar (T("+-."), url[i], false) >= 0)
        ++i;

    return url[i] == T(':') ? i + 1 : 0;
}

const String URL::getDomain() const
{
    int start = findStartOfDomain (url);
    while (url[start] == T('/'))
        ++start;

    const int end1 = url.indexOfChar (start, T('/'));
    const int end2 = url.indexOfChar (start, T(':'));

    const int end = (end1 < 0 || end2 < 0) ? jmax (end1, end2)
                                           : jmin (end1, end2);

    return url.substring (start, end);
}

const String URL::getSubPath() const
{
    int start = findStartOfDomain (url);
    while (url[start] == T('/'))
        ++start;

    const int startOfPath = url.indexOfChar (start, T('/')) + 1;

    return startOfPath <= 0 ? String::empty
                            : url.substring (startOfPath);
}

const String URL::getScheme() const
{
    return url.substring (0, findStartOfDomain (url) - 1);
}

const URL URL::withNewSubPath (const String& newPath) const
{
    int start = findStartOfDomain (url);
    while (url[start] == T('/'))
        ++start;

    const int startOfPath = url.indexOfChar (start, T('/')) + 1;

    URL u (*this);

    if (startOfPath > 0)
        u.url = url.substring (0, startOfPath);

    if (! u.url.endsWithChar (T('/')))
        u.url << '/';

    if (newPath.startsWithChar (T('/')))
        u.url << newPath.substring (1);
    else
        u.url << newPath;

    return u;
}

//==============================================================================
bool URL::isProbablyAWebsiteURL (const String& possibleURL)
{
    if (possibleURL.startsWithIgnoreCase (T("http:"))
         || possibleURL.startsWithIgnoreCase (T("ftp:")))
        return true;

    if (possibleURL.startsWithIgnoreCase (T("file:"))
         || possibleURL.containsChar (T('@'))
         || possibleURL.endsWithChar (T('.'))
         || (! possibleURL.containsChar (T('.'))))
        return false;

    if (possibleURL.startsWithIgnoreCase (T("www."))
         && possibleURL.substring (5).containsChar (T('.')))
        return true;

    const char* commonTLDs[] = { "com", "net", "org", "uk", "de", "fr", "jp" };

    for (int i = 0; i < numElementsInArray (commonTLDs); ++i)
        if ((possibleURL + T("/")).containsIgnoreCase (T(".") + String (commonTLDs[i]) + T("/")))
            return true;

    return false;
}

bool URL::isProbablyAnEmailAddress (const String& possibleEmailAddress)
{
    const int atSign = possibleEmailAddress.indexOfChar (T('@'));

    return atSign > 0
            && possibleEmailAddress.lastIndexOfChar (T('.')) > (atSign + 1)
            && (! possibleEmailAddress.endsWithChar (T('.')));
}

//==============================================================================
void* juce_openInternetFile (const String& url,
                             const String& headers,
                             const MemoryBlock& optionalPostData,
                             const bool isPost,
                             URL::OpenStreamProgressCallback* callback,
                             void* callbackContext,
                             int timeOutMs);

void juce_closeInternetFile (void* handle);
int juce_readFromInternetFile (void* handle, void* dest, int bytesToRead);
int juce_seekInInternetFile (void* handle, int newPosition);
int64 juce_getInternetFileContentLength (void* handle);


//==============================================================================
class WebInputStream  : public InputStream
{
public:
    //==============================================================================
    WebInputStream (const URL& url,
                    const bool isPost_,
                    URL::OpenStreamProgressCallback* const progressCallback_,
                    void* const progressCallbackContext_,
                    const String& extraHeaders,
                    int timeOutMs_)
      : position (0),
        finished (false),
        isPost (isPost_),
        progressCallback (progressCallback_),
        progressCallbackContext (progressCallbackContext_),
        timeOutMs (timeOutMs_)
    {
        server = url.toString (! isPost);

        if (isPost_)
            createHeadersAndPostData (url);

        headers += extraHeaders;

        if (! headers.endsWithChar (T('\n')))
            headers << "\r\n";

        handle = juce_openInternetFile (server, headers, postData, isPost,
                                        progressCallback_, progressCallbackContext_,
                                        timeOutMs);
    }

    ~WebInputStream()
    {
        juce_closeInternetFile (handle);
    }

    bool isError() const
    {
        return handle == 0;
    }

    //==============================================================================
    int64 getTotalLength()
    {
        return juce_getInternetFileContentLength (handle);
    }

    bool isExhausted()
    {
        return finished;
    }

    int read (void* dest, int bytes)
    {
        if (finished || isError())
        {
            return 0;
        }
        else
        {
            const int bytesRead = juce_readFromInternetFile (handle, dest, bytes);
            position += bytesRead;

            if (bytesRead == 0)
                finished = true;

            return bytesRead;
        }
    }

    int64 getPosition()
    {
        return position;
    }

    bool setPosition (int64 wantedPos)
    {
        if (wantedPos != position)
        {
            finished = false;

            const int actualPos = juce_seekInInternetFile (handle, (int) wantedPos);

            if (actualPos == wantedPos)
            {
                position = wantedPos;
            }
            else
            {
                if (wantedPos < position)
                {
                    juce_closeInternetFile (handle);

                    position = 0;
                    finished = false;

                    handle = juce_openInternetFile (server, headers, postData, isPost,
                                                    progressCallback, progressCallbackContext,
                                                    timeOutMs);
                }

                skipNextBytes (wantedPos - position);
            }
        }

        return true;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    String server, headers;
    MemoryBlock postData;
    int64 position;
    bool finished;
    const bool isPost;
    void* handle;
    URL::OpenStreamProgressCallback* const progressCallback;
    void* const progressCallbackContext;
    const int timeOutMs;

    void createHeadersAndPostData (const URL& url)
    {
        if (url.getFilesToUpload().size() > 0)
        {
            // need to upload some files, so do it as multi-part...
            String boundary (String::toHexString (Random::getSystemRandom().nextInt64()));

            headers << "Content-Type: multipart/form-data; boundary=" << boundary << "\r\n";

            appendUTF8ToPostData ("--" + boundary);

            int i;
            for (i = 0; i < url.getParameters().size(); ++i)
            {
                String s;
                s << "\r\nContent-Disposition: form-data; name=\""
                  << url.getParameters().getAllKeys() [i]
                  << "\"\r\n\r\n"
                  << url.getParameters().getAllValues() [i]
                  << "\r\n--"
                  << boundary;

                appendUTF8ToPostData (s);
            }

            for (i = 0; i < url.getFilesToUpload().size(); ++i)
            {
                const File f (url.getFilesToUpload().getAllValues() [i]);
                const String paramName (url.getFilesToUpload().getAllKeys() [i]);

                String s;
                s << "\r\nContent-Disposition: form-data; name=\""
                  << paramName
                  << "\"; filename=\""
                  << f.getFileName()
                  << "\"\r\n";

                const String mimeType (url.getMimeTypesOfUploadFiles()
                                          .getValue (paramName, String::empty));

                if (mimeType.isNotEmpty())
                    s << "Content-Type: " << mimeType << "\r\n";

                s << "Content-Transfer-Encoding: binary\r\n\r\n";

                appendUTF8ToPostData (s);

                f.loadFileAsData (postData);

                s = "\r\n--" + boundary;

                appendUTF8ToPostData (s);
            }

            appendUTF8ToPostData ("--\r\n");
        }
        else
        {
            appendUTF8ToPostData (getMangledParameters (url.getParameters()));
            appendUTF8ToPostData (url.getPostData());

            // just a short text attachment, so use simple url encoding..
            headers = "Content-Type: application/x-www-form-urlencoded\r\nContent-length: "
                        + String ((unsigned int) postData.getSize())
                        + "\r\n";
        }
    }

    void appendUTF8ToPostData (const String& text)
    {
        postData.append (text.toUTF8(),
                         (int) strlen (text.toUTF8()));
    }

    WebInputStream (const WebInputStream&);
    const WebInputStream& operator= (const WebInputStream&);
};

InputStream* URL::createInputStream (const bool usePostCommand,
                                     OpenStreamProgressCallback* const progressCallback,
                                     void* const progressCallbackContext,
                                     const String& extraHeaders,
                                     const int timeOutMs) const
{
    ScopedPointer <WebInputStream> wi (new WebInputStream (*this, usePostCommand,
                                                           progressCallback, progressCallbackContext,
                                                           extraHeaders,
                                                           timeOutMs));

    return wi->isError() ? 0 : wi.release();
}

//==============================================================================
bool URL::readEntireBinaryStream (MemoryBlock& destData,
                                  const bool usePostCommand) const
{
    const ScopedPointer <InputStream> in (createInputStream (usePostCommand));

    if (in != 0)
    {
        in->readIntoMemoryBlock (destData, -1);
        return true;
    }

    return false;
}

const String URL::readEntireTextStream (const bool usePostCommand) const
{
    const ScopedPointer <InputStream> in (createInputStream (usePostCommand));

    if (in != 0)
        return in->readEntireStreamAsString();

    return String::empty;
}

XmlElement* URL::readEntireXmlStream (const bool usePostCommand) const
{
    XmlDocument doc (readEntireTextStream (usePostCommand));
    return doc.getDocumentElement();
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
const String URL::removeEscapeChars (const String& s)
{
    const int len = s.length();
    HeapBlock <uint8> resultUTF8 (len * 4);
    uint8* r = resultUTF8;

    for (int i = 0; i < len; ++i)
    {
        char c = (char) s[i];
        if (c == 0)
            break;

        if (c == '+')
        {
            c = ' ';
        }
        else if (c == '%')
        {
            c = (char) s.substring (i + 1, i + 3).getHexValue32();
            i += 2;
        }

        *r++ = c;
    }

    return String::fromUTF8 (resultUTF8);
}

const String URL::addEscapeChars (const String& s, const bool isParameter)
{
    String result;
    result.preallocateStorage (s.length() + 8);
    const char* utf8 = s.toUTF8();
    const char* legalChars = isParameter ? "_-.*!'()"
                                         : "_-$.*!'(),";

    while (*utf8 != 0)
    {
        const char c = *utf8++;

        if (CharacterFunctions::isLetterOrDigit (c)
             || CharacterFunctions::indexOfChar (legalChars, c, false) >= 0)
        {
            result << c;
        }
        else
        {
            const int v = (int) (uint8) c;

            if (v < 0x10)
                result << T("%0");
            else
                result << T('%');

            result << String::toHexString (v);
        }
    }

    return result;
}

//==============================================================================
extern bool juce_launchFile (const String& fileName, const String& parameters);

bool URL::launchInDefaultBrowser() const
{
    String u (toString (true));

    if (u.contains (T("@")) && ! u.contains (T(":")))
        u = "mailto:" + u;

    return juce_launchFile (u, String::empty);
}

END_JUCE_NAMESPACE
