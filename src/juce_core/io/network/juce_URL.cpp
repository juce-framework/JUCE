/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_URL.h"
#include "../../basics/juce_Random.h"
#include "../../text/juce_XmlDocument.h"


//==============================================================================
URL::URL() throw()
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
      parameters (other.parameters),
      filesToUpload (other.filesToUpload),
      mimeTypes (other.mimeTypes)
{
}

const URL& URL::operator= (const URL& other)
{
    url = other.url;
    parameters = other.parameters;
    filesToUpload = other.filesToUpload;
    mimeTypes = other.mimeTypes;

    return *this;
}

URL::~URL() throw()
{
}

static const String getMangledParameters (const StringPairArray& parameters)
{
    String p;

    for (int i = 0; i < parameters.size(); ++i)
    {
        if (i > 0)
            p += T("&");

        p << URL::addEscapeChars (parameters.getAllKeys() [i])
          << T("=")
          << URL::addEscapeChars (parameters.getAllValues() [i]);
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

//==============================================================================
bool URL::isProbablyAWebsiteURL (const String& possibleURL)
{
    return (possibleURL.containsChar (T('.'))
            && (! possibleURL.containsChar (T('@')))
            && (! possibleURL.endsWithChar (T('.')))
            && (possibleURL.startsWithIgnoreCase (T("www."))
                || possibleURL.startsWithIgnoreCase (T("http:"))
                || possibleURL.startsWithIgnoreCase (T("ftp:"))
                || possibleURL.endsWithIgnoreCase (T(".com"))
                || possibleURL.endsWithIgnoreCase (T(".net"))
                || possibleURL.endsWithIgnoreCase (T(".org"))
                || possibleURL.endsWithIgnoreCase (T(".co.uk")))
        || possibleURL.startsWithIgnoreCase (T("file:")));
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
                             void* callbackContext);

void juce_closeInternetFile (void* handle);
int juce_readFromInternetFile (void* handle, void* dest, int bytesToRead);
int juce_seekInInternetFile (void* handle, int newPosition);


//==============================================================================
class WebInputStream  : public InputStream
{
public:
    //==============================================================================
    WebInputStream (const URL& url,
                    const bool isPost_,
                    URL::OpenStreamProgressCallback* const progressCallback_,
                    void* const progressCallbackContext_)
      : position (0),
        finished (false),
        isPost (isPost_),
        progressCallback (progressCallback_),
        progressCallbackContext (progressCallbackContext_)
    {
        server = url.toString (! isPost);

        if (isPost_)
            createHeadersAndPostData (url);

        handle = juce_openInternetFile (server, headers, postData, isPost,
                                        progressCallback_, progressCallbackContext_);
    }

    ~WebInputStream()
    {
        juce_closeInternetFile (handle);
    }

    bool isError() const throw()
    {
        return handle == 0;
    }

    //==============================================================================
    int64 getTotalLength()
    {
        return -1;
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
                                                    progressCallback, progressCallbackContext);
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
            // just a short text attachment, so use simple url encoding..
            const String params (getMangledParameters (url.getParameters()));

            headers = "Content-Type: application/x-www-form-urlencoded\r\nContent-length: "
                        + String ((int) strlen (params.toUTF8()))
                        + "\r\n";

            appendUTF8ToPostData (params);
        }
    }

    void appendUTF8ToPostData (const String& text) throw()
    {
        postData.append (text.toUTF8(),
                         (int) strlen (text.toUTF8()));
    }

    WebInputStream (const WebInputStream&);
    const WebInputStream& operator= (const WebInputStream&);
};

InputStream* URL::createInputStream (const bool usePostCommand,
                                     OpenStreamProgressCallback* const progressCallback,
                                     void* const progressCallbackContext) const
{
    WebInputStream* wi = new WebInputStream (*this, usePostCommand,
                                             progressCallback, progressCallbackContext);

    if (wi->isError())
    {
        delete wi;
        wi = 0;
    }

    return wi;
}

//==============================================================================
bool URL::readEntireBinaryStream (MemoryBlock& destData,
                                  const bool usePostCommand) const
{
    InputStream* const in = createInputStream (usePostCommand);

    if (in != 0)
    {
        in->readIntoMemoryBlock (destData, -1);
        delete in;

        return true;
    }

    return false;
}

const String URL::readEntireTextStream (const bool usePostCommand) const
{
    String result;
    InputStream* const in = createInputStream (usePostCommand);

    if (in != 0)
    {
        result = in->readEntireStreamAsString();
        delete in;
    }

    return result;
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
    URL u (*this);
    u.filesToUpload.set (parameterName, fileToUpload.getFullPathName());
    u.mimeTypes.set (parameterName, mimeType);
    return u;
}

const StringPairArray& URL::getParameters() const throw()
{
    return parameters;
}

const StringPairArray& URL::getFilesToUpload() const throw()
{
    return filesToUpload;
}

const StringPairArray& URL::getMimeTypesOfUploadFiles() const throw()
{
    return mimeTypes;
}

//==============================================================================
const String URL::removeEscapeChars (const String& s)
{
    const int len = s.length();
    uint8* const resultUTF8 = (uint8*) juce_calloc (len * 4);
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

    const String stringResult (String::fromUTF8 (resultUTF8));
    juce_free (resultUTF8);
    return stringResult;
}

const String URL::addEscapeChars (const String& s)
{
    String result;
    result.preallocateStorage (s.length() + 8);
    const char* utf8 = s.toUTF8();

    while (*utf8 != 0)
    {
        const char c = *utf8++;

        if (c == ' ')
        {
            result += T('+');
        }
        else if (CharacterFunctions::isLetterOrDigit (c)
                  || CharacterFunctions::indexOfChar ("_-$.*!'(),", c, false) >= 0)
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
extern bool juce_launchFile (const String& fileName,
                             const String& parameters) throw();

bool URL::launchInDefaultBrowser() const
{
    String u (toString (true));

    if (u.contains (T("@")) && ! u.contains (T(":")))
        u = "mailto:" + u;

    return juce_launchFile (u, String::empty);
}

END_JUCE_NAMESPACE
