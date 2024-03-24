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
    Some information about a document.

    Each instance represents some information about the document at the point when the instance
    was created.

    Instance information is not updated automatically. If you think some file information may
    have changed, create a new instance.

    @tags{Core}
*/
class AndroidDocumentInfo
{
public:
    AndroidDocumentInfo() = default;

    /** True if this file really exists. */
    bool exists() const                 { return isJuceFlagSet (flagExists); }

    /** True if this is a directory rather than a file. */
    bool isDirectory() const;

    /** True if this is a file rather than a directory. */
    bool isFile() const                 { return type.isNotEmpty() && ! isDirectory(); }

    /** True if this process has permission to read this file.

        If this returns true, and the AndroidDocument refers to a file rather than a directory,
        then AndroidDocument::createInputStream should work on this document.
    */
    bool canRead() const                { return isJuceFlagSet (flagHasReadPermission) && type.isNotEmpty(); }

    /** True if this is a document that can be written, or a directory that can be modified.

        If this returns true, and the AndroidDocument refers to a file rather than a directory,
        then AndroidDocument::createOutputStream should work on this document.
    */
    bool canWrite() const
    {
        return isJuceFlagSet (flagHasWritePermission)
               && type.isNotEmpty()
               && (isNativeFlagSet (flagSupportsWrite)
                   || isNativeFlagSet (flagSupportsDelete)
                   || isNativeFlagSet (flagDirSupportsCreate));
    }

    /** True if this document can be removed completely from the filesystem. */
    bool canDelete() const              { return isNativeFlagSet (flagSupportsDelete); }

    /** True if this is a directory and adding child documents is supported. */
    bool canCreateChildren() const      { return isNativeFlagSet (flagDirSupportsCreate); }

    /** True if this document can be renamed. */
    bool canRename() const              { return isNativeFlagSet (flagSupportsRename); }

    /** True if this document can be copied. */
    bool canCopy() const                { return isNativeFlagSet (flagSupportsCopy); }

    /** True if this document can be moved. */
    bool canMove() const                { return isNativeFlagSet (flagSupportsMove); }

    /** True if this document isn't a physical file on storage. */
    bool isVirtual() const              { return isNativeFlagSet (flagVirtualDocument); }

    /** The user-facing name.

        This may or may not contain a file extension. For files identified by a URL, the MIME type
        is stored separately.
    */
    String getName() const              { return name; }

    /** The MIME type of this document. */
    String getType() const              { return isDirectory() ? String{} : type; }

    /** Timestamp when a document was last modified, in milliseconds since January 1, 1970 00:00:00.0 UTC.

        Use isLastModifiedValid() to determine whether or not the result of this
        function is valid.
    */
    int64 getLastModified() const       { return isJuceFlagSet (flagValidModified) ? lastModified : 0; }

    /** True if the filesystem provided a modification time. */
    bool isLastModifiedValid() const    { return isJuceFlagSet (flagValidModified); }

    /** The size of the document in bytes, if known.

        Use isSizeInBytesValid() to determine whether or not the result of this
        function is valid.
    */
    int64 getSizeInBytes() const        { return isJuceFlagSet (flagValidSize) ? sizeInBytes : 0; }

    /** True if the filesystem provided a size in bytes. */
    bool isSizeInBytesValid() const     { return isJuceFlagSet (flagValidSize); }

    /** @internal */
    class Args;

private:
    explicit AndroidDocumentInfo (Args);

    bool isNativeFlagSet (int flag) const { return (nativeFlags & flag) != 0; }
    bool isJuceFlagSet   (int flag) const { return (juceFlags   & flag) != 0; }

    /*  Native Android flags that might be set in the COLUMN_FLAGS for a particular document */
    enum
    {
        flagSupportsWrite       = 0x0002,
        flagSupportsDelete      = 0x0004,
        flagDirSupportsCreate   = 0x0008,
        flagSupportsRename      = 0x0040,
        flagSupportsCopy        = 0x0080,
        flagSupportsMove        = 0x0100,
        flagVirtualDocument     = 0x0200,
    };

    /*  Flags for other binary properties that aren't exposed in COLUMN_FLAGS */
    enum
    {
        flagExists              = 1 << 0,
        flagValidModified       = 1 << 1,
        flagValidSize           = 1 << 2,
        flagHasReadPermission   = 1 << 3,
        flagHasWritePermission  = 1 << 4,
    };

    String name;
    String type;
    int64 lastModified = 0;
    int64 sizeInBytes  = 0;
    int nativeFlags = 0, juceFlags = 0;
};

//==============================================================================
/**
    Represents a permission granted to an application to read and/or write to a particular document
    or tree.

    This class also contains static methods to request, revoke, and query the permissions of your
    app. These functions are no-ops on all platforms other than Android.

    @tags{Core}
*/
class AndroidDocumentPermission
{
public:
    /** The url of the document with persisted permissions. */
    URL getUrl() const                          { return url; }

    /** The time when the permissions were persisted, in milliseconds since January 1, 1970 00:00:00.0 UTC. */
    int64 getPersistedTime() const              { return time; }

    /** True if the permission allows read access. */
    bool isReadPermission() const               { return read; }

    /** True if the permission allows write access. */
    bool isWritePermission() const              { return write; }

    /** Gives your app access to a particular document or tree, even after the device is rebooted.

        If you want to persist access to a folder selected through a native file chooser, make sure
        to pass the exact URL returned by the file picker. Do NOT call AndroidDocument::fromTree
        and then pass the result of getUrl to this function, as the resulting URL may differ from
        the result of the file picker.
    */
    static void takePersistentReadWriteAccess (const URL&);

    /** Revokes persistent access to a document or tree. */
    static void releasePersistentReadWriteAccess (const URL&);

    /** Returns all of the permissions that have previously been granted to the app, via
        takePersistentReadWriteAccess();
    */
    static std::vector<AndroidDocumentPermission> getPersistedPermissions();

private:
    URL url;
    int64 time = 0;
    bool read = false, write = false;
};

//==============================================================================
/**
    Provides access to a document on Android devices.

    In this context, a 'document' may be a file or a directory.

    The main purpose of this class is to provide access to files in shared storage on Android.
    On newer Android versions, such files cannot be accessed directly by a file path, and must
    instead be read and modified using a new URI-based DocumentsContract API.

    Example use-cases:

    - After showing the system open dialog to allow the user to open a file, pass the FileChooser's
      URL result to AndroidDocument::fromDocument. Then, you can use getInfo() to retrieve
      information about the file, and createInputStream to read from the file. Other functions allow
      moving, copying, and deleting the file.

    - Similarly to the 'open' use-case, you may use createOutputStream to write to a file, normally
      located using the system save dialog.

    - To allow reading or writing to a tree of files in shared storage, you can show the system
      open dialog in 'selects directories' mode, and pass the resulting URL to
      AndroidDocument::fromTree. Then, you can iterate the files in the directory, query them,
      and create new files. This is a good way to store multiple files that the user can access from
      other apps, and that will be persistent after uninstalling and reinstalling your app.

    Note that you probably do *not* need this class if your app only needs to access files in its
    own internal sandbox. juce::File instances should work as expected in that case.

    AndroidDocument is a bit like the DocumentFile class from the androidx extension library,
    in that it represents a single document, and is implemented using DocumentsContract functions.

    @tags{Core}
*/
class AndroidDocument
{
public:
    /** Create a null document. */
    AndroidDocument();

    /** Create an AndroidDocument representing a file or directory at a particular path.

        This is provided for use on older API versions (lower than 19), or on other platforms, so
        that the same AndroidDocument API can be used regardless of the runtime platform version.

        If the runtime platform version is 19 or higher, and you wish to work with a URI obtained
        from a native file picker, use fromDocument() or fromTree() instead.

        If this function fails, hasValue() will return false on the returned document.
    */
    static AndroidDocument fromFile (const File& filePath);

    /** Create an AndroidDocument representing a single document.

        The argument should be a URL representing a document. Such URLs are returned by the system
        file-picker when it is not in folder-selection mode. If you pass a tree URL, this function
        will fail.

        This function may fail on Android devices with API level 18 or lower, and on non-Android
        platforms. If this function fails, hasValue() will return false on the returned document.
        If calling this function fails, you may want to retry creating an AndroidDocument
        with fromFile(), passing the result of URL::getLocalFile().
    */
    static AndroidDocument fromDocument (const URL& documentUrl);

    /** Create an AndroidDocument representing the root of a tree of files.

        The argument should be a URL representing a tree. Such URLs are returned by the system
        file-picker when it is in folder-selection mode. If you pass a URL referring to a document
        inside a tree, this will return a document referring to the root of the tree. If you pass
        a URL referring to a single file, this will fail.

        When targeting platform version 30 or later, access to the filesystem via file paths is
        heavily restricted, and access to shared storage must use a new URI-based system instead.
        At time of writing, apps uploaded to the Play Store must target API 30 or higher.
        If you want read/write access to a shared folder, you must:

        - Use a native FileChooser in canSelectDirectories mode, to allow the user to select a
          folder that your app can access. Your app will only have access to the contents of this
          directory; it cannot escape to the filesystem root. The system will not allow the user
          to grant access to certain locations, including filesystem roots and the Download folder.
        - Pass the URI that the user selected to fromTree(), and use the resulting AndroidDocument
          to read/write to the file system.

        This function may fail on Android devices with API level 20 or lower, and on non-Android
        platforms. If this function fails, hasValue() will return false on the returned document.
    */
    static AndroidDocument fromTree (const URL& treeUrl);

    AndroidDocument (const AndroidDocument&);
    AndroidDocument (AndroidDocument&&) noexcept;

    AndroidDocument& operator= (const AndroidDocument&);
    AndroidDocument& operator= (AndroidDocument&&) noexcept;

    ~AndroidDocument();

    /** True if the URLs of the two documents match. */
    bool operator== (const AndroidDocument&) const;

    /** False if the URLs of the two documents match. */
    bool operator!= (const AndroidDocument&) const;

    /** Attempts to delete this document, and returns true on success. */
    bool deleteDocument() const;

    /** Renames the document, and returns true on success.

        This may cause the document's URI and metadata to change, so ensure to invalidate any
        cached information about the document (URLs, AndroidDocumentInfo instances) after calling
        this function.
    */
    bool renameTo (const String& newDisplayName);

    /** Attempts to create a new nested document with a particular type and name.

        The type should be a standard MIME type string, e.g. "image/png", "text/plain".

        The file name doesn't need to contain an extension, as this information is passed via the
        type argument. If this document is File-based rather than URL-based, then an appropriate
        file extension will be chosen based on the MIME type.

        On failure, the returned AndroidDocument may be invalid, and will return false from hasValue().
    */
    AndroidDocument createChildDocumentWithTypeAndName (const String& type, const String& name) const;

    /** Attempts to create a new nested directory with a particular name.

        On failure, the returned AndroidDocument may be invalid, and will return false from hasValue().
    */
    AndroidDocument createChildDirectory (const String& name) const;

    /** True if this object actually refers to a document.

        If this function returns false, you *must not* call any function on this instance other
        than the special member functions to copy, move, and/or destruct the instance.
    */
    bool hasValue() const               { return pimpl != nullptr; }

    /** Like hasValue(), but allows declaring AndroidDocument instances directly in 'if' statements. */
    explicit operator bool() const      { return hasValue(); }

    /** Creates a stream for reading from this document. */
    std::unique_ptr<InputStream>  createInputStream()  const;

    /** Creates a stream for writing to this document. */
    std::unique_ptr<OutputStream> createOutputStream() const;

    /** Returns the content URL describing this document. */
    URL getUrl() const;

    /** Fetches information about this document. */
    AndroidDocumentInfo getInfo() const;

    /** Experimental: Attempts to copy this document to a new parent, and returns an AndroidDocument
        representing the copy.

        On failure, the returned AndroidDocument may be invalid, and will return false from hasValue().

        This function may fail if the document doesn't allow copying, and when using URI-based
        documents on devices with API level 23 or lower. On failure, the returned AndroidDocument
        will return false from hasValue(). In testing, copying was not supported on the Android
        emulator for API 24, 30, or 31, so there's a good chance this function won't work on real
        devices.

        @see AndroidDocumentInfo::canCopy
    */
    AndroidDocument copyDocumentToParentDocument (const AndroidDocument& target) const;

    /** Experimental: Attempts to move this document from one parent to another, and returns true on
        success.

        This may cause the document's URI and metadata to change, so ensure to invalidate any
        cached information about the document (URLs, AndroidDocumentInfo instances) after calling
        this function.

        This function may fail if the document doesn't allow moving, and when using URI-based
        documents on devices with API level 23 or lower.
    */
    bool moveDocumentFromParentToParent (const AndroidDocument& currentParent,
                                         const AndroidDocument& newParent);

    /** @internal */
    struct NativeInfo;

    /** @internal */
    NativeInfo getNativeInfo() const;

private:
    struct Utils;
    class Pimpl;

    explicit AndroidDocument (std::unique_ptr<Pimpl>);

    void swap (AndroidDocument& other) noexcept { std::swap (other.pimpl, pimpl); }

    std::unique_ptr<Pimpl> pimpl;
};

//==============================================================================
/**
    An iterator that visits child documents in a directory.

    Instances of this iterator can be created by calling makeRecursive() or
    makeNonRecursive(). The results of these functions can additionally be used
    in standard algorithms, and in range-for loops:

    @code
    AndroidDocument findFileWithName (const AndroidDocument& parent, const String& name)
    {
        for (const auto& child : AndroidDocumentIterator::makeNonRecursive (parent))
            if (child.getInfo().getName() == name)
                  return child;

          return AndroidDocument();
    }

    std::vector<AndroidDocument> findAllChildrenRecursive (const AndroidDocument& parent)
    {
        std::vector<AndroidDocument> children;
        std::copy (AndroidDocumentIterator::makeRecursive (doc),
                   AndroidDocumentIterator(),
                   std::back_inserter (children));
        return children;
    }
    @endcode

    @tags{Core}
*/
class AndroidDocumentIterator final
{
public:
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using iterator_category = std::input_iterator_tag;

    /** Create an iterator that will visit each item in this directory. */
    static AndroidDocumentIterator makeNonRecursive (const AndroidDocument&);

    /** Create an iterator that will visit each item in this directory, and all nested directories. */
    static AndroidDocumentIterator makeRecursive (const AndroidDocument&);

    /** Creates an end/sentinel iterator. */
    AndroidDocumentIterator() = default;

    bool operator== (const AndroidDocumentIterator& other) const noexcept { return pimpl == nullptr && other.pimpl == nullptr; }
    bool operator!= (const AndroidDocumentIterator& other) const noexcept { return ! operator== (other); }

    /** Returns the document to which this iterator points. */
    AndroidDocument operator*() const;

    /** Moves this iterator to the next position. */
    AndroidDocumentIterator& operator++();

    /** Allows this iterator to be used directly in a range-for. */
    AndroidDocumentIterator begin() const { return *this; }

    /** Allows this iterator to be used directly in a range-for. */
    AndroidDocumentIterator end()   const { return AndroidDocumentIterator{}; }

private:
    struct Utils;
    struct Pimpl;

    explicit AndroidDocumentIterator (std::unique_ptr<Pimpl>);

    std::shared_ptr<Pimpl> pimpl;
};

} // namespace juce
