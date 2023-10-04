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

/*  This is mainly used to pass implementation information from AndroidDocument to
    AndroidDocumentIterator. This needs to be defined in a .cpp because it uses the internal
    GlobalRef type.

    To preserve encapsulation, this struct should only contain information that would normally be
    public, were internal types not in use.
*/
struct AndroidDocument::NativeInfo
{
   #if JUCE_ANDROID
    GlobalRef uri;
   #endif
};

//==============================================================================
struct AndroidDocumentDetail
{
    ~AndroidDocumentDetail() = delete; // This struct is a single-file namespace

    struct Opt
    {
        Opt() = default;

        explicit Opt (int64 v) : value (v), valid (true) {}

        int64 value = 0;
        bool valid = false;
    };

    static constexpr auto dirMime = "vnd.android.document/directory";

   #if JUCE_ANDROID
    /*
        A very basic type that acts a bit like an iterator, in that it can be incremented, and read-from.

        Instances of this type can be passed to the constructor of AndroidDirectoryIterator to provide
        stdlib-like iterator facilities.
    */
    template <typename Columns>
    class AndroidIteratorEngine
    {
    public:
        AndroidIteratorEngine (Columns columnsIn, jobject uri)
            : columns (std::move (columnsIn)),
              cursor { LocalRef<jobject> { getEnv()->CallObjectMethod (AndroidContentUriResolver::getContentResolver().get(),
                                                                       ContentResolver.query,
                                                                       uri,
                                                                       columns.getColumnNames().get(),
                                                                       nullptr,
                                                                       nullptr,
                                                                       nullptr) } }
        {
            // Creating the cursor may throw if the document doesn't exist.
            // In that case, cursor will still be null.
            jniCheckHasExceptionOccurredAndClear();
        }

        auto read() const { return columns.readFromCursor (cursor.get()); }

        bool increment()
        {
            if (cursor.get() == nullptr)
                return false;

            return getEnv()->CallBooleanMethod (cursor.get(), AndroidCursor.moveToNext);
        }

    private:
        Columns columns;
        GlobalRef cursor;
    };

    template <typename... Args, size_t... Ix>
    static LocalRef<jobjectArray> makeStringArray (std::index_sequence<Ix...>, Args&&... args)
    {
        auto* env = getEnv();
        LocalRef<jobjectArray> array { env->NewObjectArray (sizeof... (args), JavaString, nullptr) };

        (env->SetObjectArrayElement (array.get(), Ix, args.get()), ...);

        return array;
    }

    template <typename... Args>
    static LocalRef<jobjectArray> makeStringArray (Args&&... args)
    {
        return makeStringArray (std::make_index_sequence<sizeof... (args)>(), std::forward<Args> (args)...);
    }

    static URL uriToUrl (jobject uri)
    {
        return URL (juceString ((jstring) getEnv()->CallObjectMethod (uri, AndroidUri.toString)));
    }

    struct Columns
    {
        GlobalRef treeUri;
        GlobalRefImpl<jstring> idColumn;

        auto getColumnNames() const
        {
            return makeStringArray (idColumn);
        }

        auto readFromCursor (jobject cursor) const
        {
            auto* env = getEnv();
            const auto idColumnIndex = env->CallIntMethod (cursor, AndroidCursor.getColumnIndex, idColumn.get());

            const auto documentUri = [&]
            {
                if (idColumnIndex < 0)
                    return LocalRef<jobject>{};

                LocalRef<jstring> documentId { (jstring) env->CallObjectMethod (cursor, AndroidCursor.getString, idColumnIndex) };
                return LocalRef<jobject> { getEnv()->CallStaticObjectMethod (DocumentsContract21,
                                                                             DocumentsContract21.buildDocumentUriUsingTree,
                                                                             treeUri.get(),
                                                                             documentId.get()) };
            }();

            return AndroidDocument::fromDocument (uriToUrl (documentUri));
        }
    };

    using DocumentsContractIteratorEngine = AndroidIteratorEngine<Columns>;

    static DocumentsContractIteratorEngine makeDocumentsContractIteratorEngine (const GlobalRef& uri)
    {
        const LocalRef <jobject> documentId { getEnv()->CallStaticObjectMethod (DocumentsContract19,
                                                                                DocumentsContract19.getDocumentId,
                                                                                uri.get()) };
        const LocalRef <jobject> childrenUri { getEnv()->CallStaticObjectMethod (DocumentsContract21,
                                                                                 DocumentsContract21.buildChildDocumentsUriUsingTree,
                                                                                 uri.get(),
                                                                                 documentId.get()) };

        return DocumentsContractIteratorEngine { Columns { GlobalRef { uri },
                                                           GlobalRefImpl<jstring> { javaString ("document_id") } },
                                                 childrenUri.get() };
    }

    class RecursiveEngine
    {
    public:
        explicit RecursiveEngine (GlobalRef uri)
            : engine (makeDocumentsContractIteratorEngine (uri)) {}

        AndroidDocument read() const
        {
            return subIterator != nullptr ? subIterator->read() : engine.read();
        }

        bool increment()
        {
            if (directory && subIterator == nullptr)
                subIterator = std::make_unique<RecursiveEngine> (engine.read().getNativeInfo().uri);

            if (subIterator != nullptr)
            {
                if (subIterator->increment())
                    return true;

                subIterator = nullptr;
            }

            if (! engine.increment())
                return false;

            directory = engine.read().getInfo().isDirectory();
            return true;
        }

    private:
        DocumentsContractIteratorEngine engine;
        std::unique_ptr<RecursiveEngine> subIterator;
        bool directory = false;
    };

    enum { FLAG_GRANT_READ_URI_PERMISSION = 1, FLAG_GRANT_WRITE_URI_PERMISSION = 2 };

    static void setPermissions (const URL& url, jmethodID func)
    {
        if (getAndroidSDKVersion() < 19)
            return;

        const auto javaUri = urlToUri (url);

        if (const auto resolver = AndroidContentUriResolver::getContentResolver())
        {
            const jint flags = FLAG_GRANT_READ_URI_PERMISSION | FLAG_GRANT_WRITE_URI_PERMISSION;
            getEnv()->CallVoidMethod (resolver, func, javaUri.get(), flags);
            jniCheckHasExceptionOccurredAndClear();
        }
    }
   #endif

    struct DirectoryIteratorEngine
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
        JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4996)
        DirectoryIteratorEngine (const File& dir, bool recursive)
            : iterator (dir, recursive, "*", File::findFilesAndDirectories) {}
        JUCE_END_IGNORE_WARNINGS_MSVC
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        auto read() const { return AndroidDocument::fromFile (iterator.getFile()); }
        bool increment() { return iterator.next(); }
        DirectoryIterator iterator;
    };

};

//==============================================================================
class AndroidDocumentInfo::Args
{
public:
    using Detail = AndroidDocumentDetail;

    Args withName       (String x)                      const { return with (&Args::name, std::move (x)); }
    Args withType       (String x)                      const { return with (&Args::type, std::move (x)); }
    Args withFlags      (int x)                         const { return with (&Args::flags, x); }
    Args withSize       (Detail::Opt x)                 const { return with (&Args::sizeInBytes, x); }
    Args withModified   (Detail::Opt x)                 const { return with (&Args::lastModified, x); }
    Args withReadPermission  (bool x)                   const { return with (&Args::readPermission,  x); }
    Args withWritePermission (bool x)                   const { return with (&Args::writePermission, x); }

    String name;
    String type;
    Detail::Opt sizeInBytes, lastModified;
    int flags = 0;
    bool readPermission = false, writePermission = false;

    static int getFlagsForFile (const File& file)
    {
        int flags = 0;

        if (file.hasReadAccess())
            flags |= AndroidDocumentInfo::flagSupportsCopy;

        if (file.hasWriteAccess())
            flags |= AndroidDocumentInfo::flagSupportsWrite
                   | AndroidDocumentInfo::flagDirSupportsCreate
                   | AndroidDocumentInfo::flagSupportsMove
                   | AndroidDocumentInfo::flagSupportsRename
                   | AndroidDocumentInfo::flagSupportsDelete;

        return flags;
    }

    AndroidDocumentInfo build() const
    {
        return AndroidDocumentInfo (*this);
    }

private:
    template <typename Value>
    Args with (Value Args::* member, Value v) const
    {
        auto copy = *this;
        copy.*member = std::move (v);
        return copy;
    }
};

AndroidDocumentInfo::AndroidDocumentInfo (Args args)
    : name (args.name),
      type (args.type),
      lastModified (args.lastModified.value),
      sizeInBytes (args.sizeInBytes.value),
      nativeFlags (args.flags),
      juceFlags (flagExists
                 | (args.lastModified.valid ? flagValidModified      : 0)
                 | (args.sizeInBytes.valid  ? flagValidSize          : 0)
                 | (args.readPermission     ? flagHasReadPermission  : 0)
                 | (args.writePermission    ? flagHasWritePermission : 0))
{
}

bool AndroidDocumentInfo::isDirectory() const            { return type == AndroidDocumentDetail::dirMime; }

//==============================================================================
class AndroidDocument::Pimpl
{
public:
    Pimpl() = default;
    Pimpl (const Pimpl&) = default;
    Pimpl (Pimpl&&) noexcept = default;
    Pimpl& operator= (const Pimpl&) = default;
    Pimpl& operator= (Pimpl&&) noexcept = default;

    virtual ~Pimpl() = default;
    virtual std::unique_ptr<Pimpl> clone() const = 0;
    virtual bool deleteDocument() const = 0;
    virtual std::unique_ptr<InputStream>  createInputStream()  const = 0;
    virtual std::unique_ptr<OutputStream> createOutputStream() const = 0;
    virtual AndroidDocumentInfo getInfo() const = 0;
    virtual URL getUrl() const = 0;
    virtual NativeInfo getNativeInfo() const = 0;

    virtual std::unique_ptr<Pimpl> copyDocumentToParentDocument (const Pimpl&) const
    {
        // This function is not supported on the current platform.
        jassertfalse;
        return {};
    }

    virtual std::unique_ptr<Pimpl> moveDocumentFromParentToParent (const Pimpl&, const Pimpl&) const
    {
        // This function is not supported on the current platform.
        jassertfalse;
        return {};
    }

    virtual std::unique_ptr<Pimpl> renameTo (const String&) const
    {
        // This function is not supported on the current platform.
        jassertfalse;
        return {};
    }

    virtual std::unique_ptr<Pimpl> createChildDocumentWithTypeAndName (const String&, const String&) const
    {
        // This function is not supported on the current platform.
        jassertfalse;
        return {};
    }

    File getFile() const { return getUrl().getLocalFile(); }

    static const Pimpl& getPimpl (const AndroidDocument& doc) { return *doc.pimpl; }
};

//==============================================================================
struct AndroidDocument::Utils
{
    using Detail = AndroidDocumentDetail;

    ~Utils() = delete; // This stuct is a single-file namespace

   #if JUCE_ANDROID
    template <typename> struct VersionTag { int version; };

    class MimeConverter
    {
    public:
        String getMimeTypeFromExtension (const String& str) const
        {
            const auto javaStr = javaString (str);
            return juceString ((jstring) getEnv()->CallObjectMethod (map.get(),
                                                                     AndroidMimeTypeMap.getMimeTypeFromExtension,
                                                                     javaStr.get()));
        }

        String getExtensionFromMimeType (const String& str) const
        {
            const auto javaStr = javaString (str);
            return juceString ((jstring) getEnv()->CallObjectMethod (map.get(),
                                                                     AndroidMimeTypeMap.getExtensionFromMimeType,
                                                                     javaStr.get()));
        }

    private:
        GlobalRef map { LocalRef<jobject> { getEnv()->CallStaticObjectMethod (AndroidMimeTypeMap,
                                                                              AndroidMimeTypeMap.getSingleton) } };
    };

    class AndroidDocumentPimplApi19 : public Pimpl
    {
    public:
        AndroidDocumentPimplApi19() = default;

        explicit AndroidDocumentPimplApi19 (const URL& uriIn)
            : AndroidDocumentPimplApi19 (urlToUri (uriIn)) {}

        explicit AndroidDocumentPimplApi19 (const LocalRef<jobject>& uriIn)
            : uri (uriIn) {}

        std::unique_ptr<Pimpl> clone() const override { return std::make_unique<AndroidDocumentPimplApi19> (*this); }

        bool deleteDocument() const override
        {
            if (const auto resolver = AndroidContentUriResolver::getContentResolver())
            {
                return getEnv()->CallStaticBooleanMethod (DocumentsContract19,
                                                          DocumentsContract19.deleteDocument,
                                                          resolver.get(),
                                                          uri.get());
            }

            return false;
        }

        std::unique_ptr<InputStream> createInputStream() const override
        {
            auto result = std::make_unique<AndroidContentUriInputStream> (uri);
            return result->openedSuccessfully() ? std::move (result) : nullptr;
        }

        std::unique_ptr<OutputStream> createOutputStream() const override
        {
            auto stream = AndroidStreamHelpers::createStream (uri, AndroidStreamHelpers::StreamKind::output);

            return stream.get() != nullptr ? std::make_unique<AndroidContentUriOutputStream> (std::move (stream))
                                           : nullptr;
        }

        AndroidDocumentInfo getInfo() const override
        {
            struct Columns
            {
                auto getColumnNames() const
                {
                    return Detail::makeStringArray (flagsColumn, nameColumn, mimeColumn, idColumn, modifiedColumn, sizeColumn);
                }

                auto readFromCursor (jobject cursor) const
                {
                    auto* env = getEnv();

                    const auto flagsColumnIndex = env->CallIntMethod (cursor, AndroidCursor.getColumnIndex, flagsColumn.get());
                    const auto nameColumnIndex  = env->CallIntMethod (cursor, AndroidCursor.getColumnIndex, nameColumn.get());
                    const auto mimeColumnIndex  = env->CallIntMethod (cursor, AndroidCursor.getColumnIndex, mimeColumn.get());
                    const auto idColumnIndex    = env->CallIntMethod (cursor, AndroidCursor.getColumnIndex, idColumn.get());
                    const auto modColumnIndex   = env->CallIntMethod (cursor, AndroidCursor.getColumnIndex, modifiedColumn.get());
                    const auto sizeColumnIndex  = env->CallIntMethod (cursor, AndroidCursor.getColumnIndex, sizeColumn.get());

                    const auto indices = { flagsColumnIndex, nameColumnIndex, mimeColumnIndex, idColumnIndex, modColumnIndex, sizeColumnIndex };

                    if (std::any_of (indices.begin(), indices.end(), [] (auto index) { return index < 0; }))
                        return AndroidDocumentInfo::Args{};

                    const LocalRef<jstring> nameString { (jstring) env->CallObjectMethod (cursor, AndroidCursor.getString, nameColumnIndex) };
                    const LocalRef<jstring> mimeString { (jstring) env->CallObjectMethod (cursor, AndroidCursor.getString, mimeColumnIndex) };

                    const auto readOpt = [&] (int column) -> Detail::Opt
                    {
                        const auto missing = env->CallBooleanMethod (cursor, AndroidCursor.isNull, column);

                        if (missing)
                            return {};

                        return Detail::Opt { env->CallLongMethod (cursor, AndroidCursor.getLong, column) };
                    };

                    return AndroidDocumentInfo::Args{}.withName (juceString (nameString.get()))
                                                      .withType (juceString (mimeString.get()))
                                                      .withFlags (env->CallIntMethod (cursor,  AndroidCursor.getInt,  flagsColumnIndex))
                                                      .withModified (readOpt (modColumnIndex))
                                                      .withSize     (readOpt (sizeColumnIndex));
                }

                GlobalRefImpl<jstring> flagsColumn    { javaString ("flags") };
                GlobalRefImpl<jstring> nameColumn     { javaString ("_display_name") };
                GlobalRefImpl<jstring> mimeColumn     { javaString ("mime_type") };
                GlobalRefImpl<jstring> idColumn       { javaString ("document_id") };
                GlobalRefImpl<jstring> modifiedColumn { javaString ("last_modified") };
                GlobalRefImpl<jstring> sizeColumn     { javaString ("_size") };
            };

            Detail::AndroidIteratorEngine<Columns> iterator { Columns{}, uri };

            if (! iterator.increment())
                return AndroidDocumentInfo{};

            auto* env = getEnv();
            auto ctx = getAppContext();

            const auto hasPermission = [&] (auto permission)
            {
                return env->CallIntMethod (ctx, AndroidContext.checkCallingOrSelfUriPermission, uri.get(), permission) == 0;
            };

            return iterator.read()
                           .withReadPermission  (hasPermission (Detail::FLAG_GRANT_READ_URI_PERMISSION))
                           .withWritePermission (hasPermission (Detail::FLAG_GRANT_WRITE_URI_PERMISSION))
                           .build();
        }

        URL getUrl() const override
        {
            return Detail::uriToUrl (uri);
        }

        NativeInfo getNativeInfo() const override { return { uri }; }

    private:
        GlobalRef uri;
    };

    //==============================================================================
    class AndroidDocumentPimplApi21 : public AndroidDocumentPimplApi19
    {
    public:
        using AndroidDocumentPimplApi19::AndroidDocumentPimplApi19;

        std::unique_ptr<Pimpl> clone() const override { return std::make_unique<AndroidDocumentPimplApi21> (*this); }

        std::unique_ptr<Pimpl> createChildDocumentWithTypeAndName (const String& type, const String& name) const override
        {
            return Utils::createPimplForSdk (LocalRef<jobject> { getEnv()->CallStaticObjectMethod (DocumentsContract21,
                                                                                                   DocumentsContract21.createDocument,
                                                                                                   AndroidContentUriResolver::getContentResolver().get(),
                                                                                                   getNativeInfo().uri.get(),
                                                                                                   javaString (type).get(),
                                                                                                   javaString (name).get()) });
        }

        std::unique_ptr<Pimpl> renameTo (const String& name) const override
        {
            if (const auto resolver = AndroidContentUriResolver::getContentResolver())
            {
                return Utils::createPimplForSdk (LocalRef<jobject> { getEnv()->CallStaticObjectMethod (DocumentsContract21,
                                                                                                       DocumentsContract21.renameDocument,
                                                                                                       resolver.get(),
                                                                                                       getNativeInfo().uri.get(),
                                                                                                       javaString (name).get()) });
            }

            return nullptr;
        }
    };

    //==============================================================================
    class AndroidDocumentPimplApi24 final : public AndroidDocumentPimplApi21
    {
    public:
        using AndroidDocumentPimplApi21::AndroidDocumentPimplApi21;

        std::unique_ptr<Pimpl> clone() const override { return std::make_unique<AndroidDocumentPimplApi24> (*this); }

        std::unique_ptr<Pimpl> copyDocumentToParentDocument (const Pimpl& target) const override
        {
            if (target.getNativeInfo().uri == nullptr)
            {
                // Cannot copy to a non-URI-based AndroidDocument
                return {};
            }

            return Utils::createPimplForSdk (LocalRef<jobject> { getEnv()->CallStaticObjectMethod (DocumentsContract24,
                                                                                                   DocumentsContract24.copyDocument,
                                                                                                   AndroidContentUriResolver::getContentResolver().get(),
                                                                                                   getNativeInfo().uri.get(),
                                                                                                   target.getNativeInfo().uri.get()) });
        }

        std::unique_ptr<Pimpl> moveDocumentFromParentToParent (const Pimpl& currentParent, const Pimpl& newParent) const override
        {
            if (currentParent.getNativeInfo().uri == nullptr || newParent.getNativeInfo().uri == nullptr)
            {
                // Cannot move document between non-URI-based AndroidDocuments
                return {};
            }

            return Utils::createPimplForSdk (LocalRef<jobject> { getEnv()->CallStaticObjectMethod (DocumentsContract24,
                                                                                                   DocumentsContract24.moveDocument,
                                                                                                   AndroidContentUriResolver::getContentResolver().get(),
                                                                                                   getNativeInfo().uri.get(),
                                                                                                   currentParent.getNativeInfo().uri.get(),
                                                                                                   newParent.getNativeInfo().uri.get()) });
        }
    };

    static std::unique_ptr<Pimpl> createPimplForSdk (const LocalRef<jobject>& uri)
    {
        if (jniCheckHasExceptionOccurredAndClear())
            return nullptr;

        return createPimplForSdkImpl (uri,
                                      VersionTag<AndroidDocumentPimplApi24> { 24 },
                                      VersionTag<AndroidDocumentPimplApi21> { 21 },
                                      VersionTag<AndroidDocumentPimplApi19> { 19 });
    }

    static std::unique_ptr<Pimpl> createPimplForSdkImpl (const LocalRef<jobject>&)
    {
        // Failed to find a suitable implementation for this platform
        jassertfalse;
        return nullptr;
    }

    template <typename T, typename... Ts>
    static std::unique_ptr<Pimpl> createPimplForSdkImpl (const LocalRef<jobject>& uri,
                                                         VersionTag<T> head,
                                                         VersionTag<Ts>... tail)
    {
        if (head.version <= getAndroidSDKVersion())
            return std::make_unique<T> (uri);

        return createPimplForSdkImpl (uri, tail...);
    }

   #else
    class MimeConverter
    {
    public:
        static String getMimeTypeFromExtension (const String& str)
        {
            return MimeTypeTable::getMimeTypesForFileExtension (str)[0];
        }

        static String getExtensionFromMimeType (const String& str)
        {
            return MimeTypeTable::getFileExtensionsForMimeType (str)[0];
        }
    };
   #endif

    //==============================================================================
    class AndroidDocumentPimplFile final : public Pimpl
    {
    public:
        explicit AndroidDocumentPimplFile (const File& f)
            : file (f)
        {
        }

        std::unique_ptr<Pimpl> clone() const override { return std::make_unique<AndroidDocumentPimplFile> (*this); }

        bool deleteDocument() const override
        {
            return file.deleteRecursively (false);
        }

        std::unique_ptr<Pimpl> renameTo (const String& name) const override
        {
            const auto target = file.getSiblingFile (name);

            return file.moveFileTo (target) ? std::make_unique<AndroidDocumentPimplFile> (target)
                                            : nullptr;
        }

        std::unique_ptr<InputStream>  createInputStream()  const override { return file.createInputStream(); }

        std::unique_ptr<OutputStream> createOutputStream() const override
        {
            auto result = file.createOutputStream();
            result->setPosition (0);
            result->truncate();
            return result;
        }

        std::unique_ptr<Pimpl> copyDocumentToParentDocument (const Pimpl& target) const override
        {
            const auto parent = target.getFile();

            if (parent == File())
                return nullptr;

            const auto actual = parent.getChildFile (file.getFileName());

            if (actual.exists())
                return nullptr;

            const auto success = file.isDirectory() ? file.copyDirectoryTo (actual)
                                                    : file.copyFileTo (actual);

            return success ? std::make_unique<AndroidDocumentPimplFile> (actual)
                           : nullptr;
        }

        std::unique_ptr<Pimpl> createChildDocumentWithTypeAndName (const String& type,
                                                                   const String& name) const override
        {
            const auto extension = mimeConverter.getExtensionFromMimeType (type);
            const auto target = file.getChildFile (extension.isNotEmpty() ? name + "." + extension : name);

            if (! target.exists() && (type == Detail::dirMime ? target.createDirectory() : target.create()))
                return std::make_unique<AndroidDocumentPimplFile> (target);

            return nullptr;
        }

        std::unique_ptr<Pimpl> moveDocumentFromParentToParent (const Pimpl& currentParentPimpl,
                                                               const Pimpl& newParentPimpl) const override
        {
            const auto currentParent = currentParentPimpl.getFile();
            const auto newParent = newParentPimpl.getFile();

            if (! file.isAChildOf (currentParent) || newParent == File())
                return nullptr;

            const auto target = newParent.getChildFile (file.getFileName());

            if (target.exists() || ! file.moveFileTo (target))
                return nullptr;

            return std::make_unique<AndroidDocumentPimplFile> (target);
        }

        AndroidDocumentInfo getInfo() const override
        {
            if (! file.exists())
                return AndroidDocumentInfo{};

            const auto size = file.getSize();
            const auto extension = file.getFileExtension().removeCharacters (".").toLowerCase();
            const auto type = file.isDirectory() ? Detail::dirMime
                                                 : mimeConverter.getMimeTypeFromExtension (extension);

            return AndroidDocumentInfo::Args{}.withName (file.getFileName())
                                              .withType (type.isNotEmpty() ? type : "application/octet-stream")
                                              .withFlags (AndroidDocumentInfo::Args::getFlagsForFile (file))
                                              .withModified (Detail::Opt { file.getLastModificationTime().toMilliseconds() })
                                              .withSize (size != 0 ? Detail::Opt { size } : Detail::Opt{})
                                              .withReadPermission (file.hasReadAccess())
                                              .withWritePermission (file.hasWriteAccess())
                                              .build();
        }

        URL getUrl() const override { return URL (file); }

        NativeInfo getNativeInfo() const override { return {}; }

    private:
        File file;
        MimeConverter mimeConverter;
    };
};

//==============================================================================
void AndroidDocumentPermission::takePersistentReadWriteAccess ([[maybe_unused]] const URL& url)
{
   #if JUCE_ANDROID
    AndroidDocumentDetail::setPermissions (url, ContentResolver19.takePersistableUriPermission);
   #endif
}

void AndroidDocumentPermission::releasePersistentReadWriteAccess ([[maybe_unused]] const URL& url)
{
   #if JUCE_ANDROID
    AndroidDocumentDetail::setPermissions (url, ContentResolver19.releasePersistableUriPermission);
   #endif
}

std::vector<AndroidDocumentPermission> AndroidDocumentPermission::getPersistedPermissions()
{
   #if ! JUCE_ANDROID
    return {};
   #else
    if (getAndroidSDKVersion() < 19)
        return {};

    auto* env = getEnv();
    const LocalRef<jobject> permissions { env->CallObjectMethod (AndroidContentUriResolver::getContentResolver().get(),
                                                                 ContentResolver19.getPersistedUriPermissions) };

    if (permissions == nullptr)
        return {};

    std::vector<AndroidDocumentPermission> result;
    const auto size = env->CallIntMethod (permissions, JavaList.size);

    for (auto i = (decltype (size)) 0; i < size; ++i)
    {
        const LocalRef<jobject> uriPermission { env->CallObjectMethod (permissions, JavaList.get, i) };

        AndroidDocumentPermission permission;
        permission.time  = env->CallLongMethod    (uriPermission, AndroidUriPermission.getPersistedTime);
        permission.read  = env->CallBooleanMethod (uriPermission, AndroidUriPermission.isReadPermission);
        permission.write = env->CallBooleanMethod (uriPermission, AndroidUriPermission.isWritePermission);
        permission.url = AndroidDocumentDetail::uriToUrl (env->CallObjectMethod  (uriPermission, AndroidUriPermission.getUri));

        result.push_back (std::move (permission));
    }

    return result;
   #endif
}

//==============================================================================
AndroidDocument::AndroidDocument() = default;

AndroidDocument AndroidDocument::fromFile (const File& filePath)
{
   #if JUCE_ANDROID
    const LocalRef<jobject> info { getEnv()->CallObjectMethod (getAppContext(), AndroidContext.getApplicationInfo) };
    const auto targetSdkVersion = getEnv()->GetIntField (info.get(), AndroidApplicationInfo.targetSdkVersion);

    // At the current API level, plain file paths may not work for accessing files in shared
    // locations. It's recommended to use fromDocument() or fromTree() instead when targeting this
    // API level.
    jassert (__ANDROID_API_Q__ <= targetSdkVersion);
   #endif

    return AndroidDocument { filePath != File() ? std::make_unique<Utils::AndroidDocumentPimplFile> (filePath)
                                                : nullptr };
}

AndroidDocument AndroidDocument::fromDocument ([[maybe_unused]] const URL& documentUrl)
{
   #if JUCE_ANDROID
    if (getAndroidSDKVersion() < 19)
    {
        // This function is unsupported on this platform.
        jassertfalse;
        return AndroidDocument{};
    }

    const auto javaUri = urlToUri (documentUrl);

    if (! getEnv()->CallStaticBooleanMethod (DocumentsContract19,
                                             DocumentsContract19.isDocumentUri,
                                             getAppContext().get(),
                                             javaUri.get()))
    {
        return AndroidDocument{};
    }

    return AndroidDocument { Utils::createPimplForSdk (javaUri) };
   #else
    return AndroidDocument{};
   #endif
}

AndroidDocument AndroidDocument::fromTree ([[maybe_unused]] const URL& treeUrl)
{
   #if JUCE_ANDROID
    if (getAndroidSDKVersion() < 21)
    {
        // This function is unsupported on this platform.
        jassertfalse;
        return AndroidDocument{};
    }

    const auto javaUri = urlToUri (treeUrl);
    LocalRef<jobject> treeDocumentId { getEnv()->CallStaticObjectMethod (DocumentsContract21,
                                                                         DocumentsContract21.getTreeDocumentId,
                                                                         javaUri.get()) };

    jniCheckHasExceptionOccurredAndClear();

    if (treeDocumentId == nullptr)
    {
        jassertfalse;
        return AndroidDocument{};
    }

    LocalRef<jobject> documentUri { getEnv()->CallStaticObjectMethod (DocumentsContract21,
                                                                      DocumentsContract21.buildDocumentUriUsingTree,
                                                                      javaUri.get(),
                                                                      treeDocumentId.get()) };

    return AndroidDocument { Utils::createPimplForSdk (documentUri) };
   #else
    return AndroidDocument{};
   #endif
}

AndroidDocument::AndroidDocument (const AndroidDocument& other)
    : AndroidDocument (other.pimpl != nullptr ? other.pimpl->clone() : nullptr) {}

AndroidDocument::AndroidDocument (std::unique_ptr<Pimpl> pimplIn)
    : pimpl (std::move (pimplIn)) {}

AndroidDocument::AndroidDocument (AndroidDocument&&) noexcept = default;

AndroidDocument& AndroidDocument::operator= (const AndroidDocument& other)
{
    AndroidDocument { other }.swap (*this);
    return *this;
}

AndroidDocument& AndroidDocument::operator= (AndroidDocument&&) noexcept = default;

AndroidDocument::~AndroidDocument() = default;

bool AndroidDocument::deleteDocument() const { return pimpl->deleteDocument(); }

bool AndroidDocument::renameTo (const String& newDisplayName)
{
    jassert (hasValue());

    auto renamed = pimpl->renameTo (newDisplayName);

    if (renamed == nullptr)
        return false;

    pimpl = std::move (renamed);
    return true;
}

AndroidDocument AndroidDocument::copyDocumentToParentDocument (const AndroidDocument& target) const
{
    jassert (hasValue() && target.hasValue());
    return AndroidDocument { pimpl->copyDocumentToParentDocument (*target.pimpl) };
}

AndroidDocument AndroidDocument::createChildDocumentWithTypeAndName (const String& type,
                                                                     const String& name) const
{
    jassert (hasValue());
    return AndroidDocument { pimpl->createChildDocumentWithTypeAndName (type, name) };
}

AndroidDocument AndroidDocument::createChildDirectory (const String& name) const
{
    return createChildDocumentWithTypeAndName (AndroidDocumentDetail::dirMime, name);
}

bool AndroidDocument::moveDocumentFromParentToParent (const AndroidDocument& currentParent,
                                                      const AndroidDocument& newParent)
{
    jassert (hasValue() && currentParent.hasValue() && newParent.hasValue());
    auto moved = pimpl->moveDocumentFromParentToParent (*currentParent.pimpl, *newParent.pimpl);

    if (moved == nullptr)
        return false;

    pimpl = std::move (moved);
    return true;
}

std::unique_ptr<InputStream>  AndroidDocument::createInputStream()  const
{
    jassert (hasValue());
    return pimpl->createInputStream();
}

std::unique_ptr<OutputStream> AndroidDocument::createOutputStream() const
{
    jassert (hasValue());
    return pimpl->createOutputStream();
}

URL AndroidDocument::getUrl() const
{
    jassert (hasValue());
    return pimpl->getUrl();
}

AndroidDocumentInfo AndroidDocument::getInfo() const
{
    jassert (hasValue());
    return pimpl->getInfo();
}

bool AndroidDocument::operator== (const AndroidDocument& other) const
{
    return getUrl() == other.getUrl();
}

bool AndroidDocument::operator!= (const AndroidDocument& other) const
{
    return ! operator== (other);
}

AndroidDocument::NativeInfo AndroidDocument::getNativeInfo() const
{
    jassert (hasValue());
    return pimpl->getNativeInfo();
}

//==============================================================================
struct AndroidDocumentIterator::Pimpl
{
    virtual ~Pimpl() = default;
    virtual AndroidDocument read() const = 0;
    virtual bool increment() = 0;
};

struct AndroidDocumentIterator::Utils
{
    using Detail = AndroidDocumentDetail;

    ~Utils() = delete; // This struct is a single-file namespace

    template <typename Engine>
    struct TemplatePimpl final : public Pimpl, public Engine
    {
        template <typename... Args>
        TemplatePimpl (Args&&... args) : Engine (std::forward<Args> (args)...) {}

        AndroidDocument read() const override { return Engine::read(); }
        bool increment() override { return Engine::increment(); }
    };

    template <typename Engine, typename... Args>
    static AndroidDocumentIterator makeWithEngineInplace (Args&&... args)
    {
        return AndroidDocumentIterator { std::make_unique<TemplatePimpl<Engine>> (std::forward<Args> (args)...) };
    }

    template <typename Engine>
    static AndroidDocumentIterator makeWithEngine (Engine engine)
    {
        return AndroidDocumentIterator { std::make_unique<TemplatePimpl<Engine>> (std::move (engine)) };
    }

    static void increment (AndroidDocumentIterator& it)
    {
        if (it.pimpl == nullptr || ! it.pimpl->increment())
            it.pimpl = nullptr;
    }
};

//==============================================================================
AndroidDocumentIterator AndroidDocumentIterator::makeNonRecursive (const AndroidDocument& dir)
{
    if (! dir.hasValue())
        return {};

    using Detail = AndroidDocumentDetail;

   #if JUCE_ANDROID
    if (21 <= getAndroidSDKVersion())
    {
        if (auto uri = dir.getNativeInfo().uri)
            return Utils::makeWithEngine (Detail::makeDocumentsContractIteratorEngine (uri));
    }
   #endif

    return Utils::makeWithEngineInplace<Detail::DirectoryIteratorEngine> (dir.getUrl().getLocalFile(), false);
}

AndroidDocumentIterator AndroidDocumentIterator::makeRecursive (const AndroidDocument& dir)
{
    if (! dir.hasValue())
        return {};

    using Detail = AndroidDocumentDetail;

   #if JUCE_ANDROID
    if (21 <= getAndroidSDKVersion())
    {
        if (auto uri = dir.getNativeInfo().uri)
            return Utils::makeWithEngine (Detail::RecursiveEngine { uri });
    }
   #endif

    return Utils::makeWithEngineInplace<Detail::DirectoryIteratorEngine> (dir.getUrl().getLocalFile(), true);
}

AndroidDocumentIterator::AndroidDocumentIterator (std::unique_ptr<Pimpl> engine)
    : pimpl (std::move (engine))
{
    Utils::increment (*this);
}

AndroidDocument AndroidDocumentIterator::operator*() const { return pimpl->read(); }

AndroidDocumentIterator& AndroidDocumentIterator::operator++()
{
    Utils::increment (*this);
    return *this;
}

} // namespace juce
