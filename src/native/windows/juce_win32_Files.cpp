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


//==============================================================================
#ifndef CSIDL_MYMUSIC
 #define CSIDL_MYMUSIC 0x000d
#endif

#ifndef CSIDL_MYVIDEO
 #define CSIDL_MYVIDEO 0x000e
#endif

#ifndef INVALID_FILE_ATTRIBUTES
 #define INVALID_FILE_ATTRIBUTES ((DWORD) -1)
#endif

//==============================================================================
const juce_wchar  File::separator        = '\\';
const juce_wchar* File::separatorString  = T("\\");


//==============================================================================
bool juce_fileExists (const String& fileName, const bool dontCountDirectories)
{
    if (fileName.isEmpty())
        return false;

    const DWORD attr = GetFileAttributes (fileName);
    return dontCountDirectories ? ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0)
                                : (attr != INVALID_FILE_ATTRIBUTES);
}

bool juce_isDirectory (const String& fileName)
{
    const DWORD attr = GetFileAttributes (fileName);
    return ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0) && (attr != INVALID_FILE_ATTRIBUTES);
}

bool juce_canWriteToFile (const String& fileName)
{
    const DWORD attr = GetFileAttributes (fileName);
    return (attr & FILE_ATTRIBUTE_READONLY) == 0;
}

bool juce_setFileReadOnly (const String& fileName, bool isReadOnly)
{
    DWORD attr = GetFileAttributes (fileName);

    if (attr == INVALID_FILE_ATTRIBUTES)
        return false;

    if (isReadOnly != juce_canWriteToFile (fileName))
        return true;

    if (isReadOnly)
        attr |= FILE_ATTRIBUTE_READONLY;
    else
        attr &= ~FILE_ATTRIBUTE_READONLY;

    return SetFileAttributes (fileName, attr) != FALSE;
}

bool File::isHidden() const
{
    return (GetFileAttributes (getFullPathName()) & FILE_ATTRIBUTE_HIDDEN) != 0;
}

//==============================================================================
bool juce_deleteFile (const String& fileName)
{
    if (juce_isDirectory (fileName))
        return RemoveDirectory (fileName) != 0;

    return DeleteFile (fileName) != 0;
}

bool File::moveToTrash() const
{
    if (! exists())
        return true;

    SHFILEOPSTRUCT fos;
    zerostruct (fos);

    // The string we pass in must be double null terminated..
    String doubleNullTermPath (getFullPathName() + " ");
    TCHAR* const p = const_cast <TCHAR*> (static_cast <const TCHAR*> (doubleNullTermPath));
    p [getFullPathName().length()] = 0;

    fos.wFunc = FO_DELETE;
    fos.pFrom = p;
    fos.fFlags = FOF_ALLOWUNDO | FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMATION
                   | FOF_NOCONFIRMMKDIR | FOF_RENAMEONCOLLISION;

    return SHFileOperation (&fos) == 0;
}

bool juce_moveFile (const String& source, const String& dest)
{
    return MoveFile (source, dest) != 0;
}

bool juce_copyFile (const String& source, const String& dest)
{
    return CopyFile (source, dest, false) != 0;
}

void juce_createDirectory (const String& fileName)
{
    if (! juce_fileExists (fileName, true))
        CreateDirectory (fileName, 0);
}

//==============================================================================
// return 0 if not possible
void* juce_fileOpen (const String& fileName, bool forWriting)
{
    HANDLE h;

    if (forWriting)
    {
        h = CreateFile (fileName, GENERIC_WRITE, FILE_SHARE_READ, 0,
                        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

        if (h != INVALID_HANDLE_VALUE)
            SetFilePointer (h, 0, 0, FILE_END);
        else
            h = 0;
    }
    else
    {
        h = CreateFile (fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);

        if (h == INVALID_HANDLE_VALUE)
            h = 0;
    }

    return (void*) h;
}

void juce_fileClose (void* handle)
{
    CloseHandle (handle);
}

//==============================================================================
int juce_fileRead (void* handle, void* buffer, int size)
{
    DWORD num = 0;
    ReadFile ((HANDLE) handle, buffer, size, &num, 0);
    return (int) num;
}

int juce_fileWrite (void* handle, const void* buffer, int size)
{
    DWORD num;
    WriteFile ((HANDLE) handle, buffer, size, &num, 0);
    return (int) num;
}

int64 juce_fileSetPosition (void* handle, int64 pos)
{
    LARGE_INTEGER li;
    li.QuadPart = pos;
    li.LowPart = SetFilePointer ((HANDLE) handle, li.LowPart, &li.HighPart, FILE_BEGIN);  // (returns -1 if it fails)
    return li.QuadPart;
}

int64 juce_fileGetPosition (void* handle)
{
    LARGE_INTEGER li;
    li.QuadPart = 0;
    li.LowPart = SetFilePointer ((HANDLE) handle, 0, &li.HighPart, FILE_CURRENT);  // (returns -1 if it fails)
    return jmax ((int64) 0, li.QuadPart);
}

void juce_fileFlush (void* handle)
{
    FlushFileBuffers ((HANDLE) handle);
}

int64 juce_getFileSize (const String& fileName)
{
    WIN32_FILE_ATTRIBUTE_DATA attributes;

    if (GetFileAttributesEx (fileName, GetFileExInfoStandard, &attributes))
        return (((int64) attributes.nFileSizeHigh) << 32) | attributes.nFileSizeLow;

    return 0;
}

//==============================================================================
static int64 fileTimeToTime (const FILETIME* const ft)
{
    static_jassert (sizeof (ULARGE_INTEGER) == sizeof (FILETIME)); // tell me if this fails!

    return (reinterpret_cast<const ULARGE_INTEGER*> (ft)->QuadPart - literal64bit (116444736000000000)) / 10000;
}

static void timeToFileTime (const int64 time, FILETIME* const ft)
{
    reinterpret_cast<ULARGE_INTEGER*> (ft)->QuadPart = time * 10000 + literal64bit (116444736000000000);
}

void juce_getFileTimes (const String& fileName,
                        int64& modificationTime,
                        int64& accessTime,
                        int64& creationTime)
{
    WIN32_FILE_ATTRIBUTE_DATA attributes;

    if (GetFileAttributesEx (fileName, GetFileExInfoStandard, &attributes))
    {
        modificationTime = fileTimeToTime (&attributes.ftLastWriteTime);
        creationTime = fileTimeToTime (&attributes.ftCreationTime);
        accessTime = fileTimeToTime (&attributes.ftLastAccessTime);
    }
    else
    {
        creationTime = accessTime = modificationTime = 0;
    }
}

bool juce_setFileTimes (const String& fileName,
                        int64 modificationTime,
                        int64 accessTime,
                        int64 creationTime)
{
    void* const h = juce_fileOpen (fileName, true);
    bool ok = false;

    if (h != 0)
    {
        FILETIME m, a, c;
        timeToFileTime (modificationTime, &m);
        timeToFileTime (accessTime, &a);
        timeToFileTime (creationTime, &c);

        ok = SetFileTime ((HANDLE) h,
                          creationTime > 0     ? &c : 0,
                          accessTime > 0       ? &a : 0,
                          modificationTime > 0 ? &m : 0) != 0;

        juce_fileClose (h);
    }

    return ok;
}

//==============================================================================
// return '\0' separated list of strings
const StringArray juce_getFileSystemRoots()
{
    TCHAR buffer [2048];
    buffer[0] = 0;
    buffer[1] = 0;
    GetLogicalDriveStrings (2048, buffer);

    const TCHAR* n = buffer;
    StringArray roots;

    while (*n != 0)
    {
        roots.add (String (n));

        while (*n++ != 0)
        {}
    }

    roots.sort (true);
    return roots;
}

//==============================================================================
static const String getDriveFromPath (const String& path)
{
    if (path.isNotEmpty() && path[1] == ':')
        return path.substring (0, 2) + '\\';

    return path;
}

const String juce_getVolumeLabel (const String& filenameOnVolume,
                                  int& volumeSerialNumber)
{
    TCHAR dest[64];
    DWORD serialNum;

    if (! GetVolumeInformation (getDriveFromPath (filenameOnVolume), dest,
                                numElementsInArray (dest), &serialNum, 0, 0, 0, 0))
    {
        dest[0] = 0;
        serialNum = 0;
    }

    volumeSerialNumber = serialNum;
    return dest;
}

static int64 getDiskSpaceInfo (const String& path, const bool total)
{
    ULARGE_INTEGER spc, tot, totFree;

    if (GetDiskFreeSpaceEx (getDriveFromPath (path), &spc, &tot, &totFree))
        return total ? (int64) tot.QuadPart
                     : (int64) spc.QuadPart;

    return 0;
}

int64 File::getBytesFreeOnVolume() const
{
    return getDiskSpaceInfo (getFullPathName(), false);
}

int64 File::getVolumeTotalSize() const
{
    return getDiskSpaceInfo (getFullPathName(), true);
}

//==============================================================================
static unsigned int getWindowsDriveType (const String& path)
{
    return GetDriveType (getDriveFromPath (path));
}

bool File::isOnCDRomDrive() const
{
    return getWindowsDriveType (getFullPathName()) == DRIVE_CDROM;
}

bool File::isOnHardDisk() const
{
    if (fullPath.isEmpty())
        return false;

    const unsigned int n = getWindowsDriveType (getFullPathName());

    if (fullPath.toLowerCase()[0] <= 'b' && fullPath[1] == ':')
        return n != DRIVE_REMOVABLE;
    else
        return n != DRIVE_CDROM && n != DRIVE_REMOTE;
}

bool File::isOnRemovableDrive() const
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
static const File juce_getSpecialFolderPath (int type)
{
    WCHAR path [MAX_PATH + 256];

    if (SHGetSpecialFolderPath (0, path, type, FALSE))
        return File (String (path));

    return File::nonexistent;
}

const File JUCE_CALLTYPE File::getSpecialLocation (const SpecialLocationType type)
{
    int csidlType = 0;

    switch (type)
    {
        case userHomeDirectory:                 csidlType = CSIDL_PROFILE; break;
        case userDocumentsDirectory:            csidlType = CSIDL_PERSONAL; break;
        case userDesktopDirectory:              csidlType = CSIDL_DESKTOP; break;
        case userApplicationDataDirectory:      csidlType = CSIDL_APPDATA; break;
        case commonApplicationDataDirectory:    csidlType = CSIDL_COMMON_APPDATA; break;
        case globalApplicationsDirectory:       csidlType = CSIDL_PROGRAM_FILES; break;
        case userMusicDirectory:                csidlType = CSIDL_MYMUSIC; break;
        case userMoviesDirectory:               csidlType = CSIDL_MYVIDEO; break;

        case tempDirectory:
        {
            WCHAR dest [2048];
            dest[0] = 0;
            GetTempPath (numElementsInArray (dest), dest);
            return File (String (dest));
        }

        case invokedExecutableFile:
        case currentExecutableFile:
        case currentApplicationFile:
        {
            HINSTANCE moduleHandle = (HINSTANCE) PlatformUtilities::getCurrentModuleInstanceHandle();

            WCHAR dest [MAX_PATH + 256];
            dest[0] = 0;
            GetModuleFileName (moduleHandle, dest, numElementsInArray (dest));
            return File (String (dest));
        }
        break;

        default:
            jassertfalse // unknown type?
            return File::nonexistent;
    }

    return juce_getSpecialFolderPath (csidlType);
}

//==============================================================================
const File File::getCurrentWorkingDirectory()
{
    WCHAR dest [MAX_PATH + 256];
    dest[0] = 0;
    GetCurrentDirectory (numElementsInArray (dest), dest);
    return File (String (dest));
}

bool File::setAsCurrentWorkingDirectory() const
{
    return SetCurrentDirectory (getFullPathName()) != FALSE;
}

//==============================================================================
const String File::getVersion() const
{
    String result;

    DWORD handle = 0;
    DWORD bufferSize = GetFileVersionInfoSize (getFullPathName(), &handle);
    HeapBlock<char> buffer;
    buffer.calloc (bufferSize);

    if (GetFileVersionInfo (getFullPathName(), 0, bufferSize, buffer))
    {
        VS_FIXEDFILEINFO* vffi;
        UINT len = 0;

        if (VerQueryValue (buffer, (LPTSTR) _T("\\"), (LPVOID*) &vffi, &len))
        {
            result << (int) HIWORD (vffi->dwFileVersionMS) << '.'
                   << (int) LOWORD (vffi->dwFileVersionMS) << '.'
                   << (int) HIWORD (vffi->dwFileVersionLS) << '.'
                   << (int) LOWORD (vffi->dwFileVersionLS);
        }
    }

    return result;
}

//==============================================================================
const File File::getLinkedTarget() const
{
    File result (*this);
    String p (getFullPathName());

    if (! exists())
        p += ".lnk";
    else if (getFileExtension() != ".lnk")
        return result;

    ComSmartPtr <IShellLink> shellLink;
    if (SUCCEEDED (shellLink.CoCreateInstance (CLSID_ShellLink, CLSCTX_INPROC_SERVER)))
    {
        ComSmartPtr <IPersistFile> persistFile;
        if (SUCCEEDED (shellLink->QueryInterface (IID_IPersistFile, (LPVOID*) &persistFile)))
        {
            if (SUCCEEDED (persistFile->Load ((const WCHAR*) p, STGM_READ))
                 && SUCCEEDED (shellLink->Resolve (0, SLR_ANY_MATCH | SLR_NO_UI)))
            {
                WIN32_FIND_DATA winFindData;
                WCHAR resolvedPath [MAX_PATH];

                if (SUCCEEDED (shellLink->GetPath (resolvedPath, MAX_PATH, &winFindData, SLGP_UNCPRIORITY)))
                    result = File (resolvedPath);
            }
        }
    }

    return result;
}


//==============================================================================
template <class FindDataType>
static void getFindFileInfo (FindDataType& findData,
                             String& filename, bool* const isDir, bool* const isHidden,
                             int64* const fileSize, Time* const modTime, Time* const creationTime,
                             bool* const isReadOnly)
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
                          Time* modTime, Time* creationTime, bool* isReadOnly)
{
    String wc (directory);

    if (! wc.endsWithChar (File::separator))
        wc += File::separator;

    wc += wildCard;

    WIN32_FIND_DATA findData;
    HANDLE h = FindFirstFile (wc, &findData);

    if (h != INVALID_HANDLE_VALUE)
    {
        getFindFileInfo (findData, firstResult, isDir, isHidden, fileSize,
                         modTime, creationTime, isReadOnly);
        return h;
    }

    firstResult = String::empty;
    return 0;
}

bool juce_findFileNext (void* handle, String& resultFile,
                        bool* isDir, bool* isHidden, int64* fileSize,
                        Time* modTime, Time* creationTime, bool* isReadOnly)
{
    WIN32_FIND_DATA findData;

    if (handle != 0 && FindNextFile ((HANDLE) handle, &findData) != 0)
    {
        getFindFileInfo (findData, resultFile, isDir, isHidden, fileSize,
                         modTime, creationTime, isReadOnly);
        return true;
    }

    resultFile = String::empty;
    return false;
}

void juce_findFileClose (void* handle)
{
    FindClose (handle);
}

//==============================================================================
bool juce_launchFile (const String& fileName, const String& parameters)
{
    HINSTANCE hInstance = 0;

    JUCE_TRY
    {
        hInstance = ShellExecute (0, 0, fileName, parameters, 0, SW_SHOWDEFAULT);
    }
    JUCE_CATCH_ALL

    return hInstance > (HINSTANCE) 32;
}

void File::revealToUser() const
{
    if (isDirectory())
        startAsProcess();
    else if (getParentDirectory().exists())
        getParentDirectory().startAsProcess();
}

//==============================================================================
class NamedPipeInternal
{
public:
    NamedPipeInternal (const String& file, const bool isPipe_)
        : pipeH (0),
          cancelEvent (0),
          connected (false),
          isPipe (isPipe_)
    {
        cancelEvent = CreateEvent (0, FALSE, FALSE, 0);

        pipeH = isPipe ? CreateNamedPipe (file, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, 0,
                                          PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, 0)
                       : CreateFile (file, GENERIC_READ | GENERIC_WRITE, 0, 0,
                                     OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
    }

    ~NamedPipeInternal()
    {
        disconnectPipe();

        if (pipeH != 0)
            CloseHandle (pipeH);

        CloseHandle (cancelEvent);
    }

    bool connect (const int timeOutMs)
    {
        if (! isPipe)
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

    void disconnectPipe()
    {
        if (connected)
        {
            DisconnectNamedPipe (pipeH);
            connected = false;
        }
    }

    HANDLE pipeH;
    HANDLE cancelEvent;
    bool connected, isPipe;
};

void NamedPipe::close()
{
    cancelPendingReads();

    const ScopedLock sl (lock);
    delete static_cast<NamedPipeInternal*> (internal);
    internal = 0;
}

bool NamedPipe::openInternal (const String& pipeName, const bool createPipe)
{
    close();

    ScopedPointer<NamedPipeInternal> intern (new NamedPipeInternal ("\\\\.\\pipe\\" + pipeName, createPipe));

    if (intern->pipeH != INVALID_HANDLE_VALUE)
    {
        internal = intern.release();
        return true;
    }

    return false;
}

int NamedPipe::read (void* destBuffer, int maxBytesToRead, int timeOutMilliseconds)
{
    const ScopedLock sl (lock);
    int bytesRead = -1;
    bool waitAgain = true;

    while (waitAgain && internal != 0)
    {
        NamedPipeInternal* const intern = static_cast<NamedPipeInternal*> (internal);
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
            DWORD waitResult = WaitForMultipleObjects (2, handles, FALSE,
                                                       timeOutMilliseconds >= 0 ? timeOutMilliseconds
                                                                                : INFINITE);
            if (waitResult != WAIT_OBJECT_0)
            {
                // if the operation timed out, let's cancel it...
                CancelIo (intern->pipeH);
                WaitForSingleObject (over.hEvent, INFINITE);  // makes sure cancel is complete
            }

            if (GetOverlappedResult (intern->pipeH, &over, &numRead, FALSE))
            {
                bytesRead = (int) numRead;
            }
            else if (GetLastError() == ERROR_BROKEN_PIPE && intern->isPipe)
            {
                intern->disconnectPipe();
                waitAgain = true;
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
    NamedPipeInternal* const intern = static_cast<NamedPipeInternal*> (internal);

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
            DWORD waitResult;

            waitResult = WaitForMultipleObjects (2, handles, FALSE,
                                                 timeOutMilliseconds >= 0 ? timeOutMilliseconds
                                                                          : INFINITE);

            if (waitResult != WAIT_OBJECT_0)
            {
                CancelIo (intern->pipeH);
                WaitForSingleObject (over.hEvent, INFINITE);
            }

            if (GetOverlappedResult (intern->pipeH, &over, &numWritten, FALSE))
            {
                bytesWritten = (int) numWritten;
            }
            else if (GetLastError() == ERROR_BROKEN_PIPE && intern->isPipe)
            {
                intern->disconnectPipe();
            }
        }

        CloseHandle (over.hEvent);
    }

    return bytesWritten;
}

void NamedPipe::cancelPendingReads()
{
    if (internal != 0)
        SetEvent (static_cast<NamedPipeInternal*> (internal)->cancelEvent);
}


#endif
