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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


#ifndef INTERNET_FLAG_NEED_FILE
  #define INTERNET_FLAG_NEED_FILE 0x00000010
#endif

//==============================================================================
bool juce_isOnLine()
{
    DWORD connectionType;

    return InternetGetConnectedState (&connectionType, 0) != 0
            || (connectionType & (INTERNET_CONNECTION_LAN | INTERNET_CONNECTION_PROXY)) != 0;
}

struct ConnectionAndRequestStruct
{
    HINTERNET connection, request;
};

static HINTERNET sessionHandle = 0;

void* juce_openInternetFile (const String& url,
                             const String& headers,
                             const MemoryBlock& postData,
                             const bool isPost,
                             URL::OpenStreamProgressCallback* callback,
                             void* callbackContext,
                             int timeOutMs)
{
    if (sessionHandle == 0)
        sessionHandle = InternetOpen (_T("juce"),
                                      INTERNET_OPEN_TYPE_PRECONFIG,
                                      0, 0, 0);

    if (sessionHandle != 0)
    {
        // break up the url..
        TCHAR file[1024], server[1024];

        URL_COMPONENTS uc;
        zerostruct (uc);

        uc.dwStructSize = sizeof (uc);
        uc.dwUrlPathLength = sizeof (file);
        uc.dwHostNameLength = sizeof (server);
        uc.lpszUrlPath = file;
        uc.lpszHostName = server;

        if (InternetCrackUrl (url, 0, 0, &uc))
        {
            if (timeOutMs == 0)
                timeOutMs = 30000;
            else if (timeOutMs < 0)
                timeOutMs = -1;

            InternetSetOption (sessionHandle, INTERNET_OPTION_CONNECT_TIMEOUT, &timeOutMs, sizeof (timeOutMs));

            const bool isFtp = url.startsWithIgnoreCase (T("ftp:"));

            HINTERNET connection = InternetConnect (sessionHandle,
                                                    uc.lpszHostName,
                                                    uc.nPort,
                                                    _T(""), _T(""),
                                                    isFtp ? INTERNET_SERVICE_FTP
                                                          : INTERNET_SERVICE_HTTP,
                                                    0, 0);

            if (connection != 0)
            {
                if (isFtp)
                {
                    HINTERNET request = FtpOpenFile (connection,
                                                     uc.lpszUrlPath,
                                                     GENERIC_READ,
                                                     FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_NEED_FILE,
                                                     0);

                    ConnectionAndRequestStruct* const result = new ConnectionAndRequestStruct();
                    result->connection = connection;
                    result->request = request;
                    return result;
                }
                else
                {
                    const TCHAR* mimeTypes[] = { _T("*/*"), 0 };

                    DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;

                    if (url.startsWithIgnoreCase (T("https:")))
                        flags |= INTERNET_FLAG_SECURE;  // (this flag only seems necessary if the OS is running IE6 -
                                                        //  IE7 seems to automatically work out when it's https)

                    HINTERNET request = HttpOpenRequest (connection,
                                                         isPost ? _T("POST")
                                                                : _T("GET"),
                                                         uc.lpszUrlPath,
                                                         0, 0, mimeTypes, flags, 0);

                    if (request != 0)
                    {
                        INTERNET_BUFFERS buffers;
                        zerostruct (buffers);
                        buffers.dwStructSize = sizeof (INTERNET_BUFFERS);
                        buffers.lpcszHeader = (LPCTSTR) headers;
                        buffers.dwHeadersLength = headers.length();
                        buffers.dwBufferTotal = (DWORD) postData.getSize();
                        ConnectionAndRequestStruct* result = 0;

                        if (HttpSendRequestEx (request, &buffers, 0, HSR_INITIATE, 0))
                        {
                            int bytesSent = 0;

                            for (;;)
                            {
                                const int bytesToDo = jmin (1024, (int) postData.getSize() - bytesSent);
                                DWORD bytesDone = 0;

                                if (bytesToDo > 0
                                     && ! InternetWriteFile (request,
                                                             ((const char*) postData.getData()) + bytesSent,
                                                             bytesToDo, &bytesDone))
                                {
                                    break;
                                }

                                if (bytesToDo == 0 || (int) bytesDone < bytesToDo)
                                {
                                    result = new ConnectionAndRequestStruct();
                                    result->connection = connection;
                                    result->request = request;

                                    HttpEndRequest (request, 0, 0, 0);
                                    return result;
                                }

                                bytesSent += bytesDone;

                                if (callback != 0 && ! callback (callbackContext, bytesSent, postData.getSize()))
                                    break;
                            }
                        }

                        InternetCloseHandle (request);
                    }

                    InternetCloseHandle (connection);
                }
            }
        }
    }

    return 0;
}

int juce_readFromInternetFile (void* handle, void* buffer, int bytesToRead)
{
    DWORD bytesRead = 0;

    const ConnectionAndRequestStruct* const crs = (const ConnectionAndRequestStruct*) handle;

    if (crs != 0)
        InternetReadFile (crs->request,
                          buffer, bytesToRead,
                          &bytesRead);

    return bytesRead;
}

int juce_seekInInternetFile (void* handle, int newPosition)
{
    if (handle != 0)
    {
        const ConnectionAndRequestStruct* const crs = (const ConnectionAndRequestStruct*) handle;

        return InternetSetFilePointer (crs->request,
                                       newPosition, 0,
                                       FILE_BEGIN, 0);
    }
    else
    {
        return -1;
    }
}

int64 juce_getInternetFileContentLength (void* handle)
{
    const ConnectionAndRequestStruct* const crs = (const ConnectionAndRequestStruct*) handle;

    if (crs != 0)
    {
        DWORD index = 0;
        DWORD result = 0;
        DWORD size = sizeof (result);

        if (HttpQueryInfo (crs->request,
                           HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
                           &result,
                           &size,
                           &index))
        {
            return (int64) result;
        }
    }

    return -1;
}

void juce_closeInternetFile (void* handle)
{
    if (handle != 0)
    {
        ConnectionAndRequestStruct* const crs = (ConnectionAndRequestStruct*) handle;
        InternetCloseHandle (crs->request);
        InternetCloseHandle (crs->connection);
        delete crs;
    }
}

//==============================================================================
static int getMACAddressViaGetAdaptersInfo (int64* addresses, int maxNum, const bool littleEndian) throw()
{
    int numFound = 0;

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
            PIP_ADAPTER_INFO adapter = adapterInfo;

            while (adapter != 0)
            {
                int64 mac = 0;
                for (unsigned int i = 0; i < adapter->AddressLength; ++i)
                    mac = (mac << 8) | adapter->Address[i];

                if (littleEndian)
                    mac = (int64) ByteOrder::swap ((uint64) mac);

                if (numFound < maxNum && mac != 0)
                    addresses [numFound++] = mac;

                adapter = adapter->Next;
            }
        }
    }

    return numFound;
}

struct ASTAT
{
    ADAPTER_STATUS adapt;
    NAME_BUFFER    NameBuff [30];
};

static int getMACAddressesViaNetBios (int64* addresses, int maxNum, const bool littleEndian) throw()
{
    int numFound = 0;

    DynamicLibraryLoader dll ("netapi32.dll");
    DynamicLibraryImport (Netbios, NetbiosCall, UCHAR, dll, (PNCB))

    if (NetbiosCall != 0)
    {
        NCB ncb;
        zerostruct (ncb);

        ASTAT astat;
        zerostruct (astat);

        LANA_ENUM enums;
        zerostruct (enums);

        ncb.ncb_command = NCBENUM;
        ncb.ncb_buffer = (unsigned char*) &enums;
        ncb.ncb_length = sizeof (LANA_ENUM);
        NetbiosCall (&ncb);

        for (int i = 0; i < enums.length; ++i)
        {
            zerostruct (ncb);
            ncb.ncb_command = NCBRESET;
            ncb.ncb_lana_num = enums.lana[i];

            if (NetbiosCall (&ncb) == 0)
            {
                zerostruct (ncb);
                memcpy (ncb.ncb_callname, "*                   ", NCBNAMSZ);
                ncb.ncb_command = NCBASTAT;
                ncb.ncb_lana_num = enums.lana[i];

                ncb.ncb_buffer = (unsigned char*) &astat;
                ncb.ncb_length = sizeof (ASTAT);

                if (NetbiosCall (&ncb) == 0)
                {
                    if (astat.adapt.adapter_type == 0xfe)
                    {
                        uint64 mac = 0;
                        for (int i = 6; --i >= 0;)
                            mac = (mac << 8) | astat.adapt.adapter_address [littleEndian ? i : (5 - i)];

                        if (numFound < maxNum && mac != 0)
                            addresses [numFound++] = mac;
                    }
                }
            }
        }
    }

    return numFound;
}

int SystemStats::getMACAddresses (int64* addresses, int maxNum, const bool littleEndian) throw()
{
    int numFound = getMACAddressViaGetAdaptersInfo (addresses, maxNum, littleEndian);

    if (numFound == 0)
        numFound = getMACAddressesViaNetBios (addresses, maxNum, littleEndian);

    return numFound;
}

//==============================================================================
typedef ULONG (WINAPI *MAPISendMailType) (LHANDLE, ULONG, lpMapiMessage, ::FLAGS, ULONG);

bool PlatformUtilities::launchEmailWithAttachments (const String& targetEmailAddress,
                                                    const String& emailSubject,
                                                    const String& bodyText,
                                                    const StringArray& filesToAttach)
{
    HMODULE h = LoadLibraryA ("MAPI32.dll");

    MAPISendMailType mapiSendMail = (MAPISendMailType) GetProcAddress (h, "MAPISendMail");
    bool ok = false;

    if (mapiSendMail != 0)
    {
        MapiMessage message;
        zerostruct (message);
        message.lpszSubject = (LPSTR) (LPCSTR) emailSubject;
        message.lpszNoteText = (LPSTR) (LPCSTR) bodyText;

        MapiRecipDesc recip;
        zerostruct (recip);
        recip.ulRecipClass = MAPI_TO;
        String targetEmailAddress_ (targetEmailAddress);
        if (targetEmailAddress_.isEmpty())
            targetEmailAddress_ = " "; // (Windows Mail can't deal with a blank address)
        recip.lpszName = (LPSTR) (LPCSTR) targetEmailAddress_;
        message.nRecipCount = 1;
        message.lpRecips = &recip;

        MemoryBlock mb (sizeof (MapiFileDesc) * filesToAttach.size());
        mb.fillWith (0);
        MapiFileDesc* files = (MapiFileDesc*) mb.getData();

        message.nFileCount = filesToAttach.size();
        message.lpFiles = files;

        for (int i = 0; i < filesToAttach.size(); ++i)
        {
            files[i].nPosition = (ULONG) -1;
            files[i].lpszPathName = (LPSTR) (LPCSTR) filesToAttach [i];
        }

        ok = (mapiSendMail (0, 0, &message, MAPI_DIALOG | MAPI_LOGON_UI, 0) == SUCCESS_SUCCESS);
    }

    FreeLibrary (h);
    return ok;
}


#endif
