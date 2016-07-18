/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_URL_H_INCLUDED
#define JUCE_URL_H_INCLUDED


//==============================================================================
/**
    Represents a URL and has a bunch of useful functions to manipulate it.

    This class can be used to launch URLs in browsers, and also to create
    InputStreams that can read from remote http or ftp sources.
*/
class JUCE_API  URL
{
public:
    //==============================================================================
    /** Creates an empty URL. */
    URL();

    /** Creates a URL from a string.
        This will parse any embedded parameters after a '?' character and store them
        in the list (see getParameterNames etc). If you don't want this to happen, you
        can use createWithoutParsing().
    */
    URL (const String& url);

    /** Creates a copy of another URL. */
    URL (const URL& other);

    /** Destructor. */
    ~URL();

    /** Copies this URL from another one. */
    URL& operator= (const URL& other);

    /** Compares two URLs.
        All aspects of the URLs must be identical for them to match, including any parameters,
        upload files, etc.
    */
    bool operator== (const URL&) const;
    bool operator!= (const URL&) const;

    //==============================================================================
    /** Returns a string version of the URL.

        If includeGetParameters is true and any parameters have been set with the
        withParameter() method, then the string will have these appended on the
        end and url-encoded.
    */
    String toString (bool includeGetParameters) const;

    /** Returns true if the URL is an empty string. */
    bool isEmpty() const noexcept;

    /** True if it seems to be valid. */
    bool isWellFormed() const;

    /** Returns just the domain part of the URL.

        E.g. for "http://www.xyz.com/foobar", this will return "www.xyz.com".
    */
    String getDomain() const;

    /** Returns the path part of the URL.

        E.g. for "http://www.xyz.com/foo/bar?x=1", this will return "foo/bar".
    */
    String getSubPath() const;

    /** Returns the scheme of the URL.

        E.g. for "http://www.xyz.com/foobar", this will return "http". (It won't
        include the colon).
    */
    String getScheme() const;

    /** Attempts to read a port number from the URL.
        @returns the port number, or 0 if none is explicitly specified.
    */
    int getPort() const;

    /** Returns a new version of this URL with a different domain and path.

        E.g. if the URL is "http://www.xyz.com/foo?x=1" and you call this with
        "abc.com/zzz", it'll return "http://abc.com/zzz?x=1".
        @see withNewSubPath
    */
    URL withNewDomainAndPath (const String& newFullPath) const;

    /** Returns a new version of this URL with a different sub-path.

        E.g. if the URL is "http://www.xyz.com/foo?x=1" and you call this with
        "bar", it'll return "http://www.xyz.com/bar?x=1".
        @see withNewDomainAndPath
    */
    URL withNewSubPath (const String& newPath) const;

    /** Returns a new URL that refers to a sub-path relative to this one.

        E.g. if the URL is "http://www.xyz.com/foo" and you call this with
        "bar", it'll return "http://www.xyz.com/foo/bar". Note that there's no way for
        this method to know whether the original URL is a file or directory, so it's
        up to you to make sure it's a directory. It also won't attempt to be smart about
        the content of the childPath string, so if this string is an absolute URL, it'll
        still just get bolted onto the end of the path.

        @see File::getChildFile
    */
    URL getChildURL (const String& subPath) const;

    //==============================================================================
    /** Returns a copy of this URL, with a GET or POST parameter added to the end.

        Any control characters in the value will be encoded.

        e.g. calling "withParameter ("amount", "some fish") for the url "www.fish.com"
        would produce a new url whose toString(true) method would return
        "www.fish.com?amount=some+fish".

        @see getParameterNames, getParameterValues
    */
    URL withParameter (const String& parameterName,
                       const String& parameterValue) const;

    /** Returns a copy of this URL, with a set of GET or POST parameters added.
        This is a convenience method, equivalent to calling withParameter for each value.
        @see withParameter
    */
    URL withParameters (const StringPairArray& parametersToAdd) const;

    /** Returns a copy of this URL, with a file-upload type parameter added to it.

        When performing a POST where one of your parameters is a binary file, this
        lets you specify the file.

        Note that the filename is stored, but the file itself won't actually be read
        until this URL is later used to create a network input stream. If you want to
        upload data from memory, use withDataToUpload().

        @see withDataToUpload
    */
    URL withFileToUpload (const String& parameterName,
                          const File& fileToUpload,
                          const String& mimeType) const;

    /** Returns a copy of this URL, with a file-upload type parameter added to it.

        When performing a POST where one of your parameters is a binary file, this
        lets you specify the file content.
        Note that the filename parameter should not be a full path, it's just the
        last part of the filename.

        @see withFileToUpload
    */
    URL withDataToUpload (const String& parameterName,
                          const String& filename,
                          const MemoryBlock& fileContentToUpload,
                          const String& mimeType) const;

    /** Returns an array of the names of all the URL's parameters.

        E.g. for the url "www.fish.com?type=haddock&amount=some+fish", this array would
        contain two items: "type" and "amount".

        You can call getParameterValues() to get the corresponding value of each
        parameter. Note that the list can contain multiple parameters with the same name.

        @see getParameterValues, withParameter
    */
    const StringArray& getParameterNames() const noexcept       { return parameterNames; }

    /** Returns an array of the values of all the URL's parameters.

        E.g. for the url "www.fish.com?type=haddock&amount=some+fish", this array would
        contain two items: "haddock" and "some fish".

        The values returned will have been cleaned up to remove any escape characters.

        You can call getParameterNames() to get the corresponding name of each
        parameter. Note that the list can contain multiple parameters with the same name.

        @see getParameterNames, withParameter
    */
    const StringArray& getParameterValues() const noexcept      { return parameterValues; }

    /** Returns a copy of this URL, with a block of data to send as the POST data.

        If you're setting the POST data, be careful not to have any parameters set
        as well, otherwise it'll all get thrown in together, and might not have the
        desired effect.

        If the URL already contains some POST data, this will replace it, rather
        than being appended to it.

        This data will only be used if you specify a post operation when you call
        createInputStream().
    */
    URL withPOSTData (const String& postData) const;

    /** Returns a copy of this URL, with a block of data to send as the POST data.

        If you're setting the POST data, be careful not to have any parameters set
        as well, otherwise it'll all get thrown in together, and might not have the
        desired effect.

        If the URL already contains some POST data, this will replace it, rather
        than being appended to it.

        This data will only be used if you specify a post operation when you call
        createInputStream().
    */
    URL withPOSTData (const MemoryBlock& postData) const;

    /** Returns the data that was set using withPOSTData(). */
    String getPostData() const noexcept                  { return postData.toString(); }

    /** Returns the data that was set using withPOSTData() as MemoryBlock. */
    const MemoryBlock& getPostDataAsMemoryBlock() const noexcept { return postData; }

    //==============================================================================
    /** Tries to launch the system's default browser to open the URL.

        Returns true if this seems to have worked.
    */
    bool launchInDefaultBrowser() const;

    //==============================================================================
    /** Takes a guess as to whether a string might be a valid website address.

        This isn't foolproof!
    */
    static bool isProbablyAWebsiteURL (const String& possibleURL);

    /** Takes a guess as to whether a string might be a valid email address.

        This isn't foolproof!
    */
    static bool isProbablyAnEmailAddress (const String& possibleEmailAddress);

    //==============================================================================
    /** This callback function can be used by the createInputStream() method.

        It allows your app to receive progress updates during a lengthy POST operation. If you
        want to continue the operation, this should return true, or false to abort.
    */
    typedef bool (OpenStreamProgressCallback) (void* context, int bytesSent, int totalBytes);

    /** Attempts to open a stream that can read from this URL.

        Note that on some platforms (Android, for example) it's not permitted to do any network
        action from the message thread, so you must only call it from a background thread.

        @param doPostLikeRequest if true, the parameters added to this class will be transferred
                                 via the HTTP headers which is typical for POST requests. Otherwise
                                 the parameters will be added to the URL address. Additionally,
                                 if the parameter httpRequestCmd is not specified (or empty) then this
                                 parameter will determine which HTTP request command will be used
                                 (POST or GET).
        @param progressCallback  if this is non-zero, it lets you supply a callback function
                                 to keep track of the operation's progress. This can be useful
                                 for lengthy POST operations, so that you can provide user feedback.
        @param progressCallbackContext  if a callback is specified, this value will be passed to
                                 the function
        @param extraHeaders      if not empty, this string is appended onto the headers that
                                 are used for the request. It must therefore be a valid set of HTML
                                 header directives, separated by newlines.
        @param connectionTimeOutMs  if 0, this will use whatever default setting the OS chooses. If
                                 a negative number, it will be infinite. Otherwise it specifies a
                                 time in milliseconds.
        @param responseHeaders   if this is non-null, all the (key, value) pairs received as headers
                                 in the response will be stored in this array
        @param statusCode        if this is non-null, it will get set to the http status code, if one
                                 is known, or 0 if a code isn't available
        @param numRedirectsToFollow specifies the number of redirects that will be followed before
                                 returning a response (ignored for Android which follows up to 5 redirects)
        @param httpRequestCmd    Specify which HTTP Request to use. If this is empty, then doPostRequest
                                 will determine the HTTP request.
        @returns    an input stream that the caller must delete, or a null pointer if there was an
                    error trying to open it.
     */
    InputStream* createInputStream (bool doPostLikeRequest,
                                    OpenStreamProgressCallback* progressCallback = nullptr,
                                    void* progressCallbackContext = nullptr,
                                    String extraHeaders = String(),
                                    int connectionTimeOutMs = 0,
                                    StringPairArray* responseHeaders = nullptr,
                                    int* statusCode = nullptr,
                                    int numRedirectsToFollow = 5,
                                    String httpRequestCmd = String()) const;


    //==============================================================================
    /** Tries to download the entire contents of this URL into a binary data block.

        If it succeeds, this will return true and append the data it read onto the end
        of the memory block.

        Note that on some platforms (Android, for example) it's not permitted to do any network
        action from the message thread, so you must only call it from a background thread.

        @param destData         the memory block to append the new data to
        @param usePostCommand   whether to use a POST command to get the data (uses
                                a GET command if this is false)
        @see readEntireTextStream, readEntireXmlStream
    */
    bool readEntireBinaryStream (MemoryBlock& destData,
                                 bool usePostCommand = false) const;

    /** Tries to download the entire contents of this URL as a string.

        If it fails, this will return an empty string, otherwise it will return the
        contents of the downloaded file. If you need to distinguish between a read
        operation that fails and one that returns an empty string, you'll need to use
        a different method, such as readEntireBinaryStream().

        Note that on some platforms (Android, for example) it's not permitted to do any network
        action from the message thread, so you must only call it from a background thread.

        @param usePostCommand   whether to use a POST command to get the data (uses
                                a GET command if this is false)
        @see readEntireBinaryStream, readEntireXmlStream
    */
    String readEntireTextStream (bool usePostCommand = false) const;

    /** Tries to download the entire contents of this URL and parse it as XML.

        If it fails, or if the text that it reads can't be parsed as XML, this will
        return 0.

        When it returns a valid XmlElement object, the caller is responsibile for deleting
        this object when no longer needed.

        Note that on some platforms (Android, for example) it's not permitted to do any network
        action from the message thread, so you must only call it from a background thread.

        @param usePostCommand   whether to use a POST command to get the data (uses
                                a GET command if this is false)

        @see readEntireBinaryStream, readEntireTextStream
    */
    XmlElement* readEntireXmlStream (bool usePostCommand = false) const;

    //==============================================================================
    /** Adds escape sequences to a string to encode any characters that aren't
        legal in a URL.

        E.g. any spaces will be replaced with "%20".

        This is the opposite of removeEscapeChars().

        If isParameter is true, it means that the string is going to be used
        as a parameter, so it also encodes '$' and ',' (which would otherwise
        be legal in a URL.

        @see removeEscapeChars
    */
    static String addEscapeChars (const String& stringToAddEscapeCharsTo,
                                  bool isParameter);

    /** Replaces any escape character sequences in a string with their original
        character codes.

        E.g. any instances of "%20" will be replaced by a space.

        This is the opposite of addEscapeChars().

        @see addEscapeChars
    */
    static String removeEscapeChars (const String& stringToRemoveEscapeCharsFrom);

    /** Returns a URL without attempting to remove any embedded parameters from the string.
        This may be necessary if you need to create a request that involves both POST
        parameters and parameters which are embedded in the URL address itself.
    */
    static URL createWithoutParsing (const String& url);

private:
    //==============================================================================
    String url;
    MemoryBlock postData;
    StringArray parameterNames, parameterValues;

    struct Upload  : public ReferenceCountedObject
    {
        Upload (const String&, const String&, const String&, const File&, MemoryBlock*);
        String parameterName, filename, mimeType;
        File file;
        ScopedPointer<MemoryBlock> data;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Upload)
    };

    friend struct ContainerDeletePolicy<Upload>;
    ReferenceCountedArray<Upload> filesToUpload;

    URL (const String&, int);
    void addParameter (const String&, const String&);
    void createHeadersAndPostData (String&, MemoryBlock&) const;
    URL withUpload (Upload*) const;

    JUCE_LEAK_DETECTOR (URL)
};


#endif   // JUCE_URL_H_INCLUDED
