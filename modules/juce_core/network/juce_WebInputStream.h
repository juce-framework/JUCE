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

//==============================================================================
/**
    An InputStream which can be used to read from a given URL.

    @tags{Core}
*/
class JUCE_API WebInputStream  : public InputStream
{
 public:
    /** Creates a new WebInputStream which can be used to read from a URL.

        @param url      the URL that should be retrieved. This parameter may also contain
                        POST data and/or parameters.
        @param addParametersToRequestBody  specifies whether any URL parameters that have
                        been set will be transferred via the request body data or added
                        to the URL address. This will also determine whether a POST or GET
                        command will be used if a custom command is not set.
    */
    WebInputStream (const URL& url, bool addParametersToRequestBody);

    /** Destructor. */
    ~WebInputStream() override;

    /** Add extra headers to the HTTP request.

        Returns a reference to itself so that several methods can be chained.

        @param extraHeaders   this string is appended onto the headers that are used for
                              the request. It must therefore be a valid set of HTML
                              header directives, separated by newlines.
    */
    WebInputStream& withExtraHeaders (const String& extraHeaders);

    /** Override the HTTP command that is sent.

        Returns a reference to itself so that several methods can be chained.

        Note that this command will not change the way parameters are sent. This
        must be specified in the constructor.

        @param customRequestCommand  this string is the custom HTTP request command such
                                     as POST or GET.
    */
    WebInputStream& withCustomRequestCommand (const String& customRequestCommand);

    /** Specify the connection time-out.

        Returns a reference to itself so that several methods can be chained.

        @param timeoutInMs  the number of milliseconds to wait until the connection
                            request is aborted.
    */
    WebInputStream& withConnectionTimeout (int timeoutInMs);

    /** Specify the number of redirects to be followed.

        Returns a reference to itself so that several methods can be chained.

        @param numRedirects  specifies the number of redirects that will be followed
                             before returning a response (ignored for Android which
                             follows up to 5 redirects)
    */
    WebInputStream& withNumRedirectsToFollow (int numRedirects);

    //==============================================================================
    /** Used to receive callbacks for POST data send progress.

        Pass one of these into the connect() method and its postDataSendProgress()
        method will be called periodically with updates on POST data upload progress.
    */
    class JUCE_API Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** This method will be called periodically with updates on POST data upload progress.

            @param request     the original request
            @param bytesSent   the number of bytes sent so far
            @param totalBytes  the total number of bytes to send

            @returns true to continue or false to cancel the upload
        */
        virtual bool postDataSendProgress (WebInputStream& request, int bytesSent, int totalBytes);
    };

    /** Wait until the first byte is ready for reading.

        This method will attempt to connect to the URL given in the constructor
        and block until the status code and all response headers have been received or
        an error has occurred.

        Note that most methods will call connect() internally if they are called without
        an established connection. Therefore, it is not necessary to explicitly
        call connect unless you would like to use a custom listener.

        After a successful call to connect(), getResponseHeaders(), getTotalLength()
        and getStatusCode() will all be non-blocking.

        @param listener    a listener to receive progress callbacks on the status
                           of a POST data upload.

        @see getResponseHeaders, getTotalLength, getStatusCode
    */
    bool connect (Listener* listener);

    /** Returns true if there was an error during the connection attempt. */
    bool isError() const;

    /** Will cancel a blocking read and prevent any subsequent connection attempts. */
    void cancel();

    /** Returns a StringPairArray of the request headers. */
    StringPairArray getRequestHeaders() const;

    /** Returns a StringPairArray of response headers.

        If getResponseHeaders() is called without an established connection, then
        getResponseHeaders() will call connect internally and block until connect
        returns - either due to a successful connection or a connection
        error.

        @see connect
    */
    StringPairArray getResponseHeaders();

    /** Returns the status code returned by the HTTP server

        If getStatusCode() is called without an established connection, then
        getStatusCode() will call connect internally and block until connect
        returns - either due to a successful connection or a connection
        error.

        @see connect
    */
    int getStatusCode();

    //==============================================================================
    /** Returns the total number of bytes available for reading in this stream.

        Note that this is the number of bytes available from the start of the
        stream, not from the current position.

        If getTotalLength() is called without an established connection, then
        getTotalLength() will call connect internally and block until connect
        returns - either due to a successful connection or a connection
        error.

        If the size of the stream isn't actually known, this will return -1.
    */
    int64 getTotalLength() override;

    /** Reads some data from the stream into a memory buffer.

        This method will block until the maxBytesToRead bytes are available.

        This method calls connect() internally if the connection hasn't already
        been established.

        @param destBuffer       the destination buffer for the data. This must not be null.
        @param maxBytesToRead   the maximum number of bytes to read - make sure the
                                memory block passed in is big enough to contain this
                                many bytes. This value must not be negative.

        @returns    the actual number of bytes that were read, which may be less than
                    maxBytesToRead if the stream is exhausted before it gets that far
    */
    int read (void* destBuffer, int maxBytesToRead) override;

    /** Returns true if the stream has no more data to read. */
    bool isExhausted() override;

    /** Returns the offset of the next byte that will be read from the stream.

        @see setPosition
    */
    int64 getPosition() override;

    /** Tries to move the current read position of the stream.

        The position is an absolute number of bytes from the stream's start.

        For a WebInputStream, this method will fail if wantedPos is smaller
        than the current position. If wantedPos is greater than the current
        position, then calling setPosition() is the same as calling read(), i.e.
        the skipped data will still be downloaded, although skipped bytes will
        be discarded immediately.

        @returns  true if the stream manages to reposition itself correctly
        @see getPosition
    */
    bool setPosition (int64 wantedPos) override;

 private:
    static void createHeadersAndPostData (const URL&, String&, MemoryBlock&, bool);
    static StringPairArray parseHttpHeaders (const String&);

    class Pimpl;
    friend class Pimpl;

    std::unique_ptr<Pimpl> pimpl;
    bool hasCalledConnect = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebInputStream)
};

} // namespace juce
