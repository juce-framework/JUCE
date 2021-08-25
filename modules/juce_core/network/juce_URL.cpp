/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

struct FallbackDownloadTask  : public URL::DownloadTask,
                               public Thread
{
    FallbackDownloadTask (std::unique_ptr<FileOutputStream> outputStreamToUse,
                          size_t bufferSizeToUse,
                          std::unique_ptr<WebInputStream> streamToUse,
                          URL::DownloadTask::Listener* listenerToUse)
        : Thread ("DownloadTask thread"),
          fileStream (std::move (outputStreamToUse)),
          stream (std::move (streamToUse)),
          bufferSize (bufferSizeToUse),
          buffer (bufferSize),
          listener (listenerToUse)
    {
        jassert (fileStream != nullptr);
        jassert (stream != nullptr);

        targetLocation = fileStream->getFile();
        contentLength  = stream->getTotalLength();
        httpCode       = stream->getStatusCode();

        startThread();
    }

    ~FallbackDownloadTask() override
    {
        signalThreadShouldExit();
        stream->cancel();
        waitForThreadToExit (-1);
    }

    //==============================================================================
    void run() override
    {
        while (! (stream->isExhausted() || stream->isError() || threadShouldExit()))
        {
            if (listener != nullptr)
                listener->progress (this, downloaded, contentLength);

            auto max = (int) jmin ((int64) bufferSize, contentLength < 0 ? std::numeric_limits<int64>::max()
                                                                         : static_cast<int64> (contentLength - downloaded));

            auto actual = stream->read (buffer.get(), max);

            if (actual < 0 || threadShouldExit() || stream->isError())
                break;

            if (! fileStream->write (buffer.get(), static_cast<size_t> (actual)))
            {
                error = true;
                break;
            }

            downloaded += actual;

            if (downloaded == contentLength)
                break;
        }

        fileStream.reset();

        if (threadShouldExit() || stream->isError())
            error = true;

        if (contentLength > 0 && downloaded < contentLength)
            error = true;

        finished = true;

        if (listener != nullptr && ! threadShouldExit())
            listener->finished (this, ! error);
    }

    //==============================================================================
    std::unique_ptr<FileOutputStream> fileStream;
    const std::unique_ptr<WebInputStream> stream;
    const size_t bufferSize;
    HeapBlock<char> buffer;
    URL::DownloadTask::Listener* const listener;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FallbackDownloadTask)
};

void URL::DownloadTaskListener::progress (DownloadTask*, int64, int64) {}

//==============================================================================
std::unique_ptr<URL::DownloadTask> URL::DownloadTask::createFallbackDownloader (const URL& urlToUse,
                                                                                const File& targetFileToUse,
                                                                                const DownloadTaskOptions& options)
{
    const size_t bufferSize = 0x8000;
    targetFileToUse.deleteFile();

    if (auto outputStream = targetFileToUse.createOutputStream (bufferSize))
    {
        auto stream = std::make_unique<WebInputStream> (urlToUse, options.usePost);
        stream->withExtraHeaders (options.extraHeaders);

        if (stream->connect (nullptr))
            return std::make_unique<FallbackDownloadTask> (std::move (outputStream),
                                                           bufferSize,
                                                           std::move (stream),
                                                           options.listener);
    }

    return nullptr;
}

URL::DownloadTask::DownloadTask() {}
URL::DownloadTask::~DownloadTask() {}

//==============================================================================
URL::URL() {}

URL::URL (const String& u)  : url (u)
{
    init();
}

URL::URL (File localFile)
{
    if (localFile == File())
        return;

   #if JUCE_WINDOWS
    bool isUncPath = localFile.getFullPathName().startsWith ("\\\\");
   #endif

    while (! localFile.isRoot())
    {
        url = "/" + addEscapeChars (localFile.getFileName(), false) + url;
        localFile = localFile.getParentDirectory();
    }

    url = addEscapeChars (localFile.getFileName(), false) + url;

   #if JUCE_WINDOWS
    if (isUncPath)
    {
        url = url.fromFirstOccurrenceOf ("/", false, false);
    }
    else
   #endif
    {
        if (! url.startsWithChar (L'/'))
            url = "/" + url;
    }

    url = "file://" + url;

    jassert (isWellFormed());
}

void URL::init()
{
    auto i = url.indexOfChar ('?');

    if (i >= 0)
    {
        do
        {
            auto nextAmp   = url.indexOfChar (i + 1, '&');
            auto equalsPos = url.indexOfChar (i + 1, '=');

            if (nextAmp < 0)
            {
                addParameter (removeEscapeChars (equalsPos < 0 ? url.substring (i + 1) : url.substring (i + 1, equalsPos)),
                              equalsPos < 0 ? String() : removeEscapeChars (url.substring (equalsPos + 1)));
            }
            else if (nextAmp > 0 && equalsPos < nextAmp)
            {
                addParameter (removeEscapeChars (equalsPos < 0 ? url.substring (i + 1, nextAmp) : url.substring (i + 1, equalsPos)),
                              equalsPos < 0 ? String() : removeEscapeChars (url.substring (equalsPos + 1, nextAmp)));
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

            auto val = url.getParameterValues()[i];

            p << URL::addEscapeChars (url.getParameterNames()[i], true);

            if (val.isNotEmpty())
                p << '=' << URL::addEscapeChars (val, true);
        }

        return p;
    }

    static int findEndOfScheme (const String& url)
    {
        int i = 0;

        while (CharacterFunctions::isLetterOrDigit (url[i])
               || url[i] == '+' || url[i] == '-' || url[i] == '.')
            ++i;

        return url.substring (i).startsWith ("://") ? i + 1 : 0;
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

    static String removeLastPathSection (const String& url)
    {
        auto startOfPath = findStartOfPath (url);
        auto lastSlash = url.lastIndexOfChar ('/');

        if (lastSlash > startOfPath && lastSlash == url.length() - 1)
            return removeLastPathSection (url.dropLastCharacters (1));

        if (lastSlash < 0)
            return url;

        return url.substring (0, std::max (startOfPath, lastSlash));
    }
}

void URL::addParameter (const String& name, const String& value)
{
    parameterNames.add (name);
    parameterValues.add (value);
}

String URL::toString (bool includeGetParameters) const
{
    if (includeGetParameters)
        return url + getQueryString();

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
    return getDomainInternal (false);
}

String URL::getSubPath (bool includeGetParameters) const
{
    auto startOfPath = URLHelpers::findStartOfPath (url);
    auto subPath = startOfPath <= 0 ? String()
                                    : url.substring (startOfPath);

    if (includeGetParameters)
        subPath += getQueryString();

    return subPath;
}

String URL::getQueryString() const
{
    if (parameterNames.size() > 0)
        return "?" + URLHelpers::getMangledParameters (*this);

    return {};
}

String URL::getScheme() const
{
    return url.substring (0, URLHelpers::findEndOfScheme (url) - 1);
}

#if ! JUCE_ANDROID
bool URL::isLocalFile() const
{
    return getScheme() == "file";
}

File URL::getLocalFile() const
{
    return fileFromFileSchemeURL (*this);
}

String URL::getFileName() const
{
    return toString (false).fromLastOccurrenceOf ("/", false, true);
}
#endif

URL::ParameterHandling URL::toHandling (bool usePostData)
{
    return usePostData ? ParameterHandling::inPostData : ParameterHandling::inAddress;
}

File URL::fileFromFileSchemeURL (const URL& fileURL)
{
    if (! fileURL.isLocalFile())
    {
        jassertfalse;
        return {};
    }

    auto path = removeEscapeChars (fileURL.getDomainInternal (true)).replace ("+", "%2B");

   #if JUCE_WINDOWS
    bool isUncPath = (! fileURL.url.startsWith ("file:///"));
   #else
    path = File::getSeparatorString() + path;
   #endif

    auto urlElements = StringArray::fromTokens (fileURL.getSubPath(), "/", "");

    for (auto urlElement : urlElements)
        path += File::getSeparatorString() + removeEscapeChars (urlElement.replace ("+", "%2B"));

   #if JUCE_WINDOWS
    if (isUncPath)
        path = "\\\\" + path;
   #endif

    return path;
}

int URL::getPort() const
{
    auto colonPos = url.indexOfChar (URLHelpers::findStartOfNetLocation (url), ':');

    return colonPos > 0 ? url.substring (colonPos + 1).getIntValue() : 0;
}

URL URL::withNewDomainAndPath (const String& newURL) const
{
    URL u (*this);
    u.url = newURL;
    return u;
}

URL URL::withNewSubPath (const String& newPath) const
{
    URL u (*this);

    auto startOfPath = URLHelpers::findStartOfPath (url);

    if (startOfPath > 0)
        u.url = url.substring (0, startOfPath);

    URLHelpers::concatenatePaths (u.url, newPath);
    return u;
}

URL URL::getParentURL() const
{
    URL u (*this);
    u.url = URLHelpers::removeLastPathSection (u.url);
    return u;
}

URL URL::getChildURL (const String& subPath) const
{
    URL u (*this);
    URLHelpers::concatenatePaths (u.url, subPath);
    return u;
}

bool URL::hasBodyDataToSend() const
{
    return filesToUpload.size() > 0 || ! postData.isEmpty();
}

void URL::createHeadersAndPostData (String& headers,
                                    MemoryBlock& postDataToWrite,
                                    bool addParametersToBody) const
{
    MemoryOutputStream data (postDataToWrite, false);

    if (filesToUpload.size() > 0)
    {
        // (this doesn't currently support mixing custom post-data with uploads..)
        jassert (postData.isEmpty());

        auto boundary = String::toHexString (Random::getSystemRandom().nextInt64());

        headers << "Content-Type: multipart/form-data; boundary=" << boundary << "\r\n";

        data << "--" << boundary;

        for (int i = 0; i < parameterNames.size(); ++i)
        {
            data << "\r\nContent-Disposition: form-data; name=\"" << parameterNames[i]
                 << "\"\r\n\r\n" << parameterValues[i]
                 << "\r\n--" << boundary;
        }

        for (auto* f : filesToUpload)
        {
            data << "\r\nContent-Disposition: form-data; name=\"" << f->parameterName
                 << "\"; filename=\"" << f->filename << "\"\r\n";

            if (f->mimeType.isNotEmpty())
                data << "Content-Type: " << f->mimeType << "\r\n";

            data << "Content-Transfer-Encoding: binary\r\n\r\n";

            if (f->data != nullptr)
                data << *f->data;
            else
                data << f->file;

            data << "\r\n--" << boundary;
        }

        data << "--\r\n";
    }
    else
    {
        if (addParametersToBody)
            data << URLHelpers::getMangledParameters (*this);

        data << postData;

        // if the user-supplied headers didn't contain a content-type, add one now..
        if (! headers.containsIgnoreCase ("Content-Type"))
            headers << "Content-Type: application/x-www-form-urlencoded\r\n";

        headers << "Content-length: " << (int) data.getDataSize() << "\r\n";
    }
}

//==============================================================================
bool URL::isProbablyAWebsiteURL (const String& possibleURL)
{
    for (auto* protocol : { "http:", "https:", "ftp:" })
        if (possibleURL.startsWithIgnoreCase (protocol))
            return true;

    if (possibleURL.containsChar ('@') || possibleURL.containsChar (' '))
        return false;

    auto topLevelDomain = possibleURL.upToFirstOccurrenceOf ("/", false, false)
                                     .fromLastOccurrenceOf (".", false, false);

    return topLevelDomain.isNotEmpty() && topLevelDomain.length() <= 3;
}

bool URL::isProbablyAnEmailAddress (const String& possibleEmailAddress)
{
    auto atSign = possibleEmailAddress.indexOfChar ('@');

    return atSign > 0
        && possibleEmailAddress.lastIndexOfChar ('.') > (atSign + 1)
        && ! possibleEmailAddress.endsWithChar ('.');
}

String URL::getDomainInternal (bool ignorePort) const
{
    auto start = URLHelpers::findStartOfNetLocation (url);
    auto end1 = url.indexOfChar (start, '/');
    auto end2 = ignorePort ? -1 : url.indexOfChar (start, ':');

    auto end = (end1 < 0 && end2 < 0) ? std::numeric_limits<int>::max()
                                      : ((end1 < 0 || end2 < 0) ? jmax (end1, end2)
                                                                : jmin (end1, end2));
    return url.substring (start, end);
}

#if JUCE_IOS
URL::Bookmark::Bookmark (void* bookmarkToUse) : data (bookmarkToUse)
{
}

URL::Bookmark::~Bookmark()
{
    [(NSData*) data release];
}

void setURLBookmark (URL& u, void* bookmark)
{
    u.bookmark = new URL::Bookmark (bookmark);
}

void* getURLBookmark (URL& u)
{
    if (u.bookmark.get() == nullptr)
        return nullptr;

    return u.bookmark.get()->data;
}

template <typename Stream> struct iOSFileStreamWrapperFlush    { static void flush (Stream*) {} };
template <> struct iOSFileStreamWrapperFlush<FileOutputStream> { static void flush (OutputStream* o) { o->flush(); } };

template <typename Stream>
class iOSFileStreamWrapper : public Stream
{
public:
    iOSFileStreamWrapper (URL& urlToUse)
        : Stream (getLocalFileAccess (urlToUse)),
          url (urlToUse)
    {}

    ~iOSFileStreamWrapper()
    {
        iOSFileStreamWrapperFlush<Stream>::flush (this);

        if (NSData* bookmark = (NSData*) getURLBookmark (url))
        {
            BOOL isBookmarkStale = false;
            NSError* error = nil;

            auto nsURL = [NSURL URLByResolvingBookmarkData: bookmark
                                                   options: 0
                                             relativeToURL: nil
                                       bookmarkDataIsStale: &isBookmarkStale
                                                     error: &error];

            if (error == nil)
            {
                if (isBookmarkStale)
                    updateStaleBookmark (nsURL, url);

                [nsURL stopAccessingSecurityScopedResource];
            }
            else
            {
                auto desc = [error localizedDescription];
                ignoreUnused (desc);
                jassertfalse;
            }
        }
    }

private:
    URL url;
    bool securityAccessSucceeded = false;

    File getLocalFileAccess (URL& urlToUse)
    {
        if (NSData* bookmark = (NSData*) getURLBookmark (urlToUse))
        {
            BOOL isBookmarkStale = false;
            NSError* error = nil;

            auto nsURL = [NSURL URLByResolvingBookmarkData: bookmark
                                                   options: 0
                                             relativeToURL: nil
                                       bookmarkDataIsStale: &isBookmarkStale
                                                      error: &error];

            if (error == nil)
            {
                securityAccessSucceeded = [nsURL startAccessingSecurityScopedResource];

                if (isBookmarkStale)
                    updateStaleBookmark (nsURL, urlToUse);

                return urlToUse.getLocalFile();
            }

            auto desc = [error localizedDescription];
            ignoreUnused (desc);
            jassertfalse;
        }

        return urlToUse.getLocalFile();
    }

    void updateStaleBookmark (NSURL* nsURL, URL& juceUrl)
    {
        NSError* error = nil;

        NSData* bookmark = [nsURL bookmarkDataWithOptions: NSURLBookmarkCreationSuitableForBookmarkFile
                           includingResourceValuesForKeys: nil
                                            relativeToURL: nil
                                                    error: &error];

        if (error == nil)
            setURLBookmark (juceUrl, (void*) bookmark);
        else
            jassertfalse;
    }
};
#endif
//==============================================================================
template <typename Member, typename Item>
static URL::InputStreamOptions with (URL::InputStreamOptions options, Member&& member, Item&& item)
{
    options.*member = std::forward<Item> (item);
    return options;
}

URL::InputStreamOptions::InputStreamOptions (ParameterHandling handling)  : parameterHandling (handling)  {}

URL::InputStreamOptions URL::InputStreamOptions::withProgressCallback (std::function<bool (int, int)> cb) const
{
    return with (*this, &InputStreamOptions::progressCallback, std::move (cb));
}

URL::InputStreamOptions URL::InputStreamOptions::withExtraHeaders (const String& headers) const
{
    return with (*this, &InputStreamOptions::extraHeaders, headers);
}

URL::InputStreamOptions URL::InputStreamOptions::withConnectionTimeoutMs (int timeout) const
{
    return with (*this, &InputStreamOptions::connectionTimeOutMs, timeout);
}

URL::InputStreamOptions URL::InputStreamOptions::withResponseHeaders (StringPairArray* headers) const
{
    return with (*this, &InputStreamOptions::responseHeaders, headers);
}

URL::InputStreamOptions URL::InputStreamOptions::withStatusCode (int* status) const
{
    return with (*this, &InputStreamOptions::statusCode, status);
}

URL::InputStreamOptions URL::InputStreamOptions::withNumRedirectsToFollow (int numRedirects) const
{
    return with (*this, &InputStreamOptions::numRedirectsToFollow, numRedirects);
}

URL::InputStreamOptions URL::InputStreamOptions::withHttpRequestCmd (const String& cmd) const
{
    return with (*this, &InputStreamOptions::httpRequestCmd, cmd);
}

//==============================================================================
std::unique_ptr<InputStream> URL::createInputStream (const InputStreamOptions& options) const
{
    if (isLocalFile())
    {
       #if JUCE_IOS
        // We may need to refresh the embedded bookmark.
        return std::make_unique<iOSFileStreamWrapper<FileInputStream>> (const_cast<URL&> (*this));
       #else
        return getLocalFile().createInputStream();
       #endif
    }

    auto webInputStream = [&]
    {
        const auto usePost = options.getParameterHandling() == ParameterHandling::inPostData;
        auto stream = std::make_unique<WebInputStream> (*this, usePost);

        auto extraHeaders = options.getExtraHeaders();

        if (extraHeaders.isNotEmpty())
            stream->withExtraHeaders (extraHeaders);

        auto timeout = options.getConnectionTimeoutMs();

        if (timeout != 0)
            stream->withConnectionTimeout (timeout);

        auto requestCmd = options.getHttpRequestCmd();

        if (requestCmd.isNotEmpty())
            stream->withCustomRequestCommand (requestCmd);

        stream->withNumRedirectsToFollow (options.getNumRedirectsToFollow());

        return stream;
    }();

    struct ProgressCallbackCaller  : public WebInputStream::Listener
    {
        ProgressCallbackCaller (std::function<bool (int, int)> progressCallbackToUse)
            : callback (std::move (progressCallbackToUse))
        {
        }

        bool postDataSendProgress (WebInputStream&, int bytesSent, int totalBytes) override
        {
            return callback (bytesSent, totalBytes);
        }

        std::function<bool (int, int)> callback;
    };

    auto callbackCaller = [&options]() -> std::unique_ptr<ProgressCallbackCaller>
    {
        if (auto progressCallback = options.getProgressCallback())
            return std::make_unique<ProgressCallbackCaller> (progressCallback);

        return {};
    }();

    auto success = webInputStream->connect (callbackCaller.get());

    if (auto* status = options.getStatusCode())
        *status = webInputStream->getStatusCode();

    if (auto* responseHeaders = options.getResponseHeaders())
        *responseHeaders = webInputStream->getResponseHeaders();

    if (! success || webInputStream->isError())
        return nullptr;

    // std::move() needed here for older compilers
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wredundant-move")
    return std::move (webInputStream);
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

#if JUCE_ANDROID
OutputStream* juce_CreateContentURIOutputStream (const URL&);
#endif

std::unique_ptr<OutputStream> URL::createOutputStream() const
{
    if (isLocalFile())
    {
       #if JUCE_IOS
        // We may need to refresh the embedded bookmark.
        return std::make_unique<iOSFileStreamWrapper<FileOutputStream>> (const_cast<URL&> (*this));
       #else
        return std::make_unique<FileOutputStream> (getLocalFile());
       #endif
    }

   #if JUCE_ANDROID
    return std::unique_ptr<OutputStream> (juce_CreateContentURIOutputStream (*this));
   #else
    return nullptr;
   #endif
}

//==============================================================================
bool URL::readEntireBinaryStream (MemoryBlock& destData, bool usePostCommand) const
{
    const std::unique_ptr<InputStream> in (isLocalFile() ? getLocalFile().createInputStream()
                                                         : createInputStream (InputStreamOptions (toHandling (usePostCommand))));

    if (in != nullptr)
    {
        in->readIntoMemoryBlock (destData);
        return true;
    }

    return false;
}

String URL::readEntireTextStream (bool usePostCommand) const
{
    const std::unique_ptr<InputStream> in (isLocalFile() ? getLocalFile().createInputStream()
                                                         : createInputStream (InputStreamOptions (toHandling (usePostCommand))));

    if (in != nullptr)
        return in->readEntireStreamAsString();

    return {};
}

std::unique_ptr<XmlElement> URL::readEntireXmlStream (bool usePostCommand) const
{
    return parseXML (readEntireTextStream (usePostCommand));
}

//==============================================================================
URL URL::withParameter (const String& parameterName,
                        const String& parameterValue) const
{
    auto u = *this;
    u.addParameter (parameterName, parameterValue);
    return u;
}

URL URL::withParameters (const StringPairArray& parametersToAdd) const
{
    auto u = *this;

    for (int i = 0; i < parametersToAdd.size(); ++i)
        u.addParameter (parametersToAdd.getAllKeys()[i],
                        parametersToAdd.getAllValues()[i]);

    return u;
}

URL URL::withPOSTData (const String& newPostData) const
{
    return withPOSTData (MemoryBlock (newPostData.toRawUTF8(), newPostData.getNumBytesAsUTF8()));
}

URL URL::withPOSTData (const MemoryBlock& newPostData) const
{
    auto u = *this;
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
    auto u = *this;

    for (int i = u.filesToUpload.size(); --i >= 0;)
        if (u.filesToUpload.getObjectPointerUnchecked (i)->parameterName == f->parameterName)
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
    auto result = s.replaceCharacter ('+', ' ');

    if (! result.containsChar ('%'))
        return result;

    // We need to operate on the string as raw UTF8 chars, and then recombine them into unicode
    // after all the replacements have been made, so that multi-byte chars are handled.
    Array<char> utf8 (result.toRawUTF8(), (int) result.getNumBytesAsUTF8());

    for (int i = 0; i < utf8.size(); ++i)
    {
        if (utf8.getUnchecked(i) == '%')
        {
            auto hexDigit1 = CharacterFunctions::getHexDigitValue ((juce_wchar) (uint8) utf8 [i + 1]);
            auto hexDigit2 = CharacterFunctions::getHexDigitValue ((juce_wchar) (uint8) utf8 [i + 2]);

            if (hexDigit1 >= 0 && hexDigit2 >= 0)
            {
                utf8.set (i, (char) ((hexDigit1 << 4) + hexDigit2));
                utf8.removeRange (i + 1, 2);
            }
        }
    }

    return String::fromUTF8 (utf8.getRawDataPointer(), utf8.size());
}

String URL::addEscapeChars (const String& s, bool isParameter, bool roundBracketsAreLegal)
{
    String legalChars (isParameter ? "_-.~"
                                   : ",$_-.*!'");

    if (roundBracketsAreLegal)
        legalChars += "()";

    Array<char> utf8 (s.toRawUTF8(), (int) s.getNumBytesAsUTF8());

    for (int i = 0; i < utf8.size(); ++i)
    {
        auto c = utf8.getUnchecked(i);

        if (! (CharacterFunctions::isLetterOrDigit (c)
                 || legalChars.containsChar ((juce_wchar) c)))
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
    auto u = toString (true);

    if (u.containsChar ('@') && ! u.containsChar (':'))
        u = "mailto:" + u;

    return Process::openDocument (u, {});
}

//==============================================================================
std::unique_ptr<InputStream> URL::createInputStream (bool usePostCommand,
                                                     OpenStreamProgressCallback* cb,
                                                     void* context,
                                                     String headers,
                                                     int timeOutMs,
                                                     StringPairArray* responseHeaders,
                                                     int* statusCode,
                                                     int numRedirectsToFollow,
                                                     String httpRequestCmd) const
{
    std::function<bool (int, int)> callback;

    if (cb != nullptr)
        callback = [context, cb] (int sent, int total) { return cb (context, sent, total); };

    return createInputStream (InputStreamOptions (toHandling (usePostCommand))
                                .withProgressCallback (std::move (callback))
                                .withExtraHeaders (headers)
                                .withConnectionTimeoutMs (timeOutMs)
                                .withResponseHeaders (responseHeaders)
                                .withStatusCode (statusCode)
                                .withNumRedirectsToFollow(numRedirectsToFollow)
                                .withHttpRequestCmd (httpRequestCmd));
}

std::unique_ptr<URL::DownloadTask> URL::downloadToFile (const File& targetLocation,
                                                        String extraHeaders,
                                                        DownloadTask::Listener* listener,
                                                        bool usePostCommand)
{
    auto options = DownloadTaskOptions().withExtraHeaders (std::move (extraHeaders))
                                        .withListener (listener)
                                        .withUsePost (usePostCommand);
    return downloadToFile (targetLocation, std::move (options));
}

} // namespace juce
