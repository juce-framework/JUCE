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

#ifdef _MSC_VER
  #pragma warning (disable: 4514)
  #pragma warning (push)
#endif

#include "win32_headers.h"
#include <wininet.h>
#include <nb30.h>
#include <iphlpapi.h>
#include "../../../src/juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/text/juce_String.h"
#include "juce_win32_DynamicLibraryLoader.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/containers/juce_MemoryBlock.h"

#ifndef INTERNET_FLAG_NEED_FILE
  #define INTERNET_FLAG_NEED_FILE 0x00000010
#endif

#ifdef _MSC_VER
  #pragma warning (pop)
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
                             const String& postText,
                             const bool isPost)
{
    if (sessionHandle == 0)
        sessionHandle = InternetOpen (_T("juce"), INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, 0);

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

        if (InternetCrackUrl (url, 0,
                              ICU_ESCAPE | ICU_DECODE,
                              &uc))
        {
            const bool isFtp = url.startsWithIgnoreCase (T("ftp:"));

            HINTERNET connection = InternetConnect (sessionHandle,
                                                    uc.lpszHostName,
                                                    uc.nPort,
                                                    _T(""), _T(""),
                                                    (isFtp) ? INTERNET_SERVICE_FTP
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
                    const TCHAR* mimeTypes[] = { _T("*"), 0 };

                    HINTERNET request = HttpOpenRequest (connection,
                                                         isPost ? _T("POST")
                                                                : _T("GET"),
                                                         uc.lpszUrlPath,
                                                         0, 0,
                                                         mimeTypes,
                                                         INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);

                    if (request != 0)
                    {
                        // (this header is needed to make webservers process a POST request correctly)
                        const String hdr ("Content-Type: application/x-www-form-urlencoded");

                        if (HttpSendRequest (request,
                                             hdr, hdr.length(),
                                             (void*) (const char*) postText,
                                             postText.length()))
                        {
                            ConnectionAndRequestStruct* const result = new ConnectionAndRequestStruct();
                            result->connection = connection;
                            result->request = request;
                            return result;
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

int juce_getStatusCodeFor (void* handle)
{
    DWORD result = 404;
    const ConnectionAndRequestStruct* const crs = (const ConnectionAndRequestStruct*) handle;

    if (crs != 0)
    {
        DWORD index = 0;
        DWORD size = sizeof (result);

        HttpQueryInfo (crs->request,
                       HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                       &result,
                       &size,
                       &index);
    }

    return (int) result;
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

static int getMACAddressViaGetAdaptersInfo (int64* addresses, int maxNum) throw()
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

                if (numFound < maxNum && mac != 0)
                    addresses [numFound++] = mac;

                adapter = adapter->Next;
            }
        }
    }

    return numFound;
}

static int getMACAddressesViaNetBios (int64* addresses, int maxNum) throw()
{
    int numFound = 0;

    DynamicLibraryLoader dll ("netapi32.dll");
    DynamicLibraryImport (Netbios, NetbiosCall, UCHAR, dll, (PNCB))

    if (NetbiosCall != 0)
    {
        NCB ncb;
        zerostruct (ncb);

        typedef struct _ASTAT_
        {
            ADAPTER_STATUS adapt;
            NAME_BUFFER    NameBuff [30];
        } ASTAT;

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
                        int64 mac = 0;
                        for (unsigned int i = 0; i < 6; ++i)
                            mac = (mac << 8) | astat.adapt.adapter_address[i];

                        if (numFound < maxNum && mac != 0)
                            addresses [numFound++] = mac;
                    }
                }
            }
        }
    }

    return numFound;
}

int SystemStats::getMACAddresses (int64* addresses, int maxNum) throw()
{
    int numFound = getMACAddressViaGetAdaptersInfo (addresses, maxNum);

    if (numFound == 0)
        numFound = getMACAddressesViaNetBios (addresses, maxNum);

    return numFound;
}

END_JUCE_NAMESPACE
