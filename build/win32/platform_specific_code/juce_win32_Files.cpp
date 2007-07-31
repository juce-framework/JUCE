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
#include "../../../src/juce_core/basics/juce_StandardHeader.h"

#include <ctime>

#ifndef _WIN32_IE
 #define _WIN32_IE 0x0400
#endif
#include <shlobj.h>

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/io/files/juce_File.h"
#include "../../../src/juce_core/io/network/juce_URL.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/io/files/juce_NamedPipe.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"

#ifdef _MSC_VER
  #pragma warning (pop)
#endif

//==============================================================================
const tchar  File::separator        = T('\\');
const tchar* File::separatorString  = T("\\");

//==============================================================================
#if JUCE_ENABLE_WIN98_COMPATIBILITY
UNICODE_FUNCTION (GetFileAttributesW, DWORD, (LPCWSTR))
UNICODE_FUNCTION (SetFileAttributesW, BOOL, (LPCWSTR, DWORD))
UNICODE_FUNCTION (RemoveDirectoryW, BOOL, (LPCWSTR))
UNICODE_FUNCTION (DeleteFileW, BOOL, (LPCWSTR))
UNICODE_FUNCTION (MoveFileW, BOOL, (LPCWSTR, LPCWSTR))
UNICODE_FUNCTION (CopyFileW, BOOL, (LPCWSTR, LPCWSTR, BOOL))
UNICODE_FUNCTION (CreateDirectoryW, BOOL, (LPCWSTR, LPSECURITY_ATTRIBUTES))
UNICODE_FUNCTION (CreateFileW, HANDLE, (LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE))
UNICODE_FUNCTION (CreateNamedPipeW, HANDLE, (LPCWSTR, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPSECURITY_ATTRIBUTES))
UNICODE_FUNCTION (GetTempPathW, DWORD, (DWORD, LPCWSTR))
UNICODE_FUNCTION (SHGetSpecialFolderPathW, BOOL, (HWND, LPCWSTR, int, BOOL))
UNICODE_FUNCTION (GetModuleFileNameW, DWORD, (HMODULE, LPCWSTR, DWORD))
UNICODE_FUNCTION (GetCurrentDirectoryW, DWORD, (DWORD, LPCWSTR))
UNICODE_FUNCTION (SetCurrentDirectoryW, BOOL, (LPCWSTR))
UNICODE_FUNCTION (FindFirstFileW, HANDLE, (LPCWSTR, LPWIN32_FIND_DATAW))
UNICODE_FUNCTION (FindNextFileW, BOOL, (HANDLE, LPWIN32_FIND_DATAW))

void juce_initialiseUnicodeFileFunctions() throw()
{
    if ((SystemStats::getOperatingSystemType() & SystemStats::WindowsNT) != 0)
    {
        HMODULE h = GetModuleHandleA ("kernel32.dll");

        UNICODE_FUNCTION_LOAD (GetFileAttributesW)
        UNICODE_FUNCTION_LOAD (SetFileAttributesW)
        UNICODE_FUNCTION_LOAD (RemoveDirectoryW)
        UNICODE_FUNCTION_LOAD (DeleteFileW)
        UNICODE_FUNCTION_LOAD (MoveFileW)
        UNICODE_FUNCTION_LOAD (CopyFileW)
        UNICODE_FUNCTION_LOAD (CreateDirectoryW)
        UNICODE_FUNCTION_LOAD (CreateFileW)
        UNICODE_FUNCTION_LOAD (CreateNamedPipeW)
        UNICODE_FUNCTION_LOAD (GetTempPathW)
        UNICODE_FUNCTION_LOAD (GetModuleFileNameW)
        UNICODE_FUNCTION_LOAD (GetCurrentDirectoryW)
        UNICODE_FUNCTION_LOAD (SetCurrentDirectoryW)
        UNICODE_FUNCTION_LOAD (FindFirstFileW)
        UNICODE_FUNCTION_LOAD (FindNextFileW)

        h = LoadLibraryA ("shell32.dll");
        UNICODE_FUNCTION_LOAD (SHGetSpecialFolderPathW)
    }
}
#endif

//==============================================================================
bool juce_fileExists (const String& fileName,
                      const bool dontCountDirectories) throw()
{
    if (fileName.isEmpty())
        return false;

#if JUCE_ENABLE_WIN98_COMPATIBILITY
    const DWORD attr = (wGetFileAttributesW != 0) ? wGetFileAttributesW (fileName)
                                                  : GetFileAttributes (fileName);
#else
    const DWORD attr = GetFileAttributesW (fileName);
#endif

    return (dontCountDirectories) ? ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0)
                                  : (attr != 0xffffffff);
}

bool juce_isDirectory (const String& fileName) throw()
{
#if JUCE_ENABLE_WIN98_COMPATIBILITY
    const DWORD attr = (wGetFileAttributesW != 0) ? wGetFileAttributesW (fileName)
                                                  : GetFileAttributes (fileName);
#else
    const DWORD attr = GetFileAttributesW (fileName);
#endif

    return (attr != 0xffffffff)
             && ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

bool juce_canWriteToFile (const String& fileName) throw()
{
#if JUCE_ENABLE_WIN98_COMPATIBILITY
    const DWORD attr = (wGetFileAttributesW != 0) ? wGetFileAttributesW (fileName)
                                                  : GetFileAttributes (fileName);
#else
    const DWORD attr = GetFileAttributesW (fileName);
#endif

    return ((attr & FILE_ATTRIBUTE_READONLY) == 0);
}

bool juce_setFileReadOnly (const String& fileName,
                           bool isReadOnly)
{
#if JUCE_ENABLE_WIN98_COMPATIBILITY
    DWORD attr = (wGetFileAttributesW != 0) ? wGetFileAttributesW (fileName)
                                            : GetFileAttributes (fileName);
#else
    DWORD attr = GetFileAttributesW (fileName);
#endif

    if (attr == 0xffffffff)
        return false;

    if (isReadOnly != juce_canWriteToFile (fileName))
        return true;

    if (isReadOnly)
        attr |= FILE_ATTRIBUTE_READONLY;
    else
        attr &= ~FILE_ATTRIBUTE_READONLY;

#if JUCE_ENABLE_WIN98_COMPATIBILITY
    if (wSetFileAttributesW != 0)
        return wSetFileAttributesW (fileName, attr) != FALSE;

    return SetFileAttributes (fileName, attr) != FALSE;
#else
    return SetFileAttributesW (fileName, attr) != FALSE;
#endif
}

//==============================================================================
bool juce_deleteFile (const String& fileName) throw()
{
#if JUCE_ENABLE_WIN98_COMPATIBILITY
    if (juce_isDirectory (fileName))
        return (wRemoveDirectoryW != 0) ? wRemoveDirectoryW (fileName) != 0
                                        : RemoveDirectory (fileName) != 0;
    else
        return (wDeleteFileW != 0) ? wDeleteFileW (fileName) != 0
                                   : DeleteFile (fileName) != 0;
#else
    if (juce_isDirectory (fileName))
        return RemoveDirectoryW (fileName) != 0;

    return DeleteFileW (fileName) != 0;
#endif
}

bool juce_moveFile (const String& source, const String& dest) throw()
{
#if JUCE_ENABLE_WIN98_COMPATIBILITY
    return (wMoveFileW != 0) ? wMoveFileW (source, dest) != 0
                             : MoveFile (source, dest) != 0;
#else
    return MoveFileW (source, dest) != 0;
#endif
}

bool juce_copyFile (const String& source, const String& dest) throw()
{
#if JUCE_ENABLE_WIN98_COMPATIBILITY
    return (wCopyFileW != 0) ? wCopyFileW (source, dest, false) != 0
                             : CopyFile (source, dest, false) != 0;
#else
    return CopyFileW (source, dest, false) != 0;
#endif
}

void juce_createDirectory (const String& fileName) throw()
{
    if (! juce_fileExists (fileName, true))
    {
#if JUCE_ENABLE_WIN98_COMPATIBILITY
        if (wCreateDirectoryW != 0)
            wCreateDirectoryW (fileName, 0);
        else
            CreateDirectory (fileName, 0);
#else
        CreateDirectoryW (fileName, 0);
#endif
    }
}

//==============================================================================
// return 0 if not possible
void* juce_fileOpen (const String& fileName, bool forWriting) throw()
{
    HANDLE h;

    if (forWriting)
    {
#if JUCE_ENABLE_WIN98_COMPATIBILITY
        if (wCreateFileW != 0)
            h = wCreateFileW (fileName, GENERIC_WRITE, FILE_SHARE_READ, 0,
                              OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        else
            h = CreateFile (fileName, GENERIC_WRITE, FILE_SHARE_READ, 0,
                            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
#else
        h = CreateFileW (fileName, GENERIC_WRITE, FILE_SHARE_READ, 0,
                         OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
#endif

        if (h != INVALID_HANDLE_VALUE)
            SetFilePointer (h, 0, 0, FILE_END);
        else
            h = 0;
    }
    else
    {
#if JUCE_ENABLE_WIN98_COMPATIBILITY
        if (wCreateFileW != 0)
            h = wCreateFileW (fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
        else
            h = CreateFile (fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
#else
        h = CreateFileW (fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
#endif

        if (h == INVALID_HANDLE_VALUE)
            h = 0;
    }

    return (void*) h;
}

void juce_fileClose (void* handle) throw()
{
    CloseHandle (handle);
}

//==============================================================================
int juce_fileRead (void* handle, void* buffer, int size) throw()
{
    DWORD num = 0;
    ReadFile ((HANDLE) handle, buffer, size, &num, 0);
    return num;
}

int juce_fileWrite (void* handle, const void* buffer, int size) throw()
{
    DWORD num;

    WriteFile ((HANDLE) handle,
               buffer, size,
               &num, 0);

    return num;
}

int64 juce_fileSetPosition (void* handle, int64 pos) throw()
{
    LARGE_INTEGER li;
    li.QuadPart = pos;
    li.LowPart = SetFilePointer ((HANDLE) handle,
                                 li.LowPart,
                                 &li.HighPart,
                                 FILE_BEGIN);  // (returns -1 if it fails)

    return li.QuadPart;
}

int64 juce_fileGetPosition (void* handle) throw()
{
    LARGE_INTEGER li;
    li.QuadPart = 0;
    li.LowPart = SetFilePointer ((HANDLE) handle,
                                 0, &li.HighPart,
                                 FILE_CURRENT);  // (returns -1 if it fails)

    return jmax ((int64) 0, li.QuadPart);
}

void juce_fileFlush (void* handle) throw()
{
    FlushFileBuffers ((HANDLE) handle);
}

int64 juce_getFileSize (const String& fileName) throw()
{
    void* const handle = juce_fileOpen (fileName, false);

    if (handle != 0)
    {
        LARGE_INTEGER li;
        li.LowPart = GetFileSize (handle, (LPDWORD) &li.HighPart);
        juce_fileClose (handle);

        if (li.LowPart != INVALID_FILE_SIZE || GetLastError() != NO_ERROR)
            return li.QuadPart;
    }

    return 0;
}

//==============================================================================
static int64 fileTimeToTime (const FILETIME* const ft) throw()
{
    // tell me if this fails!
    static_jassert (sizeof (ULARGE_INTEGER) == sizeof (FILETIME));

#if JUCE_GCC
    return (((const ULARGE_INTEGER*) ft)->QuadPart - 116444736000000000LL) / 10000;
#else
    return (((const ULARGE_INTEGER*) ft)->QuadPart - 116444736000000000) / 10000;
#endif
}

static void timeToFileTime (const int64 time, FILETIME* const ft) throw()
{
#if JUCE_GCC
    ((ULARGE_INTEGER*) ft)->QuadPart = time * 10000 + 116444736000000000LL;
#else
    ((ULARGE_INTEGER*) ft)->QuadPart = time * 10000 + 116444736000000000;
#endif
}

void juce_getFileTimes (const String& fileName,
                        int64& modificationTime,
                        int64& accessTime,
                        int64& creationTime) throw()
{
    creationTime = accessTime = modificationTime = 0;
    void* const h = juce_fileOpen (fileName, false);

    if (h != 0)
    {
        FILETIME m, a, c;

        if (GetFileTime ((HANDLE) h, &c, &a, &m))
        {
            creationTime = fileTimeToTime (&c);
            accessTime = fileTimeToTime (&a);
            modificationTime = fileTimeToTime (&m);
        }

        juce_fileClose (h);
    }
}

bool juce_setFileTimes (const String& fileName,
                        int64 modificationTime,
                        int64 accessTime,
                        int64 creationTime) throw()
{
    FILETIME m, a, c;

    if (modificationTime > 0)
        timeToFileTime (modificationTime, &m);

    if (accessTime > 0)
        timeToFileTime (accessTime, &a);

    if (creationTime > 0)
        timeToFileTime (creationTime, &c);

    void* const h = juce_fileOpen (fileName, true);
    bool ok = false;

    if (h != 0)
    {
        ok = SetFileTime ((HANDLE) h,
                          (creationTime > 0)     ? &c : 0,
                          (accessTime > 0)       ? &a : 0,
                          (modificationTime > 0) ? &m : 0) != 0;
        juce_fileClose (h);
    }

    return ok;
}

//==============================================================================
// return '\0' separated list of strings
const StringArray juce_getFileSystemRoots() throw()
{
    TCHAR buffer [2048];
    buffer[0] = 0;
    buffer[1] = 0;
    GetLogicalDriveStrings (2048, buffer);

    TCHAR* n = buffer;
    StringArray roots;

    while (*n != 0)
    {
        roots.add (String (n));

        while (*n++ != 0)
        {
        }
    }

    roots.sort (true);
    return roots;
}

//==============================================================================
const String juce_getVolumeLabel (const String& filenameOnVolume,
                                  int& volumeSerialNumber) throw()
{
    TCHAR n [4];
    n[0] = *(const TCHAR*) filenameOnVolume;
    n[1] = L':';
    n[2] = L'\\';
    n[3] = 0;

    TCHAR dest [64];
    DWORD serialNum;

    if (! GetVolumeInformation (n, dest, 64, (DWORD*) &serialNum, 0, 0, 0, 0))
    {
        dest[0] = 0;
        serialNum = 0;
    }

    volumeSerialNumber = serialNum;
    return String (dest);
}

int64 File::getBytesFreeOnVolume() const throw()
{
    String fn (getFullPathName());
    if (fn[1] == T(':'))
        fn = fn.substring (0, 2) + T("\\");

    ULARGE_INTEGER spc;
    ULARGE_INTEGER tot;
    ULARGE_INTEGER totFree;

    if (GetDiskFreeSpaceEx (fn, &spc, &tot, &totFree))
        return (int64)(spc.QuadPart);

    return 0;
}

//==============================================================================
static unsigned int getWindowsDriveType (const String& fileName) throw()
{
    TCHAR n[4];
    n[0] = *(const TCHAR*) fileName;
    n[1] = L':';
    n[2] = L'\\';
    n[3] = 0;

    return GetDriveType (n);
}

bool File::isOnCDRomDrive() const throw()
{
    return getWindowsDriveType (getFullPathName()) == DRIVE_CDROM;
}

bool File::isOnHardDisk() const throw()
{
    if (fullPath.isEmpty())
        return false;

    const unsigned int n = getWindowsDriveType (getFullPathName());

    if (fullPath.toLowerCase()[0] <= 'b'
         && fullPath[1] == T(':'))
    {
        return n != DRIVE_REMOVABLE;
    }
    else
    {
        return n != DRIVE_CDROM && n != DRIVE_REMOTE;
    }
}

bool File::isOnRemovableDrive() const throw()
{
    if (fullPath.isEmpty())
        return false;

    const unsigned int n = getWindowsDriveType (getFullPathName());

    return n == DRIVE_CDROM
        || n == DRIVE_REMOTE
        || n == DRIVE_REMOVABLE
        || n == DRIVE_RAMDISK;
}

//==============================================================================
#define MAX_PATH_CHARS (MAX_PATH + 256)

static const File juce_getSpecialFolderPath (int type) throw()
{
#if JUCE_ENABLE_WIN98_COMPATIBILITY
    if (wSHGetSpecialFolderPathW != 0)
    {
        WCHAR path [MAX_PATH_CHARS];

        if (wSHGetSpecialFolderPathW (0, path, type, 0))
            return File (String (path));
    }
    else
    {
        TCHAR path [MAX_PATH_CHARS];

        if (SHGetSpecialFolderPath (0, path, type, 0))
            return File (String (path));
    }
#else
    WCHAR path [MAX_PATH_CHARS];

    if (SHGetSpecialFolderPathW (0, path, type, 0))
        return File (String (path));
#endif

    return File::nonexistent;
}

const File JUCE_CALLTYPE File::getSpecialLocation (const SpecialLocationType type)
{
    switch (type)
    {
    case userHomeDirectory:
    case userDocumentsDirectory:
        return juce_getSpecialFolderPath (CSIDL_PERSONAL);

    case userDesktopDirectory:
        return juce_getSpecialFolderPath (CSIDL_DESKTOP);

    case userApplicationDataDirectory:
        return juce_getSpecialFolderPath (CSIDL_APPDATA);

    case commonApplicationDataDirectory:
        return juce_getSpecialFolderPath (CSIDL_COMMON_APPDATA);

    case globalApplicationsDirectory:
        return juce_getSpecialFolderPath (CSIDL_PROGRAM_FILES);

    case tempDirectory:
#if JUCE_ENABLE_WIN98_COMPATIBILITY
        if (wGetTempPathW != 0)
        {
            WCHAR dest [2048];
            dest[0] = 0;
            wGetTempPathW (2048, dest);
            return File (String (dest));
        }
        else
        {
            TCHAR dest [2048];
            dest[0] = 0;
            GetTempPath (2048, dest);
            return File (String (dest));
        }
#else
        {
            WCHAR dest [2048];
            dest[0] = 0;
            GetTempPathW (2048, dest);
            return File (String (dest));
        }
#endif

    case currentExecutableFile:
    case currentApplicationFile:
        {
            HINSTANCE moduleHandle = (HINSTANCE) PlatformUtilities::getCurrentModuleInstanceHandle();

#if JUCE_ENABLE_WIN98_COMPATIBILITY
            if (wGetModuleFileNameW != 0)
            {
                WCHAR dest [MAX_PATH_CHARS];
                dest[0] = 0;
                wGetModuleFileNameW (moduleHandle, dest, MAX_PATH_CHARS);
                return File (String (dest));
            }
            else
            {
                TCHAR dest [MAX_PATH_CHARS];
                dest[0] = 0;
                GetModuleFileName (moduleHandle, dest, MAX_PATH_CHARS);
                return File (String (dest));
            }
#else
            WCHAR dest [MAX_PATH_CHARS];
            dest[0] = 0;
            GetModuleFileNameW (moduleHandle, dest, MAX_PATH_CHARS);
            return File (String (dest));
#endif
        }
        break;

    default:
        jassertfalse // unknown type?
        break;
    }

    return File::nonexistent;
}


void juce_setCurrentExecutableFileName (const String&) throw()
{
    // n/a on windows
}

//==============================================================================
const File File::getCurrentWorkingDirectory() throw()
{
#if JUCE_ENABLE_WIN98_COMPATIBILITY
    if (wGetCurrentDirectoryW != 0)
    {
        WCHAR dest [MAX_PATH_CHARS];
        dest[0] = 0;
        wGetCurrentDirectoryW (MAX_PATH_CHARS, dest);
        return File (String (dest));
    }
    else
    {
        TCHAR dest [MAX_PATH_CHARS];
        dest[0] = 0;
        GetCurrentDirectory (MAX_PATH_CHARS, dest);
        return File (String (dest));
    }
#else
    WCHAR dest [MAX_PATH_CHARS];
    dest[0] = 0;
    GetCurrentDirectoryW (MAX_PATH_CHARS, dest);
    return File (String (dest));
#endif
}

bool File::setAsCurrentWorkingDirectory() const throw()
{
#if JUCE_ENABLE_WIN98_COMPATIBILITY
    return (wSetCurrentDirectoryW != 0) ? wSetCurrentDirectoryW (getFullPathName()) != FALSE
                                        : SetCurrentDirectory (getFullPathName()) != FALSE;
#else
    return SetCurrentDirectoryW (getFullPathName()) != FALSE;
#endif
}

//==============================================================================
template <class FindDataType>
static void getFindFileInfo (FindDataType& findData,
                             String& filename, bool* const isDir, bool* const isHidden,
                             int64* const fileSize, Time* const modTime, Time* const creationTime,
                             bool* const isReadOnly) throw()
{
    filename = findData.cFileName;

    if (isDir != 0)
        *isDir = ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);

    if (isHidden != 0)
        *isHidden = ((findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0);

    if (fileSize != 0)
        *fileSize = findData.nFileSizeLow + (((int64) findData.nFileSizeHigh) << 32);

    if (modTime != 0)
        *modTime = fileTimeToTime (&findData.ftLastWriteTime);

    if (creationTime != 0)
        *creationTime = fileTimeToTime (&findData.ftCreationTime);

    if (isReadOnly != 0)
        *isReadOnly = ((findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0);

}

void* juce_findFileStart (const String& directory, const String& wildCard, String& firstResult,
                          bool* isDir, bool* isHidden, int64* fileSize,
                          Time* modTime, Time* creationTime, bool* isReadOnly) throw()
{
    String wc (directory);

    if (! wc.endsWithChar (File::separator))
        wc += File::separator;

    wc += wildCard;

#if JUCE_ENABLE_WIN98_COMPATIBILITY
    if (wFindFirstFileW != 0)
    {
        WIN32_FIND_DATAW findData;
        HANDLE h = wFindFirstFileW (wc, &findData);

        if (h != INVALID_HANDLE_VALUE)
        {
            getFindFileInfo (findData, firstResult, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly);
            return h;
        }
    }
    else
    {
        WIN32_FIND_DATA findData;
        HANDLE h = FindFirstFile (wc, &findData);

        if (h != INVALID_HANDLE_VALUE)
        {
            getFindFileInfo (findData, firstResult, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly);
            return h;
        }
    }
#else
    WIN32_FIND_DATAW findData;
    HANDLE h = FindFirstFileW (wc, &findData);

    if (h != INVALID_HANDLE_VALUE)
    {
        getFindFileInfo (findData, firstResult, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly);
        return h;
    }
#endif

    firstResult = String::empty;
    return 0;
}

bool juce_findFileNext (void* handle, String& resultFile,
                        bool* isDir, bool* isHidden, int64* fileSize,
                        Time* modTime, Time* creationTime, bool* isReadOnly) throw()
{
#if JUCE_ENABLE_WIN98_COMPATIBILITY
    if (wFindNextFileW != 0)
    {
        WIN32_FIND_DATAW findData;

        if (handle != 0 && wFindNextFileW ((HANDLE) handle, &findData) != 0)
        {
            getFindFileInfo (findData, resultFile, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly);
            return true;
        }
    }
    else
    {
        WIN32_FIND_DATA findData;

        if (handle != 0 && FindNextFile ((HANDLE) handle, &findData) != 0)
        {
            getFindFileInfo (findData, resultFile, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly);
            return true;
        }
    }
#else
    WIN32_FIND_DATAW findData;

    if (handle != 0 && FindNextFileW ((HANDLE) handle, &findData) != 0)
    {
        getFindFileInfo (findData, resultFile, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly);
        return true;
    }
#endif

    resultFile = String::empty;
    return false;
}

void juce_findFileClose (void* handle) throw()
{
    FindClose (handle);
}

//==============================================================================
bool juce_launchFile (const String& fileName,
                      const String& parameters) throw()
{
    HINSTANCE hInstance = 0;

    JUCE_TRY
    {
        hInstance = ShellExecute (0, 0, fileName, parameters, 0, SW_SHOWDEFAULT);
    }
    JUCE_CATCH_ALL

    return hInstance > (HINSTANCE) 32;
}

//==============================================================================
struct NamedPipeInternal
{
    HANDLE pipeH;
    HANDLE cancelEvent;
    bool connected, createdPipe;

    NamedPipeInternal()
        : pipeH (0),
          cancelEvent (0),
          connected (false),
          createdPipe (false)
    {
        cancelEvent = CreateEvent (0, FALSE, FALSE, 0);
    }

    ~NamedPipeInternal()
    {
        disconnect();

        if (pipeH != 0)
            CloseHandle (pipeH);

        CloseHandle (cancelEvent);
    }

    bool connect (const int timeOutMs)
    {
        if (! createdPipe)
            return true;

        if (! connected)
        {
            OVERLAPPED over;
            zerostruct (over);

            over.hEvent = CreateEvent (0, TRUE, FALSE, 0);

            if (ConnectNamedPipe (pipeH, &over))
            {
                connected = false;  // yes, you read that right. In overlapped mode it should always return 0.
            }
            else
            {
                const int err = GetLastError();

                if (err == ERROR_IO_PENDING || err == ERROR_PIPE_LISTENING)
                {
                    HANDLE handles[] = { over.hEvent, cancelEvent };

                    if (WaitForMultipleObjects (2, handles, FALSE,
                                                timeOutMs >= 0 ? timeOutMs : INFINITE) == WAIT_OBJECT_0)
                        connected = true;
                }
                else if (err == ERROR_PIPE_CONNECTED)
                {
                    connected = true;
                }
            }

            CloseHandle (over.hEvent);
        }

        return connected;
    }

    void disconnect()
    {
        if (connected)
        {
            DisconnectNamedPipe (pipeH);
            connected = false;
        }
    }
};

void NamedPipe::close()
{
    NamedPipeInternal* const intern = (NamedPipeInternal*) internal;
    delete intern;
    internal = 0;
}

bool NamedPipe::openInternal (const String& pipeName, const bool createPipe)
{
    close();

    NamedPipeInternal* const intern = new NamedPipeInternal();

    String file ("\\\\.\\pipe\\");
    file += pipeName;

    intern->createdPipe = createPipe;

    if (createPipe)
    {
#if JUCE_ENABLE_WIN98_COMPATIBILITY
        if (wCreateNamedPipeW != 0)
            intern->pipeH = wCreateNamedPipeW (file, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, 0,
                                               1, 64, 64, 0, NULL);
        else
            intern->pipeH = CreateNamedPipe (file, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, 0,
                                             1, 64, 64, 0, NULL);
#else
        intern->pipeH = CreateNamedPipeW (file, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, 0,
                                          1, 64, 64, 0, NULL);
#endif
    }
    else
    {
#if JUCE_ENABLE_WIN98_COMPATIBILITY
        if (wCreateFileW != 0)
            intern->pipeH = wCreateFileW (file, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING,
                                          FILE_FLAG_OVERLAPPED, 0);
        else
            intern->pipeH = CreateFile (file, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING,
                                        FILE_FLAG_OVERLAPPED, 0);
#else
        intern->pipeH = CreateFileW (file, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING,
                                     FILE_FLAG_OVERLAPPED, 0);
#endif
    }

    if (intern->pipeH != INVALID_HANDLE_VALUE)
    {
        internal = intern;
        return true;
    }

    delete intern;
    return false;
}

int NamedPipe::read (void* destBuffer, int maxBytesToRead, int timeOutMilliseconds)
{
    int bytesRead = -1;
    bool waitAgain = true;

    while (waitAgain && internal != 0)
    {
        NamedPipeInternal* const intern = (NamedPipeInternal*) internal;
        waitAgain = false;

        if (! intern->connect (timeOutMilliseconds))
            break;

        if (maxBytesToRead <= 0)
            return 0;

        OVERLAPPED over;
        zerostruct (over);
        over.hEvent = CreateEvent (0, TRUE, FALSE, 0);

        unsigned long numRead;

        if (ReadFile (intern->pipeH, destBuffer, maxBytesToRead, &numRead, &over))
        {
            bytesRead = (int) numRead;
        }
        else if (GetLastError() == ERROR_IO_PENDING)
        {
            HANDLE handles[] = { over.hEvent, intern->cancelEvent };

            if (WaitForMultipleObjects (2, handles, FALSE,
                                        timeOutMilliseconds >= 0 ? timeOutMilliseconds
                                                                 : INFINITE) == WAIT_OBJECT_0)
            {
                if (GetOverlappedResult (intern->pipeH, &over, &numRead, FALSE))
                {
                    bytesRead = (int) numRead;
                }
                else if (GetLastError() == ERROR_BROKEN_PIPE && intern->createdPipe)
                {
                    intern->disconnect();
                    waitAgain = true;
                }
            }
        }
        else
        {
            waitAgain = internal != 0;
            Sleep (5);
        }

        CloseHandle (over.hEvent);
    }

    return bytesRead;
}

int NamedPipe::write (const void* sourceBuffer, int numBytesToWrite, int timeOutMilliseconds)
{
    int bytesWritten = -1;
    NamedPipeInternal* const intern = (NamedPipeInternal*) internal;

    if (intern != 0 && intern->connect (timeOutMilliseconds))
    {
        if (numBytesToWrite <= 0)
            return 0;

        OVERLAPPED over;
        zerostruct (over);

        over.hEvent = CreateEvent (0, TRUE, FALSE, 0);

        unsigned long numWritten;

        if (WriteFile (intern->pipeH, sourceBuffer, numBytesToWrite, &numWritten, &over))
        {
            bytesWritten = (int) numWritten;
        }
        else if (GetLastError() == ERROR_IO_PENDING)
        {
            HANDLE handles[] = { over.hEvent, intern->cancelEvent };

            if (WaitForMultipleObjects (2, handles, FALSE, timeOutMilliseconds >= 0 ? timeOutMilliseconds
                                                                                    : INFINITE) == WAIT_OBJECT_0)
            {
                if (GetOverlappedResult (intern->pipeH, &over, &numWritten, FALSE))
                {
                    bytesWritten = (int) numWritten;
                }
                else if (GetLastError() == ERROR_BROKEN_PIPE && intern->createdPipe)
                {
                    intern->disconnect();
                }
            }
        }

        CloseHandle (over.hEvent);
    }

    return bytesWritten;
}

void NamedPipe::cancelPendingReads()
{
    NamedPipeInternal* const intern = (NamedPipeInternal*) internal;

    if (intern != 0)
        SetEvent (intern->cancelEvent);
}


END_JUCE_NAMESPACE
