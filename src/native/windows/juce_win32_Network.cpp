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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


#ifndef INTERNET_FLAG_NEED_FILE
 #define INTERNET_FLAG_NEED_FILE 0x00000010
#endif

#ifndef INTERNET_OPTION_DISABLE_AUTODIAL
 #define INTERNET_OPTION_DISABLE_AUTODIAL 70
#endif

//==============================================================================
#ifndef WORKAROUND_TIMEOUT_BUG
 //#define WORKAROUND_TIMEOUT_BUG 1
#endif

#if WORKAROUND_TIMEOUT_BUG
// Required because of a Microsoft bug in setting a timeout
class InternetConnectThread  : public Thread
{
public:
    InternetConnectThread (URL_COMPONENTS& uc_, HINTERNET sessionHandle_, HINTERNET& connection_, const bool isFtp_)
        : Thread ("Internet"), uc (uc_), sessionHandle (sessionHandle_), connection (connection_), isFtp (isFtp_)
    {
        startThread();
    }

    ~InternetConnectThread()
    {
        stopThread (60000);
    }

    void run()
    {
        connection = InternetConnect (sessionHandle, uc.lpszHostName,
                                      uc.nPort, _T(""), _T(""),
                                      isFtp ? INTERNET_SERVICE_FTP
                                            : INTERNET_SERVICE_HTTP,
                                      0, 0);
        notify();
    }

private:
    URL_COMPONENTS& uc;
    HINTERNET sessionHandle;
    HINTERNET& connection;
    const bool isFtp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InternetConnectThread);
};
#endif


//==============================================================================
class WebInputStream  : public InputStream
{
public:
    //==============================================================================
    WebInputStream (const String& address_, bool isPost_, const MemoryBlock& postData_,
                    URL::OpenStreamProgressCallback* progressCallback, void* progressCallbackContext,
                    const String& headers_, int timeOutMs_, StringPairArray* responseHeaders)
      : connection (0), request (0),
        address (address_), headers (headers_), postData (postData_), position (0),
        finished (false), isPost (isPost_), timeOutMs (timeOutMs_)
    {
        createConnection (progressCallback, progressCallbackContext);

        if (responseHeaders != nullptr && ! isError())
        {
            DWORD bufferSizeBytes = 4096;

            for (;;)
            {
                HeapBlock<char> buffer ((size_t) bufferSizeBytes);

                if (HttpQueryInfo (request, HTTP_QUERY_RAW_HEADERS_CRLF, buffer.getData(), &bufferSizeBytes, 0))
                {
                    StringArray headersArray;
                    headersArray.addLines (reinterpret_cast <const WCHAR*> (buffer.getData()));

                    for (int i = 0; i < headersArray.size(); ++i)
                    {
                        const String& header = headersArray[i];
                        const String key (header.upToFirstOccurrenceOf (": ", false, false));
                        const String value (header.fromFirstOccurrenceOf (": ", false, false));
                        const String previousValue ((*responseHeaders) [key]);

                        responseHeaders->set (key, previousValue.isEmpty() ? value : (previousValue + "," + value));
                    }

                    break;
                }

                if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                    break;
            }

        }
    }

    ~WebInputStream()
    {
        close();
    }

    //==============================================================================
    bool isError() const        { return request == 0; }
    bool isExhausted()          { return finished; }
    int64 getPosition()         { return position; }

    int64 getTotalLength()
    {
        if (! isError())
        {
            DWORD index = 0, result = 0, size = sizeof (result);

            if (HttpQueryInfo (request, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &result, &size, &index))
                return (int64) result;
        }

        return -1;
    }

    int read (void* buffer, int bytesToRead)
    {
        DWORD bytesRead = 0;

        if (! (finished || isError()))
        {
            InternetReadFile (request, buffer, bytesToRead, &bytesRead);
            position += bytesRead;

            if (bytesRead == 0)
                finished = true;
        }

        return (int) bytesRead;
    }

    bool setPosition (int64 wantedPos)
    {
        if (isError())
            return false;

        if (wantedPos != position)
        {
            finished = false;
            position = (int64) InternetSetFilePointer (request, (LONG) wantedPos, 0, FILE_BEGIN, 0);

            if (position == wantedPos)
                return true;

            if (wantedPos < position)
            {
                close();
                position = 0;
                createConnection (0, 0);
            }

            skipNextBytes (wantedPos - position);
        }

        return true;
    }

private:
    //==============================================================================
    HINTERNET connection, request;
    String address, headers;
    MemoryBlock postData;
    int64 position;
    bool finished;
    const bool isPost;
    int timeOutMs;

    void close()
    {
        if (request != 0)
        {
            InternetCloseHandle (request);
            request = 0;
        }

        if (connection != 0)
        {
            InternetCloseHandle (connection);
            connection = 0;
        }
    }

    void createConnection (URL::OpenStreamProgressCallback* progressCallback,
                           void* progressCallbackContext)
    {
        static HINTERNET sessionHandle = InternetOpen (_T("juce"), INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, 0);

        close();

        if (sessionHandle != 0)
        {
            // break up the url..
            TCHAR file[1024], server[1024], username[1024], password[1024];

            URL_COMPONENTS uc = { 0 };
            uc.dwStructSize = sizeof (uc);
            uc.lpszUrlPath = file;
            uc.dwUrlPathLength = numElementsInArray (file);
            uc.lpszHostName = server;
            uc.dwHostNameLength = numElementsInArray (server);
            uc.lpszUserName = username;
            uc.dwUserNameLength = numElementsInArray (username);
            uc.lpszPassword = password;
            uc.dwPasswordLength = numElementsInArray (password);

            if (InternetCrackUrl (address.toWideCharPointer(), 0, 0, &uc))
            {
                int disable = 1;
                InternetSetOption (sessionHandle, INTERNET_OPTION_DISABLE_AUTODIAL, &disable, sizeof (disable));

                if (timeOutMs == 0)
                    timeOutMs = 30000;
                else if (timeOutMs < 0)
                    timeOutMs = -1;

                InternetSetOption (sessionHandle, INTERNET_OPTION_CONNECT_TIMEOUT, &timeOutMs, sizeof (timeOutMs));

                const bool isFtp = address.startsWithIgnoreCase ("ftp:");

              #if WORKAROUND_TIMEOUT_BUG
                connection = 0;

                {
                    InternetConnectThread connectThread (uc, sessionHandle, connection, isFtp);
                    connectThread.wait (timeOutMs);

                    if (connection == 0)
                    {
                        InternetCloseHandle (sessionHandle);
                        sessionHandle = 0;
                    }
                }
              #else
                connection = InternetConnect (sessionHandle, uc.lpszHostName, uc.nPort,
                                              uc.lpszUserName, uc.lpszPassword,
                                              isFtp ? INTERNET_SERVICE_FTP
                                                    : INTERNET_SERVICE_HTTP,
                                              0, 0);
              #endif

                if (connection != 0)
                {
                    if (isFtp)
                    {
                        request = FtpOpenFile (connection, uc.lpszUrlPath, GENERIC_READ,
                                               FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_NEED_FILE, 0);
                    }
                    else
                    {
                        const TCHAR* mimeTypes[] = { _T("*/*"), 0 };

                        DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES;

                        if (address.startsWithIgnoreCase ("https:"))
                            flags |= INTERNET_FLAG_SECURE;  // (this flag only seems necessary if the OS is running IE6 -
                                                            //  IE7 seems to automatically work out when it's https)

                        request = HttpOpenRequest (connection, isPost ? _T("POST") : _T("GET"),
                                                   uc.lpszUrlPath, 0, 0, mimeTypes, flags, 0);

                        if (request != 0)
                        {
                            INTERNET_BUFFERS buffers = { 0 };
                            buffers.dwStructSize = sizeof (INTERNET_BUFFERS);
                            buffers.lpcszHeader = headers.toWideCharPointer();
                            buffers.dwHeadersLength = headers.length();
                            buffers.dwBufferTotal = (DWORD) postData.getSize();

                            if (HttpSendRequestEx (request, &buffers, 0, HSR_INITIATE, 0))
                            {
                                int bytesSent = 0;

                                for (;;)
                                {
                                    const int bytesToDo = jmin (1024, (int) postData.getSize() - bytesSent);
                                    DWORD bytesDone = 0;

                                    if (bytesToDo > 0
                                         && ! InternetWriteFile (request,
                                                                 static_cast <const char*> (postData.getData()) + bytesSent,
                                                                 bytesToDo, &bytesDone))
                                    {
                                        break;
                                    }

                                    if (bytesToDo == 0 || (int) bytesDone < bytesToDo)
                                    {
                                        if (HttpEndRequest (request, 0, 0, 0))
                                            return;

                                        break;
                                    }

                                    bytesSent += bytesDone;

                                    if (progressCallback != nullptr && ! progressCallback (progressCallbackContext, bytesSent, postData.getSize()))
                                        break;
                                }
                            }
                        }

                        close();
                    }
                }
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebInputStream);
};

InputStream* URL::createNativeStream (const String& address, bool isPost, const MemoryBlock& postData,
                                      OpenStreamProgressCallback* progressCallback, void* progressCallbackContext,
                                      const String& headers, const int timeOutMs, StringPairArray* responseHeaders)
{
    ScopedPointer <WebInputStream> wi (new WebInputStream (address, isPost, postData,
                                                           progressCallback, progressCallbackContext,
                                                           headers, timeOutMs, responseHeaders));

    return wi->isError() ? nullptr : wi.release();
}


//==============================================================================
namespace MACAddressHelpers
{
    void getViaGetAdaptersInfo (Array<MACAddress>& result)
    {
        DynamicLibraryLoader dll ("iphlpapi.dll");
        DynamicLibraryImport (GetAdaptersInfo, getAdaptersInfo, DWORD, dll, (PIP_ADAPTER_INFO, PULONG))

        if (getAdaptersInfo != 0)
        {
            ULONG len = sizeof (IP_ADAPTER_INFO);
            MemoryBlock mb;
            PIP_ADAPTER_INFO adapterInfo = (PIP_ADAPTER_INFO) mb.getData();

            if (getAdaptersInfo (adapterInfo, &len) == ERROR_BUFFER_OVERFLOW)
            {
                mb.setSize (len);
                adapterInfo = (PIP_ADAPTER_INFO) mb.getData();
            }

            if (getAdaptersInfo (adapterInfo, &len) == NO_ERROR)
            {
                for (PIP_ADAPTER_INFO adapter = adapterInfo; adapter != 0; adapter = adapter->Next)
                {
                    if (adapter->AddressLength >= 6)
                        result.addIfNotAlreadyThere (MACAddress (adapter->Address));
                }
            }
        }
    }

    void getViaNetBios (Array<MACAddress>& result)
    {
        DynamicLibraryLoader dll ("netapi32.dll");
        DynamicLibraryImport (Netbios, NetbiosCall, UCHAR, dll, (PNCB))

        if (NetbiosCall != 0)
        {
            LANA_ENUM enums = { 0 };

            {
                NCB ncb = { 0 };
                ncb.ncb_command = NCBENUM;
                ncb.ncb_buffer = (unsigned char*) &enums;
                ncb.ncb_length = sizeof (LANA_ENUM);
                NetbiosCall (&ncb);
            }

            for (int i = 0; i < enums.length; ++i)
            {
                NCB ncb2 = { 0 };
                ncb2.ncb_command = NCBRESET;
                ncb2.ncb_lana_num = enums.lana[i];

                if (NetbiosCall (&ncb2) == 0)
                {
                    NCB ncb = { 0 };
                    memcpy (ncb.ncb_callname, "*                   ", NCBNAMSZ);
                    ncb.ncb_command = NCBASTAT;
                    ncb.ncb_lana_num = enums.lana[i];

                    struct ASTAT
                    {
                        ADAPTER_STATUS adapt;
                        NAME_BUFFER    NameBuff [30];
                    };

                    ASTAT astat = { 0 };
                    ncb.ncb_buffer = (unsigned char*) &astat;
                    ncb.ncb_length = sizeof (ASTAT);

                    if (NetbiosCall (&ncb) == 0 && astat.adapt.adapter_type == 0xfe)
                        result.addIfNotAlreadyThere (MACAddress (astat.adapt.adapter_address));
                }
            }
        }
    }
}

void MACAddress::findAllAddresses (Array<MACAddress>& result)
{
    MACAddressHelpers::getViaGetAdaptersInfo (result);
    MACAddressHelpers::getViaNetBios (result);
}

//==============================================================================
bool PlatformUtilities::launchEmailWithAttachments (const String& targetEmailAddress,
                                                    const String& emailSubject,
                                                    const String& bodyText,
                                                    const StringArray& filesToAttach)
{
    HMODULE h = LoadLibraryA ("MAPI32.dll");

    typedef ULONG (WINAPI *MAPISendMailType) (LHANDLE, ULONG, lpMapiMessage, ::FLAGS, ULONG);

    MAPISendMailType mapiSendMail = (MAPISendMailType) GetProcAddress (h, "MAPISendMail");
    bool ok = false;

    if (mapiSendMail != 0)
    {
        MapiMessage message = { 0 };
        message.lpszSubject = (LPSTR) emailSubject.toUTF8().getAddress();
        message.lpszNoteText = (LPSTR) bodyText.toUTF8().getAddress();

        MapiRecipDesc recip = { 0 };
        recip.ulRecipClass = MAPI_TO;
        String targetEmailAddress_ (targetEmailAddress);
        if (targetEmailAddress_.isEmpty())
            targetEmailAddress_ = " "; // (Windows Mail can't deal with a blank address)
        recip.lpszName = (LPSTR) targetEmailAddress_.toUTF8().getAddress();
        message.nRecipCount = 1;
        message.lpRecips = &recip;

        HeapBlock <MapiFileDesc> files;
        files.calloc (filesToAttach.size());

        message.nFileCount = filesToAttach.size();
        message.lpFiles = files;

        for (int i = 0; i < filesToAttach.size(); ++i)
        {
            files[i].nPosition = (ULONG) -1;
            files[i].lpszPathName = (LPSTR) filesToAttach[i].toUTF8().getAddress();
        }

        ok = (mapiSendMail (0, 0, &message, MAPI_DIALOG | MAPI_LOGON_UI, 0) == SUCCESS_SUCCESS);
    }

    FreeLibrary (h);
    return ok;
}


#endif
