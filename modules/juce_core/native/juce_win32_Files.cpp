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

#ifndef INVALID_FILE_ATTRIBUTES
 #define INVALID_FILE_ATTRIBUTES ((DWORD) -1)
#endif

//==============================================================================
namespace WindowsFileHelpers
{
    DWORD getAtts (const String& path)
    {
        return GetFileAttributes (path.toWideCharPointer());
    }

    int64 fileTimeToTime (const FILETIME* const ft)
    {
        static_jassert (sizeof (ULARGE_INTEGER) == sizeof (FILETIME)); // tell me if this fails!

        return (int64) ((reinterpret_cast<const ULARGE_INTEGER*> (ft)->QuadPart - literal64bit (116444736000000000)) / 10000);
    }

    FILETIME* timeToFileTime (const int64 time, FILETIME* const ft) noexcept
    {
        if (time <= 0)
            return nullptr;

        reinterpret_cast<ULARGE_INTEGER*> (ft)->QuadPart = (ULONGLONG) (time * 10000 + literal64bit (116444736000000000));
        return ft;
    }

    String getDriveFromPath (String path)
    {
        if (path.isNotEmpty() && path[1] == ':' && path[2] == 0)
            path << '\\';

        const size_t numBytes = CharPointer_UTF16::getBytesRequiredFor (path.getCharPointer()) + 4;
        HeapBlock<WCHAR> pathCopy;
        pathCopy.calloc (numBytes, 1);
        path.copyToUTF16 (pathCopy, (int) numBytes);

        if (PathStripToRoot (pathCopy))
            path = static_cast <const WCHAR*> (pathCopy);

        return path;
    }

    int64 getDiskSpaceInfo (const String& path, const bool total)
    {
        ULARGE_INTEGER spc, tot, totFree;

        if (GetDiskFreeSpaceEx (getDriveFromPath (path).toWideCharPointer(), &spc, &tot, &totFree))
            return total ? (int64) tot.QuadPart
                         : (int64) spc.QuadPart;

        return 0;
    }

    unsigned int getWindowsDriveType (const String& path)
    {
        return GetDriveType (getDriveFromPath (path).toWideCharPointer());
    }

    File getSpecialFolderPath (int type)
    {
        WCHAR path [MAX_PATH + 256];

        if (SHGetSpecialFolderPath (0, path, type, FALSE))
            return File (String (path));

        return File::nonexistent;
    }

    File getModuleFileName (HINSTANCE moduleHandle)
    {
        WCHAR dest [MAX_PATH + 256];
        dest[0] = 0;
        GetModuleFileName (moduleHandle, dest, (DWORD) numElementsInArray (dest));
        return File (String (dest));
    }

    Result getResultForLastError()
    {
        TCHAR messageBuffer [256] = { 0 };

        FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr, GetLastError(), MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                       messageBuffer, (DWORD) numElementsInArray (messageBuffer) - 1, nullptr);

        return Result::fail (String (messageBuffer));
    }
}

//==============================================================================
const juce_wchar File::separator = '\\';
const String File::separatorString ("\\");


//==============================================================================
bool File::exists() const
{
    return fullPath.isNotEmpty()
            && WindowsFileHelpers::getAtts (fullPath) != INVALID_FILE_ATTRIBUTES;
}

bool File::existsAsFile() const
{
    return fullPath.isNotEmpty()
            && (WindowsFileHelpers::getAtts (fullPath) & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool File::isDirectory() const
{
    const DWORD attr = WindowsFileHelpers::getAtts (fullPath);
    return ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0) && (attr != INVALID_FILE_ATTRIBUTES);
}

bool File::hasWriteAccess() const
{
    if (exists())
        return (WindowsFileHelpers::getAtts (fullPath) & FILE_ATTRIBUTE_READONLY) == 0;

    // on windows, it seems that even read-only directories can still be written into,
    // so checking the parent directory's permissions would return the wrong result..
    return true;
}

bool File::setFileReadOnlyInternal (const bool shouldBeReadOnly) const
{
    const DWORD oldAtts = WindowsFileHelpers::getAtts (fullPath);

    if (oldAtts == INVALID_FILE_ATTRIBUTES)
        return false;

    const DWORD newAtts = shouldBeReadOnly ? (oldAtts |  FILE_ATTRIBUTE_READONLY)
                                           : (oldAtts & ~FILE_ATTRIBUTE_READONLY);
    return newAtts == oldAtts
            || SetFileAttributes (fullPath.toWideCharPointer(), newAtts) != FALSE;
}

bool File::isHidden() const
{
    return (WindowsFileHelpers::getAtts (fullPath) & FILE_ATTRIBUTE_HIDDEN) != 0;
}

//==============================================================================
bool File::deleteFile() const
{
    if (! exists())
        return true;

    return isDirectory() ? RemoveDirectory (fullPath.toWideCharPointer()) != 0
                         : DeleteFile (fullPath.toWideCharPointer()) != 0;
}

bool File::moveToTrash() const
{
    if (! exists())
        return true;

    // The string we pass in must be double null terminated..
    const size_t numBytes = CharPointer_UTF16::getBytesRequiredFor (fullPath.getCharPointer()) + 8;
    HeapBlock<WCHAR> doubleNullTermPath;
    doubleNullTermPath.calloc (numBytes, 1);
    fullPath.copyToUTF16 (doubleNullTermPath, (int) numBytes);

    SHFILEOPSTRUCT fos = { 0 };
    fos.wFunc = FO_DELETE;
    fos.pFrom = doubleNullTermPath;
    fos.fFlags = FOF_ALLOWUNDO | FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMATION
                   | FOF_NOCONFIRMMKDIR | FOF_RENAMEONCOLLISION;

    return SHFileOperation (&fos) == 0;
}

bool File::copyInternal (const File& dest) const
{
    return CopyFile (fullPath.toWideCharPointer(), dest.getFullPathName().toWideCharPointer(), false) != 0;
}

bool File::moveInternal (const File& dest) const
{
    return MoveFile (fullPath.toWideCharPointer(), dest.getFullPathName().toWideCharPointer()) != 0;
}

Result File::createDirectoryInternal (const String& fileName) const
{
    return CreateDirectory (fileName.toWideCharPointer(), 0) ? Result::ok()
                                                             : WindowsFileHelpers::getResultForLastError();
}

//==============================================================================
int64 juce_fileSetPosition (void* handle, int64 pos)
{
    LARGE_INTEGER li;
    li.QuadPart = pos;
    li.LowPart = SetFilePointer ((HANDLE) handle, (LONG) li.LowPart, &li.HighPart, FILE_BEGIN);  // (returns -1 if it fails)
    return li.QuadPart;
}

void FileInputStream::openHandle()
{
    HANDLE h = CreateFile (file.getFullPathName().toWideCharPointer(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);

    if (h != INVALID_HANDLE_VALUE)
        fileHandle = (void*) h;
    else
        status = WindowsFileHelpers::getResultForLastError();
}

void FileInputStream::closeHandle()
{
    CloseHandle ((HANDLE) fileHandle);
}

size_t FileInputStream::readInternal (void* buffer, size_t numBytes)
{
    if (fileHandle != 0)
    {
        DWORD actualNum = 0;
        if (! ReadFile ((HANDLE) fileHandle, buffer, (DWORD) numBytes, &actualNum, 0))
            status = WindowsFileHelpers::getResultForLastError();

        return (size_t) actualNum;
    }

    return 0;
}

//==============================================================================
void FileOutputStream::openHandle()
{
    HANDLE h = CreateFile (file.getFullPathName().toWideCharPointer(), GENERIC_WRITE, FILE_SHARE_READ, 0,
                           OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if (h != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER li;
        li.QuadPart = 0;
        li.LowPart = SetFilePointer (h, 0, &li.HighPart, FILE_END);

        if (li.LowPart != INVALID_SET_FILE_POINTER)
        {
            fileHandle = (void*) h;
            currentPosition = li.QuadPart;
            return;
        }
    }

    status = WindowsFileHelpers::getResultForLastError();
}

void FileOutputStream::closeHandle()
{
    CloseHandle ((HANDLE) fileHandle);
}

int FileOutputStream::writeInternal (const void* buffer, int numBytes)
{
    if (fileHandle != nullptr)
    {
        DWORD actualNum = 0;
        if (! WriteFile ((HANDLE) fileHandle, buffer, (DWORD) numBytes, &actualNum, 0))
            status = WindowsFileHelpers::getResultForLastError();

        return (int) actualNum;
    }

    return 0;
}

void FileOutputStream::flushInternal()
{
    if (fileHandle != nullptr)
        if (! FlushFileBuffers ((HANDLE) fileHandle))
            status = WindowsFileHelpers::getResultForLastError();
}

Result FileOutputStream::truncate()
{
    if (fileHandle == nullptr)
        return status;

    flush();
    return SetEndOfFile ((HANDLE) fileHandle) ? Result::ok()
                                              : WindowsFileHelpers::getResultForLastError();
}

//==============================================================================
MemoryMappedFile::MemoryMappedFile (const File& file, MemoryMappedFile::AccessMode mode)
    : address (nullptr),
      length (0),
      fileHandle (nullptr)
{
    jassert (mode == readOnly || mode == readWrite);

    DWORD accessMode = GENERIC_READ, createType = OPEN_EXISTING;
    DWORD protect = PAGE_READONLY, access = FILE_MAP_READ;

    if (mode == readWrite)
    {
        accessMode = GENERIC_READ | GENERIC_WRITE;
        createType = OPEN_ALWAYS;
        protect = PAGE_READWRITE;
        access = FILE_MAP_ALL_ACCESS;
    }

    HANDLE h = CreateFile (file.getFullPathName().toWideCharPointer(), accessMode, FILE_SHARE_READ, 0,
                           createType, FILE_ATTRIBUTE_NORMAL, 0);

    if (h != INVALID_HANDLE_VALUE)
    {
        fileHandle = (void*) h;
        const int64 fileSize = file.getSize();

        HANDLE mappingHandle = CreateFileMapping (h, 0, protect, (DWORD) (fileSize >> 32), (DWORD) fileSize, 0);
        if (mappingHandle != 0)
        {
            address = MapViewOfFile (mappingHandle, access, 0, 0, (SIZE_T) fileSize);

            if (address != nullptr)
                length = (size_t) fileSize;

            CloseHandle (mappingHandle);
        }
    }
}

MemoryMappedFile::~MemoryMappedFile()
{
    if (address != nullptr)
        UnmapViewOfFile (address);

    if (fileHandle != nullptr)
        CloseHandle ((HANDLE) fileHandle);
}

//==============================================================================
int64 File::getSize() const
{
    WIN32_FILE_ATTRIBUTE_DATA attributes;

    if (GetFileAttributesEx (fullPath.toWideCharPointer(), GetFileExInfoStandard, &attributes))
        return (((int64) attributes.nFileSizeHigh) << 32) | attributes.nFileSizeLow;

    return 0;
}

void File::getFileTimesInternal (int64& modificationTime, int64& accessTime, int64& creationTime) const
{
    using namespace WindowsFileHelpers;
    WIN32_FILE_ATTRIBUTE_DATA attributes;

    if (GetFileAttributesEx (fullPath.toWideCharPointer(), GetFileExInfoStandard, &attributes))
    {
        modificationTime = fileTimeToTime (&attributes.ftLastWriteTime);
        creationTime     = fileTimeToTime (&attributes.ftCreationTime);
        accessTime       = fileTimeToTime (&attributes.ftLastAccessTime);
    }
    else
    {
        creationTime = accessTime = modificationTime = 0;
    }
}

bool File::setFileTimesInternal (int64 modificationTime, int64 accessTime, int64 creationTime) const
{
    using namespace WindowsFileHelpers;

    bool ok = false;
    HANDLE h = CreateFile (fullPath.toWideCharPointer(), GENERIC_WRITE, FILE_SHARE_READ, 0,
                           OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if (h != INVALID_HANDLE_VALUE)
    {
        FILETIME m, a, c;

        ok = SetFileTime (h,
                          timeToFileTime (creationTime, &c),
                          timeToFileTime (accessTime, &a),
                          timeToFileTime (modificationTime, &m)) != 0;

        CloseHandle (h);
    }

    return ok;
}

//==============================================================================
void File::findFileSystemRoots (Array<File>& destArray)
{
    TCHAR buffer [2048] = { 0 };
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

    for (int i = 0; i < roots.size(); ++i)
        destArray.add (roots [i]);
}

//==============================================================================
String File::getVolumeLabel() const
{
    TCHAR dest[64];
    if (! GetVolumeInformation (WindowsFileHelpers::getDriveFromPath (getFullPathName()).toWideCharPointer(), dest,
                                (DWORD) numElementsInArray (dest), 0, 0, 0, 0, 0))
        dest[0] = 0;

    return dest;
}

int File::getVolumeSerialNumber() const
{
    TCHAR dest[64];
    DWORD serialNum;

    if (! GetVolumeInformation (WindowsFileHelpers::getDriveFromPath (getFullPathName()).toWideCharPointer(), dest,
                                (DWORD) numElementsInArray (dest), &serialNum, 0, 0, 0, 0))
        return 0;

    return (int) serialNum;
}

int64 File::getBytesFreeOnVolume() const
{
    return WindowsFileHelpers::getDiskSpaceInfo (getFullPathName(), false);
}

int64 File::getVolumeTotalSize() const
{
    return WindowsFileHelpers::getDiskSpaceInfo (getFullPathName(), true);
}

//==============================================================================
bool File::isOnCDRomDrive() const
{
    return WindowsFileHelpers::getWindowsDriveType (getFullPathName()) == DRIVE_CDROM;
}

bool File::isOnHardDisk() const
{
    if (fullPath.isEmpty())
        return false;

    const unsigned int n = WindowsFileHelpers::getWindowsDriveType (getFullPathName());

    if (fullPath.toLowerCase()[0] <= 'b' && fullPath[1] == ':')
        return n != DRIVE_REMOVABLE;
    else
        return n != DRIVE_CDROM && n != DRIVE_REMOTE;
}

bool File::isOnRemovableDrive() const
{
    if (fullPath.isEmpty())
        return false;

    const unsigned int n = WindowsFileHelpers::getWindowsDriveType (getFullPathName());

    return n == DRIVE_CDROM
        || n == DRIVE_REMOTE
        || n == DRIVE_REMOVABLE
        || n == DRIVE_RAMDISK;
}

//==============================================================================
File JUCE_CALLTYPE File::getSpecialLocation (const SpecialLocationType type)
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
        case userMusicDirectory:                csidlType = 0x0d /*CSIDL_MYMUSIC*/; break;
        case userMoviesDirectory:               csidlType = 0x0e /*CSIDL_MYVIDEO*/; break;

        case tempDirectory:
        {
            WCHAR dest [2048];
            dest[0] = 0;
            GetTempPath ((DWORD) numElementsInArray (dest), dest);
            return File (String (dest));
        }

        case invokedExecutableFile:
        case currentExecutableFile:
        case currentApplicationFile:
            return WindowsFileHelpers::getModuleFileName ((HINSTANCE) Process::getCurrentModuleInstanceHandle());

        case hostApplicationPath:
            return WindowsFileHelpers::getModuleFileName (0);

        default:
            jassertfalse; // unknown type?
            return File::nonexistent;
    }

    return WindowsFileHelpers::getSpecialFolderPath (csidlType);
}

//==============================================================================
File File::getCurrentWorkingDirectory()
{
    WCHAR dest [MAX_PATH + 256];
    dest[0] = 0;
    GetCurrentDirectory ((DWORD) numElementsInArray (dest), dest);
    return File (String (dest));
}

bool File::setAsCurrentWorkingDirectory() const
{
    return SetCurrentDirectory (getFullPathName().toWideCharPointer()) != FALSE;
}

//==============================================================================
String File::getVersion() const
{
    String result;

    DWORD handle = 0;
    DWORD bufferSize = GetFileVersionInfoSize (getFullPathName().toWideCharPointer(), &handle);
    HeapBlock<char> buffer;
    buffer.calloc (bufferSize);

    if (GetFileVersionInfo (getFullPathName().toWideCharPointer(), 0, bufferSize, buffer))
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
File File::getLinkedTarget() const
{
    File result (*this);
    String p (getFullPathName());

    if (! exists())
        p += ".lnk";
    else if (getFileExtension() != ".lnk")
        return result;

    ComSmartPtr <IShellLink> shellLink;
    if (SUCCEEDED (shellLink.CoCreateInstance (CLSID_ShellLink)))
    {
        ComSmartPtr <IPersistFile> persistFile;
        if (SUCCEEDED (shellLink.QueryInterface (persistFile)))
        {
            if (SUCCEEDED (persistFile->Load (p.toWideCharPointer(), STGM_READ))
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
class DirectoryIterator::NativeIterator::Pimpl
{
public:
    Pimpl (const File& directory, const String& wildCard)
        : directoryWithWildCard (File::addTrailingSeparator (directory.getFullPathName()) + wildCard),
          handle (INVALID_HANDLE_VALUE)
    {
    }

    ~Pimpl()
    {
        if (handle != INVALID_HANDLE_VALUE)
            FindClose (handle);
    }

    bool next (String& filenameFound,
               bool* const isDir, bool* const isHidden, int64* const fileSize,
               Time* const modTime, Time* const creationTime, bool* const isReadOnly)
    {
        using namespace WindowsFileHelpers;
        WIN32_FIND_DATA findData;

        if (handle == INVALID_HANDLE_VALUE)
        {
            handle = FindFirstFile (directoryWithWildCard.toWideCharPointer(), &findData);

            if (handle == INVALID_HANDLE_VALUE)
                return false;
        }
        else
        {
            if (FindNextFile (handle, &findData) == 0)
                return false;
        }

        filenameFound = findData.cFileName;

        if (isDir != nullptr)         *isDir        = ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
        if (isHidden != nullptr)      *isHidden     = ((findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0);
        if (isReadOnly != nullptr)    *isReadOnly   = ((findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0);
        if (fileSize != nullptr)      *fileSize     = findData.nFileSizeLow + (((int64) findData.nFileSizeHigh) << 32);
        if (modTime != nullptr)       *modTime      = Time (fileTimeToTime (&findData.ftLastWriteTime));
        if (creationTime != nullptr)  *creationTime = Time (fileTimeToTime (&findData.ftCreationTime));

        return true;
    }

private:
    const String directoryWithWildCard;
    HANDLE handle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl);
};

DirectoryIterator::NativeIterator::NativeIterator (const File& directory, const String& wildCard)
    : pimpl (new DirectoryIterator::NativeIterator::Pimpl (directory, wildCard))
{
}

DirectoryIterator::NativeIterator::~NativeIterator()
{
}

bool DirectoryIterator::NativeIterator::next (String& filenameFound,
                                              bool* const isDir, bool* const isHidden, int64* const fileSize,
                                              Time* const modTime, Time* const creationTime, bool* const isReadOnly)
{
    return pimpl->next (filenameFound, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly);
}


//==============================================================================
bool Process::openDocument (const String& fileName, const String& parameters)
{
    HINSTANCE hInstance = 0;

    JUCE_TRY
    {
        hInstance = ShellExecute (0, 0, fileName.toWideCharPointer(), parameters.toWideCharPointer(), 0, SW_SHOWDEFAULT);
    }
    JUCE_CATCH_ALL

    return hInstance > (HINSTANCE) 32;
}

void File::revealToUser() const
{
   #if JUCE_MINGW
    jassertfalse; // not supported in MinGW..
   #else
    #pragma warning (push)
    #pragma warning (disable: 4090) // (alignment warning)
    ITEMIDLIST* const itemIDList = ILCreateFromPath (fullPath.toWideCharPointer());
    #pragma warning (pop)

    if (itemIDList != nullptr)
    {
        SHOpenFolderAndSelectItems (itemIDList, 0, nullptr, 0);
        ILFree (itemIDList);
    }
   #endif
}

//==============================================================================
class NamedPipe::Pimpl
{
public:
    Pimpl (const String& file, const bool isPipe_)
        : pipeH (0),
          cancelEvent (CreateEvent (0, FALSE, FALSE, 0)),
          connected (false),
          isPipe (isPipe_)
    {
        pipeH = isPipe ? CreateNamedPipe (file.toWideCharPointer(),
                                          PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, 0,
                                          PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, 0)
                       : CreateFile (file.toWideCharPointer(),
                                     GENERIC_READ | GENERIC_WRITE, 0, 0,
                                     OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
    }

    ~Pimpl()
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
            OVERLAPPED over = { 0 };
            over.hEvent = CreateEvent (0, TRUE, FALSE, 0);

            if (ConnectNamedPipe (pipeH, &over))
            {
                connected = false;  // yes, you read that right. In overlapped mode it should always return 0.
            }
            else
            {
                const DWORD err = GetLastError();

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

    bool isConnected() const noexcept  { return connected; }

    HANDLE pipeH, cancelEvent;
    bool connected, isPipe;
};

NamedPipe::NamedPipe()
{
}

NamedPipe::~NamedPipe()
{
    close();
}

bool NamedPipe::isOpen() const
{
    return pimpl != nullptr && pimpl->connected;
}

void NamedPipe::cancelPendingReads()
{
    if (pimpl != nullptr)
        SetEvent (pimpl->cancelEvent);
}

void NamedPipe::close()
{
    cancelPendingReads();

    const ScopedLock sl (lock);
    ScopedPointer<Pimpl> deleter (pimpl); // (clears the pimpl member variable before deleting it)
}

bool NamedPipe::openInternal (const String& pipeName, const bool createPipe)
{
    close();

    pimpl = new Pimpl ("\\\\.\\pipe\\" + File::createLegalFileName (pipeName), createPipe);

    if (pimpl->pipeH != INVALID_HANDLE_VALUE)
        return true;

    pimpl = nullptr;
    return false;
}

int NamedPipe::read (void* destBuffer, const int maxBytesToRead, const int timeOutMilliseconds)
{
    const ScopedLock sl (lock);
    int bytesRead = -1;
    bool waitAgain = true;

    while (waitAgain && pimpl != nullptr)
    {
        waitAgain = false;

        if (! pimpl->connect (timeOutMilliseconds))
            break;

        if (maxBytesToRead <= 0)
            return 0;

        OVERLAPPED over = { 0 };
        over.hEvent = CreateEvent (0, TRUE, FALSE, 0);

        unsigned long numRead;

        if (ReadFile (pimpl->pipeH, destBuffer, (DWORD) maxBytesToRead, &numRead, &over))
        {
            bytesRead = (int) numRead;
        }
        else
        {
            const DWORD lastError = GetLastError();

            if (lastError == ERROR_IO_PENDING)
            {
                HANDLE handles[] = { over.hEvent, pimpl->cancelEvent };
                DWORD waitResult = WaitForMultipleObjects (2, handles, FALSE,
                                                           timeOutMilliseconds >= 0 ? timeOutMilliseconds
                                                                                    : INFINITE);
                if (waitResult != WAIT_OBJECT_0)
                {
                    // if the operation timed out, let's cancel it...
                    CancelIo (pimpl->pipeH);
                    WaitForSingleObject (over.hEvent, INFINITE);  // makes sure cancel is complete
                }

                if (GetOverlappedResult (pimpl->pipeH, &over, &numRead, FALSE))
                {
                    bytesRead = (int) numRead;
                }
                else if ((GetLastError() == ERROR_BROKEN_PIPE || GetLastError() == ERROR_PIPE_NOT_CONNECTED) && pimpl->isPipe)
                {
                    pimpl->disconnectPipe();
                    waitAgain = true;
                }
            }
            else if (pimpl->isPipe)
            {
                waitAgain = true;

                if (lastError == ERROR_BROKEN_PIPE || lastError == ERROR_PIPE_NOT_CONNECTED)
                    pimpl->disconnectPipe();
                else
                    Sleep (5);
            }
        }

        CloseHandle (over.hEvent);
    }

    return bytesRead;
}

int NamedPipe::write (const void* sourceBuffer, int numBytesToWrite, int timeOutMilliseconds)
{
    int bytesWritten = -1;

    if (pimpl != nullptr && pimpl->connect (timeOutMilliseconds))
    {
        if (numBytesToWrite <= 0)
            return 0;

        OVERLAPPED over = { 0 };
        over.hEvent = CreateEvent (0, TRUE, FALSE, 0);

        unsigned long numWritten;

        if (WriteFile (pimpl->pipeH, sourceBuffer, (DWORD) numBytesToWrite, &numWritten, &over))
        {
            bytesWritten = (int) numWritten;
        }
        else if (GetLastError() == ERROR_IO_PENDING)
        {
            HANDLE handles[] = { over.hEvent, pimpl->cancelEvent };
            DWORD waitResult;

            waitResult = WaitForMultipleObjects (2, handles, FALSE,
                                                 timeOutMilliseconds >= 0 ? timeOutMilliseconds
                                                                          : INFINITE);

            if (waitResult != WAIT_OBJECT_0)
            {
                CancelIo (pimpl->pipeH);
                WaitForSingleObject (over.hEvent, INFINITE);
            }

            if (GetOverlappedResult (pimpl->pipeH, &over, &numWritten, FALSE))
            {
                bytesWritten = (int) numWritten;
            }
            else if (GetLastError() == ERROR_BROKEN_PIPE && pimpl->isPipe)
            {
                pimpl->disconnectPipe();
            }
        }

        CloseHandle (over.hEvent);
    }

    return bytesWritten;
}
