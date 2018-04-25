/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 FIELD (providers, "providers", "[Landroid/content/pm/ProviderInfo;")

DECLARE_JNI_CLASS (AndroidPackageInfo, "android/content/pm/PackageInfo");
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 FIELD (authority, "authority", "Ljava/lang/String;")

DECLARE_JNI_CLASS (AndroidProviderInfo, "android/content/pm/ProviderInfo");
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor,       "<init>",            "(Landroid/os/ParcelFileDescriptor;JJ)V") \
 METHOD (createInputStream, "createInputStream", "()Ljava/io/FileInputStream;") \
 METHOD (getLength,         "getLength",         "()J")

DECLARE_JNI_CLASS (AssetFileDescriptor, "android/content/res/AssetFileDescriptor");
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (close, "close", "()V")

DECLARE_JNI_CLASS (JavaCloseable, "java/io/Closeable");
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor,   "<init>",        "(L" JUCE_ANDROID_SHARING_CONTENT_PROVIDER_CLASSPATH ";JLjava/lang/String;I)V") \
 METHOD (startWatching, "startWatching", "()V") \
 METHOD (stopWatching,  "stopWatching",  "()V")

DECLARE_JNI_CLASS (JuceContentProviderFileObserver, JUCE_ANDROID_SHARING_CONTENT_PROVIDER_CLASSPATH "$ProviderFileObserver");
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (addRow,      "addRow", "([Ljava/lang/Object;)V") \
 METHOD (constructor, "<init>", "(L" JUCE_ANDROID_SHARING_CONTENT_PROVIDER_CLASSPATH ";J[Ljava/lang/String;)V")

DECLARE_JNI_CLASS (JuceContentProviderFileObserverCursor, JUCE_ANDROID_SHARING_CONTENT_PROVIDER_CLASSPATH "$ProviderCursor");
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 STATICMETHOD (open, "open", "(Ljava/io/File;I)Landroid/os/ParcelFileDescriptor;")

DECLARE_JNI_CLASS (ParcelFileDescriptor, "android/os/ParcelFileDescriptor");
#undef JNI_CLASS_MEMBERS

//==============================================================================
class AndroidContentSharerCursor
{
public:
    class Owner
    {
    public:
        virtual ~Owner() {}

        virtual void cursorClosed (const AndroidContentSharerCursor&) = 0;
    };

    AndroidContentSharerCursor (Owner& ownerToUse, JNIEnv* env,
                                const LocalRef<jobject>& contentProvider,
                                const LocalRef<jobjectArray>& resultColumns)
        : owner (ownerToUse),
          cursor (GlobalRef (LocalRef<jobject> (env->NewObject (JuceContentProviderFileObserverCursor,
                                                                JuceContentProviderFileObserverCursor.constructor,
                                                                contentProvider.get(),
                                                                reinterpret_cast<jlong> (this),
                                                                resultColumns.get()))))
    {
        // the content provider must be created first
        jassert (contentProvider.get() != 0);
    }

    jobject getNativeCursor() { return cursor.get(); }

    void cursorClosed()
    {
        MessageManager::callAsync ([this] { owner.cursorClosed (*this); });
    }

private:
    Owner& owner;
    GlobalRef cursor;
};

//==============================================================================
class AndroidContentSharerFileObserver
{
public:
    class Owner
    {
    public:
        virtual ~Owner() {}

        virtual void fileHandleClosed (const AndroidContentSharerFileObserver&) = 0;
    };

    AndroidContentSharerFileObserver (Owner& ownerToUse, JNIEnv* env,
                                      const LocalRef<jobject>& contentProvider,
                                      const String& filepathToUse)
        : owner (ownerToUse),
          filepath (filepathToUse),
          fileObserver (GlobalRef (LocalRef<jobject> (env->NewObject (JuceContentProviderFileObserver,
                                                                      JuceContentProviderFileObserver.constructor,
                                                                      contentProvider.get(),
                                                                      reinterpret_cast<jlong> (this),
                                                                      javaString (filepath).get(),
                                                                      open | access | closeWrite | closeNoWrite))))
    {
        // the content provider must be created first
        jassert (contentProvider.get() != 0);

        env->CallVoidMethod (fileObserver, JuceContentProviderFileObserver.startWatching);
    }

    void onFileEvent (int event, const LocalRef<jstring>& path)
    {
        ignoreUnused (path);

        if (event == open)
        {
            ++numOpenedHandles;
        }
        else if (event == access)
        {
            fileWasRead = true;
        }
        else if (event == closeNoWrite || event == closeWrite)
        {
            --numOpenedHandles;

            // numOpenedHandles may get negative if we don't receive open handle event.
            if (fileWasRead && numOpenedHandles <= 0)
            {
                MessageManager::callAsync ([this]
                {
                    getEnv()->CallVoidMethod (fileObserver, JuceContentProviderFileObserver.stopWatching);
                    owner.fileHandleClosed (*this);
                });
            }
        }
    }

private:
    static constexpr int open = 32;
    static constexpr int access = 1;
    static constexpr int closeWrite = 8;
    static constexpr int closeNoWrite = 16;

    bool fileWasRead = false;
    int numOpenedHandles = 0;

    Owner& owner;
    String filepath;
    GlobalRef fileObserver;
};

//==============================================================================
class AndroidContentSharerPrepareFilesThread    : private Thread
{
public:
    AndroidContentSharerPrepareFilesThread (AsyncUpdater& ownerToUse,
                                            const Array<URL>& fileUrlsToUse,
                                            const String& packageNameToUse,
                                            const String& uriBaseToUse)
        : Thread ("AndroidContentSharerPrepareFilesThread"),
          owner (ownerToUse),
          fileUrls (fileUrlsToUse),
          resultFileUris (GlobalRef (LocalRef<jobject> (getEnv()->NewObject (JavaArrayList,
                                                                             JavaArrayList.constructor,
                                                                             fileUrls.size())))),
          packageName (packageNameToUse),
          uriBase (uriBaseToUse)
    {
        startThread();
    }

    ~AndroidContentSharerPrepareFilesThread()
    {
        signalThreadShouldExit();
        waitForThreadToExit (10000);

        for (auto& f : temporaryFilesFromAssetFiles)
            f.deleteFile();
    }

    jobject getResultFileUris()  { return resultFileUris.get(); }
    const StringArray& getMimeTypes() const { return mimeTypes; }
    const StringArray& getFilePaths() const { return filePaths; }

private:
    struct StreamCloser
    {
        StreamCloser (jobject streamToUse)
            : stream (GlobalRef (streamToUse))
        {
        }

        ~StreamCloser()
        {
            if (stream.get() != 0)
                getEnv()->CallVoidMethod (stream, JavaCloseable.close);
        }

        GlobalRef stream;
    };

    void run() override
    {
        auto* env = getEnv();

        bool canSpecifyMimeTypes = true;

        for (auto f : fileUrls)
        {
            auto scheme = f.getScheme();

            // Only "file://" scheme or no scheme (for files in app bundle) are allowed!
            jassert (scheme.isEmpty() || scheme == "file");

            if (scheme.isEmpty())
            {
                // Raw resource names need to be all lower case
                jassert (f.toString (true).toLowerCase() == f.toString (true));

                // This will get us a file with file:// URI
                f = copyAssetFileToTemporaryFile (env, f.toString (true));

                if (f.isEmpty())
                    continue;
            }

            if (threadShouldExit())
                return;

            auto filepath = URL::removeEscapeChars (f.toString (true).fromFirstOccurrenceOf ("file://", false, false));

            filePaths.add (filepath);

            auto filename = filepath.fromLastOccurrenceOf ("/", false, true);
            auto fileExtension = filename.fromLastOccurrenceOf (".", false, true);
            auto contentString = uriBase + String (filePaths.size() - 1) + "/" + filename;

            auto uri = LocalRef<jobject> (env->CallStaticObjectMethod (AndroidUri, AndroidUri.parse,
                                                                       javaString (contentString).get()));

            if (canSpecifyMimeTypes)
                canSpecifyMimeTypes = fileExtension.isNotEmpty();

            if (canSpecifyMimeTypes)
                mimeTypes.addArray (getMimeTypesForFileExtension (fileExtension));
            else
                mimeTypes.clear();

            env->CallBooleanMethod (resultFileUris, JavaArrayList.add, uri.get());
        }

        owner.triggerAsyncUpdate();
    }

    URL copyAssetFileToTemporaryFile (JNIEnv* env, const String& filename)
    {
        auto resources = LocalRef<jobject> (env->CallObjectMethod (android.activity, JuceAppActivity.getResources));
        int fileId = env->CallIntMethod (resources, AndroidResources.getIdentifier, javaString (filename).get(),
                                         javaString ("raw").get(), javaString (packageName).get());

        // Raw resource not found. Please make sure that you include your file as a raw resource
        // and that you specify just the file name, without an extention.
        jassert (fileId != 0);

        if (fileId == 0)
            return {};

        auto assetFd = LocalRef<jobject> (env->CallObjectMethod (resources,
                                                                 AndroidResources.openRawResourceFd,
                                                                 fileId));

        auto inputStream = StreamCloser (LocalRef<jobject> (env->CallObjectMethod (assetFd,
                                                                                   AssetFileDescriptor.createInputStream)));

        auto exception = LocalRef<jobject> (env->ExceptionOccurred());

        if (exception != 0)
        {
            // Failed to open file stream for resource
            jassertfalse;

            env->ExceptionClear();
            return {};
        }

        auto tempFile = File::createTempFile ({});
        tempFile.createDirectory();
        tempFile = tempFile.getChildFile (filename);

        auto outputStream = StreamCloser (LocalRef<jobject> (env->NewObject (JavaFileOutputStream,
                                                                             JavaFileOutputStream.constructor,
                                                                             javaString (tempFile.getFullPathName()).get())));

        exception = LocalRef<jobject> (env->ExceptionOccurred());

        if (exception != 0)
        {
            // Failed to open file stream for temporary file
            jassertfalse;

            env->ExceptionClear();
            return {};
        }

        auto buffer = LocalRef<jbyteArray> (env->NewByteArray (1024));
        int bytesRead = 0;

        while (true)
        {
            if (threadShouldExit())
                return {};

            bytesRead = env->CallIntMethod (inputStream.stream, JavaFileInputStream.read, buffer.get());

            exception = LocalRef<jobject> (env->ExceptionOccurred());

            if (exception != 0)
            {
                // Failed to read from resource file.
                jassertfalse;

                env->ExceptionClear();
                return {};
            }

            if (bytesRead < 0)
                break;

            env->CallVoidMethod (outputStream.stream, JavaFileOutputStream.write, buffer.get(), 0, bytesRead);

            if (exception != 0)
            {
                // Failed to write to temporary file.
                jassertfalse;

                env->ExceptionClear();
                return {};
            }
        }

        temporaryFilesFromAssetFiles.add (tempFile);

        return URL (tempFile);
    }

    AsyncUpdater& owner;
    Array<URL> fileUrls;

    GlobalRef resultFileUris;
    String packageName;
    String uriBase;

    StringArray filePaths;
    Array<File> temporaryFilesFromAssetFiles;
    StringArray mimeTypes;
};

//==============================================================================
class ContentSharer::ContentSharerNativeImpl  : public ContentSharer::Pimpl,
                                                public AndroidContentSharerFileObserver::Owner,
                                                public AndroidContentSharerCursor::Owner,
                                                public AsyncUpdater,
                                                private Timer
{
public:
    ContentSharerNativeImpl (ContentSharer& cs)
        : owner (cs),
          packageName (juceString (LocalRef<jstring> ((jstring) getEnv()->CallObjectMethod (android.activity,
                                                                                            JuceAppActivity.getPackageName)))),
          uriBase ("content://" + packageName + ".sharingcontentprovider/")
    {
    }

    ~ContentSharerNativeImpl()
    {
        masterReference.clear();
    }

    void shareFiles (const Array<URL>& files) override
    {
        if (! isContentSharingEnabled())
        {
            // You need to enable "Content Sharing" in Projucer's Android exporter.
            jassertfalse;
            owner.sharingFinished (false, {});
        }

        prepareFilesThread.reset (new AndroidContentSharerPrepareFilesThread (*this, files, packageName, uriBase));
    }

    void shareText (const String& text) override
    {
        if (! isContentSharingEnabled())
        {
            // You need to enable "Content Sharing" in Projucer's Android exporter.
            jassertfalse;
            owner.sharingFinished (false, {});
        }

        auto* env = getEnv();

        auto intent = LocalRef<jobject> (env->NewObject (AndroidIntent, AndroidIntent.constructor));
        env->CallObjectMethod (intent, AndroidIntent.setAction,
                               javaString ("android.intent.action.SEND").get());
        env->CallObjectMethod (intent, AndroidIntent.putExtra,
                               javaString ("android.intent.extra.TEXT").get(),
                               javaString (text).get());
        env->CallObjectMethod (intent, AndroidIntent.setType, javaString ("text/plain").get());

        auto chooserIntent = LocalRef<jobject> (env->CallStaticObjectMethod (AndroidIntent, AndroidIntent.createChooser,
                                                                             intent.get(), javaString ("Choose share target").get()));

        env->CallVoidMethod (android.activity, JuceAppActivity.startActivityForResult, chooserIntent.get(), 1003);
    }

    //==============================================================================
    void cursorClosed (const AndroidContentSharerCursor& cursor) override
    {
        cursors.removeObject (&cursor);
    }

    void fileHandleClosed (const AndroidContentSharerFileObserver&) override
    {
        decrementPendingFileCountAndNotifyOwnerIfReady();
    }

    //==============================================================================
    void* openFile (const LocalRef<jobject>& contentProvider,
                    const LocalRef<jobject>& uri, const LocalRef<jstring>& mode)
    {
        ignoreUnused (mode);

        WeakReference<ContentSharerNativeImpl> weakRef (this);

        if (weakRef == nullptr)
            return nullptr;

        auto* env = getEnv();

        auto uriElements = getContentUriElements (env, uri);

        if (uriElements.filepath.isEmpty())
            return nullptr;

        return getAssetFileDescriptor (env, contentProvider, uriElements.filepath);
    }

    void* query (const LocalRef<jobject>& contentProvider, const LocalRef<jobject>& uri,
                 const LocalRef<jobjectArray>& projection, const LocalRef<jobject>& selection,
                 const LocalRef<jobjectArray>& selectionArgs, const LocalRef<jobject>& sortOrder)
    {
        ignoreUnused (selection, selectionArgs, sortOrder);

        StringArray requestedColumns = javaStringArrayToJuce (projection);
        StringArray supportedColumns = getSupportedColumns();

        StringArray resultColumns;

        for (const auto& col : supportedColumns)
        {
            if (requestedColumns.contains (col))
                resultColumns.add (col);
        }

        // Unsupported columns were queried, file sharing may fail.
        if (resultColumns.isEmpty())
            return nullptr;

        auto resultJavaColumns = juceStringArrayToJava (resultColumns);

        auto* env = getEnv();

        auto cursor = cursors.add (new AndroidContentSharerCursor (*this, env, contentProvider,
                                                                   resultJavaColumns));

        auto uriElements = getContentUriElements (env, uri);

        if (uriElements.filepath.isEmpty())
            return cursor->getNativeCursor();

        auto values = LocalRef<jobjectArray> (env->NewObjectArray ((jsize) resultColumns.size(),
                                                                   JavaObject, 0));

        for (int i = 0; i < resultColumns.size(); ++i)
        {
            if (resultColumns.getReference (i) == "_display_name")
            {
                env->SetObjectArrayElement (values, i, javaString (uriElements.filename).get());
            }
            else if (resultColumns.getReference (i) == "_size")
            {
                auto javaFile = LocalRef<jobject> (env->NewObject (JavaFile, JavaFile.constructor,
                                                                   javaString (uriElements.filepath).get()));

                jlong fileLength = env->CallLongMethod (javaFile, JavaFile.length);

                env->SetObjectArrayElement (values, i, env->NewObject (JavaLong,
                                                                       JavaLong.constructor,
                                                                       fileLength));
            }
        }

        auto nativeCursor = cursor->getNativeCursor();
        env->CallVoidMethod (nativeCursor, JuceContentProviderFileObserverCursor.addRow, values.get());

        return nativeCursor;
    }

    void* getStreamTypes (const LocalRef<jobject>& uri, const LocalRef<jstring>& mimeTypeFilter)
    {
        auto* env = getEnv();

        auto extension = getContentUriElements (env, uri).filename.fromLastOccurrenceOf (".", false, true);

        if (extension.isEmpty())
            return nullptr;

        return juceStringArrayToJava (filterMimeTypes (getMimeTypesForFileExtension (extension),
                                                                  juceString (mimeTypeFilter.get())));
    }

    void sharingFinished (int resultCode)
    {
        sharingActivityDidFinish = true;

        succeeded = resultCode == -1;

        // Give content sharer a chance to request file access.
        if (nonAssetFilesPendingShare.get() == 0)
            startTimer (2000);
        else
            notifyOwnerIfReady();
    }

private:
    bool isContentSharingEnabled() const
    {
        auto* env = getEnv();

        auto packageManager = LocalRef<jobject> (env->CallObjectMethod (android.activity,
                                                                        JuceAppActivity.getPackageManager));

        constexpr int getProviders = 8;
        auto packageInfo = LocalRef<jobject> (env->CallObjectMethod (packageManager,
                                                                     AndroidPackageManager.getPackageInfo,
                                                                     javaString (packageName).get(),
                                                                     getProviders));
        auto providers = LocalRef<jobjectArray> ((jobjectArray) env->GetObjectField (packageInfo,
                                                                                     AndroidPackageInfo.providers));

        if (providers == nullptr)
            return false;

        auto sharingContentProviderAuthority = packageName + ".sharingcontentprovider";
        const int numProviders = env->GetArrayLength (providers.get());

        for (int i = 0; i < numProviders; ++i)
        {
            auto providerInfo = LocalRef<jobject> (env->GetObjectArrayElement (providers, i));
            auto authority = LocalRef<jstring> ((jstring) env->GetObjectField (providerInfo,
                                                                               AndroidProviderInfo.authority));

            if (juceString (authority) == sharingContentProviderAuthority)
                return true;
        }

        return false;
    }

    void handleAsyncUpdate() override
    {
        jassert (prepareFilesThread != nullptr);

        if (prepareFilesThread == nullptr)
            return;

        filesPrepared (prepareFilesThread->getResultFileUris(), prepareFilesThread->getMimeTypes());
    }

    void filesPrepared (jobject fileUris, const StringArray& mimeTypes)
    {
        auto* env = getEnv();

        auto intent = LocalRef<jobject> (env->NewObject (AndroidIntent, AndroidIntent.constructor));
        env->CallObjectMethod (intent, AndroidIntent.setAction,
                               javaString ("android.intent.action.SEND_MULTIPLE").get());

        env->CallObjectMethod (intent, AndroidIntent.setType,
                               javaString (getCommonMimeType (mimeTypes)).get());

        constexpr int grantReadPermission = 1;
        env->CallObjectMethod (intent, AndroidIntent.setFlags, grantReadPermission);

        env->CallObjectMethod (intent, AndroidIntent.putParcelableArrayListExtra,
                               javaString ("android.intent.extra.STREAM").get(),
                               fileUris);

        auto chooserIntent = LocalRef<jobject> (env->CallStaticObjectMethod (AndroidIntent,
                                                                             AndroidIntent.createChooser,
                                                                             intent.get(),
                                                                             javaString ("Choose share target").get()));

        env->CallVoidMethod (android.activity, JuceAppActivity.startActivityForResult, chooserIntent.get(), 1003);
    }

    void decrementPendingFileCountAndNotifyOwnerIfReady()
    {
        --nonAssetFilesPendingShare;

        notifyOwnerIfReady();
    }

    void notifyOwnerIfReady()
    {
        if (sharingActivityDidFinish && nonAssetFilesPendingShare.get() == 0)
            owner.sharingFinished (succeeded, {});
    }

    void timerCallback() override
    {
        stopTimer();

        notifyOwnerIfReady();
    }

    //==============================================================================
    struct ContentUriElements
    {
        String index;
        String filename;
        String filepath;
    };

    ContentUriElements getContentUriElements (JNIEnv* env, const LocalRef<jobject>& uri) const
    {
        jassert (prepareFilesThread != nullptr);

        if (prepareFilesThread == nullptr)
            return {};

        auto fullUri = juceString ((jstring) env->CallObjectMethod (uri.get(), AndroidUri.toString));

        auto index = fullUri.fromFirstOccurrenceOf (uriBase, false, false)
                             .upToFirstOccurrenceOf ("/", false, true);

        auto filename = fullUri.fromLastOccurrenceOf ("/", false, true);

        return { index, filename, prepareFilesThread->getFilePaths()[index.getIntValue()] };
    }

    static StringArray getSupportedColumns()
    {
        return StringArray ("_display_name", "_size");
    }

    void* getAssetFileDescriptor (JNIEnv* env, const LocalRef<jobject>& contentProvider,
                                  const String& filepath)
    {
        // This function can be called from multiple threads.
        {
            const ScopedLock sl (nonAssetFileOpenLock);

            if (! nonAssetFilePathsPendingShare.contains (filepath))
            {
                nonAssetFilePathsPendingShare.add (filepath);
                ++nonAssetFilesPendingShare;

                nonAssetFileObservers.add (new AndroidContentSharerFileObserver (*this, env,
                                                                                 contentProvider,
                                                                                 filepath));
            }
        }

        auto javaFile = LocalRef<jobject> (env->NewObject (JavaFile, JavaFile.constructor,
                                                           javaString (filepath).get()));

        constexpr int modeReadOnly = 268435456;
        auto parcelFileDescriptor = LocalRef<jobject> (env->CallStaticObjectMethod (ParcelFileDescriptor,
                                                                                    ParcelFileDescriptor.open,
                                                                                    javaFile.get(), modeReadOnly));

        auto exception = LocalRef<jobject> (env->ExceptionOccurred());

        if (exception != 0)
        {
            // Failed to create file descriptor. Have you provided a valid file path/resource name?
            jassertfalse;

            env->ExceptionClear();
            return nullptr;
        }

        jlong startOffset = 0;
        jlong unknownLength = -1;

        assetFileDescriptors.add (GlobalRef (LocalRef<jobject> (env->NewObject (AssetFileDescriptor,
                                                                                AssetFileDescriptor.constructor,
                                                                                parcelFileDescriptor.get(),
                                                                                startOffset, unknownLength))));

        return assetFileDescriptors.getReference (assetFileDescriptors.size() - 1).get();
    }

    ContentSharer& owner;
    String packageName;
    String uriBase;

    std::unique_ptr<AndroidContentSharerPrepareFilesThread> prepareFilesThread;

    bool succeeded = false;
    String errorDescription;

    bool sharingActivityDidFinish = false;

    OwnedArray<AndroidContentSharerCursor> cursors;

    Array<GlobalRef> assetFileDescriptors;

    CriticalSection nonAssetFileOpenLock;
    StringArray nonAssetFilePathsPendingShare;
    Atomic<int> nonAssetFilesPendingShare { 0 };
    OwnedArray<AndroidContentSharerFileObserver> nonAssetFileObservers;

    WeakReference<ContentSharerNativeImpl>::Master masterReference;
    friend class WeakReference<ContentSharerNativeImpl>;
};

//==============================================================================
ContentSharer::Pimpl* ContentSharer::createPimpl()
{
    return new ContentSharerNativeImpl (*this);
}

//==============================================================================
void* juce_contentSharerQuery (void* contentProvider, void* uri, void* projection,
                               void* selection, void* selectionArgs, void* sortOrder)
{
    auto* pimpl = (ContentSharer::ContentSharerNativeImpl*) ContentSharer::getInstance()->pimpl.get();
    return pimpl->query (LocalRef<jobject>      (static_cast<jobject> (contentProvider)),
                         LocalRef<jobject>      (static_cast<jobject> (uri)),
                         LocalRef<jobjectArray> (static_cast<jobjectArray> (projection)),
                         LocalRef<jobject>      (static_cast<jobject> (selection)),
                         LocalRef<jobjectArray> (static_cast<jobjectArray> (selectionArgs)),
                         LocalRef<jobject>      (static_cast<jobject> (sortOrder)));
}

void* juce_contentSharerOpenFile (void* contentProvider, void* uri, void* mode)
{
    auto* pimpl = (ContentSharer::ContentSharerNativeImpl*) ContentSharer::getInstance()->pimpl.get();
    return pimpl->openFile (LocalRef<jobject> (static_cast<jobject> (contentProvider)),
                            LocalRef<jobject> (static_cast<jobject> (uri)),
                            LocalRef<jstring> (static_cast<jstring> (mode)));
}

void juce_contentSharingCompleted (int resultCode)
{
    auto* pimpl = (ContentSharer::ContentSharerNativeImpl*) ContentSharer::getInstance()->pimpl.get();
    return pimpl->sharingFinished (resultCode);
}

void* juce_contentSharerGetStreamTypes (void* uri, void* mimeTypeFilter)
{
    auto* pimpl = (ContentSharer::ContentSharerNativeImpl*) ContentSharer::getInstance()->pimpl.get();
    return pimpl->getStreamTypes (LocalRef<jobject> (static_cast<jobject> (uri)),
                                  LocalRef<jstring> (static_cast<jstring> (mimeTypeFilter)));
}

//==============================================================================
JUCE_JNI_CALLBACK (JUCE_ANDROID_SHARING_CONTENT_PROVIDER_CLASSNAME, contentSharerFileObserverEvent, void,
                   (JNIEnv* env, jobject /*fileObserver*/, jlong host, int event, jstring path))
{
    setEnv (env);

    reinterpret_cast<AndroidContentSharerFileObserver*> (host)->onFileEvent (event, LocalRef<jstring> (path));
}

//==============================================================================
JUCE_JNI_CALLBACK (JUCE_ANDROID_SHARING_CONTENT_PROVIDER_CLASSNAME, contentSharerQuery, jobject,
                   (JNIEnv* env, jobject contentProvider, jobject uri, jobjectArray projection,
                    jobject selection, jobjectArray selectionArgs, jobject sortOrder))
{
    setEnv (env);

    return (jobject) juce_contentSharerQuery (contentProvider, uri, projection, selection, selectionArgs, sortOrder);
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_SHARING_CONTENT_PROVIDER_CLASSNAME, contentSharerCursorClosed, void,
                   (JNIEnv* env, jobject /*cursor*/, jlong host))
{
    setEnv (env);

    reinterpret_cast<AndroidContentSharerCursor*> (host)->cursorClosed();
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_SHARING_CONTENT_PROVIDER_CLASSNAME, contentSharerOpenFile, jobject,
                   (JNIEnv* env, jobject contentProvider, jobject uri, jstring mode))
{
    setEnv (env);

    return (jobject) juce_contentSharerOpenFile ((void*) contentProvider, (void*) uri, (void*) mode);
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_SHARING_CONTENT_PROVIDER_CLASSNAME, contentSharerGetStreamTypes, jobject,
                   (JNIEnv* env, jobject /*contentProvider*/, jobject uri, jstring mimeTypeFilter))
{
    setEnv (env);

    return (jobject) juce_contentSharerGetStreamTypes ((void*) uri, (void*) mimeTypeFilter);
}

} // namespace juce
